/*
 * cpl-platform.cpp
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

#include "stdafx.h"
#include "cpl-platform.h"

#ifdef __unix__
#include <net/if.h> 
#include <netinet/in.h>
#include <sys/ioctl.h>
#endif

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/network/IONetworkInterface.h>
#include <IOKit/network/IOEthernetInterface.h>
#include <IOKit/network/IOEthernetController.h>
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

#else
#error "Not implemented for this platform"
#endif

	return CPL_OK;
}
