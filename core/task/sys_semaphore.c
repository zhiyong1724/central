#include "sys_semaphore.h"
#include "sys_semaphore_manager.h"
int port_yield(stack_size_t **stack_top);
int port_disable_interrupts();
int port_recovery_interrupts(int state);
sys_task_t *sys_task_get_running_task();
static sys_semaphore_manager_t *s_semaphore_manager;
int sys_semaphore_init(sys_semaphore_manager_t *semaphore_manager, sys_task_manager_t *task_manager)
{
    sys_trace();
    s_semaphore_manager = semaphore_manager;
    return sys_semaphore_manager_init(s_semaphore_manager, task_manager);
}

int sys_semaphore_create(sys_semaphore_t *semaphore, int count, int max_count)
{
    sys_trace();
    if (0 == max_count)
    {
        max_count = SYS_MAX_SEMAPHORE_COUNT;
    }
    return sys_semaphore_manager_semaphore_init(s_semaphore_manager, semaphore, count, max_count);
}

int sys_semaphore_destory(sys_semaphore_t *semaphore)
{
    sys_trace();
    int ret = -1;
    int state = port_disable_interrupts();
    if (NULL == semaphore->wait_rt_task_list && NULL == semaphore->wait_task_list)
    {
        sys_semaphore_reset(semaphore);
        ret = 0;
    }
    port_recovery_interrupts(state);
    return ret;
}

void sys_semaphore_reset(sys_semaphore_t *semaphore)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_semaphore_manager_reset(s_semaphore_manager, semaphore);
    port_recovery_interrupts(state);
}

int sys_semaphore_post(sys_semaphore_t *semaphore)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = NULL;
    int state = port_disable_interrupts();
    ret = sys_semaphore_manager_post(s_semaphore_manager, &task, semaphore);
    if (0 == ret)
    {
        if (task != NULL)
        {
            port_yield(&task->stack_top);
        }
    }
    port_recovery_interrupts(state);
    return ret;
}

int sys_semaphore_wait1(sys_semaphore_t *semaphore, uint64_t wait)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = NULL;
    ret = sys_semaphore_manager_wait(s_semaphore_manager, &task, semaphore, wait);
    if (0 == ret)
    {
        if (task != NULL)
        {
            port_yield(&task->stack_top);
            port_recovery_interrupts(1);
            port_disable_interrupts();
            task = sys_task_get_running_task();
            if (task->arg != NULL)
            {
                task->arg = NULL;
            }
            else
            {
                sys_semaphore_manager_remove_task(s_semaphore_manager, semaphore, task);
                ret = -1;
            }
        }
    }
    return ret;
}

int sys_semaphore_wait(sys_semaphore_t *semaphore, uint64_t wait)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_semaphore_wait1(semaphore, wait);
    port_recovery_interrupts(state);
    return ret;
}

int sys_semaphore_get_semaphore_count(sys_semaphore_t *semaphore)
{
    sys_trace();
    return sys_semaphore_manager_get_semaphore_count(s_semaphore_manager, semaphore);
}

int sys_semaphore_get_max_semaphore_count(sys_semaphore_t *semaphore)
{
    sys_trace();
    return sys_semaphore_manager_get_max_semaphore_count(s_semaphore_manager, semaphore);
}