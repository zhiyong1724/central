#include "sys_spin_lock.h"
#include "sys_atomic.h"
void sys_spin_lock_init(sys_spin_lock_t *lock)
{
    sys_trace();
    sys_atomic_store(&lock->locked, 0);
}

void sys_spin_lock_lock(sys_spin_lock_t *lock)
{
    sys_trace();
    for (int expected = 0; !sys_atomic_compare_exchange(&lock->locked, &expected, 1); expected = 0)
    {
    }
}

void sys_spin_lock_unlock(sys_spin_lock_t *lock)
{
    sys_trace();
    sys_atomic_store(&lock->locked, 0);
}

int sys_spin_lock_lock_and_irq_save(sys_spin_lock_t *lock)
{
    sys_trace();
    int state = sys_local_irq_save();
    sys_spin_lock_lock(lock);
    return state;
}

void sys_spin_lock_unlock_and_irq_restore(sys_spin_lock_t *lock, int state)
{
    sys_trace();
    sys_spin_lock_unlock(lock);
    sys_local_irq_restore(state);
}