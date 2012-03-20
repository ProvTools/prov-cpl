/*
 * tool-disclose.cpp
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

#include <getopt_compat.h>
#include <list>
#include <vector>

#include <sys/stat.h>
#include <dirent.h>

using namespace std;


/**
 * Recursive mode
 */
static bool recursive = false;


/**
 * Verbose mode
 */
static bool verbose = false;


/**
 * Short command-line options
 */
static const char* SHORT_OPTIONS = "hRrv";


/**
 * Long command-line options
 */
static struct option LONG_OPTIONS[] =
{
	{"help",                 no_argument,       0, 'h'},
	{"recursive",            no_argument,       0, 'R'},
	{"verbose",              no_argument,       0, 'v'},
	{0, 0, 0, 0}
};


/**
 * Print the usage information
 */
static void
usage(void)
{
#define P(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
	P("Usage: %s %s [OPTIONS] SOURCE_FILE... TARGET_FILE",
			program_name, tool_name);
	P(" ");
	P("Options:");
	P("  -h, --help               Print this message and exit");
	P("  -R, -r, --recursive      Traverse directories recursively");
	P("  -v, --verbose            Enable verbose mode");
#undef P
}


/**
 * Disclose data or control flow from the source to the target
 *
 * @param source the source
 * @param target the target name
 * @param target_id the target object ID
 */
static void
process(const char* source, const char* target, const cpl_id_t target_id)
{
	if (*source == '\0') return;


	// Stat the source

	struct stat st;
	if (stat(source, &st) != 0) {
		throw CPLException("Cannot stat \"%s\" -- %s", source, strerror(errno));
	}


	// Handle directories

	if (S_ISDIR(st.st_mode)) {
		if (!recursive) {
			fprintf(stderr, "%s %s: \"%s\" is a directory (skipped).\n",
					program_name, tool_name, source);
			return;
		}


		// Process the directory recursively

		DIR *dp;
		struct dirent *dirp;
		if ((dp = opendir(source)) == NULL) {
			throw CPLException("Cannot list \"%s\" -- %s", source, strerror(errno));
		}

		std::list<std::string> files;
		while ((dirp = readdir(dp)) != NULL) {
			std::string s = std::string(dirp->d_name);
			if (s == "." || s == "..") continue;
			files.push_back(s);
		}
		closedir(dp);

		std::string prefix = source;
		if (source[strlen(source)-1] != '/') prefix += "/";

		for (std::list<std::string>::iterator i = files.begin();
				i != files.end(); i++) {
			std::string s = prefix + *i;
			process(s.c_str(), target, target_id);
		}

		return;
	}


	// Get the file object

	cpl_id_t source_id;
	cpl_return_t ret = cpl_lookup_file(source, CPL_F_CREATE_IF_DOES_NOT_EXIST,
			&source_id, NULL);
	if (!CPL_IS_OK(ret)) {
		throw CPLException("Cannot lookup or create a provenance object for "
				"\"%s\" -- %s", source, cpl_error_string(ret));
	}

	if (source_id == target_id) {
		fprintf(stderr, "%s %s: \"%s\" is the target (skipped).\n",
				program_name, tool_name, source);
		return;
	}


	// Disclose the provenance

	if (verbose) {
		// Note: The provenance edges are in the data flow direction
		printf("\"%s\" --> \"%s\"\n", source, target);
	}

	// TODO The type should be configurable
	ret = cpl_data_flow(target_id, source_id, CPL_DATA_INPUT);
	if (!CPL_IS_OK(ret)) {
		throw CPLException("Cannot disclose provenance for source "
				"\"%s\" -- %s", source, cpl_error_string(ret));
	}
}


/**
 * Disclose provenance
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_disclose(int argc, char** argv)
{
	// Parse the command-line arguments

	int c, option_index = 0;
	while ((c = getopt_long(argc, argv, SHORT_OPTIONS,
							LONG_OPTIONS, &option_index)) >= 0) {

		switch (c) {

		case 'h':
			usage();
			return 0;

		case 'r':
		case 'R':
			recursive = true;
			break;

		case 'v':
			verbose = true;
			break;

		case '?':
		case ':':
			// getopt_long already printed an error message
			return 1;

		default:
			abort();
		}
	}


	// Check to see if we have input and output files

	if (optind + 1 >= argc) {
		usage();
		return 1;
	}


	// Get the target

	const char* target = argv[argc-1];
	struct stat target_st;
	if (stat(target, &target_st) != 0) {
		if (errno == ENOENT) {
			throw CPLException("Target \"%s\" does not exist", target);
		}
		else {
			throw CPLException("Target \"%s\" is not accessible -- %s",
					target, strerror(errno));
		}
	}

	/*if (!S_ISREG(target_st.st_mode) && !S_ISLNK(target_st.st_mode)) {
		throw CPLException("Target \"%s\" is not a regular file or a symbolic link",
				target);
	}*/

	if (S_ISDIR(target_st.st_mode)) {
		throw CPLException("Target \"%s\" is a directory.", target);
	}

	cpl_id_t target_id;
	cpl_return_t ret = cpl_lookup_file(target, CPL_F_CREATE_IF_DOES_NOT_EXIST,
			&target_id, NULL);
	if (!CPL_IS_OK(ret)) {
		throw CPLException("Cannot lookup or create a provenance object for "
				"target \"%s\" -- %s", target, cpl_error_string(ret));
	}


	// Iterate over the source arguments and perform the operation

	for (int i = optind; i < argc-1; i++) {
		process(argv[i], target, target_id);
	}

	return 0;
}

