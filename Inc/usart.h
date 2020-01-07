/**
  ******************************************************************************
  * File Name          : USART.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usart_H
#define __usart_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "stm32f0xx_hal.h"
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

/* USER CODE BEGIN Private defines */
#define USART1_DMA_REC_SIZE 256
#define USART1_REC_SIZE 1024
	 
typedef struct   //������� DMAbuff �ͽ���Buff
{
	uint8_t Usart1RecFlag;        //�������ݽ��յ��ı�־λ
	uint16_t Usart1DMARecLen;    //����DMA�������ݵĳ���
	uint16_t Usart1RecLen;       //��������������������ݵĳ���
	
	uint8_t Usart1DMARecBuff[USART1_DMA_REC_SIZE];//����DMABuff
	uint8_t Usart1RecBuff[USART1_REC_SIZE];       //�������buff
}tsUsart1type;

extern tsUsart1type Usart1type;
	 
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);

/* USER CODE BEGIN Prototypes */
extern void EnableUartIT(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ usart_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
