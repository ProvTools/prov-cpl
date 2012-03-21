/*
 * common.cpp
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

#include "stdafx.h"
#include "cpl-tool.h"

#include <list>

#include <sys/stat.h>
#include <dirent.h>

using namespace std;


/**
 * Private data for process_recursively()
 */
typedef struct process_recursively_private
{
	bool include_dir;
	bool recursive;
	process_recursively_callback_t callback;
	void* context;
} process_recursively_private_t;


/**
 * Recursively traverse the given file or directory
 *
 * @param filename the name of a file or directory
 * @param directory the directory with a trailing / (empty string = current)
 * @param depth the directory depth (0 = current)
 * @param p shared private data
 */
void
process_recursively_worker(const char* filename, const char* directory,
		int depth, process_recursively_private_t* p)
{
	if (*filename == '\0') return;


	// Stat the file

	struct stat st;
	if (stat(filename, &st) != 0) {
		throw CPLException("Cannot stat \"%s\" -- %s",
				filename, strerror(errno));
	}


	// Handle directories

	if (S_ISDIR(st.st_mode)) {
		
		if (p->include_dir) {
			p->callback(filename, directory, depth, &st, p->context);
		}

		if (!p->recursive) {
			if (!p->include_dir) {
				fprintf(stderr, "%s %s: \"%s\" is a directory (skipped).\n",
						program_name, tool_name, filename);
			}
			return;
		}


		// Process the directory recursively

		DIR *dp;
		struct dirent *dirp;
		if ((dp = opendir(filename)) == NULL) {
			throw CPLException("Cannot list \"%s\" -- %s",
					filename, strerror(errno));
		}

		std::list<std::string> files;
		while ((dirp = readdir(dp)) != NULL) {
			std::string s = std::string(dirp->d_name);
			if (s == "." || s == "..") continue;
			files.push_back(s);
		}
		closedir(dp);

		std::string prefix = filename;
		if (filename[strlen(filename)-1] != '/') prefix += "/";

		for (std::list<std::string>::iterator i = files.begin();
				i != files.end(); i++) {
			std::string s = prefix + *i;
			process_recursively_worker(s.c_str(), prefix.c_str(), depth + 1, p);
		}

		return;
	}


	// Callback

	p->callback(filename, directory, depth, &st, p->context);
}


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
		process_recursively_callback_t callback, void* context)
{
	process_recursively_private_t p;
	p.include_dir = include_dir;
	p.recursive = recursive;
	p.callback = callback;
	p.context = context;
	process_recursively_worker(filename, "", 0, &p);
}


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
		struct stat* st, void* context)
{
	std::list<std::string>* p = (std::list<std::string>*) context;
	p->push_back(std::string(filename));
}

