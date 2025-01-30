#ifndef __SYS_DEVFS_H__
#define __SYS_DEVFS_H__
#include "sys_vfs.h"
#include "sys_tree.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define DEVFS_MAX_DRIVER_NAME_LEN 32
#define DEVFS_MAX_DEVICE_NAME_LEN 32

struct devfs_stat_t
{
    int64_t st_size;      //文件大小
    int st_blksize;       //块大小
    int64_t st_blocks;    //块数量
};

struct devfs_device_t
{
    sys_tree_node_t l;
    char name[DEVFS_MAX_DEVICE_NAME_LEN];
    char driver_name[DEVFS_MAX_DRIVER_NAME_LEN];
    void *data;
    int mode;
    int ref_count;
};

struct devfs_file_t;
struct devfs_operations_t
{
    int (*open)(struct devfs_file_t *file);
    int (*close)(struct devfs_file_t *file);
    int (*read)(struct devfs_file_t *file, void *buff, int count, int64_t offset);
    int (*write)(struct devfs_file_t *file, const void *buff, int count, int64_t offset);
    int64_t (*lseek)(struct devfs_file_t *file, int64_t offset, int whence);
    int (*syncfs)(struct devfs_file_t *file);
    int (*ioctl)(struct devfs_file_t *file, int cmd, int64_t arg);
    int (*stat)(struct devfs_device_t *device, struct devfs_stat_t *stat);
};

struct devfs_driver_t
{
    sys_tree_node_t l;
    char name[DEVFS_MAX_DRIVER_NAME_LEN];
    struct devfs_operations_t operations;
};

struct devfs_file_t
{
    struct devfs_device_t *device;
    struct devfs_driver_t *driver;
    int flags;
    int64_t offset;
};

struct devfs_t
{
    struct devfs_driver_t *drivers;
    struct devfs_device_t *devices;
};

/*********************************************************************************************************************
* 注册驱动程序
* driver：设备驱动
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_register_driver(struct devfs_driver_t *driver);
/*********************************************************************************************************************
* 创建设备
* driver_name：驱动名
* name：设备名
* data：设备数据
* mode：文件模式
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_create_device(const char *driver_name, const char *name, void *data, int mode);
/*********************************************************************************************************************
* 移除设备
* name：设备名
* release：释放函数
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_delete_device(const char *name);
/*********************************************************************************************************************
* 打开设备文件
* file：设备文件
* name：设备名
* flags：打开标志
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_open(struct devfs_file_t *file, const char *name, int flags);
/*********************************************************************************************************************
* 关闭设备文件
* file：设备文件
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_close(struct devfs_file_t *file);
/*********************************************************************************************************************
* 读取设备文件
* file：设备文件
* buff：数据缓存
* count：读取字节数
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_read(struct devfs_file_t *file, void *buff, int count);
/*********************************************************************************************************************
* 写设备文件
* file：设备文件
* buff：数据缓存
* count：写入字节数
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_write(struct devfs_file_t *file, const void *buff, int count);
/*********************************************************************************************************************
* 偏移设备文件
* file：设备文件
* offset：偏移值
* whence：从哪里开始偏移
* return：sys_error_t
*********************************************************************************************************************/
int64_t sys_devfs_lseek(struct devfs_file_t *file,  int64_t offset, int whence);
/*********************************************************************************************************************
* 获取文件偏移
* file：设备文件
* return：成功：文件偏移值；失败sys_error_t
*********************************************************************************************************************/
int64_t sys_devfs_ftell(struct devfs_file_t *file);
/*********************************************************************************************************************
* IO口控制
* file：设备文件
* offset：偏移值
* cmd：命令
* arg：参数
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_iostl(struct devfs_file_t *file,  int cmd, int64_t arg);
/*********************************************************************************************************************
* 把缓存数据写入到存储设备
* file：设备文件
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_syncfs(struct devfs_file_t *file);
/*********************************************************************************************************************
* 获取文件信息
* name：文件名
* stat：文件信息
* return：sys_error_t
*********************************************************************************************************************/
int sys_devfs_stat(const char *name, struct devfs_stat_t *stat);
#ifdef __cplusplus
}
#endif
#endif