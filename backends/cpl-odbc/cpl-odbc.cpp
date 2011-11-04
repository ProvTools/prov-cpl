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
 * Read a single nonnegative number from the result set and close the cursor
 *
 * @param stmt the statement handle
 * @return the given number, CPL_E_NOT_FOUND if empty or NULL, or an error code
 */
static long long
cpl_sql_fetch_single_llong(SQLHSTMT stmt) {

	long long r = 0;
	SQLLEN cb = 0;
	SQLRETURN ret = 0;

	ret = SQLFetch(stmt);
	if (ret == SQL_NO_DATA) goto err_nf;
	SQL_ASSERT_NO_ERROR(SQLFetch, stmt, err);
	
	ret = SQLGetData(stmt, 1, SQL_C_SBIGINT, &r, 0, &cb);
	if (ret == SQL_NO_DATA) goto err_nf;
	SQL_ASSERT_NO_ERROR(SQLGetData, stmt, err);
	if (cb <= 0) goto err_nf;

	ret = SQLCloseCursor(stmt);
	SQL_ASSERT_NO_ERROR(SQLCloseCursor, stmt, err);

	if (!CPL_IS_OK(r)) return CPL_E_BACKEND_INTERNAL_ERROR;
	return r;

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
}



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

	mutex_init(odbc->create_object_lock);
	mutex_init(odbc->lookup_object_lock);
	mutex_init(odbc->create_version_lock);
	mutex_init(odbc->get_version_lock);
	mutex_init(odbc->add_ancestry_edge_lock);
	mutex_init(odbc->has_immediate_ancestor_lock);


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
	
	ALLOC_STMT(create_object_insert_stmt);
	ALLOC_STMT(create_object_insert_container_stmt);
	ALLOC_STMT(create_object_get_id_stmt);
	ALLOC_STMT(create_object_insert_version_stmt);
	ALLOC_STMT(lookup_object_stmt);
	ALLOC_STMT(create_version_stmt);
	ALLOC_STMT(get_version_stmt);
	ALLOC_STMT(add_ancestry_edge_stmt);
	ALLOC_STMT(has_immediate_ancestor_stmt);
	ALLOC_STMT(has_immediate_ancestor_with_ver_stmt);

#undef ALLOC_STMT


	// Prepare the statements

#define PREPARE(handle, text) { \
	ret = SQLPrepare(odbc->handle, (SQLCHAR*) text, SQL_NTS); \
	if (!SQL_SUCCEEDED(ret)) { \
		r = CPL_E_PREPARE_STATEMENT_ERROR; \
		print_odbc_error("SQLPrepare", odbc->handle, SQL_HANDLE_STMT); \
		goto err_stmts; \
	}}

	if (db_type == CPL_ODBC_POSTGRESQL) {
		// TODO Check whether this works
		PREPARE(create_object_insert_stmt,
				"INSERT INTO cpl_objects SET originator=?, name=?, type=? "
				"RETURNING id;");
		PREPARE(create_object_insert_container_stmt,
				"INSERT INTO cpl_objects SET originator=?, name=?, type=?, "
				"container_id=?, container_ver=? RETURNING id;");
	}
	else {
		PREPARE(create_object_insert_stmt,
				"INSERT INTO cpl_objects SET originator=?, name=?, type=?;");
		PREPARE(create_object_insert_container_stmt,
				"INSERT INTO cpl_objects SET originator=?, name=?, type=?, "
				"container_id=?, container_ver=?;");
	}

	if (db_type == CPL_ODBC_MYSQL) {
		// NOTE This does not work so well - and it will need to change
		// if more than one table in the database uses auto increments
		PREPARE(create_object_get_id_stmt,
				"SELECT LAST_INSERT_ID();");
	}
	else if (db_type == CPL_ODBC_POSTGRESQL) {
		// TODO Check whether this works
		PREPARE(create_object_get_id_stmt,
				"SELECT CURRVAL('cpl_objects_id_seq');");
	}
	else {
		// This is ugly
		PREPARE(create_object_get_id_stmt,
				"SELECT MAX(id) FROM cpl_objects;");
	}

	PREPARE(create_object_insert_version_stmt,
			"INSERT INTO cpl_versions SET id=?, version=0;");

	PREPARE(lookup_object_stmt,
			"SELECT MAX(id) FROM cpl_objects WHERE originator=? "
			"AND name=? AND type=?;");

	PREPARE(create_version_stmt,
			"INSERT INTO cpl_versions SET id=?, version=?;");

	PREPARE(get_version_stmt,
			"SELECT MAX(version) FROM cpl_versions WHERE id=?;");

	PREPARE(add_ancestry_edge_stmt,
			"INSERT INTO cpl_ancestry SET from_id=?, from_version=?, "
			"to_id=?, to_version=?, type=?;");

	PREPARE(has_immediate_ancestor_stmt,
			"SELECT to_version FROM cpl_ancestry WHERE to_id=? AND "
			"to_version<=? AND from_id=? LIMIT 1;");

	PREPARE(has_immediate_ancestor_with_ver_stmt,
			"SELECT to_version FROM cpl_ancestry WHERE to_id=? AND "
			"to_version<=? AND from_id=? AND from_version<=? LIMIT 1;");

#undef PREPARE


	// Return

	*out = (cpl_db_backend_t*) odbc;
	return CPL_OK;


	// Error handling -- the variable r must be set

	assert(!CPL_IS_OK(r));

err_stmts:
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_get_id_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_ancestry_edge_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_with_ver_stmt);

	SQLDisconnect(odbc->db_connection);

err_handles:
	SQLFreeHandle(SQL_HANDLE_DBC, odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, odbc->db_environment);

err_sync:
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->create_version_lock);
	mutex_destroy(odbc->get_version_lock);
	mutex_destroy(odbc->add_ancestry_edge_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);

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

	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_get_id_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->lookup_object_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->create_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->get_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->add_ancestry_edge_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, odbc->has_immediate_ancestor_with_ver_stmt);

	ret = SQLDisconnect(odbc->db_connection);
	if (!SQL_SUCCEEDED(ret)) {
		fprintf(stderr, "Warning: Could not terminate the ODBC connection.\n");
	}
	
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->lookup_object_lock);
	mutex_destroy(odbc->create_version_lock);
	mutex_destroy(odbc->get_version_lock);
	mutex_destroy(odbc->add_ancestry_edge_lock);
	mutex_destroy(odbc->has_immediate_ancestor_lock);

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
	ret = SQLBindParameter(stmt, arg, SQL_PARAM_INPUT, \
			SQL_C_CHAR, SQL_VARCHAR, size, 0, \
			(SQLCHAR*) (value), 0, NULL); \
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
	*__p = value; \
	ret = SQLBindParameter(stmt, arg, SQL_PARAM_INPUT, \
			SQL_C_SBIGINT, SQL_INTEGER, 0, 0, \
			(void*) __p, 0, NULL); \
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err); \
}


/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


/**
 * Create an object.
 *
 * @param backend the pointer to the backend structure
 * @param originator the originator
 * @param name the object name
 * @param type the object type
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @param container_version the version of the container (if not CPL_NONE)
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_odbc_create_object(struct _cpl_db_backend_t* backend,
					   const char* originator,
					   const char* name,
					   const char* type,
					   const cpl_id_t container,
					   const cpl_version_t container_version)
{
	assert(backend != NULL && originator != NULL && name != NULL && type != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_id_t r = CPL_NONE;

	mutex_lock(odbc->create_object_lock);

	
	// Bind the statement parameters

	SQLHSTMT stmt = container == CPL_NONE
		? odbc->create_object_insert_stmt
		: odbc->create_object_insert_container_stmt;

	SQL_BIND_VARCHAR(stmt, 1, 255, originator);
	SQL_BIND_VARCHAR(stmt, 2, 255, name);
	SQL_BIND_VARCHAR(stmt, 3, 100, type);

	if (container != CPL_NONE) {
		SQL_BIND_INTEGER(stmt, 4, container);
		SQL_BIND_INTEGER(stmt, 5, container_version);
	}


	// Insert the new row to the objects table and get the last insert ID

	if (odbc->db_type == CPL_ODBC_POSTGRESQL) {

		// Execute the insert statement and get the object ID at the same time

		ret = SQLExecute(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
			goto err;
		}
	}
	else {

		// Execute the insert statement

		ret = SQLExecute(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
			goto err;
		}


		// Determine the object ID

		stmt = odbc->create_object_get_id_stmt;
		
		ret = SQLExecute(stmt);
		if (!SQL_SUCCEEDED(ret)) {
			print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
			goto err;
		}
	}


	// Get the object ID from the result set

	r = cpl_sql_fetch_single_llong(stmt);
	if (r == CPL_NONE) r = CPL_E_BACKEND_INTERNAL_ERROR;
	if (r == CPL_E_NOT_FOUND) r = CPL_E_BACKEND_INTERNAL_ERROR;
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->create_object_lock);
		return r;
	}


	// Insert the corresponding entry to the versions table

	stmt = odbc->create_object_insert_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, r);
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}

	mutex_unlock(odbc->create_object_lock);
	return r;


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
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_odbc_lookup_object(struct _cpl_db_backend_t* backend,
					   const char* originator,
					   const char* name,
					   const char* type)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	long long r;

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

	r = cpl_sql_fetch_single_llong(stmt);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->lookup_object_lock);
		return r;
	}


	// Cleanup

	mutex_unlock(odbc->lookup_object_lock);
	return r;


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
 * @return the error code
 */
cpl_return_t
cpl_odbc_create_version(struct _cpl_db_backend_t* backend,
						const cpl_id_t object_id,
						const cpl_version_t version)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;

	mutex_lock(odbc->create_version_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->create_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, object_id);
	SQL_BIND_INTEGER(stmt, 2, version);


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
 * @return the object version or an error code
 */
extern "C" cpl_version_t
cpl_odbc_get_version(struct _cpl_db_backend_t* backend,
					 const cpl_id_t id)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	long long r;

	mutex_lock(odbc->get_version_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->get_version_stmt;
	SQL_BIND_INTEGER(stmt, 1, id);


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the result

	r = cpl_sql_fetch_single_llong(stmt);
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->get_version_lock);
		return (cpl_version_t) r;
	}


	// Cleanup

	mutex_unlock(odbc->get_version_lock);
	return (cpl_version_t) r;


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
	
	SQLRETURN ret;

	mutex_lock(odbc->add_ancestry_edge_lock);


	// Prepare the statement

	SQLHSTMT stmt = odbc->add_ancestry_edge_stmt;
	SQL_BIND_INTEGER(stmt, 1, from_id);
	SQL_BIND_INTEGER(stmt, 2, from_ver);
	SQL_BIND_INTEGER(stmt, 3, to_id);
	SQL_BIND_INTEGER(stmt, 4, to_ver);
	SQL_BIND_INTEGER(stmt, 5, type);


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
 * @return a positive number if yes, 0 if no, or a negative error code
 */
extern "C" cpl_return_t
cpl_odbc_has_immediate_ancestor(struct _cpl_db_backend_t* backend,
								const cpl_id_t object_id,
								const cpl_version_t version_hint,
								const cpl_id_t query_object_id,
								const cpl_version_t query_object_max_version)
{
	assert(backend != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	long long r;
	cpl_return_t cr = CPL_E_INTERNAL_ERROR;

	mutex_lock(odbc->has_immediate_ancestor_lock);


	// Prepare the statement

	SQLHSTMT stmt = version_hint == CPL_VERSION_NONE 
		? odbc->has_immediate_ancestor_stmt
		: odbc->has_immediate_ancestor_with_ver_stmt;
	SQL_BIND_INTEGER(stmt, 1, query_object_id);
	SQL_BIND_INTEGER(stmt, 2, query_object_max_version);
	SQL_BIND_INTEGER(stmt, 3, object_id);
	if (version_hint != CPL_VERSION_NONE) {
		SQL_BIND_INTEGER(stmt, 4, version_hint);
	}


	// Execute
	
	ret = SQLExecute(stmt);
	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLExecute", stmt, SQL_HANDLE_STMT);
		goto err;
	}


	// Fetch the data

	r = cpl_sql_fetch_single_llong(stmt);
	if (r >= 0) cr = 1;
	if (r == CPL_E_NOT_FOUND) { r = CPL_OK; cr = 0; }
	if (!CPL_IS_OK(r)) {
		mutex_unlock(odbc->has_immediate_ancestor_lock);
		return r;
	}


	// Cleanup

	mutex_unlock(odbc->has_immediate_ancestor_lock);
	return cr;


	// Error handling

err:
	mutex_unlock(odbc->has_immediate_ancestor_lock);
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
	cpl_odbc_create_object,
	cpl_odbc_lookup_object,
	cpl_odbc_create_version,
	cpl_odbc_get_version,
	cpl_odbc_add_ancestry_edge,
	cpl_odbc_has_immediate_ancestor,
};

