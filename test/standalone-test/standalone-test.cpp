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

#define ORIGINATOR "standalone-test"


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

	
	// Initialize

	cpl_db_backend_t* backend;
	cpl_return_t ret;

#ifndef _WINDOWS
	if (strcmp(backend_type, "RDF") == 0 || strcmp(backend_type, "rdf") == 0) {
		ret = cpl_create_rdf_backend("http://localhost:8080/sparql/",
									 "http://localhost:8080/update/",
									 CPL_RDF_4STORE,
									 &backend);
		if (!CPL_IS_OK(ret)) {
			fprintf(stderr, "Could not open the SPARQL connection\n");
			return 1;
		}
	}
	else
#endif
	if (strcmp(backend_type, "ODBC") == 0
			|| strcmp(backend_type, "odbc") == 0) {
		ret = cpl_create_odbc_backend("DSN=CPL;UID=cpl;PWD=cplcplcpl;",
									  CPL_ODBC_MYSQL,
									  &backend);
		if (!CPL_IS_OK(ret)) {
			fprintf(stderr, "Could not open the ODBC connection\n");
			return 1;
		}
	}
	else {
		fprintf(stderr, "Invalid database backend type: %s\n", backend_type);
	}

	CPL_InitializationHelper __cpl(backend); (void) __cpl;


	// Yay

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

