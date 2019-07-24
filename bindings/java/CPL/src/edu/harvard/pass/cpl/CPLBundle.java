package edu.harvard.pass.cpl;

/*
 * CPLBundle.java
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
 * Contributor(s): Jackson Okuhn, Peter Macko
 */

import swig.direct.CPLDirect.*;

import java.util.Vector;

import java.math.BigInteger;

/**
 * A provenance bundle
 *
 * @author Jackson Okuhn
 */
public class CPLBundle extends CPLObject {

	/// The creation session (cache)
	private CPLSession creationSession = null;

	/**
	 * Create an instance of CPLBundle from its ID
	 *
	 * @param id the internal CPL bundle ID
	 */
	public CPLBundle(BigInteger id) {
		super(id);
		this.type = CPLDirect.CPL_BUNDLE;
	}

	/**
	 * Create a new CPLBundle
	 *
	 * @param name the bundle name
     * @return the new bundle
	 */
	public static CPLBundle create(String name, String prefix) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_create_object(prefix, name, CPLDirect.CPL_BUNDLE, id);
		CPLException.assertSuccess(r);

		CPLBundle o = new CPLBundle(id[0]);
		o.prefix = prefix;
		o.name = name;

		return o;
	}

	/**
	 * Lookup an existing bundle; return null if not found
	 *
	 * @param name the bundle name
	 * @return the bundles, or null if not found
	 */
	public static CPLBundle tryLookup(String name, String prefix) {
		BigInteger[] id = new BigInteger[]{nullId};
		int r = CPLDirect.cpl_lookup_object(prefix, name, CPLDirect.CPL_BUNDLE, id);
		if (CPLException.isError(r)) {
			if (r == -11) {
				return null;
			} else {
				throw new CPLException(r);
			}
		} else {
			CPLBundle o = new CPLBundle(id[0]);
			o.prefix = prefix;
			o.name = name;
			return o;
		}
	}

	/**
	 * Lookup an existing bundle
	 *
	 * @param name the bundle name
	 * @param prefix the prefix for the bundle
	 * @return the bundle
	 */
	public static CPLBundle lookup(String name, String prefix) {
		CPLBundle o = tryLookup(name, prefix);
		if (o == null) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return o;
	}


	/**
	 * Lookup an existing bundle
	 *
	 * @param name the bundle name
	 * @return the collection of bundle, or an empty collection if not found
	 */
	public static Vector<CPLBundle> tryLookupAll(String name, String prefix) {
		Vector<CPLObject> objs =  CPLObject.tryLookupAll(name, prefix, CPLDirect.CPL_BUNDLE);
		Vector<CPLBundle> result = new Vector<>();
		for (CPLObject o : objs) {
			CPLBundle b = new CPLBundle(o.id);
			b.name = o.name;
			b.prefix = o.name;
			result.add(b);
		}
		return result;
	}


	/**
	 * Lookup an existing bundle
	 *
	 * @param name the bundle name
	 * @return the collection of bundle
	 */
	public static Vector<CPLBundle> lookupAll(String name, String prefix) {
		Vector<CPLBundle> r = tryLookupAll(name, prefix);
		if (r.isEmpty()) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return r;
	}

	/**
	 * Get the session that created this bundle
	 *
	 * @return the session that created this bundle
	 */
	public CPLSession getCreationSession() {
		if (!knowCreationInfo) fetchInfo();
		return creationSession;
	}

	/**
	 * Add a prefix
	 *
	 * @param prefix the prefix
	 * @param iri the namespace iri
	 */
	public void addPrefix(String prefix, String iri) {

		int r = CPLDirect.cpl_add_prefix(id, prefix, iri);
		CPLException.assertSuccess(r);
	}


	/**
	 * Get all objects belonging to a bundle
	 *
	 * @return the vector of matching objects (empty if not found)
	 */
	public Vector<CPLObject> getObjects() {

		SWIGTYPE_p_std_vector_cplxx_object_info_t pVector
			= CPLDirect.new_std_vector_cplxx_object_info_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_get_bundle_objects(id,
					CPLDirect.cpl_cb_collect_object_info_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_object_info_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_object_info_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cplxx_object_info_t e = v.get((int) i);
                BigInteger obj_id = e.getId();

                CPLObject o = new CPLObject(obj_id);

				result.add(o);
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_object_info_tp(pVector);
		}

		return result;
	}

	/**
	 * Get all relations belonging to a bundle
	 *
	 * @return the vector of matching relations (empty if not found)
	 */
	public Vector<CPLRelation> getRelations() {

		SWIGTYPE_p_std_vector_cpl_relation_t pVector
			= CPLDirect.new_std_vector_cpl_relation_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_relation_t_to_p_void(pVector);
		Vector<CPLRelation> result = null;

		try {
			int r = CPLDirect.cpl_get_bundle_relations(id, 
				CPLDirect.cpl_cb_collect_relation_vector, pv);
			CPLException.assertSuccess(r);

			cpl_relation_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_relation_t(pVector);
			long l = v.size();
			result = new Vector<CPLRelation>((int) l);
			for (long i = 0; i < l; i++) {
				cpl_relation_t e = v.get((int) i);
				result.add(new CPLRelation(
						e.getId(),
						new CPLObject(e.getQuery_object_id()),
						new CPLObject(e.getOther_object_id()),
						e.getType(),
						true));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_relation_tp(pVector);
		}

		return result;
	}
}
