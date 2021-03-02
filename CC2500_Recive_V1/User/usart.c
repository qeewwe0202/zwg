/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : usart.c
  版 本 号   : 初稿
  作    者   : tofu
  生成日期   : 2013年7月8日
  最近修改   :
  功能描述   : InsertFileHeader
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月8日
    作    者   : tofu
    修改内容   : 创建文件

******************************************************************************/
#include "usart.h"

void Uart_Config(void)
{
    USART_InitTypeDef USART1_InitStruct;
    USART1_InitStruct.USART_BaudRate = 115200;
    USART1_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART1_InitStruct.USART_StopBits = USART_StopBits_1;
    USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART1_InitStruct.USART_Parity = USART_Parity_No;
    USART1_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1,&USART1_InitStruct);
    USART_ClearFlag(USART1,USART_FLAG_RXNE);
    USART_ClearFlag(USART1,USART_FLAG_TXE);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1,ENABLE);
}

void Uart_Sendbyte(u8 byte)
{
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    USART_SendData(USART1, byte);
}

void Uart_SendString(u8 * pStr)
{
    while(*pStr!='\0')
    {
        Uart_Sendbyte(*pStr++);
    }
}

void Uart_SendData(u8 * pData,u8 Len)
{
    u8 i;
    for(i=0;i<Len;i++)
    {
        Uart_Sendbyte(pData[i]);
    }
}




