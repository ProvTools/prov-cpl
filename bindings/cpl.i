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

%{
#include <cpl.h>
#include <backends/cpl-odbc.h>

#if defined(__unix__) || defined(__APPLE__)
#include <backends/cpl-rdf.h>
#endif

typedef cpl_db_backend_t* p_cpl_db_backend_t;

inline _cpl_db_backend_t*
cpl_dereference_pp_cpl_db_backend_t(p_cpl_db_backend_t* p) {
    return *p;
}

inline int
cpl_is_ok(cpl_return_t ret) {
    return CPL_IS_OK(ret);
}
%}

inline _cpl_db_backend_t*
cpl_dereference_pp_cpl_db_backend_t(p_cpl_db_backend_t* p);

inline int
cpl_is_ok(cpl_return_t ret);

%include "../../../include/cpl.h"
%include "../../../include/backends/cpl-odbc.h"

%include cpointer.i
%pointer_functions(p_cpl_db_backend_t, cpl_db_backend_tpp);

