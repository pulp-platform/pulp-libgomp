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
   info         Libgomp main entry point */

#include "pulp.h"

/*Application Entry Point*/
extern int main(int argc, char **argv, char **envp);

/* Private vars */
int _argc;
char **_argv;
char **_envp;

static void omp_initenv(int, int);
static int omp_SPMD_worker(int);

#include <hwTrace.h>

#ifdef RISCV
#define PCER_ALL_EVENTS_MASK CSR_PCER_ALL_EVENTS_MASK
#define PCMR_ACTIVE CSR_PCMR_ACTIVE
#define PCMR_SATURATE CSR_PCMR_SATURATE
#else
#define PCER_ALL_EVENTS_MASK SPR_PCER_ALL_EVENTS_MASK
#define PCMR_ACTIVE SPR_PCMR_ACTIVE
#define PCMR_SATURATE SPR_PCMR_SATURATE
#endif

static inline void perfInitAndStart()
{
#ifdef PROFILE0
  cpu_perf_conf_events(PCER_ALL_EVENTS_MASK);
  cpu_perf_setall(0);
  cpu_perf_conf(PCMR_ACTIVE | PCMR_SATURATE);
#endif
}

/* main routine */
int
omp_init ()
{
    int id = get_proc_id() - 1;
    int procs = get_proc_num();


    //TODO wrap this ifdef inside a function call
#if EU_VERSION == 1
    set_evnt_mask_low(id,1); //configure the event mask
#else
    eu_evt_maskSet(1<<0);
#endif

    perfInitAndStart();
    
    /* The MASTER executes omp_initenv().. */
    if (id == MASTER_ID)
      omp_initenv(procs, id);
    return omp_SPMD_worker(id);  
}

/* omp_initenv() - initialize environment & synchronization constructs */
static void
omp_initenv(int nprocs, int pid)
{  
    gomp_team_t * root_team;
    int i;
    
    shmalloc_init();
        
    GLOBAL_IDLE_CORES = nprocs - 1;
    GLOBAL_THREAD_POOL = (1 << MASTER_ID);
        
    //Reset global info lock
    GLOBAL_INFOS_SIGNAL();
    for(i=0; i<nprocs; i++){
        CURR_TEAM(i) = (gomp_team_t *) NULL;
    }
    
    #ifndef __NO_OMP_PREALLOC__
    /* Init preallocated team pool descriptor*/
    TEAMMEM_FREE_MEM = (void *)((unsigned int) TEAMMEM_ADDR);
    TEAMMEM_FREE_LIST = 0;

    unsigned int new_team;
    unsigned int team_id;
    for(new_team = (unsigned int)TEAMMEM_FREE_MEM, team_id = NUM_HW_BARRIER-1; new_team < TEAMMEM_LIMIT; new_team += sizeof(gomp_team_t), --team_id) {
        ((gomp_team_t *)new_team)->barrier_id = team_id;
    }
    
    //Reset team descriptor pool lock
    TEAMMEM_LOCK_SIGNAL();
    
    /* Init preallocated ws pool*/
    WSMEM_FREE_MEM = (void *)((unsigned int) WSMEM_ADDR);
    WSMEM_FREE_LIST = 0;
    
    //Reset workshare descriptor pool lock
    WSMEM_LOCK_SIGNAL();
    #endif
    
    #ifdef OMP_DEBUG
    _printstrn ("----------- OpenMP Environment ------------");
    _printstrn ("------------- Address check ---------------");
    _printstrn ("");
    printf("BARRIER_BASE      %x\n", BARRIER_BASE);   
    printf("GLOBAL_INFOS_BASE %x\n", GLOBAL_INFOS_BASE);
    printf("GLOBAL_INFOS_LOCK %x\n", GLOBAL_LOCK);
    printf("SHMEM_NEXT        %x\n", SHMEM_NEXT);
    printf("SHMEM_LOCK        %x\n", SHMEM_LOCK);
    printf("CURR_TEAM_ADDR    %x\n", CURR_TEAM_ADDR);
    #ifndef __NO_OMP_PREALLOC__
    printf("TEAMMEM           %x\n", TEAMMEM_ADDR);
    printf("TEAMMEM_LOCK      %x\n", TEAMMEM_LOCK);
    printf("TEAMMEM_LIMIT     %x\n", TEAMMEM_LIMIT);
    printf("WSMEM             %x\n", WSMEM_ADDR);
    printf("WSMEM_LOCK        %x\n", WSMEM_LOCK);
    printf("WSMEM_LIMIT       %x\n", WSMEM_LIMIT);
    #endif
    #endif
    
    /* Create "main" team descriptor. This also intializes master core's curr_team descriptor */
    gomp_master_region_start(main, NULL, 1, &root_team);
}

/* parallel execution */

#define OMP_SLAVE_EXIT 0xfeedbeef

/* omp_SPMD_worker() - worker threads spin until work provided via GOMP_parallel_start() */
static int
omp_SPMD_worker(int myid)
{
    /* For slaves */
    volatile task_f * omp_task_f;
    volatile int **omp_args;
    int i, nprocs;
    int retval = 0;
    
    nprocs = prv_num_procs;
    unsigned int timer;
    
    if (myid == MASTER_ID)
    {   
        MSlaveBarrier_Wait_init(nprocs, (unsigned int *) CURR_TEAM(myid)->proc_ids);
        
        retval = main(_argc, _argv, _envp);

        for(i=1; i<nprocs; i++)
            CURR_TEAM(i) = (gomp_team_t *) OMP_SLAVE_EXIT;
        
        /* We release slaves inside gomp_parallel_end() */
        MSlaveBarrier_Release(nprocs, (unsigned int *) CURR_TEAM(myid)->proc_ids, (1<<nprocs)-1);
    } // MASTER
    else
    {
        MSlaveBarrier_SlaveEnter_init(myid);
        
        while (1)
        {
            /* Exit runtime loop... */
            if ( (volatile unsigned int) CURR_TEAM(myid) ==  OMP_SLAVE_EXIT) 
            {
                // we are done!!
                break;
            }      
            /* Have work! */
            else
            {
#ifdef PROFILE0
	        pulp_trace(TRACE_OMP_PARALLEL_ENTER);
#endif
                omp_task_f = (void*) (&CURR_TEAM(myid)->omp_task_f);
                omp_args = (void*) (&CURR_TEAM(myid)->omp_args);
                (**omp_task_f)((int) *omp_args);
#ifdef PROFILE0
		pulp_trace(TRACE_OMP_PARALLEL_EXIT);
#endif
            } // ! omp_task_f
	    
            MSlaveBarrier_SlaveEnter(myid, (unsigned int ) CURR_TEAM(myid)->proc_ids[0], (unsigned int ) CURR_TEAM(myid)->nthreads, (unsigned int* ) &(CURR_TEAM(myid)->barrier_lock), (unsigned int* ) &(CURR_TEAM(myid)->barrier_counter),(unsigned int ) CURR_TEAM(myid)->barrier_id);
        } // while 
    } // if master/slave
    
    return retval;
} // omp_worker

/******************************************************************************/

int
omp_get_num_procs(void)
{
	return prv_num_procs;
}
