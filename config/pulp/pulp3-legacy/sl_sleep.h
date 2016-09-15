#ifndef __SLEEP_SLAVE_H__
#define __SLEEP_SLAVE_H__

#include "appsupport.h"
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

static inline void Release_sl_sleep(int team_mask, unsigned int masterPID) {
    trigg_core(team_mask^(1<<masterPID));
    return;
}

#endif
