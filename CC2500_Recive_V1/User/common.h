/******************************************************************************

                  版权所有 (C), 2012-2022, FreeTeam

 ******************************************************************************
  文 件 名   : common.h
  版 本 号   : 初稿
  作    者   : tofu
  生成日期   : 2013年7月6日
  最近修改   :
  功能描述   : 数据结构定义
  函数列表   :
  修改历史   :
  1.日    期   : 2013年7月6日
    作    者   : tofu
    修改内容   : 创建文件

******************************************************************************/
#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f10x.h"

#define MAXPACKETSIZE   32
#define MAXROUTERDEEP   3
#define MAXLABELNUM     15

typedef enum NRF905PACKETTYPE
{
    NoType,//sb写
    NRF905PacketSearchingChild,//父节点发送查询子节点帧
    NRF905PacketReplySearching,//子节点接到父节点帧，回复查询
    NRF905PacketJoinedSucceed,//父节点回复子节点入网成功，定ID
    NRF905PacketWorkState,//心跳帧
    NRF905PacketReplyWorkState,//回复心跳帧
    NRF905PacketSendData,//发送数据包
    NRF905PacketReplyData, //回复数据包(说明数据包接收成功)
    NRF905PacketLabelJoined // 反馈主机标签入网成功
}NRF905PACKETTYPE_ENUM;

typedef enum CC2500PACKETTYPE
{
    CC2500NoType,
    CC2500PacketSearchingLabel,
    CC2500PacketReplySearching,
    CC2500PacketJoinedSucceed,
    CC2500PacketWorkState,
    CC2500PacketReplyWorkState,
    CC2500PacketSendData,
    CC2500PacketReplyData
}CC2500PACKETTYPE_ENUM;

typedef enum ROUTER_WORKSTATE
{
    idle,
    busy
}ROUTER_WORKSTATE_ENUM;

typedef enum LABEL_WORKSTATE
{
    Idle,
    busy1
}LABEL_WORKSTATE_ENUM;

typedef struct ROUTER
{
    u8 FatherAddr[4];           //Current Father Node
    u8 FatherAddrBackup[4];     //Used when cannot connect Current Father Node
    u8 MyAddr[4];               //Network address
    u8 ChildNum;                //Child Node Number
}ROUTER_STRU;

typedef struct LABEL
{
    u8 FatherAddr[4];           //Current Father Node
    u8 FatherAddrBackup[4];     //used when cannot connect Current Father Node
    u8 MyAddr[4];               //Network address
}LABEL_STRU;

extern u8 gCC2500RxBuf[MAXPACKETSIZE];
extern u8 gCC2500TxBuf[MAXPACKETSIZE];

extern u8 gNRF905RxBuf[MAXPACKETSIZE];
extern u8 gNRF905TxBuf[MAXPACKETSIZE];

extern u8 gUartRxBuf[MAXPACKETSIZE];
extern u8 gUartTxBuf[MAXPACKETSIZE];

extern ROUTER_WORKSTATE_ENUM ROUTER_WORKSTATE;
extern LABEL_WORKSTATE_ENUM LABEL_WORKSTATE;

extern ROUTER_STRU ROUTER;
extern LABEL_STRU LABEL;

extern NRF905PACKETTYPE_ENUM NRF905PACKETTYPE;
extern CC2500PACKETTYPE_ENUM CC2500PACKETTYPE;

extern u8 gRouterNum;
extern u8 gLabelNum;
extern volatile ErrorStatus gFlag;
extern u8 count;

extern void ClearDataBuf(u8 *pBuf, u8 Len);
extern void delay_nus(u32 Nus);
extern void delay_nms(u16 nms);
extern void delay_init(u8 SYSCLK);
extern void delay_ns(u16 ns);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H__ */






