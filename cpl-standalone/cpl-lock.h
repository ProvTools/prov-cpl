/*
 * cpl-lock.h
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

#ifndef __CPL_LOCK_H__
#define __CPL_LOCK_H__

#include <cpl.h>


/***************************************************************************/
/** Types                                                                 **/
/***************************************************************************/

/**
 * A light-weight lock
 */
typedef long cpl_lock_t;


/***************************************************************************/
/** Functions - Initialization/Cleanup                                    **/
/***************************************************************************/

/**
 * Initialize the locking subsystem
 * 
 * @return CPL_OK or an error code
 */
cpl_return_t
cpl_lock_initialize(void);

/**
 * Cleanup
 */
void
cpl_lock_cleanup(void);


/***************************************************************************/
/** Functions - Locks                                                     **/
/***************************************************************************/

/**
 * Lock
 *
 * @param lock the pointer to the lock
 * @param yield whether to yield while waiting
 */
void
cpl_lock(cpl_lock_t* lock, bool yield=true);

/**
 * Unlock
 *
 * @param lock the pointer to the lock
 */
void
cpl_unlock(cpl_lock_t* lock);


/***************************************************************************/
/** Functions - Host-Unique IDs                                           **/
/***************************************************************************/

/**
 * Generate a host-local unique ID
 *
 * @return the unique 64-bit ID
 */
unsigned long long
cpl_next_unique_id(void);


/***************************************************************************/
/** Convenience Classes                                                   **/
/***************************************************************************/

/**
 * Automatic unlock
 */
class CPL_AutoUnlock
{
	/**
	 * The pointer to the lock
	 */
	cpl_lock_t* m_lock;


public:
		
	/**
	 * Create an instance of CPL_AutoUnlock
	 *
	 * @param lock the pointer to the lock
	 */
	CPL_AutoUnlock(cpl_lock_t* lock) { m_lock = lock; }

	/**
	 * Destroy the class
	 */
	~CPL_AutoUnlock(void) { if (m_lock != NULL) cpl_unlock(m_lock); }
};


#endif

