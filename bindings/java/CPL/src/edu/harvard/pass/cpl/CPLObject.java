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


/**
 * A provenance object
 *
 * @author Peter Macko
 */
public class CPLObject {

	/// The null object
	private static final cpl_id_t nullId = CPLDirect.getCPL_NONE();

	/// The internal object ID
	cpl_id_t id;

	/// The object originator (cache)
	private String originator = null;

	/// The object name (cache)
	private String name = null;

	/// The object type (cache)
	private String type = null;

	/// The object containter (cache)
	private CPLObject container = null;

	/// Whether the container is known
	private boolean knowContainer = false;

	/// The version of the object containter (cache)
	private int containerVersion = -1;

	/// Whether the version of the container is known
	private boolean knowContainerVersion = false;

	/// The creation time (cache)
	private long creationTime = 0;

	/// The creation session (cache)
	private CPLSession creationSession = null;

	/// Whether the object creation information is known
	private boolean knowCreationInfo = false;


	/**
	 * Create an instance of CPLObject from its ID
	 *
	 * @param id the internal CPL object ID
	 */
	CPLObject(cpl_id_t id) {
		this.id = id;
	}


	/**
	 * Create a new CPLObject
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 * @param container the object container
	 */
	public CPLObject(String originator, String name, String type,
			CPLObject container) {

		this.originator = originator;
		this.name = name;
		this.type = type;
		this.container = container;
		this.knowContainer = true;

		this.id = new cpl_id_t();
		int r = CPLDirect.cpl_create_object(originator, name, type,
				container == null ? nullId : container.id, this.id);
		CPLException.assertSuccess(r);
	}


	/**
	 * Create a new CPLObject
	 *
	 * @param originator the originator
	 * @param name the object name
	 * @param type the object type
	 */
	public CPLObject(String originator, String name, String type) {
		this(originator, name, type, null);
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

		cpl_id_t id = new cpl_id_t();
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

		cpl_id_t id = new cpl_id_t();
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

		if (r == CPLDirectConstants.CPL_S_OBJECT_CREATED) {
			o.container = container;
			o.knowContainer = true;
		}

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
	 * Determine whether this and the other object are equal
	 *
	 * @param other the other object
	 * @return true if they are equal
	 */
	@Override
	public boolean equals(Object other) {
		if (other instanceof CPLObject) {
			CPLObject o = (CPLObject) other;
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


	/**
	 * Fetch the object info if it is not already present
	 *
	 * @return true if the info was just fetched, false if we already had it
	 */
	protected boolean fetchInfo() {

		if (originator != null && knowContainer && knowContainerVersion
				&& knowCreationInfo) return false;


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

			cpl_id_t containerId = info.getContainer_id();
			if (CPL.isNone(containerId)) {
				container = null;
				containerVersion = -1;
			}
			else {
				container = new CPLObject(containerId);
				containerVersion = info.getContainer_version();
			}

			cpl_id_t creationSessionId = info.getCreation_session();
			if (CPL.isNone(creationSessionId)) {
				creationSession = null;		// This should never happen!
			}
			else {
				creationSession = new CPLSession(creationSessionId);
			}
			creationTime = info.getCreation_time();

			knowContainer = true;
			knowContainerVersion = true;
			knowCreationInfo = true;

			CPLDirect.cpl_free_object_info(info);
		}
		finally {
			CPLDirect.delete_cpl_object_info_tpp(ppInfo);
		}

		return true;
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
	public CPLObject getContainer() {
		if (!knowContainer) fetchInfo();
		return container;
	}


	/**
	 * Get the version of the container
	 *
	 * @return the version of the container, or -1 if none
	 */
	public int getContainerVersion() {
		if (!knowContainerVersion) fetchInfo();
		return containerVersion;
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
	 * Get the current version of the object
	 *
	 * @return the current version
	 */
	public int getVersion() {

		int v = Integer.MIN_VALUE;
		SWIGTYPE_p_int vp = CPLDirect.new_cpl_version_tp();

		try {
			int r = CPLDirect.cpl_get_version(id, vp);
			CPLException.assertSuccess(r);
			v = CPLDirect.cpl_version_tp_value(vp);
		}
		finally {
			CPLDirect.delete_cpl_version_tp(vp);
		}

		return v;
	}


	/**
	 * Get information about the given version of the object
	 *
	 * @param version the version number
	 * @return the information
	 */
	public CPLVersionInfo getVersionInfo(int version) {
		return new CPLVersionInfo(this, version);
	}


	/**
	 * Get information about the current version of the object
	 *
	 * @return the information
	 */
	public CPLVersionInfo getVersionInfo() {
		return new CPLVersionInfo(this);
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
			CPLVersionInfo v = getVersionInfo();

			sb.append("Current version     : ");
			sb.append(v.getVersion());
			sb.append("\n");

			sb.append("Container ID        : ");
			sb.append(getContainer());
			sb.append("\n");

			sb.append("Container version   : ");
			sb.append(getContainerVersion());
			sb.append("\n");

			sb.append("Creation session    : ");
			sb.append(getCreationSession());
			sb.append("\n");

			sb.append("Creation time       : ");
			sb.append(new java.sql.Date(1000L * getCreationTime()));
			sb.append(" ");
			sb.append(new java.sql.Time(1000L * getCreationTime()));
			sb.append("\n");

			sb.append("Modification session: ");
			sb.append(v.getSession());
			sb.append("\n");

			sb.append("Modification time   : ");
			sb.append(new java.sql.Date(1000L * v.getCreationTime()));
			sb.append(" ");
			sb.append(new java.sql.Time(1000L * v.getCreationTime()));
			sb.append("\n");
		}

		return sb.toString();
	}
}

