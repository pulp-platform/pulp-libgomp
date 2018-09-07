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
#include "vmm/vmm.h"

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
    uint32_t cmd = (uint32_t)NULL;

    // Offloaded function pointer and arguments
    void (*offloadFn)(void **) = NULL;
    void **offloadArgs = NULL;

    int cycles = 0;

    while(1) {
        if (DEBUG_LEVEL_OFFLOAD_MANAGER > 0)
            printf("Waiting for command...\n");

        // (1) Wait for the offload trigger.
        mailbox_read((unsigned int *)&cmd);
        if (PULP_STOP == cmd) {
            if (DEBUG_LEVEL_OFFLOAD_MANAGER > 0)
                printf("Got PULP_STOP from host, stopping execution now.");
            break;
        }

        // (2) The host sends through the mailbox the pointer to the function that should be
        // executed on the accelerator.
        mailbox_read((unsigned int *)&offloadFn);

        if (DEBUG_LEVEL_OFFLOAD_MANAGER > 0)
            printf("tgt_fn @ 0x%x\n",(unsigned int)offloadFn);

        // (3) The host sends through the mailbox the pointer to the arguments that should
        // be used.
        mailbox_read((unsigned int *)&offloadArgs);

        if (DEBUG_LEVEL_OFFLOAD_MANAGER > 0)
            printf("tgt_vars @ 0x%x\n",(unsigned int)offloadArgs);

        // (4) Ensure access to offloadArgs. It might be in SVM.
        unsigned tmp = pulp_tryread_prefetch((unsigned int *)offloadArgs);
        if (tmp)
            map_page((void *)offloadArgs);

        reset_timer();
        start_timer();

        // (5) Execute the offloaded function.
        offloadFn(offloadArgs);

        stop_timer();
        cycles = get_time();

        mailbox_write(TO_RUNTIME | 2);
        mailbox_write(PULP_DONE);
        mailbox_write(cycles);

        if (DEBUG_LEVEL_OFFLOAD_MANAGER > 0)
            printf("Kernel execution time [PULP cycles] = %d\n", cycles);
    }

    return 0;
}
