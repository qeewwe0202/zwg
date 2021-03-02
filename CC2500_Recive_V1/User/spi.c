/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : spi.c
  版 本 号   : 初稿
  作    者   : tofu
  生成日期   : 2013年7月8日
  最近修改   :
  功能描述   : configure SPI
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月8日
    作    者   : tofu
    修改内容   : 创建文件

******************************************************************************/
#include "spi.h"
#include "gpio.h"

#if 0
void SPI_Config(SPI_TypeDef * SPIx)
{
    SPI_InitTypeDef SPI_Init_Stru;

    SPI_Cmd(SPIx,DISABLE);
    SPI_Init_Stru.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //全双工
    SPI_Init_Stru.SPI_Mode = SPI_Mode_Master;       //Stm32为主设备
    SPI_Init_Stru.SPI_DataSize = SPI_DataSize_8b;   //每帧数据8bit
    SPI_Init_Stru.SPI_CPHA = SPI_CPHA_1Edge;    //在同步时钟的第一个沿采样数据
    SPI_Init_Stru.SPI_CPOL = SPI_CPOL_Low;  //空闲态处于低电平，即在上升沿采样数据
    SPI_Init_Stru.SPI_NSS = SPI_NSS_Soft;   //软件控制
    SPI_Init_Stru.SPI_CRCPolynomial = 7;    //
    SPI_Init_Stru.SPI_FirstBit = SPI_FirstBit_MSB;//高位在前
    SPI_Init_Stru.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//72M/2/256
    SPI_Init(SPIx,&SPI_Init_Stru);

    SPI_Cmd(SPIx,ENABLE);    
}
#endif

void CC2500_SPI_Write(u8 data)
{
    u8 cnt;
    for(cnt = 0; cnt < 8; cnt++)
    {
        if(data&0x80)
            CC_MO_H;
        else
            CC_MO_L;
        data<<=1;
        CC_SLK_L;
        delay_nus(1);
        CC_SLK_H;
    }
}

u8 CC2500_SPI_Read(void)
{
    u8 cnt;
    u8 data = 0;

    for(cnt=0;cnt<8;cnt++)
    {
        data<<=1;
        CC_SLK_L;
        delay_nus(1);
        CC_SLK_H;
        if(CC2500_MI)
            data++;
    }
    return data;
}

u8 CC2500_SPI_RW(u8 Addr)
{
    u8 cnt;
    u8 data = 0;
    
    for(cnt = 0;cnt < 8;cnt++)
    {
        data<<=1;
        if(Addr&0x80)
            CC_MO_H;
        else
            CC_MO_L;
        CC_SLK_L;
        delay_nus(1);
        CC_SLK_H;
        if(CC2500_MI)
            data++;
    }
    return data;
}


