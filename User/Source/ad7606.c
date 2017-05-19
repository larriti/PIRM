/**
  ******************************************************************************
  * @file    User/Source/ad7606.c
  * @author  wangxian
  * @version V1.6.1
  * @date    21-Sep-2016
  * @brief   dac program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ad7606.h"


/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
SemaphoreHandle_t xSemaphore_AD7606_Busy;
SemaphoreHandle_t xSemaphore_AD7606_Finished;

AD7606_CHx_Typedef AD7606_CHx;
uint16_t Sample_Counter = 0;
uint8_t Buffer_Status = 0;

/* Private function prototypes -----------------------------------------------*/
static void AD7606_Reset(void);
static void AD7606_StartConv(void);
static void AD7606_Delay(uint32_t nus);
static void AD7606_Timer_Config();
static void EXTI_Config(void);

/* Private functions ---------------------------------------------------------*/

static void AD7606_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_AHB1PeriphClockCmd(AD7606_PORT_CLK, ENABLE);

    GPIO_InitStruct.GPIO_Pin = AD7606_CS | AD7606_SCLK | AD7606_RST | AD7606_CONVST | \
                               AD7606_OS2 | AD7606_OS1 | AD7606_OS0;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AD7606_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = AD7606_BUSY | AD7606_DOUTA | AD7606_DOUTB;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(AD7606_PORT, &GPIO_InitStruct);
}

static void EXTI_Config(void)
{
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    //使能SYSCFGs时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(AD7606_EXTI_PORT, AD7606_BUSY_PIN_SOURCE);

    EXTI_InitStruct.EXTI_Line = AD7606_BUSY_EXTI_LINE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);

    /* Setting the timer nvic irq*/
    NVIC_InitStruct.NVIC_IRQChannel = AD7606_BUSY_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = AD7606_IRQ_PRIORITY;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

static void AD7606_Timer_Config()
{
    /* Calculate output compare value for timer2. */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    /* Enable the timer clock */
    RCC_APB1PeriphClockCmd(AD7606_TIMER_RCC, ENABLE);
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    /* To generated 10khz clock */
    TIM_TimeBaseInitStruct.TIM_Prescaler = 0;
    TIM_TimeBaseInitStruct.TIM_Period = (uint16_t)(APB1_TIM4_FREQ/AD7606_SAMPLE_FREQ)-1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(AD7606_TIMER_DEV, &TIM_TimeBaseInitStruct);
    TIM_ClearFlag(AD7606_TIMER_DEV, TIM_FLAG_Update);
    TIM_ITConfig(AD7606_TIMER_DEV, TIM_IT_Update, DISABLE);
    TIM_Cmd(AD7606_TIMER_DEV, DISABLE);

    /* Setting the timer nvic irq*/
    NVIC_InitStruct.NVIC_IRQChannel = AD7606_TIMER_IRQ;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = AD7606_IRQ_PRIORITY;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
}

void AD7606_Init(void)
{
    AD7606_GPIO_Config();
    AD7606_Timer_Config();
    EXTI_Config();
    AD7606_Delay(100);
    GPIO_ResetBits(AD7606_PORT, AD7606_RST);
    GPIO_SetBits(AD7606_PORT, AD7606_CS);
    GPIO_SetBits(AD7606_PORT, AD7606_CONVST);
    GPIO_SetBits(AD7606_PORT, AD7606_SCLK);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS2);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS1);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS0);
    AD7606_Delay(100);
    AD7606_Reset();
}

static void AD7606_Reset(void)
{
    GPIO_SetBits(AD7606_PORT, AD7606_RST);
    AD7606_Delay(1);
    GPIO_ResetBits(AD7606_PORT, AD7606_RST);
    AD7606_Delay(1);
}

static void AD7606_StartConv(void)
{
    GPIO_ResetBits(AD7606_PORT, AD7606_CONVST);
    AD7606_Delay(1);
    GPIO_SetBits(AD7606_PORT, AD7606_CONVST);
    AD7606_Delay(1);
}

static void AD7606_Delay(volatile uint32_t nus)
{
    while(nus--);
}

void AD7606_StartSample(void)
{
    TIM_ClearFlag(AD7606_TIMER_DEV, TIM_FLAG_Update);
    TIM_SetCounter(AD7606_TIMER_DEV, 0);
    TIM_ITConfig(AD7606_TIMER_DEV, TIM_IT_Update, ENABLE);
    TIM_Cmd(AD7606_TIMER_DEV, ENABLE);
}

void AD7606_StopSample(void)
{
    TIM_ClearFlag(AD7606_TIMER_DEV, TIM_FLAG_Update);
    TIM_SetCounter(AD7606_TIMER_DEV, 0);
    TIM_ITConfig(AD7606_TIMER_DEV, TIM_IT_Update, DISABLE);
    TIM_Cmd(AD7606_TIMER_DEV, DISABLE);
}

// AD7606数据采样
void AD7606_ReadOneSample(uint8_t Channels)
{
    uint8_t i,j;
    uint16_t TempA;

    GPIO_ResetBits(AD7606_PORT, AD7606_CS);
    for(i = 0; i < Channels; i++)
    {
        TempA = 0;
        for (j = 0; j < 16; j++)
        {
            GPIO_ResetBits(AD7606_PORT, AD7606_SCLK);
            TempA = TempA<<1;
            if(GPIO_ReadInputDataBit(AD7606_PORT, AD7606_DOUTA))
            {
                TempA |= 0x0001;
            }
            GPIO_SetBits(AD7606_PORT, AD7606_SCLK);
        }
        switch (i)
        {
            case 0:AD7606_CHx.AD7606_CH1[Buffer_Status][Sample_Counter] = (float)((int16_t)TempA*AD7606_RANGE/AD7606_MAX_NUM);break;
            case 1:AD7606_CHx.AD7606_CH2[Buffer_Status][Sample_Counter] = (float)((int16_t)TempA*AD7606_RANGE/AD7606_MAX_NUM);break;
            case 2:AD7606_CHx.AD7606_CH3[Buffer_Status][Sample_Counter] = (float)((int16_t)TempA*AD7606_RANGE/AD7606_MAX_NUM);break;
            case 3:AD7606_CHx.AD7606_CH4 = (float)((int16_t)TempA*AD7606_RANGE/AD7606_MAX_NUM);break;
            default:break;
        }
    }
    GPIO_SetBits(AD7606_PORT, AD7606_CS);
    Sample_Counter++;
    if(Sample_Counter >= SAMPLE_NUMBER)
    {
        Sample_Counter = 0;
        // 切换缓冲区
        if(Buffer_Status == 0)
            Buffer_Status = 1;
        else
            Buffer_Status = 0;
        //发送信号完成信号量
        xSemaphoreGive(xSemaphore_AD7606_Finished);
    }
}

// 2KHz 定时转换采样
void AD7606_TIMER_IRQ_HANDLER(void)
{
    if(TIM_GetITStatus(AD7606_TIMER_DEV, TIM_IT_Update) == SET)
    {
        AD7606_StartConv();
        TIM_ClearITPendingBit(AD7606_TIMER_DEV, TIM_IT_Update);
    }
}

// busy引脚中段通知进行数据读取
void AD7606_BUSY_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(EXTI_GetITStatus(AD7606_BUSY_EXTI_LINE) == SET)
    {
        if(xSemaphore_AD7606_Busy != NULL)
        {
            xSemaphoreGiveFromISR(xSemaphore_AD7606_Busy, &xHigherPriorityTaskWoken);
            EXTI_ClearITPendingBit(AD7606_BUSY_EXTI_LINE);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
