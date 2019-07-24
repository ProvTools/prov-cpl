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

*   The cpl class.
*   The cpl_relation class.
*   The cpl_session class.
*   The cpl_session_info class.
*   The cpl_object class.
*   The cpl_object_info class.
*   Helper functions to print and construct cpl objects
'''

import sys
import CPLDirect


#
# Constants
#

NONE = CPLDirect.CPL_NONE

CPL_VERSION_STR = CPLDirect.CPL_VERSION_STR

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
NUM_R_TYPES = CPLDirect.CPL_NUM_R_TYPES
BUNDLERELATION = CPLDirect.BUNDLERELATION

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
J_EXTERN_OBJ = CPLDirect.CPL_J_EXTERN_OBJ
D_ANCESTORS = CPLDirect.CPL_D_ANCESTORS
D_DESCENDANTS = CPLDirect.CPL_D_DESCENDANTS



#
# Private constants
#

__relation_list = ["wasInfluencedBy",
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

__relation_dict = {"wasInfluencedBy"    :1,
               "alternateOf"            :2,
               "derivedByInsertionFrom" :3,
               "derivedByRemovalFrom"   :4,
               "hadMember"              :5,
               "hadDictionaryMember"    :6,
               "specializationOf"       :7,
               "wasDerivedFrom"         :8,
               "wasGeneratedBy"         :9,
               "wasInvalidatedBy"       :10,
               "wasAttributedTo"        :11,
               "used"                   :12,
               "wasInformedBy"          :13,
               "wasStartedBy"           :14,
               "wasEndedBy"             :15,
               "hadPlan"                :16,
               "wasAssociatedWith"      :17,
               "actedOnBehalfOf"        :18}

__object_list = ["entity",
                 "activity",
                 "agent",
                 "bundle"]

__object_dict = {"entity"   :1,
                 "activity" :2,
                 "agent"    :3}
                 

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
        return __relation_list[val-1]
    else:
        return 'unknown'

def object_type_to_str(val):
    '''
    Given an object type, convert it to a string

    Method calls::
        strval = relation_type_to_str(val)
    '''
    if val > 0 and val < NUM_O_TYPES :
        return __object_list[val-1]
    else:
        return 'unknown'

def relation_str_to_type(str):

    ret = __relation_dict.get(str)
    if ret is None:
        return 0
    else:
        return ret

def object_str_to_type(str):

    ret = __object_dict.get(str)
    if ret is None:
        return 0
    else:
        return ret
        
def p_id(id, with_newline = False):
    '''
    Print a CPL id, optionally with newline after it.

    Method calls::
        p_id(id, with_newline = False)
    '''
    sys.stdout.write('id:' + str(id) + ' ')
    if with_newline:
        sys.stdout.write('\n')


def p_object(obj):
    '''
    Print information about an object

    Method calls:
            p_object(obj)
    '''

    i = obj.info()
    p_id(i.object.id)
    sys.stdout.write('prefix:' + i.prefix + ' name:' + i.name +
        ' type:' + __object_list[i.type-1])
    print 'creation_time:' + str(i.creation_time)
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


class CPLException(Exception):

    def __init__(self, msg, code):
        super(CPLException, self).__init__(msg)
        self.code = code
        self.msg = msg
#      
# Provenance relation
#

class cpl_relation:
    '''
    Stores the same data as a cpl_relation_t, but in a Python
    class that we manage.
    '''


    def __init__(self, id, aid, did, type, direction):
        '''
        Create an instance of cpl_relation_t
        '''
        self.id = id
        self.ancestor = cpl_object(aid)
        self.descendant = cpl_object(did)
        self.type = type

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
            '; id: ' + str(self.id) + ']')

    def string_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current relation.

        If key is set to something other than None, return only those
        properties matching key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_string_property_entry_tp()

        ret = CPLDirect.cpl_get_relation_string_properties(self.id, prefix, key,
                                                           CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_string_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_string_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_string_property_entry_tp(vp)
        return l

    def numerical_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current relation.

        If key is set to something other than None, return only those
        properties matching key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_numerical_property_entry_tp()

        ret = CPLDirect.cpl_get_relation_numerical_properties(self.id, prefix, key,
                                                           CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_numerical_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_numerical_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_numerical_property_entry_tp(vp)
        return l

    def boolean_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current relation.

        If key is set to something other than None, return only those
        properties matching key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_boolean_property_entry_tp()

        ret = CPLDirect.cpl_get_relation_boolean_properties(self.id, prefix, key,
                                                           CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_boolean_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_boolean_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_boolean_property_entry_tp(vp)
        return l

    def add_string_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_relation_string_property(self.id, prefix, name, value)

    def add_numerical_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_relation_numerical_property(self.id, prefix, name, value)

    def add_boolean_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_relation_boolean_property(self.id, prefix, name, value)

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
                raise CPLException("Could not get current session " +
                       CPLDirect.cpl_error_string(ret), ret)

            return id

        backend = CPLDirect.new_cpl_db_backend_tpp()
        ret = CPLDirect.cpl_create_odbc_backend(cstring,
            CPLDirect.CPL_ODBC_GENERIC, backend)
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException("Could not create ODBC connection " +
                   CPLDirect.cpl_error_string(ret), ret)
        self.db = CPLDirect.cpl_dereference_pp_cpl_db_backend_t(backend)
        ret = CPLDirect.cpl_attach(self.db)
        CPLDirect.delete_cpl_db_backend_tpp(backend)
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException("Could not open ODBC connection " +
                   CPLDirect.cpl_error_string(ret), ret)
        self.session = cpl_session(get_current_session())

        _cpl_connection = self


    def __del__(self):
        '''
        Destructor - automatically closes the connection.
        '''
        if self == _cpl_connection and not self.closed:
            self.close()


    def __create_or_lookup_cpl_object(self, prefix,
             name, type=None, create=None):
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
        '''

        if create == None:
            ret, idp = CPLDirect.cpl_lookup_or_create_object(prefix, name,
                              type)
            if ret == S_OBJECT_CREATED:
                ret = S_OK
        elif create:
            ret, idp = CPLDirect.cpl_create_object(prefix,
                        name, type)
        else:
            ret, idp = CPLDirect.cpl_lookup_object(prefix, name, type)
            if ret == E_NOT_FOUND:
                raise LookupError('Not found')
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not find or create' +
                ' provenance object: ' + CPLDirect.cpl_error_string(ret), ret)
        
        r = cpl_object(idp)
        return r

    def lookup_object_property_wildcard(self, value):
        ret, idp = CPLDirect.cpl_lookup_object_property_wildcard(value)

        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not find' +
                               ' object property: ' + CPLDirect.cpl_error_string(ret), ret)
        r = cpl_object(idp)
        return r

    def create_bundle(self, name, prefix):
        ret, idp = CPLDirect.cpl_create_bundle(name, prefix)

        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not create' +
                ' provenance bundle: ' + CPLDirect.cpl_error_string(ret), ret)
        
        r = cpl_object(idp)
        return r


    def lookup_bundle(self, name, prefix):
        ret, idp = CPLDirect.cpl_lookup_bundle(name, prefix)

        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not find' +
                ' provenance bundle: ' + CPLDirect.cpl_error_string(ret), ret)
        
        r = cpl_object(idp)
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
            raise CPLException('Unable to lookup all bundles: ' +
                    CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_timestamp_t(vp)
        l = []
        if v != S_NO_DATA :
            for e in v:
                l.append(cpl_object(e.id))

        CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
        return l


    def lookup_relation(self, from_id, to_id, type):
        ret, idp = CPLDirect.cpl_lookup_relation(from_id, to_id, type)

        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not find' +
                               ' relation: ' + CPLDirect.cpl_error_string(ret), ret)
        r = cpl_relation(idp, to_id, from_id, type, D_ANCESTORS)
        return r

    def create_relation(self, src, dest, type):
        '''
        Add relation type from src to dest.
        '''

        ret, idp = CPLDirect.cpl_add_relation(src, dest, type)
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not add relation: ' +
                               CPLDirect.cpl_error_string(ret), ret)
        r = cpl_relation(idp, src, dest, type, D_ANCESTORS)
        return r

    def get_all_objects(self, prefix, fast=False):
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
        ret = CPLDirect.cpl_get_all_objects(prefix, flags,
            CPLDirect.cpl_cb_collect_object_info_vector, vp)

        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
            raise CPLException('Unable to get all objects: ' +
                    CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_object_info_t(vp)
        l = []
        if v != S_NO_DATA :
            for e in v:
                l.append(cpl_object_info(cpl_object(e.id),
                    e.creation_time, e.prefix, e.name,
                    e.type))

        CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
        return l
            

    def get_object(self, prefix, name, type):
        '''
        Get the object, with the designated prefix (string),
        name (int), type (int), and bundle (ID) creating it if necessary.

        '''
        return self.__create_or_lookup_cpl_object(prefix, name, type,
                create=None)


    def create_object(self, prefix, name, type):
        '''
        Create object, returns None if object already exists.
        '''
        return self.__create_or_lookup_cpl_object(prefix, name, type,
                create=True)


    def lookup_object(self, prefix, name, type):
        '''
        Look up object; raise LookupError if the object does not exist.
        '''
        o = self.__create_or_lookup_cpl_object(prefix, name, type,
                create=False)
        return o


    def try_lookup_object(self, prefix, name, type):
        '''
        Look up object; returns None if the object does not exist.
        '''
        try:
            o = self.__create_or_lookup_cpl_object(prefix, name, type,
                    create=False)
        except LookupError:
            o = None
        return o

    def __lookup_by_property_helper(self, vp, ret):
        if ret == E_NOT_FOUND:
            CPLDirect.delete_std_vector_cpl_id_tp(vp)
            raise LookupError('Not found')
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cpl_id_tp(vp)
            raise CPLException('Unable to lookup by property ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cpl_id_t(vp)
        l = []
        for e in v:
            l.append(cpl_object(e))

        CPLDirect.delete_std_vector_cpl_id_tp(vp)
        return l

    def lookup_by_string_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified; raise
        LookupError if no such object is found.
        '''
        vp = CPLDirect.new_std_vector_cpl_id_tp()
        ret = CPLDirect.cpl_lookup_object_by_string_property(prefix, key, value,
            CPLDirect.cpl_cb_collect_property_lookup_vector, vp)
        return self.__lookup_by_property_helper(vp, ret)

    def lookup_by_numerical_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified; raise
        LookupError if no such object is found.
        '''
        vp = CPLDirect.new_std_vector_cpl_id_tp()
        ret = CPLDirect.cpl_lookup_object_by_numerical_property(prefix, key, value,
                                                      CPLDirect.cpl_cb_collect_property_lookup_vector, vp)
        return self.__lookup_by_property_helper(vp, ret)


    def lookup_by_boolean_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified; raise
        LookupError if no such object is found.
        '''
        vp = CPLDirect.new_std_vector_cpl_id_tp()
        ret = CPLDirect.cpl_lookup_object_by_boolean_property(prefix, key, value,
                                                      CPLDirect.cpl_cb_collect_property_lookup_vector, vp)
        return self.__lookup_by_property_helper(vp, ret)


    def try_lookup_by_string_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified, but do
        not fail if no such object is found -- return an empty list instead.
        '''
        try:
            o = self.lookup_by_string_property(prefix, key, value)
        except LookupError:
            o = []
        return o

    def try_lookup_by_numerical_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified, but do
        not fail if no such object is found -- return an empty list instead.
        '''
        try:
            o = self.lookup_by_numerical_property(prefix, key, value)
        except LookupError:
            o = []
        return o

    def try_lookup_by_boolean_property(self, prefix, key, value):
        '''
        Return all objects that have the key/value property specified, but do
        not fail if no such object is found -- return an empty list instead.
        '''
        try:
            o = self.lookup_by_boolean_property(prefix, key, value)
        except LookupError:
            o = []
        return o

    def lookup_all_objects(self, prefix, name, type):
        '''
        Return all objects that have the specified prefix, name,
        type, or bundle.
        '''
        
        vp = CPLDirect.new_std_vector_cpl_id_timestamp_tp()
        ret = CPLDirect.cpl_lookup_object_ext(prefix, name, type,
            L_NO_FAIL, CPLDirect.cpl_cb_collect_id_timestamp_vector, vp)

        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cpl_id_timestamp_tp(vp)
            raise CPLException('Unable to lookup all objects: ' +
                    CPLDirect.cpl_error_string(ret), ret)

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
            CPLDirect.delete_std_vector_cplxx_object_info_tp(vp)
            raise CPLException('Unable to lookup all objects: ' +
                    CPLDirect.cpl_error_string(ret), ret)

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
            raise CPLException('Error retrieving relations: ' +
                    CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cpl_relation_t(vp)
        l = []
        for entry in v:
            a = cpl_relation(entry.id, entry.query_object_id,
                entry.other_object_id, entry.type, D_DESCENDANTS)
            l.append(a)

        CPLDirect.delete_std_vector_cpl_relation_tp(vp)
        return l


    def delete_bundle(self, bundle):
        '''
        Delete the specified bundle and everything in it.
        '''
        ret = CPLDirect.cpl_delete_bundle(bundle.id)
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Error deleting bundle: ' +
                    CPLDirect.cpl_error_string(ret), ret)
        return None


    def validate_json(self, json):
        '''
        Checks a Prov-JSON document (in string format) for cycles and correctness.
        '''
        ret = CPLDirect.validate_json(json)
        if CPLDirect.cpl_is_ok(ret.return_code):
            return None
        return ret.out_string


    def import_document_json(self, json,
            bundle_name, anchor_objects = None, flags = 0):
        '''
        Imports a Prov-JSON document into the CPL as a bundle.

        ** Parameters **
            json the JSON document in string format
            prefix
            bundle_name
            anchor_objects: a list of cpl_object, name tuples, can be None
            flags: a logical combination of CPL_J_* flags
        '''
        if anchor_objects == None:
            id_name_vector = CPLDirect.cplxx_id_name_pair_vector()
        else:
            id_name_pairs = [(entry.get(0).id, entry.get(1)) for entry in anchor_objects]
            id_name_vector = CPLDirect.cplxx_id_name_pair_vector(id_name_pairs)
        ret, idp = CPLDirect.import_document_json(json, bundle_name,
              id_name_vector, flags)
        if not CPLDirect.cpl_is_ok(ret): 
            raise CPLException('Error importing document:' +
                    CPLDirect.cpl_error_string(ret), ret)
        return cpl_object(idp)
        

    def export_bundle_json(self, bundles):
        '''
        Exports bundles as a Prov-JSON document. Only works with single bundles currently.
        '''
        bundle_ids = [bundle.id for bundle in bundles]
        bundles_vec = CPLDirect.cpl_id_t_vector(bundle_ids)
        ret = CPLDirect.export_bundle_json(bundles_vec)
        if not CPLDirect.cpl_is_ok(ret.return_code):
            raise CPLException('Error exporting bundle:' +
                    CPLDirect.cpl_error_string(ret.return_code), ret)
        return ret.out_string


    def close(self):
        '''
        Close database connection and session
        '''
        global _cpl_connection
        
        if self != _cpl_connection or self.closed:
            return

        ret = CPLDirect.cpl_detach()
        if not CPLDirect.cpl_is_ok(ret):
            raise CPLException('Could not detach ' +
                    CPLDirect.cpl_error_string(ret), ret)

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
            raise CPLException('Could not find session information: ' +
                    CPLDirect.cpl_error_string(ret), ret)

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

    def __init__(self, object, creation_time,
            prefix, name, type):
        '''
        Create an instance of this object
        '''

        self.object = object
        self.creation_time = creation_time
        self.prefix = prefix
        self.name = name
        self.type = type



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

    def add_string_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_object_string_property(self.id, prefix, name, value)

    def add_numerical_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_object_numerical_property(self.id, prefix, name, value)

    def add_boolean_property(self, prefix, name, value):
        '''
        Add name/value pair as a property to current object.
        '''
        return CPLDirect.cpl_add_object_boolean_property(self.id, prefix, name, value)

    def info(self):
        '''
        Return cpl_object_info_t corresponding to the current object.
        '''
        objectpp = CPLDirect.new_cpl_object_info_tpp()

        ret = CPLDirect.cpl_get_object_info(self.id,
            CPLDirect.cpl_convert_pp_cpl_object_info_t(objectpp))
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_cpl_object_info_tpp(objectpp)
            raise CPLException('Unable to get object info: ' +
                    CPLDirect.cpl_error_string(ret), ret)

        op = CPLDirect.cpl_dereference_pp_cpl_object_info_t(objectpp)
        object = CPLDirect.cpl_object_info_tp_value(op)

        _info = cpl_object_info(self,
                object.creation_time,
                object.prefix, object.name, object.type)

        CPLDirect.cpl_free_object_info(op)
        CPLDirect.delete_cpl_object_info_tpp(objectpp)

        return _info


    def relations(self, direction=D_ANCESTORS, flags=0):
        '''
        Return a list of cpl_relations
        '''
        vp = CPLDirect.new_std_vector_cpl_relation_tp()

        ret = CPLDirect.cpl_get_object_relations(self.id,
            direction, flags, CPLDirect.cpl_cb_collect_relation_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cpl_relation_tp(vp)
            raise CPLException('Error retrieving relations: ' +
                    CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cpl_relation_t(vp)
        l = []
        if direction == D_ANCESTORS:
            for entry in v:
                a = cpl_relation(entry.id, entry.other_object_id,
                    entry.query_object_id, entry.type, direction)
                l.append(a)
        else:
            for entry in v:
                a = cpl_relation(entry.id, entry.query_object_id,
                    entry.other_object_id, entry.type, direction)
                l.append(a)

        CPLDirect.delete_std_vector_cpl_relation_tp(vp)
        return l


    def string_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current object.

        If prefix/key is set to something other than None, return only those
        properties matching prefix/key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_string_property_entry_tp()

        ret = CPLDirect.cpl_get_object_string_properties(self.id, prefix, key,
                                                          CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_string_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_string_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_string_property_entry_tp(vp)
        return l

    def numerical_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current object.

        If prefix/key is set to something other than None, return only those
        properties matching prefix/key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_numerical_property_entry_tp()

        ret = CPLDirect.cpl_get_object_numerical_properties(self.id, prefix, key,
                                                          CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_numerical_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_numerical_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_numerical_property_entry_tp(vp)
        return l

    def boolean_properties(self, prefix=None, key=None):
        '''
        Return all the properties associated with the current object.

        If prefix/key is set to something other than None, return only those
        properties matching prefix/key.
        '''
        vp = CPLDirect.new_std_vector_cplxx_boolean_property_entry_tp()

        ret = CPLDirect.cpl_get_object_boolean_properties(self.id, prefix, key,
                                                  CPLDirect.cpl_cb_collect_properties_vector, vp)
        if not CPLDirect.cpl_is_ok(ret):
            CPLDirect.delete_std_vector_cplxx_boolean_property_entry_tp(vp)
            raise CPLException('Error retrieving properties: ' +
                               CPLDirect.cpl_error_string(ret), ret)

        v = CPLDirect.cpl_dereference_p_std_vector_cplxx_boolean_property_entry_t(vp)
        l = []
        for e in v:
            l.append([e.prefix, e.key, e.value])
        CPLDirect.delete_std_vector_cplxx_boolean_property_entry_tp(vp)
        return l
