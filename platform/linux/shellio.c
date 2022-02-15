#include "shellio.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ostask.h"
#include "osf.h"
#include "osmem.h"
static Shell sShell;
static char sShellBuffer[1024];
static char sShellPathBuffer[OS_MAX_FILE_PATH_LENGTH];
static short shellRead(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        int ret = getchar();
        if (ret != EOF)
        {
            data[i] = (char)ret;
        }
        else
        {
            break;
        }
    }
    return len;
}

static short shellWrite(char *data, unsigned short len)
{
    for (unsigned short i = 0; i < len; i++)
    {
        putchar(data[i]);
    }
    return 0;
}

static void *_shellTask(void *arg)
{
    while (1)
    {
        shellTask(arg);
        osTaskSleep(50);
    }
    return NULL;
}

int shellIOInit()
{
    sShell.read = shellRead;
    sShell.write = shellWrite;
    shellInit(&sShell, sShellBuffer, 1024);
    osFGetCWD(sShellPathBuffer, OS_MAX_FILE_PATH_LENGTH);
    shellSetPath(&sShell, sShellPathBuffer);
    os_tid_t tid;
    osTaskCreate(&tid, _shellTask, &sShell, "shell", 0, 0);
    system("stty -echo");
    system("stty -icanon");
    return 0;
}

static void printFSError(OsFileError error)
{
    switch (error)
    {
    case OS_FILE_ERROR_OK:
    break;
    case OS_FILE_ERROR_FS_MAX:
    break;
    case OS_FILE_ERROR_NOMEM:
    shellWriteString(&sShell, "没有足够的内存\n");
    break;
    case OS_FILE_ERROR_DISK_ERR:
    shellWriteString(&sShell, "存储设备访问失败\n");
    break;
    case OS_FILE_ERROR_CORRUPT:
    shellWriteString(&sShell, "块损坏\n");
    break;
    case OS_FILE_ERROR_NOT_READY:
    shellWriteString(&sShell, "存储设备没有准备好\n");
    break;
    case OS_FILE_ERROR_WRITE_PROTECTED:
    shellWriteString(&sShell, "存储设备写保护\n");
    break;
    case OS_FILE_ERROR_INVALID_DRIVE:
    shellWriteString(&sShell, "没有有效的驱动\n");
    break;
    case OS_FILE_ERROR_NO_FILESYSTEM:
    shellWriteString(&sShell, "没有文件系统\n");
    break;
    case OS_FILE_ERROR_IS_DIR:
    shellWriteString(&sShell, "这是一个目录\n");
    break;
    case OS_FILE_ERROR_DIR_NOTEMPTY:
    shellWriteString(&sShell, "目录非空\n");
    break;
    case OS_FILE_ERROR_NO_PAGE:
    shellWriteString(&sShell, "存储空间不足\n");
    break;
    case OS_FILE_ERROR_NO_FILE:
    shellWriteString(&sShell, "找不到文件或目录\n");
    break;
    case OS_FILE_ERROR_NO_PATH:
    shellWriteString(&sShell, "找不到路径\n");
    break;
    case OS_FILE_ERROR_INVALID_NAME:
    shellWriteString(&sShell, "无效的名称\n");
    break;
    case OS_FILE_ERROR_EXIST:
    shellWriteString(&sShell, "文件已存在\n");
    break;
    case OS_FILE_ERROR_INVALID_OBJECT:
    shellWriteString(&sShell, "文件对象无效\n");
    break;
    case OS_FILE_ERROR_DENIED:
    shellWriteString(&sShell, "访问受限\n");
    break;
    case OS_FILE_ERROR_INVALID_PARAMETER:
    shellWriteString(&sShell, "参数错误\n");
    break;
    case OS_FILE_ERROR_PATH_TOO_LONG:
    shellWriteString(&sShell, "路径太长\n");
    break;
    case OS_FILE_ERROR_FILE_TOO_LARGE:
    shellWriteString(&sShell, "文件太大\n");
    break;
    case OS_FILE_ERROR_NAME_TOO_LONG:
    shellWriteString(&sShell, "文件名太长\n");
    break;
    case OS_FILE_ERROR_NONSUPPORT:
    shellWriteString(&sShell, "文件系统不支持该功能\n");
    break;
    case OS_FILE_ERROR_DIFF_MOUNT:
    shellWriteString(&sShell, "操作失败，因为路径挂载目录不一样\n");
    break;
    case OS_FILE_ERROR_ALREADY_MOUNT:
    shellWriteString(&sShell, "该目录已经被挂载\n");
    break;
    case OS_FILE_ERROR_OTHER:
    shellWriteString(&sShell, "发生未知错误\n");
    break;    
    default:
        break;
    }
}

void shellMount(int argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFMount(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellUnmount(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFUnmount(argv[1]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellPWD(int argc, char *argv[])
{
    char buffer[OS_MAX_FILE_PATH_LENGTH];
    osFGetCWD(buffer, OS_MAX_FILE_PATH_LENGTH);
    shellWriteString(&sShell, buffer);
}

void shellCD(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFChDir(argv[1]);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            osFGetCWD(sShellPathBuffer, OS_MAX_FILE_PATH_LENGTH);
            shellSetPath(&sShell, sShellPathBuffer);
        }
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
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
    sprintf(temp, " %lld %d %d %d", fileInfo->fileSize, fileInfo->changeTime.year, fileInfo->changeTime.month, fileInfo->changeTime.day);
    strcat(buff, temp);
    sprintf(temp, " %d:%d ", fileInfo->changeTime.hour, fileInfo->changeTime.minute);
    strcat(buff, temp);
    strcat(buff, fileInfo->name);
    shellWriteString(&sShell, buff);
    shellWriteString(&sShell, "\n");
}

void shellLS(int argc, char *argv[])
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

void shellFind(int argc, char *argv[])
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
        shellWriteString(&sShell, "参数不足\n");
    }
}

static void showFSInfo(const OsFS *fs)
{
    shellPrint(&sShell, "文件系统类型：%s  ", fs->type);
    shellPrint(&sShell, "页大小：%d  ", fs->pageSize);
    shellPrint(&sShell, "可用页：%lld  ", fs->freePages);
    shellPrint(&sShell, "所有页：%lld  ", fs->totalPages);
}

void shellStatFS(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFS fs;
        OsFileError result = osFStatFS(argv[1], &fs);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            showFSInfo(&fs);
            shellPrint(&sShell, "\n");
        }
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

static void showMountInfo(const OsMountInfo *mountInfo)
{
    shellPrint(&sShell, "文件系统：%s  ", mountInfo->drive);
    shellPrint(&sShell, "挂载点：%s  ", mountInfo->path);
}

void shellDF(int argc, char *argv[])
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
            shellPrint(&sShell, "\n");
        }
        else
        {
            break;
        }
    }
}

void shellMkDir(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFMkDir(argv[1]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellTouch(int argc, char *argv[])
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
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellChMod(int argc, char *argv[])
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
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellStat(int argc, char *argv[])
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
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellMV(int argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFRename(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellRM(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFUnlink(argv[1]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellCP(int argc, char *argv[])
{
    if (argc >= 3)
    {
        OsFileError result = osFCopy(argv[1], argv[2]);
        printFSError(result);
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellFree(int argc, char *argv[])
{
    shellPrint(&sShell, "所有内存：%ld\n", osTotalMem());
    shellPrint(&sShell, "可用内存：%ld\n", osFreeMem());
    shellPrint(&sShell, "所有页：%ld\n", osTotalPage());
    shellPrint(&sShell, "可用页：%ld\n", osFreePage());
}

void shellPlay(int argc, char *argv[])
{
    if (argc >= 2)
    {
        if (strcmp(argv[1], "stop") == 0)
        {
        }
        else
        {
        }
    }
    else
    {
        shellWriteString(&sShell, "参数不足\n");
    }
}

void shellPS(int argc, char *argv[])
{
    os_task_ptr ptr;
    OsTaskInfo taskInfo;
    int result = osTaskFindFirst(&ptr, &taskInfo);
    while (0 == result)
    {
        shellPrint(&sShell, "tid: %ld  ptid: %ld  stack: 0x%x  stack size: %ld  state: %d  type: %d  priority: %ld  name: %s\n",
                   taskInfo.tid, taskInfo.ptid, taskInfo.stack, taskInfo.stackSize, taskInfo.taskState, taskInfo.taskType, taskInfo.priority, taskInfo.name);
        result = osTaskFindNext(&ptr, &taskInfo);
    }
}