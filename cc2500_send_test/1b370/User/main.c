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

//RF 工作参数
struct
{
    uchar Chn;
    uchar LockChnFlag; //锁定频点
    uchar WorkFlag;
    uchar SendSn;
    uchar SendValue;  //==0xff时,表示不发送
    uchar SendAgain;	//重复发送计数器
    u16  SendCnt;//发送次数
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
**		函数名：CC2500 PA控制初始化
**		
**		返回值：无
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
	
			RX_OFF;		//接收PA关闭
			TX_OFF;		//发送PA关闭
			PWR_ON;		//上电保持供电,lcd亮屏
}

/********************************************************************
**		函数名：定时器中断14配置
**		
**		返回值：无
*******************************************************************/
void TIM14_Configuration(void) //10ms
{
		TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
		NVIC_InitTypeDef NVIC_InitStruct;

		TIM_DeInit(TIM14);//初始化
		
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);//打开时钟
		
		TIM_TimeBaseStructure.TIM_Period = 10 * 1000- 1; //10ms  //修改 900
		TIM_TimeBaseStructure.TIM_Prescaler = 24 - 1;	//1次计数1us,计数10*1000次是10ms
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
**		函数名：定时器14中断函数
**		
**		返回值：无
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
				//CC2500.Sendcount = 1;//500ms发送一次
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
**		函数名：定时器中断3配置
**		
**		返回值：无
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
		
		//TIM_Cmd(TIM3, ENABLE);	没有开启定时器3,第一次会进入中断
}

/********************************************************************
**		函数名：定时器3中断函数
**		
**		返回值：无
*******************************************************************/

void TIM3_IRQHandler(void) //50ms
{
    TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update);
		TIM_Cmd(TIM3, DISABLE);
}

/********************************************************************
**		函数名：串口初始化函数
**		
**		返回值：无
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

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
	USART_ClearFlag(USART1,USART_FLAG_TC);//清除USARTx的待处理标志位
}

/********************************************************************
**		函数名：串口通信中断
**		
**		返回值：无
*******************************************************************/
void USART1_IRQHandler(void)	//串口1中断函数
{
	static u8 k;
	USART_ClearFlag(USART1,USART_FLAG_TC);
	if(USART_GetITStatus(USART1,USART_IT_RXNE)!=Bit_RESET)//检查指定的USART中断发生与否
	{
		k=USART_ReceiveData(USART1);
		//k++;	
		USART_SendData(USART1,k);//通过外设USARTx发送单个数据
		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==Bit_RESET);	
	}
}

/********************************************************************
**		函数名：读取CPU ID
**		
**		返回值：无
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
**		函数名：数据校验
**		
**		返回值：无
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
**		函数名：发送对码值
**		
**		返回值：无
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
**		函数名：发送对码值
**		
**		返回值：无
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
**		函数名：发送对码值
**		
**		返回值：无
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
**		函数名：发送对码值
**		
**		返回值：无
*******************************************************************/
uchar Rf_SendKeyValue(uchar KEYnumber,uchar mode)
{
		uchar tmp[20];
	
		tmp[0]	=	0x06;
		tmp[1]	=	Rf.SendSn;	//随机码
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
   	Rf.SendSn++;	//每发送一次数据,该码值不同
   	Rf_SendData(tmp,13);	//发送完数据以后,切换为接收模式,接收控制盒传输的数据

	return(FALSE);
}

/********************************************************************
**		函数名：发送对码值
**		
**		返回值：无
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
	  FLASH_ErasePage(addr);		//addr = 0x08009000,写数据之前，先擦除页地址
	  while(len--)
		{
					halfWord=*(p++);
			    halfWord |=*(p++)<<8;
          FLASH_ProgramHalfWord(addr,halfWord);	//一次写入2个字节 word表示4byte
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
**		函数名：自动掉电模式
**		
**		返回值：无
*******************************************************************/
void PWR_DOWNmode(void)	//zwg 0306
{
		if(Me.PowerSaveDelay>=6000)	//num1,,,num4按键无效，其它按键有效  200  zwg 0305  400000
		{
				Me.PowerSaveDelay=0;
				PWR_OFF;	//相当于mcu断电
				Delay_ms(1000);
		}	
}	

/********************************************************************
**		函数名：自动关电
**		
**		返回值：无
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
**		函数名：按键扫描处理
**		
**		返回值：无
*******************************************************************/
void lcd1602key_Handle(void)   //??????
{			
			u8 i;
	
			PWR_ON;	//开启背光
			if((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0))	//num1,,,num4按键无效
					Me.PowerSaveDelay=0;
			switch(lcd1602key_Scan())
			{
					case KEY_ON:			
						TX_OFF;		//只有key_en有效																							
						break;
				
					case 0:			  		
						TX_OFF;		//其它按键																							
						break;
					
					
					case OPEN_LED:
						tmpFashFlag=0;
						TX_EN;																									
						while(1)
						{																			
							Me.PowerSaveDelay=0;
							LCD_write_string(0,1,"Status: LedON...");		
							if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x00); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_ON,0x00); 	//双数
									Delay_ms(10);
								}
							}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x01); 		//双数
								}
							}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue_2(LED_ON,0x02); 	//单数
								}
							}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
								while(Me.PowerSaveDelay<=30){
									Rf_SendKeyValue(LED_ON,0x10); 		//单数 //0x10
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_ON,0x10); 	//双数	//0x10
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
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x00); 		//单数
										Delay_ms(10);
										Rf_SendKeyValue_2(LED_OFF,0x00); 	//双数
										Delay_ms(10);
									}
								}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x01); 		//双数
									}
								}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue_2(LED_OFF,0x02); 	//单数
									}
								}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									while(Me.PowerSaveDelay<=30){
										Rf_SendKeyValue(LED_OFF,0x10); 		//单数 //0x10
										Delay_ms(10);
										Rf_SendKeyValue_2(LED_OFF,0x10); 	//双数	//0x10
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
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(LED_Red,0x00); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Red,0x00); 	//双数
									Delay_ms(10);
								}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Red,0x01); 		//双数
								}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(LED_Red,0x02); 	//单数
								}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Red,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Red,0x10); 	//双数
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

								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(LED_Green,0x00); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Green,0x00); 	//双数
									Delay_ms(10);
								}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Green,0x01); 		//双数
								}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(LED_Green,0x02); 	//单数
								}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Green,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Green,0x10); 	//双数
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
								 
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(LED_Blue,0x00); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Blue,0x00); 	//双数
									Delay_ms(10);
								}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Blue,0x01); 		//双数
								}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(LED_Blue,0x02); 	//单数
								}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Blue,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Blue,0x10); 	//双数
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
								
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(LED_Off,0x00); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Off,0x00); 	//双数
									Delay_ms(10);
								}else if(Clean_flag==2){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Off,0x01); 		//双数
								}else if(Clean_flag==1){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(LED_Off,0x02); 	//单数
								}else if(Clean_flag==0){	//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(LED_Off,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(LED_Off,0x10); 	//双数
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
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(KEY_out,0x00); 		//单数
//									Delay_ms(10);
//									Rf_SendKeyValue_2(KEY_out,0x00); 	//双数
//									Delay_ms(10);
//									switch(FASH)
//									{																			
//										case 0:	 LCD_write_string(0,1," Stand  Opening ");   break;																
//										case 1:  LCD_write_string(0,1,"                ");   break;
//										case 2:  LCD_write_string(0,1," Stand  Opening ");   break;																
//										case 3:	 LCD_write_string(0,1,"                ");   break;
//									}
								}else if(Clean_flag==2){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(KEY_out,0x01); 		//单数
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Odd  Opening  ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Odd  Opening  ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==1){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(KEY_out,0x02); 		//双数
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Even  Opening ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Even  Opening ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==0){			//同时控制单/双椅子，只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(KEY_out,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(KEY_out,0x10); 	//双数
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
						for(i=0;i<100;i++)	//按键松手以后，发送停止信号，停止马达
						{ALL_STOP;Delay_ms(20);}
						Me.PowerSaveDelay=0;
						break;
																
					case KEY_out:			
						TX_EN;
						while((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))
						{																		
								Me.PowerSaveDelay=0;
								if(Clean_flag==3){			//同时控制单/双椅子,无论椅子是否有操作手控器HC225
									Rf_SendKeyValue(KEY_in,0x00); 		//单数
//									Delay_ms(10);
//									Rf_SendKeyValue_2(KEY_in,0x00); 		//双数
//									Delay_ms(10);
//									switch(FASH)
//									{																
//										case 0:	 LCD_write_string(0,1," Stand  Closing ");	  break;															
//										case 1:  LCD_write_string(0,1,"                ");   break;
//										case 2:  LCD_write_string(0,1," Stand  Closing ");	  break;															
//										case 3:	 LCD_write_string(0,1,"                ");   break;
//									}
								}else if(Clean_flag==2){	//控制单数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(KEY_in,0x01); 		//单数
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Odd  Closing  ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Odd  Closing  ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==1){	//控制双数,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue_2(KEY_in,0x02); 		//双数
									switch(FASH)
									{																			
										case 0:	 LCD_write_string(0,1,"  Even  Closing ");   break;																
										case 1:  LCD_write_string(0,1,"                ");   break;
										case 2:  LCD_write_string(0,1,"  Even  Closing ");   break;																
										case 3:	 LCD_write_string(0,1,"                ");   break;
									}
								}else if(Clean_flag==0){			//同时控制单/双椅子,只能控制那些被手控器HC225操作过的椅子
									Rf_SendKeyValue(KEY_in,0x10); 		//单数
									Delay_ms(10);
									Rf_SendKeyValue_2(KEY_in,0x10); 		//双数
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
										case 4: LCD_write_string(0,1," Enter  Pair 0  "); Enter_succee=1;break;	//必须一直长按，否则系统不会发送配对码
								}
										
								while(Enter_succee)
								{ 
										while((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
										{
												Pair_time=0;																			
												LCD_write_string(1,1,"Enter  Pair OK  ");
										}
												
										while((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0));			//等待松手,必须所有的按键都释放																																		
										
										Me.PowerSaveDelay=0;	//重新开始计时
										if(Pair_flag==0)		//默认情况，Pair_flag=0，第一次进入单数配对模式
										{
											switch(FASH)
											{
												case 0: LCD_write_string(0,1," Odd Pairing.   "); break;
												case 1: LCD_write_string(0,1," Odd Pairing..  "); break;
												case 2: LCD_write_string(0,1," Odd Pairing... "); break;
												case 3: LCD_write_string(0,1," Odd Pairing...."); break;
											}	
										}
										else if(Pair_flag==1)	//第二次进入双数配对模式
										{
											switch(FASH)
											{
														
												case 0: LCD_write_string(0,1,"Even Pairing.   "); break;
												case 1: LCD_write_string(0,1,"Even Pairing..  "); break;
												case 2: LCD_write_string(0,1,"Even Pairing... "); break;
												case 3: LCD_write_string(0,1,"Even Pairing...."); break;
											}	
										}
										if(Pair_flag==0){	//单数模式配对，发送对应的配对码(当前mcu的id)
											Rf_SendStudyId();
										}else{				//双数模式配对，发送对应的配对码(当前mcu的id)
											Rf_SendStudyId_2();
										}
												
										if((KEY_EN==1)||(KEY_OPEN==0)||(KEY_CLOSE==0)||(Over_Pair_time>= 200))	//超时退出或者按键KEY_EN,KEY_OPEN,KEY_CLOSE退出
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
//						if(Clean_flag==0){	//easymode下，如果没有按下按键操作马达
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
								else if(Clean_flag==1){		//进入此模式，需要判断是否有hc225操作&坐垫坐下
									for(i=0;i<50;i++){
										Rf_SendKeyValue(KEY_easy_clean_enter,0xee);		//zwg 0305
										Delay_ms(10);
										Rf_SendKeyValue_2(KEY_easy_clean_enter,0xee);	//zwg 0305
										Delay_ms(10);
									}
								}
								else if(Clean_flag==0){
									if((easyMode_openFlag==0)&&(easyMode_closeFlag==0)){	//easymode下，如果没有按下按键操作马达,不发送复位指令
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
**		函数名：写入EEPROM
**		
**		返回值：无
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
**		函数名：Flash初始化程序
**		
**		返回值：无
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
**		函数名：密码设置
**		
**		返回值：无
********************************************************************/
u16 len=6;
u32 addr=0x08009000;
uchar j=0;

 void 	ERROR_Password(void);
 void Change_Setpassword(void)	//若不对i初始化，则会导致第一次输入的原始密码，存储在数组中的位置是一个随机数，导致最终"显示密码"错误，需要再次输入密码，才能ok
{
		u8 fisrtEnterThisFunc=0;
		u8 passwordtmp1[10];
		u8 passwordtmp2[10];
		u8 passwordtmp3[10];
		u8 passwordtmp4[10];
		u8 passwordtmp5[10];
	
cc:
		if(fisrtEnterThisFunc==1)	//第一次进入此函数，不需要刷新屏幕
		{
			LCD_Clear();	//显示清屏
			Delay_ms(10);	
			LCD_write_string(1,0,"Init Password");
		}
		fisrtEnterThisFunc=1;
		i=0;	//必须加入此初始化语句，否则有bug
		Password.Pass_flag=1;
		Password.a=1;
		Password.b=1;
		Password.c=1;
		while(1)
		{		//先输入正确的原始密码，再输入新密码
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
													if(!KEY_NUMall)	//低电平有效
													{
															passwordtmp1[i]=Password_keyScan();
														
															while(!KEY_NUMall);	//松手检测
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
															LCD_write_string(1,1,"Input:****** ");	//密码输入完成，此时等待按下"start" key 确认，其它按键无效
														
															Delay_ms(500);
													}
													break;
												
							case 7:  		if( KEY_EN == 1 )	// 按下"start" key 确认
													{
															Delay_ms(20);
															if( KEY_EN == 1 )
															{
																	Password.a=0;	//case 索引标号清零
																	//判断输入的密码与保存的密码是否一致
																	if(((Password_Save1[0]==passwordtmp1[0])&&(Password_Save1[1]==passwordtmp1[1])
																		&&(Password_Save1[2]==passwordtmp1[2])&&(Password_Save1[3]==passwordtmp1[3])
																		&&(Password_Save1[4]==passwordtmp1[4])&&(Password_Save1[5]==passwordtmp1[5]))
																		||((Password_Read[0]==passwordtmp1[0])&&(Password_Read[1]==passwordtmp1[1])
																		&&(Password_Read[2]==passwordtmp1[2])&&(Password_Read[3]==passwordtmp1[3])
																		&&(Password_Read[4]==passwordtmp1[4])&&(Password_Read[5]==passwordtmp1[5])))
																		{
																				i=0;	//存储输入密码的数组下标索引清零
																				Delay_ms(500);
																				LCD_Clear(); 
																				Delay_ms(2);				
																				aa:	
																				LCD_write_string(1,0," New Password  ");	
																				while(Password.Pass_flag)
																				{	 											
																						switch(Password.b)     //代码结构同上述
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
																																Password.b=0;	//清除相关标志位																 
																																strcpy(passwordtmp5,passwordtmp2);	//拷贝输入的密码，进行存储
																																Delay_ms(500);
																																LCD_Clear(); 
																																Delay_ms(2);
																																LCD_write_string(1,0," New Password"); 			
																																while(Password.Pass_flag)		//代码结构同上述
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
																																													Password.c=0;			//清除标志位
																																													//判断两次输入的密码是否一致
																																													if(	(passwordtmp5[0]==passwordtmp4[0])&&(passwordtmp5[1]==passwordtmp4[1])
																																															&&(passwordtmp5[2]==passwordtmp4[2])&&(passwordtmp5[3]==passwordtmp4[3])
																																															&&(passwordtmp5[4]==passwordtmp4[4])&&(passwordtmp5[5]==passwordtmp4[5]))
																																													{	
																																															FASH=0;
																																															faultcount=0;	//密码相同，清除计数
																																															CC2500.key_keepdown=0;
																																															Password.Pass_flag=0;	//清除标志位，退出死循环
																																															j=0;	//清除数组下标
																																															Password.b=0;	//清除标志物
																																															Password.a=0;
																																															Password.c=0;  
																																															WritePasswordToFLASE1(addr,passwordtmp4,len);		//保存密码到mcu指定flash地址
																																															ReadFLASEToPassWord2( addr,Password_Read, len);	//读出数据进行验证
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
																																															faultcount++;	//密码错误，计数
																																															LCD_Clear();
																																															LCD_write_string(1,0,"2nd input error");
																																															LCD_write_string(1,1,"Pls input again");
																																															Delay_ms(2000);
																																															//重新初始化参数
																																															FASH=0;
																																															CC2500.key_keepdown=0;
																																															Password.Pass_flag=1;
																																															Password.a=1;
																																															Password.b=1;
																																															Password.c=1;
																																															j=0;
																																															ERROR_Password();
																																															goto  aa;	//重新输入第一次密码
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
																				faultcount++;	//输入密码与原始密码不一致
																				i=0;			//重新初始化数组下标						
																				LCD_Clear();
																				LCD_write_string(1,0,"  Init Error  ");
																				Delay_ms(1500);
																				LCD_Clear();
																				ERROR_Password();
																				goto cc;			//重新输入原始密码							
																		}
															}
													}
													break;
						} 	
				}	
			
				if(KEY_OPEN==1)	//低电平有效，设置完成以后自行退出死循环。
						break;   						
		}
		printf("nienienie5");	
}	
/********************************************************************
**		函数名：上电读出密码
**		
**		返回值：无
********************************************************************/
void load_password()
{	
			//每次烧录软件，都会擦除addr地址存储的password，因为在keil的设置项中，没有区分password的存储空间，每次擦除sector，则擦除整个64kb flash
			
			//如果在mcu中读出的密码为0/0xff，则说明flash中未使用新的密码，默认采用系统初始化密码Password_Save(1,2,3,4,1,2)
			//Password_Read表示当前存储在mcu中的密码
			if(((Password_Read[0]==0)&&(Password_Read[1]==0)&&(Password_Read[2]==0)&&(Password_Read[3]==0)
				&&(Password_Read[4]==0)&&(Password_Read[5]==0) )||(Password_Read[0]==0xFF)&&(Password_Read[1]==0xFF)
				&&(Password_Read[2]==0xFF)&&(Password_Read[3]==0xFF)&&(Password_Read[4]==0xFF)&&(Password_Read[5]==0xFF))
			{
					strcpy( Password_Save1, Password_Save );
			}
			else if( strcmp(Password_Read,Password_Save) == 0 )	//返回值为0,表示二者相等  //重新修改后的密码,而且修改的密码与Password_Save相同
			{ 
					strcpy( Password_Save1, Password_Save );
			}
			else	//重新修改后的密码,而且修改的密码与Password_Save不同
			{
					for(i=0;i<6;i++)
						Password_Save1[i] = Password_Read[i];
			}
}

void Reset_Default_Password(void)
{
	//同时按下num1,num4 key,复位初始密码					
	LCD_Clear();
	Delay_ms(10);
	LCD_write_string(1,0,"Reset Password");
	LCD_write_string(1,1,"Open:Y  Close:N");	//zwg 2019.03.02
	
	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
**		函数名：上电后需密码正确才能操作
**		
**		返回值：无
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
				PWR_DOWNmode();	//Me.PowerSaveDelay计时，30s无按键操作（num1..num4按下才有效），自动断电mcu
				switch(Password.a)
				{
						case 1:		switch(FASH)	//10ms刷新1次
											{
													case 0:LCD_write_string(1,1,"Input:         ");break;
													case 1:LCD_write_string(1,1,"Input:  _      ");break;				
													case 2:LCD_write_string(1,1,"Input:         ");break;
													case 3:LCD_write_string(1,1,"Input:  _      ");break;												
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
												{
													Reset_Default_Password();
													goto aa;
												}
											}
											else if(!KEY_NUMall)	
											{
													Password_Input[i]=Password_keyScan();//获取键值
												
													while(!KEY_NUMall);
													i=i+1;
													Password.a+=1;
													Me.PowerSaveDelay=0;	//重新计时
											}
											break;
						case 2:		switch(FASH)
											{
													case 0:LCD_write_string(1,1,"Input:  *      ");break;
													case 1:LCD_write_string(1,1,"Input:  *_     ");break;
													case 2:LCD_write_string(1,1,"Input:  *      ");break;
													case 3:LCD_write_string(1,1,"Input:  *_     ");break;
											}
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
											if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
											{
												Delay_ms(20);
												if( (KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)&&(KEY_NUM1==0) )	//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效
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
												
													LCD_write_string(1,1,"Input:  ****** ");	//输入密码完成
													Delay_ms(500);
													LCD_write_string(1,1,"Input:         ");	//清除显示
													Delay_ms(100);
													Me.PowerSaveDelay=0;
											}
											break;
						case 7:   ReadFLASEToPassWord2( addr,Password_Read, len);	//stm32f030c8 64kb flash,8kb sram, addr=0x08009000,len=6
						          load_password();	//读取当前存储在mcu中的password到Password_Save1数组中
						          if(strcmp(Password_Save1,Password_Input)==0)		//密码相同		
											{														
													FASH=0;
													CC2500.key_keepdown=0;
													Password.Pass_flag=0;
													Password.a=0;
													i=0;	//清除相关标志位
													
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
																break;	//退出死循环
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
														
													Password.Pass_flag=1;	//标志位重新置位
													i=0;
													Password.a=1;
													ERROR_Password();	//连续输入密码错误3次，进行相关提示
													Me.PowerSaveDelay=0;
													goto bb;
											}
											Me.PowerSaveDelay=0;
											break;
				}				
		}
}
/********************************************************************
**		函数名：输入密码错误3次
**		
**		返回值：无
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
**		函数名：主函数
**		
**		返回值：无
********************************************************************/
int main(void)
{   		   
		//系统接入29v并不能烧录，需要一直hold “start“ key才能烧录，此按键按下，三极管导通，mcu有3.3v供电; //程序运行以后，mcu通过控制IO口，使lcd亮屏;	//按下"reset" key,强制拉低并关闭三极管，mcu断电，lcd灭屏
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
		CC2500_PA_Init();	//初始化配置TX_EN,RX_EN 推挽输出
		lcd1602key_Init();
		USART1_Configuration();
		TIM3_Configuration();	//定时50ms
		TIM14_Configuration();//定时10ms/9ms
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
				/* //此按键功能与clean mode功能冲突，可以改为同时按下num1,num2,num3,num4
				if( KEY_OPEN==0 && KEY_CLOSE	==0 && KEY_EN == 0 )	//start key没有按下 并且 open&close key按下								
				{
						Delay_ms(20);
						if( KEY_OPEN==0 && KEY_CLOSE	== 0 && KEY_EN == 0 )
						{ 		             
								Change_Setpassword();  //重新设置密码
						}
				}
				*/
				if(KEY_NUM1==1 && KEY_NUM2==0 && KEY_NUM3==0 && KEY_NUM4==1)	//同时按下num1,num2,num3,num4 key					
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
								Change_Setpassword();  //重新设置密码
								Me.PowerSaveDelay=0;
						}
				}
//				else if(KEY_NUM1==0 && KEY_NUM2==1 && KEY_NUM3==1 && KEY_NUM4==0)	//同时按下num1,num4 key,复位初始密码					
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
				PWR_DOWNmode();	//钥匙锁不需要此程序，密码锁则要。				
		}
}
//easy clean mode下，如果没有发送 opening、closing命令，则长按open&close无效 easyMode_closeFlag，easyMode_openFlag

//key_en和KEY_OPEN，KEY_CLOSE有效,Key1有效,复位初始密码1，2，3，4，1，2

//同时按下num1,num2,num3,num4 key,重新设置密码
