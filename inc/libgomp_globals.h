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
   info         Libgomp support data structures declaration */

#ifndef __LIBGOMP_GLOBALS_H__
#define __LIBGOMP_GLOBALS_H__

#include "appsupport.h"

#ifdef PULP3
#define NUM_HW_BARRIER 6
#endif

#define GOMP_WARN_NOT_SUPPORTED(what) qprintf("[libGOMP] " what " is not supported yet.", 0, 0, 0, 0);

/* Platform Information */
#define prv_proc_num (get_proc_id() - 1)
#define prv_num_procs (get_proc_num())

#include "libgomp_config.h"
#include "omp-lock.h"

/* task function type */
typedef void (*task_f)(int);

/* Threads/tasks support */
typedef struct global_infos_s
{
    /* NOTE If you change ANY dimension of these fields you have also to update macros below here */
    unsigned int thread_pool; //Busy = 1, available = 0. Big endian
    unsigned int idle_cores;
    omp_lock_t   lock;
} global_infos_t;

/* Threads/tasks support */
/* This struct encapsulate a generic work-share
 * (e.g. dynamic loop, sections, etc)
 */
typedef struct gomp_work_share_s
{
    /* This is the "business" lock for the WS */
    omp_lock_t lock;
    int end;
    int next;
    int chunk_size;
    int incr;
    
    /* These locks are to decouple enter phase (and exit phase)
     * from the "business" phase. If only one WS is defined,
     * they are the same lock (see gomp_new_work_share()) */
    omp_lock_t enter_lock;
    //FIXME should be removed
    int checkfirst;
    omp_lock_t exit_lock;
    unsigned int completed;
    
    /* This ptr point to next_ws in case you use NOWAIT constructs */
    struct gomp_work_share_s *next_ws;
    struct gomp_work_share_s *prev_ws;
    
    #ifndef __NO_OMP_PREALLOC__    
    /*This field indicates the next team descriptor on the pool of preallocated descriptor, when the current
     is not used. **/
    struct gomp_work_share_s *next_free;
    #endif  
} gomp_work_share_t;

#define WS_INITED     (0xfeeddead)
#define WS_NOT_INITED (0x0)

/* This structure contains all of the thread-local data associated with 
 * a thread team.  This is the data that must be saved when a thread
 * encounters a nested PARALLEL construct.
 */
typedef struct gomp_team_s
{
    /****************** 1) Thread Info ****************************/
    /* This is the function that any thread
     * in the team must run upon launch. */
    void (*omp_task_f) (void *data);
    void *omp_args;
    
    /* Nesting level.  */
    unsigned level;
    struct gomp_team_s *parent;
    
    /******************** 2) Team Info ****************************/
    /* This is the number of threads in the current team.  */
    unsigned nthreads;
    
    /* This is the team descriptor/mask */
    unsigned int team;
    
    /* These are the local ids assigned to processors */
    unsigned int proc_ids[MAX_PARREG_THREADS];
    unsigned int thread_ids[DEFAULT_MAXPROC];
    volatile unsigned int barrier_counter;
    
    /******************** 3) Work Info ****************************/
    /* This is the task that the thread is currently executing.  */
    /* Team (parreg) specific locks */
    omp_lock_t critical_lock;
    omp_lock_t atomic_lock;
    omp_lock_t barrier_lock;
#ifdef PULP3
    unsigned int barrier_id;
#endif
    
    #ifdef __OMP_SINGLE_WS__
    gomp_work_share_t *work_share;
    #else
    gomp_work_share_t *work_share[DEFAULT_MAXPROC];
    #endif
    
    #ifndef __NO_OMP_PREALLOC__    
    /*This field indicates the next team descriptor on the pool of preallocated descriptor, when the current
     is* not used. */
    struct gomp_team_s *next;
    #endif  
} gomp_team_t;

typedef struct {
} omp_t;



/* Statically allocated global variables
 * (to avoid declaring a "real" global variable */

#define GLOBAL_INFOS_BASE       (LIBGOMP_BASE + MS_BARRIER_SIZE)
#define GLOBAL_INFOS_SIZE       (sizeof(global_infos_t))

#define GLOBAL_THREAD_POOL      (*((unsigned int*) (GLOBAL_INFOS_BASE)))
#define GLOBAL_IDLE_CORES_ADDR  (GLOBAL_INFOS_BASE + SIZEOF_UNSIGNED)
#define GLOBAL_IDLE_CORES       (*((unsigned int *) GLOBAL_IDLE_CORES_ADDR))
#define GLOBAL_LOCK_ADDR        (GLOBAL_INFOS_BASE + SIZEOF_UNSIGNED + SIZEOF_PTR)
#define GLOBAL_LOCK             ((unsigned int *) GLOBAL_LOCK_ADDR)

#define GLOBAL_INFOS_WAIT()     gomp_hal_lock(GLOBAL_LOCK)
#define GLOBAL_INFOS_SIGNAL()   gomp_hal_unlock(GLOBAL_LOCK)

#define SHMEM_NEXT_ADDR         (GLOBAL_INFOS_BASE + GLOBAL_INFOS_SIZE)
#define SHMEM_NEXT              (*((unsigned int*) SHMEM_NEXT_ADDR))
#define SHMEM_NEXT_SIZE         (SIZEOF_UNSIGNED)

#define SHMEM_LOCK_ADDR         (SHMEM_NEXT_ADDR + SHMEM_NEXT_SIZE)
#define SHMEM_LOCK              ((unsigned int *)SHMEM_LOCK_ADDR)
#define SHMEM_LOCK_SIZE         (SIZEOF_UNSIGNED)
#define SHMEM_LOCK_WAIT()       gomp_hal_lock(SHMEM_LOCK)
#define SHMEM_LOCK_SIGNAL()     gomp_hal_unlock(SHMEM_LOCK)

#define CURR_TEAM_ADDR          (SHMEM_LOCK_ADDR + SHMEM_LOCK_SIZE)
#define CURR_TEAM_PTR(_id)      ((gomp_team_t *volatile*) (CURR_TEAM_ADDR + (_id << 2)))
#define CURR_TEAM(_id)          (*CURR_TEAM_PTR(_id))
#define CURR_TEAM_SIZE          (SIZEOF_PTR * DEFAULT_MAXPROC)

#ifndef __NO_OMP_PREALLOC__

#define MAX_TEAM_DESC                   6
#define MAX_WS                          12

//Team Descriptor Pre-allocated Pool
#define TEAMMEM_LOCK_ADDR               (CURR_TEAM_ADDR + CURR_TEAM_SIZE)
#define TEAMMEM_LOCK                    ((unsigned int *) (TEAMMEM_LOCK_ADDR))
#define TEAMMEM_LOCK_SIZE               (SIZEOF_PTR)
#define TEAMMEM_LOCK_WAIT()             gomp_hal_lock(TEAMMEM_LOCK)
#define TEAMMEM_LOCK_SIGNAL()           gomp_hal_unlock(TEAMMEM_LOCK)

#define TEAMMEM_FREE_MEM_ADDR           (TEAMMEM_LOCK_ADDR + TEAMMEM_LOCK_SIZE)
#define TEAMMEM_FREE_MEM                (*((void **) (TEAMMEM_FREE_MEM_ADDR)))
#define TEAMMEM_FREE_MEM_SIZE           (SIZEOF_PTR)

#define TEAMMEM_FREE_LIST_ADDR          (TEAMMEM_FREE_MEM_ADDR + TEAMMEM_FREE_MEM_SIZE )
#define TEAMMEM_FREE_LIST               (*((gomp_team_t  **) (TEAMMEM_FREE_LIST_ADDR)))
#define TEAMMEM_FREE_LIST_SIZE          (SIZEOF_PTR)

#define TEAMMEM_ADDR                    (TEAMMEM_FREE_LIST_ADDR + TEAMMEM_FREE_LIST_SIZE )
#define TEAMMEM                         (*((char *) (TEAMMEM_MEM_ADDR)))
#define TEAMMEM_SIZE                    (sizeof(gomp_team_t) * MAX_TEAM_DESC)
#define TEAMMEM_LIMIT                   (TEAMMEM_ADDR + TEAMMEM_SIZE)

//Workshare Pool Pre-allocated Pool
#define WSMEM_LOCK_ADDR                 (TEAMMEM_ADDR + TEAMMEM_SIZE)
#define WSMEM_LOCK                      ((unsigned int *) (WSMEM_LOCK_ADDR))
#define WSMEM_LOCK_SIZE                 (SIZEOF_PTR)
#define WSMEM_LOCK_WAIT()               gomp_hal_lock(WSMEM_LOCK)
#define WSMEM_LOCK_SIGNAL()             gomp_hal_unlock(WSMEM_LOCK)

#define WSMEM_FREE_MEM_ADDR             (WSMEM_LOCK_ADDR + WSMEM_LOCK_SIZE)
#define WSMEM_FREE_MEM                  (*((void **) (WSMEM_FREE_MEM_ADDR)))
#define WSMEM_FREE_MEM_SIZE             (SIZEOF_PTR)

#define WSMEM_FREE_LIST_ADDR            (WSMEM_FREE_MEM_ADDR + WSMEM_FREE_MEM_SIZE )
#define WSMEM_FREE_LIST                 (*((gomp_work_share_t  **) (WSMEM_FREE_LIST_ADDR)))
#define WSMEM_FREE_LIST_SIZE            (SIZEOF_PTR)

#define WSMEM_ADDR                      (WSMEM_FREE_LIST_ADDR + WSMEM_FREE_LIST_SIZE )
#define WSMEM                           (*((char *) (WSMEM_ADDR)))
#define WSMEM_SIZE                      (sizeof(gomp_work_share_t) * MAX_WS)
#define WSMEM_LIMIT                     (WSMEM_ADDR + WSMEM_SIZE)

#define STATIC_TCDM_SIZE                (WSMEM_LIMIT-LIBGOMP_BASE)
#else

#define STATIC_TCDM_SIZE                (MS_BARRIER_SIZE + GLOBAL_INFOS_SIZE + SHMEM_NEXT_SIZE + SHMEM_LOCK_SIZE + LOCKMEM_NEXT_SIZE + CURR_TEAM_SIZE)
#endif


#ifdef __OMP_SINGLE_WS__
#define CURR_WS(_id)                    (((gomp_team_t *) CURR_TEAM(_id))->work_share)
#else
#define CURR_WS(_id)                    (((gomp_team_t *) CURR_TEAM(_id))->work_share[(_id)])
#endif

#endif // __LIBGOMP_GLOBALS_H__
