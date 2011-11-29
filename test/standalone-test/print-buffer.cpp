/*
 * print-buffer.cpp
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
#include "print-buffer.h"

#include <private/cpl-platform.h>

#include <cstdarg>


/***************************************************************************/
/** The Buffer                                                            **/
/***************************************************************************/

/**
 * The maximum length of the buffer line
 */
#define MAX_BUFFER_LINE		(256-1)

/**
 * A line in the buffer
 */
typedef struct buffer_line {
	size_t seq_number;
	int level;
	char str[MAX_BUFFER_LINE + 1];
} buffer_line_t;

/**
 * The maximum number of lines
 */
#define MAX_LINES			1024

/**
 * The buffer
 */
static buffer_line_t buffer[MAX_LINES];

/**
 * The "from" index to the buffer
 */
static size_t buffer_from = 0;

/**
 * The "to" index to the buffer
 */
static size_t buffer_to = 0;

/**
 * The number of lines written to the buffer (can be > MAX_LINES)
 */
static size_t buffer_num_lines = 0;

/**
 * The lock for the buffer
 */
static Mutex buffer_lock;

/**
 * The output level
 */
static int output_level = L_MAX;



/***************************************************************************/
/** Buffer output                                                         **/
/***************************************************************************/


/**
 * Set the buffer output level
 *
 * @param l the new output level (0 = silent)
 * @return the old output level
 */
int
set_output_level(int l)
{
	int old = output_level;
	if (l < 0) l = 0;
	if (l > L_MAX) l = L_MAX;
	output_level = l;
	return old;
}


/**
 * Clear the buffer
 */
void
clear_buffer(void)
{
	buffer_lock.Lock();
	buffer_from = 0;
	buffer_to = 0;
	buffer_num_lines = 0;
	buffer_lock.Unlock();
}


/**
 * Print a line to the buffer, and depending on the level, also to stdout
 *
 * @param level the output level (between 1 and L_MAX)
 * @param format the format string
 * @param ... the arguments of the format
 */
void
print(int level, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	if (level < 1) level = 1;
	if (level > L_MAX) level = L_MAX;

	buffer_lock.Lock();

	char str[MAX_BUFFER_LINE+1];
#if defined _WIN64 || defined _WIN32
	vsnprintf_s(str, MAX_BUFFER_LINE, _TRUNCATE, format, args);
#else
	vsnprintf(str, MAX_BUFFER_LINE, format, args);
#endif
	str[MAX_BUFFER_LINE] = '\0';

	size_t index = buffer_to;
	buffer[index].level = level;
	buffer[index].seq_number = ++buffer_num_lines;
	strcpy(buffer[index].str, str);

	buffer_to++;
	if (buffer_to >= MAX_LINES) buffer_to = 0;
	if (buffer_to == buffer_from) buffer_from++;
	if (buffer_from >= MAX_LINES) buffer_from = MAX_LINES;

	buffer_lock.Unlock();

	if (level <= output_level) {
		printf("%s\n", str);
	}

	va_end(args);
}



/***************************************************************************/
/** Buffer access                                                         **/
/***************************************************************************/

/**
 * Print the entire buffer
 *
 * @param out the output file
 */
void
print_buffer(std::FILE* out)
{
	buffer_lock.Lock();

	size_t index = buffer_from;
	while (index != buffer_to) {
		fprintf(out, "%4lu : %s\n", buffer[index].seq_number,buffer[index].str);
		index++;
		if (index >= MAX_LINES) index = 0;
	}

	buffer_lock.Unlock();
}

