#include "nandflash.h"
#include "fmc.h"
#include "stm32h7xx_hal_nand.h"
#include "stm32h7xx_hal.h"
#include <stdint.h>
#define NAND_RB  	            HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_6)//NAND Flash的闲/忙引脚

#define NAND_ADDRESS			0X80000000	//nand flash的访问地址,接NCE3,地址为:0X8000 0000
#define NAND_CMD				1<<16		//发送命令
#define NAND_ADDR				1<<17		//发送地址

//NAND FLASH命令
#define NAND_READID         	0X90    	//读ID指令
#define NAND_FEATURE			0XEF    	//设置特性指令
#define NAND_RESET          	0XFF    	//复位NAND
#define NAND_READSTA        	0X70   	 	//读状态
#define NAND_AREA_A         	0X00   
#define NAND_AREA_TRUE1     	0X30  
#define NAND_WRITE0        	 	0X80
#define NAND_WRITE_TURE1    	0X10
#define NAND_ERASE0        	 	0X60
#define NAND_ERASE1         	0XD0
#define NAND_MOVEDATA_CMD0  	0X00
#define NAND_MOVEDATA_CMD1  	0X35
#define NAND_MOVEDATA_CMD2  	0X85
#define NAND_MOVEDATA_CMD3  	0X10

//NAND FLASH状态
#define NSTA_READY       	   	0X40		//nand已经准备好
#define NSTA_ERROR				0X01		//nand错误
#define NSTA_TIMEOUT        	0X02		//超时
#define NSTA_ECC1BITERR       	0X03		//ECC 1bit错误
#define NSTA_ECC2BITERR       	0X04		//ECC 2bit以上错误


//NAND FLASH型号和对应的ID号
#define MT29F4G08ABADA			0XDC909556	//MT29F4G08ABADA
#define MT29F16G08ABABA			0X48002689	//MT29F16G08ABABA

//MPU相关设置
#define NAND_REGION_NUMBER      MPU_REGION_NUMBER4	    //NAND FLASH使用region4
#define NAND_ADDRESS_START      0X80000000              //NAND FLASH区的首地址
#define NAND_REGION_SIZE        MPU_REGION_SIZE_256MB   //NAND FLASH区大小
uint8_t NAND_EraseBlock(uint32_t BlockNum)
{
	BlockNum<<=6;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE0;
    //发送块地址
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)BlockNum;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(BlockNum>>8);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(BlockNum>>16);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_ERASE1;
	HAL_Delay(20);
	if(HAL_NAND_Read_Status(&hnand1) != NAND_READY)return NSTA_ERROR;//失败
    return 0;	//成功   
} 

uint8_t NAND_ReadPage(uint32_t PageNum,uint16_t ColNum,uint8_t *pBuffer,uint16_t NumByteToRead)
{
    uint16_t i=0;
     *(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_A;
    //发送地址
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)ColNum;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(ColNum>>8);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)PageNum;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(PageNum>>8);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(PageNum>>16);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_AREA_TRUE1;
    HAL_Delay(2);
    for (i = 0; i < NumByteToRead; i++)
    {
        *(__IO uint8_t *)pBuffer++ = *(__IO uint8_t *)NAND_ADDRESS;
    }
    HAL_Delay(2);
    return 0;	//成功   
} 

uint8_t NAND_WritePage(uint32_t PageNum,uint16_t ColNum,uint8_t *pBuffer,uint16_t NumByteToWrite)
{
    uint16_t i=0;  
	
	*(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE0;
    //发送地址
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)ColNum;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(ColNum>>8);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)PageNum;
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(PageNum>>8);
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_ADDR)=(uint8_t)(PageNum>>16);
	HAL_Delay(2);
    for (i = 0; i < NumByteToWrite; i++) //写入数据
    {
        *(__IO uint8_t *)NAND_ADDRESS = *(__IO uint8_t *)pBuffer++;
    }
    *(__IO uint8_t *)(NAND_ADDRESS|NAND_CMD)=NAND_WRITE_TURE1; 
	while(1)
    {
        HAL_Delay(2);
        if (HAL_NAND_Read_Status(&hnand1) == NAND_READY)
        {
            break;
        }
    }
    return 0;//成功   
}

void nandFlashInit()
{
    if (HAL_NAND_Reset(&hnand1) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_Delay(100);
    NAND_EraseBlock(1);
    //nandFlashEraseBlock(0);
    static uint8_t buff[NAND_FLASH_PAGE_SIZE];
    for (size_t i = 0; i < NAND_FLASH_PAGE_SIZE; i++)
    {
        buff[i] = 165;
    }
    NAND_WritePage(65, 0, buff, NAND_FLASH_PAGE_SIZE);
    NAND_ReadPage(65, 0, buff, NAND_FLASH_PAGE_SIZE);
    nandFlashWritePage(0, 0, buff);
    nandFlashReadPage(0, 0, buff);
    nandFlashReadPage(0, 0, buff);
}

void nandFlashEraseBlock(unsigned int block)
{
    block <<= 6;
    while (HAL_NAND_Read_Status(&hnand1) != NAND_READY);
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_ERASE0;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = block;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = block >> 8;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = block >> 16;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA))  = NAND_CMD_ERASE1;
      __DSB();
    HAL_Delay(20);
}

void nandFlashWritePage(unsigned int block, unsigned int page, const void *buffer)
{
    while (HAL_NAND_Read_Status(&hnand1) != NAND_READY);
    unsigned int pageAddress = block * NAND_FLASH_BLOCK_SIZE + page;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE0;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = 0x00U;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = 0x00U;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)pageAddress;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 16);
    __DSB();
    HAL_Delay(2);
    const uint8_t *data = (const uint8_t *)buffer;
    for (unsigned int i = 0; i < NAND_FLASH_PAGE_SIZE; i++)
    {
        *(__IO uint8_t *)NAND_READY = *data;
        data++;
        __DSB();
    }
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_WRITE_TRUE1;
    __DSB();
    while(1)
    {
        HAL_Delay(2);
        if (HAL_NAND_Read_Status(&hnand1) == NAND_READY)
        {
            break;
        }
    }
}

void nandFlashReadPage(unsigned int block, unsigned int page, void *buffer)
{
    while (HAL_NAND_Read_Status(&hnand1) != NAND_READY);
    unsigned int pageAddress = block * NAND_FLASH_BLOCK_SIZE + page;
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA)) = NAND_CMD_AREA_A;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = 0x00U;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = 0x00U;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)pageAddress;
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 8);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | ADDR_AREA)) = (uint8_t)(pageAddress >> 16);
    __DSB();
    *(__IO uint8_t *)((uint32_t)(NAND_DEVICE | CMD_AREA))  = NAND_CMD_AREA_TRUE1;
      __DSB();
    HAL_Delay(2);
    uint8_t *data = (uint8_t *)buffer;
    for (unsigned int i = 0; i < NAND_FLASH_PAGE_SIZE; i++)
    {
        *data = *((uint8_t *)NAND_DEVICE);
        data++;
        __DSB();
    }
}