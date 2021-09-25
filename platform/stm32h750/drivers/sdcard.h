#ifndef __SDCARD_H__
#define __SDCARD_H__
#include <stdint.h>
void sdcardInit();
uint32_t sdcardGetBlockSize();
uint32_t sdcardGetBlockNumber();
int sdcardWriteBlock(uint32_t block, uint32_t num, const void *buffer);
int sdcardReadBlock(uint32_t block, uint32_t num, void *buffer);
#endif