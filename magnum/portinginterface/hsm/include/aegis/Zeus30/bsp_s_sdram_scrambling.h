/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/

#ifndef BSP_S_SDRAM_SCRAMBLING_ENABLE_H__
#define BSP_S_SDRAM_SCRAMBLING_ENABLE_H__



typedef enum BCMD_SdramScram_InCmdOperation_e
{
    BCMD_SdramScram_InCmdOperation_eReserved_5_0  =  (5<<2)+0,
  BCMD_SdramScram_InCmdOperation_eMemcType = (5 << 2)+1,
  BCMD_SdramScram_InCmdOperation_eSubCmd = (5 << 2)+2,
  BCMD_SdramScram_InCmdOperation_eBootType = (5 << 2) + 3,
  BCMD_SdramScram_InCmdOperation_eMax = (22 << 2) + 0

} BCMD_SdramScram_InCmdOperation_e;

typedef enum BCMD_SdramScram_OutCmdOperation_e
{
    BCMD_SdramScram_OutCmdOperation_eStatus = (5 << 2)+3,
    BCMD_SdramScram_OutCmdOperation_eKeyLoadStatus = (6 << 2)+3,
    BCMD_SdramScram_OutCmdOperation_eMax
} BCMD_SdramScram_OutCmdOperation_e;



typedef enum BCMD_SdramScram_MemcType_e
{
	BCMD_SdramScram_MemcType_eAll = 0,
    BCMD_SdramScram_MemcType_eReserved1  =  1,
    BCMD_SdramScram_MemcType_eReserved2  =  2,
    BCMD_SdramScram_MemcType_eReserved3  =  3,
	BCMD_SdramScram_MemcType_eMax
} BCMD_SdramScram_MemcType_e;


typedef enum BCMD_SdramScram_SubCmd_e
{
	BCMD_SdramScram_SubCmd_eEnable = 0,
	BCMD_SdramScram_SubCmd_eDisable = 1,
	BCMD_SdramScram_SubCmd_eUseOneShotForDisable = 2,
	BCMD_SdramScram_SubCmd_eMax
} BCMD_SdramScram_SubCmd_e;



typedef enum BCMD_SdramScram_BootType_e
{
  BCMD_SdramScram_BootType_eColdBootNewKey = 0,
  BCMD_SdramScram_BootType_eColdBootReuseKey = 1,
  BCMD_SdramScram_BootType_eWarmBoot = 2,
  BCMD_SdramScram_BootType_eMax

} BCMD_SdramScram_BootType_e;

typedef enum BCMD_SdramScram_KeyLoadStatus_e
{
  BCMD_SdramScram_KeyLoadStatus_eNewKey = 0,
  BCMD_SdramScram_KeyLoadStatus_eReusedKey = 1,
  BCMD_SdramScram_KeyLoadStatus_eForcedNewKey = 2,
  BCMD_SdramScram_KeyLoadStatus_eMax
} BCMD_SdramScram_KeyLoadStatus_e;


#endif
