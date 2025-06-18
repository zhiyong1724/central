#include "sys_rwlock.h"
void sys_rwlock_init(sys_rwlock_t *lock)
{
    sys_trace();
    sys_recursive_mutex_init(&lock->mutex);
    lock->read_count = 0;
    sys_spin_lock_init(&lock->lock);
}

void sys_rwlock_rdlock(sys_rwlock_t *lock)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&lock->lock);
    if (0 == lock->read_count)
    {
        sys_recursive_mutex_lock(&lock->mutex);
    }
    lock->read_count++;
    sys_spin_lock_unlock_and_irq_restore(&lock->lock, state);
}

void sys_rwlock_wrlock(sys_rwlock_t *lock)
{
    sys_trace();
    sys_recursive_mutex_lock(&lock->mutex);
}

void sys_rwlock_rdunlock(sys_rwlock_t *lock)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&lock->lock);
    if (lock->read_count > 0)
    {
        lock->read_count--;
        if (0 == lock->read_count)
        {
            sys_recursive_mutex_unlock(&lock->mutex);
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&lock->lock, state);
}

void sys_rwlock_wrunlock(sys_rwlock_t *lock)
{
    sys_trace();
    sys_recursive_mutex_unlock(&lock->mutex);
}