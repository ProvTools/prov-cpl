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

#include <iostream>
#include <sstream>
#include <string>



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


	// Initialize the shared semaphore for version creation

	rdf->sem_create_version = cpl_shared_semaphore_open(CPL_RDF_SEM_INIT);
	if (rdf->sem_create_version == NULL) {
		r = CPL_E_PLATFORM_ERROR;
		goto err_close_all;
	}


	// Return

	*out = (cpl_db_backend_t*) rdf;
	return CPL_OK;


err_close_all:
	cpl_rdf_connection_close(rdf->connection_update);

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

	cpl_shared_semaphore_close(rdf->sem_create_version);
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
	sprintf(session_str, "s:%llx-%llx", session.hi, session.lo);

	std::ostringstream ss;

	// Only for debugging
	// cpl_rdf_connection_execute_update(rdf->connection_update,
	//		"DELETE { ?s ?p ?o } WHERE { ?s ?p ?o }");

	ss << "PREFIX s: <session:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "INSERT DATA { " << session_str;
	ss << " p:mac_address \"" << cpl_rdf_escape_string(mac_address) << "\";";
	ss << " p:username \"" << cpl_rdf_escape_string(user) << "\";";
	ss << " p:pid " << pid << ";";
	ss << " p:program \"" << cpl_rdf_escape_string(program) << "\";";
	ss << " p:initialization_time " << time(NULL) << " }\n";

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
	sprintf(session_str, "s:%llx-%llx", session.hi, session.lo);

	char id_str[64];
	sprintf(id_str, "o:%llx-%llx", id.hi, id.lo);

	char node_str[64];
	sprintf(node_str, "n:%llx-%llx-0", id.hi, id.lo);

	time_t creation_time = time(NULL);

	std::ostringstream ss;

	ss << "PREFIX s: <session:>\n";
	ss << "PREFIX o: <object:>\n";
	ss << "PREFIX n: <node:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	ss << "INSERT DATA { " << id_str;
	ss << " p:originator \"" << cpl_rdf_escape_string(originator) << "\";";
	ss << " p:name \"" << cpl_rdf_escape_string(name) << "\";";
	ss << " p:type \"" << cpl_rdf_escape_string(type) << "\";";
	if (container != CPL_NONE) {
		char container_str[64];
		sprintf(container_str, "n:%llx-%llx-%x", container.hi,
				container.lo, container_version);
		ss << " r:container " << container_str << ";";
	}
	ss << " p:creation_time " << creation_time << ";";
	ss << " r:version " << node_str << " .";
	ss << " " << node_str << " r:session " << session_str << ";";
	ss << " p:version 0;";
	ss << " p:creation_time " << creation_time << ";";
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	std::ostringstream ss;

	ss << "PREFIX o: <object:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "SELECT ?obj WHERE { ?obj";
	ss << " p:originator \"" << cpl_rdf_escape_string(originator) << "\";";
	ss << " p:name \"" << cpl_rdf_escape_string(name) << "\";";
	ss << " p:type \"" << cpl_rdf_escape_string(type) << "\";";
	ss << " p:creation_time ?t .";
	ss << " OPTIONAL { ?o_obj";
	ss << "  p:originator \"" << cpl_rdf_escape_string(originator) << "\";";
	ss << "  p:name \"" << cpl_rdf_escape_string(name) << "\";";
	ss << "  p:type \"" << cpl_rdf_escape_string(type) << "\";";
	ss << "  p:creation_time ?o_t .";
	ss << "  FILTER ( ?o_t > ?t ) . } . FILTER ( !bound(?o_t) ) . }";

	RDFResultSet rs;
	cpl_return_t ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) return ret;

	if (out_id != NULL) {
		if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
		//if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;

		RDFValue* v;
		ret = rs[0].get_s("obj", RDF_XSD_URI, &v);
		if (!CPL_IS_OK(ret)) return ret;
		int r = sscanf(v->v_uri, "object:%llx-%llx", &out_id->hi, &out_id->lo);
		if (r != 2) return CPL_E_BACKEND_INTERNAL_ERROR;
	}
	
	return CPL_OK;
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
	assert(backend != NULL);
	assert(version > 0);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	char session_str[64];
	sprintf(session_str, "s:%llx-%llx", session.hi, session.lo);

	char id_str[64];
	sprintf(id_str, "o:%llx-%llx", object_id.hi, object_id.lo);

	char node_str[64];
	sprintf(node_str, "n:%llx-%llx-%x", object_id.hi, object_id.lo, version);

	std::ostringstream ss_check;
	std::ostringstream ss_create;

	ss_check << "PREFIX n: <node:>\n";
	ss_check << "PREFIX p: <prop:>\n";
	ss_check << "SELECT ?v WHERE { " << node_str << " p:version ?v }";

	ss_create << "PREFIX s: <session:>\n";
	ss_create << "PREFIX o: <object:>\n";
	ss_create << "PREFIX n: <node:>\n";
	ss_create << "PREFIX p: <prop:>\n";
	ss_create << "PREFIX r: <rel:>\n";
	ss_create << "INSERT DATA { ";
	ss_create << " " << id_str << " r:version " << node_str << " .";
	ss_create << " " << node_str << " r:session " << session_str << ";";
	ss_create << " p:version " << version << ";";
	ss_create << " p:creation_time " << time(NULL) << ";";
	ss_create << "}";

	cpl_shared_semaphore_wait(rdf->sem_create_version);

	RDFResultSet rs;
	cpl_return_t ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss_check.str().c_str(), &rs);

	if (!CPL_IS_OK(ret)) {
		cpl_shared_semaphore_post(rdf->sem_create_version);
		return ret;
	}
	if (ret != CPL_S_NO_DATA || rs.size() != 0) {
		cpl_shared_semaphore_post(rdf->sem_create_version);
		return CPL_E_ALREADY_EXISTS;
	}

	ret = cpl_rdf_connection_execute_update(rdf->connection_update,
			ss_create.str().c_str());
	if (!CPL_IS_OK(ret)) {
		cpl_shared_semaphore_post(rdf->sem_create_version);
		return ret;
	}

	cpl_shared_semaphore_post(rdf->sem_create_version);
	return CPL_OK;
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
	sprintf(id_str, "o:%llx-%llx", id.hi, id.lo);

	std::ostringstream ss;

	ss << "PREFIX o: <object:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	ss << "SELECT ?v WHERE { ";
	ss << " " << id_str << " r:version ?node . ?node p:version ?v .";
	ss << " OPTIONAL {";
	ss << "  " << id_str <<" r:version ?o_node . ?o_node p:version ?o_v .";
	ss << "  FILTER ( ?o_v > ?v ) . } . FILTER ( !bound(?o_v) ) . }";

	RDFResultSet rs;
	cpl_return_t ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) return ret;

	if (out_version != NULL) {
		if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
		//if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;

		RDFValue* v;
		ret = rs[0].get_s("v", RDF_XSD_INTEGER, &v);
		if (!CPL_IS_OK(ret)) return ret;
		*out_version = (cpl_version_t) v->v_integer;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	char node_from_str[64];
	sprintf(node_from_str, "n:%llx-%llx-%x", from_id.hi, from_id.lo, from_ver);

	char node_to_str[64];
	sprintf(node_to_str, "n:%llx-%llx-%x", to_id.hi, to_id.lo, to_ver);

	char edge[32];
	sprintf(edge, "i:%x", type);

	std::ostringstream ss;

	ss << "PREFIX n: <node:>\n";
	ss << "PREFIX i: <input:>\n";
	ss << "INSERT DATA {";
	ss << " " << node_from_str << " " << edge << " " << node_to_str << " }";

	cpl_return_t ret = cpl_rdf_connection_execute_update(rdf->connection_update,
			ss.str().c_str());
	
	return ret;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;

	char q_id_str[64];
	sprintf(q_id_str, "o:%llx-%llx", query_object_id.hi, query_object_id.lo);

	std::ostringstream ss;

	char id_str[64];
	sprintf(id_str, "o:%llx-%llx", object_id.hi, object_id.lo);

	char node_str[64];
	if (version_hint != CPL_VERSION_NONE) {
		sprintf(node_str, "n:%llx-%llx-%x",
				object_id.hi, object_id.lo, version_hint);
	}

	ss << "PREFIX o: <object:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	if (version_hint != CPL_VERSION_NONE) {
		ss << "PREFIX n: <node:>\n";
	}
	ss << "SELECT ?v WHERE { ";
	if (version_hint != CPL_VERSION_NONE) {
		ss << " " << node_str << " ?edge ?q_node .";
	}
	else {
		ss << " " << id_str << " r:version ?node .";
		ss << " ?node ?edge ?q_node .";
	}
	ss << " " << q_id_str << " r:version ?q_node . ?q_node p:version ?q_v .";
	ss << " FILTER ( ?q_v <= " << query_object_max_version << " ) }";

	RDFResultSet rs;
	cpl_return_t ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (!CPL_IS_OK(ret)) {
		//rs.print_error_messages(std::cerr);
		return ret;
	}
	if (out != NULL) *out = (ret == CPL_S_NO_DATA) ? 0 : 1;
	return CPL_OK;
}


/**
 * Get information about the given provenance session.
 *
 * @param id the session ID
 * @param out_info the pointer to store the session info structure
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_rdf_get_session_info(struct _cpl_db_backend_t* backend,
						 const cpl_session_t id,
						 cpl_session_info_t** out_info)
{
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;
	cpl_return_t ret;


	// Prepare the query

	char id_str[64];
	sprintf(id_str, "s:%llx-%llx", id.hi, id.lo);

	std::ostringstream ss;

	ss << "PREFIX s: <session:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "SELECT ?mac_address, ?username, ?pid, ?program, ";
	ss << "?initialization_time WHERE { " << id_str;
	ss << " p:mac_address ?mac_address ;";
	ss << " p:username ?username ;";
	ss << " p:pid ?pid ;";
	ss << " p:program ?program ;";
	ss << " p:initialization_time ?initialization_time .";
	ss << "}";


	// Execute query

	RDFResultSet rs;
	ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) {
		//rs.print_error_messages(std::cerr);
		return ret;
	}


	// Process the result

	if (out_info == NULL) return CPL_OK;
	if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
	if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;

	cpl_session_info_t* p = (cpl_session_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;

	RDFValue* v;

	ret = rs[0].get_s("mac_address", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->mac_address = strdup(v->v_string);

	ret = rs[0].get_s("username", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->user = strdup(v->v_string);

	ret = rs[0].get_s("pid", RDF_XSD_INTEGER, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->pid = v->v_integer;

	ret = rs[0].get_s("program", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->program = strdup(v->v_string);

	ret = rs[0].get_s("initialization_time", RDF_XSD_INTEGER, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->start_time = v->v_integer;

	*out_info = p;
	return CPL_OK;

err:
	if (p->mac_address != NULL) free(p->mac_address);
	if (p->user != NULL) free(p->user);
	if (p->program != NULL) free(p->program);
	free(p);

	return ret;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;
	cpl_return_t ret;


	// Get the version

	cpl_version_t version;

	if (version_hint == CPL_VERSION_NONE) {
		ret = cpl_rdf_get_version(backend, id, &version);
		if (!CPL_IS_OK(ret)) return ret;
	}
	else {
		version = version_hint;
	}


	// Prepare the query

	char id_str[64];
	sprintf(id_str, "o:%llx-%llx", id.hi, id.lo);

	char node_str[64];
	sprintf(node_str, "n:%llx-%llx-0", id.hi, id.lo);

	std::ostringstream ss;

	ss << "PREFIX o: <object:>\n";
	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	ss << "PREFIX n: <node:>\n";
	ss << "SELECT ?session, ?time, ?orig, ?name, ?type, ?container WHERE {";
	ss << " " << node_str << " r:session ?session .";
	ss << " " << id_str << " p:creation_time ?time ;";
	ss << " p:originator ?orig ;";
	ss << " p:name ?name ;";
	ss << " p:type ?type .";
	ss << " OPTIONAL {";
	ss << "  " << id_str << " r:container ?container .";
	ss << " } ";
	ss << "}";


	// Execute query

	RDFResultSet rs;
	ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) {
		//rs.print_error_messages(std::cerr);
		return ret;
	}


	// Process the result

	if (out_info == NULL) return CPL_OK;
	if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
	//if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;

	cpl_object_info_t* p = (cpl_object_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;
	p->version = version;

	RDFValue* v;
	int r;

	ret = rs[0].get_s("session", RDF_XSD_URI, &v);
	if (!CPL_IS_OK(ret)) goto err;
	r = sscanf(v->v_uri, "session:%llx-%llx",
			&p->creation_session.hi, &p->creation_session.lo);
	if (r != 2) { ret = CPL_E_BACKEND_INTERNAL_ERROR; goto err; }

	ret = rs[0].get_s("time", RDF_XSD_INTEGER, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->creation_time = v->v_integer;

	ret = rs[0].get_s("orig", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->originator = strdup(v->v_string);

	ret = rs[0].get_s("name", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->name = strdup(v->v_string);

	ret = rs[0].get_s("type", RDF_XSD_STRING, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->type = strdup(v->v_string);

	ret = rs[0].get_s("container", RDF_XSD_URI, &v);
	if (ret == CPL_E_DB_KEY_NOT_FOUND) {
		p->container_id = CPL_NONE;
		p->container_version = CPL_VERSION_NONE;
	}
	else if (CPL_IS_OK(ret)) {
		r = sscanf(v->v_uri, "node:%llx-%llx-%x",
				&p->container_id.hi,&p->container_id.lo,&p->container_version);
		if (r != 3) { ret = CPL_E_BACKEND_INTERNAL_ERROR; goto err; }
	}
	else goto err;

	*out_info = p;
	return CPL_OK;

err:
	if (p->originator != NULL) free(p->originator);
	if (p->name != NULL) free(p->name);
	if (p->type != NULL) free(p->type);
	free(p);

	return ret;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;
	cpl_return_t ret;


	// Prepare the query

	char node_str[64];
	sprintf(node_str, "n:%llx-%llx-%x", id.hi, id.lo, version);

	std::ostringstream ss;

	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	ss << "PREFIX n: <node:>\n";
	ss << "SELECT ?session, ?time WHERE {";
	ss << " " << node_str << " r:session ?session ;";
	ss << " p:creation_time ?time }";


	// Execute query

	RDFResultSet rs;
	ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) return CPL_E_NOT_FOUND;
	if (!CPL_IS_OK(ret)) {
		//rs.print_error_messages(std::cerr);
		return ret;
	}


	// Process the result

	if (out_info == NULL) return CPL_OK;
	if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;
	//if (rs.size() >  1) return CPL_E_BACKEND_INTERNAL_ERROR;

	cpl_version_info_t* p = (cpl_version_info_t*) malloc(sizeof(*p));
	if (p == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	memset(p, 0, sizeof(*p));
	p->id = id;
	p->version = version;

	RDFValue* v;
	int r;

	ret = rs[0].get_s("session", RDF_XSD_URI, &v);
	if (!CPL_IS_OK(ret)) goto err;
	r = sscanf(v->v_uri, "session:%llx-%llx", &p->session.hi, &p->session.lo);
	if (r != 2) { ret = CPL_E_BACKEND_INTERNAL_ERROR; goto err; }

	ret = rs[0].get_s("time", RDF_XSD_INTEGER, &v);
	if (!CPL_IS_OK(ret)) goto err;
	p->creation_time = v->v_integer;

	*out_info = p;
	return CPL_OK;

err:
	free(p);
	return ret;
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
	assert(backend != NULL);
	cpl_rdf_t* rdf = (cpl_rdf_t*) backend;
	cpl_return_t ret;


	// Prepare the query

	char str[64];
	if (version == CPL_VERSION_NONE) {
		sprintf(str, "o:%llx-%llx", id.hi, id.lo);
	}
	else {
		sprintf(str, "n:%llx-%llx-%x", id.hi, id.lo, version);
	}

	std::ostringstream ss;

	ss << "PREFIX p: <prop:>\n";
	ss << "PREFIX r: <rel:>\n";
	if (version == CPL_VERSION_NONE) {
		ss << "PREFIX o: <object:>\n";
	}
	else {
		ss << "PREFIX n: <node:>\n";
	}
	ss << "SELECT ?edge, ?other";
	if (version == CPL_VERSION_NONE) {
		ss << ", ?node";
	}
	ss << " WHERE {";
	if (direction == CPL_D_ANCESTORS) {
		if (version == CPL_VERSION_NONE) {
			ss << " " << str << " r:version ?node . ?node";
		}
		else {
			ss << " " << str;
		}
		ss << " ?edge ?other .";
	}
	else {
		if (version == CPL_VERSION_NONE) {
			ss << " " << str << " r:version ?node .";
			ss << " ?other ?edge ?node .";
		}
		else {
			ss << " ?other ?edge " << str << " .";
		}
	}
	ss << " FILTER( isURI(?other) ) }";


	// Execute query

	RDFResultSet rs;
	ret = cpl_rdf_connection_execute_query(rdf->connection_query,
			ss.str().c_str(), &rs);

	if (ret == CPL_S_NO_DATA) {
		// We do not need to check here whether the object exists, because
		// the query would at least return the session object in one case
		// or the actual object in the other case if the given object
		// exits (we do not fully filter the results in the query itself).
		return CPL_E_NOT_FOUND;
	}
	if (!CPL_IS_OK(ret)) {
		//rs.print_error_messages(std::cerr);
		return ret;
	}


	// Process the result

	if (rs.size() == 0) return CPL_E_BACKEND_INTERNAL_ERROR;

	RDFValue* v;
	int r;
	bool found_matching_edges = false;

	for (unsigned u = 0; u < rs.size(); u++) {

		// Parse the edge and filter out non-ancestry edges

		ret = rs[u].get_s("edge", RDF_XSD_URI, &v);
		if (!CPL_IS_OK(ret)) return ret;
		if (strncmp(v->v_uri, "input:", 6) != 0) continue;

		int type;
		r = sscanf(v->v_uri, "input:%x", &type);
		if (r != 1) return CPL_E_BACKEND_INTERNAL_ERROR;


		// Filter by type

		int type_category = CPL_GET_DEPENDENCY_CATEGORY((int) type);
		if (type_category == CPL_DEPENDENCY_CATEGORY_DATA
				&& (flags & CPL_A_NO_DATA_DEPENDENCIES) != 0) continue;
		if (type_category == CPL_DEPENDENCY_CATEGORY_CONTROL
				&& (flags & CPL_A_NO_CONTROL_DEPENDENCIES) != 0) continue;


		// Get the query node

		cpl_version_t query_version;

		if (version == CPL_VERSION_NONE) {
			ret = rs[u].get_s("node", RDF_XSD_URI, &v);
			if (!CPL_IS_OK(ret)) return ret;

			cpl_id_t query_id;
			r = sscanf(v->v_uri, "node:%llx-%llx-%x",
					&query_id.hi, &query_id.lo, &query_version);
			if (r != 3) return CPL_E_BACKEND_INTERNAL_ERROR;
		}
		else {
			query_version = version;
		}


		// Get the other node

		ret = rs[u].get_s("other", RDF_XSD_URI, &v);
		if (!CPL_IS_OK(ret)) return ret;

		cpl_id_t other_id;
		cpl_version_t other_version;
		r = sscanf(v->v_uri, "node:%llx-%llx-%x",
				&other_id.hi, &other_id.lo, &other_version);
		if (r != 3) return CPL_E_BACKEND_INTERNAL_ERROR;


		// Call the callback function (if available)

		found_matching_edges = true;
		if (iterator != NULL) {
			ret = iterator(id, query_version, other_id, other_version,
						   type, context);
			if (!CPL_IS_OK(ret)) return ret;
		}
	}

	return found_matching_edges ? CPL_OK : CPL_S_NO_DATA;
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
	cpl_rdf_get_session_info,
	cpl_rdf_get_object_info,
	cpl_rdf_get_version_info,
	cpl_rdf_get_object_ancestry,
};

