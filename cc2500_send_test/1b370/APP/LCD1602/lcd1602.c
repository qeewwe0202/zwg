#include "lcd1602.h"

u16 GPIO_PINS[8]={ GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2 ,GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7 }; 

void LCD_Init(void)
{
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
	
			GPIO_InitStructure.GPIO_Pin = LCD_DATA | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;	//LCD_EN,LCD_RW,LCD_RS �������
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
			GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
			GPIO_Init(GPIOA, &GPIO_InitStructure);		
			LCD_INIT();
}

void GPIO_Write_8bit(u8 Com_data)
{
			u8 i;
	
			for( i = 0; i < 8; i++ )
			{
				 if( Com_data & 0x01 )
					GPIO_SetBits(GPIOA,GPIO_PINS[i]); 
				 else
					GPIO_ResetBits(GPIOA,GPIO_PINS[i]);
				 
				 Com_data=Com_data>>1;				 
			}
}


u8 LCD_check_busy()//������æ
{
			//u8 ReadValue;
			//GPIO_Write(GPIOA,0xFF);
			GPIO_Write_8bit(0xFF);
			RS_CLR;
			Delay_us(1);
			RW_SET;
			Delay_us(1);
			do
			{
				EN_CLR;
				Delay_us(200);
				EN_SET;
				Delay_us(200);
			}while(GPIOA->IDR&0X80);
			
			 return (u8)0;
}

void LCD_initwrite_com(unsigned char com)//�����ʼ��
{

			EN_CLR;
			Delay_us(1);
			RS_CLR;
			Delay_us(1);
			RW_CLR;
			Delay_us(5);
			//GPIO_Write(GPIOA,com);
	
			GPIO_Write_8bit(com);
	
			Delay_us(200);
			EN_SET;
			Delay_us(200);
			EN_CLR;
			Delay_us(200);
}

void LCD_write_com(unsigned char com)//д����
{
			EN_CLR;
			Delay_us(1);
			RS_CLR;
			Delay_us(1);
			RW_CLR;
			Delay_us(1);
	
			GPIO_Write_8bit(com);
	
			Delay_us(100);
			EN_SET;
			Delay_ms(1);
			EN_CLR;
			Delay_us(100);
}

void LCD_write_data(unsigned char Data)//д����
{
			EN_CLR;
			Delay_us(1);
			RS_SET;
			Delay_us(1);
			RW_CLR;
			Delay_us(1);
	
			GPIO_Write_8bit(Data);
	
			Delay_us(400);
			EN_SET;
			Delay_ms(1);
			EN_CLR;
			Delay_us(100);
}

void LCD_Clear(void)//���lcd��
{
			LCD_write_com(0x01);
			Delay_ms(4);
}

void LCD_write_string(unsigned char x,unsigned char y,unsigned char *s )//д�ַ���
{
			if (y == 0)
			{
				LCD_write_com(0x80+x);
			}
			else
			{
				LCD_write_com(0xc0+x);
			}
			
			while (*s)
			{
				LCD_write_data(*s);
				s++;
				Delay_us(400);
			}
}

void LCD_write_char(unsigned char x,unsigned char y,unsigned char data)//д���ַ�
{
			if (y == 0)
				{
					LCD_write_com(0x80+x);
				}
			else
				{
					LCD_write_com(0xc0+x);
				}
					LCD_write_data(data);
					Delay_us(400);
}
void LCD_INIT(void)//LCD��ʼ��
{
			Delay_ms(10);
			LCD_write_com(0x38); 
			Delay_ms(4);
			LCD_write_com(0x38);
			Delay_ms(4);
			LCD_write_com(0x38);
			Delay_ms(4);
			LCD_write_com(0x38);	//8λ����ʾ2�У�5*7�ַ�
			Delay_ms(2);
			LCD_write_com(0x08); 	//��ʾ�ر�
			Delay_ms(4);
			LCD_write_com(0x01); 	//��ʾ����
			Delay_ms(4);
			LCD_write_com(0x06); 	//����ƶ�����
			Delay_ms(4);
			LCD_write_com(0x0c); 	//��ʾ�������������
			Delay_ms(4);
}

