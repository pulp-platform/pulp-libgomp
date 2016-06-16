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

#include "hal/pulp.h"
#include "config.h"
#include "pulp.h"
#if EU_VERSION == 1
#include "events.h"
#endif
#if defined(PULP3) || defined(PULP3_LEGACY)
#if EU_VERSION == 1
#include "events_ids.h"
#endif
#include "utils.h"
#include "string_lib.h"
#endif

void abort(void) __attribute__((noreturn));

/***********************************
 * Memory-mapped support functions *
 ***********************************/
static inline unsigned int get_proc_id(){
#ifdef NATIVE
  return 0;
#else
    #if defined(PULP3)
    int proc_id, value;
    proc_id = get_core_id() + 1;
    #elif defined(PULP2) || defined(PULP3_LEGACY)
    int proc_id = get_core_id() + 1;
    #endif
    
    return (unsigned long int) proc_id;
#endif
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
#if EU_VERSION == 1
static inline void trigg_core( unsigned int event_mask ){
    *(volatile int*) (TRIGG_BARRIER) = event_mask;
    return;
}

static inline void wait_barrier_buff(unsigned int barrier_id)  {
    *(volatile int*) (WAIT_BARRIER) =  barrier_id;
    *(volatile int*) (CORE_CLKGATE) =  0x1;
    // Flush the pipeline
#ifdef __riscv__
  asm volatile ("WFI");
#else
  asm volatile ("l.psync");
#endif
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

static inline void clear_barrier_buff()  {
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

static inline void set_barrier(unsigned int barrier_id, unsigned int num_threads, unsigned int coreMask, unsigned int mask_to_trigg)  {
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
    // Flush the pipelineY
#ifdef __riscv__
  asm volatile ("WFI");
#else
  asm volatile ("l.psync");
#endif
    *(volatile int*) (EV_BUFF_CLEAR) = 0x1;
}

#else

static inline void trigg_core( unsigned int event_mask ){
    eu_evt_trig(eu_evt_trig_addr(0), event_mask);
}

static inline void wait_barrier_buff(unsigned int barrier_id)  {
    eu_bar_trig_wait_clr(eu_bar_addr(barrier_id));
}

static inline void clear_barrier_buff()  {
    eu_bar_trig_wait_clr(eu_bar_addr(0));
}

static inline void set_barrier(unsigned int barrier_id, unsigned int num_threads, unsigned int coreMask, unsigned int mask_to_trigg)  {
    eu_bar_setup_mask(eu_bar_addr(barrier_id), coreMask, mask_to_trigg);
}

static inline unsigned int check_event(unsigned int core_id)  {
    printf("%d: %s %d\n", get_core_id(), __FILE__, __LINE__);
    return 0;
}

static inline void get_barrier(int core_id,unsigned int barrier_id)  {
    printf("%d: %s %d\n", get_core_id(), __FILE__, __LINE__);
}

static inline void wait_event_buff()  {
    eu_evt_waitAndClr(1<<0);
}


#endif

#endif

/*************************************************************
 * Print functions *
 *************************************************************/

#define _printdecp(a, b) printf("%s %d - Processor %d\n", a, b, get_proc_id() - 1)
#define _printdect(a, b) printf("%s %d - Time %d\n", a, b, get_time())
#define _printdecn(a, b) printf("%s %d\n", a, b)

#define _printhexp(a, b) printf("%s %x - Processor %d\n", a, b, get_proc_id() - 1)
#define _printhext(a, b) printf("%s %x - Time %d\n", a, b, get_time())
#define _printhexn(a, b) printf("%s %x\n", a, b)

#define _printstrp(a) printf("%s - Processor %d\n", a, get_proc_id() - 1)
#define _printstrt(a) printf("%s - Time %d\n", a, get_time())
#define _printstrn(a) printf("%s\n", a)

#endif // __APPSUPPORT_H__
