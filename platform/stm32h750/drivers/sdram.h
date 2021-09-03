#ifndef _SDRAM_H
#define _SDRAM_H
#include <stdint.h>
//SDRAM配置参数
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint32_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint32_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint32_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint32_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint32_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint32_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint32_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint32_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint32_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint32_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint32_t)0x0200)

void SDRAM_Initialization_Sequence();
#endif
