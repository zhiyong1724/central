#include "vfs.h"
#include "sys_mem.h"
#include "sys_string.h"
#include "cregex.h"
#include "sys_error.h"
static void delete_all_dentry(struct vfs_dentry_t *dentry)
{
    sys_trace();
    if (dentry != NULL)
    {
        while (dentry->children != NULL)
        {
            struct vfs_dentry_t *child = (struct vfs_dentry_t *)dentry->children;
            sys_delete_node((sys_tree_node_t **)&dentry->children, &child->l);
            delete_all_dentry(child);
        }
        dentry->super_block->ref_count--;
        if (0 == dentry->super_block->ref_count)
        {
            sys_free(dentry->super_block);
        }
        sys_free(dentry);
    }
}

static void delete_invalid_dentry(struct vfs_dentry_t *dentry)
{
    sys_trace();
    if (dentry != NULL)
    {
        if (NULL == dentry->children && (NULL == dentry->super_block || dentry->super_block->ref_count > 1))
        {
            if (dentry->super_block != NULL)
            {
                dentry->super_block->ref_count--;
            }
            if (dentry->parent != NULL)
            {
                sys_delete_node((sys_tree_node_t **)&dentry->parent->children, &dentry->l);
            }
            sys_free(dentry);
        }
    }
}

static int on_compare(void *key1, void *key2, void *arg)
{
    sys_trace();
    struct vfs_dentry_t *dentry = (struct vfs_dentry_t *)key1;
    return sys_strcmp(dentry->name, (const char *)key2);
}

static void init_dentry(struct vfs_dentry_t *dentry)
{
    dentry->parent = NULL;
    dentry->children = NULL;
    dentry->name[0] = '\0';
    dentry->dir[0] = '\0';
    dentry->super_block = NULL;
}

static int create_dentry(struct vfs_dentry_t *parent, const char *name, struct vfs_dentry_t **dentry)
{
    sys_trace();
    *dentry = (struct vfs_dentry_t *)sys_malloc(sizeof(struct vfs_dentry_t));
    if (NULL == *dentry)
    {
        sys_error("Out of memory.");
        return SYS_ERROR_NOMEM;
    }
    init_dentry(*dentry);
    sys_strcpy((*dentry)->name, name, VFS_MAX_FILE_NAME_LEN);
    if (sys_insert_node((sys_tree_node_t **)&parent->children, &(*dentry)->l, on_compare, NULL) < 0)
    {
        sys_free(*dentry);
        *dentry = NULL;
        sys_error("File exists.");
        return SYS_ERROR_EXIST;
    }
    (*dentry)->parent = parent;
    sys_strcpy((*dentry)->dir, parent->dir, VFS_MAX_FILE_PATH_LEN);
    sys_strcat((*dentry)->dir, "/", VFS_MAX_FILE_PATH_LEN);
    sys_strcpy((*dentry)->dir, parent->name, VFS_MAX_FILE_PATH_LEN);
    sys_strcpy((*dentry)->name, name, VFS_MAX_FILE_NAME_LEN);
    (*dentry)->super_block = parent->super_block;
    if (parent->super_block != NULL)
    {
        parent->super_block->ref_count++;
    }
    return 0;
}

static struct vfs_dentry_t *find_child(struct vfs_dentry_t *parent, const char *name)
{
    sys_trace();
    struct vfs_dentry_t *dentry = (struct vfs_dentry_t *)sys_find_node((sys_tree_node_t *)parent->children, (void *)name, on_compare, NULL);
    return dentry;
}

static void dump_str(char *dest, const char *src, const cregex_match_t *match)
{
    sys_trace();
    int n = match->len < VFS_MAX_FILE_NAME_LEN ? match->len : VFS_MAX_FILE_NAME_LEN;
    sys_memcpy(dest, &src[match->begin], n);
    dest[n] = '\0';
}

static int find_dentry(struct vfs_t *vfs, const char *path, struct vfs_dentry_t **dentry, int *position, int create)
{
    sys_trace();
    if (path[0] != '/')
    {
        return SYS_ERROR_NOTDIR;
    }
    int ret = 0;
    if (NULL == vfs->root)
    {
        if (create > 0)
        {
            vfs->root = (struct vfs_dentry_t *)sys_malloc(sizeof(struct vfs_dentry_t));
            if (NULL == vfs->root)
            {
                sys_error("Out of memory.");
                ret = SYS_ERROR_NOMEM;
                goto exception;
            }
            init_dentry(vfs->root);
        }
        else
        {
            sys_error("Not a directory.");
            ret = SYS_ERROR_NOTDIR;
            goto exception;
        }
    }
    *dentry = vfs->root;
    *position = 0;
    cregex_t *regex = cregex_compile("/+([^/]+)(/+$)?");
    if (NULL == regex)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    cregex_match_t matchs[2];
    while (cregex_search(regex, path, matchs, 2, 0) == 0)
    {
        char name[VFS_MAX_FILE_NAME_LEN] = {'\0'};
        dump_str(name, path, &matchs[1]);
        *position = matchs[0].begin + matchs[0].len; 
        if (sys_strcmp(name, ".") == 0)
        {
            continue;
        }
        else if (sys_strcmp(name, "..") == 0)
        {
            if ((*dentry)->parent != NULL)
            {
                *dentry = (*dentry)->parent;
            }
        }
        else
        {
            struct vfs_dentry_t *child = find_child(*dentry, name);
            if (NULL == child)
            {
                if (create > 0)
                {
                    ret = create_dentry(*dentry, name, &child);
                    if (ret < 0)
                    {
                        goto exception;
                    }
                }
                else
                {
                    goto finally;
                }
            }
            *dentry = child;
        }
    }
goto finally;
exception:
finally:
    if (regex != NULL)
    {
        cregex_free(regex);
    }
    return ret;
}

struct vfs_file_t *get_file_by_tid(struct vfs_t *vfs, int fd)
{
    sys_trace();
    int size = sys_vector_size(&vfs->files);
    if (fd < size)
    {
        struct vfs_file_t **pfile = (struct vfs_file_t **)sys_vector_at(&vfs->files, fd);
        if (*pfile != NULL)
        {
            return *pfile;
        }
    }
    return NULL;
}

int vfs_init(struct vfs_t *vfs)
{
    sys_trace();
    vfs->fs = NULL;
    vfs->root = NULL;
    int ret = sys_id_manager_init(&vfs->id_manager);
    if (ret < 0)
    {
        return ret;
    }
    ret = sys_vector_init(&vfs->files, sizeof(void *));
    return ret;
}

void vfs_free(struct vfs_t *vfs)
{
    sys_trace();
    while (vfs->fs != NULL)
    {
        struct vfs_fs_t *fs = (struct vfs_fs_t *)vfs->fs;
        sys_remove_from_single_list((sys_single_list_node_t **)&vfs->fs);
        sys_free(fs);
    }
    delete_all_dentry(vfs->root);
    vfs->root = NULL;
    sys_id_manager_uninit(&vfs->id_manager);
    for (int i = 0; i < vfs->files.size; i++)
    {
        void **p = (void **)sys_vector_at(&vfs->files, i);
        if (*p != NULL)
        {
            sys_free(*p);
        }
    }
    sys_vector_free(&vfs->files);
}

int vfs_registerfs(struct vfs_t *vfs, struct vfs_fs_t *fs)
{
    sys_trace();
    sys_insert_to_single_list((sys_single_list_node_t **)&vfs->fs, &fs->l);
    return 0;
}

int vfs_mount(struct vfs_t *vfs, const char *path, const char *device)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 1);
    if (ret < 0)
    {
        goto exception;
    }
    if (dentry->super_block != NULL && 1 == dentry->super_block->ref_count)
    {
        sys_error("File exists.");
        ret = SYS_ERROR_EXIST;
        goto exception;
    }
    struct vfs_super_block_t *super_block = (struct vfs_super_block_t *)sys_malloc(sizeof(struct vfs_super_block_t));
    if (NULL == super_block)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    struct vfs_fs_t *fs = (struct vfs_fs_t *)vfs->fs;
    for (; fs != NULL; fs = (struct vfs_fs_t *)vfs->fs->l.next)
    {
        fs->init_super_block(super_block, device);
        ret = super_block->fs_operations.mount(super_block, path, device);
        if (0 == ret)
        {
            break;
        }
    }
    if (ret < 0)
    {
        sys_error("Block device required.");
        ret = SYS_ERROR_NOTBLK;
        goto exception;
    }
    super_block->fs = fs;
    sys_strcpy(super_block->device, device, VFS_MAX_FILE_PATH_LEN);
    super_block->ref_count = 1;
    if (dentry->super_block != NULL)
    {
        dentry->super_block->ref_count--;
    }
    dentry->super_block = super_block;
goto finally;
exception:
    if (dentry != NULL)
    {
        delete_invalid_dentry(dentry);
        if (vfs->root == dentry)
        {
            vfs->root = NULL;
        }
    }
    if (super_block != NULL)
    {
        sys_free(super_block);
    }
finally:
    return ret;
}

int vfs_umount(struct vfs_t *vfs, const char *path)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        goto exception;
    }
    if (position < sys_strlen(path))
    {
        sys_error("Not a directory.");
        ret = SYS_ERROR_NOTDIR;
        goto exception;
    }
    if (dentry->super_block->ref_count > 1)
    {
        sys_error("Invalid argument.");
        ret = SYS_ERROR_INVAL;
        goto exception;
    }
    ret = dentry->super_block->fs_operations.unmount(dentry->super_block);
    if (ret < 0)
    {
        goto exception;
    }
    dentry->super_block->fs->release(dentry->super_block);
    struct vfs_dentry_t *parent = dentry->parent;
    delete_all_dentry(dentry);
    delete_invalid_dentry(parent);
    goto finally;
exception:
finally:
    return ret;
}

int vfs_open(struct vfs_t *vfs, const char *path, int flags, int mode)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    struct vfs_file_t *file = NULL;
    int fd = 0;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        goto exception;
    }
    if (((flags + 1) & (dentry->super_block->fs->flags + 1) & VFS_FLAG_ACCMODE) != ((flags + 1) & VFS_FLAG_ACCMODE))
    {
        sys_error("Permission denied.");
        ret = SYS_ERROR_ACCES;
        goto exception;
    }
    fd = sys_id_alloc(&vfs->id_manager);
    if (fd < 0)
    {
        ret = fd;
        goto exception;
    }
    file = (struct vfs_file_t *)sys_malloc(sizeof(struct vfs_file_t));
    if (NULL == file)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    file->flags = flags;
    file->super_block = dentry->super_block;
    ret = dentry->super_block->node.file_operations.open(file, &path[position], mode);
    if (ret < 0)
    {
        goto exception;
    }
    int size = sys_vector_size(&vfs->files);
    if (fd >= size)
    {
        void *p = NULL;
        ret = sys_vector_push_back(&vfs->files, &p);
        if (ret < 0)
        {
            sys_error("Out of memory.");
            ret = SYS_ERROR_NOMEM;
            goto exception;
        }
    }
    void **pp = (void **)sys_vector_at(&vfs->files, fd);
    *pp = file;
    goto finally;
exception:
    if (fd >= 0)
    {
        sys_id_free(&vfs->id_manager, fd);
    }
    if (file != NULL)
    {
        sys_free(file);
    }
finally:
    return ret;
}

int vfs_close(struct vfs_t *vfs, int fd)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    int ret = file->super_block->node.file_operations.close(file);
    if (ret < 0)
    {
        return ret;
    }
    sys_id_free(&vfs->id_manager, fd);
    sys_free(file);
    void **pp = (void **)sys_vector_at(&vfs->files, fd);
    *pp = NULL;
    return 0;
}

int vfs_read(struct vfs_t *vfs, int fd, void *buff, int count)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Invalid argument.");
        return SYS_ERROR_INVAL;
    }
    if (((file->flags + 1) & (VFS_FLAG_RDONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return file->super_block->node.file_operations.read(file, buff, count);
}

int vfs_write(struct vfs_t *vfs, int fd, const void *buff, int count)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    if (((file->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return file->super_block->node.file_operations.write(file, buff, count);
}

int64_t vfs_lseek(struct vfs_t *vfs, int fd, int64_t offset, int whence)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    return file->super_block->node.file_operations.lseek(file, offset, whence);
}

int64_t vfs_ftell(struct vfs_t *vfs, int fd)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    return file->super_block->node.file_operations.ftell(file);
}

int vfs_fstat(struct vfs_t *vfs, int fd, struct vfs_stat_t *stat)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    if (((file->flags + 1) & (VFS_FLAG_RDONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return file->super_block->node.file_operations.fstat(file, stat);
}

int vfs_syncfs(struct vfs_t *vfs, int fd)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    return file->super_block->node.file_operations.syncfs(file);
}

int vfs_ftruncate(struct vfs_t *vfs, int fd, int64_t length)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    if (((file->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return file->super_block->node.file_operations.ftruncate(file, length);
}

int vfs_link(struct vfs_t *vfs, const char *oldpath, const char *newpath)
{
    sys_trace();
    struct vfs_dentry_t *old_dentry = NULL;
    int old_position = 0;
    int ret = find_dentry(vfs, oldpath, &old_dentry, &old_position, 0);
    if (ret < 0)
    {
        return ret;
    }
    struct vfs_dentry_t *new_dentry = NULL;
    int new_position = 0;
    ret = find_dentry(vfs, newpath, &new_dentry, &new_position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (old_dentry != new_dentry)
    {
        sys_error("Cross-device link.");
        return SYS_ERROR_XDEV;
    }
    if (((old_dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return old_dentry->super_block->node.node_operations.link(old_dentry->super_block, &oldpath[old_position], &newpath[new_position]);
}

int vfs_unlink(struct vfs_t *vfs, const char *path)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (((dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return dentry->super_block->node.node_operations.unlink(dentry->super_block, &path[position]);
}

int vfs_chmod(struct vfs_t *vfs, const char *path, int mode)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (((dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return dentry->super_block->node.node_operations.chmod(dentry->super_block, &path[position], mode);
}

int vfs_rename(struct vfs_t *vfs, const char *oldpath, const char *newpath)
{
    sys_trace();
    struct vfs_dentry_t *old_dentry = NULL;
    int old_position = 0;
    int ret = find_dentry(vfs, oldpath, &old_dentry, &old_position, 0);
    if (ret < 0)
    {
        return ret;
    }
    struct vfs_dentry_t *new_dentry = NULL;
    int new_position = 0;
    ret = find_dentry(vfs, newpath, &new_dentry, &new_position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (old_dentry != new_dentry)
    {
        sys_error("Cross-device link.");
        return SYS_ERROR_XDEV;
    }
    if (((old_dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return old_dentry->super_block->node.node_operations.rename(old_dentry->super_block, &oldpath[old_position], &newpath[new_position]);
}

int vfs_mkdir(struct vfs_t *vfs, const char *path, int mode)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (((dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return dentry->super_block->node.node_operations.mkdir(dentry->super_block, &path[position], mode);
}

int vfs_rmdir(struct vfs_t *vfs, const char *path)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        return ret;
    }
    if (((dentry->super_block->fs->flags + 1) & (VFS_FLAG_WRONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return dentry->super_block->node.node_operations.rmdir(dentry->super_block, &path[position]);
}

int vfs_opendir(struct vfs_t *vfs, const char *path)
{
    sys_trace();
    struct vfs_dentry_t *dentry = NULL;
    struct vfs_file_t *file = NULL;
    int fd = 0;
    int position = 0;
    int ret = find_dentry(vfs, path, &dentry, &position, 0);
    if (ret < 0)
    {
        goto exception;
    }
    if (((dentry->super_block->fs->flags + 1) & (VFS_FLAG_RDONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    fd = sys_id_alloc(&vfs->id_manager);
    if (fd < 0)
    {
        ret = fd;
        goto exception;
    }
    file = (struct vfs_file_t *)sys_malloc(sizeof(struct vfs_file_t));
    if (NULL == file)
    {
        sys_error("Out of memory.");
        ret = SYS_ERROR_NOMEM;
        goto exception;
    }
    file->flags = VFS_FLAG_RDONLY;
    file->super_block = dentry->super_block;
    ret = dentry->super_block->node.node_operations.opendir(file, &path[position]);
    if (ret < 0)
    {
        goto exception;
    }
    int size = sys_vector_size(&vfs->files);
    if (fd >= size)
    {
        void *p = NULL;
        ret = sys_vector_push_back(&vfs->files, &p);
        if (ret < 0)
        {
            sys_error("Out of memory.");
            ret = SYS_ERROR_NOMEM;
            goto exception;
        }
    }
    void **pp = (void **)sys_vector_at(&vfs->files, fd);
    *pp = file;
    goto finally;
exception:
    if (fd >= 0)
    {
        sys_id_free(&vfs->id_manager, fd);
    }
    if (file != NULL)
    {
        sys_free(file);
    }
finally:
    return ret;
}

int vfs_closedir(struct vfs_t *vfs, int fd)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    int ret = file->super_block->node.node_operations.closedir(file);
    if (ret < 0)
    {
        return ret;
    }
    sys_id_free(&vfs->id_manager, fd);
    sys_free(file);
    void **pp = (void **)sys_vector_at(&vfs->files, fd);
    *pp = NULL;
    return 0;
}

int vfs_readdir(struct vfs_t *vfs, int fd, struct vfs_dirent_t *dirent)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    if (((file->flags + 1) & (VFS_FLAG_RDONLY + 1)) == 0)
    {
        sys_error("Permission denied.");
        return SYS_ERROR_ACCES;
    }
    return file->super_block->node.node_operations.readdir(file, dirent);
}

int vfs_rewinddir(struct vfs_t *vfs, int fd)
{
    sys_trace();
    struct vfs_file_t *file = get_file_by_tid(vfs, fd);
    if (NULL == file)
    {
        sys_error("Bad file number.");
        return SYS_ERROR_BADF;
    }
    return file->super_block->node.node_operations.rewinddir(file);
}
