/**
  ******************************************************************************
  * @file    User/Source/main.c
  * @author  wangxian
  * @version V1.6.1
  * @date    21-Sep-2016
  * @brief   Main program body
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
#include "main.h"

/* FreeRTOS Includes ---------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Modbus Includes -----------------------------------------------------------*/
#include "mb.h"

/* User Includes --------------------------------------------------------------*/
#include "dac.h"
#include "usart.h"
#include "ad7606.h"
#include "led.h"
#include "portfunction.h"

/* Private typedef -----------------------------------------------------------*/
typedef union {
	float org;
	uint8_t des[4];
}ftc_union;
ftc_union res_union;
ftc_union vol_union;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define CONTACT_RES			3.0			// 接触电阻3mR
#define SCALE 					31.0/36.0

/* Private variables ---------------------------------------------------------*/
AD7606_CHx_Vpp_Typedef AD7606_CHx_Vpp;
uint8_t WWDG_CNT = 0x7F;

/* Private function prototypes -----------------------------------------------*/
static void WWDG_Init(uint8_t tr, uint8_t wr, uint32_t fprer);
static void vLEDTask(void *pvParameters);
static void vMBTask(void *pvParameters);
static void vAD7606_Sample_Task(void *pvParameters);
static void vAD7606_Handle_Task(void *pvParameters);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	// 中断组选择
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	// 配置窗口看门狗，超时时间为t=1/42000000*4096*8*(0x7F-0x3F)=49.93ms
	WWDG_Init(0x7F, 0x7F, WWDG_Prescaler_8);
	// DAC 初始化配置
	DAC_Init_Config();
	// 串口1配置
	USART1_Config();

  /* Create task */
	xTaskCreate(vAD7606_Sample_Task, "AD7606 sample task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(vLEDTask, "LED task", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
  xTaskCreate(vMBTask, "Modbus task", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
	// 因为有printf,所以栈大小要比较大
	xTaskCreate(vAD7606_Handle_Task, "AD7606 handle task", 256, NULL, 4, NULL);

	/* Start task scheduler */
	vTaskStartScheduler();

	/* Infinite loop */
	while (1)
	{
	}
}

static void vLEDTask(void *pvParameters)
{
	LED_Init();
    for( ;; )
    {
        LED_Toggle();
        vTaskDelay(200);
    }
}

static void vMBTask( void *pvParameters )
{
	/* Select either ASCII or RTU Mode. */
	eMBInit( MB_RTU, 0x02, 0, 9600, MB_PAR_NONE );
	/* Enable the Modbus Protocol Stack. */
	eMBEnable();
	for( ;; )
	{
	    /* Call the main polling loop of the Modbus protocol stack. Internally
	     * the polling loop waits for a new event by calling the port
	     * dependent function xMBPortEventGet(  ). In the FreeRTOS port the
	     * event layer is built with queues.
	     */
	    ( void )eMBPoll();
	    vTaskDelay(50);
	}
}

static void vAD7606_Sample_Task(void *pvParameters)
{
	xSemaphore_AD7606_Busy = xSemaphoreCreateBinary();
	if ( xSemaphore_AD7606_Busy != NULL )
	{
		//初始化要将信号量take
		xSemaphoreTake(xSemaphore_AD7606_Busy, pdMS_TO_TICKS(1));
		AD7606_Init();
		AD7606_StartSample();
	}
	while(1)
	{
		if ( xSemaphore_AD7606_Busy != NULL )
		{
			// 等待AD7606 busy中断发送信号量
			if (xSemaphoreTake(xSemaphore_AD7606_Busy, portMAX_DELAY) == pdTRUE)
			{
				AD7606_ReadOneSample(4);
			}
		}
	}
}

static void vAD7606_Handle_Task(void *pvParameters)
{
	uint16_t i;
	uint8_t index = 0;
	uint8_t Vpp_index = 0;
	float ch1_min,ch1_max;
	float ch2_min,ch2_max;
	float power_res, power_volatage;
	float ch1_sum, ch1_avg, ch2_sum, ch2_avg;

	xSemaphore_AD7606_Finished = xSemaphoreCreateBinary();
	//初始化要将信号量take
	xSemaphoreTake(xSemaphore_AD7606_Finished, pdMS_TO_TICKS(1));

	while(1)
	{
		if(xSemaphoreTake(xSemaphore_AD7606_Finished, portMAX_DELAY) == pdTRUE)
		{
			// 如果缓冲区1在存数据则处理缓冲区0的数据,反之
			if(Buffer_Status)
				index = 0;
			else
				index = 1;
			ch1_min = 0;ch1_max = 0;ch2_min = 0;ch2_max = 0;
			for (i = 0; i < SAMPLE_NUMBER; i++)
			{
				if (AD7606_CHx.AD7606_CH1[index][i]<0)	// 判断是负电压则找出最小值
				{
					if (AD7606_CHx.AD7606_CH1[index][i]<ch1_min)
					{
						ch1_min  = AD7606_CHx.AD7606_CH1[index][i];
					}
				}
				else	// 判断是正电压则找出最大值
				{
					if (AD7606_CHx.AD7606_CH1[index][i]>ch1_max)
					{
						ch1_max  = AD7606_CHx.AD7606_CH1[index][i];
					}
				}
				if (AD7606_CHx.AD7606_CH2[index][i]<0)	// 判断是负电压则找出最小值
				{
					if (AD7606_CHx.AD7606_CH2[index][i]<ch2_min)
					{
						ch2_min  = AD7606_CHx.AD7606_CH2[index][i];
					}
				}
				else	// 判断是正电压则找出最大值
				{
					if (AD7606_CHx.AD7606_CH2[index][i]>ch2_max)
					{
						ch2_max  = AD7606_CHx.AD7606_CH2[index][i];
					}
				}
			}
			if(Vpp_index < FILTER_ORDER-1)
			{
				AD7606_CHx_Vpp.AD7606_CH1_Vpp[Vpp_index] = ch1_max-ch1_min;
				AD7606_CHx_Vpp.AD7606_CH2_Vpp[Vpp_index] = ch2_max-ch2_min;
				Vpp_index++;
			}
			else
			{
				// 20阶移动平均滤波
				AD7606_CHx_Vpp.AD7606_CH1_Vpp[Vpp_index] = ch1_max-ch1_min;		// 最新数据放到数组最后
				AD7606_CHx_Vpp.AD7606_CH2_Vpp[Vpp_index] = ch2_max-ch2_min;
				ch1_sum = 0;
				ch2_sum = 0;
				// 求20阶平均值
				for(i=0; i<FILTER_ORDER; i++)
				{
					ch1_sum += AD7606_CHx_Vpp.AD7606_CH1_Vpp[i];
					ch2_sum += AD7606_CHx_Vpp.AD7606_CH2_Vpp[i];
				}
				ch1_avg = ch1_sum/(float)FILTER_ORDER;
				ch2_avg = ch2_sum/(float)FILTER_ORDER;
				//数组向前移动一个
				for(i=1; i<FILTER_ORDER; i++)
				{
					AD7606_CHx_Vpp.AD7606_CH1_Vpp[i-1] = AD7606_CHx_Vpp.AD7606_CH1_Vpp[i];
					AD7606_CHx_Vpp.AD7606_CH2_Vpp[i-1] = AD7606_CHx_Vpp.AD7606_CH2_Vpp[i];
				}
				// 计算出电池内组
				power_res = ch1_avg/ch2_avg*AD7606_STANDARD_RES;
				// 减去接触电阻
				power_res = power_res*SCALE - CONTACT_RES;
				// 计算出电池电压
				power_volatage = AD7606_CHx.AD7606_CH4*(AD7606_POWER_R18+AD7606_POWER_R17)/AD7606_POWER_R18;
				// 浮点拆分成4个字节
				res_union.org = power_res;
				vol_union.org = power_volatage;
				usRegHoldingBuf[0] = res_union.des[3]*256+res_union.des[2];
				usRegHoldingBuf[1] = res_union.des[1]*256+res_union.des[0];
				usRegHoldingBuf[2] = vol_union.des[3]*256+vol_union.des[2];
				usRegHoldingBuf[3] = vol_union.des[1]*256+vol_union.des[0];
				// printf("Res=%.3fmR, Vp=%.3fV\r\n", power_res, power_volatage);
			}
		}
	}
}

static void WWDG_Init(uint8_t tr, uint8_t wr, uint32_t fprer)
{
	NVIC_InitTypeDef NVIC_InitStruct;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

	WWDG_CNT = tr&WWDG_CNT;
	WWDG_SetPrescaler(fprer);
	WWDG_SetCounter(WWDG_CNT);
	WWDG_SetWindowValue(wr);
	WWDG_Enable(WWDG_CNT);

	NVIC_InitStruct.NVIC_IRQChannel = WWDG_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	WWDG_ClearFlag();
	WWDG_EnableIT();
}

void WWDG_IRQHandler(void)
{
	WWDG_SetCounter(WWDG_CNT);
	WWDG_ClearFlag();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
