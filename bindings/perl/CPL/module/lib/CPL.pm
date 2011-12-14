#
# CPL.pm
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

package CPL;

use 5.012004;
use strict;
use warnings;

use CPLDirect;
use Carp;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use CPL ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	attach_odbc
	detach
	create_object
	lookup_object
	try_lookup_object
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	attach_odbc
	detach
	create_object
	lookup_object
	try_lookup_object
);

our $VERSION = '1.00';



#############################################################################
# Public API: Variables and Constants                                       #
#############################################################################

our %NONE = ( hi => 0, lo => 0 );

*VERSION_NONE = *CPLDirect::CPL_VERSION_NONE;



#############################################################################
# Public API: Attach / Detach                                               #
#############################################################################


#
# Initialize the CPL by opening an ODBC connection
#
sub attach_odbc {
	my ($connection_string) = @_;

	my $backend_ptr = CPLDirect::new_cpl_db_backend_tpp();
	my $ret = CPLDirect::cpl_create_odbc_backend($connection_string,
			$CPLDirect::CPL_ODBC_GENERIC, $backend_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_db_backend_tpp($backend_ptr);
		croak "Could not open an ODBC connection: " . \
			CPLDirect::cpl_error_string($ret);
	}

	my $backend = CPLDirect::cpl_dereference_pp_cpl_db_backend_t($backend_ptr);
	$ret = CPLDirect::cpl_attach($backend);
	CPLDirect::delete_cpl_db_backend_tpp($backend_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not initialize CPL: " . CPLDirect::cpl_error_string($ret);
	}

	return 1;
}


#
# Detach from CPL and perform clean-up
#
sub detach {

	my $ret = CPLDirect::cpl_detach();
	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not detach CPL: " . CPLDirect::cpl_error_string($ret);
	}

	return 1;
}



#############################################################################
# Public API: Disclosed Provenance API                                      #
#############################################################################


#
# Create a provenance object
#
sub create_object {
	my ($originator, $name, $type, %container) = @_;

	if (!%container) {
		%container = %NONE;
	}

	my $c_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($c_ptr, $container{hi});
	CPLDirect::cpl_id_t::swig_lo_set($c_ptr, $container{lo});
	my $c = CPLDirect::cpl_id_tp_value($c_ptr);

	my $obj_ptr = CPLDirect::new_cpl_id_tp();
	my $ret = CPLDirect::cpl_create_object($originator, $name, $type,
			$c, $obj_ptr);
	CPLDirect::delete_cpl_id_tp($c_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_id_tp($obj_ptr);
		croak "Could not create a provenance object:\n" .
			"    Originator: \"$originator\"\n" .
			"    Name: \"$name\"\n" .
			"    Type: \"$type\"\n" .
			"Error: " . CPLDirect::cpl_error_string($ret) . "\n";
	}

	my $obj = CPLDirect::cpl_id_tp_value($obj_ptr);
	my %id = (
	   hi => CPLDirect::cpl_id_t::swig_hi_get($obj),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($obj)
	);
	CPLDirect::delete_cpl_id_tp($obj_ptr);

	return %id;
}


#
# Lookup a provenance object
#
sub lookup_object {
	my ($originator, $name, $type, $ok_if_not_found) = @_;

	my $obj_ptr = CPLDirect::new_cpl_id_tp();
	my $ret = CPLDirect::cpl_lookup_object($originator, $name, $type, $obj_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_id_tp($obj_ptr);

		if ($ok_if_not_found) {
			return %NONE;
		}

		croak "Could not determine the ID of the following object:\n" .
			"    Originator: \"$originator\"\n" .
			"    Name: \"$name\"\n" .
			"    Type: \"$type\"\n" .
			"Error: " . CPLDirect::cpl_error_string($ret) . "\n";
	}

	my $obj = CPLDirect::cpl_id_tp_value($obj_ptr);
	my %id = (
	   hi => CPLDirect::cpl_id_t::swig_hi_get($obj),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($obj)
	);
	CPLDirect::delete_cpl_id_tp($obj_ptr);

	return %id;
}


#
# Lookup a provenance object, but do not fail if it does not exist
#
sub try_lookup_object {
	return lookup_object(@_, 1);
}



#############################################################################
# Finish                                                                    #
#############################################################################

1;
__END__




#############################################################################
# Documentation                                                             #
#############################################################################

=head1 NAME

CPL - Perl bindings for Core Provenance Library



=head1 SYNOPSIS

  use CPL;
  CPL::attach_odbc("DSN=CPL;");
  my %id  = CPL::create_object("com.example.myapp", "/bin/sh", "proc");
  my %id1 = CPL::create_object("com.example.myapp", "~/a.txt", "file", %id);
  my %id2 = CPL::lookup_object("com.example.myapp", "/bin/sh", "proc");
  my %id3 = CPL::try_lookup_object("com.example.myapp", "/bin/sh", "proc");
  CPL::detach();



=head1 DESCRIPTION

Core Provenance Library (CPL) is a cross-platform, multi-language library that
enables application programmers to easily add provenance collection and
qurying to their programs. This package contains Perl bindings for the CPL.

=head2 Functions
 
The following functions are exported by default:

=head3 attach_odbc

  CPL::attach_odbc($connection_string);

Initializes the Core Provenance Library for this program and attaches it to
the CPL's backend database via an ODBC connection specified through the
given $connection_string.

=head3 detach

  CPL::detach();

Detaches from the backend database and cleans up the CPL.

=head3 create_object

  my %id = CPL::create_object($originator, $name, $type);
  my %id = CPL::create_object($originator, $name, $type, %container);

Creates a provenance object %id with the given $originator (the program
responsible for creating the objects; it also acts as a namespace), $name,
$type, and an optional %container. If the %container parameter is omitted
or set to %CPL::NONE, the created object would not belong to any container.

=head3 lookup_object

  my %id = CPL::lookup_object($originator, $name, $type);

Looks up a provenance object based on its $originator, $name, and $type
and returns its %id. If more than one object matches the search criteria,
return the one with the most recent timestamp.

=head3 try_lookup_object

  my %id = CPL::try_lookup_object($originator, $name, $type);

The same as CPL::lookup_object() described above, except that the function
call does not fail if the object does not exist, in which case it returns
%CPL::NONE in place of the object %id.



=head1 HISTORY

=over 8

=item 1.00

Original version.

=back



=head1 SEE ALSO

TODO



=head1 AUTHOR

Peter Macko <pmacko@eecs.harvard.edu>



=head1 COPYRIGHT AND LICENSE

Copyright 2011
     The President and Fellows of Harvard College.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

3. Neither the name of the University nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.

Contributor(s): Peter Macko


=cut
