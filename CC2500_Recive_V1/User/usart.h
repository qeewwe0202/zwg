/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : usart.h
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

