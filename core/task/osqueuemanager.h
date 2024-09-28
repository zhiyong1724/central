#ifndef __OSQUEUEMANAGER_H__
#define __OSQUEUEMANAGER_H__
#include "ostaskdef.h"
#include "oslist.h"
#include "ostaskmanager.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsQueueManager
{
    OsTaskManager *taskManager;
} OsQueueManager;
/*********************************************************************************************************************
* OsQueueManager初始化
* queueManager：OsQueueManager对象
* taskManager：任务管理器
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerInit(OsQueueManager *queueManager, OsTaskManager *taskManager);
/*********************************************************************************************************************
* OsQueue初始化
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* messageSize：消息大小
* queueLength：队列长度
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerQueueInit(OsQueueManager *queueManager, OsMsgQueue *queue, size_t queueLength, size_t messageSize);
/*********************************************************************************************************************
* 释放OsQueue
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerQueueUninit(OsQueueManager *queueManager, OsMsgQueue *queue);
/*********************************************************************************************************************
* 发送消息
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* message：消息
* nextTask：不为NULL表示下一个任务
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerSend(OsQueueManager *queueManager, OsMsgQueue *queue, void *message, OsTask **nextTask);
/*********************************************************************************************************************
* 发送消息到队列前面
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* message：消息
* nextTask：不为NULL表示下一个任务
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerSendToFront(OsQueueManager *queueManager, OsMsgQueue *queue, void *message, OsTask **nextTask);
/*********************************************************************************************************************
* 接收消息
* queueManager：OsQueueManager对象
* message：消息
* nextTask：不为NULL表示下一个任务
* queue：OsQueue对象
* wait：等待时间，0表示马上返回，OS_MESSAGE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerReceive(OsQueueManager *queueManager, void *message, OsTask **nextTask, OsMsgQueue *queue, uint64_t wait);
/*********************************************************************************************************************
* 获取消息数量
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：消息数量
*********************************************************************************************************************/
size_t osQueueManagerGetMessageCount(OsQueueManager *queueManager, OsMsgQueue *queue);
/*********************************************************************************************************************
* 获取队列长度
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：队列长度
*********************************************************************************************************************/
size_t osQueueManagerGetQueueLength(OsQueueManager *queueManager, OsMsgQueue *queue);
/*********************************************************************************************************************
* 弹出一条消息，如果没有消息则返回NULL
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：最前面的消息
*********************************************************************************************************************/
void *osQueueManagerQueuePop(OsQueueManager *queueManager, OsMsgQueue *queue);
/*********************************************************************************************************************
* 移除等待的任务
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* task：移除的任务
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerRemoveTask(OsQueueManager *queueManager, OsMsgQueue *queue, OsTask *task);
/*********************************************************************************************************************
* 清空消息队列
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerReset(OsQueueManager *queueManager, OsMsgQueue *queue);
#ifdef __cplusplus
}
#endif
#endif