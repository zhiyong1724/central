#ifndef __ES8388_H__
#define __ES8388_H__
#include <stdint.h>
uint8_t ES8388_Init(void);
uint8_t ES8388_Write_Reg(uint8_t reg, uint8_t val);
uint8_t ES8388_Read_Reg(uint8_t reg);
void ES8388_I2S_Cfg(uint8_t fmt, uint8_t len);
void ES8388_HPvol_Set(uint8_t volume);
void ES8388_SPKvol_Set(uint8_t volume);
void ES8388_3D_Set(uint8_t depth);
void ES8388_ADDA_Cfg(uint8_t dacen,uint8_t adcen);
void ES8388_Output_Cfg(uint8_t o1en,uint8_t o2en);
void ES8388_MIC_Gain(uint8_t gain);
void ES8388_ALC_Ctrl(uint8_t sel,uint8_t maxgain,uint8_t mingain);
void ES8388_Input_Cfg(uint8_t in);
#endif


