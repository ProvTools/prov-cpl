package edu.harvard.pass.cpl;

/*
 * JsonUtility.java
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


import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import org.jgrapht.DirectedGraph;
import org.jgrapht.graph.*;
import org.jgrapht.alg.*;
import java.io.FileReader;
import java.util.Vector;
import java.util.Set;
import java.util.Iterator;

/**
 * A utility for processing Prov JSON
 *
 * @author Jackson Okuhn
 */
class JsonUtility {

	ProvFactory pFactory;

	JSONObject document;

	/**
	 * Create an instance of JsonUtility
	 */
	public JsonUtility() {
		pFactory = null;
		document = null;
	}

	/**
	 * Verify the correctness of a Prov JSON document
	 *
	 *@param fileName document location
	 *@return a string detailing errors if any exist
	 */
	public String verifyJson(String fileName){
		
		JSONParser parser = new JSONParser();

		String out = "JSON verification failed. Reason unspecified";

		try {

			Object obj = parser.parse(new FileReader(fileName));

			document = (JSONObject) obj;

		   	DirectedGraph<String, DefaultEdge> directedGraph =
	        new DefaultDirectedGraph<String, DefaultEdge>
	        (DefaultEdge.class);

	        JSONObject relations;

			relations = (JSONObject) document.get("alternateOf");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:alternate1");
				String dest = (String) relationJson.get("prov:alternate2");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("derivedByInsertionFrom");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:after");
				String dest = (String) relationJson.get("prov:before");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("derivedByRemovalFrom");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:after");
				String dest = (String) relationJson.get("prov:before");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("hadMember");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:collection");
				String dest = (String) relationJson.get("prov:entity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("hadDictionaryMember");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:dictionary");
				String dest = (String) relationJson.get("prov:entity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("specializationOf");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:specificEntity");
				String dest = (String) relationJson.get("prov:generalEntity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasDerivedFrom");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:generatedEntity");
				String dest = (String) relationJson.get("prov:usedEntity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasGeneratedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:entity");
				String dest = (String) relationJson.get("prov:activity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasInvalidatedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:entity");
				String dest = (String) relationJson.get("prov:activity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasAttributedTo");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:entity");
				String dest = (String) relationJson.get("prov:agent");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("used");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:activity");
				String dest = (String) relationJson.get("prov:entity");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasInformedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:informed");
				String dest = (String) relationJson.get("prov:informant");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasStartedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:activity");
				String dest = (String) relationJson.get("prov:trigger");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasEndedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:activity");
				String dest = (String) relationJson.get("prov:trigger");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("hadPlan");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:agent");
				String dest = (String) relationJson.get("prov:plan");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasAssociatedWith");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:activity");
				String dest = (String) relationJson.get("prov:agent");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("actedOnBehalfOf");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:delegate");
				String dest = (String) relationJson.get("prov:responsible");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			relations = (JSONObject) document.get("wasInfluencedBy");
			for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){
				String name = (String) relIterator.next();

				JSONObject relationJson = (JSONObject) relations.get(name); 

				String source = (String) relationJson.get("prov:influencee");
				String dest = (String) relationJson.get("prov:influencer");
				directedGraph.addVertex(source);
				directedGraph.addVertex(dest);
				directedGraph.addEdge(source, dest);
			}

			CycleDetector cycleDetector = new CycleDetector(directedGraph);

			Set<String> cycleVertices = cycleDetector.findCycles();

			if(!cycleVertices.isEmpty()){
				out = "JSON verification failed." +
					"The following vertices belong to one or more cycles:\n";
				for(String vertex : cycleVertices) {
					out += vertex + "\n";
				}

			} else {
				out = "JSON verified";
			}

		} catch (FileNotFoundException e) {
			e.printStackTrace();

		} catch (IOException e) {
			e.printStackTrace();

		} catch (ParseException e) {
			e.printStackTrace();
		}

		return out;
	}

	private void importBundlePrefixes(CPLObject bundle) {

		JSONObject prefixes = (JSONObject) document.get("prefix");

		for (Iterator preIterator = prefixes.keySet().iterator(); preIterator.hasNext();) {

			String key = (String) preIterator.next();

			bundle.addProperty(key, prefixes.get(key).toString());
		}
	}

	private void importObjects(String type) {

		JSONObject objects = (JSONObject) document.get(type);

		for (Iterator objIterator = objects.keySet().iterator(); objIterator.hasNext();) {

			String name = (String) objIterator.next();

			CPLObject object = null;

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

			if(object != null) {
				JSONObject properties = (JSONObject) objects.get(name);

				for (Iterator propIterator = properties.keySet().iterator(); propIterator.hasNext();){

					String property = (String) propIterator.next();

					object.addProperty(property, (String) properties.get(property));
				}
			}
		}	
	}

	private void importRelations(String typeStr, int typeInt) {

		JSONObject relations = (JSONObject) document.get(typeStr);

		for (Iterator relIterator = relations.keySet().iterator(); relIterator.hasNext();){

			String relationName = (String) relIterator.next();

			JSONObject relationJson = (JSONObject) relations.get(relationName);

			String o = pFactory.getOriginator(); //may need a fix later

			CPLObject source, dest;
			CPLRelation relation;

			switch (typeInt) {
				//TODO deal with relations w/o dest
				case CPLRelation.ALTERNATEOF:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:alternate1"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:alternate2"), "entity");
					relation = pFactory.createAlternateOf(source, dest);
		
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.DERIVEDBYINSERTIONFROM:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:after"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:before"), "entity");
					relation = pFactory.createDerivedByInsertionFrom(source, dest);

					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.DERIVEDBYREMOVALFROM:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:after"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:before"), "entity");
					relation = pFactory.createDerivedByRemovalFrom(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.HADMEMBER:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:collection"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					relation = pFactory.createHadMember(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.HADDICTIONARYMEMBER:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:dictionary"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					relation = pFactory.createHadDictionaryMember(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.SPECIALIZATIONOF:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:specificlEntity"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:generalEntity"), "entity");
					relation = pFactory.createSpecializationOf(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASDERIVEDFROM:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:generatedEntity"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:usedEntity"), "entity");
					relation = pFactory.createWasDerivedFrom(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASGENERATEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					relation = pFactory.createWasGeneratedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASINVALIDATEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					relation = pFactory.createWasInvalidatedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASATTRIBUTEDTO:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:agent"), "agent");
					relation = pFactory.createWasAttributedTo(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.USED:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:entity"), "entity");
					relation = pFactory.createUsed(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASINFORMEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:informed"), "activity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:informant"), "activity");
					relation = pFactory.createWasInformedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASSTARTEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:trigger"), "entity");
					relation = pFactory.createWasStartedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASENDEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:trigger"), "entity");
					relation = pFactory.createWasEndedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.HADPLAN:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:agent"), "agent");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:plan"), "entity");
					relation = pFactory.createHadPlan(source, dest);
						
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASASSOCIATEDWITH:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:activity"), "activity");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:agent"), "agent");
					relation = pFactory.createWasAssociatedWith(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.ACTEDONBEHALFOF:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:delegate"), "agent");
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:responsible"), "agent");
					relation = pFactory.createActedOnBehalfOf(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				case CPLRelation.WASINFLUENCEDBY:
					source = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:influencee"), null);
					dest = CPLObject.tryLookup(o,
						(String) relationJson.get("prov:influencer"), null);
					relation = pFactory.createWasInfluencedBy(source, dest);
					
					for (Iterator propIterator = relationJson.keySet().iterator(); propIterator.hasNext();){
							String property = (String) propIterator.next();
							relation.addProperty(property, (String) relationJson.get(property));
					}
					break;

				default:
					break;
			}
		}
	}

	/**
	 * Import a Prov JSON document into Prov-CPL as a bundle. Does not verify correctness.
	 * Currently supports only one anchor object.
	 *
	 * @param fileName document location
	 * @param originator document originator
	 * @param bundleName name of document bundle
	 * @param anchorObject Prov-CPL object identical to an object in the document
	 * @param bundleAgent agent responsible for the document bundle
	 */

	public void importJson(String fileName, String originator, 
			String bundleName, CPLObject anchorObject, CPLObject bundleAgent) {

		//TODO improve error handling

		JSONParser parser = new JSONParser();

		try {
			Object obj = parser.parse(new FileReader(fileName));

			document = (JSONObject) obj;

			pFactory = new ProvFactory(originator);

			//TODO: exit if failed
			pFactory.setBundle(bundleName);

			importBundlePrefixes(pFactory.getBundle());

			if(bundleAgent != null){
				pFactory.createWasAttributedTo(pFactory.getBundle(), bundleAgent);
			}

			//TODO implement addObjects("bundle"); next release

			importObjects("entity");
			importObjects("agent");	
			importObjects("activity");

			if(anchorObject != null){
				pFactory.createAlternateOf(anchorObject,
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

		} catch (FileNotFoundException e) {
			e.printStackTrace();

		} catch (IOException e) {
			e.printStackTrace();

		} catch (ParseException e) {
			e.printStackTrace();
		}
	}

	public void exportBundlePrefixes(CPLObject bundle){
		
		Vector<CPLObjectPropertyEntry> prefixVec = bundle.getProperties();

		if (!prefixVec.isEmpty()) {
			JSONObject prefixes =  new JSONObject();
			for(CPLObjectPropertyEntry prefix: prefixVec){
				prefixes.put(prefix.getKey(), prefix.getValue());
			}
			
			document.put("prefixes", prefixes);
		}
	}

	public void exportObjects(CPLObject bundle){

		Vector<CPLObject> objectsVec = CPLObject.getBundleObjects(bundle);

		if(!objectsVec.isEmpty()) {
			JSONObject objects = new JSONObject();
			JSONObject entities = new JSONObject();
			JSONObject activities = new JSONObject();
			JSONObject agents = new JSONObject();

			Vector<CPLObjectPropertyEntry> propertiesVec;

			for(CPLObject object: objectsVec) {
				JSONObject properties = new JSONObject();
				propertiesVec = object.getProperties();
				for(CPLObjectPropertyEntry property: propertiesVec) {
					properties.put(property.getKey(), property.getValue());
				}

				switch(object.getType()) {
					case CPLObject.ENTITY:
						entities.put(object.getName(), properties);
						break;
					case CPLObject.ACTIVITY:
						activities.put(object.getName(), properties);
						break;
					case CPLObject.AGENT:
						agents.put(object.getName(), properties);
						break;
					default:
						break;
				}
			}

			if(!entities.isEmpty())
				document.put("entity", entities);
			if(!activities.isEmpty())
				document.put("activity", activities);
			if(!agents.isEmpty())
				document.put("agent", agents);
		}
	}

	public void exportRelations(CPLObject bundle){

		Vector<CPLRelation> relationsVec = CPLObject.getBundleRelations(bundle);

		if(!relationsVec.isEmpty()){
			JSONObject relations = new JSONObject();
			JSONObject alternateOf = new JSONObject();
			JSONObject derivedByInsertionFrom = new JSONObject();
			JSONObject derivedByRemovalFrom = new JSONObject();
			JSONObject hadMember = new JSONObject();
			JSONObject hadDictionaryMember = new JSONObject();
			JSONObject specializationOf = new JSONObject();
			JSONObject wasDerivedFrom = new JSONObject();
			JSONObject wasGeneratedBy = new JSONObject();
			JSONObject wasInvalidatedBy = new JSONObject();
			JSONObject wasAttributedTo = new JSONObject();
			JSONObject used = new JSONObject();
			JSONObject wasInformedBy = new JSONObject();
			JSONObject wasStartedBy = new JSONObject();
			JSONObject wasEndedBy = new JSONObject();
			JSONObject hadPlan = new JSONObject();
			JSONObject wasAssociatedWith = new JSONObject();
			JSONObject actedOnBehalfOf = new JSONObject();
			JSONObject wasInfluencedBy = new JSONObject();

			Vector<CPLRelationPropertyEntry> propertiesVec;

			for(CPLRelation relation: relationsVec) {
				JSONObject properties = new JSONObject();
				propertiesVec = relation.getProperties();
				for(CPLRelationPropertyEntry property: propertiesVec) {
					properties.put(property.getKey(), property.getValue());
				}

				int type = relation.getType();

				switch(type) {
					case CPLRelation.ALTERNATEOF:
						alternateOf.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.DERIVEDBYINSERTIONFROM:
						derivedByInsertionFrom.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.DERIVEDBYREMOVALFROM:
						derivedByRemovalFrom.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.HADMEMBER:
						hadMember.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.HADDICTIONARYMEMBER:
						hadDictionaryMember.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.SPECIALIZATIONOF:
						specializationOf.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASDERIVEDFROM:
						wasDerivedFrom.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASGENERATEDBY:
						wasGeneratedBy.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASINVALIDATEDBY:
						wasInvalidatedBy.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASATTRIBUTEDTO:
						wasAttributedTo.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.USED:
						used.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASINFORMEDBY:
						wasInformedBy.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASSTARTEDBY:
						wasStartedBy.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASENDEDBY:
						wasEndedBy.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.HADPLAN:
						hadPlan.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASASSOCIATEDWITH:
						wasAssociatedWith.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.ACTEDONBEHALFOF:
						actedOnBehalfOf.put(relation.getId().toString(), properties);
						break;
					case CPLRelation.WASINFLUENCEDBY:
						wasInfluencedBy.put(relation.getId().toString(), properties);
						break;
					default:
						break;
				}
			}

			if(!alternateOf.isEmpty())
				document.put("alternateOf",alternateOf);
			if(!derivedByInsertionFrom.isEmpty())
				document.put("derivedByInsertionFrom",derivedByInsertionFrom);
			if(!derivedByRemovalFrom.isEmpty())
				document.put("derivedByRemovalFrom",derivedByRemovalFrom);
			if(!hadMember.isEmpty())
				document.put("hadMember",hadMember);
			if(!hadDictionaryMember.isEmpty())
				document.put("hadDictionaryMember",hadDictionaryMember);
			if(!specializationOf.isEmpty())
				document.put("specializationOf",specializationOf);
			if(!wasDerivedFrom.isEmpty())
				document.put("wasDerivedFrom",wasDerivedFrom);
			if(!wasGeneratedBy.isEmpty())
				document.put("wasGeneratedBy",wasGeneratedBy);
			if(!wasInvalidatedBy.isEmpty())
				document.put("wasInvalidatedBy",wasInvalidatedBy);
			if(!wasAttributedTo.isEmpty())
				document.put("wasAttributedTo",wasAttributedTo);
			if(!used.isEmpty())
				document.put("used",used);
			if(!wasInformedBy.isEmpty())
				document.put("wasInformedBy",wasInformedBy);
			if(!wasStartedBy.isEmpty())
				document.put("wasStartedBy",wasStartedBy);
			if(!wasEndedBy.isEmpty())
				document.put("wasEndedBy",wasEndedBy);
			if(!hadPlan.isEmpty())
				document.put("hadPlan",hadPlan);
			if(!wasAssociatedWith.isEmpty())
				document.put("wasAssociatedWith",wasAssociatedWith);
			if(!actedOnBehalfOf.isEmpty())
				document.put("actedOnBehalfOf",actedOnBehalfOf);
			if(!wasInfluencedBy.isEmpty())
				document.put("wasInfluencedBy",wasInfluencedBy);
		}
	}

	/**
	 * Export a Prov bundle as a JSON document
	 *
	 * @param bundle bundle to export
	 * @return a JSONObject containing the bundle as a document
	 */
	public JSONObject exportJson(CPLObject bundle) {

		document = new JSONObject();

		exportBundlePrefixes(bundle);

		exportObjects(bundle);
		
		exportRelations(bundle);

 		return document;
	}
}