#include "sys_queue_manager.h"
#include "sys_task.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
#define SYS_MESSAGE_MAX_WAIT_TIME ((uint64_t)-1 / 1000 / 1000)
int sys_task_wakeup(sys_tid_t tid);
sys_task_t *sys_task_get_running_task();
int sys_queue_manager_init(sys_queue_manager_t *queue_manager, sys_task_manager_t *task_manager)
{
    sys_trace();
    queue_manager->task_manager = task_manager;
    return 0;
}

int sys_queue_manager_queue_init(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, int queue_length, int message_size)
{
    sys_trace();
    queue->buffer = (unsigned char *)sys_malloc(queue_length * message_size);
    if (NULL == queue->buffer)
    {
        sys_error("Out of memory.");
        return SYS_ERROR_NOMEM;
    }
    queue->message_count = 0;
    queue->length = queue_length;
    queue->message_size = message_size;
    queue->write_index = 0;
    queue->read_index = 0;
    queue->high_priority_task = NULL;
    queue->wait_rt_task_list = NULL;
    queue->wait_task_list = NULL;
    return 0;
}

void sys_queue_manager_queue_uninit(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue)
{
    sys_trace();
    if (queue->buffer != NULL)
    {
        sys_free(queue->buffer);
        queue->buffer = NULL;
    }
    queue->message_count = 0;
    queue->length = 0;
    queue->message_size = 0;
    queue->write_index = 0;
    queue->read_index = 0;
    queue->high_priority_task = NULL;
    queue->wait_rt_task_list = NULL;
    queue->wait_task_list = NULL;
}

int sys_queue_manager_send(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, void *message, sys_task_t **next_task)
{
    sys_trace();
    if (queue->message_count >= queue->length)
    {
        return -1;
    }
    sys_task_t *task = NULL;
    unsigned char *high_priority_task = (unsigned char *)queue->high_priority_task;
    if (high_priority_task != NULL)
    {
        sys_delete_node(&queue->wait_rt_task_list, (sys_tree_node_t *)high_priority_task);
        queue->high_priority_task = sys_get_left_most_node(queue->wait_rt_task_list);
        task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
    }
    if (NULL == task)
    {
        high_priority_task = (unsigned char *)queue->wait_task_list;
        if (high_priority_task != NULL)
        {
            sys_remove_from_list(&queue->wait_task_list, (sys_list_node_t *)high_priority_task);
            task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
        }
    }
    sys_memcpy(&queue->buffer[queue->write_index * queue->message_size], message, queue->message_size);
    queue->write_index++;
    if (queue->write_index >= queue->length)
    {
        queue->write_index = 0;
    }
    queue->message_count++;
    if (task != NULL)
    {
        task->arg = queue;
        if (SYS_TASK_STATE_BLOCKED == task->task_control_block.task_state)
        {
            task->task_control_block.task_state = SYS_TASK_STATE_SUSPENDED;
            int ret = sys_task_manager_resume(queue_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
        }
        else
        {
            int ret = sys_task_manager_wakeup(queue_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
        }
    }
    return 0;
}

int sys_queue_manager_send_to_front(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, void *message, sys_task_t **next_task)
{
    sys_trace();
    if (queue->message_count >= queue->length)
    {
        return -1;
    }
    sys_task_t *task = NULL;
    unsigned char *high_priority_task = (unsigned char *)queue->high_priority_task;
    if (high_priority_task != NULL)
    {
        sys_delete_node(&queue->wait_rt_task_list, (sys_tree_node_t *)high_priority_task);
        queue->high_priority_task = sys_get_left_most_node(queue->wait_rt_task_list);
        task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
    }
    if (NULL == task)
    {
        high_priority_task = (unsigned char *)queue->wait_task_list;
        if (high_priority_task != NULL)
        {
            sys_remove_from_list(&queue->wait_task_list, (sys_list_node_t *)high_priority_task);
            task = (sys_task_t *)(high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
        }
    }
    if (queue->read_index > 0)
    {
        queue->read_index--;
    }
    else
    {
        queue->read_index = queue->length - 1;
    }
    sys_memcpy(&queue->buffer[queue->read_index * queue->message_size], message, queue->message_size);
    queue->message_count++;
    if (task != NULL)
    {
        task->arg = queue;
        if (SYS_TASK_STATE_BLOCKED == task->task_control_block.task_state)
        {
            task->task_control_block.task_state = SYS_TASK_STATE_SUSPENDED;
            int ret = sys_task_manager_resume(queue_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
        }
        else
        {
            int ret = sys_task_manager_wakeup(queue_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
        }
    }
    return 0;
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

int sys_queue_manager_receive(sys_queue_manager_t *queue_manager, void *message, sys_task_t **next_task, sys_msg_queue_t *queue, uint64_t wait)
{
    sys_trace();
    *next_task = NULL;
    void *result = sys_queue_manager_queue_pop(queue_manager, queue);
    if (result != NULL)
    {
        sys_memcpy(message, result, queue->message_size);
    }
    else
    {
        sys_task_t *task = sys_task_manager_get_running_task(queue_manager->task_manager);
        if (wait > 0 && wait < SYS_MESSAGE_MAX_WAIT_TIME)
        {
            sys_task_manager_sleep(queue_manager->task_manager, next_task, wait * 1000 * 1000);
        }
        else if (SYS_MESSAGE_MAX_WAIT_TIME == wait)
        {
            int ret = sys_task_manager_supend(queue_manager->task_manager, next_task, task->tid);
            if (ret < 0)
            {
                return ret;
            }
            task->task_control_block.task_state = SYS_TASK_STATE_BLOCKED;
        }
        if (SYS_TASK_TYPE_RT == task->task_control_block.scheduler_id)
        {
            sys_insert_node(&queue->wait_rt_task_list, &task->exnode.tree_node, on_compare, NULL);
            if (NULL == queue->high_priority_task)
            {
                queue->high_priority_task = &task->exnode.tree_node;
            }
            else
            {
                sys_task_t *high_priority_task = (sys_task_t *)((unsigned char *)queue->high_priority_task - sizeof(sys_task_control_block_t) - sizeof(sys_list_node_t) - sizeof(task->real_task_control_block));
                if (task->real_task_control_block.rt_task_control_block.priority < high_priority_task->real_task_control_block.rt_task_control_block.priority)
                {
                    queue->high_priority_task = &task->exnode.tree_node;
                }
            }
        }
        else
        {
            sys_insert_to_back(&queue->wait_task_list, &task->exnode.list_node);
        }
    }
    return 0;
}

int sys_queue_manager_get_message_count(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue)
{
    sys_trace();
    return queue->message_count;
}

int sys_queue_manager_get_queue_length(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue)
{
    sys_trace();
    return queue->length;
}

void *sys_queue_manager_queue_pop(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue)
{
    sys_trace();
    void *ret = NULL;
    if (queue->message_count > 0)
    {
        ret = &queue->buffer[queue->read_index * queue->message_size];
        queue->read_index++;
        if (queue->read_index >= queue->length)
        {
            queue->read_index = 0;
        }
        queue->message_count--;
    }
    return ret;
}

int sys_queue_manager_remove_task(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, sys_task_t *task)
{
    sys_trace();
    if (SYS_TASK_TYPE_RT == task->task_control_block.scheduler_id)
    {
        sys_delete_node(&queue->wait_rt_task_list, &task->exnode.tree_node);
        queue->high_priority_task = sys_get_left_most_node(queue->wait_rt_task_list);
    }
    else
    {
        sys_remove_from_list(&queue->wait_task_list, &task->exnode.list_node);
    }
    return 0;
}

void sys_queue_manager_reset(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue)
{
    sys_trace();
    queue->message_count = 0;
    queue->write_index = 0;
    queue->read_index = 0;
}