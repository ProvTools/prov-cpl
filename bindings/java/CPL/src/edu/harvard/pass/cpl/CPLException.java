package edu.harvard.pass.cpl;

/*
 * CPLException.java
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


/**
 * CPL exception
 *
 * @author Jackson Okuhn, Peter Macko
 */
public class CPLException extends RuntimeException {

	/// The CPL error code
	private int code;


    /**
     * Create an instance of CPLException
     *
	 * @param message custom message
     * @param code the internal CPL error code
	 * @param cause the cause
     */
    public CPLException(String message, int code, Throwable cause) {
		super("" + message + " -- " + CPLDirect.cpl_error_string(code), cause);
		this.code = code;
    }


    /**
     * Create an instance of CPLException
     *
     * @param code the internal CPL error code
	 * @param cause the cause
     */
    public CPLException(int code, Throwable cause) {
		super(CPLDirect.cpl_error_string(code), cause);
		this.code = code;
    }


    /**
     * Create an instance of CPLException
     *
	 * @param message custom message
     * @param code the internal CPL error code
     */
    public CPLException(String message, int code) {
		super("" + message + " -- " + CPLDirect.cpl_error_string(code));
		this.code = code;
    }


    /**
     * Create an instance of CPLException
     *
     * @param code the internal CPL error code
     */
    public CPLException(int code) {
		super(CPLDirect.cpl_error_string(code));
		this.code = code;
    }


	/**
	 * Determine whether the given status code is a success code
	 *
	 * @param code the numerical status code
	 * @return true if it is a success code
	 */
	public static boolean isSuccess(int code) {
		return code >= 0;
	}


	/**
	 * Determine whether the given status code is an error code
	 *
	 * @param code the numerical status code
	 * @return true if it is an error code
	 */
	public static boolean isError(int code) {
		return code < 0;
	}


	/**
	 * Assert that no error occurred
	 *
	 * @param message custom message
     * @param code the internal CPL status code
	 */
	public static void assertSuccess(String message, int code) {
		if (code < 0) {
			throw new CPLException(message, code);
		}
	}


	/**
	 * Assert that no error occurred
	 *
     * @param code the internal CPL status code
	 */
	public static void assertSuccess(int code) {
		if (code < 0) {
			throw new CPLException(code);
		}
	}


	/**
	 * Get the numerical CPL error code
	 * 
	 * @return the numerical error code
	 */
	public int getErrorCode() {
		return code;
	}
}

