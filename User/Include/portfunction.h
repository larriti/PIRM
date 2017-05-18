#ifndef _PORTFUNCTION_H
#define _PORTFUNCTION_H

/* STM32 Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"

/* FreeRTOS Includes ---------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Modbus Includes -----------------------------------------------------------*/
#include "mb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define REG_INPUT_START     0x0001
#define REG_INPUT_NREGS     8
#define REG_HOLDING_START 	0x0001
#define REG_HOLDING_NREGS 	8

/* Private variables ---------------------------------------------------------*/
uint16_t usRegInputStart = REG_INPUT_START;
extern uint16_t usRegInputBuf[REG_INPUT_NREGS];
uint16_t usRegHoldingStart = REG_HOLDING_START;
extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];


#endif
