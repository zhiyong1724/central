#include "central.h"
#include "sys_mem.h"
#include "sys_task_scheduler.h"
#include "sys_error.h"
#include "sys_cfg.h"
#include "vfs.h"
#include "devfs.h"
static sys_task_scheduler_t s_task_scheduler;
static struct vfs_t s_vfs;
static struct devfs_t s_devfs;
long sys_mem_init(void *start_address, long size);
int sys_task_init(sys_task_scheduler_t *scheduler);
int sys_vfs_init(struct vfs_t *vfs);
int sys_devfs_init(struct devfs_t *devfs);
void register_devfs();
void unregister_devfs();
int sys_init()
{
    sys_trace();
    if (sys_mem_init(SYS_HEAP_ADDRESS, SYS_HEAP_SIZE) <= 0)
    {
        sys_error("Initialize mem manager fail.");
        return SYS_ERROR_NOMEM;
    }
    int ret = sys_task_init(&s_task_scheduler);
    if (ret < 0)
    {
        sys_error("Initialize task manager fail.");
        return ret;
    }
    ret = sys_vfs_init(&s_vfs);
    if (ret < 0)
    {
        sys_error("Initialize vfs fail.");
        return ret;
    }
    ret = sys_devfs_init(&s_devfs);
    if (ret < 0)
    {
        sys_error("Initialize devfs fail.");
        return ret;
    }
    register_devfs();
    sys_mount("/dev", "");
    unregister_devfs();
    return 0;
}

const char *sys_version()
{
    sys_trace();
    return SYS_VERSION;
}