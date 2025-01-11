#include "sys_idle_scheduler.h"
int sys_idle_scheduler_init(sys_idle_scheduler_t *idle_scheduler)
{
    sys_trace();
    idle_scheduler->running_task = NULL;
    return 0;
}

int sys_idle_task_control_block_init(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block, int priority)
{
    sys_trace();
    idle_task_control_block->priority = priority;
    return 0;
}

sys_idle_task_control_block_t *sys_idle_scheduler_tick(sys_idle_scheduler_t *idle_scheduler, uint64_t *ns)
{
    sys_trace();
    *ns = -1;
    return idle_scheduler->running_task;
}

int sys_idle_scheduler_add_task(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block)
{
    sys_trace();
    idle_scheduler->running_task = idle_task_control_block;
    return 0;
}

sys_idle_task_control_block_t *sys_idle_scheduler_remove_task(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block)
{
    sys_trace();
    if (idle_scheduler->running_task == idle_task_control_block)
    {
        idle_scheduler->running_task = NULL;
    }
    return idle_scheduler->running_task;
}

int sys_idle_scheduler_modify_priority(sys_idle_scheduler_t *idle_scheduler, sys_idle_task_control_block_t *idle_task_control_block, int priority)
{
    sys_trace();
    idle_task_control_block->priority = priority;
    return 0;
}

sys_idle_task_control_block_t *sys_idle_scheduler_get_running_task(sys_idle_scheduler_t *idle_scheduler)
{
    sys_trace();
    return idle_scheduler->running_task;
}

sys_idle_task_control_block_t *sys_idle_scheduler_yield(sys_idle_scheduler_t *idle_scheduler)
{
    sys_trace();
    return idle_scheduler->running_task;
}