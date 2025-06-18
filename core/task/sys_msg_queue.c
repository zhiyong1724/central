#include "sys_msg_queue.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
int sys_msg_queue_create(sys_msg_queue_t *queue, int queue_length, int message_size)
{
    sys_trace();
    queue->message_size = message_size;
    queue->message_count = 0;
    queue->length = queue_length;
    queue->buffer = (uint8_t *)sys_malloc(queue_length * message_size);
    if (NULL == queue->buffer)
    {
        return SYS_ERROR_NOMEM;
    }
    queue->write_index = 0;
    queue->read_index = 0;
    sys_semaphore_init(&queue->semaphore, 0, queue_length);
    sys_spin_lock_init(&queue->lock);
    return 0;
}

int sys_msg_queue_destory(sys_msg_queue_t *queue)
{
    sys_trace();
    int ret = -1;
    int state = sys_spin_lock_lock_and_irq_save(&queue->lock);
    if (NULL == queue->semaphore.wait_fifo_task_list && NULL == queue->semaphore.wait_rt_task_list && NULL == queue->semaphore.wait_task_list)
    {
        sys_free(queue->buffer);
        ret = 0;
    }
    sys_spin_lock_unlock_and_irq_restore(&queue->lock, state);
    return ret;
}

int sys_msg_queue_send(sys_msg_queue_t *queue, void *message)
{
    sys_trace();
    int ret = -1;
    int state = sys_spin_lock_lock_and_irq_save(&queue->lock);
    if (queue->message_count < queue->length)
    {
        sys_memcpy(&queue->buffer[queue->write_index * queue->message_size], message, queue->message_size);
        queue->write_index++;
        if (queue->write_index >= queue->length)
        {
            queue->write_index = 0;
        }
        queue->message_count++;
        sys_semaphore_post(&queue->semaphore);
        ret = 0;
    }
    sys_spin_lock_unlock_and_irq_restore(&queue->lock, state);
    return ret;
}

int sys_msg_queue_receive(sys_msg_queue_t *queue, void *message, uint64_t wait)
{
    sys_trace();
    int ret = sys_semaphore_wait(&queue->semaphore, wait);
    int state = sys_spin_lock_lock_and_irq_save(&queue->lock);
    if (0 == ret)
    {
        sys_memcpy(message, &queue->buffer[queue->read_index * queue->message_size], queue->message_size);
        queue->read_index++;
        if (queue->read_index >= queue->length)
        {
            queue->read_index = 0;
        }
        queue->message_count--;
    }
    sys_spin_lock_unlock_and_irq_restore(&queue->lock, state);
    return ret;
}

int sys_msg_queue_get_message_count(sys_msg_queue_t *queue)
{
    sys_trace();
    return queue->message_count;
}

int sys_msg_queue_get_queue_length(sys_msg_queue_t *queue)
{
    sys_trace();
    return queue->length;
}