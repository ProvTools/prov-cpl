package edu.harvard.pass.cpl;

/*
 * CPL.java
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
 * The main class
 *
 * @author Peter Macko
 */
public class CPL {

	/// The class instance
	private static CPL cpl = null;


    /**
     * Initialize
     */
    static {
        
        // Load the shared library
        System.loadLibrary("CPLDirect-java");
    }


    /**
     * Create the CPL object and attach
     *
     * @param backend the CPL database backend
     */
    private CPL(SWIGTYPE_p__cpl_db_backend_t backend) {

        // Attach to the CPL

        int r = CPLDirect.cpl_attach(backend);
		CPLException.assertSuccess("Could not attach to the CPL", r);
    }


    /**
     * Destructor
     */
    @Override
    protected void finalize() {

        // Detach from the CPL

        int r = CPLDirect.cpl_detach();
		CPLException.assertSuccess("Could not detach from the CPL", r);
    }


	/**
	 * Attach to the CPL using an ODBC connection
	 *
	 * @param connectionString the ODBC connection string
	 */
	public static synchronized void attachODBC(String connectionString) {

		if (cpl != null) {
			throw new CPLException(CPLDirectConstants.CPL_E_ALREADY_INITIALIZED);
		}

        SWIGTYPE_p_p_cpl_db_backend_t outDb = CPLDirect.new_cpl_db_backend_tpp();
        int r = CPLDirect.cpl_create_odbc_backend(connectionString, 0, outDb);
		CPLException.assertSuccess("Could not open database connection", r);

		cpl = new CPL(CPLDirect.cpl_dereference_pp_cpl_db_backend_t(outDb));
	}


	/**
	 * Attach to the CPL using an RDF/SPARQL connection
	 *
	 * @param queryURL the query URL
	 * @param updateURL the update URL
	 */
	public static synchronized void attachRDF(String queryURL, String updateURL) {

		if (cpl != null) {
			throw new CPLException(CPLDirectConstants.CPL_E_ALREADY_INITIALIZED);
		}

        SWIGTYPE_p_p_cpl_db_backend_t outDb = CPLDirect.new_cpl_db_backend_tpp();
        int r = CPLDirect.cpl_create_rdf_backend(queryURL, updateURL, 0, outDb);
		CPLException.assertSuccess("Could not open database connection", r);

		cpl = new CPL(CPLDirect.cpl_dereference_pp_cpl_db_backend_t(outDb));
	}


	/**
	 * Determine whether the CPL is attached
	 *
	 * @return true if it is already attached
	 */
	public static synchronized boolean isAttached() {
		return cpl != null;
	}
}

