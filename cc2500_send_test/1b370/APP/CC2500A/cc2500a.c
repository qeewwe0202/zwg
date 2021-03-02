//=======================================================================
// 基于spi对cc2500进行上层驱动,完成数据包的发送与读取等功能.
// 晶振频率 = 26 MHz
// 传输码率 = 250.000000 kbps
// 射频频率 = 2.4GHz
// 数据包长度 = 255
// GDO0信号当sync发送/接收完成时提示
// GDO2可选Serial Clock
//=======================================================================
#include "cc2500a.h"

u8 Channel = 0;
u8 MY_ID[3] = {0};
u8 paTable[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
u8 paTableLen = 8;
u8 RX_ENABLE = 0;

u8 Link_mode = 0;
u8 Chip_ID[8] = {0};


//#if TI_CC_RF_FREQ == 2400                          // 2.4GHz
// Product = CC2500
// Crystal accuracy = 40 ppm
// X-tal frequency = 26 MHz
// RF output power = 0 dBm
// RX filterbandwidth = 540.000000 kHz
// Deviation = 0.000000
// Return state:  Return to RX state upon leaving either TX or RX
// Datarate = 250.000000 kbps
// Modulation = (7) MSK
// Manchester enable = (0) Manchester disabled
// RF Frequency = 2433.000000 MHz
// Channel spacing = 199.950000 kHz
// Channel number = 0
// Optimization = Sensitivity
// Sync mode = (3) 30/32 sync word bits detected
// Format of RX/TX data = (0) Normal mode, use FIFOs for RX and TX
// CRC operation = (1) CRC calculation in TX and CRC check in RX enabled
// Forward Error Correction = (0) FEC disabled
// Length configuration = (1) Variable length packets, packet length configured by the first received byte after sync word.
// Packetlength = 255
// Preamble count = (2)  4 bytes
// Append status = 1
// Address check = (0) No address check
// FIFO autoflush = 0
// Device address = 0
// GDO0 signal selection = ( 6) Asserts when sync word has been sent / received, and de-asserts at the end of the packet
// GDO2 signal selection = (11) Serial Clock


void spi_WriteReg(u8 addr, u8 data)
{ 
  	u16 overtime;
    
    SPI1_CSL;
    
	//send addr
	overtime = 0;
    while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && MISO_Status != Bit_RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
    SPI_I2S_SendData16(SPI1, addr);

	//send data
	overtime = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
	SPI_I2S_SendData16(SPI1, data);
    
      Delay_us(15);
      SPI1_CSH;
}

void spi_WriteBurstReg(u8 addr, u8 *buffer, u8 length)
{
	u16 i, overtime;
    
  	SPI1_CSL;
    
	//send addr
    addr += CC2500_WRITE_BURST;
	overtime = 0;
    while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && MISO_Status != Bit_RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
		
	SPI_I2S_SendData16(SPI1, addr);
    
    //send data
	for (i = 0; i < length; i++)
	{
        overtime = 0;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        {
            if(overtime < 5000)
                overtime++;
            else
                break;   
        }
        SPI_I2S_SendData16(SPI1,buffer[i]);
	}
    
    if(length < 3)
        Delay_us(10);
    else if(length < 24)
        Delay_us(12);
    else
        Delay_us(length);
    
      SPI1_CSH;    
}

u8 spi_ReadReg(u8 addr)
{
  	u16 temp, overtime;
    
    SPI1_CSL;
  
    //send addr
    addr |= CC2500_READ_SINGLE;
    overtime = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET && MISO_Status != Bit_RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }        
    SPI_I2S_SendData16(SPI1, addr);
    
    overtime = 0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
    temp = SPI_I2S_ReceiveData16(SPI1);
    
	//send 0 to receive
    overtime = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }        
    SPI_I2S_SendData16(SPI1, 0x00);
    
    //then start receive
    overtime = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }        
    temp = SPI_I2S_ReceiveData16(SPI1);
    
    Delay_us(10);
    SPI1_CSH;
    
    return temp;
}

void spi_ReadBurstReg(u8 addr, u8 *buffer, u8 length)
{
	u16 i, overtime;
    
  	SPI1_CSL;
    
	//send addr
    addr |= CC2500_READ_BURST;
    overtime = 0;
	while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && MISO_Status != Bit_RESET)    
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
	SPI_I2S_SendData16(SPI1, addr);
	
    //burst receive
    for (i = 0; i < length; i++)
	{
        //send 0 to receive
        overtime = 0;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
        {
            if(overtime < 5000)
                overtime++;
            else
                break;   
        }
        SPI_I2S_SendData16(SPI1, 0);
        
        //then start receive
        overtime = 0;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if(overtime < 5000)
                overtime++;
            else
                break;   
        }
        buffer[i] = SPI_I2S_ReceiveData16(SPI1);
	}
    
    if(length < 3)
        Delay_us(10);
    else if(length < 24)
        Delay_us(12);
    else
        Delay_us(length);
    
      SPI1_CSH;
}

u8 spi_ReadStatus(u8 addr)
{    
  	u16 overtime;
    
    SPI1_CSL;
    
    //send addr
    addr |= CC2500_READ_BURST;
    overtime = 0;
	while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && MISO_Status != Bit_RESET)    
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
	SPI_I2S_SendData16(SPI1, addr);
	
	//send 0 to receive
    overtime = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)    
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
	SPI_I2S_SendData16(SPI1, 0x00);
    
    //then start receive
    overtime = 0;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)    
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
    
      Delay_us(10);
      SPI1_CSH;
    
	return SPI_I2S_ReceiveData16(SPI1);
}

void spi_Strobe(u8 strobe)
{
    u16 overtime;
    
    SPI1_CSL;
    
    overtime = 0;
    while ((SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) && MISO_Status != Bit_RESET)
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
    SPI_I2S_SendData16(SPI1, strobe); 

    Delay_us(10);
    SPI1_CSH;    
}


void RF_Set(void)
{
    // 寄存器设置
	spi_WriteReg(CC2500_IOCFG2,   0x06); // GDO2 ???????
	spi_WriteReg(CC2500_IOCFG0,   0x06); // GDO0 ???????
	spi_WriteReg(CC2500_PKTLEN,   0x0D); // ?????
	spi_WriteReg(CC2500_PKTCTRL1, 0x0C); // ???????
	spi_WriteReg(CC2500_PKTCTRL0, 0x04); // ???????
	spi_WriteReg(CC2500_ADDR,     0x00); // ????
	spi_WriteReg(CC2500_CHANNR,   0x00); // ??
	spi_WriteReg(CC2500_FSCTRL1,  0x07); // synthesizer????
	spi_WriteReg(CC2500_FSCTRL0,  0x00); // synthesizer????
	spi_WriteReg(CC2500_FREQ2,    0x5D); // ?????, high byte
	spi_WriteReg(CC2500_FREQ1,    0x93); // ?????, middle byte.
	spi_WriteReg(CC2500_FREQ0,    0xB1); // ?????, low byte.
	spi_WriteReg(CC2500_MDMCFG4,  0x2D); // 2D
	spi_WriteReg(CC2500_MDMCFG3,  0x3B); // 3B
	spi_WriteReg(CC2500_MDMCFG2,  0x73); // Modem?????
	spi_WriteReg(CC2500_MDMCFG1,  0xA2); // Modem?????
	spi_WriteReg(CC2500_MDMCFG0,  0xF8); // Modem?????
	spi_WriteReg(CC2500_DEVIATN,  0x01); // Modem dev (when FSK mod en)
	spi_WriteReg(CC2500_MCSM0 ,   0x18); // MainRadio Cntrl State Machine
	spi_WriteReg(CC2500_FOCCFG,   0x1D); // Freq Offset Compens. Config
	spi_WriteReg(CC2500_BSCFG,    0x1C); // Bit synchronization config.
	spi_WriteReg(CC2500_AGCCTRL2, 0xC7); // AGC control.
	spi_WriteReg(CC2500_AGCCTRL1, 0x00); // AGC control.
	spi_WriteReg(CC2500_AGCCTRL0, 0xB0); // AGC control.
	spi_WriteReg(CC2500_FREND1,   0xB6); // Front end RX configuration.
	spi_WriteReg(CC2500_FREND0,   0x10); // Front end RX configuration.
	spi_WriteReg(CC2500_FSCAL3,   0xEA); // Frequency synthesizer cal.
	spi_WriteReg(CC2500_FSCAL2,   0x0A); // Frequency synthesizer cal.
	spi_WriteReg(CC2500_FSCAL1,   0x00); // Frequency synthesizer cal.
	spi_WriteReg(CC2500_FSCAL0,   0x11); // Frequency synthesizer cal.
	spi_WriteReg(CC2500_FSTEST,   0x59); // Frequency synthesizer cal.
	spi_WriteReg(CC2500_TEST2,    0x88); // Various test settings.
	spi_WriteReg(CC2500_TEST1,    0x31); // Various test settings.
	spi_WriteReg(CC2500_TEST0,    0x0B); // Various test settings.
}

//RF初始化
void CC2500_Init(void)
{    
    spi_Strobe(CC2500_SRES); 
    Delay_us(50);
    RF_Set();
    spi_WriteBurstReg(CC2500_PATABLE, paTable, paTableLen);
    spi_Strobe(CC2500_SIDLE);
   
    if(spi_ReadReg(CC2500_IOCFG2) != 0xFF && spi_ReadStatus(CC2500_PKTLEN) != 0xFF)
        Link_mode = 1;  
}
//发送数据包
u8 RF_Send(u8 *txBuffer, u8 length, u8 HOP_ENABLE)
{
    u16 overtime;
    
    RX_ENABLE = 0;
    
    spi_Strobe(CC2500_SIDLE);
    
    if(HOP_ENABLE)
    {
        switch(Channel)
        {
            case 0x00:  Channel = 0x7F; break;
            case 0x7F:  Channel = 0xFF; break;
            case 0xFF:  Channel = 0x00; break;
        }                    
        
        spi_WriteReg(CC2500_CHANNR, Channel);
    }
  
    spi_WriteBurstReg(CC2500_TXFIFO, txBuffer, length); // 写入数据
    spi_Strobe(CC2500_STX);         // 选择发送模式

    while (GD0_Status == Bit_RESET) // 等待 GDO0 变高 -> sync TX'ed
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }       
    while (GD0_Status != Bit_RESET) // 等待 GDO0 清零 -> end of pkt
    {
        if(overtime < 5000)
            overtime++;
        else
            break;   
    }
    
    Delay_us(30);
    spi_Strobe(CC2500_SIDLE);	// Exit RX / TX
    spi_Strobe(CC2500_SFRX);	// Flush the RX FIFO buffer.
    spi_Strobe(CC2500_SRX);		// Enable RX.
    RX_ENABLE = 1;
    
    return 1;
}

//接收数据包
u8 RF_Receive(u8 *rxBuffer,  u8 *length)
{
	u8 status[2];
	u8 pktLen;

	if ((spi_ReadStatus(CC2500_RXBYTES) & CC2500_NUM_RXBYTES))
	{
		pktLen = spi_ReadReg(CC2500_RXFIFO);    // 读取数据包长度

		if (pktLen <= *length)                  // 数据包长度小于缓冲区长度
		{
			spi_ReadBurstReg(CC2500_RXFIFO, rxBuffer, pktLen);  //弹出数据
			*length = pktLen;                                   // 返回长度
			spi_ReadBurstReg(CC2500_RXFIFO, status, 2);         // 读取状态字节			
			return (u8)(status[CC2500_LQI_RX]&CC2500_CRC_OK);   // 返回CRC校验位
		}                                       
		else//长度超出缓冲区
		{
			*length = pktLen;                     // 返回长度
			spi_Strobe(CC2500_SFRX);              // 清空 RXFIFO
			return 0;                             // 出错信息
		}
	}
	else
		return 0;                               // 报错
}





