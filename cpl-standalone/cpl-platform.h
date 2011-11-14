/*
 * cpl-platform.h
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

#ifndef __CPL_PLATFORM_H__
#define __CPL_PLATFORM_H__

#include <cpl.h>


/***************************************************************************/
/** Windows Implementations of UNIX Functions                             **/
/***************************************************************************/

#ifdef _WINDOWS

// From: http://suacommunity.com/dictionary/gettimeofday-entry.php
struct timezone
{
	int  tz_minuteswest; /* minutes W of Greenwich */
	int  tz_dsttime;     /* type of dst correction */
};

/**
 * Get the current UNIX time
 *
 * @param tv the output timeval structure (or NULL)
 * @param tz the output timezone structure (or NULL)
 * @return 0 on success or -1 on error
 */
int
gettimeofday(struct timeval *tv, struct timezone *tz);

#endif



/***************************************************************************/
/** Multi-Platform Type Definitions                                       **/
/***************************************************************************/

/**
 * MAC address
 */
typedef unsigned char cpl_mac_address_t[6];


/***************************************************************************/
/** Multi-Platform Functions                                              **/
/***************************************************************************/

/**
 * Get the MAC address of the first logical IP-enabled network interface
 *
 * @param out the output character array
 * @return CPL_OK on success or an error code
 */
cpl_return_t
cpl_platform_get_mac_address(cpl_mac_address_t* out);


#endif

