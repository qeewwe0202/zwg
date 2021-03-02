/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : cc2500.c
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
#include "CC2500.h"
#include "common.h"
#include "spi.h"
#include "ISR.h"
#include "CC2500_DEF.h"

u8 paTable[8] = {0xFB,0xFE};

void ClearDataBuf(u8 * pBuf,u8 Len)
{
    u8 cnt;
    for(cnt=0;cnt<Len;cnt++)
    {
        pBuf[cnt] = 0x00;
    }
}

RF_SETTINGS RFSettings = {
    0x06,   //IOCFG2    将GDO2作为收发状态指示引脚
    0x2E,   //IOCFG1    High impedance (3-state) 
    0x06,   //IOCFG0    将GDO0作为收发状态指示引脚
    0x06,   //FIFOTHR   设置TX FIFO和RX FIFO的阈值，分别为33和32
    0xD3,   //SYNC1     16位同步字的高八位
    0x91,   //SYNC0     16位同步字的低八位
    0xFF,   //PKTLEN    数据包长度指定寄存器,最大长度为255字节
    0x04,   //PKTCTRL1  两个状态字节附加在数据包的有效载荷上
    0x45,   //PKTCTRL0  采用可变长数据包模式
    0x00,   //ADDR      广播地址（0x00）
    0x00,   //CHANNR    无需跳频通信故共用信道
    0x09,   //FSCTRL1   设置IF频率为260kHz
    0x00,   //FSCTRL0   设置频率偏移量大小，无需频偏，去默认值0x00
    0x5D,   //FREQ2     配合FREQ1、FREQ0设置载波频率为2438MHz
    0x93,   //FREQ1     
    0xB1,   //FREQ0
    0x2D,   //MDMCFG4   设置滤波带宽为203kHz
    0x3B,   //MDMCFG3   配合MDMCFG4将数据传输速率设置为250bps
    0x73,   //MDMCFG2   选择MSK调制,Manchester Disable,30/32同步字模式
    0x22,   //MDMCFG1   FEC Disable,前导字节为4字节
    0xF8,   //MDMCFG0   与MDMCFG1一起设置信道带宽
    0x47,   //DEVIATN   信道背离频率设置
    0x00,   //MCSM2
    0x00,   //MCSM1     RX或TX过程结束后，都回到IDLE状态,收到一个数据包，就启用CCA
    0x18,   //MCSM0     从IDLE到TX或者RX时，自动载波频率校准,64次后纹波计数器终止
    0x1D,   //FOCCFG    由SmartRF Studio给出，设置频率偏移补偿
    0x1C,   //BSCFG     由SmartRF Studio给出，位同步配置
    0xC7,   //AGCCTRL2  由SmartRF Studio给出，AGC控制寄存器
    0x00,   //AGCCTRL1  
    0xB2,   //AGCCTRL0
    0x00,   //WOREVT1   事件 0 工作暂停寄存器的高字节
    0x00,   //WOREVT0   事件 0 工作暂停寄存器的低字节
    0x00,   //WORCTRL
    0xB6,   //FREND1    前端 RX 配置，主要是调整电流输出
    0x10,   //FREND0    设定PA功率，为PATABLE这个表格的索引值（000）
    0xEA,   //FSCAL3    频率合成器校准配置以及结果存储
    0x0A,   //FSCAL2    有软件进行取值，得到最佳结果
    0x00,   //FSCAL1
    0x11,   //FSCAL0
    0x00,   //RCCTRL1
    0x00,   //RCCTRL0
    0x59,   //FSTEST    测试用，不需要写这个寄存器
    0x00,   //PTEST     当在IDLE状态，写入0xBF，片内的温度传感器可用
    0x00,   //AGCTEST
    0x88,   //TEST2
    0x31,   //TEST1
    0x0B,   //TEST0    
};


void CC2500_Init()
{
    /*****Power-On Reset with SRES---RFCSn_wave****
    ***_(30us)__           ___(45us)_____          
    ***         |_(30us)__|              |________
    **********************************************/
    CC_SLK_H;
    CC_MO_L;

    CC_CSn_H;
    delay_nus(30);
    CC_CSn_L;
    delay_nus(30);
    CC_CSn_H;
    delay_nus(60);

    CC_CSn_L;
    while(CC2500_MI&CC_MISO);             //check busy or not

    CC2500_SPI_Write(SRES);
    
    while(CC2500_MI&CC_MISO);
	
    CC_CSn_H;

    CC2500_WriteSettings(&RFSettings);
    CC2500_WriteStrobe(SFRX);
    delay_nus(1);
    CC2500_WriteStrobe(SFTX);
    delay_nus(1);
    CC2500_SetPA();
}

void CC2500_WriteByte(u8 Addr,u8 data)
{
    CC_CSn_L;    
    while(CC2500_MI&CC_MISO);
    CC2500_SPI_Write(Addr+WRITE_SINGLE);
    __NOP();
    __NOP();
    __NOP();
    CC2500_SPI_Write(data);
    __NOP();
    __NOP();
    __NOP();
    CC_CSn_H;
}

void CC2500_BustWrite(u8 Addr,u8 * pData,u8 len)
{
    u8 i,cnt;
    u8 temp;
    CC_CSn_L;
    
    while(CC2500_MI&CC_MISO);
    CC2500_SPI_Write(Addr+WRITE_BURST);
    __NOP();
    __NOP();
    __NOP();
    for(i=0;i<len;i++)
    {
        temp = pData[i];
        for(cnt=0;cnt<8;cnt++)
        {
            if(temp&0x80)
                CC_MO_H;
            else
                CC_MO_L;
            temp<<=1;
            CC_SLK_L;
            delay_nus(1);
            CC_SLK_H;
            delay_nus(1);
        }
    }
    
    CC_CSn_H;
}

u8 CC2500_ReadReg(u8 Addr)
{
    u8 data=0xff;
    CC_CSn_L;
    
    while(CC2500_MI&CC_MISO);
    CC2500_SPI_Write(Addr+READ_SINGLE);
    __NOP();
    __NOP();
    __NOP();
    data = CC2500_SPI_RW(0xFF);
    __NOP();
    __NOP();
    __NOP();
    CC_CSn_H;

    return data;
}

void CC2500_BurstRead(u8 Addr,u8 *pBuf,u8 Len)
{
    u8 i;
    CC_CSn_L;

    while(CC2500_MI);
    CC2500_SPI_Write(Addr+READ_BURST);
    __NOP();
    __NOP();
    __NOP();
    for(i=0;i<Len;i++)
    {
        pBuf[i] = CC2500_SPI_RW(0xFF);
    }

    CC_CSn_H;
}

void CC2500_WriteSettings(RF_SETTINGS *pRfSettings)
{
    CC2500_WriteByte(FSCTRL1,pRfSettings->sFSCTRL1);
    CC2500_WriteByte(FSCTRL0,pRfSettings->sFSCTRL0);
    CC2500_WriteByte(FREQ2,pRfSettings->sFREQ2);
    CC2500_WriteByte(FREQ1,pRfSettings->sFREQ1);
    CC2500_WriteByte(FREQ0,pRfSettings->sFREQ0);
    CC2500_WriteByte(MDMCFG4,pRfSettings->sMDMCFG4);
    CC2500_WriteByte(MDMCFG3,pRfSettings->sMDMCFG3);
    CC2500_WriteByte(MDMCFG2,pRfSettings->sMDMCFG2);
    CC2500_WriteByte(MDMCFG1,pRfSettings->sMDMCFG1);
    CC2500_WriteByte(MDMCFG0,pRfSettings->sMDMCFG0);
    CC2500_WriteByte(CHANNR,pRfSettings->sCHANNR);
    CC2500_WriteByte(DEVIATN,pRfSettings->sDEVIATN);
    CC2500_WriteByte(FREND1,pRfSettings->sFREND1);
    CC2500_WriteByte(FREND0,pRfSettings->sFREND0);
    CC2500_WriteByte(MCSM0,pRfSettings->sMCSM0);
    CC2500_WriteByte(FOCCFG,pRfSettings->sFOCCFG);
    CC2500_WriteByte(BSCFG,pRfSettings->sBSCFG);
    CC2500_WriteByte(AGCCTRL2,pRfSettings->sAGCCTRL2);
    CC2500_WriteByte(AGCCTRL1,pRfSettings->sAGCCTRL1);
    CC2500_WriteByte(AGCCTRL0,pRfSettings->sAGCCTRL0);
    CC2500_WriteByte(FSCAL3,pRfSettings->sFSCAL3);
    CC2500_WriteByte(FSCAL2,pRfSettings->sFSCAL2);
    CC2500_WriteByte(FSCAL1,pRfSettings->sFSCAL1);
    CC2500_WriteByte(FSCAL0,pRfSettings->sFSCAL0);
    CC2500_WriteByte(FSTEST,pRfSettings->sFSTEST);
    CC2500_WriteByte(TEST2,pRfSettings->sTEST2);
    CC2500_WriteByte(TEST1,pRfSettings->sTEST1);
    CC2500_WriteByte(TEST0,pRfSettings->sTEST0);    
    CC2500_WriteByte(IOCFG2,pRfSettings->sIOCFG2);
    CC2500_WriteByte(IOCFG0,pRfSettings->sIOCFG0);
    //CC2500_WriteByte(FIFOTHR,pRfSettings->sFIFOTHR);0x07
    //CC2500_WriteByte(SYNC1,pRfSettings->sSYNC1);0xD3
    //CC2500_WriteByte(SYNC0,pRfSettings->sSYNC0);0x91    
    CC2500_WriteByte(PKTCTRL1,pRfSettings->sPKTCTRL1); 
    CC2500_WriteByte(PKTCTRL0,pRfSettings->sPKTCTRL0);    
    CC2500_WriteByte(ADDR,pRfSettings->sADDR);
    CC2500_WriteByte(PKTLEN,pRfSettings->sPKTLEN);    
}

void CC2500_Disable_Interrupt(void)
{
    NVIC_DisableIRQ(EXTI9_5_IRQn);
}

void CC2500_Enable_Interrupt(void)
{
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void CC2500_SetPA(void)
{
    CC2500_BustWrite(PATABLE,paTable,1);
    //CC2500_WriteByte(PATABLE,0xFE);
}

void CC2500_WriteStrobe(u8 Strobe)
{
    CC_CSn_L;
    
    while(CC2500_MI);
    CC2500_SPI_Write(Strobe);
    __NOP();
    
    CC_CSn_H;
}


void CC2500_ModeIdle(void)
{
    CC2500_WriteStrobe(SIDLE);
    delay_nus(50);
}

void CC2500_ModeRx(void)
{
    CC2500_WriteStrobe(SRX);
    delay_nus(50);
    CC2500_Enable_Interrupt();
}

void CC2500_ModeTx(void)
{
    CC2500_WriteStrobe(STX);
    delay_nus(50);
    CC2500_Disable_Interrupt();
}

void CC2500_ModePWRdown(void)
{
    CC2500_ModeIdle();
    delay_nus(50);
    CC2500_WriteStrobe(SPWD);
    delay_nus(50);
}

void CC2500_WaitTX_Done(void)
{
    while(!GDO0_IN);    //wait high--->transitting
    while(GDO0_IN);     //wait low---->finished
}

void CC2500_ClrRXFIFO(void)
{
    CC2500_ModeIdle();
    delay_nus(50);
    CC2500_WriteStrobe(SFRX);
    delay_nus(1);
}

void CC2500_ClrTXFIFO(void)
{
    CC2500_ModeIdle();
    delay_nus(50);
    CC2500_WriteStrobe(SFTX);
    delay_nus(1);
}

u8 CC2500_ReadStatus(void)
{
    u8 status;
    CC_CSn_L;
    while(CC2500_MI);
    status = CC2500_SPI_RW(SNOP);
		CC_CSn_H;
    return (status>>4);
}

ErrorStatus CC2500_RevPacket(void)
{
    u8 pktlen;
    ClearDataBuf(gCC2500RxBuf,MAXPACKETSIZE);
    CC2500_BurstRead(RXBYTES,&pktlen,1);
    if(pktlen&NUM_RXBYTES)
    {
        pktlen = CC2500_ReadReg(RXFIFO);
        gCC2500RxBuf[0] = pktlen;
        if(pktlen <= MAXPACKETSIZE)
        {
            CC2500_BurstRead(RXFIFO,gCC2500RxBuf+1,pktlen);
            CC2500_ClrRXFIFO();
            return SUCCESS;
        }
        else
        {
            CC2500_ClrRXFIFO();
            return ERROR;
        }
    }
    else
    {
        CC2500_ClrRXFIFO();
            return ERROR;
    }
}

void CC2500_SendPacket(void)
{
    //u8 test=0xff;
    CC2500_Disable_Interrupt();
    CC2500_BustWrite(TXFIFO,gCC2500TxBuf,gCC2500TxBuf[0]+1);
    //CC2500_BurstRead(TXBYTES,&test,1);
    CC2500_ModeTx();
    CC2500_WaitTX_Done();
    delay_nus(1);
    CC2500_ClrTXFIFO();
    //ClearDataBuf(gCC2500TxBuf,MAXPACKETSIZE);
}

