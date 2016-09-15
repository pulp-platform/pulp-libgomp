/* Copyright 2014 DEI - Universita' di Bologna
   author       Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         PULP memory allocation */

#ifndef __MEMUTILS_H__
#define __MEMUTILS_H__

#include "omp-hal.h"
#include <stdint.h>
#include <malloc.h>

static inline void
gompmem_init ()
{
	uint32_t i;
	for (i = 0; i < get_num_procs(); ++i)
		*NFLAGS(i) = 0x0U;
}

static inline void *
shmalloc(uint32_t size)
{
	return (void*)l1malloc(size); 
}

static inline void
shfree(void *address)
{
	l1free(address);
}


#endif
