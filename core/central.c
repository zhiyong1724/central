#include "central.h"
#include "sys_mem.h"
#include "sys_task_manager.h"
#include "sys_semaphore_manager.h"
#include "sys_queue_manager.h"
#include "sys_error.h"
#include "sys_cfg.h"
#include "vfs.h"
static sys_task_manager_t s_task_manager;
static sys_semaphore_manager_t s_semaphore_manager;
static sys_queue_manager_t s_queue_manager;
static struct vfs_t s_vfs;
long sys_mem_init(void *start_address, long size);
int sys_task_init(sys_task_manager_t *task_manager);
int sys_semaphore_init(sys_semaphore_manager_t *semaphore_manager, sys_task_manager_t *task_manager);
int sys_msg_queue_init(sys_queue_manager_t *queue_manager, sys_task_manager_t *task_manager);
int sys_vfs_init(struct vfs_t *vfs);
int sys_init()
{
    sys_trace();
    if (sys_mem_init(SYS_HEAP_ADDRESS, SYS_HEAP_SIZE) <= 0)
    {
        sys_error("Initialize mem manager fail.");
        return SYS_ERROR_NOMEM;
    }
    int ret = sys_task_init(&s_task_manager);
    if (ret < 0)
    {
        sys_error("Initialize task manager fail.");
        return ret;
    }
    ret = sys_semaphore_init(&s_semaphore_manager, &s_task_manager);
    if (ret < 0)
    {
        sys_error("Initialize semaphore manager fail.");
        return ret;
    }
    ret = sys_msg_queue_init(&s_queue_manager, &s_task_manager);
    if (ret < 0)
    {
        sys_error("Initialize queue manager fail.");
        return ret;
    }
    ret = sys_vfs_init(&s_vfs);
    if (ret < 0)
    {
        sys_error("Initialize vfs fail.");
        return ret;
    }
    return 0;
}

const char *sys_version()
{
    sys_trace();
    return SYS_VERSION;
}