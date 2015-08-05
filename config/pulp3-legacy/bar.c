#include "libgomp.h"

#define PULP3_LEGACY_ALL_SLEEP
// #define PULP3_LEGACY_SLAVE_SLEEP
// #define PULP3_LEGACY_BAR_POLLING






#if defined(PULP3_LEGACY_ALL_SLEEP)
#   include "all_sleep.h"

#elif defined (PULP3_LEGACY_SLAVE_SLEEP)
#include "sl_sleep.h"

#elif defined (PULP3_LEGACY_BAR_POLLING)
#include "polling.h"
#endif

/* This is the barrier code executed by each SLAVE core */
void
MSGBarrier_SlaveEnter(int myid,
                      unsigned int master_id,
                      int nthreads,
                      unsigned int *lock, 
                      volatile unsigned int *barr_cnt) {

#if   defined(PULP3_LEGACY_ALL_SLEEP)
    SlaveEnter_all_sleep(myid, master_id, lock, barr_cnt);
#elif defined(PULP3_LEGACY_SLAVE_SLEEP)
    SlaveEnter_sl_sleep(myid, nthreads, lock, barr_cnt);
#elif defined(PULP3_LEGACY_BAR_POLLING)
    SlaveEnter_polling(myid);
#endif

    return;
} // MSGBarrier_SlaveEnter

/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Wait(int nthreads,
                unsigned int *slave_ids,
                unsigned int *lock,
                volatile unsigned int *barr_cnt) {

#if   defined(PULP3_LEGACY_ALL_SLEEP)
    Wait_all_sleep(nthreads, slave_ids, lock, barr_cnt);
#elif defined(PULP3_LEGACY_SLAVE_SLEEP)
    Wait_sl_sleep(nthreads, barr_cnt);
#elif defined(PULP3_LEGACY_BAR_POLLING)
    Wait_polling(nthreads, slave_ids);
#endif

    return;
} // MSGBarrier_Wait

/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Release(int nthreads, unsigned int *slave_ids) {

#if   defined(PULP3_LEGACY_ALL_SLEEP)
    Release_all_sleep(nthreads, slave_ids);
#elif defined(PULP3_LEGACY_SLAVE_SLEEP)
    Release_sl_sleep(nthreads, slave_ids);
#elif defined(PULP3_LEGACY_BAR_POLLING)
    Release_polling(nthreads, slave_ids);
#endif

    return;
}


void
MSlaveBarrier_SlaveEnter_init(int myid) {
    volatile _DTYPE *exit = SLAVE_FLAG(myid);
    volatile _DTYPE *flag = (volatile _DTYPE *) SLAVE_FLAG(myid);
    
    /* Read start value */
    volatile _DTYPE g = *flag;

    *(MASTER_FLAG(myid)) = 1;
    
#ifdef PULP3_LEGACY_BAR_POLLING
    while(g == *exit);
#else  
    wait_event_buff(myid);
#endif

    return;
}


ALWAYS_INLINE void
MSlaveBarrier_Wait_init(int nthreads, unsigned int *slave_ids) {
    unsigned int curr_proc_id, i;
    
    for(i = 1; i < nthreads; i++) {
        curr_proc_id = slave_ids[i];      
        while(!(*(MASTER_FLAG(curr_proc_id)))) {
            continue;
        }
        *(MASTER_FLAG(curr_proc_id)) = 0;
    }
    return;
}

ALWAYS_INLINE void
gomp_hal_barrier() {
    unsigned int myid, nthreads;
    gomp_team_t *team;
    unsigned int *proc_ids;
    unsigned int *lock;
    volatile unsigned int *barr_cnt;


    myid = prv_proc_num;
    team = (gomp_team_t *) CURR_TEAM(myid);

    nthreads  = team->nthreads;
    proc_ids  = team->proc_ids;
    lock =(&(team->barrier_lock));
    barr_cnt =(&(team->barrier_counter));
    
    /* We can fetch master core ID looking 
    * at the first position of proc_ids */
    if(myid == proc_ids[0]) {
        MSGBarrier_Wait(nthreads, proc_ids, lock,barr_cnt);
        MSGBarrier_Release(nthreads, proc_ids);
    }
    else
        MSGBarrier_SlaveEnter(myid, proc_ids[0], nthreads ,lock, barr_cnt);
}
