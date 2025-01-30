#include "sys_vfs.h"
#include "vfs.h"
static struct vfs_t *s_vfs;
int sys_vfs_init(struct vfs_t *vfs)
{
    sys_trace();
    s_vfs = vfs;
    sys_rwlock_create(&s_vfs->lock);
    return vfs_init(s_vfs);
}

void sys_vfs_free()
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    vfs_free(s_vfs);
    sys_rwlock_wrunlock(&s_vfs->lock);
    sys_rwlock_destory(&s_vfs->lock);
    s_vfs = NULL;
}

int sys_registerfs(struct vfs_fs_t *fs)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_registerfs(s_vfs, fs);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_unregisterfs(struct vfs_fs_t *fs)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_unregisterfs(s_vfs, fs);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_mount(const char *path, const char *device)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_mount(s_vfs, path, device);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_umount(const char *path)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_umount(s_vfs, path);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_open(const char *path, int flags, int mode)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_open(s_vfs, path, flags, mode);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_close(int fd)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_close(s_vfs, fd);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_read(int fd, void *buff, int count)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_read(s_vfs, fd, buff, count);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_write(int fd, const void *buff, int count)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_write(s_vfs, fd, buff, count);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int64_t sys_lseek(int fd, int64_t offset, int whence)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_lseek(s_vfs, fd, offset, whence);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int64_t sys_ftell(int fd)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_ftell(s_vfs, fd);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_syncfs(int fd)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_syncfs(s_vfs, fd);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_ftruncate(int fd, int64_t length)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_ftruncate(s_vfs, fd, length);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_stat(const char *path, struct vfs_stat_t *stat)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_stat(s_vfs, path, stat);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_link(const char *oldpath, const char *newpath)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_link(s_vfs, oldpath, newpath);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_unlink(const char *path)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_unlink(s_vfs, path);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_chmod(const char *path, int mode)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_chmod(s_vfs, path, mode);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_rename(const char *oldpath, const char *newpath)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_rename(s_vfs, oldpath, newpath);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_mkdir(const char *path, int mode)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_mkdir(s_vfs, path, mode);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_rmdir(const char *path)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_rmdir(s_vfs, path);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_opendir(const char *path)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_opendir(s_vfs, path);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_closedir(int fd)
{
    sys_trace();
    sys_rwlock_wrlock(&s_vfs->lock);
    int ret = vfs_closedir(s_vfs, fd);
    sys_rwlock_wrunlock(&s_vfs->lock);
    return ret;
}

int sys_readdir(int fd, struct vfs_dirent_t *dirent)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_readdir(s_vfs, fd, dirent);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_rewinddir(int fd)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_rewinddir(s_vfs, fd);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_statfs(const char *path, struct vfs_statfs_t *statfs)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_statfs(s_vfs, path, statfs);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

int sys_iostl(int fd,  int cmd, int64_t arg)
{
    sys_trace();
    sys_rwlock_rdlock(&s_vfs->lock);
    int ret = vfs_iostl(s_vfs, fd, cmd, arg);
    sys_rwlock_rdunlock(&s_vfs->lock);
    return ret;
}

static void do_dump_fs(struct vfs_dentry_t *dentry)
{
    sys_trace();
    if (dentry != NULL && dentry->super_block != NULL)
    {
        struct vfs_statfs_t statfs;
        sys_mutex_lock(&dentry->super_block->lock);
        dentry->super_block->fs_operations.statfs(dentry->super_block, "/", &statfs);
        sys_mutex_unlock(&dentry->super_block->lock);
        printf("%-24s", dentry->super_block->fs->name);
        printf("%-20ld", (statfs.f_blocks - statfs.f_bfree) * statfs.f_bsize);
        printf("%-20ld", statfs.f_bfree * statfs.f_bsize);
        printf("%ld%-12s", statfs.f_blocks > 0 ? (statfs.f_blocks - statfs.f_bfree) / statfs.f_blocks : 0, "%");
        printf("%s/", dentry->dir);
        printf("%s\n", dentry->name);
    }
}

static void traverse_fs(struct vfs_dentry_t *dentry)
{
    sys_trace();
    if (dentry != NULL)
    {   
        if (dentry->l.left != &g_leaf_node)
        {
            traverse_fs((struct vfs_dentry_t *)dentry->l.left);
        }
        if (dentry->l.right != &g_leaf_node)
        {
            traverse_fs((struct vfs_dentry_t *)dentry->l.right);
        }
        do_dump_fs(dentry);
        if (dentry->children != NULL)
        {
            traverse_fs(dentry->children);
        }
    }
}

void dump_fs()
{
    printf("文件系统                已用                可用                已用%%        挂载点\n");
    sys_rwlock_rdlock(&s_vfs->lock);
    do_dump_fs(s_vfs->root);
    traverse_fs(s_vfs->root->children);
    sys_rwlock_rdunlock(&s_vfs->lock);
}