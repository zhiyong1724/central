#include "fatfsadapter.h"
#include "osf.h"
#include "ff.h"
#include "osmem.h"
static int open(OsFile *file, const char *path, uint32_t mode)
{
    int ret = OS_FILE_ERROR_MALLOC_ERR;
    file->obj = osMalloc(sizeof(FIL));
    if (file->obj != NULL)
    {
        BYTE fmode = 0;
        if ((mode & OS_FILE_MODE_READ) > 0)
        {
            fmode |= FA_READ;
        }
        if ((mode & OS_FILE_MODE_WRITE) > 0)
        {
            fmode |= FA_WRITE;
        }
        if ((mode & OS_FILE_MODE_OPEN_EXISTING) > 0)
        {
            fmode |= FA_OPEN_EXISTING;
        }
        if ((mode & OS_FILE_MODE_CREATE_NEW) > 0)
        {
            fmode |= FA_CREATE_NEW;
        }
        if ((mode & OS_FILE_MODE_CREATE_ALWAYS) > 0)
        {
            fmode |= FA_CREATE_ALWAYS;
        }
        if ((mode & OS_FILE_MODE_OPEN_ALWAYS) > 0)
        {
            fmode |= FA_OPEN_ALWAYS;
        }
        if ((mode & OS_FILE_MODE_OPEN_APPEND) > 0)
        {
            fmode |= FA_OPEN_APPEND;
        }
        FRESULT result = f_open((FIL *)file->obj, (const TCHAR *)path, fmode);
        switch (result)
        {
        case FR_OK:
            ret = OS_FILE_ERROR_OK;
            break;
        case FR_DISK_ERR:
            ret = OS_FILE_ERROR_DISK_ERR;
            break;
        case FR_NOT_READY:
            ret = OS_FILE_ERROR_NOT_READY;
            break;
        case FR_WRITE_PROTECTED:
            ret = OS_FILE_ERROR_WRITE_PROTECTED;
            break;
        case FR_INVALID_DRIVE:
            ret = OS_FILE_ERROR_INVALID_DRIVE;
            break;
        case FR_NO_FILESYSTEM:
            ret = OS_FILE_ERROR_NO_FILESYSTEM;
            break;
        case FR_NO_FILE:
            ret = OS_FILE_ERROR_NO_FILE;
            break;
        case FR_NO_PATH:
            ret = OS_FILE_ERROR_NO_PATH;
            break;
        case FR_INVALID_NAME:
            ret = OS_FILE_ERROR_INVALID_NAME;
            break;
        case FR_EXIST:
            ret = OS_FILE_ERROR_EXIST;
            break;
        case FR_INVALID_OBJECT:
            ret = OS_FILE_ERROR_INVALID_OBJECT;
            break;
        default:
            ret = OS_FILE_ERROR_OTHER;
            break;
        }
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static int close(OsFile *file)
{
    int ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_close((FIL *)file->obj);
        switch (result)
        {
        case FR_OK:
            ret = OS_FILE_ERROR_OK;
            break;
        case FR_DISK_ERR:
            ret = OS_FILE_ERROR_DISK_ERR;
            break;
        case FR_INVALID_OBJECT:
            ret = OS_FILE_ERROR_INVALID_OBJECT;
            break;
        default:
            ret = OS_FILE_ERROR_OTHER;
            break;
        }
        if (ret != OS_FILE_ERROR_OK)
        {
            osFree(file->obj);
            file->obj = NULL;
        }
    }
    return ret;
}

static int read(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    int ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_read((FIL *)file->obj, buff, (UINT)size, (UINT *)length);
        switch (result)
        {
        case FR_OK:
            ret = OS_FILE_ERROR_OK;
            break;
        case FR_DISK_ERR:
            ret = OS_FILE_ERROR_DISK_ERR;
            break;
        case FR_INVALID_OBJECT:
            ret = OS_FILE_ERROR_INVALID_OBJECT;
            break;
        default:
            ret = OS_FILE_ERROR_OTHER;
            break;
        }
    }
    return ret;
}

static int write(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    int ret = OS_FILE_ERROR_INVALID_OBJECT;
    if (file != NULL && file->obj != NULL)
    {
        FRESULT result = f_write((FIL *)file->obj, buff, (UINT)size, (UINT *)length);
        switch (result)
        {
        case FR_OK:
            ret = OS_FILE_ERROR_OK;
            break;
        case FR_DISK_ERR:
            ret = OS_FILE_ERROR_DISK_ERR;
            break;
        case FR_INVALID_OBJECT:
            ret = OS_FILE_ERROR_INVALID_OBJECT;
            break;
        default:
            ret = OS_FILE_ERROR_OTHER;
            break;
        }
    }
    return ret;
}

int (*seek)(OsFile *file, int64_t offset, OsSeekType whence);
int (*truncate)(OsFile *file, uint64_t size);
int (*sync)(OsFile *file);
int (*openDir)(OsDir *dir, const char *path);
int (*closeDir)(OsDir *dir);
int (*readDir)(OsDir *dir, OsFileInfo *fileInfo);
int (*findFirst)(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
int (*findNext)(OsDir *dir, OsFileInfo *fileInfo);
int (*mkdir)(const char *path);
int (*unlink)(const char *path);
int (*rename)(const char *oldPath, const char *newPath);
int (*stat)(const char *path, OsFileInfo *fileInfo);
int (*chmod)(const char *path, uint32_t attr, uint32_t mask);
int (*chdrive)(const char *path);
int (*getFree)(const char *path, uint64_t *clusters, OsFS *fs);

int registerFatfs()
{
    return 0;
}