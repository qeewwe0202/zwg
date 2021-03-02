/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : usart.h
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
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x_usart.h"

extern void Uart_Config(void);
extern void Uart_Sendbyte(u8 byte);
extern void Uart_SendString(u8 *pStr);
extern void Uart_SendData(u8 *pData,u8 Len);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

