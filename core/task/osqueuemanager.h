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
typedef struct OsMessage
{
    OsListNode node;
} OsMessage;

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
int osQueueManagerQueueInit(OsQueueManager *queueManager, OsQueue *queue, os_size_t queueLength, os_size_t messageSize);
/*********************************************************************************************************************
* 发送消息
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* message：消息
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerSend(OsQueueManager *queueManager, OsQueue *queue, OsMessage *message);
/*********************************************************************************************************************
* 发送消息到队列前面
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* message：消息
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerSendToFront(OsQueueManager *queueManager, OsQueue *queue, OsMessage *message);
/*********************************************************************************************************************
* 接收消息
* queueManager：OsQueueManager对象
* message：消息
* nextTask：不为NULL表示下一个任务
* queue：OsQueue对象
* wait：等待时间，0表示马上返回，OS_MESSAGE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerReceive(OsQueueManager *queueManager, OsMessage **message, OsTask **nextTask, OsQueue *queue, os_size_t wait);
/*********************************************************************************************************************
* 获取消息数量
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：消息数量
*********************************************************************************************************************/
os_size_t osQueueManagerGetMessageCount(OsQueueManager *queueManager, OsQueue *queue);
/*********************************************************************************************************************
* 获取队列长度
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：队列长度
*********************************************************************************************************************/
os_size_t osQueueManagerGetQueueLength(OsQueueManager *queueManager, OsQueue *queue);
/*********************************************************************************************************************
* 弹出一条消息，如果没有消息则返回NULL
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* return：最前面的消息
*********************************************************************************************************************/
OsMessage *osQueueManagerQueuePop(OsQueueManager *queueManager, OsQueue *queue);
/*********************************************************************************************************************
* 移除等待的任务
* queueManager：OsQueueManager对象
* queue：OsQueue对象
* task：移除的任务
* return：0：调用成功
*********************************************************************************************************************/
int osQueueManagerRemoveTask(OsQueueManager *queueManager, OsQueue *queue, OsTask *task);
#ifdef __cplusplus
}
#endif
#endif