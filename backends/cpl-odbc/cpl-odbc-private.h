/*
 * cpl-odbc-private.h
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

#ifndef __CPL_ODBC_PRIVATE_H__
#define __CPL_ODBC_PRIVATE_H__

#include <backends/cpl-odbc.h>
#include <private/cpl-platform.h>
#include <cplxx.h>

#ifdef __APPLE__
	#include </usr/local/include/sql.h>
	#include </usr/local/include/sqlext.h>
#else
	#include <sql.h>
	#include <sqlext.h>
#endif
	

#include <string>



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
	 * The connection string
	 */
	std::string connection_string;

	/**
	 * Lock for session creation
	 */
	sema_t create_session_sem;
	mutex_t create_session_lock;

	/**
	 * The insert statement for session creation
	 */
	SQLHSTMT* create_session_stmts;

	/**
	 * Lock for object creation
	 */
	sema_t create_object_sem;
	mutex_t create_object_lock;

	/**
	 * The insert statement for object creation
	 */
	SQLHSTMT* create_object_stmts;

	/**
	 * The lock for lookup_object
	 */
	sema_t lookup_object_nt_sem;
	mutex_t lookup_object_nt_lock;
	sema_t lookup_object_t_sem;
	mutex_t lookup_object_t_lock;
	/**
	 * The statements for looking up an object by name and originator
	 * and optionally type
	 */
	SQLHSTMT* lookup_object_nt_stmts;
	SQLHSTMT* lookup_object_t_stmts;

	/**
	 * The lock for lookup_object_ext
	 */
	sema_t lookup_object_nt_ext_sem;
	mutex_t lookup_object_nt_ext_lock;
	sema_t lookup_object_t_ext_sem;
	mutex_t lookup_object_t_ext_lock;

	/**
	 * The statements for looking up objects by name and originator
	 * and optionally type
	 */
	SQLHSTMT* lookup_object_nt_ext_stmts;
	SQLHSTMT* lookup_object_t_ext_stmts;

	/**
	 * The lock for add_relation_edge
	 */
	sema_t add_relation_sem;
	mutex_t add_relation_lock;
	sema_t bundle_relation_helper_sem;
	mutex_t bundle_relation_helper_lock;

	/**
	 * The statement that adds a new relation
	 */
	SQLHSTMT* add_relation_stmts;
	SQLHSTMT* bundle_relation_helper_stmts;
	/**
	 * Lock for bundle creation
	 */
	sema_t create_bundle_sem;
	mutex_t create_bundle_lock;

	/**
	 * The insert statement for object creation - with bundle
	 */
	SQLHSTMT* create_bundle_stmts;

	/**
	 * The lock for lookup_bundle
	 */
	sema_t lookup_bundle_sem;
	mutex_t lookup_bundle_lock;

	/**
	 * The statement for looking up a bundle
	 */
	SQLHSTMT* lookup_bundle_stmts;
	/**
	 * The lock for lookup_object_ext
	 */
	sema_t lookup_bundle_ext_sem;
	mutex_t lookup_bundle_ext_lock;

	/**
	 * The statement for looking up bundles
	 */
	SQLHSTMT* lookup_bundle_ext_stmts;

    /**
    * The lock for lookup_relation
    */
    sema_t lookup_relation_sem;
    mutex_t lookup_relation_lock;

    /**
     * The statement for looking up a relation
     */
    SQLHSTMT* lookup_relation_stmts;

    /**
     * The lock for searching by object property value, with wildcards
     */
    sema_t lookup_object_property_wildcard_sem;
    mutex_t lookup_object_property_wildcard_lock;

    /**
     * The statement for searching by object property value, with wildcards
     */
    SQLHSTMT* lookup_object_property_wildcard_stmts;

    /**
	 * The lock for add_object_property
	 */
	sema_t add_object_property_sem;
	mutex_t add_object_property_lock;

	/**
	 * The statement that adds a new object property
	 */
	SQLHSTMT* add_object_property_stmts;

	/**
	* The lock for add_relation_property
	*/
	sema_t add_relation_property_sem;
	mutex_t add_relation_property_lock;

	/**
	* The statement that adds a new relation property
	*/
	SQLHSTMT* add_relation_property_stmts;

	/**
	* The lock for add_bundle_property
	*/
	sema_t add_bundle_property_sem;
	mutex_t add_bundle_property_lock;

	/**
	* The statement that adds a new bundle property
	*/
	SQLHSTMT* add_bundle_property_stmts;

	/**
	* The lock for add_prefix
	*/
	sema_t add_prefix_sem;
	mutex_t add_prefix_lock;

	/**
	* The statement that adds a new prefix
	*/
	SQLHSTMT* add_prefix_stmts;

	/*s*
	 * The lock for get_session_info
	 */
	sema_t get_session_info_sem;
	mutex_t get_session_info_lock;

	/**
	 * The statement that returns information about a provenance session
	 */
	SQLHSTMT* get_session_info_stmts;

	/**
	 * The lock for get_all_objects
	 */
	sema_t get_all_objects_sem;
	mutex_t get_all_objects_lock;

	/**
	 * The statement that returns information about all provenance objects
	 */
	SQLHSTMT* get_all_objects_stmts;

	/**
	 * The lock for get_object_info
	 */
	sema_t get_object_info_sem;
	mutex_t get_object_info_lock;

	/**
	 * The statement that returns information about a provenance object
	 */
	SQLHSTMT* get_object_info_stmts;

	/**
	 * The mutex for get_object_relations
	 */
	sema_t get_object_ancestors_sem;
	mutex_t get_object_ancestors_lock;
	sema_t get_object_descendants_sem;
	mutex_t get_object_descendants_lock;
	/**
	 * The statement for listing ancestors
	 */
	SQLHSTMT* get_object_ancestors_stmts;

	/**
	 * The statement for listing descendants
	 */
	SQLHSTMT* get_object_descendants_stmts;

	/**
	 * The mutex for get_object_properties
	 */
	sema_t get_object_properties_sem;
	mutex_t get_object_properties_lock;
	sema_t get_object_properties_with_key_sem;
	mutex_t get_object_properties_with_key_lock;

	/**
	 * The statement for listing object properties
	 */
	SQLHSTMT* get_object_properties_stmts;

	/**
	 * The statement for listing specific object properties
	 */
	SQLHSTMT* get_object_properties_with_key_stmts;

	/**
	 * The mutex for lookup_object_by_property
	 */
	sema_t lookup_object_by_property_sem;
	mutex_t lookup_object_by_property_lock;

	/**
	 * The statement for looking up by a property value
	 */
	SQLHSTMT* lookup_object_by_property_stmts;

	/**
	* The mutex for get_relation_properties
	*/
	sema_t get_relation_properties_sem;
	mutex_t get_relation_properties_lock;
	sema_t get_relation_properties_with_key_sem;
	mutex_t get_relation_properties_with_key_lock;

	/**
	* The statement for listing object properties
	*/
	SQLHSTMT* get_relation_properties_stmts;

	/**
	* The statement for listing specific object properties
	*/
	SQLHSTMT* get_relation_properties_with_key_stmts;
	
	/**
	* The mutex for has_immediate_ancestor
	*/
	sema_t has_immediate_ancestor_sem;
	mutex_t has_immediate_ancestor_lock;

	/**
	* The statement for determining whether an object has a particular ancestor
	*/
	SQLHSTMT* has_immediate_ancestor_stmts;

	/**
	* The mutex for delete_bundle
	*/
	sema_t delete_bundle_sem;
	mutex_t delete_bundle_lock;

	/**
	* The statement for deleting a bundle
	*/
	SQLHSTMT* delete_bundle_stmts;

	/**
	 * The lock for get_bundle_info
	 */
	sema_t get_bundle_info_sem;
	mutex_t get_bundle_info_lock;

	/**
	 * The statement that returns information about a provenance bundle
	 */
	SQLHSTMT* get_bundle_info_stmts;
	
	/**
	* The mutex for get_bundle_objects
	*/
	sema_t get_bundle_objects_sem;
	mutex_t get_bundle_objects_lock;

	/**
	* The statement for getting bundle objects
	*/
	SQLHSTMT* get_bundle_objects_stmts;

	/**
	* The mutex for get_bundle_relations
	*/
	sema_t get_bundle_relations_sem;
	mutex_t get_bundle_relations_lock;

	/**
	* The statement for getting bundle relations
	*/
	SQLHSTMT* get_bundle_relations_stmts;

	/**
	* The mutex for get_bundle_properties
	*/
	sema_t get_bundle_properties_sem;
	mutex_t get_bundle_properties_lock;
	sema_t get_bundle_properties_with_key_sem;
	mutex_t get_bundle_properties_with_key_lock;
	/**
	* The statement for getting bundle properties
	*/
	SQLHSTMT* get_bundle_properties_stmts;

	/**
	* The statement for listing specific bundle properties
	*/
	SQLHSTMT* get_bundle_properties_with_key_stmts;

	/**
	* The mutex for get_prefixes
	*/
	sema_t get_prefixes_sem;
	mutex_t get_prefixes_lock;
	sema_t get_prefixes_with_key_sem;
	mutex_t get_prefixes_with_key_lock;

	/**
	* The statement for getting prefixes
	*/
	SQLHSTMT* get_prefixes_stmts;

	/**
	* The statement for listing specific bundle prefixes
	*/
	SQLHSTMT* get_prefixes_with_key_stmts;

} cpl_odbc_t;


/**
 * The ODBC interface
 */
extern const cpl_db_backend_t CPL_ODBC_BACKEND;



/***************************************************************************/
/** Miscellaneous                                                         **/
/***************************************************************************/

/**
 * ODBC error record
 */
typedef struct {

	SQLSMALLINT index;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[256];
	SQLSMALLINT length;

} cpl_odbc_error_record_t;


#endif

