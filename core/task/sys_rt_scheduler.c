#include "sys_rt_scheduler.h"
#include "sys_string.h"
#include "sys_bitmap_index.h"
#include "sys_error.h"
int sys_rt_scheduler_init(sys_rt_scheduler_t *rt_scheduler)
{
    sys_trace();
    sys_memset(rt_scheduler->ready_task_table, 0xff, SYS_RTSCHED_MAX_PRIORITY / 8);
    rt_scheduler->ready_group_table = 0xff;
    for (int i = 0; i < SYS_RTSCHED_MAX_PRIORITY; i++)
    {
        rt_scheduler->task_list_array[i] = NULL;
    }
    rt_scheduler->task_count = 0;
    rt_scheduler->running_task = NULL;
    rt_scheduler->interval = 0;
    return 0;
}

int sys_rt_task_control_block_init(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block, int priority)
{
    sys_trace();
    rt_task_control_block->priority = priority;
    return 0;
}

static int get_minimun_priority(sys_rt_scheduler_t *rt_scheduler)
{
    sys_trace();
    int i = g_bitmap_index[rt_scheduler->ready_group_table];
    int j = g_bitmap_index[rt_scheduler->ready_task_table[i]];
    return (i << 3) + j;
}

static void set_bitmap(sys_rt_scheduler_t *rt_scheduler, int priority, int value)
{
    sys_trace();
    int i = priority >> 3;
    int j = priority & 0x07;
    unsigned char mask = 0x80;
    mask >>= j;
    if (0 == value)
    {
        mask = ~mask;
        rt_scheduler->ready_task_table[i] &= mask;
    }
    else
    {
        rt_scheduler->ready_task_table[i] |= mask;
    }
    mask = 0x80;
    mask >>= i;
    if (rt_scheduler->ready_task_table[i] < 0xff)
    {
        mask = ~mask;
        rt_scheduler->ready_group_table &= mask;
    }
    else
    {
        rt_scheduler->ready_group_table |= mask;
    }
}

sys_rt_task_control_block_t *sys_rt_scheduler_tick(sys_rt_scheduler_t *rt_scheduler, uint64_t *ns)
{
    //sys_trace();
    if (rt_scheduler->task_count > 0)
    {
        rt_scheduler->interval += *ns;
        if (rt_scheduler->interval >= SYS_RTSCHED_MIN_SWITCH_INTERVAL_NS)
        {
            sys_remove_from_list(&rt_scheduler->task_list_array[rt_scheduler->running_task->priority], &rt_scheduler->running_task->node);
            sys_insert_to_back(&rt_scheduler->task_list_array[rt_scheduler->running_task->priority], &rt_scheduler->running_task->node);
            int priority = get_minimun_priority(rt_scheduler);
            rt_scheduler->running_task = (sys_rt_task_control_block_t *)rt_scheduler->task_list_array[priority];
            *ns = SYS_RTSCHED_MIN_SWITCH_INTERVAL_NS;
        }
        else
        {
            *ns = SYS_RTSCHED_MIN_SWITCH_INTERVAL_NS - rt_scheduler->interval;
        }
    }
    else
    {
        *ns = -1;
    }
    return rt_scheduler->running_task;
}

int sys_rt_scheduler_add_task(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block)
{
    sys_trace();
    if (rt_task_control_block->priority < 0 || rt_task_control_block->priority >= SYS_RTSCHED_MAX_PRIORITY)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    set_bitmap(rt_scheduler, rt_task_control_block->priority, 0);
    sys_insert_to_back(&rt_scheduler->task_list_array[rt_task_control_block->priority], &rt_task_control_block->node);
    rt_scheduler->task_count++;
    if (NULL == rt_scheduler->running_task)
    {
        rt_scheduler->running_task = rt_task_control_block;
    }
    return 0;
}

sys_rt_task_control_block_t *sys_rt_scheduler_remove_task(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block)
{
    sys_trace();
    if (rt_scheduler->task_count > 0)
    {
        sys_remove_from_list(&rt_scheduler->task_list_array[rt_task_control_block->priority], &rt_task_control_block->node);
        if (NULL == rt_scheduler->task_list_array[rt_task_control_block->priority])
        {
            set_bitmap(rt_scheduler, rt_task_control_block->priority, 1);
        }
        rt_scheduler->task_count--;
        if (rt_scheduler->running_task == rt_task_control_block)
        {
            if (0 == rt_scheduler->task_count)
            {
                rt_scheduler->running_task = NULL;
            }
            else
            {
                int priority = get_minimun_priority(rt_scheduler);
                rt_scheduler->running_task = (sys_rt_task_control_block_t *)rt_scheduler->task_list_array[priority];
            }
        }
    }
    return rt_scheduler->running_task;
}

int sys_rt_scheduler_modify_priority(sys_rt_scheduler_t *rt_scheduler, sys_rt_task_control_block_t *rt_task_control_block, int priority)
{
    sys_trace();
    if (rt_task_control_block->priority < 0 || rt_task_control_block->priority >= SYS_RTSCHED_MAX_PRIORITY)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    sys_remove_from_list(&rt_scheduler->task_list_array[rt_task_control_block->priority], &rt_task_control_block->node);
    if (NULL == rt_scheduler->task_list_array[rt_task_control_block->priority])
    {
        set_bitmap(rt_scheduler, rt_task_control_block->priority, 1);
    }
    rt_task_control_block->priority = priority;
    set_bitmap(rt_scheduler, rt_task_control_block->priority, 0);
    sys_insert_to_back(&rt_scheduler->task_list_array[rt_task_control_block->priority], &rt_task_control_block->node);
    return 0;
}

sys_rt_task_control_block_t *sys_rt_scheduler_get_running_task(sys_rt_scheduler_t *rt_scheduler)
{
    sys_trace();
    return rt_scheduler->running_task;
}

sys_rt_task_control_block_t *sys_rt_scheduler_yield(sys_rt_scheduler_t *rt_scheduler)
{
    sys_trace();
    if (rt_scheduler->task_count > 0)
    {
        sys_remove_from_list(&rt_scheduler->task_list_array[rt_scheduler->running_task->priority], &rt_scheduler->running_task->node);
        sys_insert_to_back(&rt_scheduler->task_list_array[rt_scheduler->running_task->priority], &rt_scheduler->running_task->node);
        int priority = get_minimun_priority(rt_scheduler);
        sys_rt_task_control_block_t *next_task = (sys_rt_task_control_block_t *)rt_scheduler->task_list_array[priority];
        if (next_task != rt_scheduler->running_task)
        {
            rt_scheduler->running_task = next_task;
        }
    }
    return rt_scheduler->running_task;
}