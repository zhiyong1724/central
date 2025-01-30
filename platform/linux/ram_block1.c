#include "ram_block1.h"
#include "ff.h"
#include "diskio.h"
#include <string.h>
#include "sys_devfs.h"
#include "sys_vfs.h"
#include "lfs_adapter.h"
#include "sys_error.h"
#define SECTOR_SIZE 512
#define SECTOR_COUNT 4096
static int s_ram_buffer[SECTOR_SIZE * SECTOR_COUNT / sizeof(int)];
#define DRIVER_NAME "ram_block1_driver"
DSTATUS RAM_disk_status()
{
    DSTATUS ret = RES_OK;
    return ret;
}

DSTATUS RAM_disk_initialize()
{
    DSTATUS ret = RES_OK;
    return ret;
}

DRESULT RAM_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    DRESULT ret = RES_OK;
    int fd = sys_open("/dev/block1", VFS_FLAG_RDONLY, 0);
    if (fd < 0)
    {
        return RES_NOTRDY;
    }
    sys_lseek(fd, sector * SECTOR_SIZE, VFS_SEEK_SET);
    int res = sys_read(fd, buff, count * SECTOR_SIZE);
    if (res < 0)
        ret = RES_ERROR;
    else
        ret = RES_OK;
    sys_close(fd);
    return ret;
}

DRESULT RAM_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    DRESULT ret = RES_OK;
    int fd = sys_open("/dev/block1", VFS_FLAG_WRONLY, 0);
    if (fd < 0)
    {
        return RES_NOTRDY;
    }
    sys_lseek(fd, sector * SECTOR_SIZE, VFS_SEEK_SET);
    int res = sys_write(fd, buff, count * SECTOR_SIZE);
    if (res < 0)
        ret = RES_ERROR;
    else
        ret = RES_OK;
    sys_close(fd);
    return ret;
}

DRESULT RAM_disk_ioctl(BYTE cmd, void *buff)
{
    DRESULT ret = RES_OK;
    switch (cmd)
    {
    case CTRL_SYNC:
        /* code */
        break;
    case GET_SECTOR_COUNT:
        *((LBA_t *)buff) = SECTOR_COUNT;
        break;
    case GET_SECTOR_SIZE:
        *((DWORD *)buff) = SECTOR_SIZE;
        break;
    case GET_BLOCK_SIZE:
        *((DWORD *)buff) = 0;
        break;
    default:
        break;
    }
    return ret;
}

void ram_block1_format()
{
    MKFS_PARM parm = 
    {
        .fmt = FM_ANY,
        .n_fat = 0,
        .align = 0,
        .n_root = 0,
        .au_size = 0
    };
    char work[FF_MAX_SS]; 
    f_mkfs("0", &parm, work, FF_MAX_SS);
}

static int ram_open(struct devfs_file_t *file)
{
    file->offset = 0;
    return 0;
}

static int ram_close(struct devfs_file_t *file)
{
    return 0;
}

static int ram_read(struct devfs_file_t *file, void *buff, int count, int64_t offset)
{
    memcpy(buff, &((char *)s_ram_buffer)[offset], count);
    return count;
}

static int ram_write(struct devfs_file_t *file, const void *buff, int count, int64_t offset)
{
    memcpy(&((char *)s_ram_buffer)[offset], buff, count);
    return count;
}

static int64_t ram_lseek(struct devfs_file_t *file, int64_t offset, int whence)
{
    int64_t ret = 0;
    switch (whence)
    {
    case VFS_SEEK_SET:
        ret = offset;
        if (ret >= 0 && ret < SECTOR_SIZE * SECTOR_COUNT)
        {
            return ret;
        }
        break;
    case VFS_SEEK_CUR:
        ret = file->offset + offset;
        if (ret >= 0 && ret < SECTOR_SIZE * SECTOR_COUNT)
        {
            return ret;
        }
        break;
    case VFS_SEEK_END:
        ret = SECTOR_SIZE * SECTOR_COUNT + offset;
        if (ret >= 0 && ret < SECTOR_SIZE * SECTOR_COUNT)
        {
            return ret;
        }
        break;
    default:
        break;
    }
    return SYS_ERROR_SPIPE;
}

static int ram_syncfs(struct devfs_file_t *file)
{
    return 0;
}

static int ram_ioctl(struct devfs_file_t *file, int cmd, int64_t arg)
{
    return 0;
}

static int ram_stat(struct devfs_device_t *device, struct devfs_stat_t *stat)
{
    stat->st_size = SECTOR_SIZE * SECTOR_COUNT;
    stat->st_blksize = SECTOR_SIZE;
    stat->st_blocks = SECTOR_COUNT;
    return 0;
}

void ram_block1_create()
{
    struct devfs_operations_t operations = {
        .open = ram_open,
        .close = ram_close,
        .read = ram_read,
        .write = ram_write,
        .lseek = ram_lseek,
        .syncfs = ram_syncfs,
        .ioctl = ram_ioctl,
        .stat = ram_stat,
    };
    static struct devfs_driver_t driver;
    strcpy(driver.name, DRIVER_NAME);
    driver.operations = operations;
    sys_devfs_register_driver(&driver);
    sys_devfs_create_device(DRIVER_NAME, "block1", NULL, VFS_MODE_IRUSR | VFS_MODE_IWUSR);
}
