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

/* FreeRTOS Includes ---------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/** @addtogroup Template_Project
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void AD7606_Reset(void);
static void AD7606_StartConv(void);
static void AD7606_Delay(uint32_t nus);
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

void AD7606_Init(void)
{
    AD7606_GPIO_Config();
    vTaskDelay(1);
    GPIO_ResetBits(AD7606_PORT, AD7606_RST);
    GPIO_SetBits(AD7606_PORT, AD7606_CS);
    GPIO_SetBits(AD7606_PORT, AD7606_CONVST);
    GPIO_SetBits(AD7606_PORT, AD7606_SCLK);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS2);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS1);
    GPIO_ResetBits(AD7606_PORT, AD7606_OS0);
    vTaskDelay(1);
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

void AD7606_Delay(volatile uint32_t nus)
{
    while(nus--);
}

uint8_t AD7606_ReadOneSample(uint16_t *DoutA, uint16_t *DoutB, uint8_t Channels)
{
    uint8_t i,j,busy;
    uint16_t TempA, TempB;

    AD7606_StartConv();

    busy = GPIO_ReadInputDataBit(AD7606_PORT, AD7606_BUSY);
    while (busy == 1) {
        busy = GPIO_ReadInputDataBit(AD7606_PORT, AD7606_BUSY);
    }

    GPIO_ResetBits(AD7606_PORT, AD7606_CS);
    for(i = 0; i < Channels; i++)
    {
        TempA = 0;
        TempB = 0;
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
        DoutA[i] = TempA;
        DoutB[i] = TempB;
    }
    GPIO_SetBits(AD7606_PORT, AD7606_CS);
    return 1;
}
