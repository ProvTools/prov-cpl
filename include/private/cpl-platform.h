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

struct sema_t {
#ifdef __APPLE__
    dispatch_semaphore_t    sem;
#else
    sem_t                   sem;
#endif
};

static inline void
sema_init(struct sema_t s, uint32_t value)
{
#ifdef __APPLE__
    dispatch_semaphore_t *sem = &s.sem;

    *sem = dispatch_semaphore_create(value);
#else
    sem_init(&s.sem, 0, value);
#endif
}

static inline void
sema_wait(struct sema_t s)
{

#ifdef __APPLE__
    dispatch_semaphore_wait(s.sem, DISPATCH_TIME_FOREVER);
#else
    sem_wait(&s.sem);
#endif
}

static inline void
sema_post(struct sema_t s)
{

#ifdef __APPLE__
    dispatch_semaphore_signal(s.sem);
#else
    sem_post(&s.sem);
#endif
}

static inline void
sema_destroy(struct sema_t s)
{

#ifdef __APPLE__
    dispatch_release(s.sem);
#else
    sem_destroy(&s.sem);
#endif
}

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

