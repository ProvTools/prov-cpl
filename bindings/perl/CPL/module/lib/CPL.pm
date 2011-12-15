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
	data_flow
	control
	data_flow_ext
	control_ext
	get_current_session
	get_version
	get_object_info
	get_version_info
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	attach_odbc
	detach
	create_object
	lookup_object
	try_lookup_object
	data_flow
	control
	data_flow_ext
	control_ext
	get_current_session
	get_version
	get_object_info
	get_version_info
);

our $VERSION = '1.00';



#############################################################################
# Public API: Variables and Constants                                       #
#############################################################################

our $NONE = { hi => 0, lo => 0 };

*VERSION_NONE = *CPLDirect::CPL_VERSION_NONE;

*DATA_INPUT = *CPLDirect::CPL_DATA_INPUT;
*DATA_IPC = *CPLDirect::CPL_DATA_IPC;
*DATA_TRANSLATION = *CPLDirect::CPL_DATA_TRANSLATION;
*DATA_COPY = *CPLDirect::CPL_DATA_COPY;

*CONTROL_OP = *CPLDirect::CPL_CONTROL_OP;
*CONTROL_START = *CPLDirect::CPL_CONTROL_START;



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
	my ($originator, $name, $type, $container) = @_;

	if (!$container) {
		$container = $NONE;
	}

	my $c_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($c_ptr, $container->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($c_ptr, $container->{lo});
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
	my $id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($obj),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($obj)
	};
	CPLDirect::delete_cpl_id_tp($obj_ptr);

	return $id;
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
			return $NONE;
		}

		croak "Could not determine the ID of the following object:\n" .
			"    Originator: \"$originator\"\n" .
			"    Name: \"$name\"\n" .
			"    Type: \"$type\"\n" .
			"Error: " . CPLDirect::cpl_error_string($ret) . "\n";
	}

	my $obj = CPLDirect::cpl_id_tp_value($obj_ptr);
	my $id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($obj),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($obj)
	};
	CPLDirect::delete_cpl_id_tp($obj_ptr);

	return $id;
}


#
# Lookup a provenance object, but do not fail if it does not exist
#
sub try_lookup_object {
	return lookup_object(@_, 1);
}


#
# Add a data dependency
#
sub data_flow {
	my ($dest, $src, $type) = @_;

	if (!$type) { $type = $CPL::DATA_INPUT }

	my $d_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($d_ptr, $dest->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($d_ptr, $dest->{lo});
	my $d = CPLDirect::cpl_id_tp_value($d_ptr);

	my $s_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($s_ptr, $src->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($s_ptr, $src->{lo});
	my $s = CPLDirect::cpl_id_tp_value($s_ptr);

	my $ret = CPLDirect::cpl_data_flow($d, $s, $type);
	CPLDirect::delete_cpl_id_tp($d_ptr);
	CPLDirect::delete_cpl_id_tp($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add a data dependency: " .
			CPLDirect::cpl_error_string($ret);
	}

	return $ret == $CPLDirect::CPL_S_DUPLICATE_IGNORED ? 0 : 1;
}


#
# Add a control dependency
#
sub control {
	my ($dest, $src, $type) = @_;

	if (!$type) { $type = $CPL::CONTROL_OP }

	my $d_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($d_ptr, $dest->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($d_ptr, $dest->{lo});
	my $d = CPLDirect::cpl_id_tp_value($d_ptr);

	my $s_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($s_ptr, $src->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($s_ptr, $src->{lo});
	my $s = CPLDirect::cpl_id_tp_value($s_ptr);

	my $ret = CPLDirect::cpl_control($d, $s, $type);
	CPLDirect::delete_cpl_id_tp($d_ptr);
	CPLDirect::delete_cpl_id_tp($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add a control dependency: " .
			CPLDirect::cpl_error_string($ret);
	}

	return $ret == $CPLDirect::CPL_S_DUPLICATE_IGNORED ? 0 : 1;
}


#
# Add a data dependency (specify the version number of the source)
#
sub data_flow_ext {
	my ($dest, $src, $src_version, $type) = @_;

	if (!$type) { $type = $CPL::DATA_INPUT }

	my $d_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($d_ptr, $dest->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($d_ptr, $dest->{lo});
	my $d = CPLDirect::cpl_id_tp_value($d_ptr);

	my $s_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($s_ptr, $src->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($s_ptr, $src->{lo});
	my $s = CPLDirect::cpl_id_tp_value($s_ptr);

	my $ret = CPLDirect::cpl_data_flow_ext($d, $s, $src_version, $type);
	CPLDirect::delete_cpl_id_tp($d_ptr);
	CPLDirect::delete_cpl_id_tp($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add a data dependency: " .
			CPLDirect::cpl_error_string($ret);
	}

	return $ret == $CPLDirect::CPL_S_DUPLICATE_IGNORED ? 0 : 1;
}


#
# Add a control dependency (specify the version number of the source)
#
sub control_ext {
	my ($dest, $src, $src_version, $type) = @_;

	if (!$type) { $type = $CPL::CONTROL_OP }

	my $d_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($d_ptr, $dest->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($d_ptr, $dest->{lo});
	my $d = CPLDirect::cpl_id_tp_value($d_ptr);

	my $s_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($s_ptr, $src->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($s_ptr, $src->{lo});
	my $s = CPLDirect::cpl_id_tp_value($s_ptr);

	my $ret = CPLDirect::cpl_control_ext($d, $s, $src_version, $type);
	CPLDirect::delete_cpl_id_tp($d_ptr);
	CPLDirect::delete_cpl_id_tp($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add a control dependency: " .
			CPLDirect::cpl_error_string($ret);
	}

	return $ret == $CPLDirect::CPL_S_DUPLICATE_IGNORED ? 0 : 1;
}



#############################################################################
# Public API: Provenance Access API                                         #
#############################################################################


#
# Get the ID of the currenet session
#
sub get_current_session {

	my $s_ptr = CPLDirect::new_cpl_id_tp();
	my $ret = CPLDirect::cpl_get_current_session($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_id_tp($s_ptr);
		croak "Could not determine the ID of the current session: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $s = CPLDirect::cpl_id_tp_value($s_ptr);
	my $id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($s),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($s)
	};
	CPLDirect::delete_cpl_id_tp($s_ptr);

	return $id;
}


#
# Get version of a provenance object
#
sub get_version {
	my ($id) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $v_ptr = CPLDirect::new_cpl_version_tp();
	my $ret = CPLDirect::cpl_get_version($x, $v_ptr);
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_version_tp($v_ptr);
		croak "Could not determine the version of an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $v = CPLDirect::cpl_version_tp_value($v_ptr);
	CPLDirect::delete_cpl_version_tp($v_ptr);

	return $v;
}


#
# Get the object info
#
sub get_object_info {
	my ($id) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $info_ptr_ptr = CPLDirect::new_cpl_object_info_tpp();
	my $ret = CPLDirect::cpl_get_object_info($x,
			CPLDirect::cpl_convert_pp_cpl_object_info_t($info_ptr_ptr));
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_object_info_tpp($info_ptr_ptr);
		croak "Could not determine information about an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $info_ptr = 
		CPLDirect::cpl_dereference_pp_cpl_object_info_t($info_ptr_ptr);
	my $info = CPLDirect::cpl_object_info_tp_value($info_ptr);

	my $info_id = CPLDirect::cpl_object_info_t::swig_id_get($info);
	my $r_id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_id),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_id)
	};

	my $info_session =
		CPLDirect::cpl_object_info_t::swig_creation_session_get($info);
	my $r_session = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_session),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_session)
	};

	my $info_container =
		CPLDirect::cpl_object_info_t::swig_container_id_get($info);
	my $r_container = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_container),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_container)
	};

	my %r = (
		id                => $r_id,
		version           => 
			CPLDirect::cpl_object_info_t::swig_version_get($info),
		creation_session  => $r_session,
		creation_time     => 
			CPLDirect::cpl_object_info_t::swig_creation_time_get($info),
		originator        => 
			CPLDirect::cpl_object_info_t::swig_originator_get($info),
		name              => 
			CPLDirect::cpl_object_info_t::swig_name_get($info),
		type              => 
			CPLDirect::cpl_object_info_t::swig_type_get($info),
		container_id      => $r_container,
		container_version =>
			CPLDirect::cpl_object_info_t::swig_container_version_get($info),
	);

	CPLDirect::cpl_free_object_info($info_ptr);
	CPLDirect::delete_cpl_object_info_tpp($info_ptr_ptr);
	return %r;
}


#
# Get the version info
#
sub get_version_info {
	my ($id, $version) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $info_ptr_ptr = CPLDirect::new_cpl_version_info_tpp();
	my $ret = CPLDirect::cpl_get_version_info($x, $version,
			CPLDirect::cpl_convert_pp_cpl_version_info_t($info_ptr_ptr));
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_version_info_tpp($info_ptr_ptr);
		croak "Could not determine information about a version of an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $info_ptr = 
		CPLDirect::cpl_dereference_pp_cpl_version_info_t($info_ptr_ptr);
	my $info = CPLDirect::cpl_version_info_tp_value($info_ptr);

	my $info_id = CPLDirect::cpl_version_info_t::swig_id_get($info);
	my $r_id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_id),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_id)
	};

	my $info_session =
		CPLDirect::cpl_version_info_t::swig_session_get($info);
	my $r_session = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_session),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_session)
	};

	my %r = (
		id                => $r_id,
		version           => 
			CPLDirect::cpl_version_info_t::swig_version_get($info),
		session           => $r_session,
		creation_time     => 
			CPLDirect::cpl_version_info_t::swig_creation_time_get($info),
	);

	CPLDirect::cpl_free_version_info($info_ptr);
	CPLDirect::delete_cpl_version_info_tpp($info_ptr_ptr);
	return %r;
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
  my $session = CPL::get_current_session();

  my $id  = CPL::create_object("com.example.myapp", "/bin/sh", "proc");
  my $id1 = CPL::create_object("com.example.myapp", "~/a.txt", "file", $id);
  my $id2 = CPL::lookup_object("com.example.myapp", "/bin/sh", "proc");
  my $id3 = CPL::try_lookup_object("com.example.myapp", "/bin/sh", "proc");
  if (%$id3 eq %$CPL::NONE) { warn "The object was not found." }

  CPL::data_flow($id1, $id);
  CPL::data_flow($id1, $id, $CPL::DATA_INPUT);
  CPL::data_flow_ext($id1, $id, 0);
  CPL::data_flow_ext($id1, $id, 0, $CPL::DATA_INPUT);

  CPL::control($id1, $id);
  CPL::control($id1, $id, $CPL::CONTROL_OP);
  CPL::control_ext($id1, $id, 0);
  CPL::control_ext($id1, $id, 0, $CPL::CONTROL_OP);

  my $ver1 = CPL::get_version($id1);
  my %info = CPL::get_object_info($id1);
  my %version_info = CPL::get_version_info($id1, 1);

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

  my $id = CPL::create_object($originator, $name, $type);
  my $id = CPL::create_object($originator, $name, $type, $container);

Creates a provenance object $id with the given $originator (the program
responsible for creating the objects; it also acts as a namespace), $name,
$type, and an optional $container. If the $container parameter is omitted
or set to $CPL::NONE, the created object would not belong to any container.

=head3 lookup_object

  my $id = CPL::lookup_object($originator, $name, $type);

Looks up a provenance object based on its $originator, $name, and $type
and returns its $id. If more than one object matches the search criteria,
return the one with the most recent timestamp.

=head3 try_lookup_object

  my $id = CPL::try_lookup_object($originator, $name, $type);

The same as CPL::lookup_object() described above, except that the function
call does not fail if the object does not exist, in which case it returns
$CPL::NONE in place of the object $id.

=head3 data_flow

  my $r = CPL::data_flow($dest, $source);
  my $r = CPL::data_flow($dest, $source, $type);

Declare a data flow to $dest from $source. This creates a data dependency,
asserting that $dest depends on the $source. The dependency relationship
will be tagged with the given $type. If $type is not specified, the library
uses $CPL::DATA_INPUT by default.

The return value is true if the dependency was added or false if it was found
to be a duplicate. Note that the current implementation of CPL does not
distinguish between different types of dependencies.

CPL recognizes the following types of data dependencies:

=over 2

=item $CPL::DATA_INPUT

The most generic type of data dependency.

=item $CPL::DATA_IPC

An IPC message that potentially involves a data transfer between two processes.
If the application programmer knows that the IPC did not involve any data,
(s)he should assert an appropriate control dependency instead.

=item $CPL::DATA_TRANSLATION

Translation of data, or potentially a part of the data, from one format to
another, or a translation of data serialized on a disk or in a network message
to its in-memory representation, or vice versa.

=item $CPL::DATA_COPY

An exact copy of the source data.

=back

=head3 data_flow_ext

  my $r = CPL::data_flow_ext($dest, $source_id, $source_version);
  my $r = CPL::data_flow_ext($dest, $source_id, $source_version, $type);

The same as CPL::data_flow(), except that it allows the application programmer
to specify the version of the source object.

=head3 control

  my $r = CPL::control($dest, $source);
  my $r = CPL::control($dest, $source, $type);

Declare that $dest received a control command from $source. This creates a
control dependency, asserting that $dest depends on the $source. The dependency
relationship will be tagged with the given $type. If $type is not specified,
the library uses $CPL::CONTROL_OP by default.

The return value is true if the dependency was added or false if it was found
to be a duplicate. Note that the current implementation of CPL does not
distinguish between different types of dependencies.

CPL recognizes the following types of control dependencies:

=over 2

=item $CPL::CONTROL_OP

The most generic type of control dependency.

=item $CPL::CONTROL_START

An assertion that the $source process started (executed) the $dest process.

=back

=head3 control_ext

  my $r = CPL::control_ext($dest, $source_id, $source_version);
  my $r = CPL::control_ext($dest, $source_id, $source_version, $type);

The same as CPL::control(), except that it allows the application programmer
to specify the version of the source object.

=head3 get_current_session

  my $session = CPL::get_current_session();

Return the ID of the current session of the provenance-aware application
(the caller).

=head3 get_version

  my $version = CPL::get_version($id);

Determine the current version of an object identified by the specified $id.

=head3 get_object_info

  my %info = CPL::get_object_info($id);

Determine the information about an object identified by the given $id, and
return a hash of its properties, such as its orginator, name, type, version,
container, creation time, and the session that created the object.

=head3 get_version_info

  my %info = CPL::get_version_info($id, $version);

Determine the information about a specific version of a provenancne object
(a node in the provenance graph) identified by the given $id and the $version
number. The function returns a hash of the node properties, such as its
creation time and the session that created it.



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
