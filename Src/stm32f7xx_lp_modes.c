/**
  ******************************************************************************
  * @file    PWR/PWR_CurrentConsumption/stm32f7xx_lp_modes.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    21-September-2015
  * @brief   This file provides firmware functions to manage the following 
  *          functionalities of the STM32F7xx Low Power Modes:
  *           - Sleep Mode
  *           - STOP mode with RTC
  *           - STANDBY mode without RTC and BKPSRAM
  *           - STANDBY mode with RTC
  *           - STANDBY mode with RTC and BKPSRAM
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f7xx_lp_modes.h"
#include "rtc.h"

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup PWR_CurrentConsumption
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define RTC_ASYNCH_PREDIV    0x7F
#define RTC_SYNCH_PREDIV     0x0130

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* RTC handler declaration */
//RTC_HandleTypeDef RTCHandle;
#define RTCHandle hrtc
/* Private function prototypes -----------------------------------------------*/
static void SYSCLKConfig_STOP(void);
extern void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  This function configures the system to enter Sleep mode for
  *         current consumption measurement purpose.
  *         Sleep Mode
  *         ==========  
  *            - System Running at PLL (216MHz)
  *            - Flash 5 wait state
  *            - Instruction and Data caches ON
  *            - Prefetch ON   
  *            - Code running from Internal FLASH
  *            - All peripherals disabled.
  *            - Wakeup using EXTI Line (USER Button)
  * @param  None
  * @retval None
  */
void SleepMode_Measure(void)
{

   
//  /* Configure USER Button */
//  BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);

  /* Suspend Tick increment to prevent wakeup by Systick interrupt. 
     Otherwise the Systick interrupt will wake up the device within 1ms (HAL time base) */
  HAL_SuspendTick();
  
  /* Request to enter SLEEP mode */
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  
  /* Resume Tick interrupt if disabled prior to sleep mode entry */
  HAL_ResumeTick();
  

}

/**
  * @brief  This function configures the system to enter Stop mode with RTC 
  *         clocked by LSE or LSI  for current consumption measurement purpose.
  *         STOP Mode with RTC clocked by LSE/LSI
  *         =====================================   
  *           - RTC Clocked by LSE or LSI
  *           - Regulator in LP mode
  *           - HSI, HSE OFF and LSI OFF if not used as RTC Clock source
  *           - No IWDG
  *           - FLASH in deep power down mode
  *           - Automatic Wakeup using RTC clocked by LSE/LSI (~20s)
  * @param  None
  * @retval None
  */
void StopMode_Measure(void)
{
  RTCHandle.Instance = RTC;
    
  /* Configure RTC prescaler and RTC data registers as follow:
  - Hour Format = Format 24
  - Asynch Prediv = Value according to source clock
  - Synch Prediv = Value according to source clock
  - OutPut = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType = Open Drain */ 
  RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
  
  /*## Configure the Wake up timer ###########################################*/
  /*  RTC Wakeup Interrupt Generation:
      Wakeup Time Base = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI))
      Wakeup Time = Wakeup Time Base * WakeUpCounter 
                  = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI)) * WakeUpCounter
      ==> WakeUpCounter = Wakeup Time / Wakeup Time Base

      To configure the wake up timer to 20s the WakeUpCounter is set to 0xA017:
        RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16 
        Wakeup Time Base = 16 /(~32.768KHz) = ~0,488 ms
        Wakeup Time = ~20s = 0,488ms  * WakeUpCounter
        ==> WakeUpCounter = ~20s/0,488ms = 40983 = 0xA017 */
  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 0xA017, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

  /* FLASH Deep Power Down Mode enabled */
  HAL_PWREx_EnableFlashPowerDown();

  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

  /* Configures system clock after wake-up from STOP: enable HSE, PLL and select 
  PLL as system clock source (HSE and PLL are disabled in STOP mode) */
  SYSCLKConfig_STOP();
  
  /* Disable Wake-up timer */
  if(HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
}

/**
  * @brief  This function configures the system to enter Standby mode for
  *         current consumption measurement purpose.
  *         STANDBY Mode
  *         ============
  *           - Backup SRAM and RTC OFF
  *           - IWDG and LSI OFF
  *           - Wakeup using WakeUp Pin (PI.11)
  * @param  None
  * @retval None
  */
void StandbyMode_Measure(void)
{
  /* Disable all used wakeup sources: Pin1(PA.0) */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
  
  /* Clear the related wakeup pin flag */
  __HAL_PWR_CLEAR_WAKEUP_FLAG(PWR_WAKEUP_PIN_FLAG1);
  
  /* Re-enable all used wakeup sources: Pin1(PA.0) */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Request to enter STANDBY mode  */
  HAL_PWR_EnterSTANDBYMode();
}

/**
  * @brief  This function configures the system to enter Standby mode with RTC 
  *         clocked by LSE or LSI for current consumption measurement purpose.
  *         STANDBY Mode with RTC clocked by LSE/LSI
  *         ========================================
  *           - RTC Clocked by LSE/LSI
  *           - IWDG OFF
  *           - Backup SRAM OFF
  *           - Automatic Wakeup using RTC clocked by LSE/LSI (after ~20s)
  * @param  None
  * @retval None
  */
void StandbyRTCMode_Measure(void)
{ 
  RTCHandle.Instance = RTC;  
  /* Configure RTC prescaler and RTC data registers as follow:
  - Hour Format = Format 24
  - Asynch Prediv = Value according to source clock
  - Synch Prediv = Value according to source clock
  - OutPut = Output Disable
  - OutPutPolarity = High Polarity
  - OutPutType = Open Drain */ 
  RTCHandle.Init.HourFormat = RTC_HOURFORMAT_24;
  RTCHandle.Init.AsynchPrediv = RTC_ASYNCH_PREDIV;
  RTCHandle.Init.SynchPrediv = RTC_SYNCH_PREDIV;
  RTCHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
  RTCHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  RTCHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  
  if(HAL_RTC_Init(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
  
  /*## Configure the Wake up timer ###########################################*/
  /*  RTC Wakeup Interrupt Generation:
      Wakeup Time Base = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI))
      Wakeup Time = Wakeup Time Base * WakeUpCounter 
                  = (RTC_WAKEUPCLOCK_RTCCLK_DIV /(LSI)) * WakeUpCounter
      ==> WakeUpCounter = Wakeup Time / Wakeup Time Base

      To configure the wake up timer to 20s the WakeUpCounter is set to 0xA017:
        RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16 
        Wakeup Time Base = 16 /(~32.768KHz) = ~0,488 ms
        Wakeup Time = ~20s = 0,488ms  * WakeUpCounter
        ==> WakeUpCounter = ~20s/0,488ms = 40983 = 0xA017 */
  /* Disable Wake-up timer */
  if(HAL_RTCEx_DeactivateWakeUpTimer(&RTCHandle) != HAL_OK)
  {
    /* Initialization Error */
    Error_Handler(); 
  }
  
  /*## Clear all related wakeup flags ########################################*/  
  /* Clear RTC Wake Up timer Flag */
  __HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RTCHandle, RTC_FLAG_WUTF);
  
  /*## Setting the Wake up time ##############################################*/
  HAL_RTCEx_SetWakeUpTimer_IT(&RTCHandle, 0xA017, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
  
  /*## Enter the Standby mode ################################################*/
  /* Request to enter STANDBY mode  */
  HAL_PWR_EnterSTANDBYMode(); 
}

/**
  * @brief  This function configures the system to enter Standby mode with
  *         backup SRAM ON for current consumption measurement purpose.
  *         STANDBY Mode
  *         ============
  *           - RTC OFF
  *           - IWDG and LSI OFF
  *           - Backup SRAM ON
  *           - Wakeup using WakeUp Pin (PI.11)
  * @param  None
  * @retval None
  */
void StandbyBKPSRAMMode_Measure(void)
{
  /* Disable all used wakeup sources: Pin0(PA.0) */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Clear all related wakeup flags */
  __HAL_PWR_CLEAR_WAKEUP_FLAG(PWR_WAKEUP_PIN_FLAG1);

  /* Re-enable all used wakeup sources: Pin1(PA.0) */
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

  /* Enable BKPRAM Clock */
  __HAL_RCC_BKPSRAM_CLK_ENABLE();

  /* Enable the Backup SRAM low power Regulator */
  HAL_PWREx_EnableBkUpReg();

  /* Request to enter STANDBY mode  */
  HAL_PWR_EnterSTANDBYMode();
}


/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
static void SYSCLKConfig_STOP(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  uint32_t pFLatency = 0;
  
  /* Get the Oscillators configuration according to the internal RCC registers */
  HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
  
  /* After wake-up from STOP reconfigure the system clock: Enable HSE and PLL */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Get the Clocks configuration according to the internal RCC registers */
  HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
