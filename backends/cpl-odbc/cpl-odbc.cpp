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
extern "C" cpl_return_t
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
	mutex_init(odbc->get_version_lock);


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
	ALLOC_STMT(get_version_stmt);

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
				"INSERT INTO cpl_objects SET name=?, type=? "
				"RETURNING id;");
		PREPARE(create_object_insert_container_stmt,
				"INSERT INTO cpl_objects SET name=?, type=?, "
				"container_id=?, container_ver=? RETURNING id;");
	}
	else {
		PREPARE(create_object_insert_stmt,
				"INSERT INTO cpl_objects SET name=?, type=?;");
		PREPARE(create_object_insert_container_stmt,
				"INSERT INTO cpl_objects SET name=?, type=?, "
				"container_id=?, container_ver=?;");
	}

	if (db_type == CPL_ODBC_MYSQL) {
		// XXX This does not work so well - and it will need to change
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

	PREPARE(get_version_stmt,
			"SELECT MAX(version) FROM cpl_versions WHERE id=?;");

#undef PREPARE


	// Return

	*out = (cpl_db_backend_t*) odbc;
	return CPL_OK;


	// Error handling -- the variable r must be set

	assert(!CPL_IS_OK(r));

err_stmts:
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_get_id_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->get_version_stmt);

	SQLDisconnect(odbc->db_connection);

err_handles:
	SQLFreeHandle(SQL_HANDLE_DBC, &odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, &odbc->db_environment);

err_sync:
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->get_version_lock);

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
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;

	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_container_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_get_id_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->create_object_insert_version_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, &odbc->get_version_stmt);

	SQLDisconnect(odbc->db_connection);
	
	mutex_destroy(odbc->create_object_lock);
	mutex_destroy(odbc->get_version_lock);

	SQLFreeHandle(SQL_HANDLE_DBC, &odbc->db_connection);
	SQLFreeHandle(SQL_HANDLE_ENV, &odbc->db_environment);
	
	delete odbc;
	
	return CPL_OK;
}



/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


/**
 * Create an object.
 *
 * @param backend the pointer to the backend structure
 * @param originator the originator ID
 * @param name the object name
 * @param type the object type
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @param container_version the version of the container (if not CPL_NONE)
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_odbc_create_object(struct _cpl_db_backend_t* backend,
					   const cpl_id_t originator,
					   const char* name,
					   const char* type,
					   const cpl_id_t container,
					   const cpl_version_t container_version)
{
	assert(backend != NULL && name != NULL && type != NULL);
	cpl_odbc_t* odbc = (cpl_odbc_t*) backend;
	
	SQLRETURN ret;
	cpl_id_t r = CPL_NONE;

	mutex_lock(odbc->create_object_lock);

	
	// Bind the statement parameters

	SQLHSTMT stmt = container == CPL_NONE
		? odbc->create_object_insert_stmt
		: odbc->create_object_insert_container_stmt;

	ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
			SQL_C_CHAR, SQL_VARCHAR, 255, 0,
			(SQLCHAR*) name, 0, NULL);
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);

	ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
			SQL_C_CHAR, SQL_VARCHAR, 100, 0,
			(SQLCHAR*) type, 0, NULL);
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);

	if (container != CPL_NONE) {
		long _container_version = (long) container_version;

		ret = SQLBindParameter(stmt, 3, SQL_PARAM_INPUT,
				SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
				(void*) &container, 0, NULL);
		SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);

		ret = SQLBindParameter(stmt, 4, SQL_PARAM_INPUT,
				SQL_C_SLONG, SQL_INTEGER, 0, 0,
				(void*) &_container_version, 0, NULL);
		SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);
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

	ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
			SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
			(void*) &r, 0, NULL);
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);
	
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
 * @param name the object name
 * @param type the object type
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_odbc_lookup_by_name(struct _cpl_db_backend_t* backend,
						const char* name,
						const char* type)
{
	return CPL_E_NOT_IMPLEMENTED;
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

	ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
			SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
			(void*) &id, 0, NULL);
	SQL_ASSERT_NO_ERROR(SQLBindParameter, stmt, err);


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
 * @return the error code
 */
extern "C" cpl_return_t
cpl_odbc_add_ancestry_edge(struct _cpl_db_backend_t* backend,
						   const cpl_id_t from_id,
						   const cpl_version_t from_ver,
						   const cpl_id_t to_id,
						   const cpl_version_t to_ver)
{
	return CPL_E_NOT_IMPLEMENTED;
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
	cpl_odbc_lookup_by_name,
	cpl_odbc_get_version,
	cpl_odbc_add_ancestry_edge,
};

