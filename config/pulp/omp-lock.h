/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Authors:
 *    Alessandro Capotondi, UNIBO, (alessandro.capotondi@unibo.it)
 *    Germain Haugou,       ETH,   (germain.haugou@iis.ee.ethz.ch)
 */

/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
 *
 * This file is part of the GNU OpenMP Library (libgomp).
 *
 * Libgomp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with this program;
 * see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef __OMP_LOCK_H__
#define __OMP_LOCK_H__

#include "appsupport.h"
#include "memutils.h"

typedef uint32_t omp_lock_t;

// #define OMP_LOCK_DEBUG

// #define BUSY_LOCK   0x1U
#define FREE_LOCK   0x0U

static inline void __mem_fence()
{
  __asm__ __volatile__ ("" : : : "memory");
}

// Lock a mutex and return its prior state.
static inline omp_lock_t mutex_trylock(omp_lock_t* const lock)
{
  __mem_fence();
  omp_lock_t res = !FREE_LOCK;
  __asm__ volatile(
          "amoor.w %[res], %[res], (%[lock])"
          : [res] "+r" (res)
          : [lock] "r" (lock)
      );
  __mem_fence();
  return res;
}

/* gomp_hal_lock() - block until able to acquire lock "lock" */
static inline void
gomp_hal_lock( omp_lock_t *lock )
{
#ifdef OMP_LOCK_DEBUG
    printf("[%d-%d][gomp_hal_lock] Locking at 0x%x\n", get_proc_id(), get_cl_id(), lock);
#endif

    while (mutex_trylock(lock) != FREE_LOCK) { }

#ifdef OMP_LOCK_DEBUG
    printf("[%d-%d][gomp_hal_lock] Locked  at 0x%x\n", get_proc_id(), get_cl_id(), lock);
#endif
}

/* gomp_hal_unlock() - release lock "lock" */
static inline void
gomp_hal_unlock( omp_lock_t *lock )
    {
    __mem_fence();
    *lock = FREE_LOCK;
    __mem_fence();
}

static inline void
gomp_hal_init_lock( omp_lock_t *lock )
{
    lock = (omp_lock_t *)l1malloc(sizeof(omp_lock_t));
    *lock = FREE_LOCK;
}

static inline int
gomp_hal_test_lock( omp_lock_t *lock )
{
    return (int ) *lock;
}

static inline void
gomp_hal_destroy_lock( omp_lock_t *lock )
{
    l1free(lock);
}

/*********************************** standard APIs ***********************************************/

void omp_set_lock(omp_lock_t *);
void omp_unset_lock(omp_lock_t *);
void omp_init_lock(omp_lock_t *);
int omp_test_lock(omp_lock_t *);
void omp_destroy_lock(omp_lock_t *);

#endif
