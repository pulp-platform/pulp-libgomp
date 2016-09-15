#include "libgomp.h"

#include "sl_sleep.h"

/* This is the barrier code executed by each SLAVE core */
void
MSGBarrier_SlaveEnter(int myid,
                      unsigned int master_id,
                      int nthreads,
                      unsigned int *lock, 
                      volatile unsigned int *barr_cnt,unsigned int barrier_id) {
#if EU_VERSION == 1
  wait_barrier_buff(barrier_id);
#else
    wait_barrier_buff(barrier_id);
    wait_event_buff(myid);  
#endif
}

/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Wait(int nthreads,
                unsigned int *slave_ids,
                unsigned int *lock,
                volatile unsigned int *barr_cnt,unsigned int barrier_id) {

  wait_barrier_buff(barrier_id);
}

/* This is the barrier code executed by the MASTER core to gather SLAVES */
ALWAYS_INLINE void
MSGBarrier_Release(int nthreads, unsigned int *slave_ids, unsigned int team_mask)
{
  trigg_core(team_mask^(1<<slave_ids[0]));
}


void
MSlaveBarrier_SlaveEnter_init(int myid) {
    volatile _DTYPE *exit = SLAVE_FLAG(myid);
    volatile _DTYPE *flag = (volatile _DTYPE *) SLAVE_FLAG(myid);
    
    /* Read start value */
    volatile _DTYPE g = *flag;

    *(MASTER_FLAG(myid)) = 1;
    wait_event_buff(myid);  
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
    unsigned int *slave_ids;
    unsigned int *lock;
    unsigned int team_mask;
    volatile unsigned int *barr_cnt;
    
    myid = prv_proc_num;
    team = (gomp_team_t *) CURR_TEAM(myid);
    
    wait_barrier_buff(team->barrier_id);
    
    return;
}
