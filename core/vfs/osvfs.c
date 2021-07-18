#include "osvfs.h"
#define ENABLE_VFS_LOG 0
#if ENABLE_VFS_LOG
#define vfsLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vfsLog(format, ...) (void)0
#endif
OsFileError osVFSInit(OsVFS *vfs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    vfs->fsCount = 0;
    return OS_FILE_ERROR_OK;
}

OsFileError osVFSAddFS(OsVFS *vfs, OsFSInterfaces *fs)
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

OsFileError osVFSOpen(OsVFS *vfs, OsFile *file, const char *path, uint32_t mode)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].open(file, path, mode);
}

OsFileError osVFSClose(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].close(file);
}

OsFileError osVFSRead(OsVFS *vfs, OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].read(file, buff, size, length);
}

OsFileError osVFSWrite(OsVFS *vfs, OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].write(file, buff, size, length);
}

OsFileError osVFSSeek(OsVFS *vfs, OsFile *file, int64_t offset, OsSeekType whence)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].seek(file, offset, whence);
}

OsFileError osVFSTruncate(OsVFS *vfs, OsFile *file, uint64_t size)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].truncate(file, size);
}

OsFileError osVFSSync(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].sync(file);
}

OsFileError osVFSOpenDir(OsVFS *vfs, OsDir *dir, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].openDir(dir, path);
}

OsFileError osVFSCloseDir(OsVFS *vfs, OsDir *dir)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].closeDir(dir);
}

OsFileError osVFSReadDir(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].readDir(dir, fileInfo);
}

OsFileError osVFSFindFirst(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].findFirst(dir, fileInfo, path, pattern);
}

OsFileError osVFSFindNext(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].findNext(dir, fileInfo);
}

OsFileError osVFSMkdir(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].mkdir(path);
}

OsFileError osVFSUnlink(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].unlink(path);
}

OsFileError osVFSRename(OsVFS *vfs, const char *oldPath, const char *newPath)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].rename(oldPath, newPath);
}

OsFileError osVFSStat(OsVFS *vfs, const char *path, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].stat(path, fileInfo);
}

OsFileError osVFSChmod(OsVFS *vfs, const char *path, uint32_t attr, uint32_t mask)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].chmod(path, attr, mask);
}

OsFileError osVFSChdrive(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].chdrive(path);
}

OsFileError osVFSStatfs(OsVFS *vfs, const char *path, OsFS *fs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->fs[0].statfs(path, fs);
}