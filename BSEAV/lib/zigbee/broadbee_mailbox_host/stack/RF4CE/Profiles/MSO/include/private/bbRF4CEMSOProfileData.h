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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/private/bbRF4CEMSOProfileData.h $
 *
 * DESCRIPTION:
 *   This is the private header file for the RF4CE MSO profile data handler.
 *
 * $Revision: 2999 $
 * $Date: 2014-07-21 13:30:43Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_PROFILE_DATA_H
#define _RF4CE_MSO_PROFILE_DATA_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbSysBasics.h"
#include "bbRF4CEMSORIB.h"
#include "bbRF4CEMSOUserControl.h"
#include "bbRF4CEMSOValidation.h"
#include "bbRF4CENWKRX.h"
#include "bbSysNvmManager.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO Profile State Values.
 */
typedef enum _RF4CE_MSO_ProfileState_t
{
    RF4CE_MSO_PS_FREE = 0,
    RF4CE_MSO_PS_RIB_GET,
    RF4CE_MSO_PS_RIB_SET
} RF4CE_MSO_ProfileState_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO Storable Profile Data type.
 */
typedef struct _RF4CE_MSO_StorableProfileData_t
{

} RF4CE_MSO_StorableProfileData_t;

/**//**
 * \brief RF4CE MSO Profile Data type.
 */
typedef struct _RF4CE_MSO_ProfileData_t
{
    uint32_t ribTimeoutGet;
    uint32_t ribTimeoutStamp;
    uint32_t ribTimeoutSet;
    uint32_t bindResponseIdleTimeout;
    uint8_t state;
#ifdef RF4CE_TARGET
    uint32_t ribTimeoutGetWait;
    NVM_ReadFileRespParams_t *confRead;
# ifdef RF4CE_NWK_NVM_ENABLED
    NVM_ReadFileIndDescr_t readNVM;
    NVM_WriteFileIndDescr_t writeNVM;
    uint8_t flagsNVM;
# endif /* RF4CE_NWK_NVM_ENABLED */
    SYS_DataPointer_t actionCommandFrame;
    uint32_t actionCommandWaitTimeout;
    uint32_t validationWaitTimeout;
    uint32_t validationInitialWatchdogTimeout;
    uint32_t validationBlackoutTimeout;
    uint8_t validationStatus;
    uint8_t readAttributeId;
    uint8_t readAttributeIndex;
    uint8_t readResult;
    uint8_t writeAttributeId;
    uint8_t writeAttributeIndex;
    uint8_t writeResult;
#else /* RF4CE_TARGET */
    RF4CE_NWK_RXEnableReqDescr_t rxOnOff;
    uint32_t ribTimeoutRIB;
    uint32_t linkLostTimeout;
    uint32_t autoRepeatValidationRequest;
    uint32_t shortRetry;
    uint32_t ucTimeout;
    RF4CE_MSO_GetRIBAttributeReqDescr_t ribRequest;
    struct
    {
        SYS_DataPointer_t nwkData[3];
        uint8_t key;
        uint8_t currentId;
        uint8_t counter[3];
        uint8_t txOptions[3];
        uint16_t bitmask;
    } keyData;
#endif /* RF4CE_TARGET */
} RF4CE_MSO_ProfileData_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Processes the MSO timeout task.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_TimerProc(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Processes the MSO cleanup task.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \param[in] _msos_ - pointer to the RF4CE_MSO_StorableProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_ResetProfileData(RF4CE_MSO_ProfileData_t *_mso_, RF4CE_MSO_StorableProfileData_t *_msos_);

/************************************************************************************//**
 \brief Get RIB Attribute Requests timeout handler.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_TimeoutGetHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Set RIB Attribute Requests timeout handler.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_TimeoutSetHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Bind Response Idle Timeout.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindResponseIdleTimeoutHandler(RF4CE_MSO_ProfileData_t *_mso_);

#ifdef RF4CE_CONTROLLER

/************************************************************************************//**
 \brief RIB Attribute Requests repost handler. Controller only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_TimeoutRIBHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Bind Link Lost Timeout Handler. Controller only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindLinkLostTimeoutHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Bind Auto Check Validation. Controller only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_BindAutoCheckValidationHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief User Control Timeout. Controller only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_UserControlTimeoutHandler(RF4CE_MSO_ProfileData_t *_mso_);

#else /* RF4CE_CONTROLLER */

/************************************************************************************//**
 \brief Processes the MSO aplActionRepeatWaitTime timeout task.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 ****************************************************************************************/
void RF4CE_MSO_ControlCommandWaitTimeoutHandler(RF4CE_MSO_ProfileData_t * _mso_);

/************************************************************************************//**
 \brief Wait before send reply handler. Target only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_RIB_TimeoutGetWaitHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Validation Failure handler. Target only.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_ValidationFailureHandler(RF4CE_MSO_ProfileData_t *_mso_);

/************************************************************************************//**
 \brief Target Blackout handler.

 \param[in] _mso_ - pointer to the RF4CE_MSO_ProfileData_t structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_TargetBlackoutHandler(RF4CE_MSO_ProfileData_t *_mso_);

#endif /* RF4CE_CONTROLLER */

#endif /* _RF4CE_MSO_PROFILE_DATA_H */