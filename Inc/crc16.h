#ifndef _CRC16_H
#define _CRC16_H
#include "stm32f0xx.h"

void ReIwdg(void);
	
uint16_t ISRcal_crc16(uint8_t *p_data,uint8_t leng) ;
char* strlwr(char *str);

#endif
