#
# test.pl
# Core Provenance Library
#
# Copyright 2011
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

use warnings;
use strict;

use CPLDirect;

my $ORIGINATOR = "edu.harvard.pass.cpl.perl.direct.test";


#
# Function: Check the return code and die on error
#

sub check_ret {
	my ($ret) = @_;
	if (!CPLDirect::cpl_is_ok($ret)) {
		print "\n";
		die "CPL Error: " . CPLDirect::cpl_error_string($ret);
	}
	return 1;
}


#
# Open a simple ODBC connection
#

my $backend = CPLDirect::new_cpl_db_backend_tpp();

print "CPLDirect::cpl_create_odbc_backend";
check_ret(CPLDirect::cpl_create_odbc_backend("DSN=CPL;", 0, $backend));
print "\n";

print "CPLDirect::cpl_attach";
check_ret(CPLDirect::cpl_attach(
			CPLDirect::cpl_dereference_pp_cpl_db_backend_t($backend)));
print "\n";

CPLDirect::delete_cpl_db_backend_tpp($backend);


#
# Get the current session
#

my $session_ptr = CPLDirect::new_cpl_session_tp();
print "CPLDirect::cpl_get_current_session";
check_ret(CPLDirect::cpl_get_current_session($session_ptr));
my $session = CPLDirect::cpl_session_tp_value($session_ptr);
printf ": %llx:%llx\n",
	   CPLDirect::cpl_id_t::swig_hi_get($session),
	   CPLDirect::cpl_id_t::swig_lo_get($session);

print "\n";


#
# Create objects
#

my $obj1_ptr = CPLDirect::new_cpl_id_tp();
my $obj2_ptr = CPLDirect::new_cpl_id_tp();
my $obj3_ptr = CPLDirect::new_cpl_id_tp();

print "CPLDirect::cpl_create_object(\"Process A\", \"Proc\", CPL_NONE)";
check_ret(CPLDirect::cpl_create_object($ORIGINATOR, "Process A", "Proc",
			$CPLDirect::CPL_NONE, $obj1_ptr));
my $obj1 = CPLDirect::cpl_id_tp_value($obj1_ptr);
printf ": %llx:%llx --> obj1\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj1),
	   CPLDirect::cpl_id_t::swig_lo_get($obj1);

print "CPLDirect::cpl_create_object(\"Object A\", \"File\", obj1)";
check_ret(CPLDirect::cpl_create_object($ORIGINATOR, "Object A", "File",
			$obj1, $obj2_ptr));
my $obj2 = CPLDirect::cpl_id_tp_value($obj2_ptr);
printf ": %llx:%llx --> obj2\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj2),
	   CPLDirect::cpl_id_t::swig_lo_get($obj2);

print "CPLDirect::cpl_create_object(\"Process B\", \"Proc\", obj1)";
check_ret(CPLDirect::cpl_create_object($ORIGINATOR, "Process B", "Proc",
			$obj1, $obj3_ptr));
my $obj3 = CPLDirect::cpl_id_tp_value($obj3_ptr);
printf ": %llx:%llx --> obj3\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj3),
	   CPLDirect::cpl_id_t::swig_lo_get($obj3);

print "\n";


#
# Object lookup
#

my $obj1x_ptr = CPLDirect::new_cpl_id_tp();
my $obj2x_ptr = CPLDirect::new_cpl_id_tp();
my $obj3x_ptr = CPLDirect::new_cpl_id_tp();

print "CPLDirect::cpl_lookup_object(\"Process A\", \"Proc\")";
check_ret(CPLDirect::cpl_lookup_object($ORIGINATOR, "Process A", "Proc",
			$obj1x_ptr));
my $obj1x = CPLDirect::cpl_id_tp_value($obj1x_ptr);
printf ": %llx:%llx\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj1x),
	   CPLDirect::cpl_id_t::swig_lo_get($obj1x);
if (CPLDirect::cpl_id_cmp($obj1, $obj1x) != 0) {
	die "Object lookup returned the wrong object";
}

print "CPLDirect::cpl_lookup_object(\"Object A\", \"File\")";
check_ret(CPLDirect::cpl_lookup_object($ORIGINATOR, "Object A", "FIle",
			$obj2x_ptr));
my $obj2x = CPLDirect::cpl_id_tp_value($obj2x_ptr);
printf ": %llx:%llx\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj2x),
	   CPLDirect::cpl_id_t::swig_lo_get($obj2x);
if (CPLDirect::cpl_id_cmp($obj2, $obj2x) != 0) {
	die "Object lookup returned the wrong object";
}

print "CPLDirect::cpl_lookup_object(\"Process B\", \"Proc\")";
check_ret(CPLDirect::cpl_lookup_object($ORIGINATOR, "Process B", "Proc",
			$obj3x_ptr));
my $obj3x = CPLDirect::cpl_id_tp_value($obj3x_ptr);
printf ": %llx:%llx\n",
	   CPLDirect::cpl_id_t::swig_hi_get($obj3x),
	   CPLDirect::cpl_id_t::swig_lo_get($obj3x);
if (CPLDirect::cpl_id_cmp($obj3, $obj3x) != 0) {
	die "Object lookup returned the wrong object";
}

print "\n";


#
# Data and control flow / dependencies
#

print "CPLDirect::cpl_data_flow(obj2, obj1, CPL_DATA_INPUT)";
check_ret(CPLDirect::cpl_data_flow($obj2, $obj1, $CPLDirect::CPL_DATA_INPUT));
print "\n";

print "CPLDirect::cpl_data_flow(obj2, obj1, CPL_DATA_INPUT)";
check_ret(CPLDirect::cpl_data_flow($obj2, $obj1, $CPLDirect::CPL_DATA_INPUT));
print "\n";

print "CPLDirect::cpl_control(obj3, obj1, CPL_CONTROL_START)";
check_ret(CPLDirect::cpl_control($obj3, $obj1, $CPLDirect::CPL_CONTROL_START));
print "\n";

print "CPLDirect::cpl_data_flow_ext(obj1, obj3, 0, CPL_DATA_TRANSLATION)";
check_ret(CPLDirect::cpl_data_flow_ext($obj1, $obj3, 0,
			$CPLDirect::CPL_DATA_TRANSLATION));
print "\n";

print "\n";


#
# Close
#

CPLDirect::delete_cpl_session_tp($session_ptr);
CPLDirect::delete_cpl_id_tp($obj1_ptr);
CPLDirect::delete_cpl_id_tp($obj2_ptr);
CPLDirect::delete_cpl_id_tp($obj3_ptr);
CPLDirect::delete_cpl_id_tp($obj1x_ptr);
CPLDirect::delete_cpl_id_tp($obj2x_ptr);
CPLDirect::delete_cpl_id_tp($obj3x_ptr);

print "CPLDirect::cpl_detach";
check_ret(CPLDirect::cpl_detach());
print "\n";

