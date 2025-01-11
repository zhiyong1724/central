#ifndef __SYS_SEMAPHORE_MANAGER_H__
#define __SYS_SEMAPHORE_MANAGER_H__
#include "sys_task_manager.h"
#include "sys_semaphore.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_semaphore_manager_t
{
    sys_task_manager_t *task_manager;
} sys_semaphore_manager_t;
/*********************************************************************************************************************
* sys_semaphore_manager_t初始化
* semaphore_manager：sys_semaphore_manager_t对象
* task_manager：任务管理器
* return：0：调用成功
*********************************************************************************************************************/
int sys_semaphore_manager_init(sys_semaphore_manager_t *semaphore_manager, sys_task_manager_t *task_manager);
/*********************************************************************************************************************
* sys_semaphore_t初始化
* semaphore_manager：sys_semaphore_manager_t对象
* semaphore：sys_semaphore_t对象
* count：初始值
* max_count：最大信号个数
* return：0：调用成功
*********************************************************************************************************************/
int sys_semaphore_manager_semaphore_init(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore, int count, int max_count);
/*********************************************************************************************************************
* 发送一个信号
* semaphore_manager：sys_semaphore_manager_t对象
* next_task：不为NULL表示下一个任务
* semaphore：sys_semaphore_t对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_semaphore_manager_post(sys_semaphore_manager_t *semaphore_manager, sys_task_t **next_task, sys_semaphore_t *semaphore);
/*********************************************************************************************************************
* 等待信号
* semaphore_manager：sys_semaphore_manager_t对象
* next_task：不为NULL表示下一个任务
* semaphore：sys_semaphore_t对象
* wait：等待时间，0表示马上返回，SYS_SEMAPHORE_MAX_WAIT_TIME表示永久等待
* return：0：调用成功
*********************************************************************************************************************/
int sys_semaphore_manager_wait(sys_semaphore_manager_t *semaphore_manager, sys_task_t **next_task, sys_semaphore_t *semaphore, uint64_t wait);
/*********************************************************************************************************************
* 获取信号数量
* semaphore_manager：sys_semaphore_manager_t对象
* semaphore：sys_semaphore_t对象
* return：消息数量
*********************************************************************************************************************/
int sys_semaphore_manager_get_semaphore_count(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore);
/*********************************************************************************************************************
* 获取最大信号数量
* semaphore_manager：sys_semaphore_manager_t对象
* semaphore：sys_semaphore_t对象
* return：最大信号数量
*********************************************************************************************************************/
int sys_semaphore_manager_get_max_semaphore_count(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore);
/*********************************************************************************************************************
* 移除等待的任务
* semaphore_manager：sys_semaphore_manager_t对象
* semaphore：sys_semaphore_t对象
* task：移除的任务
* return：0：调用成功
*********************************************************************************************************************/
int sys_semaphore_manager_remove_task(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore, sys_task_t *task);
/*********************************************************************************************************************
* 清空信号量
* semaphore_manager：sys_semaphore_manager_t对象
* semaphore：信号量对象
*********************************************************************************************************************/
void sys_semaphore_manager_reset(sys_semaphore_manager_t *semaphore_manager, sys_semaphore_t *semaphore);
#ifdef __cplusplus
}
#endif
#endif