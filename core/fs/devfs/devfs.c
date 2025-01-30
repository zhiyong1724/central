#include "devfs.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "sys_error.h"
static int driver_insert_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    struct devfs_driver_t *driver1 = (struct devfs_driver_t *)key1;
    struct devfs_driver_t *driver2 = (struct devfs_driver_t *)key2;
    return sys_strcmp(driver1->name, driver2->name);
}

static int driver_find_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    struct devfs_driver_t *driver = (struct devfs_driver_t *)key1;
    return sys_strcmp((const char *)key2, driver->name);
}

static int device_insert_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    struct devfs_device_t *device1 = (struct devfs_device_t *)key1;
    struct devfs_device_t *device2 = (struct devfs_device_t *)key2;
    return sys_strcmp(device1->name, device2->name);
}

static int device_find_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    struct devfs_device_t *device = (struct devfs_device_t *)key1;
    return sys_strcmp((const char *)key2, device->name);
}

static struct devfs_device_t *find_device(struct devfs_t *devfs, const char *name)
{
    sys_trace();
    int i = 0;
    for (; name[i] == '/'; i++);
    struct devfs_device_t *device = (struct devfs_device_t *)sys_find_node((sys_tree_node_t *)devfs->devices, (void *)&name[i], device_find_compare, NULL);
    return device;
}

static struct devfs_driver_t *find_driver(struct devfs_t *devfs, const char *name)
{
    sys_trace();
    int i = 0;
    for (; name[i] == '/'; i++);
    struct devfs_driver_t *driver = (struct devfs_driver_t *)sys_find_node((sys_tree_node_t *)devfs->drivers, (void *)&name[i], driver_find_compare, NULL);
    return driver;
}

int devfs_init(struct devfs_t *devfs)
{
    sys_trace();
    devfs->drivers = NULL;
    devfs->devices = NULL;
    return 0;
}

void devfs_free(struct devfs_t *devfs)
{
    sys_trace();
    devfs->drivers = NULL;
    while (devfs->devices != NULL)
    {
        struct devfs_device_t *device = devfs->devices;
        sys_delete_node((sys_tree_node_t **)&devfs->drivers, &device->l);
        sys_free(device);
    }
}

int devfs_register_driver(struct devfs_t *devfs, struct devfs_driver_t *driver)
{
    sys_trace();
    sys_insert_node((sys_tree_node_t **)&devfs->drivers, &driver->l, driver_insert_compare, NULL);
    return 0;
}

int devfs_create_device(struct devfs_t *devfs, const char *driver_name, const char *name, void *data, int mode)
{
    sys_trace();
    int ret = 0;
    if (NULL == name || '\0' == name[0])
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
        goto exception;
    }
    struct devfs_device_t *device = find_device(devfs, name);
    if (device != NULL)
    {
        sys_info("File exists.");
        ret = SYS_ERROR_EXIST;
        goto exception;
    }
    device = sys_malloc(sizeof(struct devfs_device_t));
    if (NULL == device)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    sys_strcpy(device->driver_name, driver_name, DEVFS_MAX_DRIVER_NAME_LEN);
    sys_strcpy(device->name, name, DEVFS_MAX_DEVICE_NAME_LEN);
    device->data = data;
    device->mode = mode | VFS_MODE_ISCHR;
    device->ref_count = 0;
    sys_insert_node((sys_tree_node_t **)&devfs->devices, &device->l, device_insert_compare, NULL);
    goto finally;
exception:
finally:
    return ret;
}

int devfs_delete_device(struct devfs_t *devfs, const char *name)
{
    sys_trace();
    struct devfs_device_t *device = find_device(devfs, name);
    if (NULL == device)
    {
        sys_info("No such file or directory.");
        return SYS_ERROR_NOENT;
    }
    if (device->ref_count > 0)
    {
        sys_info("Too many open files.");
        return SYS_ERROR_MFILE;
    }
    sys_delete_node((sys_tree_node_t **)&devfs->devices, &device->l);
    sys_free(device);
    return 0;
}

int devfs_open(struct devfs_t *devfs, struct devfs_file_t *file, const char *name, int flags)
{
    sys_trace();
    struct devfs_device_t *device = find_device(devfs, name);
    if (NULL == device)
    {
        sys_info("No such file or directory.");
        return SYS_ERROR_NOENT;
    }
    struct devfs_driver_t *driver = find_driver(devfs, device->driver_name);
    if (NULL == driver)
    {
        sys_info("No such device.");
        return SYS_ERROR_NODEV;
    }
    file->driver = driver;
    device->ref_count++;
    file->device = device;
    file->flags = flags;
    file->offset = 0;
    if (NULL == file->driver->operations.open)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    return file->driver->operations.open(file);
}

int devfs_close(struct devfs_t *devfs, struct devfs_file_t *file)
{
    sys_trace();
    int ret = 0;
    if (NULL == file->driver->operations.close)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    ret = file->driver->operations.close(file);
    if (ret < 0)
    {
        return ret;
    }
    file->device->ref_count--;
    file->device = NULL;
    file->driver = NULL;
    return ret;
}

int devfs_read(struct devfs_t *devfs, struct devfs_file_t *file, void *buff, int count)
{
    sys_trace();
    if (((file->flags + 1) & (VFS_FLAG_RDONLY + 1)) == 0)
    {
        sys_info("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    if (NULL == file->driver->operations.read)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = file->driver->operations.read(file, buff, count, file->offset);
    if (ret < 0)
    {
        return ret;
    }
    file->offset += ret;
    return ret;
}

int devfs_write(struct devfs_t *devfs, struct devfs_file_t *file, const void *buff, int count)
{
    sys_trace();
    if (((file->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_info("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    if (NULL == file->driver->operations.write)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = file->driver->operations.write(file, buff, count, file->offset);
    if (ret < 0)
    {
        return ret;
    }
    file->offset += ret;
    return ret;
}

int64_t devfs_lseek(struct devfs_t *devfs, struct devfs_file_t *file,  int64_t offset, int whence)
{
    sys_trace();
    if (NULL == file->driver->operations.lseek)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int64_t ret = file->driver->operations.lseek(file, offset, whence);
    if (ret < 0)
    {
        return ret;
    }
    file->offset = ret;
    return ret;
}

int64_t devfs_ftell(struct devfs_t *devfs, struct devfs_file_t *file)
{
    sys_trace();
    return file->offset;
}

int devfs_iostl(struct devfs_t *devfs, struct devfs_file_t *file,  int cmd, int64_t arg)
{
    sys_trace();
    if (NULL == file->driver->operations.ioctl)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = file->driver->operations.ioctl(file, cmd, arg);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

int devfs_syncfs(struct devfs_t *devfs, struct devfs_file_t *file)
{
    sys_trace();
    if (NULL == file->driver->operations.syncfs)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    int ret = file->driver->operations.syncfs(file);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

int devfs_stat(struct devfs_t *devfs, const char *name, struct devfs_stat_t *stat)
{
    sys_trace();
    struct devfs_device_t *device = find_device(devfs, name);
    if (NULL == device)
    {
        sys_info("No such file or directory.");
        return SYS_ERROR_NOENT;
    }
    struct devfs_driver_t *driver = find_driver(devfs, device->driver_name);
    if (NULL == driver)
    {
        sys_info("No such device.");
        return SYS_ERROR_NODEV;
    }
    if (NULL == driver->operations.open)
    {
        sys_info("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    return driver->operations.stat(device, stat);
}