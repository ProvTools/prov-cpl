/*
 * cpl.h
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

#ifndef __CPL_H__
#define __CPL_H__

#if defined _WIN64 || defined _WIN32
#pragma once
#endif

#include <stddef.h>

#ifdef __cplusplus
#include <cpl-exception.h>
#include <list>
#include <vector>
#endif

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

struct _cpl_db_backend_t;

#if defined _WIN64 || defined _WIN32
	#define EXPORT __declspec(dllexport)
	// Use the /D "_CPL_DLL" compiler option in VC++ to distinguish
	// the CPL library that defines the given symbols from its users
	#if defined(_CPL_DLL)
	#define WINDLL_API __declspec(dllexport)
	#else
	#define WINDLL_API __declspec(dllimport)
	#endif
#else
	#define EXPORT
	#define WINDLL_API
#endif


/***************************************************************************/
/** Standard types                                                        **/
/***************************************************************************/

/**
 * The CPL version - major number
 */
#define CPL_VERSION_MAJOR		1

/**
 * The CPL version - minor number (two digits)
 */
#define CPL_VERSION_MINOR		0

/**
 * The CPL version - as a string
 */
#define CPL_VERSION_STR			"1.00"


/***************************************************************************/
/** Standard types                                                        **/
/***************************************************************************/

/**
 * A generic type for an ID. It is used primarily for object IDs.
 */
typedef struct cpl_id {
#ifndef SWIG
	union {
		struct {
#endif
			unsigned long long hi;
			unsigned long long lo;
#ifndef SWIG
		};
		char bytes[16];
	};
#endif
} cpl_id_t;

/**
 * A version number.
 */
typedef int cpl_version_t;

/**
 * A session ID.
 */
typedef cpl_id_t cpl_session_t;

/**
 * A generic function return type.
 */
typedef int cpl_return_t;

/**
 * Information about a provenance object.
 */
typedef struct cpl_object_info {
	
	/// The object ID.
	cpl_id_t id;
	
	/// The object version.
	cpl_version_t version;

	/// The session ID of the process that created the object (not necessarily
	/// the latest version).
	cpl_session_t creation_session;

	/// The object creation time expressed as UNIX time.
	unsigned long creation_time;

	/// The string that uniquely identifies the application that created
	/// the object in the first place (this string also acts as a namespace).
	char* originator;

	/// The object name.
	char* name;

	/// The object type.
	char* type;

	/// The object ID of the container, or CPL_NONE if none.
	cpl_id_t container_id;

	/// The version number of the container, or CPL_VERSION_NONE if none.
	cpl_version_t container_version;

} cpl_object_info_t;

/**
 * Information about a specific version of a provenance object.
 */
typedef struct cpl_version_info {

	/// The object ID.
	cpl_id_t id;

	/// The object version.
	cpl_version_t version;

	/// The session ID od the process that created this version.
	cpl_session_t session;

	/// The version creation time expressed as UNIX time.
	unsigned long creation_time;

} cpl_version_info_t;

/**
 * The iterator callback function used by cpl_get_object_ancestry().
 *
 * @param query_object_id the ID of the object on which we are querying
 * @param query_object_verson the version of the queried object
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param other_object_version the version of the other object
 * @param type the type of the data or the control dependency
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_ancestry_iterator_t)
						(const cpl_id_t query_object_id,
						 const cpl_version_t query_object_version,
						 const cpl_id_t other_object_id,
						 const cpl_version_t other_object_version,
						 const int type,
						 void* context);

/**
 * The arguments of cpl_ancestry_iterator_t() expressed as a struct (excluding
 * the caller-provided context).
 */
typedef struct cpl_ancestry_entry {

	/// The ID of the object on which we are querying.
	cpl_id_t query_object_id;

	/// The version of the queried object.
	cpl_version_t query_object_version;

	/// The ID of the object on the other end of the dependency/ancestry edge.
	cpl_id_t other_object_id;

	/// The version of the other object.
	cpl_version_t other_object_version;

	/// The type of the data or the control dependency.
	int type;

} cpl_ancestry_entry_t;

/*
 * Static assertions
 */
#ifdef _DEBUG
extern int __cpl_assert__cpl_id_size[sizeof(cpl_id_t) == 16 ? 1 : -1];
#endif



/***************************************************************************/
/** ID Manipulation                                                       **/
/***************************************************************************/

/**
 * Copy an ID
 *
 * @param dest the destination ID
 * @param src the source ID
 */
inline void
cpl_id_copy(cpl_id_t* dest, const cpl_id_t* src)
{
	dest->hi = src->hi;
	dest->lo = src->lo;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return negative if a < b, 0 if a == b, or positive if b > a
 */
inline int
cpl_id_cmp(const cpl_id_t* a, const cpl_id_t* b)
{
	if (a->hi != b->hi) return a->hi < b->hi ? -1 : 1;
	if (a->lo != b->lo) return a->lo < b->lo ? -1 : 1;
	return 0;
}



/***************************************************************************/
/** Basic Constants                                                       **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
WINDLL_API extern const cpl_id_t CPL_NONE;

/**
 * An invalid version number
 */
#ifdef SWIG
#define CPL_VERSION_NONE				-1
#else
#define CPL_VERSION_NONE				((cpl_version_t) -1)
#endif



/***************************************************************************/
/** Dependency Edge Types                                                 **/
/***************************************************************************/

/**
 * The data dependency category
 */
#define CPL_DEPENDENCY_CATEGORY_DATA	1

/**
 * The control dependency category
 */
#define CPL_DEPENDENCY_CATEGORY_CONTROL	2


/**
 * Data dependency
 *
 * @param n the dependency subtype
 */
#define CPL_DATA_DEPENDENCY(n)		((CPL_DEPENDENCY_CATEGORY_DATA << 8) | (n))

/**
 * Control dependency
 *
 * @param n the dependency subtype
 */
#define CPL_CONTROL_DEPENDENCY(n)	((CPL_DEPENDENCY_CATEGORY_CONTROL<<8) | (n))


/**
 * Return the dependency category
 *
 * @param d the dependency code
 * @return the dependency category
 */
#define CPL_GET_DEPENDENCY_CATEGORY(d)	((d) >> 8)


/**
 * An unspecified dependency type
 */
#define CPL_DEPENDENCY_NONE				0


/**
 * Generic data dependency
 */
#define CPL_DATA_INPUT					CPL_DATA_DEPENDENCY(0)

/**
 * Potential data dependency via an observed IPC
 */
#define CPL_DATA_IPC					CPL_DATA_DEPENDENCY(1)

/**
 * Data translation
 */
#define CPL_DATA_TRANSLATION			CPL_DATA_DEPENDENCY(2)

/**
 * Data copy
 */
#define CPL_DATA_COPY					CPL_DATA_DEPENDENCY(3)


/**
 * Generic control dependency
 */
#define CPL_CONTROL_OP					CPL_CONTROL_DEPENDENCY(0)

/**
 * Process start/fork
 */
#define CPL_CONTROL_START				CPL_CONTROL_DEPENDENCY(1)



/***************************************************************************/
/** Return Codes                                                          **/
/***************************************************************************/

/**
 * Check whether the given return value is OK
 *
 * @param r the return value
 * @return true if it is OK
 */
#define CPL_IS_OK(r)					((r) >= 0)
#define CPL_IS_SUCCESS(r)				CPL_IS_OK(r)
#define CPL_SUCCEEDED(r)				CPL_IS_OK(r)

/**
 * No error
 */
#define CPL_S_OK						0
#define __CPL_S_STR__0					"Success"

/**
 * No error (aliases)
 */
#define CPL_OK							CPL_S_OK
#define __CPL_E_STR__0					__CPL_S_STR__0

/**
 * Success, but the given dependency edge (or object, version, etc.) was not
 * added due to duplicate elimination
 */
#define CPL_S_DUPLICATE_IGNORED			1
#define __CPL_S_STR__1					"Success (duplicate ignored)"

/**
 * Success, but the function did not return any data
 */
#define CPL_S_NO_DATA					2
#define __CPL_S_STR__2					"Success (no data)"

/**
 * Invalid argument
 */
#define CPL_E_INVALID_ARGUMENT			-1
#define __CPL_E_STR__1					"Invalid argument"

/**
 * Out of resources
 */
#define CPL_E_INSUFFICIENT_RESOURCES	-2
#define __CPL_E_STR__2					"Insufficient resources"

/**
 * Database backend connection error
 */
#define CPL_E_DB_CONNECTION_ERROR		-3
#define __CPL_E_STR__3					"Database connection error"

/**
 * The requested feature is not (yet) implemented
 */
#define CPL_E_NOT_IMPLEMENTED			-4
#define __CPL_E_STR__4					"Not implemented"

/**
 * The CPL library is already initialized
 */
#define CPL_E_ALREADY_INITIALIZED		-5
#define __CPL_E_STR__5					"CPL has already been initialized"

/**
 * The CPL library was not yet initialized
 */
#define CPL_E_NOT_INITIALIZED			-6
#define __CPL_E_STR__6					"CPL has not yet been initialized"

/**
 * The database backend failed to compile a query / prepare a statement
 */
#define CPL_E_PREPARE_STATEMENT_ERROR	-7
#define __CPL_E_STR__7	"The database failed to compile a prepared statement"

/**
 * The database backend failed to execute a statement (or bind a parameter)
 */
#define CPL_E_STATEMENT_ERROR			-8
#define __CPL_E_STR__8	"The database failed to execute a statement (a query)"

/**
 * The internal error
 */
#define CPL_E_INTERNAL_ERROR			-9
#define __CPL_E_STR__9					"Internal error"

/**
 * The backend internal error
 */
#define CPL_E_BACKEND_INTERNAL_ERROR	-10
#define __CPL_E_STR__10	"Database or the database driver internal error"

/**
 * The requested object/version/etc. was not found
 */
#define CPL_E_NOT_FOUND					-11
#define __CPL_E_STR__11					"Not found"

/**
 * The requested object/version/etc. already exists
 */
#define CPL_E_ALREADY_EXISTS			-12
#define __CPL_E_STR__12					"Already exits"
 
/**
 * An error originated by the underlying platform
 */
#define CPL_E_PLATFORM_ERROR			-13
#define __CPL_E_STR__13	"Could not handle an error returned by the native API"

/**
 * An error originated by the underlying platform
 */
#define CPL_E_INVALID_VERSION			-14
#define __CPL_E_STR__14					"Invalid version"

/**
 * The database returned an unexpected NULL value
 */
#define CPL_E_DB_NULL					-15
#define __CPL_E_STR__15	"The database returned an unexpected NULL value"

/**
 * The key was not found
 */
#define CPL_E_DB_KEY_NOT_FOUND			-16
#define __CPL_E_STR__16	"The database did not find the specified key"

/**
 * The value has a wrong type
 */
#define CPL_E_DB_INVALID_TYPE			-17
#define __CPL_E_STR__17	"The value in a database has an unexpected type"



/***************************************************************************/
/** Graph Traversal & Query Flags                                         **/
/***************************************************************************/

/**
 * Get ancestors
 */
#define CPL_D_ANCESTORS					0

/**
 * Get descendants
 */
#define CPL_D_DESCENDANTS				1

/**
 * Ignore data dependencies
 */
#define CPL_A_NO_DATA_DEPENDENCIES		(1 << 1)

/**
 * Ignore control dependencies
 */
#define CPL_A_NO_CONTROL_DEPENDENCIES	(1 << 2)



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
EXPORT cpl_return_t
cpl_attach(struct _cpl_db_backend_t* backend);

/**
 * Perform the cleanup and detach the library from the database backend.
 * Please note that this function is not thread-safe.
 *
 * @return the error code
 */
EXPORT cpl_return_t
cpl_detach(void);


/***************************************************************************/
/** Helpers                                                               **/
/***************************************************************************/

/**
 * Return the string version of the given error code
 *
 * @param error the error code
 * @return the error string (the function always succeeds)
 */
EXPORT const char*
cpl_error_string(cpl_return_t error);


/***************************************************************************/
/** Disclosed Provenance API                                              **/
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
EXPORT cpl_return_t
cpl_create_object(const char* originator,
				  const char* name,
				  const char* type,
				  const cpl_id_t container,
				  cpl_id_t* out_id);

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
EXPORT cpl_return_t
cpl_lookup_object(const char* originator,
				  const char* name,
				  const char* type,
				  cpl_id_t* out_id);

/**
 * Disclose a data flow.
 *
 * @param data_dest the destination object
 * @param data_source the source object
 * @param type the data dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
EXPORT cpl_return_t
cpl_data_flow(const cpl_id_t data_dest,
			  const cpl_id_t data_source,
			  const int type);

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
EXPORT cpl_return_t
cpl_data_flow_ext(const cpl_id_t data_dest,
				  const cpl_id_t data_source,
				  const cpl_version_t data_source_ver,
				  const int type);

/**
 * Disclose a control flow operation.
 *
 * @param object_id the ID of the controlled object
 * @param controller the object ID of the controller
 * @param type the control dependency edge type
 * @return CPL_OK, CPL_S_DUPLICATE_IGNORED, or an error code
 */
EXPORT cpl_return_t
cpl_control(const cpl_id_t object_id,
			const cpl_id_t controller,
			const int type);

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
EXPORT cpl_return_t
cpl_control_ext(const cpl_id_t object_id,
				const cpl_id_t controller,
				const cpl_version_t controller_ver,
				const int type);


/***************************************************************************/
/** Provenance Access API                                                 **/
/***************************************************************************/

/**
 * Get a version of a provenance object
 *
 * @param id the object ID
 * @param out_version the pointer to store the version of the object
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_version(const cpl_id_t id,
				cpl_version_t* out_version);

/**
 * Get the ID of the current session
 *
 * @param out_session the pointer to store the ID of the current session
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_current_session(cpl_session_t* out_session);

/**
 * Get information about the given provenance object
 *
 * @param id the object ID
 * @param out_info the pointer to store the object info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_object_info(const cpl_id_t id,
					cpl_object_info_t** out_info);

/**
 * Free cpl_object_info_t
 *
 * @param info the pointer to the object info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_free_object_info(cpl_object_info_t* info);

/**
 * Get information about the specific version of a provenance object
 *
 * @param id the object ID
 * @param version the version of the given provenance object
 * @param out_info the pointer to store the version info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_version_info(const cpl_id_t id,
					 const cpl_version_t version,
					 cpl_version_info_t** out_info);

/**
 * Free cpl_version_info_t
 *
 * @param info the pointer to the version info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_free_version_info(cpl_version_info_t* info);

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
EXPORT cpl_return_t
cpl_get_object_ancestry(const cpl_id_t id,
						const cpl_version_t version,
						const int direction,
						const int flags,
						cpl_ancestry_iterator_t iterator,
						void* context);


/***************************************************************************/
/** Utility functions                                                     **/
/***************************************************************************/

/**
 * A 64-bit mix hash function
 *
 * @param key the key
 * @return the hash value
 */
inline size_t
cpl_hash_int64(const long long key)
{
	// From: http://www.concentric.net/~ttwang/tech/inthash.htm

	long long k = key;

	k = (~k) + (k << 21); // k = (k << 21) - k - 1;
	k = k ^ (k >> 24);
	k = (k + (k << 3)) + (k << 8); // k * 265
	k = k ^ (k >> 14);
	k = (k + (k << 2)) + (k << 4); // k * 21
	k = k ^ (k >> 28);
	k = k + (k << 31);

	return (size_t) k;
}

/**
 * A 64-bit mix hash function for ID's
 *
 * @param key the key
 * @return the hash value
 */
inline size_t
cpl_hash_id(const cpl_id_t key)
{
	return cpl_hash_int64(key.lo) ^ ~cpl_hash_int64(key.hi);
}


#ifdef __cplusplus
}
#endif



/***************************************************************************/
/** ID Manipulation in C++                                                **/
/***************************************************************************/

#ifdef __cplusplus

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a < b
 */
inline bool
operator<(const cpl_id_t& a, const cpl_id_t& b)
{
	return cpl_id_cmp(&a, &b) < 0;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a <= b
 */
inline bool
operator<=(const cpl_id_t& a, const cpl_id_t& b)
{
	return cpl_id_cmp(&a, &b) <= 0;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a > b
 */
inline bool
operator>(const cpl_id_t& a, const cpl_id_t& b)
{
	return cpl_id_cmp(&a, &b) > 0;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a >= b
 */
inline bool
operator>=(const cpl_id_t& a, const cpl_id_t& b)
{
	return cpl_id_cmp(&a, &b) >= 0;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a == b
 */
inline bool
operator==(const cpl_id_t& a, const cpl_id_t& b)
{
	return a.hi == b.hi && a.lo == b.lo;
}

/**
 * Compare ID's
 *
 * @param a the first ID
 * @param b the second ID
 * @return true if a != b
 */
inline bool
operator!=(const cpl_id_t& a, const cpl_id_t& b)
{
	return a.hi != b.hi || a.lo != b.lo;
}

#endif /* __cplusplus */


/***************************************************************************/
/** Enhanced C++ Functionality                                            **/
/***************************************************************************/

#ifdef __cplusplus

/**
 * Traits
 */
struct cpl_traits_id_t
{
	/**
	 * Mean bucket size that the container should try not to exceed
	 */
	static const size_t bucket_size = 10;

	/**
	 * Minimum number of buckets, power of 2, >0
	 */
	static const size_t min_buckets = (1 << 10);

	/**
	 * Compute the hash value for the given argument
	 *
	 * @param key the argument
	 * @return the hash value
	 */
	inline size_t operator() (const cpl_id_t& key) const
	{
		return cpl_hash_id(key);
	}

	/**
	 * Determine whether the two parameters are equal
	 *
	 * @param a the first argument
	 * @param b the second argument
	 * @return true if they are equal
	 */
	inline bool operator() (const cpl_id_t& a, const cpl_id_t& b) const
	{
		return a == b;
	}
};

/**
 * The initialization & cleanup helper
 */
class CPLInitializationHelper
{

public:

	/**
	 * Create the instance of the class and initialize the library
	 *
	 * @param backend the database backend (or NULL to just handle cleanup)
	 */
	CPLInitializationHelper(struct _cpl_db_backend_t* backend)
	{
		if (backend != NULL) {
			if (!CPL_IS_OK(cpl_attach(backend))) {
				throw CPLException("Failed to initialize the Core Provenance "
						"Library");
			}
		}
	}

	/**
	 * Destroy the instance of this class and deinitialize the library
	 */
	~CPLInitializationHelper(void)
	{
		cpl_detach();
	}
};

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
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_ancestry_list(const cpl_id_t query_object_id,
							 const cpl_version_t query_object_version,
							 const cpl_id_t other_object_id,
							 const cpl_version_t other_object_version,
							 const int type,
							 void* context);

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
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_ancestry_vector(const cpl_id_t query_object_id,
							   const cpl_version_t query_object_version,
							   const cpl_id_t other_object_id,
							   const cpl_version_t other_object_version,
							   const int type,
							   void* context);

#endif /* __cplusplus */

#endif /* __CPL_STANDALONE_H__ */

