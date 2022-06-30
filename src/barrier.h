/*
    Ophidia Primitives
    Copyright (C) 2012-2022 CMCC Foundation

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * barrier.h
 *
 * This header file describes the "barrier" synchronization
 * construct. The type barrier_t describes the full state of the
 * barrier including the POSIX 1003.1c synchronization objects
 * necessary.
 *
 * A barrier causes threads to wait until a set of threads has
 * all "reached" the barrier. The number of threads required is
 * set when the barrier is initialized, and cannot be changed
 * except by reinitializing.
 */
#include <pthread.h>

/*
 * Structure describing a barrier.
 */
typedef struct barrier_tag {
    pthread_mutex_t     mutex;          /* Control access to barrier */
    pthread_cond_t      cv;             /* wait for barrier */
    int                 valid;          /* set when valid */
    int                 threshold;      /* number of threads required */
    int                 counter;        /* current number of threads */
    int                 cycle;          /* alternate wait cycles (0 or 1) */
} barrier_t;

#define BARRIER_VALID   0xdbcafe

/*
 * Support static initialization of barriers
 */
#define BARRIER_INITIALIZER(cnt) \
    {PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, \
    BARRIER_VALID, cnt, cnt, 0}

/*
 * Define barrier functions
 */
extern int barrier_init (barrier_t *barrier, int count);
extern int barrier_destroy (barrier_t *barrier);
extern int barrier_wait (barrier_t *barrier);
