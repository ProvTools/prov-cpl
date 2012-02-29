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

import swig.direct.CPLDirect.*;


/**
 * CPLDirect test
 *
 * @author Peter Macko
 */
public class test {

    /// The command-line arguments
    protected String[] args;


    /**
     * Initialize
     */
    static {
        
        // Load the shared library
        System.loadLibrary("CPLDirect-java");
    }


    /**
     * Create the object
     *
     * @param args the command-line arguments
     */
    public test(String[] args) {

        this.args = args;


        // Open the database connection

        SWIGTYPE_p_p_cpl_db_backend_t outDb = CPLDirect.new_cpl_db_backend_tpp();
        int r = CPLDirect.cpl_create_odbc_backend("DSN=cpl", 0, outDb);
        //if (r != CPLDirectConstants.CPL_OK) {
        if (CPLDirect.cpl_is_ok(r) == 0) {
            throw new RuntimeException("Cannot initialize the database " +
                    "connection -- " + CPLDirect.cpl_error_string(r));
        }


        // Attach to the CPL

        r = CPLDirect.cpl_attach(CPLDirect.cpl_dereference_pp_cpl_db_backend_t(outDb));
        if (CPLDirect.cpl_is_ok(r) == 0) {
            throw new RuntimeException("Cannot attach to CPL " +
                    "-- " + CPLDirect.cpl_error_string(r));
        }
    }


    /**
     * Destructor
     */
    @Override
    protected void finalize() {

        // Detach from the CPL

        int r = CPLDirect.cpl_detach();
        if (CPLDirect.cpl_is_ok(r) == 0) {
            throw new RuntimeException("Cannot detach from CPL " +
                    "-- " + CPLDirect.cpl_error_string(r));
        }
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
    }
}

