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
#include <private/cpl-lock.h>
#include <cpl-exception.h>
#include "cpl-platform.h"

#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#endif

/*
 * Configuration
 */
//#define _CPL_DEBUG_UNIX_SEM
//#define _CPL_CUSTOM_GLOBALLY_UNIQUE_IDS

#ifdef _CPL_CUSTOM_GLOBALLY_UNIQUE_IDS
#define CPL_LOCK_SEM_INIT		"edu.harvard.pass.cpl.uid_gen"

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
 * The unique machine identifier
 */
static unsigned long long cpl_unique_machine_id = 0;


/**
 * (Re)Initialize the host-local unique ID generator
 *
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_host_unique_id_generator_initialize(void)
{
	cpl_shared_semaphore_t s = cpl_shared_semaphore_open(CPL_LOCK_SEM_INIT);
	if (s == NULL) return CPL_E_PLATFORM_ERROR;
	cpl_shared_semaphore_wait(s);

	usleep(10 * 1000 /* us */);

	timeval tv;
	gettimeofday(&tv, NULL);

	cpl_unique_base = (((unsigned long long) (tv.tv_sec - 1000000000UL)) << 32)
		| (((unsigned long) (tv.tv_usec / 1000)) << 22);
	cpl_unique_base &= 0x7fffffffffffffffull;	// Ensure that it is positive
	cpl_unique_counter = 0;

	cpl_shared_semaphore_post(s);
	cpl_shared_semaphore_close(s);

	return CPL_OK;
}

#endif


/**
 * Initialize the locking subsystem
 * 
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_lock_initialize(void)
{
	cpl_return_t r;
	(void) r;

#ifdef _CPL_CUSTOM_GLOBALLY_UNIQUE_IDS

	// Initialize the host-local unique ID generator

	r = cpl_host_unique_id_generator_initialize();
	if (!CPL_IS_OK(r)) return r;


	// Get the machine's MAC address

	assert(sizeof(cpl_unique_machine_id) >= sizeof(cpl_mac_address_t));
	cpl_unique_machine_id = 0;
	r = cpl_platform_get_mac_address((cpl_mac_address_t*) &cpl_unique_machine_id);
	if (!CPL_IS_OK(r)) {
		// If we can't get the unique machine ID, get a random number
		cpl_unique_machine_id = rand();
	}
	cpl_unique_machine_id &= 0x7fffffffffffffffull;	// Make it positive

#endif

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
#else
#error "Not implemented"
#endif
}


/**
 * Open (initialize) a shared semaphore
 *
 * @param name the semaphore name without a prefix or a '/'
 * @return the shared semaphore, or NULL on error
 */
cpl_shared_semaphore_t
cpl_shared_semaphore_open(const char* name)
{
#if defined(__unix__) || defined(__APPLE__)

	char* n = (char*) alloca(strlen(name) + 4);
	sprintf(n, "/%s", name);

	mode_t u = umask(0);
	sem_t* s = sem_open(n, O_CREAT, 0777, 1);
	if (s == SEM_FAILED) {
		fprintf(stderr, "Error while creating a semaphore: %s\n",
				strerror(errno));
		umask(u);
		return NULL;
	}

#ifdef _CPL_DEBUG_UNIX_SEM
	int v = -7777;
	sem_getvalue(s, &v);
	fprintf(stderr, "[Semaphore %p] Open: name=%s, value=%d\n", s, n, v);
#endif

	umask(u);

	return s;

	snprintf(n, n_size, "Global\\%s", name);
#endif

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
	hSemaphore = CreateSemaphore(&sa, 1, 1, n);
	if (NULL == hSemaphore)	{  
		fprintf(stderr, "CreateSemaphore Error %u\n",
			GetLastError());
		goto cleanup; 
	}

cleanup:
	if (pEveryoneSID) FreeSid(pEveryoneSID);
	if (pACL) LocalFree(pACL);
	if (pSD) LocalFree(pSD);

	return hSemaphore;

#else
#error "Not implemented for this platform."
#endif
}


/**
 * Close a shared semaphore
 *
 * @param sem the shared semaphore
 */
void
cpl_shared_semaphore_close(cpl_shared_semaphore_t sem)
{
#if defined(__unix__) || defined(__APPLE__)
#ifdef _CPL_DEBUG_UNIX_SEM
	int v = -7777;
	sem_getvalue((sem_t*) sem, &v);
	fprintf(stderr, "[Semaphore %p] Close: value=%d\n", (sem_t*) sem, v);
#endif
	sem_close((sem_t*) sem);
#else
#error "Not implemented for this platform."
#endif
}


/**
 * Wait on a shared semaphore
 *
 * @param sem the shared semaphore
 */
void
cpl_shared_semaphore_wait(cpl_shared_semaphore_t sem)
{
#if defined(__unix__) || defined(__APPLE__)
#ifdef _CPL_DEBUG_UNIX_SEM
	int v = -7777;
	sem_getvalue((sem_t*) sem, &v);
	fprintf(stderr, "[Semaphore %p] Wait: value=%d\n", (sem_t*) sem, v);
#endif
	if (sem_wait((sem_t*) sem) != 0) {
		fprintf(stderr, "sem_wait() failed.\n");
		std::abort();
	}
#ifdef _CPL_DEBUG_UNIX_SEM
	v = -7777;
	sem_getvalue((sem_t*) sem, &v);
	fprintf(stderr, "[Semaphore %p] Wait - after: value=%d\n", (sem_t*) sem, v);
#endif
#else
#error "Not implemented for this platform."
#endif
}


/**
 * Release (post) a shared semaphore
 *
 * @param sem the shared semaphore
 */
void
cpl_shared_semaphore_post(cpl_shared_semaphore_t sem)
{
#if defined(__unix__) || defined(__APPLE__)
#ifdef _CPL_DEBUG_UNIX_SEM
	int v = -7777;
	sem_getvalue((sem_t*) sem, &v);
	fprintf(stderr, "[Semaphore %p] Post: value=%d\n", (sem_t*) sem, v);
#endif
	if (sem_post((sem_t*) sem) != 0) {
		fprintf(stderr, "sem_post() failed.\n");
		std::abort();
	}
#ifdef _CPL_DEBUG_UNIX_SEM
	v = -7777;
	sem_getvalue((sem_t*) sem, &v);
	fprintf(stderr, "[Semaphore %p] Post - after: value=%d\n", (sem_t*) sem, v);
#endif
#else
#error "Not implemented for this platform."
#endif
}


#ifdef _CPL_CUSTOM_GLOBALLY_UNIQUE_IDS

/**
 * Generate a host-local unique ID
 *
 * @return the unique 64-bit ID
 */
unsigned long long
cpl_next_host_unique_id(void)
{
	cpl_lock(&cpl_unique_lock);


	// Check for the counter overflow

	if (cpl_unique_counter >= 1ULL << 22) {

		// Need to reinitialize the counter

		cpl_return_t r = cpl_host_unique_id_generator_initialize();
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

#endif

