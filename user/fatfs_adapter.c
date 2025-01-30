#include "fatfs_adapter.h"
#include "ff.h"
#include <string.h>
#include "sys_mem.h"
#include "sys_error.h"
#include "sys_devfs.h"
#include "cregex.h"
struct fatfs_handle_t
{
    char device[DEVFS_MAX_DEVICE_NAME_LEN];
    FATFS fatfs;
};

static int parse_result(int result)
{
    int ret = 0;
    switch (result)
    {
    case FR_OK:
        ret = SYS_ERROR_OK;
        break;
    case FR_DISK_ERR:
        sys_info("I/O error..");
        ret = SYS_ERROR_IO;
        break;
    case FR_INT_ERR:
        sys_info("Assertion failed.");
        ret = SYS_ERROR_PERM;
        break;
    case FR_NOT_READY:
        sys_info("No such device or address.");
        ret = SYS_ERROR_NXIO;
        break;
    case FR_NO_FILE:
        sys_info("No such file or directory.");
        ret = SYS_ERROR_NOENT;
        break;
    case FR_NO_PATH:
        sys_info("Not a directory.");
        ret = SYS_ERROR_NOTDIR;
        break;
    case FR_INVALID_NAME:
        sys_info("File name too long.");
        ret = SYS_ERROR_NAMETOOLONG;
        break;
    case FR_DENIED:
        sys_info("Permission denied.");
        ret = SYS_ERROR_ACCES;
        break;
    case FR_EXIST:
        sys_info("File exists.");
        ret = SYS_ERROR_EXIST;
        break;
    case FR_INVALID_OBJECT:
        sys_info("Bad file number.");
        ret = SYS_ERROR_BADF;
        break;
    case FR_WRITE_PROTECTED:
        sys_info("Permission denied.");
        ret = SYS_ERROR_ACCES;
        break;
    case FR_INVALID_DRIVE:
        sys_info("No such device.");
        ret = SYS_ERROR_NODEV;
        break;
    case FR_NOT_ENABLED:
        sys_info("No space left on device.");
        ret = SYS_ERROR_NOSPC;
        break;
    case FR_NO_FILESYSTEM:
        sys_info("No data available.");
        ret = SYS_ERROR_NODATA;
        break;
    case FR_MKFS_ABORTED:
        sys_info("Illegal byte sequence.");
        ret = SYS_ERROR_ILSEQ;
        break;
    case FR_TIMEOUT:
        sys_info("Try again.");
        ret = SYS_ERROR_AGAIN;
        break;
    case FR_LOCKED:
        sys_info("Operation not permitted.");
        ret = SYS_ERROR_PERM;
        break;
    case FR_NOT_ENOUGH_CORE:
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        break;
    case FR_TOO_MANY_OPEN_FILES:
        sys_info("Too many open files.");
        ret = SYS_ERROR_MFILE;
        break;
    case FR_INVALID_PARAMETER:
        sys_info("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        break;
    }
    return ret;
}

static int vfs_flags_to_fatfs_flags(int vfs_flags)
{
    int flags = 0;
    if (((vfs_flags + 1) & (VFS_FLAG_RDONLY + 1)) == VFS_FLAG_RDONLY + 1)
    {
        flags |= FA_READ;
    }
    else if (((vfs_flags + 1) & (VFS_FLAG_WRONLY + 1)) == VFS_FLAG_WRONLY + 1)
    {
        flags |= FA_WRITE;
    }
    else if (((vfs_flags + 1) & (VFS_FLAG_RDWR + 1)) == VFS_FLAG_RDWR + 1)
    {
        flags |= FA_READ | FA_WRITE;
    }
    if ((vfs_flags & VFS_FLAG_CREAT) == VFS_FLAG_CREAT)
    {
        if ((vfs_flags & VFS_FLAG_EXCL) == VFS_FLAG_EXCL)
        {
            flags |= FA_OPEN_ALWAYS | FA_CREATE_NEW;
        }
        else
        {
            flags |= FA_OPEN_ALWAYS | FA_CREATE_ALWAYS;
        }
    }
    else
    {
        flags |= FA_OPEN_EXISTING;
    }
    if ((vfs_flags & VFS_FLAG_APPEND) == VFS_FLAG_APPEND)
    {
        flags |= FA_OPEN_APPEND;
    }
    return flags;
}

static int fatfs_open(struct vfs_file_t *file, const char *path, int mode)
{
    int ret = 0;
    file->obj = sys_malloc(sizeof(FIL));
    if (NULL == file->obj)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    int flags = vfs_flags_to_fatfs_flags(file->flags);
    ret = f_open((FIL *)file->obj, path, flags);
    if (ret != FR_OK)
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

static int fatfs_close(struct vfs_file_t *file)
{
    int ret = f_close((FIL *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    sys_free(file->obj);
    file->obj = NULL;
    return ret;
}

static int fatfs_read(struct vfs_file_t *file, void *buff, int count)
{
    UINT len = 0;
    int ret = f_read((FIL *)file->obj, buff, count, &len);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return len;
}

static int fatfs_write(struct vfs_file_t *file, const void *buff, int count)
{
    UINT len = 0;
    int ret = f_write((FIL *)file->obj, buff, count, &len);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return len;
}

static int64_t fatfs_lseek(struct vfs_file_t *file, int64_t offset, int whence)
{
    int64_t off = 0;
    switch (whence)
    {
    case VFS_SEEK_SET:
        off = offset;
        break;
    case VFS_SEEK_CUR:
        off = f_tell((FIL *)file->obj) + offset;
        break;
    case VFS_SEEK_END:
        off = f_size((FIL *)file->obj) + offset;
        break;
    default:
        break;
    }
    int64_t ret = f_lseek((FIL *)file->obj, off);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int64_t fatfs_ftell(struct vfs_file_t *file)
{
    int ret = f_tell((FIL *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_mode_to_vfs_mode(int fatfs_mode)
{
    int mode = 0;
    if ((fatfs_mode & AM_DIR) == AM_DIR)
    {
        mode |= VFS_MODE_ISDIR;
    }
    else
    {
        mode |= VFS_MODE_ISREG;
    }
    return mode;
}

static int fatfs_stat(struct vfs_super_block_t *super_block, const char *path, struct vfs_stat_t *stat)
{
    FILINFO info;
    int ret = f_stat(path, &info);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    stat->st_mode = fatfs_mode_to_vfs_mode(info.fattrib);
    stat->st_size = info.fsize;
    return ret;
}

static int fatfs_syncfs(struct vfs_file_t *file)
{
    int ret = f_sync((FIL *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_ftruncate(struct vfs_file_t *file, int64_t length)
{
    int ret = f_truncate((FIL *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_ioctl(struct vfs_file_t *file, int cmd, int64_t arg)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int fatfs_link(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int fatfs_unlink(struct vfs_super_block_t *super_block, const char *path)
{
    int ret = f_unlink(path);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_chmod(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    sys_info("The file system is nonsupport such operation.");
    return SYS_ERROR_PERM;
}

static int fatfs_rename(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath)
{
    int ret = f_rename(oldpath, newpath);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_mkdir(struct vfs_super_block_t *super_block, const char *path, int mode)
{
    int ret = f_mkdir(path);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_rmdir(struct vfs_super_block_t *super_block, const char *path)
{
    int ret = f_unlink(path);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_opendir(struct vfs_file_t *file, const char *path)
{
    int ret = 0;
    file->obj = sys_malloc(sizeof(DIR));
    if (NULL == file->obj)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    ret = f_opendir((DIR *)file->obj, path);
    if (ret != FR_OK)
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

static int fatfs_closedir(struct vfs_file_t *file)
{
    int ret = f_closedir((DIR *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    sys_free(file->obj);
    return ret;
}

static int fatfs_readdir(struct vfs_file_t *file, struct vfs_dirent_t *dirent)
{
    FILINFO info;
    int ret = f_readdir((DIR *)file->obj, &info);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    memset(dirent, 0, sizeof(struct vfs_dirent_t));
    dirent->d_type = fatfs_mode_to_vfs_mode(info.fattrib);
    if (info.fname[0] != '\0')
    {
        strcpy(dirent->d_name, info.fname);
        return 1;
    }
    return 0;
}

static int fatfs_rewinddir(struct vfs_file_t *file)
{
    int ret = f_rewinddir((DIR *)file->obj);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    return ret;
}

static int fatfs_mount(struct vfs_super_block_t *super_block, const char *device)
{
    int ret = 0;
    cregex_t *regex = NULL;
    struct fatfs_handle_t *handle = sys_malloc(sizeof(struct fatfs_handle_t));
    if (NULL == handle)
    {
        sys_info("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    super_block->obj = handle;
    memset(handle, 0, sizeof(struct fatfs_handle_t));
    strcpy(handle->device, device);
    regex = cregex_compile("\\d");
    cregex_match_t matchs;
    ret = cregex_search(regex, handle->device, &matchs, 1, 0);
    if (ret < 0)
    {
        goto exception;
    }
    ret = f_mount(&handle->fatfs, &device[matchs.begin], 1);
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        return ret;
    }
    goto finally;
exception:
    if (handle != NULL)
    {
        sys_free(handle);
    }
finally:
    if (regex != NULL)
    {
        cregex_free(regex);
    }
    return ret;
}

static int fatfs_unmount(struct vfs_super_block_t *super_block)
{
    struct fatfs_handle_t *handle = (struct fatfs_handle_t *)super_block->obj;
    sys_free(handle);
    super_block->obj = NULL;
    return 0;
}

static int fatfs_statfs(struct vfs_super_block_t *super_block, const char *path, struct vfs_statfs_t *statfs)
{
    struct fatfs_handle_t *handle = (struct fatfs_handle_t *)super_block->obj;
    cregex_t *regex = cregex_compile("\\d");
    cregex_match_t matchs;
    int ret = cregex_search(regex, handle->device, &matchs, 1, 0);
    if (ret < 0)
    {
        goto exception;
    }
    DWORD free;
    FATFS *fatfs;
    ret = f_getfree (&handle->device[matchs.begin], &free, &fatfs);	
    if (ret != FR_OK)
    {
        ret = parse_result(ret);
        goto exception;
    }
    memset(statfs, 0, sizeof(struct vfs_statfs_t));
    statfs->f_bsize = fatfs->ssize * fatfs->csize;
    statfs->f_blocks = fatfs->n_fatent;
    statfs->f_bfree = free * fatfs->csize;
    statfs->f_bavail = statfs->f_bfree;
    statfs->f_namelen = FF_MAX_LFN;
    goto finally;
exception:
finally:
    if (regex != NULL)
    {
        cregex_free(regex);
    }
    return ret;
}

static void init_super_block(struct vfs_super_block_t *super_block, const char *device)
{
    super_block->fs_operations.mount = fatfs_mount;
    super_block->fs_operations.unmount = fatfs_unmount;
    super_block->fs_operations.statfs = fatfs_statfs;

    super_block->node.node_operations.link = fatfs_link;
    super_block->node.node_operations.unlink = fatfs_unlink;
    super_block->node.node_operations.chmod = fatfs_chmod;
    super_block->node.node_operations.rename = fatfs_rename;
    super_block->node.node_operations.mkdir = fatfs_mkdir;
    super_block->node.node_operations.rmdir = fatfs_rmdir;
    super_block->node.node_operations.opendir = fatfs_opendir;
    super_block->node.node_operations.closedir = fatfs_closedir;
    super_block->node.node_operations.readdir = fatfs_readdir;
    super_block->node.node_operations.rewinddir = fatfs_rewinddir;

    super_block->node.file_operations.open = fatfs_open;
    super_block->node.file_operations.close = fatfs_close;
    super_block->node.file_operations.read = fatfs_read;
    super_block->node.file_operations.write = fatfs_write;
    super_block->node.file_operations.lseek = fatfs_lseek;
    super_block->node.file_operations.ftell = fatfs_ftell;
    super_block->node.node_operations.stat = fatfs_stat;
    super_block->node.file_operations.syncfs = fatfs_syncfs;
    super_block->node.file_operations.ftruncate = fatfs_ftruncate;
    super_block->node.file_operations.ioctl = fatfs_ioctl;

    super_block->block_size = 0;
    super_block->block_count = 0;
    super_block->obj = NULL;
    strcpy(super_block->device, device);
}

static void release(struct vfs_super_block_t *super_block)
{
}

int register_fatfs()
{
    static struct vfs_fs_t s_fs =
    {
        .name = "fatfs",
        .init_super_block = init_super_block,
        .release = release,
        .flags = VFS_FLAG_RDWR
    };
    return sys_registerfs(&s_fs);
}