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
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ProfileManager/include/private/bbRF4CEPMPrivatePair.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE profile manager pair/unpair handlers.
 *
 * $Revision: 2869 $
 * $Date: 2014-07-10 08:15:06Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_PM_PRIVATE_PAIR_H
#define _RF4CE_PM_PRIVATE_PAIR_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWK.h"
#include "bbRF4CEPMExternalIndications.h"
#include "bbRF4CEPMProfiles.h"

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Callback handling Pair Indication primitive.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairInd(RF4CE_NWK_PairIndParams_t *indication);

/************************************************************************************//**
 \brief Callback handling Unpair Indication primitive.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_UnpairInd(RF4CE_NWK_UnpairIndParams_t *indication);

/************************************************************************************//**
 \brief Callback handling Unpair Indication primitive for MSO profile.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UnpairInd(RF4CE_PairingReferenceIndParams_t *indication);

/************************************************************************************//**
 \brief Callback handling Unpair Indication primitive for GDP/ZRC profiles.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_GDP_UnpairInd(RF4CE_PairingReferenceIndParams_t *indication);

/************************************************************************************//**
 \brief Resets the profile data entry.

 \param[in] entry - pointer to the profile data structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ResetProfileData(RF4CE_PM_ProfilesDataQueue_t *entry);

#endif /* _RF4CE_PM_PRIVATE_PAIR_H */