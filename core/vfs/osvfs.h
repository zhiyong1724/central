#ifndef __OSVFS_H__
#define __OSVFS_H__
#include "osf.h"
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct OsVFS
{
    OsFSInterfaces fs[MAX_FS_COUNT];
    uint32_t fsCount;
} OsVFS;
/*********************************************************************************************************************
* 初始化
* vfs：虚拟文件系统对象
* return：0：调用成功
*********************************************************************************************************************/
int osVFSInit(OsVFS *vfs);
/*********************************************************************************************************************
* 添加文件系统
* vfs：虚拟文件系统对象
* fs：文件系统接口
* return：0：调用成功
*********************************************************************************************************************/
int osVFSAddFS(OsVFS *vfs, OsFSInterfaces *fs);
/*********************************************************************************************************************
* 打开文件
* vfs：虚拟文件系统对象
* file：返回打开的文件对象
* path：文件路径
* mode：打开模式
* return：0：调用成功
*********************************************************************************************************************/
int osVFSOpen(OsVFS *vfs, OsFile *file, const char *path, uint32_t mode);
/*********************************************************************************************************************
* 关闭文件
* vfs：虚拟文件系统对象
* file：打开的文件对象
* return：0：调用成功
*********************************************************************************************************************/
int osVFSClose(OsVFS *vfs, OsFile *file);
/*********************************************************************************************************************
* 读文件
* vfs：虚拟文件系统对象
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际读取到的长度
* return：0：调用成功
*********************************************************************************************************************/
int osVFSRead(OsVFS *vfs, OsFile *file, void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 写文件
* vfs：虚拟文件系统对象
* file：打开的文件对象
* buff：数据buffer
* size：buffer大小
* length：实际写入的长度
* return：0：调用成功
*********************************************************************************************************************/
int osVFSWrite(OsVFS *vfs, OsFile *file, const void *buff, uint64_t size, uint64_t *length);
/*********************************************************************************************************************
* 移动文件指针
* vfs：虚拟文件系统对象
* file：打开的文件对象
* offset：偏移大小
* whence：偏移位置
* return：0：调用成功
*********************************************************************************************************************/
int osVFSSeek(OsVFS *vfs, OsFile *file, int64_t offset, OsSeekType whence);
/*********************************************************************************************************************
* 截断文件
* vfs：虚拟文件系统对象
* file：打开的文件对象
* size：截断大小
* return：0：调用成功
*********************************************************************************************************************/
int osVFSTruncate(OsVFS *vfs, OsFile *file, uint64_t size);
/*********************************************************************************************************************
* 把写入内容同步到存储器
* vfs：虚拟文件系统对象
* file：打开的文件对象
* return：0：调用成功
*********************************************************************************************************************/
int osVFSSync(OsVFS *vfs, OsFile *file);
/*********************************************************************************************************************
* 打开目录
* vfs：虚拟文件系统对象
* dir：返回打开的目录
* path：目录路径
* return：0：调用成功
*********************************************************************************************************************/
int osVFSOpenDir(OsVFS *vfs, OsDir *dir, const char *path);
/*********************************************************************************************************************
* 关闭目录
* vfs：虚拟文件系统对象
* dir：打开的目录
* return：0：调用成功
*********************************************************************************************************************/
int osVFSCloseDir(OsVFS *vfs, OsDir *dir);
/*********************************************************************************************************************
* 读取目录中的文件
* vfs：虚拟文件系统对象
* dir：打开的目录
* fileInfo：读取到的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osVFSReadDir(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 搜索第一个目录中的文件
* vfs：虚拟文件系统对象
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* path：搜索的路径
* pattern：模式字符串
* return：0：调用成功
*********************************************************************************************************************/
int osVFSFindFirst(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo, const char *path, const char *pattern);
/*********************************************************************************************************************
* 搜索下一个目录中的文件
* vfs：虚拟文件系统对象
* dir：返回打开的目录
* fileInfo：读取到的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osVFSFindNext(OsVFS *vfs, OsDir *dir, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 创建新目录
* vfs：虚拟文件系统对象
* path：目录的路径
* return：0：调用成功
*********************************************************************************************************************/
int osVFSMkdir(OsVFS *vfs, const char *path);
/*********************************************************************************************************************
* 删除目录或文件
* vfs：虚拟文件系统对象
* path：删除文件的路径
* return：0：调用成功
*********************************************************************************************************************/
int osVFSUnlink(OsVFS *vfs, const char *path);
/*********************************************************************************************************************
* 更换路径
* vfs：虚拟文件系统对象
* oldPath：旧路径
* newPath：新路径
* return：0：调用成功
*********************************************************************************************************************/
int osVFSRename(OsVFS *vfs, const char *oldPath, const char *newPath);
/*********************************************************************************************************************
* 获取文件信息
* vfs：虚拟文件系统对象
* path：文件路径
* fileInfo：返回的文件信息
* return：0：调用成功
*********************************************************************************************************************/
int osVFSStat(OsVFS *vfs, const char *path, OsFileInfo *fileInfo);
/*********************************************************************************************************************
* 修改文件权限
* vfs：虚拟文件系统对象
* path：文件路径
* attr：权限信息
* mask：要修改的权限信息掩码
* return：0：调用成功
*********************************************************************************************************************/
int osVFSChmod(OsVFS *vfs, const char *path, uint32_t attr, uint32_t mask);
/*********************************************************************************************************************
* 修改驱动路径
* vfs：虚拟文件系统对象
* path：驱动路径
* return：0：调用成功
*********************************************************************************************************************/
int osVFSChdrive(OsVFS *vfs, const char *path);
/*********************************************************************************************************************
* 获取文件系统信息
* vfs：虚拟文件系统对象
* path：驱动路径
* clusters：空闲的簇数量
* fs：文件系统信息
* return：0：调用成功
*********************************************************************************************************************/
int osVFSGetFree(OsVFS *vfs, const char *path, uint64_t *clusters, OsFS *fs);
#ifdef __cplusplus
}
#endif
#endif