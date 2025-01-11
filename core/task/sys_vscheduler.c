#include "sys_vscheduler.h"
int sys_vscheduler_init(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    for (int i = 0; i < SYS_MAX_SCHEDULER_COUNT; i++)
    {
        vscheduler->schedulers[i] = NULL;
    }
    vscheduler->scheduler_count = 0;
    vscheduler->clock = 0;
    vscheduler->suspended_list = NULL;
    vscheduler->sleep_tree = NULL;
    vscheduler->running_task = NULL;
    vscheduler->min_sleep_task = NULL;
    return 0;
}

int sys_task_control_block_init(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    task_control_block->scheduler_id = 0;
    task_control_block->task_state = SYS_TASK_STATE_DELETED;
    task_control_block->sleep_time = 0;
    return 0;
}

int sys_vscheduler_add_scheduler(sys_vscheduler_t *vscheduler, void *scheduler, sys_scheduler_interfaces_t *scheduler_interfaces)
{
    sys_trace();
    if (vscheduler->scheduler_count < SYS_MAX_SCHEDULER_COUNT)
    {
        vscheduler->schedulers[vscheduler->scheduler_count] = scheduler;
        vscheduler->scheduler_interfaces[vscheduler->scheduler_count] = *scheduler_interfaces;
        vscheduler->scheduler_count++;
        return 0;
    }
    return -1;
}

int sys_vscheduler_add_task(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT)
    {
        if (NULL == vscheduler->running_task)
        {
            vscheduler->running_task = task_control_block;
        }
        task_control_block->task_state = SYS_TASK_STATE_READY;
        return vscheduler->scheduler_interfaces[task_control_block->scheduler_id].add_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
    }
    return -1;
}

int sys_vscheduler_modify_priority(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block, int priority)
{
    sys_trace();
    if (task_control_block->scheduler_id < SYS_MAX_SCHEDULER_COUNT)
    {
        return vscheduler->scheduler_interfaces[task_control_block->scheduler_id].modify_priority(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1, priority);
    }
    return -1;
}

static void sleep_tree_tick(sys_vscheduler_t *vscheduler, uint64_t *ns)
{
    //sys_trace();
    if (vscheduler->min_sleep_task != NULL)
    {
        for (;;)
        {
            if (vscheduler->min_sleep_task != NULL && vscheduler->min_sleep_task->sleep_time - vscheduler->clock <= *ns)
            {
                sys_delete_node(&vscheduler->sleep_tree, &vscheduler->min_sleep_task->node.tree_node);
                vscheduler->min_sleep_task->task_state = SYS_TASK_STATE_READY;
                vscheduler->scheduler_interfaces[vscheduler->min_sleep_task->scheduler_id].add_task(vscheduler->schedulers[vscheduler->min_sleep_task->scheduler_id], vscheduler->min_sleep_task + 1);
                vscheduler->min_sleep_task = (sys_task_control_block_t *)sys_get_left_most_node(vscheduler->sleep_tree);
            }
            else
            {
                break;
            }
        }
        vscheduler->clock += *ns;
        *ns = -1;
        if (vscheduler->min_sleep_task != NULL)
        {
            *ns = vscheduler->min_sleep_task->sleep_time - vscheduler->clock;
        }
    }
    else
    {
        *ns = -1;
    }
}

sys_task_control_block_t *sys_vscheduler_tick(sys_vscheduler_t *vscheduler, int scheduler_id, uint64_t *ns)
{
    //sys_trace();
    uint64_t next_sleep_tick_interval = *ns;
    sleep_tree_tick(vscheduler, &next_sleep_tick_interval);
    uint64_t next_tick_interval = *ns;
    if (scheduler_id < SYS_MAX_SCHEDULER_COUNT)
    {
        vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[scheduler_id].tick(vscheduler->schedulers[scheduler_id], &next_tick_interval);
    }
    else
    {
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].tick(vscheduler->schedulers[i], &next_tick_interval);
            if (vscheduler->running_task != NULL)
            {
                break;
            }
        }
    }
    *ns = next_sleep_tick_interval < next_tick_interval ? next_sleep_tick_interval : next_tick_interval;
    if (vscheduler->running_task != NULL)
    {
        vscheduler->running_task--;
    }
    return vscheduler->running_task;
}

sys_task_control_block_t *sys_vscheduler_supend(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    if (SYS_TASK_STATE_READY == task_control_block->task_state || SYS_TASK_STATE_SLEEP == task_control_block->task_state)
    {
        switch (task_control_block->task_state)
        {
        case SYS_TASK_STATE_READY:
            vscheduler->scheduler_interfaces[task_control_block->scheduler_id].remove_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
            break;
        case SYS_TASK_STATE_SLEEP:
            sys_delete_node(&vscheduler->sleep_tree, &task_control_block->node.tree_node);
            if (task_control_block == vscheduler->min_sleep_task)
            {
                vscheduler->min_sleep_task = (sys_task_control_block_t *)sys_get_left_most_node(vscheduler->sleep_tree);
            }
            break;
        default:
            break;
        }

        sys_insert_to_back(&vscheduler->suspended_list, &task_control_block->node.list_node);
        task_control_block->task_state = SYS_TASK_STATE_SUSPENDED;
        if (task_control_block == vscheduler->running_task)
        {
            for (int i = 0; i < vscheduler->scheduler_count; i++)
            {
                vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].get_running_task(vscheduler->schedulers[i]);
                if (vscheduler->running_task != NULL)
                {
                    vscheduler->running_task--;
                    break;
                }
            }
        }
    }
    return vscheduler->running_task;
}

sys_task_control_block_t *sys_vscheduler_resume(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    if (SYS_TASK_STATE_SUSPENDED == task_control_block->task_state)
    {
        sys_remove_from_list(&vscheduler->suspended_list, &task_control_block->node.list_node);
        vscheduler->scheduler_interfaces[task_control_block->scheduler_id].add_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
        task_control_block->task_state = SYS_TASK_STATE_READY;
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].yield(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

static int on_compare(void *key1, void *key2, void *arg)
{
	sys_trace();
    sys_vscheduler_t *task_manager = (sys_vscheduler_t *)arg;
    sys_task_control_block_t *task1 = (sys_task_control_block_t *)key1;
    sys_task_control_block_t *task2 = (sys_task_control_block_t *)key2;
    if (task1->sleep_time - task_manager->clock < task2->sleep_time - task_manager->clock)
    {
        return -1;
    }
    else
    {
        return 1;
    }
}

sys_task_control_block_t *sys_vscheduler_sleep(sys_vscheduler_t *vscheduler, uint64_t ns)
{
    sys_trace();
    if (vscheduler->running_task != NULL)
    {
        vscheduler->scheduler_interfaces[vscheduler->running_task->scheduler_id].remove_task(vscheduler->schedulers[vscheduler->running_task->scheduler_id], vscheduler->running_task + 1);
        vscheduler->running_task->sleep_time = vscheduler->clock + ns;
        sys_insert_node(&vscheduler->sleep_tree, &vscheduler->running_task->node.tree_node, on_compare, vscheduler);
        vscheduler->running_task->task_state = SYS_TASK_STATE_SLEEP;
        if (NULL == vscheduler->min_sleep_task || vscheduler->running_task->sleep_time - vscheduler->clock < vscheduler->min_sleep_task->sleep_time - vscheduler->clock)
        {
            vscheduler->min_sleep_task = vscheduler->running_task;
        }

        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].get_running_task(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

sys_task_control_block_t *sys_vscheduler_wakeup(sys_vscheduler_t *vscheduler, sys_task_control_block_t *task_control_block)
{
    sys_trace();
    if (SYS_TASK_STATE_SLEEP == task_control_block->task_state)
    {
        sys_delete_node(&vscheduler->sleep_tree, &task_control_block->node.tree_node);
        task_control_block->task_state = SYS_TASK_STATE_READY;
        vscheduler->scheduler_interfaces[task_control_block->scheduler_id].add_task(vscheduler->schedulers[task_control_block->scheduler_id], task_control_block + 1);
        vscheduler->min_sleep_task = (sys_task_control_block_t *)sys_get_left_most_node(vscheduler->sleep_tree);
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].yield(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

sys_task_control_block_t *sys_vscheduler_exit(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    if (vscheduler->running_task != NULL)
    {
        vscheduler->scheduler_interfaces[vscheduler->running_task->scheduler_id].remove_task(vscheduler->schedulers[vscheduler->running_task->scheduler_id], vscheduler->running_task + 1);
        vscheduler->running_task->task_state = SYS_TASK_STATE_DELETED;
        for (int i = 0; i < vscheduler->scheduler_count; i++)
        {
            vscheduler->running_task = (sys_task_control_block_t *)vscheduler->scheduler_interfaces[i].get_running_task(vscheduler->schedulers[i]);
            if (vscheduler->running_task != NULL)
            {
                vscheduler->running_task--;
                break;
            }
        }
    }
    return vscheduler->running_task;
}

sys_task_control_block_t *sys_vscheduler_get_running_task(sys_vscheduler_t *vscheduler)
{
    sys_trace();
    return vscheduler->running_task;
}