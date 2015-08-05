#ifndef __POLLING_H__
#define __POLLING_H__

#include "omp-bar.h"

static inline void SlaveEnter_polling(int myid) {
    volatile _DTYPE *exit = SLAVE_FLAG(myid);
    volatile _DTYPE *flag = (volatile _DTYPE *) SLAVE_FLAG(myid);
    
    /* Read start value */
    volatile _DTYPE g = *flag;
    
    #ifdef DEBUG
    qprintf("SLAVE  FLAG : %d, ID: %d, ADDR: %x\n",*(SLAVE_FLAG(myid)),myid,(SLAVE_FLAG(myid)));
    qprintf("MASTER FLAG : %d, ID : %d, ADDR: %x\n",*(MASTER_FLAG(myid)),myid,(MASTER_FLAG(myid)));
    #endif
    *(MASTER_FLAG(myid)) = 1;
    while(g == *exit)
    {
    }
    return;
}

static inline void Wait_polling(int num_threads, unsigned int *local_slave_ids) {
    unsigned int curr_proc_id, i;
    
    for(i = 1; i < num_threads; ++i) {
        curr_proc_id = local_slave_ids[i];
        
        #ifdef DEBUG
        qprintf("CURR PROC ID: %d, MASTER FLAG : %d, ADDR: %x, NUM_THREADS: %d\n",curr_proc_id,*(MASTER_FLAG(curr_proc_id)),(MASTER_FLAG(curr_proc_id)), num_threads );
        #endif
        
        while(!(*(MASTER_FLAG(curr_proc_id))))
        {
            continue;
        }
        *(MASTER_FLAG(curr_proc_id)) = 0;
    }
    
    return;
}

void Release_polling(int num_threads, unsigned int *local_slave_ids){
    unsigned int curr_proc_id, i;
    
    for(i = 1; i < num_threads; ++i) {
        curr_proc_id = local_slave_ids[i];
        
        #ifdef DEBUG
        qprintf("CURR PROC ID: %d, SLAVE FLAG : %d, ADDR: %x, NUM_THREADS: %d\n",*(MASTER_FLAG(curr_proc_id)),curr_proc_id,(MASTER_FLAG(curr_proc_id)), num_threads );
        #endif
        
        volatile _DTYPE *exit = (volatile _DTYPE *) SLAVE_FLAG(curr_proc_id);
        /* Increase exit count */
        (*exit)++;
    }
    return;
}

#endif
