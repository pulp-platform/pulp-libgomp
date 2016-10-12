/* Copyright (C) 2013-2016 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.
   This file is part of the GNU Offloading and Multi Processing Library
   (libgomp).
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
                Alessandro Capotondi - alessandro.capotondi@unibo.it */

/* This file contains the support of offloading.  */

#include "libgomp.h"

void
GOMP_target (int device, void (*fn) (void *), const void *unused,
       size_t mapnum, void **hostaddrs, size_t *sizes,
       unsigned char *kinds)
{
  target_desc.fn = fn;
  target_desc.hostaddrs = hostaddrs;
  target_desc.nteams = 0x1U;
  target_desc.threadLimit = DEFAULT_TARGET_THREAD_LIMIT;
  
  fn(hostaddrs);

  MSGBarrier_swDocking_Wait(target_desc.nteams);
}

void
GOMP_teams (unsigned int num_teams, unsigned int thread_limit)
{
  if(get_cl_id() == MASTER_ID)
  {
    target_desc.nteams = num_teams > DEFAULT_TARGET_MAX_NTEAMS ? DEFAULT_TARGET_MAX_NTEAMS : num_teams;
    target_desc.threadLimit = thread_limit > DEFAULT_TARGET_THREAD_LIMIT ? DEFAULT_TARGET_THREAD_LIMIT : thread_limit;

    for(uint32_t i = 1; i < target_desc.nteams; ++i)
      gomp_hal_hwTrigg_Team( i );
  }
  gomp_set_thread_pool_idle_cores( target_desc.threadLimit - 1);
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
