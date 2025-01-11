#include "sys_vfs.h"
#include "vfs.h"
static struct vfs_t *s_vfs;
int sys_vfs_init(struct vfs_t *vfs)
{
    sys_trace();
    s_vfs = vfs;
    return vfs_init(s_vfs);
}

void sys_vfs_free()
{
    sys_trace();
    vfs_free(s_vfs);
    s_vfs = NULL;
}

int sys_registerfs(struct vfs_fs_t *fs)
{
    sys_trace();
    int ret = vfs_registerfs(s_vfs, fs);
    return ret;
}

int sys_mount(const char *path, const char *device)
{
    sys_trace();
    int ret = vfs_mount(s_vfs, path, device);
    return ret;
}

int sys_umount(const char *path)
{
    sys_trace();
    int ret = vfs_umount(s_vfs, path);
    return ret;
}

int sys_open(const char *path, int flags, int mode)
{
    sys_trace();
    int ret = vfs_open(s_vfs, path, flags, mode);
    return ret;
}

int sys_close(int fd)
{
    sys_trace();
    int ret = vfs_close(s_vfs, fd);
    return ret;
}

int sys_read(int fd, void *buff, int count)
{
    sys_trace();
    int ret = vfs_read(s_vfs, fd, buff, count);
    return ret;
}

int sys_write(int fd, const void *buff, int count)
{
    sys_trace();
    int ret = vfs_write(s_vfs, fd, buff, count);
    return ret;
}

int64_t sys_lseek(int fd, int64_t offset, int whence)
{
    sys_trace();
    int ret = vfs_lseek(s_vfs, fd, offset, whence);
    return ret;
}

int64_t sys_ftell(int fd)
{
    sys_trace();
    int ret = vfs_ftell(s_vfs, fd);
    return ret;
}

int sys_fstat(int fd, struct vfs_stat_t *stat)
{
    sys_trace();
    int ret = vfs_fstat(s_vfs, fd, stat);
    return ret;
}

int sys_syncfs(int fd)
{
    sys_trace();
    int ret = vfs_syncfs(s_vfs, fd);
    return ret;
}

int sys_ftruncate(int fd, int64_t length)
{
    sys_trace();
    int ret = vfs_ftruncate(s_vfs, fd, length);
    return ret;
}

int sys_link(const char *oldpath, const char *newpath)
{
    sys_trace();
    int ret = vfs_link(s_vfs, oldpath, newpath);
    return ret;
}

int sys_unlink(const char *path)
{
    sys_trace();
    int ret = vfs_unlink(s_vfs, path);
    return ret;
}

int sys_chmod(const char *path, int mode)
{
    sys_trace();
    int ret = vfs_chmod(s_vfs, path, mode);
    return ret;
}

int sys_rename(const char *oldpath, const char *newpath)
{
    sys_trace();
    int ret = vfs_rename(s_vfs, oldpath, newpath);
    return ret;
}

int sys_mkdir(const char *path, int mode)
{
    sys_trace();
    int ret = vfs_mkdir(s_vfs, path, mode);
    return ret;
}

int sys_rmdir(const char *path)
{
    sys_trace();
    int ret = vfs_rmdir(s_vfs, path);
    return ret;
}

int sys_opendir(const char *path)
{
    sys_trace();
    int ret = vfs_opendir(s_vfs, path);
    return ret;
}

int sys_closedir(int fd)
{
    sys_trace();
    int ret = vfs_closedir(s_vfs, fd);
    return ret;
}

int sys_readdir(int fd, struct vfs_dirent_t *dirent)
{
    sys_trace();
    int ret = vfs_readdir(s_vfs, fd, dirent);
    return ret;
}

int rewinddir(int fd)
{
    sys_trace();
    int ret = vfs_rewinddir(s_vfs, fd);
    return ret;
}