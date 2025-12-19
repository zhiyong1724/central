#include "central.h"
#include <stdio.h>
static void dump_trace()
{
    void *array[64];
    int len = sys_backtrace(array, 64);
    for (int i = 0; i < len; i++)
    {
        printf("trace:%p\n", array[i]);
    }
}

__attribute__((weak)) void instruction_address_misaligned()
{
    printf("Exception instruction_address_misaligned.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void instruction_access_fault()
{
    printf("Exception instruction_access_fault.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void illegal_instruction()
{
    printf("Exception illegal_instruction.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void breakpoint()
{
}

__attribute__((weak)) void load_address_misaligned()
{
    printf("Exception load_address_misaligned.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void load_access_fault()
{
    printf("Exception load_access_fault.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void store_address_misaligned()
{
    printf("Exception store_address_misaligned.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void store_access_fault()
{
    printf("Exception store_access_fault.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void env_call_from_umode()
{
}

__attribute__((weak)) void env_call_from_smode()
{
}

__attribute__((weak)) void env_call_from_mmode()
{
}

__attribute__((weak)) void illegal_page_fault()
{
    printf("store_page_fault illegal_page_fault.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void load_page_fault()
{
    printf("store_page_fault load_page_fault.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void store_page_fault()
{
    printf("store_page_fault bad_trap.\n");
    dump_trace();
    for (;;)
    {
    }
}

__attribute__((weak)) void supervisor_software_interrupt()
{
}

// __attribute__((weak)) void machine_software_interrupt()
// {
// }

__attribute__((weak)) void supervisor_timer_interrupt()
{
}

// __attribute__((weak)) void machine_timer_interrupt()
// {
// }

__attribute__((weak)) void supervisor_external_interrupt()
{
}

__attribute__((weak)) void machine_external_interrupt()
{
}

__attribute__((weak)) void bad_trap()
{
    printf("Exception bad_trap.\n");
    dump_trace();
    for (;;)
    {
    }
}