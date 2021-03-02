#include "key.h"
#include "lcd1602.h"

unsigned int openKey_TimeDelay=0;
unsigned int openKey_pressNum=0;

unsigned int closeKey_TimeDelay=0;
unsigned int closeKey_pressNum=0;

u16 keyflag1,keyflag2;
void lcd1602key_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);	
	
		GPIO_InitStructure.GPIO_Pin = KEYOPEN |KEYCLOSE;  	//open,close key�������룬�ߵ�ƽ��Ч
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 
		GPIO_InitStructure.GPIO_PuPd=	GPIO_PuPd_UP;						
		GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
		GPIO_InitStructure.GPIO_Pin = KEYEN; 		// start key	��������,�ߵ�ƽ��Ч
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_10MHz;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
		GPIO_InitStructure.GPIO_PuPd=	GPIO_PuPd_DOWN;	
    GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
		GPIO_InitStructure.GPIO_Pin = NUM1|NUM2|NUM3|NUM4; //4������key,�������룬�ߵ�ƽ��Ч
		GPIO_InitStructure.GPIO_Speed= GPIO_Speed_10MHz;	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;  
		GPIO_InitStructure.GPIO_PuPd=	GPIO_PuPd_UP;	
    GPIO_Init(GPIOB, &GPIO_InitStructure);
		
		GPIO_InitStructure.GPIO_Pin = PWR_KEEP;	//mcu ��Դ����io
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		PWR_ON;		
}
/********************************************************************
**		��������lcd1602����ɨ��
**		
**		����ֵ����
********************************************************************/

u8	KEYnumber;
u8 lcd1602key_Scan()	//����ɨ��
{
		u8 Button_number;
		if((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==1) && KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )		//ֻ��key_en��Ч
		{  
				Delay_ms(10);
				if((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==1) && KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )
				{
					Button_number=KEY_ON;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))	//key_en��KEY_CLOSE��Ч
		{
				Delay_ms(10);
				if((KEY_EN==1)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))
				{
					Button_number=KEY_out;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==1))	//key_en��KEY_OPEN��Ч
		{
				Delay_ms(10);
				if((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==1))
				{
					Button_number=KEY_in;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))	//key_en��KEY_OPEN��KEY_CLOSE��Ч(���)
		{
				Delay_ms(10);
				if((KEY_EN==1)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
				{					
					Button_number=KEY_study;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==0)) //key_en��Ч��KEY_OPEN��KEY_CLOSE��Ч���������ģʽ
		{
				Delay_ms(10);
				if((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==0))
				{
					Button_number=KEY_clean_enter;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if( KEY_EN==1 && !KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )	//key_en��KEY_NUM1��Ч
		{
				Delay_ms(10);
				if( KEY_EN==1 && !KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )
				{					
					Button_number=LED_Red;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		else if( KEY_EN==1 && KEY_NUM1 && !KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )	//key_en��KEY_NUM2��Ч
		{
				Delay_ms(10);
				if( KEY_EN==1 && KEY_NUM1 && !KEY_NUM2 && KEY_NUM3 && KEY_NUM4 )
				{					
					Button_number=LED_Green;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		
		else if( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && !KEY_NUM3 && KEY_NUM4 )	//key_en��KEY_NUM3��Ч
		{
				Delay_ms(10);
				if( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && !KEY_NUM3 && KEY_NUM4 )
				{					
					Button_number=LED_Blue;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		
		else if( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && !KEY_NUM4 )	//key_en��KEY_NUM4��Ч
		{
				Delay_ms(10);
				if( KEY_EN==1 && KEY_NUM1 && KEY_NUM2 && KEY_NUM3 && !KEY_NUM4 )
				{					
					Button_number=LED_Off;
					
					openKey_pressNum=0;
					closeKey_pressNum=0;
				}		
		}
		
		//modify
		else if((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==1))	//ֻ����open key
		{
				Delay_ms(10);
				if((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==1))	//ֻ����open key
				{
					while((KEY_EN==0)&&(KEY_OPEN==0)&&(KEY_CLOSE==1));
					if(openKey_pressNum==0){
						openKey_TimeDelay=0;
					}
					openKey_pressNum++;
					
					if( (openKey_TimeDelay<=150) && (openKey_pressNum>=3) ){		//1.5s�ڰ�������3�ΰ��£���Ч
						Button_number=OPEN_LED;
						
						openKey_pressNum=0;
						openKey_TimeDelay=0;
						
					}
						
				}
				
		}
		else if((KEY_EN==0)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))	//ֻ����close key
		{
				Delay_ms(10);
				if((KEY_EN==0)&&(KEY_OPEN==1)&&(KEY_CLOSE==0))	//ֻ����close key
				{
					while((KEY_EN==0)&&(KEY_OPEN==1)&&(KEY_CLOSE==0));
					if(closeKey_pressNum==0){
						closeKey_TimeDelay=0;
					}
					closeKey_pressNum++;
					
					if( (closeKey_TimeDelay<=150) && (closeKey_pressNum>=3) ){		//1.5s�ڰ�������3�ΰ��£���Ч
						Button_number=CLOSE_LED;
						
						closeKey_pressNum=0;
						closeKey_TimeDelay=0;
					}
				}
				
		}
		else{
			Button_number=0;	//��������
		}
		
		if(openKey_TimeDelay>150){  //1.5s�ڰ���С��3�ΰ��£���Ч
			openKey_pressNum=0;
			openKey_TimeDelay=0;
		}
		
		if(closeKey_TimeDelay>150){  //1.5s�ڰ���С��3�ΰ��£���Ч
			closeKey_pressNum=0;
			closeKey_TimeDelay=0;
		}
		
		
		
		return (u8)Button_number;
}
/********************************************************************
**		�����������밴��ɨ��
**		
**		����ֵ����
********************************************************************/


u16 keyflag2; 
u8 Password_keyScan()	//����ɨ��
{
		
     keyflag2=0;	
    	
		if((KEY_NUM1==0)&&(KEY_NUM2==1)&&(KEY_NUM3==1)&&(KEY_NUM4==1))
		{
			  keyflag2=1;
				Delay_ms(50);
				if((KEY_NUM1==0)&&(KEY_NUM2==1)&&(KEY_NUM3==1)&&(KEY_NUM4==1))
				{
					 KEYnumber=1;
					 
				}		
		}
		
		else if((KEY_NUM1==1)&&(KEY_NUM2==0)&&(KEY_NUM3==1)&&(KEY_NUM4==1))
		{
			  keyflag2=1;
				Delay_ms(50);
				if((KEY_NUM1==1)&&(KEY_NUM2==0)&&(KEY_NUM3==1)&&(KEY_NUM4==1))
				{
					KEYnumber=2;
				}		
		}
		
		else if((KEY_NUM1==1)&&(KEY_NUM2==1)&&(KEY_NUM3==0)&&(KEY_NUM4==1))
		{
			  keyflag2=1;
				Delay_ms(50);
				if((KEY_NUM1==1)&&(KEY_NUM2==1)&&(KEY_NUM3==0)&&(KEY_NUM4==1))
				{
					KEYnumber=3;
				}		
		}
		
		else if((KEY_NUM1==1)&&(KEY_NUM2==1)&&(KEY_NUM3==1)&&(KEY_NUM4==0))
		{
			  keyflag2=1;
				Delay_ms(50);
				if((KEY_NUM1==1)&&(KEY_NUM2==1)&&(KEY_NUM3==1)&&(KEY_NUM4==0))
				{					
					KEYnumber=4;
				}		
		}
		
		else KEYnumber=0;
		
		return (u8)KEYnumber;
}





















