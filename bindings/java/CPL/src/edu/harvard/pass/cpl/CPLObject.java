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

import java.util.Vector;


/**
 * A provenance object
 *
 * @author Peter Macko
 */
public class CPLObject {

	/// The null object
	private static final cpl_id_t nullId = CPLDirect.getCPL_NONE();

	/// Data flow type: Generic
	public static final int DATA_INPUT = CPLDirectConstants.CPL_DATA_INPUT;

	/// Data flow type: IPC
	public static final int DATA_IPC = CPLDirectConstants.CPL_DATA_IPC;

	/// Data flow type: Translation
	public static final int DATA_TRANSLATION
		= CPLDirectConstants.CPL_DATA_TRANSLATION;

	/// Data flow type: Copy
	public static final int DATA_COPY = CPLDirectConstants.CPL_DATA_COPY;

	/// Control flow type: Generic
	public static final int CONTROL_OP = CPLDirectConstants.CPL_CONTROL_OP;

	/// Control flow type: Start
	public static final int CONTROL_START=CPLDirectConstants.CPL_CONTROL_START;

	/// Traversal direction: Ancestors
	public static final int D_ANCESTORS = CPLDirectConstants.CPL_D_ANCESTORS;

	/// Traversal direction: Descendants
	public static final int D_DESCENDANTS=CPLDirectConstants.CPL_D_DESCENDANTS;

	/// Traversal option: No data dependencies
	public static final int A_NO_DATA_DEPENDENCIES
		= CPLDirectConstants.CPL_A_NO_DATA_DEPENDENCIES;

	/// Traversal option: No control dependencies
	public static final int A_NO_CONTROL_DEPENDENCIES
		= CPLDirectConstants.CPL_A_NO_CONTROL_DEPENDENCIES;

	/// Specify all versions where supported
	public static final int ALL_VERSIONS = -1;

	/// The internal object ID
	cpl_id_t id;

	/// The object originator (cache)
	private String originator = null;

	/// The object name (cache)
	private String name = null;

	/// The object type (cache)
	private String type = null;

	/// The object containter (cache)
	private CPLObjectVersion container = null;

	/// Whether we know the container
	private boolean knowContainer = false;

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
		this.id = CPLDirect.new_cpl_id_tp();
		CPLDirect.cpl_id_copy(this.id, id);
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
		this.knowContainer = container == null;

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

			cpl_id_t containerId = info.getContainer_id();
			if (CPL.isNone(containerId)) {
				container = null;
			}
			else {
				container = new CPLObjectVersion(new CPLObject(containerId),
						info.getContainer_version());
			}

			cpl_id_t creationSessionId = info.getCreation_session();
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
	public CPLObjectVersion getContainer() {
		if (!knowContainer) fetchInfo();
		return container;
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
	 * Get a specific version of the object
	 *
	 * @param version the version number
	 * @return the specific version of the object
	 */
	public CPLObjectVersion getSpecificVersion(int version) {
		return new CPLObjectVersion(this, version);
	}


	/**
	 * Get the latest version of an object
	 *
	 * @return the latest version of the object
	 */
	public CPLObjectVersion getLatestVersion() {
		return new CPLObjectVersion(this, getVersion());
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
			CPLObjectVersion v = getLatestVersion();

			sb.append("Current version     : ");
			sb.append(v.getVersion());
			sb.append("\n");

			sb.append("Container ID        : ");
			sb.append(getContainer());
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


	/**
	 * Disclose data flow
	 *
	 * @param source the data source object
	 * @param source_version the version of the source object
	 * @param type the data flow type (one of CPLObject.DATA_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean dataFlowFrom(CPLObject source, int source_version, int type){
		int r = CPLDirect.cpl_data_flow_ext(id, source.id, source_version,type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose data flow
	 *
	 * @param source the data source object and version
	 * @param type the data flow type (one of CPLObject.DATA_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean dataFlowFrom(CPLObjectVersion source, int type){
		int r = CPLDirect.cpl_data_flow_ext(id, source.getObject().id,
				source.getVersion(), type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose data flow
	 *
	 * @param source the data source object and version
	 * @param type the data flow type (one of CPLObject.DATA_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean dataFlowFrom(CPLObjectVersion source){
		int r = CPLDirect.cpl_data_flow_ext(id, source.getObject().id,
				source.getVersion(), DATA_INPUT);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose data flow
	 *
	 * @param source the data source object
	 * @param type the data flow type (one of CPLObject.DATA_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean dataFlowFrom(CPLObject source, int type) {
		int r = CPLDirect.cpl_data_flow(id, source.id, type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose data flow
	 *
	 * @param source the data source object
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean dataFlowFrom(CPLObject source) {
		int r = CPLDirect.cpl_data_flow(id, source.id, DATA_INPUT);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose control flow
	 *
	 * @param controller the controller object
	 * @param controller_version the version of the controller object
	 * @param type the data flow type (one of CPLObject.CONTROL_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean controlledBy(CPLObject controller, int controller_version,
			int type) {
		int r = CPLDirect.cpl_control_ext(id, controller.id, controller_version,
				type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose control flow
	 *
	 * @param controller the controller object and version
	 * @param type the data flow type (one of CPLObject.CONTROL_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean controlledBy(CPLObjectVersion controller, int type) {
		int r = CPLDirect.cpl_control_ext(id, controller.getObject().id,
				controller.getVersion(), type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose control flow
	 *
	 * @param controller the controller object and version
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean controlledBy(CPLObjectVersion controller) {
		int r = CPLDirect.cpl_control_ext(id, controller.getObject().id,
				controller.getVersion(), CONTROL_OP);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose control flow
	 *
	 * @param controller the controller object
	 * @param type the data flow type (one of CPLObject.CONTROL_*)
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean controlledBy(CPLObject controller, int type) {
		int r = CPLDirect.cpl_control(id, controller.id, type);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Disclose control flow
	 *
	 * @param controller the controller object
	 * @return true if the dependency was added, false if it is a duplicate
	 */
	public boolean controlledBy(CPLObject controller) {
		int r = CPLDirect.cpl_control(id, controller.id, CONTROL_OP);
		CPLException.assertSuccess(r);
		return r != CPLDirectConstants.CPL_S_DUPLICATE_IGNORED;
	}


	/**
	 * Query the ancestry of the object
	 *
	 * @param version a specific version, or ALL_VERSIONS for all versions
	 * @param direction the direction, either D_ANCESTORS or D_DESCENDANTS
	 * @param flags a combination of A_* flags, or 0 for defaults
	 * @return a vector of results &ndash; instances of CPLAncestryEntry
	 * @see CPLAncestryEntry
	 */
	public Vector<CPLAncestryEntry> getAncestry(int version, int direction,
			int flags) {

		SWIGTYPE_p_std_vector_cpl_ancestry_entry_t pVector
			= CPLDirect.new_std_vector_cpl_ancestry_entry_tp();
		SWIGTYPE_p_void pv = CPLDirect
			.cpl_convert_p_std_vector_cpl_ancestry_entry_t_to_p_void(pVector);
		Vector<CPLAncestryEntry> result = null;

		try {
			int r = CPLDirect.cpl_get_object_ancestry(id, version, direction,
					flags, CPLDirect.cpl_cb_collect_ancestry_vector, pv);
			CPLException.assertSuccess(r);

			cpl_ancestry_entry_t_vector v = CPLDirect
				.cpl_dereference_p_std_vector_cpl_ancestry_entry_t(pVector);
			long l = v.size();
			result = new Vector<CPLAncestryEntry>((int) l);
			for (long i = 0; i < l; i++) {
				cpl_ancestry_entry_t e = v.get((int) i);
				result.add(new CPLAncestryEntry(
						new CPLObjectVersion(this, e.getQuery_object_version()),
						new CPLObjectVersion(new CPLObject(e.getOther_object_id()),
							e.getOther_object_version()),
						e.getType(),
						direction == D_ANCESTORS));
			}
		}
		finally {
			CPLDirect.delete_std_vector_cpl_ancestry_entry_tp(pVector);
		}

		return result;
	}
}

