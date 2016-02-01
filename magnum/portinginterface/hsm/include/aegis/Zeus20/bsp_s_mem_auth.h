/***************************************************************************
 *     Copyright (c) 2005-2014, Broadcom Corporation
 *     All Rights Reserved
 *     Highly Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 ***************************************************************************/


#ifndef BSP_S_MEM_AUTH_H__
#define BSP_S_MEM_AUTH_H__

typedef enum BCMD_MemAuth_InCmdField_e
{
    BCMD_MemAuth_InCmdField_eRegionOp       = (5<<2) + 3,
    BCMD_MemAuth_InCmdField_eRegionNum      = (6<<2) + 3,
    BCMD_MemAuth_InCmdField_eStartAddr      = (7<<2) + 0,
    BCMD_MemAuth_InCmdField_eEndAddr        = (8<<2) + 0,
    BCMD_MemAuth_InCmdField_eSigStartAddr   = (9<<2) + 0,
    BCMD_MemAuth_InCmdField_eSigEndAddr     = (10<<2) + 0,
    BCMD_MemAuth_InCmdField_eIntervalCheckBw = (11<<2) + 3,
    BCMD_MemAuth_InCmdField_eScbBurstSize   = (11<<2) + 2,
    BCMD_MemAuth_InCmdField_eReserved0      = (11<<2) + 1,
    BCMD_MemAuth_InCmdField_eResetOnVerifyFailure = (11<<2) + 0,
    BCMD_MemAuth_InCmdField_eRsaKeyId       = (12<<2) + 3,
    BCMD_MemAuth_InCmdField_eCodeRules      = (13<<2) + 0,
    BCMD_MemAuth_InCmdField_eNoRelocatableCode = (13<<2) + 3,
    BCMD_MemAuth_InCmdField_eCpuType        = (13<<2) + 2,
    BCMD_MemAuth_InCmdField_eReserved2      = (13<<2) + 1,
    BCMD_MemAuth_InCmdField_eReserved1      = (13<<2) + 0,
    BCMD_MemAuth_InCmdField_eMarketID       = (14<<2) + 0,
    BCMD_MemAuth_InCmdField_eMarketIDMask   = (15<<2) + 0,
    BCMD_MemAuth_InCmdField_eReservedE1     = (16<<2) + 0,
    BCMD_MemAuth_InCmdField_eReservedE2     = (16<<2) + 1,
    BCMD_MemAuth_InCmdField_eEpochMask      = (16<<2) + 2,
    BCMD_MemAuth_InCmdField_eEpoch          = (16<<2) + 3,
    BCMD_MemAuth_InCmdField_eMax

} BCMD_MemAuth_InCmdField_e;

typedef enum BCMD_MemAuth_OutCmdField_e
{
    BCMD_MemAuth_OutCmdField_eStatus        = (5<<2) + 3,
    BCMD_MemAuth_OutCmdField_eRegionNum     = (6<<2) + 3,
    BCMD_MemAuth_OutCmdField_eRegionPauseStatus = (6<<2) + 3,
    BCMD_MemAuth_OutCmdField_eRegion0Status = (7<<2) + 3,
    BCMD_MemAuth_OutCmdField_eMax

} BCMD_MemAuth_OutCmdField_e;

typedef enum BCMD_MemAuth_Operation_e
{
    BCMD_MemAuth_Operation_eDisableRegion = 0,
    BCMD_MemAuth_Operation_eEnableRegion = 1,
    BCMD_MemAuth_Operation_eQueryRegionInfo = 2,
    BCMD_MemAuth_Operation_ePause = 3,
    BCMD_MemAuth_Operation_eRegionVerified = 4,
    BCMD_MemAuth_Operation_eRegionVerifPaused = 5,
    BCMD_MemAuth_Operation_eReserved1 = 6,
    BCMD_MemAuth_Operation_eDefineRegion = 7,
    BCMD_MemAuth_Operation_eMichDisable = 8,
    BCMD_MemAuth_Operation_eHideMipsBootRom = 9,

    BCMD_MemAuth_Operation_eReserved10 = 10,
    BCMD_MemAuth_Operation_eMax

} BCMD_MemAuth_Operation_e;

typedef enum BCMD_ScbBurstSize_e
{
    BCMD_ScbBurstSize_e64    = 0,
    BCMD_ScbBurstSize_e128   = 1,
    BCMD_ScbBurstSize_e256   = 2,
    BCMD_ScbBurstSize_eMax
} BCMD_ScbBurstSize_e;

typedef enum BCMD_MemAuth_CpuType_e
{
    BCMD_MemAuth_CpuType_eMips   = 0,
    BCMD_MemAuth_CpuType_eRaaga  = 1,
    BCMD_MemAuth_CpuType_eAvd    = 2,
    BCMD_MemAuth_CpuType_eRave   = 3,
    BCMD_MemAuth_CpuType_eSvd    = 4,
    BCMD_MemAuth_CpuType_eVice   = 5,
    BCMD_MemAuth_CpuType_eSid    = 6,
    BCMD_MemAuth_CpuType_eMax

} BCMD_MemAuth_CpuType_e;

typedef enum BCMD_MemAuth_ResetOnVerifyFailure_e
{
    BCMD_MemAuth_ResetOnVerifyFailure_eNoReset = 0x0,
    BCMD_MemAuth_ResetOnVerifyFailure_eReset   = 0x5,
    BCMD_MemAuth_ResetOnVerifyFailure_eMax

} BCMD_MemAuth_ResetOnVerifyFailure_e;


#endif
