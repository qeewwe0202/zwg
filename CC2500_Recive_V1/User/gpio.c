/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : gpio.c
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��8��
  ����޸�   :
  ��������   : GPIO Configuration
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��8��
    ��    ��   : tofu
    �޸�����   : �����ļ�

******************************************************************************/
#include "gpio.h"


//PB11
void LED_PIN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStru;
    
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Pin = LED;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOB,&GPIO_InitStru);
}


//PB12~PB15  PA8 PA11
void CC2500_PIN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStru;

    //SPI_2 Interface
    GPIO_InitStru.GPIO_Pin = CC_SCLK + CC_MOSI;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB,&GPIO_InitStru);

    GPIO_InitStru.GPIO_Pin = CC_MISO;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB,&GPIO_InitStru);
    
    GPIO_InitStru.GPIO_Pin = CC_CSn;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB,&GPIO_InitStru);

    //GDO PINs
    GPIO_InitStru.GPIO_Pin = CC_GDO0 + CC_GDO2;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;

    GPIO_Init(GPIOA,&GPIO_InitStru);
}

//PA1~PA7  PB0~PB2
void NRF905_PIN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStru;

    GPIO_InitStru.GPIO_Pin = NRF_SCLK + NRF_MOSI;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA,&GPIO_InitStru);

    GPIO_InitStru.GPIO_Pin = NRF_MISO;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOA,&GPIO_InitStru);
    
    GPIO_InitStru.GPIO_Pin = NRF_CSn + NRF_PWR_UP + NRF_TRX_CE + NRF_TX_EN;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA,&GPIO_InitStru);

    GPIO_InitStru.GPIO_Pin = NRF_AM + NRF_CD + NRF_DR;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB,&GPIO_InitStru);
}

//PB5~PB9   PA12
void Nokia_PIN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStru;

    GPIO_InitStru.GPIO_Pin = Nokia_CLK + Nokia_RST + Nokia_DIN + 
                             Nokia_DC + Nokia_SCE ;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(GPIOB,&GPIO_InitStru);

    GPIO_InitStru.GPIO_Pin = Nokia_LIGHT;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA,&GPIO_InitStru);
}


//PA9,PA10
void USART1_PIN_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStru;
    GPIO_InitStru.GPIO_Pin = USART1_RX + USART1_TX;
    GPIO_InitStru.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStru.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA,&GPIO_InitStru);
}







