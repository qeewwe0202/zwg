#ifndef __CC2500_H__
#define __CC2500_H__

#include "stm32f0xx.h"
#include "system.h"

#define MISO_Status     GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)
#define GD0_Status      GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8)

#define SPI1_CSL        GPIO_ResetBits(GPIOA, GPIO_Pin_15)
#define SPI1_CSH        GPIO_SetBits(GPIOA, GPIO_Pin_15)


#define PAIR_RF_ACK     0xE1
#define PAIR_BT_ACK     0xE2
#define BT_STA_WORK     0xE3
#define BT_STA_PAIR     0xE4
#define BT_STA_IDLE     0xE5
#define BT_STA_OFF      0xE6
#define BT_STA_CHEK     0xE7

//CC2500 registers
#define CC2500_IOCFG2       0x00        // GDO2 Pin Setting
#define CC2500_IOCFG1       0x01        // GDO1 Pin Setting
#define CC2500_IOCFG0       0x02        // GDO0 Pin Setting
#define CC2500_FIFOTHR      0x03        // RX FIFO & TX FIFO (share)
#define CC2500_SYNC1        0x04        // Sync High byte
#define CC2500_SYNC0        0x05        // Sync Low byte
#define CC2500_PKTLEN       0x06        // Packet length
#define CC2500_PKTCTRL1     0x07        // Packet automation control
#define CC2500_PKTCTRL0     0x08        // Packet automation control
#define CC2500_ADDR         0x09        // Device address
#define CC2500_CHANNR       0x0A        // Channel number
#define CC2500_FSCTRL1      0x0B        // Frequency synthesizer control
#define CC2500_FSCTRL0      0x0C        // Frequency synthesizer control
#define CC2500_FREQ2        0x0D        // Frequency control word, high byte
#define CC2500_FREQ1        0x0E        // Frequency control word, middle byte
#define CC2500_FREQ0        0x0F        // Frequency control word, low byte
#define CC2500_MDMCFG4      0x10        // Modem configuration
#define CC2500_MDMCFG3      0x11        // Modem configuration
#define CC2500_MDMCFG2      0x12        // Modem configuration
#define CC2500_MDMCFG1      0x13        // Modem configuration
#define CC2500_MDMCFG0      0x14        // Modem configuration
#define CC2500_DEVIATN      0x15        // Modem deviation setting
#define CC2500_MCSM2        0x16        // Main Radio Cntrl State Machine config
#define CC2500_MCSM1        0x17        // Main Radio Cntrl State Machine config
#define CC2500_MCSM0        0x18        // Main Radio Cntrl State Machine config
#define CC2500_FOCCFG       0x19        // Frequency Offset Compensation config
#define CC2500_BSCFG        0x1A        // Bit Synchronization configuration
#define CC2500_AGCCTRL2     0x1B        // AGC control
#define CC2500_AGCCTRL1     0x1C        // AGC control
#define CC2500_AGCCTRL0     0x1D        // AGC control
#define CC2500_WOREVT1      0x1E        // High byte Event 0 timeout
#define CC2500_WOREVT0      0x1F        // Low byte Event 0 timeout
#define CC2500_WORCTRL      0x20        // Wake On Radio control
#define CC2500_FREND1       0x21        // Front end RX configuration
#define CC2500_FREND0       0x22        // Front end TX configuration
#define CC2500_FSCAL3       0x23        // Frequency synthesizer calibration
#define CC2500_FSCAL2       0x24        // Frequency synthesizer calibration
#define CC2500_FSCAL1       0x25        // Frequency synthesizer calibration
#define CC2500_FSCAL0       0x26        // Frequency synthesizer calibration
#define CC2500_RCCTRL1      0x27        // RC oscillator configuration
#define CC2500_RCCTRL0      0x28        // RC oscillator configuration
#define CC2500_FSTEST       0x29        // Frequency synthesizer cal control
#define CC2500_PTEST        0x2A        // Production test
#define CC2500_AGCTEST      0x2B        // AGC test
#define CC2500_TEST2        0x2C        // Various test settings
#define CC2500_TEST1        0x2D        // Various test settings
#define CC2500_TEST0        0x2E        // Various test settings

// Strobe commands
#define CC2500_SRES         0x30        // Reset chip.
#define CC2500_SFSTXON      0x31        // Enable/calibrate freq synthesizer
#define CC2500_SXOFF        0x32        // Turn off crystal oscillator.
#define CC2500_SCAL         0x33        // Calibrate freq synthesizer & disable
#define CC2500_SRX          0x34        // Enable RX.
#define CC2500_STX          0x35        // Enable TX.
#define CC2500_SIDLE        0x36        // Exit RX / TX
#define CC2500_SAFC         0x37        // AFC adjustment of freq synthesizer
#define CC2500_SWOR         0x38        // Start automatic RX polling sequence
#define CC2500_SPWD         0x39        // Enter pwr down mode when CSn goes hi
#define CC2500_SFRX         0x3A        // Flush the RX FIFO buffer.
#define CC2500_SFTX         0x3B        // Flush the TX FIFO buffer.
#define CC2500_SWORRST      0x3C        // Reset real time clock.
#define CC2500_SNOP         0x3D        // No operation.

// Status registers
#define CC2500_PARTNUM      0x30        // Part number
#define CC2500_VERSION      0x31        // Current version number
#define CC2500_FREQEST      0x32        // Frequency offset estimate
#define CC2500_LQI          0x33        // Demodulator estimate for link quality
#define CC2500_RSSI         0x34        // Received signal strength indication
#define CC2500_MARCSTATE    0x35        // Control state machine state
#define CC2500_WORTIME1     0x36        // High byte of WOR timer
#define CC2500_WORTIME0     0x37        // Low byte of WOR timer
#define CC2500_PKTSTATUS    0x38        // Current GDOx status and packet status
#define CC2500_VCO_VC_DAC   0x39        // Current setting from PLL cal module
#define CC2500_TXBYTES      0x3A        // Underflow and # of bytes in TXFIFO
#define CC2500_RXBYTES      0x3B        // Overflow and # of bytes in RXFIFO
#define CC2500_NUM_RXBYTES  0x7F        // Mask "# of bytes" field in _RXBYTES

// Other memory locations
#define CC2500_PATABLE      0x3E
#define CC2500_TXFIFO       0x3F
#define CC2500_RXFIFO       0x3F

// Masks for appended status bytes
#define CC2500_LQI_RX       0x01        // Position of LQI byte
#define CC2500_CRC_OK       0x80        // Mask "CRC_OK" bit within LQI byte

// Definitions to support burst/single access:
#define CC2500_WRITE_BURST  0x40
#define CC2500_READ_SINGLE  0x80
#define CC2500_READ_BURST   0xC0

void CC2500_Init(void);
void spi_Strobe(u8 strobe);
void spi_WriteReg(u8 addr, u8 data);
u8 spi_ReadReg(u8 addr);
u8 spi_ReadStatus(u8 addr);
void spi_ReadBurstReg(u8 addr, u8 *buffer, u8 length);
u8 RF_Send(u8 *txBuffer, u8 length);
u8 RF_Receive(u8 *rxBuffer,  u8 *length);
void Hopping(void);

extern u8 paTable[8];
extern u8 paTableLen;
extern u8 RF_RX_ENABLE;
#endif
