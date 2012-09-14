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
#include <set>
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
 * The iterator callback function used by cpl_lookup_object_ext()
 *
 * @param id the object ID
 * @param timestamp the timestamp
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_lookup_object_ext(const cpl_id_t id,
				  	 const unsigned long timestamp,
					 void* context)
{
	std::map<cpl_id_t, unsigned long>* m
		= (std::map<cpl_id_t, unsigned long>*) context;
	(*m)[id] = timestamp;
	return CPL_OK;
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
cb_lookup_by_property(const cpl_id_t id,
				  	  const cpl_version_t version,
					  const char* key,
					  const char* value,
					  void* context)
{
	std::set<std::pair<cpl_id_t, cpl_version_t> >* s
		= (std::set<std::pair<cpl_id_t, cpl_version_t> >*) context;
	s->insert(std::pair<cpl_id_t, cpl_version_t>(id, version));
	return CPL_OK;
}


/**
 * Check the time
 *
 * @param t the time
 * @return true if t is within 30 seconds of now
 */
static bool
check_time(long t)
{
	time_t now = time(NULL);
	return t - 5 <= now && t + 30 >= now;
}


/**
 * Check whether the map contains the given id
 *
 * @param m the map
 * @param id the ID
 * @return true if it contains the given id
 */
static bool
contains(std::map<cpl_id_t, unsigned long>& m, const cpl_id_t id)
{
	return m.find(id) != m.end();
}


/**
 * Check whether the multimap contains the given pair
 *
 * @param m the multimap
 * @param key the key
 * @param value the value
 * @return true if it contains the given pair
 */
static bool
contains(std::multimap<std::string, std::string>& m, const char* key,
		 const char* value)
{
	std::multimap<std::string, std::string>::iterator i;
	for (i = m.find(std::string(key)); i != m.end(); i++) {
		if (i->first != key) return false;
		if (i->second == value) return true;
	}
	return false;
}


/**
 * Check whether the set contains the given pair
 *
 * @param s the set
 * @param id the ID
 * @param version the version
 * @return true if it contains the given pair
 */
static bool
contains(std::set<std::pair<cpl_id_t, cpl_version_t> >& s, const cpl_id_t id,
		 const cpl_version_t version)
{
	return s.find(std::pair<cpl_id_t, cpl_version_t>(id, version)) != s.end();
}


/**
 * Create a random binary file
 *
 * @param size the file size
 * @return the file name
 */
std::string
create_random_file(size_t size=256)
{
#ifdef _WINDOWS
	char _dir[MAX_PATH];
	DWORD r = GetTempPath(MAX_PATH, _dir);
	if (r <= 0 || r > MAX_PATH) {
		throw CPLException("Could not determine the directory for temporary files.");
	}
	char _name[MAX_PATH];
	r = GetTempFileName(_dir, "cpltest", 0, _name);
	char* name = r == 0 ? NULL : _name;
#else
	char _name[L_tmpnam + 4];
	char* name = tmpnam(_name);
#endif
	if (name == NULL) {
		throw CPLException("Could not generate a new name for a temporary file.");
	}

#ifdef _WINDOWS
	FILE* f = NULL;
	errno_t err = fopen_s(&f, name, "wb");
	if (err != 0) f = NULL;
	if (f == NULL) {
		char msg[128];
		strerror_s(msg, sizeof(msg), err);
		throw CPLException("Could not create file %s: %s", name, msg);
	}
#else
	FILE* f = fopen(name, "wb");
	if (f == NULL) {
		throw CPLException("Could not create file: %s", strerror(errno));
	}
#endif

	size_t remaining = size;
	char buffer[256];
	while (remaining > 0) {
		size_t l = remaining;
		if (l > sizeof(buffer)) l = sizeof(buffer);
		remaining -= l;

		for (size_t i = 0; i < l; i++) buffer[i] = rand() & 0xff;
		buffer[l-1]=0;
		size_t r = fwrite(buffer, 1, l, f);
		if (l != r || ferror(f)) {
#ifdef _WINDOWS
			char msg[128];
			strerror_s(msg, sizeof(msg), errno);
			throw CPLException("Could not write to file: %s", msg);
#else
			throw CPLException("Could not write to file: %s", strerror(errno));
#endif
		}
	}

	if (fclose(f) == EOF) {
#ifdef _WINDOWS
		char msg[128];
		strerror_s(msg, sizeof(msg), errno);
		throw CPLException("Could not close file: %s", msg);
#else
		throw CPLException("Could not close file: %s", strerror(errno));
#endif
	}
	return std::string(name);
}


/**
 * Sleep for a small amount of time (on the order of seconds)
 */
void
delay()
{
	int t = 2;

#if defined(_WINDOWS)
	Sleep(t * 1000);
#else
	usleep(t * 1000000);
#endif
}


/**
 * The simplest possible test
 */
void
test_simple(void)
{
	cpl_return_t ret;
	cpl_session_t session;

	bool with_delays = false;

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

	if (with_delays) delay();


	// Object creation

	ret = cpl_create_object(ORIGINATOR, "Process A", "Proc", CPL_NONE, &obj);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj.hi, obj.lo, ret);
	CPL_VERIFY(cpl_create_object, ret);
	if (with_delays) delay();

	ret = cpl_create_object(ORIGINATOR, "Object A", "File", obj, &obj2);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj2.hi,obj2.lo,ret);
	CPL_VERIFY(cpl_create_object, ret);
	if (with_delays) delay();

	ret = cpl_create_object(ORIGINATOR, "Process B", "Proc", obj, &obj3);
	print(L_DEBUG, "cpl_create_object --> %llx:%llx [%d]", obj3.hi,obj3.lo,ret);
	CPL_VERIFY(cpl_create_object, ret);
	if (with_delays) delay();

	ret = cpl_lookup_or_create_object(ORIGINATOR, "Process C", "Proc", CPL_NONE, &obj4);
	print(L_DEBUG, "cpl_lookup_or_create_object --> %llx:%llx [%d]",
			obj4.hi,obj4.lo,ret);
	CPL_VERIFY(cpl_lookup_or_create_object, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Object lookup

	cpl_id_t objx;

	ret = cpl_lookup_object(ORIGINATOR, "Process A", "Proc", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if (obj!=objx)throw CPLException("Object lookup returned the wrong object");
	if (with_delays) delay();

	ret = cpl_lookup_object(ORIGINATOR, "Object A", "File", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj2!=objx)throw CPLException("Object lookup returned the wrong object");
	if (with_delays) delay();

	ret = cpl_lookup_object(ORIGINATOR, "Process B", "Proc", &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx:%llx [%d]", objx.hi,objx.lo,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj3!=objx)throw CPLException("Object lookup returned the wrong object");
	if (with_delays) delay();

    std::map<cpl_id_t, unsigned long> ectx;
	ret = cpl_lookup_object_ext(ORIGINATOR, "Process B", "Proc", CPL_L_NO_FAIL,
            cb_lookup_object_ext, &ectx);
    if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_lookup_object_ext --> [%d]", ret);
        CPL_VERIFY(cpl_lookup_object, ret);
    }
    if (!contains(ectx, obj3)) {
        print(L_DEBUG, "cpl_lookup_object_ext --> not found (%lu results) [%d]",
                ectx.size(), ret);
        throw CPLException("Object lookup result does not contain the object");
    }
    print(L_DEBUG, "cpl_lookup_object_ext --> found (%lu results) [%d]",
            ectx.size(), ret);
    if (!with_delays && !check_time(ectx[obj3])) {
        throw CPLException("The returned timestamp information is incorrect");
    }
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Data and control flow / dependencies

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	print(L_DEBUG, "cpl_data_flow --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);
	if (with_delays) delay();

	ret = cpl_data_flow(obj2, obj, CPL_DATA_INPUT);
	print(L_DEBUG, "cpl_data_flow --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);
	if (ret != CPL_S_DUPLICATE_IGNORED) {
		throw CPLException("This is a duplicate, but it was not ignored");
	}
	if (with_delays) delay();

	ret = cpl_control_flow(obj3, obj, CPL_CONTROL_START);
	print(L_DEBUG, "cpl_control_flow --> %d", ret);
	CPL_VERIFY(cpl_control_flow, ret);
	if (with_delays) delay();

	ret = cpl_data_flow_ext(obj, obj3, 0, CPL_DATA_TRANSLATION);
	print(L_DEBUG, "cpl_data_flow_ext --> %d", ret);
	CPL_VERIFY(cpl_data_flow, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Session info

	cpl_session_info_t* sinfo = NULL;

	ret = cpl_get_session_info(session, &sinfo);
	print(L_DEBUG, "cpl_get_session_info --> %d", ret);
	CPL_VERIFY(cpl_get_session_info, ret);

	print_session_info(sinfo);
	if (sinfo->id != session
			|| (!with_delays && !check_time(sinfo->start_time))) {
		throw CPLException("The returned session information is incorrect");
	}
	if (with_delays) delay();

	ret = cpl_free_session_info(sinfo);
	CPL_VERIFY(cpl_free_session_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");


    // Object listing
    
    std::vector<cplxx_object_info_t> oiv;
    ret = cpl_get_all_objects(0, cpl_cb_collect_object_info_vector, &oiv);
	print(L_DEBUG, "cpl_get_all_objects --> %d objects [%d]",
          (int) oiv.size(), ret);
	CPL_VERIFY(cpl_get_all_objects, ret);
    bool found = false;
    bool found2 = false;
    bool found3 = false;
    bool found4 = false;
    for (size_t i = 0; i < oiv.size(); i++) {
        cplxx_object_info_t& info = oiv[i];
        if (info.id == obj ) found  = true;
        if (info.id == obj2) found2 = true;
        if (info.id == obj3) found3 = true;
        if (info.id == obj4) found4 = true;
        if (i < 10) {
            print(L_DEBUG, "  %s : %s : %s, ver. %d", info.originator.c_str(),
                  info.name.c_str(), info.type.c_str(), info.version);
        }
    }
    if (oiv.size() > 10) print(L_DEBUG, "  ...");
    if (!found )
        throw CPLException("Object listing did not return a certain object");
    if (!found2)
        throw CPLException("Object listing did not return a certain object");
    if (!found3)
        throw CPLException("Object listing did not return a certain object");
    if (!found4)
        throw CPLException("Object listing did not return a certain object");
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Object info

	cpl_object_info_t* info = NULL;
	cpl_version_t version = CPL_VERSION_NONE;

	ret = cpl_get_version(obj, &version);
	print(L_DEBUG, "cpl_get_version --> %d [%d]", version, ret);
	CPL_VERIFY(cpl_get_version, ret);
	if (with_delays) delay();
	cpl_version_t version1 = version;

	ret = cpl_get_object_info(obj, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);
	if (with_delays) delay();

	print_object_info(info);
	if (info->id != obj || info->version != version
			|| info->creation_session != session
			|| (!with_delays && !check_time(info->creation_time))
			|| strcmp(info->originator, ORIGINATOR) != 0
			|| strcmp(info->name, "Process A") != 0
			|| strcmp(info->type, "Proc") != 0
			|| info->container_id != CPL_NONE
			|| info->container_version != CPL_VERSION_NONE) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	ret = cpl_get_version(obj2, &version);
	print(L_DEBUG, "cpl_get_version --> %d [%d]", version, ret);
	CPL_VERIFY(cpl_get_version, ret);
	cpl_version_t version2 = version;
	if (with_delays) delay();

	ret = cpl_get_object_info(obj2, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);
	if (with_delays) delay();

	print_object_info(info);
	if (info->id != obj2 || info->version != version
			|| info->creation_session != session
			|| (!with_delays && !check_time(info->creation_time))
			|| strcmp(info->originator, ORIGINATOR) != 0
			|| strcmp(info->name, "Object A") != 0
			|| strcmp(info->type, "File") != 0
			|| info->container_id != obj
			|| info->container_version != 0) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Version info

	cpl_version_info_t* vinfo = NULL;

	version = version1;
	ret = cpl_get_version_info(obj, version, &vinfo);
	print(L_DEBUG, "cpl_get_version_info --> %d", ret);
	CPL_VERIFY(cpl_get_version_info, ret);
	if (with_delays) delay();

	print_version_info(vinfo);
	if (vinfo->id != obj || vinfo->version != version
			|| vinfo->session != session
			|| (!with_delays && !check_time(vinfo->creation_time))) {
		throw CPLException("The returned version information is incorrect");
	}

	ret = cpl_free_version_info(vinfo);
	CPL_VERIFY(cpl_free_version_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	version = version2;
	ret = cpl_get_version_info(obj2, version, &vinfo);
	print(L_DEBUG, "cpl_get_version_info --> %d", ret);
	CPL_VERIFY(cpl_get_version_info, ret);
	if (with_delays) delay();

	print_version_info(vinfo);
	if (vinfo->id != obj2 || vinfo->version != version
			|| vinfo->session != session
			|| (!with_delays && !check_time(vinfo->creation_time))) {
		throw CPLException("The returned version information is incorrect");
	}

	ret = cpl_free_version_info(vinfo);
	CPL_VERIFY(cpl_free_version_info, ret);
	if (with_delays) delay();

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
	if (with_delays) delay();

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
	if (with_delays) delay();

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
	if (with_delays) delay();

	if (actx.results.size() != 0) throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");

	actx.direction = CPL_D_DESCENDANTS;
	actx.results.clear();
	print(L_DEBUG, "Descendants of version 0 (excluding the next version):");
	ret = cpl_get_object_ancestry(obj, 0, actx.direction,
                                  CPL_A_NO_PREV_NEXT_VERSION,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);
	if (with_delays) delay();

	if (actx.results.size() != 2) throw CPLException("Invalid ancestry");
	if ((actx.results[0].id != obj3 || actx.results[1].id != obj2)
			&& (actx.results[1].id != obj3 || actx.results[0].id != obj2))
		throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");

	actx.direction = CPL_D_DESCENDANTS;
	actx.results.clear();
	print(L_DEBUG, "Descendants of version 0 (including the next version):");
	ret = cpl_get_object_ancestry(obj, 0, actx.direction, 0,
								  cb_object_ancestry, &actx);
	print(L_DEBUG, "cpl_get_object_ancestry --> %d", ret);
	CPL_VERIFY(cpl_get_object_ancestry, ret);
	if (with_delays) delay();

	if (actx.results.size() != 3) throw CPLException("Invalid ancestry");
	if (actx.results[0].id != obj && actx.results[1].id != obj
            && actx.results[2].id != obj) throw CPLException("Invalid ancestry");

	print(L_DEBUG, " ");


	// Properties

	ret = cpl_add_property(obj, "LABEL", "Process A [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (with_delays) delay();

	ret = cpl_add_property(obj2, "LABEL", "Object A [File]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (with_delays) delay();

	ret = cpl_add_property(obj3, "LABEL", "Process B [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (with_delays) delay();

	cpl_version_t obj3pv;
	ret = cpl_get_version(obj3, &obj3pv);
	CPL_VERIFY(cpl_get_version, ret);
	if (with_delays) delay();

	ret = cpl_add_property(obj3, "LABEL", "Yay -- Process B [Proc]");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (with_delays) delay();

	ret = cpl_add_property(obj3, "TAG", "Hello");
	print(L_DEBUG, "cpl_add_property --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	print(L_DEBUG, "Properties of object 3:");

	std::multimap<std::string, std::string> pctx;

	print(L_DEBUG, "All:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (!contains(pctx, "LABEL", "Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (!contains(pctx, "LABEL", "Yay -- Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (!contains(pctx, "TAG", "Hello"))
		throw CPLException("The object is missing a property.");
	if (pctx.size() != 3)
		throw CPLException("The object has unexpected properties.");

	print(L_DEBUG, "All - version %d:", obj3pv);
	pctx.clear();
	ret = cpl_get_properties(obj3, obj3pv, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (!contains(pctx, "LABEL", "Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (with_delays) delay();

	print(L_DEBUG, "LABEL:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (!contains(pctx, "LABEL", "Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (!contains(pctx, "LABEL", "Yay -- Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (pctx.size() != 2)
		throw CPLException("The object has unexpected properties.");
	if (with_delays) delay();

	print(L_DEBUG, "LABEL - version %d:", obj3pv);
	pctx.clear();
	ret = cpl_get_properties(obj3, obj3pv, "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (!contains(pctx, "LABEL", "Process B [Proc]"))
		throw CPLException("The object is missing a property.");
	if (with_delays) delay();

	print(L_DEBUG, "HELLO:");
	pctx.clear();
	ret = cpl_get_properties(obj3, CPL_VERSION_NONE, "HELLO",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (pctx.size() != 0)
		throw CPLException("The object has unexpected properties.");
	if (with_delays) delay();

	print(L_DEBUG, "HELLO - version %d:", obj3pv);
	pctx.clear();
	ret = cpl_get_properties(obj3, obj3pv, "HELLO",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_properties --> %d", ret);
	CPL_VERIFY(cpl_get_properties, ret);
	if (pctx.size() != 0)
		throw CPLException("The object has unexpected properties.");
	if (with_delays) delay();


	print(L_DEBUG, " ");

	std::set<std::pair<cpl_id_t, cpl_version_t> > lctx;
	ret = cpl_lookup_by_property("LABEL", "Process B [Proc]",
			cb_lookup_by_property, &lctx);
	print(L_DEBUG, "cpl_lookup_by_property --> %d", ret);
	CPL_VERIFY(cpl_lookup_by_property, ret);
	if (!contains(lctx, obj3, obj3pv))
		throw CPLException("The object is missing in the result set.");
	if (with_delays) delay();

	print(L_DEBUG, " ");


	/*
	 * Creating new versions
	 */

	cpl_version_t objv, vx;

	ret = cpl_get_version(obj, &objv);
	CPL_VERIFY(cpl_get_version, ret);
	if (with_delays) delay();

	ret = cpl_new_version(obj, &vx);
	print(L_DEBUG, "cpl_new_version --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (objv + 1 != vx)
		throw CPLException("The version number did not increase by 1.");
	objv = vx;
	if (with_delays) delay();

	ret = cpl_new_version(obj, &vx);
	print(L_DEBUG, "cpl_new_version --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (objv + 1 != vx)
		throw CPLException("The version number did not increase by 1.");
	objv = vx;
	if (with_delays) delay();

	ret = cpl_new_version(obj, &vx);
	print(L_DEBUG, "cpl_new_version --> %d", ret);
	CPL_VERIFY(cpl_add_property, ret);
	if (objv + 1 != vx)
		throw CPLException("The version number did not increase by 1.");
	objv = vx;
	if (with_delays) delay();

	print(L_DEBUG, " ");


    /*
     * File API
     */

    cpl_id_t f1id;
    cpl_version_t f1v;
	std::string f1n = create_random_file();
    ret = cpl_lookup_file(f1n.c_str(),
            CPL_F_OPEN_BY_CONTENT | CPL_F_CREATE_IF_DOES_NOT_EXIST,
            &f1id, &f1v);
	print(L_DEBUG, "cpl_lookup_file --> %llx:%llx-%d [%d]",
            f1id.hi, f1id.lo, f1v, ret);
	CPL_VERIFY(cpl_lookup_file, ret);
	if (with_delays) delay();

    cpl_id_t f2id;
    cpl_version_t f2v;
	std::string f2n = create_random_file();
    ret = cpl_lookup_file(f2n.c_str(),
            CPL_F_OPEN_BY_CONTENT | CPL_F_ALWAYS_CREATE,
            &f2id, &f2v);
	print(L_DEBUG, "cpl_lookup_file --> %llx:%llx-%d [%d]",
            f2id.hi, f2id.lo, f1v, ret);
	CPL_VERIFY(cpl_lookup_file, ret);
	if (with_delays) delay();

    cpl_id_t f3id;
    cpl_version_t f3v;
	std::string f3n = create_random_file();
    ret = cpl_lookup_file(f3n.c_str(),
            CPL_F_CREATE_IF_DOES_NOT_EXIST,
            &f3id, &f3v);
	print(L_DEBUG, "cpl_lookup_file --> %llx:%llx-%d [%d]",
            f3id.hi, f3id.lo, f1v, ret);
	CPL_VERIFY(cpl_lookup_file, ret);
	if (with_delays) delay();

	std::string f0n = "/tmp/*hello*!@#$%";
    ret = cpl_lookup_file(f0n.c_str(), 0, NULL, NULL);
	print(L_DEBUG, "cpl_lookup_file --> [%d] (should fail)", ret);
	if (CPL_IS_OK(ret)) {
		throw CPLException("The function call was expected to fail");
	}
	if (ret != CPL_E_NOT_FOUND) {
		CPL_VERIFY(cpl_lookup_file, ret);
	}
	if (with_delays) delay();

	f0n = create_random_file();
    ret = cpl_lookup_file(f0n.c_str(), 0, NULL, NULL);
	print(L_DEBUG, "cpl_lookup_file --> [%d] (should fail)", ret);
#ifdef _WINDOWS
	_unlink(f0n.c_str());
#else
	unlink(f0n.c_str());
#endif
	if (CPL_IS_OK(ret)) {
		throw CPLException("The function call was expected to fail");
	}
	if (ret != CPL_E_NOT_FOUND) {
		CPL_VERIFY(cpl_lookup_file, ret);
	}
	if (with_delays) delay();


#ifdef _WINDOWS
	_unlink(f1n.c_str());
	_unlink(f2n.c_str());
	_unlink(f3n.c_str());
#else
	unlink(f1n.c_str());
	unlink(f2n.c_str());
	unlink(f3n.c_str());
#endif

	print(L_DEBUG, " ");
}

