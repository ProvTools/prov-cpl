/*
 * cpl-rdf-private.h
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

#ifndef __CPL_RDF_PRIVATE_H__
#define __CPL_RDF_PRIVATE_H__

#include <backends/cpl-rdf.h>
#include <private/cpl-lock.h>
#include <private/cpl-platform.h>
#include <string>

#include <cplxx.h>
#include "cpl-connection.h"


#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif

#define CPL_RDF_SEM_INIT		"edu.harvard.pass.cpl.rdf.ver"


/***************************************************************************/
/** RDF Database Backend                                                 **/
/***************************************************************************/

/**
 * The RDF database backend
 */
typedef struct {

	/**
	 * The backend interface (must be first)
	 */
	cpl_db_backend_t backend;

	/**
	 * The database type
	 */
	int db_type;

	/**
	 * The SPARQL URL endpoint for queries
	 */
	std::string url_query;

	/**
	 * The connection handle for queries
	 */
	cpl_rdf_connection_t* connection_query;

	/**
	 * The SPARQL URL endpoint for updates
	 */
	std::string url_update;

	/**
	 * The connection handle for queries
	 */
	cpl_rdf_connection_t* connection_update;

	/**
	 * The shared semaphore for creating new versions
	 */
	cpl_shared_semaphore_t sem_create_version;

} cpl_rdf_t;


/**
 * The RDF interface
 */
extern const cpl_db_backend_t CPL_RDF_BACKEND;


#ifdef __cplusplus
}
#endif

#endif

