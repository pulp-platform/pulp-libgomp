/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Appsupport for PULP */

#include <pulp.h>

void abort()
{
  printf("ERROR: aborting...\n");
  exit(1);
  eoc(1);
}

