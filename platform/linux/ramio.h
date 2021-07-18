#ifndef __RAMIO_H__
#define __RAMIO_H__
#include "diskio.h"
#ifdef __cplusplus
extern "C" {
#endif
int ramIOInit();
int ramIOStatus();
int ramIORead(unsigned char *buff, unsigned int sector, unsigned int count);
int ramIOWrite(const unsigned char *buff, unsigned int sector, unsigned int count);
int ramIOCtl(unsigned char cmd, void *buff);
#ifdef __cplusplus
}
#endif
#endif