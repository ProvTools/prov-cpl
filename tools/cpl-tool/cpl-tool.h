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

#include <cstddef>
#include <list>
#include <string>


/***************************************************************************/
/** Common Variables                                                      **/
/***************************************************************************/

/// The program name
extern const char* program_name;

/// The tool name
extern const char* tool_name;


/***************************************************************************/
/** Termcap Variables                                                     **/
/***************************************************************************/

/// Alternative character set start
extern const char* termcap_ac_start;

/// Alternative character set end
extern const char* termcap_ac_end;

/// Vertical line
extern const char* termcap_vertical_line;

/// Horizontal line
extern const char* termcap_horizontal_line;

/// Left tee
extern const char* termcap_left_tee;

/// Left bottom corner
extern const char* termcap_left_bottom_corner;

/// Right tee
extern const char* termcap_right_tee;

/// Right upper corner
extern const char* termcap_right_upper_corner;


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


/**
 * Callback for process_recursively()
 *
 * @param filename the file name
 * @param directory the directory with a trailing / (empty string = current)
 * @param depth the directory depth (0 = current)
 * @param st the stat of the file
 * @param context the caller-supplied context
 */
typedef void (*process_recursively_callback_t)(const char* filename,
		const char* directory, int depth, struct stat* st, void* context);



/***************************************************************************/
/** Common Functions                                                      **/
/***************************************************************************/

/**
 * Recursively traverse the given file or directory
 *
 * @param filename the file name (file or directory)
 * @param include_dir whether to include directories in the result
 * @param recursive whether to traverse recursively
 * @param callback the callback
 * @param context the caller-supplied context
 */
void
process_recursively(const char* filename, bool include_dir, bool recursive,
		process_recursively_callback_t callback, void* context);



/***************************************************************************/
/** Common Callbacks                                                      **/
/***************************************************************************/

/**
 * Callback that collects file names into an instance of std::list<std::string>
 *
 * @param filename the file name
 * @param directory the directory with a trailing / (empty string = current)
 * @param depth the directory depth (0 = current)
 * @param st the stat of the file
 * @param context the caller-supplied context
 */
void
cb_collect_file_names(const char* filename, const char* directory, int depth,
		struct stat* st, void* context);



/***************************************************************************/
/** Tools (in their respective .cpp files)                                **/
/***************************************************************************/

/**
 * List all ancestors of a file
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_ancestors(int argc, char** argv);

/**
 * List all descendants of a file
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_descendants(int argc, char** argv);

/**
 * Disclose provenance
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_disclose(int argc, char** argv);

/**
 * Print information about the object
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_obj_info(int argc, char** argv);

#endif

