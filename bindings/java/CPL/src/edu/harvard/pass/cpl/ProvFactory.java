public class ProvFactory{

	public String originator;
	public CPLObject bundle;

	public ProvFactory(String originator){
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
			bundle = CPLObject.create(o, name, BUNDLE, null);
		} else {
			bundle = CPLObject.create(originator, name, BUNDLE, null);
		}
	}

    public CPLObject createEntity(String name) {
        return CPLObject.create(originator, name, ENTITY, bundle);
    }

    public CPLObject createAgent(String name) {
        return CPLObject.create(originator, name, AGENT, bundle);
    }

    public CPLObject createActivity(String name) {
        return CPLObject.create(originator, name, ACTIVITY, bundle);
    }

    public CPLObject createEmptyDictionary(String name) {
        CPLobject v = CPLObject.create(originator, name, ENTITY, bundle);
        v.addProperty("prov:type", "EmptyDictionary");
        return v;
    }

    public CPLObject createPerson(String name) {
        CPLObject v = CPLObject.create(originator, name, AGENT, bundle);
        v.addProperty("prov:type", "Person");
        return v;
    }

    public CPLObject createSoftwareAgent(String name) {
        CPLObject v = CPLObject.create(originator, name, AGENT, bundle);
        v.addProperty("prov:type", "SoftwareAgent");
        return v;
    }


    public CPLObject createEmptyCollection(String name) {
        CPLObject v = CPLObject.create(originator, name, ENTITY, bundle);
        v.addProperty("prov:type", "EmptyCollection");
        return v;
    }

    public CPLObject createCollection(String name) {
        CPLObject v = CPLObject.create(originator, name, ENTITY, bundle);
        v.addProperty("prov:type", "Collection");
        return v;
    }

    public CPLObject createDocument(String name) {
        CPLObject v = CPLObject.create(originator, name, ENTITY, bundle);
        v.addProperty("prov:type", "Document");
        return v;
    }

    public CPLObject createOrganization(String name) {
        CPLObject v = CPLObject.create(originator, name, AGENT, bundle);
        v.addProperty("prov:type", "Organization");
        return v;
    }

    public CPLObject createDictionary(String name) {
        CPLObject v = CPLObject.create(originator, name, ENTITY, bundle);
        v.addProperty("prov:type", "Dictionary");
        return v;
    }

    public CPLObject createPlan(String name) {
        CPLObject v = CPLObject.create(originator, name, AGENT, bundle);
        v.addProperty("prov:type", "Plan");
        return v;
    }

    public CPLObject createNamedBundle(String name) {
        CPLObject v = CPLObject.create(originator, name, BUNDLE, null);
        return v;
    }

    public CPLAncestryEntry createWasInfluencedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASINFLUENCEDBY);
    }

    public CPLAncestryEntry createWasAssociatedWith(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASASSOCIATEDWITH);
    }


    public CPLAncestryEntry createRevision(CPLObject source, CPLObject dest) {
        CPLAncestryEntry e = CPLAncestryEntry.create(source, dest, WASDERIVEDFROM);
        e.addProperty("prov:type", "Revision");
        return e;
    }

    public CPLAncestryEntry createPrimarySource(CPLObject source, CPLObject dest) {
        CPLAncestryEntry e = CPLAncestryEntry.create(source, dest, WASDERIVEDFROM);
        e.addProperty("prov:type", "PrimarySource");
        return e;
    }

    public CPLAncestryEntry createQuotation(CPLObject source, CPLObject dest) {
        CPLAncestryEntry e = CPLAncestryEntry.create(source, dest, WASDERIVEDFROM);
        e.addProperty("prov:type", "Quotation");
        return e;
    }

    public CPLAncestryEntry createWasDerivedFrom(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASDERIVEDFROM);
    }

    public CPLAncestryEntry createUsed(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, USED);
    }

    public CPLAncestryEntry createActedOnBehalfOf(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, ACTEDONBEHALFOF);
    }

    public CPLAncestryEntry createWasInformedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASINFORMEDBY);
    }

    public CPLAncestryEntry createHadDictionaryMember(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, HADDICTIONARYMEMBER);
    }

    public CPLAncestryEntry createWasEndedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASENDEDBY);
    }

    public CPLAncestryEntry createHadMember(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, HADMEMBER);
    }

    public CPLAncestryEntry createAlternateOf(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, ALTERNATEOF);
    }

    public CPLAncestryEntry createSpecializationOf(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, SPECIALIZATIONOF);
    }

    public CPLAncestryEntry createWasAttributedTo(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASATTRIBUTEDTO);
    }

    public CPLAncestryEntry createMentionOf(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASINFORMEDBY);
    }

    public CPLAncestryEntry createWasInvalidatedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASINFORMEDBY);
    }

    public CPLAncestryEntry createDerivedByInsertionFrom(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, DERIVEDBYINSERTIONFROM);
    }

    public CPLAncestryEntry createDerivedByRemovalFrom(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, DERIVEDBYREMOVALFROM);
    }

    public CPLAncestryEntry createWasGeneratedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASGENERATEDBY);
    }

    public CPLAncestryEntry createWasStartedBy(CPLObject source, CPLObject dest) {
        return CPLAncestryEntry.create(source, dest, WASSTARTEDBY);
    }

}