#include "sys_cpu_thread.h"
#include "sys_error.h"
#include "sys_mem.h"
#include "sys_task.h"
#define SYS_TASK_STACK_MAGIC                         0xaa5555aa         //用户堆栈魔术数
#define SYS_CPU_USAGE_CAL_CYCLE                     (1 * 1000 * 1000 * 1000l)
static void add_schedulers(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    sys_scheduler_interfaces_t scheduler_interfaces;
    scheduler_interfaces.add_task = (add_task_t)sys_fifo_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_fifo_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_fifo_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_fifo_scheduler_yield;
    scheduler_interfaces.set_priority = (set_priority_t)sys_fifo_scheduler_set_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_fifo_scheduler_get_running_task;
    sys_vscheduler_add_scheduler(&sys_cpu_thread->vscheduler, &sys_cpu_thread->fifo_scheduler, &scheduler_interfaces);

    scheduler_interfaces.add_task = (add_task_t)sys_rt_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_rt_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_rt_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_rt_scheduler_yield;
    scheduler_interfaces.set_priority = (set_priority_t)sys_rt_scheduler_set_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_rt_scheduler_get_running_task;
    sys_vscheduler_add_scheduler(&sys_cpu_thread->vscheduler, &sys_cpu_thread->rt_scheduler, &scheduler_interfaces);

    scheduler_interfaces.add_task = (add_task_t)sys_dt_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_dt_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_dt_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_dt_scheduler_yield;
    scheduler_interfaces.set_priority = (set_priority_t)sys_dt_scheduler_set_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_dt_scheduler_get_running_task;
    sys_vscheduler_add_scheduler(&sys_cpu_thread->vscheduler, &sys_cpu_thread->dt_scheduler, &scheduler_interfaces);

    scheduler_interfaces.add_task = (add_task_t)sys_idle_scheduler_add_task;
    scheduler_interfaces.remove_task = (remove_task_t)sys_idle_scheduler_remove_task;
    scheduler_interfaces.tick = (tick_t)sys_idle_scheduler_tick;
    scheduler_interfaces.yield = (yield_t)sys_idle_scheduler_yield;
    scheduler_interfaces.set_priority = (set_priority_t)sys_idle_scheduler_set_priority;
    scheduler_interfaces.get_running_task = (get_running_task_t)sys_idle_scheduler_get_running_task;
    sys_vscheduler_add_scheduler(&sys_cpu_thread->vscheduler, &sys_cpu_thread->idle_scheduler, &scheduler_interfaces);
}

static void check_stack(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    int ret = -1;
    sys_task_control_block_t *task = sys_container_of(sys_vscheduler_get_running_task(&sys_cpu_thread->vscheduler), sys_task_control_block_t, vtask_control_block);
    if ((stack_size_t)SYS_TASK_STACK_MAGIC == *task->task_stack_magic)
    {
#if (SYS_TASK_STACK_GROWTH > 0)
        if ((char *)task->stack_top > (char *)task->stack_start + sizeof(stack_size_t))
        {
            ret = 0;
        }
#else
        if ((char *)task->stack_top < (char *)task->stack_start + task->stack_size - sizeof(stack_size_t))
        {
            ret = 0;
        }
#endif
    }
    if (ret < 0)
    {
        sys_error("Task %p stack overflow\n", task->task_function);
        sys_assert(0);
        for (;;)
        {
        }
    }
}

void sys_cpu_thread_init(sys_cpu_thread_t *sys_cpu_thread, sys_cpu_info_t *cpu_info, sys_cpu_operations_t *operations)
{
    sys_trace();
    sys_cpu_thread->info = *cpu_info;
    sys_cpu_thread->operations = *operations;
    sys_vscheduler_init(&sys_cpu_thread->vscheduler);
    sys_fifo_scheduler_init(&sys_cpu_thread->fifo_scheduler);
    sys_rt_scheduler_init(&sys_cpu_thread->rt_scheduler);
    sys_dt_scheduler_init(&sys_cpu_thread->dt_scheduler);
    sys_idle_scheduler_init(&sys_cpu_thread->idle_scheduler);
    sys_cpu_thread->sleep_tree = NULL;
    sys_cpu_thread->min_sleep_task = NULL;
    add_schedulers(sys_cpu_thread);
    sys_cpu_thread->clock = 0;
    sys_atomic_store(&sys_cpu_thread->atomic_clock, 0);
    sys_cpu_thread->all_time = 0;
    sys_cpu_thread->idle_task_time = 0;
    sys_cpu_thread->cpu_usage = 0;
    sys_spin_lock_init(&sys_cpu_thread->lock);
    sys_cpu_thread->handle = NULL;
    sys_cpu_thread->load_leveling = NULL;
    sys_cpu_thread->softirq_task = NULL;
    sys_cpu_thread->tasklet_list = NULL;
}

int sys_cpu_thread_add_task(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block, task_function_t task_function, void *arg, sys_task_type_t task_type, int priority, int stack_size)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    int ret = 0;
    task_control_block->sleep_time = 0;
    task_control_block->cpu_thread = sys_cpu_thread;
    task_control_block->desired_cpu = sys_cpu_thread;
    task_control_block->wakeup_callback = NULL;
    task_control_block->wakeup_arg = NULL;
    task_control_block->wait = 0;
    task_control_block->stack_start = (stack_size_t *)sys_malloc(stack_size);
    if (NULL == task_control_block->stack_start)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    task_control_block->task_function = task_function;
    task_control_block->stack_size = stack_size;
#if (SYS_TASK_STACK_GROWTH > 0)
    task_control_block->task_stack_magic = task_control_block->stack_start;
    *task_control_block->task_stack_magic = (stack_size_t)SYS_TASK_STACK_MAGIC;
    task_control_block->stack_top = (stack_size_t *)((char *)task_control_block->stack_start + stack_size);
#else
    task_control_block->task_stack_magic = (stack_size_t *)((char *)task_control_block->stack_start + stack_size - sizeof(stack_size_t));
    *task_control_block->task_stack_magic = (stack_size_t)SYS_TASK_STACK_MAGIC;
    task_control_block->stack_top = task_control_block->stack_start;
#endif
    sys_cpu_thread->operations.initialize_stack(&task_control_block->stack_top, task_control_block->stack_size, task_control_block->task_function, arg);
    sys_vtask_control_block_init(&sys_cpu_thread->vscheduler, &task_control_block->vtask_control_block);
    task_control_block->vtask_control_block.scheduler_id = task_type;
    switch (task_type)
    {
    case SYS_TASK_TYPE_FIFO:
        sys_fifo_task_control_block_init(&sys_cpu_thread->fifo_scheduler, &task_control_block->real_task_control_block.fifo_task_control_block, priority);
        break;
    case SYS_TASK_TYPE_RT:
        sys_rt_task_control_block_init(&sys_cpu_thread->rt_scheduler, &task_control_block->real_task_control_block.rt_task_control_block, priority);
        break;
    case SYS_TASK_TYPE_DT:
        sys_dt_task_control_block_init(&sys_cpu_thread->dt_scheduler, &task_control_block->real_task_control_block.dt_task_control_block, priority);
        break;
    case SYS_TASK_TYPE_IDLE:
        sys_idle_task_control_block_init(&sys_cpu_thread->idle_scheduler, &task_control_block->real_task_control_block.idle_task_control_block, priority);
        break;
    default:
        break;
    }
    ret = sys_vscheduler_add_task(&sys_cpu_thread->vscheduler, &task_control_block->vtask_control_block);
    if (ret < 0)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    goto finally;
exception:
    if (task_control_block->stack_start != NULL)
    {
        sys_free(task_control_block->stack_start);
    }
finally:
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return ret;
}

int sys_cpu_thread_set_priority(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block, int priority)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    int ret = sys_vscheduler_set_priority(&sys_cpu_thread->vscheduler, &task_control_block->vtask_control_block, priority);
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return ret;
}

static int on_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    sys_cpu_thread_t *sys_cpu_thread = (sys_cpu_thread_t *)arg;
    sys_task_control_block_t *task1 = sys_container_of(key1, sys_task_control_block_t, sleep_tree_node);
    sys_task_control_block_t *task2 = sys_container_of(key2, sys_task_control_block_t, sleep_tree_node);
    if (task1->sleep_time - sys_cpu_thread->clock < task2->sleep_time - sys_cpu_thread->clock)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

static void sys_cpu_thread_suspend_inner(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    sys_task_control_block_t *next = sys_container_of(sys_vscheduler_suspend(&sys_cpu_thread->vscheduler, &task_control_block->vtask_control_block), sys_task_control_block_t, vtask_control_block);
    sys_cpu_thread->operations.yield(&next->stack_top);
}

sys_task_control_block_t *sys_cpu_thread_sleep(sys_cpu_thread_t *sys_cpu_thread, uint64_t ns, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    sys_task_control_block_t *task = sys_container_of(sys_vscheduler_get_running_task(&sys_cpu_thread->vscheduler), sys_task_control_block_t, vtask_control_block);
    sys_cpu_thread_suspend_inner(sys_cpu_thread, task);
    task->sleep_time = sys_cpu_thread->clock + ns;
    sys_insert_node(&sys_cpu_thread->sleep_tree, &task->sleep_tree_node, on_compare, sys_cpu_thread);
    if (NULL == sys_cpu_thread->min_sleep_task)
    {
        sys_cpu_thread->min_sleep_task = task;
    }
    else
    {
        sys_cpu_thread->min_sleep_task = sys_container_of(sys_get_left_most_node(sys_cpu_thread->sleep_tree), sys_task_control_block_t, sleep_tree_node);
    }
    task->vtask_control_block.task_state = SYS_TASK_STATE_SLEEP;
    task->wakeup_callback = wakeup_callback;
    task->wakeup_arg = arg;
    task->wait = 1;
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return task;
}

static void sys_cpu_thread_resume_inner(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    sys_task_control_block_t *next = sys_container_of(sys_vscheduler_resume(&sys_cpu_thread->vscheduler, &task_control_block->vtask_control_block), sys_task_control_block_t, vtask_control_block);
    sys_cpu_thread->operations.yield(&next->stack_top);
}

int sys_cpu_thread_wakeup(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    if (NULL == task || task->vtask_control_block.task_state != SYS_TASK_STATE_SLEEP)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_delete_node(&sys_cpu_thread->sleep_tree, &task->sleep_tree_node);
    task->vtask_control_block.task_state = SYS_TASK_STATE_SUSPEND;
    sys_cpu_thread->min_sleep_task = sys_container_of(sys_get_left_most_node(sys_cpu_thread->sleep_tree), sys_task_control_block_t, sleep_tree_node);
    task->cpu_thread = task->desired_cpu;
    if (task->cpu_thread == sys_cpu_thread)
    {
        sys_cpu_thread_resume_inner(sys_cpu_thread, task);
    }
    else
    {
        sys_spin_lock_unlock(&sys_cpu_thread->lock);
        sys_cpu_thread_resume(task->cpu_thread, task);
        sys_spin_lock_lock(&sys_cpu_thread->lock);
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return ret;
}

static void sleep_tree_tick(sys_cpu_thread_t *sys_cpu_thread, uint64_t ns)
{
    // sys_trace();
    sys_cpu_thread->clock += ns;
    sys_atomic_fetch_add(&sys_cpu_thread->atomic_clock, (int)ns);
    for (;;)
    {
        if (sys_cpu_thread->min_sleep_task != NULL && sys_cpu_thread->min_sleep_task->sleep_time + ns - sys_cpu_thread->clock <= ns)
        {
            sys_delete_node(&sys_cpu_thread->sleep_tree, &sys_cpu_thread->min_sleep_task->sleep_tree_node);
            sys_cpu_thread->min_sleep_task->vtask_control_block.task_state = SYS_TASK_STATE_SUSPEND;
            if (sys_cpu_thread->min_sleep_task->wakeup_callback != NULL)
            {
                sys_cpu_thread->min_sleep_task->wakeup_callback(sys_cpu_thread->min_sleep_task, sys_cpu_thread->min_sleep_task->wakeup_arg);
                sys_cpu_thread->min_sleep_task->wakeup_callback = NULL;
            }
            sys_task_control_block_t *task = sys_cpu_thread->min_sleep_task;
            sys_cpu_thread->min_sleep_task = sys_container_of(sys_get_left_most_node(sys_cpu_thread->sleep_tree), sys_task_control_block_t, sleep_tree_node);
            task->cpu_thread = task->desired_cpu;            
            if (task->cpu_thread == sys_cpu_thread)
            {
                sys_cpu_thread_resume_inner(sys_cpu_thread, task);
            }
            else
            {
                sys_spin_lock_unlock(&sys_cpu_thread->lock);
                sys_cpu_thread_resume(task->cpu_thread, task);
                sys_spin_lock_lock(&sys_cpu_thread->lock);
            }
        }
        else
        {
            break;
        }
    }
}

void sys_cpu_thread_tick(sys_cpu_thread_t *sys_cpu_thread, uint64_t ns)
{
    //sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    check_stack(sys_cpu_thread);
    sys_task_control_block_t *task = sys_container_of(sys_vscheduler_tick(&sys_cpu_thread->vscheduler, ns), sys_task_control_block_t, vtask_control_block);
    sys_cpu_thread->all_time += ns;
    if (SYS_TASK_TYPE_IDLE == task->vtask_control_block.scheduler_id)
    {
        sys_cpu_thread->idle_task_time += ns;
    }
    if (sys_cpu_thread->all_time >= SYS_CPU_USAGE_CAL_CYCLE)
    {
        sys_cpu_thread->cpu_usage = 100 - (int)(100 * sys_cpu_thread->idle_task_time / sys_cpu_thread->all_time);
        sys_cpu_thread->all_time = 0;
        sys_cpu_thread->idle_task_time = 0;
    }
    sys_cpu_thread->operations.yield(&task->stack_top);
    if (sys_cpu_thread->dt_scheduler.load_leveling_flag > 0)
    {
        sys_cpu_thread->dt_scheduler.load_leveling_flag = 0;
        sys_spin_lock_unlock(&sys_cpu_thread->lock);
        sys_cpu_thread->load_leveling(sys_cpu_thread->handle, sys_cpu_thread);
        sys_spin_lock_lock(&sys_cpu_thread->lock);
    }
    sleep_tree_tick(sys_cpu_thread, ns);
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

void sys_cpu_thread_suspend(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    if (task_control_block->vtask_control_block.task_state != SYS_TASK_STATE_READY)
    {
        sys_error("Invalid argument.");
        goto exception;
    }
    sys_cpu_thread_suspend_inner(sys_cpu_thread, task_control_block);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

void sys_cpu_thread_resume(sys_cpu_thread_t *sys_cpu_thread, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    if (task_control_block->vtask_control_block.task_state != SYS_TASK_STATE_SUSPEND)
    {
        sys_error("Invalid argument.");
        goto exception;
    }
    task_control_block->cpu_thread = task_control_block->desired_cpu;
    if (task_control_block->cpu_thread == sys_cpu_thread)
    {
        sys_cpu_thread_resume_inner(sys_cpu_thread, task_control_block);
    }
    else
    {
        sys_spin_lock_unlock(&sys_cpu_thread->lock);
        sys_cpu_thread_resume(task_control_block->cpu_thread, task_control_block);
        sys_spin_lock_lock(&sys_cpu_thread->lock);
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

sys_task_control_block_t *sys_cpu_thread_get_running_task(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    sys_task_control_block_t *task = sys_container_of(sys_vscheduler_get_running_task(&sys_cpu_thread->vscheduler), sys_task_control_block_t, vtask_control_block);
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return task;
}

int sys_cpu_thread_get_task_count(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    int count = sys_vscheduler_get_task_count(&sys_cpu_thread->vscheduler);
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return count;
}

void sys_cpu_thread_start(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    sys_task_control_block_t *task = sys_container_of(sys_vscheduler_get_running_task(&sys_cpu_thread->vscheduler), sys_task_control_block_t, vtask_control_block);
    if (task != NULL)
    {
        sys_cpu_thread->operations.start_thread(&task->stack_top);
    }
    while (1)
    {
    }
}

int sys_cpu_thread_get_cpu_usage(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    int ret;
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    ret = sys_cpu_thread->cpu_usage;
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return ret;
}

void sys_cpu_thread_set_load_leveling_callback(sys_cpu_thread_t *sys_cpu_thread, void *handle, load_leveling_t load_leveling)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    sys_cpu_thread->handle = handle;
    sys_cpu_thread->load_leveling = load_leveling;
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

void sys_cpu_thread_softirq_task(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    for (;;)
    {
        int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
        while (sys_cpu_thread->tasklet_list != NULL)
        {
            sys_tasklet_t *tasklet = sys_container_of(sys_cpu_thread->tasklet_list, sys_tasklet_t, node);
            sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
            {
                int state1 = sys_spin_lock_lock_and_irq_save(&tasklet->lock);
                state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
                if (SYS_TASKLET_STATE_SCHED == sys_atomic_load(&tasklet->state))
                {
                    sys_remove_from_list(&sys_cpu_thread->tasklet_list, &tasklet->node);
                    sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_DETACHED);
                    if (tasklet->enable > 0)
                    {
                        sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_RUN);
                        sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
                        sys_spin_lock_unlock_and_irq_restore(&tasklet->lock, state1);
                        tasklet->func(tasklet->arg);
                        sys_atomic_store(&tasklet->state, SYS_TASKLET_STATE_DETACHED);
                        state1 = sys_spin_lock_lock_and_irq_save(&tasklet->lock);
                        state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
                    }
                }
                sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
                sys_spin_lock_unlock_and_irq_restore(&tasklet->lock, state1);
            }
            state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
        }
        sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
        sys_cpu_thread_suspend(sys_cpu_thread, sys_cpu_thread->softirq_task);
    }
}

void sys_cpu_thread_add_tasklet(sys_cpu_thread_t *sys_cpu_thread, sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    sys_insert_to_back(&sys_cpu_thread->tasklet_list, &tasklet->node);
    if (SYS_TASK_STATE_SUSPEND == sys_cpu_thread->softirq_task->vtask_control_block.task_state)
    {
        sys_cpu_thread_resume_inner(sys_cpu_thread, sys_cpu_thread->softirq_task);
    }
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

void sys_cpu_thread_remove_tasklet(sys_cpu_thread_t *sys_cpu_thread, sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    sys_remove_from_list(&sys_cpu_thread->tasklet_list, &tasklet->node);
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
}

uint64_t sys_cpu_thread_get_clock(sys_cpu_thread_t *sys_cpu_thread)
{
    sys_trace();
    uint64_t clock;
    int state = sys_spin_lock_lock_and_irq_save(&sys_cpu_thread->lock);
    clock = sys_cpu_thread->clock;
    sys_spin_lock_unlock_and_irq_restore(&sys_cpu_thread->lock, state);
    return clock;
}
