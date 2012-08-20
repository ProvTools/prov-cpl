/*
 * cplxx.h
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

#ifndef __CPLXX_H__
#define __CPLXX_H__

#if defined _WIN64 || defined _WIN32
#pragma once
#endif

#include <cpl.h>

#ifdef __cplusplus



/***************************************************************************/
/** Header Files and Namespaces                                           **/
/***************************************************************************/

#include <cpl-exception.h>

#include <list>
#include <string>
#include <vector>

#ifdef __GNUC__
#include <ext/hash_map>
#include <ext/hash_set>
#else
#include <hash_map>
#include <hash_set>
#endif

#ifdef __GNUC__
using namespace __gnu_cxx;
#endif

#if defined _WIN32 || defined _WIN64
using namespace stdext;
#endif



/***************************************************************************/
/** ID Manipulation                                                       **/
/***************************************************************************/

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



/***************************************************************************/
/** Templates                                                             **/
/***************************************************************************/

/**
 * Traits for cpl_id_t
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
	 * Determine whether the two parameters are equal on UNIX or a < b on Windows
	 *
	 * @param a the first argument
	 * @param b the second argument
	 * @return true if they are equal on UNIX or a < b on Windows
	 */
	inline bool operator() (const cpl_id_t& a, const cpl_id_t& b) const
	{
#if defined _WIN64 || defined _WIN32
		return a < b;
#else
		return a == b;
#endif
	}
};


/**
 * Hash set: cpl_id_t
 */
#if defined _WIN32 || defined _WIN64
typedef hash_set<cpl_id_t, cpl_traits_id_t>
	cpl_hash_set_id_t;
#else
typedef hash_set<cpl_id_t, cpl_traits_id_t, cpl_traits_id_t>
	cpl_hash_set_id_t;
#endif


/**
 * Hash map template: cpl_id_t --> T
 */
#if defined _WIN32 || defined _WIN64
template <class T>
struct cpl_hash_map_id_t
{
	typedef hash_map<cpl_id_t, T, cpl_traits_id_t>
		type;
};
#else
template <class T>
struct cpl_hash_map_id_t
{
	typedef hash_map<cpl_id_t, T, cpl_traits_id_t, cpl_traits_id_t>
		type;
};
#endif



/***************************************************************************/
/** Special Classes                                                       **/
/***************************************************************************/

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



/***************************************************************************/
/** Special Data Types                                                    **/
/***************************************************************************/

/**
 * Information about a provenance object.
 */
typedef struct cplxx_object_info {
	
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
    std::string originator;

	/// The object name.
    std::string name;

	/// The object type.
	std::string type;

	/// The object ID of the container, or CPL_NONE if none.
	cpl_id_t container_id;

	/// The version number of the container, or CPL_VERSION_NONE if none.
	cpl_version_t container_version;

} cplxx_object_info_t;

/**
 * An entry in the collection of properties
 */
typedef struct cplxx_property_entry {

	/// The object ID
	cpl_id_t id;

	/// The object version
	cpl_version_t version;

	/// The property key (name)
	std::string key;

	/// The property value
	std::string value;

} cplxx_property_entry_t;



/***************************************************************************/
/** Callbacks                                                             **/
/***************************************************************************/

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
							      void* context);

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
#ifdef SWIG
%constant
#endif
EXPORT cpl_return_t
cpl_cb_collect_id_timestamp_vector(const cpl_id_t id,
								   const unsigned long timestamp,
								   void* context);

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
								 void* context);

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
									  void* context);

#endif /* __cplusplus */

#endif /* __CPLXX_H__ */

