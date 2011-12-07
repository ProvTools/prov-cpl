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

#include <sstream>
#include <string>


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

	char session_str[64];
	sprintf(session_str, "session:%llx-%llx", session.hi, session.lo);

	std::ostringstream ss;

	// XXX Only for debugging
	//cpl_rdf_connection_execute_update(rdf->connection_update,
	//		"DELETE { ?s ?p ?o } WHERE { ?s ?p ?o }");

	ss << "PREFIX session: <session:>\n";
	ss << "PREFIX prop: <prop:>\n";
	ss << "INSERT DATA { " << session_str;
	ss << " prop:mac_address \"" << cpl_rdf_escape_string(mac_address) << "\";";
	ss << " prop:username \"" << cpl_rdf_escape_string(user) << "\";";
	ss << " prop:pid " << pid << ";";
	ss << " prop:program \"" << cpl_rdf_escape_string(program) << "\";";
	ss << " prop:initialization_time " << time(NULL) << " }\n";

	cpl_return_t ret = cpl_rdf_connection_execute_update(rdf->connection_update,
			ss.str().c_str());
	
	return ret;
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

	char session_str[64];
	sprintf(session_str, "session:%llx-%llx", session.hi, session.lo);

	char id_str[64];
	sprintf(id_str, "object:%llx-%llx", id.hi, id.lo);

	char node_str[64];
	sprintf(node_str, "node:%llx-%llx-0", id.hi, id.lo);

	time_t creation_time = time(NULL);

	std::ostringstream ss;

	ss << "PREFIX session: <session:>\n";
	ss << "PREFIX object: <object:>\n";
	ss << "PREFIX node: <node:>\n";
	ss << "PREFIX prop: <prop:>\n";
	ss << "PREFIX rel: <rel:>\n";
	ss << "INSERT DATA { " << id_str;
	ss << " prop:originator \"" << cpl_rdf_escape_string(originator) << "\";";
	ss << " prop:name \"" << cpl_rdf_escape_string(name) << "\";";
	ss << " prop:type \"" << cpl_rdf_escape_string(type) << "\";";
	if (container != CPL_NONE) {
		char container_str[64];
		sprintf(container_str, "node:%llx-%llx-%x", container.hi,
				container.lo, container_version);
		ss << " rel:container " << container_str << ";";
	}
	ss << " prop:creation_time " << creation_time << ";";
	ss << " rel:version " << node_str << " .";
	ss << " " << node_str << " prop:session " << session_str << ";";
	ss << " prop:version 0;";
	ss << " prop:creation_time " << creation_time << ";";
	ss << "}\n";

	cpl_return_t ret = cpl_rdf_connection_execute_update(rdf->connection_update,
			ss.str().c_str());
	
	return ret;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	char id_str[64];
	sprintf(id_str, "object:%llx-%llx", id.hi, id.lo);

	std::ostringstream ss;

	ss << "PREFIX object: <object:>\n";
	ss << "PREFIX prop: <prop:>\n";
	ss << "PREFIX rel: <rel:>\n";
	ss << "SELECT ?v WHERE { ";
	ss << " " << id_str << " rel:version ?node . ?node prop:version ?v .";
	ss << " OPTIONAL {";
	ss << "  " << id_str <<" rel:version ?o_node . ?o_node prop:version ?o_v .";
	ss << "  FILTER ( ?o_v > ?v ) . } . FILTER ( !bound(?o_v) ) . }";

	RDFResultSet rs;
	cpl_return_t ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) return ret;

	if (out_version != NULL) {
		if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
		if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;
		//*out_version = rs[0]["v"]->v_int;
	}
	
	return CPL_OK;
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
cpl_rdf_get_object_ancestry(struct _cpl_db_backend_t* backend,
							const cpl_id_t id,
							const cpl_version_t version,
							const int direction,
							const int flags,
							cpl_ancestry_iterator_t iterator,
							void* context)
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
	cpl_rdf_get_object_ancestry,
};

