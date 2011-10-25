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

#include <sql.h>
#include <sqlext.h>

#if !(defined _WIN32 || defined _WIN64)
#include <pthread.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


/***************************************************************************/
/** Cross-Platform Compatibility                                          **/
/***************************************************************************/

#if defined _WIN32 || defined _WIN64

/**
 * Mutex
 */
typedef CRITICAL_SECTION mutex_t;

/**
 * Initialize a mutex
 *
 * @param m the mutex
 */
#define mutex_init(m) InitializeCriticalSection(&(m));

/**
 * Destroy a mutex
 *
 * @param m the mutex
 */
#define mutex_destroy(m) DeleteCriticalSection(&(m));

/**
 * Lock a mutex
 *
 * @param m the mutex
 */
#define mutex_lock(m) EnterCriticalSection(&(m));

/**
 * Unlock a mutex
 *
 * @param m the mutex
 */
#define mutex_unlock(m) LeaveCriticalSection(&(m));

#else

/**
 * Mutex
 */
typedef pthread_mutex_t mutex_t;

/**
 * Initialize a mutex
 *
 * @param m the mutex
 */
#define mutex_init(m) pthread_mutex_init(&(m), NULL);

/**
 * Destroy a mutex
 *
 * @param m the mutex
 */
#define mutex_destroy(m) pthread_mutex_destroy(&(m));

/**
 * Lock a mutex
 *
 * @param m the mutex
 */
#define mutex_lock(m) pthread_mutex_lock(&(m));

/**
 * Unlock a mutex
 *
 * @param m the mutex
 */
#define mutex_unlock(m) pthread_mutex_unlock(&(m));

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
	cpl_db_backend_t interface;

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
	 * The lock for get_version
	 */
	mutex_t get_version_lock;

	/**
	 * The statement that determines the version of an object given its ID
	 */
	SQLHSTMT get_version_stmt;

} cpl_odbc_t;


/**
 * The ODBC interface
 */
extern const cpl_db_backend_t CPL_ODBC_BACKEND;


#ifdef __cplusplus
}
#endif

#endif

