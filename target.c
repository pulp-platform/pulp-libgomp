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

// static void
// gomp_target_fallback (void (*fn) (void *), void **hostaddrs)
// {
//   struct gomp_thread old_thr, *thr = gomp_thread ();
//   old_thr = *thr;
//   memset (thr, '\0', sizeof (*thr));
//   if (gomp_places_list)
//     {
//       thr->place = old_thr.place;
//       thr->ts.place_partition_len = gomp_places_list_len;
//     }
//   fn (hostaddrs);
//   gomp_free_thread (thr);
//   *thr = old_thr;
// }

void
GOMP_target (int device, void (*fn) (void *), const void *unused,
       size_t mapnum, void **hostaddrs, size_t *sizes,
       unsigned char *kinds)
{
  gomp_team_t *curr_team = CURR_TEAM(get_proc_id());
  targetFn = fn;
  targetData = hostaddrs;
  targetFn(targetData);
  // fn(hostaddrs);
}

void
GOMP_teams (unsigned int num_teams, unsigned int thread_limit)
{
  if(get_cl_id() == 0)
  {
    int i;
    nteams = num_teams;
    threadLimit = thread_limit;

    gomp_team_t *curr_team = CURR_TEAM(get_proc_id());

    for(i = 1; i < omp_get_num_teams(); ++i)
    {
      CURR_LEAGUE(i) = curr_team;
      gomp_hal_hwTrigg_Team(i);
    } 
  }
}

int
omp_get_team_num(void)
{
  return get_cl_id();
}

int
omp_get_num_teams(void)
{
  return nteams;
}
