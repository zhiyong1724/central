#ifndef __OSTASKDEF_H__
#define __OSTASKDEF_H__
#include "osdefine.h"
#include "oslist.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef os_size_t os_tid_t;
typedef void *(*TaskFunction)(void *arg);
typedef enum OsTaskState
{
    OS_TASK_STATE_READY,
    OS_TASK_STATE_SLEEP,
    OS_TASK_STATE_SUSPENDED,
    OS_TASK_STATE_DELETED,
    OS_TASK_STATE_RUNNING,
    OS_TASK_STATE_BLOCKED,
} OsTaskState;

typedef enum OsTaskType
{
    OS_TASK_TYPE_RT,
    OS_TASK_TYPE_DT,
    OS_TASK_TYPE_IDLE,
} OsTaskType;
typedef void *(*TaskFunction)(void *arg);

typedef struct OsSemaphore
{
    os_size_t count;
    os_size_t maxCount;
    OsTreeNode *highPriorityTask;
    OsTreeNode *waitRtTaskList;
    OsListNode *waitTaskList;
} OsSemaphore;

typedef struct OsMutex
{
    OsSemaphore semaphore;
} OsMutex;

typedef struct OsRecursiveMutex
{
    OsSemaphore semaphore;
    os_size_t recursiveCount;
    void *owner;
} OsRecursiveMutex;

typedef struct OsMsgQueue
{
    OsListNode *messages;
    os_size_t messageSize;
    os_size_t messageCount;
    os_size_t length;
    OsTreeNode *highPriorityTask;
    OsTreeNode *waitRtTaskList;
    OsListNode *waitTaskList;
} OsMsgQueue;
#ifdef __cplusplus
}
#endif
#endif