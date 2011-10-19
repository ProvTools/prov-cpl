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

	SQLINTEGER	 i = 0;
	SQLINTEGER	 native;
	SQLCHAR	 state[ 7 ];
	SQLCHAR	 text[256];
	SQLSMALLINT	 len;
	SQLRETURN	 ret;

	fprintf(stderr,
			"\n"
			"The ODBC driver reported the following while running "
			"%s:\n\n",
			fn);

	do {
		ret = SQLGetDiagRec(type, handle, ++i, state, &native,
							text, sizeof(text), &len);
		if (SQL_SUCCEEDED(ret)) {
			printf("%s:%ld:%ld:%s\n", state, (long) i, (long) native, text);
		}
	}
	while (ret == SQL_SUCCESS);
}



/***************************************************************************/
/** Constructor and Destructor                                            **/
/***************************************************************************/

/**
 * Create an ODBC backend
 *
 * @param connection_string the ODBC connection string
 * @param out the pointer to the database backend variable
 * @return the error code
 */
extern "C" cpl_return_t
cpl_create_odbc_backend(const char* connection_string,
						cpl_db_backend_t** out)
{
	assert(out != NULL);
	assert(connection_string != NULL);


	// Allocate the backend struct

	cpl_odbc_t* odbc = new cpl_odbc_t;
	if (odbc == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memcpy(&odbc->interface, &CPL_ODBC_BACKEND, sizeof(odbc->interface));


	// Open the ODBC connection

	SQLRETURN ret;
	SQLCHAR outstr[1024];
	SQLCHAR* connection_string_copy;
	SQLSMALLINT outstrlen;

	connection_string_copy = (SQLCHAR*) malloc(strlen(connection_string) + 1);
	if (connection_string_copy == NULL) {
		delete odbc;
		return CPL_E_INSUFFICIENT_RESOURCES;
	}
	strcpy((char*) connection_string_copy, connection_string);

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &odbc->db_environment);
	SQLSetEnvAttr(&odbc->db_environment,
				  SQL_ATTR_ODBC_VERSION,
				  (void *) SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, &odbc->db_environment, &odbc->db_connection);
	
	ret = SQLDriverConnect(&odbc->db_connection, NULL,
						   connection_string_copy, SQL_NTS,
						   outstr, sizeof(outstr), &outstrlen,
						   SQL_DRIVER_COMPLETE);
	free(connection_string_copy);
	connection_string_copy = NULL;

	if (!SQL_SUCCEEDED(ret)) {
		print_odbc_error("SQLDriverConnect",
						 &odbc->db_connection, SQL_HANDLE_DBC);
		SQLFreeHandle(SQL_HANDLE_DBC, &odbc->db_connection);
		SQLFreeHandle(SQL_HANDLE_ENV, &odbc->db_environment);
		delete odbc;
		return CPL_E_DB_CONNECTION_ERROR;
	}


	// Return

	*out = (cpl_db_backend_t*) odbc;
	return CPL_OK;
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
	return CPL_E_NOT_IMPLEMENTED;
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
	return CPL_E_NOT_IMPLEMENTED;
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

