#include <stdint.h>
#include "hal/pulp.h"

#include "offload-manager.h"

//FIXME consider to move these variables to L1 (vars it is ususally Empty when OpenMP4 Offload is used)
void** offload_func_table;
uint32_t nb_offload_funcs;
void** offload_var_table;
uint32_t nb_offload_vars;

void
target_register_lib (const void *target_table)
{
    return;
}

int
gomp_offload_manager ( )
{
    //Init the manager (handshake btw host and accelerator is here)
    gomp_init_offload_manager();

    //FIXME For the momenent we are not using the cmd sended as trigger.
    // It should be used to perform the deactivation of the accelerator,
    // as well as other operations, like local data allocation or movement.
    //FIXME Note that the offload at the moment use several time the mailbox.
    // We should compact the offload descriptor and just sent a pointer to
    // that descriptor.
    uint32_t cmd = NULL;

    // Offloaded function pointer and arguments
    void (*offloadFn)(void **) = NULL;
    void **offloadArgs = NULL;
    
    //FIXME this function desnt return yet
    while(1)
    {
        // (1) Wait the offload trigger.
        while (mailbox_read(&cmd))
        {
            my_sleep(100);
        }

        // EOC signal RESET used to notify
        // offload execution termination to the host
        *(volatile int*)(EOC_UNIT_BASE_ADDR) = 0;

        // (2) The host send throught the mailbox
        // the pointer to the function that should
        // be executed on the accelerator
        mailbox_read((uint32_t *) &offloadFn);

        // (3) The host send throught the mailbox
        // the pointer to the arguments that should
        // be used
        mailbox_read((uint32_t *) &offloadArgs);

        reset_timer();
        start_timer();
        
        // (4) Execute the Offloaded Function!!!
        offloadFn(offloadArgs);
        
        // EOC signal NOTIFY used to notify
        // offload execution termination to the host
        *(volatile int*)(EOC_UNIT_BASE_ADDR) = 1;
        stop_timer();

#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("+->pulp kernel   [PULP cycles] = %d\n", get_time());
#endif        
    }

    return 0;
}
