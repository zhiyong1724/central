#ifndef __PCF8574_H
#define __PCF8574_H
#include <stdint.h>
//PCF8574各个IO的功能
#define BEEP_IO         0		//蜂鸣器控制引脚  		P0
#define AP_INT_IO       1   	//AP3216C中断引脚		P1
#define DCMI_PWDN_IO    2    	//DCMI的电源控制引脚	P2
#define USB_PWR_IO      3    	//USB电源控制引脚		P3
#define MD_PD      		4    	//MD_PD					P4
#define MPU_INT_IO      5   	//MPU9250中断引脚		P5
#define RS485_RE_IO     6    	//RS485_RE引脚			P6
#define ETH_RESET_IO    7    	//以太网复位引脚		P7
uint8_t PCF8574_Init(void); 
uint8_t PCF8574_ReadOneByte(void); 
void PCF8574_WriteOneByte(uint8_t DataToWrite);
void PCF8574_WriteBit(uint8_t bit,uint8_t sta);
uint8_t PCF8574_ReadBit(uint8_t bit);
#endif

