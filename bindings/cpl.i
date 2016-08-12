/*
 * cpl.i
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

 
%module CPLDirect

%include cpointer.i
/*%include std_list.i*/
%include std_vector.i
%include std_string.i


/*
 * Additional code for the C wrapper
 */

%{
#include <cpl.h>
#include <cplxx.h>

#include <backends/cpl-odbc.h>

#if defined(__unix__) || defined(__APPLE__)
#include <backends/cpl-rdf.h>
#endif

typedef cpl_db_backend_t* p_cpl_db_backend_t;
typedef cpl_session_info_t* p_cpl_session_info_t;
typedef cpl_object_info_t* p_cpl_object_info_t;

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

inline cpl_session_info_t**
cpl_convert_pp_cpl_session_info_t(p_cpl_session_info_t* p) {
    return p;
}

inline cpl_object_info_t**
cpl_convert_pp_cpl_object_info_t(p_cpl_object_info_t* p) {
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

/*typedef std::list<cpl_ancestry_entry_t> std_list_cpl_ancestry_entry_t;*/
typedef std::vector<cpl_ancestry_entry_t> std_vector_cpl_ancestry_entry_t;

inline std::vector<cpl_ancestry_entry_t>&
cpl_dereference_p_std_vector_cpl_ancestry_entry_t(
        std_vector_cpl_ancestry_entry_t* p) {
    return *p;
}

inline void*
cpl_convert_p_std_vector_cpl_ancestry_entry_t_to_p_void(
        std_vector_cpl_ancestry_entry_t* p) {
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

inline cpl_session_info_t**
cpl_convert_pp_cpl_session_info_t(p_cpl_session_info_t* p);

inline cpl_object_info_t**
cpl_convert_pp_cpl_object_info_t(p_cpl_object_info_t* p);

inline int
cpl_is_ok(cpl_return_t ret);


/*
 * Include the CPL header files
 */

%include "../../../include/cpl.h"
%include "../../../include/cplxx.h"

%include "../../../include/backends/cpl-odbc.h"

/* XXX The RDF driver does not work on Windows, so this should be conditional */
%include "../../../include/backends/cpl-rdf.h"


/*
 * STL containers
 */

%template (cpl_id_timestamp_t_vector) std::vector<cpl_id_timestamp_t>;

inline std::vector<cpl_id_timestamp_t>&
cpl_dereference_p_std_vector_cpl_id_timestamp_t(
    std_vector_cpl_id_timestamp_t* p);

inline void*
cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(
        std_vector_cpl_id_timestamp_t* p);

/*%template (cpl_ancestry_entry_t_list) std::list<cpl_ancestry_entry_t>;*/
%template (cpl_ancestry_entry_t_vector) std::vector<cpl_ancestry_entry_t>;

inline std::vector<cpl_ancestry_entry_t>&
cpl_dereference_p_std_vector_cpl_ancestry_entry_t(
    std_vector_cpl_ancestry_entry_t* p);

inline void*
cpl_convert_p_std_vector_cpl_ancestry_entry_t_to_p_void(
        std_vector_cpl_ancestry_entry_t* p);

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

%template (cpl_id_t_vector) std::vector<cpl_id_t>;

inline std::vector<cpl_id_t>&
cpl_dereference_p_std_vector_cpl_id_t(
    std_vector_cpl_id_t* p);

inline void*
cpl_convert_p_std_vector_cpl_id_t_to_p_void(
        std_vector_cpl_id_t* p);
/*
 * Pointers
 */

%pointer_functions(p_cpl_db_backend_t, cpl_db_backend_tpp);
%pointer_functions(p_cpl_session_info_t, cpl_session_info_tpp);
%pointer_functions(p_cpl_object_info_t, cpl_object_info_tpp);

%pointer_functions(cpl_session_t, cpl_session_tp);
%pointer_functions(cpl_id_t, cpl_id_tp);

%pointer_functions(cpl_session_info_t, cpl_session_info_tp);
%pointer_functions(cpl_object_info_t, cpl_object_info_tp);

%pointer_functions(std_vector_cpl_id_timestamp_t,
        std_vector_cpl_id_timestamp_tp);
/*%pointer_functions(std_list_cpl_ancestry_entry_t,
        std_list_cpl_ancestry_entry_tp);*/
%pointer_functions(std_vector_cpl_ancestry_entry_t,
        std_vector_cpl_ancestry_entry_tp);

%pointer_functions(std_vector_cplxx_object_info_t,
        std_vector_cplxx_object_info_tp);

%pointer_functions(std_vector_cplxx_property_entry_t,
        std_vector_cplxx_property_entry_tp);

%pointer_functions(std_vector_cpl_id_t, std_vector_cpl_id_tp);

