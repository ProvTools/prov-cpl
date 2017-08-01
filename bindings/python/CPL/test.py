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

originator = 'python_test'
c = CPL.cpl_connection()

print "Session information: "
CPL.p_session(c.session)

# Create objects 
print
print '----- Create object tests -----'
print

bundle_name = 'Bundle'
bundle_type = CPL.BUNDLE
print ('Create object name:' +
	bundle_name + ' type:' + str(bundle_type) + ' bundle:void')
bundle = c.create_object(originator, bundle_name, bundle_type)
CPL.p_object(bundle, False)

entity_name = 'Entity'
entity_type = CPL.ENTITY
print ('Create object name:' + str(entity_name) + ' type:' + str(entity_type) +
    ' bundle:' + str(bundle.id))
entity = c.create_object(originator, entity_name, entity_type, bundle)
CPL.p_object(entity)

agent_name = 'Agent'
agent_type = CPL.AGENT
print ('Create object name:' +
	str(agent_name) + ' type:' + str(agent_type) + ' bundle:' + str(bundle.id))
agent = c.create_object(originator, agent_name, agent_type, bundle)
CPL.p_object(agent)

print('Lookup or create object:' + str(entity_name) + ' type:' + str(entity_type) +
 	' bundle:' + str(bundle.id))
entityt = c.get_object(originator, entity_name, entity_type, bundle = bundle)
CPL.p_object(entityt)
if entity.id != entity.id:
	print "Lookup returned wrong object!"
	sys.exit(1)

activity_name = 'Activity'
activity_type = CPL.ACTIVITY
print ('Create object name:' +
	str(activity_name) + ' type:' + str(activity_type) + ' bundle:' + str(bundle.id))
activity = c.create_object(originator, activity_name, activity_type, bundle)
CPL.p_object(bundle)

# Lookup Objects
print
print '----- Lookup object tests -----'
print

print ('Looking up object name:' + str(bundle_name) + ' type:' + str(bundle_type))
bundle_check = c.lookup_object(originator, bundle_name, bundle_type, None)
if bundle.id != bundle_check.id:
	sys.stdout.write('Lookup returned wrong object:' + str(bundle_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(entity_name) + ' type:' + str(entity_type))
entity_check = c.lookup_object(originator, entity_name, entity_type, bundle)
if entity.id != entity_check.id:
	sys.stdout.write('Lookup returned wrong object:' + str(entity_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(agent_name) + ' type:' + str(agent_type))
agent_check = c.lookup_object(originator, agent_name, agent_type, bundle)
if agent.id != agent_check.id:
	sys.stdout.write('Lookup returned wrong object:' + str(agent_check.id))
	sys.exit(1)

print ('Looking up object name:' + str(activity_name) + ' type:' + str(activity_type))
activity_check = c.lookup_object(originator, activity_name, activity_type, bundle)
if activity.id != activity_check.id:
	sys.stdout.write('Lookup returned wrong object:' + str(activity_check.id))
	sys.exit(1)

print ('Looking up object in wrong bundle name:' +
 	str(entity_name) + ' type:' + str(entity_type) + ' bundle:Agent')
try:
	fail0 = c.lookup_object(originator, entity_name, entity_type, agent)
except LookupError:
	pass
else:
	if fail0 != None:
	 	sys.stdout.write('Lookup returned an object:' + str(fail0.id))
	 	sys.exit(1)

print 'Look up non-existent object (type failure)'
fail1 = c.try_lookup_object(originator, bundle_name, entity_type)
if fail1:
	print 'Returned an object:' + str(fail1.id)
	sys.exit(1)

print 'Look up non-existent object (name failure)'
fail2 = c.try_lookup_object(originator, 'no-name', bundle_type)
if fail2:
	print 'Returned an object:' + str(fail2.id)
	sys.exit(1)

print 'Look up non-existent object (originator failure)'
fail3 = c.try_lookup_object('no-originator', bundle_name, bundle_type)
if fail3:
	print 'Returned an object:' + str(fail3.id)
	sys.exit(1)

print 'Look up all objects with name:' + str(bundle_name) + ' type:' + str(bundle_type)
bundle_all = c.lookup_all(originator, bundle_name, bundle_type)
i = 0
for t in bundle_all:
	CPL.p_id(t.id, with_newline = True)
	i += 1
	if i >= 10 and len(bundle_all) > 10:
		print '  ... (' + str(len(bundle_all)) + ' objects total)'
		break

print 'All objects'
all_objects = c.get_all_objects(True)
i = 0
for t in all_objects:
	CPL.p_id(t.object.id, with_newline = False)
	i += 1
	if i >= 10 and len(all_objects) > 10:
		print '  ... (' + str(len(all_objects)) + ' objects total)'
		break

print "Lookup all objects with bundle_id:" + str(bundle.id)
bundle_objects = c.get_bundle_objects(bundle)
if len(bundle_objects) != 3:
	print 'Returned wrong number of objects:' + str(len(bundle_objects))
	sys.exit(1)

# Relations

print
print '----- Create relation tests -----'
print

print 'relation wasAttributedTo from Entity to Agent'
r1 = entity.relation_to(agent, CPL.WASATTRIBUTEDTO, bundle)
if not r1:
	print 'ERROR: ignoring duplicate'
	sys.exit(1)

print 'relation wasGeneratedBy from Entity to Activity'
r2 = entity.relation_to(activity, CPL.WASGENERATEDBY, bundle)
if not r2:
	print 'ERROR: ignoring duplicate'
	sys.exit(1)

print 'relation wasAssociatedWith from Activity to Agent'
r3 = agent.relation_from(activity, CPL.WASASSOCIATEDWITH, bundle)
if not r3:
	print 'ERROR: ignoring duplicate'
	sys.exit(1)

print
print '----- Lookup relation tests -----'
print

print 'getting relations from Entity'
entity_anc = entity.relations(CPL.D_ANCESTORS)
if len(entity_anc) != 2:
	print 'Returned wrong number of relations:' + str(len(entity_anc))

print 'getting relations to Entity'
entity_desc = entity.relations(CPL.D_DESCENDANTS)
if len(entity_desc) != 0:
	print 'Returned wrong number of relations: ' + str(len(entity_anc))

print 'getting relations from Agent'
agent_anc = agent.relations(CPL.D_ANCESTORS)
if len(agent_anc) != 0:
	print 'Returned wrong number of relations: ' + str(len(agent_anc))

print 'getting relations to Agent'
agent_desc = agent.relations(CPL.D_DESCENDANTS)
if len(agent_desc) != 2:
	print 'Returned wrong number of relations: ' + str(len(agent_anc))

print 'getting relations from Activity'
activity_anc = activity.relations(CPL.D_ANCESTORS)
if len(activity_anc) != 1:
	print 'Returned wrong number of relations: ' + str(len(activity_anc))

print 'getting relations to Activity'
activity_desc = activity.relations(CPL.D_DESCENDANTS)
if len(activity_desc) != 1:
	print 'Returned wrong number of relations: ' + str(len(activity_anc))

print "Lookup all relations with bundle " + bundle_name
bundle_relations = c.get_bundle_relations(bundle)
if len(bundle_relations) != 3:
	print 'Returned wrong number of relation: ' + str(len(bundle_relations))
	sys.exit(1)


#Object info
print
print '----- Object Info -----'
print

print 'Object info'
for o in [bundle, entity, agent, activity]:
	CPL.p_object(o)


#Properties
print
print '----- Properties -----'
print

print 'Adding LABEL/1 to entity'
entity.add_property('LABEL', '1')

print 'Adding LABEL/2 to bundle'
agent.add_property('LABEL', '2')

print 'Adding LABEL/3 to entity'
activity.add_property('LABEL', '3')

print 'Adding TAG/Hello to agent'
activity.add_property('TAG', 'HELLO')

print 'Getting properties for entity'
print entity.properties()

print 'Getting properties for agent'
print agent.properties()

print 'Getting properties for activity'
print activity.properties()

print 'Getting all objects with LABEL/3 property'
tuples = c.lookup_by_property('LABEL', '3')
i = 0
for t in tuples:
	print str(t)
	i += 1
	if i >= 10 and len(tuples) > 10:
		print '  ... (' + str(len(tuples)) + ' tuples total)'
		break

print 'Adding LABEL/1 to r1'
r1.add_property('LABEL', '1')

print 'Adding LABEL/2 to r2'
r2.add_property('LABEL', '2')

print 'Adding LABEL/3 to r3'
r3.add_property('LABEL', '3')

print 'Adding TAG/Hello to r3'
r3.add_property('TAG', 'Hello')

print 'Getting properties for r1'
print r1.properties()

print 'Getting properties for r2'
print r2.properties()

print 'Getting properties for r3'
print r3.properties()


# Exit
print
print "Closing connection"
c.close()
