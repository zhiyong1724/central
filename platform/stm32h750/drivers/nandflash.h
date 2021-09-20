#ifndef __NANDFLASH_H__
#define __NANDFLASH_H__
#define NAND_FLASH_PLANE_NUMBER 2
#define NAND_FLASH_PLANE_SIZE 2048
#define NAND_FLASH_BLOCK_SIZE 64
#define NAND_FLASH_PAGE_SIZE 2048
void nandFlashInit();
void nandFlashEraseBlock(unsigned int block);
void nandFlashWritePage(unsigned int block, unsigned int page, const void *buffer);
void nandFlashReadPage(unsigned int block, unsigned int page, void *buffer);
#endif