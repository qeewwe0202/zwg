#include "save_flash.h"


void Save_Data(u8 *Data,u8 len)
{
	s32 temp;
	u8 i;
	FLASH_Unlock();
	FLASH_ErasePage(SAVE_ADDR_BASE);//保存在芯片FLASH前，先进行页删除
	for(i=0;i<=len;i=i+2)
	{
		temp = Data[i];
		FLASH_ProgramHalfWord(SAVE_ADDR_BASE+i, temp);
	}
	FLASH_Lock();//上锁
}

void Read_Data(void)
{
	s32 tempfac;
	tempfac=STMFLASH_ReadHalfWord(SAVE_ADDR_BASE+14);	 //本子程序来源于正点原子的stmflash.h
}
