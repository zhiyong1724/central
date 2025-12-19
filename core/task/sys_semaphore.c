#include "sys_semaphore.h"
#include "sys_task_scheduler.h"
sys_task_t *sys_task_sleep_inner(uint64_t ms, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg);
int sys_task_wakeup_inner(sys_task_t *task);
sys_task_t *sys_task_suspend_inner();
int sys_task_resume_inner(sys_task_t *task);
void sys_semaphore_init(sys_semaphore_t *semaphore, int count, int max_count)
{
    sys_trace();
    semaphore->count = count;
    semaphore->max_count = max_count;
    semaphore->wait_fifo_task_list = NULL;
    semaphore->wait_rt_task_list = NULL;
    semaphore->wait_task_list = NULL;
    sys_spin_lock_init(&semaphore->lock);
}

int sys_semaphore_post(sys_semaphore_t *semaphore)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&semaphore->lock);
    sys_task_t *task = NULL;
    if (semaphore->wait_fifo_task_list != NULL)
    {
        task = sys_container_of(semaphore->wait_fifo_task_list, sys_task_t, exnode);
        sys_remove_from_list(&semaphore->wait_fifo_task_list, &task->exnode.list_node);
    }
    else if (semaphore->wait_rt_task_list != NULL)
    {
        task = sys_container_of(sys_get_left_most_node(semaphore->wait_rt_task_list), sys_task_t, exnode);
        sys_delete_node(&semaphore->wait_rt_task_list, &task->exnode.tree_node);
    }
    else if (semaphore->wait_task_list != NULL)
    {
        task = sys_container_of(semaphore->wait_task_list, sys_task_t, exnode);
        sys_remove_from_list(&semaphore->wait_task_list, &task->exnode.list_node);
    }
    if (task != NULL)
    {
        task->task_control_block.wait = 0;
        if (task->task_control_block.sleep_time < SYS_SEMAPHORE_MAX_WAIT_TIME)
        {
            sys_task_wakeup_inner(task);
        }
        else
        {
            sys_task_resume_inner(task);
        }
    }
    else
    {
        if (semaphore->count < semaphore->max_count)
        {
            semaphore->count++;
        }
        else
        {
            ret = -1;
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&semaphore->lock, state);
    return ret;
}

static int on_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    sys_task_t *task1 = sys_container_of(key1, sys_task_t, exnode);
    sys_task_t *task2 = sys_container_of(key2, sys_task_t, exnode);
    if (task1->task_control_block.real_task_control_block.rt_task_control_block.priority < task2->task_control_block.real_task_control_block.rt_task_control_block.priority)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

static void wakeup_callback(sys_task_control_block_t *task_control_block, void *arg)
{
    sys_trace();
    sys_semaphore_t *semaphore = (sys_semaphore_t *)arg;
    sys_task_t *task = sys_container_of(task_control_block, sys_task_t, task_control_block);
    int state = sys_spin_lock_lock_and_irq_save(&semaphore->lock);
    if (SYS_TASK_TYPE_RT == task_control_block->vtask_control_block.scheduler_id)
    {
        sys_delete_node(&semaphore->wait_rt_task_list, &task->exnode.tree_node);
    }
    else
    {
        sys_remove_from_list(&semaphore->wait_task_list, &task->exnode.list_node);
    }
    sys_spin_lock_unlock_and_irq_restore(&semaphore->lock, state);
}

int sys_semaphore_wait(sys_semaphore_t *semaphore, uint64_t wait)
{
    sys_trace();
    int ret = 0;
    sys_task_t *task = NULL;
    int state = sys_spin_lock_lock_and_irq_save(&semaphore->lock);
    if (semaphore->count > 0)
    {
        semaphore->count--;
    }
    else
    {
        if (wait < SYS_SEMAPHORE_MAX_WAIT_TIME)
        {
            task = sys_task_sleep_inner(wait, wakeup_callback, semaphore);
        }
        else
        {
            task = sys_task_suspend_inner();
            task->task_control_block.sleep_time = SYS_SEMAPHORE_MAX_WAIT_TIME;
        }
        if (SYS_TASK_TYPE_FIFO == task->task_control_block.vtask_control_block.scheduler_id)
        {
            sys_insert_to_back(&semaphore->wait_fifo_task_list, &task->exnode.list_node);
        }
        else if (SYS_TASK_TYPE_RT == task->task_control_block.vtask_control_block.scheduler_id)
        {
            sys_insert_node(&semaphore->wait_rt_task_list, &task->exnode.tree_node, on_compare, NULL);
        }
        else
        {
            sys_insert_to_back(&semaphore->wait_task_list, &task->exnode.list_node);
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&semaphore->lock, state);
    if (task != NULL)
    {
        ret = task->task_control_block.wait;
    }
    return ret;
}

int sys_semaphore_get_semaphore_count(sys_semaphore_t *semaphore)
{
    sys_trace();
    return semaphore->count;
}

int sys_semaphore_get_max_semaphore_count(sys_semaphore_t *semaphore)
{
    sys_trace();
    return semaphore->max_count;
}