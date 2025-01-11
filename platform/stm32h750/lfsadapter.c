#include "lfs_adapter.h"
#include "lfs.h"
#include <string.h>
#include "sys_mem.h"
extern const struct lfs_config g_lfs_config;
extern lfs_t g_lfs;
static int sMount = 0;
static sys_file_error_t parseResult(int result)
{
    sys_file_error_t ret = SYS_FILE_ERROR_OK;
    switch (result)
    {
    case LFS_ERR_OK:
        ret = SYS_FILE_ERROR_OK;
        break;
    case LFS_ERR_IO:
        ret = SYS_FILE_ERROR_DISK_ERR;
        break;
    case LFS_ERR_CORRUPT:
        ret = SYS_FILE_ERROR_CORRUPT;
        break;
    case LFS_ERR_NOENT:
        ret = SYS_FILE_ERROR_NO_PATH;
        break;
    case LFS_ERR_EXIST:
        ret = SYS_FILE_ERROR_EXIST;
        break;
    case LFS_ERR_NOTDIR:
        ret = SYS_FILE_ERROR_NO_FILE;
        break;
    case LFS_ERR_ISDIR:
        ret = SYS_FILE_ERROR_IS_DIR;
        break;
    case LFS_ERR_NOTEMPTY:
        ret = SYS_FILE_ERROR_DIR_NOTEMPTY;
        break;
    case LFS_ERR_FBIG:
        ret = SYS_FILE_ERROR_FILE_TOO_LARGE;
        break;
    case LFS_ERR_INVAL:
        ret = SYS_FILE_ERROR_INVALID_PARAMETER;
        break;
    case LFS_ERR_NOSPC:
        ret = SYS_FILE_ERROR_NO_PAGE;
        break;
    case LFS_ERR_NOMEM:
        ret = SYS_FILE_ERROR_NOMEM;
        break;
    case LFS_ERR_NAMETOOLONG:
        ret = SYS_FILE_ERROR_NAME_TOO_LONG;
        break;
    default:
        ret = SYS_FILE_ERROR_OTHER;
        break;
    }
    return ret;
}

static sys_file_error_t lfsOpen(sys_file_t *file, const char *path, uint32_t mode)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    file->obj = sys_malloc(sizeof(lfs_file_t));
    if (file->obj != NULL)
    {
        memset(file->obj, 0, sizeof(lfs_file_t));
        static struct lfs_file_config cfg;
        memset(&cfg, 0, sizeof(struct lfs_file_config));
        cfg.buffer = sys_malloc(g_lfs.cfg->cache_size);
        if (cfg.buffer != NULL)
        {
            int flags = 0;
            if ((mode & SYS_FILE_MODE_READ) > 0 && 0 == (mode & SYS_FILE_MODE_WRITE))
            {
                flags |= LFS_O_RDONLY;
            }
            else if (0 == (mode & SYS_FILE_MODE_READ) && (mode & SYS_FILE_MODE_WRITE) > 0)
            {
                flags |= LFS_O_WRONLY;
            }
            else if ((mode & SYS_FILE_MODE_READ) > 0 && (mode & SYS_FILE_MODE_WRITE) > 0)
            {
                flags |= LFS_O_RDWR;
            }

            if ((mode & SYS_FILE_MODE_OPEN_EXISTING) > 0)
            {
            }
            if ((mode & SYS_FILE_MODE_CREATE_NEW) > 0)
            {
                flags |= LFS_O_CREAT | LFS_O_EXCL;
            }
            if ((mode & SYS_FILE_MODE_CREATE_ALWAYS) > 0)
            {
                flags |= LFS_O_CREAT | LFS_O_TRUNC;
            }
            if ((mode & SYS_FILE_MODE_OPEN_ALWAYS) > 0)
            {
                flags |= LFS_O_CREAT;
            }
            if ((mode & SYS_FILE_MODE_OPEN_APPEND) > 0)
            {
                flags |= LFS_O_APPEND;
            }
            int result = lfs_file_opencfg(&g_lfs, (lfs_file_t *)file->obj, path, flags, &cfg);
            ret = parseResult(result);
            if (ret != SYS_FILE_ERROR_OK)
            {
                sys_free(cfg.buffer);
                sys_free(file->obj);
                file->obj = NULL;
            }
        }
        else
        {
            sys_free(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t lfsClose(sys_file_t *file)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_close(&g_lfs, (lfs_file_t *)file->obj);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            sys_free(((lfs_file_t *)file->obj)->cache.buffer);
            sys_free(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t lfsRead(sys_file_t *file, void *buff, uint64_t size, uint64_t *length)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_ssize_t result = lfs_file_read(&g_lfs, (lfs_file_t *)file->obj, buff, size);
        if (result >= 0)
        {
            ret = SYS_FILE_ERROR_OK;
            *length = (uint64_t)result;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static sys_file_error_t lfsWrite(sys_file_t *file, const void *buff, uint64_t size, uint64_t *length)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_ssize_t result = lfs_file_write(&g_lfs, (lfs_file_t *)file->obj, buff, size);
        if (result >= 0)
        {
            ret = SYS_FILE_ERROR_OK;
            *length = (uint64_t)result;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static sys_file_error_t lfsSeek(sys_file_t *file, int64_t offset, sys_seek_type_t whence)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int flag = 0;
        switch (whence)
        {
        case SYS_SEEK_TYPE_SET:
            flag = LFS_SEEK_SET;
            break;
        case SYS_SEEK_TYPE_CUR:
            flag = LFS_SEEK_CUR;
            break;
        case SYS_SEEK_TYPE_END:
            flag = LFS_SEEK_END;
            break;
        default:
            break;
        }
        
        lfs_soff_t result = lfs_file_seek(&g_lfs, (lfs_file_t *)file->obj, (lfs_soff_t)offset, flag);
        if (result >= 0)
        {
            ret = SYS_FILE_ERROR_OK;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static sys_file_error_t lfsTell(sys_file_t *file, uint64_t *offset)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_soff_t result = lfs_file_tell(&g_lfs, (lfs_file_t *)file->obj);
        if (result >= 0)
        {
            ret = SYS_FILE_ERROR_OK;
            *offset = result;
        }
        else
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static sys_file_error_t lfsTruncate(sys_file_t *file, uint64_t size)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_truncate(&g_lfs, (lfs_file_t *)file->obj, (lfs_off_t)size);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t lfsSync(sys_file_t *file)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_sync(&g_lfs, (lfs_file_t *)file->obj);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t lfsOpenDir(sys_dir_t *dir, const char *path)
{
    sys_file_error_t ret = SYS_FILE_ERROR_NOMEM;
    dir->obj = sys_malloc(sizeof(lfs_dir_t));
    if (dir->obj != NULL)
    {
        memset(dir->obj, 0, sizeof(lfs_dir_t));
        int result = lfs_dir_open(&g_lfs, (lfs_dir_t *)dir->obj, path);
        ret = parseResult(result);
        if (ret != SYS_FILE_ERROR_OK)
        {
            sys_free(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static sys_file_error_t lfsCloseDir(sys_dir_t *dir)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        int result = lfs_dir_close(&g_lfs, (lfs_dir_t *)dir->obj);
        ret = parseResult(result);
        if (SYS_FILE_ERROR_OK == ret)
        {
            sys_free(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static void parseFileInfo(const struct lfs_info *fno, sys_file_info_t *file_info)
{
    if (LFS_TYPE_DIR == fno->type)
    {
        file_info->type = SYS_FILE_TYPE_DIRECTORY;
    }
    else 
    {
        file_info->type = SYS_FILE_TYPE_NORMAL;
    }
    file_info->attribute = SYS_FILE_ATTR_OWNER_READ | SYS_FILE_ATTR_OWNER_WRITE | SYS_FILE_ATTR_OWNER_EXE;
    sys_file_time_t time;
    memset(&time, 0, sizeof(sys_file_time_t));
    file_info->create_time = time;
    file_info->change_time = time;
    file_info->access_time = time;
    file_info->file_size = (uint64_t)fno->size;
    strcpy(file_info->name, fno->name);
}

static sys_file_error_t lfsReadDir(sys_dir_t *dir, sys_file_info_t *file_info)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        struct lfs_info fno;
        fno.name[0] = '\0';
        int result = lfs_dir_read(&g_lfs, (lfs_dir_t *)dir->obj, &fno);
        if (result >= 0)
        {
            ret = SYS_FILE_ERROR_OK;
            parseFileInfo(&fno, file_info);
        }
        else
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static sys_file_error_t lfsFindFirst(sys_dir_t *dir, sys_file_info_t *file_info, const char *path, const char *pattern)
{
    return SYS_FILE_ERROR_NONSUPPORT;
}

static sys_file_error_t lfsFindNext(sys_dir_t *dir, sys_file_info_t *file_info)
{
    return SYS_FILE_ERROR_NONSUPPORT;
}

static sys_file_error_t lfsMkDir(const char *path)
{
    int result = lfs_mkdir(&g_lfs, path);
    return parseResult(result);
}

static sys_file_error_t lfsUnlink(const char *path)
{
    int result = lfs_remove(&g_lfs, path);
    return parseResult(result);
}

static sys_file_error_t lfsRename(const char *oldPath, const char *newPath)
{
    int result = lfs_rename(&g_lfs, oldPath, newPath);
    return parseResult(result);
}

static sys_file_error_t lfsStat(const char *path, sys_file_info_t *file_info)
{
    struct lfs_info fno;
    fno.name[0] = '\0';
    int result = lfs_stat(&g_lfs, path, &fno);
    sys_file_error_t ret = parseResult(result);
    if (SYS_FILE_ERROR_OK == ret)
    {
        parseFileInfo(&fno, file_info);
    }
    return ret;
}

static sys_file_error_t lfsChMod(const char *path, uint32_t attr, uint32_t mask)
{
    return SYS_FILE_ERROR_NONSUPPORT;
}

static sys_file_error_t lfsChDriver(const char *path)
{
    return SYS_FILE_ERROR_NONSUPPORT;
}

static sys_file_error_t lfsStatFS(const char *path, sys_fs_t *fs)
{
    sys_file_error_t ret = SYS_FILE_ERROR_INVALID_PARAMETER;
    lfs_ssize_t result = lfs_fs_size(&g_lfs);
    if (result >= 0)
    {
        ret = SYS_FILE_ERROR_OK;
        fs->free_pages = (uint64_t)((g_lfs.cfg->block_count - result) * (g_lfs.cfg->block_size / g_lfs.cfg->read_size));
        fs->page_size = (uint64_t)g_lfs.cfg->read_size;
        fs->total_pages = (uint64_t)(g_lfs.cfg->block_count * (g_lfs.cfg->block_size / g_lfs.cfg->read_size));
        strcpy(fs->type, "littlefs");
    }
    else
    {
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t lfsMount(sys_mount_info_t *mount_info)
{
    int ret = SYS_FILE_ERROR_INVALID_DRIVER;
    if (0 == sMount)
    {
        sMount = 1;
        mount_info->obj = &g_lfs;
        int result = lfs_mount(&g_lfs, &g_lfs_config);
        ret = parseResult(result);
    }
    return ret;
}

static sys_file_error_t lfsUnmount(sys_mount_info_t *mount_info)
{
    int ret = SYS_FILE_ERROR_INVALID_DRIVER;
    if (1 == sMount)
    {
        sMount = 0;
        mount_info->obj = NULL;
        int result = lfs_unmount(&g_lfs);
        ret = parseResult(result);
    }
    return ret;
}

sys_file_error_t register_lfs()
{
    sys_fs_interfaces_t fsInterfaces;
    fsInterfaces.open = lfsOpen;
    fsInterfaces.close = lfsClose;
    fsInterfaces.read = lfsRead;
    fsInterfaces.write = lfsWrite;
    fsInterfaces.seek = lfsSeek;
    fsInterfaces.tell = lfsTell;
    fsInterfaces.truncate = lfsTruncate;
    fsInterfaces.sync = lfsSync;
    fsInterfaces.openDir = lfsOpenDir;
    fsInterfaces.closeDir = lfsCloseDir;
    fsInterfaces.readDir = lfsReadDir;
    fsInterfaces.findFirst = lfsFindFirst;
    fsInterfaces.findNext = lfsFindNext;
    fsInterfaces.mkDir = lfsMkDir;
    fsInterfaces.unlink = lfsUnlink;
    fsInterfaces.rename = lfsRename;
    fsInterfaces.stat = lfsStat;
    fsInterfaces.chMod = lfsChMod;
    fsInterfaces.chDriver = lfsChDriver;
    fsInterfaces.statFS = lfsStatFS;
    fsInterfaces.mount = lfsMount;
    fsInterfaces.unmount = lfsUnmount;
    return osFAddFS(&fsInterfaces);
}