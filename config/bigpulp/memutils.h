/* Copyright 2014 DEI - Universita' di Bologna
   author       Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         PULP memory allocation */

#ifndef __MEMUTILS_H__
#define __MEMUTILS_H__

#include <stdint.h>
#include <malloc.h>

static inline int l1malloc(int size)
{
  int ptr = (int)rt_alloc(RT_ALLOC_CL_DATA, size + 4);
  if (ptr == 0) return 0;
  *(int *)(ptr) = size + 4;
  return ptr;
}

static inline void l1free(int ptr)
{
  ptr -= 4;
  int size = *(int *)(ptr);
  rt_free(RT_ALLOC_CL_DATA, ptr, size);
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
