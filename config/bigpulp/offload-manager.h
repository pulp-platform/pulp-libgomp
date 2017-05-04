#ifndef __OFFLOAD_MANAGER_H__
#define __OFFLOAD_MANAGER_H__

extern void** offload_func_table;
extern uint32_t nb_offload_funcs;
extern void** offload_var_table;
extern uint32_t nb_offload_vars;
extern void *__OFFLOAD_TARGET_TABLE__[];

// void target_register_lib (const void *target_table)
// {
//     return;
// }

// #define OFFLOAD_MANAGER_VERBOSE

static inline void
gomp_init_offload_manager ( )
{
    uint32_t i;

    offload_func_table = ((void **) __OFFLOAD_TARGET_TABLE__)[0];
    nb_offload_funcs = ((uint32_t)((void **) __OFFLOAD_TARGET_TABLE__)[1] - (uint32_t) ((void **) __OFFLOAD_TARGET_TABLE__)[0]) / 0x4U;
    offload_var_table  = ((void **) __OFFLOAD_TARGET_TABLE__)[2];
    nb_offload_vars  = ((uint32_t)((void **) __OFFLOAD_TARGET_TABLE__)[3] - (uint32_t) ((void **) __OFFLOAD_TARGET_TABLE__)[2]) / 0x4U;

    mailbox_write(TO_RUNTIME | 2);
    mailbox_write(nb_offload_funcs);
    mailbox_write(nb_offload_vars);

    if(nb_offload_funcs) {
        mailbox_write(TO_RUNTIME | nb_offload_funcs);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(offload_func_table = %x {", offload_func_table);
#endif
        for(i = 0; i < nb_offload_funcs; i++){
#ifdef OFFLOAD_MANAGER_VERBOSE          
            printf ("func%d: %x, ", i, offload_func_table[i]);
#endif
            mailbox_write((uint32_t) offload_func_table[i]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    if(nb_offload_vars)
    {
        mailbox_write(TO_RUNTIME | 2*nb_offload_vars);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(offload_var_table = %x {", offload_var_table);
#endif    
        for(i = 0; i < nb_offload_vars; i++)
        {
#ifdef OFFLOAD_MANAGER_VERBOSE
            printf ("var%d: %x size %x,", i, offload_var_table[2*i], offload_var_table[2*i+1]);
#endif
            mailbox_write((uint32_t) offload_var_table[2*i]);
            mailbox_write((uint32_t) offload_var_table[2*i+1]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    return;
}

int gomp_offload_manager ( void );
#endif