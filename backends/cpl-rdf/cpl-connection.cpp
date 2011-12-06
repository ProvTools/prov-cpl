/*
 * cpl-connection.cpp
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

#include "stdafx.h"
#include "cpl-connection.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <string>



/***************************************************************************/
/** Constructor and Destructor                                            **/
/***************************************************************************/

/**
 * Initialize the connection
 *
 * @param url the SPARQL endpoint URL
 * @return the connection handle, or NULL on error
 */
cpl_rdf_connection_t*
cpl_rdf_connection_init(const char* url)
{
	// Allocate the struct

	cpl_rdf_connection_t* c = new cpl_rdf_connection_t;
	if (c == NULL) return NULL;

	c->url = url;
	RDFResultSet rs; //XXX


	// Initialize lock

	mutex_init(c->lock);


	// Initialize Curl

	c->curl = curl_easy_init();
	if (c->curl == NULL) goto err_free;

	c->headers = NULL;
    c->headers = curl_slist_append(c->headers,
			"Accept: text/plain, application/sparql-results+xml");
    curl_easy_setopt(c->curl, CURLOPT_HTTPHEADER, c->headers);


	// Finalize
	cpl_rdf_connection_execute_query(c, "SELECT * WHERE { ?s ?p ?o }", &rs);

	return c;


	// Error handling

err_free:
	mutex_destroy(c->lock);
	delete c;
	return NULL;
}


/**
 * Close the connection
 *
 * @param connection the connection handle
 */
void
cpl_rdf_connection_close(cpl_rdf_connection_t* connection)
{
	assert(connection != NULL);
	assert(connection->curl != NULL);

	curl_slist_free_all(connection->headers);
	curl_easy_cleanup(connection->curl);

	mutex_destroy(connection->lock);

	delete connection;
}



/***************************************************************************/
/** Public API: Result Set                                                **/
/***************************************************************************/


/**
 * Create an empty result set
 */
RDFResultSet::RDFResultSet(void)
{
}


/**
 * Destroy the result set
 */
RDFResultSet::~RDFResultSet(void)
{
	for (unsigned u = 0; u < m_results.size(); u++) {
		RDFResult* r = m_results[u];
		for (RDFResult::iterator i = r->begin(); i != r->end(); i++) {
			delete i->second;
		}
		delete r;
	}
}


/**
 * Append an error message to the vector of error messages
 *
 * @param format the format of the exception
 * @param ... the arguments of the format
 */
void
RDFResultSet::append_error_message(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	char s[256];

#ifdef _WINDOWS
	vsnprintf_s(s, sizeof(s), _TRUNCATE, format, args);
#else
	vsnprintf(s, sizeof(s), format, args);
#endif
	s[sizeof(s)-1] = '\0';

	va_end(args);
	m_errors.push_back(std::string(s));
}



/***************************************************************************/
/** Private API: Parsing XML responses                                    **/
/***************************************************************************/


/**
 * Parse a single value
 *
 * @param cur the cursor to the &gt;binding&lt;...&gt;/binding&lt; tag
 * @param result_set the result set
 * @param out the pointer to an already allocated value object
 * @return CPL_OK or an error code
 */
static cpl_return_t
cpl_rdf_parse_xml_value(xmlNodePtr cur,
						RDFResultSet* result_set,
						RDFValue* out)
{
	assert(out != NULL);
	xmlNodePtr cur_value;
	bool found = true;

	for (cur_value = cur->children; cur_value != NULL;
		 cur_value = cur_value->next) {


		// Ignore selected nodes

		if (strcmp((char*) cur_value->name, "text") == 0) continue;


		// URI

		if (strcmp((char*) cur_value->name, "uri") == 0) {
			out->type = RDF_T_URI;
			if (cur_value->children==NULL) return CPL_E_BACKEND_INTERNAL_ERROR;
			out->raw = (char*) XML_GET_CONTENT(cur_value->children);
			out->v_uri = out->raw.c_str();
			found = true;
			continue;
		}


		// Error: Unexpected node

		if (result_set != NULL) {
			result_set->append_error_message("Invalid node \"%s\" in the "
					"<binding> tag of the server response", cur_value->name);
		}
		return CPL_E_BACKEND_INTERNAL_ERROR;
	}

	return found ? CPL_OK : CPL_E_BACKEND_INTERNAL_ERROR;
}


/**
 * Parse a single result
 *
 * @param cur the cursor to the &gt;result&lt;...&gt;/result&lt; tag
 * @param result_set the result set
 * @param out the pointer to an already allocated result object
 * @return CPL_OK or an error code
 */
static cpl_return_t
cpl_rdf_parse_xml_result(xmlNodePtr cur,
						 RDFResultSet* result_set,
						 RDFResult* out)
{
	assert(out != NULL);
	xmlNodePtr cur_binding;

	for (cur_binding = cur->children; cur_binding != NULL;
		 cur_binding = cur_binding->next) {


		// Ignore selected nodes

		if (strcmp((char*) cur_binding->name, "text") == 0) continue;


		// Binding

		if (strcmp((char*) cur_binding->name, "binding") == 0) {

			std::string name = "";
			bool found = false;


			// Find the variable name in the list of properties
			// NOTE I guess we don't need the for loop at all...

			xmlAttrPtr a;
			for (a = cur_binding->properties; a != NULL; a = a->next) {

				if (strcmp((char*) a->name, "name") == 0) {
					xmlChar* s = xmlGetProp(cur_binding,
											(const xmlChar*)"name");
					if (s == NULL) continue;
					name = (char*) s;
					found = true;
					continue;
				}


				// Error: Unexpected property

				if (result_set != NULL) {
					result_set->append_error_message("Invalid property \"%s\" "
							"in the <binding> node of the server response",
							a->name);
				}
				return CPL_E_BACKEND_INTERNAL_ERROR;
			}

			if (!found) {
				if (result_set != NULL) {
					result_set->append_error_message("No property \"name\" "
							"in the <binding> node of the server response");
				}
				return CPL_E_BACKEND_INTERNAL_ERROR;
			}


			// Parse out the value

			RDFValue* v = new RDFValue;
			if (v == NULL) return CPL_E_INSUFFICIENT_RESOURCES;
			cpl_return_t ret;
			ret = cpl_rdf_parse_xml_value(cur_binding, result_set, v);
			if (!CPL_IS_OK(ret)) {
				delete v;
				return ret;
			}


			// Add it to the variable binding

			(*out)[name] = v;

			continue;
		}


		// Error: Unexpected node

		if (result_set != NULL) {
			result_set->append_error_message("Invalid node \"%s\" in the "
					"server response", cur_binding->name);
		}
		return CPL_E_BACKEND_INTERNAL_ERROR;
	}

	return CPL_OK;
}


/**
 * Parse an XML result set
 *
 * @param str the XML string
 * @param str_len the length of the string, or 0 for a NULL-terminated string
 * @param out the result set to which the result should be appended (must be
 *            already allocated)
 * @return CPL_OK, CPL_S_NO_DATA, or an error code
 */
static cpl_return_t
cpl_rdf_parse_xml_result_set(const char* str,
							 size_t str_len,
							 RDFResultSet* out)
{
	cpl_return_t ret = CPL_E_BACKEND_INTERNAL_ERROR;
	size_t num_results = 0;

	xmlNodePtr cur;
	xmlNodePtr cur_result;
	xmlNodePtr root;


	// Initialize the parser

	xmlDocPtr doc = xmlReadMemory(str, str_len == 0 ? strlen(str) : str_len,
			 					  "server-response.xml", NULL, 0);
	if (doc == NULL) {
		ret = CPL_E_STATEMENT_ERROR;
		goto err;
	}


	// Get the root node

	root = xmlDocGetRootElement(doc);


	// Iterate over the root node

	for (cur = root->children; cur != NULL; cur = cur->next) {


		// Ignore selected nodes

		if (strcmp((char*) cur->name, "text") == 0) continue;
		if (strcmp((char*) cur->name, "head") == 0) continue;


		// Results

		if (strcmp((char*) cur->name, "results") == 0) {
			for (cur_result = cur->children; cur_result != NULL;
				 cur_result = cur_result->next) {


				// Ignore selected nodes

				if (strcmp((char*) cur_result->name, "text") == 0) continue;


				// Result

				if (strcmp((char*) cur_result->name, "result") == 0) {
					RDFResult* r = new RDFResult;
					if (r == NULL) {
						return CPL_E_INSUFFICIENT_RESOURCES;
						goto err;
					}
					ret = cpl_rdf_parse_xml_result(cur_result, out, r);
					if (!CPL_IS_OK(ret)) {
						delete r;
						goto err;
					}
					if (out != NULL) out->append(r); else delete r;
					num_results++;
					continue;
				}


				// Error: Unexpected node

				if (out != NULL) {
					out->append_error_message("Invalid node \"%s\" in the "
							"server response", cur_result->name);
				}
				ret = CPL_E_BACKEND_INTERNAL_ERROR;
				goto err;
			}
			continue;
		}


		// Error: Unexpected node

		if (out != NULL) {
			out->append_error_message("Invalid node \"%s\" in the server"
					" response", cur->name);
		}
		ret = CPL_E_BACKEND_INTERNAL_ERROR;
		goto err;
	}


	// Finish

	ret = num_results == 0 ? CPL_S_NO_DATA : CPL_OK;

err:
	xmlFreeDoc(doc);
	return ret;
}



/***************************************************************************/
/** Public API: Submitting simple queries                                 **/
/***************************************************************************/


/**
 * The CURLOPT_WRITEDATA data
 */
typedef struct _cpl_rdf_writedata
{
	cpl_rdf_connection_t* connection;
	std::string buffer;
} _cpl_rdf_writedata_t;


/**
 * The write function handler for cpl_rdf_connection_execute_query()
 *
 * @param s the pointer to the data received from cURL
 * @param size
 * @param nmemb
 * @param userdata the user data
 * @return the number of data taken care of
 */
static size_t
query_write_function(char *s, size_t size, size_t nmemb, void* userdata)
{
	_cpl_rdf_writedata_t* wd = (_cpl_rdf_writedata_t*) userdata;
	wd->buffer += s;
	return size * nmemb;
}


/**
 * Execute a query
 *
 * @param connection the connection handle
 * @param statement the query statement
 * @param out the pointer to an already initialized result set
 * @return an error code
 */
cpl_return_t
cpl_rdf_connection_execute_query(cpl_rdf_connection_t* connection,
								 const char* statement,
								 RDFResultSet* out)
{
	char my_curl_error[CURL_ERROR_SIZE];
	long response_code = 0;


	// Lock

	mutex_lock(connection->lock);


	// Connection options

	_cpl_rdf_writedata_t wd;
	wd.connection = connection;
	wd.buffer = "";

	curl_easy_setopt(connection->curl, CURLOPT_WRITEFUNCTION, query_write_function);
	curl_easy_setopt(connection->curl, CURLOPT_WRITEDATA, &wd);
	curl_easy_setopt(connection->curl, CURLOPT_ERRORBUFFER, my_curl_error);


	// Configure GET

	curl_easy_setopt(connection->curl, CURLOPT_POST, 0);

	char *encoded = curl_easy_escape(connection->curl, statement, 0);
	if (encoded == NULL) {
		mutex_unlock(connection->lock);
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	char *new_url = (char*) alloca(strlen(connection->url.c_str())
									+ strlen(encoded) + 16);
	if (new_url == NULL) {
		curl_free(encoded);
		mutex_unlock(connection->lock);
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	sprintf(new_url, "%s%cquery=%s", connection->url.c_str(),
			strchr(connection->url.c_str(), '?') ? '&' : '?', encoded);
	curl_free(encoded);
	curl_easy_setopt(connection->curl, CURLOPT_URL, new_url);


	// Execute the statement

	CURLcode code = curl_easy_perform(connection->curl);

	if (code) {
		if (out != NULL) {
			out->append_error_message("cURL Connection Error: %s",
					my_curl_error);
		}
		mutex_unlock(connection->lock);
		return CPL_E_DB_CONNECTION_ERROR;
	}


	// Get the HTTP response code

	curl_easy_getinfo(connection->curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (response_code != 200) {
		if (out != NULL) {
			out->append_error_message("%s", wd.buffer.c_str());
		}
		mutex_unlock(connection->lock);
		return CPL_E_STATEMENT_ERROR;
	}


	// Unlock the connection

	mutex_unlock(connection->lock);


	// Parse the response

	cpl_return_t ret = cpl_rdf_parse_xml_result_set(wd.buffer.c_str(),
							wd.buffer.length(), out);
	if (!CPL_IS_OK(ret)) return ret;

	return CPL_OK;
}



/**
 * The write function handler for cpl_rdf_connection_execute_update()
 *
 * @param s the pointer to the data received from cURL
 * @param size
 * @param nmemb
 * @param userdata the user data
 * @return the number of data taken care of
 */
static size_t
update_write_function(char *s, size_t size, size_t nmemb, void* userdata)
{
	_cpl_rdf_writedata_t* wd = (_cpl_rdf_writedata_t*) userdata;
	wd->buffer += s;
	return size * nmemb;
}


/**
 * Execute an update statement
 *
 * @param connection the connection handle
 * @param statement the update statement
 * @param out the pointer to an already initialized result set (to store
 *            error messages)
 * @return an error code
 */
cpl_return_t
cpl_rdf_connection_execute_update(cpl_rdf_connection_t* connection,
								  const char* statement,
								  RDFResultSet* out)
{
	char my_curl_error[CURL_ERROR_SIZE];
	long response_code = 0;


	// Lock

	mutex_lock(connection->lock);


	// Connection options

	_cpl_rdf_writedata_t wd;
	wd.connection = connection;
	wd.buffer = "";

	curl_easy_setopt(connection->curl, CURLOPT_WRITEFUNCTION, update_write_function);
	curl_easy_setopt(connection->curl, CURLOPT_WRITEDATA, &wd);
	curl_easy_setopt(connection->curl, CURLOPT_ERRORBUFFER, my_curl_error);
	curl_easy_setopt(connection->curl, CURLOPT_URL, connection->url.c_str());


	// Configure POST

	curl_easy_setopt(connection->curl, CURLOPT_POST, 1);

	char *encoded = curl_easy_escape(connection->curl, statement, 0);
	if (encoded == NULL) {
		mutex_unlock(connection->lock);
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	char *field = (char*) alloca(strlen(encoded) + 16);
	if (field == NULL) {
		curl_free(encoded);
		mutex_unlock(connection->lock);
		return CPL_E_INSUFFICIENT_RESOURCES;
	}

	sprintf(field, "update=%s", encoded);
	curl_free(encoded);
	curl_easy_setopt(connection->curl, CURLOPT_POSTFIELDS, field);


	// Execute the statement

	CURLcode code = curl_easy_perform(connection->curl);

	if (code) {
		if (out != NULL) {
			out->append_error_message("cURL Connection Error: %s",
					my_curl_error);
		}
		mutex_unlock(connection->lock);
		return CPL_E_DB_CONNECTION_ERROR;
	}


	// Get the HTTP response code

	curl_easy_getinfo(connection->curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (response_code != 200) {
		if (out != NULL) {
			out->append_error_message("%s", wd.buffer.c_str());
		}
		mutex_unlock(connection->lock);
		return CPL_E_STATEMENT_ERROR;
	}

	mutex_unlock(connection->lock);
	return CPL_OK;
}



/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


