/*
 * main.cpp
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

#include <backends/cpl-odbc.h>
#include <backends/cpl-rdf.h>
#include <getopt_compat.h>

#ifdef __APPLE__
#include <libgen.h>
#endif

#include <curses.h>
#include <term.h>
#include <termcap.h>


/**
 * The program base name
 */
const char* program_name = NULL;
char __program_name[2048];


/**
 * The tool name
 */
const char* tool_name = NULL;


/**
 * strcasecmp() for Windows
 */
#ifdef _WINDOWS
#define strcasecmp		lstrcmpiA
#endif


/**
 * Tools
 */
static struct tool_info TOOLS[] =
{
	{"ancestors",    "List all ancestors of a file",     tool_ancestors    },
	//{"copy",         "Copy one or more files",           NULL              },
	{"descendants",  "List all descendants of a file",   tool_descendants  },
	{"disclose",     "Disclose data or control flow",    tool_disclose     },
	//{"move",         "Move one or more files",           NULL              },
	{0, 0, 0}
};


/**
 * Short command-line options
 */
static const char* SHORT_OPTIONS = "hV";


/**
 * Long command-line options
 */
static struct option LONG_OPTIONS[] =
{
	{"help",                 no_argument,       0, 'h'},
	{"version",              no_argument,       0, 'V'},
	{"odbc",                 required_argument, 0,  0 },
	{"rdf",                  no_argument,       0,  0 },
	{0, 0, 0, 0}
};


/**
 * Termcap variables
 */

/// Alternative character set start
const char* termcap_ac_start = "";

/// Alternative character set end
const char* termcap_ac_end = "";

/// Vertical line
const char* termcap_vertical_line = "|";

/// Horizontal line
const char* termcap_horizontal_line = "-";

/// Left tee
const char* termcap_left_tee = "+";

/// Left bottom corner
const char* termcap_left_bottom_corner = "`";

/// Right tee
const char* termcap_right_tee = "+";

/// Right upper corner
const char* termcap_right_upper_corner = "`";


/**
 * Set the program name
 *
 * @param name the program name (does not need to be the base-name)
 */
void
set_program_name(const char* name)
{
#ifdef _WINDOWS
	_splitpath_s(name, NULL, 0, NULL, 0, __program_name,
		sizeof(__program_name) / sizeof(char) - 1, NULL, 0);
	program_name = __program_name;
#else
	char* n = strdup(name);
	strncpy(__program_name, n, sizeof(__program_name) / sizeof(char) - 1);
	__program_name[sizeof(__program_name) / sizeof(char) - 1] = '\0';

	program_name = basename(__program_name);
	free(n);
#endif
}


/**
 * Print the usage information
 */
static void
usage(void)
{
#define P(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
	P("Usage: %s [OPTIONS] COMMAND [ARG...]", program_name);
	P(" ");
	P("Options:");
	P("  -h, --help               Print this message and exit");
	P("  -V, --version            Print the CPL version and exist");
	P("  --odbc DSN|CONNECT_STR   Use an ODBC connection");
	P(" ");
	P("Commands:");
	for (const struct tool_info* t = TOOLS; t->name != NULL; t++) {
		P("  %-24s %s", t->name, t->description);
	}
#undef P
}


/**
 * The main function
 *
 * @param argc the number of command-line arguments
 * @param argv the vector of command-line arguments
 * @return the exit code
 */
int
main(int argc, char** argv)
{
	const char* backend_type = "ODBC";
	const char* odbc_connection_string = "CPL";
	const tool_info* tool = NULL;

	set_program_name(argv[0]);
	srand((unsigned int) time(NULL));


	// Initialize the termcap
	
	char termcap_buf[4096];
	char termcap_chr[256];

	if (tgetent(termcap_buf, getenv("TERM")) >= 0) {
		char* tp = termcap_chr;
		termcap_ac_start = tgetstr("as", &tp);
		termcap_ac_end = tgetstr("ae", &tp);
		termcap_vertical_line = "x";
		termcap_horizontal_line = "q";
		termcap_left_tee = "t";
		termcap_left_bottom_corner = "m";
		termcap_right_tee = "u";
		termcap_right_upper_corner = "k";
	}


	// Determine the number of CPL-specific arguments

	int cpl_argc = 1;
	while (cpl_argc < argc) {
		char* s = argv[cpl_argc];
		if (*s == '\0') {
			cpl_argc++;
		}
		else if (*s == '-') {
			cpl_argc++;
			if (*(s + 1) == '-') {
				if (*(s + 2) != '\0') {
					for (struct option* p = LONG_OPTIONS; p->name != NULL; p++){
						if (strcmp(p->name, s + 2) == 0) {
							switch (p->has_arg) {
								case no_argument: break;
								case required_argument: cpl_argc++; break;
								case optional_argument:
									if (cpl_argc < argc) {
										if (argv[cpl_argc][0] != '-') {
											cpl_argc++;
										}
									}
									break;
								default: assert(0);
							}
						}
					}
				}
				else {
					break;
				}
			}
			else if (*(s + 1) != '\0') {
				const char* x = strchr(SHORT_OPTIONS, *(s+1));
				if (x != NULL) if (*(x+1) == ':') cpl_argc++;
			}
		}
		else {
			break;
		}
	}


	// Parse the command-line arguments

	try {
		int c, option_index = 0;
		while ((c = getopt_long(cpl_argc, argv, SHORT_OPTIONS,
								LONG_OPTIONS, &option_index)) >= 0) {

			switch (c) {

			case 0:
				if (strcmp(LONG_OPTIONS[option_index].name, "odbc") == 0) {
					backend_type = "ODBC";
					odbc_connection_string = optarg;
				}
				if (strcmp(LONG_OPTIONS[option_index].name, "rdf") == 0) {
					backend_type = "RDF";
				}
				break;

			case 'h':
				usage();
				return 0;

			case 'V':
				fprintf(stderr, "Core Provenance Library ver. %s\n",
						CPL_VERSION_STR);
				return 0;

			case '?':
			case ':':
				// getopt_long already printed an error message
				return 1;

			default:
				abort();
			}
		}


		// Get the tool

		if (cpl_argc >= argc) {
			usage();
			return 1;
		}

		tool = NULL;
		for (const struct tool_info* t = TOOLS; t->name != NULL; t++) {
			if (strcasecmp(argv[cpl_argc], t->name) == 0) {
				tool = t;
				tool_name = t->name;
				break;
			}
		}
		if (tool == NULL) {
			throw CPLException("Invalid tool: \"%s\"", argv[cpl_argc]);
		}
	}
	catch (std::exception& e) {
		fprintf(stderr, "%s: %s\n", program_name, e.what());
		return 1;
	}

	
	// Create the database backend

	cpl_db_backend_t* backend = NULL;

	try {
		cpl_return_t ret;


		// ODBC

		if (strcasecmp(backend_type, "ODBC") == 0) {

			std::string dsn;
			std::string conn;


			// Get the connection string

			if (strchr(odbc_connection_string, '=') == NULL) {
				if (strchr(odbc_connection_string, ';') != NULL
						|| strchr(odbc_connection_string, '{') != NULL
						|| strchr(odbc_connection_string, '}') != NULL) {
					throw CPLException("Invalid ODBC DSN");
				}

				dsn = odbc_connection_string;
				conn = "DSN="; conn += dsn; conn += "";
			}
			else {
				conn = odbc_connection_string;
				dsn = "";
			}


			// Open the ODBC connection

			ret = cpl_create_odbc_backend(conn.c_str(), CPL_ODBC_GENERIC,
					&backend);
			if (!CPL_IS_OK(ret)) {
				throw CPLException("Could not open the ODBC connection");
			}
		}


		// RDF/SPARQL (currently *nix-only)

#ifndef _WINDOWS
		else if (strcasecmp(backend_type, "RDF") == 0) {

			// Open the database connection

			ret = cpl_create_rdf_backend("http://localhost:8080/sparql/",
										 "http://localhost:8080/update/",
										 CPL_RDF_GENERIC,
										 &backend);
			if (!CPL_IS_OK(ret)) {
				throw CPLException("Could not open the SPARQL connection");
			}
		}
#endif

		// Handle errors

		else if (strcmp(backend_type, "") == 0) {
			throw CPLException("No database connection has been specified");
		}

		else {
			throw CPLException("Invalid database backend type: %s",
							   backend_type);
		}

		
		assert(backend != NULL);
	}
	catch (std::exception& e) {
		fprintf(stderr, "%s: %s\n", program_name, e.what());
		if (backend != NULL) backend->cpl_db_destroy(backend);
		return 1;
	}


	// Initialize the CPL

	try {

		cpl_return_t ret = cpl_attach(backend);
		if (!CPL_IS_OK(ret)) {
			throw CPLException("Failed to initialize the Core Provenance "
					"Library");
		}
	}
	catch (std::exception& e) {
		fprintf(stderr, "%s: %s\n", program_name, e.what());
		if (backend != NULL) backend->cpl_db_destroy(backend);
		return 1;
	}


	// Automatically close the CPL connection when the program ends

	CPLInitializationHelper __cpl(NULL); (void) __cpl;


	// Run the tool

	int r = -1;

	try {
		r = tool->func(argc - cpl_argc, argv + cpl_argc);
	}
	catch (std::exception& e) {
		fprintf(stderr, "%s %s: %s\n", program_name, tool_name, e.what());
		return 1;
	}

	return r;
}

