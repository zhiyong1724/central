#ifndef __NORFLASH_H__
#define __NORFLASH_H__
#include <stdint.h>
void norflashInit();
void norflashSectorErase(uint32_t address);                      //一次擦除4K数据
void norflashWriteData(uint32_t address, uint8_t *data, uint32_t size);  //一次最多只能写入256byte数据
void norflashReadData(uint32_t address, uint8_t *data, uint32_t size);
void norflashMemoryMapped();
#endif