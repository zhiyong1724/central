#include "sys_tree.h"
#include <stdio.h>
#include "sys_buddy.h"
#include "sys_mem_pool.h"
#include "sys_mem_manager.h"
#include "sys_rt_scheduler.h"
#include "sys_dt_scheduler.h"
#include "sys_mem.h"
#include "sys_vector.h"
#include "sys_task.h"
#include "sys_id_manager.h"
#include "central.h"
#include "sys_msg_queue.h"
#include "sys_semaphore.h"
#include "sys_lock.h"
#include "shellio.h"
#include "sys_vfs.h"
#include "lfs_adapter.h"
#include "lfs.h"
#include "lvglio.h"
#include "ram_block.h"
#include "ram_block1.h"
#include "fatfs_adapter.h"
extern const struct lfs_config g_lfs_config;
extern lfs_t g_lfs;
struct Test
{
    sys_tree_node_t node;
    int value;
};

int on_compare(void *key1, void *key2, void *arg)
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

static void searchTree(sys_tree_node_t *root, int *nodeNum, int *minDeep, int *maxDeep, int *deep)
{
    if (root != NULL)
    {
        (*deep)++;
        (*nodeNum)++;
        if (root->left != &g_leaf_node)
        {
            searchTree(root->left, nodeNum, minDeep, maxDeep, deep);
        }
        else
        {
            if (*minDeep > *deep)
            {
                *minDeep = *deep;
            }
            if (*maxDeep < *deep)
            {
                *maxDeep = *deep;
            }
        }
        if (root->right != &g_leaf_node)
        {
            searchTree(root->right, nodeNum, minDeep, maxDeep, deep);
        }
        else
        {
            if (*minDeep > *deep)
            {
                *minDeep = *deep;
            }
            if (*maxDeep < *deep)
            {
                *maxDeep = *deep;
            }
        }
        (*deep)--;
    }
}

void testTree()
{
    static sys_tree_node_t *handle = NULL;
    static struct Test nodes[30000];
    for (int i = 0; i < 30000; i++)
    {
        nodes[i].value = i;
    }
    //正插入
    for (int i = 0; i < 10000; i++)
    {
        sys_insert_node(&handle, &nodes[i].node, on_compare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    int nodeNum = 0;
    int minDeep = 10000; 
    int maxDeep = 0;
    int deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
    //倒插入
    for (int i = 19999; i >= 10000; i--)
    {
        sys_insert_node(&handle, &nodes[i].node, on_compare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    minDeep = 10000; 
    maxDeep = 0;
    deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
    //正插入
    for (int i = 20000; i < 30000; i++)
    {
        sys_insert_node(&handle, &nodes[i].node, on_compare, NULL);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    minDeep = 100000; 
    maxDeep = 0;
    deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
    //倒删除
    for (int i = 19999; i >= 10000; i--)
    {
        sys_delete_node(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    minDeep = 10000; 
    maxDeep = 0;
    deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
    //正删除
    for (int i = 20000; i < 30000; i++)
    {
        sys_delete_node(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    minDeep = 10000; 
    maxDeep = 0;
    deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
    //正删除
    for (int i = 0; i < 10000; i++)
    {
        sys_delete_node(&handle, &nodes[i].node);
    }
    printf("handle = %ld\n", (unsigned long)handle);
    nodeNum = 0;
    minDeep = 10000; 
    maxDeep = 0;
    deep = 0;
    searchTree(handle, &nodeNum, &minDeep, &maxDeep, &deep);
    printf("nodeNum = %d, minDeep = %d, maxDeep = %d\n", nodeNum, minDeep, maxDeep);
}

void testBuddy()
{
    static char sBuff[0x40000];
    sys_buddy_t buddy;
    sys_buddy_init(&buddy, sBuff, 0x40000);
    void *buff[64] = {NULL, };
    //正申请
    for (int i = 0; i < 63; i++)
    {
        buff[i] = sys_buddy_alloc_pages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //正释放
    for (int i = 0; i < 63; i++)
    {
        sys_buddy_free_pages(&buddy, buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //正申请
    for (int i = 0; i < 63; i++)
    {
        buff[i] = sys_buddy_alloc_pages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //倒释放
    for (int i = 62; i >= 0; i--)
    {
        sys_buddy_free_pages(&buddy, buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //倒申请
    for (int i = 62; i >= 0; i--)
    {
        buff[i] = sys_buddy_alloc_pages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //正释放
    for (int i = 0; i < 63; i++)
    {
        sys_buddy_free_pages(&buddy, buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }

    buff[0] = sys_buddy_alloc_pages(&buddy, 30);
    printf("address = 0x%p\n", buff[0]);
    printf("free = %ld\n", sys_buddy_free_page_num(&buddy));

    sys_buddy_free_pages(&buddy, buff[0]);
    printf("free = %ld\n", sys_buddy_free_page_num(&buddy));

    //倒申请
    for (int i = 62; i >= 0; i--)
    {
        buff[i] = sys_buddy_alloc_pages(&buddy, 1);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
    //倒释放
    for (int i = 62; i >= 0; i--)
    {
        sys_buddy_free_pages(&buddy, buff[i]);
        printf("free = %ld\n", sys_buddy_free_page_num(&buddy));
    }
}

void testMemPool()
{
    static char sBuff[1024];
    sys_mem_pool_t mem_pool;
    sys_mem_pool_init(&mem_pool, sBuff, 1024, 30);
    void *buff[32] = {NULL, };
    //正申请
    for (int i = 0; i < 31; i++)
    {
        buff[i] = sys_mem_pool_alloc_page(&mem_pool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //正释放
    for (int i = 0; i < 31; i++)
    {
        int ret = sys_mem_pool_free_page(&mem_pool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //正申请
    for (int i = 0; i < 31; i++)
    {
        buff[i] = sys_mem_pool_alloc_page(&mem_pool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //倒释放
    for (int i = 30; i >= 0; i--)
    {
        int ret = sys_mem_pool_free_page(&mem_pool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //倒申请
    for (int i = 30; i >= 0; i--)
    {
        buff[i] = sys_mem_pool_alloc_page(&mem_pool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //正释放
    for (int i = 0; i < 31; i++)
    {
        int ret = sys_mem_pool_free_page(&mem_pool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //倒申请
    for (int i = 30; i >= 0; i--)
    {
        buff[i] = sys_mem_pool_alloc_page(&mem_pool);
        printf("address = 0x%p\n", buff[i]);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
    //倒释放
    for (int i = 30; i >= 0; i--)
    {
        int ret = sys_mem_pool_free_page(&mem_pool, buff[i]);
        printf("ret = %d\n", ret);
        printf("free = %ld\n", sys_mem_pool_free_page_num(&mem_pool));
    }
}

void testMem()
{
    sys_init();
    printf("total mem = %ld\n", sys_total_mem());
    printf("total page = %ld\n", sys_total_page());

    printf("free mem = %ld\n", sys_free_mem());
    printf("free page = %ld\n", sys_free_page());
    static void *buff[1024 * 1024];
    for (int i = 1; i < 1024; i++)
    {
        buff[i] = sys_malloc(i);
        memset(buff[i], 1, i);
        printf("address = 0x%p\n", buff[i]);
        printf("free mem = %ld\n", sys_free_mem());
        printf("free page = %ld\n", sys_free_page());
    }
    for (int i = 1; i < 1024; i++)
    {
        sys_free(buff[i]);
        printf("free mem = %ld\n", sys_free_mem());
        printf("free page = %ld\n", sys_free_page());
    }
    for (int i = 1; i < 21463; i++)
    {
        buff[i] = sys_malloc(30);
        memset(buff[i], 1, 30);
        printf("i = %d\n", i);
        printf("address = 0x%p\n", buff[i]);
        printf("free mem = %ld\n", sys_free_mem());
        printf("free page = %ld\n", sys_free_page());
    }
    for (int i = 1; i < 21463; i++)
    {
        sys_free(buff[i]);
        printf("free mem = %ld\n", sys_free_mem());
        printf("free page = %ld\n", sys_free_page());
    }
}

void testVector()
{
    sys_vector_t vector;
    sys_vector_init(&vector, sizeof(int));
    for (int i = 0; i < 1000; i++)
    {
        sys_vector_push_back(&vector, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)sys_vector_at(&vector, i);
        printf("%d, ", *value);
    }
    printf("\n");
    sys_vector_clear(&vector);

    for (int i = 0; i < 1000; i++)
    {
        sys_vector_push_front(&vector, &i);
    }
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)sys_vector_at(&vector, i);
        printf("%d, ", *value);
    }
    printf("\n");
    sys_vector_clear(&vector);

    for (int i = 0; i < 1000; i++)
    {
        sys_vector_push_back(&vector, &i);
        int *value = (int *)sys_vector_back(&vector);
        printf("%d, ", *value);
    }
    printf("\n");
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)sys_vector_back(&vector);
        printf("%d, ", *value);
        sys_vector_pop_back(&vector);
    }
    printf("\n");

    for (int i = 0; i < 1000; i++)
    {
        sys_vector_push_front(&vector, &i);
        int *value = (int *)sys_vector_front(&vector);
        printf("%d, ", *value);
    }
    printf("\n");
    for (int i = 0; i < 1000; i++)
    {
        int *value = (int *)sys_vector_front(&vector);
        printf("%d, ", *value);
        sys_vector_pop_front(&vector);
    }
    printf("\n");
    sys_vector_free(&vector);
    printf("free mem = %ld\n", sys_free_mem());
    printf("free page = %ld\n", sys_free_page());
}

void testRtScheduler()
{
    sys_rt_task_control_block_t *running_task = NULL;
    sys_rt_scheduler_t rt_scheduler;
    sys_rt_scheduler_init(&rt_scheduler);

    sys_rt_task_control_block_t taskA;
    sys_rt_task_control_block_init(&rt_scheduler, &taskA, 50);

    sys_rt_task_control_block_t taskB;
    sys_rt_task_control_block_init(&rt_scheduler, &taskB, 50);

    sys_rt_task_control_block_t taskC;
    sys_rt_task_control_block_init(&rt_scheduler, &taskC, 20);

    sys_rt_task_control_block_t taskD;
    sys_rt_task_control_block_init(&rt_scheduler, &taskD, 20);

    sys_rt_scheduler_add_task(&rt_scheduler, &taskA);
    sys_rt_scheduler_add_task(&rt_scheduler, &taskB);
    uint64_t ns = 100;
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    
    sys_rt_scheduler_add_task(&rt_scheduler, &taskC);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    sys_rt_scheduler_add_task(&rt_scheduler, &taskD);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    sys_rt_scheduler_modify_priority(&rt_scheduler, &taskA, 0);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    sys_rt_scheduler_remove_task(&rt_scheduler, &taskC);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    sys_rt_scheduler_remove_task(&rt_scheduler, &taskD);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
    sys_rt_scheduler_modify_priority(&rt_scheduler, &taskB, 0);
    for (size_t i = 0; i < 100; i++)
    {
        running_task = sys_rt_scheduler_tick(&rt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
    }
}

void testDtScheduler()
{
    sys_dt_task_control_block_t *running_task = NULL;
    sys_dt_scheduler_t dt_scheduler;
    sys_dt_scheduler_init(&dt_scheduler);

    sys_dt_task_control_block_t taskA;
    sys_dt_task_control_block_init(&dt_scheduler, &taskA, 0);
    sys_dt_scheduler_add_task(&dt_scheduler, &taskA);

    sys_dt_task_control_block_t taskB;
    sys_dt_task_control_block_init(&dt_scheduler, &taskB, 5);
    sys_dt_scheduler_add_task(&dt_scheduler, &taskB);

    sys_dt_task_control_block_t taskC;
    sys_dt_task_control_block_init(&dt_scheduler, &taskC, 10);
    sys_dt_scheduler_add_task(&dt_scheduler, &taskC);

    sys_dt_task_control_block_t taskD;
    sys_dt_task_control_block_init(&dt_scheduler, &taskD, 15);
    sys_dt_scheduler_add_task(&dt_scheduler, &taskD);

    sys_dt_task_control_block_t taskE;
    
    sys_dt_scheduler_remove_task(&dt_scheduler, &taskC);
    uint64_t ns = 100;
    for (size_t i = 0; i < 1000; i++)
    {
        running_task = sys_dt_scheduler_tick(&dt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
        else if (running_task == &taskE)
        {
            printf("This is task E\n");
        }
    }
    
    sys_dt_task_control_block_init(&dt_scheduler, &taskE, 0);
    taskE.vruntime -= 1;
    sys_dt_scheduler_add_task(&dt_scheduler, &taskE);
    for (size_t i = 0; i < 1000; i++)
    {
        running_task = sys_dt_scheduler_tick(&dt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
        else if (running_task == &taskE)
        {
            printf("This is task E\n");
        }
    }
    sys_dt_scheduler_modify_priority(&dt_scheduler, &taskD, 0);
    for (size_t i = 0; i < 1000; i++)
    {
        running_task = sys_dt_scheduler_tick(&dt_scheduler, &ns);
        if (running_task == &taskA)
        {
            printf("This is task A\n");
        }
        else if (running_task == &taskB)
        {
            printf("This is task B\n");
        }
        else if (running_task == &taskC)
        {
            printf("This is task C\n");
        }
        else if (running_task == &taskD)
        {
            printf("This is task D\n");
        }
        else if (running_task == &taskE)
        {
            printf("This is task E\n");
        }
    }
}

void testTid()
{
    sys_init();
    sys_id_manager_t id_manager;
    sys_id_manager_init(&id_manager);
    for (size_t i = 0; i < 100000; i++)
    {
        size_t tid = sys_id_alloc(&id_manager);
        printf("tid = %ld\n", tid);
        if (tid != i)
        {
            printf("error\n");
        }
    }
    for (size_t i = 0; i < 100; i++)
    {
        sys_id_free(&id_manager, i);
    }
    for (size_t i = 0; i < 110; i++)
    {
        size_t tid = sys_id_alloc(&id_manager);
        printf("tid = %ld\n", tid);
    }
    for (size_t i = 0; i < 100000; i++)
    {
        sys_id_free(&id_manager, i);
    }
    for (size_t i = 0; i < 100000; i++)
    {
        size_t tid = sys_id_alloc(&id_manager);
        printf("tid = %ld\n", tid);
        if (tid != i)
        {
            printf("error\n");
        }
    }
}
void *testMutexTaskD(void *arg);
static sys_mutex_t s_mutex;
void *testMutexTaskA(void *arg)
{
    sys_mutex_lock(&s_mutex);
    printf("This is task A\n");
    sys_mutex_unlock(&s_mutex);
    sys_tid_t tid;
    sys_task_create(&tid, testMutexTaskD, NULL, "testMutexTaskD", 20, 512);
    return NULL;
}

void *testMutexTaskB(void *arg)
{
    printf("This is task B\n");
    sys_mutex_unlock(&s_mutex);
    sys_mutex_unlock(&s_mutex);
    sys_mutex_unlock(&s_mutex);
    sys_mutex_lock(&s_mutex);
    sys_tid_t tid;
    sys_task_create_rt(&tid, testMutexTaskA, NULL, "testMutexTaskA", 20, 512);
    sys_task_sleep(5000);
    sys_mutex_unlock(&s_mutex);
    return NULL;
}

sys_recursive_lock_t s_secursive_lock;

void *testMutexTaskC(void *arg)
{
    sys_recursive_lock_lock(&s_secursive_lock);
    printf("This is task C\n");
    sys_recursive_lock_unlock(&s_secursive_lock);
    return NULL;
}

void *testMutexTaskD(void *arg)
{
    printf("This is task D\n");
    sys_recursive_lock_unlock(&s_secursive_lock);
    sys_recursive_lock_unlock(&s_secursive_lock);
    sys_recursive_lock_unlock(&s_secursive_lock);
    sys_recursive_lock_lock(&s_secursive_lock);
    sys_recursive_lock_lock(&s_secursive_lock);
    sys_recursive_lock_lock(&s_secursive_lock);
    sys_tid_t tid;
    sys_task_create_rt(&tid, testMutexTaskC, NULL, "testMutexTaskC", 20, 512);
    sys_task_sleep(2000);
    printf("sys_recursive_lock_unlock\n");
    sys_recursive_lock_unlock(&s_secursive_lock);
    sys_task_sleep(2000);
    printf("sys_recursive_lock_unlock\n");
    sys_recursive_lock_unlock(&s_secursive_lock);
    sys_task_sleep(2000);
    printf("sys_recursive_lock_unlock\n");
    sys_recursive_lock_unlock(&s_secursive_lock);
    return NULL;
}

void testMutex()
{
    sys_init();
    sys_mutex_create(&s_mutex);
    sys_recursive_lock_create(&s_secursive_lock);
    sys_tid_t tid;
    sys_task_create(&tid, testMutexTaskB, NULL, "testMutexTaskB", 20, 512);
    sys_task_start();
}

static sys_semaphore_t sSemaphore;
void *testSemaphoreTaskA(void *arg)
{
    printf("This is task A\n");
    sys_task_sleep(5000);
    for (int i = 0; i < 10; i++)
    {
        sys_semaphore_post(&sSemaphore);
    }
    sys_task_sleep(1000);
    sys_semaphore_destory(&sSemaphore);
    return NULL;
}

void *testSemaphoreTaskB(void *arg)
{
    printf("This is task B\n");
    int ret = sys_semaphore_wait(&sSemaphore, SYS_MESSAGE_MAX_WAIT_TIME);
    printf("ret = %d\n", ret);
    return NULL;
}

void *testSemaphoreTaskC(void *arg)
{
    printf("This is task C\n");
    int ret = sys_semaphore_wait(&sSemaphore, SYS_MESSAGE_MAX_WAIT_TIME);
    printf("ret = %d\n", ret);
    return NULL;
}

void *testSemaphoreTaskD(void *arg)
{
    printf("This is task D\n");
    int ret = sys_semaphore_wait(&sSemaphore, 2000);
    printf("ret = %d\n", ret);
    return NULL;
}

void testSemaphore()
{
    sys_init();
    sys_semaphore_create(&sSemaphore, 0, 0);
    for (int i = 0; i < 10; i++)
    {
        sys_semaphore_post(&sSemaphore);
    }
    for (int i = 0; i < 10; i++)
    {
        int ret = sys_semaphore_wait(&sSemaphore, 0);
        printf("ret = %d\n", ret);
    }
    sys_tid_t tid;
    sys_task_create(&tid, testSemaphoreTaskA, NULL, "testSemaphoreTaskA", 20, 512);
    sys_task_create_rt(&tid, testSemaphoreTaskB, NULL, "testSemaphoreTaskB", 20, 512);
    sys_task_create_rt(&tid, testSemaphoreTaskC, NULL, "testSemaphoreTaskC", 0, 512);
    sys_task_create(&tid, testSemaphoreTaskD, NULL, "testSemaphoreTaskC", 20, 512);
    sys_task_start();
}

static sys_msg_queue_t sQueue;
void *testQueueTaskA(void *arg)
{
    printf("This is task A\n");
    sys_task_sleep(5000);
    for (int i = 0; i < 10; i++)
    {
        sys_msg_queue_send(&sQueue, &i);
    }
    sys_task_sleep(1000);
    sys_msg_queue_destory(&sQueue);
    return NULL;
}

void *testQueueTaskB(void *arg)
{
    printf("This is task B\n");
    int message = -1;
    sys_msg_queue_receive(&sQueue, &message, SYS_MESSAGE_MAX_WAIT_TIME);
    printf("message = %d\n", message);
    return NULL;
}

void *testQueueTaskC(void *arg)
{
    printf("This is task C\n");
    int message = -1;
    sys_msg_queue_receive(&sQueue, &message, SYS_MESSAGE_MAX_WAIT_TIME);
    printf("message = %d\n", message);
    return NULL;
}

void *testQueueTaskD(void *arg)
{
    printf("This is task D\n");
    int message = -1;
    sys_msg_queue_receive(&sQueue, &message, 2000);
    printf("message = %d\n", message);
    return NULL;
}

void testQueue()
{
    sys_init();
    sys_msg_queue_create(&sQueue, 0, sizeof(int));
    for (int i = 0; i < 10; i++)
    {
        sys_msg_queue_send_to_front(&sQueue, &i);
    }
    for (int i = 0; i < 10; i++)
    {
        int message;
        sys_msg_queue_receive(&sQueue, &message, 0);
        printf("message = %d\n", message);
    }
    sys_tid_t tid;
    sys_task_create(&tid, testQueueTaskA, NULL, "testQueueTaskA", 20, 512);
    sys_task_create_rt(&tid, testQueueTaskB, NULL, "testQueueTaskB", 20, 512);
    sys_task_create_rt(&tid, testQueueTaskC, NULL, "testQueueTaskC", 0, 512);
    sys_task_create(&tid, testQueueTaskD, NULL, "testQueueTaskD", 20, 512);
    sys_task_start();
}

void *taskA(void *arg)
{
    for (;;)
    {
        printf("This is task A\n");
        printf("系统滴答：%ld\n", sys_task_get_tick_count());
        printf("任务个数：%d\n", sys_task_get_task_count());

        sys_tid_t tid = sys_task_get_tid();
        printf("任务tid：%d\n", tid);

        char name[SYS_TASK_MAX_NAME_LEN];
        int ret = sys_task_get_task_name(name, SYS_TASK_MAX_NAME_LEN, tid);
        printf("任务名：%s\n", name);

        sys_task_type_t type;
        ret = sys_task_get_task_type(&type, tid);
        printf("任务类型：%d\n", type);

        sys_task_state_t state;
        ret = sys_task_get_task_state(&state, tid);
        printf("任务状态：%d\n", state);

        int priority;
        ret = sys_task_get_task_priority(&priority, tid);
        printf("任务优先级：%d\n", priority);

        int stack_size;
        ret = sys_task_get_task_stack_size(&stack_size, tid);
        printf("任务堆栈大小：%d\n", stack_size);

        (void)ret;
        sys_task_sleep(1000);
    }
    return NULL;
}

void *taskB(void *arg)
{
    for (;;)
    {
        printf("This is task B\n");
        sys_task_supend(sys_task_get_tid());
    }
    return NULL;
}

void *taskC(void *arg)
{
    sys_tid_t tid;
    sys_task_create(&tid, taskB, NULL, "task b", 20, 512);
    for (;;)
    {
        printf("This is task C\n");
        sys_task_sleep(500);
        sys_task_resume(tid);
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
    sys_task_sleep(15000);
    return NULL;
}

void *taskF(void *arg)
{
    printf("This is task F\n");
    sys_task_sleep(10000);
    return taskF;
}

void *taskG(void *arg)
{
    printf("This is task G\n");
    sys_tid_t tid;
    sys_task_create_rt(&tid, taskD, NULL, "task d", 20, 512);
    sys_task_create_rt(&tid, taskE, NULL, "task e", 20, 512);
    sys_task_create_rt(&tid, taskF, NULL, "task f", 20, 512);
    sys_task_sleep(5000);
    void *retval;
    sys_task_join(&retval, tid);
    return NULL;
}

int main()
{   
    //testTree();
    //testBuddy();
    //testMemPool();
    //testMem();
    // testVector();
    //testDtScheduler();
    // testRtScheduler();
    // testTid();
    //testSemaphore();
    //testMutex();
    //testQueue();
    sys_init();
    //sys_tid_t tid;
    //sys_task_create(&tid, taskA, NULL, "task a", 20, 512);
    // sys_task_create(&tid, taskC, NULL, "task c", 20, 512);
    // sys_task_create_rt(&tid, taskG, NULL, "task g", 20, 512);
    ram_block_format();
    ram_block_create();
    register_lfs();
    sys_mount("/", "/dev/block");
    ram_block1_create();
    ram_block1_format();
    register_fatfs();
    sys_mkdir("/sd", VFS_MODE_IRUSR | VFS_MODE_IWUSR);
    sys_mount("/sd", "/dev/block1");
    //register_fatfs(); 
    //lfs_format(&g_lfs, &g_lfs_config);
    //f_mkfs("0:", NULL, NULL, FF_MAX_SS);
    shell_io_init();
    //lvglIOInit();
    sys_task_start();
    return 0;
}