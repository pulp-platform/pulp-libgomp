/* Copyright 2014 DEI - Universita' di Bologna
 *   author       DEI - Universita' di Bologna
 *                Davide Rossi         - davide.rossi@unibo.it
 *                Alessandro Capotondi - alessandro.capotondi@unibo.it
 *   info         Master Slave Software Barriers implementation. */


#ifndef __OMP_BAR_H__
#define __OMP_BAR_H__        i

#include "omp-hal.h"

/* HW Wakeup Cores */
static inline void
gomp_hal_hwTrigg_core( uint32_t cid,
                       uint32_t cmask)
{    
    *(volatile uint32_t*) ( get_hal_addr( cid, OFFSET_TRIGG_BARRIER )) = cmask;
}

//FIXME: for the moment the sw barriers are not supported
#if 0
/* Software Barriers Data Type */
typedef int32_t MSGBarrier;
#define Log2SizeofMSGBarrier (2U)

/* Software Barriers Base Address */
#define SWBAR_BASE_ADDR                     (LIBGOMP_BASE)
#define SWBAR_GBASE_ADDR                    (LIBGOMP_GBASE_ADDR)
#define SWBAR_RFLAGS_SIZE                   ((DEFAULT_MAX_PE  << Log2SizeofMSGBarrier))
#define SWBAR_NFLAGS_SIZE                   ((DEFAULT_MAXPROC << Log2SizeofMSGBarrier))
#define SWBAR_SIZE                          (SWBAR_RFLAGS_SIZE + SWBAR_NFLAGS_SIZE)
#define SWBAR_ID                            (0xFFFFFFFFU)

#include "bar-sw.h"
#endif

#include "bar-hw.h"

#endif /*__BAR_H__*/
