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
from CPLDirect import *

# Utility functions
flags = dict()

data_dict = ['data input', 'data ipc', 'data translation', 'data copy']
control_dict = ['control op', 'control start']
def getSignedNumber(number, bitLength):
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

def cpl_type_to_str(val):
	'''
	Given an edge type, convert it to a string

	Method calls::
		strval = cpl_type_to_str(val)
	'''
	which = val >> 8
	if which == CPL_DEPENDENCY_CATEGORY_DATA:
		return data_dict[val & 7]
	elif which == CPL_DEPENDENCY_CATEGORY_CONTROL:
		return control_dict[val & 7]
	elif which == CPL_DEPENDENCY_CATEGORY_VERSION:
		return 'version'
	else:
		return ""

def copy_id(idp):
	'''
	Construct a cpl identifier type consisting of the hi and lo values.

	Method calls::
		id = copy_id(idp)
	'''
	i = cpl_id_t()
	i.hi = idp.hi
	i.lo = idp.lo
	return i

def copy_idv(idvp):
	'''
	Copy a cpl identifier + version type into a local cpl_id_version_t

	Method calls::
		idv = copy_idv(idvp)
	'''
	i = cpl_id_version_t()
	i.id = copy_id(idvp.id)
	i.version = idvp.version
	return i

def p_id(id, with_newline = False):
	'''
	Print hi and lo fields of a CPL id, optionally with newline after it.

	Method calls::
		p_id(id, with_newline = False)
	'''
	sys.stdout.write('id: ' + str(getSignedNumber(id.hi, 64)) +
	    ':' + str(getSignedNumber(id.lo, 64)))
	if with_newline:
		sys.stdout.write('\n')

def p_obj(obj, with_session = False):
	i = obj.info()
	p_id(obj.id)
	print(' version: ' + str(i.version))
	sys.stdout.write('container_id: ')
	p_id(i.container_id)
	print(' container version: ' + str(i.container_version))
	print('originator: ' + i.originator + ' name:' + i.name +
	    ' type: ' + i.type)
	if with_session:
		print('creation_time: ' + i.creation_time)
		p_session(i.creation_session())

def p_session(si):
	p_id(si.id, with_newline = True)
	print(' mac_address: ' + si.mac_address + ' pid: ' + str(si.pid))
	print('\t(' + str(si.start_time) + ')' + ' user: ' +
	    si.user + ' cmdline: ' + si.cmdline + ' program: ' + si.program)

class cpl_ancestor:
	'''
	Stores the same data as a cpl_ancestry_entry_t, but in a python
	class that we manage.
	'''
	def __init__(self, fid, fversion, tid, tversion, type):
		self.from_id = copy_id(fid)
		self.from_version = fversion
		self.to_id = copy_id(tid)
		self.to_version = tversion
		self.type = type

	def dump(self):
		'''
		Display cpl_ancestor object
		'''
		sys.stdout.write(cpl_type_to_str(self.type) + ':')
		p_id(self.from_id)
		sys.stdout.write('(' + str(self.from_version) + ') to ')
		p_id(self.to_id)
		sys.stdout.write('(' + str(self.to_version) + ')\n')

class cpl_connection:
	'''
	Core provenance library connection -- maintains state for the current
	session and the current database backend.
	'''
	def __init__(self, cstring = "DSN=CPL;"):
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
			idp = new_cpl_id_tp()
			ret = cpl_get_current_session(idp)

			if not cpl_is_ok(ret):
				delete_cpl_id_tp(idp)
				print ("Could not get current session" +
				       cpl_error_string(ret))

			s = cpl_id_tp_value(idp)
			i = copy_id(s)
			delete_cpl_id_tp(idp)
			return i

		backend = new_cpl_db_backend_tpp()
		ret = cpl_create_odbc_backend(cstring,
		    CPL_ODBC_GENERIC, backend)
		if not cpl_is_ok(ret):
			print ("Could not create ODBC connection" +
			       cpl_error_string(ret))
		self.db = cpl_dereference_pp_cpl_db_backend_t(backend)
		ret = cpl_attach(self.db)
		delete_cpl_db_backend_tpp(backend)
		if not cpl_is_ok(ret):
			print ("Could not open ODBC connection" +
			       cpl_error_string(ret))
		self.session = get_current_session()


	def obj_from_id(self, id):
		'''
		Given a CPL id, return a cpl_object
		'''
		ipp = new_cpl_object_info_tpp()
		ret = cpl_get_object_info(id,
					  cpl_convert_pp_cpl_object_info_t(ipp))
		if not cpl_is_ok(ret):
			print ("Unable to get object info " +
			       cpl_error_string(ret))
			delete_cpl_object_info_tpp(ipp)
			# Exception
			return None

		ip = cpl_dereference_pp_cpl_object_info_t(ipp)
		o_info = cpl_object_info_tp_value(ip)
		o = self.lookup(o_info.originator, o_info.name, o_info.type)
		delete_cpl_object_info_tpp(ipp)
		return o
			

	def get_obj(self, originator, name, type, container=CPL_NONE):
		'''
		Get the object, with the designated originator (string),
		name (string), and type (string), creating it if necessary.

		If you want an object in a specific container, set the container
		parameter to the ID of the object in which you want this object
		created.
		'''
		try:
			o = cpl_object(originator, name,
				  type, create = None, container=container)
		except LookupError:
			# Return None or propagate exception?
			o = None

		return o

	def create_obj(self, originator, name, type, container=CPL_NONE):
		'''
		Create object, returns None if object already exists.
		'''
		try:
			o = cpl_object(originator, name,
				  type, create = True, container=container)
		except LookupError:
			# Return None or propagate exception?
			o = None
		return o

	def lookup(self, originator, name, type):
		'''
		Look up object; returns None if object already exiss.
		'''
		try:
			o = cpl_object(originator, name, type, create = False)
		except LookupError:
			# Return None or propagate exception?
			o = None
		return o

	def lookup_by_property(self, key, value):
		'''
		Return all objects that have the key/value property specified.
		'''
		vp = new_std_vector_cpl_id_version_tp()
		ret = cpl_lookup_by_property(key, value,
			cpl_cb_collect_property_lookup_vector, vp)

		if not cpl_is_ok(ret):
			print ("Unable to lookup by property " +
				cpl_error_string(ret))
			delete_std_vector_cpl_id_version_tp(vp)
			return None
		v = cpl_dereference_p_std_vector_cpl_id_version_t(vp)
		l = []
		for e in v:
			p_id(e.id)
			print ': ', e.version
			l.append(copy_idv(e))

		delete_std_vector_cpl_id_version_tp(vp)
		return l

	def lookup_all(self, originator, name, type):
		'''
		Return all objects that have the specified originator, name,
		and type (they might differ by container).
		'''
		vp = new_std_vector_cpl_id_version_tp()
		ret = cpl_lookup_object_ext(originator, name, type,
			CPL_L_NO_FAIL,
			d_timestamp_t_vector_iterator, vp)

		if not cpl_is_ok(ret):
			print ("Unable to lookup all objects " +
				cpl_error_string(ret))
			delete_std_vector_cpl_id_version_tp(vp)
			return None
		v = cpl_dereference_p_std_vector_cpl_id_version_t(vp)
		l = []
		if v != CPL_S_NO_DATA :
			for e in v:
				l.append(copy_idv(e))

		delete_std_vector_cpl_id_version_tp(vp)
		return l

	def session_info(self):
		'''
		Return cpl_session_info_t object associated with this session.
		'''

		sessionpp = new_cpl_session_info_tpp()
		ret = cpl_get_session_info(self.session,
		    cpl_convert_pp_cpl_session_info_t(sessionpp))
		if not cpl_is_ok(ret):
			# Should throw an exception
			delete_cpl_session_info_tpp(sessionpp)
			print ("Could not find session information " +
				cpl_error_string(ret))
			return None

		sessionp = cpl_dereference_pp_cpl_session_info_t(sessionpp)
		info = cpl_session_info_tp_value(sessionp)
		delete_cpl_session_info_tpp(sessionpp)
		return info

	def close(self):
		'''
		Close database connection and session
		'''
		ret = cpl_detach()
		if not cpl_is_ok(ret):
			print "Could not detach ", cpl_error_string(ret)

class cpl_object:
	'''
	CPL Provenance object
	'''
	def __init__(self, originator,
		     name, type, create = None, container = CPL_NONE):
		'''
		Class constructor

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
		idp = new_cpl_id_tp()
		if create == None:
			ret = cpl_lookup_or_create_object(originator, name,
							  type, container, idp)
			if ret == CPL_S_OBJECT_CREATED:
				ret = CPL_S_OK
		elif create:
			ret = cpl_create_object(originator,
						name, type, container, idp)
		else:
			ret = cpl_lookup_object(originator, name, type, idp)

		if not cpl_is_ok(ret):
			raise LookupError('Could not find/create' +
			    ' provenance object: ' + cpl_error_string(ret))
			
		self.id = copy_id(idp)

		delete_cpl_id_tp(idp)

	def control_flow_to(self, dest,
			    type = CPL_CONTROL_OP, version = CPL_VERSION_NONE):
		'''
		Add control flow edge of type from self to dest. If version
		is specified, then add flow to dest with explicit version,
		else add to most recent version.

		Allowed types:
			CPL_CONTROL_OP (default)
			CPL_CONTROL_START
		'''

		if version == CPL_VERSION_NONE:
			version = self.info().version

		ret = cpl_control_flow_ext(dest.id, self.id, version, type)
		if not cpl_is_ok(ret):
			print ("Could not add control dependency " +
				cpl_error_string(ret))
			return 0
		return not ret == CPL_S_DUPLICATE_IGNORED

	def data_flow_to(self, dest,
			 type = CPL_DATA_INPUT, version = CPL_VERSION_NONE):
		'''
		Add data flow edge of type from self to dest. If version
		is specified, then add flow to dest with explicit version,
		else add to most recent version.

		Allowed types:
			CPL_DATA_INPUT
			CPL_DATA_IPC
			CPL_DATA_TRANSLATION
			CPL_DATA_COPY
		'''

		if version == CPL_VERSION_NONE:
			version = self.info().version

		ret = cpl_data_flow_ext(dest.id, self.id, version, type)
		if not cpl_is_ok(ret):
			print ("Could not add data dependency " +
				cpl_error_string(ret))
			return 0
		return not ret == CPL_S_DUPLICATE_IGNORED

	def has_ancestor(self, id):
		'''
		Return True if the object referenced by id is an ancestor
		of the object.
		'''
		ancestors = self.ancestry()
		for a in ancestors:
			if cpl_id_cmp(a.to_id, id) == 0:
				return True
		return False

	def add_property(self, name, value):
		'''
		Add name/value pair as a property to current object.
		'''
		return cpl_add_property(self.id, name, value)

	def info(self, version=CPL_VERSION_NONE):
		'''
		Return cpl_object_info_t corresopnding to the current object.
		By default returns info for the most recent version, but if
		version is set, then returns info for the designated version.
		'''
		objectpp = new_cpl_object_info_tpp()
		ret = cpl_get_object_info(self.id,
		    cpl_convert_pp_cpl_object_info_t(objectpp))
		if not cpl_is_ok(ret):
			# Exception?
			print ("Unable to get object info " +
				cpl_error_string(ret))
			object = None
		else:
			op = cpl_dereference_pp_cpl_object_info_t(objectpp)
			object = cpl_object_info_tp_value(op)

		delete_cpl_object_info_tpp(objectpp)
		return object

	def ancestry(self, version = CPL_VERSION_NONE,
		     direction = CPL_D_ANCESTORS, flags = 0):
		'''
		Return a list of cpl_ancestor objects
		'''
		vp = new_std_vector_cpl_ancestry_entry_tp()
		ret = cpl_get_object_ancestry(self.id, version,
		    direction, flags, cpl_cb_collect_ancestry_vector, vp)
		if not cpl_is_ok(ret):
			print ("Error retrieving ancestry " +
				cpl_error_string(ret))
			return None
		v = cpl_dereference_p_std_vector_cpl_ancestry_entry_t(vp)
		l = []
		for entry in v:
			a = cpl_ancestor(entry.query_object_id,
			    entry.query_object_version,
			    entry.other_object_id,
			    entry.other_object_version, entry.type)
			l.append(a)
		delete_std_vector_cpl_ancestry_entry_tp(vp)
		return l

	def properties(self, key = None, version = CPL_VERSION_NONE):
		'''
		Return all the properties associated with the current object.

		If key is set to something other than None, return only those
		properties matching key.

		By default, returns properties for the current version of
		the object, but if version is set to a value other than
		CPL_VERSION_NONE, then will return properties for that version.
		'''
		vp = new_std_vector_cplxx_property_entry_tp()
		ret = cpl_get_properties(self.id, version,
		    key, cpl_cb_collect_properties_vector, vp)
		if not cpl_is_ok(ret):
			print ("Error retrieving properties " +
				cpl_error_string(ret))
			return None
		v = cpl_dereference_p_std_vector_cplxx_property_entry_t(vp)
		l = []
		for e in v:
			l.append([e.key, e.value])
		delete_std_vector_cplxx_property_entry_tp(vp)
		return l

