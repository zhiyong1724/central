#ifndef __OSTASKDEF_H__
#define __OSTASKDEF_H__
#include "osdefine.h"
#include "oslist.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef size_t os_tid_t;
typedef void *(*TaskFunction)(void *arg);
typedef enum OsTaskState
{
    OS_TASK_STATE_READY,
    OS_TASK_STATE_SLEEP,
    OS_TASK_STATE_SUSPENDED,
    OS_TASK_STATE_DELETED,
    OS_TASK_STATE_BLOCKED,
} OsTaskState;

typedef enum OsTaskType
{
    OS_TASK_TYPE_RT,
    OS_TASK_TYPE_DT,
    OS_TASK_TYPE_IDLE,
} OsTaskType;
typedef void *(*TaskFunction)(void *arg);

typedef size_t os_task_ptr;
typedef struct OsTaskInfo
{
    os_tid_t tid;
    os_tid_t ptid;
    char name[OS_TASK_MAX_NAME_LEN];
    stack_size_t *stack;
    size_t stackSize;
    OsTaskState taskState;
    OsTaskType taskType;
    size_t priority;
} OsTaskInfo;

typedef struct OsSemaphore
{
    size_t count;
    size_t maxCount;
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
    size_t recursiveCount;
    void *owner;
} OsRecursiveMutex;

typedef struct OsMsgQueue
{
    size_t messageSize;
    size_t messageCount;
    size_t length;
    unsigned char *buffer;
    size_t writeIndex;
    size_t readIndex;
    OsTreeNode *highPriorityTask;
    OsTreeNode *waitRtTaskList;
    OsListNode *waitTaskList;
} OsMsgQueue;
#ifdef __cplusplus
}
#endif
#endif