#ifndef __lcd1602_H__
#define __lcd1602_H__

#include "stm32f0xx.h"
#include "system.h"

#define RS_SET GPIO_SetBits(GPIOA,GPIO_Pin_10)
#define RS_CLR GPIO_ResetBits(GPIOA,GPIO_Pin_10)

#define RW_CLR GPIO_ResetBits(GPIOA,GPIO_Pin_9)
#define RW_SET GPIO_SetBits(GPIOA,GPIO_Pin_9)

#define EN_CLR GPIO_ResetBits(GPIOA,GPIO_Pin_8)
#define EN_SET GPIO_SetBits(GPIOA,GPIO_Pin_8)

#define LCD_DATA ( GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 )

void LCD_Init(void);
void GPIO_Write_8bit(u8 Com_data);
u8 LCD_check_busy(void);
void LCD_initwrite_com(unsigned char com);
void LCD_write_com(unsigned char com);
void LCD_write_data(unsigned char Data);
void LCD_Clear(void);
void LCD_write_string(unsigned char x,unsigned char y,unsigned char *s );
void LCD_write_char(unsigned char x,unsigned char y,unsigned char data);
void LCD_INIT(void);

#endif
