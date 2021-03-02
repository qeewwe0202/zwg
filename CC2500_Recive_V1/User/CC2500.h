/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : CC2500.h
  版 本 号   : 初稿
  作    者   : tofu
  生成日期   : 2013年7月6日
  最近修改   :
  功能描述   : CC2500.c HeaderFile
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月6日
    作    者   : tofu
    修改内容   : 创建文件

******************************************************************************/
#ifndef __CC2500_H__
#define __CC2500_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"

typedef const struct S_RF_SETTINGS{
    u8 sIOCFG2;    // GDO2 output pin configuration
    u8 sIOCFG1;    // 
    u8 sIOCFG0;    // GDO0 output pin configuration
    u8 sFIFOTHR;   // RXFIFO and TXFIFO thresholds.
    u8 sSYNC1;     // Sync word, high byte 
    u8 sSYNC0;     // Sync word, low byte 
    u8 sPKTLEN;    // Packet length.
    u8 sPKTCTRL1;  // Packet automation control.
    u8 sPKTCTRL0;  // Packet automation control.
    u8 sADDR;      // Device address.
    u8 sCHANNR;    // Channel number.    
    u8 sFSCTRL1;   // Frequency synthesizer control.
    u8 sFSCTRL0;   // Frequency synthesizer control.    
    u8 sFREQ2;     // Frequency control word, high byte.
    u8 sFREQ1;     // Frequency control word, middle byte.
    u8 sFREQ0;     // Frequency control word, low byte.
    u8 sMDMCFG4;   // Modem configuration.
    u8 sMDMCFG3;   // Modem configuration.
    u8 sMDMCFG2;   // Modem configuration.
    u8 sMDMCFG1;   // Modem configuration.
    u8 sMDMCFG0;   // Modem configuration.    
    u8 sDEVIATN;   // Modem deviation setting (when FSK modulation is enabled).
    u8 sMCSM2;     // Main Radio Control State Machine configuration 
    u8 sMCSM1;     // Main Radio Control State Machine configuration
    u8 sMCSM0;     // Main Radio Control State Machine configuration.
    u8 sFOCCFG;    // Frequency Offset Compensation configuration 
    u8 sBSCFG;     // Bit Synchronization configuration 
    u8 sAGCCTRL2;  // AGC control.
	u8 sAGCCTRL1;  // AGC control.
    u8 sAGCCTRL0;  // AGC control.
    u8 sWOREVT1;   // High byte Event 0 timeout 
    u8 sWOREVT0;   // Low byte Event 0 timeout 
    u8 sWORCTRL;   // Wake On Radio control  
    u8 sFREND1;    // Front end RX configuration.
    u8 sFREND0;    // Front end TX configuration.    
    u8 sFSCAL3;    // Frequency synthesizer calibration.
    u8 sFSCAL2;    // Frequency synthesizer calibration.
	u8 sFSCAL1;    // Frequency synthesizer calibration.
    u8 sFSCAL0;    // Frequency synthesizer calibration.
    u8 sRCCTRL1;   // RC oscillator configuration 
    u8 sRCCTRL2;   // RC oscillator configuration 
    u8 sFSTEST;    // Frequency synthesizer calibration control
    u8 sPTEST;     // Production test 
    u8 sAGCTEST;   // AGC test 
    u8 sTEST2;     // Various test settings.
    u8 sTEST1;     // Various test settings.
    u8 sTEST0;     // Various test settings.    
} RF_SETTINGS;

extern RF_SETTINGS RFSettings;

extern void CC2500_Init(void);
extern void CC2500_SetPA(void);

extern void CC2500_Disable_Interrupt(void);
extern void CC2500_Enable_Interrupt(void);

extern void CC2500_WriteStrobe(u8 Strobe);

extern void CC2500_ModeTx(void);
extern void CC2500_ModeRx(void);
extern void CC2500_ModeIdle(void);
extern void CC2500_ModePWRdown(void);

extern void CC2500_WaitTX_Done(void);

extern void CC2500_ClrRXFIFO(void);
extern void CC2500_ClrTXFIFO(void);

extern void CC2500_WriteByte(u8 Addr,u8 data);
extern void CC2500_BustWrite(u8 Addr,u8 * pData,u8 len);
extern void CC2500_WriteSettings(RF_SETTINGS * pRfSettings);

extern u8   CC2500_ReadReg(u8 Addr);
extern u8   CC2500_ReadStatus(void);
extern void CC2500_BurstRead(u8 Addr,u8 *pBuf,u8 Len);

extern ErrorStatus CC2500_RevPacket(void);
extern void CC2500_SendPacket(void);

#ifdef __cplusplus
}
#endif

#endif /* __CC2500_H__ */

