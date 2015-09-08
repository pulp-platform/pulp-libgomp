/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
 C ontributed by Richard Henderson <r*th@redhat.com>.
 
 This file is part of the GNU OpenMP Library (libgomp).
 
 Libgomp is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.
 
 Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
 WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 more details.
 
 Under Section 7 of GPL version 3, you are granted additional
 permissions described in the GCC Runtime Library Exception, version
 3.1, as published by the Free Software Foundation.
 
 You should have received a copy of the GNU General Public License and
 a copy of the GCC Runtime Library Exception along with this program;
 see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 <http://www.gnu.org/licenses/>.  */

/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         #pragma omp parallel implementation */

#include "libgomp.h"
#include <cpu_hal.h>
#ifdef PULP3
#include "events.h"
#endif

void
GOMP_parallel_start (void *fn, void *data, int num_threads)
{
    /* The thread descriptor for slaves of the newly-created team */
    gomp_team_t *new_team;  
    
    gomp_team_start (fn, data, num_threads, &new_team);

    MSlaveBarrier_Release(new_team->nthreads, new_team->proc_ids, new_team->team);
}

void
GOMP_parallel_end (void)
{
    unsigned int myid;
    gomp_team_t *the_team;
    int barrier_id;

    myid = prv_proc_num;    
    the_team = (gomp_team_t *) CURR_TEAM(myid);

    barrier_id = (the_team->barrier_id);
    
    *(volatile int*) (SET_BARRIER_BASE+4*(barrier_id) ) = (the_team->nthreads<<16)+(1<<(myid) ); //set barrier
    *(volatile int*) (WAIT_BARRIER) =  barrier_id;
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    // Flush the pipeline
#ifdef __riscv__
  asm volatile ("WFI");
#else
  asm volatile ("l.psync");
#endif
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;

    gomp_team_end();
}

void
GOMP_parallel (void (*fn) (void*), void *data, int num_threads, unsigned int flags)
{
    /* The thread descriptor for slaves of the newly-created team */
    
    gomp_team_t *new_team;
    
#ifdef PROFILE0
#ifdef PROFILE1
    pulp_trace_perf(TRACE_OMP_PARALLEL_ENTER);
#else
    pulp_trace(TRACE_OMP_PARALLEL_ENTER);
#endif
#endif

    gomp_team_start (fn, data, num_threads, &new_team);

    MSlaveBarrier_Release(new_team->nthreads, new_team->proc_ids, new_team->team);

    fn(data);
    
#ifdef PROFILE0
#ifdef PROFILE1
    pulp_trace_perf(TRACE_OMP_PARALLEL_EXIT);
#else
    pulp_trace(TRACE_OMP_PARALLEL_EXIT);
#endif
#endif

    GOMP_parallel_end();


}

/* The public OpenMP API for thread and team related inquiries.  */

ALWAYS_INLINE int
omp_get_num_threads()
{
    return CURR_TEAM(prv_proc_num)->nthreads;
}

ALWAYS_INLINE int
omp_get_max_threads(void)
{
    return GLOBAL_IDLE_CORES + 1;
}

ALWAYS_INLINE int
omp_get_thread_num()
{
    unsigned int myid = prv_proc_num;
    return CURR_TEAM(myid)->thread_ids[myid];
}

ALWAYS_INLINE int
omp_in_parallel()
{
    unsigned int myid = prv_proc_num;
    return CURR_TEAM(myid)->level != 0;
}
