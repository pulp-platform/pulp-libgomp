/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Appsupport for PULP */

#include <pulp.h>
#include <appsupport.h>

void abort()
{
  printf("ERROR: aborting...\n");
  exit(1);
  eoc(1);
}


int pulp_mchan_transfer(unsigned int len, char type, char incr, char twd, unsigned int ext_addr, unsigned int tcdm_addr, unsigned short int count, unsigned short int stride){
	return mchan_transfer(len, type, incr, twd, ext_addr, tcdm_addr, count, stride);
}

void pulp_mchan_barrier(int id) {
	return mchan_barrier(id);
}

void *
pulp_l1malloc(int size)
{
	return l1malloc(size);
}

void *
pulp_l2malloc(int size)
{
	return l2malloc(size);
}

void
pulp_l1free(void * a)
{
	l1free(a);
}

void
pulp_l2free(void * a)
{
	l2free(a);
}