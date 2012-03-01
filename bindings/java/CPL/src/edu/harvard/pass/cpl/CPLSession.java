package edu.harvard.pass.cpl;

/*
 * CPLSession.java
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
 * A session of the provenance-aware application
 *
 * @author Peter Macko
 */
public class CPLSession {

	/// The current session
	private static CPLSession current;

	/// The internal id
	private cpl_id_t id;


	/**
	 * Create an instance of CPLSession from its ID
	 *
	 * @param id the internal CPL session ID
	 */
	private CPLSession(cpl_id_t id) {
		this.id = id;
	}


	/**
	 * Get the current session
	 *
	 * @return the current session
	 */
	public static CPLSession getCurrentSession() {

		// No need to be synchronized, since we would get the same behavior
		// even in the case of a race condition.

		if (current == null) {
			cpl_id_t id = new cpl_id_t();
			int r = CPLDirect.cpl_get_current_session(id);
			CPLException.assertSuccess(r);
			current = new CPLSession(id);
		}

		return current;
	}


	/**
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLSession) {
			CPLSession o = (CPLSession) other;
			return o.id.getHi().equals(this.id.getHi())
				&& o.id.getLo().equals(this.id.getLo());
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
		return id.getHi().hashCode() ^ id.getLo().hashCode();
	}


	/**
	 * Return a string representation of the object. Note that this is based
	 * on the internal object ID, since the name might not be known.
	 *
	 * @return the string representation
	 */
	@Override
	public String toString() {
		return id.getHi().toString(16) + ":" + id.getLo().toString(16);
	}
}

