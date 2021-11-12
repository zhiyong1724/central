#include "es8388.h"
#include "softiic.h"
#include "pcf8574.h"
#include "stm32h7xx_hal.h"
#define ES8388_ADDR     0x10	//ES8388的器件地址,固定为0x10 
//ES8388初始化
//返回值:0,初始化正常
//    其他,错误代码
uint8_t ES8388_Init(void)
{
    IIC_Init();                 	//初始化IIC接口
    PCF8574_WriteBit(MD_PD, 0);		//打开功放

    //软复位ES8388
    ES8388_Write_Reg(0, 0x80);
    ES8388_Write_Reg(0, 0x00);
    HAL_Delay(100);					//等待复位

    ES8388_Write_Reg(0x01, 0x58);
    ES8388_Write_Reg(0x01, 0x50);
    ES8388_Write_Reg(0x02, 0xF3);
    ES8388_Write_Reg(0x02, 0xF0);

    ES8388_Write_Reg(0x03, 0x09);	//麦克风偏置电源关闭
    ES8388_Write_Reg(0x00, 0x06);	//使能参考		500K驱动使能
    ES8388_Write_Reg(0x04, 0x00);	//DAC电源管理，不打开任何通道
    ES8388_Write_Reg(0x08, 0x00);	//MCLK不分频
    ES8388_Write_Reg(0x2B, 0x80);	//DAC控制	DACLRC与ADCLRC相同

    ES8388_Write_Reg(0x09, 0x88);	//ADC L/R PGA增益配置为+24dB
    ES8388_Write_Reg(0x0C, 0x4C);	//ADC	数据选择为left data = left ADC, right data = left ADC 	音频数据为16bit
    ES8388_Write_Reg(0x0D, 0x02);	//ADC配置 MCLK/采样率=256
    ES8388_Write_Reg(0x10, 0x00);	//ADC数字音量控制将信号衰减 L	设置为最小！！！
    ES8388_Write_Reg(0x11, 0x00);	//ADC数字音量控制将信号衰减 R	设置为最小！！！

    ES8388_Write_Reg(0x17, 0x18);	//DAC 音频数据为16bit
    ES8388_Write_Reg(0x18, 0x02);	//DAC	配置 MCLK/采样率=256
    ES8388_Write_Reg(0x1A, 0x00);	//DAC数字音量控制将信号衰减 L	设置为最小！！！
    ES8388_Write_Reg(0x1B, 0x00);	//DAC数字音量控制将信号衰减 R	设置为最小！！！
    ES8388_Write_Reg(0x27, 0xB8);	//L混频器
    ES8388_Write_Reg(0x2A, 0xB8);	//R混频器
    HAL_Delay(100);
    return 0;
}

//ES8388写寄存器
//reg:寄存器地址
//val:要写入寄存器的值
//返回值:0,成功;
//    其他,错误代码
uint8_t ES8388_Write_Reg(uint8_t reg, uint8_t val)
{
    IIC_Start();
    IIC_Send_Byte((ES8388_ADDR << 1)|0); 	//发送器件地址+写命令
    if(IIC_Wait_Ack())return 1;				//等待应答(成功?/失败?)
    IIC_Send_Byte(reg);						//写寄存器地址
    if(IIC_Wait_Ack())return 2;				//等待应答(成功?/失败?)
    IIC_Send_Byte(val & 0XFF);				//发送数据
    if(IIC_Wait_Ack())return 3;				//等待应答(成功?/失败?)
    IIC_Stop();
    return 0;
}

//ES8388读寄存器
//reg:寄存器地址
//返回值:读到的数据
uint8_t ES8388_Read_Reg(uint8_t reg)
{
    uint8_t temp = 0;

    IIC_Start();
    IIC_Send_Byte((ES8388_ADDR << 1)|0); 	//发送器件地址+写命令
    if(IIC_Wait_Ack())return 1;				//等待应答(成功?/失败?)
    IIC_Send_Byte(reg);						//写寄存器地址
    if(IIC_Wait_Ack())return 1;				//等待应答(成功?/失败?)
    IIC_Start();
    IIC_Send_Byte((ES8388_ADDR << 1)|1); 	//发送器件地址+读命令
    if(IIC_Wait_Ack())return 1;				//等待应答(成功?/失败?)
    temp = IIC_Read_Byte(0);
    IIC_Stop();

    return temp;
}

//设置I2S工作模式
//fmt:0,飞利浦标准I2S;1,MSB(左对齐);2,LSB(右对齐);3,PCM/DSP
//len:0,24bit;1,20bit;2,18bit;3,16bit;4,32bit
void ES8388_I2S_Cfg(uint8_t fmt, uint8_t len)
{
    fmt&=0X03;
    len&=0X07; //限定范围
    ES8388_Write_Reg(23,(fmt << 1)|(len << 3));	//R23,ES8388工作模式设置
}

//设置耳机音量
//volume:音量大小(0-33)
void ES8388_HPvol_Set(uint8_t volume)
{
    uint8_t i=0;
    if(volume > 30)
        volume=30;
    for(i=0; i<volume; i++)
    {
        ES8388_Write_Reg(0x2E,i);
        ES8388_Write_Reg(0x2F,i);
    }
}

//设置喇叭音量
//volume:音量大小(0-33)
void ES8388_SPKvol_Set(uint8_t volume)
{
    uint8_t i=0;
    if(volume > 30)
        volume=30;
    for(i=0; i<volume; i++)
    {
        ES8388_Write_Reg(0x30,i);
        ES8388_Write_Reg(0x31,i);
    }
}

//设置3D环绕声
//depth:0~7(3D强度,0关闭,7最强)
void ES8388_3D_Set(uint8_t depth)
{ 
	depth&=0X7;//限定范围 
 	ES8388_Write_Reg(0X1D,depth<<2);	//R7,3D环绕设置 	
}

//ES8388 DAC/ADC配置
//dacen: dac使能(1)/关闭(0)
//adcen: adc使能(1)/关闭(0)
void ES8388_ADDA_Cfg(uint8_t dacen,uint8_t adcen)
{
	uint8_t tempreg = 0;
	tempreg|=((!dacen)<<0);
	tempreg|=((!adcen)<<1);
	tempreg|=((!dacen)<<2);
	tempreg|=((!adcen)<<3);
	ES8388_Write_Reg(0x02,tempreg);
}

//ES8388 DAC输出通道配置
//o1en: 通道1使能(1)/禁止(0)
//o2en: 通道2使能(1)/禁止(0)
void ES8388_Output_Cfg(uint8_t o1en,uint8_t o2en)
{
	uint8_t tempreg=0;
	tempreg|=o1en*(3<<4);
	tempreg|=o2en*(3<<2);	
	ES8388_Write_Reg(0x04,tempreg);
}

//ES8388 MIC增益设置(MIC PGA增益)
//gain:0~8,对应0~24dB  3dB/Step
void ES8388_MIC_Gain(uint8_t gain)
{
	gain&=0X0F; 
	gain|=gain<<4;
	ES8388_Write_Reg(0X09,gain);		//R9,左右通道PGA增益设置
}

//ES8388 ALC设置
//sel:0,关闭ALC;1,右通道ALC;2,左通道ALC;3,立体声ALC
//maxgain:0~7,对应-6.5~+35.5dB  
//minigain:0~7,对应-12~+30dB  	6dB/STEP
void ES8388_ALC_Ctrl(uint8_t sel,uint8_t maxgain,uint8_t mingain)
{
	uint8_t tempreg=0;
	tempreg=sel<<6;
	tempreg|=(maxgain&0X07)<<3;
	tempreg|=mingain&0X07;
	ES8388_Write_Reg(0X12,tempreg);		//R18,ALC设置
}

//ES8388 ADC输出通道配置
//in: 0 通道1输入; 1 通道2输入
void ES8388_Input_Cfg(uint8_t in)
{
	ES8388_Write_Reg(0x0A,(5*in)<<4);	//ADC1 输入通道选择L/R	INPUT1
}
