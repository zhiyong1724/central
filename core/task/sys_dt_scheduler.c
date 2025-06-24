#include "sys_dt_scheduler.h"
#include "sys_string.h"
#include "sys_error.h"
#include "sys_rand.h"
#define SYS_DTSCHED_MAX_PRIORITY                     40
#define SYS_MIN_MEAN_LOAD_CAL_CYCLE                     (1 * 1000 * 1000 * 1000l)
static const int s_weighting_table[] = 
{
    10,    11,    12,    13,    15,    16,    18,     19,                    //0-7
    21,    24,    26,    29,    31,    35,    38,     42,                    //8-15
    46,    51,    56,    61,    67,    74,    81,     90,                    //16-23
    98,    108,   119,   131,   144,   159,   174,    192,                   //24-31
    211,   232,   255,   281,   309,   340,   374,    411,                   //32-39
};

void sys_dt_scheduler_init(sys_dt_scheduler_t *dt_scheduler)
{
    sys_trace();
    dt_scheduler->min_vruntime = 0;
    dt_scheduler->task_count = 0;
    dt_scheduler->task_tree = NULL;
    dt_scheduler->running_task = NULL;
    dt_scheduler->switch_interval = 0;
    dt_scheduler->interval = 0;
    dt_scheduler->load = 0;
    dt_scheduler->acc_load = 0;
    dt_scheduler->mean_load = 0;
    dt_scheduler->mean_load_cal_interval = 0;
    dt_scheduler->mean_load_cal_cycle = SYS_MIN_MEAN_LOAD_CAL_CYCLE + sys_rand() % 1000000001;
    dt_scheduler->load_leveling_flag = 0;
}

void sys_dt_task_control_block_init(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority)
{
    sys_trace();
    dt_task_control_block->priority = priority;
    dt_task_control_block->vruntime = dt_scheduler->min_vruntime;
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

sys_dt_task_control_block_t *sys_dt_scheduler_tick(sys_dt_scheduler_t *dt_scheduler, uint64_t ns)
{
    //sys_trace();
    dt_scheduler->acc_load += dt_scheduler->load * ns;
    dt_scheduler->mean_load_cal_interval += ns;
    if (dt_scheduler->mean_load_cal_interval >= dt_scheduler->mean_load_cal_cycle)
    {
        dt_scheduler->mean_load = dt_scheduler->acc_load / dt_scheduler->mean_load_cal_interval;
        dt_scheduler->mean_load_cal_interval = 0;
        dt_scheduler->mean_load_cal_cycle = SYS_MIN_MEAN_LOAD_CAL_CYCLE + sys_rand() % 1000000001;
        dt_scheduler->acc_load = 0;
        dt_scheduler->load_leveling_flag = 1;
    }
    if (dt_scheduler->task_count > 0)
    {
        dt_scheduler->interval += ns;
        if (dt_scheduler->interval >= dt_scheduler->switch_interval)
        {
            dt_scheduler->running_task->vruntime += dt_scheduler->interval * s_weighting_table[dt_scheduler->running_task->priority];
            sys_delete_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node);
            sys_insert_node(&dt_scheduler->task_tree, &dt_scheduler->running_task->node, on_compare, dt_scheduler);
            dt_scheduler->running_task = (sys_dt_task_control_block_t *)sys_get_left_most_node(dt_scheduler->task_tree);
            dt_scheduler->min_vruntime = dt_scheduler->running_task->vruntime;
            dt_scheduler->interval = 0;
        }
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
    if (dt_task_control_block->vruntime - dt_scheduler->min_vruntime > dt_scheduler->switch_interval * dt_scheduler->task_count * s_weighting_table[dt_task_control_block->priority])
    {
        dt_task_control_block->vruntime = dt_scheduler->min_vruntime - 1;
    }
    if (NULL == dt_scheduler->running_task)
    {
        dt_scheduler->running_task = dt_task_control_block;
    }
    dt_scheduler->load += s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
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
        dt_scheduler->load -= s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
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
            dt_scheduler->running_task->vruntime += dt_scheduler->switch_interval * s_weighting_table[dt_task_control_block->priority] >> 1;
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

int sys_dt_scheduler_set_priority(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block, int priority)
{
    sys_trace();
    if (dt_task_control_block->priority < 0 || dt_task_control_block->priority >= SYS_DTSCHED_MAX_PRIORITY)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    dt_scheduler->load -= s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
    dt_task_control_block->priority = priority;
    dt_scheduler->load += s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
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
        dt_scheduler->running_task->vruntime += dt_scheduler->switch_interval * s_weighting_table[dt_scheduler->running_task->priority] >> 1;
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

sys_dt_task_control_block_t *sys_dt_scheduler_find_last_task(sys_dt_scheduler_t *dt_scheduler)
{
    sys_trace();
    return (sys_dt_task_control_block_t *)sys_get_right_most_node(dt_scheduler->task_tree);
}

uint64_t sys_dt_scheduler_load_if_remove_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block)
{
    sys_trace();
    uint64_t load = dt_scheduler->load - s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
    return load;
}

uint64_t sys_dt_scheduler_load_if_add_task(sys_dt_scheduler_t *dt_scheduler, sys_dt_task_control_block_t *dt_task_control_block)
{
    sys_trace();
    uint64_t load = dt_scheduler->load + s_weighting_table[SYS_DTSCHED_MAX_PRIORITY - dt_task_control_block->priority - 1];
    return load;
}
