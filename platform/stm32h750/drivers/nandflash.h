#ifndef __NANDFLASH_H__
#define __NANDFLASH_H__
#include <stdint.h>
#define NAND_FLASH_PLANE_NUMBER 2
#define NAND_FLASH_PLANE_SIZE 2048
#define NAND_FLASH_BLOCK_SIZE 64
#define NAND_FLASH_PAGE_SIZE 2048
void nandFlashInit();
void nandFlashEraseBlock(uint32_t block);
int nandFlashWritePage(uint32_t block, uint32_t page, const void *buffer);
int nandFlashReadPage(uint32_t block, uint32_t page, void *buffer);
#endif