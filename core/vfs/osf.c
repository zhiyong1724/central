#include "osf.h"
#include "osvfs.h"
#define ENABLE_F_LOG 0
#if ENABLE_F_LOG
#define fLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define fLog(format, ...) (void)0
#endif
static OsVFS *sVFS;
OsFileError osFInit(OsVFS *vfs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    sVFS = vfs;
    return osVFSInit(sVFS);
}

OsFileError osFAddFS(OsFSInterfaces *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSAddFS(sVFS, fs);
}

OsFileError osFOpen(OsFile *file, const char *path, uint32_t mode)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSOpen(sVFS, file, path, mode);
}

OsFileError osFClose(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSClose(sVFS, file);
}

OsFileError osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSRead(sVFS, file, buff, size, length);
}

OsFileError osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSWrite(sVFS, file, buff, size, length);
}

OsFileError osFSeek(OsFile *file, int64_t offset, OsSeekType whence)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSSeek(sVFS, file, offset, whence);
}

OsFileError osFTruncate(OsFile *file, uint64_t size)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSTruncate(sVFS, file, size);
}

OsFileError osFSync(OsFile *file)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSSync(sVFS, file);
}

OsFileError osFOpenDir(OsDir *dir, const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSOpenDir(sVFS, dir, path);
}

OsFileError osFCloseDir(OsDir *dir)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSCloseDir(sVFS, dir);
}

OsFileError osFReadDir(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSReadDir(sVFS, dir, fileInfo);
}

OsFileError osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSFindFirst(sVFS, dir, fileInfo, path, pattern);
}

OsFileError osFFindNext(OsDir *dir, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSFindNext(sVFS, dir, fileInfo);
}

OsFileError osFMkdir(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSMkdir(sVFS, path);
}

OsFileError osFUnlink(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSUnlink(sVFS, path);
}

OsFileError osFRename(const char *oldPath, const char *newPath)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSRename(sVFS, oldPath, newPath);
}

OsFileError osFStat(const char *path, OsFileInfo *fileInfo)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSStat(sVFS, path, fileInfo);
}

OsFileError osFChmod(const char *path, uint32_t attr, uint32_t mask)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSChmod(sVFS, path, attr, mask);
}

OsFileError osFChdrive(const char *path)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSChdrive(sVFS, path);
}

OsFileError osFStatfs(const char *path, OsFS *fs)
{
    fLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return osVFSStatfs(sVFS, path, fs);
}