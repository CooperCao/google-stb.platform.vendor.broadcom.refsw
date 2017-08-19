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
 *      This is the header file for the RF4CE profile manager.
 *
*******************************************************************************/

#ifndef _RF4CE_PM_H
#define _RF4CE_PM_H


/******************************** RF4CE Profile Manager DOCUMENTATION STRUCTURE ***************************/
/**//**
 * \defgroup RF4CE (RF4CE API)
 @{
 * \defgroup Profiles (RF4CE Profiles)
 @{
 * \defgroup ProfileManager (Profile Manager API)
 @{
 * \defgroup RF4CE_PM_Types (Profile Manager Types)
 @{
 * \defgroup RF4CE_PairingInd (Pairing Indication)
 * \defgroup RF4CE_UnpairReq (Unpair Request)
 * \defgroup RF4CE_UnpairConf (Unpair Confirmation)
 * \defgroup RF4CE_ProfileDataInd (Profile Data Indication)
 * \defgroup RF4CE_SetSupportedDevicesReq (Set Supported Devices Request)
 * \defgroup RF4CE_SetSupportedDevicesConf (Set Supported Devices Confirmation)
 * \defgroup RF4CE_StartReq (Start Request)
 * \defgroup RF4CE_StartResetConf (Start/Reset Confirmation)
 * \defgroup RF4CE_ResetReq (Reset Request)
 @}
 * \defgroup RF4CE_PM_Functions (Profile Manager Routines)
 @}
 @}
 @}
 */


/************************* INCLUDES ****************************************************/
#include "bbRF4CENWK.h"
#include "bbRF4CEPMStartReset.h"
#include "private/bbRF4CEPMDiscovery.h"
#include "bbRF4CEPMPair.h"
#include "private/bbRF4CEPMData.h"
#include "bbRF4CEPMProfiles.h"
#include "bbRF4CEPMStaticData.h"
#include "bbRF4CEPMExternalIndications.h"
#include "bbHalSystemTimer.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief The timeout value in ms.
 */
#define RF4CE_PM_TIMEOUT_VALUE HAL_TIMER_TASK_PERIOD_MS
#define RF4CE_PM_TIMEOUT(data) (((uint32_t)(data) + RF4CE_PM_TIMEOUT_VALUE - 1) / RF4CE_PM_TIMEOUT_VALUE)

#endif /* _RF4CE_PM_H */

/* eof bbRF4CEPM.h */