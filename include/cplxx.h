/*
 * cplxx.h
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

#ifndef __CPLXX_H__
#define __CPLXX_H__

#include <cpl.h>

#ifdef __cplusplus



/***************************************************************************/
/** Header Files and Namespaces                                           **/
/***************************************************************************/

#include <cpl-exception.h>

#include <json.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <list>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <utility>



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

	/// The object creation time expressed as UNIX time.
	unsigned long creation_time;

	/// Namespace prefix
    std::string prefix;

	/// The object name.
    std::string name;

	/// The object type.
	int type;

} cplxx_object_info_t;

/**
 * An entry in the collection of properties
 */
typedef struct cplxx_property_entry {

	/// The object ID
	cpl_id_t id;

	/// Namespace prefix
	std::string prefix;

	/// The property key (name)
	std::string key;

	/// The property value
	std::string value;

} cplxx_property_entry_t;

/**
 * An entry in the collection of prefixes
 */
typedef struct cplxx_prefix_entry {

	/// The object ID
	cpl_id_t id;

	/// Namespace prefix
	std::string prefix;

	/// The namespace iri
	std::string iri;

} cplxx_prefix_entry_t;


/**
 * General information about PROV relation type
 */

typedef struct prov_relation_data {
	int type;
	std::string type_str;
	std::string source_str;
	int source_t;
	std::string dest_str;
	int dest_t;
} prov_relation_data_t;

/***************************************************************************/
/** Callbacks                                                             **/
/***************************************************************************/
/**
 * The iterator callback for cpl_get_all_objects() that collects the returned
 * information in an instance of std::vector<cpl_object_info_t>.
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
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        relation
 * @param other_object_version the version of the other object
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
							 void* context);

/**
 * The iterator callback for cpl_get_object_ancestry() that collects
 * the information in an instance of std::vector<cpl_ancestry_entry_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        relation
 * @param other_object_version the version of the other object
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
							   void* context);

/**
 * The iterator callback for cpl_get_properties() that collects the returned
 * information in an instance of std::vector<cplxx_property_entry_t>.
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
								 void* context);

/**
 * The iterator callback for cpl_get_prefixes() that collects the returned
 * information in an instance of std::vector<cplxx_prefix_entry_t>.
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
							   void* context);

/**
 * The iterator callback for cpl_lookup_by_property() that collects
 * the returned information in an instance of std::vector<cpl_id_t>.
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
									  void* context);


/***************************************************************************/
/** Document Handling                                                     **/
/***************************************************************************/

/*
 * Verifies the correctness of a Prov-JSON document. Not currently exhaustive.
 * 
 * @param json_string the JSON document as a string
 * @param string_out error output string
 * @return 0 on successful validation or -1 on failure
 */
EXPORT cpl_return_t
validate_json(const std::string& json_string,
			  std::string& string_out);

/*
 * Imports a Prov-JSON document into Prov-CPL.
 *
 * @param json_string the JSON document as a string
 * @param bundle_name desired name of document bundle, must be unique
 * @param anchor_objects optional vector of ID string pairs that match
 *		                 objects in the database with objects in the document
 * @param flags a logical combination of CPL_J_* flags
 * @param out_id the ID of the imported bundle
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
import_document_json(const std::string& json_string,
					 const std::string& bundle_name,
					 const std::vector<std::pair<cpl_id_t, std::string>>& anchor_objects,
					 const int flags,
					 cpl_id_t* out_id);


/*
 * Exports a Prov-CPL bundle as a Prov-JSON document.
 *
 * @param bundle a vector of bundle IDs, currently only supports one
 * @param json_string the JSON document as a string
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
export_bundle_json(const std::vector<cpl_id_t>& bundles, 
				   std::string& json_string);

#endif /* __cplusplus */

#endif /* __CPLXX_H__ */

