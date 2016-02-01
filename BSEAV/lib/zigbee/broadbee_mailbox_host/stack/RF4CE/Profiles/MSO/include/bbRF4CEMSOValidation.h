/******************************************************************************
* (c) 2014 Broadcom Corporation
*
* This program is the proprietary software of Broadcom Corporation and/or its
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
******************************************************************************/
/*****************************************************************************
 *
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/MSO/include/bbRF4CEMSOValidation.h $
 *
 * DESCRIPTION:
 *   This is the header file for the MSO RF4CE Profile
 *   Validation handling.
 *
 * $Revision: 1616 $
 * $Date: 2014-02-26 13:46:36Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_MSO_VALIDATION_H
#define _RF4CE_MSO_VALIDATION_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CEMSOConstants.h"
#include "bbRF4CENWKConstants.h"
#include "bbSysQueue.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE MSO Validation status.
 */
typedef enum _RF4CE_MSO_ValidationStatus_t
{
    RF4CE_MSO_VALIDATION_SUCCESS    = 0,    /*!< The validation is successful. */
    RF4CE_MSO_VALIDATION_PENDING    = 0xC0, /*!< The validation is still in progress. */
    RF4CE_MSO_VALIDATION_TIMEOUT    = 0xC1, /*!< The validation timed out, and the binding procedure SHOULD
                                                 continue with other devices in the list. */
    RF4CE_MSO_VALIDATION_COLLISION  = 0xC2, /*!< The validation was terminated at the target side, as more than one
                                                 controller tried to pair. */
    RF4CE_MSO_VALIDATION_FAILURE    = 0xC3, /*!< The validation failed, and the binding procedure SHOULD continue
                                                 with other devices in the list. */
    RF4CE_MSO_VALIDATION_ABORT      = 0xC4, /*!< The validation is aborted, and the binding procedure SHOULD
                                                 continue with other devices in the list. */
    RF4CE_MSO_VALIDATION_FULL_ABORT = 0xC5  /*!< The validation is aborted, and the binding procedure SHOULD
                                                 NOT continue with other devices in the list. */
} RF4CE_MSO_ValidationStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE MSO Check Validation indication parameters.
 */
typedef struct _RF4CE_MSO_CheckValidationIndParams_t
{
    uint8_t pairingRef;
    uint8_t flags;
} RF4CE_MSO_CheckValidationIndParams_t;

/**//**
 * \brief RF4CE MSO Watchdog Kick or Validate request parameters.
 */
typedef struct _RF4CE_MSO_WatchDogKickOrValidateReqParams_t
{
    uint8_t pairingRef;
    uint8_t status;
} RF4CE_MSO_WatchDogKickOrValidateReqParams_t;

/**//**
 * \brief RF4CE MSO Watchdog Kick or Validate request descriptor.
 */
typedef struct _RF4CE_MSO_WatchDogKickOrValidateReqDescr_t
{
    RF4CE_MSO_WatchDogKickOrValidateReqParams_t params;
} RF4CE_MSO_WatchDogKickOrValidateReqDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief MSO Watchdog Kick or Validate request.

 \param[in] request - pointer to the request structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_MSO_WatchDogKickOrValidateReq(RF4CE_MSO_WatchDogKickOrValidateReqDescr_t *request);

#endif /* _RF4CE_MSO_VALIDATION_H */