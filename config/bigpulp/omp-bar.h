/* Copyright 2014 DEI - Universita' di Bologna
 *   author       DEI - Universita' di Bologna
 *                Davide Rossi         - davide.rossi@unibo.it
 *                Alessandro Capotondi - alessandro.capotondi@unibo.it
 *   info         Master Slave Software Barriers implementation. */


#ifndef __OMP_BAR_H__
#define __OMP_BAR_H__

#include "omp-hal.h"

#if EU_VERSION == 1
#include "events.h"
#include "timer.h"
#include "events_ids.h"
#endif

/* HW Wakeup Cores */
static inline void
gomp_hal_hwTrigg_core( uint32_t cid,
                       uint32_t cmask)
{
#if EU_VERSION == 1
    *(volatile uint32_t*) ( get_hal_addr( cid, OFFSET_TRIGG_BARRIER )) = cmask;
#endif
}


/* Software Barriers Data Type */
typedef volatile int32_t MSGBarrier;
#define Log2SizeofMSGBarrier (2U)

/* Software Barriers Base Address */
#define SWBAR_BASE_ADDR                     ( LIBGOMP_BASE )

/* Local Aliased Address */
#define SWBAR_LBASE_ADDR                    ( LIBGOMP_LBASE )

#define SWBAR_RFLAGS_SIZE                   (( DEFAULT_MAX_PE  << Log2SizeofMSGBarrier ))
#define SWBAR_NFLAGS_SIZE                   (( DEFAULT_MAXPROC << Log2SizeofMSGBarrier ))
#define SWBAR_SIZE                          ( SWBAR_RFLAGS_SIZE + SWBAR_NFLAGS_SIZE )
#define SWBAR_ID                            ( 0xFFFFFFFFU )

#include "bar-sw.h"
#include "bar-hw.h"

#endif /*__BAR_H__*/
