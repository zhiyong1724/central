#include "sys_tasklet.h"
void sys_task_add_tasklet(int cpu, sys_tasklet_t *tasklet);
void sys_task_remove_tasklet(int cpu, sys_tasklet_t *tasklet);
void sys_tasklet_init(sys_tasklet_t *tasklet, void (*func)(void *arg), void *arg)
{
    sys_trace();
    sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_DETACHED);
    tasklet->cpu = -1;
    tasklet->enable = 1;
    tasklet->func = func;
    tasklet->arg = arg;
    sys_spin_lock_init(&tasklet->lock);
}

void sys_tasklet_kill(sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&tasklet->lock);
    while (SYS_TASKLET_STATE_RUN == sys_atomic_load(&tasklet->state));
    if (SYS_TASKLET_STATE_SCHED == sys_atomic_load(&tasklet->state))
    {
        sys_task_remove_tasklet(tasklet->cpu, tasklet);
        sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_DETACHED);
    }
    sys_spin_lock_unlock_and_irq_restore(&tasklet->lock, state);
}

void sys_tasklet_enable(sys_tasklet_t *tasklet)
{
    sys_trace();
    tasklet->enable = 1;
}

void sys_tasklet_disable(sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&tasklet->lock);
    while (SYS_TASKLET_STATE_RUN == sys_atomic_load(&tasklet->state));
    tasklet->enable = 0;
    sys_spin_lock_unlock_and_irq_restore(&tasklet->lock, state);
}

void sys_tasklet_disable_nosync(sys_tasklet_t *tasklet)
{
    sys_trace();
    tasklet->enable = 0;
}

void sys_tasklet_schedule(sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&tasklet->lock);
    if (tasklet->cpu < 0)
    {
        tasklet->cpu = sys_get_cpu();
    }

    if (SYS_TASKLET_STATE_DETACHED == sys_atomic_load(&tasklet->state))
    {
        sys_task_add_tasklet(tasklet->cpu, tasklet);
        sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_SCHED);
    }
    sys_spin_lock_unlock_and_irq_restore(&tasklet->lock, state);
}