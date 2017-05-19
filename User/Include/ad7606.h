#ifndef _AD7606_H
#define _AD7606_H

/* STM32 Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"

/* FreeRTOS Includes ---------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "dac.h"

/* Private define ------------------------------------------------------------*/
#define AD7606_TIMER_DEV            ( TIM4 )
#define AD7606_TIMER_RCC            ( RCC_APB1Periph_TIM4 )
#define AD7606_TIMER_IRQ            ( TIM4_IRQn )
#define AD7606_TIMER_IRQ_HANDLER    ( TIM4_IRQHandler )
#define AD7606_IRQ_PRIORITY         ( 5 )                       // 这里一定优先级要比系统低，大于5，中断调用
#define APB1_TIM4_FREQ         		((uint32_t)84000000)       	// TIM4 counter clock (prescaled APB1)
#define AD7606_SAMPLE_FREQ          ( 2000 )

#define AD7606_PORT                 GPIOC
#define AD7606_PORT_CLK             RCC_AHB1Periph_GPIOC
#define AD7606_EXTI_PORT            EXTI_PortSourceGPIOC
#define AD7606_BUSY                 GPIO_Pin_0
#define AD7606_BUSY_EXTI_LINE       EXTI_Line0
#define AD7606_BUSY_PIN_SOURCE      EXTI_PinSource0
#define AD7606_BUSY_IRQn            EXTI0_IRQn
#define AD7606_BUSY_IRQHandler      EXTI0_IRQHandler

#define AD7606_CS                   GPIO_Pin_1
#define AD7606_SCLK                 GPIO_Pin_2
#define AD7606_RST                  GPIO_Pin_3
#define AD7606_CONVST               GPIO_Pin_4
#define AD7606_OS2                  GPIO_Pin_5
#define AD7606_OS1                  GPIO_Pin_6
#define AD7606_OS0                  GPIO_Pin_7
#define AD7606_DOUTB                GPIO_Pin_13
#define AD7606_DOUTA                GPIO_Pin_14

#define BUFFER_COUNTER              2                                  //2个缓冲区缓存数据
#define FILTER_ORDER                20                                 //20阶滤波
#define SAMPLE_NUMBER               (AD7606_SAMPLE_FREQ/OUT_FREQ)      //每次采一个周期
#define AD7606_RANGE                10.0                               //AD7606测量范围
#define AD7606_MAX_NUM              32768.0
#define AD7606_STANDARD_RES         100.0            // 标准电阻0.1R, 100mR
#define AD7606_POWER_R18            3.090            // 测电源电压R18电阻为3.09K
#define AD7606_POWER_R17            56.0             // 测电源电压R17电阻为56K

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    float AD7606_CH1[BUFFER_COUNTER][SAMPLE_NUMBER];
    float AD7606_CH2[BUFFER_COUNTER][SAMPLE_NUMBER];
    float AD7606_CH3[BUFFER_COUNTER][SAMPLE_NUMBER];
    float AD7606_CH4;
} AD7606_CHx_Typedef;

typedef struct {
    float AD7606_CH1_Vpp[FILTER_ORDER];
    float AD7606_CH2_Vpp[FILTER_ORDER];
    float AD7606_CH3_Vpp[FILTER_ORDER];
}AD7606_CHx_Vpp_Typedef;

/* Private macro -------------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
extern SemaphoreHandle_t xSemaphore_AD7606_Finished;
extern SemaphoreHandle_t xSemaphore_AD7606_Busy;
extern AD7606_CHx_Typedef AD7606_CHx;
extern AD7606_CHx_Vpp_Typedef AD7606_CHx_Vpp;
extern uint8_t Buffer_Status;

/* Exported functions ------------------------------------------------------- */
void AD7606_Init(void);
void AD7606_StartSample(void);
void AD7606_StopSample(void);
void AD7606_ReadOneSample(uint8_t Channels);

#endif
