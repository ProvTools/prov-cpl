/*
 * test-simple.cpp
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

#include <map>
#include <vector>


using namespace std;


/**
 * Print the cpl_session_info_t structure
 *
 * @param info the info structure
 */
static void
print_session_info(cpl_session_info_t* info)
{
	time_t start_time = (time_t) info->start_time;
#ifdef _WINDOWS
	char s_start_time[64];
	ctime_s(s_start_time, sizeof(s_start_time), &start_time);
#else
	char* s_start_time = ctime(&start_time);
#endif
	if (s_start_time[strlen(s_start_time)-1] == '\n') {
		s_start_time[strlen(s_start_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx:%llx", info->id.hi, info->id.lo);
	print(L_DEBUG, "  MAC Address      : %s", info->mac_address);
	print(L_DEBUG, "  User Name        : %s", info->user);
	print(L_DEBUG, "  PID              : %d", info->pid);
	print(L_DEBUG, "  Program Name     : %s", info->program);
	print(L_DEBUG, "  Command Line     : %s", info->cmdline);
	print(L_DEBUG, "  Start Time       : %s", s_start_time);
}


/**
 * Print the cpl_object_info_t structure
 *
 * @param info the info structure
 */
static void
print_object_info(cpl_object_info_t* info)
{
	time_t creation_time = (time_t) info->creation_time;
#ifdef _WINDOWS
	char s_creation_time[64];
	ctime_s(s_creation_time, sizeof(s_creation_time), &creation_time);
#else
	char* s_creation_time = ctime(&creation_time);
#endif
	if (s_creation_time[strlen(s_creation_time)-1] == '\n') {
		s_creation_time[strlen(s_creation_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx:%llx", info->id.hi, info->id.lo);
	print(L_DEBUG, "  Version          : %d", info->version);
	print(L_DEBUG, "  Creation Session : %llx:%llx", info->creation_session.hi,
			                                         info->creation_session.lo);
	print(L_DEBUG, "  Creation Time    : %s", s_creation_time);
	print(L_DEBUG, "  Originator       : %s", info->originator);
	print(L_DEBUG, "  Name             : %s", info->name);
	print(L_DEBUG, "  Type             : %s", info->type);
	print(L_DEBUG, "  Container ID     : %llx:%llx", info->container_id.hi,
			                                         info->container_id.lo);
	print(L_DEBUG, "  Container Version: %d", info->container_version);
}


/**
 * Print the cpl_version_info_t structure
 *
 * @param info the info structure
 */
static void
print_version_info(cpl_version_info_t* info)
{
	time_t creation_time = (time_t) info->creation_time;
#ifdef _WINDOWS
	char s_creation_time[64];
	ctime_s(s_creation_time, sizeof(s_creation_time), &creation_time);
#else
	char* s_creation_time = ctime(&creation_time);
#endif
	if (s_creation_time[strlen(s_creation_time)-1] == '\n') {
		s_creation_time[strlen(s_creation_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx:%llx", info->id.hi, info->id.lo);
	print(L_DEBUG, "  Version          : %d", info->version);
	print(L_DEBUG, "  Session          : %llx:%llx", info->session.hi,
			                                         info->session.lo);
	print(L_DEBUG, "  Creation Time    : %s", s_creation_time);
}


/**
 * A side of a dependecy edge + the dependency type
 */
typedef struct cpl_id_ver_type {
	cpl_id_t id;
	cpl_version_t version;
	int type;
} cpl_id_ver_type_t;


/**
 * The context type for cb_object_ancestry()
 */
typedef struct cb_object_ancestry_context {
	int direction;
	vector<cpl_id_ver_type_t> results;
} cb_object_ancestry_context_t;


/**
 * The iterator callback function used by cpl_get_object_ancestry().
 *
 * @param query_object_id the ID of the object on which we are querying
 * @param query_object_verson the version of the queried object
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param other_object_version the version of the other object
 * @param type the type of the data or the control dependency
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_object_ancestry(const cpl_id_t query_object_id,
				   const cpl_version_t query_object_version,
				   const cpl_id_t other_object_id,
				   const cpl_version_t other_object_version,
				   const int type,
				   void* context)
{
	cb_object_ancestry_context_t* ctx = (cb_object_ancestry_context_t*) context; 
	int direction = ctx->direction;

	cpl_id_ver_type_t x;
	x.id = other_object_id;
	x.version = other_object_version;
	x.type = type;
	ctx->results.push_back(x);

	print(L_DEBUG, "  %llx:%llx-%d %c--%c %llx:%llx-%d  Type: %d:%02d",
			query_object_id.hi, query_object_id.lo, query_object_version,
			direction == CPL_D_ANCESTORS ? '-' : '<',
			direction == CPL_D_ANCESTORS ? '>' : '-',
			other_object_id.hi, other_object_id.lo, other_object_version,
			CPL_GET_DEPENDENCY_CATEGORY(type), type & 255);

	return CPL_OK;
}


/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param version the object version
 * @param key the property name
 * @param value the property value
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_get_properties(const cpl_id_t id,
				  const cpl_version_t version,
				  const char* key,
				  const char* value,
				  void* context)
{
	std::multimap<std::string, std::string>* m
		= (std::multimap<std::string, std::string>*) context;
	m->insert(std::pair<std::string, std::string>(std::string(key),
				std::string(value)));
	
	print(L_DEBUG, "  %llx:%llx-%d %s = %s",
			id.hi, id.lo, version, key, value);

	return CPL_OK;
}


/**
 * Check the time
 *
 * @param t the time
 * @return true if t is within 10 seconds of now
 */
static bool
check_time(long t) {

	long now = time(NULL);
	return t <= now && t + 10 >= now;
}


/**
 * The simplest possible test
 */
void
test_simple(void)
{
	cpl_return_t ret;
	cpl_session_t session;

	cpl_id_t obj  = CPL_NONE;
	cpl_id_t obj2 = CPL_NONE;
	cpl_id_t obj3 = CPL_NONE;
	cpl_id_t obj4 = CPL_NONE;


	// Get the current session

	ret = cpl_get_current_session(&session);
	print(L_DEBUG, "cpl_get_current_session --> %llx:%llx [%d]",
		  session.hi, session.lo, ret);
	CPL_VERIFY(cpl_get_current_session, ret);

	print(L_DEBUG, " ");


	// Object creation

	ret = cpl_create_object(ORIGINATOR, "Process A", "Proc", CPL_NONE, &obj);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj.hi, obj.lo, ret);
	CPL_VERIFY(cpl_create_object, ret);

	ret = cpl_create_object(ORIGINATOR, "Object A", "File", obj, &obj2);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj2.hi,obj2.lo,ret);
	CPL_VERIFY(cpl_create_object, ret);

	ret = cpl_create_object(ORIGINATOR, "Process B", "Proc", obj, &obj3);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj3.hi,obj3.lo,ret);
	CPL_VERIFY(cpl_create_object, ret);

	ret = cpl_lookup_or_create_object(ORIGINATOR, "Process C", "Proc", CPL_NONE, &obj4);
	print(L_DEBUG, "cpl_lookup_or_create_object --> %llx:%llx [%d]",
			obj4.hi,obj4.lo,ret);
	CPL_VERIFY(cpl_lookup_or_create_object, ret);

	print(L_DEBUG, " ");


	// Object lookup

	cpl_id_t objx;

	ret = cpl_lookup_object(ORIGINATOR, "Process A", "Proc", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if (obj!=objx)throw CPLException("Object lookup returned the wrong object");

	ret = cpl_lookup_object(ORIGINATOR, "Object A", "File", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj2!=objx)throw CPLException("Object lookup returned the wrong object");

	ret = cpl_lookup_object(ORIGINATOR, "Process B", "Proc", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj3!=objx)throw CPLException("Object lookup returned the wrong object");

	print(L_DEBUG, " ");


	// Data and control flow / dependencies

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	print(L_DEBUG, "cpl_data_flow --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	print(L_DEBUG, "cpl_data_flow --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);

	ret = cpl_control(obj3, obj, CPL_CONTROL_START);
	print(L_DEBUG, "cpl_control --> %d", ret);
	CPL_VERIFY(cpl_control, ret);

	ret = cpl_data_flow_ext(obj, obj3, 0, CPL_DATA_TRANSLATION);
	print(L_DEBUG, "cpl_data_flow_ext --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);

	print(L_DEBUG, " ");


	// Session info (assume that the session started less than 10 sec. ago)

	cpl_session_info_t* sinfo = NULL;

	ret = cpl_get_session_info(session, &sinfo);
	print(L_DEBUG, "cpl_get_session_info --> %d", ret);
	CPL_VERIFY(cpl_get_session_info, ret);

	print_session_info(sinfo);
	if (sinfo->id != session
			|| !check_time(sinfo->start_time)) {
		throw CPLException("The returned session information is incorrect");
	}

	ret = cpl_free_session_info(sinfo);
	CPL_VERIFY(cpl_free_session_info, ret);

	print(L_DEBUG, " ");


	// Object info (assume that the objects were created less than 10 sec. ago)

	cpl_object_info_t* info = NULL;
	cpl_version_t version = CPL_VERSION_NONE;

	ret = cpl_get_version(obj, &version);
	print(L_DEBUG, "cpl_get_version --> %d [%d]", version, ret);
	CPL_VERIFY(cpl_get_version, ret);
	cpl_version_t version1 = version;

	ret = cpl_get_object_info(obj, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);

	print_object_info(info);
	if (info->id != obj || info->version != version
			|| info->creation_session != session
			|| !check_time(info->creation_time)
			|| strcmp(info->originator, ORIGINATOR) != 0
			|| strcmp(info->name, "Process A") != 0
			|| strcmp(info->type, "Proc") != 0
			|| info->container_id != CPL_NONE
			|| info->container_version != CPL_VERSION_NONE) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);

	print(L_DEBUG, " ");

	ret = cpl_get_version(obj2, &version);
	print(L_DEBUG, "cpl_get_version --> %d [%d]", version, ret);
	CPL_VERIFY(cpl_get_version, ret);
	cpl_version_t version2 = version;

	ret = cpl_get_object_info(obj2, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);

	print_object_info(info);
	if (info->id != obj2 || info->version != version
			|| info->creation_session != session
			|| !check_time(info->creation_time)
			|| strcmp(info->originator, ORIGINATOR) != 0
			|| strcmp(info->name, "Object A") != 0
			|| strcmp(info->type, "File") != 0
			|| info->container_id != obj
			|| info->container_version != 0) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);

	print(L_DEBUG, " ");


	// Version info

	cpl_version_info_t* vinfo = NULL;

	version = version1;
	ret = cpl_get_version_info(obj, version, &vinfo);
	print(L_DEBUG, "cpl_get_version_info --> %d", ret);
	CPL_VERIFY(cpl_get_version_info, ret);

	print_version_info(vinfo);
	if (vinfo->id != obj || vinfo->version != version
			|| vinfo->session != session
			|| !check_time(vinfo->creation_time)) {
		throw CPLException("The returned version information is incorrect");
	}

	ret = cpl_free_version_info(vinfo);
	CPL_VERIFY(cpl_free_version_info, ret);

	print(L_DEBUG, " ");

	version = version2;
	ret = cpl_get_version_info(obj2, version, &vinfo);
	print(L_DEBUG, "cpl_get_version_info --> %d", ret);
	CPL_VERIFY(cpl_get_version_info, ret);

	print_version_info(vinfo);
	if (vinfo->id != obj2 || vinfo->version != version
			|| vinfo->session != session
			|| !check_time(vinfo->creation_time)) {
		throw CPLException("The returned version information is incorrect");
	}

	ret = cpl_free_version_info(vinfo);
	CPL_VERIFY(cpl_free_version_info, ret);

	print(L_DEBUG, " ");


	// Ancestry (all checks assume the cycle avoidance algorithm)

	cb_object_ancestry_context_t actx;

	actx.direction = CPL_D_ANCESTORS;
	actx.results.clear();
	print(L_DEBUG, "Ancestors:");
	ret = cpl_get_object_ancestry(obj, CPL_VERSION_NONE, actx.direction, 0,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);

	if (actx.results.size() != 1) throw CPLException("Invalid ancestry");
	if (actx.results[0].id != obj3) throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");

	actx.direction = CPL_D_DESCENDANTS;
	actx.results.clear();
	print(L_DEBUG, "Descendants:");
	ret = cpl_get_object_ancestry(obj, CPL_VERSION_NONE, actx.direction, 0,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);

	if (actx.results.size() != 2) throw CPLException("Invalid ancestry");
	if ((actx.results[0].id != obj3 || actx.results[1].id != obj2)
			&& (actx.results[1].id != obj3 || actx.results[0].id != obj2))
		throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");

	actx.direction = CPL_D_ANCESTORS;
	actx.results.clear();
	print(L_DEBUG, "Ancestors of version 0:");
	ret = cpl_get_object_ancestry(obj, 0, actx.direction, 0,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);

	if (actx.results.size() != 0) throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");

	actx.direction = CPL_D_DESCENDANTS;
	actx.results.clear();
	print(L_DEBUG, "Descendants of version 0:");
	ret = cpl_get_object_ancestry(obj, 0, actx.direction, 0,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);

	if (actx.results.size() != 2) throw CPLException("Invalid ancestry");
	if ((actx.results[0].id != obj3 || actx.results[1].id != obj2)
			&& (actx.results[1].id != obj3 || actx.results[0].id != obj2))
		throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");


	// Properties

	ret = cpl_add_property(obj, "LABEL", "Process A [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);

	ret = cpl_add_property(obj2, "LABEL", "Object A [File]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);

	ret = cpl_add_property(obj3, "LABEL", "Process B [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);

	ret = cpl_add_property(obj3, "LABEL", "Yay -- Process B [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);

	ret = cpl_add_property(obj3, "TAG", "Hello");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);

	print(L_DEBUG, " ");

	print(L_DEBUG, "Properties of object 3:");

	std::multimap<std::string, std::string> pctx;

	print(L_DEBUG, "All:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);

	print(L_DEBUG, "All - version 1:");
	pctx.clear();
	ret = cpl_get_properties(obj3, 1, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);

	print(L_DEBUG, "LABEL:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);

	print(L_DEBUG, "LABEL - version 2:");
	pctx.clear();
	ret = cpl_get_properties(obj3, 2, "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);

	print(L_DEBUG, "HELLO:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, "HELLO",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);

	print(L_DEBUG, "HELLO - version 1:");
	pctx.clear();
	ret = cpl_get_properties(obj3, 1, "HELLO",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
}

