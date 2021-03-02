#ifndef _SAVE_FLASH_H__
#define _SAVE_FLASH_H__

#include "stm32f10x.h"
#include "stmflash.h"

#define SAVE_ADDR_BASE 0x0803f800

void Save_Data(u8 *Data,u8 len);
u8 Read_Data(u8 i);

#endif 


