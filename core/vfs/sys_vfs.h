#ifndef __SYS_VFS_H__
#define __SYS_VFS_H__
#include "sys_cfg.h"
#include "sys_list.h"
#include "sys_tree.h"
#include "sys_vector.h"
#include "sys_lock.h"
#include <sys/statfs.h>
#ifdef __cplusplus
extern "C"
{
#endif
#define VFS_MAX_FS_NAME_LEN 16

enum vfs_mode_t
{
    VFS_MODE_ISDIR = (1 << 9),         //目录文件
    VFS_MODE_ISCHR = (2 << 9),         //字符文件
    VFS_MODE_ISBLK = (3 << 9),         //块文件
    VFS_MODE_ISREG = (4 << 9),         //普通文件
    VFS_MODE_ISLNK = (5 << 9),         //链接文件
    VFS_MODE_IRUSR = (1 << 8),         //读权限，拥有者
    VFS_MODE_IWUSR = (1 << 7),         //写权限，拥有者
    VFS_MODE_IXUSR = (1 << 6),         //执行权限，拥有者
    VFS_MODE_IRGRP = (1 << 5),         //读权限，拥有组
    VFS_MODE_IWGRP = (1 << 4),         //写权限，拥有组
    VFS_MODE_IXGRP = (1 << 3),         //执行权限，拥有组
    VFS_MODE_IROTH = (1 << 2),         //读权限，其他人
    VFS_MODE_IWOTH = (1 << 1),         //写权限，其他人
    VFS_MODE_IXOTH = (1 << 0),         //执行权限，其他人
};

enum vfs_flags_t
{
    VFS_FLAG_ACCMODE = 03,             //读写模式掩码
    VFS_FLAG_RDONLY = 00,              //只读模式
    VFS_FLAG_WRONLY = 01,              //只写模式
    VFS_FLAG_RDWR = 02,                //读写模式
    VFS_FLAG_CREAT = 0100,             //创建新文件
    VFS_FLAG_EXCL = 0200,              //和VFS_FLAG_CREAT一起使用，如果文件存在会返回失败
    VFS_FLAG_NOCTTY = 0400,            //不作为终端
    VFS_FLAG_TRUNC = 01000,            //文件截断
    VFS_FLAG_APPEND = 02000,           //添加新内容
    VFS_FLAG_NONBLOCK = 04000,         //非阻塞模式
    VFS_FLAG_SYNC = 04010000,          //文件同步
};

enum vfs_seek_t
{
    VFS_SEEK_SET = 0,                  //从开始位置seek
    VFS_SEEK_CUR = 1,                  //从当前位置seek
    VFS_SEEK_END = 2,                  //从末尾位置seek
};

struct vfs_stat_t
{
    int st_dev;           //设备id
    int st_ino;           //inode号
    int st_mode;          //文件属性
    int st_nlink;         //硬链接数
    int64_t st_size;      //文件大小
    int st_blksize;       //块大小
    int64_t st_blocks;    //块数量
    int64_t st_atim;      //最后访问时间
    int64_t st_mtim;      //最后修改时间
    int64_t st_ctim;      //最后一次状态改变时间
};

struct vfs_statfs_t
{
    int f_type;        // 文件系统类型
    int f_bsize;       // 块大小
    int64_t f_blocks;  // 文件系统数据块总数
    int64_t f_bfree;   // 可用块数
    int64_t f_bavail;  // 非超级用户可获取的块数
    int64_t f_files;   // 文件结点总数
    int64_t f_ffree;   // 可用文件结点数
    int f_fsid;        // 文件系统标识
    int f_namelen;     // 文件名的最大长度
    int f_frsize;      // 片段大小
    int f_flags;       // 文件系统安装标志
};

struct vfs_dirent_t
{
    int d_ino;                             // 文件编号
    int d_off;                             // 目录文件偏移
    int d_reclen;                          // 目录项长度
    int d_type;                            // 文件类型
    char d_name[VFS_MAX_FILE_NAME_LEN];    // 文件名
};

struct vfs_super_block_t;
struct vfs_file_t
{
    void *obj;
    int flags;
    struct vfs_super_block_t *super_block;
    sys_mutex_t lock;
};

struct vfs_fs_operations_t
{
    int (*mount)(struct vfs_super_block_t *super_block, const char *path, const char *device);
    int (*unmount)(struct vfs_super_block_t *super_block);
    int (*statfs)(struct vfs_super_block_t *super_block, const char *path, struct vfs_statfs_t *statfs);
};

struct vfs_node_operations_t
{
    int (*stat)(struct vfs_super_block_t *super_block, const char *path, struct vfs_stat_t *stat);
    int (*link)(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath);
    int (*unlink)(struct vfs_super_block_t *super_block, const char *path);
    int (*chmod)(struct vfs_super_block_t *super_block, const char *path, int mode);
    int (*rename)(struct vfs_super_block_t *super_block, const char *oldpath, const char *newpath);
    int (*mkdir)(struct vfs_super_block_t *super_block, const char *path, int mode);
    int (*rmdir)(struct vfs_super_block_t *super_block, const char *path);
    int (*opendir)(struct vfs_file_t *file, const char *path);
    int (*closedir)(struct vfs_file_t *file);
    int (*readdir)(struct vfs_file_t *file, struct vfs_dirent_t *dirent);
    int (*rewinddir)(struct vfs_file_t *file);
};

struct vfs_file_operations_t
{
    int (*open)(struct vfs_file_t *file, const char *path, int mode);
    int (*close)(struct vfs_file_t *file);
    int (*read)(struct vfs_file_t *file, void *buff, int count);
    int (*write)(struct vfs_file_t *file, const void *buff, int count);
    int (*lseek)(struct vfs_file_t *file, int64_t offset, int whence);
    int64_t (*ftell)(struct vfs_file_t *file);
    int (*syncfs)(struct vfs_file_t *file);
    int (*ftruncate)(struct vfs_file_t *file, int64_t length);
};

struct vfs_node_t
{
    struct vfs_node_operations_t node_operations;
    struct vfs_file_operations_t file_operations;
};

struct vfs_fs_t;
struct vfs_super_block_t
{
    //用户填充
    struct vfs_fs_operations_t fs_operations;
    struct vfs_node_t node;
    int64_t block_size;
    int64_t block_count;
    void *obj;
    char device[VFS_MAX_FILE_PATH_LEN];
    //内部使用
    struct vfs_fs_t *fs;
    int ref_count;
};

struct vfs_fs_t
{
    sys_single_list_node_t l;
    char name[VFS_MAX_FS_NAME_LEN];
    void (*init_super_block)(struct vfs_super_block_t *super_block, const char *device);
    void (*release)(struct vfs_super_block_t *super_block);
    int flags;
};

struct vfs_dentry_t
{
    sys_tree_node_t l;
    struct vfs_dentry_t *parent;
    struct vfs_dentry_t *children;
    char name[VFS_MAX_FILE_NAME_LEN];
    char dir[VFS_MAX_FILE_PATH_LEN];
    struct vfs_super_block_t *super_block;
};
/*********************************************************************************************************************
* 添加文件系统
* fs：文件系统接口对象
* return：sys_error_t
*********************************************************************************************************************/
int sys_registerfs(struct vfs_fs_t *fs);
/*********************************************************************************************************************
* 挂载文件系统
* path：挂载路径
* device：块设备
* return：sys_error_t
*********************************************************************************************************************/
int sys_mount(const char *path, const char *device);
/*********************************************************************************************************************
* 卸载文件系统
* path：挂载路径
* return：sys_error_t
*********************************************************************************************************************/
int sys_umount(const char *path);
/*********************************************************************************************************************
* 打开文件
* path：文件路径
* flags：打开标记
* mode：打开模式
* return：成功：文件fd；失败：sys_error_t
*********************************************************************************************************************/
int sys_open(const char *path, int flags, int mode);
/*********************************************************************************************************************
* 关闭文件
* fd：文件句柄
* return：sys_error_t
*********************************************************************************************************************/
int sys_close(int fd);
/*********************************************************************************************************************
* 读取文件
* fd：文件句柄
* buff：数据缓存
* count：读取字节数
* return：成功：读取字节数；失败sys_error_t
*********************************************************************************************************************/
int sys_read(int fd, void *buff, int count);
/*********************************************************************************************************************
* 写入文件
* fd：文件句柄
* buff：数据缓存
* count：写入字节数
* return：成功：写入字节数；失败sys_error_t
*********************************************************************************************************************/
int sys_write(int fd, const void *buff, int count);
/*********************************************************************************************************************
* 偏移文件
* fd：文件句柄
* offset：偏移值
* whence：从哪里开始偏移
* return：成功：文件偏移值；失败sys_error_t
*********************************************************************************************************************/
int64_t sys_lseek(int fd, int64_t offset, int whence);
/*********************************************************************************************************************
* 获取文件偏移
* fd：文件句柄
* return：成功：文件偏移值；失败sys_error_t
*********************************************************************************************************************/
int64_t sys_ftell(int fd);
/*********************************************************************************************************************
* 把缓存数据写入到存储设备
* fd：文件句柄
* return：sys_error_t
*********************************************************************************************************************/
int sys_syncfs(int fd);
/*********************************************************************************************************************
* 截断文件
* fd：文件句柄
* length：从哪里开始截断
* return：sys_error_t
*********************************************************************************************************************/
int sys_ftruncate(int fd, int64_t length);
/*********************************************************************************************************************
* 获取文件信息
* path：文件路径
* stat：返回的文件信息
* return：sys_error_t
*********************************************************************************************************************/
int sys_stat(const char *path, struct vfs_stat_t *stat);
/*********************************************************************************************************************
* 创建硬链接
* oldpath：旧的文件路径
* newpath：新链接文件路径
* return：sys_error_t
*********************************************************************************************************************/
int sys_link(const char *oldpath, const char *newpath);
/*********************************************************************************************************************
* 删除文件
* path：文件路径
* return：sys_error_t
*********************************************************************************************************************/
int sys_unlink(const char *path);
/*********************************************************************************************************************
* 修改文件权限
* path：文件路径
* mode：文件模式
* return：sys_error_t
*********************************************************************************************************************/
int sys_chmod(const char *path, int mode);
/*********************************************************************************************************************
* 文件重命名
* oldpath：旧的文件路径
* newpath：新链接文件路径
* return：sys_error_t
*********************************************************************************************************************/
int sys_rename(const char *oldpath, const char *newpath);
/*********************************************************************************************************************
* 创建目录
* path：文件路径
* mode：文件模式
* return：sys_error_t
*********************************************************************************************************************/
int sys_mkdir(const char *path, int mode);
/*********************************************************************************************************************
* 删除目录
* path：文件路径
* return：sys_error_t
*********************************************************************************************************************/
int sys_rmdir(const char *path);
/*********************************************************************************************************************
* 打开目录
* path：目录路径
* return：成功：文件fd；失败：sys_error_t
*********************************************************************************************************************/
int sys_opendir(const char *path);
/*********************************************************************************************************************
* 关闭目录
* fd：文件句柄
* return：sys_error_t
*********************************************************************************************************************/
int sys_closedir(int fd);
/*********************************************************************************************************************
* 读取目录
* fd：文件句柄
* dirent：返回的目录信息
* return：sys_error_t
*********************************************************************************************************************/
int sys_readdir(int fd, struct vfs_dirent_t *dirent);
/*********************************************************************************************************************
* 重置目录对象
* fd：文件句柄
* return：sys_error_t
*********************************************************************************************************************/
int sys_rewinddir(int fd);
/*********************************************************************************************************************
* 获取文件系统信息
* path：目录路径
* statfs：文件系统信息
* return：sys_error_t
*********************************************************************************************************************/
int sys_statfs(const char *path, struct vfs_statfs_t *statfs);
#ifdef __cplusplus
}
#endif
#endif