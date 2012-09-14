#!/usr/bin/env python

#
# setup.py
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
# Contributor(s): Margo Seltzer, Peter Macko
#

'''
Class and functions supporting the Python bindings of the 'Core Provenance
Library <http://http://code.google.com/p/core-provenance-library/>'_.

This module contains:

*	The cpl class.
*	The cpl_object class.
*	Helper functions to print and construct cpl objects
'''

import sys
import CPLDirect


#
# Constants
#

NONE = CPLDirect.CPL_NONE
VERSION_NONE = CPLDirect.CPL_VERSION_NONE
DEPENDENCY_CATEGORY_DATA = CPLDirect.CPL_DEPENDENCY_CATEGORY_DATA
DEPENDENCY_CATEGORY_CONTROL = CPLDirect.CPL_DEPENDENCY_CATEGORY_CONTROL
DEPENDENCY_CATEGORY_VERSION = CPLDirect.CPL_DEPENDENCY_CATEGORY_VERSION
DEPENDENCY_NONE = CPLDirect.CPL_DEPENDENCY_NONE
DATA_INPUT = CPLDirect.CPL_DATA_INPUT
DATA_GENERIC = CPLDirect.CPL_DATA_GENERIC
DATA_IPC = CPLDirect.CPL_DATA_IPC
DATA_TRANSLATION = CPLDirect.CPL_DATA_TRANSLATION
DATA_COPY = CPLDirect.CPL_DATA_COPY
CONTROL_OP = CPLDirect.CPL_CONTROL_OP
CONTROL_GENERIC = CPLDirect.CPL_CONTROL_GENERIC
CONTROL_START = CPLDirect.CPL_CONTROL_START
VERSION_PREV = CPLDirect.CPL_VERSION_PREV
VERSION_GENERIC = CPLDirect.CPL_VERSION_GENERIC
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
O_FILESYSTEM = CPLDirect.CPL_O_FILESYSTEM
O_INTERNET = CPLDirect.CPL_O_INTERNET
T_ARTIFACT = CPLDirect.CPL_T_ARTIFACT
T_FILE = CPLDirect.CPL_T_FILE
T_PROCESS = CPLDirect.CPL_T_PROCESS
T_URL = CPLDirect.CPL_T_URL
L_NO_FAIL = CPLDirect.CPL_L_NO_FAIL
I_NO_CREATION_SESSION = CPLDirect.CPL_I_NO_CREATION_SESSION
I_NO_VERSION = CPLDirect.CPL_I_NO_VERSION
I_FAST = CPLDirect.CPL_I_FAST
D_ANCESTORS = CPLDirect.CPL_D_ANCESTORS
D_DESCENDANTS = CPLDirect.CPL_D_DESCENDANTS
A_NO_PREV_NEXT_VERSION = CPLDirect.CPL_A_NO_PREV_NEXT_VERSION
A_NO_DATA_DEPENDENCIES = CPLDirect.CPL_A_NO_DATA_DEPENDENCIES
A_NO_CONTROL_DEPENDENCIES = CPLDirect.CPL_A_NO_CONTROL_DEPENDENCIES



#
# Private constants
#

__data_dict = ['data input', 'data ipc', 'data translation', 'data copy']
__control_dict = ['control op', 'control start']



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
# CPLDirect enhancements
#

def __cpl_id_t__eq__(self, other):
	'''
	Compare this and another ID, and return true if they are equal
	'''
	return self.lo == other.lo and self.hi == other.hi


def __cpl_id_t__ne__(self, other):
	'''
	Compare this and another ID, and return true if they are not equal
	'''
	return self.lo != other.lo  or self.hi != other.hi


def __cpl_id_t__str__(self):
	'''
	Create and return a string representation of this object
	'''
	return "%x:%x" % (self.hi, self.lo)


CPLDirect.cpl_id_t.__eq__ = __cpl_id_t__eq__
CPLDirect.cpl_id_t.__ne__ = __cpl_id_t__ne__
CPLDirect.cpl_id_t.__str__ = __cpl_id_t__str__



#
# Public utility functions
#

def dependency_type_to_str(val):
	'''
	Given a dependency (edge) type, convert it to a string

	Method calls::
		strval = dependency_type_to_str(val)
	'''
	which = val >> 8
	if which == DEPENDENCY_CATEGORY_DATA:
		return __data_dict[val & 7]
	elif which == DEPENDENCY_CATEGORY_CONTROL:
		return __control_dict[val & 7]
	elif which == DEPENDENCY_CATEGORY_VERSION:
		return 'version'
	else:
		return 'unknown'


def copy_id(idp):
	'''
	Construct a cpl identifier type consisting of the hi and lo values.

	Method calls::
		id = copy_id(idp)
	'''
	i = CPLDirect.cpl_id_t()
	i.hi = idp.hi
	i.lo = idp.lo
	return i


def p_id(id, with_newline = False):
	'''
	Print hi and lo fields of a CPL id, optionally with newline after it.

	Method calls::
		p_id(id, with_newline = False)
	'''
	sys.stdout.write('id: ' + str(id))
	if with_newline:
		sys.stdout.write('\n')


def p_obj(obj, with_session = False):
	'''
	Print information about an object

	Method calls:
		p_obj(obj, with_session = False)
	'''
	i = obj.info()
	p_id(obj.id)
	print(' version: ' + str(i.version))
	sys.stdout.write('container_id: ')
	if i.container is not None:
		p_id(i.container.object.id)
		print(' container version: ' + str(i.container.version))
	else:
		sys.stdout.write('none')
		print(' container version: none')
	print('originator: ' + i.originator + ' name:' + i.name +
	    ' type: ' + i.type)
	if with_session:
		print('creation_time: ' + i.creation_time)
		p_session(i.creation_session())


def p_session(si):
	'''
	Print information about a session

	Method calls:
		p_session(si)
	'''
	p_id(si.id, with_newline = True)
	print(' mac_address: ' + si.mac_address + ' pid: ' + str(si.pid))
	print('\t(' + str(si.start_time) + ')' + ' user: ' +
	    si.user + ' cmdline: ' + si.cmdline + ' program: ' + si.program)



#
# Object & version
#

class cpl_object_version:
	'''
	Stores a reference to a provenance object and a version number
	'''


	def __init__(self, object, version):
		'''
		Create an instance of this object
		''',
		self.object = object
		self.version = version


	def __eq__(self, other):
		'''
		Compare this and the other object, and return true if they are equal
		'''
		return self.id == other.id and self.version == other.version


	def __ne__(self, other):
		'''
		Compare this and the other object, and return true if they are not equal
		'''
		return self.id != other.id or self.version != other.version


	def __str__(self):
		'''
		Create and return a human-readable string representation of this object
		'''
		return str(self.object) + '-' + str(self.version)



#
# Provenance ancestry entry
#

class cpl_ancestor:
	'''
	Stores the same data as a cpl_ancestry_entry_t, but in a Python
	class that we manage.
	'''


	def __init__(self, aid, aversion, did, dversion, type):
		self.ancestor = cpl_object_version(cpl_object(aid), aversion)
		self.descendant = cpl_object_version(cpl_object(did), dversion)
		self.type = type


	def dump(self):
		'''
		Display the cpl_ancestor object
		'''
		sys.stdout.write(dependency_type_to_str(self.type) + ': ')
		sys.stdout.write(str(self.ancestor))
		sys.stdout.write(' -- ')
		sys.stdout.write(str(self.descendant))
		sys.stdout.write('\n')



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
		self.connection_string = cstring

		def get_current_session():
			idp = CPLDirect.new_cpl_id_tp()
			ret = CPLDirect.cpl_get_current_session(idp)

			if not CPLDirect.cpl_is_ok(ret):
				CPLDirect.delete_cpl_id_tp(idp)
				print ("Could not get current session" +
				       CPLDirect.cpl_error_string(ret))

			s = CPLDirect.cpl_id_tp_value(idp)
			i = copy_id(s)
			CPLDirect.delete_cpl_id_tp(idp)
			return i

		backend = CPLDirect.new_cpl_db_backend_tpp()
		ret = CPLDirect.cpl_create_odbc_backend(cstring,
		    CPLDirect.CPL_ODBC_GENERIC, backend)
		if not CPLDirect.cpl_is_ok(ret):
			print ("Could not create ODBC connection" +
			       CPLDirect.cpl_error_string(ret))
		self.db = CPLDirect.cpl_dereference_pp_cpl_db_backend_t(backend)
		ret = CPLDirect.cpl_attach(self.db)
		CPLDirect.delete_cpl_db_backend_tpp(backend)
		if not CPLDirect.cpl_is_ok(ret):
			print ("Could not open ODBC connection" +
			       CPLDirect.cpl_error_string(ret))
		self.session = get_current_session()


	def __create_or_lookup_cpl_object(self, originator,
		     name, type, create=None, container=None):
		'''
		Create or lookup a CPL object

		** Parameters **
			originator 
			name: originator-local name
			type: originator-local type
			create:
				None: lookup or create
				True: create only
				False: lookup only
			container:
				Id of container into which to place this object.
				Only applies to create
		'''
		if container is None:
			container = NONE
		idp = CPLDirect.new_cpl_id_tp()
		if create == None:
			ret = CPLDirect.cpl_lookup_or_create_object(originator, name,
							  type, container, idp)
			if ret == S_OBJECT_CREATED:
				ret = S_OK
		elif create:
			ret = CPLDirect.cpl_create_object(originator,
						name, type, container, idp)
		else:
			ret = CPLDirect.cpl_lookup_object(originator, name, type, idp)

		if ret == E_NOT_FOUND:
			raise LookupError('Not found')
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not find or create' +
			    ' provenance object: ' + CPLDirect.cpl_error_string(ret))
			
		r = cpl_object(idp)

		CPLDirect.delete_cpl_id_tp(idp)
		return r
			

	def get_obj(self, originator, name, type, container=None):
		'''
		Get the object, with the designated originator (string),
		name (string), and type (string), creating it if necessary.

		If you want an object in a specific container, set the container
		parameter to the ID of the object in which you want this object
		created.
		'''
		return self.__create_or_lookup_cpl_object(originator, name, type,
				create=None, container=container)


	def create_obj(self, originator, name, type, container=None):
		'''
		Create object, returns None if object already exists.
		'''
		return self.__create_or_lookup_cpl_object(originator, name, type,
				create=True, container=container)


	def lookup(self, originator, name, type):
		'''
		Look up object; returns None if object already exiss.
		'''
		try:
			o = self.__create_or_lookup_cpl_object(originator, name, type,
					create=False)
		except LookupError:
			# Return None or propagate exception?
			o = None
		return o


	def lookup_by_property(self, key, value):
		'''
		Return all objects that have the key/value property specified.
		'''
		vp = CPLDirect.new_std_vector_cpl_id_version_tp()
		ret = CPLDirect.cpl_lookup_by_property(key, value,
			CPLDirect.cpl_cb_collect_property_lookup_vector, vp)

		if ret == E_NOT_FOUND:
			CPLDirect.delete_std_vector_cpl_id_version_tp(vp)
			raise LookupError('Not found')
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_id_version_tp(vp)
			raise Exception('Unable to lookup by property ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_version_t(vp)
		l = []
		for e in v:
			l.append(cpl_object_version(cpl_object(e.id), e.version))

		CPLDirect.delete_std_vector_cpl_id_version_tp(vp)
		return l


	def lookup_all(self, originator, name, type):
		'''
		Return all objects that have the specified originator, name,
		and type (they might differ by container).
		'''
		vp = CPLDirect.new_std_vector_cpl_id_version_tp()
		ret = CPLDirect.cpl_lookup_object_ext(originator, name, type,
			L_NO_FAIL, CPLDirect.cpl_cb_collect_id_timestamp_vector, vp)

		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_id_version_tp(vp)
			raise Exception('Unable to lookup all objects: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_version_t(vp)
		l = []
		if v != S_NO_DATA :
			for e in v:
				l.append(cpl_object(e))

		CPLDirect.delete_std_vector_cpl_id_version_tp(vp)
		return l


	def session_info(self):
		'''
		Return cpl_session_info_t object associated with this session.
		'''

		sessionpp = CPLDirect.new_cpl_session_info_tpp()
		ret = CPLDirect.cpl_get_session_info(self.session,
		    CPLDirect.cpl_convert_pp_cpl_session_info_t(sessionpp))
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_session_info_tpp(sessionpp)
			raise Exception('Could not find session information: ' +
					CPLDirect.cpl_error_string(ret))

		sessionp = CPLDirect.cpl_dereference_pp_cpl_session_info_t(sessionpp)
		info = CPLDirect.cpl_session_info_tp_value(sessionp)
		CPLDirect.delete_cpl_session_info_tpp(sessionpp)
		return info


	def close(self):
		'''
		Close database connection and session
		'''
		ret = CPLDirect.cpl_detach()
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not detach ' +
					CPLDirect.cpl_error_string(ret))



#
# Information about a provenance object
#

class cpl_object_info:
	'''
	Information about a provenance object
	'''

	def __init__(self, object, version, creation_session, creation_time,
			originator, name, type, container):
		'''
		Create an instance of this object
		'''

		self.object = object
		self.version = version
		self.creation_session = creation_session
		self.creation_time = creation_time
		self.originator = originator
		self.name = name
		self.type = type
		self.container = container



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


	def version(self):
		'''
		Determine the current version of this provenance object
		'''
		vp = CPLDirect.new_cpl_version_tp()

		ret = CPLDirect.cpl_get_version(self.id, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_version_tp(vp)
			raise Exception('Could not determine the version of an object: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_version_tp_value(vp)
		CPLDirect.delete_cpl_version_tp(vp)
		return v


	def new_version(self):
		'''
		Create a new version of this object and return the new version.
		'''
		vp = CPLDirect.new_cpl_version_tp()

		ret = CPLDirect.cpl_new_version(self.id, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_cpl_version_tp(vp)
			raise Exception('Could not createa a new version of an object: ' +
					CPLDirect.cpl_error_string(ret))

		v = CPLDirect.cpl_version_tp_value(vp)
		CPLDirect.delete_cpl_version_tp(vp)
		return v


	def control_flow_to(self, dest, type=CONTROL_OP, version=None):
		'''
		Add control flow edge of type from self to dest. If version
		is specified, then add flow to dest with explicit version,
		else add to most recent version.

		Allowed types:
			CPL.CONTROL_OP (default)
			CPL.CONTROL_START
		'''

		if version is None or version == VERSION_NONE:
			version = self.version()

		ret = CPLDirect.cpl_control_flow_ext(dest.id, self.id, version, type)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add control dependency: ' +
					CPLDirect.cpl_error_string(ret))
		return not ret == S_DUPLICATE_IGNORED


	def data_flow_to(self, dest, type=DATA_INPUT, version=None):
		'''
		Add data flow edge of type from self to dest. If version
		is specified, then add flow to dest with explicit version,
		else add to most recent version.

		Allowed types:
			CPL.DATA_INPUT (default)
			CPL.DATA_IPC
			CPL.DATA_TRANSLATION
			CPL.DATA_COPY
		'''

		if version is None or version == VERSION_NONE:
			version = self.version()

		ret = CPLDirect.cpl_data_flow_ext(dest.id, self.id, version, type)
		if not CPLDirect.cpl_is_ok(ret):
			raise Exception('Could not add data dependency ' +
					CPLDirect.cpl_error_string(ret))
		return not ret == S_DUPLICATE_IGNORED


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
		return CPLDirect.cpl_add_property(self.id, name, value)


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
		CPLDirect.delete_cpl_object_info_tpp(objectpp)

		if object.container_id == NONE or object.container_version < 0:
			container = None
		else:
			container = cpl_object_version(cpl_object(object.container_id),
					object.container_version)

		return cpl_object_info(object, object.version, object.creation_session,
				object.creation_time, object.originator, object.name,
				object.type, container)


	def ancestry(self, version=None, direction=D_ANCESTORS, flags=0):
		'''
		Return a list of cpl_ancestor objects
		'''
		if version is None:
			version = VERSION_NONE
		vp = CPLDirect.new_std_vector_cpl_ancestry_entry_tp()

		ret = CPLDirect.cpl_get_object_ancestry(self.id, version,
		    direction, flags, CPLDirect.cpl_cb_collect_ancestry_vector, vp)
		if not CPLDirect.cpl_is_ok(ret):
			CPLDirect.delete_std_vector_cpl_ancestry_entry_tp(vp)
			raise Exception('Error retrieving ancestry: ' +
					CPLDirect.cpl_error_string(ret))
			return None

		v = CPLDirect.cpl_dereference_p_std_vector_cpl_ancestry_entry_t(vp)
		l = []
		if direction == D_ANCESTORS:
			for entry in v:
				a = cpl_ancestor(entry.other_object_id,
					entry.other_object_version,
					entry.query_object_id,
					entry.query_object_version, entry.type)
				l.append(a)
		else:
			for entry in v:
				a = cpl_ancestor(entry.query_object_id,
					entry.query_object_version,
					entry.other_object_id,
					entry.other_object_version, entry.type)
				l.append(a)

		CPLDirect.delete_std_vector_cpl_ancestry_entry_tp(vp)
		return l


	def properties(self, key=None, version=None):
		'''
		Return all the properties associated with the current object.

		If key is set to something other than None, return only those
		properties matching key.

		By default, returns properties for the current version of
		the object, but if version is set to a value other than
		CPL.VERSION_NONE, then will return properties for that version.
		'''
		if version is None:
			version = VERSION_NONE
		vp = CPLDirect.new_std_vector_cplxx_property_entry_tp()

		ret = CPLDirect.cpl_get_properties(self.id, version,
		    key, CPLDirect.cpl_cb_collect_properties_vector, vp)
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

