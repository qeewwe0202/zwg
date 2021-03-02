/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : spi.c
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��8��
  ����޸�   :
  ��������   : configure SPI
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��8��
    ��    ��   : tofu
    �޸�����   : �����ļ�

******************************************************************************/
#include "spi.h"
#include "gpio.h"

#if 0
void SPI_Config(SPI_TypeDef * SPIx)
{
    SPI_InitTypeDef SPI_Init_Stru;

    SPI_Cmd(SPIx,DISABLE);
    SPI_Init_Stru.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //ȫ˫��
    SPI_Init_Stru.SPI_Mode = SPI_Mode_Master;       //Stm32Ϊ���豸
    SPI_Init_Stru.SPI_DataSize = SPI_DataSize_8b;   //ÿ֡����8bit
    SPI_Init_Stru.SPI_CPHA = SPI_CPHA_1Edge;    //��ͬ��ʱ�ӵĵ�һ���ز�������
    SPI_Init_Stru.SPI_CPOL = SPI_CPOL_Low;  //����̬���ڵ͵�ƽ�����������ز�������
    SPI_Init_Stru.SPI_NSS = SPI_NSS_Soft;   //�������
    SPI_Init_Stru.SPI_CRCPolynomial = 7;    //
    SPI_Init_Stru.SPI_FirstBit = SPI_FirstBit_MSB;//��λ��ǰ
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


