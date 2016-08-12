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
	cpl_id_t id;

	/// The MAC address
	private String macAddress = null;

	/// Program name
	private String program = null;

	/// Program command line
	private String cmdline = null;

	/// User name
	private String user = null;

	/// PID
	private int pid = -1;

	/// Start time
	private long startTime = -1;

	/// Whether we know the above information about the session
	private boolean knowInfo = false;


	/**
	 * Create an instance of CPLSession from its ID
	 *
	 * @param id the internal CPL session ID
	 */
	CPLSession(cpl_id_t id) {
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
	 * Fetch the session info if it is not already present
	 *
	 * @return true if the info was just fetched, false if we already had it
	 */
	protected boolean fetchInfo() {

		if (knowInfo) return false;


		// Fetch the info from CPL
		
		SWIGTYPE_p_p_cpl_session_info_t ppInfo
			= CPLDirect.new_cpl_session_info_tpp();

		try {
			int r = CPLDirect.cpl_get_session_info(id,
					CPLDirect.cpl_convert_pp_cpl_session_info_t(ppInfo));
			CPLException.assertSuccess(r);

			cpl_session_info_t info
				= CPLDirect.cpl_dereference_pp_cpl_session_info_t(ppInfo);

			macAddress = info.getMac_address();
			user = info.getUser();
			pid = info.getPid();
			program = info.getProgram();
			cmdline = info.getCmdline();
			startTime = info.getStart_time();

			knowInfo = true;
			CPLDirect.cpl_free_session_info(info);
		}
		finally {
			CPLDirect.delete_cpl_session_info_tpp(ppInfo);
		}

		return true;
	}


	/**
	 * Get the MAC address of the computer associated with the session
	 *
	 * @return the MAC address as a string
	 */
	public String getMACAddress() {
		if (!knowInfo) fetchInfo();
		return macAddress;
	}


	/**
	 * Get the user name
	 *
	 * @return the user name
	 */
	public String getUser() {
		if (!knowInfo) fetchInfo();
		return user;
	}


	/**
	 * Get the program name
	 *
	 * @return the program name
	 */
	public String getProgram() {
		if (!knowInfo) fetchInfo();
		return program;
	}


	/**
	 * Get the program's command line
	 *
	 * @return the program's command line
	 */
	public String getCommandLine() {
		if (!knowInfo) fetchInfo();
		return cmdline;
	}


	/**
	 * Get the PID of the program
	 *
	 * @return the PID
	 */
	public int getPID() {
		if (!knowInfo) fetchInfo();
		return pid;
	}


	/**
	 * Get the session start time
	 *
	 * @return the session start time expressed as Unix time
	 */
	public long getStartTime() {
		if (!knowInfo) fetchInfo();
		return startTime;
	}


	/**
	 * Create a more detailed string representation of the object
	 *
	 * @param detail whether to provide even more detail
	 * @return a multi-line string describing the object
	 */
	public String toString(boolean detail) {

		StringBuilder sb = new StringBuilder();

		sb.append("MAC Address ");
		if (detail) sb.append(": "); else sb.append(": ");
		sb.append(getMACAddress());
		sb.append("\n");

		sb.append("Program name");
		if (detail) sb.append(": "); else sb.append(": ");
		sb.append(getProgram());
		sb.append("\n");

		if (detail) {
			sb.append("Command line: ");
			sb.append(getCommandLine());
			sb.append("\n");

			sb.append("Program PID : ");
			sb.append(getPID());
			sb.append("\n");
		}

		sb.append("User name   ");
		if (detail) sb.append(": "); else sb.append(": ");
		sb.append(getUser());
		sb.append("\n");

		if (detail) {
			sb.append("Start time  : ");
			sb.append(new java.sql.Date(1000L * getStartTime()));
			sb.append(" ");
			sb.append(new java.sql.Time(1000L * getStartTime()));
			sb.append("\n");
		}

		return sb.toString();
	}
}

