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
 *      This is the header file for the MSO Profile.
 *
*******************************************************************************/

#ifndef _RF4CE_MSO_H
#define _RF4CE_MSO_H


/******************************** RF4CE MSO DOCUMENTATION STRUCTURE ***************************/
/**//**
 * \defgroup RF4CE (RF4CE API)
 @{
 * \defgroup Profiles (RF4CE Profiles)
 @{
 * \defgroup MSO (MSO Profile API)
 @{
 * \defgroup RF4CE_MSO_Types (MSO Types)
 @{
 * \defgroup RF4CE_MSO_BindReq (Bind Request)
 * \defgroup RF4CE_MSO_BindConf (Bind Confirmation)
 * \defgroup RF4CE_MSO_UserControlInd (User Control Indication)
 * \defgroup RF4CE_MSO_UserControlReq (User Control Request)
 * \defgroup RF4CE_MSO_UserControlConf (User Control Confirmation)
 * \defgroup RF4CE_MSO_ProfileAttributeMisc (Get/Set Profile Attribute Misc)
 * \defgroup RF4CE_MSO_GetProfileAttributeReq (Get Profile Attribute Request)
 * \defgroup RF4CE_MSO_GetProfileAttributeConf (Get Profile Attribute Confirmation)
 * \defgroup RF4CE_MSO_SetProfileAttributeReq (Set Profile Attribute Request)
 * \defgroup RF4CE_MSO_SetProfileAttributeConf (Set Profile Attribute Confirmation)
 * \defgroup RF4CE_MSO_GetRIBAttributeReq (Get RIB Attribute Request)
 * \defgroup RF4CE_MSO_GetRIBAttributeConf (Get RIB Attribute Confirmation)
 * \defgroup RF4CE_MSO_SetRIBAttributeReq (Set RIB Attribute Request)
 * \defgroup RF4CE_MSO_SetRIBAttributeConf (Set RIB Attribute Confirmation)
 * \defgroup RF4CE_MSO_CheckValidationReq (Check Validation Request)
 * \defgroup RF4CE_MSO_CheckValidationInd (Check Validation Indication)
 @}
 * \defgroup RF4CE_MSO_Functions (MSO Routines)
 @}
 @}
 @}
 */


/************************* INCLUDES ****************************************************/
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CEMSOProfileAttributes.h"
#include "bbRF4CEMSORIB.h"
#include "bbRF4CEMSOExternalIndications.h"
#include "bbRF4CEMSOBind.h"
#include "bbRF4CEMSOValidation.h"
#include "bbRF4CEMSOStartReset.h"
#include "bbRF4CEMSOUserControl.h"
#include "bbRF4CEMSOStaticData.h"

#endif /* _RF4CE_MSO_H */

/* eof bbRF4CEMSO.h */