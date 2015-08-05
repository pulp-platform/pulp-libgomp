/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Library main file for single compilation flow */

#include "libgomp.h"

/* SOURCES */

#include "appsupport.c"
#include "lock.c"
#include "memutils.c"
#include "omp-bar.c"

#include "barrier.c"
#include "critical.c"
#include "env.c"
#include "iter.c"
#include "libgomp.c"
#include "loop.c"
#include "parallel.c"
#include "sections.c"
#include "single.c"
#include "team.c"
#include "work.c"
