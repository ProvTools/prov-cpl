/*
 * cpl.h
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


/***************************************************************************/
/** Versioning                                                      **/
/***************************************************************************/

/**
 * The CPL version - major number
 */
#define CPL_VERSION_MAJOR		3

/**
 * The CPL version - minor number (two digits)
 */
#define CPL_VERSION_MINOR		0

/**
 * The CPL version - as a string
 */
#define CPL_VERSION_STR			"3.0"


/***************************************************************************/
/** Standard types                                                        **/
/***************************************************************************/

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
 * Information about a provenance bundle.
 */
typedef struct cpl_bundle_info {
	
	/// The bundle ID.
	cpl_id_t id;

	/// The session ID of the process that created the object
	cpl_session_t creation_session;

	/// The object creation time expressed as UNIX time.
	unsigned long creation_time;

	/// The object name.
	char* name;

} cpl_bundle_info_t;

/**
 * Information about a provenance object.
 */
typedef struct cpl_object_info {
	
	/// The object ID.
	cpl_id_t id;

	/// The object creation time expressed as UNIX time.
	unsigned long creation_time;

	/// Namespace prefix
	char* prefix;

	/// The object name.
	char* name;

	/// The object type.
	int type;

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
 * The iterator callback function used by cpl_get_object_relations().
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        ancestry edge
 * @param type the type of the relation
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_relation_iterator_t)
						(const cpl_id_t relation_id,
						 const cpl_id_t query_object_id,
						 const cpl_id_t other_object_id,
						 const int type,
						 void* context);

/**
 * The arguments of cpl_relation_iterator_t() expressed as a struct (excluding
 * the caller-provided context).
 */
typedef struct cpl_relation {

	/// The relation ID
	cpl_id_t id;

	/// The ID of the object on which we are querying.
	cpl_id_t query_object_id;

	/// The ID of the object on the other end of the relation.
	cpl_id_t other_object_id;

	/// The type of the relation.
	int type;

} cpl_relation_t;

/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the property name
 * @param value the property value
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_property_iterator_t)
						(const cpl_id_t id,
						 const char* prefix,
						 const char* key,
						 const char* value,
						 void* context);

/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param iri the namespace iri
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
typedef cpl_return_t (*cpl_prefix_iterator_t)
						(const cpl_id_t id,
						 const char* prefix,
						 const char* iri,
						 void* context);


/***************************************************************************/
/** Basic Constants                                                       **/
/***************************************************************************/

/**
 * An invalid ID signifying no object
 */
extern const cpl_id_t CPL_NONE;

extern cpl_session_t cpl_session;

/***************************************************************************/
/** Relation Types                                                        **/
/***************************************************************************/

#define WASINFLUENCEDBY  		1
#define ALTERNATEOF				2
#define	DERIVEDBYINSERTIONFROM	3
#define	DERIVEDBYREMOVALFROM	4
#define	HADMEMBER 				5
#define	HADDICTIONARYMEMBER		6
#define	SPECIALIZATIONOF		7
#define	WASDERIVEDFROM			8
#define	WASGENERATEDBY			9
#define	WASINVALIDATEDBY		10
#define	WASATTRIBUTEDTO			11
#define	USED 					12
#define	WASINFORMEDBY			13
#define	WASSTARTEDBY			14
#define	WASENDEDBY				15
#define HADPLAN					16
#define	WASASSOCIATEDWITH		17
#define ACTEDONBEHALFOF			18
#define INBUNDLE                19
#define BUNDLE_RELATION         20

#define WASINFLUENCEDBY_STR			"wasInfluencedBy"
#define ALTERNATEOF_STR				"alternateOf"
#define	DERIVEDBYINSERTIONFROM_STR	"derivedByInsertionFrom"
#define	DERIVEDBYREMOVALFROM_STR	"derivedByRemovalFrom"
#define	HADMEMBER_STR 				"hadMember"
#define	HADDICTIONARYMEMBER_STR		"hadDictionaryMember"
#define	SPECIALIZATIONOF_STR		"specializationOf"
#define	WASDERIVEDFROM_STR			"wasDerivedFrom"
#define	WASGENERATEDBY_STR			"wasGeneratedBy"
#define	WASINVALIDATEDBY_STR		"wasInvalidatedBy"
#define	WASATTRIBUTEDTO_STR			"wasAttributedTo"
#define	USED_STR 					"used"
#define	WASINFORMEDBY_STR			"wasInformedBy"
#define	WASSTARTEDBY_STR			"wasStartedBy"
#define	WASENDEDBY_STR				"wasEndedBy"
#define HADPLAN_STR					"hadPlan"
#define	WASASSOCIATEDWITH_STR		"wasAssociatedWith"
#define ACTEDONBEHALFOF_STR			"actedOnBehalfOf"
#define INBUNDLE_STR                "inBundle"

#define CPL_IS_RELATION_TYPE(r)			((r) > 0 && (r) < 20)

#define CPL_NUM_R_TYPES					19
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

/**
 * The JSON isn't valid
 */
#define CPL_E_INVALID_JSON			-18
#define __CPL_E_STR__18	"The JSON is invalid"

/***************************************************************************/
/** Standard Object Types                                                 **/
/***************************************************************************/

/**
 * The default entity type
 */
#define CPL_ENTITY							1

/**
 * The default activity type
 */
#define CPL_ACTIVITY						2

/**
 * The default agent type
 */
#define CPL_AGENT 							3

#define CPL_BUNDLE                          4


#define CPL_ENTITY_STR						"entity"
#define CPL_ACTIVITY_STR					"activity"
#define CPL_AGENT_STR						"agent"
#define CPL_BUNDLE_STR                      "bundle"

#define CPL_IS_OBJECT_TYPE(r)			((r) > 0 && (r) < 4)

#define CPL_NUM_O_TYPES						3
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

#define CPL_J_EXTERN_OBJ				(1 << 0)
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
 * @param prefix the object prefix, must be an existing bundle prefix
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_create_object(const char* prefix,
				  const char* name,
				  const int type,
				  cpl_id_t* out_id);

/**
 * Look up an object by name. If multiple objects share the same name,
 * get the latest one.
 *
 * @param prefix the object prefix
 * @param name the object name
 * @param type the object type, CPL_NONE for no type
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_object(const char* prefix,
				  const char* name,
				  const int type,
				  cpl_id_t* out_id);

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
EXPORT cpl_return_t
cpl_lookup_object_ext(const char* prefix,
					  const char* name,
					  const int type,
					  const int flags,
					  cpl_id_timestamp_iterator_t iterator,
					  void* context);

/**
 * Lookup or create an object if it does not exist.
 *
 * @param prefix the object prefix
 * @param name the object name
 * @param type the object type
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_or_create_object(const char* prefix,
							const char* name,
							const int type,
							cpl_id_t* out_id);

/**
 * Add a property to the given object.
 *
 * @param id the object ID
 * @param prefix the prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_add_object_property(const cpl_id_t id,
					    const char* prefix,
					    const char* key,
	                    const char* value);

/**
* Add a relation
* 
* @param from_id the source's ID
* @param to_id the destination's ID
* @param type the relation's PROV type
* @param bundle the ID of the bundle that should contain this relation
* @param out_id the pointer to store the ID of the newly created relation
* @return CPL_OK or an error code
**/
EXPORT cpl_return_t
cpl_add_relation(const cpl_id_t from_id,
		  	     const cpl_id_t to_id,
			     const int type,
			     cpl_id_t* out_id);

/**
 * Add a property to the given relation.
 *
 * @param id the object ID
  * @param prefix the prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_add_relation_property(const cpl_id_t id,
						  const char* prefix,
						  const char* key,
		                  const char* value);

/**
 * Create a bundle.
 *
 * @param name the object name
 * @param out_id the pointer to store the ID of the newly created object
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_create_bundle(const char* name,
				  const char* prefix,
				  cpl_id_t* out_id);

/**
 * Look up a bundle by name. If multiple bundles share the same name,
 * get the latest one.
 *
 * @param name the bundle name
 * @param out_id the pointer to store the object ID
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_bundle(const char* name,
				  const char* prefix,
				  cpl_id_t* out_id);

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
EXPORT cpl_return_t
cpl_lookup_bundle_ext(const char* name,
					  const char* prefix,
					  const int flags,
 					  cpl_id_timestamp_iterator_t iterator,
					  void* context);

/**
 * Look up a relation by from_id, to_id and type.
 * If multiple relations match, get the latest one.
 *
 * @param from_id object id of source
 * @param to_id object id of destination
 * @param type the type of the relation
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_relation(const cpl_id_t from_id,
                    const cpl_id_t to_id,
                    const long type,
                    cpl_id_t* out_id);

/**
 * Search object properties by value, with wildcards
 *
 * @param value the value of the object property
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_object_property_wildcard(const char* value,
                    cpl_id_t* out_id);

/**
 * Add a property to the given relation.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param key the key
 * @param value the value
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_add_bundle_property(const cpl_id_t id,
					    const char* prefix,
					    const char* key,
	                    const char* value);

/**
 * Add a prefix to a bundle.
 *
 * @param prefix the namespace prefix
 * @param iri the namespace iri
 * @param value the value
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_add_prefix(const cpl_id_t id,
			   const char* prefix,
               const char* iri);

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
cpl_get_all_objects( const char* prefix,
					const int flags,
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
						 cpl_relation_iterator_t iterator,
						 void* context);

/**
 * Get the properties associated with the given provenance object.
 *
 * @param id the the object ID
 * @param prefix the property prefix to fetch - or NULL 
 *               (along with key)for all properties
 * @param key the property key to fetch - or NULL 
 *            (along with prefix) for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
EXPORT cpl_return_t
cpl_get_object_properties(const cpl_id_t id,
	                      const char* prefix,
				          const char* key,
				          cpl_property_iterator_t iterator,
				          void* context);

/**
 * Lookup an object based on a property value.
 *
 * @param prefix the property prefix
 * @param key the property name
 * @param value the property value
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_E_NOT_FOUND, or an error code
 */
EXPORT cpl_return_t
cpl_lookup_object_by_property(const char* prefix,
							  const char* key,
					          const char* value,
					          cpl_property_iterator_t iterator,
					          void* context);

/**
 * Get the properties associated with the given provenance object.
 * 
 * @param id the the object ID
 * @param prefix the property prefix to fetch - or NULL 
 *               (along with key)for all properties
 * @param key the property key to fetch - or NULL 
 *            (along with prefix) for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
EXPORT cpl_return_t
cpl_get_relation_properties(const cpl_id_t id,
							const char* prefix,
				            const char* key,
				            cpl_property_iterator_t iterator,
				            void* context);

/**
 * Deletes a bundle and all objects and relations belonging to it.
 *
 * @param id the bundle ID
 * @return CPL_OK, or an error code
 */
EXPORT cpl_return_t
cpl_delete_bundle(const cpl_id_t id);

/**
 * Get information about the given provenance bundle.
 *
 * @param id the bundle ID
 * @param out_info the pointer to store the bundle info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_get_bundle_info(const cpl_id_t id,
					cpl_bundle_info_t** out_info);

/**
 * Free cpl_bundle_info_t.
 *
 * @param info the pointer to the bundle info structure
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_free_bundle_info(cpl_bundle_info_t* info);

/**
 * Get all objects belonging to a bundle
 *
 * @paramID the bundle ID
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, or an error code
 */
EXPORT cpl_return_t
cpl_get_bundle_objects(const cpl_id_t id,
					   cpl_object_info_iterator_t iterator,
					   void* context);

/**
 * Get all relations belonging to a bundle
 *
 * @param ID the bundle ID
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, or an error code
 */
EXPORT cpl_return_t
cpl_get_bundle_relations(const cpl_id_t id,
					     cpl_relation_iterator_t iterator,
					     void* context);

/**
 * Get the properties associated with the given provenance bundle.
 *
 * @param id the the bundle ID
 * @param prefix the property prefix to fetch - or NULL 
 *               (along with key)for all properties
 * @param key the property key to fetch - or NULL 
 *            (along with prefix) for all keys
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
EXPORT cpl_return_t
cpl_get_bundle_properties(const cpl_id_t id,
						  const char* prefix,
			              const char* key,
			              cpl_property_iterator_t iterator,
			              void* context);

/**
 * Get the prefixes associated with the given provenance bundle.
 *
 * @param id the the bundle ID
 * @param prefix the property prefix to fetch - or NULL for all prefixes
 * @param iterator the iterator callback function
 * @param context the user context to be passed to the iterator function
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
EXPORT cpl_return_t
cpl_get_prefixes(const cpl_id_t id,
						const char* prefix,
			            cpl_prefix_iterator_t iterator,
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

