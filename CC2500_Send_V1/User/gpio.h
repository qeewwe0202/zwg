/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : gpio.h
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x_gpio.h"

//NRF905 PINs --->SPI_1
#define NRF_CSn     GPIO_Pin_4      //PA4
#define NRF_SCLK    GPIO_Pin_5      //PA5    
#define NRF_MISO    GPIO_Pin_6      //PA6
#define NRF_MOSI    GPIO_Pin_7      //PA7
#define NRF_TX_EN   GPIO_Pin_1      //PA1
#define NRF_TRX_CE  GPIO_Pin_2      //PA2
#define NRF_PWR_UP  GPIO_Pin_3      //PA3
#define NRF_CD      GPIO_Pin_0      //PB0
#define NRF_AM      GPIO_Pin_1      //PB1
#define NRF_DR      GPIO_Pin_10     //PB10
    
//CC2500 PINs --->SPI_2
#define CC_CSn      GPIO_Pin_12     //PB12
#define CC_SCLK     GPIO_Pin_13     //PB13
#define CC_MISO     GPIO_Pin_14     //PB14
#define CC_MOSI     GPIO_Pin_15     //PB15
#define CC_GDO0     GPIO_Pin_8      //PA8
#define CC_GDO2     GPIO_Pin_11     //PA11

#define CC_CSn_L    (GPIO_ResetBits(GPIOB,CC_CSn))   
#define CC_CSn_H    (GPIO_SetBits(GPIOB,CC_CSn))

#define CC_SLK_H    (GPIO_SetBits(GPIOB,CC_SCLK))
#define CC_SLK_L    (GPIO_ResetBits(GPIOB,CC_SCLK))

#define CC_MO_H     (GPIO_SetBits(GPIOB,CC_MOSI))
#define CC_MO_L     (GPIO_ResetBits(GPIOB,CC_MOSI))

#define CC2500_MI   GPIO_ReadInputDataBit(GPIOB,CC_MISO)
#define GDO0_IN 	GPIO_ReadInputDataBit(GPIOA,CC_GDO0)
#define GDO2_IN     GPIO_ReadInputDataBit(GPIOA,CC_GDO2)

//Nokia 5110 PINs
#define Nokia_CLK   GPIO_Pin_5      //PB5
#define Nokia_DIN   GPIO_Pin_6      //PB6
#define Nokia_DC    GPIO_Pin_7      //PB7
#define Nokia_SCE   GPIO_Pin_8      //PB8
#define Nokia_RST   GPIO_Pin_9      //PB9
#define Nokia_LIGHT GPIO_Pin_12     //PA12

//LED PIN
#define LED         GPIO_Pin_11     //PB11
#define LED_ON  (GPIO_SetBits(GPIOB,LED))
#define LED_OFF (GPIO_ResetBits(GPIOB,LED))
//USART
#define USART1_RX   GPIO_Pin_10     //PA10
#define USART1_TX   GPIO_Pin_9      //PA9

//Funciton Declaration
extern void CC2500_PIN_Config(void);
extern void NRF905_PIN_Config(void);
extern void Nokia_PIN_Config(void);
extern void LED_PIN_Config(void);
extern void USART1_PIN_Config(void);

#ifdef __cplusplus
}
#endif

#endif /* __GPIO_H__ */






