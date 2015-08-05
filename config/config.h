#ifndef __CONFIG_H__
#define __CONFIG_H__

//#define PULP2
// #define PULP3_LEGACY
#define PULP3


// #define __NO_OMP_MALLOC_ON_PREALLOC__

#include "pulp.h"

//------------------------------------------------------------
// Builtin command line parameter defaults
//------------------------------------------------------------
#define SIZEOF_UNSIGNED         0x4
#define SIZEOF_PTR              0x4
#define SIZEOF_INT              0x4
#define SIZEOF_WORD             0x4


#define LIBGOMP_BASE            (LIBGOMP_BASE_ADDR)

#define _DTYPE                  int

#endif
