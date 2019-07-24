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
	/*
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
	*/

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
 * @param prefix the namespace prefix
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_object(const char* prefix,
				  const char* name,
				  const int type,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;

	// Argument check

	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_O_TYPE(type);

	// Call the backend

	cpl_id_t id;
	cpl_return_t ret;

	ret = cpl_db_backend->cpl_db_create_object(cpl_db_backend,
											   prefix,
											   name,
											   type,
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
 * @param prefix the namespace prefix
 * @param name the object name
 * @param type the object type, CPL_NONE for no type
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object(const char* prefix,
				  const char* name,
				  const int type,
				  cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;

	// Argument check

	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(name);


	// Call the backend

	cpl_return_t ret;
	cpl_id_t id;
	
	ret = cpl_db_backend->cpl_db_lookup_object(cpl_db_backend,
											   prefix,
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
 * @param prefix the object prefix
 * @param name the object name
 * @param type the object type, CPL_NONE for no type
 * @param flags a logical combination of CPL_L_* flags
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object_ext(const char* prefix,
					  const char* name,
					  const int type,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context)
{
	CPL_ENSURE_INITIALIZED;

	// Argument check

	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(name);
	CPL_ENSURE_NOT_NULL(iterator);


	// Call the backend

	//TODO mess with flags

	cpl_return_t ret;
	ret = cpl_db_backend->cpl_db_lookup_object_ext(cpl_db_backend,
												   prefix,
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
 * @param prefix the namespace prefix
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_or_create_object(const char* prefix,
							const char* name,
							const int type,
							cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;

	int r = CPL_E_INTERNAL_ERROR;

	//TODO think about locks here
	cpl_shared_semaphore_wait(cpl_lookup_or_create_object_semaphore);

	r = cpl_lookup_object(prefix, name, type, out_id);
	if (r != CPL_E_NOT_FOUND) goto out;

	r = cpl_create_object(prefix, name, type, out_id);
	if (CPL_IS_OK(r)) r = CPL_S_OBJECT_CREATED;

out:
	cpl_shared_semaphore_post(cpl_lookup_or_create_object_semaphore);
	return r;
}


/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_object_string_property(const cpl_id_t id,
				 const char* prefix,
				 const char* key,
                 const char* value)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(key);
    CPL_ENSURE_NOT_NULL(value);

    // Call the backend
	return cpl_db_backend->cpl_db_add_object_property(cpl_db_backend,
                                                       id,
                                                       prefix,
                                                       key,
                                                       value,
                                                       STRINGPROPERTY);
}

/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_object_numerical_property(const cpl_id_t id,
                        const char* prefix,
                        const char* key,
                        const double value)
{
    CPL_ENSURE_INITIALIZED;

    // Check the arguments
    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the backend
    return cpl_db_backend->cpl_db_add_object_property(cpl_db_backend,
                                                      id,
                                                      prefix,
                                                      key,
                                                      std::to_string(value).c_str(),
                                                      NUMERICALPROPERTY);
}

/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_object_boolean_property(const cpl_id_t id,
                        const char* prefix,
                        const char* key,
                        const bool value)
{
    CPL_ENSURE_INITIALIZED;

    // Check the arguments
    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the backend
    return cpl_db_backend->cpl_db_add_object_property(cpl_db_backend,
                                                      id,
                                                      prefix,
                                                      key,
                                                      std::to_string(value).c_str(),
                                                      BOOLEANPROPERTY);
}

/**
 * Add a relation between two objects.
 *
 * @param from_id the "from" end of the relation
 * @param to_id the "to" end of the relation
 * @param type the relation type
 * @param bundle the relation bundle
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const int type,
				   cpl_id_t* out_id)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments

	CPL_ENSURE_NOT_NONE(from_id);
	CPL_ENSURE_NOT_NONE(to_id);
	CPL_ENSURE_R_TYPE(type);


	cpl_id_t id;
	cpl_return_t ret;

	ret = cpl_db_backend->cpl_db_add_relation(cpl_db_backend,
													from_id,
													to_id,
													type,
													&id);


	CPL_RUNTIME_VERIFY(ret);

	// Finish

	if (out_id != NULL) *out_id = id;
	return CPL_OK;
}


/**
 * Add a property to the given relation.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation_string_property(const cpl_id_t id,
				 		  const char* prefix,
						  const char* key,
						  const char* value)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);

	// Call the backend

	return cpl_db_backend->cpl_db_add_relation_property(cpl_db_backend,
														id,
														prefix,
														key,
														value,
														STRINGPROPERTY);
};

/**
 * Add a property to the given relation.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation_numerical_property(const cpl_id_t id,
                                  const char* prefix,
                                  const char* key,
                                  const double value)
{
    CPL_ENSURE_INITIALIZED;

    // Check the arguments

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the backend

    return cpl_db_backend->cpl_db_add_relation_property(cpl_db_backend,
                                                        id,
                                                        prefix,
                                                        key,
                                                        std::to_string(value).c_str(),
                                                        NUMERICALPROPERTY);
};

/**
 * Add a property to the given relation.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_relation_boolean_property(const cpl_id_t id,
                                  const char* prefix,
                                  const char* key,
                                  const bool value)
{
    CPL_ENSURE_INITIALIZED;

    // Check the arguments

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the backend

    return cpl_db_backend->cpl_db_add_relation_property(cpl_db_backend,
                                                        id,
                                                        prefix,
                                                        key,
                                                        std::to_string(value).c_str(),
                                                        BOOLEANPROPERTY);
};

/**
 * Create a bundle.
 *
 * @param name the object name
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_create_bundle(const char* name,
				  const char* prefix,
				  cpl_id_t* out_id)
{
	return cpl_create_object(prefix, name, CPL_BUNDLE, out_id);
}

/**
 * Look up a bundle by name. If multiple bundles share the same name,
 * get the latest one.
 *
 * @param name the bundle name
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_bundle(const char* name,
				  const char* prefix,
				  cpl_id_t* out_id)
{
	return cpl_lookup_object(prefix, name, CPL_BUNDLE, out_id);
}


/**
 * Look up a bundle by name. If multiple bundles share the same name,
 * return all of them.
 *
 * @param name the bundle name
 * @param flags a logical combination of CPL_L_* flags
 * @param iterator the iterator to be called for each matching bundle
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_bundle_ext(const char* name,
					  const char* prefix,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context)
{
	return cpl_lookup_object_ext(prefix, name, CPL_BUNDLE, flags, iterator, context);
}

/**
 * Look up a relation by from_id, to_id and type.
 * If multiple relations match, get the latest one.
 *
 * @param from_id object id of source
 * @param to_id object id of destination
 * @param type the type of the relation
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_relation(const cpl_id_t from_id,
                    const cpl_id_t to_id,
                    const long type,
                    cpl_id_t* out_id)
{
    CPL_ENSURE_INITIALIZED;
    CPL_ENSURE_NOT_NONE(from_id);
    CPL_ENSURE_NOT_NONE(to_id);
    CPL_ENSURE_R_TYPE(type);

    // Call the database backend
    return cpl_db_backend->cpl_db_lookup_relation(cpl_db_backend,
                                                            from_id, to_id, type,
                                                            out_id);
}

/**
 * Lookup object property by the value, with wildcards
 *
 * @param fragment the value of the object property
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_lookup_object_property_wildcard(const char* value,
                    cpl_id_t* out_id)
{
    CPL_ENSURE_INITIALIZED;

    // Call the database backend
    return cpl_db_backend->cpl_db_lookup_object_property_wildcard(cpl_db_backend, value, out_id);
}

/**
 * Add a prefix to a bundle.
 *
 * @param prefix the namespace prefix
 * @param iri the namespace iri
 * @param value the value
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_add_prefix(const cpl_id_t id,
	 		   const char* prefix,
			   const char* iri)
{
	CPL_ENSURE_INITIALIZED;

	// Check the arguments

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(iri);

	// Call the backend

	return cpl_db_backend->cpl_db_add_prefix(cpl_db_backend,
											 id,
											 prefix,
											 iri);
};

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
cpl_get_all_objects(const char* prefix,
                    const int flags,
					cpl_object_info_iterator_t iterator,
					void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NULL(iterator);

	return cpl_db_backend->cpl_db_get_all_objects(cpl_db_backend, prefix, flags,
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

	if (info->prefix != NULL) free(info->prefix);
	if (info->name != NULL) free(info->name);

	free(info);
	return CPL_OK;
}


/**
 * Iterate over the relations of a provenance object.
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
 * @param prefix the prefix to fetch - or NULL for all prefixes
 * @param key the property key to fetch - or NULL for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_object_string_properties(const cpl_id_t id,
						  const char* prefix,
						  const char* key,
						  cpl_property_iterator_t iterator,
						  void* context)
{
	CPL_ENSURE_INITIALIZED;

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	// Call the database backend

	return cpl_db_backend->cpl_db_get_object_properties(cpl_db_backend,
											    id, prefix, key, STRINGPROPERTY,
											    iterator, context);
}

/**
 * Get the properties associated with the given provenance object.
 *
 * @param id the the object ID
 * @param prefix the prefix to fetch - or NULL for all prefixes
 * @param key the property key to fetch - or NULL for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_object_numerical_properties(const cpl_id_t id,
                          const char* prefix,
                          const char* key,
                          cpl_property_iterator_t iterator,
                          void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(iterator);

    // Call the database backend

    return cpl_db_backend->cpl_db_get_object_properties(cpl_db_backend,
                                                        id, prefix, key, NUMERICALPROPERTY,
                                                        iterator, context);
}

/**
 * Get the properties associated with the given provenance object.
 *
 * @param id the the object ID
 * @param prefix the prefix to fetch - or NULL for all prefixes
 * @param key the property key to fetch - or NULL for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_object_boolean_properties(const cpl_id_t id,
                          const char* prefix,
                          const char* key,
                          cpl_property_iterator_t iterator,
                          void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(iterator);

    // Call the database backend

    return cpl_db_backend->cpl_db_get_object_properties(cpl_db_backend,
                                                        id, prefix, key, BOOLEANPROPERTY,
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
cpl_lookup_object_by_string_property(const char* prefix,
							  const char* key,
						      const char* value,
						      cpl_property_iterator_t iterator,
						      void* context)
{
	CPL_ENSURE_INITIALIZED;

	CPL_ENSURE_NOT_NULL(iterator);
	CPL_ENSURE_NOT_NULL(prefix);
	CPL_ENSURE_NOT_NULL(key);
	CPL_ENSURE_NOT_NULL(value);

	// Call the database backend
	return cpl_db_backend->cpl_db_lookup_object_by_property(cpl_db_backend,
											        prefix, key, value,
											        STRINGPROPERTY,
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
cpl_lookup_object_by_numerical_property(const char* prefix,
                              const char* key,
                              const double value,
                              cpl_property_iterator_t iterator,
                              void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NULL(iterator);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the database backend
    return cpl_db_backend->cpl_db_lookup_object_by_property(cpl_db_backend,
                                                            prefix, key,
                                                            std::to_string(value).c_str(),
                                                            NUMERICALPROPERTY,
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
cpl_lookup_object_by_boolean_property(const char* prefix,
                              const char* key,
                              const bool value,
                              cpl_property_iterator_t iterator,
                              void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NULL(iterator);
    CPL_ENSURE_NOT_NULL(prefix);
    CPL_ENSURE_NOT_NULL(key);

    // Call the database backend
    return cpl_db_backend->cpl_db_lookup_object_by_property(cpl_db_backend,
                                                            prefix, key,
                                                            std::to_string(value).c_str(),
                                                            BOOLEANPROPERTY,
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
cpl_get_relation_string_properties(const cpl_id_t id,
						    const char* prefix,
						    const char* key,
						    cpl_property_iterator_t iterator,
						    void* context)
{
	CPL_ENSURE_INITIALIZED;

	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	// Call the database backend

	return cpl_db_backend->cpl_db_get_relation_properties(cpl_db_backend,
										          id, prefix, key, STRINGPROPERTY,
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
cpl_get_relation_numerical_properties(const cpl_id_t id,
                            const char* prefix,
                            const char* key,
                            cpl_property_iterator_t iterator,
                            void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(iterator);

    // Call the database backend

    return cpl_db_backend->cpl_db_get_relation_properties(cpl_db_backend,
                                                          id, prefix, key, NUMERICALPROPERTY,
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
cpl_get_relation_boolean_properties(const cpl_id_t id,
                            const char* prefix,
                            const char* key,
                            cpl_property_iterator_t iterator,
                            void* context)
{
    CPL_ENSURE_INITIALIZED;

    CPL_ENSURE_NOT_NONE(id);
    CPL_ENSURE_NOT_NULL(iterator);

    // Call the database backend

    return cpl_db_backend->cpl_db_get_relation_properties(cpl_db_backend,
                                                          id, prefix, key, BOOLEANPROPERTY,
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
 * Get information about the given provenance bundle.
 *
 * @param id the bundle ID
 * @param out_info the pointer to store the bundle info structure
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_bundle_info(const cpl_id_t id,
					cpl_bundle_info_t** out_info)
{
	return cpl_get_object_info(id, (cpl_object_info_t**) out_info);

}


/**
 * Free cpl_bundle_info_t.
 *
 * @param info the pointer to the bundle info structure.
 * @return CPL_OK or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_free_bundle_info(cpl_bundle_info_t* info)
{
	CPL_ENSURE_NOT_NULL(info);

	if (info->name != NULL) free(info->name);

	free(info);
	return CPL_OK;
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

/**
 * Get the prefixes associated with the given provenance bundle.
 *
 * @param id the the bundle ID
 * @param prefix the property prefix to fetch - or NULL for all prefixes
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
extern "C" EXPORT cpl_return_t
cpl_get_prefixes(const cpl_id_t id,
			     const char* prefix,
			     cpl_prefix_iterator_t iterator,
			     void* context)
{
	CPL_ENSURE_INITIALIZED;
	CPL_ENSURE_NOT_NONE(id);
	CPL_ENSURE_NOT_NULL(iterator);

	// Call the database backend

	return cpl_db_backend->cpl_db_get_prefixes(cpl_db_backend,
										       id, prefix,
										       iterator, context);
}
/***************************************************************************/
/** Public API: Enhanced C++ Functionality                                **/
/***************************************************************************/

#ifdef __cplusplus

/**
 * The iterator callback for cpl_get_all_objects() that collects the returned
 * information in an instance of vector<cplxx_object_info_t>.
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
    e.creation_time = info->creation_time;
    e.prefix = info->prefix;
    e.name = info->name;
    e.type = info->type;

	std::vector<cplxx_object_info_t>& l =
		*((std::vector<cplxx_object_info_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_lookup_object_ext() that collects the returned
 * information in an instance of vector<cpl_id_timestamp_t>.
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
 * The iterator callback
 *for cpl_get_object_relations() that collects
 * the passed-in information in an instance of list<cpl_relation_t>.
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
 * the information in an instance of vector<cpl_relation_t>.
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
							   void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cpl_relation_t e;
	e.id = relation_id;
	e.query_object_id = query_object_id;
	e.other_object_id = other_object_id;
	e.type = type;

	std::vector<cpl_relation_t>& l =
		*((std::vector<cpl_relation_t>*) context);
	l.push_back(e);

	return CPL_OK;
}


/**
 * The iterator callback for cpl_get_properties() that collects the returned
 * information in an instance of vector<cplxx_property_entry_t>.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
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
	                             const char* prefix,
								 const char* key,
								 const char* value,
								 const int type,
								 void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	switch (type) {
	    case STRINGPROPERTY: {
            cplxx_string_property_entry_t e;
            e.id = id;
            e.prefix = prefix;
            e.key = key;
            e.value = value;

            std::vector<cplxx_string_property_entry_t> &l =
                    *((std::vector<cplxx_string_property_entry_t> *) context);
            l.push_back(e);
            break;
        }
	    case NUMERICALPROPERTY:  {
            cplxx_numerical_property_entry_t e;
            e.id = id;
            e.prefix = prefix;
            e.key = key;
            e.value = std::stod(value);

            std::vector<cplxx_numerical_property_entry_t> &l =
                    *((std::vector<cplxx_numerical_property_entry_t> *) context);
            l.push_back(e);
            break;
        }
	    case BOOLEANPROPERTY:  {
            cplxx_boolean_property_entry_t e;
            e.id = id;
            e.prefix = prefix;
            e.key = key;
            e.value = (strcmp(value, "1") == 0);

            std::vector<cplxx_boolean_property_entry_t> &l =
                    *((std::vector<cplxx_boolean_property_entry_t> *) context);
            l.push_back(e);
            break;
        }
        default:
            break;
	}

	return CPL_OK;
}

/**
 * The iterator callback for cpl_get_prefixes() that collects the returned
 * information in an instance of vector<cplxx_prefix_entry_t>.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param iri the namespace iri
 * @param context the pointer to an instance of the vector 
 * @return CPL_OK or an error code
 */
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_prefixes_vector(const cpl_id_t id,
                               const char* prefix,
						       const char* iri,
						       void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	cplxx_prefix_entry_t e;
	e.id = id;
	e.prefix = prefix;
	e.iri = iri;

	std::vector<cplxx_prefix_entry_t>& l =
		*((std::vector<cplxx_prefix_entry_t>*) context);
	l.push_back(e);

	return CPL_OK;
}

/**
 * The iterator callback for cpl_lookup_by_property() that collects
 * the returned information in an instance of vector<cpl_id_t>.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
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
	                                  const char* prefix,
									  const char* key,
									  const char* value,
									  const int type,
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
	{WASINFLUENCEDBY, WASINFLUENCEDBY_STR,
	"prov:influencee", CPL_NONE,
	"prov:influencer", CPL_NONE},
	{ALTERNATEOF, ALTERNATEOF_STR,
	"prov:alternate1", CPL_ENTITY,
	"prov:alternate2", CPL_ENTITY},
	{DERIVEDBYINSERTIONFROM, DERIVEDBYINSERTIONFROM_STR,
	"prov:after", CPL_ENTITY,
	"prov:before", CPL_ENTITY},
	{DERIVEDBYREMOVALFROM, DERIVEDBYREMOVALFROM_STR,
	"prov:after", CPL_ENTITY,
	"prov:before", CPL_ENTITY},
	{HADMEMBER, HADMEMBER_STR,
	"prov:collection", CPL_ENTITY,
	"prov:before", CPL_ENTITY},
	{HADDICTIONARYMEMBER, HADDICTIONARYMEMBER_STR,
	"prov:dictionary", CPL_ENTITY,
	"prov:entity", CPL_ENTITY},
	{SPECIALIZATIONOF, SPECIALIZATIONOF_STR,
	"prov:specificEntity", CPL_ENTITY,
	"prov:generalEntity", CPL_ENTITY},
	{WASDERIVEDFROM, WASDERIVEDFROM_STR,
	"prov:generatedEntity", CPL_ENTITY,
	"prov:usedEntity", CPL_ENTITY},
	{WASGENERATEDBY, WASGENERATEDBY_STR,
	"prov:entity", CPL_ENTITY,
	"prov:activity", CPL_ACTIVITY},
	{WASINVALIDATEDBY, WASINVALIDATEDBY_STR,
	"prov:entity", CPL_ENTITY,
	"prov:activity", CPL_ACTIVITY},
	{WASATTRIBUTEDTO, WASATTRIBUTEDTO_STR,
	"prov:entity", CPL_ENTITY,
	"prov:agent", CPL_AGENT},
	{USED, USED_STR,
	"prov:activity", CPL_ACTIVITY,
	"prov:entity", CPL_ENTITY},
	{WASINFORMEDBY, WASINFORMEDBY_STR,
	"prov:informed", CPL_ACTIVITY,
	"prov:informant", CPL_ACTIVITY},
	{WASSTARTEDBY, WASSTARTEDBY_STR,
	"prov:activity", CPL_ACTIVITY,
	"prov:trigger", CPL_ENTITY},
	{WASENDEDBY, WASENDEDBY_STR,
	"prov:activity", CPL_ACTIVITY,
	"prov:trigger", CPL_ENTITY},
	{HADPLAN, HADPLAN_STR,
	"prov:agent", CPL_AGENT,
	"prov:plan", CPL_ENTITY},
	{WASASSOCIATEDWITH, WASASSOCIATEDWITH_STR,
	"prov:activity", CPL_ACTIVITY,
	"prov:agent", CPL_AGENT},
	{ACTEDONBEHALFOF, ACTEDONBEHALFOF_STR,
	"prov:delegate", CPL_AGENT,
	"prov:responsible", CPL_AGENT},
	{INBUNDLE, INBUNDLE_STR,
      "prov:bundle", CPL_BUNDLE,
      "prov:object", CPL_ENTITY},
      {INBUNDLE, INBUNDLE_STR,
      "prov:bundle", CPL_BUNDLE,
      "prov:object", CPL_AGENT},
      {INBUNDLE, INBUNDLE_STR,
      "prov:bundle", CPL_BUNDLE,
      "prov:object", CPL_ACTIVITY}
};

using json = nlohmann::json;

/*
 * Verifies the correctness of a Prov-JSON document. Not currently exhaustive.
 * Free *string_out after use.
 * 
 * @param json_string the JSON document as a string
 * @param string_out error output string
 * @return CPL_OK on successful validation or CPL_E_INVALID_JSON on failure
 */

//TODO add edge type checking
EXPORT cpl_return_t
validate_json(const std::string& json_string,
	 		  std::string& string_out)
{
	string_out = "Validation failed on upload \n";
	
	json document = json::parse(json_string);

	if(document == NULL || document.empty()){
		return CPL_E_INTERNAL_ERROR;
	}

	if(!document.is_object()) return CPL_E_INVALID_JSON;


	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> directed_graph_t;
  	typedef boost::graph_traits<directed_graph_t>::vertex_descriptor vertex_t;
	typedef std::pair<int, int> edge_t;

	std::vector<std::string> objects;
	std::vector<edge_t> edges;

	string_out = "Invalid JSON formatting \n";

	for(int i=0; i<CPL_NUM_R_TYPES; i++){
		prov_relation_data_t entry = rdata_array[i];
		auto doc_check = document.find(entry.type_str);

		if(doc_check != document.end()){

			json relations = *doc_check;
			for (json::iterator it = relations.begin(); it != relations.end(); ++it){

			 	auto val = it.value();
				auto source_check = val.find(entry.source_str);

				auto dest_check = val.find(entry.dest_str);
				if(source_check == val.end()) return CPL_E_INVALID_JSON;

				if(dest_check != val.end()){
					std::string source = *source_check;
					std::string dest = *dest_check;

					//TODO 
					size_t source_ind, dest_ind;
					auto pos = std::find(objects.begin(), objects.end(), source);
					if (pos != objects.end()){
						source_ind = distance(objects.begin(), pos);
					} else {
						objects.push_back(source);
						source_ind = objects.size()-1;
					}
					pos = std::find(objects.begin(), objects.end(), dest);
					if (pos != objects.end()){
						dest_ind = distance(objects.begin(), pos);
					} else {
						objects.push_back(dest);
						dest_ind = objects.size()-1;
					}

					edges.push_back(edge_t(source_ind, dest_ind));
				}
			}
		}
	}

	directed_graph_t g(edges.begin(), edges.end(), objects.size());

	std::vector<vertex_t> c;

	try{
		 boost::topological_sort(g, back_inserter(c));
	} catch (const boost::not_a_dag& e){
		string_out = "Provenance graph is not a DAG \n";
		return CPL_E_INVALID_JSON;
	}

	string_out = "Valid JSON \n";
	return CPL_OK;
}

typedef std::pair<std::string, std::string> token_pair_t;

token_pair_t
name_to_tokens(const std::string& name_string){
		std::string tok1, tok2;
		std::stringstream full_name(name_string);
		std::getline(full_name, tok1, ':');
		std::getline(full_name, tok2);
		if(tok2.empty()){
			tok2 = tok1;
			tok1 = "";
		}
		return token_pair_t(tok1, tok2);
}

/*
 * Imports prefixes. import_document_json helper function.
 */
cpl_return_t
import_bundle_prefixes_json(const cpl_id_t bundle,
							json document)
{
	auto doc_check = document.find("prefix");
	if(doc_check == document.end()) return CPL_OK;
	
	json prefixes = *doc_check;
	if(!prefixes.is_object()) return CPL_E_INVALID_JSON;

	json val_json; 
	for (json::iterator it = prefixes.begin(); it != prefixes.end(); ++it) {
		val_json = *it;
		if(val_json.is_null() || val_json.is_object() || val_json.is_array()) return CPL_E_INVALID_JSON;
		if(!CPL_IS_OK(cpl_add_prefix(bundle, it.key().c_str(), val_json.get<std::string>().c_str()))){
			return CPL_E_INTERNAL_ERROR;
		}
	}

	return CPL_OK;
}


/*
 * Imports objects. import_document_json helper function.
 */
cpl_return_t
import_objects_json(const int type,
					const std::string& type_str,
					std::map<std::string, cpl_id_t>& lookup_tbl,
					json& document)
{
	auto doc_check = document.find(type_str);
	if(doc_check == document.end()) return CPL_OK;

	json o = *doc_check;
	if(!o.is_object()) return CPL_E_INVALID_JSON;

	for (json::iterator it = o.begin(); it != o.end(); ++it) {

		cpl_id_t obj_id = CPL_NONE;

		token_pair_t pair = name_to_tokens(it.key());

		if(!CPL_IS_OK(cpl_create_object(pair.first.c_str(), pair.second.c_str(), type, &obj_id))){
			return CPL_E_INTERNAL_ERROR;
		}

		lookup_tbl.emplace(it.key(), obj_id);

		json properties = it.value();
		for (json::iterator it2 = properties.begin(); it2 != properties.end(); ++it2){

			pair = name_to_tokens(it2.key());

			json val_json = *it2;
			switch (val_json.type()) {
                case json::value_t::string:
                    cpl_add_object_string_property(obj_id,
                                                   pair.first.c_str(),
                                                   pair.second.c_str(),
                                                   val_json.get<std::string>().c_str());
                    break;
                case json::value_t::number_integer:
                case json::value_t::number_unsigned:
                case json::value_t::number_float:
                    cpl_add_object_numerical_property(obj_id,
                                                  pair.first.c_str(),
                                                  pair.second.c_str(),
                                                  val_json.get<float>());
                    break;
                case json::value_t::boolean:
                    cpl_add_object_boolean_property(obj_id,
                                                    pair.first.c_str(),
                                                    pair.second.c_str(),
                                                    val_json.get<bool>());
                    break;
			    default:
                    return CPL_E_INVALID_ARGUMENT;
			}
		}
	}

	return CPL_OK;
}

/*
 * Imports relations. import_document_json helper function.
 */
cpl_return_t
import_relations_json(const cpl_id_t bundle_id,
					  const std::map<std::string, cpl_id_t>& lookup_tbl,
					  json& document,
					  const int extern_obj_f)
{
	for(int i=0; i<CPL_NUM_R_TYPES; i++){
		prov_relation_data_t entry = rdata_array[i];
		
		auto doc_check = document.find(entry.type_str);
		
		if(doc_check != document.end()){

			json relations = *doc_check;
			if(!relations.is_object()) return CPL_E_INVALID_JSON;

			for (json::iterator it = relations.begin(); it != relations.end(); ++it) {

				cpl_id_t source, dest, relation_id;
				token_pair_t pair;
				auto obj_check = it.value().find(entry.source_str);
				if(obj_check == it.value().end()) return CPL_E_INVALID_JSON;

				std::string obj_name = *obj_check;
				auto tbl_it = lookup_tbl.find(obj_name);

				if( tbl_it != lookup_tbl.end()){
					source = tbl_it->second;
				} else if(extern_obj_f){
					pair = name_to_tokens(obj_name);

					if(!CPL_IS_OK(cpl_lookup_object(pair.first.c_str(), 
													pair.second.c_str(),
													entry.source_t,
													&source))){
						return CPL_E_INTERNAL_ERROR;
					}				
				} else {
					return CPL_E_INTERNAL_ERROR;
				}

				obj_check = it.value().find(entry.dest_str);
				if(obj_check == it.value().end()) return CPL_E_INVALID_JSON;

				obj_name = *obj_check;

				tbl_it = lookup_tbl.find(obj_name);

				if( tbl_it != lookup_tbl.end()){
					dest = tbl_it->second;
				} else if(extern_obj_f){
					pair = name_to_tokens(obj_name);

					if(!CPL_IS_OK(cpl_lookup_object(pair.first.c_str(), 
													pair.second.c_str(),
													entry.dest_t,
													&dest))){
						return CPL_E_INTERNAL_ERROR;
					}				
				} else {
					return CPL_E_INTERNAL_ERROR;
				}

				if(!CPL_IS_OK(cpl_add_relation(source, dest, entry.type, &relation_id))){
					return CPL_E_INTERNAL_ERROR;
				}
                if(!CPL_IS_OK(cpl_add_relation(bundle_id, relation_id, BUNDLERELATION, NULL))){
                    return CPL_E_INTERNAL_ERROR;
                }

				json properties = it.value();
				for (json::iterator it2 = properties.begin(); it2 != properties.end(); ++it2){

					if(it.key() != entry.source_str && it.key() != entry.dest_str){

						pair = name_to_tokens(it2.key());

						json val_json = *it2;
                        switch (val_json.type()) {
                            case json::value_t::string:
                                cpl_add_relation_string_property(relation_id,
                                                               pair.first.c_str(),
                                                               pair.second.c_str(),
                                                               val_json.get<std::string>().c_str());
                                break;
                            case json::value_t::number_integer:
                            case json::value_t::number_unsigned:
                            case json::value_t::number_float:
                                cpl_add_relation_numerical_property(relation_id,
                                                                  pair.first.c_str(),
                                                                  pair.second.c_str(),
                                                                  val_json.get<float>());
                                break;
                            case json::value_t::boolean:
                                cpl_add_relation_boolean_property(relation_id,
                                                                pair.first.c_str(),
                                                                pair.second.c_str(),
                                                                val_json.get<bool>());
                                break;
                            default:
                                return CPL_E_INVALID_ARGUMENT;
                        }
					}
				}
			}
		}
	}

	return CPL_OK;
}

/*
 * Imports a Prov-JSON document into Prov-CPL.
 *
 * @param json_string the JSON document as a string
 * @param bundle_name desired name of document bundle
 * @param anchor_object optional PROV-CPL object identical to an object in the document
 * @param flags a logical combination of CPL_J_* flags
 * @param out_id the ID of the imported bundle
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
import_document_json(const std::string& json_string,
					 const std::string& bundle_name,
					 const std::vector<std::pair<cpl_id_t, std::string>>& anchor_objects,
					 const int flags,
					 cpl_id_t* out_id)
{
	json document = json::parse(json_string);
	int extern_obj_f = (flags && CPL_J_EXTERN_OBJ) ? 1 : 0;

	if(document == NULL || document.empty()){
		return CPL_E_INTERNAL_ERROR;
	}

	if(!document.is_object()) return CPL_E_INVALID_JSON;

 	std::map<std::string, cpl_id_t> lookup_tbl;

	// Create bundle
	cpl_id_t bundle_id;
	if(!CPL_IS_OK(cpl_create_bundle(bundle_name.c_str(), "", &bundle_id))){
		goto error;
	}

	if(!CPL_IS_OK(import_bundle_prefixes_json(bundle_id, document))){
		goto error;
	}
	// Import objects
	if(!CPL_IS_OK(import_objects_json(CPL_ENTITY, CPL_ENTITY_STR, lookup_tbl, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(CPL_AGENT, CPL_AGENT_STR, lookup_tbl, document))){
		goto error;
	}
	if(!CPL_IS_OK(import_objects_json(CPL_ACTIVITY, CPL_ACTIVITY_STR, lookup_tbl, document))){
		goto error;
	}

	// Connect anchor objects
	for (auto & id_string: anchor_objects){
		
		token_pair_t pair = name_to_tokens(id_string.second);

		cpl_id_t source;
		if(CPL_IS_OK(cpl_lookup_object(pair.first.c_str(), pair.second.c_str(), CPL_NONE, &source))){
            cpl_id_t relation_id;
			if(!CPL_IS_OK(cpl_add_relation(source, id_string.first,
						ALTERNATEOF, &relation_id))){
				goto error;		
			}
            if(!CPL_IS_OK(cpl_add_relation(bundle_id, relation_id,
                                           BUNDLERELATION, NULL))){
                goto error;
            }
		}
	}

	if(!CPL_IS_OK(import_relations_json(bundle_id, lookup_tbl, document, extern_obj_f))){
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
cpl_return_t
export_bundle_prefixes_json(const std::vector<cpl_id_t>& bundles,
							json& document)
{
	if(bundles.size() == 1){

		cpl_return_t ret;
		cpl_id_t bundle = bundles.at(0);

		std::vector<cplxx_prefix_entry_t> prefix_vec;
		if(!CPL_IS_OK(ret = cpl_get_prefixes(bundle, NULL, cpl_cb_collect_prefixes_vector, &prefix_vec)))
				return ret;

		json prefixes;
		if (!prefix_vec.empty()){
			for(auto & entry: prefix_vec){

				prefixes[entry.prefix] = entry.iri;
			}

			document["prefix"] = prefixes;
		}

	} else {

		/**
		typedef map<pair<string, string>, vector<cpl_id_t>> prefix_id_map;
		prefix_id_map map;

		for (auto & bundle: bundles){

			vector<cplxx_prefix_entry_t> prefix_vec;
			cpl_get_prefixes(bundle, NULL, cpl_cb_collect_prefixes_vector, &prefix_vec);

			for(auto & entry: prefix_vec){
				pair<string, string> key(entry.prefix, entry.iri);
				prefix_id_map::iterator vec_it = map.find(key);
				if(vec_it == map.end()){
					vector<cpl_id_t> id_vec;
					id_vec.push_back(bundle);
					map.emplace(pair<string, string>(entry.prefix, entry.iri), id_vec);
				} else {
					vec_it->push_back(bundle);
				}
			}
		}

		json document_prefixes;
		map<cpl_id_t, json> bundle_prefixes;

		for(auto & entry: prefix_id_map){
			if(entry->second.size() == 1){
				document_prefixes[entry->first.first] = entry->first.second;
			} else {
				for( auto & id: entry->second){
					map<cpl_id_t, json>::iterator vec_it = bundle_prefixes.find(cpl_id_t);
					if(vec_it == map.end()){
						json p[entry->second.first] = entry->second.second;
						map.emplace(cpl_id_t, p);
					} else {
						vec_it->second[entry->second.first] = entry->second.second;
					}
				}
			}
		}

		//throw prefixes into appropriate bundles
		**/
		return CPL_E_INVALID_ARGUMENT;
	}

	return CPL_OK;
}

/*
 * Retrieves bundle objects. export_bundle_json helper function.
 */
cpl_return_t
export_objects_json(const std::vector<cpl_id_t>& bundles, 
					boost::unordered_map<cpl_id_t, std::string>& lookup_tbl,
					json& document)
{
	if(bundles.size() == 1){

		cpl_return_t ret;
		cpl_id_t bundle = bundles.at(0);

		std::vector<cplxx_object_info_t> object_vec;
		if(!CPL_IS_OK(ret = cpl_get_bundle_objects(bundle, cpl_cb_collect_object_info_vector, &object_vec)))
			return ret;

		if(object_vec.empty()){
			return CPL_OK;
		}

		std::vector<cplxx_string_property_entry_t> string_property_vec;
        std::vector<cplxx_numerical_property_entry_t> numerical_property_vec;
        std::vector<cplxx_boolean_property_entry_t> boolean_property_vec;

		for(auto & obj: object_vec){

			json properties;
			if(!CPL_IS_OK(ret = cpl_get_object_string_properties(obj.id, NULL, NULL,
				cpl_cb_collect_properties_vector, &string_property_vec))) return ret;
            if(!CPL_IS_OK(ret = cpl_get_object_numerical_properties(obj.id, NULL, NULL,
                cpl_cb_collect_properties_vector, &numerical_property_vec))) return ret;
            if(!CPL_IS_OK(ret = cpl_get_object_boolean_properties(obj.id, NULL, NULL,
                cpl_cb_collect_properties_vector, &boolean_property_vec))) return ret;

			for(auto & property: string_property_vec){
				std::string full_prop_name(property.prefix);
				if(full_prop_name != "") full_prop_name.append(":");
				full_prop_name.append(property.key);

				properties[full_prop_name] = property.value;
			}

            for(auto & property: numerical_property_vec){
                std::string full_prop_name(property.prefix);
                if(full_prop_name != "") full_prop_name.append(":");
                full_prop_name.append(property.key);

                properties[full_prop_name] = property.value;
            }

            for(auto & property: boolean_property_vec){
                std::string full_prop_name(property.prefix);
                if(full_prop_name != "") full_prop_name.append(":");
                full_prop_name.append(property.key);

                properties[full_prop_name] = property.value;
            }

            string_property_vec.clear();
            numerical_property_vec.clear();
            boolean_property_vec.clear();

			std::string full_obj_name(obj.prefix);
			if(full_obj_name != "") full_obj_name.append(":");
			full_obj_name.append(obj.name);

			switch(obj.type){
				case CPL_ENTITY: 
					document["entity"][full_obj_name] = properties;
					break;
				case CPL_AGENT:
					document["agent"][full_obj_name] = properties;
					break;
				case CPL_ACTIVITY:
					document["activity"][full_obj_name] = properties;
					break;
			}

			lookup_tbl.emplace(obj.id, full_obj_name);
		}

	} else {
		return CPL_E_INVALID_ARGUMENT;
	}

	return CPL_OK;
}

/*
 * Retrieves bundle relations. export_bundle_json helper function.
 */
cpl_return_t
export_relations_json(const std::vector<cpl_id_t>& bundles,
					  const boost::unordered_map<cpl_id_t, std::string>& lookup_tbl,
				      json& document)
{	
	if(bundles.size() == 1){
		cpl_return_t ret;
		cpl_id_t bundle = bundles.at(0);

		std::vector<cpl_relation_t> relation_vec;

		if(!CPL_IS_OK(ret = cpl_get_bundle_relations(bundle, cpl_cb_collect_relation_vector, &relation_vec))) {
		    return ret;
		}


		if(relation_vec.empty()) {
			return CPL_OK;
		}

        std::vector<cplxx_string_property_entry_t> string_property_vec;
        std::vector<cplxx_numerical_property_entry_t> numerical_property_vec;
        std::vector<cplxx_boolean_property_entry_t> boolean_property_vec;

		for(auto & relation: relation_vec){

			json properties;
			if(!CPL_IS_OK(ret = cpl_get_relation_string_properties(relation.id, NULL, NULL,
				cpl_cb_collect_properties_vector, &string_property_vec))) return ret;
            if(!CPL_IS_OK(ret = cpl_get_relation_numerical_properties(relation.id, NULL, NULL,
                cpl_cb_collect_properties_vector, &numerical_property_vec))) return ret;
            if(!CPL_IS_OK(ret = cpl_get_relation_boolean_properties(relation.id, NULL, NULL,
                cpl_cb_collect_properties_vector, &boolean_property_vec))) return ret;

            for(auto & property: string_property_vec){
                std::string full_prop_name(property.prefix);
                if(full_prop_name != "") full_prop_name.append(":");
                full_prop_name.append(property.key);

                properties[full_prop_name] = property.value;
            }

            for(auto & property: numerical_property_vec){
                std::string full_prop_name(property.prefix);
                if(full_prop_name != "") full_prop_name.append(":");
                full_prop_name.append(property.key);

                properties[full_prop_name] = property.value;
            }

            for(auto & property: boolean_property_vec){
                std::string full_prop_name(property.prefix);
                if(full_prop_name != "") full_prop_name.append(":");
                full_prop_name.append(property.key);

                properties[full_prop_name] = property.value;
            }

            string_property_vec.clear();
            numerical_property_vec.clear();
            boolean_property_vec.clear();

			cpl_object_info *from_info, *to_info;
			bool is_from_info = false;
			bool is_to_info = false;

			auto tbl_it = lookup_tbl.find(relation.query_object_id);

			if(tbl_it != lookup_tbl.end()){
				properties[rdata_array[relation.type-1].source_str] = tbl_it->second;
			} else if(!CPL_IS_OK(ret = cpl_get_object_info(relation.query_object_id, &from_info))){
				return ret;
			} else {
				properties[rdata_array[relation.type-1].source_str] = from_info->name;
				is_from_info = true;
			}

			tbl_it = lookup_tbl.find(relation.other_object_id);

			if(tbl_it != lookup_tbl.end()){
				properties[rdata_array[relation.type-1].dest_str] = tbl_it->second;
			} else if(!CPL_IS_OK(ret = cpl_get_object_info(relation.other_object_id, &to_info))){
				return ret;
			} else {
				properties[rdata_array[relation.type-1].dest_str] = to_info->name;
				is_from_info = true;
			}

			if(is_from_info){
				if(!CPL_IS_OK(ret = cpl_free_object_info(from_info))) return ret;
			}

			if(is_to_info){
				if(!CPL_IS_OK(ret = cpl_free_object_info(to_info))) return ret;
			}

			is_from_info = false;
			is_to_info = false;

			document[rdata_array[relation.type-1].type_str][std::to_string(relation.id)] = properties;
		}

	} else {
		return CPL_E_INVALID_ARGUMENT;
	}

	return CPL_OK;
}

/*
 * Exports a Prov-CPL bundle as a Prov-JSON document.
 *
 * @param bundles vector of bundle IDs, currently only single bundle supported
 * @param json_string the bundle as a string in JSON format
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
export_bundle_json(const std::vector<cpl_id_t>& bundles, 
				   std::string& json_string)
{

	json document;
	cpl_return_t ret;

	std::ofstream outputFile("/Users/jpokuhn/Desktop/test-output.txt");

	outputFile << "entered function" << "\n";

 	boost::unordered_map<cpl_id_t, std::string> lookup_tbl;

//	if(!CPL_IS_OK(ret = export_bundle_prefixes_json(bundles, document))){
//		return ret;
//	}
    for (int i = 0; i<bundles.size(); i++) {
        const std::vector<cpl_id_t>& sub_bundles = {bundles.at(i)};
        if(!CPL_IS_OK(ret = export_objects_json(sub_bundles, lookup_tbl, document))){
		    return ret;
	    }
	    if(!CPL_IS_OK(ret = export_relations_json(sub_bundles, lookup_tbl, document))){
		    return ret;
	    }
    }


	json_string = document.dump();

	return CPL_OK;
}

#endif /* __cplusplus */
