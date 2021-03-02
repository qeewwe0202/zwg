/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : spi.h
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




