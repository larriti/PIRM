#ifndef _LED_H
#define _LED_H

/* STM32 Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"

/* FreeRTOS Includes ---------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Private define ------------------------------------------------------------*/
#define LED_PORT            ( GPIOB )
#define LED_PIN             ( GPIO_Pin_8 )
#define LED_CLOCK           ( RCC_AHB1Periph_GPIOB )

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
void LED_Init();
void LED_Toggle();

#endif
