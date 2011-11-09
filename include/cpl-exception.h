/*
 * cpl-exception.h
 * Core Provenance Library
 *
 * Copyright 2011
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

#ifndef __CPL_EXCEPTION_H__
#define __CPL_EXCEPTION_H__
#ifdef __cplusplus

#include <cstdarg>
#include <cstdio>
#include <exception>

#define CPL_EXCEPTION_MAX_MSG_LENGTH 256


/**
 * An exception with a message
 *
 * @author Peter Macko
 */
class CPLException : public std::exception
{

public:

	/**
	 * Constructor of the exception
	 *
	 * @param format the format of the exception
	 * @param ... the arguments of the format
	 */
	CPLException(const char* format, ...)
	{
		va_list args;
		va_start(args, format);

		vsnprintf(m_message, CPL_EXCEPTION_MAX_MSG_LENGTH, format, args);
		m_message[CPL_EXCEPTION_MAX_MSG_LENGTH] = '\0';

		va_end(args);
	}

	/**
	 * Destructor of the exception
	 */
	virtual ~CPLException(void) throw() {}

	/**
	 * Return the message of the exception
	 *
	 * @return the message
	 */
	virtual const char* what() const throw() { return m_message; }


private:

	char m_message[CPL_EXCEPTION_MAX_MSG_LENGTH + 4];
};


#endif
#endif

