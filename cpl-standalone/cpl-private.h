/*
 * cpl-private.h
 * Prov-CPL
 *
 * Copyright 2016
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
 * Contributor(s): Jackson Okuhn, Peter Macko
 */

#ifndef __CPL_PRIVATE_H__
#define __CPL_PRIVATE_H__

#include <cplxx.h>
#include <cpl-db-backend.h>
#include <private/cpl-lock.h>


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
 * Ensure that the argument is not CPL_NONE
 *
 * @param n the ID to check
 */
#define CPL_ENSURE_NOT_NONE(n) { \
	if ((n) == CPL_NONE) return CPL_E_INVALID_ARGUMENT; }

#define CPL_ENSURE_O_TYPE(n) { \
	if ((n) < 1 || (n) > 3) return CPL_E_INVALID_ARGUMENT; }

#define CPL_ENSURE_R_TYPE(n) { \
	if ((n) < 1 || (n) > 18) return CPL_E_INVALID_ARGUMENT; }
/**
 * Ensure that the originator is valid
 *
 * @param id the originator ID
 */
#define CPL_ENSURE_ORIGINATOR(id)	(void) id;
// TODO Implement the check


#endif

