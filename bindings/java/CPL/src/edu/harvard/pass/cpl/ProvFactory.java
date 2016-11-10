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


public class ProvFactory{

    //TODO lookupOrCreates for objects
	public String originator;
	public CPLObject bundle;

	public ProvFactory(String o){
		originator = o;
		bundle = null;
	}

	public String getOriginator(){
		return originator;
	}

	public void setOriginator(String o){
		originator = o;
	}

	public CPLObject getBundle(){
		return bundle;
	}

	public void SetBundle(String o, String name){
		if (o != null){
			bundle = CPLObject.create(o, name, CPLObject.BUNDLE, null);
		} else {
			bundle = CPLObject.create(originator, name, CPLObject.BUNDLE, null);
		}
	}

    public void SetBundle(String name){
        bundle = CPLObject.create(originator, name, CPLObject.BUNDLE, null);
    }

    public CPLObject createEntity(String name) {
        return CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
    }

    public CPLObject createAgent(String name) {
        return CPLObject.create(originator, name, CPLObject.AGENT, bundle);
    }

    public CPLObject createActivity(String name) {
        return CPLObject.create(originator, name, CPLObject.ACTIVITY, bundle);
    }

    public CPLObject createBundle(String name) {
        return CPLObject.create(originator, name, CPLObject.BUNDLE, bundle);
    }

    public CPLObject createEmptyDictionary(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "EmptyDictionary");
        return v;
    }

    public CPLObject createPerson(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Person");
        return v;
    }

    public CPLObject createSoftwareAgent(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "SoftwareAgent");
        return v;
    }


    public CPLObject createEmptyCollection(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "EmptyCollection");
        return v;
    }

    public CPLObject createCollection(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Collection");
        return v;
    }

    public CPLObject createDocument(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Document");
        return v;
    }

    public CPLObject createOrganization(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Organization");
        return v;
    }

    public CPLObject createDictionary(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Dictionary");
        return v;
    }

    public CPLObject createPlan(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Plan");
        return v;
    }

    public CPLObject createNamedBundle(String name) {
        CPLObject v = CPLObject.create(originator, name, CPLObject.BUNDLE, null);
        return v;
    }

   public CPLObject lookupOrCreateEntity(String name) {
        return CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
    }

    public CPLObject lookupOrCreateAgent(String name) {
        return CPLObject.lookupOrCreate(originator, name, CPLObject.AGENT, bundle);
    }

    public CPLObject lookupOrCreateActivity(String name) {
        return CPLObject.lookupOrCreate(originator, name, CPLObject.ACTIVITY, bundle);
    }

    public CPLObject lookupOrCreateEmptyDictionary(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "EmptyDictionary");
        return v;
    }

    public CPLObject lookupOrCreatePerson(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Person");
        return v;
    }

    public CPLObject lookupOrCreateSoftwareAgent(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "SoftwareAgent");
        return v;
    }


    public CPLObject lookupOrCreateEmptyCollection(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "EmptyCollection");
        return v;
    }

    public CPLObject lookupOrCreateCollection(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Collection");
        return v;
    }

    public CPLObject lookupOrCreateDocument(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Document");
        return v;
    }

    public CPLObject lookupOrCreateOrganization(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Organization");
        return v;
    }

    public CPLObject lookupOrCreateDictionary(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.ENTITY, bundle);
        v.addProperty("prov:type", "Dictionary");
        return v;
    }

    public CPLObject lookupOrCreatePlan(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.AGENT, bundle);
        v.addProperty("prov:type", "Plan");
        return v;
    }

    public CPLObject lookupOrCreateNamedBundle(String name) {
        CPLObject v = CPLObject.lookupOrCreate(originator, name, CPLObject.BUNDLE, null);
        return v;
    }
    
    public CPLRelation createWasInfluencedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFLUENCEDBY);
    }
    

    public CPLRelation createWasAssociatedWith(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASASSOCIATEDWITH, bundle);
    }


    public CPLRelation createRevision(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM, bundle);
        e.addProperty("prov:type", "Revision");
        return e;
    }

    public CPLRelation createPrimarySource(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM, bundle);
        e.addProperty("prov:type", "PrimarySource");
        return e;
    }

    public CPLRelation createQuotation(CPLObject source, CPLObject dest) {
        CPLRelation e = CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM, bundle);
        e.addProperty("prov:type", "Quotation");
        return e;
    }

    public CPLRelation createWasDerivedFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASDERIVEDFROM, bundle);
    }

    public CPLRelation createUsed(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.USED, bundle);
    }

    public CPLRelation createActedOnBehalfOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.ACTEDONBEHALFOF, bundle);
    }

    public CPLRelation createWasInformedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY, bundle);
    }

    public CPLRelation createHadDictionaryMember(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.HADDICTIONARYMEMBER, bundle);
    }

    public CPLRelation createWasEndedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASENDEDBY, bundle);
    }

    public CPLRelation createHadMember(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.HADMEMBER, bundle);
    }

    public CPLRelation createAlternateOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.ALTERNATEOF, bundle);
    }

    public CPLRelation createSpecializationOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.SPECIALIZATIONOF, bundle);
    }

    public CPLRelation createWasAttributedTo(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASATTRIBUTEDTO, bundle);
    }

    public CPLRelation createMentionOf(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY, bundle);
    }

    public CPLRelation createWasInvalidatedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASINFORMEDBY, bundle);
    }

    public CPLRelation createDerivedByInsertionFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.DERIVEDBYINSERTIONFROM, bundle);
    }

    public CPLRelation createDerivedByRemovalFrom(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.DERIVEDBYREMOVALFROM, bundle);
    }

    public CPLRelation createWasGeneratedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASGENERATEDBY, bundle);
    }

    public CPLRelation createWasStartedBy(CPLObject source, CPLObject dest) {
        return CPLRelation.create(source, dest, CPLRelation.WASSTARTEDBY, bundle);
    }

}