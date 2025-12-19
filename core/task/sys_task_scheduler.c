#include "sys_task_scheduler.h"
#include "sys_error.h"
#include "sys_string.h"
#include "sys_mem.h"
#define SYS_TASK_MAX_SIZE (int)(((unsigned int)~0) >> 1)

static sys_task_t *get_task_by_tid(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int size = sys_vector_size(&scheduler->task_list);
    if (tid < size)
    {
        sys_task_t **ptask = (sys_task_t **)sys_vector_at(&scheduler->task_list, tid);
        if (*ptask != NULL)
        {
            return *ptask;
        }
    }
    return NULL;
}

static void delete_task(sys_task_scheduler_t *scheduler, sys_task_t *task)
{
    sys_trace();
    sys_id_free(&scheduler->id_manager, task->tid);
    sys_free(task->task_control_block.stack_start);
    void **pp = (void **)sys_vector_at(&scheduler->task_list, task->tid);
    *pp = NULL;
    sys_free(task);
    scheduler->task_count--;
}

static void move_children(sys_task_scheduler_t *scheduler, sys_task_t *task)
{
    sys_trace();
    while (task->children_count > 0)
    {
        sys_task_t *child = sys_container_of(task->children, sys_task_t, list_node);
        sys_remove_from_list(&task->children, &child->list_node);
        if (SYS_TASK_STATE_DEAD == child->task_control_block.vtask_control_block.task_state)
        {
            delete_task(scheduler, child);
        }
        else
        {
            child->parent = scheduler->init_task;
            sys_insert_to_back(&scheduler->init_task->children, &child->list_node);
            scheduler->init_task->children_count++;
        }
        task->children_count--;
    }
}

int sys_task_scheduler_init(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int ret = 0;
    scheduler->cpu_threads = NULL;
    scheduler->cpu_thread_count = 0;
    ret = sys_id_manager_init(&scheduler->id_manager);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_vector_init(&scheduler->task_list, sizeof(void *));
    if (ret < 0)
    {
        return ret;
    }
    scheduler->task_count = 0;
    sys_spin_lock_init(&scheduler->lock);
    scheduler->dead_task_list = NULL;
    scheduler->rt_task_tree = NULL;
    scheduler->high_load_cpu = NULL;
    return ret;
}

static void *idle_task(void *arg)
{
    for (;;)
    {
    }
    return NULL;
}

static void *softirq_task(void *arg)
{
    sys_cpu_thread_softirq_task((sys_cpu_thread_t *)arg);
    return NULL;
} 

static void *init_task(void *arg)
{
    sys_trace();
    sys_task_scheduler_t *scheduler = (sys_task_scheduler_t *)arg;
    for (;;)
    {
        int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
        while (scheduler->dead_task_list != NULL)
        {
            sys_task_t *task = sys_container_of(scheduler->dead_task_list, sys_task_t, list_node);
            sys_remove_from_list(&scheduler->dead_task_list, &task->list_node);
            delete_task(scheduler, task);
        }
        sys_cpu_thread_suspend(scheduler->init_task->task_control_block.cpu_thread, &scheduler->init_task->task_control_block);
        sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    }
    return NULL;
}

static int on_insert_rt_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    sys_task_t *task1 = sys_container_of(key1, sys_task_t, rt_task_node);
    sys_task_t *task2 = sys_container_of(key2, sys_task_t, rt_task_node);
    if (task1->task_control_block.real_task_control_block.rt_task_control_block.priority < task2->task_control_block.real_task_control_block.rt_task_control_block.priority)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

static sys_cpu_thread_t *get_cpu_thread_by_index(sys_task_scheduler_t *scheduler, int index)
{
    sys_trace();
    sys_cpu_thread_t *cpu_thread = (sys_cpu_thread_t *)scheduler->cpu_threads;
    for (int i = 0; i < index; i++, cpu_thread = (sys_cpu_thread_t *)cpu_thread->node.next)
    {
        if (NULL == cpu_thread)
        {
            sys_error("Invalid argument.");
            break;
        }
    }
    return cpu_thread;
}

sys_task_t *sys_task_scheduler_get_running_task_inner(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int cpu = sys_get_cpu();
    sys_cpu_thread_t *cpu_thread = get_cpu_thread_by_index(scheduler, cpu);
    if (cpu_thread != NULL)
    {
        sys_task_control_block_t *task_control_block = sys_cpu_thread_get_running_task(cpu_thread);
        return sys_container_of(task_control_block, sys_task_t, task_control_block);
    }
    return NULL;
}

static int create_task(sys_task_scheduler_t *scheduler, int cpu, sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, sys_task_type_t task_type, int priority, int stack_size, sys_task_t **out)
{
    sys_trace();
    int ret = 0;
    sys_task_t *task = NULL;
    if (scheduler->task_count >= SYS_TASK_MAX_SIZE)
    {
        sys_error("No child processes.");
        ret = SYS_ERROR_CHILD;
        goto exception;
    }
    task = (sys_task_t *)sys_malloc(sizeof(sys_task_t));
    if (NULL == task)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    sys_strcpy(task->name, name, SYS_TASK_MAX_NAME_LEN);
    task->tid = sys_id_alloc(&scheduler->id_manager);
    if (task->tid < 0)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    task->parent = sys_task_scheduler_get_running_task_inner(scheduler);
    if (NULL == task->parent)
    {
        task->parent = scheduler->init_task;
    }
    task->children = NULL;
    task->children_count = 0;
    task->ret_arg = NULL;
    task->wait_tid = 0;

    int size = sys_vector_size(&scheduler->task_list);
    if (task->tid >= size)
    {
        void *p = NULL;
        ret = sys_vector_push_back(&scheduler->task_list, &p);
        if (ret < 0)
        {
            sys_error("Out of memory.");
            ret = SYS_ERROR_NOMEM;
            goto exception;
        }
    }
    void **t = (void **)sys_vector_at(&scheduler->task_list, task->tid);
    *t = task;
    sys_cpu_thread_t *cpu_thread = get_cpu_thread_by_index(scheduler, cpu);
    ret = sys_cpu_thread_add_task(cpu_thread, &task->task_control_block, task_function, arg, task_type, priority, stack_size);
    if (ret < 0)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    if (task->parent != NULL)
    {
        sys_insert_to_back(&task->parent->children, &task->list_node);
        task->parent->children_count++;
    }
    scheduler->task_count++;
    *tid = task->tid;
    if (SYS_TASK_TYPE_RT == task_type)
    {
        sys_insert_node(&scheduler->rt_task_tree, &task->rt_task_node, on_insert_rt_compare, NULL);
    }
    *out = task;
    goto finally;
exception:
    if (task != NULL && task->tid >= 0)
    {
        sys_id_free(&scheduler->id_manager, task->tid);
    }
    if (task != NULL)
    {
        sys_free(task);
    }
finally:
    return ret;
}

static void load_leveling(sys_cpu_thread_t *high_load_cpu, sys_cpu_thread_t *low_load_cpu)
{
    sys_trace();
    sys_spin_lock_lock(&high_load_cpu->lock);
    sys_spin_lock_lock(&low_load_cpu->lock);
    uint64_t high_load = high_load_cpu->dt_scheduler.mean_load * 100 / high_load_cpu->info.ability;
    uint64_t low_load = low_load_cpu->dt_scheduler.mean_load * 100 / low_load_cpu->info.ability;
    if (high_load > low_load && high_load - low_load > (high_load + low_load) >> 2)
    {
        for (;;)
        {
            if (high_load_cpu->dt_scheduler.task_count <= 1)
            {
                break;
            }
            uint64_t diff = high_load_cpu->dt_scheduler.load * 100 / high_load_cpu->info.ability - low_load_cpu->dt_scheduler.load * 100 / low_load_cpu->info.ability;
            sys_dt_task_control_block_t *dt_task_control_block = (sys_dt_task_control_block_t *)sys_dt_scheduler_find_last_task(&high_load_cpu->dt_scheduler);
            high_load = sys_dt_scheduler_load_if_remove_task(&high_load_cpu->dt_scheduler, dt_task_control_block) * 100 / high_load_cpu->info.ability;
            low_load = sys_dt_scheduler_load_if_add_task(&low_load_cpu->dt_scheduler, dt_task_control_block) * 100 / low_load_cpu->info.ability;
            uint64_t diff1 = high_load > low_load ? high_load - low_load : low_load - high_load;
            if (diff1 >= diff)
            {
                break;
            }
            sys_vscheduler_suspend(&high_load_cpu->vscheduler, (sys_vtask_control_block_t *)dt_task_control_block - 1);
            sys_dt_task_control_block_init(&low_load_cpu->dt_scheduler, dt_task_control_block, dt_task_control_block->priority);
            sys_vscheduler_resume(&low_load_cpu->vscheduler, (sys_vtask_control_block_t *)dt_task_control_block - 1);
            sys_task_control_block_t *task_control_block = sys_container_of(dt_task_control_block, sys_task_control_block_t, real_task_control_block);
            sys_task_t *task = sys_container_of(task_control_block, sys_task_t, task_control_block);
            task->task_control_block.cpu_thread = low_load_cpu;
            task->task_control_block.desired_cpu = low_load_cpu;
        }
    }
    sys_spin_lock_unlock(&low_load_cpu->lock);
    sys_spin_lock_unlock(&high_load_cpu->lock);
}

static void load_leveling_callback(sys_task_scheduler_t *scheduler, sys_cpu_thread_t *cpu_thread)
{
    sys_trace();
    if (scheduler->cpu_thread_count > 1)
    {
        sys_spin_lock_lock(&scheduler->lock);
        if (NULL == scheduler->high_load_cpu)
        {
            scheduler->high_load_cpu = cpu_thread;
        }
        if (cpu_thread->dt_scheduler.load * 100 / cpu_thread->info.ability > scheduler->high_load_cpu->dt_scheduler.load * 100 / scheduler->high_load_cpu->info.ability)
        {
            scheduler->high_load_cpu = cpu_thread;
        }
        sys_cpu_thread_t *high_load_cpu = scheduler->high_load_cpu;
        sys_spin_lock_unlock(&scheduler->lock);
        if (high_load_cpu != cpu_thread)
        {
            load_leveling(high_load_cpu, cpu_thread);
        }
    }
}

int sys_task_scheduler_add_cpu_thread(sys_task_scheduler_t *scheduler, sys_cpu_thread_t *cpu_thread)
{
    sys_trace();
    int ret;
    sys_insert_to_back(&scheduler->cpu_threads, &cpu_thread->node);
    scheduler->cpu_thread_count++;
    sys_cpu_thread_set_load_leveling_callback(cpu_thread, scheduler, (load_leveling_t)load_leveling_callback);
    sys_tid_t tid;
    if (1 == scheduler->cpu_thread_count)
    {
        sys_task_t *task = NULL;
        ret = create_task(scheduler, cpu_thread->info.index, &tid, init_task, scheduler, "init", SYS_TASK_TYPE_RT, 63, SYS_DEFAULT_TASK_STACK_SIZE, &task);
        if (ret < 0)
            return ret;
        scheduler->init_task = task;
    }
    sys_task_t *task = NULL;
    ret = create_task(scheduler, cpu_thread->info.index, &tid, idle_task, NULL, "idle", SYS_TASK_TYPE_IDLE, 0, SYS_DEFAULT_TASK_STACK_SIZE, &task);
    if (ret < 0)
        return ret;
    ret = create_task(scheduler, cpu_thread->info.index, &tid, softirq_task, cpu_thread, "softirq", SYS_TASK_TYPE_FIFO, 0, SYS_DEFAULT_TASK_STACK_SIZE, &task);
    if (ret < 0)
        return ret;
    cpu_thread->softirq_task = &task->task_control_block;
    return ret;
}

static int find_idle_cpu(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int m = 0;
    int n = (unsigned int)-1 >> 1;
    for (int i = 0; i < scheduler->cpu_thread_count; i++)
    {
        sys_cpu_thread_t *cpu = get_cpu_thread_by_index(scheduler, i);
        if (0 == cpu->vscheduler.task_count)
        {
            m = i;
            break;
        }
        else if (cpu->vscheduler.task_count < n)
        {
            m = i;
            n = cpu->vscheduler.task_count;
        }
    }
    return m;
}

static void do_rt_task_load_leveling(sys_task_scheduler_t *scheduler, sys_task_t *task, int *cpu)
{
    sys_trace();
    if (task->rt_task_node.left != &g_leaf_node)
    {
        do_rt_task_load_leveling(scheduler, sys_container_of(task->rt_task_node.left, sys_task_t, rt_task_node), cpu);
    }
    task->task_control_block.desired_cpu = get_cpu_thread_by_index(scheduler, *cpu);
    (*cpu)++;
    if (*cpu >= scheduler->cpu_thread_count)
    {
        *cpu = 0;
    }
    if (task->rt_task_node.right != &g_leaf_node)
    {
        do_rt_task_load_leveling(scheduler, sys_container_of(task->rt_task_node.right, sys_task_t, rt_task_node), cpu);
    }
}

static void rt_task_load_leveling(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    if (scheduler->cpu_thread_count > 1)
    {
        int cpu = 0;
        do_rt_task_load_leveling(scheduler, sys_container_of(scheduler->rt_task_tree, sys_task_t, rt_task_node), &cpu);
    }
}

int sys_task_scheduler_create_task(sys_task_scheduler_t *scheduler, sys_tid_t *tid, task_function_t task_function, void *arg, const char *name, sys_task_type_t task_type, int priority, int stack_size)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    int cpu = find_idle_cpu(scheduler);
    sys_task_t *task = NULL;
    ret = create_task(scheduler, cpu, tid, task_function, arg, name, task_type, priority, stack_size, &task);
    if (SYS_TASK_TYPE_RT == task_type)
    {
        rt_task_load_leveling(scheduler);
    }
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_set_priority(sys_task_scheduler_t *scheduler, sys_tid_t tid, int priority)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    ret = sys_cpu_thread_set_priority(task->task_control_block.cpu_thread, &task->task_control_block, priority);
    if (SYS_TASK_TYPE_RT == task->task_control_block.vtask_control_block.scheduler_id)
    {
        rt_task_load_leveling(scheduler);
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

sys_task_t *sys_task_scheduler_sleep_inner(sys_task_scheduler_t *scheduler, uint64_t ns, void (*wakeup_callback)(struct sys_task_control_block_t *task, void *arg), void *arg)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    int index = sys_get_cpu();
    sys_cpu_thread_t *cpu = get_cpu_thread_by_index(scheduler, index);
    sys_task_control_block_t *task_control_block = sys_cpu_thread_sleep(cpu, ns, wakeup_callback, arg);
    sys_task_t *task = sys_container_of(task_control_block, sys_task_t, task_control_block);
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return task;
}

void sys_task_scheduler_sleep(sys_task_scheduler_t *scheduler, uint64_t ns)
{
    sys_trace();
    sys_task_scheduler_sleep_inner(scheduler, ns, NULL, NULL);
}

int sys_task_scheduler_wakeup_inner(sys_task_scheduler_t *scheduler, sys_task_t *task)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    int index = sys_get_cpu();
    sys_cpu_thread_t *cpu = get_cpu_thread_by_index(scheduler, index);
    int ret = sys_cpu_thread_wakeup(cpu, &task->task_control_block);
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_wakeup(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    int index = sys_get_cpu();
    sys_cpu_thread_t *cpu = get_cpu_thread_by_index(scheduler, index);
    ret = sys_cpu_thread_wakeup(cpu, &task->task_control_block);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

sys_task_t *sys_task_scheduler_suspend_inner(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    sys_task_t *task = sys_task_scheduler_get_running_task_inner(scheduler);
    sys_cpu_thread_suspend(task->task_control_block.cpu_thread, &task->task_control_block);
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return task;
}

int sys_task_scheduler_suspend(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task || task->task_control_block.vtask_control_block.task_state != SYS_TASK_STATE_READY)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_cpu_thread_suspend(task->task_control_block.cpu_thread, &task->task_control_block);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_resume_inner(sys_task_scheduler_t *scheduler, sys_task_t *task)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (NULL == task || task->task_control_block.vtask_control_block.task_state != SYS_TASK_STATE_SUSPEND)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_cpu_thread_resume(task->task_control_block.cpu_thread, &task->task_control_block);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_resume(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task || task->task_control_block.vtask_control_block.task_state != SYS_TASK_STATE_SUSPEND)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_cpu_thread_resume(task->task_control_block.cpu_thread, &task->task_control_block);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

void sys_task_scheduler_start(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    sys_cpu_thread_start((sys_cpu_thread_t *)scheduler->cpu_threads);
}

void sys_task_scheduler_exit(sys_task_scheduler_t *scheduler, void *arg)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    sys_task_t *task = sys_task_scheduler_get_running_task_inner(scheduler);
    sys_cpu_thread_suspend(task->task_control_block.cpu_thread, &task->task_control_block);
    task->task_control_block.vtask_control_block.task_state = SYS_TASK_STATE_DEAD;
    if (SYS_TASK_TYPE_RT == task->task_control_block.vtask_control_block.scheduler_id)
    {
        sys_delete_node(&scheduler->rt_task_tree, &task->rt_task_node);
        rt_task_load_leveling(scheduler);
    }
    move_children(scheduler, task);
    if (task->parent == scheduler->init_task)
    {
        sys_remove_from_list(&scheduler->init_task->children, &task->list_node);
        scheduler->init_task->children_count--;
        sys_insert_to_back(&scheduler->dead_task_list, &task->list_node);
        sys_cpu_thread_resume(scheduler->init_task->task_control_block.cpu_thread, &scheduler->init_task->task_control_block);
    }
    else
    {
        task->ret_arg = arg;
        if (task->parent->wait_tid == task->tid)
        {
            task->parent->task_control_block.vtask_control_block.task_state = SYS_TASK_STATE_SUSPEND;
            sys_cpu_thread_resume(task->parent->task_control_block.cpu_thread, &task->parent->task_control_block);
        }
    }
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
}

int sys_task_scheduler_join(sys_task_scheduler_t *scheduler, void **retval, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *running_task = sys_task_scheduler_get_running_task_inner(scheduler);
    if (task->parent != running_task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    if (SYS_TASK_STATE_DEAD == task->task_control_block.vtask_control_block.task_state)
    {
        *retval = task->ret_arg;
        sys_remove_from_list(&running_task->children, &task->list_node);
        running_task->children_count--;
        delete_task(scheduler, task);
    }
    else
    {
        running_task->wait_tid = tid;
        sys_cpu_thread_suspend(running_task->task_control_block.cpu_thread, &running_task->task_control_block);
        running_task->task_control_block.vtask_control_block.task_state = SYS_TASK_STATE_SLEEP;
        sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
        state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
        running_task->wait_tid = 0;
        *retval = task->ret_arg;
        sys_remove_from_list(&running_task->children, &task->list_node);
        running_task->children_count--;
        delete_task(scheduler, task);
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_detach(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    if (task->parent != scheduler->init_task && task->parent->wait_tid != task->tid)
    {
        sys_remove_from_list(&task->parent->children, &task->list_node);
        task->parent->children_count--;
        task->parent = scheduler->init_task;
        sys_insert_to_back(&scheduler->init_task->children, &task->list_node);
        scheduler->init_task->children_count++;
    }
    else
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

uint64_t sys_task_scheduler_get_clock(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    if (scheduler->cpu_threads != NULL)
    {
        struct sys_cpu_thread_t *cpu_thread = sys_container_of(scheduler->cpu_threads, sys_cpu_thread_t, node);
        return sys_cpu_thread_get_clock(cpu_thread);
    }
    return 0;
}

int sys_task_scheduler_get_task_count(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    ret = scheduler->task_count;
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

sys_tid_t sys_task_scheduler_get_tid(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    sys_task_t *task = sys_task_scheduler_get_running_task_inner(scheduler);
    sys_tid_t tid = task->tid;
    return tid;
}

int sys_task_scheduler_get_task_priority(sys_task_scheduler_t *scheduler, int *priority, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    switch (task->task_control_block.vtask_control_block.scheduler_id)
    {
    case SYS_TASK_TYPE_FIFO:
        *priority = task->task_control_block.real_task_control_block.fifo_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_RT:
        *priority = task->task_control_block.real_task_control_block.rt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_DT:
        *priority = task->task_control_block.real_task_control_block.dt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_IDLE:
        *priority = task->task_control_block.real_task_control_block.idle_task_control_block.priority;
        break;
    default:
        break;
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_task_type(sys_task_scheduler_t *scheduler, sys_task_type_t *type, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    *type = task->task_control_block.vtask_control_block.scheduler_id;
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_task_state(sys_task_scheduler_t *scheduler, sys_task_state_t *state, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int irq_state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    *state = task->task_control_block.vtask_control_block.task_state;
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, irq_state);
    return ret;
}

int sys_task_scheduler_get_task_name(sys_task_scheduler_t *scheduler, char *name, int size, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    size = size < SYS_TASK_MAX_NAME_LEN ? size : SYS_TASK_MAX_NAME_LEN;
    sys_strcpy(name, task->name, size);
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_task_stack_size(sys_task_scheduler_t *scheduler, int *stack_size, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    *stack_size = task->task_control_block.stack_size;
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_joinable(sys_task_scheduler_t *scheduler, sys_tid_t tid)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (tid <= 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    sys_task_t *task = get_task_by_tid(scheduler, tid);
    if (NULL == task)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    if (task->parent == scheduler->init_task)
    {
        ret = 0;
    }
    else
    {
        ret = 1;
    }
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_cpu_thread_count(sys_task_scheduler_t *scheduler)
{
    sys_trace();
    int ret;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    ret = scheduler->cpu_thread_count;
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_cpu_info(sys_task_scheduler_t *scheduler, sys_cpu_info_t *cpu_info, int *count)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    if (0 == scheduler->cpu_thread_count || 0 == *count)
    {
        *count = 0;
        ret = -1;
        goto exception;
    }
    int len = *count < scheduler->cpu_thread_count ? *count : scheduler->cpu_thread_count;
    int i = 0;
    sys_cpu_thread_t *cpu = (sys_cpu_thread_t *)scheduler->cpu_threads;
    for (; i < len; i++, cpu = (sys_cpu_thread_t *)cpu->node.next)
    {
        if (NULL == cpu)
        {
            break;
        }
        cpu_info[i] = cpu->info;
    }
    *count = i;
    ret = *count < scheduler->cpu_thread_count ? 1 : 0;
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

int sys_task_scheduler_get_cpu_usage(sys_task_scheduler_t *scheduler, int index)
{
    sys_trace();
    sys_cpu_thread_t *cpu_thread = get_cpu_thread_by_index(scheduler, index);
    if (NULL == cpu_thread)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    return sys_cpu_thread_get_cpu_usage(cpu_thread);
}

int sys_task_scheduler_find_first(sys_task_scheduler_t *scheduler, sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    *task_ptr = 0;
    return sys_task_scheduler_find_next(scheduler, task_ptr, task_info);
}

int sys_task_scheduler_find_next(sys_task_scheduler_t *scheduler, sys_task_ptr *task_ptr, sys_task_info_t *task_info)
{
    sys_trace();
    int ret = 0;
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    int count = sys_vector_size(&scheduler->task_list);
    sys_task_t *task = NULL;
    for (; *task_ptr < count; (*task_ptr)++)
    {
        task = get_task_by_tid(scheduler, *task_ptr);
        if (task != NULL)
        {
            break;
        }
    }
    if (*task_ptr >= count)
    {
        //sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    (*task_ptr)++;
    task_info->tid = task->tid;
    if (task->parent != NULL)
    {
        task_info->ptid = task->parent->tid;
    }
    else
    {
        task_info->ptid = 0;
    }
    sys_strcpy(task_info->name, task->name, SYS_TASK_MAX_NAME_LEN);
    task_info->stack = task->task_control_block.stack_start;
    task_info->stack_size = task->task_control_block.stack_size;
    task_info->task_state = task->task_control_block.vtask_control_block.task_state;
    task_info->task_type = task->task_control_block.vtask_control_block.scheduler_id;
    switch (task->task_control_block.vtask_control_block.scheduler_id)
    {
    case SYS_TASK_TYPE_FIFO:
        task_info->priority = task->task_control_block.real_task_control_block.fifo_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_RT:
        task_info->priority = task->task_control_block.real_task_control_block.rt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_DT:
        task_info->priority = task->task_control_block.real_task_control_block.dt_task_control_block.priority;
        break;
    case SYS_TASK_TYPE_IDLE:
        task_info->priority = task->task_control_block.real_task_control_block.idle_task_control_block.priority;
        break;
    default:
        break;
    }
    task_info->cpu = task->task_control_block.cpu_thread->info.index;
    goto finally;
exception:
finally:
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
    return ret;
}

void sys_task_scheduler_add_tasklet(sys_task_scheduler_t *scheduler, int cpu, sys_tasklet_t *tasklet)
{
    sys_trace();
    int state = sys_spin_lock_lock_and_irq_save(&scheduler->lock);
    sys_cpu_thread_t *cpu_thread = get_cpu_thread_by_index(scheduler, cpu);
    if (cpu_thread != NULL)
    {
        sys_cpu_thread_add_tasklet(cpu_thread, tasklet);
    }
    sys_spin_lock_unlock_and_irq_restore(&scheduler->lock, state);
}

void sys_task_scheduler_remove_tasklet(sys_task_scheduler_t *scheduler, int cpu, sys_tasklet_t *tasklet)
{
    sys_trace();
    sys_cpu_thread_t *cpu_thread = get_cpu_thread_by_index(scheduler, cpu);
    if (cpu_thread != NULL)
    {
        sys_cpu_thread_remove_tasklet(cpu_thread, tasklet);
    }
}

void sys_task_scheduler_ndelay(sys_task_scheduler_t *scheduler, unsigned int ns)
{
    sys_trace();
    if (scheduler->cpu_threads != NULL)
    {
        struct sys_cpu_thread_t *cpu_thread = sys_container_of(scheduler->cpu_threads, sys_cpu_thread_t, node);
        unsigned int end = sys_atomic_load(&cpu_thread->atomic_clock) + ns;
        while (end - (unsigned int)sys_atomic_load(&cpu_thread->atomic_clock) <= ns);
    }
}