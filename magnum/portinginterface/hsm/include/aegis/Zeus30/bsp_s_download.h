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



#ifndef BSP_S_DOWNLOAD_H__
#define BSP_S_DOWNLOAD_H__


typedef enum BCMD_Download_InCmdSecondTierKeyVerify_e
{
    BCMD_Download_InCmdSecondTierKeyVerify_eKeyIdentifier   = ( 5 << 2)+3,
    BCMD_Download_InCmdSecondTierKeyVerify_eRootKeySource   = ( 6 << 2)+3,
    BCMD_Download_InCmdSecondTierKeyVerify_eMultiTierKey    = ( 6 << 2)+2,
    BCMD_Download_InCmdSecondTierKeyVerify_eChipResetOnFail = ( 6 << 2)+1,
    BCMD_Download_InCmdSecondTierKeyVerify_eAddress         = ( 7 << 2)+0,
    BCMD_Download_InCmdSecondTierKeyVerify_eMax
}BCMD_Download_InCmdSecondTierKeyVerify_e;

typedef enum BCMD_Download_OutCmdSecondTierKeyVerify_e
{
    BCMD_Download_OutCmdSecondTierKeyVerify_eReserved       = (6 << 2)+0,
    BCMD_Download_OutCmdSecondTierKeyVerify_eMax
}BCMD_Download_OutCmdSecondTierKeyVerify_e;


typedef enum BCMD_Download_InCmdSecondStageCodeLoad_e
{
    BCMD_Download_InCmdSecondStageCodeLoad_eSecondStageCodePtrs = (5 << 2)+0,
    BCMD_Download_InCmdSecondStageCodeLoad_eSecondStageCodeSig = (6 << 2)+0,
    BCMD_Download_InCmdSecondStageCodeLoad_eHashLocked = (7 << 2)+1,
    BCMD_Download_InCmdSecondStageCodeLoad_eLoadFromDram = (7 << 2)+2,
    BCMD_Download_InCmdSecondStageCodeLoad_eKeySelect = (7 << 2)+3,
    BCMD_Download_InCmdSecondStageCodeLoad_eMax
}BCMD_Download_InCmdSecondStageCodeLoad_e;

typedef enum BCMD_Download_OutCmdSecondStageCodeLoad_e
{
    BCMD_Download_OutCmdSecondStageCodeLoad_eReserved = (6 << 2)+0,
    BCMD_Download_OutCmdSecondStageCodeLoad_eMax
}BCMD_Download_OutCmdSecondStageCodeLoad_e;



typedef enum BCMD_SecondTierKeyId_e
{
    BCMD_SecondTierKeyId_eKey1  = 1,
    BCMD_SecondTierKeyId_eKey2  = 2,
    BCMD_SecondTierKeyId_eKey3  = 3,
    BCMD_SecondTierKeyId_eKey4  = 4,
    BCMD_SecondTierKeyId_eMax
} BCMD_SecondTierKeyId_e;

typedef enum BCMD_FirstTierKeyId_e
{
    BCMD_FirstTierKeyId_eKey0Prime  = 0,
    BCMD_FirstTierKeyId_eKey0       = 1,
    BCMD_FirstTierKeyId_eMax
} BCMD_FirstTierKeyId_e;

#define RSAKEYID_IS_VALID(k)   (((k) >= BCMD_SecondTierKeyId_eKey1) && ((k) <= BCMD_SecondTierKeyId_eKey4))

#endif
