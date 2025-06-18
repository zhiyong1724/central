#include "sys_fifo_scheduler.h"
void sys_fifo_scheduler_init(sys_fifo_scheduler_t *fifo_scheduler)
{
    sys_trace();
    fifo_scheduler->task_list = NULL;
    fifo_scheduler->task_count = 0;
    fifo_scheduler->running_task = NULL;
    fifo_scheduler->interval = 0;
}

void sys_fifo_task_control_block_init(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block, int priority)
{
    sys_trace();
    fifo_task_control_block->priority = priority;
}

sys_fifo_task_control_block_t *sys_fifo_scheduler_tick(sys_fifo_scheduler_t *fifo_scheduler, uint64_t ns)
{
    //sys_trace();
    if (fifo_scheduler->task_count > 0)
    {
        fifo_scheduler->interval += ns;
        if (fifo_scheduler->interval >= SYS_FIFOSCHED_MIN_SWITCH_INTERVAL_NS)
        {
            sys_remove_from_list(&fifo_scheduler->task_list, &fifo_scheduler->running_task->node);
            sys_insert_to_back(&fifo_scheduler->task_list, &fifo_scheduler->running_task->node);
            fifo_scheduler->running_task = (sys_fifo_task_control_block_t *)fifo_scheduler->task_list;
        }
    }
    return fifo_scheduler->running_task;
}

int sys_fifo_scheduler_add_task(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block)
{
    sys_trace();
    sys_insert_to_back(&fifo_scheduler->task_list, &fifo_task_control_block->node);
    fifo_scheduler->task_count++;
    if (NULL == fifo_scheduler->running_task)
    {
        fifo_scheduler->running_task = fifo_task_control_block;
    }
    return 0;
}

sys_fifo_task_control_block_t *sys_fifo_scheduler_remove_task(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block)
{
    sys_trace();
    if (fifo_scheduler->task_count > 0)
    {
        sys_remove_from_list(&fifo_scheduler->task_list, &fifo_task_control_block->node);
        fifo_scheduler->task_count--;
        if (fifo_scheduler->running_task == fifo_task_control_block)
        {
            fifo_scheduler->running_task = (sys_fifo_task_control_block_t *)fifo_scheduler->task_list;
        }
    }
    return fifo_scheduler->running_task;
}

int sys_fifo_scheduler_set_priority(sys_fifo_scheduler_t *fifo_scheduler, sys_fifo_task_control_block_t *fifo_task_control_block, int priority)
{
    sys_trace();
    fifo_task_control_block->priority = priority;
    return 0;
}

sys_fifo_task_control_block_t *sys_fifo_scheduler_get_running_task(sys_fifo_scheduler_t *fifo_scheduler)
{
    sys_trace();
    return fifo_scheduler->running_task;
}

sys_fifo_task_control_block_t *sys_fifo_scheduler_yield(sys_fifo_scheduler_t *fifo_scheduler)
{
    sys_trace();
    if (fifo_scheduler->task_count > 0)
    {
        sys_remove_from_list(&fifo_scheduler->task_list, &fifo_scheduler->running_task->node);
        sys_insert_to_back(&fifo_scheduler->task_list, &fifo_scheduler->running_task->node);
        sys_fifo_task_control_block_t *next_task = (sys_fifo_task_control_block_t *)fifo_scheduler->task_list;
        fifo_scheduler->running_task = next_task;
    }
    return fifo_scheduler->running_task;
}