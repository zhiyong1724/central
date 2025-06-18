#include "sys_vscheduler.h"
void sys_vscheduler_init(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    for (int i = 0; i < SYS_MAX_SCHEDULER_COUNT; i++)
    {
        vscheduler->schedulers[i] = NULL;
    }
    vscheduler->scheduler_count = 0;
    vscheduler->task_count = 0;
    vscheduler->running_task = NULL;
}

void sys_vtask_control_block_init(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block)
{
    sys_trace();
    task_control_block->scheduler_id = 0;
    task_control_block->task_state = SYS_TASK_STATE_DEAD;
}

int sys_vscheduler_add_scheduler(sys_vscheduler_t *vscheduler, void *scheduler, sys_scheduler_interfaces_t *scheduler_interfaces)
{
    sys_trace();
    if (vscheduler->scheduler_count < SYS_MAX_SCHEDULER_COUNT)
    {
        vscheduler->schedulers[vscheduler->scheduler_count] = scheduler;
        vscheduler->scheduler_interfaces[vscheduler->scheduler_count] = *scheduler_interfaces;
        vscheduler->scheduler_count++;
        return 0;
    }
    return -1;
}

int sys_vscheduler_add_task(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT)
    {
        task_control_block->task_state = SYS_TASK_STATE_READY;
        if (NULL == vscheduler->running_task)
        {
            vscheduler->running_task = task_control_block;
        }
        vscheduler->task_count++;
        return vscheduler->scheduler_interfaces[task_control_block->scheduler_id].add_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
    }
    return -1;
}

int sys_vscheduler_set_priority(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block, int priority)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT)
    {
        return vscheduler->scheduler_interfaces[task_control_block->scheduler_id].set_priority(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1, priority);
    }
    return -1;
}

sys_vtask_control_block_t *sys_vscheduler_tick(sys_vscheduler_t *vscheduler, uint64_t ns)
{
    //sys_trace();
    for (int i = 0; i < vscheduler->scheduler_count; i++)
    {
        vscheduler->running_task = (sys_vtask_control_block_t *)vscheduler->scheduler_interfaces[i].tick(vscheduler->schedulers[i], ns);
        if (vscheduler->running_task != NULL)
        {
            break;
        }
    }
    if (vscheduler->running_task != NULL)
    {
        vscheduler->running_task--;
    }
    return vscheduler->running_task;
}

sys_vtask_control_block_t *sys_vscheduler_suspend(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT && SYS_TASK_STATE_READY == task_control_block->task_state)
    {
        vscheduler->scheduler_interfaces[task_control_block->scheduler_id].remove_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
        task_control_block->task_state = SYS_TASK_STATE_SUSPEND;
        vscheduler->task_count--;
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_vtask_control_block_t *)vscheduler->scheduler_interfaces[i].get_running_task(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

sys_vtask_control_block_t *sys_vscheduler_resume(sys_vscheduler_t *vscheduler, sys_vtask_control_block_t *task_control_block)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT && SYS_TASK_STATE_SUSPEND == task_control_block->task_state)
    {
        vscheduler->scheduler_interfaces[task_control_block->scheduler_id].add_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
        task_control_block->task_state = SYS_TASK_STATE_READY;
        vscheduler->task_count++;
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_vtask_control_block_t *)vscheduler->scheduler_interfaces[i].yield(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

sys_vtask_control_block_t *sys_vscheduler_get_running_task(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    return vscheduler->running_task;
}

int sys_vscheduler_get_task_count(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    return vscheduler->task_count;
}