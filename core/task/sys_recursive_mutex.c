#include "sys_recursive_mutex.h"
#include "sys_task_scheduler.h"
sys_task_t *sys_task_get_running_task_inner();
void sys_recursive_mutex_init(sys_recursive_mutex_t *lock)
{
    sys_trace();
    sys_semaphore_init(&lock->semaphore, 1, 1);
    lock->recursive_count = 0;
    lock->owner = NULL;
    sys_spin_lock_init(&lock->lock);
}

void sys_recursive_mutex_lock(sys_recursive_mutex_t *lock)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&lock->lock);
    sys_task_t *task = sys_task_get_running_task_inner();
    if (task != lock->owner)
    {
        sys_semaphore_wait(&lock->semaphore, SYS_SEMAPHORE_MAX_WAIT_TIME);
        sys_spin_lock_unlock_and_irq_restore(&lock->lock, state);
        state = sys_spin_lock_lock_and_irq_save(&lock->lock);
        lock->owner = task;
    }
    lock->recursive_count++;
    sys_spin_lock_unlock_and_irq_restore(&lock->lock, state);
}

void sys_recursive_mutex_unlock(sys_recursive_mutex_t *lock)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&lock->lock);
    sys_task_t *task = sys_task_get_running_task_inner();
    if (task == lock->owner)
    {
        lock->recursive_count--;
        if (0 == lock->recursive_count)
        {
            lock->owner = NULL;
            sys_semaphore_post(&lock->semaphore);
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&lock->lock, state);
}