#ifndef __ALL_SLEEP_H__
#define __ALL_SLEEP_H__

#include "appsupport.h"
#include "omp-lock.h"

static inline void SlaveEnter_all_sleep(int myid, unsigned int masterPID, unsigned int *lock, volatile unsigned int *cnt)
{
    gomp_hal_lock(lock);
    if( *(cnt) == 1 ){ 
        gomp_hal_unlock(lock);
        trigg_barrier(1<<masterPID);
        wait_event_buff(myid);
    }else{
        *(cnt)=(*(cnt))-1;
        gomp_hal_unlock(lock);
        wait_event_buff(myid);  
    }
    return;
} // MSGBarrier_SlaveEnter

static inline void Wait_all_sleep(int nthreads, unsigned int masterPID, unsigned int *lock, volatile unsigned int *cnt)
{      
    gomp_hal_lock(lock);
    if(*cnt == 1 ){ 
        gomp_hal_unlock(lock);       
    }
    else{
        //i am not the last, go to sleep
        *(cnt)=(*(cnt))-1;
        gomp_hal_unlock(lock);     
        wait_event_buff(masterPID);  
    }
    
    //NOTE[Ale] I am not sure it is so safe?!
    *(cnt) = nthreads;
    return;
}

static inline void Release_all_sleep(unsigned int  team_mask, unsigned int masterPID) {
    trigg_barrier(team_mask^(1<<masterPID));
    return;
}

#endif
