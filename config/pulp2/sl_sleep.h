#ifndef __SLEEP_SLAVE_H__
#define __SLEEP_SLAVE_H__

#include "events.h"
#include "omp-lock.h"

static inline void SlaveEnter_sl_sleep(int myid, unsigned int *lock, volatile unsigned int *cnt) {
    gomp_hal_lock(lock);
    *(cnt)=(*(cnt))-1;
    gomp_hal_unlock(lock);
    wait_event_buff(myid);  
    return;
}

static inline void Wait_sl_sleep(int nthreads, volatile unsigned int *cnt) {
    //NOTE[Ale] I am not sure it is so safe?!
    while( *(cnt) != 1 ) 
    {   
        continue;
    }
    *(cnt) = nthreads;
    return;
}

static inline void Release_sl_sleep(int nthreads, unsigned int *ids) {
    int i;
    for(i = 1; i < nthreads; ++i)
        trigg_event(ids[i]+GP0);

    return;
}

#endif
