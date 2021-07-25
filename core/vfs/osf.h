#ifndef __OSF_H__
#define __OSF_H__
#include <stdint.h>
#include "osdefine.h"
#include "oslist.h"
#ifdef __cplusplus
extern "C"
{
#endif
#if OS_USE_VFS
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
    OS_FILE_ERROR_NOMEM,
    OS_FILE_ERROR_DISK_ERR,
    OS_FILE_ERROR_CORRUPT,
    OS_FILE_ERROR_NOT_READY,
    OS_FILE_ERROR_WRITE_PROTECTED,
    OS_FILE_ERROR_INVALID_DRIVE,
    OS_FILE_ERROR_NO_FILESYSTEM,
    OS_FILE_ERROR_IS_DIR,
    OS_FILE_ERROR_DIR_NOTEMPTY,
    OS_FILE_ERROR_NO_PAGE,
    OS_FILE_ERROR_NO_PATH,
    OS_FILE_ERROR_NO_FILE,
    OS_FILE_ERROR_INVALID_NAME,
    OS_FILE_ERROR_EXIST,
    OS_FILE_ERROR_INVALID_OBJECT,
    OS_FILE_ERROR_DENIED,
    OS_FILE_ERROR_INVALID_PARAMETER,
    OS_FILE_ERROR_PATH_TOO_LONG,
    OS_FILE_ERROR_FILE_TOO_LARGE,
    OS_FILE_ERROR_NAME_TOO_LONG,
    OS_FILE_ERROR_NONSUPPORT,
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

typedef struct OsFileTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} OsFileTime;

typedef struct OsFileInfo
{
    OsFileType type;
    uint32_t attribute;
    OsFileTime createTime;
    OsFileTime changeTime;
    OsFileTime accessTime;
    char name[OS_MAX_FILE_NAME_LENGTH];
    uint64_t fileSize;
} OsFileInfo;

typedef struct OsFile
{
    void *obj;
    void *mountInfo;
} OsFile;

typedef struct OsDir
{
    void *obj;
    void *mountInfo;
} OsDir;

typedef struct OsMountInfo
{
    OsListNode node;
    void *obj;
    char *path;
    char *drive;
    uint32_t fs;
} OsMountInfo;

typedef struct OsFS
{
    char type[OS_MAX_FILE_NAME_LENGTH];
    uint32_t pageSize;
    uint64_t freePages;
    uint64_t totalPages;
} OsFS;

typedef struct OsFSInterfaces
{
    OsFileError (*open)(OsFile *file, const char *path, uint32_t mode);
    OsFileError (*close)(OsFile *file);
    OsFileError (*read)(OsFile *file, void *buff, uint64_t size, uint64_t *length);
    OsFileError (*write)(OsFile *file, const void *buff, uint64_t size, uint64_t *length);
    OsFileError (*seek)(OsFile *file, int64_t offset, OsSeekType whence);
    OsFileError (*tell)(OsFile *file, uint64_t *offset);
    OsFileError (*truncate)(OsFile *file, uint64_t size);
    OsFileError (*sync)(OsFile *file);
    OsFileError (*openDir)(OsDir *dir, const char *path);
    OsFileError (*closeDir)(OsDir *dir);
    OsFileError (*readDir)(OsDir *dir, OsFileInfo *fileInfo);
    OsFileError (*findFirst)(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
    OsFileError (*findNext)(OsDir *dir, OsFileInfo *fileInfo);
    OsFileError (*mkDir)(const char *path);
    OsFileError (*unlink)(const char *path);
    OsFileError (*rename)(const char *oldPath, const char *newPath);
    OsFileError (*stat)(const char *path, OsFileInfo *fileInfo);
    OsFileError (*chMod)(const char *path, uint32_t attr, uint32_t mask);
    OsFileError (*chDrive)(const char *path);
    OsFileError (*statFS)(const char *path, OsFS *fs);
    OsFileError (*mount)(OsMountInfo *mountInfo);
    OsFileError (*unmount)(OsMountInfo *mountInfo);
} OsFSInterfaces;

/*********************************************************************************************************************
* 添加文件系统
* fs：文件系统接口
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFAddFS(OsFSInterfaces *fs);
/*********************************************************************************************************************
* 打开文件
* file：返回打开的文件对象
* path：文件路径
* mode：打开模式
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFOpen(OsFile *file, const char *path, uint32_t mode);
/*********************************************************************************************************************
* 关闭文件
* file：打开的文件对象
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFClose(OsFile *file);
/*********************************************************************************************************************
* 读文件
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际读取到的长度
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFRead(OsFile *file, void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 写文件
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际写入的长度
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFWrite(OsFile *file, const void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 移动文件指针
* file：打开的文件对象
* offset：偏移大小
* whence：偏移位置
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFSeek(OsFile *file, int64_t offset, OsSeekType whence);
/*********************************************************************************************************************
* 获取当前文件偏移
* file：打开的文件对象
* offset：偏移大小
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFTell(OsFile *file, uint64_t *offset);
/*********************************************************************************************************************
* 截断文件
* file：打开的文件对象
* size：截断大小
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFTruncate(OsFile *file, uint64_t size);
/*********************************************************************************************************************
* 把写入内容同步到存储器
* file：打开的文件对象
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFSync(OsFile *file);
/*********************************************************************************************************************
* 打开目录
* dir：返回打开的目录
* path：目录路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFOpenDir(OsDir *dir, const char *path);
/*********************************************************************************************************************
* 关闭目录
* dir：打开的目录
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFCloseDir(OsDir *dir);
/*********************************************************************************************************************
* 读取目录中的文件
* dir：打开的目录
* fileInfo：读取到的文件信息
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFReadDir(OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 搜索第一个目录中的文件
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* path：搜索的路径
* pattern：模式字符串
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFFindFirst(OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
/*********************************************************************************************************************
* 搜索下一个目录中的文件
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFFindNext(OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 创建新目录
* path：目录的路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFMkDir(const char *path);
/*********************************************************************************************************************
* 删除目录或文件
* path：删除文件的路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFUnlink(const char *path);
/*********************************************************************************************************************
* 更换路径
* oldPath：旧路径
* newPath：新路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFRename(const char *oldPath, const char *newPath);
/*********************************************************************************************************************
* 获取文件信息
* path：文件路径
* fileInfo：返回的文件信息
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFStat(const char *path, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 修改文件权限
* path：文件路径
* attr：权限信息
* mask：要修改的权限信息掩码
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFChMod(const char *path, uint32_t attr, uint32_t mask);
/*********************************************************************************************************************
* 获取文件系统信息
* path：驱动路径
* fs：文件系统信息
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFStatFS(const char *path, OsFS *fs);
/*********************************************************************************************************************
* 挂载文件系统
* path：挂载路径
* drive：驱动
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFMount(const char *path, const char *drive);
/*********************************************************************************************************************
* 卸载文件系统
* path：挂载路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFUnmount(const char *path);
/*********************************************************************************************************************
* 修改当前路径
* path：当前路径
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFChDir(const char *path);
/*********************************************************************************************************************
* 获取当前路径
* buffer：指向内容返回空间
* size：buffer大小
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFGetCWD(char *buffer, uint32_t size);
/*********************************************************************************************************************
* 获取挂载信息
* mountInfo：指向内容的指针地址,第一次调用必须指向NULL，返回NULL指针表示轮询完毕
* return：OsFileError
*********************************************************************************************************************/
OsFileError osFGetMountInfo(const OsMountInfo **mountInfo);
#endif
#ifdef __cplusplus
}
#endif
#endif