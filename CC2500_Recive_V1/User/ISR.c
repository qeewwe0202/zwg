/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : ISR.c
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
#include "ISR.h"
#include "gpio.h"

/**
  * @brief  Configure PA8(CC2500_GDO0) in interrupt mode
  * @param  None
  * @retval None
  */
void CC2500_Interrupt_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStru;
    EXTI_InitTypeDef EXTI_InitStru;

    /* Connect EXTI0 Line to PA8 pin */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource8);
    /* Configure EXTI8 line */
    EXTI_InitStru.EXTI_Line = EXTI_Line8;
    EXTI_InitStru.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStru.EXTI_Trigger = EXTI_Trigger_Falling; 
    EXTI_InitStru.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStru);

    /* Enable and set EXTI0 Interrupt to the lowest priority */
    NVIC_InitStru.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStru.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStru.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStru.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStru);
}



