#include "ram_block.h"
#include "lfs.h"
#include <string.h>
#include "sys_devfs.h"
#include "sys_vfs.h"
#include "lfs_adapter.h"
#include "sys_error.h"
#define PAGE_SIZE 512
#define BLOCK_SIZE 4096
#define BLOCK_COUNT 1024
static int s_ram_buffer[BLOCK_SIZE * BLOCK_COUNT / sizeof(int)];
static int s_lfs_read_buffer[BLOCK_SIZE / sizeof(int)];
static int s_lfs_prog_buffer[BLOCK_SIZE / sizeof(int)];
static int s_lfs_lookahead_buffer[BLOCK_COUNT / 8 / sizeof(int)];
#define DRIVER_NAME "ram_block_driver"
static int read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    memcpy(&((char *)buffer)[off], &s_ram_buffer[block * BLOCK_SIZE + off / sizeof(int)], size);
    return LFS_ERR_OK;
}

static int prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    memcpy(&s_ram_buffer[block * BLOCK_SIZE + off / sizeof(int)], &((char *)buffer)[off], size);
    return LFS_ERR_OK;
}

static int erase(const struct lfs_config *c, lfs_block_t block)
{
    return LFS_ERR_OK;
}

static int sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

void ram_block_format()
{
    const struct lfs_config config =
        {
            .read = read,
            .prog = prog,
            .erase = erase,
            .sync = sync,
            .read_size = PAGE_SIZE,
            .prog_size = PAGE_SIZE,
            .block_size = BLOCK_SIZE,
            .block_count = BLOCK_COUNT,
            .block_cycles = 100,
            .cache_size = BLOCK_SIZE,
            .lookahead_size = BLOCK_COUNT / 8,
            .read_buffer = s_lfs_read_buffer,
            .prog_buffer = s_lfs_prog_buffer,
            .lookahead_buffer = s_lfs_lookahead_buffer,
            .name_max = 0,
            .file_max = 0,
            .attr_max = 0,
            .metadata_max = 0,
        };
    lfs_t lfs;
    memset(&lfs, 0, sizeof(lfs_t));
    lfs_format(&lfs, &config);
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
        if (ret >= 0 && ret < BLOCK_SIZE * BLOCK_COUNT)
        {
            return ret;
        }
        break;
    case VFS_SEEK_CUR:
        ret = file->offset + offset;
        if (ret >= 0 && ret < BLOCK_SIZE * BLOCK_COUNT)
        {
            return ret;
        }
        break;
    case VFS_SEEK_END:
        ret = BLOCK_SIZE * BLOCK_COUNT + offset;
        if (ret >= 0 && ret < BLOCK_SIZE * BLOCK_COUNT)
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
    int64_t *v = (int64_t *)arg;
    switch (cmd)
    {
    case LFS_GET_PAGE_SIZE:
        *v = PAGE_SIZE;
        break;
    case LFS_GET_BLOCK_SIZE:
        *v = BLOCK_SIZE;
        break;
    case LFS_GET_BLOCK_COUNT:
        *v = BLOCK_COUNT;
        break;
    case LFS_ERASE:
        /* code */
        break;
    default:
        break;
    }
    return 0;
}

static int ram_stat(struct devfs_device_t *device, struct devfs_stat_t *stat)
{
    stat->st_size = BLOCK_SIZE * BLOCK_COUNT;
    stat->st_blksize = BLOCK_SIZE;
    stat->st_blocks = BLOCK_COUNT;
    return 0;
}

void ram_block_create()
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
    sys_devfs_create_device(DRIVER_NAME, "block", NULL, VFS_MODE_IRUSR | VFS_MODE_IWUSR);
}
