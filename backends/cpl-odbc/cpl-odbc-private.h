/*
 * cpl-odbc-private.h
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

#ifndef __CPL_ODBC_PRIVATE_H__
#define __CPL_ODBC_PRIVATE_H__

#include <backends/cpl-odbc.h>
#include <private/cpl-platform.h>

#include <sql.h>
#include <sqlext.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif



/***************************************************************************/
/** ODBC Database Backend                                                 **/
/***************************************************************************/

/**
 * The ODBC database backend
 */
typedef struct {

	/**
	 * The backend interface (must be first)
	 */
	cpl_db_backend_t backend;

	/**
	 * The ODBC environment
	 */
	SQLHENV db_environment;

	/**
	 * The ODBC database connection handle
	 */
	SQLHDBC db_connection;

	/**
	 * The database type
	 */
	int db_type;

	/**
	 * Lock for getting insert ID
	 */
	mutex_t get_insert_id_lock;

	/**
	 * Lock for session creation
	 */
	mutex_t create_session_lock;

	/**
	 * The insert statement for session creation
	 */
	SQLHSTMT create_session_insert_stmt;

	/**
	 * The statement that determines the ID of the session that was just created
	 */
	SQLHSTMT create_session_get_id_stmt;

	/**
	 * Lock for object creation
	 */
	mutex_t create_object_lock;

	/**
	 * The insert statement for object creation
	 */
	SQLHSTMT create_object_insert_stmt;

	/**
	 * The insert statement for object creation - with container
	 */
	SQLHSTMT create_object_insert_container_stmt;

	/**
	 * The statement that determines the ID of the object that was just created
	 */
	SQLHSTMT create_object_get_id_stmt;

	/**
	 * Insert a new 0th version to the versions table (to be used while
	 * creating a new object)
	 */
	SQLHSTMT create_object_insert_version_stmt;

	/**
	 * The lock for lookup_object
	 */
	mutex_t lookup_object_lock;

	/**
	 * The statement for looking up an object by name (including originator
	 * and type)
	 */
	SQLHSTMT lookup_object_stmt;

	/**
	 * Lock for version creation
	 */
	mutex_t create_version_lock;

	/**
	 * The insert statement for version creation
	 */
	SQLHSTMT create_version_stmt;

	/**
	 * The lock for get_version
	 */
	mutex_t get_version_lock;

	/**
	 * The statement that determines the version of an object given its ID
	 */
	SQLHSTMT get_version_stmt;

	/**
	 * The lock for add_ancestry_edge
	 */
	mutex_t add_ancestry_edge_lock;

	/**
	 * The statement that adds a new ancestry edge
	 */
	SQLHSTMT add_ancestry_edge_stmt;

	/**
	 * The lock for has_immediate_ancestor
	 */
	mutex_t has_immediate_ancestor_lock;

	/**
	 * The statement that determines whether the given object is present
	 * in the immediate ancestry
	 */
	SQLHSTMT has_immediate_ancestor_stmt;

	/**
	 * The statement that determines whether the given object is present
	 * in the immediate ancestry -- also considering the version of the object
	 */
	SQLHSTMT has_immediate_ancestor_with_ver_stmt;

} cpl_odbc_t;


/**
 * The ODBC interface
 */
extern const cpl_db_backend_t CPL_ODBC_BACKEND;


#ifdef __cplusplus
}
#endif

#endif

