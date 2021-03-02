#include "common.h"

//使用SysTick的普通计数模式对延迟进行管理
//包括delay_us,delay_ms  
static u8  fac_us=0;//us延时倍乘数
static u16 fac_ms=0;//ms延时倍乘数
//初始化延迟函数
//SYSCLK 取值如72,36等
void delay_init(u8 SYSCLK)
{
SysTick->CTRL&=0xfffffffb;//选择内部时钟 HCLK/8
fac_us=SYSCLK/8;      //72M/8=9M,,每次计数1/9us，所以计数为9时正好1us
fac_ms=(u16)fac_us*1000;
}            
//延时Nms
//注意Nms的范围
//Nms<=0xffffff*8/SYSCLK
//对72M条件下,Nms<=1864 
void delay_nms(u16 nms)
{    
SysTick->LOAD=(u32)nms*fac_ms; //时间加载  
SysTick->CTRL|=0x01;               //开始倒数    
while(!(SysTick->CTRL&(1<<16)));   //等待时间到达 
SysTick->CTRL&=0XFFFFFFFE;         //关闭计数器
SysTick->VAL=0X00000000;           //清空计数器     
}   
//延时us           
void delay_nus(u32 Nus)
{ 
SysTick->LOAD=Nus*fac_us;       //时间加载      
SysTick->CTRL|=0x01;            //开始倒数    
while(!(SysTick->CTRL&(1<<16)));//等待时间到达 
SysTick->CTRL=0X00000000;       //关闭计数器
SysTick->VAL=0X00000000;        //清空计数器     
} 

void delay_ns(u16 ns)
{
    while(ns--)
    {
        delay_nms(1000);
    }
}

