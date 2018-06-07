/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#ifndef BSP_S_SDRAM_SCRAMBLING_ENABLE_H__
#define BSP_S_SDRAM_SCRAMBLING_ENABLE_H__

typedef enum BCMD_SdramScram_InCmdOperation_e
{
    BCMD_SdramScram_InCmdOperation_eReserved_5_0  =  (5<<2)+0,
    BCMD_SdramScram_InCmdOperation_eMemcType = (5 << 2) + 1,
    BCMD_SdramScram_InCmdOperation_eSubCmd = (5 << 2) + 2,
    BCMD_SdramScram_InCmdOperation_eBootType = (5 << 2) + 3,

    BCMD_SdramScram_InCmdOperation_eReserved_22_0  =  (22<<2)+0,
    BCMD_SdramScram_InCmdOperation_eMax
} BCMD_SdramScram_InCmdOperation_e;

typedef enum BCMD_SdramScram_OutCmdOperation_e
{
    BCMD_SdramScram_OutCmdOperation_eStatus = (5 << 2) + 3,
    BCMD_SdramScram_OutCmdOperation_eKeyLoadStatus = (6 << 2) + 3,
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
