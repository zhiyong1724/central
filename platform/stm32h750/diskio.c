/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include "diskio.h"		/* Declarations of disk functions */
#include "sdcard.h"
#include <stdio.h>
/* Definitions of physical drive number for each drive */
#define DEV_RAM		1	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		0	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

static DSTATUS mmcStatus()
{
    return 0;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat = RES_OK;
	//int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_status();

		// translate the reslut code here

		return STA_NODISK;

	case DEV_MMC :
		//result = MMC_disk_status();

		// translate the reslut code here

		return mmcStatus();

	case DEV_USB :
		//result = USB_disk_status();

		// translate the reslut code here

		return STA_NODISK;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
static DSTATUS mmcInit()
{
    return 0;
}

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat = RES_OK;
	//int result;

	switch (pdrv) {
	case DEV_RAM :
		//result = RAM_disk_initialize();

		// translate the reslut code here

		return STA_NODISK;

	case DEV_MMC :
		//result = MMC_disk_initialize();

		// translate the reslut code here

		return mmcInit();

	case DEV_USB :
		//result = USB_disk_initialize();

		// translate the reslut code here

		return STA_NODISK;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
static DRESULT mmcRead(BYTE *buff, LBA_t sector, UINT count)
{
	DRESULT ret = RES_ERROR;
	if (sdcardReadBlock((uint32_t)sector, (uint32_t)count, (void *)buff) == 0)
	{
		ret = RES_OK;
	}
	return ret;
}

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	//DRESULT res = RES_OK;
	//int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_read(buff, sector, count);

		// translate the reslut code here

		return RES_NOTRDY;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return mmcRead(buff, sector, count);

	case DEV_USB :
		// translate the arguments here

		//result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return RES_NOTRDY;
	}

	return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0
static DRESULT mmcWrite(const BYTE *buff, LBA_t sector, UINT count)
{
	DRESULT ret = RES_ERROR;
	if (sdcardWriteBlock((uint32_t)sector, (uint32_t)count, (const void *)buff) == 0)
	{
		ret = RES_OK;
	}
	return ret;
}

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	//DRESULT res = RES_OK;
	//int result;

	switch (pdrv) {
	case DEV_RAM :
		// translate the arguments here

		//result = RAM_disk_write(buff, sector, count);

		// translate the reslut code here

		return RES_NOTRDY;

	case DEV_MMC :
		// translate the arguments here

		//result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return mmcWrite(buff, sector, count);

	case DEV_USB :
		// translate the arguments here

		//result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return RES_NOTRDY;
	}

	return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
static DRESULT mmcIoctl(BYTE cmd, void *buff)
{
    if (cmd > CTRL_TRIM)
    {
        return RES_PARERR;
    }
    switch (cmd)
    {
    case CTRL_SYNC:
        break;
    case GET_SECTOR_COUNT:
        *((unsigned int *)buff) = (unsigned int)sdcardGetBlockNumber();
        break;
    case GET_SECTOR_SIZE:
        *((unsigned int *)buff) = (unsigned int)sdcardGetBlockSize();
        break;
    case GET_BLOCK_SIZE:
        *((unsigned int *)buff) = 1;
        break;
    case CTRL_TRIM:
        break;
    default:
        break;
    }
    return RES_OK;
}

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	//DRESULT res = RES_OK;
	//int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return RES_NOTRDY;

	case DEV_MMC :

		// Process of the command for the MMC/SD card

		return mmcIoctl(cmd, buff);

	case DEV_USB :

		// Process of the command the USB drive

		return RES_NOTRDY;
	}

	return RES_PARERR;
}

DWORD get_fattime()
{
    return 0;
}
