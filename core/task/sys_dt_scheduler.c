#include "sys_dt_scheduler.h"
#include "sys_string.h"
#include "sys_error.h"
#define SYS_DTSCHED_MAX_PRIORITY                     40
static const int s_weighting_table[] = 
{
    10,    12,    15,    18,    22,    27,    33,     40,                    //0-7
    49,    60,    73,    89,    109,   133,   162,    197,                   //8-15
    241,   294,   358,   437,   534,   651,   794,    969,                   //16-23
    1182,  1442,  1759,  2146,  2619,  3195,  3898,   4755,                  //24-31
    5801,  7077,  8634,  10534, 12852, 15679, 19128,  23336,                 //32-39
};

int sys_dt_scheduler_init(sys_dt_scheduler_t *dt_scheduler)
{
    sys_trace();
    dt_scheduler->min_vruntime = 0;
    dt_scheduler->task_count = 0;
    dt_scheduler->task_tree = NULL;
    dt_scheduler->running_task = NULL;
    dt_scheduler->switch_interval = 0;
    dt_scheduler->interval = 0; 
    return 0;
}

int sys_dt_task_control_block_init(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority)
{
    sys_trace();
    dt_task_control_block->priority = priority;
    dt_task_control_block->vruntime = dt_scheduler->min_vruntime;
    return 0;
}

static int on_compare(void *key1, void *key2, void *arg)
{
	sys_trace();
    sys_dt_scheduler_t *dt_scheduler = (sys_dt_scheduler_t *)arg;
    sys_dt_task_control_block_t *task1 = (sys_dt_task_control_block_t *)key1;
    sys_dt_task_control_block_t *task2 = (sys_dt_task_control_block_t *)key2;
    if (task1->vruntime - dt_scheduler->min_vruntime < task2->vruntime - dt_scheduler->min_vruntime)
    {
        return -1;
    }
    else if (task1->vruntime == dt_scheduler->min_vruntime - 1)
    {
        task1->vruntime = dt_scheduler->min_vruntime;
        return -1;
    }
    else
    {
        return 1;
    }
}

sys_dt_task_control_block_t *sys_dt_scheduler_tick(sys_dt_scheduler_t *dt_scheduler, uint64_t *ns)
{
    //sys_trace();
    if (dt_scheduler->task_count > 0)
    {
        dt_scheduler->interval += *ns;
        if (dt_scheduler->interval >= dt_scheduler->switch_interval)
        {
            dt_scheduler->running_task->vruntime += dt_scheduler->interval * s_weighting_table[dt_scheduler->running_task->priority];
            sys_delete_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node);
            sys_insert_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node, on_compare, dt_scheduler);
            dt_scheduler->running_task = (sys_dt_task_control_block_t *)sys_get_left_most_node(dt_scheduler->task_tree);
            dt_scheduler->min_vruntime = dt_scheduler->running_task->vruntime;
            dt_scheduler->interval = 0;
            *ns = dt_scheduler->switch_interval;
        }
        else
        {
            *ns = dt_scheduler->switch_interval - dt_scheduler->interval;
        }
    }
    else
    {
        *ns = -1;
    }
    return dt_scheduler->running_task;
}

int sys_dt_scheduler_add_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block)
{
    sys_trace();
    if (dt_task_control_block->priority < 0 || dt_task_control_block->priority >= SYS_DTSCHED_MAX_PRIORITY)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    if (dt_task_control_block->vruntime - dt_scheduler->min_vruntime > SYS_DTSCHED_MAX_SCHED_CYCLE_NS * s_weighting_table[dt_task_control_block->priority])
    {
        dt_task_control_block->vruntime = dt_scheduler->min_vruntime - 1;
    }
    if (NULL == dt_scheduler->running_task)
    {
        dt_scheduler->running_task = dt_task_control_block;
    }
    sys_insert_node(&dt_scheduler->task_tree, &dt_task_control_block->node, on_compare, dt_scheduler);
    dt_scheduler->task_count++;
    dt_scheduler->switch_interval = SYS_DTSCHED_MAX_SCHED_CYCLE_NS / dt_scheduler->task_count;
    if (dt_scheduler->switch_interval < SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS)
    {
        dt_scheduler->switch_interval = SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS;
    }
    return 0;
}

sys_dt_task_control_block_t *sys_dt_scheduler_remove_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block)
{
    sys_trace();
    if (dt_scheduler->task_count > 0)
    {
        sys_delete_node(&dt_scheduler->task_tree, &dt_task_control_block->node);
        dt_scheduler->task_count--;
        if (dt_scheduler->task_count > 0)
        {
            dt_scheduler->switch_interval = SYS_DTSCHED_MAX_SCHED_CYCLE_NS / dt_scheduler->task_count;
            if (dt_scheduler->switch_interval < SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS)
            {
                dt_scheduler->switch_interval = SYS_DTSCHED_MIN_SWITCH_INTERVAL_NS;
            }
        }
        else
        {
            dt_scheduler->switch_interval = 0;
        }

        if (dt_scheduler->running_task == dt_task_control_block)
        {
            dt_scheduler->running_task->vruntime += dt_scheduler->switch_interval * s_weighting_table[dt_task_control_block->priority] / 2;
            if (0 == dt_scheduler->task_count)
            {
                dt_scheduler->running_task = NULL;
            }
            else
            {
                dt_scheduler->running_task = (sys_dt_task_control_block_t *)sys_get_left_most_node(dt_scheduler->task_tree);
                dt_scheduler->min_vruntime = dt_scheduler->running_task->vruntime;
            }
        }
    }
    return dt_scheduler->running_task;
}

int sys_dt_scheduler_modify_priority(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority)
{
    sys_trace();
    if (dt_task_control_block->priority < 0 || dt_task_control_block->priority >= SYS_DTSCHED_MAX_PRIORITY)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    dt_task_control_block->priority = priority;
    return 0;
}

sys_dt_task_control_block_t *sys_dt_scheduler_get_running_task(sys_dt_scheduler_t *dt_scheduler)
{
    sys_trace();
    return dt_scheduler->running_task;
}

sys_dt_task_control_block_t *sys_dt_scheduler_yield(sys_dt_scheduler_t *dt_scheduler)
{
    sys_trace();
    if (dt_scheduler->task_count > 0)
    {
        dt_scheduler->running_task->vruntime += dt_scheduler->switch_interval * s_weighting_table[dt_scheduler->running_task->priority] / 2;
        sys_delete_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node);
        sys_insert_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node, on_compare, dt_scheduler);
        sys_dt_task_control_block_t *next_task = (sys_dt_task_control_block_t *)sys_get_left_most_node(dt_scheduler->task_tree);
        dt_scheduler->min_vruntime = next_task->vruntime;
        if (next_task != dt_scheduler->running_task)
        {
            dt_scheduler->running_task = next_task;
        }
    }
    return dt_scheduler->running_task;
}
