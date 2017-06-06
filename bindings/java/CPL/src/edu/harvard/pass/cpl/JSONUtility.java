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
import org.json.simple.parser.ParseException;
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
	public String validateJson(String fileName){
		
		String errorString = NULL;

		CPLDirect.validate_json(fileName, errorString);

		return errorString;
	}

	/**
	 * Import a Prov JSON document into Prov-CPL as a bundle. 
	 * Does not verify correctness.
	 * Currently supports only one anchor object and no bundles.
	 *
	 * @param fileName document location
	 * @param originator document originator
	 * @param bundleName name of document bundle
	 * @param anchorObject Prov-CPL object identical to an object in the document
	 * @param bundleAgent agent responsible for the document bundle
	 */

	public void importJson(String fileName, String originator, 
			String bundleName, CPLObject anchorObject, CPLObject bundleAgent) {

		CPLDirect.import_document_json(fileName, originator, bundleName,
									   anchorObject.getId(), bundleAgent.getId());
	}

	/**
	 * Export a Prov bundle as a JSON document
	 *
	 * @param bundle bundle to export
	 * @return a JSONObject containing the bundle as a document
	 */
	public void exportBundleJson(CPLObject bundle, String filepath) {

		CPLDirect.export_bundle_json(bundle.getId(), filepath);
	}
}