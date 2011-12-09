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
#include <iostream>
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
	RDF_XSD_URI,
	RDF_XSD_STRING,
	RDF_XSD_INTEGER,
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
		long long v_integer;
	};
} RDFValue;

/**
 * Write a human-readable version of the value to the output stream
 * 
 * @param out the output stream
 * @param value the value
 * @return the output stream
 */
std::ostream&
operator<< (std::ostream& out, const RDFValue& value);

/**
 * A single result in the result set
 */
class RDFResult
{
	friend class RDFResultSet;


public:

	/**
	 * The result map type
	 */
	typedef std::map<std::string, RDFValue*> RDFResultMap;


public:

	/**
	 * Create an empty result
	 */
	RDFResult(void);

	/**
	 * Destroy the result
	 */
	~RDFResult(void);

	/**
	 * Put a key/value pair to the result
	 *
	 * @param key the key
	 * @param value the value (will be destroyed together with this object)
	 */
	void
	put(const std::string& key, RDFValue* value);

	/**
	 * Retrieve a value based on the key
	 *
	 * @param key the key
	 * @return a reference to the value
	 * @throws CPLException if the key does not exist
	 */
	RDFValue&
	operator[] (const char* key);

	/**
	 * Safely retrieve a value based on the key
	 *
	 * @param key the key
	 * @param type the expected type
	 * @param out the place to write the retrieved value
	 * @return CPL_OK or an error code
	 */
	cpl_return_t
	get_s(const char* key, int type, RDFValue** out) const;

	/**
	 * Get a const_iterator over the results
	 *
	 * @return the const_iterator
	 */
	inline RDFResultMap::const_iterator
	begin(void) const { return m_results.begin(); }

	/**
	 * Get the end of the const_iterator over the results
	 *
	 * @return the const_iterator
	 */
	inline RDFResultMap::const_iterator
	end(void) const { return m_results.end(); }


protected:

	/**
	 * The result map
	 */
	RDFResultMap m_results;
};

/**
 * Write a human-readable version of the result to the output stream
 * 
 * @param out the output stream
 * @param result the result
 * @return the output stream
 */
std::ostream&
operator<< (std::ostream& out, const RDFResult& result);

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
	size(void) const { return m_results.size(); }

	/**
	 * Return the given result from the result set
	 *
	 * @param index the index between 0 and size()-1
	 * @return a reference to the result
	 */
	inline const RDFResult&
	operator[] (size_t index) const { return *(m_results[index]); }

	/**
	 * Return the vector of error messages
	 *
	 * @return a reference to the vector of error messages
	 */
	inline const std::vector<std::string>&
	error_messages(void) const { return m_errors; }

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

	/**
	 * Print the error messages
	 *
	 * @param out the output stream
	 * @param prefix the line prefix
	 */
	void
	print_error_messages(std::ostream& out, const char* prefix = NULL) const;

	/**
	 * Print the error messages
	 *
	 * @param file the output file
	 * @param prefix the line prefix
	 */
	void
	print_error_messages(FILE* file, const char* prefix = NULL) const;


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

/**
 * Write a human-readable version of the result set to the output stream
 * 
 * @param out the output stream
 * @param rs the result set
 * @return the output stream
 */
std::ostream&
operator<< (std::ostream& out, const RDFResultSet& rs);


#endif

