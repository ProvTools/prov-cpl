/*
 * cpl-standalone.cpp
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
#include "cpl-private.h"


/***************************************************************************/
/** Private state                                                         **/
/***************************************************************************/

/**
 * Flag marking whether the library has been initialized
 */
static bool cpl_initialized = false;

/**
 * The cache of open objects
 */
static cpl_hash_map_id_to_open_object_t cpl_open_objects;

/**
 * The lock for the cache of open objects
 */
static cpl_lock_t cpl_open_objects_lock;

/**
 * The database backend
 */
static cpl_db_backend_t* cpl_db_backend = NULL;



/***************************************************************************/
/** Private API                                                           **/
/***************************************************************************/


/**
 * Make sure that the library has been initialized
 */
#define CPL_ENSURE_INITALIZED { \
	if (!cpl_initialized) return CPL_E_NOT_INITIALIZED; }


/**
 * Create an instance of an in-memory state. The object is returned locked.
 *
 * @param version the object version
 * @return the instantiated object, or NULL if out of resources
 */
cpl_open_object_t*
cpl_new_open_object(const cpl_version_t version)
{
	cpl_open_object_t* obj = new cpl_open_object_t;
	if (obj == NULL) return NULL;

	obj->locked = 1;
	obj->version = version;
	obj->frozen = true;
	obj->last_originator = CPL_NONE;

	return obj;
}


/**
 * Get the open object handle, opening the object if necessary,
 * and return the object locked
 *
 * @param id the object ID
 * @param out the output
 * @return the error code
 */
cpl_return_t
cpl_get_open_object_handle(const cpl_id_t id, cpl_open_object_t** out)
{
	assert(out != NULL);
	cpl_hash_map_id_to_open_object_t::iterator i; 


	// Check to see if the object is already in the open objects cache

	cpl_lock(&cpl_open_objects_lock);
	
	i = cpl_open_objects.find(id);
	if (i != cpl_open_objects.end()) {
		*out = i->second;
		cpl_lock(&(*out)->locked);
		cpl_unlock(&cpl_open_objects_lock);
		return CPL_OK;
	}
	cpl_unlock(&cpl_open_objects_lock);


	// If not, look up the object in a database

	cpl_version_t v = cpl_db_backend->cpl_db_get_version(cpl_db_backend, id);
	CPL_RUNTIME_VERIFY(v);


	// Create the corresponding in-memory state, but do another check just
	// in case

	cpl_lock(&cpl_open_objects_lock);
	
	i = cpl_open_objects.find(id);
	if (i != cpl_open_objects.end()) {
		*out = i->second;
		cpl_lock(&(*out)->locked);
		cpl_unlock(&cpl_open_objects_lock);
		return CPL_OK;
	}

	cpl_open_object_t* obj = cpl_new_open_object(v);
	if (obj == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	cpl_open_objects[id] = obj;

	cpl_unlock(&cpl_open_objects_lock);


	// Return

	*out = obj;
	return CPL_OK;
}



/***************************************************************************/
/** Initialization and Cleanup                                            **/
/***************************************************************************/


/**
 * Initialize the library. Please note that this function is not thread-safe.
 *
 * @param backend the database backend
 * @return the error code
 */
extern "C" cpl_return_t
cpl_initialize(struct _cpl_db_backend_t* backend)
{
	CPL_ENSURE_NOT_NULL(backend);
	if (cpl_initialized) return CPL_E_ALREADY_INITIALIZED;
	cpl_initialized = true;

	cpl_db_backend = backend;

	return CPL_OK;
}


/**
 * Perform the cleanup. Please note that this function is not thread-safe.
 *
 * @return the error code
 */
extern "C" cpl_return_t
cpl_cleanup(void)
{
	CPL_ENSURE_INITALIZED;
	cpl_initialized = false;

	cpl_db_backend->cpl_db_destroy(cpl_db_backend);
	cpl_db_backend = NULL;

	return CPL_OK;
}



/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


/**
 * Create an object.
 *
 * @param originator the originator ID
 * @param name the object name
 * @param type the object type
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_create_object(const cpl_id_t originator,
				  const char* name,
				  const char* type,
				  const cpl_id_t container)
{
	CPL_ENSURE_INITALIZED;


	// Argument check

	CPL_ENSURE_ORIGINATOR(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);


	// Get the container version

	cpl_version_t container_version = 0;
	if (container != CPL_NONE) {
		cpl_open_object_t* obj_container = NULL;
		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(container,
													  &obj_container));
		container_version = obj_container->version;
		cpl_unlock(&obj_container->locked);
	}


	// Call the backend

	cpl_id_t id = cpl_db_backend->cpl_db_create_object(cpl_db_backend,
													   originator,
													   name,
													   type,
													   container,
													   container_version);
	CPL_RUNTIME_VERIFY(id);


	// Create an in-memory state

	cpl_open_object_t* obj = cpl_new_open_object(0);
	if (obj == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
	cpl_unlock(&obj->locked);

	cpl_lock(&cpl_open_objects_lock);
	assert(cpl_open_objects.find(id) == cpl_open_objects.end());
	cpl_open_objects[id] = obj;
	cpl_unlock(&cpl_open_objects_lock);

	return id;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param name the object name
 * @param type the object type
 * @return the object ID, or a negative value on error
 */
extern "C" cpl_id_t
cpl_lookup_by_name(const char* name,
				   const char* type)
{
	CPL_ENSURE_INITALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);


	// Call the backend

	cpl_id_t id = cpl_db_backend->cpl_db_lookup_by_name(cpl_db_backend,
														name,
														type);
	CPL_RUNTIME_VERIFY(id);


	// Do not get the version number yet

	return id;
}


/**
 * Disclose a data transfer.
 *
 * @param originator the provenance originator ID
 * @param source the source object
 * @param dest the destination object
 * @return the operation return value
 */
extern "C" cpl_return_t
cpl_disclose_data_transfer(const cpl_id_t originator,
						   const cpl_id_t source,
						   const cpl_id_t dest)
{
	CPL_ENSURE_INITALIZED;


	// Check arguments

	CPL_ENSURE_ORIGINATOR(originator);


	// Get the data source version

	cpl_open_object_t* obj_source = NULL;
	CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(source, &obj_source));
	cpl_version_t source_version = obj_source->version;
	cpl_unlock(&obj_source->locked);
	obj_source = NULL;


	// Get the handle of the destination

	cpl_open_object_t* obj_dest = NULL;
	CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(dest, &obj_dest));


	// Automatically unlock the object if it is still locked by the time
	// we hit return

	CPL_AutoUnlock __au_dest(&obj_dest->locked); (void) __au_dest;


	// Call the analyzer

	// TODO


	// Get the version of the destination object and unlock

	cpl_version_t dest_version = obj_dest->version;
	cpl_unlock(&obj_dest->locked);


	// Call the database backend (the provenance "depends on"/"input" edges
	// are oriented opposite to the data flow)

	CPL_RUNTIME_VERIFY(cpl_db_backend->cpl_db_add_ancestry_edge(cpl_db_backend,
																dest,
																dest_version,
																source,
																source_version));

	return CPL_OK;
}

