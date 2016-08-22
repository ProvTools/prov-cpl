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

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

struct _cpl_db_backend_t;

#define EXPORT
#define WINDLL_API


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
#define CPL_VERSION_STR			"1.0"


/***************************************************************************/
/** Standard types                                                        **/
/***************************************************************************/
//TODO add ancestry types
/**
 * A generic type for an ID. It is used primarily for object IDs.
 */
typedef unsigned long long cpl_id_t;


/**
 * A combination of the ID and the UNIX timestamp.
 */
typedef struct cpl_id_timestamp {

	/// The ID
	cpl_id_t id;

	/// The UNIX timestamp
	unsigned long timestamp;

} cpl_id_timestamp_t;

/**
 * A session ID.
 */
typedef cpl_id_t cpl_session_t;

/**
 * A generic function return type.
 */
typedef int cpl_return_t;

/**
 * Information about a provenance session.
 */
typedef struct cpl_session_info {

	/// The session ID.
	cpl_session_t id;

	/// The MAC address of the computer responsible for the provenance record.
	char* mac_address;

	/// The user name.
	char* user;

	/// The PID of the application that created the record.
	int pid;

	/// The program name.
	char* program;

	/// The program's command line.
	char* cmdline;

	/// The start time of the session (expressed as UNIX time).
	unsigned long start_time;

} cpl_session_info_t;

/**
 * Information about a provenance object.
 */
typedef struct cpl_object_info {
	
	/// The object ID.
	cpl_id_t id;

	/// The session ID of the process that created the object
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

} cpl_object_info_t;

/**
 * The iterator callback for getting multiple object infos. The caller will
 * take care of destroying the passed-in info object.
 *
 * @param info the object info
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_object_info_iterator_t)
						(const cpl_object_info_t* info,
						 void* context);


/**
 * The iterator callback function used by cpl_lookup_object_ext().
 *
 * @param id the object ID
 * @param timestamp the object creation time expressed as UNIX time
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_id_timestamp_iterator_t)
						(const cpl_id_t id,
						 const unsigned long timestamp,
						 void* context);

/**
 * The iterator callback function used by cpl_get_object_ancestry().
 *
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param type the type of the data or the control dependency
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_ancestry_iterator_t)
						(const cpl_id_t query_object_id,
						 const cpl_id_t other_object_id,
						 const int type,
						 void* context);

/**
 * The arguments of cpl_ancestry_iterator_t() expressed as a struct (excluding
 * the caller-provided context).
 */
typedef struct cpl_ancestry_entry {

	cpl_id_t id;

	/// The ID of the object on which we are querying.
	cpl_id_t query_object_id;

	/// The ID of the object on the other end of the dependency/ancestry edge.
	cpl_id_t other_object_id;

	/// The type of the data or the control dependency.
	int type;

} cpl_ancestry_entry_t;

/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param key the property name
 * @param value the property value
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_property_iterator_t)
						(const cpl_id_t id,
						 const char* key,
						 const char* value,
						 void* context);

/*
 * Static assertions
 */
#ifdef _DEBUG
extern int __cpl_assert__cpl_id_size[sizeof(cpl_id_t) == 8 ? 1 : -1];
#endif


/***************************************************************************/
/** Basic Constants                                                       **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
WINDLL_API extern const cpl_id_t CPL_NONE;




/***************************************************************************/
/** Dependency Edge Types                                                 **/
/***************************************************************************/
// TODO finish

#define NOT_SPECIFIED 			0
#define F_ENT_T_ENT				1
#define F_ENT_T_ACT				2
#define F_ENT_T_AGT				3
#define F_ACT_T_ENT				4
#define F_ACT_T_ACT				5
#define F_ACT_T_AGT				6
#define F_AGT_T_ENT				7
#define F_AGT_T_ACT				8
#define F_AGT_T_AGT				9

#define FROM_TO_VAL(t,v)		((t << 8) | v)

#define ALTERNATEOF				FROM_TO_VAL(F_ENT_T_ENT, 1)
#define	DERIVEDBYINSERTIONFROM	FROM_TO_VAL(F_ENT_T_ENT, 2)
#define	DERIVEDBYREMOVALFROM	FROM_TO_VAL(F_ENT_T_ENT, 3)
#define	HADMEMBER 				FROM_TO_VAL(F_ENT_T_ENT, 4)
#define	HADDICTIONARYMEMBER		FROM_TO_VAL(F_ENT_T_ENT, 5)
#define	SPECIALIZATIONOF		FROM_TO_VAL(F_ENT_T_ENT, 6)
#define	WASDERIVEDFROM			FROM_TO_VAL(F_ENT_T_ENT, 7)

#define	WASGENERATEDBY			FROM_TO_VAL(F_ENT_T_ACT, 1)
#define	WASINVALIDATEDBY		FROM_TO_VAL(F_ENT_T_ACT, 2)

#define	WASATTRIBUTEDTO			FROM_TO_VAL(F_ENT_T_AGT, 1)


#define	USED 					FROM_TO_VAL(F_ACT_T_ENT, 1)

#define	WASINFORMEDBY			FROM_TO_VAL(F_ACT_T_ACT, 1)

#define	WASSTARTEDBY			FROM_TO_VAL(F_ACT_T_AGT, 1)
#define	WASENDEDBY				FROM_TO_VAL(F_ACT_T_AGT, 2)

#define HADPLAN					FROM_TO_VAL(F_AGT_T_ENT, 1)

#define	WASASSOCIATEDWITH		FROM_TO_VAL(F_AGT_T_ACT, 1)

#define ACTEDONBEHALFOF			FROM_TO_VAL(F_AGT_T_AGT, 1)

#define WASINFLUENCEDBY(t)		FROM_TO_VAL(t, 1)

#define GET_RELATION_CATEGORY(n) (n >> 8)
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
 * Success, but the desired object was not found, so it was automatically
 * created
 */
#define CPL_S_OBJECT_CREATED			3
#define __CPL_S_STR__3					"Success (object created)"

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
/** Standard Object Originators                                           **/
/***************************************************************************/

/**
 * The file system originator (use for files, set object name to real-path)
 */
#define CPL_O_FILESYSTEM				"/fs"

/**
 * The Internet originator (use for everything on the Internet)
 */
#define CPL_O_INTERNET					"/internet"



/***************************************************************************/
/** Standard Object Types                                                 **/
/***************************************************************************/

/**
 * The default entity type
 */
#define ENTITY							"ENTITY"

/**
 * The default activity type
 */
#define ACTIVITY						"ACTIVITY"

/**
 * The default agent type
 */
#define AGENT 							"AGENT"

/**
 * The default bundle type
 */
#define BUNDLE							"BUNDLE"



/***************************************************************************/
/** Graph Traversal, Query, and Lookup Flags                              **/
/***************************************************************************/

/**
 * Do not fail during lookup (return CPL_S_NO_DATA if not found)
 */
#define CPL_L_NO_FAIL					(1 << 0)

/**
 * Do not get the creation session information, if it is not readily available
 */
#define CPL_I_NO_CREATION_SESSION		(1 << 0)


/**
 * Do not get any information that is not readily available at the lookup time
 */
#define CPL_I_FAST						CPL_I_NO_CREATION_SESSION

/**
 * Get ancestors
 */
#define CPL_D_ANCESTORS					0

/**
 * Get descendants
 */
#define CPL_D_DESCENDANTS				1


#define NO_ENTITIES						(1 << 1)
#define NO_ACTIVITIES					(1 << 2)
#define NO_AGENTS						(1 << 3)




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

//TODO edit function headers
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
EXPORT cpl_return_t
cpl_lookup_object_ext(const char* originator,
					  const char* name,
					  const char* type,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context);

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
EXPORT cpl_return_t
cpl_lookup_or_create_object(const char* originator,
							const char* name,
							const char* type,
							const cpl_id_t container,
							cpl_id_t* out_id);

/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_add_object_property(const cpl_id_t id,
				 const char* key,
                 const char* value);

EXPORT cpl_return_t
cpl_add_relation(const cpl_id_t from_id,
			  	   const cpl_id_t to_id,
				   const int type,
				   cpl_id_t* out_id);


/***************************************************************************/
/** Provenance Access API                                                 **/
/***************************************************************************/

/**
 * Get the ID of the current session.
 *
 * @param out_session the pointer to store the ID of the current session
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_current_session(cpl_session_t* out_session);

/**
 * Get information about the given provenance session.
 *
 * @param id the session ID
 * @param out_info the pointer to store the session info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_session_info(const cpl_session_t id,
					 cpl_session_info_t** out_info);

/**
 * Free cpl_session_info_t.
 *
 * @param info the pointer to the session info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_free_session_info(cpl_session_info_t* info);

/**
 * Get all objects in the database
 *
 * @param flags a logical combination of CPL_I_* flags
 * @param iterator the iterator to be called for each matching object
 * @param context the caller-provided iterator context
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_all_objects(const int flags,
					cpl_object_info_iterator_t iterator,
					void* context);

/**
 * Get information about the given provenance object.
 *
 * @param id the object ID
 * @param out_info the pointer to store the object info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_object_info(const cpl_id_t id,
					cpl_object_info_t** out_info);

/**
 * Free cpl_object_info_t.
 *
 * @param info the pointer to the object info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_free_object_info(cpl_object_info_t* info);


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
EXPORT cpl_return_t
cpl_get_object_relations(const cpl_id_t id,
						const int direction,
						const int flags,
						cpl_ancestry_iterator_t iterator,
						void* context);

/**
 * Get the properties associated with the given provenance object.
 *
 * @param id the the object ID
 * @param key the property to fetch - or NULL for all properties
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
EXPORT cpl_return_t
cpl_get_object_properties(const cpl_id_t id,
				   const char* key,
				   cpl_property_iterator_t iterator,
				   void* context);

/**
 * Lookup an object based on a property value.
 *
 * @param key the property name
 * @param value the property value
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
 */
EXPORT cpl_return_t
cpl_lookup_object_by_property(const char* key,
					   const char* value,
					   cpl_property_iterator_t iterator,
					   void* context);

EXPORT cpl_return_t
cpl_get_relation_properties(const cpl_id_t id,
				   const char* key,
				   cpl_property_iterator_t iterator,
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
	return cpl_hash_int64(key);
}


#ifdef __cplusplus
}
#endif

#endif /* __CPL_H__ */

