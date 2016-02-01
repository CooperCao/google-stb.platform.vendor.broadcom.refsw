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

#ifndef BBRF4CEZRCVALIDATION_H
#define BBRF4CEZRCVALIDATION_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"
#include "bbSysMemMan.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Validation status.
 */
typedef enum _RF4CE_ZRC2_ValidationStatus_t
{
    RF4CE_ZRC2_VALIDATION_SUCCESS = 0,
    RF4CE_ZRC2_VALIDATION_PENDING = 1,
    RF4CE_ZRC2_VALIDATION_TIMEOUT = 2,
    RF4CE_ZRC2_VALIDATION_FAILURE = 3
} RF4CE_ZRC2_ValidationStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 CheckValidation indication parameters.
 */
typedef struct _RF4CE_ZRC2_CheckValidationIndParams_t
{
    uint8_t pairingRef;      /*!< Pairing reference of the Check Validation request */
    uint8_t isAutoValidated; /*!< true if automatical validation took place */
} RF4CE_ZRC2_CheckValidationIndParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 CheckValidation Response parameters.
 */
typedef struct _RF4CE_ZRC2_CheckValidationRespParams_t
{
    uint8_t pairingRef; /*!< Pairing reference of the Check Validation response */
    uint8_t status;     /*!< Validation status. One of the RF4CE_ZRC2_ValidationStatus_t values */
} RF4CE_ZRC2_CheckValidationRespParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 CheckValidation Response descriptor.
 */
typedef struct _RF4CE_ZRC2_CheckValidationRespDescr_t
{
    RF4CE_ZRC2_CheckValidationRespParams_t params; /*!< Response parameters */
} RF4CE_ZRC2_CheckValidationRespDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts ZRC 2.0 Check Validation Response.

 \param[in] response - pointer to the response structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_CheckValidationResp(RF4CE_ZRC2_CheckValidationRespDescr_t *response);

/************************************************************************************//**
 \brief Check Validation ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2CheckValidationInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

#endif // BBRF4CEZRCVALIDATION_H
