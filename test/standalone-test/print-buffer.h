/*
 * print-buffer.h
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

#ifndef __STANDALONE_TEST__PRINT_BUFFER_H__
#define __STANDALONE_TEST__PRINT_BUFFER_H__

#include <cstdio>


/***************************************************************************/
/** Constants                                                             **/
/***************************************************************************/

#define L_ERROR			1000
#define L_WARNING		2000
#define L_DEBUG			4000
#define L_MAX			10000


/***************************************************************************/
/** Buffer output                                                         **/
/***************************************************************************/

/**
 * Set the buffer output level
 *
 * @param l the new output level (L_MAX = silent)
 * @return the old output level
 */
int
set_output_level(int l);

/**
 * Clear the buffer
 */
void
clear_buffer(void);

/**
 * Print a line to the buffer, and depending on the level, also to stdout
 *
 * @param level the output level (between 0 and L_MAX-1)
 * @param format the format string
 * @param ... the arguments of the format
 */
void
print(int level, const char* format, ...);


/***************************************************************************/
/** Buffer access                                                         **/
/***************************************************************************/

/**
 * Print the entire buffer
 *
 * @param out the output file
 */
void
print_buffer(std::FILE* out);

#endif

