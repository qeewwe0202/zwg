#include "stm32f10x_rcc.h"
#include "common.h"
#include "cc2500.h"
#include "usart.h"
#include "spi.h"
#include "gpio.h"
#include "5110LCD.h"
#include "ISR.h"
#include "CC2500_DEF.h"

void RCC_Configuration(void);

ErrorStatus volatile gFlag = ERROR;
u8 count = 0;

u8 gCC2500RxBuf[];
u8 gCC2500TxBuf[] = {0x06,0x01,0x02,0x03,0x04,0x05,0x06};

int main(void)
{
  u8 status=0xff;    
  ROUTER_WORKSTATE_ENUM ROUTER_WORKSTATE;
  LABEL_WORKSTATE_ENUM LABEL_WORKSTATE;
	SystemInit();
	RCC_Configuration();
	LED_PIN_Config();
	LED_OFF;
	Nokia_PIN_Config();
	CC2500_PIN_Config();
	LCD_Init();
	LCD_WriteString_en(21,1,"FreeTeam");
  CC2500_Init();
  CC2500_ModeIdle();

//when Rx mode
#if 0
    
    CC2500_Interrupt_Config();
    CC2500_ModeRx();
    while(1);

#endif

//When Tx Packets
#if 1
	while(1)
	{
        delay_ns(8);
        LED_ON;
        delay_ns(2);
        LED_OFF;
        CC2500_SendPacket();
	}
#endif
	
}

void RCC_Configuration(void)
{
    ErrorStatus HSEStartUpStatus; 
    RCC_DeInit(); 
    RCC_HSEConfig(RCC_HSE_ON);
    HSEStartUpStatus = RCC_WaitForHSEStartUp(); 
    if(HSEStartUpStatus == SUCCESS)
    {
        RCC_HCLKConfig(RCC_SYSCLK_Div1);    //APB  = SYSCLK
        RCC_PCLK2Config(RCC_HCLK_Div2);     //APB2 = HCLK
        RCC_PCLK1Config(RCC_HCLK_Div2);     //APB1 = HCLK/2
        //FLASH_SetLatency(FLASH_Latency_2);
        //FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);
        RCC_PLLCmd(ENABLE);
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
        while(RCC_GetSYSCLKSource() != 0x08); 
    }//启用外部高速晶振，主时钟频率72M；SYSCLK = 72M
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE); //
    delay_init(72);
}



