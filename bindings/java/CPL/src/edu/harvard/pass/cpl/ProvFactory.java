package edu.harvard.pass.cpl;

/*
 * ProvFactory.java
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

/**
 * A factory for creating Prov objects
 *
 * @author Jackson Okuhn
 */
public class ProvFactory{

	public String prefix;
	public CPLBundle bundle;

    /**
     * Create an instance of ProvFactory
     *
     * @param p prefix
     * @param bundleName the bundle name
     */
	public ProvFactory(String p, String bundleName){
		prefix = prefix;
		bundle = CPLBundle.create(bundleName, prefix);
	}

    /**
     * Get the prefix
     * 
     * @return prefix
     */
	public String getPrefix(){
		return prefix;
	}

    /**
     * Set the prefix
     *
     * @param prefix
     */
	public void setPrefix(String o){
		prefix = o;
	}

    /**
     * Get the bundle
     *
     * return bundle
     */
	public CPLBundle getBundle(){
		return bundle;
	}

    /**
     * Set the bundle
     *
     * @param name bundle name
     */
	public void setBundle(String name, String prefix){
		bundle = CPLBundle.create(name, prefix);
	}

    /**
     * Set the bundle
     *
     * @param bun a bundle object
     */
    public void setBundle(CPLBundle bun){
        bundle = bun;
    }
    
    /**
     * Create prov:Entity
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createEntity(String name) {
        return CPLObject.create(prefix, name, CPLObject.ENTITY);
    }

    /**
     * Create prov:Agent
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createAgent(String name) {
        return CPLObject.create(prefix, name, CPLObject.AGENT);
    }

    /**
     * Create prov:Activity
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createActivity(String name) {
        return CPLObject.create(prefix, name, CPLObject.ACTIVITY);
    }

    /**
     * Create prov:EmptyDictionary
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createEmptyDictionary(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "EmptyDictionary");
        return v;
    }

    /**
     * Create prov:Dictionary
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createDictionary(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Dictionary");
        return v;
    }

    /**
     * Create prov:Person
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createPerson(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Person");
        return v;
    }

    /**
     * Create prov:SoftwareAgent
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createSoftwareAgent(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "SoftwareAgent");
        return v;
    }

    /**
     * Create prov:EmptyCollection
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createEmptyCollection(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "EmptyCollection");
        return v;
    }

    /**
     * Create prov:Collection
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createCollection(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Collection");
        return v;
    }

    /**
     * Create prov:Document
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createDocument(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Document");
        return v;
    }

    /**
     * Create prov:Organization
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createOrganization(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Organization");
        return v;
    }

    /**
     * Create prov:Plan
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject createPlan(String name) {
        CPLObject v = CPLObject.create(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Plan");
        return v;
    }

    /**
     * Lookup or create prov:Entity
     *
     * @param name
     * @return CPLObject
     */
   public CPLObject lookupOrCreateEntity(String name) {
        return CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
    }

    /**
     * Lookup or create prov:Agent
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateAgent(String name) {
        return CPLObject.lookupOrCreate(prefix, name, CPLObject.AGENT);
    }

    /**
     * Lookup or create prov:Activity
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateActivity(String name) {
        return CPLObject.lookupOrCreate(prefix, name, CPLObject.ACTIVITY);
    }

    /**
     * Lookup or create prov:EmptyDictionary
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateEmptyDictionary(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "EmptyDictionary");
        return v;
    }

    /**
     * Lookup or create prov:Person
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreatePerson(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Person");
        return v;
    }

    /**
     * Lookup or create prov:SoftwareAgent
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateSoftwareAgent(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "SoftwareAgent");
        return v;
    }

    /**
     * Lookup or create prov:EmptyCollection
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateEmptyCollection(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "EmptyCollection");
        return v;
    }

    /**
     * Lookup or create prov:Dictionary
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateDictionary(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Dictionary");
        return v;
    }

    /**
     * Lookup or create prov:Collection
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateCollection(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Collection");
        return v;
    }

    /**
     * Lookup or create prov:Document
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateDocument(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.ENTITY);
        v.addProperty("prov", "type", "Document");
        return v;
    }

    /**
     * Lookup or create prov:Organization
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreateOrganization(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Organization");
        return v;
    }

    /**
     * Lookup or create prov:plan
     *
     * @param name
     * @return CPLObject
     */
    public CPLObject lookupOrCreatePlan(String name) {
        CPLObject v = CPLObject.lookupOrCreate(prefix, name, CPLObject.AGENT);
        v.addProperty("prov", "type", "Plan");
        return v;
    }
    
    /**
     * Create prov:wasInfluencedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasInfluencedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFLUENCEDBY);
    }
    
    /**
     * Create prov:wasAssociatedWith
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasAssociatedWith(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASASSOCIATEDWITH);
    }

    /**
     * Create prov:wasDerivedFrom with subproperty prov:Revision
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasRevisionOf(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM);
        e.addProperty("prov", "type", "Revision");
        return e;
    }

    /**
     * Create prov:wasDerivedFrom with subproperty prov:PrimarySource
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */  
    public CPLRelation createHadPrimarySource(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM);
        e.addProperty("prov", "type", "PrimarySource");
        return e;
    }

    /**
     * Create prov:wasDerivedFrom with subproperty prov:Quotation
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */  
    public CPLRelation createWasQuotedFrom(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM);
        e.addProperty("prov", "type", "Quotation");
        return e;
    }

    /**
     * Create prov:wasDerivedFrom
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasDerivedFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM);
    }

    /**
     * Create prov:used
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createUsed(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.USED);
    }

    /**
     * Create prov:actedOnBehalfOf
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createActedOnBehalfOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.ACTEDONBEHALFOF);
    }

    /**
     * Create prov:wasInformedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasInformedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY);
    }

    /**
     * Create prov:hadDictionaryMember
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createHadDictionaryMember(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.HADDICTIONARYMEMBER);
    }

    /**
     * Create prov:wasEndedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasEndedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASENDEDBY);
    }

    /**
     * Create prov:hadMember
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createHadMember(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.HADMEMBER);
    }

    /**
     * Create prov:alternateOf
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createAlternateOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.ALTERNATEOF);
    }

    /**
     * Create prov:specializationOf
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createSpecializationOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.SPECIALIZATIONOF);
    }

    /**
     * Create prov:wasAttributedTo
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasAttributedTo(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASATTRIBUTEDTO);
    }

    /**
     * Create prov:mentionOf
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createMentionOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY);
    }

    /**
     * Create prov:wasInvalidatedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasInvalidatedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY);
    }

    /**
     * Create prov:derivedByInsertionFrom
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createDerivedByInsertionFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.DERIVEDBYINSERTIONFROM);
    }

    /**
     * Create prov:derivedByRemovalFrom
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */   
    public CPLRelation createDerivedByRemovalFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.DERIVEDBYREMOVALFROM);
    }

    /**
     * Create prov:wasGeneratedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasGeneratedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASGENERATEDBY);
    }

    /**
     * Create prov:wasStartedBy
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createWasStartedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASSTARTEDBY);
    }

    /**
     * Create prov:hadPlan
     *
     * @param source source CPLObject
     * @param dest destination CPLObject
     * @return CPLRelation
     */    
    public CPLRelation createHadPlan(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.HADPLAN);
    }

}