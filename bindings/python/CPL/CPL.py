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
NUM_R_TYPES = CPLDirect. NUM_R_TYPES

# Object Types 
ENTITY = CPLDirect.ENTITY
ACTIVITY = CPLDirect.ACTIVITY
AGENT = CPLDirect.AGENT
BUNDLE = CPLDirect.BUNDLE
NUM_O_TYPES = CPLDirect.NUM_O_TYPES

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
				 "agent",
				 "bundle"]
				 

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
		return __relation_dict[val];
	else:
		return 'unknown'


def p_id(id, with_newline = False):
	'''
	Print a CPL id, optionally with newline after it.

	Method calls::
		p_id(id, with_newline = False)
	'''
	sys.stdout.write('id: ' + str(id))
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
	sys.stdout.write('bundle_ ')
	if i.bundle is not None:
		p_id(i.bundle.object.id)
	else:
		sys.stdout.write('id: none')
	print('originator: ' + i.originator + ' name:' + i.name +
	    ' type: ' + __object_dict[i.type])
	if with_session:
		print('creation_time: ' + str(i.creation_time))
		p_session(i.creation_session)


def p_session(session):
	'''
	Print information about a session

	Method calls:
		p_session(session)
	'''
	si = session.info()
	sys.stdout.write('session ')
	p_id(si.session.id, with_newline = True)
	print(' mac_address: ' + si.mac_address + ' pid: ' + str(si.pid))
	print('\t(' + str(si.start_time) + ')' + ' user: ' +
	    si.user + ' cmdline: ' + si.cmdline + ' program: ' + si.program)

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

	def properties(self, key=None, version=None):
		'''
		Return all the properties associated with the current relation.

		If key is set to something other than None, return only those
		properties matching key.
		'''
		vp = CPLDirect.new_std_vector_cplxx_r_property_entry_tp()

		ret = CPLDirect.cpl_get_relation_properties(self.id, key,
		    CPLDirect.cpl_cb_collect_properties_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
			raise Exception('Error retrieving properties: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.key, e.value])
		CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
		return l

	def add_property(self, name, value):
		'''
		Add name/value pair as a property to current object.
		'''
		return CPLDirect.cpl_add_relation_property(self.id, name, value)


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
			idp = NONE
			ret = CPLDirect.cpl_get_current_session(idp)

			if not CPLDirect.cpl_is_ok(ret):
				CPLDirect.delete_cpl_id_tp(idp)
				raise Exception("Could not get current session" +
				       CPLDirect.cpl_error_string(ret))

			s = CPLDirect.cpl_id_tp_value(idp)
			i = copy_id(s)
			CPLDirect.delete_cpl_id_tp(idp)
			return i

		backend = CPLDirect.new_cpl_db_backend_tpp()
		ret = CPLDirect.cpl_create_odbc_backend(cstring,
		    CPLDirect.CPL_ODBC_GENERIC, backend)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception("Could not create ODBC connection" +
			       CPLDirect.cpl_error_string(ret))
		self.db = CPLDirect.cpl_dereference_pp_cpl_db_backend_t(backend)
		ret = CPLDirect.cpl_attach(self.db)
		CPLDirect.delete_cpl_db_backend_tpp(backend)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception("Could not open ODBC connection" +
			       CPLDirect.cpl_error_string(ret))
		self.session = cpl_session(get_current_session())

		_cpl_connection = self


	def __del__(self):
		'''
		Destructor - automatically closes the connection.
		'''
		if self == _cpl_connection and not self.closed:
			self.close()


	def __create_or_lookup_cpl_object(self, originator,
		     name, type=None, create=None, bundle=None):
		'''
		Create or lookup a CPL object

		** Parameters **
			originator 
			name: originator-local name
			type: originator-local type, type can be none for lookup
			create:
				None: lookup or create
				True: create only
				False: lookup only
			bundle:
				Id of bundle into which to place this object.
				Only applies to create
		'''
		if bundle == None:
			bundle_id = NONE
		else:
			bundle_id = bundle.id

		idp = NONE
		if create == None:
			ret = CPLDirect.cpl_lookup_or_create_object(originator, name,
							  type, bundle_id, idp)
			if ret == S_OBJECT_CREATED:
				ret = S_OK
		elif create:
			ret = CPLDirect.cpl_create_object(originator,
						name, type, bundle_id, idp)
		else:
			ret = CPLDirect.cpl_lookup_object(originator, name, type, idp)

			raise LookupError('Not found')
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not find or create' +
			    ' provenance object: ' + CPLDirect.cpl_error_string(ret))
			
		r = cpl_object(idp)

		return r


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
				if e.bundler_id == NONE:
					bundle = None
				else:
					bundle = cpl_object(e.bundle_id)
				if e.creation_session == NONE:
					creation_session = None
				else:
					creation_session = cpl_session(e.creation_session)
				l.append(cpl_object_info(cpl_object(e.id),
					creation_session, e.creation_time, e.originator, e.name,
					e.type, bundle))

		CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
		return l
			

	def get_object(self, originator, name, type, bundle=None):
		'''
		Get the object, with the designated originator (string),
		name (int), and type (int), creating it if necessary.

		If you want an object in a specific bundle, set the bundle
		parameter to the ID of the object in which you want this object
		created.
		'''
		return self.__create_or_lookup_cpl_object(originator, name, type,
				create=None, bundle=bundle)


	def create_object(self, originator, name, type, bundle=None):
		'''
		Create object, returns None if object already exists.
		'''
		return self.__create_or_lookup_cpl_object(originator, name, type,
				create=True, bundle=bundle)


	def lookup_object(self, originator, name, type, bundle=None):
		'''
		Look up object; raise LookupError if the object does not exist.
		'''
		o = self.__create_or_lookup_cpl_object(originator, name, type,
				create=False)
		return o


	def try_lookup_object(self, originator, name, type, bundle=None):
		'''
		Look up object; returns None if the object does not exist.
		'''
		try:
			o = self.__create_or_lookup_cpl_object(originator, name, type,
					create=False)
		except LookupError:
			o = None
		return o


	def lookup_by_property(self, key, value):
		'''
		Return all objects that have the key/value property specified; raise
		LookupError if no such object is found.
		'''
		vp = CPLDirect.new_std_vector_cpl_id_tp()
		ret = CPLDirect.cpl_lookup_by_property(key, value,
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
			l.append(cpl_object(e.id))

		CPLDirect.delete_std_vector_cpl_id_tp(vp)
		return l


	def try_lookup_by_property(self, key, value):
		'''
		Return all objects that have the key/value property specified, but do
		not fail if no such object is found -- return an empty list instead.
		'''
		try:
			o = self.lookup_by_property(key, value)
		except LookupError:
			o = []
		return o


	def lookup_all(self, originator, name, type, bundle):
		'''
		Return all objects that have the specified originator, name,
		type, or bundle.
		'''
		vp = CPLDirect.new_std_vector_cpl_id_timestamp_tp()
		ret = CPLDirect.cpl_lookup_object_ext(originator, name, type, bundle,
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


	def get_bundle_objects(bundle):
		vp = CPLDirect.new_std_vector_cpl_id_timestamp_tp()
		ret = CPLDirect.cpl_get_bundle_objects(bundle.id, CPLDirect.cpl_cb_collect_id_timestamp_vector, vp)

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
		


	def get_bundle_relations(bundle):
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
			a = cpl_ancestor(entry.id, entry.query_object_id,
				entry.other_object_id, entry.type, 
				entry.bundle, D_DESCENDANTS)
			l.append(a)

		CPLDirect.delete_std_vector_cpl_relation_tp(vp)
		return l

	def delete_bundle(bundle):
		ret = CPLDirect.cpl_delete_bundle(bundle.id)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Error deleting bundle: ' +
					CPLDirect.cpl_error_string(ret))
		return None


	def validate_json(filepath):
		ret = CPLDirect.validate_json(filepath, stringout)
		if not r:
			return None
		return stringout


	def import_document_json(filepath, originator, 
			bundle_name, anchor_object, bundle_agent):
		ret = CPLDirect.import_document_json(filepath, originator, bundle_name,
			  anchor_object.id, bundle_agent.id, out_id)
		if not CPLDirect.cpl_is_ok(ret): 
			raise Exception('Error importing document:' +
					CPLDirect.cpl_error_string(ret))
		return out_id
		

	def export_bundle_json(bundle, filepath):
		ret = CPLDirect.export_bundle_json(bundle.id, filepath)
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
		self.id = copy_id(id)


	def __eq__(self, other):
		'''
		Compare this and the other object and return true if they are equal
		'''
		return self.id == other.id


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
			originator, name, type, bundle):
		'''
		Create an instance of this object
		'''

		self.object = object
		self.creation_session = creation_session
		self.creation_time = creation_time
		self.originator = originator
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
		self.id = copy_id(id)


	def __eq__(self, other):
		'''
		Compare this and the other object and return true if they are equal
		'''
		return self.id == other.id


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
		ret = CPLDirect.cpl_add_relation(self.id, dest.id, type, bundle, rid)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add relation: ' +
					CPLDirect.cpl_error_string(ret))

		r = cpl_relation(rid, dest.id, self.id, type, bundle, D_ANCESTORS)

		return r


	def relation_from(self, src, type, bundle):
		'''
		Add relation type from src to self.
		'''
		ret = CPLDirect.cpl_add_relation(src.id, self.id, type, bundle, rid)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add relation: ' +
					CPLDirect.cpl_error_string(ret))

		r = cpl_relation(rid, self.id, src.id, type, bundle, D_DESCENDANTS)

		return r


	def has_ancestor(self, other):
		'''
		Return True if the other object is an ancestor of the object.
		'''
		ancestors = self.ancestry()
		for a in ancestors:
			if a.ancestor.object == other:
				return True
		return False


	def add_property(self, name, value):
		'''
		Add name/value pair as a property to current object.
		'''
		return CPLDirect.cpl_add_object_property(self.id, name, value)


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
				object.originator, object.name, object.type, bundle)

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
					entry.bundle, direction)
				l.append(a)
		else:
			for entry in v:
				a = cpl_ancestor(entry.id, entry.query_object_id,
					entry.other_object_id, entry.type, 
					entry.bundle, direction)
				l.append(a)

		CPLDirect.delete_std_vector_cpl_relation_tp(vp)
		return l


	def properties(self, key=None, version=None):
		'''
		Return all the properties associated with the current object.

		If key is set to something other than None, return only those
		properties matching key.
		'''
		vp = CPLDirect.new_std_vector_cplxx_property_entry_tp()

		ret = CPLDirect.cpl_get_object_properties(self.id, key,
		    CPLDirect.cpl_cb_collect_properties_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
			raise Exception('Error retrieving properties: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.key, e.value])
		CPLDirect.delete_std_vector_cplxx_property_entry_tp(vp)
		return l

