#include "ostree.h"
#include <stdio.h>
#include "osbuddy.h"
#include "osmempool.h"
#include "osmemmanager.h"
#include "osrtscheduler.h"
#include "osdtscheduler.h"
#include "osmem.h"
#include "osvector.h"
#include "ostask.h"
#include "ostidmanager.h"
#include "oscentral.h"
#include "osqueue.h"
#include "ossemaphore.h"
#include "osmutex.h"
#include "shellio.h"
#include "osf.h"
#include "ff.h"
#include "fatfsadapter.h"
#include "lfsadapter.h"
#include "lfs.h"
#include "lvglio.h"
extern const struct lfs_config gLfsConfig;
extern lfs_t gLFS;
struct Test
{
    OsTreeNode node;
    int value;
};

int onCompare(void *key1, void *key2, void *arg)
{
    struct Test *test1 = (struct Test *)key1;
    struct Test *test2 = (struct Test *)key2;
    if (test1->value < test2->value)
    {
        return -1;
    }
    else if (test1->value > test2->value)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}

void searchTree(OsTreeNode *root, int *nodeNum)
{
    if (root != NULL)
    {
        if (root != &gLeafNode)
        {
            (*nodeNum)++;
        }
        if (root->leftTree != NULL)
        {
            searchTree(root->leftTree, nodeNum);
        }
        if (root->rightTree != NULL)
        {
            searchTree(root->rightTree, nodeNum);
        }
    }
}

void testTree()
{
    static OsTreeNode *handle = NULL;
    static struct Test nodes[4000];
    for (int i = 0; i < 4000; i++)
    {
        nodes[i].value = i;
    }
    //正插入
    for (int i = 0; i < 1000; i++)
    {
        osInsertNode(&handle, &nodes[i].node, onCompare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    int nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //倒插入
    for (int i = 1999; i >= 1000; i--)
    {
        osInsertNode(&handle, &nodes[i].node, onCompare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //正移除
    for (int i = 0; i < 1000; i++)
    {
        osDeleteNode(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //倒插入
    for (int i = 2999; i >= 2000; i--)
    {
        osInsertNode(&handle, &nodes[i].node, onCompare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //正插入
    for (int i = 3000; i < 4000; i++)
    {
        osInsertNode(&handle, &nodes[i].node, onCompare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //倒移除
    for (int i = 2999; i >= 2000; i--)
    {
        osDeleteNode(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //正插入
    for (int i = 0; i < 1000; i++)
    {
        osInsertNode(&handle, &nodes[i].node, onCompare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //倒移除
    for (int i = 3999; i >= 3000; i--)
    {
        osDeleteNode(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //正移除
    for (int i = 1000; i < 2000; i++)
    {
        osDeleteNode(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
    //倒移除
    for (int i = 999; i >= 0; i--)
    {
        osDeleteNode(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    searchTree(handle, &nodeNum);
    printf("nodeNum = %d\n", nodeNum);
}

void testBuddy()
{
    static char sBuff[0x40000];
    OsBuddy buddy;
    osBuddyInit(&buddy, sBuff, 0x40000);
    void *buff[64] = {NULL, };
    //正申请
    for (int i = 0; i < 63; i++)
    {
        buff[i] = osBuddyAllocPages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //正释放
    for (int i = 0; i < 63; i++)
    {
        int ret = osBuddyFreePages(&buddy, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //正申请
    for (int i = 0; i < 63; i++)
    {
        buff[i] = osBuddyAllocPages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //倒释放
    for (int i = 62; i >= 0; i--)
    {
        int ret = osBuddyFreePages(&buddy, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //倒申请
    for (int i = 62; i >= 0; i--)
    {
        buff[i] = osBuddyAllocPages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //正释放
    for (int i = 0; i < 63; i++)
    {
        int ret = osBuddyFreePages(&buddy, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }

    buff[0] = osBuddyAllocPages(&buddy, 30);
    printf("address = 0x%p\n", buff[0]);
    printf("free = %ld\n", osBuddyFreePageNum(&buddy));

    int ret = osBuddyFreePages(&buddy, buff[0]);
    printf("ret = %d\n", ret);
    printf("free = %ld\n", osBuddyFreePageNum(&buddy));

    //倒申请
    for (int i = 62; i >= 0; i--)
    {
        buff[i] = osBuddyAllocPages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
    //倒释放
    for (int i = 62; i >= 0; i--)
    {
        int ret = osBuddyFreePages(&buddy, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osBuddyFreePageNum(&buddy));
    }
}

void testMemPool()
{
    static char sBuff[1024];
    OsMemPool memPool;
    osMemPoolInit(&memPool, sBuff, 1024, 30);
    void *buff[32] = {NULL, };
    //正申请
    for (int i = 0; i < 31; i++)
    {
        buff[i] = osMemPoolAllocPage(&memPool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //正释放
    for (int i = 0; i < 31; i++)
    {
        int ret = osMemPoolFreePage(&memPool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //正申请
    for (int i = 0; i < 31; i++)
    {
        buff[i] = osMemPoolAllocPage(&memPool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //倒释放
    for (int i = 30; i >= 0; i--)
    {
        int ret = osMemPoolFreePage(&memPool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //倒申请
    for (int i = 30; i >= 0; i--)
    {
        buff[i] = osMemPoolAllocPage(&memPool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //正释放
    for (int i = 0; i < 31; i++)
    {
        int ret = osMemPoolFreePage(&memPool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //倒申请
    for (int i = 30; i >= 0; i--)
    {
        buff[i] = osMemPoolAllocPage(&memPool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
    //倒释放
    for (int i = 30; i >= 0; i--)
    {
        int ret = osMemPoolFreePage(&memPool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", osMemPoolFreePageNum(&memPool));
    }
}

void testMem()
{
    osInit();
    printf("total mem = %ld\n", osTotalMem());
    printf("total page = %ld\n", osTotalPage());

    printf("free mem = %ld\n", osFreeMem());
    printf("free page = %ld\n", osFreePage());
    static void *buff[1024 * 1024];
    for (int i = 1; i < 1024; i++)
    {
        buff[i] = osMalloc(i);
        printf("address = 0x%p\n", buff[i]);
        printf("free mem = %ld\n", osFreeMem());
        printf("free page = %ld\n", osFreePage());
    }
    for (int i = 1; i < 1024; i++)
    {
        int ret = osFree(buff[i]);
        printf("ret = %d\n", ret);
        printf("free mem = %ld\n", osFreeMem());
        printf("free page = %ld\n", osFreePage());
    }
    for (int i = 1; i < 21463; i++)
    {
        buff[i] = osMalloc(30);
        printf("i = %d\n", i);
        printf("address = 0x%p\n", buff[i]);
        printf("free mem = %ld\n", osFreeMem());
        printf("free page = %ld\n", osFreePage());
    }
    for (int i = 1; i < 21463; i++)
    {
        int ret = osFree(buff[i]);
        printf("ret = %d\n", ret);
        printf("free mem = %ld\n", osFreeMem());
        printf("free page = %ld\n", osFreePage());
    }
}

void testVector()
{
    OsVector vector;
    osVectorInit(&vector, sizeof(int));
    for (int i = 0; i < 1000; i++)
    {
        osVectorPushBack(&vector, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)osVectorAt(&vector, i);
        printf("%d, ", *value);
    }
    printf("\n");
    osVectorClear(&vector);

    for (int i = 0; i < 1000; i++)
    {
        osVectorPushFront(&vector, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)osVectorAt(&vector, i);
        printf("%d, ", *value);
    }
    printf("\n");
    osVectorClear(&vector);

    for (int i = 0; i < 1000; i++)
    {
        osVectorPushBack(&vector, &i);
        int *value = (int *)osVectorBack(&vector);
        printf("%d, ", *value);
    }
    printf("\n");
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)osVectorBack(&vector);
        printf("%d, ", *value);
        osVectorPopBack(&vector);
    }
    printf("\n");

    for (int i = 0; i < 1000; i++)
    {
        osVectorPushFront(&vector, &i);
        int *value = (int *)osVectorFront(&vector);
        printf("%d, ", *value);
    }
    printf("\n");
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)osVectorFront(&vector);
        printf("%d, ", *value);
        osVectorPopFront(&vector);
    }
    printf("\n");
    osVectorFree(&vector);
    printf("free mem = %ld\n", osFreeMem());
    printf("free page = %ld\n", osFreePage());
}

void testRtScheduler()
{
    OsRtTaskControlBlock *runningTask = NULL;
    OsRtScheduler rtScheduler;
    osRtSchedulerInit(&rtScheduler, 1000 * 1000);

    OsRtTaskControlBlock taskA;
    osRtTaskControlBlockInit(&rtScheduler, &taskA, 50);

    OsRtTaskControlBlock taskB;
    osRtTaskControlBlockInit(&rtScheduler, &taskB, 50);

    OsRtTaskControlBlock taskC;
    osRtTaskControlBlockInit(&rtScheduler, &taskC, 20);

    OsRtTaskControlBlock taskD;
    osRtTaskControlBlockInit(&rtScheduler, &taskD, 20);

    osRtSchedulerAddTask(&rtScheduler, &taskA);
    osRtSchedulerAddTask(&rtScheduler, &taskB);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    
    osRtSchedulerAddTask(&rtScheduler, &taskC);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    osRtSchedulerAddTask(&rtScheduler, &taskD);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    osRtSchedulerModifyPriority(&rtScheduler, &taskA, 0);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    osRtSchedulerRemoveTask(&rtScheduler, &taskC);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    osRtSchedulerRemoveTask(&rtScheduler, &taskD);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
    osRtSchedulerModifyPriority(&rtScheduler, &taskB, 0);
    for (size_t i = 0; i < 100; i++)
    {
        runningTask = osRtSchedulerTick(&rtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
    }
}

void testDtScheduler()
{
    OsDtTaskControlBlock *runningTask = NULL;
    OsDtScheduler dtScheduler;
    osDtSchedulerInit(&dtScheduler, 1000 * 1000);

    OsDtTaskControlBlock taskA;
    osDtTaskControlBlockInit(&dtScheduler, &taskA, 0);
    osDtSchedulerAddTask(&dtScheduler, &taskA);

    OsDtTaskControlBlock taskB;
    osDtTaskControlBlockInit(&dtScheduler, &taskB, 5);
    osDtSchedulerAddTask(&dtScheduler, &taskB);

    OsDtTaskControlBlock taskC;
    osDtTaskControlBlockInit(&dtScheduler, &taskC, 10);
    osDtSchedulerAddTask(&dtScheduler, &taskC);

    OsDtTaskControlBlock taskD;
    osDtTaskControlBlockInit(&dtScheduler, &taskD, 15);
    osDtSchedulerAddTask(&dtScheduler, &taskD);

    OsDtTaskControlBlock taskE;
    
    osDtSchedulerRemoveTask(&dtScheduler, &taskC);

    for (size_t i = 0; i < 1000; i++)
    {
        runningTask = osDtSchedulerTick(&dtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
        else if (runningTask == &taskE)
        {
            printf("This is task E\n");
        }
    }
    
    osDtTaskControlBlockInit(&dtScheduler, &taskE, 0);
    taskE.vRunTime += 100000;
    osDtSchedulerAddTask(&dtScheduler, &taskE);
    for (size_t i = 0; i < 1000; i++)
    {
        runningTask = osDtSchedulerTick(&dtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
        else if (runningTask == &taskE)
        {
            printf("This is task E\n");
        }
    }
    osDtSchedulerModifyPriority(&dtScheduler, &taskD, 0);
    for (size_t i = 0; i < 1000; i++)
    {
        runningTask = osDtSchedulerTick(&dtScheduler);
        if (runningTask == &taskA)
        {
            printf("This is task A\n");
        }
        else if (runningTask == &taskB)
        {
            printf("This is task B\n");
        }
        else if (runningTask == &taskC)
        {
            printf("This is task C\n");
        }
        else if (runningTask == &taskD)
        {
            printf("This is task D\n");
        }
        else if (runningTask == &taskE)
        {
            printf("This is task E\n");
        }
    }
}

void testTid()
{
    osInit();
    OsTidManager tidManager;
    osTidManagerInit(&tidManager);
    for (size_t i = 0; i < 100000; i++)
    {
        os_size_t tid = osTidAlloc(&tidManager);
        printf("tid = %ld\n", tid);
        if (tid != i)
        {
            printf("error\n");
        }
    }
    for (size_t i = 0; i < 100; i++)
    {
        osTidFree(&tidManager, i);
    }
    for (size_t i = 0; i < 110; i++)
    {
        os_size_t tid = osTidAlloc(&tidManager);
        printf("tid = %ld\n", tid);
    }
    for (size_t i = 0; i < 100000; i++)
    {
        osTidFree(&tidManager, i);
    }
    for (size_t i = 0; i < 100000; i++)
    {
        os_size_t tid = osTidAlloc(&tidManager);
        printf("tid = %ld\n", tid);
        if (tid != i)
        {
            printf("error\n");
        }
    }
}
void *testMutexTaskD(void *arg);
static OsMutex sMutex;
void *testMutexTaskA(void *arg)
{
    osMutexLock(&sMutex);
    printf("This is task A\n");
    osMutexUnlock(&sMutex);
    os_tid_t tid;
    osTaskCreate(&tid, testMutexTaskD, NULL, "testMutexTaskD", 20, 512);
    return NULL;
}

void *testMutexTaskB(void *arg)
{
    printf("This is task B\n");
    osMutexUnlock(&sMutex);
    osMutexUnlock(&sMutex);
    osMutexUnlock(&sMutex);
    osMutexLock(&sMutex);
    os_tid_t tid;
    osTaskCreateRT(&tid, testMutexTaskA, NULL, "testMutexTaskA", 20, 512);
    osTaskSleep(5000);
    osMutexUnlock(&sMutex);
    return NULL;
}

OsRecursiveMutex sRecursiveMutex;

void *testMutexTaskC(void *arg)
{
    osRecursiveMutexLock(&sRecursiveMutex);
    printf("This is task C\n");
    osRecursiveMutexUnlock(&sRecursiveMutex);
    return NULL;
}

void *testMutexTaskD(void *arg)
{
    printf("This is task D\n");
    osRecursiveMutexUnlock(&sRecursiveMutex);
    osRecursiveMutexUnlock(&sRecursiveMutex);
    osRecursiveMutexUnlock(&sRecursiveMutex);
    osRecursiveMutexLock(&sRecursiveMutex);
    osRecursiveMutexLock(&sRecursiveMutex);
    osRecursiveMutexLock(&sRecursiveMutex);
    os_tid_t tid;
    osTaskCreateRT(&tid, testMutexTaskC, NULL, "testMutexTaskC", 20, 512);
    osTaskSleep(2000);
    printf("osRecursiveMutexUnlock\n");
    osRecursiveMutexUnlock(&sRecursiveMutex);
    osTaskSleep(2000);
    printf("osRecursiveMutexUnlock\n");
    osRecursiveMutexUnlock(&sRecursiveMutex);
    osTaskSleep(2000);
    printf("osRecursiveMutexUnlock\n");
    osRecursiveMutexUnlock(&sRecursiveMutex);
    return NULL;
}

void testMutex()
{
    osInit();
    osMutexCreate(&sMutex);
    osRecursiveMutexCreate(&sRecursiveMutex);
    os_tid_t tid;
    osTaskCreate(&tid, testMutexTaskB, NULL, "testMutexTaskB", 20, 512);
    osTaskStart();
}

static OsSemaphore sSemaphore;
void *testSemaphoreTaskA(void *arg)
{
    printf("This is task A\n");
    osTaskSleep(5000);
    for (int i = 0; i < 10; i++)
    {
        osSemaphorePost(&sSemaphore);
    }
    osTaskSleep(1000);
    osSemaphoreDestory(&sSemaphore);
    return NULL;
}

void *testSemaphoreTaskB(void *arg)
{
    printf("This is task B\n");
    int ret = osSemaphoreWait(&sSemaphore, OS_MESSAGE_MAX_WAIT_TIME);
    printf("ret = %d\n", ret);
    return NULL;
}

void *testSemaphoreTaskC(void *arg)
{
    printf("This is task C\n");
    int ret = osSemaphoreWait(&sSemaphore, OS_MESSAGE_MAX_WAIT_TIME);
    printf("ret = %d\n", ret);
    return NULL;
}

void *testSemaphoreTaskD(void *arg)
{
    printf("This is task D\n");
    int ret = osSemaphoreWait(&sSemaphore, 2000);
    printf("ret = %d\n", ret);
    return NULL;
}

void testSemaphore()
{
    osInit();
    osSemaphoreCreate(&sSemaphore, 0, 0);
    for (int i = 0; i < 10; i++)
    {
        osSemaphorePost(&sSemaphore);
    }
    for (int i = 0; i < 10; i++)
    {
        int ret = osSemaphoreWait(&sSemaphore, 0);
        printf("ret = %d\n", ret);
    }
    os_tid_t tid;
    osTaskCreate(&tid, testSemaphoreTaskA, NULL, "testSemaphoreTaskA", 20, 512);
    osTaskCreateRT(&tid, testSemaphoreTaskB, NULL, "testSemaphoreTaskB", 20, 512);
    osTaskCreateRT(&tid, testSemaphoreTaskC, NULL, "testSemaphoreTaskC", 0, 512);
    osTaskCreate(&tid, testSemaphoreTaskD, NULL, "testSemaphoreTaskC", 20, 512);
    osTaskStart();
}

static OsQueue sQueue;
void *testQueueTaskA(void *arg)
{
    printf("This is task A\n");
    osTaskSleep(5000);
    for (int i = 0; i < 10; i++)
    {
        osQueueSend(&sQueue, &i);
    }
    osTaskSleep(1000);
    osQueueDestory(&sQueue);
    return NULL;
}

void *testQueueTaskB(void *arg)
{
    printf("This is task B\n");
    int message = -1;
    osQueueReceive(&message, &sQueue, OS_MESSAGE_MAX_WAIT_TIME);
    printf("message = %d\n", message);
    return NULL;
}

void *testQueueTaskC(void *arg)
{
    printf("This is task C\n");
    int message = -1;
    osQueueReceive(&message, &sQueue, OS_MESSAGE_MAX_WAIT_TIME);
    printf("message = %d\n", message);
    return NULL;
}

void *testQueueTaskD(void *arg)
{
    printf("This is task D\n");
    int message = -1;
    osQueueReceive(&message, &sQueue, 2000);
    printf("message = %d\n", message);
    return NULL;
}

void testQueue()
{
    osInit();
    osQueueCreate(&sQueue, 0, sizeof(int));
    for (int i = 0; i < 10; i++)
    {
        osQueueSend(&sQueue, &i);
    }
    for (int i = 0; i < 10; i++)
    {
        int message;
        osQueueReceive(&message, &sQueue, 0);
        printf("message = %d\n", message);
    }
    os_tid_t tid;
    osTaskCreate(&tid, testQueueTaskA, NULL, "testQueueTaskA", 20, 512);
    osTaskCreateRT(&tid, testQueueTaskB, NULL, "testQueueTaskB", 20, 512);
    osTaskCreateRT(&tid, testQueueTaskC, NULL, "testQueueTaskC", 0, 512);
    osTaskCreate(&tid, testQueueTaskD, NULL, "testQueueTaskD", 20, 512);
    osTaskStart();
}

void *taskA(void *arg)
{
    for (;;)
    {
        printf("This is task A\n");
        printf("时钟周期：%lld\n", osTaskGetClockPeriod());
        printf("系统滴答：%lld\n", osTaskGetTickCount());
        printf("任务个数：%ld\n", osTaskGetTaskCount());

        os_tid_t tid = osTaskGetTid();
        printf("任务tid：%ld\n", tid);

        char name[OS_TASK_MAX_NAME_LEN];
        int ret = osTaskGetTaskName(name, OS_TASK_MAX_NAME_LEN, tid);
        printf("任务名：%s\n", name);

        OsTaskType type;
        ret = osTaskGetTaskType(&type, tid);
        printf("任务类型：%d\n", type);

        OsTaskState state;
        ret = osTaskGetTaskState(&state, tid);
        printf("任务状态：%d\n", state);

        os_size_t priority;
        ret = osTaskGetTaskPriority(&priority, tid);
        printf("任务优先级：%ld\n", priority);

        os_size_t stackSize;
        ret = osTaskGetTaskStackSize(&stackSize, tid);
        printf("任务堆栈大小：%ld\n", stackSize);

        (void)ret;
        osTaskSleep(1000);
    }
    return NULL;
}

void *taskB(void *arg)
{
    for (;;)
    {
        printf("This is task B\n");
        osTaskSupend(osTaskGetTid());
    }
    return NULL;
}

void *taskC(void *arg)
{
    os_tid_t tid;
    osTaskCreate(&tid, taskB, NULL, "task b", 20, 512);
    for (;;)
    {
        printf("This is task C\n");
        osTaskSleep(500);
        osTaskResume(tid);
    }
    return NULL;
}

void *taskD(void *arg)
{
    printf("This is task D\n");
    return NULL;
}

void *taskE(void *arg)
{
    printf("This is task E\n");
    osTaskSleep(15000);
    return NULL;
}

void *taskF(void *arg)
{
    printf("This is task F\n");
    osTaskSleep(10000);
    return taskF;
}

void *taskG(void *arg)
{
    printf("This is task G\n");
    os_tid_t tid;
    osTaskCreateRT(&tid, taskD, NULL, "task d", 20, 512);
    osTaskCreateRT(&tid, taskE, NULL, "task e", 20, 512);
    osTaskCreateRT(&tid, taskF, NULL, "task f", 20, 512);
    osTaskSleep(5000);
    void *retval;
    osTaskJoin(&retval, tid);
    return NULL;
}

void testFS()
{
}

int main()
{   
    // double ia;
    // double ib;
    // double ic;
    // double input = 25000000;
    // double output = 6144000;
    // double offset = output;
    // for (double i = 0; i <= 63; i++)
    // {
    //     for (double j = 0; j <= 512; j++)
    //     {
    //         for (double k = 0; k < 128; k++)
    //         {
    //             double real = input / i * j / k - output;
    //             if (real < 0)
    //             {
    //                 real = real * -1.0;
    //             }
    //             if (real < offset)
    //             {
    //                 offset = real;
    //                 ia = i;
    //                 ib = j;
    //                 ic = k;
    //             }
    //         }
    //     }
    // }
    // printf("offset = %lf, ia = %lf, ib = %lf, ic = %lf\n", offset, ia, ib, ic);
    // testTree();
    // testBuddy();
    // testMemPool();
    //testMem();
    // testVector();
    // testDtScheduler();
    // testRtScheduler();
    // testTid();
    //testSemaphore();
    //testMutex();
    //testQueue();
    osInit();
    //lvglIOInit();
    // os_tid_t tid;
    // osTaskCreate(&tid, taskA, NULL, "task a", 20, 512);
    // osTaskCreate(&tid, taskC, NULL, "task c", 20, 512);
    // osTaskCreateRT(&tid, taskG, NULL, "task g", 20, 512);
    registerLFS();
    registerFatfs(); 
    lfs_format(&gLFS, &gLfsConfig);
    f_mkfs("0:", NULL, NULL, FF_MAX_SS);
    osFMount("/", "RAM");
    shellIOInit();
    osTaskStart();
    return 0;
}