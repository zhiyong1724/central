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
static OsMutex sMutex;
OsFileError osFInit(OsVFS *vfs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sVFS = vfs;
    osMutexCreateStatic(&sMutex);
    return osVFSInit(sVFS);
}

OsFileError osFAddFS(OsFSInterfaces *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSAddFS(sVFS, fs);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFOpen(OsFile *file, const char *path, uint32_t mode)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSOpen(sVFS, file, path, mode);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFClose(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSClose(sVFS, file);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSRead(sVFS, file, buff, size, length);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSWrite(sVFS, file, buff, size, length);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSSeek(sVFS, file, offset, whence);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFTell(OsFile *file, uint64_t *offset)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSTell(sVFS, file, offset);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFTruncate(OsFile *file, uint64_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSTruncate(sVFS, file, size);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFSync(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSSync(sVFS, file);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFOpenDir(OsDir *dir, const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSOpenDir(sVFS, dir, path);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFCloseDir(OsDir *dir)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSCloseDir(sVFS, dir);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSReadDir(sVFS, dir, fileInfo);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSFindFirst(sVFS, dir, fileInfo, path, pattern);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSFindNext(sVFS, dir, fileInfo);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFMkDir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSMkDir(sVFS, path);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFUnlink(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSUnlink(sVFS, path);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFRename(const char *oldPath, const char *newPath)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSRename(sVFS, oldPath, newPath);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFStat(const char *path, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSStat(sVFS, path, fileInfo);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFChMod(const char *path, uint32_t attr, uint32_t mask)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSChMod(sVFS, path, attr, mask);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFStatFS(const char *path, OsFS *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSStatFS(sVFS, path, fs);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFMount(const char *path, const char *drive)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSMount(sVFS, path, drive);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFUnmount(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSUnmount(sVFS, path);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFChDir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSChDir(sVFS, path);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFGetCWD(char *buffer, uint32_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSGetCWD(sVFS, buffer, size);
    osMutexUnlock(&sMutex);
    return ret;
}

OsFileError osFGetMountInfo(const OsMountInfo **mountInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osMutexLock(&sMutex);
    OsFileError ret = osVFSGetMountInfo(sVFS, mountInfo);
    osMutexUnlock(&sMutex);
    return ret;
}
#endif