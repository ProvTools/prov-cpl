/*
 * tool-ancestry.cpp
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
 * Whether to include session info
 */
static bool include_session = false;


/**
 * Session cache - type
 */
typedef cpl_hash_map_id_t<cpl_session_info*>::type session_map_t;


/**
 * Session cache - type
 */
static session_map_t session_map;


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
	P("Usage: %s %s [OPTIONS] SOURCE_FILE...",
			program_name, tool_name);
	P(" ");
	P("Options:");
	P("  -h, --help               Print this message and exit");
	P("  -R, -r, --recursive      Traverse the directories recursively");
	P("  -v, --verbose            Enable verbose mode");
#undef P
}


/** 
 * Custom atexit() handler
 */
static void
cb_atexit(void)
{
	// Cleanup
	
	for (session_map_t::iterator i = session_map.begin();
			i != session_map.end(); i++) {
		cpl_free_session_info(i->second);
	}
}


/**
 * Print a single level of provenance for the given file
 * 
 * @param filename the file name
 * @param direction CPL_D_ANCESTORS or CPL_D_DESCENDANTS
 */
void
print_provenance(const char* filename, int direction)
{
	printf("%s\n", filename);


	// Lookup the object and query the provenance
	
	cpl_id_t id;
	cpl_return_t ret = cpl_lookup_file(filename, 0, &id, NULL);
	if (ret == CPL_E_NOT_FOUND) return;
	if (!CPL_IS_OK(ret)) {
		throw CPLException("Could not look up the provenance of \"%s\" -- %s",
				filename, cpl_error_string(ret));
	}
	
	std::list<cpl_ancestry_entry_t> l;
	ret = cpl_get_object_ancestry(id, CPL_VERSION_NONE, direction, 0,
			cpl_cb_collect_ancestry_list, &l);
	if (!CPL_IS_OK(ret)) {
		throw CPLException("Could not look up the %s of \"%s\" -- %s",
				direction == CPL_D_ANCESTORS ? "ancestors" : "descendants",
				filename, cpl_error_string(ret));
	}


	// Process each ancestry entry
	
	for (std::list<cpl_ancestry_entry_t>::iterator next = l.begin();
			next != l.end();) {
		std::list<cpl_ancestry_entry_t>::iterator i = next++;


		// Get the object info

		cpl_object_info_t* info;
		ret = cpl_get_object_info(i->other_object_id, &info);
		if (!CPL_IS_OK(ret)) {
			throw CPLException("Could not lookup one of the %s of \"%s\" -- %s",
					direction == CPL_D_ANCESTORS ? "ancestors" : "descendants",
					filename, cpl_error_string(ret));
		}


		// Get the session info (if needed)

		cpl_session_info_t* session = NULL;
		if (include_session) {

			cpl_version_info_t* vinfo;
			ret = cpl_get_version_info(i->query_object_id,
					i->query_object_version, &vinfo);
			if (!CPL_IS_OK(ret)) {
				cpl_free_object_info(info);
				throw CPLException("Could not lookup the version info of "
						"\"%s\" -- %s", filename, cpl_error_string(ret));
			}
			cpl_session_t sid = vinfo->session;
			cpl_free_version_info(vinfo);

			session_map_t::iterator j = session_map.find(sid);
			if (j != session_map.end()) {
				session = j->second;
			}
			else {
				ret = cpl_get_session_info(sid, &session);
				if (!CPL_IS_OK(ret)) {
					throw CPLException("Could not lookup the session info of "
							"\"%s\" -- %s", filename, cpl_error_string(ret));
				}
				session_map[sid] = session;
			}
		}


		// Print

		printf(" %s%s%s%s ",
				termcap_ac_start,
				next == l.end() ? termcap_left_bottom_corner : termcap_left_tee,
				termcap_horizontal_line,
				termcap_ac_end);

		if (strcmp(info->originator, CPL_O_FILESYSTEM) == 0) {
			printf("%s", info->name);
			if (strcmp(info->type, CPL_T_FILE) != 0) {
				printf(" [%s]", info->type);
			}
		}
		else {
			printf("%s :: %s [%s]", info->originator, info->name, info->type);
		}

		printf(", ver. %d", i->other_object_version);

		if (include_session) {
			printf(", by %s", session->cmdline);
		}

		printf("\n");
		
		cpl_free_object_info(info);
	}
}


/**
 * List all ancestors or descendants of a file
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @param direction CPL_D_ANCESTORS or CPL_D_DESCENDANTS
 * @return the exit code
 */
int
tool_ancestry(int argc, char** argv, int direction)
{
	atexit(cb_atexit);


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

	if (verbose) {
		include_session = true;
	}


	// Check to see if we have input files

	if (optind >= argc) {
		usage();
		return 1;
	}


	// Iterate over the source arguments and collect the list of files
	
	std::list<std::string> files;

	for (int i = optind; i < argc; i++) {
		process_recursively(argv[i], false, recursive, cb_collect_file_names,
				&files);
	}

	if (files.empty()) return 0;


	// Collect and print provenance for each file
	
	for (std::list<std::string>::iterator i = files.begin();
			i != files.end(); i++) {
		print_provenance(i->c_str(), direction);
	}

	return 0;
}


/**
 * List all ancestors of a file
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_ancestors(int argc, char** argv)
{
	return tool_ancestry(argc, argv, CPL_D_ANCESTORS);
}


/**
 * List all descendants of a file
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
tool_descendants(int argc, char** argv)
{
	return tool_ancestry(argc, argv, CPL_D_DESCENDANTS);
}

