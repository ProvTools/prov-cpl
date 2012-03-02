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

	/// The queried object
	private CPLObject object;

	/// The version of the queried object
	private int version;

	/// The other object
	private CPLObject other;

	/// The version of the other object
	private int otherVersion;

	/// The type of the dependency
	private int type;

	/// The direction of the query
	private boolean otherIsAncestor;


	/**
	 * Create an instance of CPLAncestryEntry
	 *
	 * @param object the queried object
	 * @param version the version of the queried object
	 * @param other the other object
	 * @param otherVersion the version of the other object
	 * @param type the dependency type
	 * @param otherIsAncestor the dependency direction
	 */
	CPLAncestryEntry(CPLObject object, int version, CPLObject other,
			int otherVersion, int type, boolean otherIsAncestor) {

		this.object = object;
		this.version = version;
		this.other = other;
		this.otherVersion = otherVersion;
		this.type = type;
		this.otherIsAncestor = otherIsAncestor;
	}


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
			return object.equals(o.object)
				&& version == o.version
				&& other.equals(o.other)
				&& otherVersion == o.otherVersion
				&& type == o.type
				&& otherIsAncestor == o.otherIsAncestor;
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
		return (object.hashCode() << 16)
			 ^ (version << 12)
			 ^ (other.hashCode() << 8)
			 ^ (otherVersion << 4)
			 ^ (type << 2)
			 ^ (otherIsAncestor ? 1 : 0);
	}


	/**
	 * Return a string representation of the object
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		String arrow = otherIsAncestor ? " --> " : " <-- ";
		return "" + object + "-" + version + arrow
			 + other + "-" + otherVersion
			 + " [type " + Integer.toHexString(type) + "]";
	}


	/**
	 * Get the queried object
	 *
	 * @return the object
	 */
	public CPLObject getObject() {
		return object;
	}


	/**
	 * Get the version of the queried object
	 *
	 * @return the object version
	 */
	public int getVersion() {
		return version;
	}


	/**
	 * Get the other object
	 *
	 * @return the object
	 */
	public CPLObject getOtherObject() {
		return other;
	}


	/**
	 * Get the version of the other object
	 *
	 * @return the object version
	 */
	public int getOtherVersion() {
		return otherVersion;
	}


	/**
	 * Get the ancestor object
	 *
	 * @return the object
	 */
	public CPLObject getAncestorObject() {
		return otherIsAncestor ? other : object;
	}


	/**
	 * Get the version of the ancestor object
	 *
	 * @return the object version
	 */
	public int getAncestorVersion() {
		return otherIsAncestor ? otherVersion : version;
	}


	/**
	 * Get the descendant object
	 *
	 * @return the object
	 */
	public CPLObject getDescendantObject() {
		return !otherIsAncestor ? other : object;
	}


	/**
	 * Get the version of the descendant object
	 *
	 * @return the object version
	 */
	public int getDescendantVersion() {
		return !otherIsAncestor ? otherVersion : version;
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
	 * Is this a data dependency?
	 *
	 * @return true if this is a data dependency
	 */
	public boolean isDataDependency() {
		return (type >> 8) == CPLDirectConstants.CPL_DEPENDENCY_CATEGORY_DATA;
	}


	/**
	 * Is this a control dependency?
	 *
	 * @return true if this is a control dependency
	 */
	public boolean isControlDependency() {
		return (type>>8) == CPLDirectConstants.CPL_DEPENDENCY_CATEGORY_CONTROL;
	}


	/**
	 * Get the direction of the dependency
	 *
	 * @return true if the other object is an ancestor
	 */
	public boolean isOtherAncestor() {
		return otherIsAncestor;
	}
}

