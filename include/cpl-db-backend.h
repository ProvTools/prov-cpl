/*
 * cpl-db-backend.h
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

#ifndef __CPL_DB_BACKEND_H__
#define __CPL_DB_BACKEND_H__

#include <cpl.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

#if defined _WIN64 || defined _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif


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
							 const cpl_session_t session,
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
	 * @param originator the object originator
	 * @param name the object name
	 * @param type the object type
	 * @param container the ID of the object that should contain this object
	 *                  (use CPL_NONE for no container)
	 * @param container_version the version of the container (if not CPL_NONE)
	 * @param session the session ID responsible for this provenance record
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_create_object)(struct _cpl_db_backend_t* backend,
							const cpl_id_t id,
							const char* originator,
							const char* name,
							const char* type,
							const cpl_id_t container,
							const cpl_version_t container_version,
							const cpl_session_t session);

	/**
	 * Look up an object by name. If multiple objects share the same name,
	 * get the latest one.
	 *
	 * @param backend the pointer to the backend structure
	 * @param originator the object originator
	 * @param name the object name
	 * @param type the object type
	 * @param out_id the pointer to store the object ID
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_object)(struct _cpl_db_backend_t* backend,
							const char* originator,
							const char* name,
					   		const char* type,
							cpl_id_t* out_id);

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
	(*cpl_db_create_version)(struct _cpl_db_backend_t* backend,
							 const cpl_id_t object_id,
							 const cpl_version_t version,
							 const cpl_session_t session);

	/**
	 * Determine the version of the object
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the object ID
	 * @param out_version the pointer to store the version of the object
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_version)(struct _cpl_db_backend_t* backend,
						  const cpl_id_t id,
						  cpl_version_t* out_version);

	/**
	 * Add an ancestry edge
	 *
	 * @param backend the pointer to the backend structure
	 * @param from_id the edge source ID
	 * @param from_ver the edge source version
	 * @param to_id the edge destination ID
	 * @param to_ver the edge destination version
	 * @param type the data or the control dependency type
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_add_ancestry_edge)(struct _cpl_db_backend_t* backend,
								const cpl_id_t from_id,
								const cpl_version_t from_ver,
								const cpl_id_t to_id,
								const cpl_version_t to_ver,
								const int type);

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
									 const cpl_version_t version_hint,
									 const cpl_id_t query_object_id,
									 const cpl_version_t query_object_max_ver,
									 int* out);

    /**
     * Add a property to the given object
     *
	 * @param backend the pointer to the backend structure
     * @param id the object ID
     * @param version the version number
     * @param key the key
     * @param value the value
     * @return CPL_OK or an error code
     */
    cpl_return_t
    (*cpl_db_add_property)(struct _cpl_db_backend_t* backend,
                           const cpl_id_t id,
                           const cpl_version_t version,
                           const char* key,
                           const char* value);

	/**
	 * Get information about the given provenance session.
	 *
	 * @param id the session ID
	 * @param out_info the pointer to store the session info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_session_info)(struct _cpl_db_backend_t* backend,
							   const cpl_session_t id,
							   cpl_session_info_t** out_info);

	/**
	 * Get information about the given provenance object
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the object ID
	 * @param version_hint the version of the given provenance object if known,
	 *                     or CPL_VERSION_NONE if not
	 * @param out_info the pointer to store the object info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_object_info)(struct _cpl_db_backend_t* backend,
							  const cpl_id_t id,
							  const cpl_version_t version_hint,
							  cpl_object_info_t** out_info);

	/**
	 * Get information about the specific version of a provenance object
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the object ID
	 * @param version the version of the given provenance object
	 * @param out_info the pointer to store the version info structure
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	(*cpl_db_get_version_info)(struct _cpl_db_backend_t* backend,
							   const cpl_id_t id,
							   const cpl_version_t version,
							   cpl_version_info_t** out_info);

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
	(*cpl_db_get_object_ancestry)(struct _cpl_db_backend_t* backend,
								  const cpl_id_t id,
								  const cpl_version_t version,
								  const int direction,
								  const int flags,
								  cpl_ancestry_iterator_t iterator,
								  void* context);

	/**
	 * Get the properties associated with the given provenance object.
	 *
	 * @param backend the pointer to the backend structure
	 * @param id the the object ID
	 * @param version the object version, or CPL_VERSION_NONE to access all
	 *                version nodes associated with the given object
	 * @param key the property to fetch - or NULL for all properties
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_S_NO_DATA, or an error code
	 */
	cpl_return_t
	(*cpl_db_get_properties)(struct _cpl_db_backend_t* backend,
							 const cpl_id_t id,
							 const cpl_version_t version,
							 const char* key,
							 cpl_property_iterator_t iterator,
							 void* context);

	/**
	 * Lookup an object based on a property value.
	 *
	 * @param backend the pointer to the backend structure
	 * @param key the property name
	 * @param value the property value
	 * @param iterator the iterator callback function
	 * @param context the user context to be passed to the iterator function
	 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
	 */
	cpl_return_t
	(*cpl_db_lookup_by_property)(struct _cpl_db_backend_t* backend,
								 const char* key,
								 const char* value,
								 cpl_property_iterator_t iterator,
								 void* context);

} cpl_db_backend_t;



#ifdef __cplusplus
}
#endif

#endif

