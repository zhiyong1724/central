#include "sys_devfs.h"
#include "devfs.h"
#include "sys_string.h"
#include "sys_error.h"
#include "sys_vector.h"
#include "sys_mem.h"
struct devfs_dir_t
{
    sys_vector_t files;
    int index;
};

struct devfs_t *s_devfs;
static int fs_mount(struct vfs_super_block_t *super_block, const char *device)
{
    sys_trace();
    return 0;
}

static int fs_unmount(struct vfs_super_block_t *super_block)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_statfs(struct vfs_super_block_t *super_block, const char *path, struct vfs_statfs_t *statfs)
{
    sys_trace();
    sys_memset(statfs, 0, sizeof(struct vfs_statfs_t));
    return 0;
}

static int fs_stat(struct vfs_super_block_t *super_block, const char *path, struct vfs_stat_t *stat)
{
    sys_trace();
    struct devfs_stat_t devfs_stat;
    int ret = sys_devfs_stat(path, &devfs_stat);
    if (ret < 0)
    {
        return ret;
    }
    sys_memset(stat, 0, sizeof(struct vfs_stat_t));
    stat->st_mode = VFS_MODE_ISBLK | VFS_MODE_IRUSR | VFS_MODE_IWUSR;
    stat->st_size = devfs_stat.st_size;
    stat->st_blksize = devfs_stat.st_blksize;
    stat->st_blocks = devfs_stat.st_blocks;
    return ret;
}

static int fs_link(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_unlink(struct vfs_super_block_t *super_block, const char *path)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_chmod(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_rename(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_mkdir(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_rmdir(struct vfs_super_block_t *super_block, const char *path)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static void traverse_files(struct devfs_dir_t *dir, struct devfs_device_t *device)
{
    sys_trace();
    if (device != NULL)
    {
        if (device->l.left != &g_leaf_node)
        {
            traverse_files(dir, (struct devfs_device_t *)device->l.left);
        }
        if (device->l.right != &g_leaf_node)
        {
            traverse_files(dir, (struct devfs_device_t *)device->l.right);
        }
        sys_vector_push_back(&dir->files, &device);
    }
}

static int fs_opendir(struct vfs_file_t *file, const char *path)
{
    sys_trace();
    int ret = -1;
    struct devfs_dir_t *dir = NULL;
    int i = 0;
    for (; path[i] == '/'; i++);
    if (path[i] != '\0')
    {
        ret = SYS_ERROR_NOTDIR;
        goto exception;
    }
    dir = (struct devfs_dir_t *)sys_malloc(sizeof(struct devfs_dir_t));
    if (NULL == dir)
    {
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    ret = sys_vector_init(&dir->files, sizeof(void *));
    if (ret < 0)
    {
        goto exception;
    }
    traverse_files(dir, s_devfs->devices);
    dir->index = 0;
    file->obj = dir;
    goto finally;
exception:
if (0 == ret)
{
    sys_vector_free(&dir->files);
}
if (dir != NULL)
{
    sys_free(dir);
}
finally:
    return ret;
}

static int fs_closedir(struct vfs_file_t *file)
{
    sys_trace();
    struct devfs_dir_t *dir = (struct devfs_dir_t *)file->obj;
    if (dir != NULL)
    {
        sys_vector_free(&dir->files);
        sys_free(dir);
        file->obj = NULL;
    }
    return 0;
}

static int fs_readdir(struct vfs_file_t *file, struct vfs_dirent_t *dirent)
{
    sys_trace();
    struct devfs_dir_t *dir = (struct devfs_dir_t *)file->obj;
    if (dir->index < dir->files.size)
    {
        struct devfs_device_t **device = (struct devfs_device_t **)sys_vector_at(&dir->files, dir->index);
        sys_memset(dirent, 0, sizeof(struct vfs_dirent_t));
        sys_strcpy(dirent->d_name, (*device)->name, VFS_MAX_FILE_NAME_LEN);
        dirent->d_type = (*device)->mode;
        dir->index++;
        return 1;
    }
    return 0;
}

static int fs_rewinddir(struct vfs_file_t *file)
{
    sys_trace();
    struct devfs_dir_t *dir = (struct devfs_dir_t *)file->obj;
    dir->index = 0;
    return 0;
}

static int fs_open(struct vfs_file_t *file, const char *path, int mode)
{
    sys_trace();
    int ret = 0;
    file->obj = sys_malloc(sizeof(struct devfs_file_t));
    if (NULL == file->obj)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    ret = sys_devfs_open((struct devfs_file_t *)file->obj, path, file->flags);
    if (ret < 0)
    {
        goto exception;
    }
    goto finally;
exception:
if (file->obj != NULL)
{
    sys_free(file->obj);
}
finally:
    return ret;
}

static int fs_close(struct vfs_file_t *file)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_close(devfs_file);
    if (ret < 0)
    {
        return ret;
    }
    sys_free(file->obj);
    return ret;
}

static int fs_read(struct vfs_file_t *file, void *buff, int count)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_read(devfs_file, buff, count);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static int fs_write(struct vfs_file_t *file, const void *buff, int count)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_write(devfs_file, buff, count);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static int64_t fs_lseek(struct vfs_file_t *file, int64_t offset, int whence)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int64_t ret = sys_devfs_lseek(devfs_file, offset, whence);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static int64_t fs_ftell(struct vfs_file_t *file)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_ftell(devfs_file);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static int fs_syncfs(struct vfs_file_t *file)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_syncfs(devfs_file);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static int fs_ftruncate(struct vfs_file_t *file, int64_t length)
{
    sys_trace();
    return SYS_ERROR_PERM;
}

static int fs_ioctl(struct vfs_file_t *file, int cmd, int64_t arg)
{
    sys_trace();
    struct devfs_file_t *devfs_file = (struct devfs_file_t *)file->obj;
    int ret = sys_devfs_iostl(devfs_file, cmd, arg);
    if (ret < 0)
    {
        return ret;
    }
    return ret;
}

static void init_super_block(struct vfs_super_block_t *super_block, const char *device)
{
    sys_trace();
    super_block->fs_operations.mount = fs_mount;
    super_block->fs_operations.unmount = fs_unmount;
    super_block->fs_operations.statfs = fs_statfs;

    super_block->node.node_operations.link = fs_link;
    super_block->node.node_operations.unlink = fs_unlink;
    super_block->node.node_operations.chmod = fs_chmod;
    super_block->node.node_operations.rename = fs_rename;
    super_block->node.node_operations.mkdir = fs_mkdir;
    super_block->node.node_operations.rmdir = fs_rmdir;
    super_block->node.node_operations.opendir = fs_opendir;
    super_block->node.node_operations.closedir = fs_closedir;
    super_block->node.node_operations.readdir = fs_readdir;
    super_block->node.node_operations.rewinddir = fs_rewinddir;

    super_block->node.file_operations.open = fs_open;
    super_block->node.file_operations.close = fs_close;
    super_block->node.file_operations.read = fs_read;
    super_block->node.file_operations.write = fs_write;
    super_block->node.file_operations.lseek = fs_lseek;
    super_block->node.file_operations.ftell = fs_ftell;
    super_block->node.node_operations.stat = fs_stat;
    super_block->node.file_operations.syncfs = fs_syncfs;
    super_block->node.file_operations.ftruncate = fs_ftruncate;
    super_block->node.file_operations.ioctl = fs_ioctl;

    super_block->block_size = 0;
    super_block->block_count = 0;
    super_block->obj = NULL;
    super_block->device[0] = '\0';
}

static void release(struct vfs_super_block_t *super_block)
{
    sys_trace();
}

static struct vfs_fs_t s_vfs_fs =
    {
        .name = "devfs",
        .init_super_block = init_super_block,
        .release = release,
        .flags = VFS_FLAG_RDWR
    };

void register_devfs()
{
    sys_registerfs(&s_vfs_fs);
}

void unregister_devfs()
{
    sys_unregisterfs(&s_vfs_fs);
}

int sys_devfs_init(struct devfs_t *devfs)
{
    sys_trace();
    s_devfs = devfs;
    return devfs_init(s_devfs);
}

void sys_devfs_free()
{
    sys_trace();
    devfs_free(s_devfs);
    s_devfs = NULL;
}

int sys_devfs_register_driver(struct devfs_driver_t *driver)
{
    sys_trace();
    return devfs_register_driver(s_devfs, driver);
}

int sys_devfs_create_device(const char *driver_name, const char *name, void *data, int mode)
{
    sys_trace();
    return devfs_create_device(s_devfs, driver_name, name, data, mode);
}

int sys_devfs_delete_device(const char *name)
{
    sys_trace();
    return devfs_delete_device(s_devfs, name);
}

int sys_devfs_open(struct devfs_file_t *file, const char *name, int flags)
{
    sys_trace();
    return devfs_open(s_devfs, file, name, flags);
}

int sys_devfs_close(struct devfs_file_t *file)
{
    sys_trace();
    return devfs_close(s_devfs, file);
}

int sys_devfs_read(struct devfs_file_t *file, void *buff, int count)
{
    sys_trace();
    return devfs_read(s_devfs, file, buff, count);
}

int sys_devfs_write(struct devfs_file_t *file, const void *buff, int count)
{
    sys_trace();
    return devfs_write(s_devfs, file, buff, count);
}

int64_t sys_devfs_lseek(struct devfs_file_t *file,  int64_t offset, int whence)
{
    sys_trace();
    return devfs_lseek(s_devfs, file, offset, whence);
}

int64_t sys_devfs_ftell(struct devfs_file_t *file)
{
    sys_trace();
    return devfs_ftell(s_devfs, file);
}

int sys_devfs_iostl(struct devfs_file_t *file,  int cmd, int64_t arg)
{
    sys_trace();
    return devfs_iostl(s_devfs, file, cmd, arg);
}

int sys_devfs_syncfs(struct devfs_file_t *file)
{
    sys_trace();
    return devfs_syncfs(s_devfs, file);
}

int sys_devfs_stat(const char *name, struct devfs_stat_t *stat)
{
    sys_trace();
    return devfs_stat(s_devfs, name, stat);
}
