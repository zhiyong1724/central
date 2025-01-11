#ifndef __SYS_QUEUE_MANAGER_H__
#define __SYS_QUEUE_MANAGER_H__
#include "sys_task_manager.h"
#include "sys_msg_queue.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_queue_manager_t
{
    sys_task_manager_t *task_manager;
} sys_queue_manager_t;
/*********************************************************************************************************************
* sys_queue_manager_t初始化
* queue_manager：sys_queue_manager_t对象
* task_manager：任务管理器
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_init(sys_queue_manager_t *queue_manager, sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* sys_queue_t初始化
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* message_size：消息大小
* queue_length：队列长度
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_queue_init(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, int queue_length, int message_size);
/*********************************************************************************************************************
* 释放sys_queue_t
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
*********************************************************************************************************************/
void sys_queue_manager_queue_uninit(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue);
/*********************************************************************************************************************
* 发送消息
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* message：消息
* next_task：不为NULL表示下一个任务
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_send(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, void *message, sys_task_t **next_task);
/*********************************************************************************************************************
* 发送消息到队列前面
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* message：消息
* next_task：不为NULL表示下一个任务
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_send_to_front(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, void *message, sys_task_t **next_task);
/*********************************************************************************************************************
* 接收消息
* queue_manager：sys_queue_manager_t对象
* message：消息
* next_task：不为NULL表示下一个任务
* queue：sys_queue_t对象
* wait：等待时间，0表示马上返回，SYS_MESSAGE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_receive(sys_queue_manager_t *queue_manager, void *message, sys_task_t **next_task, sys_msg_queue_t *queue, uint64_t wait);
/*********************************************************************************************************************
* 获取消息数量
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* return：消息数量
*********************************************************************************************************************/
int sys_queue_manager_get_message_count(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue);
/*********************************************************************************************************************
* 获取队列长度
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* return：队列长度
*********************************************************************************************************************/
int sys_queue_manager_get_queue_length(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue);
/*********************************************************************************************************************
* 弹出一条消息，如果没有消息则返回NULL
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* return：最前面的消息
*********************************************************************************************************************/
void *sys_queue_manager_queue_pop(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue);
/*********************************************************************************************************************
* 移除等待的任务
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
* task：移除的任务
* return：0：调用成功
*********************************************************************************************************************/
int sys_queue_manager_remove_task(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue, sys_task_t *task);
/*********************************************************************************************************************
* 清空消息队列
* queue_manager：sys_queue_manager_t对象
* queue：sys_queue_t对象
*********************************************************************************************************************/
void sys_queue_manager_reset(sys_queue_manager_t *queue_manager, sys_msg_queue_t *queue);
#ifdef __cplusplus
}
#endif
#endif