/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Lock primitives implementation */

#ifndef __LOCK_H__
#define __LOCK_H__

#include "appsupport.h"
#include "omp-lock.h"

#define BUSY_LOCK   0xffffffff
#define FREE_LOCK   0x0
/* gomp_hal_lock() - block until able to acquire lock "id" */
ALWAYS_INLINE void gomp_hal_lock(unsigned int *id) {
    //FIXME[ALE] Write an assembly code please!
    //Convert on TAS alias
    volatile unsigned int *tas_addr = (volatile unsigned int *)((unsigned int)id + 0x100000);
    while (*tas_addr == BUSY_LOCK);
}

/* gomp_hal_unlock() - release lock "id" */
ALWAYS_INLINE void gomp_hal_unlock(unsigned int *id) {
    *id = FREE_LOCK;
}

/* gomp_hal_init_lock () - get a lock */
//FIXME[ALE] check if it work and if it is complaint to the standard!!
void gomp_hal_init_lock(unsigned int *id);

/* gomp_hal_test_lock () - test a lock */
int gomp_hal_test_lock(unsigned int *id);

/* gomp_hal_destroy_lock () - destroys a lock (FIXME does nothing) */
void gomp_hal_destroy_lock(unsigned int *id);

/*********************************** standard APIs ***********************************************/

void omp_set_lock(omp_lock_t *lock);

void omp_unset_lock(omp_lock_t *lock);

void omp_init_lock(omp_lock_t *lock);

int omp_test_lock(omp_lock_t *lock);

void omp_destroy_lock(omp_lock_t *lock);

#endif