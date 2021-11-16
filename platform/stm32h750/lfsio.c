#include "lfsio.h"
#include "nandflash.h"
static unsigned int sReadBuffer[NAND_FLASH_PAGE_SIZE / sizeof(unsigned int)];
static unsigned int sProgBuffer[NAND_FLASH_PAGE_SIZE / sizeof(unsigned int)];
static unsigned int sLookaheadBuffer[NAND_FLASH_PLANE_NUMBER * NAND_FLASH_PLANE_SIZE / 8 / sizeof(unsigned int)];
static int read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    int ret = LFS_ERR_OK;
    uint8_t *data = (uint8_t *)buffer;
    for (lfs_size_t i = 0; i < size; i += NAND_FLASH_PAGE_SIZE)
    {
        if (nandFlashReadPage(block, (off + i) / NAND_FLASH_PAGE_SIZE, data + i) != 0)
        {
            ret = LFS_ERR_CORRUPT;
            break;
        }
    }
    return ret;
}

static int prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    int ret = LFS_ERR_OK;
    uint8_t *data = (uint8_t *)buffer;
    for (lfs_size_t i = 0; i < size; i += NAND_FLASH_PAGE_SIZE)
    {
        if (nandFlashWritePage(block, (off + i) / NAND_FLASH_PAGE_SIZE, data + i) != 0)
        {
            ret = LFS_ERR_CORRUPT;
            break;
        }
    }
    return ret;
}

static int erase(const struct lfs_config *c, lfs_block_t block)
{
    nandFlashEraseBlock((uint32_t)block);
    return LFS_ERR_OK;
}

static int sync(const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

const struct lfs_config gLfsConfig =
{
    .read = read,
    .prog = prog,
    .erase = erase,
    .sync = sync,
    .read_size = NAND_FLASH_PAGE_SIZE,
    .prog_size = NAND_FLASH_PAGE_SIZE,
    .block_size = NAND_FLASH_BLOCK_SIZE * NAND_FLASH_PAGE_SIZE,
    .block_count = NAND_FLASH_PLANE_NUMBER * NAND_FLASH_PLANE_SIZE,
    .block_cycles = 100,
    .cache_size = NAND_FLASH_PAGE_SIZE,
    .lookahead_size = NAND_FLASH_PLANE_NUMBER * NAND_FLASH_PLANE_SIZE / 8,
    .read_buffer = sReadBuffer,
    .prog_buffer = sProgBuffer,
    .lookahead_buffer = sLookaheadBuffer,
    .name_max = 0,
    .file_max = 0,
    .attr_max = 0,
    .metadata_max = 0,
};

lfs_t gLFS;