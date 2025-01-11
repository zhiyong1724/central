#include "sys_semaphore_manager.h"
#include "sys_task.h"
#include "sys_error.h"
#define SYS_SEMAPHORE_MAX_WAIT_TIME ((uint64_t)-1 / 1000 / 1000)
int sys_task_wakeup(sys_tid_t tid);
sys_task_t *sys_task_get_running_task();
int sys_semaphore_manager_init(sys_semaphore_manager_t *semaphore_manager, sys_task_manager_t *task_manager)
{
    sys_trace();
    semaphore_manager->task_manager = task_manager;
    return 0;
}

int sys_semaphore_manager_semaphore_init(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore, int count, int max_count)
{
    sys_trace();
    semaphore->count = count;
    semaphore->max_count = max_count;
    semaphore->high_priority_task = NULL;
    semaphore->wait_rt_task_list = NULL;
    semaphore->wait_task_list = NULL;
    return 0;
}

int sys_semaphore_manager_post(sys_semaphore_manager_t *semaphore_manager, sys_task_t **next_task, sys_semaphore_t *semaphore)
{
    sys_trace();
    int ret = 0;
    if (semaphore->count > semaphore->max_count)
    {
        return -1;
    }
    sys_task_t *task = NULL;
    unsigned char *high_priority_task = (unsigned char *)semaphore->high_priority_task;
    if (high_priority_task != NULL)
    {
        sys_delete_node(&semaphore->wait_rt_task_list, (sys_tree_node_t *)high_priority_task);
        semaphore->high_priority_task = sys_get_left_most_node(semaphore->wait_rt_task_list);
        task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
    }
    if (NULL == task)
    {
        high_priority_task = (unsigned char *)semaphore->wait_task_list;
        if (high_priority_task != NULL)
        {
            sys_remove_from_list(&semaphore->wait_task_list, (sys_list_node_t *)high_priority_task);
            task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
        }
    }
    if (NULL == task)
    {
        semaphore->count++;
    }
    else
    {
        task->arg = semaphore;
        if (SYS_TASK_STATE_BLOCKED == task->task_control_block.task_state)
        {
            task->task_control_block.task_state = SYS_TASK_STATE_SUSPENDED;
            ret = sys_task_manager_resume(semaphore_manager->task_manager, next_task, task->tid);
        }
        else
        {
            ret = sys_task_manager_wakeup(semaphore_manager->task_manager, next_task, task->tid);
        }
    }
    return ret;
}

static int on_compare(void *key1, void *key2, void *arg)
{
	sys_trace();
    sys_task_t *task1 = (sys_task_t *)((unsigned char *)key1 - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task1->real_task_control_block));
    sys_task_t *task2 = (sys_task_t *)((unsigned char *)key2 - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task2->real_task_control_block));
    if (task1->real_task_control_block.rt_task_control_block.priority < task2->real_task_control_block.rt_task_control_block.priority)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

int sys_semaphore_manager_wait(sys_semaphore_manager_t *semaphore_manager, sys_task_t **next_task, sys_semaphore_t *semaphore, uint64_t wait)
{
    sys_trace();
    int ret = 0;
    *next_task = NULL;
    if (semaphore->count > 0)
    {
        semaphore->count--;
    }
    else
    {
        sys_task_t *task = sys_task_manager_get_running_task(semaphore_manager->task_manager);
        if (wait > 0 && wait < SYS_SEMAPHORE_MAX_WAIT_TIME)
        {
            sys_task_manager_sleep(semaphore_manager->task_manager, next_task, wait * 1000 * 1000);
        }
        else if (SYS_SEMAPHORE_MAX_WAIT_TIME == wait)
        {
            ret = sys_task_manager_supend(semaphore_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
            task->task_control_block.task_state = SYS_TASK_STATE_BLOCKED;
        }
        if (SYS_TASK_TYPE_RT == task->task_control_block.scheduler_id)
        {
            sys_insert_node(&semaphore->wait_rt_task_list, &task->exnode.tree_node, on_compare, NULL);
            if (NULL == semaphore->high_priority_task)
            {
                semaphore->high_priority_task = &task->exnode.tree_node;
            }
            else
            {
                sys_task_t *high_priority_task = (sys_task_t *)((unsigned char *)semaphore->high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
                if (task->real_task_control_block.rt_task_control_block.priority < high_priority_task->real_task_control_block.rt_task_control_block.priority)
                {
                    semaphore->high_priority_task = &task->exnode.tree_node;
                }
            }
        }
        else
        {
            sys_insert_to_back(&semaphore->wait_task_list, &task->exnode.list_node);
        }
    }
    return 0;
}

int sys_semaphore_manager_get_semaphore_count(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore)
{
    sys_trace();
    return semaphore->count;
}

int sys_semaphore_manager_get_max_semaphore_count(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore)
{
    sys_trace();
    return semaphore->max_count;
}

int sys_semaphore_manager_remove_task(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore, sys_task_t *task)
{
    sys_trace();
    if (SYS_TASK_TYPE_RT == task->task_control_block.scheduler_id)
    {
        sys_delete_node(&semaphore->wait_rt_task_list, &task->exnode.tree_node);
        semaphore->high_priority_task = sys_get_left_most_node(semaphore->wait_rt_task_list);
    }
    else
    {
        sys_remove_from_list(&semaphore->wait_task_list, &task->exnode.list_node);
    }
    return 0;
}

void sys_semaphore_manager_reset(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore)
{
    sys_trace();
    semaphore->count = 0;
}