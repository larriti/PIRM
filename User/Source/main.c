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

/* Modbus Includes -----------------------------------------------------------*/
#include "mb.h"

/* DAC Includes --------------------------------------------------------------*/
#include "dac.h"
#include "usart.h"
#include "ad7606.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_PORT            ( GPIOB )
#define LED_PIN             ( GPIO_Pin_8 )
#define LED_CLOCK           ( RCC_AHB1Periph_GPIOB )

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static void vLEDInit();
static void vLEDToggle();
static void vLEDTask(void *pvParameters);
static void vMBTask(void *pvParameters);
static void vAD7606Task(void *pvParameters);
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
	// DAC 初始化配置
	DAC_Init_Config();
	// 串口1配置
	USART1_Config();

    /* Create led task */
	xTaskCreate(vAD7606Task, "AD7606 task", 2048, NULL, 3, NULL);
    xTaskCreate(vLEDTask, "LED task", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(vMBTask, "Modbus task", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    /* Start task scheduler */
    vTaskStartScheduler();

	/* Infinite loop */
	while (1)
	{
	}
}

void vLEDInit()
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_AHB1PeriphClockCmd(LED_CLOCK, ENABLE);

    GPIO_InitStruct.GPIO_Pin = LED_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(LED_PORT, &GPIO_InitStruct);
    GPIO_SetBits(LED_PORT, LED_PIN);
}

void vLEDToggle()
{
    if(GPIO_ReadOutputDataBit(LED_PORT, LED_PIN) == 0)
    {
        GPIO_SetBits(LED_PORT, LED_PIN);
    }
    else
    {
        GPIO_ResetBits(LED_PORT, LED_PIN);
    }
}

static void vLEDTask(void *pvParameters)
{
	vLEDInit();
    for( ;; )
    {
        vLEDToggle();
        vTaskDelay(200);
    }
}

static void vMBTask( void *pvParameters )
{
    /* Select either ASCII or RTU Mode. */
    eMBInit( MB_RTU, 0x01, 0, 9600, MB_PAR_NONE );
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

static void vAD7606Task(void *pvParameters)
{
	uint16_t DataA[4];
	uint16_t DataB[4];
	int16_t DataA0[1000];
	int16_t DataA1[1000];
	float num;
	uint16_t i;

	AD7606_Init();
	while(1)
	{
		// AD7606_ReadOneSample(DataA, DataB, 4);
		// num = (int16_t)DataA[3]*10.0/32768.0;
		// printf("%f\r\n", num);
		for (i=0; i<1000; i++)
		{
			AD7606_ReadOneSample(DataA, DataB, 4);
			DataA0[i] = (int16_t)DataA[0];
			DataA1[i] = (int16_t)DataA[1];
			vTaskDelay(1);
		}
		printf("----------CH1--------\r\n");
		for (i=0; i<1000; i++)
		{
			num = (int16_t)DataA0[i]*10.0/32768.0;
			printf("%f\r\n", num);
		}
		printf("----------CH2--------\r\n");
		for (i=0; i<1000; i++)
		{
			num = (int16_t)DataA1[i]*10.0/32768.0;
			printf("%f\r\n", num);
		}
		printf("\r\n");

		vTaskDelay(10);
	}
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
