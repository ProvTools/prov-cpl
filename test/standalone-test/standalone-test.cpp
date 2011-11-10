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

#include <backends/cpl-odbc.h>
#include <backends/cpl-rdf.h>
#include <cpl.h>

#include <getopt_compat.h>

#define ORIGINATOR "standalone-test"


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
 * Long command-line options
 */
static struct option LONG_OPTIONS[] =
{
	{"help",                 no_argument,       0, 'h'},
	{"odbc",                 required_argument, 0,  0 },
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
	P("Usage: %s [OPTIONS]", program_name);
	P(" ");
	P("Options:");
	P("  -h, --help               Print this message and exit");
	P("  --db-type DATABASE_TYPE  Specify the database type (MySQL, Jena,...)");
	P("  --odbc DSN|CONNECT_STR   Use an ODBC connection");
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

	set_program_name(argv[0]);


	// Parse the command-line arguments

	try {
		int c, option_index = 0;
		while ((c = getopt_long(argc, argv, "h",
								LONG_OPTIONS, &option_index)) >= 0) {

			switch (c) {

			case 0:
				if (strcmp(LONG_OPTIONS[option_index].name, "odbc") == 0) {
					backend_type = "ODBC";
					odbc_connection_string = optarg;
				}
				if (strcmp(LONG_OPTIONS[option_index].name, "db-type") == 0) {
					db_type = optarg;
				}
				break;

			case 'h':
				usage();
				return 0;

			case '?':
			case ':':
				// getopt_long already printed an error message
				return 1;

			default:
				abort();
			}
		}


		// Check the non-option arguments

		if (optind != argc) {
			throw CPLException("Too many arguments; please use -h for help.");
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
			if (type == CPL_ODBC_UNKNOWN) {
				fprintf(stderr, "%s: Warning: The database type is not set; "
						"please use the --db-type option\n", program_name);
			}


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
			if (type == CPL_RDF_UNKNOWN) {
				fprintf(stderr, "%s: Warning: The database type is not set; "
						"please use the --db-type option\n", program_name);
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


	// Yay

	cpl_return_t ret;
	cpl_id_t obj  = CPL_NONE;
	cpl_id_t obj2 = CPL_NONE;
	cpl_id_t obj3 = CPL_NONE;

	ret = cpl_create_object(ORIGINATOR, "Process A", "Proc", CPL_NONE, &obj);
	printf("cpl_create_object --> %lld [%d]\n", obj, ret);

	ret = cpl_lookup_object(ORIGINATOR, "Process A", "Proc", &obj);
	printf("cpl_lookup_object --> %lld [%d]\n", obj, ret);

	ret = cpl_create_object(ORIGINATOR, "Object A", "File", obj, &obj2);
	printf("cpl_create_object --> %lld [%d]\n", obj2, ret);

	ret = cpl_create_object(ORIGINATOR, "Process B", "Proc", obj, &obj3);
	printf("cpl_create_object --> %lld [%d]\n", obj3, ret);

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	printf("cpl_data_flow --> %d\n", ret);

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	printf("cpl_data_flow --> %d\n", ret);

	ret = cpl_control(obj3, obj, CPL_CONTROL_START);
	printf("cpl_control --> %d\n", ret);

	ret = cpl_data_flow(obj, obj3, CPL_DATA_INPUT);
	printf("cpl_data_flow --> %d\n", ret);

	return 0;
}

