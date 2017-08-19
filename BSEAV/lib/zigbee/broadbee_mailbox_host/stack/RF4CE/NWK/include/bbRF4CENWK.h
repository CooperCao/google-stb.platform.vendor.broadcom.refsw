/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

/*******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the RF4CE Network Layer component.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_H
#define _RF4CE_NWK_H


/******************************** RF4CE DOCUMENTATION STRUCTURE ***************************/
/**//**
 * \defgroup RF4CE (RF4CE API)
 @{
 * \defgroup NWK (Network API)
 @{
 * \defgroup RF4CE_NWK_Types (RF4CE Network Types)
 @{
 * \defgroup RF4CE_NWK_DataReq (Data Request)
 * \defgroup RF4CE_NWK_DataInd (Data Indication)
 * \defgroup RF4CE_NWK_DataConf (Data Confirmation)
 * \defgroup RF4CE_NWK_DiscoveryConf (Discovery Confirmation)
 * \defgroup RF4CE_NWK_DiscoveryReq (Discovery Request)
 * \defgroup RF4CE_NWK_DiscoveryInd (Discovery Indication)
 * \defgroup RF4CE_NWK_DiscoveryResp (Discovery Response)
 * \defgroup RF4CE_NWK_CommStatusInd (CommStatus Indication)
 * \defgroup RF4CE_NWK_AutoDiscoveryReq (AutoDiscovery Request)
 * \defgroup RF4CE_NWK_AutoDiscoveryConf (AutoDiscovery Confirmation)
 * \defgroup RF4CE_NWK_SetReq (Set Request)
 * \defgroup RF4CE_NWK_SetConf (Set Confirmation)
 * \defgroup RF4CE_NWK_GetReq (Get Request)
 * \defgroup RF4CE_NWK_GetConf (Get Confirmation)
 * \defgroup RF4CE_NWK_PairReq (Pair Request)
 * \defgroup RF4CE_NWK_PairConf (Pair Confirmation)
 * \defgroup RF4CE_NWK_PairInd (Pair Indication)
 * \defgroup RF4CE_NWK_PairResp (Pair Response)
 * \defgroup RF4CE_NWK_UnpairReq (Unpair Request)
 * \defgroup RF4CE_NWK_UnpairConf (Unpair Confirmation)
 * \defgroup RF4CE_NWK_UnpairResp (Unpair Response)
 * \defgroup RF4CE_NWK_UnpairInd (Unpair Indication)
 * \defgroup RF4CE_NWK_KeySeedStartInd (KeySeed Start Request)
 * \defgroup RF4CE_NWK_RxEnableReq (RxEnable Request)
 * \defgroup RF4CE_NWK_RxEnableConf (RxEnable Confirmation)
 * \defgroup RF4CE_NWK_StartReq (Start Request)
 * \defgroup RF4CE_NWK_StartConf (Start Confirmation)
 * \defgroup RF4CE_NWK_ResetReq (Reset Request)
 * \defgroup RF4CE_NWK_ResetConf (Reset Confirmation)
 * \defgroup RF4CE_NWK_UpdateKeyReq (UpdateKey Request)
 * \defgroup RF4CE_NWK_UpdateKeyConf (UpdateKey Confirmation)
 @}
 * \defgroup RF4CE_NWK_Functions (RF4CE Network Routines)
 @}
 @}
 */


/************************* INCLUDES ****************************************************/
#include "bbRF4CEConfig.h"
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbRF4CENWKNIBAttributes.h"
#include "bbRF4CENWKStaticData.h"
#include "bbRF4CENWKStartReset.h"
#include "bbRF4CENWKDiscovery.h"
#include "bbRF4CENWKPair.h"
#include "bbRF4CENWKRX.h"
#include "bbRF4CENWKUpdateKey.h"
#include "bbRF4CENWKData.h"
#include "bbRF4CENWKTasks.h"
#include "bbRF4CENWKEncryptDecrypt.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE Free Payload macro.
 */
#define RF4CE_FREE_PAYLOAD(p) \
    if (SYS_CheckPayload(p)) \
        SYS_FreePayload(p)

#endif /* _RF4CE_NWK_H */

/* eof bbRF4CENWK.h */