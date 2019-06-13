/*
 * cpl.i
 * Prov-CPL
 *
 * Copyright 2016
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

 
%module CPLDirect

%include cpointer.i
/*%include std_list.i*/
%include std_vector.i
%include std_string.i
%include std_pair.i

/*
 * Additional code for the C wrapper
 */

%{
#include <cpl.h>
#include <cplxx.h>

#include <backends/cpl-odbc.h>

typedef cpl_db_backend_t* p_cpl_db_backend_t;
typedef cpl_session_info_t* p_cpl_session_info_t;
typedef cpl_object_info_t* p_cpl_object_info_t;
typedef cpl_bundle_info_t* p_cpl_bundle_info_t;

inline _cpl_db_backend_t*
cpl_dereference_pp_cpl_db_backend_t(p_cpl_db_backend_t* p) {
    return *p;
}

inline cpl_session_info_t*
cpl_dereference_pp_cpl_session_info_t(p_cpl_session_info_t* p) {
    return *p;
}

inline cpl_object_info_t*
cpl_dereference_pp_cpl_object_info_t(p_cpl_object_info_t* p) {
    return *p;
}

inline cpl_bundle_info_t*
cpl_dereference_pp_cpl_bundle_info_t(p_cpl_bundle_info_t* p) {
    return *p;
}

inline cpl_session_info_t**
cpl_convert_pp_cpl_session_info_t(p_cpl_session_info_t* p) {
    return p;
}

inline cpl_object_info_t**
cpl_convert_pp_cpl_object_info_t(p_cpl_object_info_t* p) {
    return p;
}

inline cpl_bundle_info_t**
cpl_convert_pp_cpl_bundle_info_t(p_cpl_bundle_info_t* p) {
    return p;
}

inline int
cpl_is_ok(cpl_return_t ret) {
    return CPL_IS_OK(ret);
}

typedef std::vector<cpl_id_timestamp_t> std_vector_cpl_id_timestamp_t;


inline std::vector<cpl_id_timestamp_t>&
cpl_dereference_p_std_vector_cpl_id_timestamp_t(
        std_vector_cpl_id_timestamp_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(
        std_vector_cpl_id_timestamp_t* p) {
    return (void*) p;
}

/*typedef std::list<cpl_relation_t> std_list_cpl_relation_t;*/
typedef std::vector<cpl_relation_t> std_vector_cpl_relation_t;

inline std::vector<cpl_relation_t>&
cpl_dereference_p_std_vector_cpl_relation_t(
        std_vector_cpl_relation_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cpl_relation_t_to_p_void(
        std_vector_cpl_relation_t* p) {
    return (void*) p;
}


typedef std::vector<cplxx_object_info_t> std_vector_cplxx_object_info_t;

inline std::vector<cplxx_object_info_t>&
cpl_dereference_p_std_vector_cplxx_object_info_t(
        std_vector_cplxx_object_info_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(
        std_vector_cplxx_object_info_t* p) {
    return (void*) p;
}

typedef std::vector<cplxx_property_entry_t> std_vector_cplxx_property_entry_t;

inline std::vector<cplxx_property_entry_t>&
cpl_dereference_p_std_vector_cplxx_property_entry_t(
        std_vector_cplxx_property_entry_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(
        std_vector_cplxx_property_entry_t* p) {
    return (void*) p;
}

typedef std::vector<cplxx_prefix_entry_t> std_vector_cplxx_prefix_entry_t;

inline std::vector<cplxx_prefix_entry_t>&
cpl_dereference_p_std_vector_cplxx_prefix_entry_t(
        std_vector_cplxx_prefix_entry_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cplxx_prefix_entry_t_to_p_void(
        std_vector_cplxx_prefix_entry_t* p) {
    return (void*) p;
}

typedef std::vector<cpl_id_t> std_vector_cpl_id_t;

inline std::vector<cpl_id_t>&
cpl_dereference_p_std_vector_cpl_id_t(
        std_vector_cpl_id_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cpl_id_t_to_p_void(
        std_vector_cpl_id_t* p) {
    return (void*) p;
}

%}

/*
 * Special functions - workarounds for SWIG limitations
 */

inline _cpl_db_backend_t*
cpl_dereference_pp_cpl_db_backend_t(p_cpl_db_backend_t* p);

inline cpl_session_info_t*
cpl_dereference_pp_cpl_session_info_t(p_cpl_session_info_t* p);

inline cpl_object_info_t*
cpl_dereference_pp_cpl_object_info_t(p_cpl_object_info_t* p);

inline cpl_bundle_info_t*
cpl_dereference_pp_cpl_bundle_info_t(p_cpl_bundle_info_t* p);

inline cpl_session_info_t**
cpl_convert_pp_cpl_session_info_t(p_cpl_session_info_t* p);

inline cpl_object_info_t**
cpl_convert_pp_cpl_object_info_t(p_cpl_object_info_t* p);

inline cpl_bundle_info_t**
cpl_convert_pp_cpl_bundle_info_t(p_cpl_bundle_info_t* p);

inline int
cpl_is_ok(cpl_return_t ret);


/*
 * Include the CPL header files
 */

%include "../../../include/cpl.h"
%include "../../../include/cplxx.h"

%include "../../../include/backends/cpl-odbc.h"

/*
 * cpl_id_t/std::string output parameter workarounds
 */

%include typemaps.i

cpl_return_t
cpl_create_bundle(const char* name,
                  const char* prefix,
                  unsigned long long* OUTPUT);
cpl_return_t
cpl_lookup_bundle(const char* name,
                  const char* prefix,
                  unsigned long long* OUTPUT);

cpl_return_t
cpl_lookup_relation(const cpl_id_t from_id,
                    const cpl_id_t to_id,
                    const long type,
                    unsigned long long* OUTPUT);

cpl_return_t
cpl_lookup_object_property_wildcard(const char* value,
                    unsigned long long* OUTPUT);

cpl_return_t
cpl_create_object(const char* prefix,
                  const char* name,
                  const int type,
                  const cpl_id_t bundle,
                  unsigned long long* OUTPUT);

cpl_return_t
cpl_lookup_object(const char* prefix,
                  const char* name,
                  const int type,
                  const cpl_id_t bundle_id,
                  unsigned long long* OUTPUT);

cpl_return_t
cpl_lookup_or_create_object(const char* prefix,
                            const char* name,
                            const int type,
                            const cpl_id_t bundle,
                            unsigned long long* OUTPUT);

cpl_return_t
cpl_add_relation(const cpl_id_t from_id,
                 const cpl_id_t to_id,
                 const int type,
                 unsigned long long* OUTPUT);

cpl_return_t
cpl_get_current_session(unsigned long long* OUTPUT);

cpl_return_t
import_document_json(const std::string& filename,
                     const std::string& bundle_name,
                     const std::vector<std::pair<cpl_id_t, std::string>>& anchor_objects,
                     const int flags,
                     unsigned long long* OUTPUT);

/*
 * STL bundles
 */

%template (cpl_id_timestamp_t_vector) std::vector<cpl_id_timestamp_t>;

inline std::vector<cpl_id_timestamp_t>&
cpl_dereference_p_std_vector_cpl_id_timestamp_t(
    std_vector_cpl_id_timestamp_t* p);

inline void*
cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(
        std_vector_cpl_id_timestamp_t* p);

/*%template (cpl_relation_t_list) std::list<cpl_relation_t>;*/
%template (cpl_relation_t_vector) std::vector<cpl_relation_t>;

inline std::vector<cpl_relation_t>&
cpl_dereference_p_std_vector_cpl_relation_t(
    std_vector_cpl_relation_t* p);

inline void*
cpl_convert_p_std_vector_cpl_relation_t_to_p_void(
        std_vector_cpl_relation_t* p);

%template (cplxx_object_info_t_vector) std::vector<cplxx_object_info_t>;

inline std::vector<cplxx_object_info_t>&
cpl_dereference_p_std_vector_cplxx_object_info_t(
    std_vector_cplxx_object_info_t* p);

inline void*
cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(
        std_vector_cplxx_object_info_t* p);

%template (cplxx_property_entry_t_vector) std::vector<cplxx_property_entry_t>;

inline std::vector<cplxx_property_entry_t>&
cpl_dereference_p_std_vector_cplxx_property_entry_t(
    std_vector_cplxx_property_entry_t* p);

inline void*
cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(
        std_vector_cplxx_property_entry_t* p);

%template (cplxx_prefix_entry_t_vector) std::vector<cplxx_prefix_entry_t>;

inline std::vector<cplxx_prefix_entry_t>&
cpl_dereference_p_std_vector_cplxx_prefix_entry_t(
    std_vector_cplxx_prefix_entry_t* p);

inline void*
cpl_convert_p_std_vector_cplxx_prefix_entry_t_to_p_void(
        std_vector_cplxx_prefix_entry_t* p);

%template (cpl_id_t_vector) std::vector<cpl_id_t>;

inline std::vector<cpl_id_t>&
cpl_dereference_p_std_vector_cpl_id_t(
    std_vector_cpl_id_t* p);

inline void*
cpl_convert_p_std_vector_cpl_id_t_to_p_void(
        std_vector_cpl_id_t* p);

%template(cplxx_id_name_pair) std::pair<cpl_id_t, std::string>;

%template (cplxx_id_name_pair_vector) std::vector<std::pair<cpl_id_t, std::string>>;

%inline %{
  struct validate_json_return_t {
    cpl_return_t return_code;
    std::string out_string;
  };

  validate_json_return_t validate_json(const std::string& filepath) {
    validate_json_return_t ret;
    ret.return_code = validate_json(filepath,ret.out_string);
    return ret;
  };
%}

%inline %{
  struct export_bundle_json_return_t {
    cpl_return_t return_code;
    std::string out_string;
  };

  export_bundle_json_return_t export_bundle_json(const std::vector<cpl_id_t> bundles) {
    struct export_bundle_json_return_t ret;
    ret.return_code = export_bundle_json(bundles, ret.out_string);
    return ret;
  };
%}

%ignore method2;

/*
 * Pointers
 */

%pointer_functions(p_cpl_db_backend_t, cpl_db_backend_tpp);
%pointer_functions(p_cpl_session_info_t, cpl_session_info_tpp);
%pointer_functions(p_cpl_object_info_t, cpl_object_info_tpp);
%pointer_functions(p_cpl_bundle_info_t, cpl_bundle_info_tpp);

%pointer_functions(cpl_session_t, cpl_session_tp);
%pointer_functions(cpl_id_t, cpl_id_tp);

%pointer_functions(cpl_session_info_t, cpl_session_info_tp);
%pointer_functions(cpl_object_info_t, cpl_object_info_tp);
%pointer_functions(cpl_bundle_info_t, cpl_bundle_info_tp);

%pointer_functions(std_vector_cpl_id_timestamp_t,
        std_vector_cpl_id_timestamp_tp);
/*%pointer_functions(std_list_cpl_relation_t,
        std_list_cpl_relation_tp);*/
%pointer_functions(std_vector_cpl_relation_t,
        std_vector_cpl_relation_tp);

%pointer_functions(std_vector_cplxx_object_info_t,
        std_vector_cplxx_object_info_tp);

%pointer_functions(std_vector_cplxx_property_entry_t,
        std_vector_cplxx_property_entry_tp);

%pointer_functions(std_vector_cplxx_prefix_entry_t,
        std_vector_cplxx_prefix_entry_tp);

%pointer_functions(std_vector_cpl_id_t, std_vector_cpl_id_tp);

