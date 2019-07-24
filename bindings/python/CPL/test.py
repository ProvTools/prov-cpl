#!/usr/bin/env python

"""
test.py
Core Provenance Library

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

Contributor(s): Margo Seltzer, Peter Macko
"""

"""
Test python bindings.

Currently, we only support ODBC, although we should add the RDF support and
tests as well.
"""

import CPL
import sys
import tempfile
import random

prefix = 'ptst'
iri = "python.test"
c = CPL.cpl_connection()

print "Session information: "
CPL.p_session(c.session)

# Create objects 
print
print '----- Create object tests -----'
print

rand = '_' + str(random.randint(0,10000000))
bundle_name = 'Bundle' + rand
print ('Create bundle name:' +
	bundle_name)
bundle = c.create_bundle(bundle_name, prefix)
CPL.p_object(bundle)

entity_name = 'Entity' + rand
entity_type = CPL.ENTITY
print ('Create object name:' + str(entity_name) + ' type:' + str(entity_type) +
    ' bundle:' + str(bundle.id))
entity = c.create_object(prefix, entity_name, entity_type)
if entity == None:
	print "ERROR: Could not create Entity object"
	sys.exit(1)
CPL.p_object(entity)

agent_name = 'Agent' + rand
agent_type = CPL.AGENT
print ('Create object name:' +
	str(agent_name) + ' type:' + str(agent_type))
agent = c.create_object(prefix, agent_name, agent_type)
if entity == None:
	print "ERROR: Could not create Agent object"
	sys.exit(1)
CPL.p_object(agent)

print('Lookup or create object:' + str(entity_name) + ' type:' + str(entity_type) +
 	' bundle:' + str(bundle.id))
entityt = c.get_object(prefix, entity_name, entity_type)
CPL.p_object(entityt)
if entityt.id != entity.id:
	print "ERROR: Lookup returned wrong object"
	sys.exit(1)

activity_name = 'Activity' + rand
activity_type = CPL.ACTIVITY
print ('Create object name:' +
	str(activity_name) + ' type:' + str(activity_type))
activity = c.create_object(prefix, activity_name, activity_type)
if activity == None:
	print "ERROR: Could not create Activity object"
	sys.exit(1)
CPL.p_object(activity)

# Lookup Objects
print
print '----- Lookup object tests -----'
print

print ('Looking up bundle name:' + str(bundle_name))
bundle_check = c.lookup_bundle(bundle_name, prefix)
if bundle.id != bundle_check.id:
	print('ERROR: Lookup returned wrong bundle:' + str(bundle_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(entity_name) + ' type:' + str(entity_type))
entity_check = c.lookup_object(prefix, entity_name, entity_type)
if entity.id != entity_check.id:
	print('ERROR: Lookup returned wrong object:' + str(entity_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(agent_name) + ' type:' + str(agent_type))
agent_check = c.lookup_object(prefix, agent_name, agent_type)
if agent.id != agent_check.id:
	print('ERROR: Lookup returned wrong object:' + str(agent_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(activity_name) + ' type:' + str(activity_type))
activity_check = c.lookup_object(prefix, activity_name, activity_type)
if activity.id != activity_check.id:
	print('ERROR: Lookup returned wrong object:' + str(activity_check.id))
	sys.exit(1)

print 'Look up non-existent object (type failure)'
fail1 = c.try_lookup_object(prefix, bundle_name, entity_type)
if fail1:
	print 'ERROR: Returned an object:' + str(fail1.id)
	sys.exit(1)

print 'Look up non-existent object (name failure)'
fail2 = c.try_lookup_object(prefix, 'no-name', entity_type)
if fail2:
	print 'ERROR: Returned an object:' + str(fail2.id)
	sys.exit(1)

print 'Look up non-existent object (prefix failure)'
fail3 = c.try_lookup_object('no-prefix', agent_name, agent_type)
if fail3:
	print 'ERROR: Returned an object:' + str(fail3.id)
	sys.exit(1)

print 'Look up all objects with name:' + str(entity_name) + ' type:' + str(entity_type)
entity_all = c.lookup_all_objects(prefix, entity_name, entity_type)
if len(entity_all) != 1:
	print 'ERROR: Returned the wrong number of objects:' + str(len(entity_all))
	sys.exit(1)
entity_ids = [e.id for e in entity_all]
if entity.id not in entity_ids:
	print 'ERROR: Lookup all objects with name is missing an object:' + str(entity.id)
	sys.exit(1)

# for t in entity_all:
# 	CPL.p_id(t.id, with_newline = True)
# 	i += 1
# 	if i >= 10 and len(bundle_all) > 10:
# 		print '  ... (' + str(len(entity_all)) + ' objects total)'
# 		break

print 'All objects (will return only the bundles)'
all_objects = c.get_all_objects(prefix, True)
all_objects_ids = [e.object.id for e in all_objects]
if bundle.id not in all_objects_ids:
	print 'ERROR: All objects is missing an object:' + str(entity.id)
	sys.exit(1)
i = 0
# for t in all_objects:
# 	CPL.p_id(t.object.id, with_newline = False)
# 	i += 1
# 	if i >= 10 and len(all_objects) > 10:
# 		print '  ... (' + str(len(all_objects)) + ' objects total)'
# 		break

# Relations

print
print '----- Create relation tests -----'
print

print 'relation wasAttributedTo from Entity to Agent'
r1 = c.create_relation(entity.id, agent.id, CPL.WASATTRIBUTEDTO)
if not r1:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print 'relation wasGeneratedBy from Entity to Activity'
r2 = c.create_relation(entity.id, activity.id, CPL.WASGENERATEDBY)
if not r2:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print 'relation wasAssociatedWith from Agent to Activity'
r3 = c.create_relation(agent.id, activity.id, CPL.WASASSOCIATEDWITH)
if not r3:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print 'relation bundleRelation from bundle to r1'
r4 = c.create_relation(bundle.id, r1.id, CPL.BUNDLERELATION)
if not r3:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print 'relation bundleRelation from bundle to r2'
r5 = c.create_relation(bundle.id, r2.id, CPL.BUNDLERELATION)
if not r3:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print 'relation bundleRelation from bundle to r3'
r6 = c.create_relation(bundle.id, r3.id, CPL.BUNDLERELATION)
if not r3:
	print 'ERROR: unable to create relation'
	sys.exit(1)

print "Lookup all objects with bundle_id:" + str(bundle.id)
bundle_objects = c.get_bundle_objects(bundle)
if len(bundle_objects) != 3:
	print 'ERROR: Returned wrong number of objects:' + str(len(bundle_objects))
	sys.exit(1)
bundle_objects_ids = [e.id for e in bundle_objects]
if entity.id not in bundle_objects_ids:
	print 'ERROR: Bundle is missing an object:' + str(entity.id)
	sys.exit(1)
if agent.id not in bundle_objects_ids:
	print 'ERROR: Bundle is missing an object:' + str(agent.id)
	sys.exit(1)
if activity.id not in bundle_objects_ids:
	print 'ERROR: Bundle is missing an object:' + str(activity.id)
	sys.exit(1)

print
print '----- Lookup relation tests -----'
print

print 'getting relations from Entity'
entity_anc = entity.relations(CPL.D_ANCESTORS)
if len(entity_anc) != 2:
	print 'ERROR: Returned wrong number of relations:' + str(len(entity_anc))
	sys.exit(1)


print 'getting relations to Entity'
entity_desc = entity.relations(CPL.D_DESCENDANTS)
if len(entity_desc) != 0:
	print 'ERROR: Returned wrong number of relations: ' + str(len(entity_anc))
	sys.exit(1)

print 'getting relations from Agent'
agent_anc = agent.relations(CPL.D_ANCESTORS)
if len(agent_anc) != 1:
	print 'ERROR: Returned wrong number of relations: ' + str(len(agent_anc))
	sys.exit(1)

print 'getting relations to Agent'
agent_desc = agent.relations(CPL.D_DESCENDANTS)
if len(agent_desc) != 1:
	print 'ERROR: Returned wrong number of relations: ' + str(len(agent_anc))
	sys.exit(1)

print 'getting relations from Activity'
activity_anc = activity.relations(CPL.D_ANCESTORS)
if len(activity_anc) != 0:
	print 'ERROR: Returned wrong number of relations: ' + str(len(activity_anc))
	sys.exit(1)

print 'getting relations to Activity'
activity_desc = activity.relations(CPL.D_DESCENDANTS)
if len(activity_desc) != 2:
	print 'ERROR: Returned wrong number of relations: ' + str(len(activity_anc))
	sys.exit(1)

print "Lookup all relations with bundle " + bundle_name
bundle_relations = c.get_bundle_relations(bundle)
if len(bundle_relations) != 3:
	print 'ERROR: Returned wrong number of relation: ' + str(len(bundle_relations))
	sys.exit(1)


#Bundle info
print
print '----- Bundle Info -----'
print

CPL.p_object(bundle)

#Object info
print
print '----- Object Info -----'
print

print 'Object info'
for o in [entity, agent, activity]:
	CPL.p_object(o)


#Properties
print
print '----- Properties -----'
print

print 'Adding LABEL to entity'
ret = entity.add_string_property(prefix, 'LABEL' + rand, '1')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL to agent'
ret = agent.add_string_property(prefix, 'LABEL' + rand, '2')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL/1 to activity'
ret = activity.add_numerical_property(prefix, 'LABEL' + rand, 3.5)
if ret != CPL.OK:
	print "ERROR: Unable to add numerical property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL/2 to activity'
ret = activity.add_string_property(prefix, 'LABEL' + rand, '1')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding TAG to activity'
ret = activity.add_boolean_property(prefix, 'TAG'  + rand, True)
if ret != CPL.OK:
	print "ERROR: Unable to add boolean property. Ret: " + str(ret)
	sys.exit(1)

print 'Getting string properties for entity'
string_properties = entity.string_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of string properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, '1'] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting string properties for agent'
string_properties = agent.string_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of string properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, '2'] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting numerical properties for activity'
string_properties = activity.numerical_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of numerical properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, 3.5] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting boolean properties for activity'
string_properties = activity.boolean_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of boolean properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'TAG' + rand, True] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Lookup by numerical property'
numerical_obj = c.lookup_by_numerical_property(prefix, 'LABEL' + rand, 3.5)
if len(numerical_obj) != 1:
	print 'ERROR: Returned the wrong number of objects:' + str(len(numerical_obj))
	sys.exit(1)
numerical_obj_ids = [e.id for e in numerical_obj]
if activity.id not in numerical_obj_ids:
	print 'ERROR: Lookup by numerical property is missing an object:' + str(activity.id)
	sys.exit(1)

print 'Lookup by boolean property'
boolean_obj = c.lookup_by_boolean_property(prefix, 'TAG' + rand, True)
if len(boolean_obj) != 1:
	print 'ERROR: Returned the wrong number of objects:' + str(len(boolean_obj))
	sys.exit(1)
boolean_obj_ids = [e.id for e in boolean_obj]
if activity.id not in boolean_obj_ids:
	print 'ERROR: Lookup by boolean property is missing an object:' + str(activity.id)
	sys.exit(1)

print 'Lookup by string property'
string_obj = c.lookup_by_string_property(prefix, 'LABEL' + rand, '1')
if len(string_obj) != 2:
	print 'ERROR: Returned the wrong number of objects:' + str(len(string_obj))
	sys.exit(1)
string_obj_ids = [e.id for e in string_obj]
if entity.id not in string_obj_ids:
	print 'ERROR: Lookup by string property is missing an object:' + str(entity.id)
	sys.exit(1)
if activity.id not in string_obj_ids:
	print 'ERROR: Lookup by string property is missing an object:' + str(activity.id)
	sys.exit(1)


print 'Adding LABEL to r1'
ret = r1.add_string_property(prefix, 'LABEL' + rand, '1')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL to r2'
ret = r2.add_string_property(prefix, 'LABEL' + rand, '2')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL/1 to r3'
ret = r3.add_numerical_property(prefix, 'LABEL' + rand, 3.5)
if ret != CPL.OK:
	print "ERROR: Unable to add numerical property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding LABEL/2 to r3'
ret = r3.add_string_property(prefix, 'LABEL' + rand, '1')
if ret != CPL.OK:
	print "ERROR: Unable to add string property. Ret: " + str(ret)
	sys.exit(1)

print 'Adding TAG to r3'
ret = r3.add_boolean_property(prefix, 'TAG'  + rand, True)
if ret != CPL.OK:
	print "ERROR: Unable to add boolean property. Ret: " + str(ret)
	sys.exit(1)

print 'Getting string properties for r1'
string_properties = r1.string_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of string properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, '1'] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting string properties for r2'
string_properties = r2.string_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of string properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, '2'] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting numerical properties for r3'
string_properties = r3.numerical_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of numerical properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'LABEL' + rand, 3.5] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)

print 'Getting boolean properties for r3'
string_properties = r3.boolean_properties()
if len(string_properties) != 1:
	print "ERROR: Wrong number of boolean properties: " + str(len(string_properties))
	sys.exit(1)
if [prefix, 'TAG' + rand, True] not in string_properties:
	print "ERROR: Wrong property was returned"
	sys.exit(1)


#TODO: uncomment this when deleting bundles is implemented

# print
# print "Deleting bundle"
# c.delete_bundle(bundle)
# try:
# 	c.lookup_bundle(bundle_name, prefix)
# except CPL.CPLException:
# 	print "ERROR: Bundle was not deleted"
# 	sys.exit(1)


# Exit
print "Closing connection"
c.close()

print
print "All tests passed."
