#!/usr/bin/env python

#
# setup.py
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
# Contributor(s): Jackson Okuhn, Margo Seltzer, Peter Macko
#

'''
Class and functions supporting the Python bindings of the 'Core Provenance
Library <http://http://code.google.com/p/core-provenance-library/>'_.

This module contains:

*	The cpl class.
*	The cpl_relation class.
*	The cpl_session class.
*	The cpl_session_info class.
*	The cpl_object class.
*	The cpl_object_info class.
*	Helper functions to print and construct cpl objects
'''

import sys
import CPLDirect


#
# Constants
#

NONE = CPLDirect.CPL_NONE

# Relation Types
WASINFLUENCEDBY = CPLDirect.WASINFLUENCEDBY
ALTERNATEOF = CPLDirect.ALTERNATEOF
DERIVEDBYINSERTIONFROM = CPLDirect.DERIVEDBYINSERTIONFROM
DERIVEDBYREMOVALFROM = CPLDirect.DERIVEDBYREMOVALFROM
HADMEMBER = CPLDirect.HADMEMBER
HADDICTIONARYMEMBER = CPLDirect.HADDICTIONARYMEMBER
SPECIALIZATIONOF = CPLDirect.SPECIALIZATIONOF
WASDERIVEDFROM = CPLDirect.WASDERIVEDFROM
WASGENERATEDBY = CPLDirect.WASGENERATEDBY
WASINVALIDATEDBY = CPLDirect.WASINVALIDATEDBY
WASATTRIBUTEDTO = CPLDirect.WASATTRIBUTEDTO
USED = CPLDirect.USED
WASINFORMEDBY = CPLDirect.WASINFORMEDBY
WASSTARTEDBY = CPLDirect.WASSTARTEDBY
WASENDEDBY = CPLDirect.WASENDEDBY
HADPLAN = CPLDirect.HADPLAN
WASASSOCIATEDWITH = CPLDirect.WASASSOCIATEDWITH
ACTEDONBEHALFOF = CPLDirect.ACTEDONBEHALFOF
NUM_R_TYPES = CPLDirect. CPL_NUM_R_TYPES

# Object Types 
ENTITY = CPLDirect.CPL_ENTITY
ACTIVITY = CPLDirect.CPL_ACTIVITY
AGENT = CPLDirect.CPL_AGENT
NUM_O_TYPES = CPLDirect.CPL_NUM_O_TYPES

# Return Codes
S_OK = CPLDirect.CPL_S_OK
OK = CPLDirect.CPL_OK
S_DUPLICATE_IGNORED = CPLDirect.CPL_S_DUPLICATE_IGNORED
S_NO_DATA = CPLDirect.CPL_S_NO_DATA
S_OBJECT_CREATED = CPLDirect.CPL_S_OBJECT_CREATED
E_INVALID_ARGUMENT = CPLDirect.CPL_E_INVALID_ARGUMENT
E_INSUFFICIENT_RESOURCES = CPLDirect.CPL_E_INSUFFICIENT_RESOURCES
E_DB_CONNECTION_ERROR = CPLDirect.CPL_E_DB_CONNECTION_ERROR
E_NOT_IMPLEMENTED = CPLDirect.CPL_E_NOT_IMPLEMENTED
E_ALREADY_INITIALIZED = CPLDirect.CPL_E_ALREADY_INITIALIZED
E_NOT_INITIALIZED = CPLDirect.CPL_E_NOT_INITIALIZED
E_PREPARE_STATEMENT_ERROR = CPLDirect.CPL_E_PREPARE_STATEMENT_ERROR
E_STATEMENT_ERROR = CPLDirect.CPL_E_STATEMENT_ERROR
E_INTERNAL_ERROR = CPLDirect.CPL_E_INTERNAL_ERROR
E_BACKEND_INTERNAL_ERROR = CPLDirect.CPL_E_BACKEND_INTERNAL_ERROR
E_NOT_FOUND = CPLDirect.CPL_E_NOT_FOUND
E_ALREADY_EXISTS = CPLDirect.CPL_E_ALREADY_EXISTS
E_PLATFORM_ERROR = CPLDirect.CPL_E_PLATFORM_ERROR
E_INVALID_VERSION = CPLDirect.CPL_E_INVALID_VERSION
E_DB_NULL = CPLDirect.CPL_E_DB_NULL
E_DB_KEY_NOT_FOUND = CPLDirect.CPL_E_DB_KEY_NOT_FOUND
E_DB_INVALID_TYPE = CPLDirect.CPL_E_DB_INVALID_TYPE
E_INVALID_JSON = CPLDirect.CPL_E_INVALID_JSON

# Flags
L_NO_FAIL = CPLDirect.CPL_L_NO_FAIL
I_NO_CREATION_SESSION = CPLDirect.CPL_I_NO_CREATION_SESSION
I_FAST = CPLDirect.CPL_I_FAST
D_ANCESTORS = CPLDirect.CPL_D_ANCESTORS
D_DESCENDANTS = CPLDirect.CPL_D_DESCENDANTS



#
# Private constants
#

__relation_dict = ["wasInfluencedBy",
			   "alternateOf",
			   "derivedByInsertionFrom",
			   "derivedByRemovalFrom",
			   "hadMember",
			   "hadDictionaryMember",
			   "specializationOf",
			   "wasDerivedFrom",
			   "wasGeneratedBy",
			   "wasInvalidatedBy",
			   "wasAttributedTo",
			   "used",
			   "wasInformedBy",
			   "wasStartedBy",
			   "wasEndedBy",
			   "hadPlan",
			   "wasAssociatedWith",
			   "actedOnBehalfOf"]

__object_dict = ["entity",
				 "activity",
				 "agent"]
				 

#
# Global variables
#

_cpl_connection = None



#
# Private utility functions
#

def __getSignedNumber(number, bitLength):
	'''
	Print out a long value as a signed bitLength-sized integer.
	Thanks to:
	http://stackoverflow.com/questions/1375897/how-to-get-the-signed-integer-value-of-a-long-in-python
	for this function.
	'''
	mask = (2 ** bitLength) - 1
	if number & (1 << (bitLength - 1)):
		return number | ~mask
	else:
		return number & mask


#
# Public utility functions
#

def current_connection():
	'''
	Return the current CPL connection object, or None if not connected
	'''
	global _cpl_connection
	return _cpl_connection


def relation_type_to_str(val):
	'''
	Given a dependency (edge) type, convert it to a string

	Method calls::
		strval = relation_type_to_str(val)
	'''
	if val > 0 and val < NUM_R_TYPES :
		return __relation_dict[val-1];
	else:
		return 'unknown'


def p_id(id, with_newline = False):
	'''
	Print a CPL id, optionally with newline after it.

	Method calls::
		p_id(id, with_newline = False)
	'''
	sys.stdout.write('id:' + str(id) + ' ')
	if with_newline:
		sys.stdout.write('\n')


def p_object(obj, with_session = False):
	'''
	Print information about an object

	Method calls:
		p_object(obj, with_session = False)
	'''

	i = obj.info()
	p_id(i.object.id)
	sys.stdout.write('prefix:' + i.prefix + ' name:' + i.name +
	    ' type:' + __object_dict[i.type-1])
	if i.bundle is not None:
		sys.stdout.write(' bundle_id:' + str(i.bundle) + ' ')
	else:
		sys.stdout.write(' bundle_id:none ')
	if with_session:
		print 'creation_time:' + str(i.creation_time)
		p_session(i.creation_session)
	print


def p_bundle(bun, with_session = False):
	'''
	Print information about a bundle

	Method calls:
		p_bundle(bun, with_session = False)
	'''

	i = bun.info()
	p_id(i.bundle.id)
	sys.stdout.write(' name:' + i.name)
	if with_session:
		print 'creation_time:' + str(i.creation_time)
		p_session(i.creation_session)
	print


def p_session(session):
	'''
	Print information about a session

	Method calls:
		p_session(session)
	'''
	if session.id == 0:
		print 'no sesson'
	else:
		si = session.info()
		sys.stdout.write('session ')
		p_id(si.session.id, with_newline = True)
		print(' mac_address:' + si.mac_address + ' pid:' + str(si.pid))
		print('\t(' + str(si.start_time) + ')' + ' user:' +
		    si.user + ' cmdline:' + si.cmdline + ' program:' + si.program)



#
# Provenance relation
#

class cpl_relation:
	'''
	Stores the same data as a cpl_relation_t, but in a Python
	class that we manage.
	'''


	def __init__(self, id, aid, did, type, bundle, direction):
		'''
		Create an instance of cpl_relation_t
		'''
		self.id = id
		self.ancestor = cpl_object(cpl_object(aid))
		self.descendant = cpl_object(cpl_object(did))
		self.type = type
		self.bundle = bundle

		if direction == D_ANCESTORS:
			self.base  = self.descendant
			self.other = self.ancestor
		else:
			self.base  = self.ancestor
			self.other = self.descendant


	def __str__(self):
		'''
		Create a printable string representation of this object
		'''
		
		arrow = ' -- '
		if self.other == self.ancestor:
			arrow = ' --> '
		else:
			arrow = ' <-- '
		return (str(self.base) + arrow + str(self.other) +
			' [type:' + relation_type_to_str(self.type) +
			'; id: ' + self.id + ']')

	def properties(self, prefix=None, key=None):
		'''
		Return all the properties associated with the current relation.

		If key is set to something other than None, return only those
		properties matching key.
		'''
		vp = CPLDirect.new_std_vector_cplxx_property_entry_tp()

		ret = CPLDirect.cpl_get_relation_properties(self.id, prefix, key,
		    CPLDirect.cpl_cb_collect_properties_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
			raise Exception('Error retrieving properties: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.prefix, e.key, e.value])
		CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
		return l

	def add_property(self, prefix, name, value):
		'''
		Add name/value pair as a property to current object.
		'''
		return CPLDirect.cpl_add_relation_property(self.id, prefix, name, value)


#
# CPL Connection
#

class cpl_connection:
	'''
	Core provenance library connection -- maintains state for the current
	session and the current database backend.
	'''


	def __init__(self, cstring="DSN=CPL;"):
		'''
		Constructor for CPL connection.

		** Parameters **
			** cstring **
			Connection string for database backend

		** Note **
		Currently the python bindings support only ODBC connection.
		RDF connector coming soon.
		'''
		global _cpl_connection

		self.connection_string = cstring
		self.closed = False

		def get_current_session():
			ret, id = CPLDirect.cpl_get_current_session()

			if not CPLDirect.cpl_is_ok(ret):
				raise Exception("Could not get current session " +
				       CPLDirect.cpl_error_string(ret))

			return id

		backend = CPLDirect.new_cpl_db_backend_tpp()
		ret = CPLDirect.cpl_create_odbc_backend(cstring,
		    CPLDirect.CPL_ODBC_GENERIC, backend)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception("Could not create ODBC connection " +
			       CPLDirect.cpl_error_string(ret))
		self.db = CPLDirect.cpl_dereference_pp_cpl_db_backend_t(backend)
		ret = CPLDirect.cpl_attach(self.db)
		CPLDirect.delete_cpl_db_backend_tpp(backend)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception("Could not open ODBC connection " +
			       CPLDirect.cpl_error_string(ret))
		self.session = cpl_session(get_current_session())

		_cpl_connection = self


	def __del__(self):
		'''
		Destructor - automatically closes the connection.
		'''
		if self == _cpl_connection and not self.closed:
			self.close()


	def __create_or_lookup_cpl_object(self, prefix,
		     name, type=None, create=None, bundle=None):
		'''
		Create or lookup a CPL object

		** Parameters **
			prefix 
			name: name
			type: type, type can be none for lookup
			create:
				None: lookup or create
				True: create only
				False: lookup only
			bundle:
				Id of bundle into which to place this object, can be none
		'''
		if bundle == None:
			bundle_id = NONE
		else:
			bundle_id = bundle.id

		if create == None:
			ret, idp = CPLDirect.cpl_lookup_or_create_object(prefix, name,
							  type, bundle_id)
			if ret == S_OBJECT_CREATED:
				ret = S_OK
		elif create:
			ret, idp = CPLDirect.cpl_create_object(prefix,
						name, type, bundle_id)
		else:
			ret, idp = CPLDirect.cpl_lookup_object(prefix, name, type, bundle_id)
			if ret == E_NOT_FOUND:
				raise LookupError('Not found')
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not find or create' +
			    ' provenance object: ' + CPLDirect.cpl_error_string(ret))
		
		r = cpl_object(idp)
		return r


	def create_bundle(self, name):
		ret, idp = CPLDirect.cpl_create_bundle(name)

		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not create' +
			    ' provenance bundle: ' + CPLDirect.cpl_error_string(ret))
		
		r = cpl_bundle(idp)
		return r


	def lookup_bundle(self, name):
		ret, idp = CPLDirect.cpl_lookup_bundle(name)

		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not find' +
			    ' provenance bundle: ' + CPLDirect.cpl_error_string(ret))
		
		r = cpl_bundle(idp)
		return r


	def lookup_all_bundles(self, name):
		'''
		Return all bundles that have the specified name
		'''

		vp = CPLDirect.new_std_vector_cpl_id_timestamp_tp()
		ret = CPLDirect.cpl_lookup_bundle_ext(name,
			L_NO_FAIL, CPLDirect.cpl_cb_collect_id_timestamp_vector, vp)

		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
			raise Exception('Unable to lookup all bundles: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_timestamp_t(vp)
		l = []
		if v != S_NO_DATA :
			for e in v:
				l.append(cpl_object(e.id))

		CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
		return l


	def get_all_objects(self, fast=False):
		'''
		Return all objects in the provenance database. If fast = True, then
		fetch only incomplete information about each object, so that it is
		faster.
		'''

		if fast:
			flags = CPLDirect.CPL_I_FAST
		else:
			flags = 0

		vp = CPLDirect.new_std_vector_cplxx_object_info_tp()
		ret = CPLDirect.cpl_get_all_objects(flags,
			CPLDirect.cpl_cb_collect_object_info_vector, vp)

		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
			raise Exception('Unable to get all objects: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_object_info_t(vp)
		l = []
		if v != S_NO_DATA :
			for e in v:
				if e.bundle_id == NONE:
					bundle = None
				else:
					bundle = cpl_object(e.bundle_id)
				if e.creation_session == NONE:
					creation_session = None
				else:
					creation_session = cpl_session(e.creation_session)
				l.append(cpl_object_info(cpl_object(e.id),
					creation_session, e.creation_time, e.prefix, e.name,
					e.type, bundle))

		CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
		return l
			

	def get_object(self, prefix, name, type, bundle):
		'''
		Get the object, with the designated prefix (string),
		name (int), type (int), and bundle (ID) creating it if necessary.

		'''
		return self.__create_or_lookup_cpl_object(prefix, name, type,
				create=None, bundle=bundle)


	def create_object(self, prefix, name, type, bundle):
		'''
		Create object, returns None if object already exists.
		'''
		return self.__create_or_lookup_cpl_object(prefix, name, type,
				create=True, bundle=bundle)


	def lookup_object(self, prefix, name, type, bundle=None):
		'''
		Look up object; raise LookupError if the object does not exist.
		'''
		o = self.__create_or_lookup_cpl_object(prefix, name, type,
				create=False, bundle=bundle)
		return o


	def try_lookup_object(self, prefix, name, type, bundle=None):
		'''
		Look up object; returns None if the object does not exist.
		'''
		try:
			o = self.__create_or_lookup_cpl_object(prefix, name, type,
					create=False, bundle=bundle)
		except LookupError:
			o = None
		return o


	def lookup_by_property(self, prefix, key, value):
		'''
		Return all objects that have the key/value property specified; raise
		LookupError if no such object is found.
		'''
		vp = CPLDirect.new_std_vector_cpl_id_tp()
		ret = CPLDirect.cpl_lookup_object_by_property(prefix, key, value,
			CPLDirect.cpl_cb_collect_property_lookup_vector, vp)

		if ret == E_NOT_FOUND:
			CPLDirect.delete_std_vector_cpl_id_tp(vp)
			raise LookupError('Not found')
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_id_tp(vp)
			raise Exception('Unable to lookup by property ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_t(vp)
		l = []
		for e in v:
			l.append(cpl_object(e))

		CPLDirect.delete_std_vector_cpl_id_tp(vp)
		return l


	def try_lookup_by_property(self, prefix, key, value):
		'''
		Return all objects that have the key/value property specified, but do
		not fail if no such object is found -- return an empty list instead.
		'''
		try:
			o = self.lookup_by_property(prefix, key, value)
		except LookupError:
			o = []
		return o


	def lookup_all_objects(self, prefix, name, type, bundle):
		'''
		Return all objects that have the specified prefix, name,
		type, or bundle.
		'''
		
		bundle = bundle.id
		vp = CPLDirect.new_std_vector_cpl_id_timestamp_tp()
		ret = CPLDirect.cpl_lookup_object_ext(prefix, name, type, bundle,
			L_NO_FAIL, CPLDirect.cpl_cb_collect_id_timestamp_vector, vp)

		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
			raise Exception('Unable to lookup all objects: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_timestamp_t(vp)
		l = []
		if v != S_NO_DATA :
			for e in v:
				l.append(cpl_object(e.id))

		CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
		return l


	def get_bundle_objects(self, bundle):
		'''
		Return all objects in the specified bundle.
		'''
		vp = CPLDirect.new_std_vector_cplxx_object_info_tp()
		ret = CPLDirect.cpl_get_bundle_objects(bundle.id, CPLDirect.cpl_cb_collect_object_info_vector, vp)

		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_object_info_tp(vp)
			raise Exception('Unable to lookup all objects: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_object_info_t(vp)
		l = []
		if v != S_NO_DATA :
			for e in v:
				l.append(cpl_object(e.id))

		CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
		return l
		


	def get_bundle_relations(self, bundle):
		'''
		Return all relations in the specified bundle.
		'''
		vp = CPLDirect.new_std_vector_cpl_relation_tp()

		ret = CPLDirect.cpl_get_bundle_relations(bundle.id,
			CPLDirect.cpl_cb_collect_relation_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_relation_tp(vp)
			raise Exception('Error retrieving relations: ' +
					CPLDirect.cpl_error_string(ret))
			return None

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_relation_t(vp)
		l = []
		for entry in v:
			a = cpl_relation(entry.id, entry.query_object_id,
				entry.other_object_id, entry.type, 
				entry.bundle_id, D_DESCENDANTS)
			l.append(a)

		CPLDirect.delete_std_vector_cpl_relation_tp(vp)
		return l


	def delete_bundle(self, bundle):
		'''
		Delete the specified bundle and everything in it.
		'''
		ret = CPLDirect.cpl_delete_bundle(bundle.id)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Error deleting bundle: ' +
					CPLDirect.cpl_error_string(ret))
		return None


	def validate_json(self, filepath):
		'''
		Checks a Prov-JSON document (at filepath) for cycles and correctness.
		'''
		ret = CPLDirect.validate_json(filepath)
		if not CPLDirect.cpl_is_ok(ret.return_code):
			return None
		return ret.out_string


	def import_document_json(self, filepath,
			bundle_name, anchor_objects):
		'''
		Imports a Prov-JSON document into the CPL as a bundle.

		** Parameters **
			filepath
			prefix
			bundle_name
			anchor_objects: a list of cpl_object, name tuples, can be None
			bundle_agent: the agent responsible for uploading the bundle, can be None
		'''
		if anchor_objects == None:
			id_name_vector = none
		else:
			id_name_pairs = [(entry.get(0).id, entry.get(1)) for entry in anchor_objects]
			id_name_vector = CPLDirect.cplxx_id_name_pair_vector(id_name_pairs)
		ret, idp = CPLDirect.import_document_json(filepath, bundle_name,
			  id_name_vector)
		if not CPLDirect.cpl_is_ok(ret): 
			raise Exception('Error importing document:' +
					CPLDirect.cpl_error_string(ret))
		return idp
		

	def export_bundle_json(self, bundles, filepath):
		'''
		Exports a bundle as a Prov-JSON document.
		'''
		bundle_ids = [bundle.id for bundle in bundles]
		bundles_vec = CPLDirect.cpl_id_t_vector(bundle_ids);
		ret = CPLDirect.export_bundle_json(bundles_vec, filepath)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Error exporting bundle:' +
					CPLDirect.cpl_error_string(ret))
		return None


	def close(self):
		'''
		Close database connection and session
		'''
		global _cpl_connection
		
		if self != _cpl_connection or self.closed:
			return

		ret = CPLDirect.cpl_detach()
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not detach ' +
					CPLDirect.cpl_error_string(ret))

		_cpl_connection = None
		self.closed = True



#
# Information about a provenance session
#

class cpl_session_info:
	'''
	Information about a provenance session
	'''

	def __init__(self, session, mac_address, user, pid, program, cmdline,
			start_time):
		'''
		Create an instance of this object
		'''

		self.session = session
		self.mac_address = mac_address
		self.user = user
		self.pid = pid
		self.program = program
		self.cmdline = cmdline
		self.start_time = start_time



#
# CPL Session
#

class cpl_session:
	'''
	CPL Session
	'''


	def __init__(self, id):
		'''
		Initialize an instance of cpl_session
		'''
		self.id = id


	def __eq__(self, other):
		'''
		Compare this and the other object and return true if they are equal
		'''
		if isinstance(other, self.__class__):
			return self.id == other.id
		else:
			return False


	def __ne__(self, other):
		'''
		Compare this and the other object and return true if they are not equal
		'''
		return self.id != other.id


	def __str__(self):
		'''
		Return a string representation of this object
		'''
		return str(self.id)


	def info(self):
		'''
		Return the cpl_session_info object associated with this session.
		'''

		sessionpp = CPLDirect.new_cpl_session_info_tpp()
		ret = CPLDirect.cpl_get_session_info(self.id,
		    CPLDirect.cpl_convert_pp_cpl_session_info_t(sessionpp))
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_session_info_tpp(sessionpp)
			raise Exception('Could not find session information: ' +
					CPLDirect.cpl_error_string(ret))

		sessionp = CPLDirect.cpl_dereference_pp_cpl_session_info_t(sessionpp)
		info = CPLDirect.cpl_session_info_tp_value(sessionp)

		_info = cpl_session_info(self, info.mac_address, info.user,
				info.pid, info.program, info.cmdline, info.start_time)
		
		CPLDirect.cpl_free_session_info(sessionp)
		CPLDirect.delete_cpl_session_info_tpp(sessionpp)
		
		return _info



#
# Information about a provenance object
#

class cpl_object_info:
	'''
	Information about a provenance object
	'''

	def __init__(self, object, creation_session, creation_time,
			prefix, name, type, bundle):
		'''
		Create an instance of this object
		'''

		self.object = object
		self.creation_session = creation_session
		self.creation_time = creation_time
		self.prefix = prefix
		self.name = name
		self.type = type
		self.bundle = bundle



#
# CPL Provenance object
#

class cpl_object:
	'''
	CPL Provenance object
	'''


	def __init__(self, id):
		'''
		Create a new instance of a provenance object from its internal ID
		'''
		self.id = id


	def __eq__(self, other):
		'''
		Compare this and the other object and return true if they are equal
		'''
		if isinstance(other, self.__class__):
			return self.id == other.id
		else:
			return False


	def __ne__(self, other):
		'''
		Compare this and the other object and return true if they are not equal
		'''
		return self.id != other.id


	def __str__(self):
		'''
		Return a string representation of this object
		'''
		return str(self.id)


	def relation_to(self, dest, type, bundle):
		'''
		Add relation type from self to dest.
		'''

		bundle = bundle.id
		ret, idp = CPLDirect.cpl_add_relation(self.id, dest.id, type, bundle)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add relation: ' +
					CPLDirect.cpl_error_string(ret))

		r = cpl_relation(idp, dest.id, self.id, type, bundle, D_ANCESTORS)

		return r


	def relation_from(self, src, type, bundle):
		'''
		Add relation type from src to self.
		'''

		bundle = bundle.id
		ret, idp = CPLDirect.cpl_add_relation(src.id, self.id, type, bundle)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add relation: ' +
					CPLDirect.cpl_error_string(ret))

		r = cpl_relation(idp, self.id, src.id, type, bundle, D_DESCENDANTS)

		return r


	def add_property(self, prefix, name, value):
		'''
		Add name/value pair as a property to current object.
		'''
		return CPLDirect.cpl_add_object_property(self.id, prefix, name, value)


	def info(self):
		'''
		Return cpl_object_info_t corresponding to the current object.
		'''
		objectpp = CPLDirect.new_cpl_object_info_tpp()

		ret = CPLDirect.cpl_get_object_info(self.id,
		    CPLDirect.cpl_convert_pp_cpl_object_info_t(objectpp))
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_object_info_tpp(objectpp)
			raise Exception('Unable to get object info: ' +
					CPLDirect.cpl_error_string(ret))

		op = CPLDirect.cpl_dereference_pp_cpl_object_info_t(objectpp)
		object = CPLDirect.cpl_object_info_tp_value(op)

		if object.bundle_id == NONE:
			bundle = None
		else:
			bundle = cpl_object(object.bundle_id)

		_info = cpl_object_info(self,
				cpl_session(object.creation_session), object.creation_time,
				object.prefix, object.name, object.type, bundle)

		CPLDirect.cpl_free_object_info(op)
		CPLDirect.delete_cpl_object_info_tpp(objectpp)

		return _info


	def relations(self, direction=D_ANCESTORS, flags=0):
		'''
		Return a list of cpl_ancestor objects
		'''
		vp = CPLDirect.new_std_vector_cpl_relation_tp()

		ret = CPLDirect.cpl_get_object_relations(self.id,
		    direction, flags, CPLDirect.cpl_cb_collect_relation_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_relation_tp(vp)
			raise Exception('Error retrieving relations: ' +
					CPLDirect.cpl_error_string(ret))
			return None

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_relation_t(vp)
		l = []
		if direction == D_ANCESTORS:
			for entry in v:
				a = cpl_relation(entry.id, entry.other_object_id,
					entry.query_object_id, entry.type, 
					entry.bundle_id, direction)
				l.append(a)
		else:
			for entry in v:
				a = cpl_relation(entry.id, entry.query_object_id,
					entry.other_object_id, entry.type, 
					entry.bundle_id, direction)
				l.append(a)

		CPLDirect.delete_std_vector_cpl_relation_tp(vp)
		return l


	def properties(self, prefix=None, key=None, version=None):
		'''
		Return all the properties associated with the current object.

		If prefix/key is set to something other than None, return only those
		properties matching prefix/key.
		'''
		vp = CPLDirect.new_std_vector_cplxx_property_entry_tp()

		ret = CPLDirect.cpl_get_object_properties(self.id, prefix, key,
		    CPLDirect.cpl_cb_collect_properties_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
			raise Exception('Error retrieving properties: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.prefix, e.key, e.value])
		CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
		return l



#
# Information about a provenance bundle
#

class cpl_bundle_info:
	'''
	Information about a provenance bundle
	'''

	def __init__(self, bundle, creation_session, creation_time,
			name):
		'''
		Create an instance of this object
		'''

		self.bundle = bundle
		self.creation_session = creation_session
		self.creation_time = creation_time
		self.name = name



#
# CPL Provenance bundle
#

class cpl_bundle:
	'''
	CPL Provenance bundle
	'''


	def __init__(self, id):
		'''
		Create a new instance of a provenance bundle from its internal ID
		'''
		self.id = id


	def __eq__(self, other):
		'''
		Compare this and the other bundle and return true if they are equal
		'''
		if isinstance(other, self.__class__):
			return self.id == other.id
		else:
			return False


	def __ne__(self, other):
		'''
		Compare this and the other object and return true if they are not equal
		'''
		return self.id != other.id


	def __str__(self):
		'''
		Return a string representation of this object
		'''
		return str(self.id)

	def add_prefix(self, prefix, iri):
		'''
		Add prefix/iri pair to current bundle.
		'''
		return CPLDirect.cpl_add_prefix(self.id, prefix, iri)

	def add_property(self, prefix, name, value):
		'''
		Add prefix/name/value as a property to current bundle.
		'''
		return CPLDirect.cpl_add_bundle_property(self.id, prefix, name, value)


	def info(self):
		'''
		Return cpl_bundle_info_t corresponding to the current bundle.
		'''
		bundlepp = CPLDirect.new_cpl_bundle_info_tpp()

		ret = CPLDirect.cpl_get_bundle_info(self.id,
		    CPLDirect.cpl_convert_pp_cpl_bundle_info_t(bundlepp))
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_bundle_info_tpp(bundlepp)
			raise Exception('Unable to get bundle info: ' +
					CPLDirect.cpl_error_string(ret))

		op = CPLDirect.cpl_dereference_pp_cpl_bundle_info_t(bundlepp)
		bundle = CPLDirect.cpl_bundle_info_tp_value(op)

		_info = cpl_bundle_info(self,
				cpl_session(bundle.creation_session), bundle.creation_time,
				bundle.name)

		CPLDirect.cpl_free_bundle_info(op)
		CPLDirect.delete_cpl_bundle_info_tpp(bundlepp)

		return _info

	def properties(self, prefix=None, key=None, version=None):
		'''
		Return all the properties associated with the current bundle.

		If prefix/key is set to something other than None, return only those
		properties matching prefix/key.
		'''
		vp = CPLDirect.new_std_vector_cplxx_property_entry_tp()

		ret = CPLDirect.cpl_get_bundle_properties(self.id, prefix, key,
		    CPLDirect.cpl_cb_collect_properties_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
			raise Exception('Error retrieving properties: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.prefix, e.key, e.value])
		CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
		return l

