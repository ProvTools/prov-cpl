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
 * A generic type for an ID. It is used primarily for object IDs.
 */
typedef struct cpl_id {
	union {
		struct {
			unsigned long long hi;
			unsigned long long lo;
		};
		char bytes[16];
	};
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

/*
 * Static assertions
 */
extern int __cpl_assert__cpl_id_size[sizeof(cpl_id_t) == 16 ? 1 : -1];



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
/** Constants                                                             **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
WINDLL_API extern const cpl_id_t CPL_NONE;

/**
 * An invalid version number
 */
#define CPL_VERSION_NONE				((cpl_version_t) -1)



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
 * Generic data dependency
 */
#define CPL_DATA_INPUT					CPL_DATA_DEPENDENCY(0)

/**
 * Data translation
 */
#define CPL_DATA_TRANSLATION			CPL_DATA_DEPENDENCY(1)

/**
 * Data copy
 */
#define CPL_DATA_COPY					CPL_DATA_DEPENDENCY(2)


/**
 * Generic control dependency
 */
#define CPL_CONTROL_OP					CPL_CONTROL_DEPENDENCY(0)

/**
 * Process start/fork
 */
#define CPL_CONTROL_START				CPL_CONTROL_DEPENDENCY(1)


/***************************************************************************/
/** Error Codes                                                           **/
/***************************************************************************/

/**
 * Check whether the given return value is OK
 *
 * @param r the return value
 * @return true if it is OK
 */
#define CPL_IS_OK(r) ((r) >= 0)

/**
 * No error
 */
#define CPL_SUCCESS						0
#define CPL_OK							CPL_SUCCESS

/**
 * Invalid argument
 */
#define CPL_E_INVALID_ARGUMENT			-1

/**
 * Out of resources
 */
#define CPL_E_INSUFFICIENT_RESOURCES	-2

/**
 * Database backend connection error
 */
#define CPL_E_DB_CONNECTION_ERROR		-3

/**
 * The requested feature is not (yet) implemented
 */
#define CPL_E_NOT_IMPLEMENTED			-4

/**
 * The CPL library is already initialized
 */
#define CPL_E_ALREADY_INITIALIZED		-5

/**
 * The CPL library was not yet initialized
 */
#define CPL_E_NOT_INITIALIZED			-6

/**
 * The database backend failed to compile a query / prepare a statement
 */
#define CPL_E_PREPARE_STATEMENT_ERROR	-7

/**
 * The database backend failed to execute a statement (or bind a parameter)
 */
#define CPL_E_STATEMENT_ERROR			-8

/**
 * The internal error
 */
#define CPL_E_INTERNAL_ERROR			-9

/**
 * The backend internal error
 */
#define CPL_E_BACKEND_INTERNAL_ERROR	-10

/**
 * The requested object/version/etc. was not found
 */
#define CPL_E_NOT_FOUND					-11

/**
 * The requested object/version/etc. already exists
 */
#define CPL_E_ALREADY_EXISTS			-12
 
/**
 * An error originated by the underlying platform
 */
#define CPL_E_PLATFORM_ERROR			-13



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
/** Provenance functions                                                  **/
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
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_data_flow(const cpl_id_t data_dest,
			  const cpl_id_t data_source,
			  const int type);

/**
 * Disclose a control flow operation.
 *
 * @param object_id the ID of the controlled object
 * @param controller the object ID of the controller
 * @param type the control dependency edge type
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_control(const cpl_id_t object_id,
			const cpl_id_t controller,
			const int type);


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

#endif /* __cplusplus */


#endif /* __CPL_STANDALONE_H__ */

