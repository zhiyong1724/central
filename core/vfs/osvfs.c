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
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
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
            if (index - 1 > 0 && '/' == dest[index - 1])
            {
                index--;
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
            else
            {
                index = 0;
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
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
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
        osStrCat(vfs->curPath, "/", OS_MAX_FILE_PATH_LENGTH);
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
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
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
    vfs->fs[curNode->fs].chDriver(curNode->driver);
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
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }
        
        file->mountInfo = (void *)mountInfo;
        ret = vfs->fs[mountInfo->fs].open(file, vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", mode);
    }
    return ret;
}

OsFileError osVFSClose(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].close(file);
}

OsFileError osVFSRead(OsVFS *vfs, OsFile *file, void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].read(file, buff, size, length);
}

OsFileError osVFSWrite(OsVFS *vfs, OsFile *file, const void *buff, uint64_t size, uint64_t *length)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].write(file, buff, size, length);
}

OsFileError osVFSSeek(OsVFS *vfs, OsFile *file, int64_t offset, OsSeekType whence)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].seek(file, offset, whence);
}

OsFileError osVFSTell(OsVFS *vfs, OsFile *file, uint64_t *offset)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].tell(file, offset);
}

OsFileError osVFSTruncate(OsVFS *vfs, OsFile *file, uint64_t size)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].truncate(file, size);
}

OsFileError osVFSSync(OsVFS *vfs, OsFile *file)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)file->mountInfo;
    return vfs->fs[mountInfo->fs].sync(file);
}

OsFileError osVFSOpenDir(OsVFS *vfs, OsDir *dir, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }

        dir->mountInfo = (void *)mountInfo;
        ret = vfs->fs[mountInfo->fs].openDir(dir, vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/");
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
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }

        dir->mountInfo = (void *)mountInfo;
        ret = vfs->fs[mountInfo->fs].findFirst(dir, fileInfo, vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", pattern);
    }
    return ret;
}

OsFileError osVFSFindNext(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    const OsMountInfo *mountInfo = (OsMountInfo *)dir->mountInfo;
    return vfs->fs[mountInfo->fs].findNext(dir, fileInfo);
}

OsFileError osVFSMkDir(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }

        ret = vfs->fs[mountInfo->fs].mkDir(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/");
    }
    return ret;
}

OsFileError osVFSUnlink(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }
        ret = vfs->fs[mountInfo->fs].unlink(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/");
    }
    return ret;
}

OsFileError osVFSRename(OsVFS *vfs, const char *oldPath, const char *newPath)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, oldPath) != NULL && preparePath(vfs, vfs->pathB, OS_MAX_FILE_PATH_LENGTH, newPath) != NULL)
    {
        ret = OS_FILE_ERROR_DIFF_MOUNT;
        const OsMountInfo *mountInfoA = getMountInfo(vfs, vfs->pathA);
        const OsMountInfo *mountInfoB = getMountInfo(vfs, vfs->pathB);
        if (mountInfoA == mountInfoB)
        {
            os_size_t index = 0;
            if (mountInfoA->fs > 0)
            {
                index = osStrLen(mountInfoA->path);
            }
            ret = vfs->fs[mountInfoA->fs].rename(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", vfs->pathB[index] != '\0' ? &vfs->pathB[index] : "/");
        }
    }
    return ret;
}

OsFileError osVFSStat(OsVFS *vfs, const char *path, OsFileInfo *fileInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }
        ret = vfs->fs[mountInfo->fs].stat(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", fileInfo);
    }
    return ret;
}

OsFileError osVFSChMod(OsVFS *vfs, const char *path, uint32_t attr, uint32_t mask)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }
        ret = vfs->fs[mountInfo->fs].chMod(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", attr, mask);
    }
    return ret;
}

OsFileError osVFSStatFS(OsVFS *vfs, const char *path, OsFS *fs)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        const OsMountInfo *mountInfo = getMountInfo(vfs, vfs->pathA);
        os_size_t index = 0;
        if (mountInfo->fs > 0)
        {
            index = osStrLen(mountInfo->path);
        }
        ret = vfs->fs[mountInfo->fs].statFS(vfs->pathA[index] != '\0' ? &vfs->pathA[index] : "/", fs);
    }
    return ret;
}

static OsFileError checkMount(OsVFS *vfs, const char *path)
{
    OsFileError ret = OS_FILE_ERROR_OK;
    OsMountInfo *curNode = (OsMountInfo *)vfs->mountList;
    if (curNode != NULL)
    {
        do
        {
            if (osStrCmp(path, curNode->path) == 0)
            {
                ret = OS_FILE_ERROR_ALREADY_MOUNT;
                break;
            }
            else
            {
                curNode = (OsMountInfo *)vfs->mountList->nextNode;
            }
        } while (curNode != (OsMountInfo *)vfs->mountList);
    }
    return ret;
}

OsFileError osVFSMount(OsVFS *vfs, const char *path, const char *driver)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        ret = checkMount(vfs, vfs->pathA);
        if (OS_FILE_ERROR_OK == ret)
        {
            if (osStrCmp(vfs->pathA, "/") == 0)
            {
                ret = OS_FILE_ERROR_OK;
            }
            else
            {
                OsDir dir;
                ret = osVFSOpenDir(vfs, &dir, vfs->pathA);
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
                    mountInfo->path = (char *)osMalloc(osStrLen(vfs->pathA) + 1);
                    osAssert(mountInfo->path != NULL);
                    if (mountInfo->path != NULL)
                    {
                        mountInfo->driver = (char *)osMalloc(osStrLen(driver) + 1);
                        osAssert(mountInfo->driver != NULL);
                        if (mountInfo->driver != NULL)
                        {
                            osStrCpy(mountInfo->driver, driver, -1);
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
                                osStrCpy(mountInfo->path, vfs->pathA, -1);
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
                                osFree(mountInfo->driver);
                                osFree(mountInfo);
                            }
                        }
                        else
                        {
                            osFree(mountInfo->path);
                            osFree(mountInfo);
                            ret = OS_FILE_ERROR_NOMEM;
                        }
                    }
                    else
                    {
                        osFree(mountInfo);
                        ret = OS_FILE_ERROR_NOMEM;
                    }
                }
            }
        }
    }
    return ret;
}

OsFileError osVFSUnmount(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        ret = OS_FILE_ERROR_NO_FILESYSTEM;
        OsMountInfo *curNode = (OsMountInfo *)vfs->mountList;
        if (curNode != NULL)
        {
            do
            {
                if (osStrCmp(vfs->pathA, curNode->path) == 0)
                {
                    ret = OS_FILE_ERROR_OK;
                    vfs->fs[curNode->fs].unmount(curNode);
                    osRemoveFromList(&vfs->mountList, &curNode->node);
                    osFree(curNode->path);
                    osFree(curNode->driver);
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
    return ret;
}

OsFileError osVFSChDir(OsVFS *vfs, const char *path)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    OsFileError ret = OS_FILE_ERROR_PATH_TOO_LONG;
    if (preparePath(vfs, vfs->pathA, OS_MAX_FILE_PATH_LENGTH, path) != NULL)
    {
        OsDir dir;
        ret = osVFSOpenDir(vfs, &dir, vfs->pathA);
        if (OS_FILE_ERROR_OK == ret)
        {
            osStrCpy(vfs->curPath, vfs->pathA, OS_MAX_FILE_PATH_LENGTH);
            osVFSCloseDir(vfs, &dir);
        }
    }
    return ret;
}

OsFileError osVFSGetCWD(OsVFS *vfs, char *buffer, uint32_t size)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    osStrCpy(buffer, vfs->curPath, size);
    return OS_FILE_ERROR_OK;
}

OsFileError osVFSGetMountInfo(OsVFS *vfs, const OsMountInfo **mountInfo)
{
    vfsLog("%s:%s:%d\n", __FILE__, __func__, __LINE__);
    if (NULL == *mountInfo)
    {
        *mountInfo = (OsMountInfo *)vfs->mountList;
    }
    else
    {
        if ((*mountInfo)->node.nextNode != vfs->mountList)
        {
            *mountInfo = (OsMountInfo *)(*mountInfo)->node.nextNode;
        }
        else 
        {
            *mountInfo = NULL;
        }
    }
    return OS_FILE_ERROR_OK;
}
#endif