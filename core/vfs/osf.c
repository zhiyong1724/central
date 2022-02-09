#include "osf.h"
#include "osvfs.h"
#include "osmutex.h"
#define ENABLE_F_LOG 0
#if ENABLE_F_LOG
#define fLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define fLog(format, ...) (void)0
#endif
#if OS_USE_VFS
static OsVFS *sVFS;
static OsRecursiveMutex sMutex;
OsFileError osFInit(OsVFS *vfs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sVFS = vfs;
    osRecursiveMutexCreate(&sMutex);
    return osVFSInit(sVFS);
}

OsFileError osFAddFS(OsFSInterfaces *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSAddFS(sVFS, fs);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFOpen(OsFile *file, const char *path, uint32_t mode)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSOpen(sVFS, file, path, mode);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFClose(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSClose(sVFS, file);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSRead(sVFS, file, buff, size, length);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSWrite(sVFS, file, buff, size, length);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSSeek(sVFS, file, offset, whence);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFTell(OsFile *file, uint64_t *offset)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSTell(sVFS, file, offset);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFTruncate(OsFile *file, uint64_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSTruncate(sVFS, file, size);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFSync(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSSync(sVFS, file);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFOpenDir(OsDir *dir, const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSOpenDir(sVFS, dir, path);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFCloseDir(OsDir *dir)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSCloseDir(sVFS, dir);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSReadDir(sVFS, dir, fileInfo);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSFindFirst(sVFS, dir, fileInfo, path, pattern);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSFindNext(sVFS, dir, fileInfo);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFMkDir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSMkDir(sVFS, path);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFUnlink(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSUnlink(sVFS, path);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFRename(const char *oldPath, const char *newPath)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSRename(sVFS, oldPath, newPath);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFStat(const char *path, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSStat(sVFS, path, fileInfo);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFChMod(const char *path, uint32_t attr, uint32_t mask)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSChMod(sVFS, path, attr, mask);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFStatFS(const char *path, OsFS *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSStatFS(sVFS, path, fs);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFMount(const char *path, const char *drive)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSMount(sVFS, path, drive);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFUnmount(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSUnmount(sVFS, path);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFChDir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSChDir(sVFS, path);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFGetCWD(char *buffer, uint32_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSGetCWD(sVFS, buffer, size);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFGetMountInfo(const OsMountInfo **mountInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileError ret = osVFSGetMountInfo(sVFS, mountInfo);
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFCopy(const char *srcPath, const char *destPath)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osRecursiveMutexLock(&sMutex);
    OsFileInfo srcFileInfo;
    OsFileError ret = osFStat(srcPath, &srcFileInfo);
    if (OS_FILE_ERROR_OK == ret)
    {
        const char *path = NULL;
        osFGetCWD(sVFS->pathB, OS_MAX_FILE_PATH_LENGTH);
        ret = osFChDir(destPath);
        if (OS_FILE_ERROR_OK == ret)
        {
            path = srcFileInfo.name;
        }
        else
        {
            path = destPath;
        }
        if (OS_FILE_TYPE_NORMAL == srcFileInfo.type)
        {
            OsFile destFile;
            ret = osFOpen(&destFile, path, OS_FILE_MODE_WRITE | OS_FILE_MODE_CREATE_NEW);
            osFChDir(sVFS->pathB);
            if (OS_FILE_ERROR_OK == ret)
            {
                OsFile srcFile;
                OsFileError ret = osFOpen(&srcFile, srcPath, OS_FILE_MODE_READ | OS_FILE_MODE_OPEN_EXISTING);
                if (OS_FILE_ERROR_OK == ret)
                {
                    osRecursiveMutexUnlock(&sMutex);
                    uint64_t buffer;
                    uint64_t length;
                    for (;;)
                    {
                        osFRead(&srcFile, &buffer, sizeof(uint64_t), &length);
                        if (0 == length)
                        {
                            break;
                        }
                        osFWrite(&destFile, &buffer, length, &length);
                    }
                    ret = osFClose(&srcFile);
                    osRecursiveMutexLock(&sMutex);
                }
                ret = osFClose(&destFile);
            }
        }
        else if (OS_FILE_TYPE_DIRECTORY == srcFileInfo.type)
        {
            ret = osFMkDir(path);
            osFChDir(sVFS->pathB);
        }
    }
    osRecursiveMutexUnlock(&sMutex);
    return ret;
}
#endif