/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : ISR.h
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��18��
  ����޸�   :
  ��������   : InsertFileHeader
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��18��
    ��    ��   : tofu
    �޸�����   : �����ļ�

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

