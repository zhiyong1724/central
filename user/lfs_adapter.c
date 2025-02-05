#include "lfs_adapter.h"
#include "lfs.h"
#include <string.h>
#include "sys_mem.h"
#include "sys_error.h"
#include "sys_devfs.h"
struct lfs_handle_t
{
    lfs_t lfs;
    struct lfs_config config;
    char device[DEVFS_MAX_DEVICE_NAME_LEN];
    int64_t page_size;
    int64_t block_size;
    int64_t block_count;
    void *read_buffer;
    void *prog_buffer;
    void *lookahead_buffer;
};

static int parse_result(int result)
{
    int ret = 0;
    switch (result)
    {
    case LFS_ERR_OK:
        ret = SYS_ERROR_OK;
        break;
    case LFS_ERR_IO:
        sys_info("I/O error..");
        ret = SYS_ERROR_IO;
        break;
    case LFS_ERR_CORRUPT:
        sys_info("Illegal byte sequence.");
        ret = SYS_ERROR_ILSEQ;
        break;
    case LFS_ERR_NOENT:
        sys_info("No such file or directory.");
        ret = SYS_ERROR_NOENT;
        break;
    case LFS_ERR_EXIST:
        sys_info("File exists.");
        ret = SYS_ERROR_EXIST;
        break;
    case LFS_ERR_NOTDIR:
        sys_info("Not a directory.");
        ret = SYS_ERROR_NOTDIR;
        break;
    case LFS_ERR_ISDIR:
        sys_info("Is a directory.");
        ret = SYS_ERROR_ISDIR;
        break;
    case LFS_ERR_NOTEMPTY:
        sys_info("Directory not empty.");
        ret = SYS_ERROR_NOTEMPTY;
        break;
    case LFS_ERR_FBIG:
        sys_info("File too large.");
        ret = SYS_ERROR_MFBIG;
        break;
    case LFS_ERR_INVAL:
        sys_info("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        break;
    case LFS_ERR_NOSPC:
        sys_info("No space left on device.");
        ret = SYS_ERROR_NOSPC;
        break;
    case LFS_ERR_NOMEM:
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        break;
    case LFS_ERR_NOATTR:
        sys_info("No data available.");
        ret = SYS_ERROR_NODATA;
        break;
    case LFS_ERR_NAMETOOLONG:
        sys_info("File name too long.");
        ret = SYS_ERROR_NAMETOOLONG;
        break;
    }
    return ret;
}

static int vfs_flags_to_lfs_flags(int vfs_flags)
{
    int flags = 0;
    if (((vfs_flags + 1) & (VFS_FLAG_RDONLY + 1)) == VFS_FLAG_RDONLY + 1)
    {
        flags |= LFS_O_RDONLY;
    }
    else if (((vfs_flags + 1) & (VFS_FLAG_WRONLY + 1)) == VFS_FLAG_WRONLY + 1)
    {
        flags |= LFS_O_WRONLY;
    }
    else if (((vfs_flags + 1) & (VFS_FLAG_RDWR + 1)) == VFS_FLAG_RDWR + 1)
    {
        flags |= LFS_O_RDWR;
    }
    if ((vfs_flags & VFS_FLAG_CREAT) == VFS_FLAG_CREAT)
    {
        flags |= LFS_O_CREAT;
    }
    if ((vfs_flags & VFS_FLAG_EXCL) == VFS_FLAG_EXCL)
    {
        flags |= LFS_O_EXCL;
    }
    if ((vfs_flags & VFS_FLAG_TRUNC) == VFS_FLAG_TRUNC)
    {
        flags |= LFS_O_TRUNC;
    }
    if ((vfs_flags & VFS_FLAG_APPEND) == VFS_FLAG_APPEND)
    {
        flags |= LFS_O_APPEND;
    }
    return flags;
}

static int lfs_open(struct vfs_file_t *file, const char *path, int mode)
{
    int ret = 0;
    static struct lfs_file_config cfg;
    memset(&cfg, 0, sizeof(struct lfs_file_config));
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    file->obj = sys_malloc(sizeof(lfs_file_t));
    if (NULL == file->obj)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    cfg.buffer = sys_malloc(handle->lfs.cfg->cache_size);
    if (NULL == cfg.buffer)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    int flags = vfs_flags_to_lfs_flags(file->flags);
    ret = lfs_file_opencfg(&handle->lfs, (lfs_file_t *)file->obj, path, flags, &cfg);
    if (ret < 0)
    {
        ret = parse_result(ret);
        goto exception;
    }
    goto finally;
exception:
if (file->obj != NULL)
{
    sys_free(file->obj);
}
if (cfg.buffer != NULL)
{
    sys_free(cfg.buffer);
}
finally:
    return ret;
}

static int lfs_close(struct vfs_file_t *file)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = lfs_file_close(&handle->lfs, (lfs_file_t *)file->obj);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    sys_free(((lfs_file_t *)file->obj)->cache.buffer);
    sys_free(file->obj);
    file->obj = NULL;
    return ret;
}

static int lfs_read(struct vfs_file_t *file, void *buff, int count)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = (int)lfs_file_read(&handle->lfs, (lfs_file_t *)file->obj, buff, count);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_write(struct vfs_file_t *file, const void *buff, int count)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = (int)lfs_file_write(&handle->lfs, (lfs_file_t *)file->obj, buff, count);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int64_t lfs_lseek(struct vfs_file_t *file, int64_t offset, int whence)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int flag = 0;
    switch (whence)
    {
    case VFS_SEEK_SET:
        flag = LFS_SEEK_SET;
        break;
    case VFS_SEEK_CUR:
        flag = LFS_SEEK_CUR;
        break;
    case VFS_SEEK_END:
        flag = LFS_SEEK_END;
        break;
    default:
        break;
    }
    int64_t ret = (int)lfs_file_seek(&handle->lfs, (lfs_file_t *)file->obj, offset, flag);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int64_t lfs_ftell(struct vfs_file_t *file)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int64_t ret = lfs_file_tell(&handle->lfs, (lfs_file_t *)file->obj);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_mode_to_vfs_mode(int lfs_mode)
{
    int mode = 0;
    if ((lfs_mode & LFS_TYPE_REG) == LFS_TYPE_REG)
    {
        mode |= VFS_MODE_ISREG;
    }
    else if ((lfs_mode & LFS_TYPE_DIR) == LFS_TYPE_DIR)
    {
        mode |= VFS_MODE_ISDIR;
    }
    return mode;
}

static int _lfs_stat(struct vfs_super_block_t *super_block, const char *path, struct vfs_stat_t *stat)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    struct lfs_info info;
    int ret = lfs_stat(&handle->lfs, path, &info);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    stat->st_mode = lfs_mode_to_vfs_mode(info.type);
    stat->st_size = info.size;
    return ret;
}

static int lfs_syncfs(struct vfs_file_t *file)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = lfs_file_sync(&handle->lfs, (lfs_file_t *)file->obj);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_ftruncate(struct vfs_file_t *file, int64_t length)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = lfs_file_truncate(&handle->lfs, (lfs_file_t *)file->obj, length);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_ioctl(struct vfs_file_t *file, int cmd, int64_t arg)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int lfs_link(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int lfs_unlink(struct vfs_super_block_t *super_block, const char *path)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    int ret = lfs_remove(&handle->lfs, path);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_chmod(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int _lfs_rename(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    int ret = lfs_rename(&handle->lfs, oldpath, newpath);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int _lfs_mkdir(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    int ret = lfs_mkdir(&handle->lfs, path);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_rmdir(struct vfs_super_block_t *super_block, const char *path)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    int ret = lfs_remove(&handle->lfs, path);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int lfs_opendir(struct vfs_file_t *file, const char *path)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = 0;
    file->obj = sys_malloc(sizeof(lfs_dir_t));
    if (NULL == file->obj)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    ret = lfs_dir_open(&handle->lfs, (lfs_dir_t *)file->obj, path);
    if (ret < 0)
    {
        ret = parse_result(ret);
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

static int lfs_closedir(struct vfs_file_t *file)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = lfs_dir_close(&handle->lfs, (lfs_dir_t *)file->obj);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    sys_free(file->obj);
    return ret;
}

static int lfs_readdir(struct vfs_file_t *file, struct vfs_dirent_t *dirent)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    struct lfs_info info;
    int ret = lfs_dir_read(&handle->lfs, (lfs_dir_t *)file->obj, &info);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    memset(dirent, 0, sizeof(struct vfs_dirent_t));
    switch (info.type)
    {
    case LFS_TYPE_REG:
        dirent->d_type |= VFS_MODE_ISREG;
        break;
    case LFS_TYPE_DIR:
        dirent->d_type |= VFS_MODE_ISDIR;
        break;
    default:
        break;
    }
    strcpy(dirent->d_name, info.name);
    return ret;
}

static int lfs_rewinddir(struct vfs_file_t *file)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)file->super_block->obj;
    int ret = lfs_dir_rewind(&handle->lfs, (lfs_dir_t *)file->obj);
    if (ret < 0)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int dev_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)c->context;
    int fd = sys_open(handle->device, VFS_FLAG_RDONLY, 0);
    if (fd < 0)
    {
        return LFS_ERR_IO;
    }
    sys_lseek(fd, block * handle->block_size + off, VFS_SEEK_SET);
    int ret = sys_read(fd, &((char *)buffer)[off], size);
    if (ret < 0)
        ret = LFS_ERR_IO;
    else
        ret = 0;
    sys_close(fd);
    return ret;
}

static int dev_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)c->context;
    int fd = sys_open(handle->device, VFS_FLAG_WRONLY, 0);
    if (fd < 0)
    {
        return LFS_ERR_IO;
    }
    sys_lseek(fd, block * handle->block_size + off, VFS_SEEK_SET);
    int ret = sys_write(fd, &((char *)buffer)[off], size);
    if (ret < 0)
        ret = LFS_ERR_IO;
    else
        ret = 0;
    sys_close(fd);
    return ret;
}

static int dev_erase(const struct lfs_config *c, lfs_block_t block)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)c->context;
    int fd = sys_open(handle->device, VFS_FLAG_WRONLY, 0);
    if (fd < 0)
    {
        return LFS_ERR_IO;
    }
    int ret = sys_iostl(fd, LFS_ERASE, block);
    if (ret < 0)
    {
        ret = LFS_ERR_IO;
    }
    sys_close(fd);
    return ret;
}

static int dev_sync(const struct lfs_config *c)
{
    struct lfs_handle_t *handle = (struct lfs_handle_t *)c->context;
    int fd = sys_open(handle->device, VFS_FLAG_WRONLY, 0);
    if (fd < 0)
    {
        return LFS_ERR_IO;
    }
    int ret = sys_syncfs(fd);
    if (ret < 0)
    {
        ret = LFS_ERR_IO;
    }
    sys_close(fd);
    return ret;
}

static int _lfs_mount(struct vfs_super_block_t *super_block, const char *device)
{
    int ret = 0;
    int fd = -1;
    struct lfs_handle_t *handle = sys_malloc(sizeof(struct lfs_handle_t));
    if (NULL == handle)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    super_block->obj = handle;
    memset(handle, 0, sizeof(struct lfs_handle_t));
    fd = sys_open(device, VFS_FLAG_RDONLY, 0);
    if (fd < 0)
    {
        ret = fd;
        goto exception;
    }
    ret = sys_iostl(fd, LFS_GET_PAGE_SIZE, (int64_t)&handle->page_size);
    if (NULL == super_block->obj)
    {
        goto exception;
    }
    ret = sys_iostl(fd, LFS_GET_BLOCK_SIZE, (int64_t)&handle->block_size);
    if (NULL == super_block->obj)
    {
        goto exception;
    }
    ret = sys_iostl(fd, LFS_GET_BLOCK_COUNT, (int64_t)&handle->block_count);
    if (NULL == super_block->obj)
    {
        goto exception;
    }
    handle->read_buffer = sys_malloc(handle->block_size);
    if (NULL == handle->read_buffer)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    handle->prog_buffer = sys_malloc(handle->block_size);
    if (NULL == handle->prog_buffer)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    handle->lookahead_buffer = sys_malloc(handle->block_count / 8);
    if (NULL == handle->lookahead_buffer)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    const struct lfs_config config =
        {
            .context = handle,
            .read = dev_read,
            .prog = dev_prog,
            .erase = dev_erase,
            .sync = dev_sync,
            .read_size = handle->page_size,
            .prog_size = handle->page_size,
            .block_size = handle->block_size,
            .block_count = handle->block_count,
            .block_cycles = 100,
            .cache_size = handle->block_size,
            .lookahead_size = handle->block_count / 8,
            .read_buffer = handle->read_buffer,
            .prog_buffer = handle->prog_buffer,
            .lookahead_buffer = handle->lookahead_buffer,
            .name_max = 0,
            .file_max = 0,
            .attr_max = 0,
            .metadata_max = 0,
        };
    handle->config = config;
    strcpy(handle->device, device);
    ret = lfs_mount(&handle->lfs, &handle->config);
    if (ret < 0)
    {
        ret = parse_result(ret);
        goto exception;
    }
    goto finally;
exception:
    if (handle != NULL && handle->read_buffer != NULL)
    {
        sys_free(handle->read_buffer);
    }
    if (handle != NULL && handle->prog_buffer != NULL)
    {
        sys_free(handle->prog_buffer);
    }
    if (handle != NULL && handle->lookahead_buffer != NULL)
    {
        sys_free(handle->lookahead_buffer);
    }
    if (handle != NULL)
    {
        sys_free(handle);
    }
finally:
    if (fd >= 0)
    {
        sys_close(fd);
    }
    return ret;
}

static int _lfs_unmount(struct vfs_super_block_t *super_block)
{
    int ret = 0;
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    ret = lfs_unmount(&handle->lfs);
    if (ret < 0)
    {
        ret = parse_result(ret);
        goto exception;
    }
    sys_free(handle->read_buffer);
    sys_free(handle->prog_buffer);
    sys_free(handle->lookahead_buffer);
    sys_free(handle);
    goto finally;
exception:
finally:
    return ret;
}

static int lfs_statfs(struct vfs_super_block_t *super_block, const char *path, struct vfs_statfs_t *statfs)
{
    int ret = 0;
    struct lfs_handle_t *handle = (struct lfs_handle_t *)super_block->obj;
    struct lfs_fsinfo fsinfo;
    ret = lfs_fs_stat(&handle->lfs, &fsinfo);
    if (ret < 0)
    {
        ret = parse_result(ret);
        goto exception;
    }
    memset(statfs, 0, sizeof(struct vfs_statfs_t));
    statfs->f_bsize = fsinfo.block_size;
    statfs->f_blocks = fsinfo.block_count;
    statfs->f_bfree = statfs->f_blocks - lfs_fs_size(&handle->lfs);
    statfs->f_bavail = statfs->f_bfree;
    statfs->f_namelen = LFS_NAME_MAX;
    goto finally;
exception:
finally:
    return ret;
}

static void init_super_block(struct vfs_super_block_t *super_block, const char *device)
{
    super_block->fs_operations.mount = _lfs_mount;
    super_block->fs_operations.unmount = _lfs_unmount;
    super_block->fs_operations.statfs = lfs_statfs;

    super_block->node.node_operations.link = lfs_link;
    super_block->node.node_operations.unlink = lfs_unlink;
    super_block->node.node_operations.chmod = lfs_chmod;
    super_block->node.node_operations.rename = _lfs_rename;
    super_block->node.node_operations.mkdir = _lfs_mkdir;
    super_block->node.node_operations.rmdir = lfs_rmdir;
    super_block->node.node_operations.opendir = lfs_opendir;
    super_block->node.node_operations.closedir = lfs_closedir;
    super_block->node.node_operations.readdir = lfs_readdir;
    super_block->node.node_operations.rewinddir = lfs_rewinddir;

    super_block->node.file_operations.open = lfs_open;
    super_block->node.file_operations.close = lfs_close;
    super_block->node.file_operations.read = lfs_read;
    super_block->node.file_operations.write = lfs_write;
    super_block->node.file_operations.lseek = lfs_lseek;
    super_block->node.file_operations.ftell = lfs_ftell;
    super_block->node.node_operations.stat = _lfs_stat;
    super_block->node.file_operations.syncfs = lfs_syncfs;
    super_block->node.file_operations.ftruncate = lfs_ftruncate;
    super_block->node.file_operations.ioctl = lfs_ioctl;

    super_block->block_size = 0;
    super_block->block_count = 0;
    super_block->obj = NULL;
    strcpy(super_block->device, device);
}

static void release(struct vfs_super_block_t *super_block)
{
}

int register_lfs()
{
    static struct vfs_fs_t s_fs =
    {
        .name = "littlefs",
        .init_super_block = init_super_block,
        .release = release,
        .flags = VFS_FLAG_RDWR
    };
    return sys_registerfs(&s_fs);
}