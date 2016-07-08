/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/


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
