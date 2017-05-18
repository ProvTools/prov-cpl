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

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif



/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 * TODO is this ok
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

//TODO Add some ancestry locks

/**
 * The database backend
 */
static cpl_db_backend_t* cpl_db_backend = NULL;

/**
 * The current session ID
 */
static cpl_session_t cpl_session = CPL_NONE;


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

#ifdef __APPLE__

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

		snprintf(mac_string, 32, "%02x:%02x:%02x:%02x:%02x:%02x",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	}
	else {
		mac_string_ptr = NULL;
	}

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
	CPL_ENSURE_INITIALIZED;
	cpl_initialized = false;

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
 *                  (use CPL_NONE for no bundle)
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
	CPL_ENSURE_NOT_NULL(type);


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
 * @param type the object type
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object(const char* originator,
				  const char* name,
				  const int type,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;


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
					  const int type,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context)
{
	CPL_ENSURE_INITIALIZED;


	// Argument check

	CPL_ENSURE_NOT_NULL(originator);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(type);
	CPL_ENSURE_NOT_NULL(iterator);


	// Call the backend

	//TODO mess with flags

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

	r = cpl_lookup_object(originator, name, type, out_id);
	if (r != CPL_E_NOT_FOUND) goto out;

	r = cpl_create_object(originator, name, type, bundle, out_id);
	if (CPL_IS_OK(r)) r = CPL_S_OBJECT_CREATED;

out:
	cpl_shared_semaphore_post(cpl_lookup_or_create_object_semaphore);
	return r;
}


/**
 * Add a property to the given object.
 * TODO names
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
 * Add a dependency
 * TODO modify
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

	//TODO add verification + caching


	cpl_id_t id;
	cpl_return_t ret;

	ret = cpl_db_backend->cpl_db_add_relation(cpl_db_backend,
													from_id,
													to_id,
													type,
													bundle,
													&id);


	CPL_RUNTIME_VERIFY(ret);

	//TODO verify type

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
	if (info->type != NULL) free(info->type);

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
 * TODO change name
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
 * TODO change name
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


extern "C" EXPORT cpl_return_t
cpl_delete_bundle(const cpl_id_t id)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);

	return cpl_db_backend->cpl_db_delete_bundle(cpl_db_backend, id);
}


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
 * The iterator callback for cpl_get_object_ancestry() that collects
 * the passed-in information in an instance of std::list<cpl_ancestry_entry_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param type the type of the data or the control dependency
 * @param context the pointer to an instance of the list
 * @return CPL_OK or an error code
 */
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
 * the information in an instance of std::vector<cpl_ancestry_entry_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param type the type of the relation
 * @param context the pointer to an instance of the vector
 * @return CPL_OK or an error code
 */
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

//TODO make sure relations go to and from correct types
int
validation_helper_json(json_t* document,
					 const char* type_str,
					 const char* source_str,
					 const char* dest_str,
					 std::vector<std::string> &objects,
					 igraph_vector_t* edges)
{
	json_t* relations;
	int source_int, dest_int;

	relations = json_object_get(document, type_str);
	json_object_foreach(relations, name, value){

		std::string source(json_object_get(relations, source_str));
		std::string dest(json_object_get(relations, dest_str));

		if(!source){
			json_decref(relations);
			return -1;
		}

		if(dest){
			if (int pos = std::find(objects.begin(), objects.end(), source) != objects.end()){
				source_int = pos;
			} else {
				objects.push_back(source);
				source_int = objects.size()-1;
			}

			if (int pos = std::find(objects.begin(), objects.end(), dest) != objects.end()){
				dest_int = pos;
			} else {
				objects.push_back(dest);
				dest_int = objects.size()-1;
			}

			igraph_vector_push_back(edges, source_int);
			igraph_vector_push_back(edges, dest_int);
		}
	}

	json_decref(relations);
	return 0;
}

#ifdef SWIG
%constant
#endif
int
validate_json(const char* path,
	 		  const char** out_msg)
{
	
	*out_msg = "Validation failed on upload";
	json_error_t err;
	json_t* document = json_load_file(filename, 0, &err);
	if(document == NULL){
		json_decref(document);
		return -1;
	}

	std::vector<std::string> objects;

	igraph_vector_t edges;
	igraph_vector_init(&edges, 0);

	*out_msg = "Invalid PROV-JSON formatting";

	if(validation_helper_json(document, ALTERNATEOF_STR,
						 	  "prov:alternate1", "prov:alternate2",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, DERIVEDBYINSERTIONFROM_STR,
						 	  "prov:after", "prov:before",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, DERIVEDBYREMOVALFROM_STR,
						 	  "prov:after", "prov:before",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, HADMEMBER_STR,
						 	  "prov:collection", "prov:entity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, HADDICTIONARYMEMBER_STR,
						 	  "prov:dictionary", "prov:entity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, SPECIALIZATIONOF_STR,
						 	  "prov:specificEntity", "prov:generalEntity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASDERIVEDFROM_STR,
						 	  "prov:generatedEntity", "prov:usedEntity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASGENERATEDBY_STR,
						 	  "prov:entity", "prov:activity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASINVALIDATEDBY_STR,
						 	  "prov:entity", "prov:activity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASATTRIBUTEDTO_STR,
						 	  "prov:entity", "prov:agent",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, USED_STR,
						 	  "prov:activity", "prov:entity",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASINFORMEDBY_STR,
						 	  "prov:informed", "prov:informant",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASSTARTEDBY_STR,
						 	  "prov:activity", "prov:trigger",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASENDEDBY_STR,
						 	  "prov:activity", "prov:trigger",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, HADPLAN_STR,
						 	  "prov:agent", "prov:plan",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASASSOCIATEDWITH_STR,
						 	  "prov:activity", "prov:agent",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, ACTEDONBEHALFOF_STR,
						 	  "prov:delegate", "prov:responsible",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}
	if(validation_helper_json(document, WASINFLUENCEDBY_STR,
						 	  "prov:influencee", "prov:influencer",
						 	  objects, &edges)){
		json_decref(document);
		return -1;
	}

	igraph_t graph;
	igraph_empty(&graph, objects.size(), IGRAPH_DIRECTED);
	igraph_add_edges(&graph, &edges, 0);

	igraph_bool_t is_dag;
	igraph_is_dag(&graph, &is_dag);

	if(!is_dag){
		*out_msg = "PROV-JSON document contains cycles";
		json_decref(document);
		return -1
	}

	*out_msg = "Valid PROV-JSON";
	json_decref(document);
	return 0;
}

cpl_return_t
import_bundle_prefixes_json(cpl_id_t bundle,
							json_t* document)
{

	json_t* prefixes = json_object_get(document, "prefix");

	json_object_foreach(prefixes, key, value){

		const char* key_str = json_string_value(key);
		const char* val_str = json_string_value(value);

		if(key_str && key_val){
			if(!CPL_IS_OK(cpl_add_object_property(bundle, key_str, val_str))){
				json_decref(prefixes);
				return CPL_E_INTERNAL_ERROR;
			}
		}
	}

	json_decref(prefixes);
	return CPL_OK;
}

cpl_return_t
import_objects_json(const int type,
					const char* originator,
					cpl_id_t bundle_id,
					json_t* document)
{

	json_t* objects = json_object_get(document, type);

	json_object_foreach(objects, name, properties){

		const char* name_str = json_string_value(name);
		cpl_id_t obj_id = NULL;

		switch(type)
		{
			case ENTITY:
				if(!CPL_IS_OK(cpl_create_object(originator, name_str, ENTITY, bundle_id, &obj_id))){
					json_decref(objects);
					return CPL_E_INTERNAL_ERROR;
				}
			case AGENT:
				if(!CPL_IS_OK(cpl_create_object(originator, name_str, AGENT, bundle_id, &obj_id))){
					json_decref(objects);
					return CPL_E_INTERNAL_ERROR;
				}
			case ACTIVITY:
				if(!CPL_IS_OK(cpl_create_object(originator, name_str, ACTIVITY, bundle_id, &obj_id))){
					json_decref(objects);
					return CPL_E_INTERNAL_ERROR;
				}
		}

		if(obj_id){
			json_object_foreach(properties, pkey, pval){

				const char* pkey_str = json_string_value(pkey);
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
	return CPL_IS_OK;
}

cpl_return_t
import_helper_json(const char* originator,
										 json_t* relation,
										 const char* slabel,
										 int stype,
										 const char* dlabel,
										 int dtype,
										 int rtype,
										 cpl_id_t bundle_id)
{	
	cpl_id_t source, dest, relation_id;

	if(!CPL_IS_OK(cpl_lookup_object(originator, 
									json_string_value(json_object_get(relation, slabel)),
									stype,
									&source))){
		return CPL_E_INTERNAL_ERROR;
	}
	if(!CPL_IS_OK(cpl_lookup_object(originator, 
									json_string_value(json_object_get(relation, dlabel)),
									dtype,
									&dest))){
		return CPL_E_INTERNAL_ERROR;
	}

	if(!CPL_IS_OK(cpl_add_relation(source, dest, rtype, bundle_id, &relation_id))){
		return CPL_E_INTERNAL_ERROR;
	}

	json_object_foreach(relation, key, value){

		const char* key_str = json_string_value(key);
		const char* val_str = json_string_value(val);

		if(key_str && key_val){
			if(!CPL_IS_OK(cpl_add_relation_property(relation_id, key_str, key_val))){
				return CPL_E_INTERNAL_ERROR;
			}
		}
	}

	return CPL_IS_OK;
}	

cpl_return_t
import_relations_json(const char* type_str, 
					  const char* originator,
					  cpl_id_t bundle_id,
					  json_t* document)
{

	json_t* relations = json_object_get(document, type_str);

	json_object_foreach(relations, name, relation){

		cpl_id_t source, dest, relation_id;

		switch(type_str)
		{
			case WASINFLUENCEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:influencee", NULL,
												 "prov:influencer", NULL,
												 WASINFLUENCEDBY))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case ALTERNATEOF_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:alternate1", ENTITY,
												 "prov:alternate2", ENTITY,
												 ALTERNATEOF))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case DERIVEDBYINSERTIONFROM_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:after", ENTITY,
												 "prov:before", ENTITY,
												 DERIVEDBYINSERTIONFROM))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case DERIVEDBYREMOVALFROM_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:after", ENTITY,
												 "prov:before", ENTITY,
												 DERIVEDBYREMOVALFROM))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case HADMEMBER_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:collection", ENTITY,
												 "prov:before", ENTITY,
												 HADMEMBER))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case HADDICTIONARYMEMBER_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:dictionary", ENTITY,
												 "prov:entity", ENTITY,
												 HADDICTIONARYMEMBER))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case SPECIALIZATIONOF_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:specificEntity", ENTITY,
												 "prov:generalEntity", ENTITY,
												 SPECIALIZATIONOF))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASDERIVEDFROM_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:generatedEntity", ENTITY,
												 "prov:usedEntity", ENTITY,
												 WASDERIVEDFROM))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASGENERATEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:entity", ENTITY,
												 "prov:activity", ACTIVITY,
												 WASGENERATEDBY))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASINVALIDATEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:entity", ENTITY,
												 "prov:activity", ACTIVITY,
												 WASINVALIDATEDBY))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASATTRIBUTEDTO_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:entity", ENTITY,
												 "prov:agent", AGENT,
												 WASATTRIBUTEDTO))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case USED_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:activity", ACTIVITY,
												 "prov:entity", ENTITY,
												 USED))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASINFORMEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:informed", ACTIVITY,
												 "prov:informant", ACTIVITY,
												 WASINFORMEDBY))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASSTARTEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:activity", ACTIVITY,
												 "prov:trigger", ENTITY,
												 WASSTARTEDBY))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASENDEDBY_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:activity", ACTIVITY,
												 "prov:trigger", ENTITY,
												 rtype))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case HADPLAN_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:agent", AGENT,
												 "prov:plan", ENTITY,
												 HADPLAN))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case WASASSOCIATEDWITH_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:activity", ACTIVITY,
												 "prov:agent", AGENT,
												 WASASSOCIATEDWITH))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}
			case ACTEDONBEHALFOF_STR:
				if(!CPL_IS_OK(import_helper_json(originator,
												 relation, bundle_id
												 "prov:delegate", AGENT,
												 "prov:responsible", AGENT,
												 rtype))){
					json_decref(relations);
					return CPL_E_INTERNAL_ERROR;
				}

		}
	}

	json_decref(relations);
	return CPL_OK;
}

#ifdef SWIG
%constant
#endif
cpl_return_t
import_document_json(const char* filename,
					 const char* originator,
					 const char* bundle_name,
					 cpl_id_t anchor_object,
					 cpl_id_t bundle_agent)
{

	json_error_t err;
	json_t* document = json_load_file(filename, 0, &err);
	if(document == NULL){
		return CPL_E_INTERNAL_ERROR;
	}

	cpl_id_t bundle_id;
	if(!CPL_IS_OK(cpl_create_object(originator, bundle_name, BUNDLE, NULL, &bundle_id))){
		goto error;
	}

	if(!CPL_IS_OK(import_bundle_prefixes_json(bundle_id, document))){
		goto error;
	}

	if(bundle_agent){
		if(!CPL_IS_OK(cpl_add_relation(bundle_id, bundle_agent, 
						WASATTRIBUTEDTO, bundle_id, NULL))){
			goto error;
		}
	}

	if(!CPL_IS_OK(import_objects_json(ENTITY, originator, bundle_id, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(AGENT, originator, bundle_id, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(ACTIVITY, originator, bundle_id, document))){
		goto error;
	}

	if(anchor_object){
		cpl_object_info_t* anchor_info;
		cpl_get_object_info(anchor_object, &anchor_info);
		cpl_id_t dest;
		if(CPL_IS_OK(cpl_lookup_object(originator, 
						anchor_info->name, anchor_info->type, &dest))){

			cpl_free_object_info(anchor_info);
			if(dest){
`				if(!CPL_IS_OK(cpl_add_relation(anchor_object, dest,
								ALTERNATEOF, bundle_id))){
					goto error;
				}
			}
		}
	}

	if(!CPL_IS_OK(import_relations_json(WASINFLUENCEDBY_STR, WASINFLUENCEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(ALTERNATEOF_STR, ALTERNATEOF))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(DERIVEDBYINSERTIONFROM_STR, DERIVEDBYINSERTIONFROM))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(DERIVEDBYREMOVALFROM_STR, DERIVEDBYREMOVALFROM))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(HADMEMBER_STR, HADMEMBER))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(HADDICTIONARYMEMBER_STR, HADDICTIONARYMEMBER))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(SPECIALIZATIONOF_STR, SPECIALIZATIONOF))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASDERIVEDFROM_STR, WASDERIVEDFROM))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASGENERATEDBY_STR, WASGENERATEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASINVALIDATEDBY_STR, WASINVALIDATEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASATTRIBUTEDTO_STR, WASATTRIBUTEDTO))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(USED_STR, USED))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASINFORMEDBY_STR, WASINFORMEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASSTARTEDBY_STR, WASSTARTEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASENDEDBY_STR, WASENDEDBY))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(HADPLAN_STR, HADPLAN))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(WASASSOCIATEDWITH_STR, WASASSOCIATEDWITH))){
		goto error;
	}
	if(!CPL_IS_OK(import_relations_json(ACTEDONBEHALFOF_STR, ACTEDONBEHALFOF))){
		goto error;
	}

	json_decref(alternateOf);
	json_decref(derivedByInsertionFrom);
	json_decref(derivedByRemovalFrom);
	json_decref(hadMember);
	json_decref(hadDictionaryMember);
	json_decref(specializationOf);
	json_decref(wasDerivedFrom);
	json_decref(wasGeneratedBy);
	json_decref(wasInvalidatedBy);
	json_decref(wasAttributedTo);
	json_decref(used);
	json_decref(wasInformedBy);
	json_decref(wasStartedBy);
	json_decref(wasEndedBy);
	json_decref(hadPlan);
	json_decref(wasAssociatedWith);
	json_decref(actedOnBehalfOf);
	json_decref(wasInfluencedBy);
	json_decref(document);

	return CPL_OK;
error:
	json_decref(alternateOf);
	json_decref(derivedByInsertionFrom);
	json_decref(derivedByRemovalFrom);
	json_decref(hadMember);
	json_decref(hadDictionaryMember);
	json_decref(specializationOf);
	json_decref(wasDerivedFrom);
	json_decref(wasGeneratedBy);
	json_decref(wasInvalidatedBy);
	json_decref(wasAttributedTo);
	json_decref(used);
	json_decref(wasInformedBy);
	json_decref(wasStartedBy);
	json_decref(wasEndedBy);
	json_decref(hadPlan);
	json_decref(wasAssociatedWith);
	json_decref(actedOnBehalfOf);
	json_decref(wasInfluencedBy);
	json_decref(document);

	cpl_delete_bundle(bundle_id);

	return CPL_E_INTERNAL_ERROR;
}




int
export_bundle_prefixes_json(cpl_id_t bundle, 
							json_t* document)
{

	std::vector<cplxx_property_entry_t> prefix_vec;
	cpl_get_object_properties(bundle, NULL, cpl_cb_collect_properties_vector, &prefix_vec);

	if (prefix_vec.size()){

		json_t* prefixes = json_object();

		for(auto & prefix: prefix_vec){
			if(json_object_set_new(prefixes, prefix->key, json_string(prefix->value))){
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

int
export_objects_json(cpl_id_t bundle, 
					json_t* document)
{

	std::vector<cplxx_object_info_vector> object_vec;
	cpl_get_bundle_objects(bundle, cpl_cb_collect_object_info_vector, object_vec);

	if(object_vec.size()){

		json_t* entities = json_object();
		json_t* activities = json_object();
		json_t* agents = json_object();

		std::vector<cplxx_property_entry_t> property_vec;

		for(auto & obj: object_vec){
			json_t* properties = json_object();
			cpl_get_object_properties(obj->id, NULL, 
				cpl_cb_collect_properties_vector, &property_vec);

			for(auto & property: property_vec){
				if(json_object_set_new(properties, property->key, json_string(property->value))){
					json_decref(properties);
					goto error;
				}
			}

			switch(obj->type){
				case ENTITY: 
					if(json_object_set(entities, obj->name, properties)){
						goto error;
					}
					break;
				case ACTIVITY:
					if(json_object_set(activities, obj->name, properties)){
						goto error;
					}
					break;
				case AGENT:
					if(json_object_set(agents, obj->name, properties)){
						goto error;
					}
					break;
				default:
					break;
			}

			json_decref(properties);
		}

		if(json_object_size(entities)){
			if(json_object_set(document, ENTITY_STR, entities)){
				goto error;
			}
		}
		if(json_object_size(activities)){
			if(json_object_set(document, ACTIVITY_STR, activities)){
				goto error;
			}
		}
		if(json_object_size(agents)){
			if(json_object_set(document, AGENT_STR, agents)){
				goto error;
			}
		}
	}

	json_decref(entities);
	json_decref(activities);
	json_decref(agents);
	return 0;

error:
	json_decref(entities);
	json_decref(activities);
	json_decref(agents);
	return -1;

}

int
export_relations_json(cpl_id_t bundle,
				      json_t* document)
{

	std::vector<cpl_relation_t> relation_vec;
	cpl_get_bundle_relations(bundle, cpl_cb_collect_relation_vector, relation_vec);

	if(relation_vec.size())
	{
		json_t* alternate_of = json_object();
		json_t* derived_by_insertion_from = json_object();
		json_t* derived_by_removal_from = json_object();
		json_t* had_member = json_object();
		json_t* had_dictionary_member = json_object();
		json_t* specialization_of = json_object();
		json_t* was_derived_from = json_object();
		json_t* was_generated_by = json_object();
		json_t* was_invalidated_by = json_object();
		json_t* was_attributed_to = json_object();
		json_t* used = json_object();
		json_t* was_informed_by = json_object();
		json_t* was_started_by = json_object();
		json_t* was_ended_by = json_object();
		json_t* had_plan = json_object();
		json_t* was_associated_with = json_object();
		json_t* acted_on_behalf_of = json_object();
		json_t* was_influenced_by = json_object();

		std::vector<cplxx_property_entry_t> property_vec;

		for(auto & relation: relation_vec){

			json_t* properties = json_object();
			cpl_get_relation_properties(relation->id, NULL, 
				cpl_cb_collect_properties_vector, property_vec);
			for(auto & property: property_vec){
				if(json_object_set_new(properties, property->name, json_string(property->value))){
					goto error;
				}
			}

			switch(relation->type)
			{
				case ALTERNATEOF:
					if(json_object_set(alternate_of, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case DERIVEDBYINSERTIONFROM:
					if(json_object_set(derived_by_insertion_from, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case DERIVEDBYREMOVALFROM:
					if(json_object_set(derived_by_removal_from, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case HADMEMBER:
					if(json_object_set(had_member, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case HADDICTIONARYMEMBER:
					if(json_object_set(had_dictionary_member, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case SPECIALIZATIONOF:
					if(json_object_set(specialization_of, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASDERIVEDFROM:
					if(json_object_set(was_derived_from, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASGENERATEDBY:
					if(json_object_set(was_generated_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASINVALIDATEDBY:
					if(json_object_set(was_invalidated_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASATTRIBUTEDTO:
					if(json_object_set(was_attributed_to, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case USED:
					if(json_object_set(used, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASINFORMEDBY:
					if(json_object_set(was_informed_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASSTARTEDBY:
					if(json_object_set(was_started_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASENDEDBY:
					if(json_object_set(was_ended_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case HADPLAN:
					if(json_object_set(had_plan, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASASSOCIATEDWITH:
					if(json_object_set(was_associated_with, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case ACTEDONBEHALFOF:
					if(json_object_set(acted_on_behalf_of, to_string(relation->id), properties)){
						goto error;
					}
					break;
				case WASINFLUENCEDBY:
					if(json_object_set(was_influenced_by, to_string(relation->id), properties)){
						goto error;
					}
					break;
				default:
					break;
			}
		}

		if(json_object_size(alternate_of)){
			if(json_object_set(document, ALTERNATEOF_STR, alternate_of)){
				goto error;
			}
		}
		if(json_object_size(derived_by_insertion_from)){
			if(json_object_set(document, DERIVEDBYINSERTIONFROM_STR, derived_by_insertion_from)){
				goto error;
			}
		}
		if(json_object_size(derived_by_removal_from)){
			if(json_object_set(document, DERIVEDBYREMOVALFROM_STR, derived_by_removal_from)){
				goto error;
			}
		}
		if(json_object_size(had_member)){
			if(json_object_set(document, HADMEMBER_STR, had_member)){
				goto error;
			}
		}
		if(json_object_size(had_dictionary_member)){
			if(json_object_set(document, HADDICTIONARYMEMBER_STR, had_dictionary_member)){
				goto error;
			}
		}
		if(json_object_size(specialization_of)){
			if(json_object_set(document, SPECIALIZATIONOF_STR, specialization_of)){
				goto error;
			}
		}
		if(json_object_size(was_derived_from)){
			if(json_object_set(document, WASDERIVEDFROM_STR, was_derived_from)){
				goto error;
			}
		}
		if(json_object_size(was_generated_by)){
			if(json_object_set(document, WASGENERATEDBY_STR, was_generated_by)){
				goto error;
			}
		}
		if(json_object_size(was_invalidated_by)){
			if(json_object_set(document, WASINVALIDATEDBY_STR, was_invalidated_by)){
				goto error;
			}
		}
		if(json_object_size(was_attributed_to)){
			if(json_object_set(document, WASATTRIBUTEDTO_STR, was_attributed_to)){
				goto error;
			}
		}
		if(json_object_size(used)){
			if(json_object_set(document, USED_STR, used)){
				goto error;
			}
		}
		if(json_object_size(was_informed_by)){
			if(json_object_set(document, WASINFORMEDBY_STR, was_informed_by)){
				goto error;
			}
		}
		if(json_object_size(was_started_by)){
			if(json_object_set(document, WASSTARTEDBY_STR, was_started_by)){
				goto error;
			}
		}
		if(json_object_size(was_ended_by)){
			if(json_object_set(document, WASENDEDBY_STR, was_ended_by)){
				goto error;
			}
		}
		if(json_object_size(had_plan)){
			if(json_object_set(document, HADPLAN_STR, had_plan)){
				goto error;
			}
		}
		if(json_object_size(was_associated_with)){
			if(json_object_set(document, WASASSOCIATEDWITH_STR, was_associated_with)){
				goto error;
			}
		}
		if(json_object_size(acted_on_behalf_of)){
			if(json_object_set(document, ACTEDONBEHALFOF_STR, acted_on_behalf_of)){
				goto error;
			}
		}
		if(json_object_size(was_influenced_by)){
			if(json_object_set(document, WASINFLUENCEDBY_STR, was_influenced_by)){
				goto error;
			}
		}

		json_decref(alternate_of);
		json_decref(derived_by_insertion_from);
		json_decref(derived_by_removal_from);
		json_decref(had_member);
		json_decref(had_dictionary_member);
		json_decref(specialization_of);
		json_decref(was_derived_from);
		json_decref(was_generated_by);
		json_decref(was_invalidated_by);
		json_decref(was_attributed_to);
		json_decref(used);
		json_decref(was_informed_by);
		json_decref(was_started_by);
		json_decref(was_ended_by);
		json_decref(had_plan);
		json_decref(was_associated_with);
		json_decref(acted_on_behalf_of);
		json_decref(was_influenced_by);
	}

	return 0;

error:

	json_decref(alternate_of);
	json_decref(derived_by_insertion_from);
	json_decref(derived_by_removal_from);
	json_decref(had_member);
	json_decref(had_dictionary_member);
	json_decref(specialization_of);
	json_decref(was_derived_from);
	json_decref(was_generated_by);
	json_decref(was_invalidated_by);
	json_decref(was_attributed_to);
	json_decref(used);
	json_decref(was_informed_by);
	json_decref(was_started_by);
	json_decref(was_ended_by);
	json_decref(had_plan);
	json_decref(was_associated_with);
	json_decref(acted_on_behalf_of);
	json_decref(was_influenced_by);

	return -1;
}

#ifdef SWIG
%constant
#endif
cpl_return_t
export_bundle_json(cpl_id_t bundle, 
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
