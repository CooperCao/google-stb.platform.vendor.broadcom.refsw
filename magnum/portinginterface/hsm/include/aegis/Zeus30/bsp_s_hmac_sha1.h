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


#ifndef BSP_S_HMAC_SHA1_H__
#define BSP_S_HMAC_SHA1_H__


typedef enum BCMD_HmacSha1_InCmdField_e
{
    BCMD_HmacSha1_InCmdField_eVKL           = (5<<2) + 1,
    BCMD_HmacSha1_InCmdField_eKeyLayer      = (5<<2) + 2,
    BCMD_HmacSha1_InCmdField_eOperation     = (5<<2) + 3,
    BCMD_HmacSha1_InCmdField_eIncludeKey    = (6<<2) + 0,
    BCMD_HmacSha1_InCmdField_eIsAddress     = (6<<2) + 1,
    BCMD_HmacSha1_InCmdField_eShaType       = (6<<2) + 2,
    BCMD_HmacSha1_InCmdField_eContextId     = (6<<2) + 3,
    BCMD_HmacSha1_InCmdField_eKeyLen        = (7<<2) + 3,
    BCMD_HmacSha1_InCmdField_eKeyData       = (8<<2) + 0,
    BCMD_HmacSha1_InCmdField_eMax

} BCMD_HmacSha1_InCmdField_e;

typedef enum BCMD_HmacSha1_OutCmdField_e
{
    BCMD_HmacSha1_OutCmdField_eStatus = (5<<2) + 3,
    BCMD_HmacSha1_OutCmdField_eDigest = (6<<2) + 0,
    BCMD_HmacSha1_OutCmdField_eMax
} BCMD_HmacSha1_OutCmdField_e;


typedef enum BCMD_VerifySecureBootParams_InCmdField_e
{
    BCMD_VerifySecureBootParams_InCmdField_eReserved    = (5<<2) + 0,
    BCMD_VerifySecureBootParams_InCmdField_eDramParams  = (6<<2) + 0,
    BCMD_VerifySecureBootParams_InCmdField_eMax
} BCMD_VerifySecureBootParams_InCmdField_e;

typedef enum BCMD_VerifySecureBootParams_OutCmdField_e
{
    BCMD_VerifySecureBootParams_OutCmdField_eStatus        = (5<<2) + 3,
    BCMD_VerifySecureBootParams_OutCmdField_eBspParamsSize = (6<<2) + 3,
    BCMD_VerifySecureBootParams_OutCmdField_eBspParams     = (7<<2) + 0,
    BCMD_VerifySecureBootParams_OutCmdField_eMax
} BCMD_VerifySecureBootParams_OutCmdField_e;


typedef enum BPI_HmacSha1_Op_e
{
    BPI_HmacSha1_Op_eSha1 = 0,
    BPI_HmacSha1_Op_eHmac = 1,
    BPI_HmacSha1_Op_eMax
} BPI_HmacSha1_Op_e;

typedef enum BPI_HmacSha1_ShaType_e
{
    BPI_HmacSha1_ShaType_eSha160 = 0,
    BPI_HmacSha1_ShaType_eSha224 = 1,
    BPI_HmacSha1_ShaType_eSha256 = 2,
    BPI_HmacSha1_ShaType_eMax
} BPI_HmacSha1_ShaType_e;

#define BPI_REGION_CONTEXT_MAX      0x1B

typedef enum BPI_HmacSha1_Context_e
{
    BPI_HmacSha1_Context_eRegionMin  = 0x00,

    BPI_HmacSha1_Context_eCpuMin     = 0x00,
    BPI_HmacSha1_Context_eCpu0       = 0x00,
    BPI_HmacSha1_Context_eCpu1       = 0x01,
    BPI_HmacSha1_Context_eCpu2       = 0x02,
    BPI_HmacSha1_Context_eCpu3       = 0x03,
    BPI_HmacSha1_Context_eCpu4       = 0x04,
    BPI_HmacSha1_Context_eCpu5       = 0x05,
    BPI_HmacSha1_Context_eCpu6       = 0x06,
    BPI_HmacSha1_Context_eCpu7       = 0x07,
    BPI_HmacSha1_Context_eCpuMax     = 0x07,

    BPI_HmacSha1_Context_eRave0      = 0x08,

    BPI_HmacSha1_Context_eAudio0     = 0x09,

    BPI_HmacSha1_Context_eAvd0_Ila   = 0x0A,
    BPI_HmacSha1_Context_eAvd0_Ola   = 0x0B,

    BPI_HmacSha1_Context_eSvd0_Ila   = 0x0C,
    BPI_HmacSha1_Context_eSvd0_Ola   = 0x0D,
    BPI_HmacSha1_Context_eSvd0_Bld   = 0x0E,

    BPI_HmacSha1_Context_eVice0_Pic  = 0x0F,
    BPI_HmacSha1_Context_eVice0_Mb   = 0x10,
    BPI_HmacSha1_Context_eRegion16   = 0x10,

    BPI_HmacSha1_Context_eSid0       = 0x11,

    BPI_HmacSha1_Context_eRaaga1     = 0x12,

    BPI_HmacSha1_Context_eVice1_Pic  = 0x13,
    BPI_HmacSha1_Context_eVice1_Mb   = 0x14,

    BPI_HmacSha1_Context_eRaaga0_IntSCM = 0x15,
    BPI_HmacSha1_Context_eRaaga0_IntAUD = 0x16,

    BPI_HmacSha1_Context_eReserved0    = 0x17,

    BPI_HmacSha1_Context_eScpu_FSBL   = 0x18,
    BPI_HmacSha1_Context_eReserved25  =  0x19,
    BPI_HmacSha1_Context_eReserved26  =  0x1A,
    BPI_HmacSha1_Context_eReserved27  =  0x1B,

    BPI_HmacSha1_Context_eRegionMax  = BPI_REGION_CONTEXT_MAX,

    BPI_HmacSha1_Context_eReserved1  = 0x1C,
    BPI_HmacSha1_Context_eReserved2  = 0x1D,
    BPI_HmacSha1_Context_eReserved3  = 0x1E,
    BPI_HmacSha1_Context_eReserved4  = 0x1F,

    BPI_HmacSha1_Context_eHmacSha1CtxMin = 0x20,
    BPI_HmacSha1_Context_eHmacSha1Ctx0 = 0x20,
    BPI_HmacSha1_Context_eHmacSha1Ctx1 = 0x21,

    BPI_HmacSha1_Context_eHmacSha1CtxSCM = 0x22,
    BPI_HmacSha1_Context_eHmacSha1CtxMax = 0x22,

    BPI_HmacSha1_Context_eCmdSig0    = 0x23,
    BPI_HmacSha1_Context_eCmdSig1    = 0x24,
    BPI_HmacSha1_Context_eAsymmUnlockCmd    = 0x25,

    BPI_HmacSha1_Context_eMax
} BPI_HmacSha1_Context_e;


typedef enum BPI_HmacSha1_IncludeKey_e
{
    BPI_HmacSha1_IncludeKey_eNo          = 0x0,
    BPI_HmacSha1_IncludeKey_eAppend      = 0x1,
    BPI_HmacSha1_IncludeKey_eMax
} BPI_HmacSha1_IncludeKey_e;

#endif
