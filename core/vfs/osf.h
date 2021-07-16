#ifndef __OSF_H__
#define __OSF_H__
#include <stdint.h>
#include "osdefine.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define OS_FILE_ATTR_OWNER_READ                0x00000100
#define OS_FILE_ATTR_OWNER_WRITE               0x00000080
#define OS_FILE_ATTR_OWNER_EXE                 0x00000040
#define OS_FILE_ATTR_GROUP_READ                0x00000020
#define OS_FILE_ATTR_GROUP_WRITE               0x00000010
#define OS_FILE_ATTR_GROUP_EXE                 0x00000008
#define OS_FILE_ATTR_OTHER_READ                0x00000004
#define OS_FILE_ATTR_OTHER_WRITE               0x00000002
#define OS_FILE_ATTR_OTHER_EXE                 0x00000001

#define	OS_FILE_MODE_READ				0x01
#define	OS_FILE_MODE_WRITE			    0x02
#define	OS_FILE_MODE_OPEN_EXISTING	    0x04
#define	OS_FILE_MODE_CREATE_NEW		    0x08
#define	OS_FILE_MODE_CREATE_ALWAYS	    0x10
#define	OS_FILE_MODE_OPEN_ALWAYS		0x20
#define	OS_FILE_MODE_OPEN_APPEND		0x40

typedef enum OsFileError
{
    OS_FILE_ERROR_OK,
    OS_FILE_ERROR_FS_MAX,
    OS_FILE_ERROR_MALLOC_ERR,
    OS_FILE_ERROR_DISK_ERR,
    OS_FILE_ERROR_NOT_READY,
    OS_FILE_ERROR_WRITE_PROTECTED,
    OS_FILE_ERROR_INVALID_DRIVE,
    OS_FILE_ERROR_NO_FILESYSTEM,
    OS_FILE_ERROR_NO_PAGE,
    OS_FILE_ERROR_NO_FILE,
    OS_FILE_ERROR_NO_PATH,
    OS_FILE_ERROR_INVALID_NAME,
    OS_FILE_ERROR_EXIST,
    OS_FILE_ERROR_INVALID_OBJECT,
    OS_FILE_ERROR_OTHER,
} OsFileError;

typedef enum OsFileType
{
    OS_FILE_TYPE_NORMAL,
    OS_FILE_TYPE_DIRECTORY,
    OS_FILE_TYPE_LINK,
} OsFileType;

typedef enum OsSeekType
{
    OS_SEEK_TYPE_SET,
    OS_SEEK_TYPE_CUR,
    OS_SEEK_TYPE_END,
} OsSeekType;

typedef struct OsFileInfo
{
    uint32_t type;
    uint32_t attribute;
    uint64_t createTime;
    uint64_t changeTime;
    uint64_t accessTime;
    char name[MAX_FILE_NAME_LENGTH];
    uint64_t fileSize;
} OsFileInfo;

typedef struct OsFile
{
    void *obj;
} OsFile;

typedef struct OsDir
{
    void *obj;
} OsDir;

typedef struct OsFS
{
    void *obj;
} OsFS;

typedef struct OsFSInterfaces
{
    int (*open)(OsFile *file, const char *path, uint32_t mode);
    int (*close)(OsFile *file);
    int (*read)(OsFile *file, void *buff, uint64_t size, uint64_t *length);
    int (*write)(OsFile *file, const void *buff, uint64_t size, uint64_t *length);
    int (*seek)(OsFile *file, int64_t offset, OsSeekType whence);
    int (*truncate)(OsFile *file, uint64_t size);
    int (*sync)(OsFile *file);
    int (*openDir)(OsDir *dir, const char *path);
    int (*closeDir)(OsDir *dir);
    int (*readDir)(OsDir *dir, OsFileInfo *fileInfo);
    int (*findFirst)(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
    int (*findNext)(OsDir *dir, OsFileInfo *fileInfo);
    int (*mkdir)(const char *path);
    int (*unlink)(const char *path);
    int (*rename)(const char *oldPath, const char *newPath);
    int (*stat)(const char *path, OsFileInfo *fileInfo);
    int (*chmod)(const char *path, uint32_t attr, uint32_t mask);
    int (*chdrive)(const char *path);
    int (*getFree)(const char *path, uint64_t *clusters, OsFS *fs);
} OsFSInterfaces;

/*********************************************************************************************************************
* 添加文件系统
* fs：文件系统接口
* return：0：调用成功
*********************************************************************************************************************/
int osFAddFS(OsFSInterfaces *fs);
/*********************************************************************************************************************
* 打开文件
* file：返回打开的文件对象
* path：文件路径
* mode：打开模式
* return：0：调用成功
*********************************************************************************************************************/
int osFOpen(OsFile *file, const char *path, uint32_t mode);
/*********************************************************************************************************************
* 关闭文件
* file：打开的文件对象
* return：0：调用成功
*********************************************************************************************************************/
int osFClose(OsFile *file);
/*********************************************************************************************************************
* 读文件
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际读取到的长度
* return：0：调用成功
*********************************************************************************************************************/
int osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 写文件
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际写入的长度
* return：0：调用成功
*********************************************************************************************************************/
int osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 移动文件指针
* file：打开的文件对象
* offset：偏移大小
* whence：偏移位置
* return：0：调用成功
*********************************************************************************************************************/
int osFSeek(OsFile *file, int64_t offset, OsSeekType whence);
/*********************************************************************************************************************
* 截断文件
* file：打开的文件对象
* size：截断大小
* return：0：调用成功
*********************************************************************************************************************/
int osFTruncate(OsFile *file, uint64_t size);
/*********************************************************************************************************************
* 把写入内容同步到存储器
* file：打开的文件对象
* return：0：调用成功
*********************************************************************************************************************/
int osFSync(OsFile *file);
/*********************************************************************************************************************
* 打开目录
* dir：返回打开的目录
* path：目录路径
* return：0：调用成功
*********************************************************************************************************************/
int osFOpenDir(OsDir *dir, const char *path);
/*********************************************************************************************************************
* 关闭目录
* dir：打开的目录
* return：0：调用成功
*********************************************************************************************************************/
int osFCloseDir(OsDir *dir);
/*********************************************************************************************************************
* 读取目录中的文件
* dir：打开的目录
* fileInfo：读取到的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osFReadDir(OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 搜索第一个目录中的文件
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* path：搜索的路径
* pattern：模式字符串
* return：0：调用成功
*********************************************************************************************************************/
int osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
/*********************************************************************************************************************
* 搜索下一个目录中的文件
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osFFindNext(OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 创建新目录
* path：目录的路径
* return：0：调用成功
*********************************************************************************************************************/
int osFMkdir(const char *path);
/*********************************************************************************************************************
* 删除目录或文件
* path：删除文件的路径
* return：0：调用成功
*********************************************************************************************************************/
int osFUnlink(const char *path);
/*********************************************************************************************************************
* 更换路径
* oldPath：旧路径
* newPath：新路径
* return：0：调用成功
*********************************************************************************************************************/
int osFRename(const char *oldPath, const char *newPath);
/*********************************************************************************************************************
* 获取文件信息
* path：文件路径
* fileInfo：返回的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osFStat(const char *path, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 修改文件权限
* path：文件路径
* attr：权限信息
* mask：要修改的权限信息掩码
* return：0：调用成功
*********************************************************************************************************************/
int osFChmod(const char *path, uint32_t attr, uint32_t mask);
/*********************************************************************************************************************
* 修改驱动路径
* path：驱动路径
* return：0：调用成功
*********************************************************************************************************************/
int osFChdrive(const char *path);
/*********************************************************************************************************************
* 获取文件系统信息
* path：驱动路径
* clusters：空闲的簇数量
* fs：文件系统信息
* return：0：调用成功
*********************************************************************************************************************/
int osFGetFree(const char *path, uint64_t *clusters, OsFS *fs);
#ifdef __cplusplus
}
#endif
#endif