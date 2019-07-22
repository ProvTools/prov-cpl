/*
 * test.java
 * Core Provenance Library
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
 * Contributor(s): Peter Macko, Jackson Okuhn
 */

import edu.harvard.pass.cpl.*;

import java.io.File;
import java.util.Vector;
import java.math.BigInteger;


/**
 * CPL test
 *
 * @author Jackson Okuhn, Peter Macko
 */
public class test {

    /// The command-line arguments
    protected String[] args;

    private static final String PREFIX = "jtst";
    private static final String IRI = "java.test";
    /**
     * Create the test object
     *
     * @param args the command-line arguments
     */
    public test(String[] args) {
        this.args = args;
		CPL.attachODBC("DSN=CPL");
    }


    /**
     * The main function
     *
     * @param args the command-line arguments
     */
    public static void main(String[] args) throws Exception {
        (new test(args)).run();
    }


    /**
     * The real main function
     */
    public void run() throws Exception {

		System.out.println("CPL ver. " + CPL.VERSION_STR);
		System.out.println();


		/*
		 * Get the current session
		 */

		System.out.print("CPLSession.getCurrentSession()");
		CPLSession session = CPLSession.getCurrentSession();
		System.out.println(": " + session);

		System.out.println();


		/*
		 * Create objects
		 */

		System.out.print("CPLBundle.create(\" Bundle\")");
		CPLBundle bundle = CPLBundle.create("Bundle", PREFIX);
		System.out.println(": " + bundle);

		System.out.println("CPLBundle.addPrefix()");
		bundle.addPrefix(PREFIX, IRI);

		System.out.print("CPLObject.create(\"Entity\", bundle)");
		CPLObject entity = CPLObject.create(PREFIX, "Entity",
										  CPLObject.ENTITY);
		System.out.println(": " + entity);

		System.out.print("CPLObject.create(\"Agent\", bundle)");
		CPLObject agent = CPLObject.create(PREFIX, "Agent",
										  CPLObject.AGENT);
		System.out.println(": " + agent);

		System.out.print("CPLObject.lookupOrCreate(\"Entity\", bundle)");
		CPLObject entityt = CPLObject.lookupOrCreate(PREFIX, "Entity",
													 CPLObject.ENTITY);
		System.out.println(": " + entityt);
		if (!entity.equals(entityt))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.lookupOrCreate(\"Activity\", bundle)");
		CPLObject activity = CPLObject.lookupOrCreate(PREFIX, "Activity",
											  CPLObject.ACTIVITY);
		System.out.println(": " + activity);

		System.out.println();


		/*
		 * Lookup objects
		 */

		System.out.print("CPLBundle.lookup(\"Bundle\")");
		CPLBundle bundlex = CPLBundle.lookup("Bundle", PREFIX);
		System.out.println(": " + bundlex);
		if (!bundle.equals(bundlex))
			throw new RuntimeException("Bundle lookup returned the wrong bundle");

		System.out.print("CPLObject.lookup(\"Entity\")");
		CPLObject entityx = CPLObject.lookup(PREFIX, "Entity", CPLObject.ENTITY);
		System.out.println(": " + entityx);
		if (!entity.equals(entityx))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(\"Agent\")");
		CPLObject agentx = CPLObject.tryLookup(PREFIX, "Agent", CPLObject.AGENT);
		System.out.println(": " + agentx);
		if (!agent.equals(agentx))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(\"Activity\")");
		CPLObject activityx = CPLObject.tryLookup(PREFIX, "Activity", CPLObject.ACTIVITY);
		System.out.println(": " + activityx);
		if (!activity.equals(activityx))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(...should fail...)");
		CPLObject objfx = CPLObject.tryLookup(PREFIX, "%%%%%%", 42);
		if (objfx == null) System.out.println(": OK");
		if (objfx != null)
			throw new RuntimeException("Object lookup did not fail as expected");

		System.out.print("CPLObject.lookupAll(\"Entity\")");
		Vector<CPLObject> entityv = CPLObject.lookupAll(PREFIX, "Entity", CPLObject.ENTITY);
		System.out.println(": " + (entityv.contains(entity) ? "" : "not ") + "found "
                + "(" + entityv.size() + " result" + (entityv.size() == 1 ? "" : "s")
                + ")");
		if (!entityv.contains(entity))
			throw new RuntimeException("Object lookup did not return the right object");

        System.out.print("CPLObject.getAllObjects()");
        Vector<CPLObject> objall = CPLObject.getAllObjects(PREFIX);
		System.out.println(": " + objall.size() + " results");
		if (objall.size() != 1)
			throw new RuntimeException("getAllObjects() has the wrong number of objects");
		if (objall.firstElement().getId().compareTo(bundle.getId()) == 1)
			throw new RuntimeException("getAllObjects() has the wrong objects");

		System.out.println();


		/*
		 * Check objects created back from their internal IDs
		 */

		System.out.print("new CPLBundle(new CPLId(bundle.getId().toString()))");
		bundlex = new CPLBundle(bundle.getId());
		System.out.println(": " + bundlex);
		if (!bundle.equals(bundlex))
			throw new RuntimeException("Bundle recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(entity.getId().toString()))");
		entityx = new CPLObject(entity.getId());
		System.out.println(": " + entityx);
		if (!entity.equals(entityx))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(agent.getId().toString()))");
		agentx = new CPLObject(agent.getId());
		System.out.println(": " + agentx);
		if (!agent.equals(agentx))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(activity.getId().toString()))");
		activityx = new CPLObject(activity.getId());
		System.out.println(": " + activityx);
		if (!activity.equals(activityx))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.println();


		/*
		 * Relation creation
		 */

		System.out.print("CPLRelation.create(entity, agent)");
		CPLRelation r1 = CPLRelation.create(entity, agent,
											CPLRelation.WASATTRIBUTEDTO);
		System.out.println();

		System.out.print("CPLRelation.create(entity, activity)");
		CPLRelation r2 = CPLRelation.create(entity, activity,
									    CPLRelation.WASGENERATEDBY);
		System.out.println();

		System.out.print("CPLRelation.create(activity, agent)");
		CPLRelation r3 = CPLRelation.create(activity, agent,
										CPLRelation.WASASSOCIATEDWITH);
		System.out.println();

		System.out.print("CPLRelation.create(bundle, agent)");
        CPLBundleRelation r4 = CPLBundleRelation.create(bundle, r1);
		System.out.println();

		System.out.print("CPLRelation.create(bundle, agent)");
        CPLBundleRelation r5 = CPLBundleRelation.create(bundle, r2);
		System.out.println();

		System.out.print("CPLRelation.create(bundle, agent)");
        CPLBundleRelation r6 = CPLBundleRelation.create(bundle, r3);
		System.out.println();

		/*
		 * Bundle objects
		 */

		System.out.print("CPLObject.getBundleObjects(bundle)");
		Vector<CPLObject> bovec = bundle.getObjects();
		System.out.println(": " + bovec.size() + " results");
		if(bovec.size() != 3){
			throw new RuntimeException("getBundleObjects() returned an incorrect vector");
		}

		/*
		 * Relation lookup
		 */

		System.out.print("entity.getRelations(D_ANCESTORS)");
		Vector<CPLRelation> rvec = entity.getRelations(CPLObject.D_ANCESTORS, 0);
		System.out.println(": " + rvec.size() + " results");
		if(rvec.size() != 2) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}

		System.out.print("entity.getRelations(D_DESCENDANTS)");
		rvec = entity.getRelations(CPLObject.D_DESCENDANTS, 0);
		System.out.println(": " + rvec.size() + " results");
		if(rvec.size() != 0) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}

		System.out.print("agent.getRelations(D_ANCESTORS)");
		rvec = agent.getRelations(CPLObject.D_ANCESTORS, 0);
		System.out.println(": " + rvec.size() + " results");
		if(rvec.size() != 0) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}

		System.out.print("agent.getRelations(D_DESCENDANTS)");
		rvec = agent.getRelations(CPLObject.D_DESCENDANTS, 0);
		System.out.println(": " + rvec.size() + " results");
		if(rvec.size() != 2) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}

		System.out.print("activity.getRelations(D_ANCESTORS)");
		rvec = activity.getRelations(CPLObject.D_ANCESTORS, 0);
		System.out.println(": " + rvec.size() + " results");
		if(rvec.size() != 1) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}

		System.out.print("activity.getRelations(D_DESCENDANTS)");
		Vector<CPLRelation> rlvec = activity.getRelations(CPLObject.D_DESCENDANTS, 0);
		System.out.println(": " + rlvec.size() + " results");
		if(rlvec.size() != 1) {
			throw new RuntimeException("Relation lookup returned an incorrect vector");
		}
		System.out.println();

		/*
		 * Bundle relations
		 */

		System.out.print("CPLObject.getBundleRelations(bundle)");
		Vector<CPLRelation> brvec = bundle.getRelations();
		System.out.println(": " + bovec.size() + " results");
		if(bovec.size() != 3){
			throw new RuntimeException("getBundleRelations() returned an incorrect vector");
		}
		/*
		 * Session info
		 */

		if(!session.getId().equals(BigInteger.ZERO)){
			System.out.println("Current Session");
			System.out.println(session.toString(true));
		}

		/*
		 * Bundle info
		 */

		System.out.println("Bundle");
		System.out.println(bundle.toString(true));

		/*
		 * Object infos
		 */

		System.out.println("Entity");
		System.out.println(entity.toString(true));

		System.out.println("Agent");
		System.out.println(agent.toString(true));

		System.out.println("Activity (less detail)");
		System.out.println(activity.toString(false));

		/*
		 * Add object properties
		 */

		System.out.print("entity.addProperty(\"LABEL\", \"1\")");
		entity.addProperty(PREFIX, "LABEL", "1");
		System.out.println();

		System.out.print("agent.addProperty(\"LABEL\", \"2\")");
		agent.addProperty(PREFIX, "LABEL", "2");
		System.out.println();

		System.out.print("activity.addProperty(\"LABEL\", \"3\")");
		activity.addProperty(PREFIX, "LABEL", "3");
		System.out.println();

		System.out.print("activity.addProperty(\"TAG\", \"Hello\")");
		activity.addProperty(PREFIX, "TAG", "Hello");
		System.out.println();

		System.out.println();


		/*
		 * List object properties
		 */

		System.out.println("Properties of activity:");

		System.out.println("activity.getProperties():");
		for (CPLPropertyEntry e : activity.getProperties()) {
			System.out.println("  " + e);
		}

		System.out.println("activity.getProperties(\"LABEL\"):");
		for (CPLPropertyEntry e : activity.getProperties(PREFIX, "LABEL")) {
			System.out.println("  " + e);
		}

		System.out.println("activity.getProperties(\"HELLO\"):");
		for (CPLPropertyEntry e : activity.getProperties(PREFIX, "HELLO")) {
			System.out.println("  " + e);
		}

		System.out.println();


		/*
		 * Lookup object by property
		 */

		System.out.print("CPLObject.lookupByProperty(\"LABEL\", \"3\")");
		Vector<CPLObject> lv = CPLObject.lookupByProperty(PREFIX, "LABEL",
				"3");
		System.out.print(": ");
		if (lv.contains(activity)) {
			System.out.println("found");
		}
		else {
			System.out.println("not found");
			throw new RuntimeException("Lookup by property did not return the correct object");
		}

		System.out.println();


		/*
		 * Add relation properties
		 */

		System.out.print("r1.addProperty(\"LABEL\", \"1\")");
		r1.addProperty(PREFIX, "LABEL", "1");
		System.out.println();

		System.out.print("r2.addProperty(\"LABEL\", \"2\")");
		r2.addProperty(PREFIX, "LABEL", "2");
		System.out.println();

		System.out.print("r3.addProperty(\"LABEL\", \"3\")");
		r3.addProperty(PREFIX, "LABEL", "3");
		System.out.println();

		System.out.print("r3.addProperty(\"TAG\", \"Hello\")");
		r3.addProperty(PREFIX, "TAG", "Hello");
		System.out.println();

		System.out.println();

		/*
		 * List relation properties
		 */

		System.out.println("Properties of r3:");

		System.out.println("r3.getProperties():");
		for (CPLPropertyEntry e : r3.getProperties()) {
			System.out.println("  " + e);
		}

		System.out.println("r3.getProperties(\"LABEL\"):");
		for (CPLPropertyEntry e : r3.getProperties(PREFIX, "LABEL")) {
			System.out.println("  " + e);
		}

		System.out.println("r3.getProperties(\"HELLO\"):");
		for (CPLPropertyEntry e : r3.getProperties(PREFIX, "HELLO")) {
			System.out.println("  " + e);
		}

		System.out.println();

		/*
		 * Add bundle properties
		 */


		System.out.print("bundle.addProperty(\"LABEL\", \"3\")");
		bundle.addProperty(PREFIX, "LABEL", "3");
		System.out.println();

		System.out.print("bundle.addProperty(\"TAG\", \"Hello\")");
		bundle.addProperty(PREFIX, "TAG", "Hello");
		System.out.println();

		System.out.println();

		/*
		 * List relation properties
		 */

		System.out.println("Properties of bundle:");

		System.out.println("bundle.getProperties():");
		for (CPLPropertyEntry e : bundle.getProperties()) {
			System.out.println("  " + e);
		}

		System.out.println("bundle.getProperties(\"LABEL\"):");
		for (CPLPropertyEntry e : bundle.getProperties(PREFIX, "LABEL")) {
			System.out.println("  " + e);
		}

		System.out.println("bundle.getProperties(\"HELLO\"):");
		for (CPLPropertyEntry e : bundle.getProperties(PREFIX, "HELLO")) {
			System.out.println("  " + e);
		}

		System.out.println();
		System.out.println("Success");
	}
}

