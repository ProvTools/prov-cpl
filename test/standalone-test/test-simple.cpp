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
 * Contributor(s): Jackson Okuhn, Peter Macko
 */

#include "stdafx.h"
#include "standalone-test.h"

#include <map>
#include <set>
#include <vector>
#include <unistd.h>


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
	char* s_start_time = ctime(&start_time);
	if (s_start_time[strlen(s_start_time)-1] == '\n') {
		s_start_time[strlen(s_start_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx", info->id);
	print(L_DEBUG, "  MAC Address      : %s", info->mac_address);
	print(L_DEBUG, "  User Name        : %s", info->user);
	print(L_DEBUG, "  PID              : %d", info->pid);
	print(L_DEBUG, "  Program Name     : %s", info->program);
	print(L_DEBUG, "  Command Line     : %s", info->cmdline);
	print(L_DEBUG, "  Start Time       : %s", s_start_time);
}

/**
 * Print the cpl_bundle_info_t structure
 *
 * @param info the info structure
 */
static void
print_bundle_info(cpl_bundle_info_t* info)
{
	time_t creation_time = (time_t) info->creation_time;
	char* s_creation_time = ctime(&creation_time);
	if (s_creation_time[strlen(s_creation_time)-1] == '\n') {
		s_creation_time[strlen(s_creation_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx", info->id);
	print(L_DEBUG, "  Creation Session : %llx", info->creation_session);
	print(L_DEBUG, "  Creation Time    : %s", s_creation_time);
	print(L_DEBUG, "  Name             : %s", info->name);
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
	char* s_creation_time = ctime(&creation_time);
	if (s_creation_time[strlen(s_creation_time)-1] == '\n') {
		s_creation_time[strlen(s_creation_time)-1] = '\0';
	}

	print(L_DEBUG, "  ID               : %llx", info->id);
	print(L_DEBUG, "  Creation Time    : %s", s_creation_time);
	print(L_DEBUG, "  Prefix       : %s", info->prefix);
	print(L_DEBUG, "  Name             : %s", info->name);
	print(L_DEBUG, "  Type             : %i", info->type);
}


/**
 * The iterator callback for cpl_lookup_object_ext() that collects the returned
 * information in an instance of std::map<cpl_id_t, unsigned long>*.
 *
 * @param id the object ID
 * @param @param timestamp the object creation time expressed as UNIX time
 * @param context the pointer to an instance of the map
 * @return CPL_OK or an error code
 */
static cpl_return_t
cb_lookup_objects(const cpl_id_t id,
		  const unsigned long timestamp,
		  void* context)
{
	if (context == NULL) return CPL_E_INVALID_ARGUMENT;

	std::map<cpl_id_t, unsigned long>* m
		= (std::map<cpl_id_t, unsigned long>*) context;
	(*m)[id] = timestamp;

	return CPL_OK;
}

/**
 * The iterator callback for cpl_get_object_relations() that collects
 * the information in an instance of std::set<cpl_id_t>.
 *
 * @param relation_id the ID of the relation
 * @param query_object_id the ID of the object on which we are querying
 * @param other_object_id the ID of the object on the other end of the
 *                        dependency/ancestry edge
 * @param type the type of relation
 * @param context the pointer to an instance of the set
 * @return CPL_OK or an error code
 */
static cpl_return_t
cb_lookup_relations(const cpl_id_t relation_id,
				   const cpl_id_t query_object_id,
				   const cpl_id_t other_object_id,
				   const int type,
				   void* context)
{
	std::set<cpl_id_t>* s
		= (std::set<cpl_id_t>*) context;
	s->insert(relation_id);
	return CPL_OK;
}

static cpl_return_t
cb_collect_object_info_set(const cpl_object_info_t* info,
							      void* context)
{
	std::set<cpl_id_t>* s
		= (std::set<cpl_id_t>*) context;
	s->insert(info->id);
	return CPL_OK;
}
/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param key the property name
 * @param value the property value
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_get_properties(const cpl_id_t id,
				  const char* prefix,
				  const char* key,
				  const char* value,
				  const int type,
				  void* context)
{
	std::multimap<std::string, std::string>* m
		= (std::multimap<std::string, std::string>*) context;
	std::string name = prefix;
	name.append(":");
	name.append(key);
	m->insert(std::pair<std::string, std::string>(name,
				std::string(value)));

	print(L_DEBUG, "  %llx %s:%s = %s",
			id, prefix, key, value);

	return CPL_OK;
}

/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param prefix the namespace prefix
 * @param iri the namespace iri
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_get_prefixes(const cpl_id_t id,
			  	const char* prefix,
			  	const char* iri,
			  	void* context)
{
	std::multimap<std::string, std::string>* m
		= (std::multimap<std::string, std::string>*) context;
	m->insert(std::pair<std::string, std::string>(std::string(prefix),
				std::string(iri)));

	print(L_DEBUG, "  %llx %s = %s",
			id, prefix, iri);

	return CPL_OK;
}
/**
 * The iterator callback function used by property accessors.
 *
 * @param id the object ID
 * @param key the property name
 * @param value the property value
 * @param context the application-provided context
 * @return CPL_OK or an error code (the caller should fail on this error)
 */
static cpl_return_t
cb_lookup_by_property(const cpl_id_t id,
					  const char* prefix,
					  const char* key,
					  const char* value,
					  const int type,
					  void* context)
{
	std::set<cpl_id_t>* s
		= (std::set<cpl_id_t>*) context;
	s->insert(id);
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
 * @return true if it contains the given pair
 */
static bool
contains(std::set<cpl_id_t>& s, const cpl_id_t id)
{
	return s.find(id) != s.end();
}


/**
 * Sleep for a small amount of time (on the order of seconds)
 */
void
delay()
{
	int t = 2;

	usleep(t * 1000000);
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

	cpl_id_t bun  = CPL_NONE;
	cpl_id_t obj1 = CPL_NONE;
	cpl_id_t obj2 = CPL_NONE;
	cpl_id_t obj3 = CPL_NONE;
	cpl_id_t rel1 = CPL_NONE;
	cpl_id_t rel2 = CPL_NONE;
    cpl_id_t rel3 = CPL_NONE;
    cpl_id_t rel4 = CPL_NONE;
    cpl_id_t rel5 = CPL_NONE;

	// Get the current session

	ret = cpl_get_current_session(&session);
	print(L_DEBUG, "cpl_get_current_session --> %llx [%d]",
		  session, ret);
	CPL_VERIFY(cpl_get_current_session, ret);

	print(L_DEBUG, " ");

	if (with_delays) delay();


	// Object creation

	ret = cpl_create_bundle("Bundle", "test", &bun);
	print(L_DEBUG, "cpl_create_bundle --> %llx [%d]", bun, ret);
	CPL_VERIFY(cpl_create_bundle, ret);
	if (with_delays) delay();

	ret = cpl_add_prefix(bun, "test", "test.iri");
	print(L_DEBUG, "cpl_add_prefix --> %d", ret);
	CPL_VERIFY(cpl_add_prefix, ret);
	if (with_delays) delay();

	ret = cpl_create_object("test", "Entity", CPL_ENTITY, &obj1);
	print(L_DEBUG, "cpl_create_object --> %llx [%d]", obj1,ret);
	CPL_VERIFY(cpl_create_object, ret);
	if (with_delays) delay();

	ret = cpl_create_object("test", "Agent", CPL_AGENT, &obj2);
	print(L_DEBUG, "cpl_create_object --> %llx [%d]", obj2,ret);
	CPL_VERIFY(cpl_create_object, ret);
	if (with_delays) delay();

	ret = cpl_lookup_or_create_object("test", "Activity", CPL_ACTIVITY, &obj3);
	print(L_DEBUG, "cpl_lookup_or_create_object --> %llx [%d]",
			obj3,ret);
	CPL_VERIFY(cpl_lookup_or_create_object, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Object lookup

	cpl_id_t objx;

	ret = cpl_lookup_bundle("Bundle", "test", &objx);
	print(L_DEBUG, "cpl_bundle --> %llx [%d]", objx ,ret);
	CPL_VERIFY(cpl_lookup_bundle, ret);
	if (bun!=objx)throw CPLException("Bundle lookup returned the wrong object");
	if (with_delays) delay();

	ret = cpl_lookup_object("test", "Entity", CPL_ENTITY, &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx [%d]", objx ,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj1!=objx)throw CPLException("Object lookup returned the wrong object");
	if (with_delays) delay();

	ret = cpl_lookup_object("test", "Agent", CPL_AGENT, &objx);
	print(L_DEBUG, "cpl_lookup_object --> %llx [%d]", objx,ret);
	CPL_VERIFY(cpl_lookup_object, ret);
	if(obj2!=objx)throw CPLException("Object lookup returned the wrong object");
	if (with_delays) delay();

    std::map<cpl_id_t, unsigned long> ectx;
	ret = cpl_lookup_object_ext("test", "Activity", CPL_ACTIVITY, CPL_L_NO_FAIL,
            cb_lookup_objects, &ectx);
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

	// Relation creation

	ret = cpl_add_relation(obj1, obj2, WASATTRIBUTEDTO, &rel1);
	print(L_DEBUG, "cpl_relation --> %d", ret);
	CPL_VERIFY(cpl_add_relation, ret);
	if (with_delays) delay();

	/*
	ret = cpl_add_relation(obj1, obj2, WASATTRIBUTEDTO, bun, &rel1);
	print(L_DEBUG, "cpl_relation --> %d", ret);
	CPL_VERIFY(cpl_add_relation, ret);
	if (ret != CPL_S_DUPLICATE_IGNORED) {
		throw CPLException("This is a duplicate, but it was not ignored");
	}
	if (with_delays) delay();

	*/

	ret = cpl_add_relation(obj1, obj3, WASGENERATEDBY, &rel2);
	print(L_DEBUG, "cpl_relation --> %d", ret);
	CPL_VERIFY(cpl_add_relation, ret);
	if (with_delays) delay();

	ret = cpl_add_relation(obj3, obj2, WASASSOCIATEDWITH, &rel3);
	print(L_DEBUG, "cpl_relation --> %d", ret);
	CPL_VERIFY(cpl_add_relation, ret);
	if (with_delays) delay();

    ret = cpl_add_relation(bun, rel3, BUNDLERELATION, &rel4);
    print(L_DEBUG, "cpl_relation --> %d", ret);
    CPL_VERIFY(cpl_add_relation, ret);
    if (with_delays) delay();

	ret = cpl_add_relation(bun, rel2, BUNDLERELATION, &rel5);
    print(L_DEBUG, "cpl_relation --> %d", ret);
    CPL_VERIFY(cpl_add_relation, ret);
    if (with_delays) delay();

	print(L_DEBUG, " ");


    //Bundle objects
    std::set<cpl_id_t> rctx;
    ret = cpl_get_bundle_objects(bun, cb_collect_object_info_set, &rctx);
    if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_bundle_objects --> [%d]", ret);
        CPL_VERIFY(cpl_lookup_object, ret);
    }
    if (!contains(rctx, obj1) || !contains(rctx, obj2) || !contains(rctx, obj3)) {
        print(L_DEBUG, "cpl_get_bundle_objects --> not found (%lu results) [%d]",
              ectx.size(), ret);
        throw CPLException("get bundle objects result does not contain the objects");
    }
    print(L_DEBUG, "cpl_get_bundle_objects --> found (%lu results) [%d]",
          rctx.size(), ret);
    if (with_delays) delay();

    print(L_DEBUG, " ");


	// Relation lookup
	rctx.clear();
	ret = cpl_get_object_relations(obj1, CPL_D_ANCESTORS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 2) {
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    if (!contains(rctx, rel1) || !contains(rctx, rel2)) {
        print(L_DEBUG, "cpl_get_object_relations --> not found (%lu results) [%d]",
                rctx.size(), ret);
        throw CPLException("get object relations result does not contain the relations");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();

	rctx.clear();
	ret = cpl_get_object_relations(obj1, CPL_D_DESCENDANTS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 0){
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();


	rctx.clear();
	ret = cpl_get_object_relations(obj2, CPL_D_ANCESTORS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 0){
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();

	rctx.clear();
	ret = cpl_get_object_relations(obj2, CPL_D_DESCENDANTS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 2){
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    if (!contains(rctx, rel1) || !contains(rctx, rel3)) {
        print(L_DEBUG, "cpl_get_object_relations --> not found (%lu results) [%d]",
                rctx.size(), ret);
        throw CPLException("get object relations result does not contain the relations");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();

	rctx.clear();
	ret = cpl_get_object_relations(obj3, CPL_D_DESCENDANTS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 1){
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    if (!contains(rctx, rel2)) {
        print(L_DEBUG, "cpl_get_object_relations --> not found (%lu results) [%d]",
                rctx.size(), ret);
        throw CPLException("get object relations result does not contain the relations");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();

	rctx.clear();
	ret = cpl_get_object_relations(obj3, CPL_D_ANCESTORS, 0, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_object_relations --> [%d]", ret);
        CPL_VERIFY(cpl_get_object_relations, ret);
    }
    if(rctx.size() != 1){
    	print(L_DEBUG, "cpl_get_object_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get object relations result is the wrong size");
    }
    if (!contains(rctx, rel3)) {
	    print(L_DEBUG, "cpl_get_object_relations --> not found (%lu results) [%d]",
	            rctx.size(), ret);
	    throw CPLException("get object relations result does not contain the relations");
    }
    print(L_DEBUG, "cpl_get_object_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();
	rctx.clear();

	//Bundle relations

	ret = cpl_get_bundle_relations(bun, cb_lookup_relations, &rctx);
	if (!CPL_IS_OK(ret)) {
        print(L_DEBUG, "cpl_get_bundle_relations --> [%d]", ret);
        CPL_VERIFY(cpl_lookup_object, ret);
    }
    if(rctx.size() != 2){
    	print(L_DEBUG, "cpl_get_bundle_relations --> wrong size (%lu results) [%d]",
    		rctx.size(), ret);
    	throw CPLException("get bundle relations result is the wrong size");
    }
    if (!contains(rctx, rel2) || !contains(rctx, rel3)) {
        print(L_DEBUG, "cpl_get_bundle_relations --> not found (%lu results) [%d]",
                rctx.size(), ret);
        throw CPLException("get bundle relations result does not contain the relations");
    }
    print(L_DEBUG, "cpl_get_bundle_relations --> found (%lu results) [%d]",
            rctx.size(), ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	// Session info

	if(session){
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
	}


    // Object listing

    std::vector<cplxx_object_info_t> oiv;
    ret = cpl_get_all_objects("test", 0, cpl_cb_collect_object_info_vector, &oiv);
	print(L_DEBUG, "cpl_get_all_objects --> %d objects [%d]",
          (int) oiv.size(), ret);
	CPL_VERIFY(cpl_get_all_objects, ret);
	if (oiv.size() != 1)
	    throw CPLException("Object listing return the wrong number of objects");
	if (oiv[0].id != bun)
	    throw CPLException("Object listing did not return the right object");
	if (with_delays) delay();

	print(L_DEBUG, " ");


	// Bundle and object info

	cpl_bundle_info_t* bun_info = NULL;

	ret = cpl_get_bundle_info(bun, &bun_info);
	print(L_DEBUG, "cpl_get_bundle_info --> %d", ret);
	CPL_VERIFY(cpl_get_bundle_info, ret);
	if (with_delays) delay();

	print_bundle_info(bun_info);
	if (bun_info->id != bun
//			|| bun_info->creation_session != session
//			|| (!with_delays && !check_time(bun_info->creation_time))
			|| strcmp(bun_info->name, "Bundle") != 0) {
		throw CPLException("The returned bundle information is incorrect");
	}

	ret = cpl_free_bundle_info(bun_info);
	CPL_VERIFY(cpl_free_bundle_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	cpl_object_info_t* info = NULL;

	ret = cpl_get_object_info(obj1, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);
	if (with_delays) delay();

    print_object_info(info);
	if (info->id != obj1
//			|| (!with_delays && !check_time(info->creation_time))
			|| strcmp(info->prefix, "test") != 0
			|| strcmp(info->name, "Entity") != 0
			|| info->type != CPL_ENTITY) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	ret = cpl_get_object_info(obj2, &info);
	print(L_DEBUG, "cpl_get_object_info --> %d", ret);
	CPL_VERIFY(cpl_get_object_info, ret);
	if (with_delays) delay();

	print_object_info(info);
	if (info->id != obj2
//			|| (!with_delays && !check_time(info->creation_time))
			|| strcmp(info->prefix, "test") != 0
			|| strcmp(info->name, "Agent") != 0
			|| info->type != CPL_AGENT) {
		throw CPLException("The returned object information is incorrect");
	}

	ret = cpl_free_object_info(info);
	CPL_VERIFY(cpl_free_object_info, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	// Object properties

	ret = cpl_add_object_string_property(obj1, "test", "LABEL", "1");
	print(L_DEBUG, "cpl_add_object_property --> %d", ret);
	CPL_VERIFY(cpl_add_object_property, ret);
	if (with_delays) delay();

	ret = cpl_add_object_numerical_property(obj2, "test", "LABEL", 2.5);
	print(L_DEBUG, "cpl_add_object_property --> %d", ret);
	CPL_VERIFY(cpl_add_object_property, ret);
	if (with_delays) delay();

	ret = cpl_add_object_boolean_property(obj3, "test", "LABEL", true);
	print(L_DEBUG, "cpl_add_object_property --> %d", ret);
	CPL_VERIFY(cpl_add_object_property, ret);
	if (with_delays) delay();

	ret = cpl_add_object_string_property(obj3, "test", "TAG", "Hello");
	print(L_DEBUG, "cpl_add_object_property --> %d", ret);
	CPL_VERIFY(cpl_add_object_property, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	print(L_DEBUG, "Properties of object 3:");

	std::multimap<std::string, std::string> pctx;

	print(L_DEBUG, "All String:");
	pctx.clear();
	ret = cpl_get_object_string_properties(obj3, NULL, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_object_string_properties --> %d", ret);
	CPL_VERIFY(cpl_get_object_string_properties, ret);
	if (!contains(pctx, "test:TAG", "Hello"))
		throw CPLException("The object is missing a property.");
	if (pctx.size() != 1)
		throw CPLException("The object has unexpected properties.");

	print(L_DEBUG, "String test:LABEL:");
	pctx.clear();
	ret = cpl_get_object_string_properties(obj1, "test", "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_object_string_properties --> %d", ret);
	CPL_VERIFY(cpl_get_object_string_properties, ret);
	if (!contains(pctx, "test:LABEL", "1"))
		throw CPLException("The object is missing a property.");
	if (pctx.size() != 1)
		throw CPLException("The object has unexpected properties.");
	if (with_delays) delay();

    print(L_DEBUG, "numerical test:LABEL:");
    pctx.clear();

    ret = cpl_get_object_numerical_properties(obj2, "test", "LABEL",
                                              cb_get_properties, &pctx);
    print(L_DEBUG, "cpl_get_object_numerical_properties --> %d", ret);
    CPL_VERIFY(cpl_get_object_numerical_properties, ret);
    if (!contains(pctx, "test:LABEL", "2.500000"))
        throw CPLException("The object is missing a property.");
    if (pctx.size() != 1)
        throw CPLException("The object has unexpected properties.");
    if (with_delays) delay();
    pctx.clear();

	print(L_DEBUG, "boolean test:LABEL:");
    ret = cpl_get_object_boolean_properties(obj3, "test", NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_object_boolean_properties --> %d", ret);
	CPL_VERIFY(cpl_get_object_boolean_properties, ret);
    if (!contains(pctx, "test:LABEL", "1"))
        throw CPLException("The object is missing a property.");
    if (pctx.size() != 1)
		throw CPLException("The object has unexpected properties.");
	if (with_delays) delay();
    pctx.clear();

    print(L_DEBUG, " ");

    std::set<cpl_id_t> lctx;
	ret = cpl_lookup_object_by_string_property("test", "LABEL", "1",
			cb_lookup_by_property, &lctx);
	print(L_DEBUG, "cpl_lookup_object_by_string_property --> %d", ret);
	CPL_VERIFY(cpl_lookup_object_by_property, ret);
	if (!contains(lctx, obj1))
		throw CPLException("The object is missing in the result set.");
	if (with_delays) delay();

    ret = cpl_lookup_object_by_numerical_property("test", "LABEL", 2.5,
                                               cb_lookup_by_property, &lctx);
    print(L_DEBUG, "cpl_lookup_object_by_string_property --> %d", ret);
    CPL_VERIFY(cpl_lookup_object_by_property, ret);
    if (!contains(lctx, obj2))
        throw CPLException("The object is missing in the result set.");
    if (with_delays) delay();

    ret = cpl_lookup_object_by_boolean_property("test", "LABEL", true,
                                               cb_lookup_by_property, &lctx);
    print(L_DEBUG, "cpl_lookup_object_by_string_property --> %d", ret);
    CPL_VERIFY(cpl_lookup_object_by_property, ret);
    if (!contains(lctx, obj3))
        throw CPLException("The object is missing in the result set.");
    if (with_delays) delay();
	print(L_DEBUG, " ");


	// Relation properties

	ret = cpl_add_relation_string_property(rel1, "test", "LABEL", "1");
	print(L_DEBUG, "cpl_add_relation_property --> %d", ret);
	CPL_VERIFY(cpl_add_relation_property, ret);
	if (with_delays) delay();

	ret = cpl_add_relation_numerical_property(rel2, "test", "LABEL", 2.5);
	print(L_DEBUG, "cpl_add_relation_property --> %d", ret);
	CPL_VERIFY(cpl_add_relation_property, ret);
	if (with_delays) delay();

	ret = cpl_add_relation_boolean_property(rel3, "test", "LABEL", false);
	print(L_DEBUG, "cpl_add_relation_property --> %d", ret);
	CPL_VERIFY(cpl_add_relation_property, ret);
	if (with_delays) delay();

	ret = cpl_add_relation_string_property(rel3, "test", "TAG", "Hello");
	print(L_DEBUG, "cpl_add_relation_property --> %d", ret);
	CPL_VERIFY(cpl_add_relation_property, ret);
	if (with_delays) delay();

	print(L_DEBUG, " ");

	print(L_DEBUG, "Properties of relation 3:");

	print(L_DEBUG, "All:");
	pctx.clear();
	ret = cpl_get_relation_string_properties(rel3, NULL, NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_relation_properties --> %d", ret);
	CPL_VERIFY(cpl_get_relation_properties, ret);
	if (!contains(pctx, "test:TAG", "Hello"))
		throw CPLException("The relation is missing a property.");
	if (pctx.size() != 1)
		throw CPLException("The relation has unexpected properties.");

	print(L_DEBUG, "string test:LABEL:");
	pctx.clear();
	ret = cpl_get_relation_string_properties(rel1, "test", "LABEL",
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_relation_string_properties --> %d", ret);
	CPL_VERIFY(cpl_get_relation_properties, ret);
	if (!contains(pctx, "test:LABEL", "1"))
		throw CPLException("The relation is missing a property.");
	if (pctx.size() != 1)
		throw CPLException("The relation has unexpected properties.");
	if (with_delays) delay();
    pctx.clear();

	print(L_DEBUG, "numerical test:LABEL:");
    ret = cpl_get_relation_numerical_properties(rel2, "test", NULL,
			cb_get_properties, &pctx);
	print(L_DEBUG, "cpl_get_relation_numerical_properties --> %d", ret);
	CPL_VERIFY(cpl_get_relation_properties, ret);
    if (!contains(pctx, "test:LABEL", "2.500000"))
        throw CPLException("The relation is missing a property.");
    if (pctx.size() != 1)
        throw CPLException("The relation has unexpected properties.");
    if (with_delays) delay();
    pctx.clear();

    print(L_DEBUG, "boolean test:LABEL:");
    ret = cpl_get_relation_boolean_properties(rel3, "test", NULL,
                                                cb_get_properties, &pctx);
    print(L_DEBUG, "cpl_get_relation_boolean_properties --> %d", ret);
    CPL_VERIFY(cpl_get_relation_properties, ret);
    if (!contains(pctx, "test:LABEL", "0"))
        throw CPLException("The relation is missing a property.");
    if (pctx.size() != 1)
        throw CPLException("The relation has unexpected properties.");
    if (with_delays) delay();
    pctx.clear();
	print(L_DEBUG, " ");

	print(L_DEBUG, "Bundle prefixes:");
	pctx.clear();
	ret = cpl_get_prefixes(bun, NULL,
			cb_get_prefixes, &pctx);
	print(L_DEBUG, "cpl_get_prefixes --> %d", ret);
	CPL_VERIFY(cpl_get_relation_properties, ret);
	if (!contains(pctx, "test", "test.iri"))
		throw CPLException("The bundle is missing a prefix.");
	if (pctx.size() != 1)
		throw CPLException("The bundle has unexpected properties.");
	if (with_delays) delay();


	cpl_delete_bundle(bun);
	cpl_detach();
}

