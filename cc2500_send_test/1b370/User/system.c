#include "system.h"
#include "stm32f0xx.h"

int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (u8) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	return ch;
}

//only for 24MHz
void Delay_us(u16 nus)
{
    while(nus--)
    {
        nus++;
        nus--;
        nus++;
        nus--;        
        nus++;
        nus--;
        nus++;
        nus--;
    }
}

void Delay_ms(u16 nms)
{	 		  	  
    u16 i;
    while(nms--)
    {
        for(i = 0; i < 3000; i++);
    }
} 

































