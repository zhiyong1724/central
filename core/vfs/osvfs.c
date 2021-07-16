#include "osvfs.h"
#define ENABLE_VFS_LOG 0
#if ENABLE_VFS_LOG
#define vfsLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vfsLog(format, ...) (void)0
#endif
int osVFSInit(OsVFS *vfs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    vfs->fsCount = 0;
    return OS_FILE_ERROR_OK;
}

int osVFSAddFS(OsVFS *vfs, OsFSInterfaces *fs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (vfs->fsCount < MAX_FS_COUNT)
    {
        vfs->fs[vfs->fsCount] = *fs;
        vfs->fsCount++;
        return OS_FILE_ERROR_OK;
    }
    return OS_FILE_ERROR_FS_MAX;
}

int osVFSOpen(OsVFS *vfs, OsFile *file, const char *path, uint32_t mode)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].open(file, path, mode);
}

int osVFSClose(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].close(file);
}

int osVFSRead(OsVFS *vfs, OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].read(file, buff, size, length);
}

int osVFSWrite(OsVFS *vfs, OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].write(file, buff, size, length);
}

int osVFSSeek(OsVFS *vfs, OsFile *file, int64_t offset, OsSeekType whence)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].seek(file, offset, whence);
}

int osVFSTruncate(OsVFS *vfs, OsFile *file, uint64_t size)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].truncate(file, size);
}

int osVFSSync(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].sync(file);
}

int osVFSOpenDir(OsVFS *vfs, OsDir *dir, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].openDir(dir, path);
}

int osVFSCloseDir(OsVFS *vfs, OsDir *dir)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].closeDir(dir);
}

int osVFSReadDir(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].readDir(dir, fileInfo);
}

int osVFSFindFirst(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].findFirst(dir, fileInfo, path, pattern);
}

int osVFSFindNext(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].findNext(dir, fileInfo);
}

int osVFSMkdir(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].mkdir(path);
}

int osVFSUnlink(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].unlink(path);
}

int osVFSRename(OsVFS *vfs, const char *oldPath, const char *newPath)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].rename(oldPath, newPath);
}

int osVFSStat(OsVFS *vfs, const char *path, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].stat(path, fileInfo);
}

int osVFSChmod(OsVFS *vfs, const char *path, uint32_t attr, uint32_t mask)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].chmod(path, attr, mask);
}

int osVFSChdrive(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].chdrive(path);
}

int osVFSGetFree(OsVFS *vfs, const char *path, uint64_t *clusters, OsFS *fs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].getFree(path, clusters, fs);
}