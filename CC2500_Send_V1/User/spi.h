/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : spi.h
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��8��
  ����޸�   :
  ��������   : InsertFileHeader
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��8��
    ��    ��   : tofu
    �޸�����   : �����ļ�

******************************************************************************/
#ifndef __SPI_H__
#define __SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x_spi.h"
#include "common.h"

//extern void SPI_Config(SPI_TypeDef * SPIx);

extern u8   CC2500_SPI_Read(void);
extern void CC2500_SPI_Write(u8 data);
extern u8   CC2500_SPI_RW(u8 Addr);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_H__ */




