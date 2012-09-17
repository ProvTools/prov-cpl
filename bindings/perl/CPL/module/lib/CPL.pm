#
# CPL.pm
# Core Provenance Library
#
# Copyright 2012
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

use 5.012003;
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
	attach_rdf
	detach
	id_eq
	create_object
	lookup_object
	lookup_or_create_object
	try_lookup_object
	lookup_all_objects
	data_flow
	control
	data_flow_ext
	control_ext
	new_version
    add_property
	get_current_session
	get_version
	get_session_info
	get_object_info
	get_version_info
    get_all_objects
    get_all_objects_fast
	get_object_ancestry
    get_properties
    lookup_by_property
    try_lookup_by_property
	get_object_for_file
	create_object_for_file
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
	attach_odbc
	attach_rdf
	detach
	id_eq
	create_object
	lookup_object
	lookup_or_create_object
	try_lookup_object
	lookup_all_objects
	data_flow
	control
	data_flow_ext
	control_ext
	new_version
    add_property
	get_current_session
	get_version
	get_session_info
	get_object_info
	get_version_info
    get_all_objects
    get_all_objects_fast
	get_object_ancestry
    get_properties
    lookup_by_property
    try_lookup_by_property
	get_object_for_file
	create_object_for_file
);

our $VERSION = '1.01';



#############################################################################
# Public API: Variables and Constants                                       #
#############################################################################

our $NONE = { hi => 0, lo => 0 };
*VERSION_NONE = *CPLDirect::CPL_VERSION_NONE;

*DATA_GENERIC = *CPLDirect::CPL_DATA_GENERIC;
*DATA_INPUT = *CPLDirect::CPL_DATA_INPUT;
*DATA_IPC = *CPLDirect::CPL_DATA_IPC;
*DATA_TRANSLATION = *CPLDirect::CPL_DATA_TRANSLATION;
*DATA_COPY = *CPLDirect::CPL_DATA_COPY;

*CONTROL_GENERIC = *CPLDirect::CPL_CONTROL_GENERIC;
*CONTROL_OP = *CPLDirect::CPL_CONTROL_OP;
*CONTROL_START = *CPLDirect::CPL_CONTROL_START;

*VERSION_GENERIC = *CPLDirect::CPL_VERSION_GENERIC;
*VERSION_PREV = *CPLDirect::CPL_VERSION_PREV;

*D_ANCESTORS = *CPLDirect::CPL_D_ANCESTORS;
*D_DESCENDANTS = *CPLDirect::CPL_D_DESCENDANTS;
*A_NO_PREV_NEXT_VERSION = *CPLDirect::CPL_A_NO_PREV_NEXT_VERSION;
*A_NO_DATA_DEPENDENCIES = *CPLDirect::CPL_A_NO_DATA_DEPENDENCIES;
*A_NO_CONTROL_DEPENDENCIES = *CPLDirect::CPL_A_NO_CONTROL_DEPENDENCIES;

our $F_LOOKUP_ONLY = 0;  # do not create if it does not already exist
*F_ALWAYS_CREATE = *CPLDirect::CPL_F_ALWAYS_CREATE;
*F_CREATE_IF_DOES_NOT_EXIST = *CPLDirect::CPL_F_CREATE_IF_DOES_NOT_EXIST;



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
# Initialize the CPL by opening a RDF/SPARQL connection
#
sub attach_rdf {
	my ($url_query, $url_update) = @_;

	my $backend_ptr = CPLDirect::new_cpl_db_backend_tpp();
	my $ret = CPLDirect::cpl_create_rdf_backend($url_query, $url_update,
			$CPLDirect::CPL_RDF_GENERIC, $backend_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_db_backend_tpp($backend_ptr);
		croak "Could not open a RDF/SPARQL connection: " . \
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
# Public API: Helpers                                                       #
#############################################################################


#
# Determine whether two IDs are equal
#
sub id_eq {
	my ($id1, $id2) = @_;

	# Is this the desired behavior?
	if (!defined($id1)) { $id1 = $NONE; }
	if (!defined($id2)) { $id2 = $NONE; }

	return %$id1 eq %$id2;
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
			#return $NONE;
			return undef;
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
# Look up an object by name. If multiple objects share the same name,
# return all of them.
#
sub lookup_all_objects {
	my ($originator, $name, $type) = @_;

	my $vector_ptr = CPLDirect::new_std_vector_cpl_id_timestamp_tp();
	my $ret = CPLDirect::cpl_lookup_object_ext($originator, $name, $type,
			$CPLDirect::CPL_L_NO_FAIL,
			$CPLDirect::cpl_cb_collect_id_timestamp_vector, $vector_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_std_vector_cpl_id_timestamp_tp($vector_ptr);
		croak "Could not lookup the given object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $vector =
		CPLDirect::cpl_dereference_p_std_vector_cpl_id_timestamp_t($vector_ptr);
	my $vector_size = CPLDirect::cpl_id_timestamp_t_vector::size($vector);

	my @r = ();
	for (my $i = 0; $i < $vector_size; $i++) {
		my $e = CPLDirect::cpl_id_timestamp_t_vector::get($vector, $i);

		my $e_id = CPLDirect::cpl_id_timestamp_t::swig_id_get($e);
		my $r_id = {
			hi => CPLDirect::cpl_id_t::swig_hi_get($e_id),
			lo => CPLDirect::cpl_id_t::swig_lo_get($e_id)
		};

		my $r_timestamp = CPLDirect::cpl_id_timestamp_t::swig_timestamp_get($e);

		my $r_element = {
			id        => $r_id,
			timestamp => $r_timestamp,
		};

		push @r, $r_element;
	}

	CPLDirect::delete_std_vector_cpl_id_timestamp_tp($vector_ptr);
	return @r;
}


#
# Lookup or create a provenance object
#
sub lookup_or_create_object {
	my ($originator, $name, $type, $container) = @_;

	if (!$container) {
		$container = $NONE;
	}

	my $c_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($c_ptr, $container->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($c_ptr, $container->{lo});
	my $c = CPLDirect::cpl_id_tp_value($c_ptr);

	my $obj_ptr = CPLDirect::new_cpl_id_tp();
	my $ret = CPLDirect::cpl_lookup_or_create_object($originator, $name, $type,
			$c, $obj_ptr);
	CPLDirect::delete_cpl_id_tp($c_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_id_tp($obj_ptr);
		croak "Could not lookup or create a provenance object:\n" .
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
sub control_flow {
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

	my $ret = CPLDirect::cpl_control_flow($d, $s, $type);
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
sub control_flow_ext {
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

	my $ret = CPLDirect::cpl_control_flow_ext($d, $s, $src_version, $type);
	CPLDirect::delete_cpl_id_tp($d_ptr);
	CPLDirect::delete_cpl_id_tp($s_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add a control dependency: " .
			CPLDirect::cpl_error_string($ret);
	}

	return $ret == $CPLDirect::CPL_S_DUPLICATE_IGNORED ? 0 : 1;
}


#
# Create a new version of a provenance object
#
sub new_version {
	my ($id) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $v_ptr = CPLDirect::new_cpl_version_tp();
	my $ret = CPLDirect::cpl_new_version($x, $v_ptr);
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_version_tp($v_ptr);
		croak "Could not create a new version of an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $v = CPLDirect::cpl_version_tp_value($v_ptr);
	CPLDirect::delete_cpl_version_tp($v_ptr);

	return $v;
}


#
# Add a property
#
sub add_property {
	my ($id, $key, $value) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $ret = CPLDirect::cpl_add_property($x, $key, $value);
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		croak "Could not add property to an object: " .
			CPLDirect::cpl_error_string($ret);
	}
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
# Get the session info
#
sub get_session_info {
	my ($id) = @_;

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $info_ptr_ptr = CPLDirect::new_cpl_session_info_tpp();
	my $ret = CPLDirect::cpl_get_session_info($x,
			CPLDirect::cpl_convert_pp_cpl_session_info_t($info_ptr_ptr));
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_session_info_tpp($info_ptr_ptr);
		croak "Could not determine information about a session: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $info_ptr = 
		CPLDirect::cpl_dereference_pp_cpl_session_info_t($info_ptr_ptr);
	my $info = CPLDirect::cpl_session_info_tp_value($info_ptr);

	my $info_id = CPLDirect::cpl_session_info_t::swig_id_get($info);
	my $r_id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($info_id),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($info_id)
	};

	my %r = (
		id                => $r_id,
		mac_address       => 
			CPLDirect::cpl_session_info_t::swig_mac_address_get($info),
		user              => 
			CPLDirect::cpl_session_info_t::swig_user_get($info),
		pid               => 
			CPLDirect::cpl_session_info_t::swig_pid_get($info),
		program           => 
			CPLDirect::cpl_session_info_t::swig_program_get($info),
		cmdline           => 
			CPLDirect::cpl_session_info_t::swig_cmdline_get($info),
		start_time        => 
			CPLDirect::cpl_session_info_t::swig_start_time_get($info),
	);

	CPLDirect::cpl_free_session_info($info_ptr);
	CPLDirect::delete_cpl_session_info_tpp($info_ptr_ptr);
	return %r;
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


#
# Get all objects in the database. If $fast is true, return incomplete
# information, but do so faster
#
sub get_all_objects {
	my ($fast) = @_;

    my $flags = 0;
    if ($fast) {
        $flags = $CPLDirect::CPL_I_FAST;
    }

	my $vector_ptr = CPLDirect::new_std_vector_cplxx_object_info_tp();
	my $ret = CPLDirect::cpl_get_all_objects($flags,
			$CPLDirect::cpl_cb_collect_object_info_vector, $vector_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_std_vector_cplxx_object_info_tp($vector_ptr);
		croak "Could not get all objects in the database: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $vector =
		CPLDirect::cpl_dereference_p_std_vector_cplxx_object_info_t($vector_ptr);
	my $vector_size = CPLDirect::cplxx_object_info_t_vector::size($vector);

	my @r = ();
	for (my $i = 0; $i < $vector_size; $i++) {
		my $info = CPLDirect::cplxx_object_info_t_vector::get($vector, $i);

		my $info_id = CPLDirect::cplxx_object_info_t::swig_id_get($info);
		my $r_id = {
			hi => CPLDirect::cpl_id_t::swig_hi_get($info_id),
			lo => CPLDirect::cpl_id_t::swig_lo_get($info_id)
		};

        my $info_session =
            CPLDirect::cplxx_object_info_t::swig_creation_session_get($info);
        my $r_session = {
           hi => CPLDirect::cpl_id_t::swig_hi_get($info_session),
           lo => CPLDirect::cpl_id_t::swig_lo_get($info_session)
        };

        my $info_container =
            CPLDirect::cplxx_object_info_t::swig_container_id_get($info);
        my $r_container = {
           hi => CPLDirect::cpl_id_t::swig_hi_get($info_container),
           lo => CPLDirect::cpl_id_t::swig_lo_get($info_container)
        };

        my $r_element;
        if ($fast) {
            $r_element = {
                id                => $r_id,
                creation_time     => 
                    CPLDirect::cplxx_object_info_t::swig_creation_time_get($info),
                originator        => 
                    CPLDirect::cplxx_object_info_t::swig_originator_get($info),
                name              => 
                    CPLDirect::cplxx_object_info_t::swig_name_get($info),
                type              => 
                    CPLDirect::cplxx_object_info_t::swig_type_get($info),
                container_id      => $r_container,
                container_version =>
                    CPLDirect::cplxx_object_info_t::swig_container_version_get($info),
            };
        }
        else {
            $r_element = {
                id                => $r_id,
                version           => 
                    CPLDirect::cplxx_object_info_t::swig_version_get($info),
                creation_session  => $r_session,
                creation_time     => 
                    CPLDirect::cplxx_object_info_t::swig_creation_time_get($info),
                originator        => 
                    CPLDirect::cplxx_object_info_t::swig_originator_get($info),
                name              => 
                    CPLDirect::cplxx_object_info_t::swig_name_get($info),
                type              => 
                    CPLDirect::cplxx_object_info_t::swig_type_get($info),
                container_id      => $r_container,
                container_version =>
                    CPLDirect::cplxx_object_info_t::swig_container_version_get($info),
            };
        }

		push @r, $r_element;
	}

    CPLDirect::delete_std_vector_cplxx_object_info_tp($vector_ptr);
	return @r;
}


#
# Get all objects in the database. This function runs faster, but it returns
# less information.
#
sub get_all_objects_fast {
    return get_all_objects(@_, 1);
}


#
# Get the object ancestry
#
sub get_object_ancestry {
	my ($id, $version, $direction, $flags) = @_;

	if (!defined($version)) { $version = $CPL::VERSION_NONE }
	if (!defined($flags)) { $flags = 0 }

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $vector_ptr = CPLDirect::new_std_vector_cpl_ancestry_entry_tp();
	my $ret = CPLDirect::cpl_get_object_ancestry($x, $version, $direction,
			$flags, $CPLDirect::cpl_cb_collect_ancestry_vector, $vector_ptr);
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_std_vector_cpl_ancestry_entry_tp($vector_ptr);
		croak "Could not determine ancestry of an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $vector =
		CPLDirect::cpl_dereference_p_std_vector_cpl_ancestry_entry_t(
				$vector_ptr);
	my $vector_size = CPLDirect::cpl_ancestry_entry_t_vector::size($vector);

	my @r = ();
	for (my $i = 0; $i < $vector_size; $i++) {
		my $e = CPLDirect::cpl_ancestry_entry_t_vector::get($vector, $i);

		my $e_query_object_id =
			CPLDirect::cpl_ancestry_entry_t::swig_query_object_id_get($e);
		my $r_query_object_id = {
			hi => CPLDirect::cpl_id_t::swig_hi_get($e_query_object_id),
			lo => CPLDirect::cpl_id_t::swig_lo_get($e_query_object_id)
		};
		my $r_query_object_version =
			CPLDirect::cpl_ancestry_entry_t::swig_query_object_version_get($e);

		my $e_other_object_id =
			CPLDirect::cpl_ancestry_entry_t::swig_other_object_id_get($e);
		my $r_other_object_id = {
			hi => CPLDirect::cpl_id_t::swig_hi_get($e_other_object_id),
			lo => CPLDirect::cpl_id_t::swig_lo_get($e_other_object_id)
		};
		my $r_other_object_version =
			CPLDirect::cpl_ancestry_entry_t::swig_other_object_version_get($e);

		my $r_type = CPLDirect::cpl_ancestry_entry_t::swig_type_get($e);

		my $r_element = {
			query_object_id      => $r_query_object_id,
			query_object_version => $r_query_object_version,
			other_object_id      => $r_other_object_id,
			other_object_version => $r_other_object_version,
			type                 => $r_type,
		};

		push @r, $r_element;
	}

	CPLDirect::delete_std_vector_cpl_ancestry_entry_tp($vector_ptr);
	return @r;
}


#
# Get properties of an object
#
sub get_properties {
	my ($id, $key, $version) = @_;

	if (!defined($version)) { $version = $CPL::VERSION_NONE }

	my $x_ptr = CPLDirect::new_cpl_id_tp();
	CPLDirect::cpl_id_t::swig_hi_set($x_ptr, $id->{hi});
	CPLDirect::cpl_id_t::swig_lo_set($x_ptr, $id->{lo});
	my $x = CPLDirect::cpl_id_tp_value($x_ptr);

	my $vector_ptr = CPLDirect::new_std_vector_cplxx_property_entry_tp();
	my $ret = CPLDirect::cpl_get_properties($x, $version, $key,
			$CPLDirect::cpl_cb_collect_properties_vector, $vector_ptr);
	CPLDirect::delete_cpl_id_tp($x_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_std_vector_cplxx_property_entry_tp($vector_ptr);
		croak "Could not get the properties of an object: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $vector =
		CPLDirect::cpl_dereference_p_std_vector_cplxx_property_entry_t(
				$vector_ptr);
	my $vector_size = CPLDirect::cplxx_property_entry_t_vector::size($vector);

	my @r = ();
	for (my $i = 0; $i < $vector_size; $i++) {
		my $e = CPLDirect::cplxx_property_entry_t_vector::get($vector, $i);

		my $r_element = {
            id      => $id,
			version => CPLDirect::cplxx_property_entry_t::swig_version_get($e),
			key     => CPLDirect::cplxx_property_entry_t::swig_key_get($e),
			value   => CPLDirect::cplxx_property_entry_t::swig_value_get($e),
		};

		push @r, $r_element;
	}

    CPLDirect::delete_std_vector_cplxx_property_entry_tp($vector_ptr);
	return @r;
}


#
# Look up objects by property
#
sub lookup_by_property {
	my ($key, $value, $ok_if_not_found) = @_;

	my $vector_ptr = CPLDirect::new_std_vector_cplxx_property_entry_tp();
	my $ret = CPLDirect::cpl_lookup_by_property($key, $value,
			$CPLDirect::cpl_cb_collect_properties_vector, $vector_ptr);

	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_std_vector_cplxx_property_entry_tp($vector_ptr);

		if ($ok_if_not_found) {
            my @r = ();
			return @r;
		}

		croak "Could not lookup objects by property: " .
			CPLDirect::cpl_error_string($ret);
	}

	my $vector =
		CPLDirect::cpl_dereference_p_std_vector_cplxx_property_entry_t(
				$vector_ptr);
	my $vector_size = CPLDirect::cplxx_property_entry_t_vector::size($vector);

	my @r = ();
	for (my $i = 0; $i < $vector_size; $i++) {
		my $e = CPLDirect::cplxx_property_entry_t_vector::get($vector, $i);

		my $e_id = CPLDirect::cplxx_property_entry_t::swig_id_get($e);
		my $r_id = {
			hi => CPLDirect::cpl_id_t::swig_hi_get($e_id),
			lo => CPLDirect::cpl_id_t::swig_lo_get($e_id)
		};

		my $r_element = {
            id      => $r_id,
			version => CPLDirect::cplxx_property_entry_t::swig_version_get($e),
		};

		push @r, $r_element;
	}

    CPLDirect::delete_std_vector_cplxx_property_entry_tp($vector_ptr);
	return @r;
}


#
# Look up objects by property, but do not fail if no such object is found
#
sub try_lookup_by_property {
	return lookup_by_property(@_, 1);
}



#############################################################################
# Public API: File API                                                      #
#############################################################################


#
# Get or create a provenance object for a file
#
sub get_object_for_file {
	my ($file_name, $mode) = @_;

	if (!defined($mode)) {
		$mode = $CPLDirect::CPL_F_CREATE_IF_DOES_NOT_EXIST;
	} 

	my $obj_ptr = CPLDirect::new_cpl_id_tp();
	my $v_ptr = CPLDirect::new_cpl_version_tp();
	my $ret = CPLDirect::cpl_lookup_file($file_name, $mode, $obj_ptr, $v_ptr);
	if (!CPLDirect::cpl_is_ok($ret)) {
		CPLDirect::delete_cpl_id_tp($obj_ptr);
		croak "Could not lookup or create a provenance object for a file: " .
			CPLDirect::cpl_error_string($ret) . "\n";
	}

	my $obj = CPLDirect::cpl_id_tp_value($obj_ptr);
	my $v = CPLDirect::cpl_version_tp_value($v_ptr);

	my $id = {
	   hi => CPLDirect::cpl_id_t::swig_hi_get($obj),
	   lo => CPLDirect::cpl_id_t::swig_lo_get($obj)
	};
	CPLDirect::delete_cpl_id_tp($obj_ptr);
	CPLDirect::delete_cpl_version_tp($v_ptr);

	return $id;
}


#
# Create a provenance object for a file
#
sub create_object_for_file {
	my ($file_name) = @_;
	return get_object_for_file($file_name, $CPLDirect::CPL_F_ALWAYS_CREATE);
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
  my $id3 = CPL::lookup_or_create_object("com.example.myapp","/bin/sh","proc");
  my $id4 = CPL::lookup_or_create_object("com.example.myapp", "~/a.txt",
                                         "file", $id);
  my $id5 = CPL::try_lookup_object("com.example.myapp", "/bin/sh", "proc");
  if (!defined($id5)) { warn "The object was not found."; }
  if (CPL::id_eq($id5, $id2)) { print "The two IDs are the same.\n"; }
  my @lst = CPL::lookup_all_objects("com.example.myapp", "/bin/sh", "proc");

  my @all_objs  = CPL::get_all_objects();
  my @all_objs2 = CPL::get_all_objects_fast();

  CPL::data_flow($id1, $id);
  CPL::data_flow($id1, $id, $CPL::DATA_INPUT);
  CPL::data_flow_ext($id1, $id, 0);
  CPL::data_flow_ext($id1, $id, 0, $CPL::DATA_INPUT);

  CPL::control($id1, $id);
  CPL::control($id1, $id, $CPL::CONTROL_OP);
  CPL::control_ext($id1, $id, 0);
  CPL::control_ext($id1, $id, 0, $CPL::CONTROL_OP);

  my $ver1 = CPL::get_version($id);
  my $ver2 = CPL::new_version($id);
  my %info = CPL::get_object_info($id);
  my %version_info = CPL::get_version_info($id, 0);

  my @ancestors  = CPL::get_object_ancestry($id, undef, $CPL::D_ANCESTORS);
  my @descenants = CPL::get_object_ancestry($id, undef, $CPL::D_DESCENDANTS);
  my @descenants_of_v0 = CPL::get_object_ancestry($id, 0, $CPL::D_DESCENDANTS);

  CPL::add_property($id, "dog", "fido");
  my @props    = CPL::get_properties($id);
  my @props_v0 = CPL::get_properties($id, undef, 0);
  my @dogs     = CPL::get_properties($id, "dog");
  my @dogs_v0  = CPL::get_properties($id, "dog", 0);
  my @fidos    = CPL::lookup_by_property("dog", "fido");
  my @fidos2   = CPL::try_lookup_by_property("dog", "fido");
  if ($#fidos2 == -1) { warn "No objects with dogs named \"fido\"."; }

  my $f1 = CPL::create_object_for_file("hello.txt");
  my $f2 = CPL::get_object_for_file("hello.txt");
  my $f3 = CPL::get_object_for_file("hello.txt", $CPL::F_LOOKUP_ONLY);

  # Error handling
  
  eval {
    my $x = CPL::lookup_object("com.example.myapp", "/bin/sh", "proc");
  };
  if ($@) {
    print "Error\n";
  }

  # Error handling using the Error module
  
  use Error qw(:try);
  try {
    my $x = CPL::lookup_object("com.example.myapp", "/bin/sh", "proc");
  }
  catch Error with {
    my $ex = shift;
    print "Error: $ex\n";
  };

  # Detach

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

=head3 attach_rdf

  CPL::attach_rdf($url_query, $url_update);

Initializes the Core Provenance Library for this program and attaches it to
the CPL's backend database via a RDF/SPARQL connection to the specfied
SPARQL query endpoint $url_query and to the update endpoint $url_update.

=head3 detach

  CPL::detach();

Detaches from the backend database and cleans up the CPL.

=head3 id_eq

  my $r = CPL::id_eq($id1, $id2);

Compares the two ID's $id1 and $id2 for equality. It sets $r to true if they
are equal or false if they are not. The function currently treats $CPL::NONE
and undef as synonyms, because both refer to an invalid/nonexistent object
or session.

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
undef instead of the object $id.

=head3 lookup_or_create_object

  my $id = CPL::lookup_or_create_object($originator, $name, $type);
  my $id = CPL::lookup_or_create_object($originator, $name, $type, $container);

Looks up a provenance object based on its $originator, $name, and $type
and returns its $id if it exists. If not, create a new object that satisfies
the given criteria and is optionally placed in the given $container.

=head3 lookup_all_objects

  my @lst = CPL::lookup_all_objects($originator, $name, $type);

Looks up all provenance objects with the matching $originator, $name, and
$type, and returns the list of hashes, where each hash containts the id of
the matching object and its timestamp.

=head3 get_all_objects

  my @lst = CPL::get_all_objects();

Returns an array of hashes with information about all provenance objects.

=head3 get_all_objects_fast

  my @lst = CPL::get_all_objects_fast();

Returns an array of hashes with information about all provenance objects with
less information than get_all_objects(), but faster.

=head3 data_flow

  my $r = CPL::data_flow($dest, $source);
  my $r = CPL::data_flow($dest, $source, $type);

Declares a data flow to $dest from $source. This creates a data dependency,
asserting that $dest depends on the $source. The dependency relationship
will be tagged with the given $type. If $type is not specified, the library
uses $CPL::DATA_INPUT by default.

The return value is true if the dependency was added or false if it was found
to be a duplicate. Note that the current implementation of CPL does not
distinguish between different types of dependencies.

CPL recognizes the following types of data dependencies:

=over 2

=item $CPL::DATA_GENERIC

The most generic type of data dependency.

=item $CPL::DATA_INPUT

The most generic type of data dependency -- an alias for $CPL::DATA_GENERIC.

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

Declares that $dest received a control command from $source. This creates a
control dependency, asserting that $dest depends on the $source. The dependency
relationship will be tagged with the given $type. If $type is not specified,
the library uses $CPL::CONTROL_OP by default.

The return value is true if the dependency was added or false if it was found
to be a duplicate. Note that the current implementation of CPL does not
distinguish between different types of dependencies.

CPL recognizes the following types of control dependencies:

=over 2

=item $CPL::CONTROL_GENERIC

The most generic type of control dependency.

=item $CPL::CONTROL_OP

The most generic type of control dependency -- an alias for
$CPL::CONTROL_GENERIC.

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

Returns the ID of the current session of the provenance-aware application
(the caller).

=head3 get_version

  my $version = CPL::get_version($id);

Determines the current version of an object identified by the specified $id.

=head3 new_version

  my $version = CPL::new_version($id);

Create a new version of an object identified by the specified $id and return
the new version number.

=head3 get_object_info

  my %info = CPL::get_object_info($id);

Determines the information about an object identified by the given $id, and
return a hash of its properties, such as its orginator, name, type, version,
container, creation time, and the session that created the object.

=head3 get_version_info

  my %info = CPL::get_version_info($id, $version);

Determines the information about a specific version of a provenancne object
(a node in the provenance graph) identified by the given $id and the $version
number. The function returns a hash of the node properties, such as its
creation time and the session that created it.

=head3 get_object_ancestry

  my @result = CPL::get_object_ancestry($id, $version, $direction);
  my @result = CPL::get_object_ancestry($id, $version, $direction, $flags);

Determines the list of ancestors or descendants of an object identified
by the given $id. If $version is specified, return only the list pertaining
to this version. If $version is undef, return all ancestors or descendants
of (all versions of) the object.

The $direction can be either $CPL::D_ANCESTORS or $CPL::D_DESCENDANTS.
The $flags are optional, and they are a logical "or" (using the | operator)
combination of flags such as $CPL::A_NO_DATA_DEPENDENCIES for ignoring all
data deoendencies or $CPL::A_NO_CONTROL_DEPENDENCIES for ignoring all control
dependencies.

In addition to the data and control dependencies described above, the function
can also return version dependencies:

=over 2

=item $CPL::VERSION_GENERIC

The most generic type of version dependency.

=item $CPL::VERSION_PREV

The most generic type of version dependency, which signifies that the ancestor
is the previous version of the descendant -- also an alias for
$CPL::VERSION_GENERIC.

=back

=head3 add_property

  CPL::add_property($id, $key, $value);

Adds a property with the specified $key and $value to the most recent version
of the object identified by the supplied $id.

=head3 get_properties

  my @lst = CPL::get_properties($id);
  my @lst = CPL::get_properties($id, $key);
  my @lst = CPL::get_properties($id, $key, $version);

Gets all properties of the given provenance object with the matching $key, if
it is specified; otherwise return all properties regardless of the key. If the
$version is specified, returns the properties associated with the given version
instead of returning the properties of all versions.

=head3 lookup_by_property

  my @lst = CPL::lookup_by_property($key, $value);

Finds and returns an array of provenance objects and their versions that have
a property with the matching $key and $value. Fails if no such objects are
found.

=head3 try_lookup_by_property

  my @lst = CPL::try_lookup_by_property($key, $value);

Just like lookup_by_property(), except that it returns an empty list instead of
failing if no such objects are found.

=head3 get_object_for_file

  my $f1 = CPL::get_object_for_file($file_name);
  my $f2 = CPL::get_object_for_file($file_name, $mode);

Gets a provenance object that corresponds to the given file on the file system
(the file must already exist), and depending on the $mode, creates it if not
found.

Please note that the CPL internally refers to the files using their full path,
so if you move the file by a utility that is not CPL-aware, a subsequent call
to this function with the same file (after it has been moved or renamed) will
not find the return back the same provenance object. Furthermore, beware that
if you use hard links, you will get different provenance objects for different
names/paths of the file.

The $mode can be one of the following values:

=over 2

=item $CPL::F_LOOKUP_ONLY

Perform only the lookup -- do not create the corresponding provenance object
if it does not already exists.

=item $CPL::F_CREATE_IF_DOES_NOT_EXIST

Create the corresponding provenance object if it does not already exist (this
is the default behavior).

=item $CPL::F_ALWAYS_CREATE

Always create a new corresponding provenance object, even if it already exists.
Use this if you completely overwrite the file.

=back

=head3 create_object_for_file

  my $f = CPL::create_object_for_file($file_name);

Creates a new provenance object that corresponds to the specified file. This is
equivalent to calling get_object_for_file() with $mode = $CPL::F_ALWAYS_CREATE.


=head1 HISTORY

=over 8

=item 1.01

Added CPL::new_version(), CPL::lookup_all_objects(), support for the file API,
and support for properties.
Minor changes to reflect changes in the underlying C API.

=item 1.00

Original version.

=back



=head1 AUTHOR

Peter Macko <pmacko@eecs.harvard.edu>



=head1 COPYRIGHT AND LICENSE

Copyright 2012
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


=cut
