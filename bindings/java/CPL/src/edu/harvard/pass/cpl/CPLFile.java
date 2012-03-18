package edu.harvard.pass.cpl;

/*
 * CPLFile.java
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

import java.io.File;


/**
 * A provenance object that represents a file on disk
 *
 * @author Peter Macko
 */
public class CPLFile extends CPLObject {

	/// The file
	private File file;

	/**
	 * The creation mode
	 */
	public enum CreationMode {
		FAIL_IF_DOES_NOT_EXIST,
		ALWAYS_CREATE,
		CREATE_IF_DOES_NOT_EXIST
	};


	/**
	 * Create an instance of CPLFile
	 *
	 * @param id the internal CPL object ID
	 * @param file the file object
	 */
	CPLFile(cpl_id_t id, File file) {
		super(id);
		this.file = file;
	}


	/**
	 * Look up a provenance object corresponding to the given file
	 *
	 * @param file the file object
	 * @param mode the creation mode of the corresponding provenance object
	 */
	public static CPLFile lookup(File file, CreationMode mode) {

		int m = 0;
		if (mode == CreationMode.ALWAYS_CREATE) {
			m = CPLDirectConstants.CPL_F_ALWAYS_CREATE;
		}
		if (mode == CreationMode.CREATE_IF_DOES_NOT_EXIST) {
			m = CPLDirectConstants.CPL_F_CREATE_IF_DOES_NOT_EXIST;
		}

		cpl_id_t id = new cpl_id_t();
		SWIGTYPE_p_int vp = CPLDirect.new_cpl_version_tp();
		int r = CPLDirect.cpl_lookup_file(file.getAbsolutePath(), m, id, vp);
		int v = CPLDirect.cpl_version_tp_value(vp);
		CPLDirect.delete_cpl_version_tp(vp);
		CPLException.assertSuccess(r);

		return new CPLFile(id, file);
	}


	/**
	 * Look up a provenance object corresponding to the given file; fail if
	 * the object does not exist
	 *
	 * @param file the file object
	 */
	public static CPLFile lookup(File file) {
		return lookup(file, CreationMode.FAIL_IF_DOES_NOT_EXIST);
	}


	/**
	 * Look up a provenance object corresponding to the given file; create it
	 * if the object does not exist
	 *
	 * @param file the file object
	 */
	public static CPLFile lookupOrCreate(File file) {
		return lookup(file, CreationMode.CREATE_IF_DOES_NOT_EXIST);
	}


	/**
	 * Create a new provenance object for the given file
	 *
	 * @param file the file object
	 */
	public static CPLFile create(File file) {
		return lookup(file, CreationMode.ALWAYS_CREATE);
	}
}

