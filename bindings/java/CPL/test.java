/*
 * test.java
 * Core Provenance Library
 *
 * Copyright 2012
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
 * Contributor(s): Peter Macko
 */

import edu.harvard.pass.cpl.*;

import java.io.File;
import java.util.Vector;


/**
 * CPL test
 *
 * @author Peter Macko
 */
public class test {

	/// The originator
	private static final String ORIGINATOR = "edu.harvard.pass.cpl.java.test";

    /// The command-line arguments
    protected String[] args;


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

		System.out.print("CPLObject.create(\"Process A\", \"Proc\")");
		CPLObject obj1 = CPLObject.create(ORIGINATOR, "Process A", "Proc");
		System.out.println(": " + obj1);

		System.out.print("CPLObject.create(\"Object A\", \"File\", obj1)");
		CPLObject obj2 = CPLObject.create(ORIGINATOR, "Object A","File",obj1);
		System.out.println(": " + obj2);

		System.out.print("CPLObject.create(\"Process B\", \"Proc\", obj1)");
		CPLObject obj3 = CPLObject.create(ORIGINATOR, "Process B","Proc",obj1);
		System.out.println(": " + obj3);

		System.out.print("CPLObject.create(\"Object B\", \"File\", null)");
		CPLObject obj4 = CPLObject.create(ORIGINATOR, "Object B", "File",null);
		System.out.println(": " + obj4);

		System.out.print("CPLObject.lookupOrCreate(\"Object B\", \"File\")");
		CPLObject obj4t = CPLObject.lookupOrCreate(ORIGINATOR, "Object B", "File");
		System.out.println(": " + obj4t);
		if (!obj4.equals(obj4t))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.lookupOrCreate(\"Object C\", \"File\", obj1)");
		CPLObject obj5 = CPLObject.lookupOrCreate(ORIGINATOR, "Object C", "File", obj1);
		System.out.println(": " + obj5);

		System.out.println();


		/*
		 * Lookup objects
		 */

		System.out.print("CPLObject.lookup(\"Process A\", \"Proc\")");
		CPLObject obj1x = CPLObject.lookup(ORIGINATOR, "Process A", "Proc");
		System.out.println(": " + obj1x);
		if (!obj1.equals(obj1x))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.lookup(\"Object A\", \"File\")");
		CPLObject obj2x = CPLObject.lookup(ORIGINATOR, "Object A", "File");
		System.out.println(": " + obj2x);
		if (!obj2.equals(obj2x))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(\"Process B\", \"Proc\")");
		CPLObject obj3x = CPLObject.tryLookup(ORIGINATOR, "Process B", "Proc");
		System.out.println(": " + obj3x);
		if (!obj3.equals(obj3x))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(\"Object B\", \"File\")");
		CPLObject obj4x = CPLObject.tryLookup(ORIGINATOR, "Object B", "File");
		System.out.println(": " + obj4x);
		if (!obj4.equals(obj4x))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(\"Object C\", \"File\")");
		CPLObject obj5x = CPLObject.tryLookup(ORIGINATOR, "Object C", "File");
		System.out.println(": " + obj5x);
		if (!obj5.equals(obj5x))
			throw new RuntimeException("Object lookup returned the wrong object");

		System.out.print("CPLObject.tryLookup(...should fail...)");
		CPLObject objfx = CPLObject.tryLookup(ORIGINATOR, "%%%%%%", "****");
		if (objfx == null) System.out.println(": OK");
		if (objfx != null)
			throw new RuntimeException("Object lookup did not fail as expected");

		System.out.print("CPLObject.lookupAll(\"Object A\", \"File\")");
		Vector<CPLObject> objv = CPLObject.lookupAll(ORIGINATOR, "Object A", "File");
		System.out.println(": " + (objv.contains(obj2) ? "" : "not ") + "found "
                + "(" + objv.size() + " result" + (objv.size() == 1 ? "" : "s")
                + ")");
		if (!objv.contains(obj2))
			throw new RuntimeException("Object lookup did not return the right object");

        System.out.print("CPLObject.getAllObjects()");
        Vector<CPLObject> objall = CPLObject.getAllObjects();
		System.out.println(": " + objall.size() + " results");
		if (!objall.contains(obj2))
			throw new RuntimeException("getAllObjects() is missing an object");

		System.out.println();


		/*
		 * Check objects created back from their internal IDs
		 */

		System.out.print("new CPLObject(new CPLId(obj1.getId().toString()))");
		obj1x = new CPLObject(new CPLId(obj1.getId().toString()));
		System.out.println(": " + obj1x);
		if (!obj1.equals(obj1x))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(obj2.getId().toString()))");
		obj2x = new CPLObject(new CPLId(obj2.getId().toString()));
		System.out.println(": " + obj2x);
		if (!obj2.equals(obj2x))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(obj3.getId().toString()))");
		obj3x = new CPLObject(new CPLId(obj3.getId().toString()));
		System.out.println(": " + obj3x);
		if (!obj3.equals(obj3x))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(obj4.getId().toString()))");
		obj4x = new CPLObject(new CPLId(obj4.getId().toString()));
		System.out.println(": " + obj4x);
		if (!obj4.equals(obj4x))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.print("new CPLObject(new CPLId(obj5.getId().toString()))");
		obj5x = new CPLObject(new CPLId(obj5.getId().toString()));
		System.out.println(": " + obj5x);
		if (!obj5.equals(obj5x))
			throw new RuntimeException("Object recreation from ID failed");

		System.out.println();


		/*
		 * Data and control dependencies
		 */

		System.out.print("obj2.dataFlowFrom(obj1)");
		boolean r1 = obj2.dataFlowFrom(obj1);
		if (!r1) { System.out.print(" [duplicate ignored]"); }
		System.out.println();

		System.out.print("obj2.dataFlowFrom(obj1, DATA_INPUT)");
		boolean r2 = obj2.dataFlowFrom(obj1, CPLObject.DATA_INPUT);
		if (!r2) { System.out.print(" [duplicate ignored]"); }
		System.out.println();

		System.out.print("obj3.dataFlowFrom(obj2, DATA_INPUT)");
		boolean r3 = obj3.dataFlowFrom(obj2, CPLObject.DATA_INPUT);
		if (!r3) { System.out.print(" [duplicate ignored]"); }
		System.out.println();

		System.out.print("obj3.controlledBy(obj1, CONTROL_START)");
		boolean r4 = obj3.controlledBy(obj1, CPLObject.CONTROL_START);
		if (!r4) { System.out.print(" [duplicate ignored]"); }
		System.out.println();

		System.out.print("obj1.dataFlowFrom(obj3, 0, DATA_TRANSLATION)");
		boolean r5 = obj1.dataFlowFrom(obj3, 0, CPLObject.DATA_TRANSLATION);
		if (!r5) { System.out.print(" [duplicate ignored]"); }
		System.out.println();

		System.out.println();


		/*
		 * Session info
		 */

		System.out.println("Current Session");
		System.out.println(session.toString(true));


		/*
		 * Object infos
		 */

		System.out.println("Object obj1");
		System.out.println(obj1.toString(true));

		System.out.println("Object obj2");
		System.out.println(obj2.toString(true));

		System.out.println("Object obj3 (less detail)");
		System.out.println(obj3.toString(false));


		/*
		 * Query ancestry
		 */

		System.out.print("obj1.getAncestry(ALL_VERSIONS, D_ANCESTORS, 0)");
		Vector<CPLAncestryEntry> anc1a
			= obj1.getAncestry(CPLObject.ALL_VERSIONS,
					CPLObject.D_ANCESTORS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1a) System.out.println(e);
		System.out.println();

		System.out.print("obj1.getAncestry(ALL_VERSIONS, D_DESCENDANTS, 0)");
		Vector<CPLAncestryEntry> anc1d
			= obj1.getAncestry(CPLObject.ALL_VERSIONS,
					CPLObject.D_DESCENDANTS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1d) System.out.println(e);
		System.out.println();

		System.out.print("obj1.getAncestry(0, D_ANCESTORS, 0)");
		Vector<CPLAncestryEntry> anc1v0a
			= obj1.getAncestry(0, CPLObject.D_ANCESTORS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v0a) System.out.println(e);
		System.out.println();

		System.out.print("obj1.getAncestry(0, D_DESCENDANTS, 0)");
		Vector<CPLAncestryEntry> anc1v0d
			= obj1.getAncestry(0, CPLObject.D_DESCENDANTS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v0d) System.out.println(e);
		System.out.println();

		System.out.print("obj1.getAncestry(1, D_ANCESTORS, 0)");
		Vector<CPLAncestryEntry> anc1v1a
			= obj1.getAncestry(1, CPLObject.D_ANCESTORS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v1a) System.out.println(e);
		System.out.println();

		System.out.print("obj1.getAncestry(1, D_DESCENDANTS, 0)");
		Vector<CPLAncestryEntry> anc1v1d
			= obj1.getAncestry(1, CPLObject.D_DESCENDANTS, 0);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v1d) System.out.println(e);
		System.out.println();

		System.out.println("obj1.getAncestry(0, D_DESCENDANTS,");
		System.out.print  ("                       A_NO_DATA_DEPENDENCIES)");
		Vector<CPLAncestryEntry> anc1v0d_1
			= obj1.getAncestry(0, CPLObject.D_DESCENDANTS,
					CPLObject.A_NO_DATA_DEPENDENCIES);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v0d_1) System.out.println(e);
		System.out.println();

		System.out.println("obj1.getAncestry(0, D_DESCENDANTS,");
		System.out.print  ("                       A_NO_CONTROL_DEPENDENCIES)");
		Vector<CPLAncestryEntry> anc1v0d_2
			= obj1.getAncestry(0, CPLObject.D_DESCENDANTS,
					CPLObject.A_NO_CONTROL_DEPENDENCIES);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v0d_2) System.out.println(e);
		System.out.println();

		System.out.println("obj1.getAncestry(0, D_DESCENDANTS,");
		System.out.print  ("  A_NO_DATA_DEPENDENCIES | A_NO_CONTROL_DEPENDENCIES)");
		Vector<CPLAncestryEntry> anc1v0d_3
			= obj1.getAncestry(0, CPLObject.D_DESCENDANTS,
					CPLObject.A_NO_DATA_DEPENDENCIES
					| CPLObject.A_NO_CONTROL_DEPENDENCIES);
		System.out.println(":");
		for (CPLAncestryEntry e : anc1v0d_3) System.out.println(e);
		System.out.println();


		/*
		 * Add properties
		 */

		System.out.print("obj1.addProperty(\"LABEL\", \"Process A [Proc]\")");
		obj1.addProperty("LABEL", "Process A [Proc]");
		System.out.println();

		System.out.print("obj2.addProperty(\"LABEL\", \"Object A [File]\")");
		obj2.addProperty("LABEL", "Object A [File]");
		System.out.println();

		System.out.print("obj3.addProperty(\"LABEL\", \"Process B [Proc]\")");
		obj3.addProperty("LABEL", "Process B [Proc]");
		System.out.println();

		CPLObjectVersion obj3lv = obj3.getCurrentVersion();

		System.out.print("obj3.addProperty(\"LABEL\", \"Yay -- Process B [Proc]\")");
		obj3.addProperty("LABEL", "Yay -- Process B [Proc]");
		System.out.println();

		System.out.print("obj3.addProperty(\"TAG\", \"Hello\")");
		obj3.addProperty("TAG", "Hello");
		System.out.println();

		System.out.println();


		/*
		 * List properties
		 */

		System.out.println("Properties of object 3:");

		System.out.println("obj3.getProperties():");
		for (CPLPropertyEntry e : obj3.getProperties()) {
			System.out.println("  " + e);
		}

		System.out.println("obj3.getSpecificVersion(2).getProperties():");
		for (CPLPropertyEntry e : obj3.getSpecificVersion(2).getProperties()) {
			System.out.println("  " + e);
		}

		System.out.println("obj3.getProperties(\"LABEL\"):");
		for (CPLPropertyEntry e : obj3.getProperties("LABEL")) {
			System.out.println("  " + e);
		}

		System.out.println("obj3lv.getProperties(\"LABEL\"):");
		for (CPLPropertyEntry e : obj3lv.getProperties("LABEL")) {
			System.out.println("  " + e);
		}

		System.out.println("obj3.getProperties(\"HELLO\"):");
		for (CPLPropertyEntry e : obj3.getProperties("HELLO")) {
			System.out.println("  " + e);
		}

		System.out.println("obj3lv.getProperties(\"HELLO\"):");
		for (CPLPropertyEntry e : obj3lv.getProperties("HELLO")) {
			System.out.println("  " + e);
		}

		System.out.println();



		/*
		 * Lookup by property
		 */

		System.out.print("CPLObject.lookupByProperty(\"LABEL\", \"Process B [Proc]\")");
		Vector<CPLObjectVersion> lv = CPLObject.lookupByProperty("LABEL",
				"Process B [Proc]");
		System.out.print(": ");
		if (lv.contains(obj3lv)) {
			System.out.println("found");
		}
		else {
			System.out.println("not found");
			throw new RuntimeException("Lookup by property did not return the correct object");
		}

		System.out.println();


		/*
		 * Create a new verson of an object
		 */

		System.out.println("obj1.getVersion(): " + obj1.getVersion());
		System.out.println("obj1.newVersion(): " + obj1.newVersion());
		System.out.println("obj1.newVersion(): " + obj1.newVersion());
		System.out.println("obj1.newVersion(): " + obj1.newVersion());

		System.out.println();


		/*
		 * File API
		 */

		File f1 = File.createTempFile("cpljtest", ".dat");
		f1.deleteOnExit();

		File f2 = File.createTempFile("cpljtest", ".dat");
		f2.deleteOnExit();

		System.out.println("CPLFile.lookup(\"" + f1.getName() + "\",");
		System.out.print  ("               CREATE_IF_DOES_NOT_EXIST)");
		CPLObject f1o = CPLFile.lookup(f1,
				CPLFile.CreationMode.CREATE_IF_DOES_NOT_EXIST);
		System.out.println(": " + f1o);

		System.out.print("CPLFile.create(\"" + f2.getName() + "\")");
		CPLObject f2o = CPLFile.create(f2);
		System.out.println(": " + f2o);

		System.out.print("CPLFile.lookup(\"" + f1.getName() + "\")");
		CPLObject f1x = CPLFile.lookup(f1);
		System.out.println(": " + f1x);
		if (!f1x.equals(f1o))
			throw new RuntimeException("Object lookup returned the wrong object");
	}
}

