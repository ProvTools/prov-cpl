/*
 * cpl-db-backend.h
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

#ifndef __CPL_DB_BACKEND_H__
#define __CPL_DB_BACKEND_H__

#include <cpl-standalone.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


/***************************************************************************/
/** Database Backend Interface                                            **/
/***************************************************************************/

typedef struct {

	/**
	 * Create an object.
	 *
	 * @param originator the originator ID
	 * @param name the object name
	 * @param type the object type
	 * @param container the ID of the object that should contain this object
	 *                  (use CPL_NONE for no container)
	 * @param container_version the version of the container (if not CPL_NONE)
	 * @return the object ID, or a negative value on error
	 */
	cpl_id_t
	(*cpl_db_create_object)(const cpl_id_t originator,
							const char* name,
							const char* type,
							const cpl_id_t container,
							const cpl_version_t container_version);

	/**
	 * Look up an object by name. If multiple objects share the same name,
	 * get the latest one.
	 *
	 * @param name the object name
	 * @param type the object type
	 * @return the object ID, or a negative value on error
	 */
	cpl_id_t
	(*cpl_db_lookup_by_name)(const char* name,
					   		 const char* type);

	/**
	 * Determine the version of the object
	 *
	 * @param id the object ID
	 * @return the object version or an error code
	 */
	cpl_version_t
	(*cpl_db_get_version)(const cpl_id_t id);

	/**
	 * Add an ancestry edge
	 *
	 * @param from_id the edge source ID
	 * @param from_ver the edge source version
	 * @param to_id the edge destination ID
	 * @param to_ver the edge destination version
	 * @return the error code
	 */
	cpl_return_t
	(*cpl_db_add_ancestry_edge)(const cpl_id_t from_id,
								const cpl_version_t from_ver,
								const cpl_id_t to_id,
								const cpl_version_t to_ver);

} cpl_db_backend_t;



#ifdef __cplusplus
}
#endif

#endif

