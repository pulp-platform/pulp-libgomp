/* Copyright 2014 DEI - Universita' di Bologna
   author       Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         PULP memory allocation */

#ifndef __MEMUTILS_H__
#define __MEMUTILS_H__

extern void shmalloc_init();
extern void *shmalloc(unsigned int);
extern void shfree(void *);

#endif
