/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Library main file for single compilation flow */

#include "libgomp.h"

/* SOURCES */
#if PULP_CHIP_FAMILY == CHIP_BIGPULP
#include "config/bigpulp/appsupport.c"
#include "config/bigpulp/lock.c"
#include "config/bigpulp/memutils.c"
#ifdef GCC_OFFLOAD_SUPPORT
#include "config/bigpulp/offload-manager.c"
#else
#else
#include "config/pulp/appsupport.c"
#include "config/pulp/lock.c"
#include "config/pulp/memutils.c"
#endif

#include "barrier.c"
#include "iter.c"                
#include "work.c"
#include "team.c"
#include "target.c"
#include "loop.c"
#include "sections.c"
#include "single.c"
#include "parallel.c"
#include "critical.c"                
#include "env.c"

#include "libgomp.c"

