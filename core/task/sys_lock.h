#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__
#include "sys_semaphore.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct sys_mutex_t
{
    sys_semaphore_t semaphore;
} sys_mutex_t;

typedef struct sys_recursive_lock_t
{
    sys_semaphore_t semaphore;
    int recursive_count;
    void *owner;
} sys_recursive_lock_t;

typedef struct sys_rwlock_t
{
    sys_recursive_lock_t lock;
    int read_count;
} sys_rwlock_t;
/*********************************************************************************************************************
* 静态创建互斥量
* lock：互斥量对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_mutex_create(sys_mutex_t *lock);
/*********************************************************************************************************************
* 删除互斥量，如果有任务正在阻塞，会删除失败
* lock：互斥量对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_mutex_destory(sys_mutex_t *lock);
/*********************************************************************************************************************
* 获得锁
* lock：互斥量对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_mutex_lock(sys_mutex_t *lock);
/*********************************************************************************************************************
* 释放锁
* lock：互斥量对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_mutex_unlock(sys_mutex_t *lock);
/*********************************************************************************************************************
* 静态创建递归锁
* lock：递归锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_recursive_lock_create(sys_recursive_lock_t *lock);
/*********************************************************************************************************************
* 删除递归锁，如果有任务正在阻塞，会删除失败
* lock：递归锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_recursive_lock_destory(sys_recursive_lock_t *lock);
/*********************************************************************************************************************
* 获得锁
* lock：递归锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_recursive_lock_lock(sys_recursive_lock_t *lock);
/*********************************************************************************************************************
* 释放锁
* lock：递归锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_recursive_lock_unlock(sys_recursive_lock_t *lock);
/*********************************************************************************************************************
* 静态创建读写锁
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_create(sys_rwlock_t *lock);
/*********************************************************************************************************************
* 删除读写锁，如果有任务正在阻塞，会删除失败
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_destory(sys_rwlock_t *lock);
/*********************************************************************************************************************
* 获得读锁
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_rdlock(sys_rwlock_t *lock);
/*********************************************************************************************************************
* 获得写锁
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_wrlock(sys_rwlock_t *lock);
/*********************************************************************************************************************
* 释放读锁
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_rdunlock(sys_rwlock_t *lock);
/*********************************************************************************************************************
* 释放写锁
* lock：读写锁对象
* return：0：调用成功
*********************************************************************************************************************/
int sys_rwlock_wrunlock(sys_rwlock_t *lock);
#ifdef __cplusplus
}
#endif
#endif