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
 * FILENAME: $Workfile: trunk/stack/RF4CE/Profiles/ZRC/include/bbRF4CEZRCKeyExchange.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE ZRC profile key exchange placeholder handler.
 *
 * $Revision: 2370 $
 * $Date: 2014-05-13 11:37:46Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC_KEY_EXCHANGE_H
#define _RF4CE_ZRC_KEY_EXCHANGE_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEPM.h"
#include "bbSysMemMan.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange flags.
 */
#define RF4CE_ZRC2_KEY_EXCHANGE_FLAG_DEFAULT_SECRET          0x01      /* Default key exchange shared secret */
#define RF4CE_ZRC2_KEY_EXCHANGE_FLAG_INITIATOR_VENDOR_SECRET 0x02      /* Initiator vendor specific shared secret */
#define RF4CE_ZRC2_KEY_EXCHANGE_FLAG_RESPONDER_VENDOR_SECRET 0x04      /* Responder vendor specific shared secret */
/* mask of the flags above */
#define RF4CE_ZRC2_KEY_EXCHANGE_FLAGS_MASK                  \
    (RF4CE_ZRC2_KEY_EXCHANGE_FLAG_DEFAULT_SECRET |                \
     RF4CE_ZRC2_KEY_EXCHANGE_FLAG_INITIATOR_VENDOR_SECRET |        \
     RF4CE_ZRC2_KEY_EXCHANGE_FLAG_RESPONDER_VENDOR_SECRET)

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange request params structure.
 */
typedef struct _RF4CE_ZRC2_KeyExchangeReqParams_t
{
    uint8_t pairingRef;        /*!< Pairing reference */
    uint8_t flags;             /*!< First byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
    uint8_t vendorSpecific;    /*!< Second byte of Key Exchange flags. See GDP 2.0 Spec r29 Figure 23. */
} RF4CE_ZRC2_KeyExchangeReqParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange confirmation parameters.
 */
typedef struct _RF4CE_ZRC2_KeyExchangeConfParams_t
{
    uint8_t status;             /*!< Status of the operation: one of the RF4CE_ZRC2GDP2_Status_t values */
} RF4CE_ZRC2_KeyExchangeConfParams_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange request structure declaration.
 */
typedef struct _RF4CE_ZRC2_KeyExchangeReqDescr_t RF4CE_ZRC2_KeyExchangeReqDescr_t;

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange request callback.
 */
typedef void (*RF4CE_ZRC2_KeyExchangeCallback_t)(RF4CE_ZRC2_KeyExchangeReqDescr_t *req, RF4CE_ZRC2_KeyExchangeConfParams_t *conf);

/**//**
 * \brief RF4CE ZRC 2.0 GDP 2.0 Key Exchange request structure.
 */
struct _RF4CE_ZRC2_KeyExchangeReqDescr_t
{
#ifndef _HOST_
    RF4CE_NWK_RequestService_t service;         /*!< Service field. */
#endif /* _HOST_ */
    RF4CE_ZRC2_KeyExchangeReqParams_t params;   /*!< Filled by application request structure */
    RF4CE_ZRC2_KeyExchangeCallback_t callback;  /*!< The request confirmation callback */
};

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts RF4CE ZRC 2.0 Key Exchange procedure.

 \param[in] request - pointer to the Key Exchange request descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_KeyExchangeReq(RF4CE_ZRC2_KeyExchangeReqDescr_t *request);

/************************************************************************************//**
 \brief Key Exchange ZRC 2.0 data indication.

 \param[in] indication - pointer to the indication data structure.
 \param[in] length - the length in bytes of the incoming payload.
 \param[in] leaveReceiverOn - true if necessary to leave receiver on.
 \return Nothing.
 ****************************************************************************************/
void rf4cezrc2KeyExchangeInd(RF4CE_NWK_DataIndParams_t *indication, uint32_t length, bool leaveReceiverOn);

#endif /* _RF4CE_ZRC_KEY_EXCHANGE_H */
