/*******************************************************************************
 *
 * @file    usart.c
 * @author  wangxian
 * @version V1.0
 * @date    2016-04-08
 * @brief   USART program body
 *
 ******************************************************************************/

#include "usart.h"

/**
  * @brief  USART1 RCC Configration
  * @param  None
  * @retval None
  */
void USART1_RCC_Config(void)
{
  /* Enable the USART GPIO clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
}

/**
  * @brief  USART1 GPIO Configration
  * @param  None
  * @retval None
  */
void USART1_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStruction;

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
	/* GPIO Configration PA.09 output PA.10 input mode */
	GPIO_InitStruction.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruction.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruction.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruction.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruction.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruction);
}

/**
  * @brief  USART1 Mode Configration
  * @param  None
  * @retval None
  */
void USART1_Mode_Config(void)
{
  USART_InitTypeDef USART_InitStruction;

  /* Set the BaudRate to 115200 */
  USART_InitStruction.USART_BaudRate = 115200;
  USART_InitStruction.USART_WordLength = USART_WordLength_8b;
  USART_InitStruction.USART_StopBits = USART_StopBits_1;
  USART_InitStruction.USART_Parity = USART_Parity_No;
  USART_InitStruction.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_InitStruction.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART1, &USART_InitStruction);
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  USART1 NVIC Configration
  * @param  None
  * @retval None
  */
void USART1_NVIC_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStruction;

  NVIC_InitStruction.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStruction.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStruction.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStruction.NVIC_IRQChannelCmd = ENABLE;

  NVIC_Init(&NVIC_InitStruction);
}

/**
  * @brief  USART1 Configration
  * @param  None
  * @retval None
  */
void USART1_Config(void)
{
  USART1_RCC_Config();
  USART1_NVIC_Config();
  USART1_GPIO_Config();
  USART1_Mode_Config();
}

void USART1_Printf(uint8_t *buffer, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		USART_SendData(USART1, buffer[i]);
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	}
}

// /* Unix/Linux 重定向 */
int _write(int file, char *ptr, int len)
{
  uint16_t todo;
  for(todo = 0; todo < len; todo++)
  {
    USART_SendData(USART1, *ptr++);
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  }

  return len;
}

/* Windows 重定向 */
// 重定向c库函数printf到USART1
// int fputc(int ch, FILE *f)
// {
//   /* 发送一个字节数据到USART1 */
//   USART_SendData(USART1, (uint8_t) ch);
//   /* 等待发送完毕 */
//   while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
//
//   return (ch);
// }
//
// // 重定向c库函数scanf到USART1
// int fgetc(FILE *f)
// {
//   /* 等待串口1输入数据 */
//   while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET);
//
//   return (int)USART_ReceiveData(USART1);
// }
