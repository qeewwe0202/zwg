#ifndef __key_H__
#define __key_H__

#include "stm32f0xx.h"
#include "system.h"

#define		KEYCLOSE  	GPIO_Pin_13 
#define		KEYOPEN   	GPIO_Pin_14 
#define		KEYEN  	 		GPIO_Pin_15 
#define		PWR_KEEP  	GPIO_Pin_10 

#define		NUM1  			GPIO_Pin_12 
#define		NUM2   			GPIO_Pin_11 
#define		NUM3  	 		GPIO_Pin_2 
#define		NUM4  			GPIO_Pin_14 

#define 	RX_EN				GPIO_SetBits(GPIOB, GPIO_Pin_0)
#define 	TX_EN				GPIO_SetBits(GPIOB, GPIO_Pin_1)

#define 	RX_OFF			GPIO_ResetBits(GPIOB, GPIO_Pin_0)
#define 	TX_OFF			GPIO_ResetBits(GPIOB, GPIO_Pin_1)	

//#define 	ALL_STOP		{Rf_SendKeyValue(0x80,Clean_flag);Delay_ms(10);Rf_SendKeyValue_2(0x80,Clean_flag);}	//zwg 2019.02.28
#define 	ALL_STOP		{Rf_SendKeyValue(0x80,0xff);}//Delay_ms(10);Rf_SendKeyValue_2(0x80,0xff);}	//zwg 2019.02.28

#define 	PWR_ON	  	GPIO_SetBits(GPIOB,PWR_KEEP)
#define 	PWR_OFF	  	GPIO_ResetBits(GPIOB,PWR_KEEP)

#define 	KEY_EN	  	GPIO_ReadInputDataBit(	GPIOC	,	KEYEN	)
#define 	KEY_OPEN	  GPIO_ReadInputDataBit(	GPIOC	,	KEYOPEN )
#define 	KEY_CLOSE		GPIO_ReadInputDataBit(	GPIOC	,	KEYCLOSE )

#define		KEY_NUM1  	GPIO_ReadInputDataBit(	GPIOB	,	NUM1	)
#define		KEY_NUM2   	GPIO_ReadInputDataBit(	GPIOB	,	NUM2	)
#define		KEY_NUM3  	GPIO_ReadInputDataBit(	GPIOB	,	NUM3	)
#define		KEY_NUM4  	GPIO_ReadInputDataBit(	GPIOB	,	NUM4	)
#define		KEY_NUMall ((KEY_NUM1==1)&&(KEY_NUM2==1)&&(KEY_NUM3==1)&&(KEY_NUM4==1))


#define   KEY_ON			0x03
#define   KEY_out			0x01
#define		KEY_in			0x02

#define   LED_Red			0xD1
#define   LED_Green		0xD2
#define		LED_Blue		0xD3
#define		LED_White		0xD4
#define		LED_Off		  0xD5

#define	  KEY_study		0xA0
#define	  KEY_clean_enter	0xB0
//zwg
#define	  KEY_easy_clean_enter	0xC0
#define	  KEY_clean_exit	0xD0

#define  OPEN_LED	0xAB
#define  CLOSE_LED  0xCD
#define	  LED_ON	0x99
#define	  LED_OFF	0x98

extern unsigned int openKey_TimeDelay;
extern unsigned int openKey_pressNum;

extern unsigned int closeKey_TimeDelay;
extern unsigned int closeKey_pressNum;



void lcd1602key_Init(void);
u8 	 lcd1602key_Scan(void);
u8 	 Password_keyScan(void);	
void lcd1602key_Handle(void);

#endif
