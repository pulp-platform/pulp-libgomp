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
   info         parallel thread team implementation */

#include "libgomp.h"

/*** Team Descriptor Pool APIs ***/

ALWAYS_INLINE void
gomp_team_pool_lock_init ( )
{
    gomp_data.team_pool_lock = (omp_lock_t) 0x0U;
}

ALWAYS_INLINE void
gomp_team_pool_lock ( )
{
    gomp_hal_lock(&gomp_data.team_pool_lock);
}

ALWAYS_INLINE void
gomp_team_pool_unlock ( )
{
    gomp_hal_unlock(&gomp_data.team_pool_lock);
}

ALWAYS_INLINE gomp_team_t *
gomp_get_team_from_pool ( uint32_t id )
{
    return &(gomp_data.team_descriptor_pool[id]);
}

ALWAYS_INLINE void
gomp_team_pool_init ( )
{
    uint32_t i;
    gomp_team_t *team;

    /* Check Gomp Memory Integrity */
    //NOTE: in aliased architecture it has some issue...
    //gomp_assert(((uint32_t) &(gomp_data) == (uint32_t) LIBGOMP_BASE));

    gomp_team_pool_lock_init( );
    
    /* Get First Team */
    team = gomp_get_team_from_pool(0x0);
    team->next = (gomp_team_t *) NULL;
    team->barrier_id = 0x0U;

    /* Use the First Team as list top */
    gomp_data.team_pool_list = team;

    for(i = 1U; i < MAX_TEAM_DESC; ++i)
    {
        team = gomp_get_team_from_pool(i);
        team->barrier_id = i;
        team->root_ws.embedded = WS_EMBEDDED;
        team->next = gomp_data.team_pool_list;
        gomp_data.team_pool_list = team;
    }
}

ALWAYS_INLINE void
gomp_push_team_pool ( gomp_team_t *team )
{
    gomp_hal_lock(&gomp_data.team_pool_lock);
    team->next = gomp_data.team_pool_list;
    gomp_data.team_pool_list = team;
    gomp_hal_unlock(&gomp_data.team_pool_lock);
}

ALWAYS_INLINE gomp_team_t *
gomp_pull_team_pool ( )
{
    gomp_team_t *team = (gomp_team_t *) NULL;;

    gomp_hal_lock(&gomp_data.team_pool_lock);
    team = gomp_data.team_pool_list;
    if(team != NULL)
        gomp_data.team_pool_list = team->next;
    gomp_hal_unlock(&gomp_data.team_pool_lock);
    
    return team;
}

//FIXME
#if 0
ALWAYS_INLINE gomp_team_t *
gomp_malloc_team()
{
    gomp_team_t *new_team
    new_team = (gomp_team_t *) shmalloc( sizeof(gomp_team_t) );
    new_team->next = NULL;
    return new_team;
}
#endif

ALWAYS_INLINE gomp_team_t *
gomp_new_team ( )
{
    gomp_team_t *new_team;

    new_team = gomp_pull_team_pool();
    gomp_assert(new_team != (gomp_team_t *) NULL);
    return new_team;
}

//FIXME Actually this method do not free the exeeded teams. In future 
ALWAYS_INLINE void
gomp_free_team ( gomp_team_t * team )
{
    return gomp_push_team_pool(team);
}

/* Safely get-and-decrement at most 'specified' free processors */
ALWAYS_INLINE int
gomp_resolve_num_threads ( int specified )
{ 
  int nthr;
  
  nthr = GLOBAL_IDLE_CORES + 1;
  
  /* If a number of threads has been specified by the user
   * and it is not bigger than max idle threads use that number
   */
  if (specified && (specified < nthr))
    nthr = specified;
  
  GLOBAL_IDLE_CORES -= (nthr - 1);
  
  return nthr;
}

ALWAYS_INLINE void
gomp_master_region_start ( __attribute__((unused)) void *fn,
                           __attribute__((unused)) void *data,
                           __attribute__((unused)) int specified,
                                                   gomp_team_t **team)
{
    uint32_t i, nprocs;
    gomp_team_t *new_team;

    nprocs    = get_num_procs();
    
    /* Create the team descriptor for current parreg */
    new_team = gomp_new_team();

    new_team->nthreads = nprocs;
    new_team->team = 0xFFU;

    for (i = 0; i < nprocs; ++i)
        new_team->proc_ids[i] = i;
    
    /* Update Team */
    new_team->parent = ( gomp_team_t * ) NULL;
    gomp_set_curr_team(0, new_team);
    *team = new_team;
}

ALWAYS_INLINE void
gomp_team_start (void *fn, void *data, int specified, gomp_team_t **team) 
{
    unsigned int i, nprocs, pid, local_id_gen, num_threads,
    curr_team_ptr, my_team_ptr;
    unsigned /*long long*/ int mask;
    gomp_team_t *new_team, *parent_team;
    
    nprocs = get_num_procs();
    pid = get_proc_id();
    
    curr_team_ptr = (unsigned int) CURR_TEAM_PTR(0);
    my_team_ptr = (unsigned int) CURR_TEAM_PTR(pid);
        
    /* Fetch free processor(s) */
    GLOBAL_INFOS_WAIT();
    
    num_threads = gomp_resolve_num_threads (specified);
    
    /* Create the team descriptor for current parreg */
    new_team = gomp_new_team();
    gomp_assert(new_team != (gomp_team_t *) NULL);

    new_team->omp_task_f = (void *)(fn);
    new_team->omp_args = data;
    new_team->nthreads = num_threads; // also the master

    new_team->team = 0x0U;
    
    /* Use the global array to fetch idle cores */
    local_id_gen = 1; // '0' is master
    
    num_threads--; // Decrease NUM_THREADS to account for the master
    new_team->team |= (1 << pid);
    new_team->thread_ids[pid] = 0;
    new_team->proc_ids[0] = pid;
    
    /*Reset the locks */
    //gomp_hal_unlock(&new_team->critical_lock);
    //gomp_hal_unlock(&new_team->atomic_lock);
    //gomp_hal_unlock(&new_team->barrier_lock);
    new_team->critical_lock = 0x0;
    new_team->atomic_lock   = 0x0;
    
    /* Init default work share */  
    gomp_work_share_t *root_ws = &(new_team->root_ws);
    root_ws->lock       = 0x0;
    root_ws->enter_lock = 0x0;
    root_ws->exit_lock  = 0x0;
    
    root_ws->next_ws = NULL;
    root_ws->prev_ws = NULL;
    
#ifdef OMP_NOWAIT_SUPPORT
    new_team->work_share[pid] = root_ws;
#else
    new_team->work_share = root_ws;
#endif
    
    unsigned int *gtpool = (unsigned int *) (GLOBAL_INFOS_BASE);
    
    for( i=1, mask = 2, curr_team_ptr += 4; /* skip p0 (global master) */
         i<nprocs && num_threads;
         i++, mask <<= 1, curr_team_ptr += 4) 
    {       
        if(!( *gtpool & mask))
        {
            *gtpool |= mask;
            
            new_team->team |= mask;
            
            new_team->proc_ids[local_id_gen] = i;
            
            new_team->thread_ids[i] = local_id_gen++;
            
            /* Update local team structure pointers of all processors of the team */
            *((gomp_team_t **) curr_team_ptr) = new_team;

#ifdef OMP_NOWAIT_SUPPORT
            new_team->work_share[i] = root_ws;
#endif

            /* How many left? */
            num_threads--;
        } // if
    } // for
    
    gomp_hal_set_hwBarrier(new_team->barrier_id, new_team->nthreads, new_team->team);
    GLOBAL_INFOS_SIGNAL();
    
    /* Update the parent team field */
    parent_team = *(gomp_team_t **) my_team_ptr;
    
    new_team->level = parent_team->level + 1;
    
    new_team->parent = parent_team;
    *((gomp_team_t **) my_team_ptr) = new_team;
    *team = new_team;
}

/* End team and destroy team descriptor */
ALWAYS_INLINE void
gomp_team_end()
{
    unsigned int i, neg_mask, pid, nthreads;
    gomp_team_t *the_team;
    
    pid = get_proc_id();
    the_team = (gomp_team_t *) CURR_TEAM(pid);
    neg_mask =~ the_team->team;
    
    neg_mask |= (1 << pid);
    nthreads = the_team->nthreads;
    
    GLOBAL_INFOS_WAIT();
    GLOBAL_IDLE_CORES += (nthreads - 1); // free all but master
    GLOBAL_THREAD_POOL &= neg_mask;
    GLOBAL_INFOS_SIGNAL();
    
    /* After this, the current parallel thread will be lost. Free...if we had any for MPARM */
    CURR_TEAM(pid) = the_team->parent;
    
#ifndef OMP_NOWAIT_SUPPORT      
    gomp_free_work_share(the_team->work_share);
#else
    gomp_free_work_share(the_team->work_share[pid]);
#endif
    
    gomp_free_team(the_team);
    
} // gomp_team_end
