/*
 * cpl-odbc.cpp
 * Prov-CPL
 *
 * Copyright 2016
 *      The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Contributor(s): Jackson Okuhn, Peter Macko
 */

#include "stdafx.h"
#include "cpl-odbc-private.h"

#include <list>
#include <vector>


// NOTE: The locking is currently too conservative -- we should improve this.
// The reason is that we want to lock a prepare statement while we are using
// it in order to prevent race conditions. Perhaps we can prepare multiple
// instances of the prepared statements, so that they can be used concurrently.


/***************************************************************************/
/** Private API                                                           **/
/***************************************************************************/

/**
 * Get the ODBC error
 *
 * @param handle the failed handle
 * @param type the handle type
 * @param errors the vector to which store the retrieved error records
 */
static void
fetch_odbc_error(SQLHANDLE handle, SQLSMALLINT type,
				 std::vector<cpl_odbc_error_record_t>& errors)
{
	SQLSMALLINT index = 0;
	SQLRETURN ret;

	do {
		cpl_odbc_error_record_t r;
		r.index = ++index;

		ret = SQLGetDiagRec(type, handle, r.index, r.state, &r.native,
							r.text, sizeof(r.text), &r.length);
		
		if (SQL_SUCCEEDED(ret)) {
			errors.push_back(r);
		}
		else if (ret != SQL_NO_DATA) {
			if (ret == SQL_ERROR) {
				fprintf(stderr, "SQLGetDiagRec failed with error SQL_ERROR\n");
			}
			else if (ret == SQL_INVALID_HANDLE) {
				fprintf(stderr, "SQLGetDiagRec failed with error "
								"SQL_INVALID_HANDLE\n");
			}
			else {
				fprintf(stderr, "SQLGetDiagRec failed with error code %ld\n",
						(long) ret);
			}
		}
	}
	while (ret == SQL_SUCCESS);
}


/**
 * Print the ODBC error to stderr
 *
 * @param fn the function that failed
 * @param errors the vector of error records to print
 */
static void
print_odbc_error(const char *fn, std::vector<cpl_odbc_error_record_t>& errors)
{
	fprintf(stderr, "\nThe ODBC driver reported the following while running "
			"%s:\n", fn);

	for (size_t i = 0; i < errors.size(); i++) {
		fprintf(stderr, "  %s:%ld:%ld:%s\n",
				errors[i].state, (long) errors[i].index,
				(long) errors[i].native, errors[i].text);
	}

	if (errors.empty()) {
		fprintf(stderr, "  (no errors returned)\n");
	}

	fprintf(stderr, "\n");
}


/**
 * Print the ODBC error to stderr
 *
 * @param fn the function that failed
 * @param handle the failed handle
 * @param type the handle type
 */
static void
print_odbc_error(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
	std::vector<cpl_odbc_error_record_t> errors;
	fetch_odbc_error(handle, type, errors);
	print_odbc_error(fn, errors);
}


/**
 * Print the ODBC error to stderr
 *
 * @param errors the vector of error records
 * @return boolean if should reconnect
 */
static bool
should_reconnect_due_to_odbc_error(std::vector<cpl_odbc_error_record_t>& errors)
{
	if (errors.empty()) return false;
	if (errors.size() != 1) return false;
	if (strcmp((const char*) errors[0].state, "08S01") == 0) return true;
	return false;
}


/**
 * If the variable ret is an error, print the SQL error
 * and goto the given label
 *
 * @param function the function for which the error is checked
 * @param handle the statement handle
 * @param error the label to jump to on error
 */
#define SQL_ASSERT_NO_ERROR(function, handle, error) { \
	if (!SQL_SUCCEEDED(ret)) { \
		print_odbc_error(#function, handle, SQL_HANDLE_STMT); \
		goto error; \
	}}


/**
 * Start a block of code that uses SQL_EXECUTE and SQL_EXECUTE_EXT
 */
#define SQL_START \
	SQLRETURN ret; \
	int retries_left = 3;


/**
 * Execute the prepared statement and handle the error, if any
 *
 * @param handle the statement handle
 * @param retry the label to jump to on retry
 * @param error the label to jump to on error
 */
#define SQL_EXECUTE_EXT(handle, retry, error) { \
	ret = SQLExecute(handle); \
	if (!SQL_SUCCEEDED(ret)) { \
		std::vector<cpl_odbc_error_record_t> errors; \
		fetch_odbc_error(handle, SQL_HANDLE_STMT, errors); \
		if (should_reconnect_due_to_odbc_error(errors)) { \
			if (retries_left-- > 0) { \
				cpl_return_t ____r = cpl_odbc_reconnect(odbc); \
				if (CPL_IS_OK(____r)) goto retry; \
			} \
		} \
		print_odbc_error("SQLExecute", errors); \
		goto error; \
	}}


/**
 * Execute the prepared statement and handle the error, if any
 *
 * @param handle the statement handle
 */
#define SQL_EXECUTE(handle) \
	SQL_EXECUTE_EXT(handle, retry, err);


/**
 * Read a single value from the result set. Close the cursor on error,
 * or if configured to do so (which is the default), also on success
 *
 * @param stmt the statement handle
 * @param type the target variable type
 * @param out the pointer to the variable to store the output
 * @param buffer_length the buffer length, or 0 for fixed-size C types
 * @param column the column number
 * @param fetch whether to fetch the new row
 * @param close_if_ok whether to close the cursor on no error
 * @param handle_nulls whether to handle null values specially
 * @return CPL_OK if okay, depending on handle_nulls CPL_E_DB_NULL (if true)
 *         or CPL_E_NOT_FOUND (if false) if empty or NULL, or an error code
 */
static cpl_return_t
cpl_sql_fetch_single_value(SQLHSTMT stmt, SQLSMALLINT type, void* out,
						   size_t buffer_length, int column=1,
		                   bool fetch=true, bool close_if_ok=true,
						   bool handle_nulls=false)
{
	SQLLEN cb = 0;
	SQLRETURN ret = 0;
	assert(out != NULL);

	if (fetch) {
		ret = SQLFetch(stmt);
		if (ret == SQL_NO_DATA) goto err_nf;
		SQL_ASSERT_NO_ERROR(SQLFetch, stmt, err);
	}
	
	ret = SQLGetData(stmt, column, type, out, buffer_length, &cb);
	if (ret == SQL_NO_DATA) goto err_null;
	SQL_ASSERT_NO_ERROR(SQLGetData, stmt, err);
	if (cb <= 0) goto err_null;

	if (close_if_ok) {
		ret = SQLCloseCursor(stmt);
		SQL_ASSERT_NO_ERROR(SQLCloseCursor, stmt, err);
	}

	return CPL_OK;


	// Error handling

err:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}
	return CPL_E_STATEMENT_ERROR;

err_nf:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}
	return CPL_E_NOT_FOUND;

err_null:
	if (!handle_nulls) {
		ret = SQLCloseCursor(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		}
	}
	return handle_nulls ? CPL_E_DB_NULL : CPL_E_NOT_FOUND;
}


/**
 * Read a single number from the result set. Close the cursor on error,
 * or if configured to do so (which is the default), also on success
 *
 * @param stmt the statement handle
 * @param out the pointer to the variable to store the output
 * @param column the column number
 * @param fetch whether to fetch the new row
 * @param close_if_ok whether to close the cursor on no error
 * @param handle_nulls whether to handle null values specially
 * @return CPL_OK if okay, depending on handle_nulls CPL_E_DB_NULL (if true)
 *         or CPL_E_NOT_FOUND (if false) if empty or NULL, or an error code
 */
static cpl_return_t
cpl_sql_fetch_single_llong(SQLHSTMT stmt, long long* out, int column=1,
		                   bool fetch=true, bool close_if_ok=true,
						   bool handle_nulls=false)
{
	long long l = 0;

	cpl_return_t r = cpl_sql_fetch_single_value(stmt, SQL_C_SBIGINT,
			&l, 0, column, fetch, close_if_ok, handle_nulls);
	if (!CPL_IS_OK(r)) return r;

	if (out != NULL) *out = l;
	return CPL_OK;
}

/**
 * Read a single number from the result set. Close the cursor on error,
 * or if configured to do so (which is the default), also on success
 *
 * @param stmt the statement handle
 * @param out the pointer to the variable to store the output
 * @param column the column number
 * @param fetch whether to fetch the new row
 * @param close_if_ok whether to close the cursor on no error
 * @param handle_nulls whether to handle null values specially
 * @return CPL_OK if okay, depending on handle_nulls CPL_E_DB_NULL (if true)
 *         or CPL_E_NOT_FOUND (if false) if empty or NULL, or an error code
 */
static cpl_return_t
cpl_sql_fetch_single_int(SQLHSTMT stmt, int* out, int column=1,
		                   bool fetch=true, bool close_if_ok=true,
						   bool handle_nulls=false)
{
	int i = 0;

	cpl_return_t r = cpl_sql_fetch_single_value(stmt, SQL_C_SLONG,
			&i, 0, column, fetch, close_if_ok, handle_nulls);
	if (!CPL_IS_OK(r)) return r;

	if (out != NULL) *out = i;
	return CPL_OK;
}

/**
 * Convert a SQL timestamp to UNIX time
 *
 * @param t the timestamp
 * @return UNIX time
 */
static unsigned long
cpl_sql_timestamp_to_unix_time(const SQL_TIMESTAMP_STRUCT& t)
{
	struct tm m;
	m.tm_year = t.year - 1900;
	m.tm_mon = t.month - 1;
	m.tm_mday = t.day;
	m.tm_hour = t.hour;
	m.tm_min = t.minute;
	m.tm_sec = t.second;
	m.tm_wday = 0;
	m.tm_yday = 0;
	m.tm_isdst = 0;
	time_t T = mktime(&m);
	struct tm mx;
	localtime_r(&T, &mx);
	if (mx.tm_isdst) T -= 3600;
	return (unsigned long) T;
}


/**
 * Read a timestamp from the result set and return it as UNIX time. Close
 * the cursor on error, or if configured to do so (which is the default),
 * also on success
 *
 * @param stmt the statement handle
 * @param out the pointer to the variable to store the output
 * @param column the column number
 * @param fetch whether to fetch the new row
 * @param close_if_ok whether to close the cursor on no error
 * @param handle_nulls whether to handle null values specially
 * @return CPL_OK if okay, depending on handle_nulls CPL_E_DB_NULL (if true)
 *         or CPL_E_NOT_FOUND (if false) if empty or NULL, or an error code
 */
static cpl_return_t
cpl_sql_fetch_single_timestamp_as_unix_time(SQLHSTMT stmt, unsigned long* out,
						int column=1, bool fetch=true, bool close_if_ok=true,
						bool handle_nulls=false)
{
	SQL_TIMESTAMP_STRUCT t;

	cpl_return_t r = cpl_sql_fetch_single_value(stmt, SQL_C_TYPE_TIMESTAMP,
			&t, sizeof(t), column, fetch, close_if_ok, handle_nulls);
	if (!CPL_IS_OK(r)) return r;

	if (out != NULL) *out = cpl_sql_timestamp_to_unix_time(t);
	return CPL_OK;
}


/**
 * Read a single string from the result set. Close the cursor on error,
 * or if configured to do so (which is the default), also on success. The
 * returned string would need to be freed using free().
 *
 * @param stmt the statement handle
 * @param out the pointer to the variable to store the output
 * @param column the column number
 * @param fetch whether to fetch the new row
 * @param close_if_ok whether to close the cursor on no error
 * @param handle_nulls whether to handle null values specially
 * @param max_length the maximum string length (not including the string
 *                   termination character)
 * @return CPL_OK if okay, depending on handle_nulls CPL_E_DB_NULL (if true)
 *         or CPL_E_NOT_FOUND (if false) if empty or NULL, or an error code
 */
static cpl_return_t
cpl_sql_fetch_single_dynamically_allocated_string(SQLHSTMT stmt, char** out,
						   int column=1, bool fetch=true, bool close_if_ok=true,
						   bool handle_nulls=false, size_t max_length=4095)
{
	char* str = (char*) malloc(max_length + 1);
	if (str == NULL) return CPL_E_INSUFFICIENT_RESOURCES;

	cpl_return_t r = cpl_sql_fetch_single_value(stmt, SQL_C_CHAR,
			str, max_length + 1, column, fetch, close_if_ok, handle_nulls);
	if (!CPL_IS_OK(r)) {
		free(str);
		if (out != NULL && handle_nulls && r == CPL_E_DB_NULL) *out = NULL;
		return r;
	}

	if (out != NULL) {
		*out = str;
	}
	else {
		free(str);
	}
	return CPL_OK;
}


/**
 * Read a single value from the result set of the statement stmt. If the column
 * number is 1, do fetch. Store the return code in variable cpl_return_t r.
 * Close the result set on error and jump to err_r.
 *
 * @param type the output variable / fetch operation type
 * @param column the column number
 * @param out the outputpointer
 * @param handle_nulls whether to handle nulls specially; if true and the value
 *                     is null, r would be CPL_E_DB_NULL, and the macro would
 *                     succeed
 */
#define CPL_SQL_SIMPLE_FETCH_EXT(type, column, out, handle_nulls) { \
	int __c = (column); \
	r = cpl_sql_fetch_single_ ## type(stmt, (out), __c, __c == 1, false, \
									  handle_nulls); \
	if (r != CPL_E_DB_NULL /* possible only if handle_nulls is true */ \
			&& !CPL_IS_OK(r)) goto err_r; \
}


/**
 * Read a single value from the result set of the statement stmt. If the column
 * number is 1, do fetch. Store the return code in variable cpl_return_t r.
 * Close the result set on error and jump to err_r.
 *
 * @param type the output variable / fetch operation type
 * @param column the column number
 * @param out the outputpointer
 */
#define CPL_SQL_SIMPLE_FETCH(type, column, out) \
	CPL_SQL_SIMPLE_FETCH_EXT(type, column, out, false);



/***************************************************************************/
/** Constructors and a Destructor: Helpers                                **/
/***************************************************************************/

/**
 * Free the statement handles
 *
 * @param odbc an initialized backend structure
 */
static void
cpl_odbc_free_statement_handles(cpl_odbc_t* odbc)
{
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_session_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_bundle_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_nt_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_nb_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_ntnb_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_ext_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_nt_ext_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_nb_ext_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_ntnb_ext_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_relation_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_object_property_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_relation_property_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_session_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_all_objects_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_all_objects_with_session_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_ancestors_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_descendants_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_properties_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_properties_with_key_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_by_property_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_relation_properties_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_relation_properties_with_key_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->delete_bundle_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_bundle_objects_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_bundle_relations_stmt);

}


/**
 * Connect to a database using ODBC
 *
 * @param odbc an initialized backend structure
 * @return the error code
 */
static cpl_return_t
cpl_odbc_connect(cpl_odbc_t* odbc)
{
	cpl_return_t r = CPL_OK;
	const char* connection_string = odbc->connection_string.c_str();


	// Open the ODBC connection

	SQLRETURN ret;
	SQLCHAR outstr[1024];
	SQLCHAR* connection_string_copy;
	SQLSMALLINT outstrlen;
	size_t l_connection_string = strlen(connection_string);

	connection_string_copy = (SQLCHAR*) malloc(l_connection_string + 4);
	if (connection_string_copy == NULL) {
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	strcpy((char*) connection_string_copy, connection_string);
	connection_string_copy[l_connection_string] = '\0';

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc->db_environment);
	SQLSetEnvAttr(odbc->db_environment,
				  SQL_ATTR_ODBC_VERSION,
				  (void *) SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, odbc->db_environment, &odbc->db_connection);
	
	ret = SQLDriverConnect(odbc->db_connection, NULL,
						   connection_string_copy, l_connection_string,
						   outstr, sizeof(outstr), &outstrlen,
						   SQL_DRIVER_NOPROMPT /*SQL_DRIVER_COMPLETE*/);
	free(connection_string_copy);
	connection_string_copy = NULL;

	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLDriverConnect",
						 odbc->db_connection, SQL_HANDLE_DBC);
		r = CPL_E_DB_CONNECTION_ERROR;
		goto err_handles;
	}


	// Allocate the statement handles

#define ALLOC_STMT(handle) \
	SQLAllocHandle(SQL_HANDLE_STMT, odbc->db_connection, &odbc->handle);
	
	ALLOC_STMT(create_session_insert_stmt);
	ALLOC_STMT(create_object_insert_stmt);
	ALLOC_STMT(create_object_insert_bundle_stmt);
	ALLOC_STMT(lookup_object_stmt);
	ALLOC_STMT(lookup_object_nt_stmt);
	ALLOC_STMT(lookup_object_nb_stmt);
	ALLOC_STMT(lookup_object_ntnb_stmt);
	ALLOC_STMT(lookup_object_ext_stmt);
	ALLOC_STMT(lookup_object_ext_stmt);
	ALLOC_STMT(lookup_object_nt_ext_stmt);
	ALLOC_STMT(lookup_object_nb_ext_stmt);
	ALLOC_STMT(lookup_object_ntnb_ext_stmt);
	ALLOC_STMT(add_relation_stmt);
	ALLOC_STMT(add_object_property_stmt);
	ALLOC_STMT(add_relation_property_stmt);
	ALLOC_STMT(get_session_info_stmt);
	ALLOC_STMT(get_all_objects_stmt);
	ALLOC_STMT(get_all_objects_with_session_stmt);
	ALLOC_STMT(get_object_info_stmt);
	ALLOC_STMT(get_object_ancestors_stmt);
	ALLOC_STMT(get_object_descendants_stmt);
	ALLOC_STMT(get_object_properties_stmt);
	ALLOC_STMT(get_object_properties_with_key_stmt);
	ALLOC_STMT(lookup_object_by_property_stmt);
	ALLOC_STMT(get_relation_properties_stmt);
	ALLOC_STMT(get_relation_properties_with_key_stmt);
	ALLOC_STMT(has_immediate_ancestor_stmt);
	ALLOC_STMT(delete_bundle_stmt);
	ALLOC_STMT(get_bundle_objects_stmt);
	ALLOC_STMT(get_bundle_relations_stmt);
#undef ALLOC_STMT


	// Prepare the statements

#define PREPARE(handle, text) { \
	ret = SQLPrepare(odbc->handle, (SQLCHAR*) text, SQL_NTS); \
	if (!SQL_SUCCEEDED(ret)) { \
		r = CPL_E_PREPARE_STATEMENT_ERROR; \
		print_odbc_error("SQLPrepare", odbc->handle, SQL_HANDLE_STMT); \
		goto err_stmts; \
	}}

	PREPARE(create_session_insert_stmt,
			"INSERT INTO cpl_sessions"
			"            (id, mac_address, username, pid, program,"
			"             cmdline)"
			"     VALUES (DEFAULT, ?, ?, ?, ?, ?)"
			"   RETURNING id;");

	PREPARE(create_object_insert_stmt,
			"INSERT INTO cpl_objects"
			"            (id, originator, name, type,"
			"			  session_id)"
			"     VALUES (DEFAULT, ?, ?, ?, ?)"
			"   RETURNING id;");

	//TODO potentially figure out ON CONFLICT logic
	PREPARE(create_object_insert_bundle_stmt,
			"INSERT INTO cpl_objects"
			"            (id, originator, name, type,"
			"             session_id,"
			"             bundle_id)"
			"     VALUES (DEFAULT, ?, ?, ?, ?, ?)"
			"   RETURNING id;");

	//TODO try merging with "where column is not null"
	PREPARE(lookup_object_stmt,
			"SELECT id"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ? AND bundle_id = ?"
			" ORDER BY creation_time DESC"
			" LIMIT 1;");

	PREPARE(lookup_object_nt_stmt,
			"SELECT id"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND bundle_id = ?"
			" ORDER BY creation_time DESC"
			" LIMIT 1;");

	PREPARE(lookup_object_nb_stmt,
			"SELECT id"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ?"
			" ORDER BY creation_time DESC"
			" LIMIT 1;");

	PREPARE(lookup_object_ntnb_stmt,
			"SELECT id"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ?"
			" ORDER BY creation_time DESC"
			" LIMIT 1;");

	PREPARE(lookup_object_ext_stmt,
			"SELECT id, creation_time"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ?;");

	PREPARE(lookup_object_nt_ext_stmt,
			"SELECT id, creation_time"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND bundle_id = ?;");

	PREPARE(lookup_object_nb_ext_stmt,
			"SELECT id, creation_time"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ?;");

	PREPARE(lookup_object_ntnb_ext_stmt,
			"SELECT id, creation_time"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ? AND bundle_id = ?;");

	//TODO do we want to explicitly handle conflicts
	PREPARE(add_relation_stmt,
			"INSERT INTO cpl_relations"
			"            (id, from_id,"
			"             to_id, type, bundle_id)"
			"     VALUES (DEFAULT, ?, ?, ?, ?)"
			"   RETURNING id;");

	PREPARE(add_object_property_stmt,
			"INSERT INTO cpl_object_properties"
			"            (id, name, value)"
			"     VALUES (?, ?, ?);");

	PREPARE(add_relation_property_stmt,
		"INSERT INTO cpl_relation_properties"
		"            (id, name, value)"
		"     VALUES (?, ?, ?);");

	PREPARE(get_all_objects_stmt,
			"SELECT id, creation_time, originator, name, type,"
			"       bundle_id"
			"  FROM cpl_objects;");

	PREPARE(get_all_objects_with_session_stmt,
			"SELECT id, creation_time, originator, name, type,"
			"       bundle_id, session_id"
			"  FROM cpl_objects");

	PREPARE(get_object_info_stmt,
			"SELECT session_id,"
			"       creation_time, originator, name, type,"
			"       bundle_id"
			"  FROM cpl_objects"
			" WHERE id = ?"
			" LIMIT 1;");

	PREPARE(get_session_info_stmt,
			"SELECT mac_address, username,"
			"       pid, program, cmdline, initialization_time"
			"  FROM cpl_sessions"
			" WHERE id = ?"
			" LIMIT 1;");

	PREPARE(get_object_ancestors_stmt,
			"SELECT id, to_id, type, bundle_id"
			"  FROM cpl_relations"
			" WHERE from_id = ?");

	PREPARE(get_object_descendants_stmt,
			"SELECT id, from_id, type, bundle_id"
			"  FROM cpl_relations"
			" WHERE to_id = ?");

	PREPARE(get_object_properties_stmt,
			"SELECT id, name, value"
			"  FROM cpl_object_properties"
			" WHERE id = ?;");

	PREPARE(get_object_properties_with_key_stmt,
			"SELECT id, name, value"
			"  FROM cpl_object_properties"
			" WHERE id = ? AND name = ?;");

	PREPARE(lookup_object_by_property_stmt,
			"SELECT id"
			"  FROM cpl_object_properties"
			" WHERE name = ? AND value = ?;");

	PREPARE(get_relation_properties_stmt,
			"SELECT id, name, value"
			" FROM cpl_relation_properties"
			" WHERE id = ?");

	PREPARE(get_relation_properties_with_key_stmt,
			"SELECT id, name, value"
			"  FROM cpl_relation_properties"
			" WHERE id = ? AND name = ?;");

	PREPARE(has_immediate_ancestor_stmt,
			"SELECT id"
			"  FROM cpl_relations"
			" WHERE from_id = ? AND to_id = ?;");

	PREPARE(delete_bundle_stmt,
			"DELETE FROM cpl_objects"
			"	WHERE id = ? AND type = 4;");

	PREPARE(get_bundle_objects_stmt,
			"SELECT id, creation_time, originator, name, type"
			"  FROM cpl_objects"
			" WHERE bundle_id = ?;")

	PREPARE(get_bundle_relations_stmt,
			"SELECT id, from_id, to_id, type"
			"  FROM cpl_relations"
			" WHERE bundle_id = ?;")
#undef PREPARE


	// Return

	return CPL_OK;


	// Error handling -- the variable r must be set

err_stmts:
	cpl_odbc_free_statement_handles(odbc);
	SQLDisconnect(odbc->db_connection);

err_handles:
	SQLFreeHandle(SQL_HANDLE_DBC, odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, odbc->db_environment);

	return r;
}


/**
 * Disconnect from a database
 *
 * @param odbc the backend structure
 * @return the error code
 */
static cpl_return_t cpl_odbc_disconnect(cpl_odbc_t* odbc)
{
	cpl_return_t r = CPL_OK;

	cpl_odbc_free_statement_handles(odbc);

	SQLRETURN ret = SQLDisconnect(odbc->db_connection);
	if (!SQL_SUCCEEDED(ret)) {
		r = CPL_E_DB_CONNECTION_ERROR;
	}

	SQLFreeHandle(SQL_HANDLE_DBC, odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, odbc->db_environment);

	return r;
}


/**
 * Reconnect
 *
 * @param odbc the backend structure
 * @return the error code
 */
static cpl_return_t
cpl_odbc_reconnect(cpl_odbc_t* odbc)
{
	cpl_odbc_disconnect(odbc);
	return cpl_odbc_connect(odbc);
}



/***************************************************************************/
/** Constructors and a Destructor                                         **/
/***************************************************************************/


/**
 * Create an ODBC backend
 *
 * @param connection_string the ODBC connection string
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_odbc_backend(const char* connection_string,
						int db_type,
						cpl_db_backend_t** out)
{
	cpl_return_t r = CPL_OK;

	assert(out != NULL);
	assert(connection_string != NULL);


	// Allocate the backend struct

	cpl_odbc_t* odbc = new cpl_odbc_t;
	if (odbc == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memcpy(&odbc->backend, &CPL_ODBC_BACKEND, sizeof(odbc->backend));
	odbc->db_type = db_type;
	odbc->connection_string = connection_string;


	// Initialize the synchronization primitives

	mutex_init(odbc->create_session_lock);
	mutex_init(odbc->create_object_lock);
	mutex_init(odbc->lookup_object_lock);
	mutex_init(odbc->lookup_object_ext_lock);
	mutex_init(odbc->add_relation_lock);
	mutex_init(odbc->add_object_property_lock);
	mutex_init(odbc->add_relation_property_lock);
	mutex_init(odbc->get_session_info_lock);
	mutex_init(odbc->get_all_objects_lock);
	mutex_init(odbc->get_object_info_lock);
	mutex_init(odbc->get_object_relations_lock);
	mutex_init(odbc->get_object_properties_lock);
	mutex_init(odbc->lookup_object_by_property_lock);
	mutex_init(odbc->get_relation_properties_lock);
	mutex_init(odbc->has_immediate_ancestor_lock);
	mutex_init(odbc->delete_bundle_lock);
	mutex_init(odbc->get_bundle_objects_lock);
	mutex_init(odbc->get_bundle_relations_lock);

	// Open the database connection
	
	r = cpl_odbc_connect(odbc);
	if (!CPL_IS_OK(r)) goto err_sync;


	// Return

	*out = (cpl_db_backend_t*) odbc;
	return CPL_OK;


	// Error handling -- the variable r must be set

err_sync:
	mutex_destroy(odbc->create_session_lock);
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->lookup_object_ext_lock);
	mutex_destroy(odbc->add_relation_lock);
	mutex_destroy(odbc->add_object_property_lock);
	mutex_destroy(odbc->add_relation_property_lock);
	mutex_destroy(odbc->get_session_info_lock);
	mutex_destroy(odbc->get_all_objects_lock);
	mutex_destroy(odbc->get_object_info_lock);
	mutex_destroy(odbc->get_object_relations_lock);
	mutex_destroy(odbc->get_object_properties_lock);
	mutex_destroy(odbc->lookup_object_by_property_lock);
	mutex_destroy(odbc->get_relation_properties_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);
	mutex_destroy(odbc->delete_bundle_lock);
	mutex_destroy(odbc->get_bundle_objects_lock);
	mutex_destroy(odbc->get_bundle_relations_lock);

	delete odbc;
	return r;
}


/**
 * Create an ODBC backend
 *
 * @param dsn the data source name
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_odbc_backend_dsn(const char* dsn,
							int db_type,
							cpl_db_backend_t** out)
{
	assert(out != NULL);
	assert(dsn != NULL);

	*out = NULL;


	// Check the DSN

	if (strchr(dsn, ';') != NULL
			|| strchr(dsn, '{') != NULL
			|| strchr(dsn, '}') != NULL) {
		return CPL_E_INVALID_ARGUMENT;
	}


	// Create the connection string

	std::string conn = "DSN=";
	conn += dsn;
	conn += ";";


	// Create the backend
	
	return cpl_create_odbc_backend(conn.c_str(), db_type, out);
}


/**
 * Destructor. If the constructor allocated the backend structure, it
 * should be freed by this function
 *
 * @param backend the pointer to the backend structure
 * @return the error code
 */
extern "C" cpl_return_t
cpl_odbc_destroy(struct _cpl_db_backend_t* backend)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	cpl_return_t r = cpl_odbc_disconnect(odbc);
	if (!CPL_IS_OK(r)) {
		fprintf(stderr, "Warning: Could not terminate the ODBC connection.\n");
	}

	mutex_destroy(odbc->create_session_lock);
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->lookup_object_ext_lock);
	mutex_destroy(odbc->add_relation_lock);
	mutex_destroy(odbc->add_object_property_lock);
	mutex_destroy(odbc->add_relation_property_lock);
	mutex_destroy(odbc->get_session_info_lock);
	mutex_destroy(odbc->get_all_objects_lock);
	mutex_destroy(odbc->get_object_info_lock);
	mutex_destroy(odbc->get_object_relations_lock);
	mutex_destroy(odbc->get_object_properties_lock);
	mutex_destroy(odbc->lookup_object_by_property_lock);
	mutex_destroy(odbc->get_relation_properties_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);
	mutex_destroy(odbc->get_bundle_objects_lock);
	mutex_destroy(odbc->get_bundle_relations_lock);
	
	delete odbc;
	
	return CPL_OK;
}



/***************************************************************************/
/** Helpers for Binding                                                   **/
/***************************************************************************/


/**
 * Bind a VARCHAR parameter. Jump to "err" on error. Variable "ret" must be
 * already defined.
 *
 * @param stmt the statement
 * @param arg the argument number
 * @param size the VARCHAR size
 * @param value the char* value
 */
#define SQL_BIND_VARCHAR(stmt, arg, size, value) { \
	SQLLEN* cb = (SQLLEN*) alloca(sizeof(SQLLEN)); \
	*cb = (value) == NULL ? SQL_NULL_DATA : SQL_NTS; \
	ret = SQLBindParameter(stmt, arg, SQL_PARAM_INPUT, \
			SQL_C_CHAR, SQL_VARCHAR, size, 0, \
			(SQLCHAR*) (value), 0, cb); \
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err); \
}


/**
 * Bind an INTEGER parameter. Jump to "err" on error. Variable "ret" must be
 * already defined.
 *
 * @param stmt the statement
 * @param arg the argument number
 * @param value the int value
 */
#define SQL_BIND_INTEGER(stmt, arg, value) { \
	long long* __p = (long long*) alloca(sizeof(long long)); \
	*__p = (long long) value; \
	ret = SQLBindParameter(stmt, arg, SQL_PARAM_INPUT, \
			SQL_C_SBIGINT, SQL_INTEGER, 0, 0, \
			(void*) __p, 0, NULL); \
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err); \
}



/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


/**
 * Create a session.
 *
 * @param backend the pointer to the backend structure
 * @param session the session ID to use
 * @param mac_address human-readable MAC address (NULL if not available)
 * @param user the user name
 * @param pid the process ID
 * @param program the program name
 * @param cmdline the command line
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_create_session(struct _cpl_db_backend_t* backend,
						cpl_session_t* out_id,
						const char* mac_address,
						const char* user,
						const int pid,
						const char* program,
						const char* cmdline)
{
	assert(backend != NULL && user != NULL && program != NULL && cmdline!=NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->create_session_lock);

	
	// Bind the statement parameters

	SQL_START;

	cpl_id_t id = CPL_NONE;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

retry:
	SQLHSTMT stmt = odbc->create_session_insert_stmt;

	SQL_BIND_VARCHAR(stmt, 1, MAC_ADDR_LEN, mac_address);
	SQL_BIND_VARCHAR(stmt, 2, USER_LEN, user);
	SQL_BIND_INTEGER(stmt, 3, pid);
	SQL_BIND_VARCHAR(stmt, 4, PROGRAM_LEN, program);
	SQL_BIND_VARCHAR(stmt, 5, CMDLINE_LEN, cmdline);


	// Insert the new row to the sessions table

	SQL_EXECUTE(stmt);

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id, 1);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->create_session_lock);
		return r;
	}

	// Finish

	mutex_unlock(odbc->create_session_lock);
	if (out_id != NULL) *out_id = id;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->create_session_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Create an object.
 *
 * @param backend the pointer to the backend structure
 * @param originator the originator
 * @param name the object name
 * @param type the object type
 * @param bundle the ID of the object that should contain this object
 *                  (use CPL_NONE for no bundle)
 * @param session the session ID responsible for this provenance record
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_create_object(struct _cpl_db_backend_t* backend,
					   const char* originator,
					   const char* name,
					   const int type,
					   const cpl_id_t bundle,
					   const cpl_session_t session,
					   cpl_id_t* out_id)
{
	assert(backend != NULL && originator != NULL
			&& name != NULL && CPL_IS_OBJECT_TYPE(type)); 
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	mutex_lock(odbc->create_object_lock);
	
	// Bind the statement parameters

	SQL_START;

	cpl_id_t id = CPL_NONE;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

retry:
	SQLHSTMT stmt = bundle == CPL_NONE
		? odbc->create_object_insert_stmt
		: odbc->create_object_insert_bundle_stmt;

	SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
	SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
	SQL_BIND_INTEGER(stmt, 3, type);
	SQL_BIND_INTEGER(stmt, 4, session);

	if (bundle != CPL_NONE) {
		SQL_BIND_INTEGER(stmt, 5, bundle);
	}


	// Insert the new row to the objects table

	SQL_EXECUTE(stmt);
	

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id, 1);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->create_session_lock);
		return r;
	}
	// Finish

	mutex_unlock(odbc->create_object_lock);

	if (out_id != NULL) *out_id = id;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->create_object_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get one.
 *
 * @param backend the pointer to the backend structure
 * @param originator the object originator (namespace)
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_lookup_object(struct _cpl_db_backend_t* backend,
					   const char* originator,
					   const char* name,
					   const int type,
					   const cpl_id_t bundle_id,
					   cpl_id_t* out_id)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQL_START;

	cpl_id_t id = CPL_NONE;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->lookup_object_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;

	if(type == 0){
		if(bundle_id == CPL_NONE){
			stmt = odbc->lookup_object_ntnb_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);

		} else {
			stmt = odbc->lookup_object_nt_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, bundle_id);

		}
	} else {
		if(bundle_id == CPL_NONE){
			stmt = odbc->lookup_object_nb_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, type);

		} else {
			stmt = odbc->lookup_object_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, type);
			SQL_BIND_INTEGER(stmt, 4, bundle_id);

		}
	}


	// Execute
	
	SQL_EXECUTE(stmt);


	// Fetch the result

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id, 1);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->lookup_object_lock);
		return r;
	}


	// Cleanup

	mutex_unlock(odbc->lookup_object_lock);
	
	if (out_id != NULL) *out_id = id;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->lookup_object_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get all of them
 *
 * @param backend the pointer to the backend structure
 * @param originator the object originator (namespace)
 * @param name the object name
 * @param type the object type
 * @param flags a logical combination of CPL_L_* flags (currently unused)
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_lookup_object_ext(struct _cpl_db_backend_t* backend,
						   const char* originator,
						   const char* name,
						   const int type,
					       const cpl_id_t bundle_id,
						   const int flags,
						   cpl_id_timestamp_iterator_t callback,
						   void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	cpl_id_timestamp_t entry;
	std::list<cpl_id_timestamp_t> entries;
	SQL_TIMESTAMP_STRUCT t;

	mutex_lock(odbc->lookup_object_ext_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;

	if(type == 0){
		if(bundle_id == CPL_NONE){
			stmt = odbc->lookup_object_ntnb_ext_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);

		} else {
			stmt = odbc->lookup_object_nt_ext_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, bundle_id);

		}
	} else {
		if(bundle_id == CPL_NONE){
			stmt = odbc->lookup_object_nb_ext_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, type);

		} else {
			stmt = odbc->lookup_object_ext_stmt;

			SQL_BIND_VARCHAR(stmt, 1, ORIGINATOR_LEN, originator);
			SQL_BIND_VARCHAR(stmt, 2, NAME_LEN, name);
			SQL_BIND_INTEGER(stmt, 3, type);
			SQL_BIND_INTEGER(stmt, 4, bundle_id);
			
		}
	}


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_TYPE_TIMESTAMP, &t, sizeof(t), NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		// Convert timestamp

		entry.timestamp = cpl_sql_timestamp_to_unix_time(t);
		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->lookup_object_ext_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_E_NOT_FOUND;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<cpl_id_timestamp_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(i->id, i->timestamp, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->lookup_object_ext_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Add a provenance relation
 *
 * @param backend the pointer to the backend structure
 * @param from_id the relation source ID
 * @param to_id the relation destination ID
 * @param type the relation type
 * @param out_id the pointer to store the relation ID
 * @return the error code
 */
extern "C" cpl_return_t
cpl_odbc_add_relation(struct _cpl_db_backend_t* backend,
						   const cpl_id_t from_id,
						   const cpl_id_t to_id,
						   const int type,
						   const cpl_id_t bundle,
						   cpl_id_t* out_id)
{
	assert(backend != NULL && from_id != CPL_NONE && CPL_IS_RELATION_TYPE(type));
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;


	// Prepare the statement

	SQL_START;

	cpl_id_t id = CPL_NONE;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->add_relation_lock);

retry:
	SQLHSTMT stmt = odbc->add_relation_stmt;

	SQL_BIND_INTEGER(stmt, 1, from_id);
	SQL_BIND_INTEGER(stmt, 2, to_id);
	SQL_BIND_INTEGER(stmt, 3, type);
	SQL_BIND_INTEGER(stmt, 4, bundle);


	// Execute
	
	SQL_EXECUTE(stmt);

	// Fetch the result

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id, 1);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->add_relation_lock);
		return r;
	}

	// Cleanup

	mutex_unlock(odbc->add_relation_lock);
	if (out_id != NULL) *out_id = id;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->add_relation_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Determine whether the given object has the given ancestor
 * 
 * @param backend the pointer to the backend structure
 * @param object_id the object ID
 * @param query_object_id the object that we want to determine whether it
 *                        is one of the immediate ancestors
 * @param out the pointer to store a positive number if yes, or 0 if no
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_has_immediate_ancestor(struct _cpl_db_backend_t* backend,
								const cpl_id_t object_id,
								const cpl_id_t query_object_id,
								int* out)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	int cr = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->has_immediate_ancestor_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt = odbc->has_immediate_ancestor_stmt;
	SQL_BIND_INTEGER(stmt, 1, query_object_id);
	SQL_BIND_INTEGER(stmt, 2, object_id);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Fetch the data

	r = cpl_sql_fetch_single_llong(stmt, NULL);
	if (CPL_IS_OK(r)) cr = 1;
	if (r == CPL_E_NOT_FOUND) { r = CPL_OK; cr = 0; }
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->has_immediate_ancestor_lock);
		return r;
	}


	// Cleanup

	mutex_unlock(odbc->has_immediate_ancestor_lock);

	if (out != NULL) *out = cr;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->has_immediate_ancestor_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Add a property to the given object
 *
 * @param backend the pointer to the backend structure
 * @param id the object ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_add_object_property(struct _cpl_db_backend_t* backend,
                      const cpl_id_t id,
                      const char* key,
                      const char* value)
{
    assert(backend != NULL);
    cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->add_object_property_lock);


	// Prepare the statement

	SQL_START;

retry:
	SQLHSTMT stmt = odbc->add_object_property_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);
	SQL_BIND_VARCHAR(stmt, 2, KEY_LEN, key);
	SQL_BIND_VARCHAR(stmt, 3, VALUE_LEN, value);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Cleanup

	mutex_unlock(odbc->add_object_property_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->add_object_property_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Add a property to the given relation
 *
 * @param backend the pointer to the backend structure
 * @param id the edge ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_add_relation_property(struct _cpl_db_backend_t* backend,
                      const cpl_id_t id,
                      const char* key,
                      const char* value)
{
    assert(backend != NULL);
    cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->add_relation_property_lock);


	// Prepare the statement

	SQL_START;

retry:
	SQLHSTMT stmt = odbc->add_relation_property_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);
	SQL_BIND_VARCHAR(stmt, 2, KEY_LEN, key);
	SQL_BIND_VARCHAR(stmt, 3, VALUE_LEN, value);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Cleanup

	mutex_unlock(odbc->add_relation_property_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->add_relation_property_lock);
	return CPL_E_STATEMENT_ERROR;
}



/**
 * Get information about the given provenance session.
 *
 * @param backend the pointer to the backend structure
 * @param id the session ID
 * @param out_info the pointer to store the session info structure
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_session_info(struct _cpl_db_backend_t* backend,
						  const cpl_session_t id,
						  cpl_session_info_t** out_info)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	long long l = 0;

	cpl_session_info_t* p = (cpl_session_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;


	// Prepare the statement

	mutex_lock(odbc->get_session_info_lock);

retry:
	SQLHSTMT stmt = odbc->get_session_info_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Fetch the result

	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 1, &p->mac_address);
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 2, &p->user);
	CPL_SQL_SIMPLE_FETCH(llong, 3, &l); p->pid = (int) l;
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 4, &p->program);
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 5, &p->cmdline);
	CPL_SQL_SIMPLE_FETCH(timestamp_as_unix_time, 6, &p->start_time);

	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Cleanup

	mutex_unlock(odbc->get_session_info_lock);
	
	*out_info = p;
	return CPL_OK;


	// Error handling

err:
	r = CPL_E_STATEMENT_ERROR;

err_r:
	mutex_unlock(odbc->get_session_info_lock);

	if (p->mac_address != NULL) free(p->mac_address);
	if (p->user != NULL) free(p->user);
	if (p->program != NULL) free(p->program);
	if (p->cmdline != NULL) free(p->cmdline);
	free(p);

	return r;
}


/**
 * Get all objects in the database
 *
 * @param backend the pointer to the backend structure
 * @param flags a logical combination of CPL_I_* flags
 * @param callback the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_all_objects(struct _cpl_db_backend_t* backend,
						 const int flags,
						 cpl_object_info_iterator_t callback,
						 void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	cplxx_object_info_t entry;
	std::list<cplxx_object_info_t> entries;
	SQL_TIMESTAMP_STRUCT t;

	size_t originator_size = ORIGINATOR_LEN + 1;
	size_t name_size = NAME_LEN + 1;

	char* entry_originator = (char*) alloca(originator_size);
	char* entry_name = (char*) alloca(name_size);

	if (entry_originator == NULL || entry_name == NULL) {
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	SQLLEN cb_bundle_id = 0;
	SQLLEN cb_session_id = 0;

	mutex_lock(odbc->get_all_objects_lock);


	// Get and execute the statement
	
retry:

	entries.clear();

	SQLHSTMT stmt = ((flags & CPL_I_NO_CREATION_SESSION) != 0)
						? odbc->get_all_objects_stmt
						: odbc->get_all_objects_with_session_stmt;


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_TYPE_TIMESTAMP, &t, sizeof(t), NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_CHAR, entry_originator, originator_size,
					 NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 4, SQL_C_CHAR, entry_name, name_size, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 5, SQL_C_SLONG, &entry.type, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

    ret = SQLBindCol(stmt, 6, SQL_C_UBIGINT, &entry.bundle_id, 0,
                     &cb_bundle_id);
    if (!SQL_SUCCEEDED(ret)) goto err_close;


	if ((flags & CPL_I_NO_CREATION_SESSION) == 0) {

		ret = SQLBindCol(stmt, 7, SQL_C_UBIGINT, &entry.creation_session,0,
						 &cb_session_id);
		if (!SQL_SUCCEEDED(ret)) goto err_close;
	}

    else {
        
        entry.creation_session = CPL_NONE;
    }



	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret == SQL_INVALID_HANDLE) {
				fprintf(stderr, "\nThe ODBC driver failed while running "
								"SQLFetch due to SQL_INVALID_HANDLE\n\n");
				goto err_close;
			}
			else if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		entry.creation_time = cpl_sql_timestamp_to_unix_time(t);
		entry.originator = entry_originator;
		entry.name = entry_name;

		if (cb_bundle_id <= 0) entry.bundle_id = CPL_NONE;
		if (cb_session_id <= 0) entry.creation_session = CPL_NONE;

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_all_objects_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<cplxx_object_info_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {

			strncpy(entry_originator, i->originator.c_str(), originator_size);
			strncpy(entry_name, i->name.c_str(), name_size);
			entry_originator[originator_size - 1] = '\0';
			entry_name[name_size - 1] = '\0';

			cpl_object_info_t e;
			e.id = i->id;
			e.creation_session = i->creation_session;
			e.creation_time = i->creation_time;
			e.originator = entry_originator;
			e.name = entry_name;
			e.type = i->type;
			e.bundle_id = i->bundle_id;

			r = callback(&e, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_all_objects_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Get information about the given provenance object
 *
 * @param backend the pointer to the backend structure
 * @param id the object ID
 * @param out_info the pointer to store the object info structure
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_object_info(struct _cpl_db_backend_t* backend,
						 const cpl_id_t id,
						 cpl_object_info_t** out_info)
{
	assert(backend != NULL);
	
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;


	cpl_object_info_t* p = (cpl_object_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;


	// Prepare the statement

	mutex_lock(odbc->get_object_info_lock);

retry:
	SQLHSTMT stmt = odbc->get_object_info_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Fetch the result

	CPL_SQL_SIMPLE_FETCH(llong, 1, (long long*) &p->creation_session);
	CPL_SQL_SIMPLE_FETCH(timestamp_as_unix_time, 2, &p->creation_time);

	CPL_SQL_SIMPLE_FETCH_EXT(dynamically_allocated_string, 3, &p->originator,
							 true);
	if (r == CPL_E_DB_NULL) p->originator = strdup("");
	CPL_SQL_SIMPLE_FETCH_EXT(dynamically_allocated_string, 4, &p->name, true);
	if (r == CPL_E_DB_NULL) p->name = strdup("");
	CPL_SQL_SIMPLE_FETCH(int, 5, (int*) &p->type);
	CPL_SQL_SIMPLE_FETCH_EXT(llong, 6, (long long*) &p->bundle_id, true);
	if (r == CPL_E_DB_NULL) p->bundle_id = CPL_NONE;
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Cleanup

	mutex_unlock(odbc->get_object_info_lock);
	
	*out_info = p;
	return CPL_OK;


	// Error handling

err:
	r = CPL_E_STATEMENT_ERROR;

err_r:
	mutex_unlock(odbc->get_object_info_lock);

	if (p->originator != NULL) free(p->originator);
	if (p->name != NULL) free(p->name);
	free(p);

	return r;
}



/**
 * 
 * An entry in the result set of the queries issued by
 * cpl_odbc_get_object_relations().
 */ 
typedef struct __get_object_relation__entry {
	cpl_id_t relation_id;
	cpl_id_t other_id;
	long type;
	cpl_id_t bundle_id;
} __get_object_relation__entry_t;


/**
 * Iterate over the ancestors or the descendants of a provenance object.
 *
 * @param backend the pointer to the backend structure
 * @param id the object ID
 * @param direction the direction of the graph traversal (CPL_D_ANCESTORS
 *                  or CPL_D_DESCENDANTS)
 * @param flags the bitwise combination of flags describing how should
 *              the graph be traversed
 * @param callback the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
cpl_return_t
cpl_odbc_get_object_relations(struct _cpl_db_backend_t* backend,
							 const cpl_id_t id,
							 const int direction,
							 const int flags,
							 cpl_relation_iterator_t callback,
							 void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	std::list<__get_object_relation__entry_t> entries;
	__get_object_relation__entry_t entry;
	SQLLEN ind_type;
	bool found = false;

	mutex_lock(odbc->get_object_relations_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;
	if (direction == CPL_D_ANCESTORS) {
		stmt = odbc->get_object_ancestors_stmt;
	}
	else {
		stmt = odbc->get_object_descendants_stmt;
	}

	SQL_BIND_INTEGER(stmt, 1, id);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.relation_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_UBIGINT, &entry.other_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_SLONG, &entry.type, 0, &ind_type);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 4, SQL_C_UBIGINT, &entry.bundle_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		found = true;

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_object_relations_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<__get_object_relation__entry_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(i->relation_id, id, i->other_id, (int) i->type, i->bundle_id, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_object_relations_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * An entry in the result set of the queries issued by
 * cpl_odbc_get_*_properties().
 */
typedef struct __get_properties__entry {
	cpl_id_t id;
	SQLCHAR key[256];
	SQLCHAR value[4096];
} __get_properties__entry_t;


/**
 * Get the properties associated with the given provenance object.
 *
 * @param backend the pointer to the backend structure
 * @param id the the object ID
 * @param key the property to fetch - or NULL for all properties
 * @param callback the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
cpl_return_t
cpl_odbc_get_object_properties(struct _cpl_db_backend_t* backend,
						const cpl_id_t id,
						const char* key,
						cpl_property_iterator_t callback,
						void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	std::list<__get_properties__entry_t*> entries;
	__get_properties__entry_t entry;
	SQLLEN ind_key, ind_value;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	bool found = false;
	std::list<__get_properties__entry_t*>::iterator i;

	mutex_lock(odbc->get_object_properties_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;
	if (key == NULL) {
		stmt = odbc->get_object_properties_stmt;
	}
	else {
		stmt = odbc->get_object_properties_with_key_stmt;
	}

	SQL_BIND_INTEGER(stmt, 1, id);

	if (key != NULL) {
		SQL_BIND_VARCHAR(stmt, 2, 255, key);
	}


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_CHAR, entry.key, sizeof(entry.key),
			&ind_key);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_CHAR, entry.value, sizeof(entry.value),
			&ind_value);
	if (!SQL_SUCCEEDED(ret)) goto err_close;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		found = true;

		if (ind_key == SQL_NULL_DATA || ind_value == SQL_NULL_DATA) {
			// NULLs should never occur here
			continue;
		}

		__get_properties__entry_t* e = new __get_properties__entry_t;
		memcpy(e, (const void*) &entry, sizeof(*e));

		entries.push_back(e);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_object_properties_lock);

	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(id, (const char*) (*i)->key,
						 (const char*) (*i)->value,
						 context);
			if (!CPL_IS_OK(r)) goto err_free;
		}
	}

	r = CPL_OK;


	// Error handling

err_free:
	for (i = entries.begin(); i != entries.end(); i++) {
		delete *i;
	}
	return r;

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_object_properties_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Lookup an object based on a property value.
 *
 * @param backend the pointer to the backend structure
 * @param key the property name
 * @param value the property value
 * @param callback the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
 */
cpl_return_t
cpl_odbc_lookup_object_by_property(struct _cpl_db_backend_t* backend,
							const char* key,
							const char* value,
							cpl_property_iterator_t callback,
							void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	std::list<cpl_id_t> entries;
	cpl_id_t entry;

	mutex_lock(odbc->lookup_object_by_property_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt = odbc->lookup_object_by_property_stmt;
	SQL_BIND_VARCHAR(stmt, 1, 255, key);
	SQL_BIND_VARCHAR(stmt, 2, 4095, value);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->lookup_object_by_property_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_E_NOT_FOUND;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<cpl_id_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(*i, key, value, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->lookup_object_by_property_lock);
	return CPL_E_STATEMENT_ERROR;
}

/**
 * Get the properties associated with the given provenance relation.
 *
 * @param backend the pointer to the backend structure
 * @param id the the relation ID
 * @param key the property to fetch - or NULL for all properties
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
cpl_return_t
cpl_odbc_get_relation_properties(struct _cpl_db_backend_t* backend,
						const cpl_id_t id,
						const char* key,
						cpl_property_iterator_t callback,
						void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	std::list<__get_properties__entry_t*> entries;
	__get_properties__entry_t entry;
	SQLLEN ind_key, ind_value;

	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	bool found = false;
	std::list<__get_properties__entry_t*>::iterator i;

	mutex_lock(odbc->get_relation_properties_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;

	if (key == NULL) {
		stmt = odbc->get_relation_properties_stmt;
	}
	else {
		stmt = odbc->get_relation_properties_with_key_stmt;
	}

	SQL_BIND_INTEGER(stmt, 1, id);

	if (key != NULL) {
		SQL_BIND_VARCHAR(stmt, 2, KEY_LEN, key);
	}


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_CHAR, entry.key, sizeof(entry.key),
			&ind_key);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_CHAR, entry.value, sizeof(entry.value),
			&ind_value);
	if (!SQL_SUCCEEDED(ret)) goto err_close;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		found = true;

		if (ind_key == SQL_NULL_DATA || ind_value == SQL_NULL_DATA) {
			// NULLs should never occur here
			continue;
		}

		__get_properties__entry_t* e = new __get_properties__entry_t;
		memcpy(e, (const void*) &entry, sizeof(*e));

		entries.push_back(e);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_relation_properties_lock);


	// If we did not get any data back, check for whether the object exists.

	if (!found) {
		goto err_free;
	}


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(id,
						 (const char*) (*i)->key,
						 (const char*) (*i)->value,
						 context);
			if (!CPL_IS_OK(r)) goto err_free;
		}
	}

	r = CPL_OK;


	// Error handling

err_free:
	for (i = entries.begin(); i != entries.end(); i++) {
		delete *i;
	}
	return r;

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_relation_properties_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Deletes a bundle along with all objects and relations it contains.
 *
 * @param backend the pointer to the backend structure
 * @param id the bundle ID
 */
cpl_return_t
cpl_odbc_delete_bundle(struct _cpl_db_backend_t* backend,
						const cpl_id_t id)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->delete_bundle_lock);


	// Prepare the statement

	SQL_START;

retry:
	SQLHSTMT stmt = odbc->delete_bundle_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);

	// Execute
	
	SQL_EXECUTE(stmt);

	// Cleanup

	mutex_unlock(odbc->delete_bundle_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->delete_bundle_lock);
	return CPL_E_STATEMENT_ERROR;

}

/**
 * Returns all objects contained in a bundle.
 *
 * @param backend the pointer to the backend structure
 * @param id the bundle ID
 * @param callback the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_bundle_objects(struct _cpl_db_backend_t* backend,
						     const cpl_id_t id,
						     cpl_object_info_iterator_t callback,
						     void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	cplxx_object_info_t entry;
	std::list<cplxx_object_info_t> entries;
	SQL_TIMESTAMP_STRUCT t;

	size_t originator_size = ORIGINATOR_LEN + 1;
	size_t name_size = NAME_LEN + 1;

	char* entry_originator = (char*) alloca(originator_size);
	char* entry_name = (char*) alloca(name_size);

	if (entry_originator == NULL || entry_name == NULL) {
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	mutex_lock(odbc->get_bundle_objects_lock);


	// Get and execute the statement
	
retry:

	entries.clear();

	SQLHSTMT stmt = odbc->get_bundle_objects_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);

	// Execute
	
	SQL_EXECUTE(stmt);

	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_TYPE_TIMESTAMP, &t, sizeof(t), NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_CHAR, entry_originator, originator_size,
					 NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 4, SQL_C_CHAR, entry_name, name_size, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 5, SQL_C_SLONG, &entry.type, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

    entry.bundle_id = id;

    entry.creation_session = CPL_NONE;




	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret == SQL_INVALID_HANDLE) {
				fprintf(stderr, "\nThe ODBC driver failed while running "
								"SQLFetch due to SQL_INVALID_HANDLE\n\n");
				goto err_close;
			}
			else if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		entry.creation_time = cpl_sql_timestamp_to_unix_time(t);
		entry.originator = entry_originator;
		entry.name = entry_name;

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_bundle_objects_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<cplxx_object_info_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {

			strncpy(entry_originator, i->originator.c_str(), originator_size);
			strncpy(entry_name, i->name.c_str(), name_size);
			entry_originator[originator_size - 1] = '\0';
			entry_name[name_size - 1] = '\0';

			cpl_object_info_t e;
			e.id = i->id;
			e.creation_session = i->creation_session;
			e.creation_time = i->creation_time;
			e.originator = entry_originator;
			e.name = entry_name;
			e.type = i->type;
			e.bundle_id = i->bundle_id;

			r = callback(&e, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_bundle_objects_lock);
	return CPL_E_STATEMENT_ERROR;
}

typedef struct __get_bundle_relation__entry {
	cpl_id_t relation_id;
	cpl_id_t from_id;
	cpl_id_t to_id;
	long type;
	cpl_id_t bundle_id;
} __get_bundle_relation__entry_t;

/**
 * Returns all relations contained in a bundle.
 *
 * @param backend the pointer to the backend structure
 * @param id the bundle ID
 * @param callback the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_bundle_relations(struct _cpl_db_backend_t* backend,
						     const cpl_id_t id,
						     cpl_relation_iterator_t callback,
							 void* context){

	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQL_START;

	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	std::list<__get_bundle_relation__entry_t> entries;
	__get_bundle_relation__entry_t entry;
	SQLLEN ind_type;
	bool found = false;

	mutex_lock(odbc->get_bundle_relations_lock);


	// Prepare the statement

retry:
	SQLHSTMT stmt;
	
	stmt = odbc->get_bundle_relations_stmt;

	SQL_BIND_INTEGER(stmt, 1, id);


	// Execute
	
	SQL_EXECUTE(stmt);


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.relation_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_UBIGINT, &entry.from_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_UBIGINT, &entry.to_id, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 4, SQL_C_SLONG, &entry.type, 0, &ind_type);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	entry.bundle_id = id;


	// Fetch the result

	while (true) {

		ret = SQLFetch(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			if (ret != SQL_NO_DATA) {
				print_odbc_error("SQLFetch", stmt, SQL_HANDLE_STMT);
				goto err_close;
			}
			break;
		}

		found = true;

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_bundle_relations_lock);


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (callback != NULL) {
		std::list<__get_bundle_relation__entry_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {
			r = callback(i->relation_id, i->from_id, i->to_id, (int) i->type, id, context);
			if (!CPL_IS_OK(r)) return r;
		}
	}

	return CPL_OK;


	// Error handling

err_close:
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
	}

err:
	mutex_unlock(odbc->get_bundle_relations_lock);
	return CPL_E_STATEMENT_ERROR;
}
/***************************************************************************/
/** The export / interface struct                                         **/
/***************************************************************************/

/**
 * The ODBC interface
 */
const cpl_db_backend_t CPL_ODBC_BACKEND = {
	cpl_odbc_destroy,
	cpl_odbc_create_session,
	cpl_odbc_create_object,
	cpl_odbc_lookup_object,
	cpl_odbc_lookup_object_ext,
	cpl_odbc_add_relation,
	cpl_odbc_has_immediate_ancestor,
	cpl_odbc_add_object_property,
	cpl_odbc_add_relation_property,
	cpl_odbc_get_session_info,
	cpl_odbc_get_all_objects,
	cpl_odbc_get_object_info,
	cpl_odbc_get_object_relations,
	cpl_odbc_get_object_properties,
	cpl_odbc_lookup_object_by_property,
	cpl_odbc_get_relation_properties,
	cpl_odbc_delete_bundle,
	cpl_odbc_get_bundle_objects,
	cpl_odbc_get_bundle_relations
};

