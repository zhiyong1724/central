#include <stdio.h>
#include <string.h>
#include "osf.h"
#include "shell.h"
extern char gShellPathBuffer[OS_MAX_FILE_PATH_LENGTH];
static void printFSError(OsFileError error)
{
    switch (error)
    {
    case OS_FILE_ERROR_OK:
    break;
    case OS_FILE_ERROR_FS_MAX:
    break;
    case OS_FILE_ERROR_NOMEM:
    printf("没有足够的内存\n");
    break;
    case OS_FILE_ERROR_DISK_ERR:
    printf("存储设备访问失败\n");
    break;
    case OS_FILE_ERROR_CORRUPT:
    printf("块损坏\n");
    break;
    case OS_FILE_ERROR_NOT_READY:
    printf("存储设备没有准备好\n");
    break;
    case OS_FILE_ERROR_WRITE_PROTECTED:
    printf("存储设备写保护\n");
    break;
    case OS_FILE_ERROR_INVALID_DRIVE:
    printf("没有有效的驱动\n");
    break;
    case OS_FILE_ERROR_NO_FILESYSTEM:
    printf("没有文件系统\n");
    break;
    case OS_FILE_ERROR_IS_DIR:
    printf("这是一个目录\n");
    break;
    case OS_FILE_ERROR_DIR_NOTEMPTY:
    printf("目录非空\n");
    break;
    case OS_FILE_ERROR_NO_PAGE:
    printf("存储空间不足\n");
    break;
    case OS_FILE_ERROR_NO_FILE:
    printf("找不到文件或目录\n");
    break;
    case OS_FILE_ERROR_NO_PATH:
    printf("找不到路径\n");
    break;
    case OS_FILE_ERROR_INVALID_NAME:
    printf("无效的名称\n");
    break;
    case OS_FILE_ERROR_EXIST:
    printf("文件已存在\n");
    break;
    case OS_FILE_ERROR_INVALID_OBJECT:
    printf("文件对象无效\n");
    break;
    case OS_FILE_ERROR_DENIED:
    printf("访问受限\n");
    break;
    case OS_FILE_ERROR_INVALID_PARAMETER:
    printf("参数错误\n");
    break;
    case OS_FILE_ERROR_PATH_TOO_LONG:
    printf("路径太长\n");
    break;
    case OS_FILE_ERROR_FILE_TOO_LARGE:
    printf("文件太大\n");
    break;
    case OS_FILE_ERROR_NAME_TOO_LONG:
    printf("文件名太长\n");
    break;
    case OS_FILE_ERROR_NONSUPPORT:
    printf("文件系统不支持该功能\n");
    break;
    case OS_FILE_ERROR_DIFF_MOUNT:
    printf("操作失败，因为路径挂载目录不一样\n");
    break;
    case OS_FILE_ERROR_ALREADY_MOUNT:
    printf("该目录已经被挂载\n");
    break;
    case OS_FILE_ERROR_OTHER:
    printf("发生未知错误\n");
    break;    
    default:
        break;
    }
}

void shellMount(long argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFMount(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellUnmount(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFUnmount(argv[1]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellPWD(long argc, char *argv[])
{
    char buffer[OS_MAX_FILE_PATH_LENGTH];
    osFGetCWD(buffer, OS_MAX_FILE_PATH_LENGTH);
    printf("%s\n", buffer);
}

void shellCD(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFChDir(argv[1]);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            osFGetCWD(gShellPathBuffer, OS_MAX_FILE_PATH_LENGTH);
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

static void showFileInfo(OsFileInfo *fileInfo)
{
    char buff[2 * OS_MAX_FILE_NAME_LENGTH];
    buff[0] = '\0';
    if (OS_FILE_TYPE_NORMAL == fileInfo->type)
    {
        strcat(buff, "-");
    }
    else if (OS_FILE_TYPE_DIRECTORY == fileInfo->type)
    {
        strcat(buff, "d");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OWNER_READ) > 0)
    {
        strcat(buff, "r");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OWNER_WRITE) > 0)
    {
        strcat(buff, "w");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OWNER_EXE) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_GROUP_READ) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_GROUP_WRITE) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_GROUP_EXE) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OTHER_READ) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OTHER_WRITE) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    if ((fileInfo->attribute & OS_FILE_ATTR_OTHER_EXE) > 0)
    {
        strcat(buff, "x");
    }
    else
    {
        strcat(buff, "-");
    }
    char temp[64];
    sprintf(temp, " %lld %d %d %d", (unsigned long long)fileInfo->fileSize, fileInfo->changeTime.year, fileInfo->changeTime.month, fileInfo->changeTime.day);
    strcat(buff, temp);
    sprintf(temp, " %d:%d ", fileInfo->changeTime.hour, fileInfo->changeTime.minute);
    strcat(buff, temp);
    strcat(buff, fileInfo->name);
    printf("%s\n", buff);
}

void shellLS(long argc, char *argv[])
{
    char *path = "";
    if (argc >= 2)
    {
        path = argv[1];
    }
    OsDir dir;
    OsFileError result = osFOpenDir(&dir, path);
    printFSError(result);
    if (OS_FILE_ERROR_OK == result)
    {
        OsFileInfo fileInfo;
        while (osFReadDir(&dir, &fileInfo) == OS_FILE_ERROR_OK && fileInfo.name[0] != '\0')
        {
            showFileInfo(&fileInfo);
        }
        osFCloseDir(&dir);
    }
}

void shellFind(long argc, char *argv[])
{
    if (argc >= 2)
    {
        char *path = "";
        if (argc >= 3)
        {
            path = argv[2];
        }
        OsDir dir;
        OsFileInfo fileInfo;
        OsFileError result = osFFindFirst(&dir, &fileInfo, path, argv[1]);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            showFileInfo(&fileInfo);
            while (osFFindNext(&dir, &fileInfo) == OS_FILE_ERROR_OK && fileInfo.name[0] != '\0')
            {
                showFileInfo(&fileInfo);
            }
            osFCloseDir(&dir);
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

static void showFSInfo(const OsFS *fs)
{
    printf("文件系统类型：%s  ", fs->type);
    printf("页大小：%d  ", (unsigned int)fs->pageSize);
    printf("可用页：%lld  ", (unsigned long long)fs->freePages);
    printf("所有页：%lld  ", (unsigned long long)fs->totalPages);
    printf("\n");
}

void shellStatFS(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFS fs;
        OsFileError result = osFStatFS(argv[1], &fs);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            showFSInfo(&fs);
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

static void showMountInfo(const OsMountInfo *mountInfo)
{
    printf("文件系统：%s  ", mountInfo->drive);
    printf("挂载点：%s  ", mountInfo->path);
}

void shellDF(long argc, char *argv[])
{
    const OsMountInfo *mountInfo = NULL;
    while (1)
    {
        osFGetMountInfo(&mountInfo);
        if (mountInfo != NULL)
        {
            showMountInfo(mountInfo);
            OsFS fs;
            OsFileError result = osFStatFS(mountInfo->path, &fs);
            printFSError(result);
            if (OS_FILE_ERROR_OK == result)
            {
                showFSInfo(&fs);
            }
        }
        else
        {
            break;
        }
    }
}

void shellMkDir(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFMkDir(argv[1]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellTouch(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFile file;
        OsFileError result = osFOpen(&file, argv[1], OS_FILE_MODE_WRITE | OS_FILE_MODE_CREATE_ALWAYS);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            osFClose(&file);
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellChMod(long argc, char *argv[])
{
    if (argc >= 3)
    {
        uint32_t attr = 0;
        uint32_t mask = 0;
        if ('+' == argv[1][0] && 'w' == argv[1][1])
        {
            attr |= OS_FILE_ATTR_OWNER_WRITE;
            mask |= OS_FILE_ATTR_OWNER_WRITE;
        }
        else if ('-' == argv[1][0] && 'w' == argv[1][1])
        {
            attr &= ~OS_FILE_ATTR_OWNER_WRITE;
            mask |= OS_FILE_ATTR_OWNER_WRITE;
        }
        
        OsFileError result = osFChMod(argv[2], attr, mask);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellStat(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileInfo fileInfo;
        OsFileError result = osFStat(argv[1], &fileInfo);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            showFileInfo(&fileInfo);
        }
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellMV(long argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFRename(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellRM(long argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFUnlink(argv[1]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}

void shellCP(long argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFCopy(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        printf("参数不足\n");
    }
}