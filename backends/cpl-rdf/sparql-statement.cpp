/*
 * sparql-statement.cpp
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

#include "stdafx.h"
#include "sparql-statement.h"


/***************************************************************************/
/** Private API                                                           **/
/***************************************************************************/


/**
 * Escape a string
 *
 * @param str the string
 * @return the escaped string (please use free() afterwards), or NULL on eror
 */
static char*
escape_string(const char* str)
{
	if (str == NULL) return NULL;


	// Count characters that would need to be escaped

	size_t to_escape = 0;
	size_t str_len = strlen(str);

	for (size_t i = 0; i < str_len; i++) {
		char c = str[i];

		if (c == '\t' || c == '\b' || c == '\n' || c == '\r' || c == '\f'
				|| c == '\\' || c == '\"' || c == '\'') to_escape++;
	}


	// The simplest case

	if (to_escape == 0) return strdup(str);


	// Escape

	char* escaped = (char*) malloc(str_len + to_escape + 4);
	size_t u = 0;

	for (size_t i = 0; i < str_len; i++) {
		char c = str[i];

		if (u >= str_len + to_escape) {
			// This should never happen unless we have a race condition
			free(escaped);
			return NULL;
		}

		switch (c) {
			case '\t':
				escaped[u++] = '\\';
				escaped[u++] = 't';
				break;
			case '\b':
				escaped[u++] = '\\';
				escaped[u++] = 'b';
				break;
			case '\n':
				escaped[u++] = '\\';
				escaped[u++] = 'n';
				break;
			case '\r':
				escaped[u++] = '\\';
				escaped[u++] = 'r';
				break;
			case '\f':
				escaped[u++] = '\\';
				escaped[u++] = 'f';
				break;
			case '\\':
			case '\"':
			case '\'':
				escaped[u++] = '\\';
				escaped[u++] = c;
				break;
			default:
				escaped[u++] = c;
		}
	}

	if (u != str_len + to_escape) {
		free(escaped);
		return NULL;
	}

	escaped[u] = '\0';

	return escaped;
}



/***************************************************************************/
/** Constructor and Destructor                                            **/
/***************************************************************************/



/***************************************************************************/
/** Public API                                                            **/
/***************************************************************************/


