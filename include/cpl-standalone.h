/*
 * cpl-standalone.h
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

#ifndef __CPL_STANDALONE_H__
#define __CPL_STANDALONE_H__

#include <stddef.h>

#ifdef __cplusplus
#include <exception>
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

struct _cpl_db_backend_t;


/***************************************************************************/
/** Standard types                                                        **/
/***************************************************************************/

/**
 * A generic type for an ID. It is being used for object IDs and originator
 * instance IDs.
 */
typedef long long cpl_id_t;

/**
 * A version number.
 */
typedef int cpl_version_t;

/**
 * A generic function return type.
 */
typedef long long cpl_return_t;


/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
#define CPL_NONE						0

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
 * Generic control dependency
 */
#define CPL_CONTROL_OP					CPL_CONTROL_DEPENDENCY(0)


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



/***************************************************************************/
/** Initialization and Cleanup                                            **/
/***************************************************************************/

/**
 * Initialize the library. Please note that this function is not thread-safe.
 *
 * @param backend the database backend
 * @return the error code
 */
cpl_return_t
cpl_initialize(struct _cpl_db_backend_t* backend);

/**
 * Perform the cleanup. Please note that this function is not thread-safe.
 *
 * @return the error code
 */
cpl_return_t
cpl_cleanup(void);


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
 * @return the object ID, or a negative value on error
 */
cpl_id_t
cpl_create_object(const char* originator,
				  const char* name,
				  const char* type,
				  const cpl_id_t container);

/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param originator the object originator
 * @param name the object name
 * @param type the object type
 * @return the object ID, or a negative value on error
 */
cpl_id_t
cpl_lookup_object(const char* originator,
				  const char* name,
				  const char* type);

/**
 * Disclose a data flow.
 *
 * @param data_dest the destination object
 * @param data_source the source object
 * @param type the data dependency edge type
 * @return the operation return value
 */
cpl_return_t
cpl_data_flow(const cpl_id_t data_dest,
			  const cpl_id_t data_source,
			  const int type);


/***************************************************************************/
/** Utility functions                                                     **/
/***************************************************************************/

/**
 * A 64-bit mix hash function for ID's
 *
 * @param key the key
 * @return the hash value
 */
inline size_t
cpl_hash_id(const cpl_id_t key)
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


#ifdef __cplusplus
}
#endif


/***************************************************************************/
/** Enhanced C++ Functionality                                            **/
/***************************************************************************/

#ifdef __cplusplus

/**
 * Compatison function for the hash map
 */
struct cpl_equals_id_t
{
	/**
	 * Determine whether the two parameters are equal
	 *
	 * @param a the first argument
	 * @param b the second argument
	 * @return true if they are equal
	 */
	inline bool operator() (const cpl_id_t a, const cpl_id_t b) const
	{
		return a == b;
	}
};

/**
 * A 64-bit hash function for ID's
 */
struct cpl_hash_id_t
{
	/**
	 * Compute the hash value for the given argument
	 *
	 * @param key the argument
	 * @return the hash value
	 */
	inline size_t operator() (const cpl_id_t key) const
	{
		return cpl_hash_id(key);
	}
};

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
	inline size_t operator() (const cpl_id_t key) const
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
	inline bool operator() (const cpl_id_t a, const cpl_id_t b) const
	{
		return a == b;
	}
};

/**
 * The initialization & cleanup helper
 */
class CPL_InitializationHelper
{

public:

	/**
	 * Create the instance of the class and initialize the library
	 *
	 * @param backend the database backend
	 */
	CPL_InitializationHelper(struct _cpl_db_backend_t* backend)
	{
		if (!CPL_IS_OK(cpl_initialize(backend))) {
			throw std::exception();
		}
	}

	/**
	 * Destroy the instance of this class and deinitialize the library
	 */
	~CPL_InitializationHelper(void)
	{
		cpl_cleanup();
	}
};

#endif /* __cplusplus */

#endif /* __CPL_STANDALONE_H__ */

