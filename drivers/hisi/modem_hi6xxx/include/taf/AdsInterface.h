/******************************************************************************

                  版权所有 (C), 2001-2011, 华为技术有限公司

 ******************************************************************************
  文 件 名   : AdsInterface.h
  版 本 号   : 初稿
  作    者   : 鲁琳/l60609
  生成日期   : 2011年12月6日
  最近修改   :
  功能描述   : ADS对外提供的接口
  函数列表   :
  修改历史   :
  1.日    期   : 2011年12月6日
    作    者   : 鲁琳/l60609
    修改内容   : 创建文件

******************************************************************************/

#ifndef __ADSINTERFACE_H__
#define __ADSINTERFACE_H__

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(4)


/*****************************************************************************
  2 宏定义
*****************************************************************************/


/*****************************************************************************
  3 枚举定义
*****************************************************************************/

/*****************************************************************************
枚举名    : ADS_MSG_ID_ENUM
枚举说明  : ADS消息ID

  1.日    期   : 2011年12月6日
    作    者   : 鲁琳/l60609
    修改内容   : 定义ADS消息ID
*****************************************************************************/
enum ADS_MSG_ID_ENUM
{
    /* PDP状态消息*/
    ID_APS_ADS_PDP_STATUS_IND,                                                  /* _H2ASN_MsgChoice ADS_PDP_STATUS_IND_STRU */

    /* CCPU复位前给ADS的消息指示 */
    ID_ADS_CCPU_RESET_START_IND         = 0x1001,                               /* _H2ASN_MsgChoice ADS_CCPU_RESET_IND_STRU */

    /* CCPU复位后给ADS的消息指示 */
    ID_ADS_CCPU_RESET_END_IND           = 0x1002,                               /* _H2ASN_MsgChoice ADS_CCPU_RESET_IND_STRU */

    ID_ADS_LOG_RPT_STATS_INFO           = 0x1103,                               /* _H2ASN_MsgChoice ADS_LOG_HEAD_STRU */

    ID_ADS_MSG_ID_ENUM_BUTT
};
typedef VOS_UINT32  ADS_MSG_ID_ENUM_UINT32;

/*****************************************************************************
 枚举名    : ADS_PDP_STATUS_ENUM
 枚举说明  : 发给ADS的PDP激活与去激活指示

  1.日    期   : 2011年12月6日
    作    者   : 鲁琳/l60609
    修改内容   : 定义PDP的状态
  2.日    期   : 2012年2月17日
    作    者   : L47619
    修改内容   : V7R1C50项目，新增PDP修改成功的枚举
*****************************************************************************/
enum ADS_PDP_STATUS_ENUM
{
    ADS_PDP_STATUS_ACT,                                                         /* PDP激活成功 */
    ADS_PDP_STATUS_MODIFY,                                                      /* PDP修改成功 */
    ADS_PDP_STATUS_DEACT,                                                       /* PDP去激活成功 */
    ADS_PDP_STATUS_BUTT
};
typedef VOS_UINT8 ADS_PDP_STATUS_ENUM_UINT8;

/*****************************************************************************
枚举名    : ADS_QCI_TYPE_ENUM
枚举说明  : ADS发送PDP状态指示时所使用的QCI枚举

  1.日    期   : 2012年2月23日
    作    者   : L47619
    修改内容   : V7R1C50 IPC项目新增
*****************************************************************************/
typedef enum
{
    ADS_QCI_TYPE_QCI1_GBR               = 0,
    ADS_QCI_TYPE_QCI2_GBR               = 1,
    ADS_QCI_TYPE_QCI3_GBR               = 2,
    ADS_QCI_TYPE_QCI4_GBR               = 3,
    ADS_QCI_TYPE_QCI5_NONGBR            = 4,
    ADS_QCI_TYPE_QCI6_NONGBR            = 5,
    ADS_QCI_TYPE_QCI7_NONGBR            = 6,
    ADS_QCI_TYPE_QCI8_NONGBR            = 7,
    ADS_QCI_TYPE_QCI9_NONGBR            = 8,

    ADS_QCI_TYPE_BUTT
}ADS_QCI_TYPE_ENUM;

typedef VOS_UINT8 ADS_QCI_TYPE_ENUM_UINT8;


typedef enum
{
    ADS_PDP_IPV4                        = 0x01,                                 /* IPV4类型 */
    ADS_PDP_IPV6                        = 0x02,                                 /* IPV6类型 */
    ADS_PDP_IPV4V6                      = 0x03,                                 /* IPV4V6类型 */
    ADS_PDP_PPP                         = 0x04,                                 /* PPP类型 */

    ADS_PDP_TYPE_BUTT                   = 0xFF
}ADS_PDP_TYPE_ENUM;

typedef VOS_UINT8 ADS_PDP_TYPE_ENUM_UINT8;

/*****************************************************************************
  4 全局变量声明
*****************************************************************************/


/*****************************************************************************
  5 消息头定义
*****************************************************************************/


/*****************************************************************************
  6 消息定义
*****************************************************************************/


/*****************************************************************************
  7 STRUCT定义
*****************************************************************************/

typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    ADS_MSG_ID_ENUM_UINT32              enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
    /* Added by l60609 for DSDA PhaseII, 2012-12-13, begin */
    MODEM_ID_ENUM_UINT16                enModemId;                              /* Modem Id*/
    /* Added by l60609 for DSDA PhaseII, 2012-12-13, end */
    VOS_UINT8                           ucRabId;                                /* Rab Id*/
    ADS_PDP_STATUS_ENUM_UINT8           enPdpStatus;                            /* PDP状态*/
    ADS_QCI_TYPE_ENUM_UINT8             enQciType;                              /* QCI */
    ADS_PDP_TYPE_ENUM_UINT8             enPdpType;                              /* PDP类型 */
    VOS_UINT8                           aucRsv[2];
}ADS_PDP_STATUS_IND_STRU;
typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    ADS_MSG_ID_ENUM_UINT32              enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
}ADS_CCPU_RESET_IND_STRU;

/*****************************************************************************
结构名    : ADS_LOG_HEAD_STRU
结构说明  : 勾log消息

  1.日    期   : 2014年02月11日
    作    者   : l60609
    修改内容   : 创建
*****************************************************************************/
typedef struct
{
    VOS_MSG_HEADER                                                              /* 消息头 */    /* _H2ASN_Skip */
    ADS_MSG_ID_ENUM_UINT32              enMsgId;                                /* 消息ID */    /* _H2ASN_Skip */
}ADS_LOG_HEAD_STRU;

/*****************************************************************************
  8 UNION定义
*****************************************************************************/


/*****************************************************************************
  9 OTHERS定义
*****************************************************************************/
/*****************************************************************************
  H2ASN顶级消息结构定义
*****************************************************************************/
typedef struct
{
    ADS_MSG_ID_ENUM_UINT32              enMsgId;            /* _H2ASN_MsgChoice_Export ADS_MSG_ID_ENUM_UINT32 */
    VOS_UINT8                           aucMsg[4];
    /***************************************************************************
        _H2ASN_MsgChoice_When_Comment          ADS_MSG_ID_ENUM_UINT32
    ****************************************************************************/
} ADS_INTERFACE_MSG_DATA;
/* _H2ASN_Length UINT32 */

typedef struct
{
    VOS_MSG_HEADER
    ADS_INTERFACE_MSG_DATA              stMsgData;
} AdsInterface_MSG;

/*****************************************************************************
  10 函数声明
*****************************************************************************/






#if (VOS_OS_VER == VOS_WIN32)
#pragma pack()
#else
#pragma pack(0)
#endif


#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif

#endif

