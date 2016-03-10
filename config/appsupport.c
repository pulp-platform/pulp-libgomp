/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Appsupport for PULP */

#include "appsupport.h"
#include "pulp.h"
#if EU_VERSION == 1
#include "events.h"
#endif

void  abort() {
  printf("ERROR - Aborting\n");
  exit(1);
}

