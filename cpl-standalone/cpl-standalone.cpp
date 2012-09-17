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
#include "cpl-platform.h"

#ifndef _WINDOWS
#include <errno.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif



/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
EXPORT const cpl_id_t CPL_NONE = { { { 0, 0 } } };

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
 * Flag for whether the library can use the object cache
 */
static bool cpl_cache = true;

/**
 * Flag for whether the library should check the cache for staleness (this
 * can be false if there is only a single running instance of CPL)
 */
static bool cpl_cache_check = true;

/**
 * The cache of open objects
 */
static cpl_hash_map_id_to_open_object_t cpl_open_objects;

/**
 * The lock for the cache of open objects
 */
static cpl_lock_t cpl_open_objects_lock;

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
static cpl_session_t cpl_session = CPL_NONE;



/***************************************************************************/
/** Advanced Private API - Function Prototypes                            **/
/***************************************************************************/


/**
 * Add a dependency
 *
 * @param from_id the "from" end of the dependency edge
 * @param to_id the "to" end of the dependency edge
 * @param to_ver the version of the "to" end of the dependency edge
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
cpl_return_t
cpl_add_dependency(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const cpl_version_t to_ver,
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
	obj->last_session = CPL_NONE;

	return obj;
}


/**
 * Get the open object handle, opening the object if necessary,
 * and return the object locked
 *
 * @param id the object ID
 * @param out the output
 * @param is_new whether the object was just created in the cache
 * @return the error code
 */
cpl_return_t
cpl_get_open_object_handle(const cpl_id_t id, cpl_open_object_t** out,
		bool* is_new)
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
			if (is_new != NULL) *is_new = false;
			return CPL_OK;
		}
		cpl_unlock(&cpl_open_objects_lock);
	}


	// Periodically drop the cache if it gets too full
	// TODO We can do much better than this
	
	if (cpl_cache && cpl_open_objects.size() > 1024 * 1024) {
		cpl_drop_object_cache(false); /* keep locked objects */
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
			if (is_new != NULL) *is_new = false;
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
	if (is_new != NULL) *is_new = true;
	return CPL_OK;
}


/**
 * Drop the cache
 *
 * @param force whether to force-close even the locked objects
 * @return the error code
 */
cpl_return_t
cpl_drop_object_cache(bool force)
{
	if (cpl_cache) return CPL_OK;

	cpl_lock(&cpl_open_objects_lock);

	cpl_hash_map_id_to_open_object_t::iterator i; 
	cpl_hash_map_id_to_open_object_t in_use;
	for (i = cpl_open_objects.begin(); i != cpl_open_objects.end(); i++) {
		if (i->second->locked && !force) {
			in_use[i->first] = i->second;
		}
		else {
			delete i->second;
		}
	}
	cpl_open_objects.clear();

	for (i = in_use.begin(); i != in_use.end(); i++) {
		cpl_open_objects[i->first] = i->second;
	}
		
	cpl_unlock(&cpl_open_objects_lock);
	
	return CPL_OK;
}


/**
 * Create (thaw) a new version of the given provenance object if necessary
 *
 * @param id the object ID
 * @param force_thaw if we have to create the new version
 * @param out_version the version number (can be NULL)
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_thaw(const cpl_id_t id,
		 const bool force_thaw,
		 cpl_version_t* out_version)
{
    cpl_return_t ret;
    cpl_version_t version;


	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);


	// Get the current version of the object

	bool must_freeze = true;
	bool is_new = false;
	cpl_open_object_t* obj = NULL;

	if (cpl_cache) {

		// Get the handle of the fromination

		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(id, &obj, &is_new));


		// Get the version of the object and check to see if the entry is stale
		
		if (cpl_cache_check && !is_new) {
			ret = cpl_db_backend->cpl_db_get_version(cpl_db_backend,
					id, &version);
			CPL_RUNTIME_VERIFY(ret);
		}
		else {
			version = obj->version;
		}


		// Determine whether to freeze and create a new version (if not stale)

		if (obj->version == version) {
			must_freeze = obj->frozen || obj->last_session != cpl_session;
		}
	}
	else {

		// Get the version of the object

		ret = cpl_get_version(id, &version);
		CPL_RUNTIME_VERIFY(ret);
	}


	// Automatically unlock the object if it is still locked by the time
	// we hit end this block

	CPL_AutoUnlock __au(obj != NULL ? &obj->locked : NULL);
	(void) __au;


	// Create a new version, if necessary
	
	if (must_freeze || force_thaw) {

		cpl_return_t r = CPL_E_ALREADY_EXISTS;
		version++;

		do {
			r = cpl_db_backend->cpl_db_create_version(cpl_db_backend,
													  id,
													  version,
													  cpl_session);
			if (r == CPL_E_ALREADY_EXISTS) {
#ifdef _WINDOWS
				Sleep(2 /* ms */);
#else
				usleep(2 * 1000 /* us */);
#endif
				version++;
			}
			else {
				CPL_RUNTIME_VERIFY(r);
			}
		}
		while (!CPL_IS_OK(r));

		assert(version != CPL_VERSION_NONE);
	}
	

	// Update the cache
	
	if (obj != NULL) {

		// Update the session and version info

		obj->version = version;
		obj->last_session = cpl_session;
		obj->frozen = false;


		// Finally, unlock (must be last)
		
		cpl_unlock(&obj->locked);
		obj = NULL;
	}


    // Finish

	if (out_version != NULL) *out_version = version;
	return CPL_OK;
}


/***************************************************************************/
/** Initialization and Cleanup                                            **/
/***************************************************************************/


/**
 * Initialize the library and attach it to the database backend. Please note
 * that this function is not thread-safe.
 *
 * @param backend the database backend
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_attach(struct _cpl_db_backend_t* backend)
{
	CPL_ENSURE_NOT_NULL(backend);
	if (cpl_initialized) return CPL_E_ALREADY_INITIALIZED;

	if (cpl_db_backend != NULL) return CPL_E_INTERNAL_ERROR; // race condition
	cpl_db_backend = backend;


	// Initialize the locking subsystem

	cpl_return_t ret = cpl_lock_initialize();
	if (!CPL_IS_OK(ret)) {
		cpl_db_backend = NULL;
		return ret;
	}


	// Create the session

	const char* user;
	const char* program;
	const char* cmdline;
	int pid;

#ifdef _WINDOWS
	DWORD _user_size = 256;
	DWORD _program_size = 4096;
	BOOL _ok;

	char* _user = new char[_user_size];
	if (_user == NULL) {
		cpl_db_backend = NULL;
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	_ok = GetUserName(_user, &_user_size);
	if (!_ok) {
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
			delete[] _user;
			_user = new char[_user_size];
			if (_user == NULL) {
				cpl_db_backend = NULL;
				return CPL_E_INSUFFICIENT_RESOURCES;
			}
			_ok = GetUserName(_user, &_user_size);
		}
	}
	if (!_ok) {
		cpl_db_backend = NULL;
		delete[] _user;
		return CPL_E_PLATFORM_ERROR;
	}

	char* _program = new char[_program_size];
	size_t _old_program_size = _program_size;
	if (_program == NULL) {
		cpl_db_backend = NULL;
		delete[] _user;
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	_program_size = GetModuleFileName(0, _program, _program_size);
	if (_program_size > _old_program_size) {
		delete[] _program;
		_program = new char[_program_size];
		_old_program_size = _program_size;
		if (_program == NULL) {
			cpl_db_backend = NULL;
			delete[] _user;
			return CPL_E_INSUFFICIENT_RESOURCES;
		}
		_program_size = GetModuleFileName(0, _program, _program_size);
	}
	if (_program_size <= 0 || _program_size > _old_program_size) {
		cpl_db_backend = NULL;
		delete[] _user;
		delete[] _program;
		return CPL_E_PLATFORM_ERROR;
	}
	_program_size++;

	user = _user;
	program = _program;
	pid = GetCurrentProcessId();
	cmdline = GetCommandLine();

#elif defined(__APPLE__)

	user = getenv("USER");
	pid = getpid();
	if (user == NULL) return CPL_E_PLATFORM_ERROR;

	uint32_t _program_size = 8192;
	char* _program_exe_path = new char[_program_size + 1];

	if (_NSGetExecutablePath(_program_exe_path, &_program_size) != 0) {
		delete[] _program_exe_path;
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
		return CPL_E_PLATFORM_ERROR;
	}

	char ps_header[32];
	if (fgets(ps_header, sizeof(ps_header), f) == NULL) {
		delete _program;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}

	char* cmd_pos = strstr(ps_header, "CMD");
	if (cmd_pos == NULL) {
		delete _program;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}

	size_t cmd_offset = cmd_pos - ps_header;
	if (cmd_offset >= strlen(ps_header)) {
		delete _program;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}

	if (fgets(ps_header, cmd_offset + 1, f) == NULL) {
		delete _program;
		fclose(f);
		return CPL_E_PLATFORM_ERROR;
	}

	std::string _cmdline = "";
	char buf[256];
	while (!feof(f)) {
		if (fgets(buf, sizeof(buf), f) == NULL) {
		delete _program;
			fclose(f);
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
	if (user == NULL) return CPL_E_PLATFORM_ERROR;
	if (program == NULL) return CPL_E_PLATFORM_ERROR;

	FILE* f = fopen("/proc/self/cmdline", "rb");
	if (f == NULL) return CPL_E_PLATFORM_ERROR;
	char* _cmdbuf = new char[4096 + 4];
	if (_cmdbuf == NULL) { fclose(f); return CPL_E_INSUFFICIENT_RESOURCES; }
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
#ifdef _WINDOWS
		sprintf_s(mac_string, 32,
#else
		snprintf(mac_string, 32,
#endif
#if 0
			) /* Hack to deal with smart but not too smart editors */
#endif
				"%02x:%02x:%02x:%02x:%02x:%02x",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else {
		mac_string_ptr = NULL;
	}

	cpl_generate_unique_id(&cpl_session);
	ret = cpl_db_backend->cpl_db_create_session(cpl_db_backend,
												cpl_session,
												mac_string_ptr,
												user,
												pid,
												program,
												cmdline);

#ifdef _WINDOWS
	delete[] _user;
	delete[] _program;
#endif

#ifdef __APPLE__
	delete[] _program;
#endif

	if (!CPL_IS_OK(ret)) {
		cpl_db_backend = NULL;
		return ret;
	}


	// Initialize the locks

	cpl_lookup_or_create_object_semaphore
		= cpl_shared_semaphore_open(CPL_LOOKUP_OR_CREATE_SEM_INIT);
	if (cpl_lookup_or_create_object_semaphore == NULL) {
		ret = CPL_E_PLATFORM_ERROR;
		cpl_db_backend = NULL;
		return ret;
	}


	// Finish

	cpl_initialized = true;
	return CPL_OK;
}


/**
 * Perform the cleanup and detach the library from the database backend.
 * Please note that this function is not thread-safe.
 *
 * @return the error code
 */
extern "C" EXPORT cpl_return_t
cpl_detach(void)
{
	CPL_ENSURE_INITALIZED;
	cpl_initialized = false;

	cpl_drop_object_cache(true);

	cpl_db_backend->cpl_db_destroy(cpl_db_backend);
	cpl_db_backend = NULL;

	cpl_shared_semaphore_close(cpl_lookup_or_create_object_semaphore);
	cpl_lock_cleanup();

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
/** Public API: Disclosed Provenance API                                  **/
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


	// Get the container version

	cpl_version_t container_version = 0;
	if (container != CPL_NONE) {
		CPL_RUNTIME_VERIFY(cpl_get_version(container, &container_version));
	}


	// Call the backend

	cpl_id_t id;
	cpl_return_t ret;
	
	cpl_generate_unique_id(&id);
	ret = cpl_db_backend->cpl_db_create_object(cpl_db_backend,
											   id,
											   originator,
											   name,
											   type,
											   container,
											   container_version,
											   cpl_session);
	CPL_RUNTIME_VERIFY(ret);


	// Create an in-memory state

	if (cpl_cache) {
		cpl_open_object_t* obj = cpl_new_open_object(0);
		if (obj == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
		obj->last_session = cpl_session;
		obj->frozen = false;
		cpl_unlock(&obj->locked);

		cpl_lock(&cpl_open_objects_lock);
		assert(cpl_open_objects.find(id) == cpl_open_objects.end());
		cpl_open_objects[id] = obj;
		cpl_unlock(&cpl_open_objects_lock);
	}


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
 * Look up an object by name. If multiple objects share the same name,
 * return all of them.
 *
 * @param originator the object originator
 * @param name the object name
 * @param type the object type
 * @param flags a logical combination of CPL_L_* flags
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object_ext(const char* originator,
					  const char* name,
					  const char* type,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context)
{
	CPL_ENSURE_INITALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);
	CPL_ENSURE_NOT_NULL(iterator);


	// Call the backend

	cpl_return_t ret;
	ret = cpl_db_backend->cpl_db_lookup_object_ext(cpl_db_backend,
												   originator,
												   name,
												   type,
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
 * @param container the ID of the object that should contain this object
 *                  (use CPL_NONE for no container)
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_or_create_object(const char* originator,
							const char* name,
							const char* type,
							const cpl_id_t container,
							cpl_id_t* out_id)
{
	CPL_ENSURE_INITALIZED;
	int r = CPL_E_INTERNAL_ERROR;

	cpl_shared_semaphore_wait(cpl_lookup_or_create_object_semaphore);

	r = cpl_lookup_object(originator, name, type, out_id);
	if (r != CPL_E_NOT_FOUND) goto out;

	r = cpl_create_object(originator, name, type, container, out_id);
	if (CPL_IS_OK(r)) r = CPL_S_OBJECT_CREATED;

out:
	cpl_shared_semaphore_post(cpl_lookup_or_create_object_semaphore);
	return r;
}


/**
 * Disclose a data flow.
 *
 * @param data_dest the destination object
 * @param data_source the source object
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_data_flow(const cpl_id_t data_dest,
			  const cpl_id_t data_source,
			  const int type)
{
	return cpl_data_flow_ext(data_dest, data_source, CPL_VERSION_NONE, type);
}


/**
 * Disclose a data flow from a specific version of the data source.
 *
 * @param data_dest the destination object
 * @param data_source the source object
 * @param data_source_ver the version of the source object (where
 *                        CPL_VERSION_NONE = current)
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_data_flow_ext(const cpl_id_t data_dest,
				  const cpl_id_t data_source,
				  const cpl_version_t data_source_ver,
				  const int type)
{
	CPL_ENSURE_INITALIZED;


	// Check the arguments

	CPL_ENSURE_NOT_NONE(data_dest);
	CPL_ENSURE_NOT_NONE(data_source);
	
	if (CPL_GET_DEPENDENCY_CATEGORY(type) != CPL_DEPENDENCY_CATEGORY_DATA)
		return CPL_E_INVALID_ARGUMENT;


	// Add the dependency

	return cpl_add_dependency(data_dest, data_source, data_source_ver, type);
}


/**
 * Disclose a control flow operation.
 *
 * @param object_id the ID of the controlled object
 * @param controller the object ID of the controller
 * @param type the control dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_control_flow(const cpl_id_t object_id,
		         const cpl_id_t controller,
                 const int type)
{
	return cpl_control_flow_ext(object_id, controller, CPL_VERSION_NONE, type);
}


/**
 * Disclose a control flow operation using a specific version of the controller.
 *
 * @param object_id the ID of the controlled object
 * @param controller the object ID of the controller
 * @param controller_ver the version of the controller object (where
 *                       CPL_VERSION_NONE = current version)
 * @param type the control dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_control_flow_ext(const cpl_id_t object_id,
			         const cpl_id_t controller,
                     const cpl_version_t controller_ver,
                     const int type)
{
    CPL_ENSURE_INITALIZED;


	// Check the arguments

	CPL_ENSURE_NOT_NONE(object_id);
	CPL_ENSURE_NOT_NONE(controller);
	
	if (CPL_GET_DEPENDENCY_CATEGORY(type) != CPL_DEPENDENCY_CATEGORY_CONTROL)
		return CPL_E_INVALID_ARGUMENT;


	// Add the dependency

	return cpl_add_dependency(object_id, controller, controller_ver, type);
}


/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_property(const cpl_id_t id,
				 const char* key,
                 const char* value)
{
	CPL_ENSURE_INITALIZED;
    
    cpl_return_t ret;
    cpl_version_t version;


	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);


	// Freeze if necessary to make sure that the session information is correct

	ret = cpl_thaw(id, false, &version);
	if (!CPL_IS_OK(ret)) return ret;


    // Call the backend

	return cpl_db_backend->cpl_db_add_property(cpl_db_backend,
											   id,
                                               version,
											   key,
											   value);
}


/**
 * Create a new version of the given provenance object.
 *
 * @param id the object ID
 * @param new_version the new version number (can be NULL)
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_new_version(const cpl_id_t id,
				cpl_version_t* new_version)
{
	CPL_ENSURE_INITALIZED;
    
	return cpl_thaw(id, true, new_version);
}



/***************************************************************************/
/** Advanced Private API: Helpers for the Disclosed Provenance API        **/
/***************************************************************************/


/**
 * Add a dependency
 *
 * @param from_id the "from" end of the dependency edge
 * @param to_id the "to" end of the dependency edge
 * @param to_ver the version of the "to" end of the dependency edge
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
cpl_return_t
cpl_add_dependency(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const cpl_version_t to_ver,
				   const int type)
{
	CPL_ENSURE_INITALIZED;

	// TODO This should also account for different types of dependency edges,
	// so that edges of different types would not necessarily get eliminated


	// Check the arguments

	CPL_ENSURE_NOT_NONE(from_id);
	CPL_ENSURE_NOT_NONE(to_id);


	// Get the current version of the "to" object

	cpl_version_t to_current_version;
	CPL_RUNTIME_VERIFY(cpl_get_version(to_id, &to_current_version));


	// Set the version of the "to" object to which we would be pointing

	cpl_version_t to_version = to_ver;
	if (to_version == CPL_VERSION_NONE) {
		to_version = to_current_version;
	}
	else {
		if (to_version > to_current_version) {
			return CPL_E_INVALID_VERSION;
		}
	}


	// Cycle-Avoidance Algorithm

	// Determine whether the dependency, or a dependency that subsumes it,
	// already exists. Also, determine the version of the "from" object,
	// but only if the dependency does not already exist, or if this
	// information is already cached

	bool dependency_exists = false;
	cpl_version_t from_version = CPL_VERSION_NONE;
	cpl_open_object_t* obj_from = NULL;
	bool check_dependency_using_db = true;
	bool is_new = false;

	if (cpl_cache) {

		// Get the handle of the from object

		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(from_id, &obj_from, &is_new));


		// Get the version of the object and check to see if the entry is stale
		
		if (cpl_cache_check && !is_new) {
			cpl_return_t ret = cpl_db_backend->cpl_db_get_version(cpl_db_backend,
					from_id, &from_version);
			CPL_RUNTIME_VERIFY(ret);
		}
		else {
			from_version = obj_from->version;
		}


		// Check the ancestor list (if not stale)

		if (from_version == obj_from->version) {

			cpl_hash_map_id_to_version_t::iterator i;
			i = obj_from->ancestors.find(to_id);

			if (i == obj_from->ancestors.end()) {
				dependency_exists = false;
			}
			else {

                // The dependency exists if the version of the immediate
                // ancestor is equal to or greater than the version of the
                // ancestor that we are adding (to_version)

				dependency_exists = i->second >= to_version;
			}

			check_dependency_using_db = false;
		}
	}


	// Automatically unlock the object if it is still locked by the time
	// we hit end this block

	CPL_AutoUnlock __au_from(obj_from != NULL ? &obj_from->locked : NULL);
	(void) __au_from;


	// Check the dependency using the database if we need to

	if (check_dependency_using_db) {

		// Call the backend to determine the dependency

		int b;
		cpl_return_t r = cpl_db_backend->cpl_db_has_immediate_ancestor(
				cpl_db_backend, from_id, CPL_VERSION_NONE,
				to_id, to_version, &b);
		CPL_RUNTIME_VERIFY(r);


        // The dependency exists if the version of the immediate ancestor is
        // equal to or greater than the version of the ancestor that we are
        // adding (to_version)

		dependency_exists = b >= to_version;


		// If the dependency exists, get the object version

		if (!dependency_exists) {
			CPL_RUNTIME_VERIFY(cpl_get_version(from_id, &from_version));
		}
	}


	// Note: Due to the virtue of the cycle avoidance algorithm, we add a new
	// dependency edge only if we first create a new version. Consequently,
	// we do not need to check whether the provenance object is already froxen
	// or whether the last_session attribute matches the current session.
	// Another bizarre consequence is that the FREEZE operation is a no-op.


	// Return if the dependency already exists - there is nothing to do

	if (dependency_exists) return CPL_S_DUPLICATE_IGNORED;


	// Freeze and create a new version

	cpl_return_t r = CPL_E_ALREADY_EXISTS;
	from_version++;

	do {
		r = cpl_db_backend->cpl_db_create_version(cpl_db_backend,
												  from_id,
												  from_version,
												  cpl_session);
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
		obj_from->last_session = cpl_session;
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



/***************************************************************************/
/** Public API: Provenance Access API                                     **/
/***************************************************************************/


/**
 * Get a version of a provenance object.
 *
 * @param id the object ID
 * @param out_version the pointer to store the version of the object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_version(const cpl_id_t id,
				cpl_version_t* out_version)
{
	CPL_ENSURE_INITALIZED;
	cpl_version_t version;

	if (cpl_cache && !cpl_cache_check) {
		cpl_open_object_t* obj = NULL;
		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(id, &obj));
		version = obj->version;
		cpl_unlock(&obj->locked);
	}
	else {
		cpl_return_t ret;
		ret = cpl_db_backend->cpl_db_get_version(cpl_db_backend,
												 id, &version);
		CPL_RUNTIME_VERIFY(ret);
	}

	if (out_version != NULL) *out_version = version;
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
	CPL_ENSURE_INITALIZED;

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
	CPL_ENSURE_INITALIZED;
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
	CPL_ENSURE_INITALIZED;
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
	CPL_ENSURE_INITALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(out_info);


	// Get the latest version of the object, if available

	cpl_version_t version_hint = CPL_VERSION_NONE;
	if (cpl_cache && !cpl_cache_check) {
		cpl_open_object_t* obj = NULL;
		CPL_RUNTIME_VERIFY(cpl_get_open_object_handle(id, &obj));
		version_hint = obj->version;
		cpl_unlock(&obj->locked);
	}


	// Call the database backend

	return cpl_db_backend->cpl_db_get_object_info(cpl_db_backend, id,
												  version_hint, out_info);
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
	if (info->type != NULL) free(info->type);

	free(info);
	return CPL_OK;
}


/**
 * Get information about the specific version of a provenance object.
 *
 * @param id the object ID
 * @param version the version of the given provenance object
 * @param out_info the pointer to store the version info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_version_info(const cpl_id_t id,
					 const cpl_version_t version,
					 cpl_version_info_t** out_info)
{
	CPL_ENSURE_INITALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(out_info);

	return cpl_db_backend->cpl_db_get_version_info(cpl_db_backend, id,
												   version, out_info);
}


/**
 * Free cpl_version_info_t.
 *
 * @param info the pointer to the version info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_free_version_info(cpl_version_info_t* info)
{
	CPL_ENSURE_NOT_NULL(info);

	free(info);
	return CPL_OK;
}


/**
 * Iterate over the ancestors or the descendants of a provenance object.
 *
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
extern "C" EXPORT cpl_return_t
cpl_get_object_ancestry(const cpl_id_t id,
						const cpl_version_t version,
						const int direction,
						const int flags,
						cpl_ancestry_iterator_t iterator,
						void* context)
{
	CPL_ENSURE_INITALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	if (direction != CPL_D_ANCESTORS && direction != CPL_D_DESCENDANTS) {
		return CPL_E_INVALID_ARGUMENT;
	}


	// Validate the object version

	cpl_version_t current_version = CPL_VERSION_NONE;
	if (version != CPL_VERSION_NONE) {
		CPL_ENSURE_NOT_NEGATIVE(version);

		CPL_RUNTIME_VERIFY(cpl_get_version(id, &current_version));
		if (version < 0 || version > current_version) {
			return CPL_E_INVALID_VERSION;
		}
	}


	// Add the previous or the next version of the object to the result set
	
	int new_flags = flags | CPL_A_NO_PREV_NEXT_VERSION;
	bool has_version_dependency = false;
	if (version != CPL_VERSION_NONE && (flags&CPL_A_NO_PREV_NEXT_VERSION) == 0){
		if (version > 0 && direction == CPL_D_ANCESTORS) {
			has_version_dependency = true;
			CPL_RUNTIME_VERIFY(iterator(id, version, id, version - 1,
										CPL_VERSION_GENERIC, context));
		}
		else if (version < current_version && direction == CPL_D_DESCENDANTS) {
			has_version_dependency = true;
			CPL_RUNTIME_VERIFY(iterator(id, version, id, version + 1,
										CPL_VERSION_GENERIC, context));
		}
	}


	// Call the database backend

	cpl_return_t r;
	r = cpl_db_backend->cpl_db_get_object_ancestry(cpl_db_backend,
												   id, version, direction,
												   new_flags, iterator,
												   context);
	
	if (r == CPL_S_NO_DATA && has_version_dependency) return CPL_OK;
	return r;
}


/**
 * Get the properties associated with the given provenance object.
 *
 * @param id the the object ID
 * @param version the object version, or CPL_VERSION_NONE to access all
 *                version nodes associated with the given object
 * @param key the property to fetch - or NULL for all properties
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_properties(const cpl_id_t id,
				   const cpl_version_t version,
				   const char* key,
				   cpl_property_iterator_t iterator,
				   void* context)
{
	CPL_ENSURE_INITALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);


	// Validate the object version

	if (version != CPL_VERSION_NONE) {
		CPL_ENSURE_NOT_NEGATIVE(version);

		cpl_version_t current_version;
		CPL_RUNTIME_VERIFY(cpl_get_version(id, &current_version));

		if (version < 0 || version > current_version) {
			return CPL_E_INVALID_VERSION;
		}
	}


	// Call the database backend

	return cpl_db_backend->cpl_db_get_properties(cpl_db_backend,
												 id, version, key,
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
cpl_lookup_by_property(const char* key,
					   const char* value,
					   cpl_property_iterator_t iterator,
					   void* context)
{
	CPL_ENSURE_INITALIZED;
	CPL_ENSURE_NOT_NULL(iterator);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);


	// Call the database backend

	return cpl_db_backend->cpl_db_lookup_by_property(cpl_db_backend,
													 key, value,
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
	e.version = info->version;
	e.creation_session = info->creation_session;
    e.creation_time = info->creation_time;
    e.originator = info->originator;
    e.name = info->name;
    e.type = info->type;
    e.container_id = info->container_id;
    e.container_version = info->container_version;

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
 * @param version the object version
 * @param key the property name
 * @param value the property value
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
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
 * The iterator callback for cpl_get_object_ancestry() that collects
 * the passed-in information in an instance of std::list<cpl_ancestry_entry_t>.
 *
 * @param query_object_id the ID of the object on which we are querying
 * @param query_object_verson the version of the queried object
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param other_object_version the version of the other object
 * @param type the type of the data or the control dependency
 * @param context the pointer to an instance of the list
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_cb_collect_ancestry_list(const cpl_id_t query_object_id,
							 const cpl_version_t query_object_version,
							 const cpl_id_t other_object_id,
							 const cpl_version_t other_object_version,
							 const int type,
							 void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_ancestry_entry_t e;
	e.query_object_id = query_object_id;
	e.query_object_version = query_object_version;
	e.other_object_id = other_object_id;
	e.other_object_version = other_object_version;
	e.type = type;

	std::list<cpl_ancestry_entry_t>& l =
		*((std::list<cpl_ancestry_entry_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_object_ancestry() that collects
 * the information in an instance of std::vector<cpl_ancestry_entry_t>.
 *
 * @param query_object_id the ID of the object on which we are querying
 * @param query_object_verson the version of the queried object
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param other_object_version the version of the other object
 * @param type the type of the data or the control dependency
 * @param context the pointer to an instance of the vector
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_cb_collect_ancestry_vector(const cpl_id_t query_object_id,
							   const cpl_version_t query_object_version,
							   const cpl_id_t other_object_id,
							   const cpl_version_t other_object_version,
							   const int type,
							   void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_ancestry_entry_t e;
	e.query_object_id = query_object_id;
	e.query_object_version = query_object_version;
	e.other_object_id = other_object_id;
	e.other_object_version = other_object_version;
	e.type = type;

	std::vector<cpl_ancestry_entry_t>& l =
		*((std::vector<cpl_ancestry_entry_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_properties() that collects the returned
 * information in an instance of std::vector<cplxx_property_entry_t>.
 *
 * @param id the object ID
 * @param version the object version
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
								 const cpl_version_t version,
								 const char* key,
								 const char* value,
								 void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cplxx_property_entry_t e;
	e.id = id;
	e.version = version;
	e.key = key;
	e.value = value;

	std::vector<cplxx_property_entry_t>& l =
		*((std::vector<cplxx_property_entry_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_lookup_by_property() that collects
 * the returned information in an instance of std::vector<cpl_id_version_t>.
 *
 * @param id the object ID
 * @param version the object version
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
									  const cpl_version_t version,
									  const char* key,
									  const char* value,
									  void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_id_version_t e;
	e.id = id;
	e.version = version;

	std::vector<cpl_id_version_t>& l =
		*((std::vector<cpl_id_version_t>*) context);
	l.push_back(e);

	return CPL_OK;
}



#endif /* __cplusplus */

