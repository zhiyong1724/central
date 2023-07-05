/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "sai.h"
#include "usart.h"
#include "gpio.h"
#include "stm32h7xx_hal_rcc.h"
#include "sdram.h"
#include "key.h"
#include "led.h"
#include <stdio.h>
#include "normalmode.h"
#include "nandflash.h"
#include "lfsio.h"
#include "lfs.h"
#include <stdlib.h>
#include "pcf8574.h"
#include "es8388.h"
#include "sai.h"
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// static void configMPU()
// {
//   MPU_Region_InitTypeDef MPU_InitStruct;

//   /* Disable the MPU */
//   HAL_MPU_Disable();
//   //ITCM
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x00000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;                 //64K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;    //写通
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER0;                 //bank0
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //Flash memory
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x08000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;                  //2M大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;    //写通
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER1;                 //bank1
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //DTCM
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x20000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;                //128K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER2;                 //bank2
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //AXI SRAM
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x24000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;                //512K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;    //写通
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;      //禁止缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER3;                 //bank3
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;               //tex1
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //SRAM1
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x30000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;                //128K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER4;                 //bank4
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //SRAM2
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x30020000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;                //128K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER5;                 //bank5
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //SRAM3
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x30040000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_32KB;                 //32K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER6;                 //bank6
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //SRAM4
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x38000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;                 //64K大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER7;                 //bank7
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //register
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x40000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_512MB;                //512M大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //设备
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;      //禁止缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER8;                 //bank8
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //SDRAM
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x60000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_512MB;                //512M大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //写回
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER9;                 //bank9
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //nor flash
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x90000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;                //256M大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;    //写通
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;          //允许缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER10;                //bank10
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   //nand flash
//   MPU_InitStruct.Enable = MPU_REGION_ENABLE;                  //使能
//   MPU_InitStruct.BaseAddress = 0x80000000;                    //基地址
//   MPU_InitStruct.Size = MPU_REGION_SIZE_256MB;                //256M大小
//   MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;   //允许访问
//   MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;        //设备
//   MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;      //禁止缓存
//   MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;      //禁止共享
//   MPU_InitStruct.Number = MPU_REGION_NUMBER11;                //bank11
//   MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;               //tex0
//   MPU_InitStruct.SubRegionDisable = 0x00;                     //禁止子Region
//   MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE; //允许指令访问
//   HAL_MPU_ConfigRegion(&MPU_InitStruct);
//   /* Enable the MPU */
//   HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
// }
extern int _isr_vector[];
/* USER CODE END 0 */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  SCB->VTOR = (uint32_t)&_isr_vector;
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */
  /* Enable I-Cache---------------------------------------------------------*/
  //SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();
  /* MCU Configuration--------------------------------------------------------*/
  //configMPU();
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* USER CODE BEGIN Init */
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
  PeriphCommonClock_Config();
  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  led0OFF();
  led1OFF();
  MX_USART1_UART_Init();
  enterNormalMode();
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1|RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLL3.PLL3M = 21;
  PeriphClkInitStruct.PLL3.PLL3N = 289;
  PeriphClkInitStruct.PLL3.PLL3P = 7;
  PeriphClkInitStruct.PLL3.PLL3Q = 7;
  PeriphClkInitStruct.PLL3.PLL3R = 12;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  printf("Error_Handler\n");
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
