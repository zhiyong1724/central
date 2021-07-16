#include "osf.h"
#include "osvfs.h"
#define ENABLE_F_LOG 0
#if ENABLE_F_LOG
#define fLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define fLog(format, ...) (void)0
#endif
static OsVFS *sVFS;
int osFInit(OsVFS *vfs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sVFS = vfs;
    return osVFSInit(sVFS);
}

int osFAddFS(OsFSInterfaces *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSAddFS(sVFS, fs);
}

int osFOpen(OsFile *file, const char *path, uint32_t mode)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSOpen(sVFS, file, path, mode);
}

int osFClose(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSClose(sVFS, file);
}

int osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSRead(sVFS, file, buff, size, length);
}

int osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSWrite(sVFS, file, buff, size, length);
}

int osFSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSSeek(sVFS, file, offset, whence);
}

int osFTruncate(OsFile *file, uint64_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSTruncate(sVFS, file, size);
}

int osFSync(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSSync(sVFS, file);
}

int osFOpenDir(OsDir *dir, const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSOpenDir(sVFS, dir, path);
}

int osFCloseDir(OsDir *dir)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSCloseDir(sVFS, dir);
}

int osFReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSReadDir(sVFS, dir, fileInfo);
}

int osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSFindFirst(sVFS, dir, fileInfo, path, pattern);
}

int osFFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSFindNext(sVFS, dir, fileInfo);
}

int osFMkdir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSMkdir(sVFS, path);
}

int osFUnlink(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSUnlink(sVFS, path);
}

int osFRename(const char *oldPath, const char *newPath)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSRename(sVFS, oldPath, newPath);
}

int osFStat(const char *path, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSStat(sVFS, path, fileInfo);
}

int osFChmod(const char *path, uint32_t attr, uint32_t mask)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSChmod(sVFS, path, attr, mask);
}

int osFChdrive(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSChdrive(sVFS, path);
}

int osFGetFree(const char *path, uint64_t *clusters, OsFS *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSGetFree(sVFS, path, clusters, fs);
}