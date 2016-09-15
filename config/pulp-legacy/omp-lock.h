/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Lock primitives implementation */

#ifndef __OMP_LOCK_H__
#define __OMP_LOCK_H__

#include "appsupport.h"
#include "memutils.h"

typedef uint32_t omp_lock_t;

#define BUSY_LOCK   0xffffffffU
#define FREE_LOCK   0x0U

/* gomp_hal_lock() - block until able to acquire lock "lock" */
static inline void
gomp_hal_lock( omp_lock_t *lock )
{
	volatile omp_lock_t *lock_ptr = (volatile omp_lock_t *)( (uint32_t) lock + TEST_AND_SET_OFFSET);
    // printf("[%d-%d][gomp_hal_lock] Locking at 0x%x (0x%x, 0x%x) \n", get_proc_id(), get_cl_id(), lock_ptr, lock, TEST_AND_SET_OFFSET);
    while (*lock_ptr == BUSY_LOCK);
    // printf("[%d-%d][gomp_hal_lock] Locked  at 0x%x\n", get_proc_id(), get_cl_id(), lock_ptr);
}

/* gomp_hal_unlock() - release lock "lock" */
static inline void
gomp_hal_unlock( omp_lock_t *lock )
{
    *lock = FREE_LOCK;
}

static inline void
gomp_hal_init_lock( omp_lock_t *lock )
{    
    lock = (omp_lock_t *)l1malloc(sizeof(omp_lock_t));
    *lock = 0x0U;
}

static inline int
gomp_hal_test_lock( omp_lock_t *lock )
{
    int ret = (int ) *lock;
    *lock = FREE_LOCK;
    return ret;
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