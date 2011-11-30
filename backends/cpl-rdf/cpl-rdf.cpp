/*
 * cpl-rdf.cpp
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
#include "cpl-rdf-private.h"



/***************************************************************************/
/** Private API                                                           **/
/***************************************************************************/




/***************************************************************************/
/** Constructor and Destructor                                            **/
/***************************************************************************/

/**
 * Create an RDF backend
 *
 * @param url_query the SPARQL query endpoint URL
 * @param url_update the SPARQL update endpoint URL
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
extern "C" cpl_return_t
cpl_create_rdf_backend(const char* url_query,
					   const char* url_update,
					   int db_type,
					   cpl_db_backend_t** out)
{
	cpl_return_t r = CPL_OK;

	assert(out != NULL);
	if (url_query  == NULL) return CPL_E_INVALID_ARGUMENT;
	if (url_update == NULL) return CPL_E_INVALID_ARGUMENT;


	// Allocate the backend struct

	cpl_rdf_t* rdf = new cpl_rdf_t;
	if (rdf == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memcpy(&rdf->backend, &CPL_RDF_BACKEND, sizeof(rdf->backend));

	rdf->db_type = db_type;
	rdf->url_query = url_query;
	rdf->url_update = url_update;


	// Initialize the connection handles

	rdf->connection_query = cpl_rdf_connection_init(rdf->url_query.c_str());
	if (rdf->connection_query == NULL) {
		r = CPL_E_DB_CONNECTION_ERROR;
		goto err_free;
	}

	rdf->connection_update = cpl_rdf_connection_init(rdf->url_update.c_str());
	if (rdf->connection_update == NULL) {
		r = CPL_E_DB_CONNECTION_ERROR;
		goto err_close_query;
	}


	// Return

	*out = (cpl_db_backend_t*) rdf;
	return CPL_OK;


err_close_query:
	cpl_rdf_connection_close(rdf->connection_query);

err_free:
	delete rdf;

	assert(!CPL_IS_OK(r));
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
cpl_rdf_destroy(struct _cpl_db_backend_t* backend)
{
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	cpl_rdf_connection_close(rdf->connection_query);
	cpl_rdf_connection_close(rdf->connection_update);

	delete rdf;
	return CPL_OK;
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
cpl_rdf_create_session(struct _cpl_db_backend_t* backend,
					   const cpl_session_t session,
					   const char* mac_address,
					   const char* user,
					   const int pid,
					   const char* program)
{
	assert(backend != NULL && user != NULL && program != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;


	// 
	
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_create_object(struct _cpl_db_backend_t* backend,
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
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;


	// 
	
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_lookup_object(struct _cpl_db_backend_t* backend,
					  const char* originator,
					  const char* name,
					  const char* type,
					  cpl_id_t* out_id)
{
	return CPL_E_NOT_IMPLEMENTED;
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
extern "C" cpl_return_t
cpl_rdf_create_version(struct _cpl_db_backend_t* backend,
					   const cpl_id_t object_id,
					   const cpl_version_t version,
					   const cpl_session_t session)
{
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_get_version(struct _cpl_db_backend_t* backend,
					const cpl_id_t id,
					cpl_version_t* out_version)
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
 * @param type the data or control dependency type
 * @return the object version or an error code
 */
extern "C" cpl_return_t
cpl_rdf_add_ancestry_edge(struct _cpl_db_backend_t* backend,
						  const cpl_id_t from_id,
						  const cpl_version_t from_ver,
						  const cpl_id_t to_id,
						  const cpl_version_t to_ver,
						  const int type)
{
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_has_immediate_ancestor(struct _cpl_db_backend_t* backend,
							   const cpl_id_t object_id,
							   const cpl_version_t version_hint,
							   const cpl_id_t query_object_id,
							   const cpl_version_t query_object_max_version,
							   int* out)
{
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_get_object_info(struct _cpl_db_backend_t* backend,
						const cpl_id_t id,
						const cpl_version_t version_hint,
						cpl_object_info_t** out_info)
{
	return CPL_E_NOT_IMPLEMENTED;
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
cpl_rdf_get_version_info(struct _cpl_db_backend_t* backend,
						 const cpl_id_t id,
						 const cpl_version_t version,
						 cpl_version_info_t** out_info)
{
	return CPL_E_NOT_IMPLEMENTED;
}



/***************************************************************************/
/** The export / interface struct                                         **/
/***************************************************************************/

/**
 * The rdf interface
 */
const cpl_db_backend_t CPL_RDF_BACKEND = {
	cpl_rdf_destroy,
	cpl_rdf_create_session,
	cpl_rdf_create_object,
	cpl_rdf_lookup_object,
	cpl_rdf_create_version,
	cpl_rdf_get_version,
	cpl_rdf_add_ancestry_edge,
	cpl_rdf_has_immediate_ancestor,
	cpl_rdf_get_object_info,
	cpl_rdf_get_version_info,
};

