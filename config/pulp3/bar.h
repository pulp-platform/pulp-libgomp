/* Copyright 2014 DEI - Universita' di Bologna
 *   author       DEI - Universita' di Bologna
 *                Davide Rossi - davide.rossi@unibo.it
 *                Alessandro Capotondi - alessandro.capotondi@unibo.it
 *   info         Barrier Implementation */

//FIXME[ALE] Merge Barriers!
#ifndef __BAR_H__
#define __BAR_H__           

#include "appsupport.h"
#include "libgomp_config.h"

#define _DTYPE          int

#define BARRIER_BASE    LIBGOMP_BASE
#define DEFAULT_MAXPROC 4

#define MASTER_FLAG(x)                     (volatile _DTYPE *) ((unsigned int) BARRIER_BASE + (x<<2))
#define SLAVE_FLAG(x)                      (volatile _DTYPE *) ((unsigned int) BARRIER_BASE + (DEFAULT_MAXPROC<<2) + (x<<2))


typedef volatile _DTYPE * MSGBarrier;
extern void MSGBarrier_SlaveEnter(int myid,unsigned int master_id, int num_threads, unsigned int* barrier_lock,volatile unsigned int* barrier_counter,unsigned int barrier_id);
extern void MSGBarrier_Wait(int num_threads, unsigned int *local_slave_ids, unsigned int* barrier_lock, volatile unsigned int* barrier_counter,unsigned int barrier_id);
extern void MSGBarrier_Release(int numProcs, unsigned int *local_slave_ids, unsigned int team_mask);
extern void MSGBarrier_Wake(int numProcs, unsigned int *local_slave_ids);
extern void MSlaveBarrier_SlaveEnter_init(int myid);
extern void MSlaveBarrier_Wait_init(int num_threads, unsigned int *local_slave_ids);

#define MSlaveBarrier_Wait(num_threads,local_slave_ids,barrier_lock, barrier_counter,barrier_id)        MSGBarrier_Wait(num_threads,local_slave_ids,barrier_lock, barrier_counter,barrier_id)
#define MSlaveBarrier_SlaveEnter(myid,master_id,num_threads,barrier_lock, barrier_counter,barrier_id)  MSGBarrier_SlaveEnter(myid,master_id,num_threads,barrier_lock, barrier_counter,barrier_id)
#define MSlaveBarrier_Release_all(numProcs, procs_ids,mask) MSGBarrier_Release(numProcs, procs_ids,mask)
#define MSlaveBarrier_Release(numProcs, procs_ids,mask)     MSGBarrier_Release(numProcs, procs_ids,mask)

#define MS_BARRIER_TYPE                 MSGBarrier
#define MS_BARRIER_SIZE                 ((DEFAULT_MAXPROC * SIZEOF_WORD * 2) + (2*sizeof(int)))

extern void gomp_hal_barrier();

#endif
