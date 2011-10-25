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
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


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


/***************************************************************************/
/** Provenance functions                                                  **/
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
cpl_id_t
cpl_create_object(const cpl_id_t originator,
				  const char* name,
				  const char* type,
				  const cpl_id_t container);

/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param name the object name
 * @param type the object type
 * @return the object ID, or a negative value on error
 */
cpl_id_t
cpl_lookup_by_name(const char* name,
				   const char* type);

/**
 * Disclose a data transfer.
 *
 * @param originator the provenance originator ID
 * @param source the source object
 * @param dest the destination object
 * @return the operation return value
 */
cpl_return_t
cpl_disclose_data_transfer(const cpl_id_t originator,
						   const cpl_id_t source,
						   const cpl_id_t dest);


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

#endif /* __cplusplus */

#endif /* __CPL_STANDALONE_H__ */

