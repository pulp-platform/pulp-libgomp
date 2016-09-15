/* Copyright 2014 DEI - Universita' di Bologna
 *   author       DEI - Universita' di Bologna
 *                Davide Rossi         - davide.rossi@unibo.it
 *                Alessandro Capotondi - alessandro.capotondi@unibo.it
 *   info         Master Slave Hardware Barriers implementation. */


#ifndef __BAR_HW_H__
#define __BAR_HW_H__           

#include "bar.h"
#if EU_VERSION == 1
#include "events.h"
#include "timer.h"
#include "events_ids.h"
#endif

/*** *** Low Level Event Unit APIs *** ***/
ALWAYS_INLINE void
gomp_hal_wait_hwBarrier_buff( uint32_t barrier_id )
{
#if EU_VERSION == 1    
    
    *(volatile int*) (WAIT_BARRIER) =  barrier_id;
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    
    // Flush the pipeline
    #ifdef __riscv__
      asm volatile ("WFI");
    #else
    asm volatile ("l.psync");
    #endif
    
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
#else
    eu_bar_trig_wait_clr(eu_bar_addr(barrier_id));
#endif
}

ALWAYS_INLINE void
gomp_hal_clear_hwBarrier_buff( )
{
#if EU_VERSION == 1    
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
#else
    eu_bar_trig_wait_clr(eu_bar_addr(0));
#endif
}


ALWAYS_INLINE void
gomp_hal_set_hwBarrier( uint32_t barrier_id,
                        uint32_t nthreads,
                        uint32_t thMask )
{
#if EU_VERSION == 1
    *(volatile uint32_t*) (SET_BARRIER_BASE + 0x4U*barrier_id) = (nthreads << 16U) + thMask;
#else
    eu_bar_setup_mask(eu_bar_addr(barrier_id), thMask, thMask);
#endif
}

ALWAYS_INLINE void
gomp_hal_wait_hwEvent_buff( )
{
#if EU_VERSION == 1
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    // Flush the pipelineY
#ifdef __riscv__
  asm volatile ("WFI");
#else
  asm volatile ("l.psync");
#endif
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
#else
    eu_evt_waitAndClr(1<<0);
#endif
}

/*** *** Master Slave Barrier APIs *** ***/

ALWAYS_INLINE void
MSGBarrier_hwWait( uint32_t barrier_id)
{
    /* Use Hardware Barrier */
    gomp_hal_wait_hwBarrier_buff( barrier_id );
}

ALWAYS_INLINE void
MSGBarrier_hwSlaveEnter( uint32_t barrier_id)
{
#if EU_VERSION == 1
    gomp_hal_wait_hwBarrier_buff( barrier_id );
#else
    gomp_hal_wait_hwBarrier_buff(barrier_id);
    gomp_hal_wait_hwEvent_buff(/*pid*/);  
#endif       
}

ALWAYS_INLINE void
MSGBarrier_hwRelease( uint32_t thMask )
{
    gomp_hal_hwTrigg_core( thMask);
}

ALWAYS_INLINE void
gomp_hal_hwBarrier( uint32_t barrier_id)
{
    gomp_hal_wait_hwBarrier_buff( barrier_id );
}

/** First Level Barriers **/
ALWAYS_INLINE void
MSGBarrier_Wait_init( uint32_t nthreads,
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
MSGBarrier_SlaveEnter_init ( uint32_t pid )
{
    /* Notify the master I'm on the barrier */
    *(NFLAGS(pid)) = 0x1U;
    gomp_hal_wait_hwEvent_buff(/*pid*/);
}

#endif /*__BAR_HW_H__*/
