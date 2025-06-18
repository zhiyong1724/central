#include "sys_timer.h"
void sys_timer_init(sys_timer_t *timer, int priority, uint64_t expires, void (*func)(void *arg), void *arg)
{
    sys_trace();
    timer->tid = 0;
    timer->priority = priority;
    timer->expires = expires;
    sys_atomic_store(&timer->state, SYS_TIMER_STATE_DISABLE);
    timer->enable = 0;
    timer->func = func;
    timer->arg = arg;
    sys_spin_lock_init(&timer->lock);
}

void sys_timer_delete(sys_timer_t *timer)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&timer->lock);
    while (SYS_TIMER_STATE_RUN == sys_atomic_load(&timer->state));
    if (SYS_TIMER_STATE_SCHED == sys_atomic_load(&timer->state))
    {
        timer->enable = 0;
    }
    sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
}

static void *timer_task(void *arg)
{
    sys_trace();
    sys_timer_t *timer = (sys_timer_t *)arg;
    int state = sys_spin_lock_lock_and_irq_save(&timer->lock);
    sys_atomic_store(&timer->state, SYS_TIMER_STATE_SCHED);
    timer->enable = 1;
    uint64_t expires = timer->expires;
    while (timer->enable && expires != 0)
    {
        sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
        sys_task_sleep(expires);
        state = sys_spin_lock_lock_and_irq_save(&timer->lock);
        if (!timer->enable || 0 == expires)
        {
            break;
        }
        timer->expires = 0;
        sys_atomic_store(&timer->state, SYS_TIMER_STATE_RUN);
        sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
        timer->func(timer->arg);
        sys_atomic_store(&timer->state, SYS_TIMER_STATE_SCHED);
        state = sys_spin_lock_lock_and_irq_save(&timer->lock);
        expires = timer->expires;
    }
    timer->tid = 0;
    sys_atomic_store(&timer->state, SYS_TIMER_STATE_DISABLE);
    sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
    return NULL;
}

void sys_timer_setup(sys_timer_t *timer)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&timer->lock);
    if (0 == timer->tid)
    {
        if (sys_task_create_rt(&timer->tid, timer_task, timer, "timer", timer->priority, SYS_DEFAULT_TASK_STACK_SIZE) == 0)
        {
            sys_task_detach(timer->tid);
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
}

void sys_timer_mod(sys_timer_t *timer, uint64_t expires)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&timer->lock);
    timer->expires = expires;
    sys_spin_lock_unlock_and_irq_restore(&timer->lock, state);
}