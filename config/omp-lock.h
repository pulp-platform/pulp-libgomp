/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         OpenMP lock set and unset */

//FIXME[ALE] Merge lock stuff!
#ifndef __OMP_LOCK_H__
#define __OMP_LOCK_H__

typedef unsigned int omp_lock_t;

void omp_set_lock (omp_lock_t *lock);
void omp_unset_lock (omp_lock_t *lock);

#endif
