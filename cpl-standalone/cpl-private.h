/*
 * cpl-private.h
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

#ifndef __CPL_PRIVATE_H__
#define __CPL_PRIVATE_H__

#include <cpl.h>
#include <cpl-db-backend.h>
#include "cpl-lock.h"

#ifdef __GNUC__
#include <ext/hash_map>
#include <ext/hash_set>
#else
#include <hash_map>
#include <hash_set>
#endif

#ifdef __GNUC__
using namespace __gnu_cxx;
#endif

#if defined _WIN32 || defined _WIN64
using namespace stdext;
#endif


/***************************************************************************/
/** Types for the private state                                           **/
/***************************************************************************/

/**
 * Hash map: cpl_id_t --> cpl_version_t
 */
#if defined _WIN32 || defined _WIN64
typedef hash_map<cpl_id_t, cpl_version_t, cpl_traits_id_t>
	cpl_hash_map_id_to_version_t;
#else
typedef hash_map<cpl_id_t, cpl_version_t, cpl_hash_id_t, cpl_equals_id_t>
	cpl_hash_map_id_to_version_t;
#endif


/**
 * State of an open object ID
 */
typedef struct {

	/**
	 * Light-weight Lock
	 */
	cpl_lock_t locked;

	/**
	 * Object version
	 */
	cpl_version_t version;

	/**
	 * Freeze status
	 */
	bool frozen;

	/**
	 * Last session ID that touched this node - might be invalid if frozen
	 */
	cpl_session_t last_session;

	/**
	 * Cache of immediate ancestors - might be incomplete
	 */
	cpl_hash_map_id_to_version_t ancestors;

} cpl_open_object_t;


/**
 * Hash map: cpl_id_t --> cpl_open_object_t*
 */
#if defined _WIN32 || defined _WIN64
typedef hash_map<cpl_id_t, cpl_open_object_t*, cpl_traits_id_t>
	cpl_hash_map_id_to_open_object_t;
#else
typedef hash_map<cpl_id_t, cpl_open_object_t*, cpl_hash_id_t, cpl_equals_id_t>
	cpl_hash_map_id_to_open_object_t;
#endif


/***************************************************************************/
/** Private functions                                                     **/
/***************************************************************************/

/**
 * Create an instance of an in-memory state
 *
 * @param version the object version
 * @return the instantiated object, or NULL if out of resources
 */
cpl_open_object_t*
cpl_new_open_object(const cpl_version_t version);

/**
 * Get the open object handle, opening the object if necessary,
 * and return the object locked
 *
 * @param id the object ID
 * @param out the output
 * @return the error code
 */
cpl_return_t
cpl_get_open_object_handle(const cpl_id_t id, cpl_open_object_t** out);


/***************************************************************************/
/** Convenience macros                                                    **/
/***************************************************************************/

/**
 * Run a function, and if it returns an error, just return it
 *
 * @param x the statement to run
 */
#define CPL_RUNTIME_VERIFY(x) { \
	cpl_return_t _x = (cpl_return_t) (x); if (!CPL_IS_OK(_x)) return (int) _x; }

/**
 * Ensure that the argument is not NULL
 *
 * @param p the pointer to check
 */
#define CPL_ENSURE_NOT_NULL(p) { \
	if ((p) == NULL) return CPL_E_INVALID_ARGUMENT; }

/**
 * Ensure that the argument is nonnegative
 *
 * @param n the number to check
 */
#define CPL_ENSURE_NOT_NEGATIVE(n) { \
	if ((n) < 0) return CPL_E_INVALID_ARGUMENT; }

/**
 * Ensure that the originator is valid
 *
 * @param id the originator ID
 */
#define CPL_ENSURE_ORIGINATOR(id)	(void) id;
// TODO Implement the check


#endif

