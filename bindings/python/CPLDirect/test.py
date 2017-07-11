#!/usr/bin/env python

#
# test.py
# Prov-CPL
#
# Copyright 2016
#      The President and Fellows of Harvard College.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#
# Contributor(s): Peter Macko
#

from CPLDirect import *
import sys

print("CPL ver. " + CPL_VERSION_STR)
print("")

print("CPLDirect.cpl_create_odbc_backend");
backend = new_cpl_db_backend_tpp()
ret = cpl_create_odbc_backend("DSN=CPL",
        CPL_ODBC_GENERIC, backend)
if not cpl_is_ok(ret):
    print ("Could not create ODBC connection" +
            cpl_error_string(ret))
    sys.exit(1)
db = cpl_dereference_pp_cpl_db_backend_t(backend)

print("CPLDirect.cpl_attach");
ret = cpl_attach(db)
delete_cpl_db_backend_tpp(backend)
if not cpl_is_ok(ret):
    print ("Could not open ODBC connection" +
            cpl_error_string(ret))
    sys.exit(1)

print("CPLDirect.cpl_detach");
cpl_detach()

