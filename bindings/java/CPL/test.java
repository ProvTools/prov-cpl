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
		CPL.attachODBC("DSN=cpl");
    }


    /**
     * The main function
     *
     * @param args the command-line arguments
     */
    public static void main(String[] args) {
        (new test(args)).run();
    }


    /**
     * The real main function
     */
    public void run() {

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

		System.out.print("new CPLObject(\"Process A\", \"Proc\")");
		CPLObject obj1 = new CPLObject(ORIGINATOR, "Process A", "Proc");
		System.out.println(": " + obj1);

		System.out.print("new CPLObject(\"Object A\", \"File\", obj1)");
		CPLObject obj2 = new CPLObject(ORIGINATOR, "Object A", "File", obj1);
		System.out.println(": " + obj2);

		System.out.print("new CPLObject(\"Process B\", \"Proc\", obj1)");
		CPLObject obj3 = new CPLObject(ORIGINATOR, "Process B", "Proc", obj1);
		System.out.println(": " + obj3);

		System.out.print("new CPLObject(\"Object B\", \"File\", null)");
		CPLObject obj4 = new CPLObject(ORIGINATOR, "Object B", "File", null);
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
    }
}

