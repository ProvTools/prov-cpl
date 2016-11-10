package edu.harvard.pass.cpl;

/*
 * JSONUtility.java
 * Prov-CPL
 *
 * Copyright 2016
 *      The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Contributor(s): Jackson Okuhn
 */


import org.json.simple;
import java.io.FileReader;

class JSONUtility {

	ProvFactory pFactory;

	public JSONUtility() {
		pFactory = null;
	}

	private static void importBundlePrefixes(CPLObject bundle, JSONObject document) {
		JSONObject prefixes = document.getJsonObject("prefix");

		for (String key : prefixes.keySet()) {

			bundle.addProperty(key, prefixes.get(key).toString());
		}
	}

	private static void importObjects(String type) {

		JSONObject objects = document.getJsonObject(type);

		for (String name : objects.keySet()) {

			CPLObject object;

			switch (type) {
				case CPLObject.ENTITY:
					object = pFactory.lookupOrCreateEntity(name);
					break;
				case CPLObject.AGENT:
					object = pFactory.lookupOrCreateAgent(name);
					break;
				case CPLObject.ACTIVITY:
					object = pFactory.lookupOrCreateActivity(name);
					break;
				default:
					break;
			}

			JSONObject properties = objects.getJsonObject(name);

			for (String property: properties.keySet()){

				entity.addProperty(property, jsonProps.getString(property));
			}
		}	
	}

	private void importRelations(String typeStr, int typeInt) {

		JSONObject relations = document.getJsonObject(typeStr);

		for (String relationName : relation.keySet()) {

			JSONObject relationJSON = objects.getJsonObject(relationName);

			String o = pFactory.getOriginator(); //may need a fix later

			CPLObject source, dest;
			CPLRelation relation;

			switch (typeInt) {
				//TODO deal with relations w/o dest
				case CPLRelation.ALTERNATEOF:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:alternate1"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:alternate2"), "entity");
					relation = pFactory.createAlternateOf(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:alternate1" || property != "prov:alternate2")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.DERIVEDBYINSERTIONFROM:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:after"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:before"), "entity");
					relation = pFactory.createDerivedByInsertionFrom(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:after" || property != "prov:before")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.DERIVEDBYREMOVALFROM:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:after"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:before"), "entity");
					relation = pFactory.createDerivedByRemovalFrom(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:after" || property != "prov:before")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.HADMEMBER:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:collection"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					relation = pFactory.createHadMember(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:collection" || property != "prov:entity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.HADDICTIONARYMEMBER:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:dictionary"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					relation = pFactory.createHadDictionaryMember(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:dictionary" || property != "prov:entity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.SPECIALIZATIONOF:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:generalEntity"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:specificEntity"), "entity");
					relation = pFactory.createSpecializationOf(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:generalEntity" || property != "prov:specificEntity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASDERIVEDFROM:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:generatedEntity"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:usedEntity"), "entity");
					relation = pFactory.createWasDerivedFrom(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:generatedEntity" || property != "prov:usedEntity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASGENERATEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					relation = pFactory.createWasGeneratedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:entity" || property != "prov:activity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASINVALIDATEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					relation = pFactory.createWasInvalidatedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:entity" || property != "prov:activity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASATTRIBUTEDTO:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:agent"), "agent");
					relation = pFactory.createWasAttributedTo(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:entity" || property != "prov:agent")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.USED:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:entity"), "entity");
					relation = pFactory.createUsed(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:activity" || property != "prov:entity")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASINFORMEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:informed"), "activity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:informant"), "activity");
					relation = pFactory.createWasInformedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:informed" || property != "prov:informant")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASSTARTEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:trigger"), "entity");
					relation = pFactory.createWasStartedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:activity" || property != "prov:trigger")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASENDEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:trigger"), "entity");
					relation = pFactory.createWasEndedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:activity" || property != "prov:trigger")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.HADPLAN:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:agent"), "agent");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:plan"), "entity");
					relation = pFactory.createHadPlan(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:agent" || property != "prov:plan")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASASSOCIATEDWITH:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:agent"), "agent");
					relation = pFactory.createWasAssociatedWith(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:activity" || property != "prov:agent")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.ACTEDONBEHALFOF:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:delegate"), "agent");
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:responsible"), "agent");
					relation = pFactory.createActedOnBehalfOf(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:delegate" || property != "prov:responsible")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				case CPLRelation.WASINFLUENCEDBY:
					source = CPLObject.tryLookup(o,
						relationJSON.getString("prov:influencee"), null);
					dest = CPLObject.tryLookup(o,
						relationJSON.getString("prov:influencer"), null);
					relation = pFactory.createWasInfluencedBy(source, dest);
					for (String property: relationJSON.keySet()) {
						//if(property != "prov:influencee" || property != "prov:influencer")
							relation.addProperty(property, relationJSON.getString(property));
					}
					break;

				default:
					break;
			}
		}
	}

	public void importJSON(String fileName, String originator, 
			String bundleName, CPLObject anchorObject, CPLObject bundleAgent) {

		//TODO add error handling

		JSONParser parser = new JSONParser();

		JSONObject document = parser.parse(new FileReader(fileName));

		//TODO implement ID cycles

		pFactory = new ProvFactory(originator);

		pFactory.setBundle(bundleName);

		importBundlePrefixes(document, pFactory.getBundle());

		if(bundleAgent != null){
			pFactory.createWasAttributedTo(pFactory.getBundle(), anchorObject);
		}

		//TODO implement addObjects("bundle");

		importObjects("entity");
		importObjects("agent");	
		importObjects("activity");

		if(anchorObject != null){
			createAlternateOf(anchorObject,
				CPLObject.tryLookup(pFactory.getOriginator(), anchorObject.getName(), anchorObject.getType()));
		}
		importRelations("alternateOf", CPLRelation.ALTERNATEOF);
		importRelations("derivedByInsertionFrom", CPLRelation.DERIVEDBYINSERTIONFROM);
		importRelations("derivedByRemovalFrom", CPLRelation.DERIVEDBYREMOVALFROM);
		importRelations("hadMember", CPLRelation.HADMEMBER);
		importRelations("hadDictionaryMember", CPLRelation.HADDICTIONARYMEMBER);
		importRelations("specializationOf", CPLRelation.SPECIALIZATIONOF);
		importRelations("wasDerivedFrom", CPLRelation.WASDERIVEDFROM);
		importRelations("wasGeneratedBy", CPLRelation.WASGENERATEDBY);
		importRelations("wasInvalidatedBy", CPLRelation.WASINVALIDATEDBY);
		importRelations("wasAttributedTo", CPLRelation.WASATTRIBUTEDTO);
		importRelations("used", CPLRelation.USED);
		importRelations("wasInformedBy", CPLRelation.WASINFORMEDBY);
		importRelations("wasStartedBy", CPLRelation.WASSTARTEDBY);
		importRelations("wasEndedBy", CPLRelation.WASENDEDBY);
		importRelations("hadPlan", CPLRelation.HADPLAN);
		importRelations("wasAssociatedWith", CPLRelation.WASASSOCIATEDWITH);
		importRelations("actedOnBehalfOf", CPLRelation.ACTEDONBEHALFOF);
		importRelations("wasInfluencedBy", CPLRelation.WASINFLUENCEDBY);
	}

	public JSONObjectBuilder exportBundlePrefixes(CPLObject bundle){
		
		Vector<CPLObjectProperty> prefixVec = bundle.getProperties();

		if (!prefixVec.isEmpty()) {
			JSONObjectBuilder prefixes =  Json.createObjectBuilder();
			for(CPLObjectPropertyEntry prefix: prefixVec){
				prefixes.add(prefix.getKey(), prefix.getValue());
			}
			
			return Json.createObjectBuilder()
				.add("prefixes", prefixes);
		}

		return null;
	}

	public JSONObjectBuilder exportObjects(CPLObject bundle){

		Vector<CPLObject> objectsVec = CPLObject.getBundleObjects(bundle);

		if(!objectsVec.isEmpty()) {
			JSONObjectBuilder objects = Json.createObjectBuilder();
			JSONObjectBuilder entities = Json.createObjectBuilder();
			JSONObjectBuilder activities = Json.createObjectBuilder();
			JSONObjectBuilder agents = Json.createObjectBuilder();
			boolean entityExists=false, activityExists=false, agentExists=false;
			Vector<CPLObjectPropertyEntry> propertiesVec;

			for(CPLObject object: objects) {
				JSONObjectBuilder properties = Json.createObjectBuilder();
				propertiesVec = object.getProperties();
				for(CPLObjectPropertyEntry property: propertiesVec) {
					properties.add(prefix.getKey(), prefix.getValue());
				}

				switch(object.getType()) {
					case CPLObject.ENTITY:
						entityExists = true;
						entities.add(object.getName(), properties);
						break;
					case CPLObject.ACTIVITY:
						activityExists = true;
						activities.add(object.getName(), properties);
						break;
					case CPLObject.AGENT:
						agentsExists = true;
						agents.add(object.getName(), properties);
						break;
					default:
						break;
				}
			}

			if(entityExists)
				objects.add("entity", entities);
			if(activityExists)
				objects.add("activity", activities);
			if(agentExists)
				objects.add("agent", agents);

			return objects;
		}

		return null;
	}

	public JSONObjectBuilder exportRelations(CPLObject bundle){

		Vector<CPLObject> relationsVec = CPLObject.getBundleRelations(bundle);

		if(!relationsVec.isEmpty()){
			JSONObjectBuilder relations = Json.createObjectBuilder();
			JSONObjectBuilder alternateOf = Json.createObjectBuilder();
			JSONObjectBuilder derivedByInsertionFrom = Json.createObjectBuilder();
			JSONObjectBuilder derivedByRemovalFrom = Json.createObjectBuilder();
			JSONObjectBuilder hadMember = Json.createObjectBuilder();
			JSONObjectBuilder hadDictionaryMember = Json.createObjectBuilder();
			JSONObjectBuilder specializationOf = Json.createObjectBuilder();
			JSONObjectBuilder wasDerivedFrom = Json.createObjectBuilder();
			JSONObjectBuilder wasGeneratedBy = Json.createObjectBuilder();
			JSONObjectBuilder wasInvalidatedBy = Json.createObjectBuilder();
			JSONObjectBuilder wasAttributedTo = Json.createObjectBuilder();
			JSONObjectBuilder used = Json.createObjectBuilder();
			JSONObjectBuilder wasInformedBy = Json.createObjectBuilder();
			JSONObjectBuilder wasStartedBy = Json.createObjectBuilder();
			JSONObjectBuilder wasEndedBy = Json.createObjectBuilder();
			JSONObjectBuilder hadPlan = Json.createObjectBuilder();
			JSONObjectBuilder wasAssociatedWith = Json.createObjectBuilder();
			JSONObjectBuilder actedOnBehalfOf = Json.createObjectBuilder();
			JSONObjectBuilder wasInfluencedBy = Json.createObjectBuilder();
			boolean alternateOfExists=false, derivedByInsertionFromExists=false, 
			derivedByRemovalFromExists=false, hadMemberExists=false, 
			hadDictionaryMemberExists=false, specializationOfExists=false, 
			wasDerivedFromExists=false, wasGeneratedByExists=false, 
			wasInvalidatedByExists=false, wasAttributedToExists=false, 
			usedExists=false, wasInformedByExists=false, 
			wasStartedByExists=false, wasEndedByExists=false, 
			hadPlanExists=false, wasAssociatedWithExists=false, 
			actedOnBehalfOfExists=false, wasInfluencedByExists=false;

			Vector<CPLObjectPropertyEntry> propertiesVec;

			for(CPLObject relation: relations) {
				JSONObjectBuilder properties = Json.createObjectBuilder();
				propertiesVec = relation.getProperties();
				for(CPLRelationPropertyEntry property: propertiesVec) {
					properties.add(prefix.getKey(), prefix.getValue());
				}

				int type = relation.getType();

				switch(type) {
					case CPLRelation.ALTERNATEOF:
						alternateOfExists = true;
						alternateOf.add(relation.getName(), properties);
						break;
					case CPLRelation.DERIVEDBYINSERTIONFROM:
						derivedByInsertionFromExists = true;
						derivedByInsertionFrom.add(relation.getName(), properties);
						break;
					case CPLRelation.DERIVEDBYREMOVALFROM:
						derivedByRemovalFromExists = true;
						derivedByRemovalFrom.add(relation.getName(), properties);
						break;
					case CPLRelation.HADMEMBER:
						hadMemberExists = true;
						hadMember.add(relation.getName(), properties);
						break;
					case CPLRelation.HADDICTIONARYMEMBER:
						hadDictionaryMemberExists = true;
						hadDictionaryMember.add(relation.getName(), properties);
						break;
					case CPLRelation.SPECIALIZATIONOF:
						specializationOfExists = true;
						specializationOf.add(relation.getName(), properties);
						break;
					case CPLRelation.WASDERIVEDFROM:
						wasDerivedFromExists = true;
						wasDerivedFrom.add(relation.getName(), properties);
						break;
					case CPLRelation.WASGENERATEDBY:
						wasGeneratedByExists = true;
						wasGeneratedBy.add(relation.getName(), properties);
						break;
					case CPLRelation.WASINVALIDATEDBY:
						wasInvalidatedByExists = true;
						wasInvalidatedBy.add(relation.getName(), properties);
						break;
					case CPLRelation.WASATTRIBUTEDTO:
						wasAttributedToExists = true;
						wasAttributedTo.add(relation.getName(), properties);
						break;
					case CPLRelation.USED:
						usedExists = true;
						used.add(relation.getName(), properties);
						break;
					case CPLRelation.WASINFORMEDBY:
						wasInformedByExists = true;
						wasInformedBy.add(relation.getName(), properties);
						break;
					case CPLRelation.WASSTARTEDBY:
						wasStartedByExists = true;
						wasStartedBy.add(relation.getName(), properties);
						break;
					case CPLRelation.WASENDEDBY:
						wasEndedByExists = true;
						wasEndedBy.add(relation.getName(), properties);
						break;
					case CPLRelation.HADPLAN:
						hadPlanExists = true;
						hadPlan.add(relation.getName(), properties);
						break;
					case CPLRelation.WASASSOCIATEDWITH:
						wasAssociatedWithExists = true;
						wasAssociatedWith.add(relation.getName(), properties);
						break;
					case CPLRelation.ACTEDONBEHALFOF:
						actedOnBehalfOfExists = true;
						actedOnBehalfOf.add(relation.getName(), properties);
						break;
					case CPLRelation.WASINFLUENCEDBY:
						wasInfluencedByExists = true;
						wasInfluencedBy.add(relation.getName(), properties);
						break;
					default:
						break;
				}
			}

			if(alternateofExists)
				relations.add("alternateOf",alternateOf);
			if(derivedByInsertionFromExists)
				relations.add("derivedByInsertionFrom",derivedByInsertionFrom);
			if(derivedByRemovalFromExists)
				relations.add("derivedByRemovalFrom",derivedByRemovalFrom);
			if(hadMemberExists)
				relations.add("hadMember",hadMember);
			if(hadDictionaryMemberExists)
				relations.add("hadDictionaryMember",hadDictionaryMember);
			if(specializationOfExists)
				relations.add("specializationOf",specializationOf);
			if(wasDerivedFromExists)
				relations.add("wasDerivedFrom",wasDerivedFrom);
			if(wasGeneratedByExists)
				relations.add("wasGeneratedBy",wasGeneratedBy);
			if(wasInvalidatedByExists)
				relations.add("wasInvalidatedBy",wasInvalidatedBy);
			if(wasAttributedToExists)
				relations.add("wasAttributedTo",wasAttributedTo);
			if(usedExists)
				relations.add("used",used);
			if(wasInformedByExists)
				relations.add("wasInformedBy",wasInformedBy);
			if(wasStartedByExists)
				relations.add("wasStartedBy",wasStartedBy);
			if(wasEndedByExists)
				relations.add("wasEndedBy",wasEndedBy);
			if(hadPlanExists)
				relations.add("hadPlan",hadPlan);
			if(wasAssociatedWithExists)
				relations.add("wasAssociatedWith",wasAssociatedWith);
			if(actedOnBehalfOfExists)
				relations.add("actedOnBehalfOf",actedOnBehalfOf);
			if(wasInfluencedByExists)
				relations.add("wasInfluencedBy",wasInfluencedBy);

			return relations;
		}

		return null;
	}

	public static JSONObject exportJSON(CPLObject bundle) {

 		JsonObject documentBuilder = Json.createObjectBuilder();


 		JSONObjectBuilder prefixBuilder = importBundlePrefixes(bundle);
 		if(prefixBuilder != null){
 			 documentBuilder.add(prefixBuilder);
 		}

 		JSONObjectBuilder objectBuilder = exportObjects(bundle);
 		if(objectBuilder != null){
 			 documentBuilder.add(objectBuilder);
 		}

 		JSONObjectBuilder relationBuilder = exportRelations(bundle);
 		if(relationBuilder != null){
 			 documentBuilder.add(relationBuilder);
 		}

 		return documentBuilder.build();
	}
}