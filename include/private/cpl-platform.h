/*
 * cpl-platform.h
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

#ifndef __CPL_PRIVATE_PLATFORM_H__
#define __CPL_PRIVATE_PLATFORM_H__

#include <pthread.h>

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif



/***************************************************************************/
/** Cross-Platform Compatibility: Mutex                                   **/
/***************************************************************************/

/**
 * Mutex
 */
typedef pthread_mutex_t mutex_t;

/**
 * Initialize a mutex
 *
 * @param m the mutex
 */
#define mutex_init(m) pthread_mutex_init(&(m), NULL);

/**
 * Destroy a mutex
 *
 * @param m the mutex
 */
#define mutex_destroy(m) pthread_mutex_destroy(&(m));

/**
 * Lock a mutex
 *
 * @param m the mutex
 */
#define mutex_lock(m) pthread_mutex_lock(&(m));

/**
 * Unlock a mutex
 *
 * @param m the mutex
 */
#define mutex_unlock(m) pthread_mutex_unlock(&(m));


/***************************************************************************/
/** Cross-Platform Compatibility: Semaphore                               **/
/***************************************************************************/

#ifdef __APPLE__

	typedef dispatch_semaphore_t sema_t;

	static inline void
	sema_create(sema_t *s, long value)
	{
		*s = dispatch_semaphore_create(value);
	}

	#define sema_init(s, value) sema_create(&s, value);

	#define sema_wait(s) dispatch_semaphore_wait(s, DISPATCH_TIME_FOREVER);

	#define sema_post(s) dispatch_semaphore_signal(s);

	#define sema_destroy(s) dispatch_release(s);
#else
	typedef sem_t sema_t;

	#define sema_init(s, value) sem_init(&s, 0, value);

	#define sema_wait(s) sem_wait(&s);

	#define sema_post(s) sem_post(&s);

	#define sema_destroy(s) sem_destroy(&s);
#endif


/***************************************************************************/
/** Helpers: Mutex                                                        **/
/***************************************************************************/

/**
 * Mutex
 */
class Mutex {

	mutex_t m_mutex;


public:

	/**
	 * Initialize the mutex
	 */
	inline Mutex(void) { mutex_init(m_mutex); }

	/**
	 * Destroy the mutex
	 */
	inline ~Mutex(void) { mutex_destroy(m_mutex); }

	/**
	 * Lock
	 */
	inline void Lock(void) { mutex_lock(m_mutex); }

	/**
	 * Unlock
	 */
	inline void Unlock(void) { mutex_unlock(m_mutex); }
};

#endif

