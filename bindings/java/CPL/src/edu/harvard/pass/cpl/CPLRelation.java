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

import java.util.Vector;

import java.math.BigInteger;

/**
 * An entry in the results of a simple ancestry query
 *
 * @author Peter Macko
 */

//TODO change to relation
public class CPLRelation {

	public static final int ALTERNATEOF				= CPLDirectConstants.ALTERNATEOF;
	public static final int DERIVEDBYINSERTIONFROM	= CPLDirectConstants.DERIVEDBYINSERTIONFROM;
	public static final int DERIVEDBYREMOVALFROM	= CPLDirectConstants.DERIVEDBYREMOVALFROM;
	public static final int HADMEMBER 				= CPLDirectConstants.HADMEMBER;
	public static final int HADDICTIONARYMEMBER		= CPLDirectConstants.HADDICTIONARYMEMBER;
	public static final int SPECIALIZATIONOF		= CPLDirectConstants.SPECIALIZATIONOF;
	public static final int WASDERIVEDFROM			= CPLDirectConstants.WASDERIVEDFROM;
	public static final int WASGENERATEDBY			= CPLDirectConstants.WASGENERATEDBY;
	public static final int WASINVALIDATEDBY		= CPLDirectConstants.WASINVALIDATEDBY;
	public static final int WASATTRIBUTEDTO			= CPLDirectConstants.WASATTRIBUTEDTO;
	public static final int USED 					= CPLDirectConstants.USED;
	public static final int WASINFORMEDBY			= CPLDirectConstants.WASINFORMEDBY;
	public static final int WASSTARTEDBY			= CPLDirectConstants.WASSTARTEDBY;
	public static final int WASENDEDBY				= CPLDirectConstants.WASENDEDBY;
	public static final int HADPLAN					= CPLDirectConstants.HADPLAN;
	public static final int WASASSOCIATEDWITH		= CPLDirectConstants.WASASSOCIATEDWITH;
	public static final int ACTEDONBEHALFOF			= CPLDirectConstants.ACTEDONBEHALFOF;
	/*
	public static final int WASINFLUENCEDBY			= CPLDirectConstants.WASINFLUENCEDBY;
	*/
	/// The null object
	private static BigInteger nullId = null;

	/// The queried object
	private CPLObject base;

	/// The other object
	private CPLObject other;

	/// The type of the relation
	private int type;

	/// The container/bundle
	private CPLObject container;

	/// The direction of the query
	private boolean otherIsAncestor;

	/// The internal object ID
	BigInteger id;

	static {
		if (CPL.isInstalled()) {
			nullId = CPLDirect.getCPL_NONE();
		}
	}

	/**
	 * Create an instance of CPLRelation
	 *
	 * @param id the dependency's id
	 * @param base the queried (base) object
	 * @param other the other object
	 * @param type the dependency type
	 * @param otherIsAncestor the dependency direction
	 */
	CPLRelation(BigInteger id, CPLObject base, CPLObject other,
			int type, CPLObject container, boolean otherIsAncestor) {

		this.id = id;
		this.base = base;
		this.other = other;
		this.type = type;
		this.container = container;
		this.otherIsAncestor = otherIsAncestor;
	}

	CPLRelation(BigInteger id){
		this.id = id;
	}

	// TODO double check this isn't borked
	public static CPLRelation create(CPLObject source, CPLObject dest,  int type, CPLObject container){

		BigInteger id = BigInteger.ZERO;
		int r = CPLDirect.cpl_add_relation(source.getId(), dest.getId(), type, container.getId(), id);
		CPLException.assertSuccess(r);

		CPLRelation a = new CPLRelation(id);
		a.base = source;
		a.other = dest;
		a.type = type;
		a.container = container;
		a.otherIsAncestor = false;

		return a;
	}

	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLRelation) {
			CPLRelation o = (CPLRelation) other;
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
		String id = this.id.toString(16);
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
	 * Get the type of the relation
	 *
	 * @return the relation type
	 */
	public int getType() {
		return type;
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
	 * Get the container of the relation
	 *
	 * @return the container
	 */
	public CPLObject getContainer() {
		return container;
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

		int r = CPLDirect.cpl_add_relation_property(id, key, value);
		CPLException.assertSuccess(r);
	}


	/**
	 * Get the properties of an ancestry edge
	 *
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	Vector<CPLRelationPropertyEntry> getProperties(String key) {
				SWIGTYPE_p_std_vector_cplxx_property_entry_t pVector
			= CPLDirect.new_std_vector_cplxx_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_property_entry_t_to_p_void(pVector);
		Vector<CPLRelationPropertyEntry> result = null;

		try {
			int r = CPLDirect.cpl_get_relation_properties(id, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_property_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLRelationPropertyEntry>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_property_entry_t e = v.get((int) i);
				result.add(new CPLRelationPropertyEntry(this,
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
	public Vector<CPLRelationPropertyEntry> getProperties() {
		return getProperties(null);
	}
}

