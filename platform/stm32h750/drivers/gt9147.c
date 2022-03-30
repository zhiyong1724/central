#include "gt9147.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
//IO方向设置
#define CT_SDA_IN()  {GPIOG->MODER&=~(3<<(7*2));GPIOG->MODER|=0<<7*2;}	//PG7输入模式
#define CT_SDA_OUT() {GPIOG->MODER&=~(3<<(7*2));GPIOG->MODER|=1<<7*2;} 	//PG7输出模式
//IO操作函数	 
#define CT_IIC_SCL(n) (n?HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOH,GPIO_PIN_6,GPIO_PIN_RESET))//SCL
#define CT_IIC_SDA(n) (n?HAL_GPIO_WritePin(GPIOG,GPIO_PIN_7,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOG,GPIO_PIN_7,GPIO_PIN_RESET))//SDA	 
#define CT_READ_SDA   HAL_GPIO_ReadPin(GPIOG,GPIO_PIN_7)//输入SDA 

//IO操作函数	 
#define GT_RST(n)  (n?HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOI,GPIO_PIN_8,GPIO_PIN_RESET))//GT9147复位引脚
#define GT_INT      HAL_GPIO_ReadPin(GPIOH,GPIO_PIN_7)  //GT9147中断引脚		
 
//I2C读写命令	
#define GT_CMD_WR 		0X28     	//写命令
#define GT_CMD_RD 		0X29		//读命令
  
//GT9147 部分寄存器定义 
#define GT_CTRL_REG 	0X8040   	//GT9147控制寄存器
#define GT_CFGS_REG 	0X8047   	//GT9147配置起始地址寄存器
#define GT_CHECK_REG 	0X80FF   	//GT9147校验和寄存器
#define GT_PID_REG 		0X8140   	//GT9147产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT9147当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址  
static void delayus(int us)
{
	volatile int n = 480 * us; 
	while (n--)
	{
	}
}

//产生IIC起始信号
static void CT_IIC_Start(void)
{
	CT_IIC_SDA(1);	  	  
	CT_IIC_SCL(1);
    delayus(1);
 	CT_IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
    delayus(1);
	CT_IIC_SCL(0);//钳住I2C总线，准备发送或接收数据 
    delayus(1);
}	 

//产生IIC停止信号
static void CT_IIC_Stop(void)
{
	CT_IIC_SDA(0);
    delayus(1);
	CT_IIC_SCL(1);
	CT_IIC_SDA(1);//STOP:when CLK is high DATA change form low to high
	delayus(1);
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
static uint8_t CT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
    uint8_t rack = 0;

	CT_IIC_SDA(1);	
	delayus(1);
	CT_IIC_SCL(1);
	delayus(1);
    
	while(CT_READ_SDA)
	{
		ucErrTime++;
        
		if(ucErrTime>250)
		{
			CT_IIC_Stop();
            rack = 1;
            break;
		} 
	}
    
	CT_IIC_SCL(0);//时钟输出0 	
    delayus(1);
    return rack;
} 

//产生ACK应答
static void CT_IIC_Ack(void)
{
	CT_IIC_SDA(0);
	delayus(1);
	CT_IIC_SCL(1);
	delayus(1);
	CT_IIC_SCL(0);
	delayus(1);
	CT_IIC_SDA(1);
	delayus(1);
}
//不产生ACK应答		    
static void CT_IIC_NAck(void)
{
	CT_IIC_SDA(1);
	delayus(1);
	CT_IIC_SCL(1);
	delayus(1);
	CT_IIC_SCL(0);
    delayus(1);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
static void CT_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   

	for(t=0;t<8;t++)
    {              
        CT_IIC_SDA((txd&0x80)>>7); 
        delayus(1);
		CT_IIC_SCL(1); 
		delayus(1);
		CT_IIC_SCL(0);	
        txd<<=1; 	      
    }
    CT_IIC_SDA(1);
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
static uint8_t CT_IIC_Read_Byte(unsigned char ack)
{
	uint8_t i,receive=0;

	for(i=0;i<8;i++ )
	{ 
        receive<<=1;
		CT_IIC_SCL(1); 	    	   
		delayus(1);
        
		if(CT_READ_SDA)receive++;   
		CT_IIC_SCL(0);	
		delayus(1);
        
	}	  				 
	if (!ack)
        CT_IIC_NAck();//发送nACK
	else 
        CT_IIC_Ack(); //发送ACK   
 	return receive;
}

//向GT9147写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
uint8_t GT9147_WR_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i;
	uint8_t ret=0;
	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   	//发送写命令 	 
	CT_IIC_Wait_Ack();
	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	CT_IIC_Wait_Ack(); 	 										  		   
	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
	for(i=0;i<len;i++)
	{	   
    	CT_IIC_Send_Byte(buf[i]);  	//发数据
		ret=CT_IIC_Wait_Ack();
		if(ret)break;  
	}
    CT_IIC_Stop();					//产生一个停止条件	    
	return ret; 
}

//从GT9147读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度			  
static void GT9147_RD_Reg(uint16_t reg,uint8_t *buf,uint8_t len)
{
	uint8_t i; 
 	CT_IIC_Start();	
 	CT_IIC_Send_Byte(GT_CMD_WR);   //发送写命令 	 
	CT_IIC_Wait_Ack();
 	CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
	CT_IIC_Wait_Ack(); 	 										  		   
 	CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
	CT_IIC_Wait_Ack();  
 	CT_IIC_Start();  	 	   
	CT_IIC_Send_Byte(GT_CMD_RD);   //发送读命令		   
	CT_IIC_Wait_Ack();	   
	for(i=0;i<len;i++)
	{	   
    	buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据	  
	} 
    CT_IIC_Stop();//产生一个停止条件    
} 
//初始化GT9147触摸屏
//返回值:0,初始化成功;1,初始化失败 
uint8_t GT9147_Init()
{
	uint8_t temp[5]; 
	GT_RST(0);				//复位
	HAL_Delay(100);
 	GT_RST(1);				//释放复位		    
	HAL_Delay(100); 
	GT9147_RD_Reg(GT_PID_REG,temp,4);//读取产品ID
	temp[4]=0;
	printf("CTP ID:%s\r\n",temp);	//打印ID
	return 0;
}

static const uint16_t GT9147_TPX_TBL[]={GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
static LcdTouchInfo sLcdTouchInfos[GT_MAX_TOUCH_NUM];
const LcdTouchInfo *GT9147_Scan()
{
	uint8_t state;
	GT9147_RD_Reg(GT_GSTID_REG, &state, 1); //读取触摸点的状态
	if (state & 0X80)
	{
		uint8_t temp = 0;
		GT9147_WR_Reg(GT_GSTID_REG, &temp, 1); //清标志
	}
	for (size_t i = 0; i < GT_MAX_TOUCH_NUM; i++)
	{
		sLcdTouchInfos[i].touch = 0;
	}
	uint8_t touchCount = state & 0x0f;
	if (touchCount > 0 && touchCount <= GT_MAX_TOUCH_NUM)
	{
		for (size_t i = 0; i < touchCount; i++)
		{
			uint8_t data[4];
			GT9147_RD_Reg(GT9147_TPX_TBL[i], data, 4);	//读取XY坐标值
			sLcdTouchInfos[i].touch = 1;
			sLcdTouchInfos[i].x = ((uint16_t)data[1] << 8) + (uint16_t)data[0];
            sLcdTouchInfos[i].y = ((uint16_t)data[3] << 8) + (uint16_t)data[2];
		}
	}
	return sLcdTouchInfos;
}