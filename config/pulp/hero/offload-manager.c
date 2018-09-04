/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 * 
 * Authors: 
 *    Alessandro Capotondi, UNIBO, (alessandro.capotondi@unibo.it)
 */

/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
 * 
 * This file is part of the GNU OpenMP Library (libgomp).
 * 
 * Libgomp is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 * 
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 * 
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with this program;
 * see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include "hal/pulp.h"

#include "hero/offload-manager.h"

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
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("Waiting cmd..\n");
#endif  
        //(1) Wait the offload trigger.
        mailbox_read((unsigned int *)&cmd);

        // (2) The host send throught the mailbox
        // the pointer to the function that should
        // be executed on the accelerator
        mailbox_read((uint32_t *) &offloadFn);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("mailbox_read: tgt_fn\n");
        printf("%x\n",(uint32_t)offloadFn);
#endif
        if((uint32_t) offloadFn == 0xdeadbeef)
            break;

        // (3) The host send throught the mailbox
        // the pointer to the arguments that should
        // be used
        mailbox_read((uint32_t *) &offloadArgs);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("mailbox_read: tgt_vars\n");
        printf("%x\n",(uint32_t)offloadArgs);
#endif
        reset_timer();
        start_timer();
        
        // (4) Execute the Offloaded Function!!!
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("execute tgt_fn...\n");
#endif
        offloadFn(offloadArgs);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("execute tgt_fn done!\n");
#endif        
        mailbox_write(TO_RUNTIME | 1);
        mailbox_write(PULP_DONE);
        stop_timer();

#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("+->pulp kernel   [PULP cycles] = %d\n", get_time());
#endif        
    }

#ifdef OFFLOAD_MANAGER_VERBOSE
    printf("bye bye...\n\n\n");
#endif  
    return 0;
}
