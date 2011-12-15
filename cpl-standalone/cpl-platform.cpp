/*
 * cpl-platform.cpp
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

#include "stdafx.h"
#include "cpl-platform.h"

#ifdef __unix__
#include <net/if.h> 
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <uuid/uuid.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOEthernetController.h>
#include <uuid/uuid.h>
#endif

#ifdef _WINDOWS
#include <iphlpapi.h>
#include <rpc.h>
#include <time.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "rpcrt4.lib")
#endif



/***************************************************************************/
/** Windows Implementations of UNIX Functions                             **/
/***************************************************************************/

#ifdef _WINDOWS

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#elif defined(_WINDOWS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif


/**
 * Get the current UNIX time
 *
 * @param tv the output timeval structure (or NULL)
 * @param tz the output timezone structure (or NULL)
 * @return 0 on success or -1 on error
 */
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
	// From: http://suacommunity.com/dictionary/gettimeofday-entry.php

	// Define a structure to receive the current Windows filetime
	FILETIME ft;

	// Initialize the present time to 0 and the timezone to UTC
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;

	if (NULL != tv)
	{
		GetSystemTimeAsFileTime(&ft);

		// The GetSystemTimeAsFileTime returns the number of 100 nanosecond 
		// intervals since Jan 1, 1601 in a structure. Copy the high bits to 
		// the 64 bit tmpres, shift it left by 32 then or in the low 32 bits.
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		// Convert to microseconds by dividing by 10
		tmpres /= 10;

		// The Unix epoch starts on Jan 1 1970.  Need to subtract the difference 
		// in seconds from Jan 1 1601.
		tmpres -= DELTA_EPOCH_IN_MICROSECS;

		// Finally change microseconds to seconds and place in the seconds value. 
		// The modulus picks up the microseconds.
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}

	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset();
			tzflag++;
		}

		// Adjust for the timezone west of Greenwich
		long t = 0; int d = 0;
		_get_timezone(&t);
		_get_daylight(&d);
		tz->tz_minuteswest = t / 60;
		tz->tz_dsttime = d;
	}

	return 0;
}

#endif



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
cpl_platform_get_mac_address(cpl_mac_address_t* out)
{

#ifdef __unix__

	// From: http://stackoverflow.com/questions/1779715/how-to-get-mac-address-of-your-machine-using-a-c-program

	ifreq ifr;
	ifconf ifc;
	char buf[1024];
	int success = 0;

	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (sock == -1) return CPL_E_PLATFORM_ERROR;

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if (ioctl(sock, SIOCGIFCONF, &ifc) == -1) return CPL_E_PLATFORM_ERROR;

	ifreq* it = ifc.ifc_req;
	const ifreq* const end = it + (ifc.ifc_len / sizeof(ifreq));

	for (; it != end; ++it) {
		strcpy(ifr.ifr_name, it->ifr_name);
		if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
			if (! (ifr.ifr_flags & IFF_LOOPBACK)) { // don't count loopback
				if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
					success = 1;
					break;
				}
			}
		}
		else { /* ignore error */ }
	}

	if (success && out) {
		memcpy(out, ifr.ifr_hwaddr.sa_data, 6);
	}

	if (!success) return CPL_E_NOT_FOUND;

#elif defined(__APPLE__)

	/*
	 * Adapted from GetMACAddress.c:
	 *   http://opensource.apple.com/source/DirectoryService
	 *            /DirectoryService-621/CoreFramework/Private/GetMACAddress.c
	 *
	 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
	 * 
	 * This file contains Original Code and/or Modifications of Original Code
	 * as defined in and that are subject to the Apple Public Source License
	 * Version 2.0 (the 'License'). You may not use this file except in
	 * compliance with the License. Please obtain a copy of the License at
	 * http://www.opensource.apple.com/apsl/ and read it before using this
	 * file.
	 * 
	 * The Original Code and all software distributed under the License are
	 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
	 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
	 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
	 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
	 * Please see the License for the specific language governing rights and
	 * limitations under the License.
	 */

	kern_return_t kernResult = KERN_FAILURE; 
	mach_port_t masterPort = MACH_PORT_NULL;
	CFMutableDictionaryRef classesToMatch = NULL;
	io_object_t intfService = MACH_PORT_NULL;
	io_object_t controllerService = MACH_PORT_NULL;
	io_iterator_t intfIterator = MACH_PORT_NULL;
	unsigned char macAddress[kIOEthernetAddressSize];
	io_iterator_t *matchingServices = &intfIterator;
	UInt8 *MACAddress = macAddress;

	// Create an iterator with Primary Ethernet interface
	kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
	if (kernResult != KERN_SUCCESS) return CPL_E_PLATFORM_ERROR;

	// Ethernet interfaces are instances of class kIOEthernetInterfaceClass
	classesToMatch = IOServiceMatching(kIOEthernetInterfaceClass);
	if (classesToMatch != NULL) {

		CFMutableDictionaryRef propertyMatch;
		propertyMatch = CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
				&kCFTypeDictionaryKeyCallBacks,
				&kCFTypeDictionaryValueCallBacks);

		CFDictionarySetValue(propertyMatch,
				CFSTR(kIOPrimaryInterface), kCFBooleanTrue);
		CFDictionarySetValue(classesToMatch,
				CFSTR(kIOPropertyMatchKey), propertyMatch);

		CFRelease(propertyMatch);

		kernResult = IOServiceGetMatchingServices(masterPort,
				classesToMatch, matchingServices);    
	}


	// Given an iterator across a set of Ethernet interfaces, return the
	// MAC address of the first one.

	intfService = IOIteratorNext(intfIterator);
	if (intfService == MACH_PORT_NULL) {
			IOObjectRelease(intfIterator);
			return CPL_E_PLATFORM_ERROR;
	}

	CFDataRef MACAddressAsCFData = NULL;        
	kernResult = IORegistryEntryGetParentEntry(intfService,
			kIOServicePlane,
			&controllerService);

	if (kernResult != KERN_SUCCESS || controllerService == MACH_PORT_NULL) {
		IOObjectRelease(intfService);
		IOObjectRelease(intfIterator);
		return CPL_E_PLATFORM_ERROR;
	}

	MACAddressAsCFData = (CFDataRef) IORegistryEntryCreateCFProperty(
			controllerService,
			CFSTR(kIOMACAddress),
			kCFAllocatorDefault,
			0);

	if (MACAddressAsCFData != NULL) {
		CFDataGetBytes(MACAddressAsCFData,
				CFRangeMake(0, kIOEthernetAddressSize), MACAddress);
		CFRelease(MACAddressAsCFData);
	}
	else {
		IOObjectRelease(controllerService);
		IOObjectRelease(intfService);
		IOObjectRelease(intfIterator);
		return CPL_E_NOT_FOUND;
	}

	IOObjectRelease(controllerService);
	IOObjectRelease(intfService);
	IOObjectRelease(intfIterator);

	if (out) memcpy(out, macAddress, 6);
	

#elif defined(_WINDOWS)

	PIP_ADAPTER_ADDRESSES AdapterAddresses;
	ULONG family = AF_UNSPEC;
	ULONG flags = 0;
	ULONG outBufLen = 0;
	bool success = false;

	DWORD dwRetVal = GetAdaptersAddresses(family, flags, NULL, NULL, &outBufLen);
	if (dwRetVal == ERROR_NO_DATA) return CPL_E_NOT_FOUND;
	if (dwRetVal == 0 && outBufLen == 0) return CPL_E_NOT_FOUND;
	if (dwRetVal != ERROR_BUFFER_OVERFLOW) return CPL_E_PLATFORM_ERROR;

	AdapterAddresses = (IP_ADAPTER_ADDRESSES*)
		malloc(sizeof(IP_ADAPTER_ADDRESSES) * outBufLen);
	if (AdapterAddresses == NULL) return CPL_E_INSUFFICIENT_RESOURCES;

	dwRetVal = GetAdaptersAddresses(family, flags, NULL, AdapterAddresses,
		&outBufLen);
	if (dwRetVal != 0) { free(AdapterAddresses); return CPL_E_PLATFORM_ERROR; }

	for (PIP_ADAPTER_ADDRESSES p = AdapterAddresses; p != NULL; p = p->Next) {
		if (p->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
		if (p->PhysicalAddressLength != 6 /* Ethernet */) continue;

		success = true;
		if (out) memcpy(out, p->PhysicalAddress, 6);
		break;
	}

	free(AdapterAddresses);

	if (!success) return CPL_E_NOT_FOUND;

#else
#error "Not implemented for this platform"
#endif

	return CPL_OK;
}


/**
 * Generate a UUID
 *
 * @param out the output character array
 * @return CPL_OK on success or an error code
 */
cpl_return_t
cpl_platform_generate_uuid(cpl_uuid_t* out)
{
#if defined(__unix__) || defined(__APPLE__)
	
	// Static assertion
	int __a[sizeof(uuid_t) == sizeof(cpl_id_t) ? 1 : -1]; (void) __a;

	uuid_t uuid;
	uuid_generate_random(uuid);
	memcpy(out, uuid, sizeof(cpl_id_t));

#elif defined(_WINDOWS)
	
	// Static assertion
	int __a[sizeof(UUID) == sizeof(cpl_id_t) ? 1 : -1]; (void) __a;

	UUID uuid;
	UuidCreate(&uuid);
	memcpy(out, &uuid, sizeof(cpl_id_t));

#else
#error "Not implemented for this platform"
#endif

	return CPL_OK;
}
