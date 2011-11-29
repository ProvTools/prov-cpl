/*
 * test-stress.cpp
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

#include <private/cpl-platform.h>

#include <vector>

using namespace std;


/**
 * The object types
 */
static const char* TYPES[] = {
	"File", "Proc",
};


/**
 * The number of types
 */
#define NUM_TYPES	2


/**
 * Determine whether the type is a process
 *
 * @param t the string type
 * @return true if it is a process
 */
#define IS_PROCESS(t) ((t)[0] == 'P')


/**
 * Struct for the objects that we created
 */
typedef struct object {
	cpl_id_t id;
	cpl_version_t last_known_version;
	int type_id;
	size_t logical_id;
#ifdef _WINDOWS
	SSIZE_T container_logical_id;
#else
	ssize_t container_logical_id;
#endif
} object_t;


/**
 * The vector of all objects that we created
 */
static vector<object_t> objects;


/**
 * The lock for appending to the objects vector
 */
static Mutex objects_mutex;


/**
 * Generate a random number between 0 and 1
 *
 * @return a random number
 */
static inline double
rand_double(void) {
	return rand() / (double) RAND_MAX;
}


/**
 * Create a random object
 *
 * @param p_container the probability of assigning a container
 * @return the object ID in the objects array (the same as logical_id)
 */
static size_t
create_random_object(double p_container)
{
	object_t obj;
	memset(&obj, 0, sizeof(obj));

	objects_mutex.Lock();

	obj.type_id = rand() % NUM_TYPES;
	obj.logical_id = objects.size();
	obj.container_logical_id = objects.empty() || rand_double() >= p_container
									? -1 : rand() % objects.size();
	
	char name[64];
#ifdef _WINDOWS
	sprintf_s(name, 64,
#else
	snprintf(name, 64,
#endif
		"Object %lu", obj.logical_id);

	cpl_return_t ret = cpl_create_object(ORIGINATOR, name, TYPES[obj.type_id],
		obj.container_logical_id < 0 ? CPL_NONE
			: objects[obj.container_logical_id].id,
		&obj.id);

	print(L_DEBUG, "cpl_create_object(\"%s\", \"%s\", %ld) --> %lu",
			name, TYPES[obj.type_id], obj.container_logical_id,
			obj.logical_id);
	if (CPL_IS_OK(ret)) objects.push_back(obj);

	objects_mutex.Unlock();

	CPL_VERIFY(cpl_create_object, ret);

	return obj.logical_id;
}


/**
 * The parameterized stress test
 *
 * @param num_precreate the number of objects to create before starting
 * @param num_operations the number of operations to perform
 * @param p_create the probability of creating a new object
 * @param p_lookup the probability of looking up an object
 * @param p_container the probability of assigning a container
 * @param p_dependency_ext the probability to use the _ext function when
 *                         disclosing a data or a control dependency
 */
void
parameterized_test_stress(size_t num_precreate, size_t num_operations,
		double p_create, double p_lookup, double p_container,
		double p_dependency_ext)
{
	cpl_return_t ret;


	// Precreate the given number of objects

	if (num_precreate < 2) {
		throw CPLException("The smallest number of objects that need to be "
				"precreated is 2");
	}

	for (size_t i = 0; i < num_precreate; i++) {
		create_random_object(p_container);
	}


	// The main loop

	for (size_t op_i = 0; op_i < num_operations; op_i++) {
		double r_op = rand_double();


		// Operation: create an object

		r_op -= p_create;
		if (r_op < 0) {
			create_random_object(p_container);
			continue;
		}


		// Operation: look up an object

		r_op -= p_lookup;
		if (r_op < 0) {
			
			char name[64];
			cpl_id_t id;

			size_t obj = rand() % objects.size();
#ifdef _WINDOWS
			sprintf_s(name, 64,
#else
			snprintf(name, 64,
#endif
				"Object %lu", obj);

			ret = cpl_lookup_object(ORIGINATOR, name,
					TYPES[objects[obj].type_id], &id);

			print(L_DEBUG, "cpl_lookup_object(\"%s\", \"%s\")",
					name, TYPES[objects[obj].type_id]);

			if (CPL_IS_OK(ret) && cpl_id_cmp(&id, &objects[obj].id) != 0) {
				ret = CPL_E_NOT_FOUND;
			}

			CPL_VERIFY(cpl_lookup_object, ret);
			continue;
		}


		// Operation: create a dependency (self-loops are okay, although not
		// desired)

		size_t obj_dest = 0;
		size_t obj_src = 0;
		int retries = 2;

		while (obj_dest == obj_src && retries --> 0) {
			obj_dest = rand() % objects.size();
			obj_src  = rand() % objects.size();
		}

		bool r_data = IS_PROCESS(TYPES[objects[obj_dest].type_id])
			&& IS_PROCESS(TYPES[objects[obj_dest].type_id])
			? rand_double() < 0.5 : true;

		bool use_ext = rand_double() < p_dependency_ext;

		if (use_ext) {
			ret = cpl_get_version(objects[obj_src].id,
					&objects[obj_src].last_known_version);
			CPL_VERIFY(cpl_get_version, ret);
			cpl_version_t ver = rand()%(1+objects[obj_src].last_known_version);

			if (r_data) {
				ret = cpl_data_flow_ext(objects[obj_dest].id,
						objects[obj_src].id, ver, CPL_DATA_INPUT);
				print(L_DEBUG, "cpl_data_flow_ext(%lu, %lu, %d)",
						obj_dest, obj_src, ver);
				CPL_VERIFY(cpl_data_flow_ext, ret);
			}
			else {
				ret = cpl_control_ext(objects[obj_dest].id,
						objects[obj_src].id, ver, CPL_CONTROL_OP);
				print(L_DEBUG, "cpl_control_ext(%lu, %lu, %d)",
						obj_dest, obj_src, ver);
				CPL_VERIFY(cpl_control_ext, ret);
			}
		}
		else {
			if (r_data) {
				ret = cpl_data_flow(objects[obj_dest].id,
						objects[obj_src].id, CPL_DATA_INPUT);
				print(L_DEBUG, "cpl_data_flow(%lu, %lu)", obj_dest, obj_src);
				CPL_VERIFY(cpl_data_flow, ret);
			}
			else {
				ret = cpl_control(objects[obj_dest].id,
						objects[obj_src].id, CPL_CONTROL_OP);
				print(L_DEBUG, "cpl_control(%lu, %lu)", obj_dest, obj_src);
				CPL_VERIFY(cpl_control, ret);
			}
		}
	}
}


/**
 * The mini stress test
 */
void
test_mini_stress(void)
{
	parameterized_test_stress(10, 1000, 0.1, 0.2, 0.2, 0.25);
}

