/*
 * standalone-test.h
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

#ifndef __STANDALONE_TEST_H__
#define __STANDALONE_TEST_H__

#include <cplxx.h>
#include <cpl-exception.h>
#include <cpl-file.h>

#include "print-buffer.h"

#define ORIGINATOR "edu.harvard.pass.cpl.standalone-test"


/***************************************************************************/
/** Common Types                                                          **/
/***************************************************************************/

struct test_info
{
	const char* name;
	const char* description;
	void (*func)(void);
};


/***************************************************************************/
/** Helpers                                                               **/
/***************************************************************************/

/**
 * Throw an exception on error
 *
 * @param f the CPL function
 * @param r the return value
 */
#define CPL_VERIFY(f, r) { \
	if (!CPL_IS_OK(r)) { \
		throw CPLException("Function %s() failed in file %s on or before " \
				"line %d -- %s (%d)", \
				#f, __FILE__, __LINE__, cpl_error_string(r), (r)); \
	} \
}


/***************************************************************************/
/** Tests (in their respective .cpp files)                                **/
/***************************************************************************/

/**
 * The simplest possible test
 */
void
test_simple(void);

/**
 * The mini stress test
 */
void
test_mini_stress(void);


#endif

