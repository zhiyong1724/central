#include "sys_msg_queue.h"
#include "sys_queue_manager.h"
#include "sys_mem.h"
#include "sys_string.h"
int port_yield(stack_size_t **stack_top);
int port_disable_interrupts();
int port_recovery_interrupts(int state);
sys_task_t *sys_task_get_running_task();
static sys_queue_manager_t *s_queue_manager;
int sys_msg_queue_init(sys_queue_manager_t *queue_manager, sys_task_manager_t *task_manager)
{
    sys_trace();
    s_queue_manager = queue_manager;
    return sys_queue_manager_init(s_queue_manager, task_manager);
}

int sys_msg_queue_create(sys_msg_queue_t *queue, int queue_length, int message_size)
{
    sys_trace();
    if (0 == queue_length)
    {
        queue_length = SYS_MAX_QUEUE_LENGTH;
    }
    return sys_queue_manager_queue_init(s_queue_manager, queue, queue_length, message_size);
}

int sys_msg_queue_destory(sys_msg_queue_t *queue)
{
    sys_trace();
    int ret = -1;
    int state = port_disable_interrupts();
    if (NULL == queue->wait_task_list && NULL == queue->wait_task_list)
    {
        sys_queue_manager_queue_uninit(s_queue_manager, queue);
        ret = 0;
    }
    port_recovery_interrupts(state);
    return ret;
}

void sys_msg_queue_reset(sys_msg_queue_t *queue)
{
    sys_trace();
    int state = port_disable_interrupts();
    sys_queue_manager_reset(s_queue_manager, queue);
    port_recovery_interrupts(state);
}

int sys_msg_queue_send(sys_msg_queue_t *queue, void *message)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = NULL;
    int state = port_disable_interrupts();
    ret = sys_queue_manager_send(s_queue_manager, queue, message, &task);
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

int sys_msg_queue_send_to_front(sys_msg_queue_t *queue, void *message)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = NULL;
    int state = port_disable_interrupts();
    ret = sys_queue_manager_send_to_front(s_queue_manager, queue, message, &task);
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

int sys_msg_queue_receive(sys_msg_queue_t *queue, void *message, uint64_t wait)
{
    sys_trace();
    int ret = -1;
    sys_task_t *task = NULL;
    int state = port_disable_interrupts();
    ret = sys_queue_manager_receive(s_queue_manager, message, &task, queue, wait);
    if (0 == ret)
    {
        if (task != NULL)
        {
            port_yield(&task->stack_top);
            port_recovery_interrupts(state);
            state = port_disable_interrupts();
            task = sys_task_get_running_task();
            if (task->arg != NULL)
            {
                task->arg = NULL;
                ret = sys_msg_queue_receive(queue, message, wait);
            }
            else
            {
                sys_queue_manager_remove_task(s_queue_manager, queue, task);
                ret = -1;
            }
        }
    }
    port_recovery_interrupts(state);
    return ret;
}

int sys_msg_queue_get_message_count(sys_msg_queue_t *queue)
{
    sys_trace();
    return sys_queue_manager_get_message_count(s_queue_manager, queue);
}

int sys_msg_queue_get_queue_length(sys_msg_queue_t *queue)
{
    sys_trace();
    return sys_queue_manager_get_queue_length(s_queue_manager, queue);
}