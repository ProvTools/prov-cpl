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
 * Flag for whether the library can use the object cache
 */
static bool cpl_cache = false;

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
/** Advanced Private API - Function Prototypes                            **/
/***************************************************************************/


/**
 * Add a dependency
 *
 * @param from_id the "from" end of the dependency edge
 * @param to_id the "to" end of the dependency edge
 * @param type the data dependency edge type
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_add_dependency(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const int type);



/***************************************************************************/
/** Basic Private API                                                     **/
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

	if (cpl_cache) {
		cpl_lock(&cpl_open_objects_lock);
		
		i = cpl_open_objects.find(id);
		if (i != cpl_open_objects.end()) {
			*out = i->second;
			cpl_lock(&(*out)->locked);
			cpl_unlock(&cpl_open_objects_lock);
			return CPL_OK;
		}
		cpl_unlock(&cpl_open_objects_lock);
	}


	// If not, look up the object in a database

	cpl_version_t v;
	cpl_return_t ret;
	ret = cpl_db_backend->cpl_db_get_version(cpl_db_backend, id, &v);
	CPL_RUNTIME_VERIFY(ret);


	// Create the corresponding in-memory state, but do another check just
	// in case

	if (cpl_cache) {
		cpl_lock(&cpl_open_objects_lock);
		
		i = cpl_open_objects.find(id);
		if (i != cpl_open_objects.end()) {
			*out = i->second;
			cpl_lock(&(*out)->locked);
			cpl_unlock(&cpl_open_objects_lock);
			return CPL_OK;
		}
	}

	cpl_open_object_t* obj = cpl_new_open_object(v);
	if (obj == NULL) return CPL_E_INSUFFICIENT_RESOURCES;

	if (cpl_cache) {
		cpl_open_objects[id] = obj;

		cpl_unlock(&cpl_open_objects_lock);
	}


	// Return

	*out = obj;
	return CPL_OK;
}


/**
 * Get a version of an object
 *
 * @param id the object ID
 * @param out_version the pointer to store the version of the object
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_get_version(cpl_id_t id, cpl_version_t* out_version)
{
	assert(out_version != NULL);

	if (cpl_cache) {
		cpl_open_object_t* obj = NULL;
		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(id, &obj));
		*out_version = obj->version;
		cpl_unlock(&obj->locked);
	}
	else {
		cpl_return_t ret;
		ret = cpl_db_backend->cpl_db_get_version(cpl_db_backend,
												 id, out_version);
		CPL_RUNTIME_VERIFY(ret);
	}

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
extern "C" EXPORT cpl_return_t
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
extern "C" EXPORT cpl_return_t
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
 * @param originator the application responsible for creating the object
 *                   and generating unique names within its namespace
 * @param name the object name
 * @param type the object type
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_object(const char* originator,
				  const char* name,
				  const char* type,
				  const cpl_id_t container,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);
	CPL_ENSURE_NOT_NEGATIVE(container);


	// Get the container version

	cpl_version_t container_version = 0;
	if (container != CPL_NONE) {
		CPL_RUNTIME_VERIFY(cpl_get_version(container, &container_version));
	}


	// Call the backend

	cpl_id_t id;
	cpl_return_t ret;
	
	ret = cpl_db_backend->cpl_db_create_object(cpl_db_backend,
											   originator,
											   name,
											   type,
											   container,
											   container_version,
											   &id);
	CPL_RUNTIME_VERIFY(ret);


	// Create an in-memory state

	if (cpl_cache) {
		cpl_open_object_t* obj = cpl_new_open_object(0);
		if (obj == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
		cpl_unlock(&obj->locked);

		cpl_lock(&cpl_open_objects_lock);
		assert(cpl_open_objects.find(id) == cpl_open_objects.end());
		cpl_open_objects[id] = obj;
		cpl_unlock(&cpl_open_objects_lock);
	}


	// Finish

	if (out_id != NULL) *out_id = id;
	return CPL_OK;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param originator the object originator
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object(const char* originator,
				  const char* name,
				  const char* type,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);


	// Call the backend

	cpl_return_t ret;
	cpl_id_t id;
	
	ret = cpl_db_backend->cpl_db_lookup_object(cpl_db_backend,
											   originator,
											   name,
											   type,
											   &id);
	CPL_RUNTIME_VERIFY(ret);


	// Do not get the version number yet

	if (out_id != NULL) *out_id = id;
	return CPL_OK;
}


/**
 * Disclose a data flow.
 *
 * @param data_dest the destination object
 * @param data_source the source object
 * @param type the data dependency edge type
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_data_flow(const cpl_id_t data_dest,
			  const cpl_id_t data_source,
			  const int type)
{
	CPL_ENSURE_INITALIZED;


	// Check the arguments

	CPL_ENSURE_NOT_NEGATIVE(data_dest);
	CPL_ENSURE_NOT_NEGATIVE(data_source);
	
	if (CPL_GET_DEPENDENCY_CATEGORY(type) != CPL_DEPENDENCY_CATEGORY_DATA)
		return CPL_E_INVALID_ARGUMENT;


	// Add the dependency

	return cpl_add_dependency(data_dest, data_source, type);
}


/**
 * Disclose a control flow operation.
 *
 * @param object_id the ID of the controlled object
 * @param controller the object ID of the controller
 * @param type the control dependency edge type
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_control(const cpl_id_t object_id,
			const cpl_id_t controller,
			const int type)
{
	CPL_ENSURE_INITALIZED;


	// Check the arguments

	CPL_ENSURE_NOT_NEGATIVE(object_id);
	CPL_ENSURE_NOT_NEGATIVE(controller);
	
	if (CPL_GET_DEPENDENCY_CATEGORY(type) != CPL_DEPENDENCY_CATEGORY_CONTROL)
		return CPL_E_INVALID_ARGUMENT;


	// Add the dependency

	return cpl_add_dependency(object_id, controller, type);
}



/***************************************************************************/
/** Advanced Private API                                                  **/
/***************************************************************************/


/**
 * Add a dependency
 *
 * @param from_id the "from" end of the dependency edge
 * @param to_id the "to" end of the dependency edge
 * @param type the data dependency edge type
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_add_dependency(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const int type)
{
	CPL_ENSURE_INITALIZED;

	// TODO This should also account for different types of dependency edges,
	// so that edges of different types would not necessarily get eliminated


	// Check the arguments

	CPL_ENSURE_NOT_NEGATIVE(from_id);
	CPL_ENSURE_NOT_NEGATIVE(to_id);


	// Get the version of the "to" object

	cpl_version_t to_version;
	CPL_RUNTIME_VERIFY(cpl_get_version(to_id, &to_version));


	// Cycle-Avoidance Algorithm

	// Determine whether the dependency, or a dependency that subsumes it,
	// already exists. Also, determine the version of the "from" object,
	// but only if the dependency does not already exist, or if this
	// information is already cached

	bool dependency_exists = false;
	cpl_version_t from_version = CPL_VERSION_NONE;
	cpl_open_object_t* obj_from = NULL;

	if (cpl_cache) {

		// Get the handle of the fromination

		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(from_id, &obj_from));


		// Get the version of the fromination object

		from_version = obj_from->version;


		// Check the ancestor list

		cpl_hash_map_id_to_version_t::iterator i;
		i = obj_from->ancestors.find(to_id);

		if (i == obj_from->ancestors.end()) {
			dependency_exists = false;
		}
		else {
			dependency_exists = i->second <= to_version;
		}
	}
	else {

		// Call the backend to determine the dependency

		int b;
		cpl_return_t r = cpl_db_backend->cpl_db_has_immediate_ancestor(
				cpl_db_backend, from_id, CPL_VERSION_NONE,
				to_id, to_version, &b);
		CPL_RUNTIME_VERIFY(r);

		dependency_exists = b > 0;


		// If the dependency exists, get the object version

		if (!dependency_exists) {
			CPL_RUNTIME_VERIFY(cpl_get_version(from_id, &from_version));
		}
	}


	// Automatically unlock the object if it is still locked by the time
	// we hit end this block

	CPL_AutoUnlock __au_from(obj_from != NULL ? &obj_from->locked : NULL);
	(void) __au_from;


	// Return if the dependency already exists - there is nothing to do

	if (dependency_exists) return CPL_OK;


	// Freeze and create a new version

	cpl_return_t r = CPL_E_ALREADY_EXISTS;
	from_version++;

	do {
		r = cpl_db_backend->cpl_db_create_version(cpl_db_backend,
												  from_id,
												  from_version);
		if (r == CPL_E_ALREADY_EXISTS) {
#ifdef _WINDOWS
			Sleep(2 /* ms */);
#else
			usleep(2 * 1000 /* us */);
#endif
			from_version++;
		}
		else {
			CPL_RUNTIME_VERIFY(r);
		}
	}
	while (!CPL_IS_OK(r));

	if (obj_from != NULL) {
		obj_from->frozen = false;
		obj_from->version = from_version;
	}


	// Call the database backend (the provenance "depends on"/"input" edges
	// are oriented opposite to the data flow)

	assert(from_version != CPL_VERSION_NONE);

	CPL_RUNTIME_VERIFY(cpl_db_backend->cpl_db_add_ancestry_edge(cpl_db_backend,
																from_id,
																from_version,
																to_id,
																to_version,
																type));
	

	// Update the ancestor list
	
	if (obj_from != NULL) {

		// Update the hash map

		obj_from->ancestors[to_id] = to_version;


		// Finally, unlock (must be last)
		
		cpl_unlock(&obj_from->locked);
		obj_from = NULL;
	}

	return CPL_OK;
}

