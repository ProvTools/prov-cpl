/*
 * cpl-odbc.h
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

#ifndef __CPL_ODBC_H__
#define __CPL_ODBC_H__

#include <cpl-db-backend.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


/***************************************************************************/
/** Database Types                                                        **/
/***************************************************************************/

/**
 * A generic SQL database
 */
#define CPL_ODBC_GENERIC		0
#define CPL_ODBC_UNKNOWN		CPL_ODBC_GENERIC

/**
 * MySQL
 */
#define CPL_ODBC_MYSQL			1

/**
 * PostgreSQL
 */
#define CPL_ODBC_POSTGRESQL		2



/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

/**
 * The combination of CPL_A_* flags that are currently supported by the driver.
 */
 /*
#define CPL_ODBC_A_SUPPORTED_FLAGS \
	(CPL_A_NO_PREV_NEXT_VERSION | CPL_A_NO_CONTROL_DEPENDENCIES \
     | CPL_A_NO_DATA_DEPENDENCIES)
*/

#define CPL_MAC_ADDR_LEN			18
#define CPL_USER_LEN				255
#define CPL_PROGRAM_LEN				4095
#define CPL_CMDLINE_LEN				4095
#define CPL_PREFIX_LEN				255
#define CPL_NAME_LEN				255
#define CPL_KEY_LEN					255
#define CPL_VALUE_LEN				4095
#define CPL_STMT_MAX				4


/***************************************************************************/
/** Constructor                                                           **/
/***************************************************************************/

/**
 * Create an ODBC backend
 *
 * @param connection_string the ODBC connection string
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
EXPORT cpl_return_t
cpl_create_odbc_backend(const char* connection_string,
						int db_type,
						cpl_db_backend_t** out);


/**
 * Create an ODBC backend
 *
 * @param dsn the data source name
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
EXPORT cpl_return_t
cpl_create_odbc_backend_dsn(const char* dsn,
							int db_type,
							cpl_db_backend_t** out);

#ifdef __cplusplus
}
#endif

#endif

