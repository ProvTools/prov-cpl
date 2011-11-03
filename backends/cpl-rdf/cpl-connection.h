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
#include <string>

#include <curl/curl.h>



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


#endif

