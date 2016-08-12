package edu.harvard.pass.cpl;

/*
 * CPLPropertyEntry.java
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


/**
 * An entry in the list of object properties
 *
 * @author Peter Macko
 */
public class CPLPropertyEntry {


    /// The provenance object
	private CPLObject object;

	/// The property name
	private String key;

	/// The property value
	private String value;


	/**
	 * Create an instance of CPLPropertyEntry
	 *
	 * @param object the provenance object
     * @param key the property name
     * @param value the property value
	 */
	CPLPropertyEntry(CPLObject object, String key, String value) {
		this.object = object;
        this.key = key;
        this.value = value;
	}

	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLPropertyEntry) {
			CPLPropertyEntry o = (CPLPropertyEntry) other;
			return o.object.equals(object)
                && o.key.equals(key)
                && (o.value == value ? true : o.value.equals(value));
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
        int valueHashCode = value == null ? 0 : value.hashCode();
		return (((object.hashCode() * 31) ^ version) << 4)
            ^ ((key.hashCode() * 31) ^ ~valueHashCode);
	}


	/**
	 * Return a string representation of the object. Note that this is based
	 * on the internal object ID, since the name might not be known.
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		return object.toString() + "-" + key + " = " + value;
	}


	/**
	 * Get the provenance object
	 *
	 * @return the provenance object
	 */
	public CPLObject getObject() {
		return object;
	}

	/**
	 * Get the property name (key)
	 *
	 * @return the key
	 */
	public String getKey() {
		return key;
	}


	/**
	 * Get the property value
	 *
	 * @return the value
	 */
	public String getValue() {
		return value;
	}


	/**
	 * Create a more detailed string representation of the object
	 *
	 * @param includeObject whether to also include the object and version
	 * @return a string describing the entry
	 */
	public String toString(boolean includeObject) {

		return (includeObject ? object.toString() + "- " : "")
            + key + " = " + value;
	}
}

