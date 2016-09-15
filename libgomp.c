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

#include "libgomp.h"

/*Application Entry Point*/
extern int main(int argc, char **argv, char **envp);

/* Private vars */
int _argc;
char **_argv;
char **_envp;

/* omp_initenv() - initialize environment & synchronization constructs */
ALWAYS_INLINE void
omp_initenv( )
{
    uint32_t i, nprocs;

    nprocs = get_num_procs();

    /* Init Thread Pool Information */
    gomp_set_thread_pool_idle_cores( nprocs - 1);
    gomp_set_thread_pool( 0x1U );
    gomp_thread_pool_lock_init();
    
    /* Init Curr Team for each threads Thread Pool Information */
    for(i = 0; i < nprocs; ++i)
        gomp_set_curr_team(i, (gomp_team_t *) NULL);

    /* Init Team Descriptor Pre-allocated Pool */
    gomp_team_pool_init();

    /* Init WS Descriptor Pre-allocated Pool */
    gomp_ws_pool_init();

    printf("Thread Pool Bitmask:\t%x (%x)\n", gomp_data.thread_pool_info.thread_pool, &(gomp_data.thread_pool_info.thread_pool));
    printf("Thread Pool #Idles: \t%d (%x)\n", gomp_data.thread_pool_info.idle_cores , &(gomp_data.thread_pool_info.idle_cores ));
    printf("Thread Pool Lock:   \t%x\n", &gomp_data.thread_pool_info.lock);
    printf("---- Check ---\n");
    printf("Thread Pool Bitmask:\t%x (%x)\n", GLOBAL_THREAD_POOL , &GLOBAL_THREAD_POOL );
    printf("Thread Pool #Idles: \t%d (%x)\n", GLOBAL_IDLE_CORES  , &GLOBAL_IDLE_CORES  );
    printf("Thread Pool Lock:   \t%x\n", GLOBAL_LOCK_ADDR);    
    return;
}


/* omp_SPMD_worker() - worker threads spin until work provided via GOMP_parallel_start() */
ALWAYS_INLINE int
omp_SPMD_worker()
{
    uint32_t i;
    int retval = 0;
    uint32_t pid = get_proc_id();
    
    if (pid == MASTER_ID)
    {
        gomp_team_t *root_team;
        
        /* Create "main" team descriptor. This also intializes master core's curr_team descriptor */
        gomp_master_region_start( NULL, NULL, 0x0, &root_team );

        /* wait all the threads */
        MSGBarrier_Wait_init(root_team->nthreads, root_team->proc_ids);

        /* Enter to the application Main */
        retval = main(_argc, _argv, _envp);

        /* Release All the Threads to conclude the runtime */
        for( i = 1; i < root_team->nthreads; ++i)
            gomp_set_curr_team(i, OMP_SLAVE_EXIT);
        MSGBarrier_hwRelease( root_team->team^(0x1<<root_team->proc_ids[0]) );

    } // MASTER
    else
    {
        MSGBarrier_SlaveEnter_init( pid );
        while (1)
        {
            volatile gomp_team_t *curr_team = (volatile gomp_team_t *) gomp_get_curr_team( pid );

            /* Exit runtime loop... */
            if ( curr_team ==  OMP_SLAVE_EXIT) 
            {
                // we are done!!
                break;
            }      
            /* Have work! */
            else
            {
                volatile task_f * omp_task_f;
                volatile int **omp_args;

#ifdef PROFILE0
                pulp_trace(TRACE_OMP_PARALLEL_ENTER);
#endif
                omp_task_f = (void*) (&curr_team->omp_task_f);
                omp_args = (void*) (&curr_team->omp_args);
                (**omp_task_f)((int) *omp_args);

#ifdef PROFILE0
                pulp_trace(TRACE_OMP_PARALLEL_EXIT);
#endif
            }
            MSGBarrier_hwSlaveEnter( curr_team->barrier_id );
        }
    }
    
    return retval;
}

/* main routine */
int
omp_init ()
{
    /* init HAL */
    gomp_hal_init();

    /* The MASTERs (one per-cluster) execute omp_initenv().. */
    if (get_proc_id() == MASTER_ID)
        omp_initenv();
    
    return omp_SPMD_worker();
}
