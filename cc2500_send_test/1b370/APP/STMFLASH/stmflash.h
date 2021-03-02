#ifndef __STMFLASH_H__
#define __STMFLASH_H__

#include "system.h"

//typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

//#define FLASH_PAGE_SIZE         ((uint32_t)0x00000400)   /* FLASH Page Size */
   /* Start @ of user Flash area */
//#define FLASH_USER_END_ADDR     ((uint32_t)0x08007FFF)   /* End @ of user Flash area */
//#define DATA_32                 ((uint32_t)0x12345678)

void STMFLASH_Write(u16 *pBuffer,u16 NumToWrite);
void STMFLASH_Read(u32 *pBuffer,u32 NumToRead);

#endif
