/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.

 ******************************************************************************/

#ifndef FLASHMAP_VERSION
#warning "FLASHMAP_VERSION is not defined under bsp_s_mem_auth.h"
#endif


#ifndef BSP_S_MEM_AUTH_H__
#define BSP_S_MEM_AUTH_H__


typedef enum BCMD_MemAuth_InCmdField_e
{
    BCMD_MemAuth_InCmdField_eRegionOp = (5 << 2) + 3,
#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V5)
    BCMD_MemAuth_InCmdField_eMVIR = (6 << 2) + 0,
    BCMD_MemAuth_InCmdField_eWDIR = (6 << 2) + 1,
#endif
    BCMD_MemAuth_InCmdField_eDisableMichPattern = (6 << 2) + 0,
    BCMD_MemAuth_InCmdField_eRegionNum = (6 << 2) + 3,
    BCMD_MemAuth_InCmdField_eStartAddrMsb = (7 << 2) + 0,
    BCMD_MemAuth_InCmdField_eStartAddr = (8 << 2) + 0,
    BCMD_MemAuth_InCmdField_eEndAddrMsb = (9 << 2) + 0,
    BCMD_MemAuth_InCmdField_eEndAddr = (10 << 2) + 0,
    BCMD_MemAuth_InCmdField_eSigStartAddrMsb = (11 << 2) + 0,
    BCMD_MemAuth_InCmdField_eSigStartAddr = (12 << 2) + 0,
    BCMD_MemAuth_InCmdField_eSigEndAddrMsb = (13 << 2) + 0,
    BCMD_MemAuth_InCmdField_eSigEndAddr = (14 << 2) + 0,
    BCMD_MemAuth_InCmdField_eIntervalCheckBw = (15 << 2) + 3,
    BCMD_MemAuth_InCmdField_eScbBurstSize = (15 << 2) + 2,
    BCMD_MemAuth_InCmdField_eVerifyPartialDigest = (15 << 2) + 1,
#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)
    BCMD_MemAuth_InCmdField_eResetOnVerifyFailure = (15 << 2) + 0,
#endif
    BCMD_MemAuth_InCmdField_eVKLId = (16 << 2) + 0,
    BCMD_MemAuth_InCmdField_eKeyLayer = (16 << 2) + 1,
    BCMD_MemAuth_InCmdField_eRsaKeyId = (16 << 2) + 3,
    BCMD_MemAuth_InCmdField_eCodeRules = (17 << 2) + 0,
    BCMD_MemAuth_InCmdField_eNoRelocatableCode = (17 << 2) + 3,
    BCMD_MemAuth_InCmdField_eCpuType = (17 << 2) + 2,
#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)

#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V5)
    BCMD_MemAuth_InCmdField_eSvpFwReleaseVersion = (17 << 2) + 1,
#else
    BCMD_MemAuth_InCmdField_eReserved2 = (17 << 2) + 1,
#endif

#else
    BCMD_MemAuth_InCmdField_eEpoch = (17 << 2) + 1,
#endif
    BCMD_MemAuth_InCmdField_eReserved1 = (17 << 2) + 0,
#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)
    BCMD_MemAuth_InCmdField_eMarketID = (18 << 2) + 0,
    BCMD_MemAuth_InCmdField_eMarketIDMask = (19 << 2) + 0,
    BCMD_MemAuth_InCmdField_eReservedE1 = (20 << 2) + 0,
    BCMD_MemAuth_InCmdField_eEpochSel = (20 << 2) + 1,
    BCMD_MemAuth_InCmdField_eEpochMask = (20 << 2) + 2,
    BCMD_MemAuth_InCmdField_eEpoch = (20 << 2) + 3,
    BCMD_MemAuth_InCmdField_eEnforceAuthentication = (21 << 2) + 0,
    BCMD_MemAuth_InCmdField_eAllowRegionDisable = (21 << 2) + 1,
    BCMD_MemAuth_InCmdField_eBgCheck = (21 << 2) + 2,
    BCMD_MemAuth_InCmdField_eInstrCheck = (21 << 2) + 3,
#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V5)
    BCMD_MemAuth_InCmdField_eSigVersion = (22 << 2) + 0,
    BCMD_MemAuth_InCmdField_eSigType = (22 << 2) + 1,
    BCMD_MemAuth_InCmdField_eReservedSig1 = (22 << 2) + 2,
    BCMD_MemAuth_InCmdField_eReservedSig2 = (22 << 2) + 3,
#else
    BCMD_MemAuth_InCmdField_eReserved3 = (22 << 2) + 0,
#endif
    BCMD_MemAuth_InCmdField_eReserved_23_2  =  (23<<2)+2,
    BCMD_MemAuth_InCmdField_eReserved_23_3  =  (23<<2)+3,
    BCMD_MemAuth_InCmdField_eAvsDMEMStartAddr = (24 << 2) + 0,
    BCMD_MemAuth_InCmdField_eAvsDMEMEndAddr = (25 << 2) + 0,

#endif
    BCMD_MemAuth_InCmdField_eMax

} BCMD_MemAuth_InCmdField_e;

typedef enum BCMD_MemAuth_OutCmdField_e
{
    BCMD_MemAuth_OutCmdField_eStatus = (5 << 2) + 3,

    BCMD_MemAuth_OutCmdField_eRegionNum = (6 << 2) + 3,
    BCMD_MemAuth_OutCmdField_eStartAddrMsb0 = (7 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eStartAddr0 = (8 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eEndAddrMsb0 = (9 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eEndAddr0 = (10 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigStartAddrMsb0 = (11 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigStartAddr0 = (12 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigEndAddrMsb0 = (13 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigEndAddr0 = (14 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eStartAddrMsb1 = (15 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eStartAddr1 = (16 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eEndAddrMsb1 = (17 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eEndAddr1 = (18 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigStartAddrMsb1 = (19 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigStartAddr1 = (20 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigEndAddrMsb1 = (21 << 2) + 0,
    BCMD_MemAuth_OutCmdField_eSigEndAddr1 = (22 << 2) + 0,

    BCMD_MemAuth_OutCmdField_eRegion0Status = (7 << 2) + 3,
    BCMD_MemAuth_OutCmdField_eRegionXStatus = (7 << 2) + 2,

    BCMD_MemAuth_OutCmdField_eMax

} BCMD_MemAuth_OutCmdField_e;

typedef enum BCMD_MemAuth_Operation_e
{
    BCMD_MemAuth_Operation_eDisableRegion = 0x00,
    BCMD_MemAuth_Operation_eEnableRegion = 0x01,
    BCMD_MemAuth_Operation_eQueryRegionInfo = 0x02,
    BCMD_MemAuth_Operation_eReserved1 = 0x03,
    BCMD_MemAuth_Operation_eRegionVerified = 0x04,
    BCMD_MemAuth_Operation_eReserved2 = 0x05,
    BCMD_MemAuth_Operation_eScpuFsblMichResize = 0x06,
    BCMD_MemAuth_Operation_eDefineRegion = 0x07,
    BCMD_MemAuth_Operation_eItchDisable = 0x08,
    BCMD_MemAuth_Operation_eReserved9  =  0x09,
    BCMD_MemAuth_Operation_eReservedA = 0x0A,
    BCMD_MemAuth_Operation_eReservedB = 0x0B,
    BCMD_MemAuth_Operation_eRegisterSageCrashResponse = 0x0C,
    BCMD_MemAuth_Operation_eMax
} BCMD_MemAuth_Operation_e;

typedef enum BCMD_ScbBurstSize_e
{
    BCMD_ScbBurstSize_e64 = 0,
    BCMD_ScbBurstSize_e128 = 1,
    BCMD_ScbBurstSize_e256 = 2,
    BCMD_ScbBurstSize_eMax
} BCMD_ScbBurstSize_e;

typedef enum BCMD_MemAuth_VerifyPartDigest_e
{
    MemAuth_VerifyPartDigest_eNoCheck = 0x00,
    MemAuth_VerifyPartDigest_eDigest1 = 0x55,
    MemAuth_VerifyPartDigest_eDigest2 = 0xAA,
    MemAuth_VerifyPartDigest_eMax
} BCMD_MemAuth_VerifyPartDigest_e;

typedef enum BCMD_MemAuth_CpuType_e
{
    BCMD_MemAuth_CpuType_eHost = 0,
    BCMD_MemAuth_CpuType_eRaaga = 1,
    BCMD_MemAuth_CpuType_eRESERVED = 2,
    BCMD_MemAuth_CpuType_eRave = 3,
    BCMD_MemAuth_CpuType_eVdec = 4,
    BCMD_MemAuth_CpuType_eVice = 5,
    BCMD_MemAuth_CpuType_eSid = 6,
    BCMD_MemAuth_CpuType_eScpu = 7,
    BCMD_MemAuth_CpuType_eRESERVED7 = 7,
    BCMD_MemAuth_CpuType_eAvs = 8,
    BCMD_MemAuth_CpuType_eMax
} BCMD_MemAuth_CpuType_e;

typedef enum BCMD_MemAuth_RegionDisable_e
{
    BCMD_MemAuth_RegionDisable_Allow = 0x00,
    BCMD_MemAuth_RegionDisable_Disallow = 0x0E,
    BCMD_MemAuth_RegionDisable_eMax
} BCMD_MemAuth_RegionDisable_e;

typedef enum BCMD_MemAuth_BgCheck_e
{
    BCMD_MemAuth_BgCheck_eEnable = 0x00,
    BCMD_MemAuth_BgCheck_eDisable = 0x0D,
    BCMD_MemAuth_BgCheck_eMax
} BCMD_MemAuth_BgCheck_e;

typedef enum BCMD_MemAuth_InstrCheck_e
{
    BCMD_MemAuth_InstrCheck_eEnable = 0x00,
    BCMD_MemAuth_InstrCheck_eDisable = 0x0C,
    BCMD_MemAuth_InstrCheck_eMax
} BCMD_MemAuth_InstrCheck_e;

typedef enum BCMD_MemAuth_EnforceAuthentication_e
{
    BCMD_MemAuth_EnforceAuthentication_NoEnforce = 0x00,
    BCMD_MemAuth_EnforceAuthentication_Enforce = 0x0B,
    BCMD_MemAuth_EnforceAuthentication_eMax
} BCMD_MemAuth_EnforceAuthentication_e;

#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V3)
typedef enum BCMD_MemAuth_ResetOnVerifyFailure_e
{
    BCMD_MemAuth_ResetOnVerifyFailure_eNoReset = 0x0,
    BCMD_MemAuth_ResetOnVerifyFailure_eReset = 0x5,
    BCMD_MemAuth_ResetOnVerifyFailure_eMax

} BCMD_MemAuth_ResetOnVerifyFailure_e;
#endif




#if (FLASHMAP_VERSION >= FLASHMAP_VERSION_V5)
#define BCMD_GLOBAL_SIG_VERSION                         0x01
#define BCMD_SECOND_TIER_KEY_SIG_VERSION                0x02
#define BCMD_REGION_VERIFY_CODE_SIG_VERSION             0x02

typedef enum BCMD_SignatureType_e
{
    BCMD_SigType_eKeys = 1,
    BCMD_SigType_eBootCode = 2,
    BCMD_SigType_eBootParams = 3,
    BCMD_SigType_eCode = 4,
    BCMD_SigType_eAssymUnlock = 5,
    BCMD_SigType_ePCIEWinSize = 6,
    BCMD_SigType_eMax
}BCMD_SignatureType_e;

typedef enum BCMD_MemAuth_SageIR_e
{
    BCMD_MemAuth_SageIR_eResetSAGEOnly = 0,
    BCMD_MemAuth_SageIR_eResetChip = 1,
    BCMD_MemAuth_SageIR_eMax
} BCMD_MemAuth_SageIR_e;
#endif

#define CPUTYPE_IS_VALID(c)     ((UINT8)(c) < (UINT8)BCMD_MemAuth_CpuType_eMax)
#define CPUTYPE_IS_HOST(c)      ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eHost)
#define CPUTYPE_IS_RAAGA(c)     ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eRaaga)
#define CPUTYPE_IS_RAVE(c)      ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eRave)
#define CPUTYPE_IS_VDEC(c)      ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eVdec)
#define CPUTYPE_IS_VICE(c)      ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eVice)
#define CPUTYPE_IS_SID(c)       ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eSid)
#define CPUTYPE_IS_AVS(c)       ((BCMD_MemAuth_CpuType_e)(c) == BCMD_MemAuth_CpuType_eAvs)





#endif
