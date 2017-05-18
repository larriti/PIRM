#ifndef __USART_H_
#define __USART_H_

#include "stm32f4xx.h"
#include <stdio.h>

void USART1_GPIO_Config(void);
void USART1_Mode_Config(void);
void USART1_NVIC_Config(void);
void USART1_Config(void);
void USART1_Printf(uint8_t *buffer, int len);

#endif
