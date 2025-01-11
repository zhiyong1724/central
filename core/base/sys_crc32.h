#ifndef __SYS_CRC32_H__
#define __SYS_CRC32_H__
#include <stdint.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C"
{
#endif
uint32_t sys_crc32(const uint8_t *data, size_t len);
#ifdef __cplusplus
}
#endif
#endif