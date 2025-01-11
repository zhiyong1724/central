#include "ramio.h"
#include <time.h>
#include "lfs.h"
#include <string.h>
#define SECTOR_SIZE 512
#define SECTOR_COUNT 2 * 1024
static unsigned int sDisk[SECTOR_COUNT * SECTOR_SIZE / sizeof(unsigned int)];
static unsigned char sInit = 0;

int ramIOInit()
{
    sInit = 1;
    return 0;
}

int ramIOStatus()
{
    if (!sInit)
    {
        return STA_NOINIT;
    }
    return 0;
}

int ramIORead(unsigned char *buff, unsigned int sector, unsigned int count)
{
    if (!sInit)
    {
        return RES_NOTRDY;
    }
    if (sector + count > SECTOR_COUNT)
    {
        return RES_PARERR;
    }
    unsigned int *data = (unsigned int *)buff;
    unsigned int start = SECTOR_SIZE * sector / sizeof(unsigned int);
    unsigned int size = SECTOR_SIZE * count / sizeof(unsigned int);
    for (unsigned int i = 0; i < size; i++)
    {
        data[i] = sDisk[start + i];
    }
    return 0;
}

int ramIOWrite(const unsigned char *buff, unsigned int sector, unsigned int count)
{
    if (!sInit)
    {
        return RES_NOTRDY;
    }
    if (sector + count > SECTOR_COUNT)
    {
        return RES_PARERR;
    }
    unsigned int *data = (unsigned int *)buff;
    unsigned int start = SECTOR_SIZE * sector / sizeof(unsigned int);
    unsigned int size = SECTOR_SIZE * count / sizeof(unsigned int);
    for (unsigned int i = 0; i < size; i++)
    {
        sDisk[start + i] = data[i];
    }
    return 0;
}

int ramIOCtl(unsigned char cmd, void *buff)
{
    if (!sInit)
    {
        return RES_NOTRDY;
    }
    if (cmd > CTRL_TRIM)
    {
        return RES_PARERR;
    }
    switch (cmd)
    {
    case CTRL_SYNC:
        break;
    case GET_SECTOR_COUNT:
        *((uint16_t *)buff) = SECTOR_COUNT;
        break;
    case GET_SECTOR_SIZE:
        *((uint16_t *)buff) = SECTOR_SIZE;
        break;
    case GET_BLOCK_SIZE:
        *((uint16_t *)buff) = 1;
        break;
    case CTRL_TRIM:
        break;
    default:
        break;
    }
    return 0;
}

DWORD get_fattime()
{
    time_t t = time(NULL);
    struct tm *localTime = localtime(&t);
    DWORD ret = 0;
    DWORD second = (DWORD)localTime->tm_sec;
    DWORD minute = (DWORD)localTime->tm_min;
    DWORD hour = (DWORD)localTime->tm_hour;
    DWORD day = (DWORD)localTime->tm_mday;
    DWORD month = (DWORD)localTime->tm_mon + 1;
    DWORD year = 1900 + (DWORD)localTime->tm_year;
    ret |= second / 2;
    ret |= minute << 5;
    ret |= hour << 11;
    ret |= day << 16;
    ret |= month << 21;
    ret |= (year - 1980) << 25;
    return ret;
}