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

import swig.direct.CPLDirect.*;

/**
 * A utility for processing Prov JSON
 *
 * @author Jackson Okuhn
 */
class JsonUtility {

	/**
	 * Create an instance of JsonUtility
	 */
	public JsonUtility() {}

	/**
	 * Verify the correctness of a Prov JSON document
	 *
	 *@param fileName document path
	 *@return a string detailing errors if any exist or NULL on success
	 */
	public static String validateJson(String fileName){
		
		String stringOutArray[] = { "" };

		int r = CPLDirect.validate_json(fileName, stringOutArray);
		if(r == 0){
			return null;
		}

		return stringOutArray[0];
	}

	/**
	 * Import a Prov JSON document into Prov-CPL as a bundle. 
	 * Does not verify correctness.
	 * Currently supports only one anchor object and no bundles.
	 *
	 * @param fileName document path
	 * @param originator document originator
	 * @param bundleName desired name of document bundle
	 * @param anchorObject Prov-CPL object identical to an object in the document
	 * @param bundleAgent agent responsible for the document bundle
	 */

	public static CPLObject importJson(String fileName, String originator, 
			String bundleName, CPLObject anchorObject, CPLObject bundleAgent) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.import_document_json(fileName, originator, bundleName,
									   anchorObject.getId(), bundleAgent.getId(), id);
		CPLException.assertSuccess(r);
		
		CPLObject o = new CPLObject(id[0]);

		return o;
	}

	/**
	 * Export a Prov bundle as a JSON document
	 *
	 * @param bundle bundle to export
	 * @param filepath path to desired output file, overwrites if file already exists
	 */
	public void exportBundleJson(CPLObject bundle, String filepath) {

		int r = CPLDirect.export_bundle_json(bundle.getId(), filepath);
		CPLException.assertSuccess(r);
	}
}