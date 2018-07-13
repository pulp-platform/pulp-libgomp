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

#ifndef __OFFLOAD_MANAGER_H__
#define __OFFLOAD_MANAGER_H__

extern void** offload_func_table;
extern uint32_t nb_offload_funcs;
extern void** offload_var_table;
extern uint32_t nb_offload_vars;
extern void *__OFFLOAD_TARGET_TABLE__[];

// #define OFFLOAD_MANAGER_VERBOSE

static inline void
gomp_init_offload_manager ( )
{
    uint32_t i;

    offload_func_table = ((void **) __OFFLOAD_TARGET_TABLE__)[0];
    nb_offload_funcs = ((uint32_t)((void **) __OFFLOAD_TARGET_TABLE__)[1] - (uint32_t) ((void **) __OFFLOAD_TARGET_TABLE__)[0]) / 0x4U;
    offload_var_table  = ((void **) __OFFLOAD_TARGET_TABLE__)[2];
    nb_offload_vars  = ((uint32_t)((void **) __OFFLOAD_TARGET_TABLE__)[3] - (uint32_t) ((void **) __OFFLOAD_TARGET_TABLE__)[2]) / 0x4U;

    hal_mailbox_write(TO_RUNTIME | 2);
    hal_mailbox_write(nb_offload_funcs);
    hal_mailbox_write(nb_offload_vars);

    if(nb_offload_funcs) {
        hal_mailbox_write(TO_RUNTIME | nb_offload_funcs);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(offload_func_table = %x {", offload_func_table);
#endif
        for(i = 0; i < nb_offload_funcs; i++){
#ifdef OFFLOAD_MANAGER_VERBOSE          
            printf ("func%d: %x, ", i, offload_func_table[i]);
#endif
            hal_mailbox_write((uint32_t) offload_func_table[i]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    if(nb_offload_vars)
    {
        hal_mailbox_write(TO_RUNTIME | 2*nb_offload_vars);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(offload_var_table = %x {", offload_var_table);
#endif    
        for(i = 0; i < nb_offload_vars; i++)
        {
#ifdef OFFLOAD_MANAGER_VERBOSE
            printf ("var%d: %x size %x,", i, offload_var_table[2*i], offload_var_table[2*i+1]);
#endif
            hal_mailbox_write((uint32_t) offload_var_table[2*i]);
            hal_mailbox_write((uint32_t) offload_var_table[2*i+1]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    return;
}

int gomp_offload_manager ( void );
void target_register_lib (const void *);
#endif
