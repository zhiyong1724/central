#include "shellio.h"
#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ostask.h"
#include "osf.h"
static Shell sShell;
static char sShellBuffer[1024];
static short shellRead(char *data, unsigned short len)
{
    system("stty -echo");
    system("stty -icanon");
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

int shellIOInit()
{
    sShell.read = shellRead;
    sShell.write = shellWrite;
    shellInit(&sShell, sShellBuffer, 1024);
    shellSetPath(&sShell, (char *)osFGetcwd());
    os_tid_t tid;
    osTaskCreate(&tid, (TaskFunction)shellTask, &sShell, "shell task", 0, 0);
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
    case OS_FILE_ERROR_MALLOC_ERR:
    shellWriteString(&sShell, "申请内存失败");
    break;
    case OS_FILE_ERROR_DISK_ERR:
    shellWriteString(&sShell, "存储设备访问失败");
    break;
    case OS_FILE_ERROR_NOT_READY:
    shellWriteString(&sShell, "存储设备没有准备好");
    break;
    case OS_FILE_ERROR_WRITE_PROTECTED:
    shellWriteString(&sShell, "存储设备写保护");
    break;
    case OS_FILE_ERROR_INVALID_DRIVE:
    shellWriteString(&sShell, "没有有效的驱动");
    break;
    case OS_FILE_ERROR_NO_FILESYSTEM:
    shellWriteString(&sShell, "没有文件系统");
    break;
    case OS_FILE_ERROR_NO_PAGE:
    shellWriteString(&sShell, "存储空间不足");
    break;
    case OS_FILE_ERROR_NO_FILE:
    shellWriteString(&sShell, "找不到文件或目录");
    break;
    case OS_FILE_ERROR_NO_PATH:
    shellWriteString(&sShell, "找不到路径");
    break;
    case OS_FILE_ERROR_INVALID_NAME:
    shellWriteString(&sShell, "无效的名称");
    break;
    case OS_FILE_ERROR_EXIST:
    shellWriteString(&sShell, "文件已存在");
    break;
    case OS_FILE_ERROR_INVALID_OBJECT:
    shellWriteString(&sShell, "文件对象无效");
    break;
    case OS_FILE_ERROR_DENIED:
    shellWriteString(&sShell, "访问受限");
    break;
    case OS_FILE_ERROR_INVALID_PARAMETER:
    shellWriteString(&sShell, "参数错误");
    break;
    case OS_FILE_ERROR_PATH_TOO_LONG:
    shellWriteString(&sShell, "路径太长");
    break;
    case OS_FILE_ERROR_OTHER:
    shellWriteString(&sShell, "发生未知错误");
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
        shellWriteString(&sShell, "参数不足");
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
        shellWriteString(&sShell, "参数不足");
    }
}

void shellPWD(int argc, char *argv[])
{
    shellWriteString(&sShell, osFGetcwd());
}

void shellCD(int argc, char *argv[])
{
    if (argc >= 2)
    {
        OsFileError result = osFChdir(argv[1]);
        printFSError(result);
        if (OS_FILE_ERROR_OK == result)
        {
            shellSetPath(&sShell, (char *)osFGetcwd());
        }
    }
    else
    {
        shellWriteString(&sShell, "参数不足");
    }
}

static void showFileInfo(OsFileInfo *fileInfo)
{
    static char buff[2 * OS_MAX_FILE_NAME_LENGTH];
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
    strcat(buff, " %d %ld %d %d %d:%d:%d ");
    sprintf(buff, buff, fileInfo->attribute, fileInfo->fileSize, fileInfo->changeTime.year, fileInfo->changeTime.month, fileInfo->changeTime.day, 
    fileInfo->changeTime.hour, fileInfo->changeTime.minute, fileInfo->changeTime.second);
    strcat(buff, fileInfo->name);
    shellWriteString(&sShell, buff);
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