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

    mailbox_write(TO_RUNTIME | 2);
    mailbox_write(nb_offload_funcs);
    mailbox_write(nb_offload_vars);

    if(nb_offload_funcs) {
        mailbox_write(TO_RUNTIME | nb_offload_funcs);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(func_table = %x {", func_table);
#endif
        for(i = 0; i < nb_offload_funcs; i++){
#ifdef OFFLOAD_MANAGER_VERBOSE          
            printf ("func%d: %x, ", i, func_table[i]);
#endif
            mailbox_write(func_table[i]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    if(nb_offload_vars)
    {
        mailbox_write(TO_RUNTIME | 2*nb_offload_vars);
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf("(var_table = %x {", var_table);
#endif    
        for(i = 0; i < nb_offload_vars; i++)
        {
#ifdef OFFLOAD_MANAGER_VERBOSE
            printf ("var%d: %x size %x,", i, var_table[2*i], var_table[2*i+1]);
#endif
            mailbox_write(offload_var_table[2*i]);
            mailbox_write(offload_var_table[2*i+1]);
        }
#ifdef OFFLOAD_MANAGER_VERBOSE
        printf ("\n");
#endif
    }

    return;
}

void gomp_offload_manager ( void );
#endif