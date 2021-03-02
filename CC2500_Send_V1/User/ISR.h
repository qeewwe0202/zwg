/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : ISR.h
  版 本 号   : 初稿
  作    者   : tofu
  生成日期   : 2013年7月18日
  最近修改   :
  功能描述   : InsertFileHeader
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月18日
    作    者   : tofu
    修改内容   : 创建文件

******************************************************************************/
#ifndef __ISR_H__
#define __ISR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "misc.h"
#include "stm32f10x_exti.h"

extern void CC2500_Interrupt_Config(void);

extern void USART1_Interrupt_Config(void);
extern void USART1_Disable_Interrupt(void);
extern void USART1_Enable_Interrupt(void);

extern void NRF905_Interrupt_config(void);
extern void NRF905_Disable_Interrupt(void);
extern void NRF905_Enable_Interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* __ISR_H__ */

