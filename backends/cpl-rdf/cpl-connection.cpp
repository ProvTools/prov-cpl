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


/***************************************************************************/
/** Private API                                                           **/
/***************************************************************************/


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
	// TODO
	return size * nmemb;
}


/**
 * Execute a query
 *
 * @param connection the connection handle
 * @param statement the query statement
 * @return an error code
 */
static cpl_return_t
cpl_rdf_connection_execute_query(cpl_rdf_connection_t* connection,
								 const char* statement)
{
	char my_curl_error[CURL_ERROR_SIZE];
	long response_code = 0;


	// Lock

	mutex_lock(connection->lock);


	// Connection options

	//curl_easy_setopt(connection->curl, CURLOPT_WRITEFUNCTION, query_write_function);
	curl_easy_setopt(connection->curl, CURLOPT_WRITEDATA, stdout);//connection);
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
		fprintf(stderr, "cURL Connection Error: %s\n", my_curl_error);
		mutex_unlock(connection->lock);
		return CPL_E_DB_CONNECTION_ERROR;
	}


	// Get the HTTP response code

	curl_easy_getinfo(connection->curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (response_code != 200) {
		mutex_unlock(connection->lock);
		return CPL_E_STATEMENT_ERROR;
	}

	mutex_unlock(connection->lock);
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
	return size * nmemb;
}


/**
 * Execute an update statement
 *
 * @param connection the connection handle
 * @param statement the update statement
 * @return an error code
 */
static cpl_return_t
cpl_rdf_connection_execute_update(cpl_rdf_connection_t* connection,
								  const char* statement)
{
	char my_curl_error[CURL_ERROR_SIZE];
	long response_code = 0;


	// Lock

	mutex_lock(connection->lock);


	// Connection options

	curl_easy_setopt(connection->curl, CURLOPT_WRITEFUNCTION, update_write_function);
	curl_easy_setopt(connection->curl, CURLOPT_WRITEDATA, connection);
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
		fprintf(stderr, "cURL Connection Error: %s\n", my_curl_error);
		mutex_unlock(connection->lock);
		return CPL_E_DB_CONNECTION_ERROR;
	}


	// Get the HTTP response code

	curl_easy_getinfo(connection->curl, CURLINFO_RESPONSE_CODE, &response_code);
	if (response_code != 200) {
		mutex_unlock(connection->lock);
		return CPL_E_STATEMENT_ERROR;
	}

	mutex_unlock(connection->lock);
	return CPL_OK;
}


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
	cpl_rdf_connection_execute_query(c, "SELECT * WHERE { ?s ?p ?o }");

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
/** Public API                                                            **/
/***************************************************************************/


