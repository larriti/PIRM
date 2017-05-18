/*
 * FreeModbus Libary: STM32 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: portserial.c,v 1.1 2006/11/02 23:14:44 wolti Exp $
 */

/* ----------------------- System includes ----------------------------------*/
#include "assert.h"

/* ----------------------- FreeRTOS includes --------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* ----------------------- STM32 includes ----------------------------------*/
#include "stm32f4xx.h"

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
#define MB_RTU_485              ( 1 )
#define MB_UART_DEV             ( USART3 )
#define MB_UART_AF              ( GPIO_AF_USART3 )
#define MB_UART_CLOCK           ( RCC_APB1Periph_USART3 )
#define MB_UART_PORT_CLOCK      ( RCC_AHB1Periph_GPIOC )
#define MB_UART_PORT            ( GPIOC )
#define MB_UART_TX_PIN          ( GPIO_Pin_10 )
#define MB_UART_TX_PIN_SOURCE   ( GPIO_PinSource10 )
#define MB_UART_RX_PIN          ( GPIO_Pin_11 )
#define MB_UART_RX_PIN_SOURCE   ( GPIO_PinSource11 )
#define MB_UART_IRQ_CH          ( USART3_IRQn )
#define MB_UART_IRQ_HANDLER     ( USART3_IRQHandler )
#define MB_IRQ_PRIORITY         ( 1 )

/* ----------------------RS485 EN -------------------------------------------*/
#ifdef MB_RTU_485
#define MB_485_EN_PORT          ( GPIOC )
#define MB_485_EN_PIN           ( GPIO_Pin_12 )
#endif

/* ----------------------- Start implementation -----------------------------*/

BOOL xMBPortSerialInit( UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    BOOL            xResult = TRUE;
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    ( void )ucPort;

    switch ( eParity )
    {
    case MB_PAR_EVEN:
        USART_InitStruct.USART_Parity = USART_Parity_Even;
        break;
    case MB_PAR_ODD:
        USART_InitStruct.USART_Parity = USART_Parity_Odd;
        break;
    case MB_PAR_NONE:
        USART_InitStruct.USART_Parity = USART_Parity_No;
        break;
    default: break;
    }

    switch ( ucDataBits )
    {
    case 7:
        if( eParity == MB_PAR_NONE )
        {
            /* not supported by our hardware. */
            xResult = FALSE;
        }
        else
        {
            USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        }
        break;
    case 8:
        if( eParity == MB_PAR_NONE )
        {
            USART_InitStruct.USART_WordLength = USART_WordLength_8b;
        }
        else
        {
            USART_InitStruct.USART_WordLength = USART_WordLength_9b;
        }
        break;
    default:
        xResult = FALSE;
    }

    if( xResult != FALSE )
    {
        /* Enable the USART GPIO AFIO clock */
        RCC_AHB1PeriphClockCmd(MB_UART_PORT_CLOCK, ENABLE);
        RCC_APB1PeriphClockCmd(MB_UART_CLOCK, ENABLE);

        GPIO_PinAFConfig(MB_UART_PORT, MB_UART_TX_PIN_SOURCE, MB_UART_AF);
        GPIO_PinAFConfig(MB_UART_PORT, MB_UART_RX_PIN_SOURCE, MB_UART_AF);
        /* GPIO Configration PA.09 output PA.10 input mode */
        GPIO_InitStruct.GPIO_Pin = MB_UART_TX_PIN | MB_UART_RX_PIN;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(MB_UART_PORT, &GPIO_InitStruct);

#ifdef MB_RTU_485      // 如果定义485则配置485发送接受使能引脚
        GPIO_InitStruct.GPIO_Pin = MB_485_EN_PIN;
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(MB_485_EN_PORT, &GPIO_InitStruct);
        GPIO_ResetBits(MB_485_EN_PORT, MB_485_EN_PIN);
#endif

        /* Configure the UART. */
        USART_InitStruct.USART_BaudRate = ulBaudRate;
        USART_InitStruct.USART_StopBits = USART_StopBits_1;
        USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
        USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
        USART_Init(MB_UART_DEV, &USART_InitStruct);
        USART_Cmd(MB_UART_DEV, ENABLE);

        /* Configure the IEC for the UART interrupts. */
        NVIC_InitStruct.NVIC_IRQChannel = MB_UART_IRQ_CH;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = MB_IRQ_PRIORITY;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStruct);
    }
    return xResult;
}

void vMBPortSerialClose(void)
{
    vMBPortSerialEnable(TRUE, FALSE);
    USART_Cmd(MB_UART_DEV, DISABLE);
}

void vMBPortClose(void)
{
    vMBPortSerialClose();
    vMBPortTimersDisable();
}

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    if( xRxEnable )
    {
        USART_ITConfig(MB_UART_DEV, USART_IT_RXNE, ENABLE);
        #ifdef MB_RTU_485
            //判断是不是发送完
            if(USART_GetFlagStatus(MB_UART_DEV, USART_FLAG_TXE)==SET)
            {
                //等待最后一个字节发送完
                while(USART_GetFlagStatus(MB_UART_DEV, USART_FLAG_TC)==RESET);
            }
            GPIO_ResetBits(MB_485_EN_PORT, MB_485_EN_PIN);
        #endif
    }
    else
    {
        USART_ITConfig(MB_UART_DEV, USART_IT_RXNE, DISABLE);
        #ifdef MB_RTU_485
            GPIO_SetBits(MB_485_EN_PORT, MB_485_EN_PIN);
        #endif
    }

    if( xTxEnable )
    {
        USART_ITConfig(MB_UART_DEV, USART_IT_TXE, ENABLE);
        // #ifdef MB_RTU_485
        //     GPIO_SetBits(MB_485_EN_PORT, MB_485_EN_PIN);
        // #endif
    }
    else
    {
        USART_ITConfig(MB_UART_DEV, USART_IT_TXE, DISABLE);
        // #ifdef MB_RTU_485
        //     GPIO_ResetBits(MB_485_EN_PORT, MB_485_EN_PIN);
        // #endif
    }
}

BOOL xMBPortSerialPutByte( CHAR ucByte )
{
    USART_SendData(MB_UART_DEV, ucByte);
    return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = USART_ReceiveData(MB_UART_DEV);
    return TRUE;
}

void MB_UART_IRQ_HANDLER(void)
{
    if(USART_GetITStatus(MB_UART_DEV, USART_IT_TXE) == SET)
    {
        //function eMBInit in mb.c pxMBFrameCBTransmitterEmpty = xMBRTUTransmitFSM
        pxMBFrameCBTransmitterEmpty();
        // USART_ClearITPendingBit(MB_UART_DEV, USART_IT_TXE);
    }
    if(USART_GetITStatus(MB_UART_DEV, USART_IT_RXNE) == SET)
    {
        //function eMBInit in mb.c pxMBFrameCBByteReceived = xMBRTUReceiveFSM
        pxMBFrameCBByteReceived();
        USART_ClearITPendingBit(MB_UART_DEV, USART_IT_RXNE);
    }
}
