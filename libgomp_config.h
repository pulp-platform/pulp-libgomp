#ifndef __LIBGOMP_CONFIG_H__
#define __LIBGOMP_CONFIG_H__

#include "config.h"

/* Default master processors ID */
#define MASTER_ID               0

/* This is the maximum number of threads we can see in a parallel region
 * In our implementation it is equal to the max number of processors */
#define MAX_PARREG_THREADS		DEFAULT_MAXPROC

#endif /* __LIBGOMP_CONFIG_H__ */
