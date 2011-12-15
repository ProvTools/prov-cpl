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

use CPL;

my $ORIGINATOR = "edu.harvard.pass.cpl.perl.direct.test";


#
# Function: Print contents of a hash
#
sub print_hash_ref {
    my ($hash, $prefix) = @_;
	if (!$prefix) { $prefix = "    " }
    
    while (my ($k, $v) = each %$hash) {
		if (ref($v) eq "HASH") {
			print "$prefix$k =>\n";
			print_hash_ref($v, $prefix . "    ");
		}
		else {
			print "$prefix$k => $v\n";
		}
    }
}


#
# Function: Stringify the contents of a hash
#
sub str_hash_ref {
    my ($hash) = @_;
    
    my $str = "";
    while (my ($k, $v) = each %$hash) {
        if ($str ne "") { $str .= ", " }
        $str .= "$k => $v";
    }

    return $str;
}


#
# Open an ODBC connection
#

CPL::attach_odbc("DSN=CPL;");

print "CPL::get_current_session()";
my $session = CPL::get_current_session();
print ": " . str_hash_ref($session) . "\n";

print "\n";


#
# Create objects
#

print "CPL::create_object(\"Process A\", \"Proc\")";
my $obj1 = CPL::create_object($ORIGINATOR, "Process A", "Proc");
print ": " . str_hash_ref($obj1) . "\n";

print "CPL::create_object(\"Object A\", \"File\", obj1)";
my $obj2 = CPL::create_object($ORIGINATOR, "Object A", "File", $obj1);
print ": " . str_hash_ref($obj2) . "\n";

print "CPL::create_object(\"Process B\", \"Proc\", obj1)";
my $obj3 = CPL::create_object($ORIGINATOR, "Process B", "Proc", $obj1);
print ": " . str_hash_ref($obj3) . "\n";

print "CPL::create_object(\"Object B\", \"File\", CPL::NONE)";
my $obj4 = CPL::create_object($ORIGINATOR, "Object B", "File", $CPL::NONE);
print ": " . str_hash_ref($obj4) . "\n";

print "\n";


#
# Lookup objects
#

print "CPL::lookup_object(\"Process A\", \"Proc\")";
my $obj1x = CPL::lookup_object($ORIGINATOR, "Process A", "Proc");
print ": " . str_hash_ref($obj1x) . "\n";
if (%$obj1 ne %$obj1x) { die "Object lookup returned the wrong object"; }

print "CPL::lookup_object(\"Object A\", \"File\")";
my $obj2x = CPL::lookup_object($ORIGINATOR, "Object A", "File");
print ": " . str_hash_ref($obj2x) . "\n";
if (%$obj2 ne %$obj2x) { die "Object lookup returned the wrong object"; }

print "CPL::try_lookup_object(\"Process B\", \"Proc\")";
my $obj3x = CPL::try_lookup_object($ORIGINATOR, "Process B", "Proc");
print ": " . str_hash_ref($obj3x) . "\n";
if (%$obj3 ne %$obj3x) { die "Object lookup returned the wrong object"; }

print "CPL::try_lookup_object(\"Object B\", \"File\")";
my $obj4x = CPL::try_lookup_object($ORIGINATOR, "Object B", "File");
print ": " . str_hash_ref($obj4x) . "\n";
if (%$obj4 ne %$obj4x) { die "Object lookup returned the wrong object"; }

print "\n";


#
# Data and control flow / dependencies
#

print "CPL::data_flow(obj2, obj1)";
my $r1 = CPL::data_flow($obj2, $obj1);
if (!$r1) { print " (duplicate ignored)" }
print "\n";

print "CPL::data_flow(obj2, obj1, CPL::DATA_INPUT)";
my $r2 = CPL::data_flow($obj2, $obj1, $CPL::DATA_INPUT);
if (!$r2) { print " (duplicate ignored)" }
print "\n";

print "CPL::data_flow(obj3, obj2, CPL::DATA_INPUT)";
my $r3 = CPL::data_flow($obj3, $obj2, $CPL::DATA_INPUT);
if (!$r3) { print " (duplicate ignored)" }
print "\n";

print "CPL::control(obj3, obj1, CPL::CONTROL_START)";
my $r4 = CPL::control($obj3, $obj1, $CPL::CONTROL_START);
if (!$r4) { print " (duplicate ignored)" }
print "\n";

print "CPL::data_flow_ext(obj1, obj3, 0, CPL::DATA_TRANSLATION)";
my $r5 = CPL::data_flow_ext($obj1, $obj3, 0, $CPL::DATA_TRANSLATION);
if (!$r5) { print " (duplicate ignored)" }
print "\n";

print "\n";


#
# Get version
#

print "CPL::get_version(obj1)";
my $ver1 = CPL::get_version($obj1);
printf ": %d\n", $ver1;

print "CPL::get_version(obj2)";
my $ver2 = CPL::get_version($obj2);
printf ": %d\n", $ver2;

print "CPL::get_version(obj3)";
my $ver3 = CPL::get_version($obj3);
printf ": %d\n", $ver3;

print "CPL::get_version(obj4)";
my $ver4 = CPL::get_version($obj4);
printf ": %d\n", $ver4;

print "\n";


#
# Get object info
#

print "CPL::get_object_info(obj1)";
my %info1 = CPL::get_object_info($obj1);
print ":\n";
print_hash_ref(\%info1);
print "\n";

print "\n";


#
# Close the connection
#

CPL::detach();

