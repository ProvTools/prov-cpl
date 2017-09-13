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
import java.math.BigInteger;
import java.util.Map;

/**
 * A utility for processing Prov JSON
 *
 * @author Jackson Okuhn
 */
public class CPLJsonUtility {

	private static BigInteger nullId = BigInteger.ZERO;

	/**
	 * Create an instance of JsonUtility
	 */
	public CPLJsonUtility() {}

	/**
	 * Verify the correctness of a Prov JSON document
	 *
	 *@param json the JSON document as a string
	 *@return a string detailing errors if any exist or NULL on success
	 */
	public static String validateJson(String json){

		validate_json_return_t r = CPLDirect.validate_json(json);
		if(CPLException.isSuccess(r.getReturn_code())){
			return null;
		}

		return r.getOut_string();
	}

	/**
	 * Import a Prov JSON document into Prov-CPL as a bundle. 
	 * Does not verify correctness.
	 * Currently supports only one anchor object and no bundles.
	 *
	 * @param json the JSON document as a string
	 * @param bundleName desired name of document bundle
	 * @param anchorObjects map of CPLObject, name pairs matching a stored object to
	 *                      an object name in the document
	 */

	public static CPLObject importJson(String json, 
			String bundleName, Map<CPLObject, String> anchorObjects) {

		cplxx_id_name_pair_vector anchorVector = new cplxx_id_name_pair_vector(anchorObjects.size());

		int pos = 0;
		for(Map.Entry<CPLObject, String> entry : anchorObjects.entrySet()){
			anchorVector.set(pos, new cplxx_id_name_pair(entry.getKey().getId(), entry.getValue()));
			pos++;
		}

		BigInteger[] id = {nullId};
		int r = CPLDirect.import_document_json(json, bundleName,
									   anchorVector, id);
		CPLException.assertSuccess(r);
		
		CPLObject o = new CPLObject(id[0]);

		return o;
	}

	/**
	 * Export a Prov bundle as a JSON document
	 *
	 * @param bundles an array of bundles to export, currently only supports one
	 */
	public static String exportBundleJson(CPLBundle[] bundles) {

		cpl_id_t_vector bundleVector = new cpl_id_t_vector(bundles.length);
		for(int i=0; i<bundles.length; i++){
			bundleVector.set(i, bundles[i].getId());
		}
		export_bundle_json_return_t r = CPLDirect.export_bundle_json(bundleVector);
		CPLException.assertSuccess(r.getReturn_code());

		return r.getOut_string();
	}
}