#include "system.h"
#include "codetable.h"
#include "lcd1602.h"
#include "key.h"
#include "cc2500B.h"
#include "string.h"

//zwg 
u8 easyMode_openFlag=0;
u8 easyMode_closeFlag=0;

#define CPU_ID_ADDR     ((u32)0x08007BE0)
#define DES_ID_ADDR     ((u32)0x08007C00)

#define USART1_TDR_ADDR ((u32)0x40013828)
#define USART1_RDR_ADDR ((u32)0x40013824)

#define 	uchar   unsigned char
#define 	FALSE  	0	
#define 	TRUE   	1

u8 HW_version[4] = {0x00,0x00,0x04,0x00};
u8 SW_version[4] = {0x00,0x00,0x05,0x01};
u8 KEY_number[4] = {0x08,0x00,0x00,0x08};                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     

//u8 Password_Save[6]={1,2,4,2,1,3};
//u8 Password_Input[6];

u8 passwordReset[10]={0};

//const 
const u8 Password_Save[10]={1,2,3,4,1,2};
u8 Password_Save1[10];

u8 Password_Read[10];
u8 Password_Input[10];


u8 DES_ID[9] = {0x00};

struct
{
	uchar KeyChar;
	uchar OldChar;
	uchar tmp;
	uchar Cnt;
	uchar Scan;	
}Key;


struct
{
	u8	a;
	u8  b;
	u8  c;
	u8	time;
	u8	Save_flag;
	u8	Pass_flag;
}Password;

struct
{
	u8 Sendcount;
	u8 RX_flag;
 	u8 TX_flag;
	u8 bKeyScanFlag;
  u8 To50Ms;
	u8 key_keepdown;
}CC2500;

u8 FASH = 0;
u32 tmpFash = 0;
u8 tmpFashFlag = 0;

//RF ��������
struct
{
    uchar Chn;
    uchar LockChnFlag; //����Ƶ��
    uchar WorkFlag;
    uchar SendSn;
    uchar SendValue;  //==0xffʱ,��ʾ������
    uchar SendAgain;	//�ظ����ͼ�����
    u16  SendCnt;//���ʹ���
}Rf;

struct
{
	u8 ChipId[3];
	u8 ChipId_2[3];
	u16 PowerSaveDelay;		//zwg 2019.03.02
	u32 MCU_ID;
	u32 MCU_ID_2;
} Me;

//zwg 2019.02.27
u8 	Pair_time;
u8  Pair_flag=0;
u8 	Clean_time;
u8  Clean_flag=3;

u8 	Pair_time;
u8 	Over_Pair_time;
u16 Tcount;

/********************************************************************
**		��������CC2500 PA���Ƴ�ʼ��
**		
**		����ֵ����
*******************************************************************/
void CC2500_PA_Init(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
			
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		
			GPIO_Init(GPIOB, &GPIO_InitStructure);
	
			RX_OFF;		//����PA�ر�
			TX_OFF;		//����PA�ر�
			PWR_ON;		//�ϵ籣�ֹ���,lcd����
}

/********************************************************************
**		����������ʱ���ж�14����
**		
**		����ֵ����
*******************************************************************/
void TIM14_Configuration(void) //10ms
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		NVIC_InitTypeDef NVIC_InitStruct;

		TIM_DeInit(TIM14);//��ʼ��
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);//��ʱ��
		
		TIM_TimeBaseStructure.TIM_Period = 10 * 1000- 1; //10ms  //�޸� 900
		TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1;	//1�μ���1us,����10*1000����10ms
		TIM_TimeBaseStructure.TIM_ClockDivision = 1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM14, &TIM_TimeBaseStructure);
		TIM_ITConfig(TIM14,TIM_IT_Update,ENABLE);
		
		NVIC_InitStruct.NVIC_IRQChannel = TIM14_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelPriority = 1;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStruct);
		
		TIM_Cmd(TIM14, ENABLE);
}

/******************************************************************
**		����������ʱ��14�жϺ���
**		
**		����ֵ����
*******************************************************************/
unsigned char timer50Ms=0;
void TIM14_IRQHandler(void) //10ms
{    
		TIM_ClearITPendingBit(TIM14 , TIM_FLAG_Update); 		
		CC2500.bKeyScanFlag=TRUE;
		Rf.SendAgain++;
		CC2500.key_keepdown++;
		if(CC2500.key_keepdown>30)	//300ms	//if(CC2500.key_keepdown>60)	//zwg 2019.02.27
		{
				CC2500.key_keepdown=0;
				FASH++;
				if(FASH>=4)
					FASH=0;
				tmpFash++;
				if(tmpFashFlag==1){
					tmpFashFlag=2;
					tmpFash=0;
				}
		}
		
		Tcount++;	
		if(Tcount>15)	//150ms
		{
				Me.PowerSaveDelay++;
				Tcount=0;
				//CC2500.Sendcount = 1;//500ms����һ��
		}
		
		CC2500.To50Ms++;
		timer50Ms++;
		if(timer50Ms>80){	//if(timer50Ms>30){
			timer50Ms=0;
			Clean_time++;
		}
		if(CC2500.To50Ms>100)	//1s  //if(CC2500.To50Ms>200)	//zwg 2019.02.27
		{
				CC2500.To50Ms=0;
				Pair_time++;
				//Clean_time++;
				Over_Pair_time++;
		}	   
}
/********************************************************************
**		����������ʱ���ж�3����
**		
**		����ֵ����
*******************************************************************/

void TIM3_Configuration(void)
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		NVIC_InitTypeDef NVIC_InitStructure;

		TIM_DeInit(TIM3);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
		TIM_TimeBaseStructure.TIM_Period = 10 * 5000 - 1; //50ms
		TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1;	//1US
		TIM_TimeBaseStructure.TIM_ClockDivision = 1;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
		
		NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPriority = 2;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		//TIM_Cmd(TIM3, ENABLE);	û�п�����ʱ��3,��һ�λ�����ж�
}

/********************************************************************
**		����������ʱ��3�жϺ���
**		
**		����ֵ����
*******************************************************************/

void TIM3_IRQHandler(void) //50ms
{
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
		TIM_Cmd(TIM3, DISABLE);
}

/********************************************************************
**		�����������ڳ�ʼ������
**		
**		����ֵ����
*******************************************************************/
void USART1_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

	//PA9 ---> USART1_TX
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
	//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	//PA10 ---> USART1_RX
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	//GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_Level_3;	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  //GPIO_InitStruct.GPIO_OType = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
		
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStruct);
	USART_Cmd(USART1, ENABLE);
	
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//ʹ�ܻ���ʧ��ָ����USART�ж� �����ж�
	USART_ClearFlag(USART1,USART_FLAG_TC);//���USARTx�Ĵ������־λ
}

/********************************************************************
**		������������ͨ���ж�
**		
**		����ֵ����
*******************************************************************/
void USART1_IRQHandler(void)	//����1�жϺ���
{
	static u8 k;
	USART_ClearFlag(USART1,USART_FLAG_TC);
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=Bit_RESET)//���ָ����USART�жϷ������
	{
		k=USART_ReceiveData(USART1);
		//k++;	
		USART_SendData(USART1,k);//ͨ������USARTx���͵�������
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==Bit_RESET);	
	}
}

/********************************************************************
**		����������ȡCPU ID
**		
**		����ֵ����
*******************************************************************/
void Read_STM32ID(void)
{
	/*
    u8 i;  
    //STM32 CPUid 
    Me.ChipId[0] = *(u8 *)0x40015800;
    Me.ChipId[1] = *(u8 *)0x40015801;
    Me.ChipId[2] = *(u8 *)0x40015802;
    
    printf("\r\nCPU_ID: ");	//0x006440 Device identifier
	
    for(i = 0; i < 3; i++)
    printf("%02X ",Me.ChipId[i]);
	*/
	
	Me.MCU_ID = *(u32 *)0x1ffff7ac;
	Me.MCU_ID_2 = *(u32 *)0x1ffff7b0;
	
	Me.ChipId[0] = Me.MCU_ID/10000;
    Me.ChipId[1] = Me.MCU_ID%10000/100;
    Me.ChipId[2] = Me.MCU_ID%10000%100;
	
	Me.ChipId_2[0] = ~Me.ChipId[0];
    Me.ChipId_2[1] = ~Me.ChipId[1];
    Me.ChipId_2[2] = ~Me.ChipId[2];
	
//    Me.ChipId[0] = Me.MCU_ID/10000;
//    Me.ChipId[1] = Me.MCU_ID%10000/100;
//    Me.ChipId[2] = Me.MCU_ID%10000%100;
//	
//	Me.ChipId_2[0] = Me.MCU_ID_2/10000;
//    Me.ChipId_2[1] = Me.MCU_ID_2%10000/100;
//    Me.ChipId_2[2] = Me.MCU_ID_2%10000%100;
}

/********************************************************************
**		������������У��
**		
**		����ֵ����
*******************************************************************/
u8 Rf_CheckData( u8 *ptr )  
{
		if(ptr[0]!=0x06||ptr[5]!=0)
			return(FALSE);

		if(((ptr[1]+ptr[2]+ptr[3]+ptr[4]+ptr[5])&0xff)!=ptr[6])
			return(FALSE);

		if(ptr[5]!=0)
			return(FALSE);

		if(ptr[2]==Me.ChipId[0]&&ptr[3]==Me.ChipId[1]&&ptr[4]==Me.ChipId[2])
			return(TRUE);

		if(ptr[2]==0x22&&ptr[3]==0x33&&ptr[4]==0x44)
			return(TRUE);

		return(FALSE);
}

/********************************************************************
**		�����������Ͷ���ֵ
**		
**		����ֵ����
*******************************************************************/
u8 Rf_SendStudyId(void)
{
		u8 tmp[20];
		tmp[0]=0x06;
		tmp[1]=Rf.SendSn;
		tmp[2]=Me.ChipId[0];
		tmp[3]=Me.ChipId[1];
		tmp[4]=Me.ChipId[2];
		tmp[5]=0x14;
		tmp[6]=tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
		tmp[7]=0x06;
		tmp[8]=0x00;
		tmp[9]=0x00;
		tmp[10]=0x00;
		tmp[11]=0x00;
		tmp[12]=0x00;
		tmp[13]=0x00;
	//WdtClear();
	//while(CC2500.bKeyScanFlag==FALSE)
	//{
			Rf_SendMode(Rf.Chn);
   		Rf.SendSn++;
   		Rf_SendData(tmp,13);
			
   		if(Rf.Chn==0)
				Rf.Chn=0x7f;
    	else if(Rf.Chn==0x7f)
				Rf.Chn=0xff;	
    	else
				Rf.Chn=0;
			
	//}
	 
		return  1;
}

/********************************************************************
**		�����������Ͷ���ֵ
**		
**		����ֵ����
*******************************************************************/
u8 Rf_SendStudyId_2(void)
{
  u8 tmp[20];
	tmp[0]=0x06;
	tmp[1]=Rf.SendSn;
	tmp[2]=Me.ChipId_2[0];
	tmp[3]=Me.ChipId_2[1];
	tmp[4]=Me.ChipId_2[2];
  tmp[5]=0x14;
  tmp[6]=tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
  tmp[7]=0x06;
  tmp[8]=0x00;
  tmp[9]=0x00;
  tmp[10]=0x00;
  tmp[11]=0x00;
  tmp[12]=0x00;
  tmp[13]=0x00;
	//WdtClear();
	//while(CC2500.bKeyScanFlag==FALSE)
		//{
			Rf_SendMode(Rf.Chn);
   		Rf.SendSn++;
   		Rf_SendData(tmp,13);
			
   		if(Rf.Chn==0)
			Rf.Chn=0x7f;
			
    	else if(Rf.Chn==0x7f)
    	Rf.Chn=0xff;	
			
    	else
    	Rf.Chn=0;
			
		//}
	 
		return  1;
}

/********************************************************************
**		�����������Ͷ���ֵ
**		
**		����ֵ����
*******************************************************************/
void Rf_StudyWithControl(void)
{
	while(1)
    {
    	//WdtClear();

    	if(CC2500.bKeyScanFlag)
    	{
    		CC2500.bKeyScanFlag=FALSE;

	    		return;
//    		Rf_SendStudyId();
    	}
    }
}

/********************************************************************
**		�����������Ͷ���ֵ
**		
**		����ֵ����
*******************************************************************/
uchar Rf_SendKeyValue(uchar KEYnumber,uchar mode)
{
		uchar tmp[20];
	
		tmp[0]	=	0x06;
		tmp[1]	=	Rf.SendSn;	//�����
		tmp[2]	=	Me.ChipId[0];
		tmp[3]	=	Me.ChipId[1];
		tmp[4]	=	Me.ChipId[2];
    tmp[5]	=	KEYnumber;
    tmp[6]	=	tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
    tmp[7]	=	0x06;
    tmp[8]	=	0x00;
    tmp[9]	=	0x00;
    tmp[10]	=	mode;	//0x00; zwg 0306
    tmp[11]	=	0x00;
    tmp[12]	=	0x00;
    tmp[13]	=	0x00;
	
//	  if(Rf.Chn==0)
//			Rf.Chn=0x7f;
//			
//    else if(Rf.Chn==0x7f)
//    	Rf.Chn=0xff;	
//			
//    else
//    	Rf.Chn=0;

	Rf.Chn=0xbf;
	
   	Rf_SendMode(Rf.Chn);		
   	Rf.SendSn++;	//ÿ����һ������,����ֵ��ͬ
   	Rf_SendData(tmp,13);	//�����������Ժ�,�л�Ϊ����ģʽ,���տ��ƺд��������

	return(FALSE);
}

/********************************************************************
**		�����������Ͷ���ֵ
**		
**		����ֵ����
*******************************************************************/
uchar Rf_SendKeyValue_2(uchar KEYnumber,uchar mode)
{
	uchar tmp[20];

	tmp[0]	=	0x06;
	tmp[1]	=	Rf.SendSn;
	tmp[2]	=	Me.ChipId_2[0];
	tmp[3]	=	Me.ChipId_2[1];
	tmp[4]	=	Me.ChipId_2[2];
	
    tmp[5]	=	KEYnumber;
    tmp[6]	=	tmp[1]+tmp[2]+tmp[3]+tmp[4]+tmp[5];
    tmp[7]	=	0x06;
    tmp[8]	=	0x00;
    tmp[9]	=	0x00;
    tmp[10]	=	mode;//0x00; zwg 0306
    tmp[11]	=	0x00;
    tmp[12]	=	0x00;
    tmp[13]	=	0x00;
	
	  if(Rf.Chn==0)
			Rf.Chn=0x7f;
			
    else if(Rf.Chn==0x7f)
    	Rf.Chn=0xff;	
			
    else
    	Rf.Chn=0;
	
   	Rf_SendMode(Rf.Chn);		
   	Rf.SendSn++;
   	Rf_SendData(tmp,13);

	return(FALSE);
}

//niejie
extern u16 keyflag1,keyflag2;
extern u8	KEYnumber;

extern void LCD_write_string(unsigned char x,unsigned char y,unsigned char *s );
//u8*  WritePasswordToFLASE(void)
//{
//	  u32 i;
//	   u32 addr;
//	   u8* p=NULL;
//    
//	FLASH_Unlock();

//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
//	
//	addr = DES_ID_ADDR;
//	
//	FLASH_ErasePage(addr);
//	
//	for(i = 0; i < 6; i++)
//	{
//			FLASH_ProgramWord(addr, Password_Save[i]);       
//			addr += 4;
//		  *p++=Password_Save[i];
//	}

//	FLASH_Lock();
//	 return p;
//}

// u8*   ReadFLASEToPassWord()
//{
//     u32 i=0, addr;
//     u32 iNbrToRead;
//	   u8* buf=NULL;
//	   
//	    addr=DES_ID_ADDR;
//	  
//	   while(i<iNbrToRead)
//		 {
//		    *(buf+i)= *(u8*)addr++;
//			   i++;		 
//		 }
//     return buf;
//}

uchar i=0;

void  WritePasswordToFLASE1(u32 addr,u8 *p,u16 len)
{
	  u32 halfWord;
	  len=len/2;
	  FLASH_Unlock();
	  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR|FLASH_FLAG_BSY);
	  FLASH_ErasePage(addr);		//addr = 0x08009000,д����֮ǰ���Ȳ���ҳ��ַ
	  while(len--)
		{
					halfWord=*(p++);
			    halfWord |=*(p++)<<8;
          FLASH_ProgramHalfWord(addr,halfWord);	//һ��д��2���ֽ� word��ʾ4byte
          addr += 2;
		}		
    FLASH_Lock();
}


void   ReadFLASEToPassWord2(u32 addr,u8 *p,u16 len)
{
			while(len--)
			{
			   *(p++)=*((u8 *)addr++);
			}
}
 
/********************************************************************
**		���������Զ�����ģʽ
**		
**		����ֵ����
*******************************************************************/
void PWR_DOWNmode(void)	//zwg 0306
{
		if(Me.PowerSaveDelay>=6000)	//num1,,,num4������Ч������������Ч  200  zwg 0305  400000
		{
				Me.PowerSaveDelay=0;
				PWR_OFF;	//�൱��mcu�ϵ�
				Delay_ms(1000);
		}	
}	

/********************************************************************
**		���������Զ��ص�
**		
**		����ֵ����
********************************************************************/
//void PWR_DOWNmode1(void)
//{
//	if(Me.PowerSaveDelay>=60)
//	{
//			Me.PowerSaveDelay=0;
//			PWR_OFF;	
//			Delay_ms(200);
//	}	
//}	

/********************************************************************
**		������������ɨ�账��
**		
**		����ֵ����
*******************************************************************/
void lcd1602key_Handle(void)   //??????
{			
			u8 i;
	
			PWR_ON;	//��������
			if((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0))	//num1,,,num4������Ч
					Me.PowerSaveDelay=0;
			switch(lcd1602key_Scan())
			{
					case KEY_ON:			
						TX_OFF;		//ֻ��key_en��Ч																							
						break;
				
					case 0:			  		
						TX_OFF;		//��������																							
						break;
					
					
					case OPEN_LED:
						tmpFashFlag=0;
						TX_EN;																									
						while(1)
						{																			
							Me.PowerSaveDelay=0;
							LCD_write_string(0,1,"Status: LedON...");		
							if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x00); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_ON,0x00); 	//˫��
									Delay_ms(10);
								}
							}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x01); 		//˫��
								}
							}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue_2(LED_ON,0x02); 	//����
								}
							}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x10); 		//���� //0x10
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_ON,0x10); 	//˫��	//0x10
									Delay_ms(10);
								}
							}
							Me.PowerSaveDelay=0;
							break;
						}
						if(tmpFashFlag==0){
							//tmpFash=0;
							tmpFashFlag=1;
						}
						break; 	
					
					case CLOSE_LED:		
						tmpFashFlag=0;
						TX_EN;																									
						while(1)
						{																			
							    Me.PowerSaveDelay=0;
							    LCD_write_string(0,1,"Status: LedOFF...");	
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x00); 		//����
										Delay_ms(10);
										Rf_SendKeyValue_2(LED_OFF,0x00); 	//˫��
										Delay_ms(10);
									}
								}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x01); 		//˫��
									}
								}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue_2(LED_OFF,0x02); 	//����
									}
								}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x10); 		//���� //0x10
										Delay_ms(10);
										Rf_SendKeyValue_2(LED_OFF,0x10); 	//˫��	//0x10
										Delay_ms(10);
									}
								}
								Me.PowerSaveDelay=0;
								break;
						}
						if(tmpFashFlag==0){
							//tmpFash=0;
							tmpFashFlag=1;
						}
						break;
					
					case LED_Red:		
						tmpFashFlag=0;
						TX_EN;																									
						while( KEY_EN==1 && !KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )
						{																			
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(LED_Red,0x00); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Red,0x00); 	//˫��
									Delay_ms(10);
								}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Red,0x01); 		//˫��
								}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(LED_Red,0x02); 	//����
								}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Red,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Red,0x10); 	//˫��
									Delay_ms(10);
								}
								if(tmpFashFlag==0){
									//tmpFash=0;
									tmpFashFlag=1;
								}
								switch(tmpFash)
								{		
										case 0:	 LCD_write_string(0,1," Status: LedW.  ");   break;																
										case 1:  LCD_write_string(0,1," Status: LedW.. ");   break;
										case 2:  LCD_write_string(0,1," Status: LedW...");   break;																
										case 3:	 LCD_write_string(0,1," Status: LedW ON");   break;
										/*
										case 0:	 LCD_write_string(0,1," Status: LedW ON ");   break;																
										case 1:  LCD_write_string(0,1," Status:         ");   break;
										case 2:  LCD_write_string(0,1," Status: LedW ON ");   break;																
										case 3:	 LCD_write_string(0,1," Status:         ");   break;
										*/
								}
								Me.PowerSaveDelay=0;
						}																		 													
						break;
															
					case LED_Green:	
						tmpFashFlag=0;
						TX_EN;																									
						while( KEY_EN==1 && KEY_NUM1 && !KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )
						{																			

								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(LED_Green,0x00); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Green,0x00); 	//˫��
									Delay_ms(10);
								}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Green,0x01); 		//˫��
								}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(LED_Green,0x02); 	//����
								}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Green,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Green,0x10); 	//˫��
									Delay_ms(10);
								}
								if(tmpFashFlag==0){
									//tmpFash=0;
									tmpFashFlag=1;
								}
								switch(tmpFash)
								{			
										case 0:	 LCD_write_string(0,1," Status: LedG.  ");   break;																
										case 1:  LCD_write_string(0,1," Status: LedG.. ");   break;
										case 2:  LCD_write_string(0,1," Status: LedG...");   break;																
										case 3:	 LCD_write_string(0,1," Status: LedG ON");   break;
									
//										case 0:	 LCD_write_string(0,1," Status: LedG ON ");   break;																
//										case 1:  LCD_write_string(0,1," Status:         ");   break;
//										case 2:  LCD_write_string(0,1," Status: LedG ON ");   break;																
//										case 3:	 LCD_write_string(0,1," Status:         ");   break;
								}
								Me.PowerSaveDelay=0;
						}																		 													
						break;														
															
					case LED_Blue:		
						tmpFashFlag=0;
						TX_EN;																									
						while( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && !KEY_NUM3 && KEY_NUM4 )
						{																			
								 
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(LED_Blue,0x00); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Blue,0x00); 	//˫��
									Delay_ms(10);
								}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Blue,0x01); 		//˫��
								}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(LED_Blue,0x02); 	//����
								}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Blue,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Blue,0x10); 	//˫��
									Delay_ms(10);
								}
								if(tmpFashFlag==0){
									//tmpFash=0;
									tmpFashFlag=1;
								}
								switch(tmpFash)
								{				
										case 0:	 LCD_write_string(0,1," Status: LedB.  ");   break;																
										case 1:  LCD_write_string(0,1," Status: LedB.. ");   break;
										case 2:  LCD_write_string(0,1," Status: LedB...");   break;																
										case 3:	 LCD_write_string(0,1," Status: LedB ON");   break;
									
//										case 0:	 LCD_write_string(0,1," Status: LedB ON ");   break;																
//										case 1:  LCD_write_string(0,1," Status:         ");   break;
//										case 2:  LCD_write_string(0,1," Status: LedB ON ");   break;																
//										case 3:	 LCD_write_string(0,1," Status:         ");   break;
								}
								Me.PowerSaveDelay=0;
						}																		 													
						break; 														
															
					case LED_Off:		  
						tmpFashFlag=0;
						TX_EN;																									
						while( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && !KEY_NUM4 )
						{
								
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(LED_Off,0x00); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Off,0x00); 	//˫��
									Delay_ms(10);
								}else if(Clean_flag==2){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Off,0x01); 		//˫��
								}else if(Clean_flag==1){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(LED_Off,0x02); 	//����
								}else if(Clean_flag==0){	//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(LED_Off,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Off,0x10); 	//˫��
									Delay_ms(10);
								}
								if(tmpFashFlag==0){
									//tmpFash=0;
									tmpFashFlag=1;
								}
								switch(tmpFash)
								{					
										case 0:	 LCD_write_string(0,1," Status: Led.   ");   break;																
										case 1:  LCD_write_string(0,1," Status: Led..  ");   break;
										case 2:  LCD_write_string(0,1," Status: Led... ");   break;																
										case 3:	 LCD_write_string(0,1," Status: Led OFF");   break;
									
//										case 0:	 LCD_write_string(0,1," Status: Led OFF ");   break;																
//										case 1:  LCD_write_string(0,1," Status:         ");   break;
//										case 2:  LCD_write_string(0,1," Status: Led OFF ");   break;																
//										case 3:	 LCD_write_string(0,1," Status:         ");   break;
								}
								Me.PowerSaveDelay=0;
						}																		 													
						break;
													
					case KEY_in:			
						TX_EN;
						while((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==1)) 
						{																			
								Me.PowerSaveDelay=0;
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(KEY_out,0x00); 		//����
//									Delay_ms(10);
//									Rf_SendKeyValue_2(KEY_out,0x00); 	//˫��
//									Delay_ms(10);
//									switch(FASH)
//									{																			
//										case 0:	 LCD_write_string(0,1," Stand  Opening ");   break;																
//										case 1:  LCD_write_string(0,1,"                ");   break;
//										case 2:  LCD_write_string(0,1," Stand  Opening ");   break;																
//										case 3:	 LCD_write_string(0,1,"                ");   break;
//									}
								}else if(Clean_flag==2){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(KEY_out,0x01); 		//����
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Odd  Opening  ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Odd  Opening  ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==1){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(KEY_out,0x02); 		//˫��
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Even  Opening ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Even  Opening ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==0){			//ͬʱ���Ƶ�/˫���ӣ�ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(KEY_out,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(KEY_out,0x10); 	//˫��
									Delay_ms(10);
									easyMode_openFlag=1;	//zwg
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Easy  Opening ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Easy  Opening ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}
								Me.PowerSaveDelay=0;
						}
						LCD_write_string(0,1," Status: Standby");
						for(i=0;i<100;i++)	//���������Ժ󣬷���ֹͣ�źţ�ֹͣ���
						{ALL_STOP;Delay_ms(20);}
						Me.PowerSaveDelay=0;
						break;
																
					case KEY_out:			
						TX_EN;
						while((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))
						{																		
								Me.PowerSaveDelay=0;
								if(Clean_flag==3){			//ͬʱ���Ƶ�/˫����,���������Ƿ��в����ֿ���HC225
									Rf_SendKeyValue(KEY_in,0x00); 		//����
//									Delay_ms(10);
//									Rf_SendKeyValue_2(KEY_in,0x00); 		//˫��
//									Delay_ms(10);
//									switch(FASH)
//									{																
//										case 0:	 LCD_write_string(0,1," Stand  Closing ");	  break;															
//										case 1:  LCD_write_string(0,1,"                ");   break;
//										case 2:  LCD_write_string(0,1," Stand  Closing ");	  break;															
//										case 3:	 LCD_write_string(0,1,"                ");   break;
//									}
								}else if(Clean_flag==2){	//���Ƶ���,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(KEY_in,0x01); 		//����
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Odd  Closing  ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Odd  Closing  ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==1){	//����˫��,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue_2(KEY_in,0x02); 		//˫��
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Even  Closing ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Even  Closing ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==0){			//ͬʱ���Ƶ�/˫����,ֻ�ܿ�����Щ���ֿ���HC225������������
									Rf_SendKeyValue(KEY_in,0x10); 		//����
									Delay_ms(10);
									Rf_SendKeyValue_2(KEY_in,0x10); 		//˫��
									Delay_ms(10);
									easyMode_closeFlag=1;	//zwg
									switch(FASH)
									{																
										case 0:	 LCD_write_string(0,1,"  Easy  Closing ");	  break;															
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Easy  Closing ");	  break;															
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}
								Me.PowerSaveDelay=0;
						}
						LCD_write_string(0,1," Status: Standby");
						for(i=0;i<100;i++)
						{ALL_STOP; Delay_ms(20);}	
						Me.PowerSaveDelay=0;
						break;
				
					case KEY_study:		
						TX_EN;
						Pair_time=0;
						while ((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
						{     																		
								u8 Enter_succee=0;
								Over_Pair_time = 0;
								switch(Pair_time)
								{
										case 1: LCD_write_string(0,1," Enter  Pair 3  "); break;
										case 2: LCD_write_string(0,1," Enter  Pair 2  "); break;
										case 3: LCD_write_string(0,1," Enter  Pair 1  "); break;	
										case 4: LCD_write_string(0,1," Enter  Pair 0  "); Enter_succee=1;break;	//����һֱ����������ϵͳ���ᷢ�������
								}
										
								while(Enter_succee)
								{ 
										while((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
										{
												Pair_time=0;																			
												LCD_write_string(1,1,"Enter  Pair OK  ");
										}
												
										while((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0));			//�ȴ�����,�������еİ������ͷ�																																		
										
										Me.PowerSaveDelay=0;	//���¿�ʼ��ʱ
										if(Pair_flag==0)		//Ĭ�������Pair_flag=0����һ�ν��뵥�����ģʽ
										{
											switch(FASH)
											{
												case 0: LCD_write_string(0,1," Odd Pairing.   "); break;
												case 1: LCD_write_string(0,1," Odd Pairing..  "); break;
												case 2: LCD_write_string(0,1," Odd Pairing... "); break;
												case 3: LCD_write_string(0,1," Odd Pairing...."); break;
											}	
										}
										else if(Pair_flag==1)	//�ڶ��ν���˫�����ģʽ
										{
											switch(FASH)
											{
														
												case 0: LCD_write_string(0,1,"Even Pairing.   "); break;
												case 1: LCD_write_string(0,1,"Even Pairing..  "); break;
												case 2: LCD_write_string(0,1,"Even Pairing... "); break;
												case 3: LCD_write_string(0,1,"Even Pairing...."); break;
											}	
										}
										if(Pair_flag==0){	//����ģʽ��ԣ����Ͷ�Ӧ�������(��ǰmcu��id)
											Rf_SendStudyId();
										}else{				//˫��ģʽ��ԣ����Ͷ�Ӧ�������(��ǰmcu��id)
											Rf_SendStudyId_2();
										}
												
										if((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0)||(Over_Pair_time>= 200))	//��ʱ�˳����߰���KEY_EN,KEY_OPEN,KEY_CLOSE�˳�
										{
												if((Pair_flag==0)&&(Enter_succee==1))
													Pair_flag=1;
												else if((Pair_flag==1)&&(Enter_succee==1))
													Pair_flag=0;
												Enter_succee=0;
												Me.PowerSaveDelay=0;
												break;
										}																			
								}
						}
						LCD_write_string(0,1," Status: Standby");
						for(i=0;i<30;i++)
						{ALL_STOP; Delay_ms(10);}
						Me.PowerSaveDelay=0;							
						break;

					case KEY_clean_enter:		  
						TX_EN;
						Clean_time=0;
//						if(Clean_flag==0){	//easymode�£����û�а��°����������
//							if((easyMode_openFlag==0)&&(easyMode_closeFlag==0)){
//								Me.PowerSaveDelay=0;
//								break;
//							}
//						}
						while ((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
						{																		
							u8 Enter_succee=0;
							Me.PowerSaveDelay=0;
							switch(Clean_time)
							{
								case 1: LCD_write_string(0,1,"Enter ClrMode 3 "); break;
							    case 2: LCD_write_string(0,1,"Enter ClrMode 2 "); break;
								case 3: LCD_write_string(0,1,"Enter ClrMode 1 "); break;	
								//case 4: LCD_write_string(0,1,"Enter ClrMode OK"); Enter_succee=1; break;
								case 4:		Enter_succee=1;break;	//case 4: LCD_write_string(0,1,"Enter ClrMode 0 "); Enter_succee=1;break;
							}
							//Over_Clean_time = 0;  //not use
							while(Enter_succee)
							{ 
								/*
								while((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
								{
									Clean_time=0;																			
									LCD_write_string(0,1,"Enter ClrMode OK");
								}*/
								
								Me.PowerSaveDelay=0;
								//while((KEY_OPEN==0)||(KEY_CLOSE==0));
								if(Clean_flag==3)
								{
									LCD_write_string(0,1," Odd Clean Mode ");
								}
								else if(Clean_flag==2)
								{
									LCD_write_string(0,1," Even Clean Mode");
								}
								else if(Clean_flag==1)
								{
									LCD_write_string(0,1," Easy Clean Mode");
								}
								else if(Clean_flag==0)
								{
									LCD_write_string(0,1,"Stand Clean Mode");
								}
								Delay_ms(100);
					
								if(Clean_flag==3){
									for(i=0;i<50;i++){
										Rf_SendKeyValue(KEY_clean_enter,0xee);
										Delay_ms(10);
										Rf_SendKeyValue_2(KEY_clean_enter,0xee);
										Delay_ms(10);
									}
								}
								else if(Clean_flag==2){
									for(i=0;i<50;i++){
										Rf_SendKeyValue(KEY_clean_enter,0xee);
										Delay_ms(10);
										Rf_SendKeyValue_2(KEY_clean_enter,0xee);
										Delay_ms(10);
									}
								}
								else if(Clean_flag==1){		//�����ģʽ����Ҫ�ж��Ƿ���hc225����&��������
									for(i=0;i<50;i++){
										Rf_SendKeyValue(KEY_easy_clean_enter,0xee);		//zwg 0305
										Delay_ms(10);
										Rf_SendKeyValue_2(KEY_easy_clean_enter,0xee);	//zwg 0305
										Delay_ms(10);
									}
								}
								else if(Clean_flag==0){
									if((easyMode_openFlag==0)&&(easyMode_closeFlag==0)){	//easymode�£����û�а��°����������,�����͸�λָ��
										/*
										for(i=0;i<50;i++){
											;
											Delay_ms(10);
											;
											Delay_ms(10);
										}*/
										;
									}else{
										easyMode_openFlag=0;
										easyMode_closeFlag=0;
										for(i=0;i<50;i++){
											Rf_SendKeyValue(KEY_clean_exit,0x00); 
											Delay_ms(10);
											Rf_SendKeyValue_2(KEY_clean_exit,0x00);
											Delay_ms(10);
										}
									}
								}
								LCD_write_string(0,1," Status: Standby");
								Me.PowerSaveDelay=0;
								while(((KEY_OPEN==0)||(KEY_CLOSE==0))||(KEY_EN==1));	
								Me.PowerSaveDelay=0;
								//if((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0)||(Over_Pair_time>= 200))
								{
									if((Clean_flag==3)&&(Enter_succee==1))
										Clean_flag=2;
									else if((Clean_flag==2)&&(Enter_succee==1))
										Clean_flag=1;
									else if((Clean_flag==1)&&(Enter_succee==1))
										Clean_flag=0;
									else if((Clean_flag==0)&&(Enter_succee==1))
										Clean_flag=3;
									Enter_succee=0;
									break;
								}
							}
							Me.PowerSaveDelay=0;	
						}
						for(i=0;i<30;i++)
						{ALL_STOP; Delay_ms(10);}	
						Me.PowerSaveDelay=0;
						break;
															
				default:					  
					TX_OFF;
					Over_Pair_time = 0;
					break;
			}	
}


/*******************************************************************
**		��������д��EEPROM
**		
**		����ֵ����
*******************************************************************/
//void Password_write(void)
//{    
//    u32 i, addr;
//    
//	FLASH_Unlock();

//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
//	
//	addr = DES_ID_ADDR;
//	
//	FLASH_ErasePage(addr);
//	
//	for(i = 0; i < 6; i++)
//	{
//			FLASH_ProgramWord(addr, Password_Save[i]);       
//			addr += 4;
//	}

//	FLASH_Lock();
//}

/********************************************************************
**		��������Flash��ʼ������
**		
**		����ֵ����
********************************************************************/
//void FLASH_Init(u32 addr,u8 *p)
//{
//      u32  count=0;
//			FLASH_Unlock();
//      FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
//      FLASH_ErasePage(addr);
//	    while(count<6)
//				{
//					 printf("444444444");
//				   FLASH_ProgramHalfWord(addr,*p);
//					 count++;
//				}
//			FLASH_Lock();
//}

void Password_Enter(void);
u16 faultcount =0;
FLASH_Status  flashstatus;

/********************************************************************
**		����������������
**		
**		����ֵ����
********************************************************************/
u16 len=6;
u32 addr=0x08009000;
uchar j=0;

 void 	ERROR_Password(void);
 void Change_Setpassword(void)	//������i��ʼ������ᵼ�µ�һ�������ԭʼ���룬�洢�������е�λ����һ�����������������"��ʾ����"������Ҫ�ٴ��������룬����ok
{
		u8 fisrtEnterThisFunc=0;
		u8 passwordtmp1[10];
		u8 passwordtmp2[10];
		u8 passwordtmp3[10];
		u8 passwordtmp4[10];
		u8 passwordtmp5[10];
	
cc:
		if(fisrtEnterThisFunc==1)	//��һ�ν���˺���������Ҫˢ����Ļ
		{
			LCD_Clear();	//��ʾ����
			Delay_ms(10);	
			LCD_write_string(1,0,"Init Password");
		}
		fisrtEnterThisFunc=1;
		i=0;	//�������˳�ʼ����䣬������bug
		Password.Pass_flag=1;
		Password.a=1;
		Password.b=1;
		Password.c=1;
		while(1)
		{		//��������ȷ��ԭʼ���룬������������
				while(Password.Pass_flag)
				{
						switch(Password.a)                                                                       //1
						{
										
							case 1:		 	switch(FASH)
													{
															case 0:LCD_write_string(1,1,"Input:       ");break;
															case 1:LCD_write_string(1,1,"Input:_      ");break;				
															case 2:LCD_write_string(1,1,"Input:       ");break;
															case 3:LCD_write_string(1,1,"Input:_      ");break;												
													}	
													if(!KEY_NUMall)	//�͵�ƽ��Ч
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);	//���ּ��
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
													}
													break;
							case 2:			switch(FASH)
													{
															case 0:LCD_write_string(1,1,"Input:*      ");break;
															case 1:LCD_write_string(1,1,"Input:*_     ");break;
															case 2:LCD_write_string(1,1,"Input:*      ");break;
															case 3:LCD_write_string(1,1,"Input:*_     ");break;
													}
													if(!KEY_NUMall)	
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);												
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
													}
													break;											
							case 3:			switch(FASH)
													{
															case 0:LCD_write_string(1,1,"Input:**     ");break;
															case 1:LCD_write_string(1,1,"Input:**_    ");break;
															case 2:LCD_write_string(1,1,"Input:**     ");break;
															case 3:LCD_write_string(1,1,"Input:**_    ");break;
													}
													if(!KEY_NUMall)	
													{
															passwordtmp1[i]=Password_keyScan();
															
															while(!KEY_NUMall);
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
													}
													break;
							case 4:			switch(FASH)
													{
															case 0:LCD_write_string(1,1,"Input:***    ");break;
															case 1:LCD_write_string(1,1,"Input:***_   ");break;
															case 2:LCD_write_string(1,1,"Input:***    ");break;
															case 3:LCD_write_string(1,1,"Input:***_   ");break;
													}
													if(!KEY_NUMall)	
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
														
														
													} 
													break;
							case 5:			switch(FASH)
													{													
															case 0:LCD_write_string(1,1,"Input:****   ");break;
															case 1:LCD_write_string(1,1,"Input:****_  ");break;
															case 2:LCD_write_string(1,1,"Input:****   ");break;
															case 3:LCD_write_string(1,1,"Input:****_  ");break;
													}
													if(!KEY_NUMall)	
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);											
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
													}
													break;
							case 6:			switch(FASH)
													{													
															case 0:LCD_write_string(1,1,"Input:*****  ");break;
															case 1:LCD_write_string(1,1,"Input:*****_ ");break;
															case 2:LCD_write_string(1,1,"Input:*****  ");break;
															case 3:LCD_write_string(1,1,"Input:*****_ ");break;
													}
													if(!KEY_NUMall)	
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);												 
															i=i+1;
															Password.a+=1;
															Me.PowerSaveDelay=0;
															LCD_write_string(1,1,"Input:****** ");	//����������ɣ���ʱ�ȴ�����"start" key ȷ�ϣ�����������Ч
														
															Delay_ms(500);
													}
													break;
												
							case 7:  		if( KEY_EN == 1 )	// ����"start" key ȷ��
													{
															Delay_ms(20);
															if( KEY_EN == 1 )
															{
																	Password.a=0;	//case �����������
																	//�ж�����������뱣��������Ƿ�һ��
																	if(((Password_Save1[0]==passwordtmp1[0])&&(Password_Save1[1]==passwordtmp1[1])
																		&&(Password_Save1[2]==passwordtmp1[2])&&(Password_Save1[3]==passwordtmp1[3])
																		&&(Password_Save1[4]==passwordtmp1[4])&&(Password_Save1[5]==passwordtmp1[5]))
																		||((Password_Read[0]==passwordtmp1[0])&&(Password_Read[1]==passwordtmp1[1])
																		&&(Password_Read[2]==passwordtmp1[2])&&(Password_Read[3]==passwordtmp1[3])
																		&&(Password_Read[4]==passwordtmp1[4])&&(Password_Read[5]==passwordtmp1[5])))
																		{
																				i=0;	//�洢��������������±���������
																				Delay_ms(500);
																				LCD_Clear(); 
																				Delay_ms(2);				
																				aa:	
																				LCD_write_string(1,0," New Password  ");	
																				while(Password.Pass_flag)
																				{	 											
																						switch(Password.b)     //����ṹͬ����
																						{
																								case 1:	switch(FASH)
																												{
																														case 0:LCD_write_string(1,1,"1.Input:       ");break;
																														case 1:LCD_write_string(1,1,"1.Input:_      ");break;				
																														case 2:LCD_write_string(1,1,"1.Input:       ");break;
																														case 3:LCD_write_string(1,1,"1.Input:_      ");break;												
																												}	
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																												
																														while(!KEY_NUMall);
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																												}
																												break;
																								case 2:	switch(FASH)
																												{
																														case 0:LCD_write_string(1,1,"1.Input:*      ");break;
																														case 1:LCD_write_string(1,1,"1.Input:*_     ");break;
																														case 2:LCD_write_string(1,1,"1.Input:*      ");break;
																														case 3:LCD_write_string(1,1,"1.Input:*_     ");break;
																												}
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																													
																														while(!KEY_NUMall);												
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																												}
																												break;
																								case 3:	switch(FASH)
																												{
																														case 0:LCD_write_string(1,1,"1.Input:**     ");break;
																														case 1:LCD_write_string(1,1,"1.Input:**_    ");break;
																														case 2:LCD_write_string(1,1,"1.Input:**     ");break;
																														case 3:LCD_write_string(1,1,"1.Input:**_    ");break;
																												}
																												
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																													
																														while(!KEY_NUMall);
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																	//												printf("%d\r\n",i);
																	//												printf("%d\r\n",Password.a);
																												}
																												break;
																								case 4:	switch(FASH)
																												{													
																														case 0:LCD_write_string(1,1,"1.Input:***    ");break;
																														case 1:LCD_write_string(1,1,"1.Input:***_   ");break;
																														case 2:LCD_write_string(1,1,"1.Input:***    ");break;
																														case 3:LCD_write_string(1,1,"1.Input:***_   ");break;
																												}
																											
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																													
																														while(!KEY_NUMall);
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																												}
																												break;
																								case 5:	switch(FASH)
																												{													
																														case 0:LCD_write_string(1,1,"1.Input:****   ");break;
																														case 1:LCD_write_string(1,1,"1.Input:****_  ");break;
																														case 2:LCD_write_string(1,1,"1.Input:****   ");break;
																														case 3:LCD_write_string(1,1,"1.Input:****_  ");break;
																												}
																											
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																													
																														while(!KEY_NUMall);											
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																												}
																												break;
																							case 6:		switch(FASH)
																												{													
																														case 0:LCD_write_string(1,1,"1.Input:*****  ");break;
																														case 1:LCD_write_string(1,1,"1.Input:*****_ ");break;
																														case 2:LCD_write_string(1,1,"1.Input:*****  ");break;
																														case 3:LCD_write_string(1,1,"1.Input:*****_ ");break;
																												}
																											
																												if(!KEY_NUMall)	
																												{
																														passwordtmp2[j]=Password_keyScan();
																													
																														while(!KEY_NUMall);												 
																														j=j+1;
																														Password.b+=1;
																														Me.PowerSaveDelay=0;
																														LCD_write_string(1,1,"1.Input:****** ");
																													
																														Delay_ms(500);
																												}
																												break;
																							case 7:   if( KEY_EN == 1 )
																												{ 
																														Delay_ms(20);
																														if( KEY_EN == 1)
																														{ 
																																j=0;
																																Password.b=0;	//�����ر�־λ																 
																																strcpy(passwordtmp5,passwordtmp2);	//������������룬���д洢
																																Delay_ms(500);
																																LCD_Clear(); 
																																Delay_ms(2);
																																LCD_write_string(1,0," New Password"); 			
																																while(Password.Pass_flag)		//����ṹͬ����
																																{	
																																		switch(Password.c)                          
																																		{									
																																				case 1:		switch(FASH)
																																									{
																																											case 0:LCD_write_string(1,1,"2.Input:       ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:_      ");break;				
																																											case 2:LCD_write_string(1,1,"2.Input:       ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:_      ");break;												
																																									}	
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j]=Password_keyScan();
																																										
																																											while(!KEY_NUMall);
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																									}
																																									break;
																																				case 2:		switch(FASH)
																																									{
																																											case 0:LCD_write_string(1,1,"2.Input:*      ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:*_     ");break;
																																											case 2:LCD_write_string(1,1,"2.Input:*      ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:*_     ");break;
																																									}
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j] = Password_keyScan();
																																											
																																											while(!KEY_NUMall);												
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																									}
																																									break;											
																																				case 3:		switch(FASH)
																																									{
																																											case 0:LCD_write_string(1,1,"2.Input:**     ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:**_    ");break;
																																											case 2:LCD_write_string(1,1,"2.Input:**     ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:**_    ");break;
																																									}
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j]=Password_keyScan();
																																										
																																											while(!KEY_NUMall);
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																									}
																																									break;
																																				case 4:		switch(FASH)
																																									{													
																																											case 0:LCD_write_string(1,1,"2.Input:***    ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:***_   ");break;
																																											case 2:LCD_write_string(1,1,"2.Input:***    ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:***_   ");break;
																																									}
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j]=Password_keyScan();
																																										
																																											while(!KEY_NUMall);
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																									}
																																									break;
																																				case 5:		switch(FASH)
																																									{													
																																											case 0:LCD_write_string(1,1,"2.Input:****   ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:****_  ");break;
																																											case 2:LCD_write_string(1,1,"2.Input:****   ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:****_  ");break;
																																									}
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j]=Password_keyScan();
																																										
																																											while(!KEY_NUMall);											
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																									}
																																									break;
																																				case 6:		switch(FASH)
																																									{													
																																											case 0:LCD_write_string(1,1,"2.Input:*****  ");break;
																																											case 1:LCD_write_string(1,1,"2.Input:*****_ ");break;
																																											case 2:LCD_write_string(1,1,"2.Input:*****  ");break;
																																											case 3:LCD_write_string(1,1,"2.Input:*****_ ");break;
																																									}
																																									if(!KEY_NUMall)	
																																									{
																																											passwordtmp4[j]=Password_keyScan();
																																										
																																											while(!KEY_NUMall);												 
																																											j=j+1;
																																											Password.c+=1;
																																											Me.PowerSaveDelay=0;
																																											LCD_write_string(1,1,"2.Input:****** ");
																																											
																																											Delay_ms(500);
																																									}
																																									break;
																																				case 7:   if( KEY_EN == 1 )
																																									{							
																																											Delay_ms(20);		
																																											if( KEY_EN == 1 )	
																																											{						
																																													Password.c=0;			//�����־λ
																																													//�ж���������������Ƿ�һ��
																																													if(	(passwordtmp5[0]==passwordtmp4[0])&&(passwordtmp5[1]==passwordtmp4[1])
																																															&&(passwordtmp5[2]==passwordtmp4[2])&&(passwordtmp5[3]==passwordtmp4[3])
																																															&&(passwordtmp5[4]==passwordtmp4[4])&&(passwordtmp5[5]==passwordtmp4[5]))
																																													{	
																																															FASH=0;
																																															faultcount=0;	//������ͬ���������
																																															CC2500.key_keepdown=0;
																																															Password.Pass_flag=0;	//�����־λ���˳���ѭ��
																																															j=0;	//��������±�
																																															Password.b=0;	//�����־��
																																															Password.a=0;
																																															Password.c=0;  
																																															WritePasswordToFLASE1(addr,passwordtmp4,len);		//�������뵽mcuָ��flash��ַ
																																															ReadFLASEToPassWord2( addr,Password_Read, len);	//�������ݽ�����֤
																																															for(i=0;i<6;i++)
																																															{
																																																	printf("%X\r\n",Password_Read[i]);
																																															}
																																															LCD_Clear();
																																															LCD_write_string(1,0,"Modify finnish!");
																																															LCD_write_string(1,1,"New Password Ok");
																																															Delay_ms(2000);
																																													}
																																													else
																																													{																	   
																																															faultcount++;	//������󣬼���
																																															LCD_Clear();
																																															LCD_write_string(1,0,"2nd input error");
																																															LCD_write_string(1,1,"Pls input again");
																																															Delay_ms(2000);
																																															//���³�ʼ������
																																															FASH=0;
																																															CC2500.key_keepdown=0;
																																															Password.Pass_flag=1;
																																															Password.a=1;
																																															Password.b=1;
																																															Password.c=1;
																																															j=0;
																																															ERROR_Password();
																																															goto  aa;	//���������һ������
																																													}																													 
																																											}
																																									}
																																									break;
																																		}											
																															 }			
																														}
																												}
																												break;       
																						}
																				}
																		}
																		else 
																		{							
																				faultcount++;	//����������ԭʼ���벻һ��
																				i=0;			//���³�ʼ�������±�						
																				LCD_Clear();
																				LCD_write_string(1,0,"  Init Error  ");
																				Delay_ms(1500);
																				LCD_Clear();
																				ERROR_Password();
																				goto cc;			//��������ԭʼ����							
																		}
															}
													}
													break;
						} 	
				}	
			
				if(KEY_OPEN==1)	//�͵�ƽ��Ч����������Ժ������˳���ѭ����
						break;   						
		}
		printf("nienienie5");	
}	
/********************************************************************
**		���������ϵ��������
**		
**		����ֵ����
********************************************************************/
void load_password()
{	
			//ÿ����¼������������addr��ַ�洢��password����Ϊ��keil���������У�û������password�Ĵ洢�ռ䣬ÿ�β���sector�����������64kb flash
			
			//�����mcu�ж���������Ϊ0/0xff����˵��flash��δʹ���µ����룬Ĭ�ϲ���ϵͳ��ʼ������Password_Save(1,2,3,4,1,2)
			//Password_Read��ʾ��ǰ�洢��mcu�е�����
			if(((Password_Read[0]==0)&&(Password_Read[1]==0)&&(Password_Read[2]==0)&&(Password_Read[3]==0)
				&&(Password_Read[4]==0)&&(Password_Read[5]==0) )||(Password_Read[0]==0xFF)&&(Password_Read[1]==0xFF)
				&&(Password_Read[2]==0xFF)&&(Password_Read[3]==0xFF)&&(Password_Read[4]==0xFF)&&(Password_Read[5]==0xFF))
			{
					strcpy( Password_Save1, Password_Save );
			}
			else if( strcmp(Password_Read,Password_Save) == 0 )	//����ֵΪ0,��ʾ�������  //�����޸ĺ������,�����޸ĵ�������Password_Save��ͬ
			{ 
					strcpy( Password_Save1, Password_Save );
			}
			else	//�����޸ĺ������,�����޸ĵ�������Password_Save��ͬ
			{
					for(i=0;i<6;i++)
						Password_Save1[i] = Password_Read[i];
			}
}

void Reset_Default_Password(void)
{
	//ͬʱ����num1,num4 key,��λ��ʼ����					
	LCD_Clear();
	Delay_ms(10);
	LCD_write_string(1,0,"Reset Password");
	LCD_write_string(1,1,"Open:Y  Close:N");	//zwg 2019.03.02
	
	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
	while((KEY_NUM1==0)||(KEY_OPEN==0)||(KEY_CLOSE==0)||(KEY_EN==1));
	while(1){
		if(KEY_OPEN==0){
			Me.PowerSaveDelay=0;
			LCD_write_string(1,1," Wait Reset ...");	//zwg 2019.03.02
			Delay_ms(1000);
			LCD_write_string(1,1," Wait Reset .. ");	//zwg 2019.03.02
			Delay_ms(1000);
			LCD_write_string(1,1," Wait Reset .  ");	//zwg 2019.03.02
			Delay_ms(1000);
			WritePasswordToFLASE1(addr,passwordReset,6);
			LCD_write_string(1,1," Wait Reset ok ");	//zwg 2019.03.02
			Delay_ms(1000);
			Me.PowerSaveDelay=0;
			return;
		}else if(KEY_CLOSE==0){
			Me.PowerSaveDelay=0;
			return;
		}
	}
}

/********************************************************************
**		���������ϵ����������ȷ���ܲ���
**		
**		����ֵ����
********************************************************************/
void Password_Enter(void)
{ 	
	
		uchar i=0;
	aa:
		i=0;
		Password.Pass_flag=1;
		Password.a=1;
	
	bb:
		LCD_write_string(1,0,"Password enter ");
	
		while(Password.Pass_flag)
		{			
				PWR_DOWNmode();	//Me.PowerSaveDelay��ʱ��30s�ް���������num1..num4���²���Ч�����Զ��ϵ�mcu
				switch(Password.a)
				{
						case 1:		switch(FASH)	//10msˢ��1��
											{
													case 0:LCD_write_string(1,1,"Input:         ");break;
													case 1:LCD_write_string(1,1,"Input:  _      ");break;				
													case 2:LCD_write_string(1,1,"Input:         ");break;
													case 3:LCD_write_string(1,1,"Input:  _      ");break;												
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();//��ȡ��ֵ
												
													while(!KEY_NUMall);
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;	//���¼�ʱ
											}
											break;
						case 2:		switch(FASH)
											{
													case 0:LCD_write_string(1,1,"Input:  *      ");break;
													case 1:LCD_write_string(1,1,"Input:  *_     ");break;
													case 2:LCD_write_string(1,1,"Input:  *      ");break;
													case 3:LCD_write_string(1,1,"Input:  *_     ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();
												
													while(!KEY_NUMall);												
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;
											}
											break;
						case 3:		switch(FASH)
											{
													case 0:LCD_write_string(1,1,"Input:  **     ");break;
													case 1:LCD_write_string(1,1,"Input:  **_    ");break;
													case 2:LCD_write_string(1,1,"Input:  **     ");break;
													case 3:LCD_write_string(1,1,"Input:  **_    ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();
												
													while(!KEY_NUMall);
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;
											}
											break;
						case 4:		switch(FASH)
											{													
													case 0:LCD_write_string(1,1,"Input:  ***    ");break;
													case 1:LCD_write_string(1,1,"Input:  ***_   ");break;
													case 2:LCD_write_string(1,1,"Input:  ***    ");break;
													case 3:LCD_write_string(1,1,"Input:  ***_   ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();
													
													while(!KEY_NUMall);
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;
											} 
											break;
						case 5:		switch(FASH)
											{													
													case 0:LCD_write_string(1,1,"Input:  ****   ");break;
													case 1:LCD_write_string(1,1,"Input:  ****_  ");break;
													case 2:LCD_write_string(1,1,"Input:  ****   ");break;
													case 3:LCD_write_string(1,1,"Input:  ****_  ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();
												
													while(!KEY_NUMall);											
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;
											}
											break;
						case 6:		switch(FASH)
											{													
													case 0:LCD_write_string(1,1,"Input:  *****  ");break;
													case 1:LCD_write_string(1,1,"Input:  *****_ ");break;
													case 2:LCD_write_string(1,1,"Input:  *****  ");break;
													case 3:LCD_write_string(1,1,"Input:  *****_ ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();
												
													while(!KEY_NUMall);												 
													i=i+1;
													Password.a+=1;
												
													LCD_write_string(1,1,"Input:  ****** ");	//�����������
													Delay_ms(500);
													LCD_write_string(1,1,"Input:         ");	//�����ʾ
													Delay_ms(100);
													Me.PowerSaveDelay=0;
											}
											break;
						case 7:   ReadFLASEToPassWord2( addr,Password_Read, len);	//stm32f030c8 64kb flash,8kb sram, addr=0x08009000,len=6
						          load_password();	//��ȡ��ǰ�洢��mcu�е�password��Password_Save1������
						          if(strcmp(Password_Save1,Password_Input)==0)		//������ͬ		
											{														
													FASH=0;
													CC2500.key_keepdown=0;
													Password.Pass_flag=0;
													Password.a=0;
													i=0;	//�����ر�־λ
													
													while(1)
													{																
															switch(FASH)
															{													
																	case 0:LCD_write_string(1,1,"Input: Correct!");break;
																	case 1:LCD_write_string(1,1,"Input:         ");break;
																	case 2:LCD_write_string(1,1,"Input: Correct!");break;
																	case 3:LCD_write_string(1,1,"Input:         ");break;
															}
															if(FASH>=3) 
																break;	//�˳���ѭ��
													}															
											}
											else
											{
													faultcount++;
													FASH=0;
													CC2500.key_keepdown=0;
													while(1)
													{																										
															switch(FASH)
															{													
																	case 0:LCD_write_string(1,1,"Input: Error!!!");break;
																	case 1:LCD_write_string(1,1,"Input:         ");break;
																	case 2:LCD_write_string(1,1,"Input: Error!!!");break;
																	case 3:LCD_write_string(1,1,"Input:         ");break;
															}
															if(FASH>=3)
																break;
													}
														
													Password.Pass_flag=1;	//��־λ������λ
													i=0;
													Password.a=1;
													ERROR_Password();	//���������������3�Σ����������ʾ
													Me.PowerSaveDelay=0;
													goto bb;
											}
											Me.PowerSaveDelay=0;
											break;
				}				
		}
}
/********************************************************************
**		�������������������3��
**		
**		����ֵ����
********************************************************************/
void 	ERROR_Password(void)
{
	  if(faultcount==3)
		 {
			 faultcount=0;
			 LCD_Clear();			 
			 LCD_write_string(1,0,"Error 3 times!");			  			 
			 Delay_ms(2);
			 LCD_write_string(1,1,"Please again!" );
		   Delay_ms(2000);
		 }
}

/********************************************************************
**		��������������
**		
**		����ֵ����
********************************************************************/
int main(void)
{   		   
		//ϵͳ����29v��������¼����Ҫһֱhold ��start�� key������¼���˰������£������ܵ�ͨ��mcu��3.3v����; //���������Ժ�mcuͨ������IO�ڣ�ʹlcd����;	//����"reset" key,ǿ�����Ͳ��ر������ܣ�mcu�ϵ磬lcd����
		uint8_t i=0,tmpclk=0;
		RCC_ClocksTypeDef RCC_Clocks;
	
		SystemInit();	
	
		RCC_GetClocksFreq(&RCC_Clocks);	//system clk 24Mhz
	
		tmpclk = RCC_GetSYSCLKSource();	//pll clk
	
		if(tmpclk>0)
			tmpclk=1;
		else
			tmpclk=0;
	
		Delay_ms(100);
		Read_STM32ID();
		CC2500_PA_Init();	//��ʼ������TX_EN,RX_EN �������
		lcd1602key_Init();
		USART1_Configuration();
		TIM3_Configuration();	//��ʱ50ms
		TIM14_Configuration();//��ʱ10ms/9ms
		SPI_Configuration();			
		RF_Init();	
		LCD_Init();		
		
		Me.PowerSaveDelay=0;
		
//		Password_Enter();	 	//zwg 2019.03.02
		
		
		LCD_write_string(0,1," Status: Standby");
		LCD_write_string(0,0," Cinema  system ");
		Me.PowerSaveDelay=0;
		while(1)
		{   		
				/* //�˰���������clean mode���ܳ�ͻ�����Ը�Ϊͬʱ����num1,num2,num3,num4
				if( KEY_OPEN==0 && KEY_CLOSE	==0 && KEY_EN == 0 )	//start keyû�а��� ���� open&close key����								
				{
						Delay_ms(20);
						if( KEY_OPEN==0 && KEY_CLOSE	== 0 && KEY_EN == 0 )
						{ 		             
								Change_Setpassword();  //������������
						}
				}
				*/
				if(KEY_NUM1==1 && KEY_NUM2==0 && KEY_NUM3==0 && KEY_NUM4==1)	//ͬʱ����num1,num2,num3,num4 key					
				{
						Delay_ms(20);
						if(KEY_NUM1==1 && KEY_NUM2==0 && KEY_NUM3==0 && KEY_NUM4==1)
						{
								LCD_Clear();
								Delay_ms(10);
								LCD_write_string(1,0,"Init Password");
								LCD_write_string(1,1,"Input:       ");	//zwg 2019.03.02
								while((KEY_NUM2==0 || KEY_NUM3==0));
								Me.PowerSaveDelay=0;
								Change_Setpassword();  //������������
								Me.PowerSaveDelay=0;
						}
				}
//				else if(KEY_NUM1==0 && KEY_NUM2==1 && KEY_NUM3==1 && KEY_NUM4==0)	//ͬʱ����num1,num4 key,��λ��ʼ����					
//				{
//						Delay_ms(20);
//						if(KEY_NUM1==0 && KEY_NUM2==1 && KEY_NUM3==1 && KEY_NUM4==0)
//						{
//								LCD_Clear();
//								Delay_ms(10);
//								LCD_write_string(1,0,"reset Password");
//								LCD_write_string(1,1,"Open:Y  Close:N");	//zwg 2019.03.02
//								while((KEY_NUM1==0 || KEY_NUM4==0));
//								while(1){
//									if(KEY_OPEN==0){
//										Me.PowerSaveDelay=0;
//										LCD_write_string(1,1," Wait Reset ...");	//zwg 2019.03.02
//										Delay_ms(1000);
//										LCD_write_string(1,1," Wait Reset .. ");	//zwg 2019.03.02
//										Delay_ms(1000);
//										LCD_write_string(1,1," Wait Reset .  ");	//zwg 2019.03.02
//										Delay_ms(1000);
//										WritePasswordToFLASE1(addr,passwordReset,6);
//										LCD_write_string(1,1," Wait Reset ok ");	//zwg 2019.03.02
//										Delay_ms(1000);
//										Me.PowerSaveDelay=0;
//										break;
//									}else if(KEY_CLOSE==0){
//										Me.PowerSaveDelay=0;
//										break;
//									}
//								}
//						}
//				}
				LCD_write_string(0,1," Status: Standby");
				LCD_write_string(0,0," Cinema  system ");
				lcd1602key_Handle();	 
				PWR_DOWNmode();	//Կ��������Ҫ�˳�����������Ҫ��				
		}
}
//easy clean mode�£����û�з��� opening��closing����򳤰�open&close��Ч easyMode_closeFlag��easyMode_openFlag

//key_en��KEY_OPEN��KEY_CLOSE��Ч,Key1��Ч,��λ��ʼ����1��2��3��4��1��2

//ͬʱ����num1,num2,num3,num4 key,������������
