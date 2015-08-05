/* Copyright 2014 DEI - Universita' di Bologna
   author       DEI - Universita' di Bologna
                Davide Rossi - davide.rossi@unibo.it
                Alessandro Capotondi - alessandro.capotondi@unibo.it
   info         Appsupport for PULP */

#ifndef __APPSUPPORT_H__
#define __APPSUPPORT_H__

#ifndef ALWAYS_INLINE
#define ALWAYS_INLINE inline __attribute__ ((always_inline))
#endif

#ifndef NULL
#define NULL ((void *) 0x0) /* Standard C */
#endif

#include "config.h"
#include "pulp.h"
#include "events.h"
#if defined(PULP3) || defined(PULP3_LEGACY)
#include "spr-defs.h"
#include "events_ids.h"
#include "utils.h"
#endif

void abort(void) __attribute__((noreturn));

/***********************************
 * Memory-mapped support functions *
 ***********************************/
static inline unsigned int get_proc_id(){
    #if defined(PULP3)
    int proc_id, value;
    asm("l.mfspr\t\t%0,%1,0" : "=r" (value) : "r" (SPR_CORE_ID));
    proc_id = value + 1;
    #elif defined(PULP2) || defined(PULP3_LEGACY)
    int proc_id = get_core_id() + 1;
    #endif
    
    return (unsigned long int) proc_id;
}

static inline unsigned int get_proc_num(){
    int proc_num = get_core_num();
    return (volatile unsigned long int) proc_num;
}

static inline unsigned int get_tile_id(){
    int tile_id = 0;
    return (unsigned long int) tile_id;
}

#if defined(PULP2)
static inline void init_buff( int coreid ){
    *(volatile int*) (BUFF_CONF_REG_BASE + 4*coreid) = 1<<coreid;
    return;
}

static inline void clear_buff(int core_id)  {
    *(volatile int*) (CLEAR_EVENT) = (core_id<<6) | (20+core_id); 
    return;
}

static inline void wait_event_buff(int core_id)  {
    *(volatile int*) (CLKGATE_REG_CORE0 + 4*core_id) = core_id+GP0;
    *(volatile int*) (CLEAR_EVENT) = (core_id<<6) | (20+core_id); 
    return;
}

#elif defined(PULP3) || defined(PULP3_LEGACY)
static inline void trigg_core( unsigned int event_mask ){
    *(volatile int*) (TRIGG_BARRIER) = event_mask;
    return;
}

static inline void wait_barrier_buff(unsigned int barrier_id)  {
    *(volatile int*) (WAIT_BARRIER) =  barrier_id;
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    // Flush the pipeline
    asm volatile ("l.psync\n");
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

static inline void clear_barrier_buff()  {
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

static inline void set_barrier(unsigned int barrier_id, unsigned int num_threads, unsigned int mask_to_trigg)  {
    *(volatile int*) (SET_BARRIER_BASE+4*barrier_id) = (num_threads<<16)+mask_to_trigg;
}

static inline unsigned int check_event(unsigned int core_id)  {
    unsigned int buff;   
    buff=*(volatile unsigned int*) (EV_BUFFER_LOW_BASE  + 4*core_id);
    return ((buff)&(1<<BARRIER));
}

static inline void get_barrier(int core_id,unsigned int barrier_id)  {
    *(volatile int*) (WAIT_BARRIER) =  barrier_id;
}

static inline void wait_event_buff()  {
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    // Flush the pipeline
    asm volatile ("l.psync\n");
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

#endif

/*************************************************************
 * Print functions *
 *************************************************************/

#define _printdecp(a, b) qprintf("%s %d - Processor %d\n", a, b, get_proc_id() - 1,0)
#define _printdect(a, b) qprintf("%s %d - Time %d\n", a, b, get_time(),0)
#define _printdecn(a, b) qprintf("%s %d\n", a, b,0,0)

#define _printhexp(a, b) qprintf("%s %x - Processor %d\n", a, b, get_proc_id() - 1,0)
#define _printhext(a, b) qprintf("%s %x - Time %d\n", a, b, get_time(),0)
#define _printhexn(a, b) qprintf("%s %x\n", a, b,0,0)

#define _printstrp(a) qprintf("%s - Processor %d\n", a, get_proc_id() - 1,0,0)
#define _printstrt(a) qprintf("%s - Time %d\n", a, get_time(),0,0)
#define _printstrn(a) qprintf("%s\n", a,0,0,0)

#endif // __APPSUPPORT_H__
