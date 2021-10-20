#include "lfsadapter.h"
#include "lfs.h"
#include <string.h>
#include "osmem.h"
#include "lfsio.h"
static int sMount = 0;
static OsFileError parseResult(int result)
{
    OsFileError ret = OS_FILE_ERROR_OK;
    switch (result)
    {
    case LFS_ERR_OK:
        ret = OS_FILE_ERROR_OK;
        break;
    case LFS_ERR_IO:
        ret = OS_FILE_ERROR_DISK_ERR;
        break;
    case LFS_ERR_CORRUPT:
        ret = OS_FILE_ERROR_CORRUPT;
        break;
    case LFS_ERR_NOENT:
        ret = OS_FILE_ERROR_NO_PATH;
        break;
    case LFS_ERR_EXIST:
        ret = OS_FILE_ERROR_EXIST;
        break;
    case LFS_ERR_NOTDIR:
        ret = OS_FILE_ERROR_NO_FILE;
        break;
    case LFS_ERR_ISDIR:
        ret = OS_FILE_ERROR_IS_DIR;
        break;
    case LFS_ERR_NOTEMPTY:
        ret = OS_FILE_ERROR_DIR_NOTEMPTY;
        break;
    case LFS_ERR_FBIG:
        ret = OS_FILE_ERROR_FILE_TOO_LARGE;
        break;
    case LFS_ERR_INVAL:
        ret = OS_FILE_ERROR_INVALID_PARAMETER;
        break;
    case LFS_ERR_NOSPC:
        ret = OS_FILE_ERROR_NO_PAGE;
        break;
    case LFS_ERR_NOMEM:
        ret = OS_FILE_ERROR_NOMEM;
        break;
    case LFS_ERR_NAMETOOLONG:
        ret = OS_FILE_ERROR_NAME_TOO_LONG;
        break;
    default:
        ret = OS_FILE_ERROR_OTHER;
        break;
    }
    return ret;
}

static OsFileError lfsOpen(OsFile *file, const char *path, uint32_t mode)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    file->obj = osMalloc(sizeof(lfs_file_t));
    if (file->obj != NULL)
    {
        memset(file->obj, 0, sizeof(lfs_file_t));
        int flags = 0;
        if ((mode & OS_FILE_MODE_READ) > 0 && 0 == (mode & OS_FILE_MODE_WRITE))
        {
            flags |= LFS_O_RDONLY;
        }
        else if (0 == (mode & OS_FILE_MODE_READ) && (mode & OS_FILE_MODE_WRITE) > 0)
        {
            flags |= LFS_O_WRONLY;
        }
        else if ((mode & OS_FILE_MODE_READ) > 0 && (mode & OS_FILE_MODE_WRITE) > 0)
        {
            flags |= LFS_O_RDWR;
        }

        if ((mode & OS_FILE_MODE_OPEN_EXISTING) > 0)
        {
        }
        if ((mode & OS_FILE_MODE_CREATE_NEW) > 0)
        {
            flags |= LFS_O_CREAT | LFS_O_EXCL;
        }
        if ((mode & OS_FILE_MODE_CREATE_ALWAYS) > 0)
        {
            flags |= LFS_O_CREAT | LFS_O_TRUNC;
        }
        if ((mode & OS_FILE_MODE_OPEN_ALWAYS) > 0)
        {
            flags |= LFS_O_CREAT;
        }
        if ((mode & OS_FILE_MODE_OPEN_APPEND) > 0)
        {
            flags |= LFS_O_APPEND;
        }
        int result = lfs_file_open(&gLFS, (lfs_file_t *)file->obj, path, flags);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static OsFileError lfsClose(OsFile *file)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_close(&gLFS, (lfs_file_t *)file->obj);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static OsFileError lfsRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_ssize_t result = lfs_file_read(&gLFS, (lfs_file_t *)file->obj, buff, size);
        if (result >= 0)
        {
            ret = OS_FILE_ERROR_OK;
            *length = (uint64_t)result;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static OsFileError lfsWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_ssize_t result = lfs_file_write(&gLFS, (lfs_file_t *)file->obj, buff, size);
        if (result >= 0)
        {
            ret = OS_FILE_ERROR_OK;
            *length = (uint64_t)result;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static OsFileError lfsSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int flag = 0;
        switch (whence)
        {
        case OS_SEEK_TYPE_SET:
            flag = LFS_SEEK_SET;
            break;
        case OS_SEEK_TYPE_CUR:
            flag = LFS_SEEK_CUR;
            break;
        case OS_SEEK_TYPE_END:
            flag = LFS_SEEK_END;
            break;
        default:
            break;
        }
        lfs_soff_t lfsOffset = 0;
        if (offset >= 0)
        {
            lfsOffset = (lfs_soff_t)offset;
        }
        else 
        {
            lfsOffset = (lfs_soff_t)(offset * -1);
            lfsOffset *= -1;
        }
        
        lfs_soff_t result = lfs_file_seek(&gLFS, (lfs_file_t *)file->obj, lfsOffset, flag);
        if (result >= 0)
        {
            ret = OS_FILE_ERROR_OK;
        }
        else 
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static OsFileError lfsTell(OsFile *file, uint64_t *offset)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        lfs_soff_t result = lfs_file_tell(&gLFS, (lfs_file_t *)file->obj);
        if (result >= 0)
        {
            ret = OS_FILE_ERROR_OK;
            *offset = result;
        }
        else
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static OsFileError lfsTruncate(OsFile *file, uint64_t size)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_truncate(&gLFS, (lfs_file_t *)file->obj, (lfs_off_t)size);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError lfsSync(OsFile *file)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        int result = lfs_file_sync(&gLFS, (lfs_file_t *)file->obj);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError lfsOpenDir(OsDir *dir, const char *path)
{
    OsFileError ret = OS_FILE_ERROR_NOMEM;
    dir->obj = osMalloc(sizeof(lfs_dir_t));
    if (dir->obj != NULL)
    {
        memset(dir->obj, 0, sizeof(lfs_dir_t));
        int result = lfs_dir_open(&gLFS, (lfs_dir_t *)dir->obj, path);
        ret = parseResult(result);
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static OsFileError lfsCloseDir(OsDir *dir)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        int result = lfs_dir_close(&gLFS, (lfs_dir_t *)dir->obj);
        ret = parseResult(result);
        if (OS_FILE_ERROR_OK == ret)
        {
            osFree(dir->obj);
            dir->obj = NULL;
        }
    }
    return ret;
}

static void parseFileInfo(const struct lfs_info *fno, OsFileInfo *fileInfo)
{
    if (LFS_TYPE_DIR == fno->type)
    {
        fileInfo->type = OS_FILE_TYPE_DIRECTORY;
    }
    else 
    {
        fileInfo->type = OS_FILE_TYPE_NORMAL;
    }
    fileInfo->attribute = OS_FILE_ATTR_OWNER_READ | OS_FILE_ATTR_OWNER_WRITE | OS_FILE_ATTR_OWNER_EXE;
    OsFileTime time;
    memset(&time, 0, sizeof(OsFileTime));
    fileInfo->createTime = time;
    fileInfo->changeTime = time;
    fileInfo->accessTime = time;
    fileInfo->fileSize = (uint64_t)fno->size;
    strcpy(fileInfo->name, fno->name);
}

static OsFileError lfsReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (dir != NULL && dir->obj != NULL)
    {
        struct lfs_info fno;
        fno.name[0] = '\0';
        int result = lfs_dir_read(&gLFS, (lfs_dir_t *)dir->obj, &fno);
        if (result >= 0)
        {
            ret = OS_FILE_ERROR_OK;
            parseFileInfo(&fno, fileInfo);
        }
        else
        {
            ret = parseResult(result);
        }
    }
    return ret;
}

static OsFileError lfsFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    return OS_FILE_ERROR_NONSUPPORT;
}

static OsFileError lfsFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    return OS_FILE_ERROR_NONSUPPORT;
}

static OsFileError lfsMkDir(const char *path)
{
    int result = lfs_mkdir(&gLFS, path);
    return parseResult(result);
}

static OsFileError lfsUnlink(const char *path)
{
    int result = lfs_remove(&gLFS, path);
    return parseResult(result);
}

static OsFileError lfsRename(const char *oldPath, const char *newPath)
{
    int result = lfs_rename(&gLFS, oldPath, newPath);
    return parseResult(result);
}

static OsFileError lfsStat(const char *path, OsFileInfo *fileInfo)
{
    struct lfs_info fno;
    fno.name[0] = '\0';
    int result = lfs_stat(&gLFS, path, &fno);
    OsFileError ret = parseResult(result);
    if (OS_FILE_ERROR_OK == ret)
    {
        parseFileInfo(&fno, fileInfo);
    }
    return ret;
}

static OsFileError lfsChMod(const char *path, uint32_t attr, uint32_t mask)
{
    return OS_FILE_ERROR_NONSUPPORT;
}

static OsFileError lfsChDrive(const char *path)
{
    return OS_FILE_ERROR_NONSUPPORT;
}

static OsFileError lfsStatFS(const char *path, OsFS *fs)
{
    OsFileError ret = OS_FILE_ERROR_INVALID_PARAMETER;
    lfs_ssize_t result = lfs_fs_size(&gLFS);
    if (result >= 0)
    {
        ret = OS_FILE_ERROR_OK;
        fs->freePages = (uint64_t)((gLFS.cfg->block_count - result) * (gLFS.cfg->block_size / gLFS.cfg->read_size));
        fs->pageSize = (uint64_t)gLFS.cfg->read_size;
        fs->totalPages = (uint64_t)(gLFS.cfg->block_count * (gLFS.cfg->block_size / gLFS.cfg->read_size));
        strcpy(fs->type, "littlefs");
    }
    else
    {
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError lfsMount(OsMountInfo *mountInfo)
{
    int ret = OS_FILE_ERROR_INVALID_DRIVE;
    if (0 == sMount)
    {
        sMount = 1;
        mountInfo->obj = &gLFS;
        int result = lfs_mount(&gLFS, &gLfsConfig);
        ret = parseResult(result);
    }
    return ret;
}

static OsFileError lfsUnmount(OsMountInfo *mountInfo)
{
    int ret = OS_FILE_ERROR_INVALID_DRIVE;
    if (1 == sMount)
    {
        sMount = 0;
        mountInfo->obj = NULL;
        int result = lfs_unmount(&gLFS);
        ret = parseResult(result);
    }
    return ret;
}

OsFileError registerLFS()
{
    OsFSInterfaces fsInterfaces;
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
    fsInterfaces.chDrive = lfsChDrive;
    fsInterfaces.statFS = lfsStatFS;
    fsInterfaces.mount = lfsMount;
    fsInterfaces.unmount = lfsUnmount;
    return osFAddFS(&fsInterfaces);
}