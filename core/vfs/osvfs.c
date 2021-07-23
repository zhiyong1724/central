#include "osvfs.h"
#include "osstring.h"
#include "osmem.h"
#define ENABLE_VFS_LOG 0
#if ENABLE_VFS_LOG
#define vfsLog(format, ...) osPrintf(format, ##__VA_ARGS__)
#else
#define vfsLog(format, ...) (void)0
#endif
#if OS_USE_VFS
static int fillPath(char *dest, os_size_t destSize, char **path, os_size_t index, char first)
{
    int ret = -1;
    if (index < destSize)
    {
        while (**path != '\0' && '/' == **path)
        {
            (*path)++;
        }
        if ((**path != '\0' && '.' == **path) && ('\0' == *(*path + 1) || '/' == *(*path + 1)))
        {
            ret = 1;
            for (; (**path != '\0' && **path != '/'); (*path)++)
            {
            }
        }
        else if ((**path != '\0' && '.' == **path) && (*(*path + 1) != '\0' && '.' == *(*path + 1)) && ('\0' == *(*path + 2) || '/' == *(*path + 2)))
        {
            ret = 2;
            for (; (**path != '\0' && **path != '/'); (*path)++)
            {
            }
        }
        else
        {
            dest[index] = '/';
            index++;
            for (; index < destSize && (**path != '\0' && **path != '/'); (*path)++, index++)
            {
                dest[index] = **path;
            }
            if (index >= destSize)
            {
                ret = -1;
            }
            else if ('\0' == **path)
            {
                dest[index] = '\0';
                ret = 0;
            }
            else
            {
                ret = 3;
            }
        }
    }
    int loop = 1;
    while (loop)
    {
        switch (ret)
        {
        case 1:
            ret = 3;
            if (first != 1)
            {
                loop = 0;
            }
            break;
        case 2:
            ret = 1;
            if (first != 1)
            {
                loop = 0;
            }
            else 
            {
                index = 0;
            }
            break;
        case 3:
            ret = fillPath(dest, destSize, path, index, 0);
            break;
        default:
            loop = 0;
            break;
        }
    }
    return ret;
}

static const char *preparePath(OsVFS *vfs, char *dest, os_size_t destSize, const char *path)
{
    const char *ret = NULL;
    if ('/' == path[0])
    {
        if (0 == fillPath(dest, destSize, (char **)&path, 0, 1))
        {
            ret = dest;
        }
    }
    else
    {
        os_size_t curLen = osStrLen(vfs->curPath);
        osStrCat(vfs->curPath, path, OS_MAX_FILE_PATH_LENGTH);
        path = vfs->curPath;
        if (0 == fillPath(dest, destSize, (char **)&path, 0, 1))
        {
            ret = dest;
        }
        vfs->curPath[curLen] = '\0';
    }
    return ret;
}

static const OsMountInfo *getMountInfo(OsVFS *vfs, const char *path)
{
    OsMountInfo *curNode = (OsMountInfo *)vfs->mountList;
    if (curNode != NULL)
    {
        do
        {
            if (osStrStr(path, curNode->path) != path)
            {
                curNode = (OsMountInfo *)vfs->mountList->nextNode;
            }
            else
            {
                break;
            }
        } while (curNode != (OsMountInfo *)vfs->mountList);
    }
    vfs->fs[curNode->fs].chdrive(curNode->drive);
    return curNode;
}

OsFileError osVFSInit(OsVFS *vfs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    vfs->fsCount = 0;
    osStrCpy(vfs->curPath, "/", OS_MAX_FILE_PATH_LENGTH);
    vfs->mountList = NULL;
    return OS_FILE_ERROR_OK;
}

OsFileError osVFSAddFS(OsVFS *vfs, OsFSInterfaces *fs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (vfs->fsCount < OS_MAX_FS_COUNT)
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
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    char *newPath = (char *)osMalloc(OS_MAX_FILE_PATH_LENGTH);
    if (newPath != NULL)
    {
        if (preparePath(vfs, newPath, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
        {
            const OsMountInfo *mountInfo = getMountInfo(vfs, newPath);
            dir->mountInfo = (void *)mountInfo;
            ret = vfs->fs[mountInfo->fs].openDir(dir, newPath);
        }
        osFree(newPath);
    }
    return ret;
}

OsFileError osVFSCloseDir(OsVFS *vfs, OsDir *dir)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)dir->mountInfo;
    return vfs->fs[mountInfo->fs].closeDir(dir);
}

OsFileError osVFSReadDir(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)dir->mountInfo;
    return vfs->fs[mountInfo->fs].readDir(dir, fileInfo);
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

OsFileError osVFSMount(OsVFS *vfs, const char *path, const char *drive)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    char *newPath = (char *)osMalloc(OS_MAX_FILE_PATH_LENGTH);
    if (newPath != NULL)
    {
        if (preparePath(vfs, newPath, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
        {
            if (osStrCmp(newPath, "/") == 0)
            {
                ret = OS_FILE_ERROR_OK;
            }
            else
            {
                OsDir dir;
                ret = osVFSOpenDir(vfs, &dir, path);
                if (OS_FILE_ERROR_OK == ret)
                {
                    ret = osVFSCloseDir(vfs, &dir);
                }
            }
            if (OS_FILE_ERROR_OK == ret)
            {
                OsMountInfo *mountInfo = (OsMountInfo *)osMalloc(sizeof(OsMountInfo));
                osAssert(mountInfo != NULL);
                if (mountInfo != NULL)
                {
                    mountInfo->path = (char *)osMalloc(osStrLen(newPath) + 1);
                    osAssert(mountInfo->path != NULL);
                    if (mountInfo->path != NULL)
                    {
                        mountInfo->drive = (char *)osMalloc(osStrLen(drive) + 1);
                        osAssert(mountInfo->drive != NULL);
                        if (mountInfo->drive != NULL)
                        {
                            osStrCpy(mountInfo->drive, drive, -1);
                            os_size_t i = 0;
                            for (; i < vfs->fsCount; i++)
                            {
                                ret = vfs->fs[i].mount(mountInfo);
                                if (OS_FILE_ERROR_OK == ret)
                                {
                                    break;
                                }
                            }
                            if (OS_FILE_ERROR_OK == ret)
                            {
                                osStrCpy(mountInfo->path, newPath, -1);
                                mountInfo->fs = i;
                                os_size_t n = 0;
                                OsMountInfo *curNode = (OsMountInfo *)vfs->mountList;
                                if (curNode != NULL)
                                {
                                    do
                                    {
                                        if (osStrCmp(mountInfo->path, curNode->path) <= 0)
                                        {
                                            n++;
                                            curNode = (OsMountInfo *)vfs->mountList->nextNode;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    } while (curNode != (OsMountInfo *)vfs->mountList);
                                }
                                osInsertToMiddle(&vfs->mountList, &mountInfo->node, n);
                            }
                            else
                            {
                                osFree(mountInfo->path);
                                osFree(mountInfo->drive);
                                osFree(mountInfo);
                            }
                        }
                        else
                        {
                            osFree(mountInfo->path);
                            osFree(mountInfo);
                            ret = OS_FILE_ERROR_MALLOC_ERR;
                        }
                    }
                    else
                    {
                        osFree(mountInfo);
                        ret = OS_FILE_ERROR_MALLOC_ERR;
                    }
                }
            }
        }
        osFree(newPath);
    }
    return ret;
}

OsFileError osVFSUnmount(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    char *newPath = (char *)osMalloc(OS_MAX_FILE_PATH_LENGTH);
    if (newPath != NULL)
    {
        if (preparePath(vfs, newPath, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
        {
            ret = OS_FILE_ERROR_NO_FILESYSTEM;
            OsMountInfo *curNode = (OsMountInfo *)vfs->mountList;
            if (curNode != NULL)
            {
                do
                {
                    if (osStrCmp(newPath, curNode->path) == 0)
                    {
                        ret = OS_FILE_ERROR_OK;
                        vfs->fs[curNode->fs].unmount(curNode);
                        osRemoveFromList(&vfs->mountList, &curNode->node);
                        osFree(curNode->path);
                        osFree(curNode->drive);
                        osFree(curNode);
                        break;
                    }
                    else
                    {
                        curNode = (OsMountInfo *)vfs->mountList->nextNode;
                    }
                } while (vfs->mountList != NULL && curNode != (OsMountInfo *)vfs->mountList);
            }
        }
        osFree(newPath);
    }
    return ret;
}

OsFileError osVFSChdir(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    char *newPath = (char *)osMalloc(OS_MAX_FILE_PATH_LENGTH);
    if (newPath != NULL)
    {
        if (preparePath(vfs, newPath, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
        {
            OsDir dir;
            ret = osVFSOpenDir(vfs, &dir, newPath);
            if (OS_FILE_ERROR_OK == ret)
            {
                osStrCpy(vfs->curPath, newPath, OS_MAX_FILE_PATH_LENGTH);
                osVFSCloseDir(vfs, &dir);
            }
        }
        osFree(newPath);
    }
    return ret;
}

const char *osVFSGetcwd(OsVFS *vfs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    return vfs->curPath;
}
#endif