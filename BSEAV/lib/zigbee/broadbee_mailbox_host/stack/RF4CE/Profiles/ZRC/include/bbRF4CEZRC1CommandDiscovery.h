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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRC1CommandDiscovery.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC 1.1 profile Command Discovery Handler handler.
 *
 * $Revision: 3515 $
 * $Date: 2014-09-10 11:15:21Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC1_COMMAND_DISCOVERY_H
#define _RF4CE_ZRC1_COMMAND_DISCOVERY_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery Request status.
 */
typedef enum _RF4CE_ZRC1_CommandDiscoveryStatus_t
{
    RF4CE_ZRC1_COMMAND_DISCOVERY_SUCCESS = 0,
    RF4CE_ZRC1_COMMAND_DISCOVERY_SEND,
    RF4CE_ZRC1_COMMAND_DISCOVERY_RECEIVED,
    RF4CE_ZRC1_COMMAND_DISCOVERY_TIMEOUT,
    RF4CE_ZRC1_COMMAND_DISCOVERY_INVALID_REQUEST
} RF4CE_ZRC1_CommandDiscoveryStatus_t;

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery Request Frame.
 */
typedef struct PACKED _RF4CE_ZRC1_CommandDiscoveryRequestFrame_t
{
    uint8_t frameControl;               /*!< Command code. */
    uint8_t reserved;                   /*!< Must be 0. */
} RF4CE_ZRC1_CommandDiscoveryRequestFrame_t;

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery Response Frame.
 */
typedef struct PACKED _RF4CE_ZRC1_CommandDiscoveryResponseFrame_t
{
    uint8_t frameControl;               /*!< Command code. */
    uint8_t reserved;                   /*!< Must be 0. */
    uint8_t bitmap[32];                 /*!< The data returned */
} RF4CE_ZRC1_CommandDiscoveryResponseFrame_t;

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery request structure.
 */
typedef struct PACKED _RF4CE_ZRC1_CommandDiscoveryReqParams_t
{
    uint8_t pairingRef;                  /*!< The pairing reference. */
} RF4CE_ZRC1_CommandDiscoveryReqParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery confirmation structure.
 */
typedef struct PACKED _RF4CE_ZRC1_CommandDiscoveryConfParams_t
{
    RF4CE_ZRC1_CommandDiscoveryStatus_t status; /*!< Status of the operation. */
    uint8_t bitmap[32];                         /*!< The requested bitmap. */
} RF4CE_ZRC1_CommandDiscoveryConfParams_t;

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery request descriptor declaration.
 */
typedef struct _RF4CE_ZRC1_CommandDiscoveryReqDescr_t RF4CE_ZRC1_CommandDiscoveryReqDescr_t;

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery request callback.
 */
typedef void (*RF4CE_ZRC1_CommandDiscoveryCallback_t)(RF4CE_ZRC1_CommandDiscoveryReqDescr_t *req, RF4CE_ZRC1_CommandDiscoveryConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 1.1 Command Discovery request descriptor.
 */
struct _RF4CE_ZRC1_CommandDiscoveryReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;
#endif /* _HOST_ */
    RF4CE_ZRC1_CommandDiscoveryReqParams_t params;
    RF4CE_ZRC1_CommandDiscoveryCallback_t callback;
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts sending ZRC 1.1 Command Discovery request.

 \param[in] request - pointer to the ZRC 1.1 Command Discovery request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC1_CommandDiscoveryReq(RF4CE_ZRC1_CommandDiscoveryReqDescr_t *request);

#endif /* _RF4CE_ZRC1_COMMAND_DISCOVERY_H */