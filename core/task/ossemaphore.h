#ifndef __OSSEMAPHORE_H__
#define __OSSEMAPHORE_H__
#include "ostaskdef.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define OS_SEMAPHORE_MAX_WAIT_TIME ((os_size_t)-1 / 1000 / 1000)
typedef struct OsSemaphore * os_semaphore_t;
/*********************************************************************************************************************
* 创建信号量
* semaphore：信号量对象
* count：初始值
* maxCount：最大信号个数
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreCreate(os_semaphore_t semaphore, os_size_t count, os_size_t maxCount);
/*********************************************************************************************************************
* 删除信号量，如果有任务正在阻塞，会删除失败
* semaphore：信号量对象
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreDestory(os_semaphore_t semaphore);
/*********************************************************************************************************************
* 清空信号量
* semaphore：信号量对象
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreReset(os_semaphore_t semaphore);
/*********************************************************************************************************************
* 释放一个信号
* semaphore：信号量对象
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphorePost(os_semaphore_t semaphore);
/*********************************************************************************************************************
* 等待信号
* semaphore：信号量对象
* wait：等待时间，0表示马上返回，OS_SEMAPHORE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreWait(os_semaphore_t semaphore, uint64_t wait);
/*********************************************************************************************************************
* 获取信号数量
* semaphore：信号量对象
* return：信号数量
*********************************************************************************************************************/
os_size_t osSemaphoreGetSemaphoreCount(os_semaphore_t semaphore);
/*********************************************************************************************************************
* 获取最大信号数量
* semaphore：信号量对象
* return：最大信号数量
*********************************************************************************************************************/
os_size_t osSemaphoreGetMaxSemaphoreCount(os_semaphore_t semaphore);
#ifdef __cplusplus
}
#endif
#endif