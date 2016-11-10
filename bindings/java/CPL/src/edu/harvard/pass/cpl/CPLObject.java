package edu.harvard.pass.cpl;

/*
 * CPLObject.java
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
 * Contributor(s): Jackson OKuhn, Peter Macko
 */


//TODO figure out if cpl_id_t works the way i think it does

import swig.direct.CPLDirect.*;

import java.util.Vector;

import java.math.BigInteger;

/**
 * A provenance object
 *
 * @author Peter Macko
 */
public class CPLObject {

	/// The null object
	private static BigInteger nullId = null;

	/// Traversal direction: Ancestors
	public static final int D_ANCESTORS = CPLDirectConstants.CPL_D_ANCESTORS;

	/// Traversal direction: Descendants
	public static final int D_DESCENDANTS = CPLDirectConstants.CPL_D_DESCENDANTS;

	public static final String ENTITY = CPLDirectConstants.ENTITY;
	public static final String ACTIVITY = CPLDirectConstants.ACTIVITY;
	public static final String AGENT = CPLDirectConstants.AGENT;	
	public static final String BUNDLE = CPLDirectConstants.BUNDLE;	

	/// The internal object ID
	BigInteger id;

	/// The object originator (cache)
	private String originator = null;

	/// The object name (cache)
	private String name = null;

	/// The object type (cache)
	private String type = null;

	/// The object container (cache)
	private BigInteger containerId = null;

	/// Whether we know the container
	private boolean knowContainer = false;

	/// The creation time (cache)
	private long creationTime = 0;

	/// The creation session (cache)
	private CPLSession creationSession = null;

	/// Whether the object creation information is known
	private boolean knowCreationInfo = false;


	/**
	 * Initialize
	 */
	static {
		if (CPL.isInstalled()) {
			nullId = CPLDirect.getCPL_NONE();
		}
	}


	/**
	 * Create an instance of CPLObject from its ID
	 *
	 * @param id the internal CPL object ID
	 */
	public CPLObject(BigInteger id) {
		this.id = id;
	}

	/**
	 * Create a new CPLObject
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @param container the object container
     * @return the new object
	 */
	public static CPLObject create(String originator, String name, String type,
			CPLObject container) {

		BigInteger id = BigInteger.ZERO;
		int r = CPLDirect.cpl_create_object(originator, name, type,
				container == null ? nullId : container.id, id);
		CPLException.assertSuccess(r);

		CPLObject o = new CPLObject(id);
		o.originator = originator;
		o.name = name;
		o.type = type;

		return o;
	}


	/**
	 * Create a new CPLObject
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
     * @return the new object
	 */
	public static CPLObject create(String originator, String name,
            String type) {
		return create(originator, name, type, null);
	}


	/**
	 * Lookup an existing object; return null if not found
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @return the object, or null if not found
	 */
	public static CPLObject tryLookup(String originator, String name,
			String type) {

		BigInteger id = BigInteger.ZERO;
		int r = CPLDirect.cpl_lookup_object(originator, name, type, id);

		if (CPLException.isError(r)) {
			if (r == CPLDirect.CPL_E_NOT_FOUND) return null;
			throw new CPLException(r);
		}

		CPLObject o = new CPLObject(id);
		o.originator = originator;
		o.name = name;
		o.type = type;

		return o;
	}


	/**
	 * Lookup an existing object
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @return the object
	 */
	public static CPLObject lookup(String originator, String name,
			String type) {
		CPLObject o = tryLookup(originator, name, type);
		if (o == null) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return o;
	}


	/**
	 * Lookup an existing object
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @return the collection of objects, or an empty collection if not found
	 */
	public static Vector<CPLObject> tryLookupAll(String originator, String name,
			String type) {

		SWIGTYPE_p_std_vector_cpl_id_timestamp_t pVector
			= CPLDirect.new_std_vector_cpl_id_timestamp_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_lookup_object_ext(originator, name, type,
                    CPLDirect.CPL_L_NO_FAIL,
					CPLDirect.cpl_cb_collect_id_timestamp_vector, pv);
			CPLException.assertSuccess(r);

			cpl_id_timestamp_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_id_timestamp_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cpl_id_timestamp_t e = v.get((int) i);
                BigInteger id = e.getId();

                CPLObject o = new CPLObject(id);
                o.originator = originator;
                o.name = name;
                o.type = type;

				result.add(o);
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_id_timestamp_tp(pVector);
		}

		return result;
	}


	/**
	 * Lookup an existing object
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @return the collection of objects
	 */
	public static Vector<CPLObject> lookupAll(String originator, String name,
			String type) {
		Vector<CPLObject> r = tryLookupAll(originator, name, type);
		if (r.isEmpty()) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return r;
	}


	/**
	 * Lookup an object, or create it if it does not exist
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @param container the object container (if the object does not exist)
	 * @return the object
	 */
	public static CPLObject lookupOrCreate(String originator, String name,
			String type, CPLObject container) {

		BigInteger id = BigInteger.ZERO;
		int r = CPLDirect.cpl_lookup_or_create_object(originator, name, type,
				container == null ? nullId : container.id, id);

		if (CPLException.isError(r)) {
			if (r == CPLDirect.CPL_E_NOT_FOUND) return null;
			throw new CPLException(r);
		}

		CPLObject o = new CPLObject(id);
		o.originator = originator;
		o.name = name;
		o.type = type;

		return o;
	}


	/**
	 * Lookup an object, or create it if it does not exist
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @return the object
	 */
	public static CPLObject lookupOrCreate(String originator, String name,
			String type) {
		return lookupOrCreate(originator, name, type, null);
	}


    /**
     * Get a collection of all provenance objects
     *
     * @return a vector of all provenance objects
     */
    public static Vector<CPLObject> getAllObjects() {

		SWIGTYPE_p_std_vector_cplxx_object_info_t pVector
			= CPLDirect.new_std_vector_cplxx_object_info_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_get_all_objects(CPLDirect.CPL_I_FAST,
					CPLDirect.cpl_cb_collect_object_info_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_object_info_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_object_info_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cplxx_object_info_t e = v.get((int) i);
                BigInteger id = e.getId();

                CPLObject o = new CPLObject(id);
                o.originator = e.getOriginator();
                o.name = e.getName();
                o.type = e.getType();
                
                o.containerId = e.getContainer_id();
                o.knowContainer = true;

				result.add(o);
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_object_info_tp(pVector);
		}

		return result;

    }


	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLObject) {
			CPLObject o = (CPLObject) other;
			return o.id.equals(this.id);
		}
		else {
			return false;
		}
	}


	/**
	 * Compute the hash code of this object
	 *
	 * @return the hash code
	 */
	@Override
	public int hashCode() {
		return id.hashCode();
	}


	/**
	 * Return a string representation of the object. Note that this is based
	 * on the internal object ID, since the name might not be known.
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		return id.toString(16);
	}


	/**
	 * Fetch the object info if it is not already present
	 *
	 * @return true if the info was just fetched, false if we already had it
	 */
	protected boolean fetchInfo() {

		if (originator != null && knowContainer && knowCreationInfo)
			return false;


		// Fetch the info from CPL
		
		SWIGTYPE_p_p_cpl_object_info_t ppInfo
			= CPLDirect.new_cpl_object_info_tpp();

		try {
			int r = CPLDirect.cpl_get_object_info(id,
					CPLDirect.cpl_convert_pp_cpl_object_info_t(ppInfo));
			CPLException.assertSuccess(r);

			cpl_object_info_t info
				= CPLDirect.cpl_dereference_pp_cpl_object_info_t(ppInfo);

			originator = info.getOriginator();
			name = info.getName();
			type = info.getType();

			containerId = info.getContainer_id();

			BigInteger creationSessionId = info.getCreation_session();
			if (CPL.isNone(creationSessionId)) {
				creationSession = null;		// This should never happen!
			}
			else {
				creationSession = new CPLSession(creationSessionId);
			}
			creationTime = info.getCreation_time();

			knowCreationInfo = true;
			knowContainer = true;

			CPLDirect.cpl_free_object_info(info);
		}
		finally {
			CPLDirect.delete_cpl_object_info_tpp(ppInfo);
		}

		return true;
	}


	/**
	 * Get the ID of the object
	 *
	 * @return the internal ID of this object
	 */
	public BigInteger getId() {
		return id;
	}

	/**
	 * Get the object originator
	 *
	 * @return the originator
	 */
	public String getOriginator() {
		if (originator == null) fetchInfo();
		return originator;
	}


	/**
	 * Get the object name
	 *
	 * @return the name
	 */
	public String getName() {
		if (name == null) fetchInfo();
		return name;
	}


	/**
	 * Get the object type
	 *
	 * @return the type
	 */
	public String getType() {
		if (type == null) fetchInfo();
		return type;
	}


	/**
	 * Get the object container
	 *
	 * @return the container, or null if none
	 */
	public BigInteger getContainerId() {
		if (!knowContainer) fetchInfo();
		return containerId;
	}


	/**
	 * Get the session that created this object
	 *
	 * @return the session that created this object
	 */
	public CPLSession getCreationSession() {
		if (!knowCreationInfo) fetchInfo();
		return creationSession;
	}


	/**
	 * Get the creation time of this object
	 *
	 * @return the time expressed as Unix time
	 */
	public long getCreationTime() {
		if (!knowCreationInfo) fetchInfo();
		return creationTime;
	}


	/**
	 * Create a more detailed string representation of the object
	 *
	 * @param detail whether to provide even more detail
	 * @return a multi-line string describing the object
	 */
	public String toString(boolean detail) {

		StringBuilder sb = new StringBuilder();

		sb.append("Originator");
		if (detail) sb.append("          : "); else sb.append(": ");
		sb.append(getOriginator());
		sb.append("\n");

		sb.append("Name      ");
		if (detail) sb.append("          : "); else sb.append(": ");
		sb.append(getName());
		sb.append("\n");

		sb.append("Type      ");
		if (detail) sb.append("          : "); else sb.append(": ");
		sb.append(getType());
		sb.append("\n");

		if (detail) {

			sb.append("Container ID        : ");
			sb.append(getContainerId());
			sb.append("\n");

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
	 * Query the ancestry of the object
	 * @param direction the direction, either D_ANCESTORS or D_DESCENDANTS
	 * @param flags a combination of A_* flags, or 0 for defaults
	 * @return a vector of results &ndash; instances of CPLAncestryEntry
	 * @see CPLAncestryEntry
	 */
	public Vector<CPLRelation> getRelations(int direction,
			int flags) {

		SWIGTYPE_p_std_vector_cpl_relation_t pVector
			= CPLDirect.new_std_vector_cpl_relation_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_relation_t_to_p_void(pVector);
		Vector<CPLRelation> result = null;

		try {
			int r = CPLDirect.cpl_get_object_relations(id, direction,
					flags, CPLDirect.cpl_cb_collect_relation_vector, pv);
			CPLException.assertSuccess(r);

			cpl_relation_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_relation_t(pVector);
			long l = v.size();
			result = new Vector<CPLRelation>((int) l);
			for (long i = 0; i < l; i++) {
				cpl_relation_t e = v.get((int) i);
				result.add(new CPLRelation(
						e.getId(),
						this,
						new CPLObject(e.getOther_object_id()),
						e.getType(),
						new CPLObject(e.getContainer_id()),
						direction == D_ANCESTORS));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_relation_tp(pVector);
		}

		return result;
	}


	/**
	 * Add a property
	 *
	 * @param key the key
	 * @param value the value
	 */
	public void addProperty(String key, String value) {

		int r = CPLDirect.cpl_add_object_property(id, key, value);
		CPLException.assertSuccess(r);
	}


	/**
	 * Lookup an object based on the property value
	 *
	 * @param key the key
	 * @param value the value
	 * @param failOnNotFound whether to fail if no matching objects were found
	 * @return the vector of objects
	 */
	protected static Vector<CPLObject> lookupByProperty(String key,
			String value, boolean failOnNotFound) {

		SWIGTYPE_p_std_vector_cpl_id_t pVector
			= CPLDirect.new_std_vector_cpl_id_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_id_t_to_p_void(pVector);
		Vector<CPLObject> result = null;

		try {
			int r = CPLDirect.cpl_lookup_object_by_property(key, value,
					CPLDirect.cpl_cb_collect_property_lookup_vector, pv);
			if (!failOnNotFound && r == CPLDirectConstants.CPL_E_NOT_FOUND) {
				return new Vector<CPLObject>();
			}
			CPLException.assertSuccess(r);

			cpl_id_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_id_t(pVector);
			long l = v.size();
			result = new Vector<CPLObject>((int) l);
			for (long i = 0; i < l; i++) {
				BigInteger e = v.get((int) i);
				result.add(new CPLObject(e));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_id_tp(pVector);
		}

		return result;
	}


	/**
	 * Get the properties of an object
	 *
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	Vector<CPLObjectPropertyEntry> getProperties(String key) {

		SWIGTYPE_p_std_vector_cplxx_property_entry_t pVector
			= CPLDirect.new_std_vector_cplxx_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(pVector);
		Vector<CPLObjectPropertyEntry> result = null;

		try {
			int r = CPLDirect.cpl_get_object_properties(id, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_property_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLObjectPropertyEntry>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_property_entry_t e = v.get((int) i);
				result.add(new CPLObjectPropertyEntry(this,
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
	 * Get all properties of an object
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLObjectPropertyEntry> getProperties() {
		return getProperties(null);
	}

	/**
	 * Lookup an object based on the property value
	 *
	 * @param key the key
	 * @param value the value
	 * @throws CPLException if no matching object is found
	 */
	public static Vector<CPLObject> lookupByProperty(String key,
			String value) {
		return lookupByProperty(key, value, true);
	}


	/**
	 * Lookup an object based on the property value, but do not fail if no
	 * objects are found
	 *
	 * @param key the key
	 * @param value the value
	 * @return the vector of matching object-version pairs (empty if not found)
	 */
	public static Vector<CPLObject> tryLookupByProperty(String key,
			String value) {
		return lookupByProperty(key, value, false);
	}

	public static void deleteBundle(CPLObject bundle) {

		int r = CPLDirect.cpl_delete_bundle(bundle.getId());
		if (CPLException.isError(r)){
			throw new CPLException(r);
		}
	}

	public static Vector<CPLObject> getBundleObjects(CPLObject bundle) {

		SWIGTYPE_p_std_vector_cplxx_object_info_t pVector
			= CPLDirect.new_std_vector_cplxx_object_info_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_get_bundle_objects(bundle.getId(),
					CPLDirect.cpl_cb_collect_object_info_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_object_info_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_object_info_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cplxx_object_info_t e = v.get((int) i);
                BigInteger obj_id = e.getId();

                CPLObject o = new CPLObject(obj_id);
                o.originator = e.getOriginator();
                o.name = e.getName();
                o.type = e.getType();
                
                o.containerId = bundle.getId();
                o.knowContainer = true;

				result.add(o);
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_object_info_tp(pVector);
		}

		return result;
	}

	public static Vector<CPLRelation> getBundleRelations(CPLObject bundle) {

		SWIGTYPE_p_std_vector_cpl_relation_t pVector
			= CPLDirect.new_std_vector_cpl_relation_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_relation_t_to_p_void(pVector);
		Vector<CPLRelation> result = null;

		try {
			int r = CPLDirect.cpl_get_bundle_relations(bundle.getId(), 
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
						new CPLObject(e.getContainer_id()),
						true));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_relation_tp(pVector);
		}

		return result;
	}

}

