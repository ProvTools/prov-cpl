package edu.harvard.pass.cpl;

/*
 * CPLObject.java
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
 * A provenance object
 *
 * @author Jackson Okuhn
 */
public class CPLObject {

	/// The null object
	private static BigInteger nullId = BigInteger.ZERO;

	/// Traversal direction: Ancestors
	public static final int D_ANCESTORS = CPLDirectConstants.CPL_D_ANCESTORS;

	/// Traversal direction: Descendants
	public static final int D_DESCENDANTS = CPLDirectConstants.CPL_D_DESCENDANTS;

	public static final int ENTITY = CPLDirectConstants.CPL_ENTITY;
	public static final int ACTIVITY = CPLDirectConstants.CPL_ACTIVITY;
	public static final int AGENT = CPLDirectConstants.CPL_AGENT;	

	/// The internal object ID
	BigInteger id;

	/// The object prefix (cache)
	private String prefix = null;

	/// The object name (cache)
	private String name = null;

	/// The object type (cache)
	private int type = 0;

	/// The object bundle (cache)
	private BigInteger bundleId = null;

	/// Whether we know the bundle
	private boolean knowbundle = false;

	/// The creation time (cache)
	private long creationTime = 0;

	/// Whether the object creation information is known
	private boolean knowCreationInfo = false;


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
	 * @param prefix the namespace prefix
	 * @param name the object name
	 * @param type the object type
	 * @param bundle the object bundle
     * @return the new object
	 */
	public static CPLObject create(String prefix, String name, int type,
			CPLBundle bundle) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_create_object(prefix, name, type,
				bundle == null ? nullId : bundle.getId(), id);
		CPLException.assertSuccess(r);

		CPLObject o = new CPLObject(id[0]);
		o.prefix = prefix;
		o.name = name;
		o.type = type;

		return o;
	}

	/**
	 * Lookup an existing object; return null if not found
	 *
	 * @param prefix the prefix
	 * @param name the object name
	 * @param type the object type, 0 if none
	 * @param bundle the object bundle, null if none
	 * @return the object, or null if not found
	 */
	public static CPLObject tryLookup(String prefix, String name,
			int type, CPLBundle bundle) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_lookup_object(prefix, name, type,
											bundle == null ? nullId : bundle.getId(),
											id);

		if (CPLException.isError(r)) {
			if (r == CPLDirect.CPL_E_NOT_FOUND) return null;
			throw new CPLException(r);
		}

		CPLObject o = new CPLObject(id[0]);
		o.prefix = prefix;
		o.name = name;
		o.type = type;

		return o;
	}

	/**
	 * Lookup an existing object
	 *
	 * @param prefix the prefix
	 * @param name the object name
	 * @param type the object type, 0 if none
	 * @param bundle the object bundle, null if none
	 * @return the object
	 */
	public static CPLObject lookup(String prefix, String name,
			int type, CPLBundle bundle) {
		CPLObject o = tryLookup(prefix, name, type, bundle);
		if (o == null) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return o;
	}


	/**
	 * Lookup an existing object
	 *
	 * @param prefix the prefix
	 * @param name the object name
	 * @param type the object type, 0 if none
	 * @param bundle the object bundle, null if none
	 * @return the collection of objects, or an empty collection if not found
	 */
	public static Vector<CPLObject> tryLookupAll(String prefix, String name,
			int type, CPLBundle bundle) {

		SWIGTYPE_p_std_vector_cpl_id_timestamp_t pVector
			= CPLDirect.new_std_vector_cpl_id_timestamp_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_id_timestamp_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_lookup_object_ext(prefix, name, type,
            		bundle == null ? nullId : bundle.getId(),
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
                o.prefix = prefix;
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
	 * @param prefix the prefix
	 * @param name the object name
	 * @param type the object type, 0 if none
	 * @param bundle the object bundle, null if none
	 * @return the collection of objects
	 */
	public static Vector<CPLObject> lookupAll(String prefix, String name,
			int type, CPLBundle bundle) {
		Vector<CPLObject> r = tryLookupAll(prefix, name, type, bundle);
		if (r.isEmpty()) throw new CPLException(CPLDirect.CPL_E_NOT_FOUND);
		return r;
	}


	/**
	 * Lookup an object, or create it if it does not exist
	 *
	 * @param prefix the prefix
	 * @param name the object name
	 * @param type the object type
	 * @param bundle the object bundle (if the object does not exist)
	 * @return the object
	 */
	public static CPLObject lookupOrCreate(String prefix, String name,
			int type, CPLBundle bundle) {

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_lookup_or_create_object(prefix, name, type,
				bundle == null ? nullId : bundle.getId(), id);

		if (CPLException.isError(r)) {
			if (r == CPLDirect.CPL_E_NOT_FOUND) return null;
			throw new CPLException(r);
		}

		CPLObject o = new CPLObject(id[0]);
		o.prefix = prefix;
		o.name = name;
		o.type = type;

		return o;
	}


    /**
     * Get a collection of all provenance objects
     *
     * @return a vector of all provenance objects
     */
    public static Vector<CPLObject> getAllObjects(String prefix) {

		SWIGTYPE_p_std_vector_cplxx_object_info_t pVector
			= CPLDirect.new_std_vector_cplxx_object_info_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_object_info_t_to_p_void(pVector);
		Vector<CPLObject> result = new Vector<CPLObject>();

		try {
            int r = CPLDirect.cpl_get_all_objects(prefix, CPLDirect.CPL_I_FAST,
					CPLDirect.cpl_cb_collect_object_info_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_object_info_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_object_info_t(pVector);
			long l = v.size();
			for (long i = 0; i < l; i++) {
				cplxx_object_info_t e = v.get((int) i);
                BigInteger id = e.getId();

                CPLObject o = new CPLObject(id);
                o.prefix = e.getPrefix();
                o.name = e.getName();
                o.type = e.getType();
                
                o.knowbundle = true;

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
			return this.id.equals(o.id);
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

		if (prefix != null && knowbundle && knowCreationInfo)
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

			prefix = info.getPrefix();
			name = info.getName();
			type = info.getType();

			creationTime = info.getCreation_time();

			knowCreationInfo = true;
			knowbundle = true;

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
	 * Get the object prefix
	 *
	 * @return the prefix
	 */
	public String getPrefix() {
		if (prefix == null) fetchInfo();
		return prefix;
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
	public int getType() {
		if (type == 0) fetchInfo();
		return type;
	}


	/**
	 * Get the object bundle
	 *
	 * @return the bundle, or null if none
	 */
	public BigInteger getbundleId() {
		if (!knowbundle) fetchInfo();
		return bundleId;
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

		sb.append("Prefix");
		if (detail) sb.append("          : "); else sb.append(": ");
		sb.append(getPrefix());
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

			sb.append("bundle ID        : ");
			sb.append(getbundleId());
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
	 * @param prefix the prefix
	 * @param key the key
	 * @param value the value
	 */
	public void addProperty(String prefix, String key, String value) {

		int r = CPLDirect.cpl_add_object_property(id, prefix, key, value);
		CPLException.assertSuccess(r);
	}


	/**
	 * Lookup an object based on the property value
	 *
	 * @param prefix the prefix
	 * @param key the key
	 * @param value the value
	 * @param failOnNotFound whether to fail if no matching objects were found
	 * @return the vector of objects
	 */
	protected static Vector<CPLObject> lookupByProperty(String prefix, String key,
			String value, boolean failOnNotFound) {

		SWIGTYPE_p_std_vector_cpl_id_t pVector
			= CPLDirect.new_std_vector_cpl_id_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_id_t_to_p_void(pVector);
		Vector<CPLObject> result = null;

		try {
			int r = CPLDirect.cpl_lookup_object_by_property(prefix, key, value,
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
			int r = CPLDirect.cpl_get_object_properties(id, prefix, key,
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
	 * Get all properties of an object
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry> getProperties() {
		return getProperties(null, null);
	}

	/**
	 * Lookup an object based on the property value
	 *
	 * @param prefix the prefix
	 * @param key the key
	 * @param value the value
	 * @throws CPLException if no matching object is found
	 */
	public static Vector<CPLObject> lookupByProperty(String prefix, String key,
			String value) {
		return lookupByProperty(prefix, key, value, true);
	}


	/**
	 * Lookup an object based on the property value, but do not fail if no
	 * objects are found
	 *
	 * @param prefix the prefix
	 * @param key the key
	 * @param value the value
	 * @return the vector of matching objects (empty if not found)
	 */
	public static Vector<CPLObject> tryLookupByProperty(String prefix, String key,
			String value) {
		return lookupByProperty(prefix, key, value, false);
	}
}

