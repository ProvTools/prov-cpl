package edu.harvard.pass.cpl;

/*
 * CPLAncestryEntry.java
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
 * An entry in the results of a simple ancestry query
 *
 * @author Peter Macko
 */
public class CPLAncestryEntry {

	/// The null object
	private static cpl_id_t nullId = null;

	/// The queried object
	private CPLObject base;

	/// The other object
	private CPLObject other;

	/// The type of the dependency
	private int type;

	/// The direction of the query
	private boolean otherIsAncestor;

	/// The internal object ID
	cpl_id_t id;

	static {
		if (CPL.isInstalled()) {
			nullId = CPLDirect.getCPL_NONE();
		}
	}

	/**
	 * Create an instance of CPLAncestryEntry
	 *
	 * @param id the dependency's id
	 * @param base the queried (base) object
	 * @param other the other object
	 * @param type the dependency type
	 * @param otherIsAncestor the dependency direction
	 */
	CPLAncestryEntry(cpl_id_t id, CPLObject base, CPLObject other,
			int type, boolean otherIsAncestor) {

		this.id = id;
		CPLDirect.cpl_id_copy(this.id, id);
		this.base = base;
		this.other = other;
		this.type = type;
		this.otherIsAncestor = otherIsAncestor;
	}

	CPLAncestryEntry(cpl_id_t id){
		this.id = id;
	}

	// TODO double check this isn't borked
	public CPLAncestryEntry create(CPLObject source, CPLObject dest, int type){

		cpl_id_t id = new cpl_id_t();
		int r = CPLDirect.cpl_add_dependency(source.get_object().id, dest.get_object().id, type, id);
		CPLException.assertSuccess(r);

		CPLAncestryEntry a = new CPLAncestryEntry(id);
		a.base = source;
		a.other = dest;
		a.type = type;
		a.otherIsAncestor = false;

		return a;
	}

	public
	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLAncestryEntry) {
			CPLAncestryEntry o = (CPLAncestryEntry) other;
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
	 * Return a string representation of the object
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		String id = id.toString(16);
		String arrow = otherIsAncestor ? " --> " : " <-- ";
		return "" + base + arrow + other
			 + " [type " + Integer.toHexString(type) + "; id " + id + "]";
	}


	/**
	 * Get the queried (base) object
	 *
	 * @return the object
	 */
	public CPLObject getBase() {
		return base;
	}


	/**
	 * Get the other object
	 *
	 * @return the object
	 */
	public CPLObject getOther() {
		return other;
	}


	/**
	 * Get the ancestor object
	 *
	 * @return the object
	 */
	public CPLObject getAncestor() {
		return otherIsAncestor ? other : base;
	}


	/**
	 * Get the descendant object
	 *
	 * @return the object
	 */
	public CPLObject getDescendant() {
		return !otherIsAncestor ? other : base;
	}


	/**
	 * Get the type of the ancestry
	 *
	 * @return the ancestry type
	 */
	public int getType() {
		return type;
	}


	/**
	 * Get the direction of the dependency
	 *
	 * @return true if the other object is an ancestor
	 */
	public boolean isOtherAncestor() {
		return otherIsAncestor;
	}

		/**
	 * Add a property
	 *
	 * @param key the key
	 * @param value the value
	 */
	public void addProperty(String key, String value) {

		int r = CPLDirect.cpl_add_ancestry_property(id, key, value);
		CPLException.assertSuccess(r);
	}


	/**
	 * Get the properties of an ancestry edge
	 *
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	Vector<CPLPropertyEntry> getProperties(String key) {
				SWIGTYPE_p_std_vector_cplxx_property_entry_t pVector
			= CPLDirect.new_std_vector_cplxx_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(pVector);
		Vector<CPLPropertyEntry> result = null;

		try {
			int r = CPLDirect.cpl_get_ancestry_properties(id, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_property_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLPropertyEntry>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_property_entry_t e = v.get((int) i);
				result.add(new CPLPropertyEntry(this,
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
		return getProperties(null);
	}


	/**
	 * Lookup an object based on the property value
	 *
	 * @param key the key
	 * @param value the value
	 * @return the vector of edges
	 * @throws CPLException if no matching object is found
	 */
	public static Vector<CPLAncestryEntry> lookupByProperty(String key,
			String value) {
		return lookupByProperty(key, value, true);
	}


	/**
	 * Lookup an object based on the property value, but do not fail if no
	 * objects are found
	 *
	 * @param key the key
	 * @param value the value
	 * @return the vector of edges
	 */
	public static Vector<CPLAncestryEntry> tryLookupByProperty(String key,
			String value) {
		return lookupByProperty(key, value, false);
	}
}

