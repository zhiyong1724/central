#include "sys_lock.h"
#include "sys_semaphore.h"
#include "sys_task_manager.h"
int port_disable_interrupts();
int port_recovery_interrupts(int state);
sys_task_t *sys_task_get_running_task();
int sys_semaphore_wait1(sys_semaphore_t *semaphore, uint64_t wait);
int sys_mutex_create(sys_mutex_t *lock)
{
    sys_trace();
    return sys_semaphore_create(&lock->semaphore, 1, 1);
}

int sys_mutex_destory(sys_mutex_t *lock)
{
    sys_trace();
    return sys_semaphore_destory(&lock->semaphore);
}

int sys_mutex_lock(sys_mutex_t *lock)
{
    sys_trace();
    return sys_semaphore_wait(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
}

int sys_mutex_unlock(sys_mutex_t *lock)
{
    sys_trace();
    return sys_semaphore_post(&lock->semaphore);
}

int sys_recursive_lock_create(sys_recursive_lock_t *lock)
{
    sys_trace();
    lock->owner = NULL;
    lock->recursive_count = 0;
    return sys_semaphore_create(&lock->semaphore, 1, 1);
}

int sys_recursive_lock_destory(sys_recursive_lock_t *lock)
{
    sys_trace();
    return sys_semaphore_destory(&lock->semaphore);
}

int sys_recursive_lock_lock(sys_recursive_lock_t *lock)
{
    sys_trace();
    int ret = 0;
    int state = port_disable_interrupts();
    if (sys_task_get_running_task() == lock->owner)
    {
        lock->recursive_count++;
    }
    else
    {
        ret = sys_semaphore_wait1(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
        lock->owner = sys_task_get_running_task();
        lock->recursive_count++;
    }
    port_recovery_interrupts(state);
    return ret;
}

int sys_recursive_lock_unlock(sys_recursive_lock_t *lock)
{
    sys_trace();
    int ret = 0;
    int state = port_disable_interrupts();
    if (sys_task_get_running_task() == lock->owner)
    {
        lock->recursive_count--;
        if (0 == lock->recursive_count)
        {
            lock->owner = NULL;
            ret = sys_semaphore_post(&lock->semaphore);
        }
    }
    port_recovery_interrupts(state);
    return ret;
}

int sys_rwlock_create(sys_rwlock_t *lock)
{
    sys_trace();
    lock->owner = NULL;
    lock->read_count = 0;
    return sys_semaphore_create(&lock->semaphore, 1, 1);
}

int sys_rwlock_destory(sys_rwlock_t *lock)
{
    sys_trace();
    return sys_semaphore_destory(&lock->semaphore);
}

int sys_rwlock_rdlock(sys_rwlock_t *lock)
{
    sys_trace();
    int ret = 0;
    int state = port_disable_interrupts();
    if (lock->owner != NULL || 0 == lock->read_count)
    {
        ret = sys_semaphore_wait1(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
    }
    lock->read_count++;
    port_recovery_interrupts(state);
    return ret;
}

int sys_rwlock_wrlock(sys_rwlock_t *lock)
{
    sys_trace();
    int state = port_disable_interrupts();
    int ret = sys_semaphore_wait1(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
    lock->owner = sys_task_get_running_task();
    port_recovery_interrupts(state);
    return ret;
}

int sys_rwlock_unlock(sys_rwlock_t *lock)
{
    sys_trace();
    int ret = 0;
    int state = port_disable_interrupts();
    if (lock->owner == sys_task_get_running_task())
    {
        lock->owner = NULL;
        ret = sys_semaphore_post(&lock->semaphore);
    }
    else if (lock->read_count > 0)    
    {
        lock->read_count--;
        if (0 == lock->read_count)
        {
            ret = sys_semaphore_post(&lock->semaphore);
        }
    }
    port_recovery_interrupts(state);
    return ret;
}