/*
 * cpl-standalone.cpp
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

#include "stdafx.h"
#include "cpl-private.h"
#include "cpl-platform.h"
#include <private/cpl-platform.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
EXPORT const cpl_id_t CPL_NONE = 0;

/**
 * The last success code number
 */
#define __CPL_S_LAST_SUCCESS			3

/**
 * Success code strings
 */
EXPORT const char* CPL_S_STR[] = {
	__CPL_S_STR__0,
	__CPL_S_STR__1,
	__CPL_S_STR__2,
	__CPL_S_STR__3,
};

/**
 * The last error code number
 */
#define __CPL_E_LAST_ERROR				-17

/**
 * Error code strings
 */
EXPORT const char* CPL_E_STR[] = {
	__CPL_E_STR__0,
	__CPL_E_STR__1,
	__CPL_E_STR__2,
	__CPL_E_STR__3,
	__CPL_E_STR__4,
	__CPL_E_STR__5,
	__CPL_E_STR__6,
	__CPL_E_STR__7,
	__CPL_E_STR__8,
	__CPL_E_STR__9,
	__CPL_E_STR__10,
	__CPL_E_STR__11,
	__CPL_E_STR__12,
	__CPL_E_STR__13,
	__CPL_E_STR__14,
	__CPL_E_STR__15,
	__CPL_E_STR__16,
	__CPL_E_STR__17,
};

/**
 * The name of the shared semaphore for preserving the atomicity of
 * cpl_lookup_or_create_object()
 */
#define CPL_LOOKUP_OR_CREATE_SEM_INIT	"edu.harvard.pass.cpl.l_or_cr"


/***************************************************************************/
/** Private state                                                         **/
/***************************************************************************/

/**
 * Flag marking whether the library has been initialized
 */
static bool cpl_initialized = false;


/**
 * The shared lock for preserving the atomicity of cpl_lookup_or_create_object
 */
static cpl_shared_semaphore_t cpl_lookup_or_create_object_semaphore;

/**
 * The database backend
 */
static cpl_db_backend_t* cpl_db_backend = NULL;

/**
 * The current session ID
 */
cpl_session_t cpl_session = CPL_NONE;


/**
* attach and detach lock
*/
static mutex_t cpl_backend_lock = PTHREAD_MUTEX_INITIALIZER;
/***************************************************************************/
/** Basic Private API                                                     **/
/***************************************************************************/


/**
 * Make sure that the library has been initialized
 */
#define CPL_ENSURE_INITIALIZED { \
	if (!cpl_initialized) return CPL_E_NOT_INITIALIZED; }



/***************************************************************************/
/** Initialization and Cleanup                                            **/
/***************************************************************************/


/**
 * Initialize the library and attach it to the database backend.
 *
 * @param backend the database backend
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_attach(struct _cpl_db_backend_t* backend)
{	
	mutex_lock(cpl_backend_lock);

	CPL_ENSURE_NOT_NULL(backend);
	if (cpl_initialized){
		mutex_unlock(cpl_backend_lock);
		return CPL_E_ALREADY_INITIALIZED;
	}

	// impossible race condition
	if (cpl_db_backend != NULL){ 
		mutex_unlock(cpl_backend_lock);
		return CPL_E_INTERNAL_ERROR;
	}

	cpl_db_backend = backend;


	// Initialize the locking subsystem

	cpl_return_t ret = cpl_lock_initialize();
	if (!CPL_IS_OK(ret)) {
		cpl_db_backend = NULL;
		mutex_unlock(cpl_backend_lock);
		return ret;
	}


	// Create the session

	const char* user;
	const char* program;
	const char* cmdline;
	int pid;

#ifdef __APPLE__

	user = getenv("USER");
	pid = getpid();
	if (user == NULL){
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	uint32_t _program_size = 8192;
	char* _program_exe_path = new char[_program_size + 1];

	if (_NSGetExecutablePath(_program_exe_path, &_program_size) != 0) {
		delete[] _program_exe_path;
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	char* _program = realpath(_program_exe_path, NULL);
	delete[] _program_exe_path;

	if (_program == NULL) return CPL_E_PLATFORM_ERROR;
	program = _program;

	char _ps[32];
	sprintf(_ps, "/bin/ps -p %d -ww", pid);
	FILE* f = popen(_ps, "r");
	if (f == NULL) {
		delete _program;
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	char ps_header[32];
	if (fgets(ps_header, sizeof(ps_header), f) == NULL) {
		delete _program;
		fclose(f);
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	char* cmd_pos = strstr(ps_header, "CMD");
	if (cmd_pos == NULL) {
		delete _program;
		fclose(f);
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	size_t cmd_offset = cmd_pos - ps_header;
	if (cmd_offset >= strlen(ps_header)) {
		delete _program;
		fclose(f);
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	if (fgets(ps_header, cmd_offset + 1, f) == NULL) {
		delete _program;
		fclose(f);
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	std::string _cmdline = "";
	char buf[256];
	while (!feof(f)) {
		if (fgets(buf, sizeof(buf), f) == NULL) {
		delete _program;
			fclose(f);
			mutex_unlock(cpl_backend_lock);
			return CPL_E_PLATFORM_ERROR;
		}
		size_t l = strlen(buf);
		if (l > 0) {
			if (buf[l-1] == '\n' || buf[l-1] == '\r') {
				buf[l-1] = '\0';
				_cmdline += buf;
				break;
			}
		}
		_cmdline += buf;
	}
	cmdline = _cmdline.c_str();

#else
	user = getenv("USER");
	pid = getpid();
	program = program_invocation_name;
	if (user == NULL){
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}
	if (program == NULL){
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}

	FILE* f = fopen("/proc/self/cmdline", "rb");
	if (f == NULL){
		mutex_unlock(cpl_backend_lock);
		return CPL_E_PLATFORM_ERROR;
	}
	char* _cmdbuf = new char[4096 + 4];
	if (_cmdbuf == NULL){ 
		fclose(f);
		mutex_unlock(cpl_backend_lock);
		return CPL_E_INSUFFICIENT_RESOURCES;
	}
	size_t l = fread(_cmdbuf, 1, 4096, f);
	_cmdbuf[l] = '\0';
	fclose(f);
	
	std::string _cmdline = "";
	std::string token = "";
	bool has_white = false;
	bool has_sq = false;
	for (size_t i = 0; i <= l; i++) {
		char c = _cmdbuf[i];
		if (c != '\0') {
			has_white = has_white || isspace(c);
			has_sq    = has_sq    || c == '\'';
			token += c;
		}
		else {
			if (has_sq) {
				std::string s = "";
				const char* _t = token.c_str();
				size_t _tl = strlen(_t);
				for (size_t j = 0; j < _tl; j++) {
					char d = _t[j];
					if (d == '\'' || d == '\\') s += '\\';
					s += d;
				}
				token = s;
			}
			if (has_white || has_sq) {
				token = "'" + token + "'";
				if (has_sq) token = "@" + token;
			}
			if (_cmdline != "") _cmdline += " ";
			_cmdline += token;
			token = "";
		}
	}
	assert(token == "");
	delete[] _cmdbuf;
	cmdline = _cmdline.c_str();
#endif

	cpl_mac_address_t mac;
	char mac_string[32];
	char* mac_string_ptr = mac_string;

	int r = cpl_platform_get_mac_address(&mac);
	if (r == 0) {

		snprintf(mac_string, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else {
		mac_string_ptr = NULL;
	}

	//currently no support for sessions 
	
	ret = cpl_db_backend->cpl_db_create_session(cpl_db_backend,
												NULL,
												mac_string_ptr,
												user,
												pid,
												program,
												cmdline);


#ifdef __APPLE__
	delete[] _program;
#endif

	if (!CPL_IS_OK(ret)) {
		cpl_db_backend = NULL;
		mutex_unlock(cpl_backend_lock);
		return ret;
	}


	// Initialize the locks

	cpl_lookup_or_create_object_semaphore
		= cpl_shared_semaphore_open(CPL_LOOKUP_OR_CREATE_SEM_INIT);
	if (cpl_lookup_or_create_object_semaphore == NULL) {
		ret = CPL_E_PLATFORM_ERROR;
		cpl_db_backend = NULL;
		mutex_unlock(cpl_backend_lock);
		return ret;
	}


	// Finish

	cpl_initialized = true;
	mutex_unlock(cpl_backend_lock);
	return CPL_OK;
}

/**
 * Initialize the library and attach it to the database backend, 
 * ignoring the errors if already initialized or attached
 *
 * @param backend the database backend
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_try_attach(struct _cpl_db_backend_t* backend){
	
	cpl_return_t ret = cpl_attach(backend);
	if(ret == CPL_E_ALREADY_INITIALIZED){
		return CPL_OK;
	}
	return ret;
}

/**
 * Perform the cleanup and detach the library from the database backend.
 *
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_detach(void)
{
	mutex_lock(cpl_backend_lock);
	CPL_ENSURE_INITIALIZED;
	cpl_initialized = false;

	cpl_db_backend->cpl_db_destroy(cpl_db_backend);
	cpl_db_backend = NULL;

	cpl_shared_semaphore_close(cpl_lookup_or_create_object_semaphore);
	cpl_lock_cleanup();

	mutex_unlock(cpl_backend_lock);
	return CPL_OK;
}
/***************************************************************************/
/** Public API: Helpers                                                   **/
/***************************************************************************/


/**
 * Return the string version of the given error (or success) code
 *
 * @param code the return (success or error) code
 * @return the error or success string (the function always succeeds)
 */
extern "C" EXPORT const char*
cpl_error_string(cpl_return_t code)
{
	if (CPL_IS_OK(code)) {
		if (code >= 0 && code <= __CPL_S_LAST_SUCCESS) return CPL_S_STR[code];
		return "Success (unknown success code)";
	}
	else {
		if (code <= 0 && code >= __CPL_E_LAST_ERROR) return CPL_E_STR[-code];
		return "Unknown error";
	}
}



/***************************************************************************/
/** Public API: Provenance API                                            **/
/***************************************************************************/


/**
 * Create an object.
 *
 * @param originator the application responsible for creating the object
 *                   and generating unique names within its namespace
 * @param name the object name
 * @param type the object type
 * @param bundle the ID of the object that should contain this object
 *                  (use CPL_NONE for no bundle, bundles may not be nested)
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_object(const char* originator,
				  const char* name,
				  const int type,
				  const cpl_id_t bundle,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);

	if(type == BUNDLE && bundle != CPL_NONE){
		return CPL_E_INVALID_ARGUMENT;
	}
	// Call the backend

	cpl_id_t id;
	cpl_return_t ret;

	ret = cpl_db_backend->cpl_db_create_object(cpl_db_backend,
											   originator,
											   name,
											   type,
											   bundle,
											   cpl_session,
											   &id);
	CPL_RUNTIME_VERIFY(ret);


	// Finish

	if (out_id != NULL) *out_id = id;
	return CPL_S_OBJECT_CREATED;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param originator the object originator
 * @param name the object name
 * @param type the object type, 0 for no type
 * @param bundle_id the bundle ID, 0 for no bundle
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object(const char* originator,
				  const char* name,
				  const int type,
				  const cpl_id_t bundle_id,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);


	// Call the backend

	cpl_return_t ret;
	cpl_id_t id;
	
	ret = cpl_db_backend->cpl_db_lookup_object(cpl_db_backend,
											   originator,
											   name,
											   type,
											   bundle_id,
											   &id);
	CPL_RUNTIME_VERIFY(ret);


	if (out_id != NULL) *out_id = id;
	return CPL_OK;
}


/**
 * Look up an object by name. If multiple objects share the same name,
 * return all of them.
 *
 * @param originator the object originator
 * @param name the object name
 * @param type the object type, 0 for no type
 * @param bundle_id the bundle ID, 0 for no bundle
 * @param flags a logical combination of CPL_L_* flags
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object_ext(const char* originator,
					  const char* name,
					  const int type,
				  	  const cpl_id_t bundle_id,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context)
{
	CPL_ENSURE_INITIALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(iterator);


	// Call the backend

	//TODO mess with flags

	cpl_return_t ret;
	ret = cpl_db_backend->cpl_db_lookup_object_ext(cpl_db_backend,
												   originator,
												   name,
												   type,
												   bundle_id,
												   flags,
												   iterator,
												   context);

	if (ret == CPL_E_NOT_FOUND && (flags & CPL_L_NO_FAIL) == CPL_L_NO_FAIL) {
		ret = CPL_S_NO_DATA;
	}
	return ret;
}


/**
 * Lookup or create an object if it does not exist.
 *
 * @param originator the application responsible for creating the object
 *                   and generating unique names within its namespace
 * @param name the object name
 * @param type the object type
 * @param bundle the ID of the object that should contain this object
 *                  (use CPL_NONE for no bundle)
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_or_create_object(const char* originator,
							const char* name,
							const int type,
							const cpl_id_t bundle,
							cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;
	int r = CPL_E_INTERNAL_ERROR;

	//TODO think about locks here
	cpl_shared_semaphore_wait(cpl_lookup_or_create_object_semaphore);

	r = cpl_lookup_object(originator, name, type, bundle, out_id);
	if (r != CPL_E_NOT_FOUND) goto out;

	r = cpl_create_object(originator, name, type, bundle, out_id);
	if (CPL_IS_OK(r)) r = CPL_S_OBJECT_CREATED;

out:
	cpl_shared_semaphore_post(cpl_lookup_or_create_object_semaphore);
	return r;
}


/**
 * Add a property to the given object.
 * @param id the object ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_object_property(const cpl_id_t id,
				 const char* key,
                 const char* value)
{
	CPL_ENSURE_INITIALIZED;
  


	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);


    // Call the backend

	return cpl_db_backend->cpl_db_add_object_property(cpl_db_backend,
											   id,
											   key,
											   value);
}


/**
 * Add a property to the given relation.
 * @param id the object ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation_property(const cpl_id_t id,
						  const char* key,
						  const char* value)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);

	// Call the backend

	return cpl_db_backend->cpl_db_add_relation_property(cpl_db_backend,
														id,
														key,
														value);
};

/**
 * Add a relation.
 * @param from_id the "from" end of the dependency edge
 * @param to_id the "to" end of the dependency edge
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const int type,
				   const cpl_id_t bundle,
				   cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments

	CPL_ENSURE_NOT_NONE(from_id);
	CPL_ENSURE_NOT_NONE(to_id);
	CPL_ENSURE_NOT_NONE(bundle);


	cpl_id_t id;
	cpl_return_t ret;

	ret = cpl_db_backend->cpl_db_add_relation(cpl_db_backend,
													from_id,
													to_id,
													type,
													bundle,
													&id);


	CPL_RUNTIME_VERIFY(ret);

	// Finish

	if (out_id != NULL) *out_id = id;
	return CPL_OK;
}


/**
 * Get the ID of the current session.
 *
 * @param out_session the pointer to store the ID of the current session
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_current_session(cpl_session_t* out_session)
{
	CPL_ENSURE_INITIALIZED;

	if (out_session != NULL) *out_session = cpl_session;
	return CPL_OK;
}


/**
 * Get information about the given provenance session.
 *
 * @param id the session ID
 * @param out_info the pointer to store the session info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_session_info(const cpl_session_t id,
					 cpl_session_info_t** out_info)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(out_info);


	// Call the database backend

	return cpl_db_backend->cpl_db_get_session_info(cpl_db_backend, id,
												   out_info);
}


/**
 * Free cpl_session_info_t.
 *
 * @param info the pointer to the session info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_free_session_info(cpl_session_info_t* info)
{
	CPL_ENSURE_NOT_NULL(info);

	if (info->mac_address != NULL) free(info->mac_address);
	if (info->user != NULL) free(info->user);
	if (info->program != NULL) free(info->program);
	if (info->cmdline != NULL) free(info->cmdline);

	free(info);
	return CPL_OK;
}


/**
 * Get all objects in the database
 *
 * @param flags a logical combination of CPL_I_* flags
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_all_objects(const int flags,
					cpl_object_info_iterator_t iterator,
					void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NULL(iterator);

	return cpl_db_backend->cpl_db_get_all_objects(cpl_db_backend, flags,
                                                  iterator, context);
}


/**
 * Get information about the given provenance object.
 *
 * @param id the object ID
 * @param out_info the pointer to store the object info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_object_info(const cpl_id_t id,
					cpl_object_info_t** out_info)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(out_info);

	// Call the database backend

	return cpl_db_backend->cpl_db_get_object_info(cpl_db_backend, id,
												  out_info);
}


/**
 * Free cpl_object_info_t.
 *
 * @param info the pointer to the object info structure.
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_free_object_info(cpl_object_info_t* info)
{
	CPL_ENSURE_NOT_NULL(info);

	if (info->originator != NULL) free(info->originator);
	if (info->name != NULL) free(info->name);

	free(info);
	return CPL_OK;
}


/**
 * Iterate over the ancestors or the descendants of a provenance object.
 *
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
extern "C" EXPORT cpl_return_t
cpl_get_object_relations(const cpl_id_t id,
						const int direction,
						const int flags,
						cpl_relation_iterator_t iterator,
						void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	if (direction != CPL_D_ANCESTORS && direction != CPL_D_DESCENDANTS) {
		return CPL_E_INVALID_ARGUMENT;
	}


	// Call the database backend

	cpl_return_t r;
	r = cpl_db_backend->cpl_db_get_object_relations(cpl_db_backend,
												   id, direction,
												   flags, iterator,
												   context);
	
	if (r == CPL_S_NO_DATA) return CPL_OK;
	return r;
}


/**
 * Get the properties associated with the given provenance object.
 * 
 * @param id the the object ID
 * @param key the property to fetch - or NULL for all properties
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_object_properties(const cpl_id_t id,
				   const char* key,
				   cpl_property_iterator_t iterator,
				   void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);



	// Call the database backend

	return cpl_db_backend->cpl_db_get_object_properties(cpl_db_backend,
												 id, key,
												 iterator, context);
}


/**
 * Lookup an object based on a property value.
 * 
 * @param key the property name
 * @param value the property value
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object_by_property(const char* key,
					   const char* value,
					   cpl_property_iterator_t iterator,
					   void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NULL(iterator);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);


	// Call the database backend

	return cpl_db_backend->cpl_db_lookup_object_by_property(cpl_db_backend,
													 key, value,
													 iterator, context);
}

/**
 * Get the properties associated with the given provenance relations.
 * 
 * @param id the the relation ID
 * @param key the property to fetch - or NULL for all properties
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_relation_properties(const cpl_id_t id,
				   const char* key,
				   cpl_property_iterator_t iterator,
				   void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	// Call the database backend

	return cpl_db_backend->cpl_db_get_relation_properties(cpl_db_backend,
												 id, key,
												 iterator, context);
}

/**
 * Deletes a bundle and all objects and relations belonging to it.
 *
 * @param id the bundle ID
 * @return CPL_OK, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_delete_bundle(const cpl_id_t id)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);

	return cpl_db_backend->cpl_db_delete_bundle(cpl_db_backend, id);
}

/**
 * Get all objects belonging to a bundle
 *
 * @paramID the bundle ID
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_bundle_objects(const cpl_id_t id,
					cpl_object_info_iterator_t iterator,
					void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	return cpl_db_backend->cpl_db_get_bundle_objects(cpl_db_backend, id,
												iterator, context);
}

/**
 * Get all relations belonging to a bundle
 *
 * @paramID the bundle ID
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_bundle_relations(const cpl_id_t id,
					cpl_relation_iterator_t iterator,
					void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	return cpl_db_backend->cpl_db_get_bundle_relations(cpl_db_backend, id,
												iterator, context);
}



/***************************************************************************/
/** Public API: Enhanced C++ Functionality                                **/
/***************************************************************************/

#ifdef __cplusplus

/**
 * The iterator callback for cpl_get_all_objects() that collects the returned
 * information in an instance of std::vector<cplxx_object_info_t>.
 *
 * @param info the object info
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_object_info_vector(const cpl_object_info_t* info,
							      void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cplxx_object_info_t e;
	e.id = info->id;
	e.creation_session = info->creation_session;
    e.creation_time = info->creation_time;
    e.originator = info->originator;
    e.name = info->name;
    e.type = info->type;
    e.bundle_id = info->bundle_id;

	std::vector<cplxx_object_info_t>& l =
		*((std::vector<cplxx_object_info_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_lookup_object_ext() that collects the returned
 * information in an instance of std::vector<cpl_id_timestamp_t>.
 *
 * @param id the object ID
 * @param @param timestamp the object creation time expressed as UNIX time
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_id_timestamp_vector(const cpl_id_t id,
								   const unsigned long timestamp,
								   void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_id_timestamp_t e;
	e.id = id;
	e.timestamp = timestamp;

	std::vector<cpl_id_timestamp_t>& l =
		*((std::vector<cpl_id_timestamp_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_object_relations() that collects
 * the passed-in information in an instance of std::list<cpl_relation_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        relation
 * @param type the type of the data or the control dependency
 * @param context the pointer to an instance of the list
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_relation_list(const cpl_id_t relation_id,
							 const cpl_id_t query_object_id,
							 const cpl_id_t other_object_id,
							 const int type,
							 void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_relation_t e;
	e.id = relation_id;
	e.query_object_id = query_object_id;
	e.other_object_id = other_object_id;
	e.type = type;

	std::list<cpl_relation_t>& l =
		*((std::list<cpl_relation_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_object_relations() that collects
 * the information in an instance of std::vector<cpl_relation_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        relation
 * @param type the type of the relation
 * @param context the pointer to an instance of the vector
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_relation_vector(const cpl_id_t relation_id,
							   const cpl_id_t query_object_id,
							   const cpl_id_t other_object_id,
							   const int type,
							   const cpl_id_t bundle_id,
							   void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_relation_t e;
	e.id = relation_id;
	e.query_object_id = query_object_id;
	e.other_object_id = other_object_id;
	e.type = type;
	e.bundle_id = bundle_id;

	std::vector<cpl_relation_t>& l =
		*((std::vector<cpl_relation_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_properties() that collects the returned
 * information in an instance of std::vector<cplxx_property_entry_t>.
 *
 * @param id the object ID
 * @param key the property name
 * @param value the property value
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_properties_vector(const cpl_id_t id,
								 const char* key,
								 const char* value,
								 void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cplxx_property_entry_t e;
	e.id = id;
	e.key = key;
	e.value = value;

	std::vector<cplxx_property_entry_t>& l =
		*((std::vector<cplxx_property_entry_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_lookup_by_property() that collects
 * the returned information in an instance of std::vector<cpl_id_t>.
 *
 * @param id the object ID
 * @param key the property name
 * @param value the property value
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_property_lookup_vector(const cpl_id_t id,
									  const char* key,
									  const char* value,
									  void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_id_t e = id;


	std::vector<cpl_id_t>& l =
		*((std::vector<cpl_id_t>*) context);
	l.push_back(e);

	return CPL_OK;
}



/***************************************************************************/
/** Public API: Document Handling                                         **/
/***************************************************************************/

/*
 * Helper array
 */
prov_relation_data_t rdata_array[] =
{ 
	{WASINFLUENCEDBY,
	WASINFLUENCEDBY_STR,
	"prov:influencee",
	0,
	"prov:influencer",
	0},
	{ALTERNATEOF,
	ALTERNATEOF_STR,
	"prov:alternate1",
	ENTITY,
	"prov:alternate2",
	ENTITY},
	{DERIVEDBYINSERTIONFROM,
	DERIVEDBYINSERTIONFROM_STR,
	"prov:after",
	ENTITY,
	"prov:before",
	ENTITY},
	{DERIVEDBYREMOVALFROM,
	DERIVEDBYREMOVALFROM_STR,
	"prov:after",
	ENTITY,
	"prov:before",
	ENTITY},
	{HADMEMBER,
	HADMEMBER_STR,
	"prov:collection",
	ENTITY,
	"prov:before",
	ENTITY},
	{HADDICTIONARYMEMBER,
	HADDICTIONARYMEMBER_STR,
	"prov:dictionary",
	ENTITY,
	"prov:entity",
	ENTITY},
	{SPECIALIZATIONOF,
	SPECIALIZATIONOF_STR,
	"prov:specificEntity",
	ENTITY,
	"prov:generalEntity",
	ENTITY},
	{WASDERIVEDFROM,
	WASDERIVEDFROM_STR,
	"prov:generatedEntity",
	ENTITY,
	"prov:usedEntity",
	ENTITY},
	{WASGENERATEDBY,
	WASGENERATEDBY_STR,
	"prov:entity",
	ENTITY,
	"prov:activity",
	ACTIVITY},
	{WASINVALIDATEDBY,
	WASINVALIDATEDBY_STR,
	"prov:entity",
	ENTITY,
	"prov:activity",
	ACTIVITY},
	{WASATTRIBUTEDTO,
	WASATTRIBUTEDTO_STR,
	"prov:entity",
	ENTITY,
	"prov:agent",
	AGENT},
	{USED,
	USED_STR,
	"prov:activity",
	ACTIVITY,
	"prov:entity",
	ENTITY},
	{WASINFORMEDBY,
	WASINFORMEDBY_STR,
	"prov:informed",
	ACTIVITY,
	"prov:informant",
	ACTIVITY},
	{WASSTARTEDBY,
	WASSTARTEDBY_STR,
	"prov:activity",
	ACTIVITY,
	"prov:trigger",
	ENTITY},
	{WASENDEDBY,
	WASENDEDBY_STR,
	"prov:activity",
	ACTIVITY,
	"prov:trigger",
	ENTITY},
	{HADPLAN,
	HADPLAN_STR,
	"prov:agent",
	AGENT,
	"prov:plan",
	ENTITY},
	{WASASSOCIATEDWITH,
	WASASSOCIATEDWITH_STR,
	"prov:activity",
	ACTIVITY,
	"prov:agent",
	AGENT},
	{ACTEDONBEHALFOF,
	ACTEDONBEHALFOF_STR,
	"prov:delegate",
	AGENT,
	"prov:responsible",
	AGENT}
};

/*
 * Verifies the correctness of a Prov-JSON document. Not currently exhaustive.
 * Free *string_out after use.
 * 
 * @param path the JSON file path
 * @param string_out error output string
 * @return 0 on successful validation or -1 on failure
 */
EXPORT int
validate_json(const char* path,
	 		  char** string_out)
{
	std::string str_msg;

	//TODO all string messages currently less than 50
	*string_out = new char[50];

	str_msg = "Validation failed on upload";
	strncpy(*string_out, str_msg.c_str(), 50);

	json_error_t err;
	FILE* fp = fopen(path, "r");
		json_t* document = json_loadf(fp, 0, &err);
	fclose(fp);

	if(document == NULL){
		return -1;
	}

	std::vector<std::string> objects;

	igraph_vector_t edges;
	igraph_vector_init(&edges, 0);

	str_msg = "Invalid Prov-JSON formatting";
	strncpy(*string_out, str_msg.c_str(), 50);

	for(int i=0; i<NUM_R_TYPES; i++){
		prov_relation_data_t entry = rdata_array[i];
		json_t* relations = json_object_get(document, entry.type_str.c_str());

		const char* name;
		json_t*  value;

		json_object_foreach(relations, name, value){

			std::string source(json_string_value(json_object_get(value, 
													entry.source_str.c_str())));
			std::string dest(json_string_value(json_object_get(value, 
													entry.dest_str.c_str())));

			if(source.empty()){
				json_decref(relations);
				json_decref(document);
				return -1;
			}

			if(!dest.empty()){
				int source_int, dest_int;

				if (int pos = std::find(objects.begin(), objects.end(), source) 
																!= objects.end()){
					source_int = pos;
				} else {
					objects.push_back(source);
					source_int = objects.size()-1;
				}

				if (int pos = std::find(objects.begin(), objects.end(), dest) 
																!= objects.end()){
					dest_int = pos;
				} else {
					objects.push_back(dest);
					dest_int = objects.size()-1;
				}

				igraph_vector_push_back(&edges, source_int);
				igraph_vector_push_back(&edges, dest_int);
			}
		}

		json_decref(relations);
	}

	igraph_t graph;
	igraph_empty(&graph, objects.size(), IGRAPH_DIRECTED);
	igraph_add_edges(&graph, &edges, 0);

	igraph_bool_t is_dag;
	igraph_is_dag(&graph, &is_dag);

	if(!is_dag){
		str_msg = "Prov-JSON document contains cycles";
		strncpy(*string_out, str_msg.c_str(), 50);
		json_decref(document);
		return -1;
	}

	str_msg = "Valid Prov-JSON";
	strncpy(*string_out, str_msg.c_str(), 50);
	//json_decref(document);
	return 0;
}

/*
 * Imports prefixes. import_document_json helper function.
 */
cpl_return_t
import_bundle_prefixes_json(cpl_id_t bundle,
							json_t* document)
{
	json_t* prefixes = json_object_get(document, "prefix");

	const char* key_str;
	json_t* value;

	json_object_foreach(prefixes, key_str, value){

		const char* val_str = json_string_value(value);

		if(key_str && val_str){
			if(!CPL_IS_OK(cpl_add_object_property(bundle, key_str, val_str))){
				json_decref(prefixes);
				return CPL_E_INTERNAL_ERROR;
			}
		}
	}

	json_decref(prefixes);
	return CPL_OK;
}


/*
 * Imports objects. import_document_json helper function.
 */
cpl_return_t
import_objects_json(int type,
					const char* type_str,
					const char* originator,
					cpl_id_t bundle_id,
					json_t* document)
{
	json_t* objects = json_object_get(document, type_str);

	const char* name_str;
	json_t* properties;

	json_object_foreach(objects, name_str, properties){

		cpl_id_t obj_id = CPL_NONE;

		if(!CPL_IS_OK(cpl_create_object(originator, name_str, type, bundle_id, &obj_id))){
			json_decref(objects);
			return CPL_E_INTERNAL_ERROR;
		}

		if(obj_id){

			const char* pkey_str;
			json_t* pval;

			json_object_foreach(properties, pkey_str, pval){

				const char* pval_str = json_string_value(pval);

				if(pkey_str && pval_str){
					if(!CPL_IS_OK(cpl_add_object_property(obj_id, pkey_str, pval_str))){
						return CPL_E_INTERNAL_ERROR;
					}
				}
			}
		}
	}

	json_decref(objects);
	return CPL_OK;
}

/*
 * Imports relations. import_document_json helper function.
 */
cpl_return_t
import_relations_json(const char* originator,
					  cpl_id_t bundle_id,
					  json_t* document)
{
	for(int i=0; i<NUM_R_TYPES; i++){
		prov_relation_data_t entry = rdata_array[i];
		json_t* relations = json_object_get(document, entry.type_str.c_str());

		const char* name;
		json_t* relation;


		json_object_foreach(relations, name, relation){

			cpl_id_t source, dest, relation_id;

			if(!CPL_IS_OK(cpl_lookup_object(originator, 
											json_string_value(json_object_get(relation, 
																entry.source_str.c_str())),
											entry.source_t,
											bundle_id,
											&source))){
				json_decref(relations);
				return CPL_E_INTERNAL_ERROR;
			}
			if(!CPL_IS_OK(cpl_lookup_object(originator, 
											json_string_value(json_object_get(relation,
																entry.dest_str.c_str())),
											entry.dest_t,
											bundle_id,
											&dest))){
				json_decref(relations);
				return CPL_E_INTERNAL_ERROR;
			}

			if(!CPL_IS_OK(cpl_add_relation(source, dest, entry.type, bundle_id, &relation_id))){
				return CPL_E_INTERNAL_ERROR;
			}

			const char* key_str;
			json_t* value;

			json_object_foreach(relation, key_str, value){

				const char* val_str = json_string_value(value);

				if(key_str && val_str){
					if(!CPL_IS_OK(cpl_add_relation_property(relation_id, key_str, val_str))){
						json_decref(relations);
						return CPL_E_INTERNAL_ERROR;
					}
				}
			}

			json_decref(relations);
		}
	}

	return CPL_OK;
}

/*
 * Imports a Prov-JSON document into Prov-CPL.
 *
 * @param filename file path to document
 * @param originator document originator
 * @param bundle_name desired name of document bundle
 * @param anchor_object optional PROV_CPL object identical to an object in the document
 * @param bundle_agent optional agent responsible for the document bundle
 * @param out_id the ID of the imported bundle
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
import_document_json(const char* filename,
					 const char* originator,
					 const char* bundle_name,
					 const cpl_id_t anchor_object,
					 const cpl_id_t bundle_agent,
					 cpl_id_t* out_id)
{
	json_error_t err;

	FILE* fp = fopen(filename, "r");
		json_t* document = json_loadf(fp, 0, &err);
	fclose(fp);

	if(document == NULL){
		return CPL_E_INTERNAL_ERROR;
	}

	// TODO maybe call validate_json

	// Create bundle
	cpl_id_t bundle_id;
	if(!CPL_IS_OK(cpl_create_object(originator, bundle_name, BUNDLE, 0, &bundle_id))){
		goto error;
	}

	if(!CPL_IS_OK(import_bundle_prefixes_json(bundle_id, document))){
		goto error;
	}

	// Connect bundle_agent to bundle
	if(bundle_agent){
		if(!CPL_IS_OK(cpl_add_relation(bundle_id, bundle_agent, 
						WASATTRIBUTEDTO, bundle_id, NULL))){
			goto error;
		}
	}

	// Import objects
	if(!CPL_IS_OK(import_objects_json(ENTITY, ENTITY_STR, originator, bundle_id, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(AGENT, AGENT_STR, originator, bundle_id, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(ACTIVITY, ACTIVITY_STR, originator, bundle_id, document))){
		goto error;
	}

	// Connect anchor object to new object from document
	// TODO possible that originator is different
	if(anchor_object){
		cpl_object_info_t* anchor_info;
		cpl_get_object_info(anchor_object, &anchor_info);
		cpl_id_t dest;
		if(CPL_IS_OK(cpl_lookup_object(originator, 
						anchor_info->name, anchor_info->type, bundle_id, &dest))){

			cpl_free_object_info(anchor_info);
			if(dest){
					if(!CPL_IS_OK(cpl_add_relation(anchor_object, dest,
								ALTERNATEOF, bundle_id, NULL))){
					goto error;
				}
			}
		}
	}

	if(!CPL_IS_OK(import_relations_json(originator, bundle_id, document))){
		goto error;
	}

	if (out_id != NULL) *out_id = bundle_id;

	return CPL_OK;

error:
	cpl_delete_bundle(bundle_id);
	return CPL_E_INTERNAL_ERROR;
}

/*
 * Retrieves bundle prefixes. export_bundle_json helper function.
 */
int
export_bundle_prefixes_json(cpl_id_t bundle, 
							json_t* document)
{

	std::vector<cplxx_property_entry_t> prefix_vec;
	cpl_get_object_properties(bundle, NULL, cpl_cb_collect_properties_vector, &prefix_vec);

	if (prefix_vec.size()){

		json_t* prefixes = json_object();

		for(auto & prefix: prefix_vec){
			if(json_object_set_new(prefixes, prefix.key.c_str(), json_string(prefix.value.c_str()))){
				json_decref(prefixes);
				return -1;
			}
		}

		if(json_object_set_new(document, "prefixes", prefixes)){
			return -1;
		}
	}

	return 0;
}

/*
 * Retrieves bundle objects. export_bundle_json helper function.
 */
int
export_objects_json(cpl_id_t bundle, 
					json_t* document)
{

	std::vector<cplxx_object_info_t> object_vec;
	cpl_get_bundle_objects(bundle, cpl_cb_collect_object_info_vector, &object_vec);

	if(object_vec.empty()){
		return 0;
	}

    json_t* json_type_array[3] = {NULL};

	std::vector<cplxx_property_entry_t> property_vec;

	for(auto & obj: object_vec){
		if(obj.type == BUNDLE){
			break;
		}

		json_t* properties = json_object();
		cpl_get_object_properties(obj.id, NULL, 
			cpl_cb_collect_properties_vector, &property_vec);

		for(auto & property: property_vec){
			if(json_object_set_new(properties, property.key.c_str(), json_string(property.value.c_str()))){
				json_decref(properties);
				goto error;
			}
		}

		if(!json_type_array[obj.type-1]){
			json_type_array[obj.type-1] = json_object();
		}

		if(json_object_set(json_type_array[obj.type-1], obj.name.c_str(), properties)){
			goto error;
		}
	}

	if(json_type_array[0]){
		if(json_object_set_new(document, ENTITY_STR, json_type_array[0])){
			goto error;
		}
	}
	if(json_type_array[1]){
		if(json_object_set_new(document, ACTIVITY_STR, json_type_array[1])){
			goto error;
		}
	}
	if(json_type_array[2]){
		if(json_object_set_new(document, AGENT_STR, json_type_array[2])){
			goto error;
		}
	}

	return 0;

error:
	for(int i=0; i<3; i++){
		if(json_type_array[i]){
			json_decref(json_type_array[i]);
		}
	}
	return -1;

}

/*
 * Retrieves bundle relations. export_bundle_json helper function.
 */
int
export_relations_json(cpl_id_t bundle,
				      json_t* document)
{

	std::vector<cpl_relation_t> relation_vec;
	cpl_get_bundle_relations(bundle, cpl_cb_collect_relation_vector, &relation_vec);

	if(relation_vec.empty()){
		return 0;
	}

	json_t* json_type_array[NUM_R_TYPES] = {NULL};

	std::vector<cplxx_property_entry_t> property_vec;

	for(auto & relation: relation_vec){

		json_t* properties = json_object();
		cpl_get_relation_properties(relation.id, NULL, 
			cpl_cb_collect_properties_vector, &property_vec);

		for(auto & property: property_vec){
			if(json_object_set_new(properties, property.key.c_str(), json_string(property.value.c_str()))){
				json_decref(properties);
				goto error;
			}
		}

		property_vec.clear();

		cpl_object_info_t* from_info = (cpl_object_info_t*) malloc(sizeof(cpl_object_info_t));
		cpl_object_info_t* to_info = (cpl_object_info_t*) malloc(sizeof(cpl_object_info_t));

		cpl_get_object_info(relation.query_object_id, &from_info);
		cpl_get_object_info(relation.other_object_id, &to_info);

		json_object_set_new(properties, rdata_array[relation.type-1].source_str.c_str(), json_string(from_info->name));
		json_object_set_new(properties, rdata_array[relation.type-1].dest_str.c_str(), json_string(to_info->name));

		cpl_free_object_info(from_info);
		cpl_free_object_info(to_info);

		if(!json_type_array[relation.type-1]){
			json_type_array[relation.type-1] = json_object();
		}

		if(json_object_set_new(json_type_array[relation.type-1], std::to_string(relation.id).c_str(), properties)){
			goto error;
		}
	}

	for(int i=0; i<NUM_R_TYPES; i++){
		if(json_type_array[i]){
			if(json_object_set_new(document, rdata_array[i].type_str.c_str(), json_type_array[i])){
				goto error;
			}
		}
	}

	return 0;

error:
	for(int i=0; i<NUM_R_TYPES; i++){
		if(json_type_array[i]){
			json_decref(json_type_array[i]);
		}
	}
	return -1;
}

/*
 * Exports a Prov-CPL bundle as a Prov-JSON document.
 *
 * @param bundle the bundle ID
 * @param path path to desired output file, overwrites if file already exists
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
export_bundle_json(const cpl_id_t bundle, 
				   const char* path)
{

	json_t* document = json_object();

	if(export_bundle_prefixes_json(bundle, document)){
		goto error;
	}

	if(export_objects_json(bundle, document)){
		goto error;
	}

	if(export_relations_json(bundle, document)){
		goto error;
	}

	if(json_dump_file(document, path, 0)){
		goto error;
	}

	json_decref(document);
	return CPL_OK;

error: 
	json_decref(document);
	return CPL_E_INTERNAL_ERROR;
}

#endif /* __cplusplus */
