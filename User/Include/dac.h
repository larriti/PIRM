#ifndef _DAC_H
#define _DAC_H

/* STM32 Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define OUT_FREQ          		20.0                               				// Output waveform frequency
#define OUT_AMP					2000.0											// Output wavefore amplitude(mv)
#define OUT_OFFSET				(OUT_AMP/2)										// Output offset voltage(mv)
#define DAC_VREF				3280.0											// DAC VREF voltage(mv)
#define SINE_LENGTH				1024											// Sine wave lookup table length
#define DAC_DHR12R1_ADDRESS  	((uint32_t)0x40007408)							// DMA writes into this reg on every request
#define CNT_FREQ          		((uint32_t)84000000) 							// TIM2 counter clock (prescaled APB1)
#define TIM_PERIOD        		((float)CNT_FREQ/(SINE_LENGTH*OUT_FREQ))		// Autoreload reg value

#define DAC_TIM					TIM2
#define DAC_TIM_CLOCK			RCC_APB1Periph_TIM2
#define DAC_TIM_TRIG			DAC_Trigger_T2_TRGO

/* Exported functions ------------------------------------------------------- */
void DAC_Init_Config(void);
void DAC_Config(void);
void DAC_TIM_Config(void);
void DAC_DMA_Config(void);
void Amp_Convert(void);

#endif
