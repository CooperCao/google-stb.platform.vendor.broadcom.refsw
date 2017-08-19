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

/******************************************************************************
 *
 * DESCRIPTION:
 *      This is the header file for the RF4CE ZRC profile.
 *
*******************************************************************************/

#ifndef _RF4CE_ZRC_H
#define _RF4CE_ZRC_H


/******************************** RF4CE ZRC DOCUMENTATION STRUCTURE ***************************/
/**//**
 * \defgroup RF4CE (RF4CE API)
 @{
 * \defgroup Profiles (RF4CE Profiles)
 @{
 * \defgroup ZRC (ZRC API)
 @{
 * \defgroup RF4CE_ZRC_Types (ZRC Types)
 @{
 * \defgroup RF4CE_ZRC1_SetAttributeReq (ZRC1 Set Attribute Request)
 * \defgroup RF4CE_ZRC1_SetAttributeConf (ZRC1 Set Attribute Confirmation)
 * \defgroup RF4CE_ZRC1_GetAttributeReq (ZRC1 Get Attribute Request)
 * \defgroup RF4CE_ZRC1_GetAttributeConf (ZRC1 Get Attribute Confirmation)
 * \defgroup RF4CE_ZRC1_BindReq (ZRC1 Bind Request)
 * \defgroup RF4CE_ZRC1_BindConf (ZRC1 Bind Confirmation)
 * \defgroup RF4CE_ZRC1_CommandDiscoveryReq (Command Discovery Request)
 * \defgroup RF4CE_ZRC1_CommandDiscoveryConf (Command Discovery Confirmation)
 * \defgroup RF4CE_ZRC1_CommandDiscoveryResp (Command Discovery Response)
 * \defgroup RF4CE_ZRC1_ControlCommandReq (ZRC1 Control Command Request)
 * \defgroup RF4CE_ZRC1_ControlCommandInd (ZRC1 Control Command Indication)
 * \defgroup RF4CE_ZRC1_ControlCommandConf (ZRC1 Control Command Confirmation)
 * \defgroup RF4CE_ZRC1_VendorSpecificReq (Vendor Specific Request)
 * \defgroup RF4CE_ZRC1_VendorSpecificConf (Vendor Specific Confirmation)
 * \defgroup RF4CE_ZRC1_VendorSpecificInd (Vendor Specific Indication)
 * \defgroup RF4CE_ZRC2_SetAttributeReq (ZRC2 Set Attribute Request)
 * \defgroup RF4CE_ZRC2_SetAttributeConf (ZRC2 Set Attribute Confirmation)
 * \defgroup RF4CE_ZRC2_GetAttributeReq (ZRC2 Get Attribute Request)
 * \defgroup RF4CE_ZRC2_GetAttributeConf (ZRC2 Get Attribute Confirmation)
 * \defgroup RF4CE_ZRC2_BindReq (ZRC2 Bind Request)
 * \defgroup RF4CE_ZRC2_BindConf (ZRC2 Bind Confirmation)
 * \defgroup RF4CE_ZRC2_BindInd (ZRC2 Bind Indication)
 * \defgroup RF4CE_ZRC2_ProxyBindReq (ProxyBind Request)
 * \defgroup RF4CE_ZRC2_ProxyBindConf (ProxyBind Confirmation)
 * \defgroup RF4CE_ZRC2_ButtonBindingReq (Button Binding Request)
 * \defgroup RF4CE_ZRC2_ButtonBindingConf (Button Binding Confirmation)
 * \defgroup RF4CE_GDP2_ClientNotificationReq (Client Notification Request)
 * \defgroup RF4CE_GDP2_ClientNotificationConf (Client Notification Confirmation)
 * \defgroup RF4CE_GDP2_ClientNotificationInd (Client Notification Indication)
 * \defgroup RF4CE_ZRC2_ControlCommandReq (ZRC2 Control Command Request)
 * \defgroup RF4CE_ZRC2_ControlCommandInd (ZRC2 Control Command Indication)
 * \defgroup RF4CE_ZRC2_ControlCommandConf (ZRC2 Control Command Confirmation)
 * \defgroup RF4CE_ZRC2_GetSharedSecretInd (Get Shared Secret Indication)
 * \defgroup RF4CE_ZRC2_GetSharedSecretResp (Get Shared Secret Response)
 * \defgroup RF4CE_GDP2_HeartbeatInd (Heartbeat Indication)
 * \defgroup RF4CE_GDP2_IdentifyCapAnnounceReq (Identify Capabilities Announce Request)
 * \defgroup RF4CE_GDP2_IdentifyCapAnnounceConf (Identify Capabilities Announce Confirmation)
 * \defgroup RF4CE_GDP2_IdentifyCapAnnounceInd (Identify Capabilities Announce Indication)
 * \defgroup RF4CE_GDP2_IdentifyInd (Identify Indication)
 * \defgroup RF4CE_GDP2_IdentifyConf (Identify Confirmation)
 * \defgroup RF4CE_GDP2_IdentifyReq (Identify Request)
 * \defgroup RF4CE_ZRC2_KeyExchangeReq (Key Exchange Request)
 * \defgroup RF4CE_ZRC2_KeyExchangeConf (Key Exchange Confirmation)
 * \defgroup RF4CE_GDP2_SetPollConstraintsReq (Set Poll Constraints Request)
 * \defgroup RF4CE_GDP2_SetPollConstraintsConf (Set Poll Constraints Confirmation)
 * \defgroup RF4CE_GDP2_PollNegotiationReq (Poll Negotiation Request)
 * \defgroup RF4CE_GDP2_PollNegotiationConf (Poll Negotiation Confirmation)
 * \defgroup RF4CE_GDP2_PollNegotiationInd (Poll Negotiation Indication)
 * \defgroup RF4CE_GDP2_PollClientUserEventReq (Poll Client User Event Request)
 * \defgroup RF4CE_GDP2_PollClientUserEventConf (Poll Client User Event Confirmation)
 * \defgroup RF4CE_ZRC2_CheckValidationInd (Check Validation Indication)
 * \defgroup RF4CE_ZRC2_CheckValidationResp (Check Validation Response)
 @}
 * \defgroup RF4CE_ZRC_Functions (ZRC Routines)
 @}
 @}
 @}
 */


/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"
#include "bbRF4CEZRCConstants.h"
#include "bbRF4CEZRCAttributes.h"
#include "bbRF4CEZRCKeyExchange.h"
#include "bbRF4CEZRCStartReset.h"
#include "bbRF4CEZRCStaticData.h"
#include "bbRF4CEZRCBind.h"
#include "bbRF4CEZRCExternalIndications.h"
#include "bbRF4CEZRCControlCommand.h"
#include "private/bbRF4CEZRCPrivateControlCommand.h"
#include "private/bbRF4CEZRCPrivateActionMap.h"
#include "private/bbRF4CEZRCTasks.h"

#include "bbRF4CEZRC1.h"

#endif /* _RF4CE_ZRC_H */

/* eof bbRF4CEZRC.h */