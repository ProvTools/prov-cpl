package edu.harvard.pass.cpl;

/*
 * CPLObject.java
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

import java.io.Serializable;
import java.math.BigInteger;


/**
 * A CPL object or session ID
 *
 * @author Peter Macko
 */
public class CPLId implements Serializable {

	/// The serial version ID
	static final long serialVersionUID = 1L;

	/// The high part of the ID
	private BigInteger hi;

	/// The low part of the ID
	private BigInteger lo;


	/**
	 * Create an instance of CPLId from an ID
	 *
	 * @param id the internal CPL object ID
	 */
	CPLId(cpl_id_t id) {
		hi = id.getHi();
		lo = id.getLo();
	}


	/**
	 * Create an instance of CPLId from a string representation of its ID
	 *
	 * @param s the string representation of the ID
	 */
	public CPLId(String s) {

		// Check that s is valid
		
		if (s.length() != 2 * 64/4) throw new NumberFormatException();
		for (int i = 0; i < s.length(); i++) {
			char c = s.charAt(i);
			if (!(Character.isDigit(c) || (c >= 'a' && c <= 'f')
						|| (c >= 'A' && c <= 'F'))) {
				throw new NumberFormatException();
			}
		}


		// Parse into the high and low components
		
		hi = new BigInteger(s.substring(   0, 64/4), 16);
		lo = new BigInteger(s.substring(64/4      ), 16);
	}


	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLId) {
			CPLId o = (CPLId) other;
			return o.getHi().equals(getHi())
				&& o.getLo().equals(getLo());
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
		return getHi().hashCode() ^ getLo().hashCode();
	}


	/**
	 * Return a string representation of the object
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		String s_hi = getHi().toString(16);
		String s_lo = getLo().toString(16);
		while (s_hi.length() < 64/4) s_hi = "0" + s_hi;
		while (s_lo.length() < 64/4) s_lo = "0" + s_lo;
		return s_hi + s_lo;
	}


	/**
	 * Get the high part of the ID
	 *
	 * @return the high part of the ID
	 */
	public BigInteger getHi() {
		return hi;
	}


	/**
	 * Get the low part of the ID
	 *
	 * @return the low part of the ID
	 */
	public BigInteger getLo() {
		return lo;
	}
}

