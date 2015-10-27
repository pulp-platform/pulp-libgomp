/* Copyright 2014 DEI - Universita' di Bologna
   author       Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         PULP memory allocation */

#include "libgomp.h"
#include "malloc.h"

//FIXME[ALE] whitout this the library doesn't work! Why we should investigate soon!!!!
// AT: this is needed by crt0 to set the master and slave flags for polling
// mode. Otherwise they are not initialized and are just 32'hXXXXXXXX
void gompmem_init()
{
    int i;
    for(i=0;i<0x200;i++)
        *(int*)(LIBGOMP_BASE + 4*i) = 0;
}

void shmalloc_init(){
}

inline void *
shmalloc(unsigned int size) {
    
  return (void*)scml1malloc(size); 
}

void shfree(void *address) {
  scml1free(address);
}
