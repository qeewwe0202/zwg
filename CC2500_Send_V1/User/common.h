/******************************************************************************

                  ��Ȩ���� (C), 2012-2022, FreeTeam

 ******************************************************************************
  �� �� ��   : common.h
  �� �� ��   : ����
  ��    ��   : tofu
  ��������   : 2013��7��6��
  ����޸�   :
  ��������   : ���ݽṹ����
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2013��7��6��
    ��    ��   : tofu
    �޸�����   : �����ļ�

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
    NoType,//sbд
    NRF905PacketSearchingChild,//���ڵ㷢�Ͳ�ѯ�ӽڵ�֡
    NRF905PacketReplySearching,//�ӽڵ�ӵ����ڵ�֡���ظ���ѯ
    NRF905PacketJoinedSucceed,//���ڵ�ظ��ӽڵ������ɹ�����ID
    NRF905PacketWorkState,//����֡
    NRF905PacketReplyWorkState,//�ظ�����֡
    NRF905PacketSendData,//�������ݰ�
    NRF905PacketReplyData, //�ظ����ݰ�(˵�����ݰ����ճɹ�)
    NRF905PacketLabelJoined // ����������ǩ�����ɹ�
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






