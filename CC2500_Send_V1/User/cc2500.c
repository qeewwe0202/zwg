/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : cc2500.c
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��8��
  ����޸�   :
  ��������   : InsertFileHeader
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��8��
    ��    ��   : tofu
    �޸�����   : �����ļ�

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
    0x06,   //IOCFG2    ��GDO2��Ϊ�շ�״ָ̬ʾ����
    0x2E,   //IOCFG1    High impedance (3-state) 
    0x06,   //IOCFG0    ��GDO0��Ϊ�շ�״ָ̬ʾ����
    0x06,   //FIFOTHR   ����TX FIFO��RX FIFO����ֵ���ֱ�Ϊ33��32
    0xD3,   //SYNC1     16λͬ���ֵĸ߰�λ
    0x91,   //SYNC0     16λͬ���ֵĵͰ�λ
    0xFF,   //PKTLEN    ���ݰ�����ָ���Ĵ���,��󳤶�Ϊ255�ֽ�
    0x04,   //PKTCTRL1  ����״̬�ֽڸ��������ݰ�����Ч�غ���
    0x45,   //PKTCTRL0  ���ÿɱ䳤���ݰ�ģʽ
    0x00,   //ADDR      �㲥��ַ��0x00��
    0x00,   //CHANNR    ������Ƶͨ�Źʹ����ŵ�
    0x09,   //FSCTRL1   ����IFƵ��Ϊ260kHz
    0x00,   //FSCTRL0   ����Ƶ��ƫ������С������Ƶƫ��ȥĬ��ֵ0x00
    0x5D,   //FREQ2     ���FREQ1��FREQ0�����ز�Ƶ��Ϊ2438MHz
    0x93,   //FREQ1     
    0xB1,   //FREQ0
    0x2D,   //MDMCFG4   �����˲�����Ϊ203kHz
    0x3B,   //MDMCFG3   ���MDMCFG4�����ݴ�����������Ϊ250bps
    0x73,   //MDMCFG2   ѡ��MSK����,Manchester Disable,30/32ͬ����ģʽ
    0x22,   //MDMCFG1   FEC Disable,ǰ���ֽ�Ϊ4�ֽ�
    0xF8,   //MDMCFG0   ��MDMCFG1һ�������ŵ�����
    0x47,   //DEVIATN   �ŵ�����Ƶ������
    0x00,   //MCSM2
    0x00,   //MCSM1     RX��TX���̽����󣬶��ص�IDLE״̬,�յ�һ�����ݰ���������CCA
    0x18,   //MCSM0     ��IDLE��TX����RXʱ���Զ��ز�Ƶ��У׼,64�κ��Ʋ���������ֹ
    0x1D,   //FOCCFG    ��SmartRF Studio����������Ƶ��ƫ�Ʋ���
    0x1C,   //BSCFG     ��SmartRF Studio������λͬ������
    0xC7,   //AGCCTRL2  ��SmartRF Studio������AGC���ƼĴ���
    0x00,   //AGCCTRL1  
    0xB2,   //AGCCTRL0
    0x00,   //WOREVT1   �¼� 0 ������ͣ�Ĵ����ĸ��ֽ�
    0x00,   //WOREVT0   �¼� 0 ������ͣ�Ĵ����ĵ��ֽ�
    0x00,   //WORCTRL
    0xB6,   //FREND1    ǰ�� RX ���ã���Ҫ�ǵ����������
    0x10,   //FREND0    �趨PA���ʣ�ΪPATABLE�����������ֵ��000��
    0xEA,   //FSCAL3    Ƶ�ʺϳ���У׼�����Լ�����洢
    0x0A,   //FSCAL2    ���������ȡֵ���õ���ѽ��
    0x00,   //FSCAL1
    0x11,   //FSCAL0
    0x00,   //RCCTRL1
    0x00,   //RCCTRL0
    0x59,   //FSTEST    �����ã�����Ҫд����Ĵ���
    0x00,   //PTEST     ����IDLE״̬��д��0xBF��Ƭ�ڵ��¶ȴ���������
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

