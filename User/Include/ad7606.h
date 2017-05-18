#ifndef _AD7606_H
#define _AD7606_H

/* STM32 Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define AD7606_PORT                 GPIOC
#define AD7606_PORT_CLK             RCC_AHB1Periph_GPIOC
#define AD7606_BUSY                 GPIO_Pin_0
#define AD7606_CS                   GPIO_Pin_1
#define AD7606_SCLK                 GPIO_Pin_2
#define AD7606_RST                  GPIO_Pin_3
#define AD7606_CONVST               GPIO_Pin_4
#define AD7606_OS2                  GPIO_Pin_5
#define AD7606_OS1                  GPIO_Pin_6
#define AD7606_OS0                  GPIO_Pin_7
#define AD7606_DOUTB                GPIO_Pin_13
#define AD7606_DOUTA                GPIO_Pin_14

/* Private macro -------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void AD7606_Init(void);
uint8_t AD7606_ReadOneSample(uint16_t *DoutA, uint16_t *DoutB, uint8_t Channels);

#endif
