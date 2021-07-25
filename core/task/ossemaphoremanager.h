#ifndef __OSSEMAPHOREMANAGER_H__
#define __OSSEMAPHOREMANAGER_H__
#include "ostaskdef.h"
#include "oslist.h"
#include "ostaskmanager.h"
#include "ostree.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsSemaphoreManager
{
    OsTaskManager *taskManager;
} OsSemaphoreManager;
/*********************************************************************************************************************
* OsSemaphoreManager初始化
* semaphoreManager：OsSemaphoreManager对象
* taskManager：任务管理器
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerInit(OsSemaphoreManager *semaphoreManager, OsTaskManager *taskManager);
/*********************************************************************************************************************
* OsSemaphore初始化
* semaphoreManager：OsSemaphoreManager对象
* semaphore：OsSemaphore对象
* count：初始值
* maxCount：最大信号个数
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerSemaphoreInit(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore, os_size_t count, os_size_t maxCount);
/*********************************************************************************************************************
* 发送一个信号
* semaphoreManager：OsSemaphoreManager对象
* semaphore：OsSemaphore对象
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerPost(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore);
/*********************************************************************************************************************
* 等待信号
* semaphoreManager：OsSemaphoreManager对象
* nextTask：不为NULL表示下一个任务
* semaphore：OsSemaphore对象
* wait：等待时间，0表示马上返回，OS_SEMAPHORE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerWait(OsSemaphoreManager *semaphoreManager, OsTask **nextTask, OsSemaphore *semaphore, uint64_t wait);
/*********************************************************************************************************************
* 获取信号数量
* semaphoreManager：OsSemaphoreManager对象
* semaphore：OsSemaphore对象
* return：消息数量
*********************************************************************************************************************/
os_size_t osSemaphoreManagerGetSemaphoreCount(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore);
/*********************************************************************************************************************
* 获取最大信号数量
* semaphoreManager：OsSemaphoreManager对象
* semaphore：OsSemaphore对象
* return：最大信号数量
*********************************************************************************************************************/
os_size_t osSemaphoreManagerGetMaxSemaphoreCount(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore);
/*********************************************************************************************************************
* 移除等待的任务
* semaphoreManager：OsSemaphoreManager对象
* semaphore：OsSemaphore对象
* task：移除的任务
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerRemoveTask(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore, OsTask *task);
/*********************************************************************************************************************
* 清空信号量
* semaphoreManager：OsSemaphoreManager对象
* semaphore：信号量对象
* return：0：调用成功
*********************************************************************************************************************/
int osSemaphoreManagerReset(OsSemaphoreManager *semaphoreManager, OsSemaphore *semaphore);
#ifdef __cplusplus
}
#endif
#endif