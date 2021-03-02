#include "save_flash.h"


void Save_Data(u8 *Data,u8 len)
{
	s32 temp;
	u8 i;
	FLASH_Unlock();
	FLASH_ErasePage(SAVE_ADDR_BASE);//������оƬFLASHǰ���Ƚ���ҳɾ��
	for(i=0;i<=len;i=i+2)
	{
		temp = Data[i];
		FLASH_ProgramHalfWord(SAVE_ADDR_BASE+i, temp);
	}
	FLASH_Lock();//����
}

void Read_Data(void)
{
	s32 tempfac;
	tempfac=STMFLASH_ReadHalfWord(SAVE_ADDR_BASE+14);	 //���ӳ�����Դ������ԭ�ӵ�stmflash.h
}
