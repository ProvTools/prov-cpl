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
public class CPLBundle {

	/// The null object
	private static BigInteger nullId = BigInteger.ZERO;

	/// The internal bundle ID
	BigInteger id;

	/// The bundle name (cache)
	private String name = null;

	/// The creation time (cache)
	private long creationTime = 0;

	/// The creation session (cache)
	private CPLSession creationSession = null;

	/// Whether the bundle creation information is known
	private boolean knowCreationInfo = false;


	/**
	 * Create an instance of CPLBundle from its ID
	 *
	 * @param id the internal CPL bundle ID
	 */
	public CPLBundle(BigInteger id) {
		this.id = id;
	}

	/**
	 * Create a new CPLBundle
	 *
	 * @param name the bundle name
     * @return the new bundle
	 */
	public static CPLBundle create(String name, String prefix) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_create_bundle(name, prefix, id);
		CPLException.assertSuccess(r);

		CPLBundle o = new CPLBundle(id[0]);
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

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_lookup_bundle(name, prefix,
											id);

		if (CPLException.isError(r)) {
			if (r == CPLDirect.CPL_E_NOT_FOUND) return null;
			throw new CPLException(r);
		}

		CPLBundle o = new CPLBundle(id[0]);
		o.name = name;

		return o;
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

		SWIGTYPE_p_std_vector_cpl_id_timestamp_t pVector
			= CPLDirect.new_std_vector_cpl_id_timestamp_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(pVector);
		Vector<CPLBundle> result = new Vector<CPLBundle>();

		try {
            int r = CPLDirect.cpl_lookup_bundle_ext(prefix, name,
                    CPLDirect.CPL_L_NO_FAIL,
					CPLDirect.cpl_cb_collect_id_timestamp_vector, pv);
			CPLException.assertSuccess(r);

			cpl_id_timestamp_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_id_timestamp_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cpl_id_timestamp_t e = v.get((int) i);
                BigInteger id = e.getId();

                CPLBundle o = new CPLBundle(id);
                o.name = name;

				result.add(o);
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_id_timestamp_tp(pVector);
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
	 * Determine whether this and the other bundle are equal
	 *
	 * @param other the other bundle
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLBundle) {
			CPLBundle o = (CPLBundle) other;
			return this.id.equals(o.id);
		}
		else {
			return false;
		}
	}


	/**
	 * Compute the hash code of this bundle
	 *
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
		return id.hashCode();
	}


	/**
	 * Return a string representation of the bundle. Note that this is based
	 * on the internal bundle ID, since the name might not be known.
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		return id.toString(16);
	}


	/**
	 * Fetch the bundle info if it is not already present
	 *
	 * @return true if the info was just fetched, false if we already had it
	 */
	protected boolean fetchInfo() {

		if (name != null && knowCreationInfo)
			return false;


		// Fetch the info from CPL

		SWIGTYPE_p_p_cpl_bundle_info_t ppInfo
			= CPLDirect.new_cpl_bundle_info_tpp();

		try {
			int r = CPLDirect.cpl_get_bundle_info(id,
					CPLDirect.cpl_convert_pp_cpl_bundle_info_t(ppInfo));
			CPLException.assertSuccess(r);

			cpl_bundle_info_t info
				= CPLDirect.cpl_dereference_pp_cpl_bundle_info_t(ppInfo);

			name = info.getName();

			BigInteger creationSessionId = info.getCreation_session();
			if (CPL.isNone(creationSessionId)) {
				creationSession = null;		// This should never happen!
			}
			else {
				creationSession = new CPLSession(creationSessionId);
			}
			creationTime = info.getCreation_time();

			knowCreationInfo = true;

			CPLDirect.cpl_free_bundle_info(info);
		}
		finally {
			CPLDirect.delete_cpl_bundle_info_tpp(ppInfo);
		}

		return true;
	}


	/**
	 * Get the ID of the bundle
	 *
	 * @return the internal ID of this bundle
	 */
	public BigInteger getId() {
		return id;
	}

	/**
	 * Get the bundle name
	 *
	 * @return the name
	 */
	public String getName() {
		if (name == null) fetchInfo();
		return name;
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
	 * Get the creation time of this bundle
	 *
	 * @return the time expressed as Unix time
	 */
	public long getCreationTime() {
		if (!knowCreationInfo) fetchInfo();
		return creationTime;
	}


	/**
	 * Create a more detailed string representation of the bundle
	 *
	 * @param detail whether to provide even more detail
	 * @return a multi-line string describing the bundle
	 */
	public String toString(boolean detail) {

		StringBuilder sb = new StringBuilder();

		sb.append("Name      ");
		if (detail) sb.append("          : "); else sb.append(": ");
		sb.append(getName());
		sb.append("\n");

		if (detail) {

			sb.append("Creation session    : ");
			sb.append(getCreationSession());
			sb.append("\n");

			sb.append("Creation time       : ");
			sb.append(new java.sql.Date(1000L * getCreationTime()));
			sb.append(" ");
			sb.append(new java.sql.Time(1000L * getCreationTime()));
			sb.append("\n");
		}

		return sb.toString();
	}


	/**
	 * Add a property
	 *
	 * @param prefix the prefix
	 * @param key the key
	 * @param value the value
	 */
	public void addProperty(String prefix, String key, String value) {

		int r = CPLDirect.cpl_add_bundle_property(id, prefix, key, value);
		CPLException.assertSuccess(r);
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
	 * Get the properties of a bundle
	 *
	 * @param prefix the namespace prefix or null for all entries
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry> getProperties(String prefix, String key) {

		SWIGTYPE_p_std_vector_cplxx_property_entry_t pVector
			= CPLDirect.new_std_vector_cplxx_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(pVector);
		Vector<CPLPropertyEntry> result = null;

		try {
			int r = CPLDirect.cpl_get_bundle_properties(id, prefix, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_property_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLPropertyEntry>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_property_entry_t e = v.get((int) i);
				result.add(new CPLPropertyEntry(e.getPrefix(),
							e.getKey(),
							e.getValue()));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_property_entry_tp(pVector);
		}

		return result;
	}


	/**
	 * Get all properties of a bundle
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry> getProperties() {
		return getProperties(null, null);
	}

	/**
	 * Deletes a bundle and all objects and relations belonging to it.
	 */
	public void delete() {

		int r = CPLDirect.cpl_delete_bundle(id);
		if (CPLException.isError(r)){
			throw new CPLException(r);
		}
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
