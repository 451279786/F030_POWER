#ifndef __DHT11_H
#define __DHT11_H 

#include "stm32f0xx_hal.h"
 
#define DHT11_Dout_PORT        GPIOA
#define DHT11_Dout_PIN         GPIO_PIN_0

#define DHT11_Dout_LOW()   HAL_GPIO_WritePin(DHT11_Dout_PORT,DHT11_Dout_PIN,GPIO_PIN_RESET)
#define DHT11_Dout_HIGH()  HAL_GPIO_WritePin(DHT11_Dout_PORT,DHT11_Dout_PIN,GPIO_PIN_SET)
#define DHT11_Data_IN()    HAL_GPIO_ReadPin(DHT11_Dout_PORT,DHT11_Dout_PIN)



uint8_t DHT11_Init(void);//初始化DHT11
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi);//读取温湿度
uint8_t DHT11_Read_Byte(void);//读出一个字节
uint8_t DHT11_Read_Bit(void);//读出一个位
uint8_t DHT11_Check(void);//检测是否存在DHT11
void DHT11_Rst(void);//复位DHT11  

void delay_us(uint32_t nus);
	
void DHT11_Mode_IPU(void);
void DHT11_Mode_Out_PP(void);
void Int_To_Str(int x,char *Str);
#endif















