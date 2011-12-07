/*
 * cpl-connection.h
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

#ifndef __CPL_CONNECTION_H__
#define __CPL_CONNECTION_H__

#include <private/cpl-platform.h>
#include <cpl.h>

#include <curl/curl.h>
#include <map>
#include <string>
#include <vector>



/***************************************************************************/
/** Types                                                                 **/
/***************************************************************************/

/**
 * The connection handle
 */
typedef struct {
	
	/**
	 * The underlying connection handle
	 */
	CURL* curl;

	/**
	 * The SPARQL endpoint URL
	 */
	std::string url;

	/**
	 * The header list
	 */
	struct curl_slist* headers;

	/**
	 * The connection lock
	 */
	mutex_t lock;

} cpl_rdf_connection_t;


/**
 * The result set
 */
class RDFResultSet;



/***************************************************************************/
/** Functions                                                             **/
/***************************************************************************/

/**
 * Initialize the connection
 *
 * @param url the SPARQL endpoint URL
 * @return the connection handle, or NULL on error
 */
cpl_rdf_connection_t*
cpl_rdf_connection_init(const char* url);


/**
 * Close the connection
 *
 * @param connection the connection handle
 */
void
cpl_rdf_connection_close(cpl_rdf_connection_t* connection);

/**
 * Escape a string
 *
 * @param str the string
 * @return the escaped string
 */
std::string
cpl_rdf_escape_string(const char* str);

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
								 RDFResultSet* out);

/**
 * Execute an update statement
 *
 * @param connection the connection handle
 * @param statement the update statement
 * @param out the pointer to an already initialized result set (to store
 *            error messages, can be NULL)
 * @return an error code
 */
cpl_return_t
cpl_rdf_connection_execute_update(cpl_rdf_connection_t* connection,
								  const char* statement,
								  RDFResultSet* out = NULL);



/***************************************************************************/
/** Result Set                                                            **/
/***************************************************************************/

/**
 * A value type
 */
typedef enum {
	RDF_T_STRING,
	RDF_T_URI,
} RDFValueType;

/**
 * A single value
 */
typedef struct {
	std::string raw;
	RDFValueType type;
	union {
		const char* v_string;
		const char* v_uri;
	};
} RDFValue;

/**
 * A single result in the result set
 */
typedef std::map<std::string, RDFValue*> RDFResult;

/**
 * The result set
 */
class RDFResultSet
{

public:

	/**
	 * Create an empty result set
	 */
	RDFResultSet(void);

	/**
	 * Destroy the result set
	 */
	~RDFResultSet(void);

	/**
	 * Determine the number of results
	 *
	 * @return the number of results
	 */
	inline size_t
	size(void) { return m_results.size(); }

	/**
	 * Return the given result from the result set
	 *
	 * @param index the index between 0 and size()-1
	 * @return a reference to the result
	 */
	inline RDFResult&
	operator[] (size_t index) { return *(m_results[index]); }

	/**
	 * Return the vector of error messages
	 *
	 * @return a reference to the vector of error messages
	 */
	inline const std::vector<std::string>&
	error_messages(void) { return m_errors; }

	/**
	 * Append a result to the result set
	 *
	 * @param result the result (will be destroyed together with this object)
	 */
	inline void
	append(RDFResult* result) { m_results.push_back(result); }

	/**
	 * Append an error message to the vector of error messages
	 *
	 * @param format the format of the exception
	 * @param ... the arguments of the format
	 */
	void
	append_error_message(const char* format, ...);


protected:

	/**
	 * The results
	 */
	std::vector<RDFResult*> m_results;

	/**
	 * The error messages
	 */
	std::vector<std::string> m_errors;
};


#endif

