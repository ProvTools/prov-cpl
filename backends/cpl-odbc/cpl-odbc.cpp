/*
 * cpl-odbc.cpp
 * Core Provenance Library
 *
 * Copyright 2011
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
 * Contributor(s): Peter Macko
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
 * Print the ODBC error to stderr
 *
 * @param fn the function that failed
 * @param handle the failed handle
 * @param type the handle type
 */
static void
print_odbc_error(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
	// From: http://www.easysoft.com/developer/languages/c/odbc_tutorial.html

	SQLSMALLINT	 i = 0;
	SQLINTEGER	 native;
	SQLCHAR	 state[ 7 ];
	SQLCHAR	 text[256];
	SQLSMALLINT	 len;
	SQLRETURN	 ret;

	fprintf(stderr,
			"\n"
			"The ODBC driver reported the following while running "
			"%s:\n",
			fn);

	do {
		ret = SQLGetDiagRec(type, handle, ++i, state, &native,
							text, sizeof(text), &len);
		
		if (SQL_SUCCEEDED(ret)) {
			fprintf(stderr, "  %s:%ld:%ld:%s\n",
					state, (long) i, (long) native, text);
		}
		else if (ret != SQL_NO_DATA) {
			fprintf(stderr, "  SQLGetDiagRec failed with error code %ld\n",
					(long) ret);
		}
	}
	while (ret == SQL_SUCCESS);

	fprintf(stderr, "\n");
}


/**
 * If the variable ret is an error, print the SQL error
 * and goto the given label
 *
 * @param function the function for which the error is checked
 * @param handle the statement handle
 * @param label the label to jump to on error
 */
#define SQL_ASSERT_NO_ERROR(function, handle, label) { \
	if (!SQL_SUCCEEDED(ret)) { \
		print_odbc_error(#function, handle, SQL_HANDLE_STMT); \
		goto label; \
	}}


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
			&t, 0, column, fetch, close_if_ok, handle_nulls);
	if (!CPL_IS_OK(r)) return r;

	if (out != NULL) {
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
		*out = (unsigned long) mktime(&m);
	}
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
						   bool handle_nulls=false, size_t max_length=255)
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
/** Constructor and Destructor                                            **/
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


	// Initialize the synchronization primitives

	mutex_init(odbc->create_session_lock);
	mutex_init(odbc->create_object_lock);
	mutex_init(odbc->lookup_object_lock);
	mutex_init(odbc->create_version_lock);
	mutex_init(odbc->get_version_lock);
	mutex_init(odbc->add_ancestry_edge_lock);
	mutex_init(odbc->has_immediate_ancestor_lock);
	mutex_init(odbc->get_session_info_lock);
	mutex_init(odbc->get_object_info_lock);
	mutex_init(odbc->get_version_info_lock);
	mutex_init(odbc->get_object_ancestry_lock);


	// Open the ODBC connection

	SQLRETURN ret;
	SQLCHAR outstr[1024];
	SQLCHAR* connection_string_copy;
	SQLSMALLINT outstrlen;

	connection_string_copy = (SQLCHAR*) malloc(strlen(connection_string) + 1);
	if (connection_string_copy == NULL) {
		r = CPL_E_INSUFFICIENT_RESOURCES;
		goto err_sync;
	}

#ifdef _WINDOWS
	strcpy_s((char*) connection_string_copy,
			 strlen(connection_string) + 1,
			 connection_string);
#else
	strcpy((char*) connection_string_copy, connection_string);
#endif

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc->db_environment);
	SQLSetEnvAttr(odbc->db_environment,
				  SQL_ATTR_ODBC_VERSION,
				  (void *) SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, odbc->db_environment, &odbc->db_connection);
	
	ret = SQLDriverConnect(odbc->db_connection, NULL,
						   connection_string_copy, SQL_NTS,
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
	ALLOC_STMT(create_object_insert_container_stmt);
	ALLOC_STMT(create_object_insert_version_stmt);
	ALLOC_STMT(lookup_object_stmt);
	ALLOC_STMT(create_version_stmt);
	ALLOC_STMT(get_version_stmt);
	ALLOC_STMT(add_ancestry_edge_stmt);
	ALLOC_STMT(has_immediate_ancestor_stmt);
	ALLOC_STMT(has_immediate_ancestor_with_ver_stmt);
	ALLOC_STMT(get_session_info_stmt);
	ALLOC_STMT(get_object_info_stmt);
	ALLOC_STMT(get_version_info_stmt);
	ALLOC_STMT(get_object_ancestors);
	ALLOC_STMT(get_object_ancestors_with_ver);
	ALLOC_STMT(get_object_descendants);
	ALLOC_STMT(get_object_descendants_with_ver);

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
			"            (id_hi, id_lo, mac_address, username, pid, program)"
			"     VALUES (?, ?, ?, ?, ?, ?);");

	PREPARE(create_object_insert_stmt,
			"INSERT INTO cpl_objects"
			"            (id_hi, id_lo, originator, name, type) "
			"     VALUES (?, ?, ?, ?, ?);");

	PREPARE(create_object_insert_container_stmt,
			"INSERT INTO cpl_objects"
			"            (id_hi, id_lo, originator, name, type,"
			"             container_id_hi, container_id_lo, container_ver)"
			"     VALUES (?, ?, ?, ?, ?, ?, ?, ?);");

	PREPARE(create_object_insert_version_stmt,
			"INSERT INTO cpl_versions"
			"            (id_hi, id_lo, version, session_id_hi, session_id_lo)"
			"     VALUES (?, ?, 0, ?, ?);");

	PREPARE(lookup_object_stmt,
			"SELECT id_hi, id_lo"
			"  FROM cpl_objects"
			" WHERE originator = ? AND name = ? AND type = ?"
			" ORDER BY creation_time DESC"
			" LIMIT 1;");

	PREPARE(create_version_stmt,
			"INSERT INTO cpl_versions"
			"            (id_hi, id_lo, version, session_id_hi, session_id_lo)"
			"     VALUES (?, ?, ?, ?, ?);");

	PREPARE(get_version_stmt,
			"SELECT MAX(version)"
			"  FROM cpl_versions"
			" WHERE id_hi = ? AND id_lo = ?;");

	PREPARE(add_ancestry_edge_stmt,
			"INSERT INTO cpl_ancestry"
			"            (from_id_hi, from_id_lo, from_version,"
			"             to_id_hi, to_id_lo, to_version, type)"
			"     VALUES (?, ?, ?, ?, ?, ?, ?);");

	PREPARE(has_immediate_ancestor_stmt,
			"SELECT to_version"
			"  FROM cpl_ancestry"
			" WHERE to_id_hi = ? AND to_id_lo = ? AND to_version <= ?"
			"   AND from_id_hi = ? AND from_id_lo = ?"
			" LIMIT 1;");

	PREPARE(has_immediate_ancestor_with_ver_stmt,
			"SELECT to_version"
			"  FROM cpl_ancestry"
			" WHERE to_id_hi = ? AND to_id_lo = ? AND to_version <= ?"
			"   AND from_id_hi = ? AND from_id_lo = ? AND from_version <= ?"
			" LIMIT 1;");

	PREPARE(get_object_info_stmt,
			"SELECT session_id_hi, session_id_lo,"
			"       cpl_objects.creation_time, originator, name, type,"
			"       container_id_hi, container_id_lo, container_ver"
			"  FROM cpl_objects, cpl_versions"
			" WHERE cpl_objects.id_hi = ? AND cpl_objects.id_lo = ?"
			"   AND cpl_objects.id_hi = cpl_versions.id_hi"
			"   AND cpl_objects.id_lo = cpl_versions.id_lo"
			"   AND version = 0"
			" LIMIT 1;");

	PREPARE(get_session_info_stmt,
			"SELECT mac_address, username,"
			"       pid, program, initialization_time"
			"  FROM cpl_sessions"
			" WHERE id_hi = ? AND id_lo = ?"
			" LIMIT 1;");

	PREPARE(get_version_info_stmt,
			"SELECT session_id_hi, session_id_lo, creation_time"
			"  FROM cpl_versions"
			" WHERE id_hi = ? AND id_lo = ? AND version = ?"
			" LIMIT 1;");

	PREPARE(get_object_ancestors,
			"SELECT to_id_hi, to_id_lo, to_version, from_version, type"
			"  FROM cpl_ancestry"
			" WHERE from_id_hi = ? AND from_id_lo = ?");

	PREPARE(get_object_ancestors_with_ver,
			"SELECT to_id_hi, to_id_lo, to_version, from_version, type"
			"  FROM cpl_ancestry"
			" WHERE from_id_hi = ? AND from_id_lo = ? AND from_version = ?");

	PREPARE(get_object_descendants,
			"SELECT from_id_hi, from_id_lo, from_version, to_version, type"
			"  FROM cpl_ancestry"
			" WHERE to_id_hi = ? AND to_id_lo = ?");

	PREPARE(get_object_descendants_with_ver,
			"SELECT from_id_hi, from_id_lo, from_version, to_version, type"
			"  FROM cpl_ancestry"
			" WHERE to_id_hi = ? AND to_id_lo = ? AND to_version = ?");


#undef PREPARE


	// Return

	*out = (cpl_db_backend_t*) odbc;
	return CPL_OK;


	// Error handling -- the variable r must be set

	assert(!CPL_IS_OK(r));

err_stmts:
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_session_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_ancestry_edge_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_with_ver_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_session_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_ancestors);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_ancestors_with_ver);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_descendants);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_descendants_with_ver);

	SQLDisconnect(odbc->db_connection);

err_handles:
	SQLFreeHandle(SQL_HANDLE_DBC, odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, odbc->db_environment);

err_sync:
	mutex_destroy(odbc->create_session_lock);
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->create_version_lock);
	mutex_destroy(odbc->get_version_lock);
	mutex_destroy(odbc->add_ancestry_edge_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);
	mutex_destroy(odbc->get_session_info_lock);
	mutex_destroy(odbc->get_object_info_lock);
	mutex_destroy(odbc->get_version_info_lock);
	mutex_destroy(odbc->get_object_ancestry_lock);

	delete odbc;
	return r;
}


/**
 * Destructor. If the constructor allocated the backend structure, it
 * should be freed by this function
 *
 * @param backend the pointer to the backend structure
 * @param the error code
 */
extern "C" cpl_return_t
cpl_odbc_destroy(struct _cpl_db_backend_t* backend)
{
	assert(backend != NULL);
	SQLRETURN ret;

	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_session_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_ancestry_edge_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_with_ver_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_session_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_info_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_ancestors);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_ancestors_with_ver);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_descendants);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_object_descendants_with_ver);

	ret = SQLDisconnect(odbc->db_connection);
	if (!SQL_SUCCEEDED(ret)) {
		fprintf(stderr, "Warning: Could not terminate the ODBC connection.\n");
	}
	
	mutex_destroy(odbc->create_session_lock);
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->create_version_lock);
	mutex_destroy(odbc->get_version_lock);
	mutex_destroy(odbc->add_ancestry_edge_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);
	mutex_destroy(odbc->get_session_info_lock);
	mutex_destroy(odbc->get_object_info_lock);
	mutex_destroy(odbc->get_version_info_lock);
	mutex_destroy(odbc->get_object_ancestry_lock);

	SQLFreeHandle(SQL_HANDLE_DBC, odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, odbc->db_environment);
	
	delete odbc;
	
	return CPL_OK;
}


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
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_create_session(struct _cpl_db_backend_t* backend,
						const cpl_session_t session,
						const char* mac_address,
						const char* user,
						const int pid,
						const char* program)
{
	assert(backend != NULL && user != NULL && program != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->create_session_lock);

	
	// Bind the statement parameters

	SQLRETURN ret;
	SQLHSTMT stmt = odbc->create_session_insert_stmt;

	SQL_BIND_INTEGER(stmt, 1, session.hi);
	SQL_BIND_INTEGER(stmt, 2, session.lo);
	SQL_BIND_VARCHAR(stmt, 3, 18, mac_address);
	SQL_BIND_VARCHAR(stmt, 4, 255, user);
	SQL_BIND_INTEGER(stmt, 5, pid);
	SQL_BIND_VARCHAR(stmt, 6, 4096, program);


	// Insert the new row to the sessions table

	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Finish

	mutex_unlock(odbc->create_session_lock);
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
 * @param id the ID of the new object
 * @param originator the originator
 * @param name the object name
 * @param type the object type
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @param container_version the version of the container (if not CPL_NONE)
 * @param session the session ID responsible for this provenance record
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_create_object(struct _cpl_db_backend_t* backend,
					   const cpl_id_t id,
					   const char* originator,
					   const char* name,
					   const char* type,
					   const cpl_id_t container,
					   const cpl_version_t container_version,
					   const cpl_session_t session)
{
	assert(backend != NULL && originator != NULL
			&& name != NULL && type != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	mutex_lock(odbc->create_object_lock);

	
	// Bind the statement parameters

	SQLRETURN ret;
	SQLHSTMT stmt = container == CPL_NONE
		? odbc->create_object_insert_stmt
		: odbc->create_object_insert_container_stmt;

	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);
	SQL_BIND_VARCHAR(stmt, 3, 255, originator);
	SQL_BIND_VARCHAR(stmt, 4, 255, name);
	SQL_BIND_VARCHAR(stmt, 5, 100, type);

	if (container != CPL_NONE) {
		SQL_BIND_INTEGER(stmt, 6, container.hi);
		SQL_BIND_INTEGER(stmt, 7, container.lo);
		SQL_BIND_INTEGER(stmt, 8, container_version);
	}


	// Insert the new row to the objects table

	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Insert the corresponding entry to the versions table

	stmt = odbc->create_object_insert_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);
	SQL_BIND_INTEGER(stmt, 3, session.hi);
	SQL_BIND_INTEGER(stmt, 4, session.lo);
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}

	
	// Finish

	mutex_unlock(odbc->create_object_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->create_object_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
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
					   const char* type,
					   cpl_id_t* out_id)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_id_t id = CPL_NONE;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->lookup_object_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->lookup_object_stmt;

	SQL_BIND_VARCHAR(stmt, 1, 255, originator);
	SQL_BIND_VARCHAR(stmt, 2, 255, name);
	SQL_BIND_VARCHAR(stmt, 3, 100, type);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id.hi, 1, true, false);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->lookup_object_lock);
		return r;
	}

	r = cpl_sql_fetch_single_llong(stmt, (long long*) &id.lo, 2, false, true);
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
 * Create a new version of the given object
 *
 * @param backend the pointer to the backend structure
 * @param object_id the object ID
 * @param version the new version of the object
 * @param session the session ID responsible for this provenance record
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_create_version(struct _cpl_db_backend_t* backend,
						const cpl_id_t object_id,
						const cpl_version_t version,
						const cpl_session_t session)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;

	mutex_lock(odbc->create_version_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->create_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, object_id.hi);
	SQL_BIND_INTEGER(stmt, 2, object_id.lo);
	SQL_BIND_INTEGER(stmt, 3, version);
	SQL_BIND_INTEGER(stmt, 4, session.hi);
	SQL_BIND_INTEGER(stmt, 5, session.lo);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		
		SQLINTEGER native;
		SQLCHAR	state[ 7 ];
		SQLCHAR	text[256];
		SQLSMALLINT	len;

		ret = SQLGetDiagRec(SQL_HANDLE_STMT, stmt, 1, state, &native,
							text, sizeof(text), &len);
		
		if (SQL_SUCCEEDED(ret)) {
			if (strcmp((const char*) state, "23000") == 0) {
				mutex_unlock(odbc->create_version_lock);
				return CPL_E_ALREADY_EXISTS;
			}
			else {
				print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
			}
		}
		else if (ret != SQL_NO_DATA) {
			fprintf(stderr, "  SQLGetDiagRec failed with error code %ld\n",
					(long) ret);
		}

		goto err;
	}


	// Cleanup

	mutex_unlock(odbc->create_version_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->create_version_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Determine the version of the object
 *
 * @param backend the pointer to the backend structure
 * @param id the object ID
 * @param out_version the pointer to store the version of the object
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_get_version(struct _cpl_db_backend_t* backend,
					 const cpl_id_t id,
					 cpl_version_t* out_version)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	long long l;
	cpl_return_t r;

	mutex_lock(odbc->get_version_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->get_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	r = cpl_sql_fetch_single_llong(stmt, &l);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->get_version_lock);
		return r;
	}


	// Cleanup

	mutex_unlock(odbc->get_version_lock);

	if (out_version != NULL) *out_version = (cpl_version_t) l;
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->get_version_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Add an ancestry edge
 *
 * @param backend the pointer to the backend structure
 * @param from_id the edge source ID
 * @param from_ver the edge source version
 * @param to_id the edge destination ID
 * @param to_ver the edge destination version
 * @param type the data or control dependency type
 * @return the error code
 */
extern "C" cpl_return_t
cpl_odbc_add_ancestry_edge(struct _cpl_db_backend_t* backend,
						   const cpl_id_t from_id,
						   const cpl_version_t from_ver,
						   const cpl_id_t to_id,
						   const cpl_version_t to_ver,
						   const int type)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	mutex_lock(odbc->add_ancestry_edge_lock);


	// Prepare the statement

	SQLRETURN ret;
	SQLHSTMT stmt = odbc->add_ancestry_edge_stmt;

	SQL_BIND_INTEGER(stmt, 1, from_id.hi);
	SQL_BIND_INTEGER(stmt, 2, from_id.lo);
	SQL_BIND_INTEGER(stmt, 3, from_ver);
	SQL_BIND_INTEGER(stmt, 4, to_id.hi);
	SQL_BIND_INTEGER(stmt, 5, to_id.lo);
	SQL_BIND_INTEGER(stmt, 6, to_ver);
	SQL_BIND_INTEGER(stmt, 7, type);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Cleanup

	mutex_unlock(odbc->add_ancestry_edge_lock);
	return CPL_OK;


	// Error handling

err:
	mutex_unlock(odbc->add_ancestry_edge_lock);
	return CPL_E_STATEMENT_ERROR;
}


/**
 * Determine whether the given object has the given ancestor
 *
 * @param backend the pointer to the backend structure
 * @param object_id the object ID
 * @param version_hint the object version (if known), or CPL_VERSION_NONE
 *                     otherwise
 * @param query_object_id the object that we want to determine whether it
 *                        is one of the immediate ancestors
 * @param query_object_max_version the maximum version of the query
 *                                 object to consider
 * @param out the pointer to store a positive number if yes, or 0 if no
 * @return CPL_OK or an error code
 */
extern "C" cpl_return_t
cpl_odbc_has_immediate_ancestor(struct _cpl_db_backend_t* backend,
								const cpl_id_t object_id,
								const cpl_version_t version_hint,
								const cpl_id_t query_object_id,
								const cpl_version_t query_object_max_version,
								int* out)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	int cr = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->has_immediate_ancestor_lock);


	// Prepare the statement

	SQLHSTMT stmt = version_hint == CPL_VERSION_NONE 
		? odbc->has_immediate_ancestor_stmt
		: odbc->has_immediate_ancestor_with_ver_stmt;
	SQL_BIND_INTEGER(stmt, 1, query_object_id.hi);
	SQL_BIND_INTEGER(stmt, 2, query_object_id.lo);
	SQL_BIND_INTEGER(stmt, 3, query_object_max_version);
	SQL_BIND_INTEGER(stmt, 4, object_id.hi);
	SQL_BIND_INTEGER(stmt, 5, object_id.lo);
	if (version_hint != CPL_VERSION_NONE) {
		SQL_BIND_INTEGER(stmt, 6, version_hint);
	}


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


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
 * Get information about the given provenance session.
 *
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

	SQLRETURN ret;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	long long l = 0;

	cpl_session_info_t* p = (cpl_session_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;


	// Prepare the statement

	mutex_lock(odbc->get_session_info_lock);
	SQLHSTMT stmt = odbc->get_session_info_stmt;

	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 1, &p->mac_address);
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 2, &p->user);
	CPL_SQL_SIMPLE_FETCH(llong, 3, &l); p->pid = (int) l;
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 4, &p->program);
	CPL_SQL_SIMPLE_FETCH(timestamp_as_unix_time, 5, &p->start_time);

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
	free(p);

	return r;
}


/**
 * Get information about the given provenance object
 *
 * @param id the object ID
 * @param version_hint the version of the given provenance object if known,
 *                     or CPL_VERSION_NONE if not
 * @param out_info the pointer to store the object info structure
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_object_info(struct _cpl_db_backend_t* backend,
						 const cpl_id_t id,
						 const cpl_version_t version_hint,
						 cpl_object_info_t** out_info)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;
	long long l;

	cpl_object_info_t* p = (cpl_object_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;


	// Get the version

	if (version_hint == CPL_VERSION_NONE) {
		r = cpl_odbc_get_version(backend, id, &p->version);
		if (!CPL_IS_OK(r)) {
			free(p);
			return r;
		}
	}
	else {
		p->version = version_hint;
	}


	// Prepare the statement

	mutex_lock(odbc->get_object_info_lock);
	SQLHSTMT stmt = odbc->get_object_info_stmt;

	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	CPL_SQL_SIMPLE_FETCH(llong, 1, (long long*) &p->creation_session.hi);
	CPL_SQL_SIMPLE_FETCH(llong, 2, (long long*) &p->creation_session.lo);
	CPL_SQL_SIMPLE_FETCH(timestamp_as_unix_time, 3, &p->creation_time);

	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 4, &p->originator);
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 5, &p->name);
	CPL_SQL_SIMPLE_FETCH(dynamically_allocated_string, 6, &p->type);

	CPL_SQL_SIMPLE_FETCH_EXT(llong, 7, (long long*) &p->container_id.hi, true);
	if (r == CPL_E_DB_NULL) p->container_id = CPL_NONE;
	CPL_SQL_SIMPLE_FETCH_EXT(llong, 8, (long long*) &p->container_id.lo, true);
	if (r == CPL_E_DB_NULL) p->container_id = CPL_NONE;
	CPL_SQL_SIMPLE_FETCH_EXT(llong, 9, &l, true);
	if (r == CPL_E_DB_NULL) l = CPL_VERSION_NONE;
	p->container_version = (cpl_version_t) l;
	
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
	if (p->type != NULL) free(p->type);
	free(p);

	return r;
}


/**
 * Get information about the specific version of a provenance object
 *
 * @param id the object ID
 * @param version the version of the given provenance object
 * @param out_info the pointer to store the version info structure
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_odbc_get_version_info(struct _cpl_db_backend_t* backend,
						  const cpl_id_t id,
						  const cpl_version_t version,
						  cpl_version_info_t** out_info)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	cpl_version_info_t* p = (cpl_version_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;
	p->version = version;

	mutex_lock(odbc->get_version_info_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->get_version_info_stmt;

	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);
	SQL_BIND_INTEGER(stmt, 3, version);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	CPL_SQL_SIMPLE_FETCH(llong, 1, (long long*) &p->session.hi);
	CPL_SQL_SIMPLE_FETCH(llong, 2, (long long*) &p->session.lo);
	CPL_SQL_SIMPLE_FETCH(timestamp_as_unix_time, 3, &p->creation_time);

	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Cleanup

	mutex_unlock(odbc->get_version_info_lock);
	
	*out_info = p;
	return CPL_OK;


	// Error handling

err:
	r = CPL_E_STATEMENT_ERROR;

err_r:
	mutex_unlock(odbc->get_version_info_lock);

	free(p);
	return r;
}


/**
 * An entry in the result set of the queries issued by
 * cpl_odbc_get_object_ancestry().
 */
typedef struct __get_object_ancestry__entry {
	cpl_id_t id;
	long version;
	long query_version;
	long type;
} __get_object_ancestry__entry_t;


/**
 * Iterate over the ancestors or the descendants of a provenance object.
 *
 * @param backend the pointer to the backend structure
 * @param id the object ID
 * @param version the object version, or CPL_VERSION_NONE to access all
 *                version nodes associated with the given object
 * @param direction the direction of the graph traversal (CPL_D_ANCESTORS
 *                  or CPL_D_DESCENDANTS)
 * @param flags the bitwise combination of flags describing how should
 *              the graph be traversed (a logical combination of the
 *              CPL_A_* flags)
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
cpl_return_t
cpl_odbc_get_object_ancestry(struct _cpl_db_backend_t* backend,
							 const cpl_id_t id,
							 const cpl_version_t version,
							 const int direction,
							 const int flags,
							 cpl_ancestry_iterator_t iterator,
							 void* context)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	if ((flags & ~CPL_ODBC_A_SUPPORTED_FLAGS) != 0) {
		return CPL_E_NOT_IMPLEMENTED;
	}
	
	SQLRETURN ret;
	cpl_return_t r = CPL_E_INTERNAL_ERROR;

	std::list<__get_object_ancestry__entry_t> entries;
	__get_object_ancestry__entry_t entry;
	SQLLEN ind_type;
	bool found = false;

	mutex_lock(odbc->get_object_ancestry_lock);


	// Prepare the statement

	SQLHSTMT stmt;
	if (direction == CPL_D_ANCESTORS) {
		stmt = version == CPL_VERSION_NONE
			? odbc->get_object_ancestors
			: odbc->get_object_ancestors_with_ver;
	}
	else {
		stmt = version == CPL_VERSION_NONE
			? odbc->get_object_descendants
			: odbc->get_object_descendants_with_ver;
	}

	SQL_BIND_INTEGER(stmt, 1, id.hi);
	SQL_BIND_INTEGER(stmt, 2, id.lo);

	if (version != CPL_VERSION_NONE) {
		SQL_BIND_INTEGER(stmt, 3, version);
	}


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Bind the columns

	ret = SQLBindCol(stmt, 1, SQL_C_UBIGINT, &entry.id.hi, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 2, SQL_C_UBIGINT, &entry.id.lo, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 3, SQL_C_SLONG, &entry.version, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 4, SQL_C_SLONG, &entry.query_version, 0, NULL);
	if (!SQL_SUCCEEDED(ret)) goto err_close;

	ret = SQLBindCol(stmt, 5, SQL_C_SLONG, &entry.type, 0, &ind_type);
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

		if (ind_type == SQL_NULL_DATA) {
			// Should we handle NULL dependency types? They should never occur.
			// entry.type = CPL_DEPENDENCY_NONE;
			continue;
		}

		int type_category = CPL_GET_DEPENDENCY_CATEGORY((int) entry.type);
		if (type_category == CPL_DEPENDENCY_CATEGORY_DATA
				&& (flags & CPL_A_NO_DATA_DEPENDENCIES) != 0) continue;
		if (type_category == CPL_DEPENDENCY_CATEGORY_CONTROL
				&& (flags & CPL_A_NO_CONTROL_DEPENDENCIES) != 0) continue;

		entries.push_back(entry);
	}
	
	ret = SQLCloseCursor(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLCloseCursor", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Unlock

	mutex_unlock(odbc->get_object_ancestry_lock);


	// If we did not get any data back, check for whether the object exists.
	// If the version is set, the library already verified that the object
	// actually exists.

	if (!found && version != CPL_VERSION_NONE) {
		// XXX This is ugly and potentially quite slow
		r = cpl_odbc_get_version(backend, id, NULL);
		if (!CPL_IS_SUCCESS(r)) return r;
	}


	// If we did not get any data back, terminate

	if (entries.empty()) return CPL_S_NO_DATA;


	// Call the user-provided callback function

	if (iterator != NULL) {
		std::list<__get_object_ancestry__entry_t>::iterator i;
		for (i = entries.begin(); i != entries.end(); i++) {
			r = iterator(id, (cpl_version_t) i->query_version,
						 i->id, (cpl_version_t) i->version,
						 (int) i->type, context);
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
	mutex_unlock(odbc->get_object_ancestry_lock);
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
	cpl_odbc_create_version,
	cpl_odbc_get_version,
	cpl_odbc_add_ancestry_edge,
	cpl_odbc_has_immediate_ancestor,
	cpl_odbc_get_session_info,
	cpl_odbc_get_object_info,
	cpl_odbc_get_version_info,
	cpl_odbc_get_object_ancestry,
};

