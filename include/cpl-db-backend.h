/*
 * cpl-db-backend.h
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

#ifndef __CPL_DB_BACKEND_H__
#define __CPL_DB_BACKEND_H__

#include <cpl.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

#define EXPORT


/***************************************************************************/
/** Database Backend Interface                                            **/
/***************************************************************************/

typedef struct _cpl_db_backend_t {

	/**
	 * Destructor. If the constructor allocated the backend structure, it
	 * should be freed by this function
	 *
	 * @param backend the pointer to the backend structure
	 * @param the error code
	 */
	cpl_return_t
	(*cpl_db_destroy)(struct _cpl_db_backend_t* backend);

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
	cpl_return_t
	(*cpl_db_create_session)(struct _cpl_db_backend_t* backend,
							 cpl_session_t* out_id,
							 const char* mac_address,
							 const char* user,
							 const int pid,
							 const char* program,
							 const char* cmdline);

	/**
	 * Create an object.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the ID of the new object
	 * @param originator the namespace prefix
	 * @param name the object name
	 * @param type the object type
	 * @param bundle the ID of the bundle that should contain this object
	 * @param session the session ID responsible for this provenance record
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_create_object)(struct _cpl_db_backend_t* backend,
							const char* prefix,
							const char* name,
							const int type,
							cpl_id_t* out_id);

	/**
	 * Look up an object by name. If multiple objects share the same name,
	 * get the latest one.
	 *
	 * @param backend the pointer to the backend structure
	 * @param prefix the namespace prefix
	 * @param name the object name
	 * @param type the object type
	 * @param out_id the pointer to store the object ID
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_object)(struct _cpl_db_backend_t* backend,
							const char* prefix,
							const char* name,
					   		const int type,
							cpl_id_t* out_id);

	/**
	 * Look up an object by name. If multiple objects share the same name,
	 * return all of them.
	 *
	 * @param backend the pointer to the backend structure
	 * @param prefix the namespace prefix
	 * @param name the object name
	 * @param type the object type
	 * @param flags a logical combination of CPL_L_* flags
	 * @param iterator the iterator to be called for each matching object
	 * @param context the caller-provided iterator context
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_object_ext)(struct _cpl_db_backend_t* backend,
								const char* prefix,
								const char* name,
								const int type,
								const int flags,
								cpl_id_timestamp_iterator_t callback,
								void* context);

    /**
     * Add a property to the given object
     *
	 * @param backend the pointer to the backend structure
	 * @param the namespace prefix
     * @param id the object ID
     * @param key the key
     * @param value the value
     * @return CPL_OK or an error code
     */

    cpl_return_t
    (*cpl_db_add_object_property)(struct _cpl_db_backend_t* backend,
                           const cpl_id_t id,
                           const char* prefix,
                           const char* key,
                           const char* value);
	/**
	 * Add a relation
	 *
	 * @param backend the pointer to the backend structure
	 * @param from_id the edge source ID
	 * @param to_id the edge destination ID
	 * @param type the type
	 * @param bundle the bundle ID
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_add_relation)(struct _cpl_db_backend_t* backend,
								const cpl_id_t from_id,
								const cpl_id_t to_id,
								const int type,
								cpl_id_t* out_id);

    /**
     * Add a property to the given relation
     *
	 * @param backend the pointer to the backend structure
	 * @param the namespace prefix
     * @param id the object ID
     * @param key the key
     * @param value the value
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_add_relation_property)(struct _cpl_db_backend_t* backend,
                           const cpl_id_t id,
                           const char* prefix,
                           const char* key,
                           const char* value);
	
	/**
	 * Create a bundle.
	 *
	 * @param backend the pointer to the backend structure
	 * @param name the object name
	 * @param out_id the pointer to store the ID of the newly created object
	 * @return CPL_OK or an error code
	 */
    cpl_return_t
    (*cpl_db_create_bundle)(struct _cpl_db_backend_t* backend,
    						const char* name,
					  		const cpl_session_t session,
    						cpl_id_t* out_id);

	/**
	 * Look up a bundle by name. If multiple bundles share the same name,
	 * get the latest one.
	 *
	 * @param name the bundle name
	 * @param out_id the pointer to store the object ID
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_bundle)(struct _cpl_db_backend_t* backend,
							const char* name,
					  		cpl_id_t* out_id);

	/**
	 * Look up a bundle by name. If multiple bundles share the same name,
	 * return all of them.
	 *
	 * @param backend the pointer to the backend structure
	 * @param name the bundle name
	 * @param flags a logical combination of CPL_L_* flags
	 * @param iterator the iterator to be called for each matching bundle
	 * @param context the caller-provided iterator context
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_bundle_ext)(struct _cpl_db_backend_t* backend,
						  	const char* name,
						  	const int flags,
	 					  	cpl_id_timestamp_iterator_t iterator,
						  	void* context);

    /**
     * Lookup an relation based on from_id, to_id and type
     *
     * @param backend the pointer to the backend structure
     * @param from_id source of the relation
     * @param too_id destination of the relation
     * @param type the type of the relation
     * @param out_id pointer to store the object ID
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_lookup_relation)(struct _cpl_db_backend_t* backend,
                               const cpl_id_t from_id,
                               const cpl_id_t to_id,
                               const long type,
                               cpl_id_t* out_id);

    /**
     * Lookup an object property based on value, with wildcards
     *
     * @param backend the pointer to the backend structure
     * @param value the fragment to search for
     * @param out_id pointer to store the object ID
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_lookup_object_property_wildcard)(struct _cpl_db_backend_t* backend,
                              const char* value,
                              cpl_id_t* out_id);

    /**
     * Add a property to the given bundle
     *
	 * @param backend the pointer to the backend structure
	 * @param the namespace prefix
     * @param id the object ID
     * @param key the key
     * @param value the value
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_add_bundle_property)(struct _cpl_db_backend_t* backend,
                           const cpl_id_t id,
                           const char* prefix,
                           const char* key,
                           const char* value);

    /**
     * Add a prefix to the given bundle
     *
	 * @param backend the pointer to the backend structure
	 * @param the namespace prefix
     * @param iri the namespace iri
     * @param value the value
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_add_prefix)(struct _cpl_db_backend_t* backend,
                       const cpl_id_t id,
                       const char* prefix,
                       const char* iri);

	/**
	 * Determine whether the given object has the given ancestor
	 *
	 * @param backend the pointer to the backend structure
	 * @param object_id the object ID
	 * @param version_hint the object version (if known), or CPL_VERSION_NONE
	 *                     otherwise
	 * @param query_object_id the object that we want to determine whether it
	 *                        is one of the immediate ancestors
	 * @param query_object_max_ver the maximum version of the query
	 *                             object to consider
	 * @param out the pointer to store a positive number if yes, or 0 if no
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_has_immediate_ancestor)(struct _cpl_db_backend_t* backend,
									 const cpl_id_t object_id,
									 const cpl_id_t query_object_id,
									 int* out);

	/**
	 * Get information about the given provenance session.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the session ID
	 * @param out_info the pointer to store the session info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_session_info)(struct _cpl_db_backend_t* backend,
							   const cpl_session_t id,
							   cpl_session_info_t** out_info);

    /**
     * Get all objects in the database
     *
	 * @param backend the pointer to the backend structure
     * @param flags a logical combination of CPL_I_* flags
     * @param iterator the iterator to be called for each matching object
     * @param context the caller-provided iterator context
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_get_all_objects)(struct _cpl_db_backend_t* backend,
                              const char * prefix,
                              const int flags,
                              cpl_object_info_iterator_t callback,
                              void* context);

	/**
	 * Get information about the given provenance object
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the object ID
	 * @param out_info the pointer to store the object info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_object_info)(struct _cpl_db_backend_t* backend,
							  const cpl_id_t id,
							  cpl_object_info_t** out_info);

	/**
	 * Iterate over the ancestors or the descendants of a provenance object.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the object ID
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
	(*cpl_db_get_object_relations)(struct _cpl_db_backend_t* backend,
								  const cpl_id_t id,
								  const int direction,
								  const int flags,
								  cpl_relation_iterator_t callback,
								  void* context);

	/**
	 * Get the properties associated with the given provenance object.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the the object ID
	 * @param prefix the property prefix to fetch - or NULL 
	 *               (along with key)for all properties
	 * @param key the property key to fetch - or NULL 
	 *            (along with prefix) for all keys
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_S_NO_DATA, or an error code
	 */
	cpl_return_t
	(*cpl_db_get_object_properties)(struct _cpl_db_backend_t* backend,
							 const cpl_id_t id,
							 const char* prefix,
							 const char* key,
							 cpl_property_iterator_t callback,
							 void* context);

	/**
	 * Lookup an object based on a property value.
	 *
	 * @param backend the pointer to the backend structure
	 * @param prefix the property prefix
	 * @param key the property name
	 * @param value the property value
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_object_by_property)(struct _cpl_db_backend_t* backend,
								 const char* prefix,
								 const char* key,
								 const char* value,
								 cpl_property_iterator_t callback,
								 void* context);

	/**
	 * Get the properties associated with the given provenance relation.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the the relation ID
	 * @param prefix the property prefix to fetch - or NULL 
	 *               (along with key)for all properties
	 * @param key the property key to fetch - or NULL 
	 *            (along with prefix) for all keys
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_S_NO_DATA, or an error code
	 */
	cpl_return_t
	(*cpl_db_get_relation_properties)(struct _cpl_db_backend_t* backend,
								 const cpl_id_t id,
								 const char* prefix,
								 const char* key,
								 cpl_property_iterator_t callback,
								 void* context);


	/**
	 * Deletes a bundle along with all objects and relations it contains.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the bundle ID
	 */
	cpl_return_t
	(*cpl_db_delete_bundle)(struct _cpl_db_backend_t* backend,
								 const cpl_id_t id);
	
	/**
	 * Get information about the given provenance bundle
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the bundle ID
	 * @param out_info the pointer to store the bundle info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_bundle_info)(struct _cpl_db_backend_t* backend,
							  const cpl_id_t id,
							  cpl_bundle_info_t** out_info);

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
	(*cpl_db_get_bundle_objects)(struct _cpl_db_backend_t* backend,
								 const cpl_id_t id,
                                 cpl_object_info_iterator_t callback,
                                 void* context);

	/**
	 * Returns all relations contained in a bundle.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the bundle ID
	 * @param callback the iterator to be called for each matching relation
	 * @param context the caller-provided iterator context
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_bundle_relations)(struct _cpl_db_backend_t* backend,
								 const cpl_id_t id,
								 cpl_relation_iterator_t callback,
								 void* context);

	/**
	 * Get the properties associated with the given provenance bundle.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the the bundle ID
	 * @param prefix the property prefix to fetch - or NULL 
	 *               (along with key)for all properties
	 * @param key the property key to fetch - or NULL 
	 *            (along with prefix) for all keys
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_S_NO_DATA, or an error code
	 */
	cpl_return_t
	(*cpl_db_get_bundle_properties)(struct _cpl_db_backend_t* backend,
								 const cpl_id_t id,
								 const char* prefix,
								 const char* key,
								 cpl_property_iterator_t callback,
								 void* context);

	/**
	 * Get the prefixes associated with the given provenance bundle.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the the bundle ID
	 * @param prefix the property prefix to fetch - or NULL for all prefixes
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_S_NO_DATA, or an error code
	 */
	cpl_return_t
	(*cpl_db_get_prefixes)(struct _cpl_db_backend_t* backend,
							const cpl_id_t id, const char* prefix,
				            cpl_prefix_iterator_t iterator,
				            void* context);

} cpl_db_backend_t;



#ifdef __cplusplus
}
#endif

#endif

