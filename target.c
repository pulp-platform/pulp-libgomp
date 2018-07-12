/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 * 
 * Authors: 
 *    Alessandro Capotondi, UNIBO, (alessandro.capotondi@unibo.it)
 */

/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
 * C ontributed by Richard Henderson <r*th@redhat.com>.
 * 
 * This file is part of the GNU OpenMP Library (libgomp).
 * 
 * Libgomp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 * 
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with this program;
 * see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/* This file handles the TARGET construct.  */

#include "libgomp.h"

// #define OMP_TARGET_DEBUG

#if PULP_CHIP == CHIP_HERO_Z_7045
void
GOMP_target (int device, void (*fn) (void *), const void *unused,
       size_t mapnum, void **hostaddrs, size_t *sizes,
       unsigned char *kinds)
{
  target_desc.fn = fn;
  target_desc.hostaddrs = hostaddrs;
  target_desc.nteams = 0x1U;
  target_desc.threadLimit = DEFAULT_TARGET_THREAD_LIMIT;

#ifdef OMP_TARGET_DEBUG  
  printf("[%d][%d][GOMP_target] fn 0x%x, data 0x%x, nteams %d, threadLimit %d\n", get_cl_id(), get_proc_id(), target_desc.fn, target_desc.hostaddrs, target_desc.nteams, target_desc.threadLimit);
#endif

  fn(hostaddrs);

#ifdef OMP_TARGET_DEBUG  
  printf("[%d][%d][GOMP_target] calling MSGBarrier_swDocking_Wait....\n", get_cl_id(), get_proc_id());
#endif
  MSGBarrier_swDocking_Wait(target_desc.nteams);
}

void
GOMP_teams (unsigned int num_teams, unsigned int thread_limit)
{
  if(get_cl_id() == MASTER_ID)
  {
    target_desc.nteams = num_teams > DEFAULT_TARGET_MAX_NTEAMS || num_teams == 0 ? DEFAULT_TARGET_MAX_NTEAMS : num_teams;
    target_desc.threadLimit = thread_limit > DEFAULT_TARGET_THREAD_LIMIT || thread_limit == 0 ? DEFAULT_TARGET_THREAD_LIMIT : thread_limit;

    for(uint32_t i = 1; i < target_desc.nteams; ++i)
      gomp_hal_hwTrigg_Team( i );
  }
  gomp_set_thread_pool_idle_cores( target_desc.threadLimit - 1);

  #ifdef OMP_TARGET_DEBUG  
  printf("[%d][%d][GOMP_teams] fn 0x%x, data 0x%x, nteams %d, threadLimit %d\n", get_cl_id(), get_proc_id(), target_desc.fn, target_desc.hostaddrs, target_desc.nteams, target_desc.threadLimit);
  #endif
}

int
omp_get_team_num(void)
{
  return get_cl_id();
}

int
omp_get_num_teams(void)
{
  return target_desc.nteams;
}

#if PULP_CHIP == CHIP_BIGPULP_Z_7045_O
#ifndef TRYX_LEGACY
/*******************************************************************************/

void GOMP_pulp_RAB_tryx_slowpath()
{
  int coreid = get_core_id();
  unsigned int mask = read_evnt_mask_low(coreid);

  // only listen to wake-up event
  set_evnt_mask_low(coreid, EVTMASK_RAB_WAKEUP);

  // go to sleep
  wait_event();

  // clear RAB miss event buffer
  clear_evnt_buff_low(EVTMASK_RAB_WAKEUP);

  // restore the event mask
  set_evnt_mask_low(coreid, mask);

  return;
}

int GOMP_pulp_RAB_tryread(unsigned int * addr)
{
  volatile unsigned int *tryx_ctrl = (unsigned int *)TRYX_CTRL_BASE_ADDR;
  unsigned int ret;
   
#if PROFILE_TRYX == 1
  int coreid = get_core_id();
  n_tryx[coreid]++;
#endif
 
#if DEBUG_TRYX == 1
#pragma omp critical
  printf("tryread to address 0x%X\n",(unsigned)addr);
#endif
   
  // actual tryread
  ret = *(volatile unsigned int*)addr;
 
  // check for a RAB miss
  if (*tryx_ctrl & 0x1) {
  
#if DEBUG_TRYX == 1
    printf("miss on address 0x%X\n",(unsigned)addr);
#endif

#if PROFILE_TRYX  == 1  
    start_core_timer(coreid);
    n_misses[coreid]++;
#endif
         
    // go to sleep
    GOMP_pulp_RAB_tryx_slowpath();
 
    // repeat tryread
    ret = *(volatile unsigned int*)addr;
 
#if PROFILE_TRYX == 1
    stop_core_timer(coreid); 
#endif 
  }
  
  return ret;
}

void GOMP_pulp_RAB_trywrite(unsigned int * addr, unsigned int value)
{
  volatile unsigned int *tryx_ctrl = (unsigned int *)TRYX_CTRL_BASE_ADDR;
 
#if PROFILE_TRYX == 1
  int coreid = get_core_id();
  n_tryx[coreid]++;
#endif

#if DEBUG_TRYX == 1
#pragma omp critical
  printf("trywrite to address 0x%X\n",(unsigned)addr);
#endif
 
  // actual trywrite
  *(volatile unsigned int*)addr = value;

  // check for a RAB miss
  if (*tryx_ctrl & 0x1) {

#if PROFILE_TRYX == 1  
    start_core_timer(coreid);
    n_misses[coreid]++;
#endif
    
    // go to sleep
    GOMP_pulp_RAB_tryx_slowpath();

    // repeat trywrite
    *(volatile unsigned int*)addr = value;

#if PROFILE_TRYX == 1  
    stop_core_timer(coreid);
#endif
  }

  return;
}

void GOMP_pulp_RAB_tryread_prefetch(unsigned int * addr)
{
  volatile unsigned int *tryx_ctrl = (unsigned int *)TRYX_CTRL_BASE_ADDR;
   
#if PROFILE_TRYX == 1
  int coreid = get_core_id();
  n_tryx[coreid]++;
#endif
 
#if DEBUG_TRYX == 1
#pragma omp critical
  printf("tryread to address 0x%X\n",(unsigned)addr);
#endif
  
  // trigger prefetch
  *tryx_ctrl = 0xFFFFFFFF; 

  // actual tryread
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  const volatile unsigned int ret = *(volatile unsigned int*)addr;
#pragma GCC diagnostic pop
 
  return;
}

/*******************************************************************************/
#else // TRYX_LEGACY
void GOMP_pulp_rab_tryread_slowpath()
{
  int coreid = get_core_id();

  // clear RAB miss event buffer 
  clear_evnt_buff_low(EVTMASK_AXI_RESP);

  // only listen to wake-up event
  set_evnt_mask_low(coreid, EVTMASK_RAB_WAKEUP_LEGACY);

  // go to sleep
  wait_event();

  // clear RAB miss event buffer
  clear_evnt_buff_low(EVTMASK_RAB_WAKEUP_LEGACY);

  return;
}

int GOMP_pulp_rab_tryread(unsigned int * addr)
{
 
  int coreid = get_core_id();
  volatile unsigned int ret;

#if PROFILE_TRYX == 1
  n_tryx[coreid]++;
#endif

#if DEBUG_TRYX == 1
  if ( !((unsigned)addr & 0xFFFFF000) ) {
#pragma omp critical
    printf("tryread to address 0x%X\n",(unsigned)addr);
  }
#endif

  // only listen to axi_resp_event
  set_evnt_mask_low(coreid, EVTMASK_AXI_RESP);

  // actual try read
  ret = *addr;

  // check for a RAB miss event
  if (read_evnt_buff_low(coreid) & EVTMASK_AXI_RESP) {

#if PROFILE_TRYX == 1
    n_misses[coreid]++;
    start_core_timer(coreid);
#endif

    GOMP_pulp_rab_tryread_slowpath();

#if PROFILE_TRYX == 1
    stop_core_timer(coreid);
#endif
  }

  // re-enable barrier
  set_evnt_mask_low(coreid, EVTMASK_BARRIER);

  return ret;
}
#endif
#endif

#endif /* CHIP_HERO_Z_7045 */

