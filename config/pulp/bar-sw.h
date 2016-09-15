/* Copyright 2014 DEI - Universita' di Bologna
 *   author       DEI - Universita' di Bologna
 *                Davide Rossi         - davide.rossi@unibo.it
 *                Alessandro Capotondi - alessandro.capotondi@unibo.it
 *   info         Master Slave Software Barriers implementation. */


#ifndef __BAR_SW_H__
#define __BAR_SW_H__           

#include "bar.h"

/* Per-Core FLAG OFFSET */
#define FLAG_OFFSET(x)                      (x << Log2SizeofMSGBarrier)

/* Software Wait Barrier */
#define RFLAGS(x)                           (volatile MSGBarrier *) ((uint32_t) SWBAR_BASE_ADDR + FLAG_OFFSET(x))
/* Software Notification Barrier */
#define NFLAGS(x)                           (volatile MSGBarrier *) ((uint32_t) SWBAR_BASE_ADDR + SWBAR_RFLAGS_SIZE + FLAG_OFFSET(x))

//FIXME: for the moment the sw barriers are not supported
#if 0
/* Global RFLAGS base address */
ALWAYS_INLINE uint32_t
GRFLAGS_BASE(uint32_t cid)
{
    uint32_t ret;
    switch(cid)
    {
        case 0x0U:
            ret = SWBAR_GBASE_ADDR + ( PULP_CLUSTER_SIZE * 0x0U );
            break;
        case 0x1U:
            ret = SWBAR_GBASE_ADDR + ( PULP_CLUSTER_SIZE * 0x1U );
            break;
        case 0x2U:
            ret = SWBAR_GBASE_ADDR + ( PULP_CLUSTER_SIZE * 0x2U );
            break;
        case 0x3U:
            ret = SWBAR_GBASE_ADDR + ( PULP_CLUSTER_SIZE * 0x3U );
            break;
        default:
            ret = SWBAR_GBASE_ADDR + ( PULP_CLUSTER_SIZE * 0x0U );
            break;
    }
    return ret;
}

/* Global NFLAGS base address */
ALWAYS_INLINE uint32_t
GNFLAGS_BASE(uint32_t cid)
{
    uint32_t ret;
    switch(cid)
    {
        case 0x0U:
            ret = SWBAR_GBASE_ADDR + SWBAR_RFLAGS_SIZE + ( PULP_CLUSTER_SIZE * 0x0U );
            break;
        case 0x1U:
            ret = SWBAR_GBASE_ADDR + SWBAR_RFLAGS_SIZE + ( PULP_CLUSTER_SIZE * 0x1U );
            break;
        case 0x2U:
            ret = SWBAR_GBASE_ADDR + SWBAR_RFLAGS_SIZE + ( PULP_CLUSTER_SIZE * 0x2U );
            break;
        case 0x3U:
            ret = SWBAR_GBASE_ADDR + SWBAR_RFLAGS_SIZE + ( PULP_CLUSTER_SIZE * 0x3U );
            break;
        default:
            ret = SWBAR_GBASE_ADDR + SWBAR_RFLAGS_SIZE + ( PULP_CLUSTER_SIZE * 0x0U );
            break;
    }
    return ret;
}

/*** *** Master Slave Barrier APIs *** ***/
ALWAYS_INLINE void
MSGBarrier_Wait( uint32_t nthreads,
                 uint32_t *slave_ids)
{     
    uint32_t i;
    for( i = 1; i < nthreads; ++i )
    {
        uint32_t slave_id = slave_ids[i];
        while( !(*(NFLAGS(slave_id))) )
            continue;

        /* Reset flag */
        *(NFLAGS(slave_id)) = 0x0U;
    }
}

ALWAYS_INLINE void
MSGBarrier_SlaveEnter(uint32_t mcid,
                      uint32_t pid)
{
    volatile MSGBarrier *rflag = RFLAGS( pid );
    /* Read start value */
    volatile MSGBarrier  old_val = *rflag;
    
    /* Notify the master I'm on the barrier */
    *(GNFLAGS(mcid, pid)) = 0x1U;
            
    while(1)
    {
        volatile MSGBarrier *curr_val = RFLAGS( pid );
        if (old_val == *curr_val)
            continue;
        break;
    }
}

ALWAYS_INLINE void
MSGBarrier_Release( uint32_t *tmasks )
{
    uint32_t cid;
    for( cid = 0; cid < DEFAULT_MAXCL; ++cid )
        if(tmasks[cid])
            gomp_hal_hwTrigg_core( cid, tmasks[cid] );
}

ALWAYS_INLINE void
gomp_hal_barrier( uint32_t mcid,
                  uint32_t nthreads,
                  uint32_t *slave_ids,
                  uint32_t *tmasks)
{
    uint32_t pid = get_proc_id();

    if(slave_ids[0] != pid)
    {
        MSGBarrier_SlaveEnter( mcid, pid );
    }
    else
    {
        MSGBarrier_Wait( nthreads, slave_ids );
        MSGBarrier_Release( tmasks);
    }
}
#endif

#endif /*__BAR_SW_H__*/
