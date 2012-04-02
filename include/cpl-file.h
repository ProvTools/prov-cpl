/*
 * cpl-file.h
 * Core Provenance Library
 *
 * Copyright 2012
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

#ifndef __CPL_FILE_H__
#define __CPL_FILE_H__

#if defined _WIN64 || defined _WIN32
#pragma once
#endif

#include <cpl.h>

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}	/* Hack for editors that try to be too smart about indentation */
#endif


/***************************************************************************/
/** Types                                                                 **/
/***************************************************************************/

/**
 * SHA1 Fingerprint
 */
typedef struct cpl_sha1 {
#ifndef SWIG
	union {
#endif
		char bytes[20];
#ifndef SWIG
		unsigned words[5];
	};
#endif
} cpl_sha1_t;


/***************************************************************************/
/** Basic Constants                                                       **/
/***************************************************************************/

/**
 * Force the creation of a new provenance object for the file
 */
#define CPL_F_ALWAYS_CREATE				1

/**
 * Create a new provenance object for the file if it does not exist
 */
#define CPL_F_CREATE_IF_DOES_NOT_EXIST	2

/**
 * Open by content instead of the file name
 */
#define CPL_F_OPEN_BY_CONTENT			4


/***************************************************************************/
/** Standard Property Names                                               **/
/***************************************************************************/

/**
 * The SHA1 fingerprint property
 */
#define CPL_P_SHA1						"SHA1"


/***************************************************************************/
/** Helpers                                                               **/
/***************************************************************************/

/**
 * Compute the SHA1 fingerprint of a file.
 *
 * @param name the file name, can be either an absolute or a relative path
 * @param out the output buffer
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_file_sha1(const char* name, cpl_sha1_t* out);


/***************************************************************************/
/** File Provenance API                                                   **/
/***************************************************************************/

/**
 * Lookup or create a provenance object associated with the given file.
 *
 * @param name the file name, can be either an absolute or a relative path
 * @param type the object type
 * @param flags a logical combination of CPL_F_* flags
 * @param out_id the pointer to store the ID of the provenance object
 * @param out_version the pointer to store the version of the provenance object
 * @return CPL_OK or an error code
 */
EXPORT cpl_return_t
cpl_lookup_file(const char* name,
				const int flags,
				cpl_id_t* out_id,
				cpl_version_t* out_version);


#ifdef __cplusplus
}
#endif

#endif /* __CPL_FILE_H__ */

