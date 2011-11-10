/*
 * cpl-lock.cpp
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
#include "cpl-lock.h"
#include "cpl-platform.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#endif

#ifdef _WINDOWS
#include <aclapi.h>
#include <tchar.h>
#endif

#ifdef _WINDOWS
#pragma intrinsic(_InterlockedCompareExchange, _InterlockedExchange)
#endif

#define CPL_LOCK_SEM_INIT_BASE	"edu.harvard.pass.cpl.unique_id_generator_init"
#if defined(__unix__)
#define CPL_LOCK_SEM_INIT		("/" CPL_LOCK_SEM_INIT_BASE)
#elif defined(_WINDOWS)
#define CPL_LOCK_SEM_INIT		("Global\\" CPL_LOCK_SEM_INIT_BASE)
#else
#define CPL_LOCK_SEM_INIT		("/" CPL_LOCK_SEM_INIT_BASE)
#endif


/**
 * The base for the unique ID generator
 */
static unsigned long long cpl_unique_base = 0;

/**
 * The process-local counter for the unique ID generator
 */
static unsigned long long cpl_unique_counter = 0;

/**
 * The lock for the process-local unique ID generator
 */
static cpl_lock_t cpl_unique_lock = 0;


/**
 * (Re)Initialize the host-local unique ID generator
 *
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_unique_id_generator_initialize(void)
{
	cpl_return_t ret = CPL_OK;

#if defined(__unix__) || defined(__APPLE__)

	mode_t u = umask(0);
	sem_t* s = sem_open(CPL_LOCK_SEM_INIT, O_CREAT, 0777, 1);
	if (s == SEM_FAILED) {
		fprintf(stderr, "Error while creating a semaphore: %s\n",
				strerror(errno));
		umask(u);
		return CPL_E_PLATFORM_ERROR;
	}
	umask(u);

	if (sem_wait(s) != 0) std::abort();

	usleep(10 * 1000 /* us */);
	timeval tv;
	gettimeofday(&tv, NULL);

	cpl_unique_base = (((unsigned long long) (tv.tv_sec - 1000000000UL)) << 32)
		| (((unsigned long) (tv.tv_usec / 1000)) << 22);
	cpl_unique_counter = 0;

	if (sem_post(s) != 0) std::abort();
	sem_close(s);

#elif defined(_WINDOWS)

	// Portions of the following code are from:
	//   http://msdn.microsoft.com/en-us/library/windows/desktop/aa446595%28v=vs.85%29.aspx

	DWORD dwRes;
	PSID pEveryoneSID = NULL;
	PACL pACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;
	EXPLICIT_ACCESS ea[1];
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld =
		SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	SECURITY_ATTRIBUTES sa;
	HANDLE hSemaphore = NULL;

	ret = CPL_E_PLATFORM_ERROR;

	// Create a well-known SID for the Everyone group.
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1,
		SECURITY_WORLD_RID,
		0, 0, 0, 0, 0, 0, 0,
		&pEveryoneSID)) {
			fprintf(stderr, "AllocateAndInitializeSid Error %u\n", GetLastError());
			goto cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE.
	// The ACE will allow Everyone full access to the mutex.
	ZeroMemory(&ea, sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = MUTEX_ALL_ACCESS;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance= NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

	// Create a new ACL that contains the new ACEs.
	dwRes = SetEntriesInAcl(1, ea, NULL, &pACL);
	if (ERROR_SUCCESS != dwRes) {
		fprintf(stderr, "SetEntriesInAcl Error %u\n", GetLastError());
		goto cleanup;
	}

	// Initialize a security descriptor.  
	pSD = (PSECURITY_DESCRIPTOR) LocalAlloc(LPTR, 
		SECURITY_DESCRIPTOR_MIN_LENGTH); 
	if (NULL == pSD) { 
		fprintf(stderr, "LocalAlloc Error %u\n", GetLastError());
		goto cleanup; 
	} 

	if (!InitializeSecurityDescriptor(pSD,
		SECURITY_DESCRIPTOR_REVISION)) {  
			fprintf(stderr, "InitializeSecurityDescriptor Error %u\n",
				GetLastError());
			goto cleanup; 
	} 

	// Add the ACL to the security descriptor. 
	if (!SetSecurityDescriptorDacl(pSD, 
		TRUE,     // bDaclPresent flag   
		pACL, 
		FALSE))   // not a default DACL 
	{  
		fprintf(stderr, "SetSecurityDescriptorDacl Error %u\n",
			GetLastError());
		goto cleanup; 
	} 

	// Initialize a security attributes structure.
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = pSD;
	sa.bInheritHandle = FALSE;

	// Create the semaphore
	hSemaphore = CreateSemaphore(&sa, 1, 1, CPL_LOCK_SEM_INIT);
	if (NULL == hSemaphore)	{  
		fprintf(stderr, "CreateSemaphore Error %u\n",
			GetLastError());
		goto cleanup; 
	}


	// Initialize the unique ID generator

	WaitForSingleObject(hSemaphore, INFINITE);

	Sleep(10 /* ms */);
	timeval tv;
	gettimeofday(&tv, NULL);

	cpl_unique_base = (((unsigned long long) (tv.tv_sec - 1000000000UL)) << 32)
		| (((unsigned long) (tv.tv_usec / 1000)) << 22);
	cpl_unique_counter = 0;

	if (!ReleaseSemaphore(hSemaphore, 1, NULL)) std::abort();


	// Finish

	ret = CPL_OK;

cleanup:
	if (pEveryoneSID) FreeSid(pEveryoneSID);
	if (pACL) LocalFree(pACL);
	if (pSD) LocalFree(pSD);
	if (hSemaphore) CloseHandle(hSemaphore);

#else
#error "Not implemented for this platform"
#endif

	return ret;
}


/**
 * Initialize the locking subsystem
 * 
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_lock_initialize(void)
{
	// Initialize the host-local unique ID generator

	cpl_return_t r = cpl_unique_id_generator_initialize();
	if (!CPL_IS_OK(r)) return r;

	return CPL_OK;
}


/**
 * Cleanup
 */
void
cpl_lock_cleanup(void)
{
	// Nothing to do
}


/**
 * Lock
 *
 * @param lock the pointer to the lock
 * @param yield whether to yield while waiting
 */
void
cpl_lock(cpl_lock_t* lock, bool yield)
{
	assert(lock != NULL);

#if defined __GNUC__
	while (__sync_lock_test_and_set(lock, 1)) {
		while (*lock) {
			if (yield) usleep(100);
		}
	}
#elif defined _WINDOWS
	while (_InterlockedCompareExchange(lock, 1, 0) == 1) {
		while (*lock) {
			if (yield) Sleep(1);
		}
	}
#else
#error "Not implemented"
#endif
}


/**
 * Unlock
 *
 * @param lock the pointer to the lock
 */
void
cpl_unlock(cpl_lock_t* lock)
{
	assert(lock != NULL);

#if defined __GNUC__
	__sync_lock_release(lock);
#elif defined _WINDOWS
	_InterlockedExchange(lock, 0);
#else
#error "Not implemented"
#endif
}


/**
 * Generate a host-local unique ID
 *
 * @return the unique 64-bit ID
 */
unsigned long long
cpl_next_unique_id(void)
{
	cpl_lock(&cpl_unique_lock);


	// Check for the counter overflow

	if (cpl_unique_counter >= 1ULL << 22) {

		// Need to reinitialize the counter

		cpl_return_t r = cpl_unique_id_generator_initialize();
		if (!CPL_IS_OK(r)) {
			cpl_unlock(&cpl_unique_lock);
			throw CPLException("Failed to reinitialize the unique ID generator");
		}
	}


	// Advance the process-local counter and generate the host-local unique ID

	unsigned long long x = (cpl_unique_counter++) | cpl_unique_base;


	// Finish

	cpl_unlock(&cpl_unique_lock);

	return x;
}

