package edu.harvard.pass.cpl;

/*
 * CPLRelation.java
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
 * A provenance relation
 *
 * @author Jackson Okuhn
 */
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
	public static final int WASINFLUENCEDBY			= CPLDirectConstants.WASINFLUENCEDBY;

	/// The null object
	private static BigInteger nullId = BigInteger.ZERO;

	/// The queried object
	private CPLObject base;

	/// The other object
	private CPLObject other;

	/// The type of the relation
	private int type;

	/// The direction of the query
	private boolean otherIsAncestor;

	/// The internal object ID
	BigInteger id;

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
			int type, boolean otherIsAncestor) {

		this.id = id;
		this.base = base;
		this.other = other;
		this.type = type;
		this.otherIsAncestor = otherIsAncestor;
	}

	CPLRelation(BigInteger id){
		this.id = id;
	}

	// TODO dest == null case
	public static CPLRelation create(CPLObject source, CPLObject dest, int type){

		BigInteger[] id = {nullId};
		int r = CPLDirect.cpl_add_relation(source.getId(), dest.getId(), type, id);
		CPLException.assertSuccess(r);

		CPLRelation a = new CPLRelation(id[0]);
		a.base = source;
		a.other = dest;
		a.type = type;
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
			 + " [type: " + Integer.toHexString(type) + "; id: " + id + "]";
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
	 * Get the ID of the relation
	 *
	 * @return the internal ID of this relation
	 */
	public BigInteger getId() {
		return id;
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
	 * @param the namespace prefix
	 * @param key the key
	 * @param value the value
	 */
	public void addStringProperty(String prefix, String key, String value) {

		int r = CPLDirect.cpl_add_relation_string_property(id, prefix, key, value);
		CPLException.assertSuccess(r);
	}

	/**
	 * Add a property
	 *
	 * @param the namespace prefix
	 * @param key the key
	 * @param value the value
	 */
	public void addNumericalProperty(String prefix, String key, double value) {

		int r = CPLDirect.cpl_add_relation_numerical_property(id, prefix, key, value);
		CPLException.assertSuccess(r);
	}

	/**
	 * Add a property
	 *
	 * @param the namespace prefix
	 * @param key the key
	 * @param value the value
	 */
	public void addBooleanProperty(String prefix, String key, boolean value) {

		int r = CPLDirect.cpl_add_relation_boolean_property(id, prefix, key, value);
		CPLException.assertSuccess(r);
	}

	/**
	 * Get the properties of an ancestry edge
	 *
	 * @param prefix the namespace prefix or null for all entries
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<String>> getStringProperties(String prefix, String key) {
				SWIGTYPE_p_std_vector_cplxx_string_property_entry_t pVector
			= CPLDirect.new_std_vector_cplxx_string_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cplxx_string_property_entry_t_to_p_void(pVector);
		Vector<CPLPropertyEntry<String>> result = null;

		try {
			int r = CPLDirect.cpl_get_relation_string_properties(id, prefix, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_string_property_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cplxx_string_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLPropertyEntry<String>>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_string_property_entry_t e = v.get((int) i);
				result.add(new CPLPropertyEntry<String>(e.getPrefix(),
							e.getKey(),
							e.getValue()));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_string_property_entry_tp(pVector);
		}

		return result;
	}

	/**
	 * Get the properties of an ancestry edge
	 *
	 * @param prefix the namespace prefix or null for all entries
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<Double>> getNumericalProperties(String prefix, String key) {
		SWIGTYPE_p_std_vector_cplxx_numerical_property_entry_t pVector
				= CPLDirect.new_std_vector_cplxx_numerical_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
				.cpl_convert_p_std_vector_cplxx_numerical_property_entry_t_to_p_void(pVector);
		Vector<CPLPropertyEntry<Double>> result = null;

		try {
			int r = CPLDirect.cpl_get_relation_numerical_properties(id, prefix, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_numerical_property_entry_t_vector v = CPLDirect
					.cpl_dereference_p_std_vector_cplxx_numerical_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLPropertyEntry<Double>>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_numerical_property_entry_t e = v.get((int) i);
				result.add(new CPLPropertyEntry<Double>(e.getPrefix(),
						e.getKey(),
						e.getValue()));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_numerical_property_entry_tp(pVector);
		}

		return result;
	}

	/**
	 * Get the properties of an ancestry edge
	 *
	 * @param prefix the namespace prefix or null for all entries
	 * @param key the property name or null for all entries
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<Boolean>> getBooleanProperties(String prefix, String key) {
		SWIGTYPE_p_std_vector_cplxx_boolean_property_entry_t pVector
				= CPLDirect.new_std_vector_cplxx_boolean_property_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
				.cpl_convert_p_std_vector_cplxx_boolean_property_entry_t_to_p_void(pVector);
		Vector<CPLPropertyEntry<Boolean>> result = null;

		try {
			int r = CPLDirect.cpl_get_relation_boolean_properties(id, prefix, key,
					CPLDirect.cpl_cb_collect_properties_vector, pv);
			CPLException.assertSuccess(r);

			cplxx_boolean_property_entry_t_vector v = CPLDirect
					.cpl_dereference_p_std_vector_cplxx_boolean_property_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLPropertyEntry<Boolean>>((int) l);
			for (long i = 0; i < l; i++) {
				cplxx_boolean_property_entry_t e = v.get((int) i);
				result.add(new CPLPropertyEntry<Boolean>(e.getPrefix(),
						e.getKey(),
						e.getValue()));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cplxx_boolean_property_entry_tp(pVector);
		}

		return result;
	}

	/**
	 * Get all properties of a relation
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<String>> getStringProperties() {
		return getStringProperties(null, null);
	}

	/**
	 * Get all properties of a relation
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<Double>> getNumericalProperties() {
		return getNumericalProperties(null, null);
	}

	/**
	 * Get all properties of a relation
	 *
	 * @return the vector of property entries
	 */
	public Vector<CPLPropertyEntry<Boolean>> getBooleanProperties() {
		return getBooleanProperties(null, null);
	}
}

