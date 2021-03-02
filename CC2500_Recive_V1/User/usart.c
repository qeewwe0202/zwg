/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : usart.c
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��8��
  ����޸�   :
  ��������   : InsertFileHeader
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��8��
    ��    ��   : tofu
    �޸�����   : �����ļ�

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




