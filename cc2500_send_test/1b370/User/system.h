#ifndef __SYSTEM_H
#define __SYSTEM_H 	
#include "stdio.h"
#include "stdlib.h"
#include "stm32f0xx.h" 

/*#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE 
#endif
*/
int fputc(int ch, FILE *f);

typedef  __IO uint32_t   vu32;
typedef  uint32_t        u32;
typedef  uint16_t        u16;
typedef  int16_t         s16;
typedef  uint8_t         u8;

void Delay_us(u16 nus);
void Delay_ms(u16 nms);

#endif





























