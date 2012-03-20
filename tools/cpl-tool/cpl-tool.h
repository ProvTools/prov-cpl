/*
 * cpl-tool.h
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

#ifndef __CPL_TOOL_H__
#define __CPL_TOOL_H__


/***************************************************************************/
/** Common Variables                                                      **/
/***************************************************************************/

/// The program name
extern const char* program_name;

/// The tool name
extern const char* tool_name;


/***************************************************************************/
/** Common Macros                                                         **/
/***************************************************************************/

/// Fail on error
#define CPL_VERIFY(ret) { \
	if (!CPL_IS_OK(ret)) { \
		throw CPLException(cpl_error_string(ret)); \
	} \
}


/***************************************************************************/
/** Common Types                                                          **/
/***************************************************************************/

struct tool_info
{
	const char* name;
	const char* description;
	int (*func)(int argc, char** argv);
};


/***************************************************************************/
/** Tools (in their respective .cpp files)                                **/
/***************************************************************************/

/**
 * Disclose provenance
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_disclose(int argc, char** argv);

#endif

