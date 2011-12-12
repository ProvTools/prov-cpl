/*
 * standalone-test.cpp
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
#include "standalone-test.h"

#include <backends/cpl-odbc.h>
#include <backends/cpl-rdf.h>
#include <getopt_compat.h>

#include <vector>

#ifdef __APPLE__
#include <libgen.h>
#endif

#ifndef _WINDOWS
#include <sys/time.h>
#endif


/**
 * The program base name
 */
const char* program_name = NULL;
char __program_name[2048];


/**
 * strcasecmp() for Windows
 */
#ifdef _WINDOWS
#define strcasecmp		lstrcmpiA
#endif


/**
 * Verbose mode
 */
static bool verbose = false;


/**
 * Tests
 */
static struct test_info TESTS[] =
{
	{"Simple",       "The Simplest Test",                test_simple       },
	{"Mini-Stress",  "The Mini Stress Test",             test_mini_stress  },
	{0, 0, 0}
};


/**
 * Long command-line options
 */
static struct option LONG_OPTIONS[] =
{
	{"help",                 no_argument,       0, 'h'},
	{"verbose",              no_argument,       0, 'v'},
	{"odbc",                 required_argument, 0,  0 },
	{"rdf",                  no_argument,       0,  0 },
	{"db-type",              required_argument, 0,  0 },
	{0, 0, 0, 0}
};


/**
 * Print the usage information
 */
void
usage(void)
{
#define P(...) { fprintf(stderr, __VA_ARGS__); fputc('\n', stderr); }
	P("Usage: %s [OPTIONS] [TEST [TEST...]]", program_name);
	P(" ");
	P("Options:");
	P("  -h, --help               Print this message and exit");
	P("  -v, --verbose            Enable the verbose mode");
	P("  --db-type DATABASE_TYPE  Specify the database type (MySQL, Jena,...)");
	P("  --odbc DSN|CONNECT_STR   Use an ODBC connection");
	P(" ");
	P("Tests:");
	for (const struct test_info* t = TESTS; t->name != NULL; t++) {
		P("  %-24s %s", t->name, t->description);
	}
#undef P
}


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
 * Print the verbose version of the test header
 *
 * @param out the output file
 * @param test the test struct
 */
void
print_verbose_test_header(FILE* out, const struct test_info* test)
{
	char header[80];
	size_t name_start = 11;

	size_t name_len = strlen(test->name);
	if (name_len > 32) name_len = 32;
	
	for (unsigned u = 0; u < 78; u++) header[u] = '-';
	
	memcpy(header + 4, " Test: ", 7);
	memcpy(header + name_start, test->name, name_len);
	header[name_start + name_len] = ' ';
	header[78] = '\0';

	fprintf(out, "\n%s\n", header);
}


/**
 * Get the current system time in seconds
 *
 * @return the current time in seconds
 */
double
current_time_seconds(void)
{
#ifdef _WINDOWS

	// TODO Implement the Windows version
	return time(NULL);

#else
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec + tv.tv_usec / 1000000.0;
#endif
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
	const char* db_type = "";

	std::vector<const struct test_info*> tests;

	set_program_name(argv[0]);
	srand((unsigned int) time(NULL));


	// Parse the command-line arguments

	try {
		int c, option_index = 0;
		while ((c = getopt_long(argc, argv, "hv",
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
				if (strcmp(LONG_OPTIONS[option_index].name, "db-type") == 0) {
					db_type = optarg;
				}
				break;

			case 'h':
				usage();
				return 0;

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


		// Parse the tests

		for (int i = optind; i < argc; i++) {
			const char* s = argv[i];
			bool ok = false;
			for (const struct test_info* t = TESTS; t->name != NULL; t++) {
				if (strcasecmp(s, t->name) == 0) {
					tests.push_back(t);
					ok = true;
					break;
				}
			}
			if (!ok) {
				throw CPLException("Invalid test: \"%s\"", s);
			}
		}

		if (tests.empty()) {
			for (const struct test_info* t = TESTS; t->name != NULL; t++) {
				tests.push_back(t);
			}
		}
	}
	catch (std::exception& e) {
		fprintf(stderr, "%s: %s\n", program_name, e.what());
		return 1;
	}


	// Set the output level

	set_output_level(verbose ? L_MAX : 0);

	
	// Create the database backend

	cpl_db_backend_t* backend = NULL;

	try {
		cpl_return_t ret;


		// ODBC

		if (strcasecmp(backend_type, "ODBC") == 0) {

			std::string dsn;
			std::string conn;
			int type = CPL_ODBC_GENERIC;


			// Determine the DB type

#define MATCH_DB_TYPE(x, c) if (strcasecmp(db_type, x) == 0) type = c;
			
			MATCH_DB_TYPE("MySQL", CPL_ODBC_MYSQL);
			MATCH_DB_TYPE("PostgreSQL", CPL_ODBC_POSTGRESQL);
			MATCH_DB_TYPE("Postgres", CPL_ODBC_POSTGRESQL);

			if (strcmp(db_type, "") != 0 && type == CPL_ODBC_UNKNOWN) {
				throw CPLException("Unsupported relational database: %s",
						db_type);
			}
			/*if (type == CPL_ODBC_UNKNOWN) {
				fprintf(stderr, "%s: Warning: The database type is not set; "
						"please use the --db-type option\n", program_name);
			}*/


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

			ret = cpl_create_odbc_backend(conn.c_str(), type, &backend);
			if (!CPL_IS_OK(ret)) {
				throw CPLException("Could not open the ODBC connection");
			}
		}


		// RDF/SPARQL (currently *nix-only)

#ifndef _WINDOWS
		else if (strcasecmp(backend_type, "RDF") == 0) {


			// Determine the database type

			int type = CPL_RDF_UNKNOWN;
			
			MATCH_DB_TYPE("4store", CPL_RDF_4STORE);
			MATCH_DB_TYPE("Jena", CPL_RDF_JENA);

			if (strcmp(db_type, "") != 0 && type == CPL_RDF_UNKNOWN) {
				throw CPLException("Unsupported RDF database: %s",
						db_type);
			}


			// Open the database connection

			ret = cpl_create_rdf_backend("http://localhost:8080/sparql/",
										 "http://localhost:8080/update/",
										 type,
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


	// Run the tests

	for (unsigned test_i = 0; test_i < tests.size(); test_i++) {
		const struct test_info* test = tests[test_i];

		try {

			clear_buffer();


			// Print the test header

			if (verbose) {
				print_verbose_test_header(stdout, test);
			}
			else {
				fprintf(stdout, "Test: %s -- ", test->name);
				fflush(stdout);
			}


			// Run the test

			double start_time = current_time_seconds();

			test->func();

			double end_time = current_time_seconds();
			double test_time = end_time - start_time;


			// Print the success message

			char str_test_time[64];
			sprintf(str_test_time, "%ld min %.2lf sec",
					((long) test_time) / 60,
					test_time - 60 * (((long) test_time) / 60));

			if (verbose) {
				fprintf(stdout, "[SUCCESS] Time: %s\n", str_test_time);
			}
			else {
				fprintf(stdout, "Success (%s)\n", str_test_time);
				fflush(stdout);
			}
		}
		catch (std::exception& e) {
			if (!verbose) {
				fprintf(stdout, "Failed\n");
				fflush(stdout);
				print_verbose_test_header(stdout, test);
				print_buffer(stdout);
			}
			fprintf(stdout, "[FAILED] %s\n\n", e.what());
			return 1;
		}
	}

	if (verbose) fputc('\n', stdout);

	return 0;
}

