#include "CC2500B.h"

void SPI_Configuration(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	 //开GPIO时
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);	 //开GPIO时钟

	//spi_CS片选功能，推挽输出，低电平有效
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_SetBits(GPIOA, GPIO_Pin_15);	//关闭cs

	//spi_SLCK
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//spi_MoSi
	GPIO_InitStruct.GPIO_Pin =  GPIO_Pin_5 ;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);		
}

u8 SPI_MISO_Input(void)
{
	u8 i=0;
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	 //开GPIO时
	
	//SPI_mi_1
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStruct.GPIO_Speed= GPIO_Speed_Level_3;	
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;  
	GPIO_InitStruct.GPIO_PuPd=	GPIO_PuPd_UP;	
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	i=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4);
	return i;
}

u8 RF_GDO_Input(void)
{
	u8 i=0;
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);	 //开GPIO时
	
	//spi_GDO2
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed= GPIO_Speed_Level_3;	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;  
	GPIO_InitStruct.GPIO_PuPd=	GPIO_PuPd_UP;	
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	i=GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);
	return i;
}


void SpiWriteByte(u8 bbyte)
{
	u8 i;
	for (i=0; i<8; i++)
	{
		if(bbyte&0x80){
			SPI_MOSI_1;
		}else{
			SPI_MOSI_0;
		}
		SPI_CLK_1;
		Delay_us(2);
		bbyte<<=1;
		SPI_CLK_0;
		Delay_us(2);
	}
}
u8 SpiReadByte(void)
{
	u8 i=0,bbyte=0;
	
	for (i=0; i<8; i++)
	{
		bbyte<<=1;
		SPI_CLK_1;
		if(SPI_MISO_IN==1){
			bbyte |= 0x01;
		}else{
			bbyte &= ~0x01;
		}
		Delay_us(2);
		SPI_CLK_0;
		Delay_us(2);
	}
	return bbyte;
}
void CC2500_WriteReg(u8 addr,u8 value)
{
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
	SpiWriteByte(addr);
	Delay_us(2);
	SpiWriteByte(value);
	Delay_us(2);
	SPI_CS_1;
}
u8 CC2500_ReadReg(u8 addr)
{
	u8 i;
	
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
	SpiWriteByte(addr|READ_SINGLE);
	Delay_us(2);
	i=SpiReadByte();
	Delay_us(2);
	SPI_CS_1;
	
	return (i);
}
void CC2500_Command(u8 cmd)
{
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
	SpiWriteByte(cmd);
	Delay_us(60);
	SPI_CS_1;
}

u8 CC2500_Init(void)
{
	 u8 CC2500_INIT_TAB[68]={
	 	 0x0B,0x07,0x0C,0x00,0x0D,0x5D,0x0E,0x93,0x0F,0xB1,0x10,0x2D,0x11,0x3B,0x12,0x73,
	     0x13,0xA2,0x14,0xF8,0x0A,0x00,0x15,0x01,0x21,0xB6,0x22,0x10,0x18,0x18,0x19,0x1D,
	     0x1A,0x1C,0x1B,0xC7,0x1C,0x00,0x1D,0xB0,0x23,0xEA,0x24,0x0A,0x25,0x00,0x26,0x11,
	     0x29,0x59,0x2C,0x88,0x2D,0x31,0x2E,0x0B,0x00,0x06,0x02,0x06,0x07,0x0C,0x08,0x04,
	     0x09,0x00,0x06,0x0D};
	u8 i,j;
	for(i=0;i<34;i++)
	{
		CC2500_WriteReg(CC2500_INIT_TAB[i*2],CC2500_INIT_TAB[i*2+1]);
		j=CC2500_ReadReg(CC2500_INIT_TAB[i*2]);
		if(j!=CC2500_INIT_TAB[i*2+1])
		{
		 	return(0);
		}
	}
	return(1);
}

/*****Power-On Reset with SRES---RFCSn_wave****
    ***_(30us)__           ___(45us)_____          
    ***         |_(30us)__|              |________
    **********************************************/
void CC2500_PowerUpAndReset(void)
{
	SPI_CLK_1;
    SPI_MOSI_0;
	
    SPI_CS_1;
  	Delay_us(30);
  	SPI_CS_0;
  	Delay_us(30);
  	SPI_CS_1;
  	Delay_us(300);
	
	SPI_CLK_0;		//这条语句不加，初始化不成功
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
	
	SpiWriteByte(CCxxx0_SRES_CMD30);
	
	//check busy or not
	while(SPI_MISO_IN==1);
	Delay_us(60);
	SPI_CS_1;
}

void CC2500_WriteBurstReg(u8 addr, u8 *buffer, u8 count)
//This function writes to multiple CCxxx0 register, using SPI burst access.
{
    u8 i;
	
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
    SpiWriteByte( addr | WRITE_BURST);
    Delay_us(30);
    for (i = 0; i < count; i++)
    {
        SpiWriteByte(buffer[i]);
        Delay_us(30);
    }
	SPI_CS_1;
}
void CC2500_ReadBurstReg(u8 addr, u8 *buffer, u8 count)
{
	u8 i;
	
	SPI_CS_0;
	//check busy or not
	while(SPI_MISO_IN==1);
	SpiWriteByte(addr | READ_BURST);
	Delay_us(30);
	for (i = 0; i < count; i++)
	{
		buffer[i] = SpiReadByte();
	}
	SPI_CS_1;
}

u8 RF_Init(void)//上电初始化
{
	u8  paTable[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	
	CC2500_PowerUpAndReset();
	if(!CC2500_Init())
		return 0;
	CC2500_Command(CCxxx0_SFRX_CMD3A);	
    Delay_us(60);
    CC2500_Command(CCxxx0_SFTX_CMD3B);
    Delay_us(60);
	
	CC2500_WriteBurstReg(CCxxx0_PATABLE, paTable, sizeof(paTable));
	return 1;
}
void Rf_SendMode(u8 chn)//发送模式
{
	CC2500_WriteReg(CCxxx0_CHANNR,chn);//频点
	CC2500_Command(CCxxx0_SIDLE_CMD36);
}

u8 Rf_SendData(u8 *ptr,u8 len) //发送数据
{
	CC2500_WriteBurstReg(CCxxx0_TXFIFO, ptr,len);	//写入要发送的数据
	CC2500_Command(CCxxx0_STX_CMD35);		                                   //进入发送模式发送数据

	while (RF_GDO_IN==0);// Wait for GDO0 to be set -> sync transmitted
	while (RF_GDO_IN==1);// Wait for GDO0 to be cleared -> end of packet
	CC2500_Command(CCxxx0_SFTX_CMD3B);          //刷新发送缓冲区
	//切换为接收
	CC2500_Command(CCxxx0_SIDLE_CMD36);
	CC2500_Command(CCxxx0_SFRX_CMD3A);
	CC2500_Command(CCxxx0_SRX_CMD34);
	return(1);
}


