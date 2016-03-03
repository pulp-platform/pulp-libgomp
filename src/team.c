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

ALWAYS_INLINE gomp_team_t *
gomp_new_team()
{
    gomp_team_t * new_team;

    #ifdef STATS_ENABLE
    stop_timer();
    timers[14] = get_time();
    start_timer();
    #endif  

    #ifndef __NO_OMP_PREALLOC__
    TEAMMEM_LOCK_WAIT();
    new_team = (gomp_team_t *) TEAMMEM_FREE_MEM;
    if((unsigned int) new_team < TEAMMEM_LIMIT)
        TEAMMEM_FREE_MEM += sizeof(gomp_team_t);
    else
    {
        new_team = TEAMMEM_FREE_LIST;
        if(new_team)
            TEAMMEM_FREE_LIST = new_team->next;
        else
        {           
            #ifndef __NO_OMP_MALLOC_ON_PREALLOC__
            new_team = (gomp_team_t *)shmalloc(sizeof(gomp_team_t));
            //_printstrp("[LIBGOMP] Warning. Dynamic Team allocation needed.");
            #else
            _printdecp("[LIBGOMP] Error. TEAM MEMORY is out of Memory. Max number of teams allowed in a single cluster is ", MAX_TEAM_DESC);
            abort();
            #endif
        }
    }
    TEAMMEM_LOCK_SIGNAL();
    #else
    new_team = (gomp_team_t *)shmalloc(sizeof(gomp_team_t));
    #endif

    #ifdef STATS_ENABLE
    stop_timer();
    timers[15] = get_time();
    start_timer();
    #endif
        
    return new_team;
}

ALWAYS_INLINE void
gomp_free_team(gomp_team_t * team)
{    
    #ifndef __NO_OMP_PREALLOC__
    TEAMMEM_LOCK_WAIT();
    team->next = TEAMMEM_FREE_LIST;
    TEAMMEM_FREE_LIST = team;
    TEAMMEM_LOCK_SIGNAL();
    #endif  
}

/* Safely get-and-decrement at most 'specified' free processors */
inline int
gomp_resolve_num_threads (int specified)
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

inline void
gomp_master_region_start (void *fn, void *data, int specified, gomp_team_t **team)
{
    unsigned int i, nprocs, myid, local_id_gen, num_threads,
    curr_team_ptr, my_team_ptr;
    unsigned long long mask;
    gomp_team_t *new_team, *parent_team;
    
    nprocs = prv_num_procs;
    myid = prv_proc_num;
    
    curr_team_ptr = (unsigned int) CURR_TEAM_PTR(0);
    /* Proc 0 calls this... */
    my_team_ptr = curr_team_ptr;
    
    /* Fetch free processor(s) */
    GLOBAL_INFOS_WAIT();
    
    num_threads = gomp_resolve_num_threads (specified);
    
    /* Create the team descriptor for current parreg */
    new_team = gomp_new_team();
    
    new_team->omp_task_f = (void *)(fn);
    new_team->omp_args = data;
    new_team->nthreads = num_threads; // also the master
    
    new_team->team = (1 << num_threads)-1; //originally was 0xFFFF, in order to work well with event also 0xF should be fine;
    
    /* Use the global array to fetch idle cores */
    local_id_gen = 1; // '0' is master
    
    num_threads--; // Decrease NUM_THREADS to account for the master
    new_team->thread_ids[myid] = 0;
    new_team->proc_ids[0] = myid;

    set_barrier(new_team->barrier_id,new_team->nthreads,new_team->team,new_team->team);

    // to make SW barrier work
    for(i=1; i<prv_num_procs; i++)
    {
        new_team->proc_ids[local_id_gen] = i;
        new_team->thread_ids[i] = local_id_gen++;
    }
    GLOBAL_INFOS_SIGNAL();
    new_team->level = 0;
    
    new_team->parent = 0x0;
    *((gomp_team_t **) my_team_ptr) = new_team;
    *team = new_team;
}

ALWAYS_INLINE void
gomp_team_start (void *fn, void *data, int specified, gomp_team_t **team) 
{
    unsigned int i, nprocs, myid, local_id_gen, num_threads,
    curr_team_ptr, my_team_ptr;
    unsigned /*long long*/ int mask;
    gomp_team_t *new_team, *parent_team;
    
    nprocs = prv_num_procs;
    myid = prv_proc_num;
    
    curr_team_ptr = (unsigned int) CURR_TEAM_PTR(0);
    my_team_ptr = (unsigned int) CURR_TEAM_PTR(myid);
        
    /* Fetch free processor(s) */
    GLOBAL_INFOS_WAIT();
    
    num_threads = gomp_resolve_num_threads (specified);
    
    /* Create the team descriptor for current parreg */
    new_team = gomp_new_team();
    new_team->omp_task_f = (void *)(fn);
    new_team->omp_args = data;
    new_team->nthreads = num_threads; // also the master
    
    new_team->team = 0x0;
    
    /* Use the global array to fetch idle cores */
    local_id_gen = 1; // '0' is master
    
    num_threads--; // Decrease NUM_THREADS to account for the master
    new_team->team |= (1 << myid);
    new_team->thread_ids[myid] = 0;
    new_team->proc_ids[0] = myid;
    
    /*Reset the locks */
    //gomp_hal_unlock(&new_team->critical_lock);
    //gomp_hal_unlock(&new_team->atomic_lock);
    //gomp_hal_unlock(&new_team->barrier_lock);
    new_team->critical_lock = 0x0;
    new_team->atomic_lock   = 0x0;
    
    /* Init default work share */  
    gomp_work_share_t *root_ws = (gomp_work_share_t *) gomp_new_work_share();
    
    #ifdef __OMP_SINGLE_WS__
    new_team->work_share = root_ws;
    #else
    new_team->work_share[myid] = root_ws;
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

            #ifndef __OMP_SINGLE_WS__
            new_team->work_share[i] = root_ws;
            #endif
            
            /* How many left? */
            num_threads--;
        } // if
    } // for
    
    if(new_team->barrier_id!=0xFF)
        set_barrier(new_team->barrier_id,new_team->nthreads,new_team->team,new_team->team);
    else
    {
        new_team->barrier_counter = num_threads; 
        new_team->barrier_lock  = 0x0;
    }

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
    unsigned int i, neg_mask, myid, nthreads, *ids;
    gomp_team_t *the_team;
    
    myid = prv_proc_num;
    
    the_team = (gomp_team_t *) CURR_TEAM(myid);
    
    ids = the_team->thread_ids;
    neg_mask =~ the_team->team;
    
    neg_mask |= (1 << myid);
    nthreads = the_team->nthreads;
    
    GLOBAL_INFOS_WAIT();
    GLOBAL_IDLE_CORES += (nthreads - 1); // free all but master
    GLOBAL_THREAD_POOL &= neg_mask;
    GLOBAL_INFOS_SIGNAL();
    
    /* After this, the current parallel thread will be lost. Free...if we had any for MPARM */
    CURR_TEAM(myid) = the_team->parent;
    
    #ifdef __OMP_SINGLE_WS__
    gomp_free_work_share(the_team->work_share);
    #else
    gomp_free_work_share(the_team->work_share[myid]);
    #endif
    
    gomp_free_team(the_team);
    
} // gomp_team_end
