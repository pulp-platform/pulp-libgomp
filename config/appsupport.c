/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Appsupport for PULP */

#include "appsupport.h"
#include "pulp.h"
#include "events.h"

void  abort() {
  qprintf("ERROR - Aborting\n");
  //eoc(0);
  while(1);
}

