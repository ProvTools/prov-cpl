/*
 * cpl-rdf.h
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

#ifndef __CPL_RDF_H__
#define __CPL_RDF_H__

#include <cpl-db-backend.h>


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


/***************************************************************************/
/** Database Types                                                        **/
/***************************************************************************/

/**
 * A generic RDF/SPARQL database
 */
#define CPL_RDF_GENERIC		0
#define CPL_RDF_UNKNOWN		CPL_RDF_GENERIC

/**
 * 4store
 */
#define CPL_RDF_4STORE			1

/**
 * Jena
 */
#define CPL_RDF_JENA			2



/***************************************************************************/
/** Constructor                                                           **/
/***************************************************************************/

/**
 * Create an RDF backend
 *
 * @param url_query the SPARQL query endpoint URL
 * @param url_update the SPARQL update endpoint URL
 * @param db_type the database type
 * @param out the pointer to the database backend variable
 * @return the error code
 */
cpl_return_t
cpl_create_rdf_backend(const char* url_query,
					   const char* url_update,
					   int db_type,
					   cpl_db_backend_t** out);

#ifdef __cplusplus
}
#endif

#endif

