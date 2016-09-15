#ifndef __ALL_SLEEP_H__
#define __ALL_SLEEP_H__

#include "events.h"
#include "omp-lock.h"

static inline void SlaveEnter_all_sleep(int myid, unsigned int masterID, unsigned int *lock, volatile unsigned int *cnt)
{
    gomp_hal_lock(lock);
    if( *(cnt) == 1 ) {
        gomp_hal_unlock(lock);
        trigg_event(masterID+GP0);
        wait_event_buff(myid);
    }
    else {
        *(cnt) = (*(cnt))-1;
        gomp_hal_unlock(lock);
        wait_event_buff(myid);  
    }
    return;
} // MSGBarrier_SlaveEnter

static inline void Wait_all_sleep(int nthreads, unsigned int *ids, unsigned int *lock, volatile unsigned int *cnt)
{      
    gomp_hal_lock(lock);
    if(*cnt == 1 ){ 
        gomp_hal_unlock(lock);       
    }
    else{
        //i am not the last, go to sleep
        *(cnt)=(*(cnt))-1;
        gomp_hal_unlock(lock);
        wait_event_buff(ids[0]);
    }
    
    //NOTE[Ale] I am not sure it is so safe?!
    *(cnt) = nthreads;
    return;
}

static inline void Release_all_sleep(int nthreads, unsigned int *ids) {
    int i;
    for(i = 1; i < nthreads; ++i)
        trigg_event(ids[i]+GP0);
    return;
}

#endif
