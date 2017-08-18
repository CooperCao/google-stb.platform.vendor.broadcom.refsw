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
 *      This is the header file for the RF4CE Network Layer component Update Key handling routines.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_UPDATE_KEY_H
#define _RF4CE_NWK_UPDATE_KEY_H
/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbSysQueue.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief RF4CE NWK NLME-UPDATE-KEY.request parameters type.
 * \ingroup RF4CE_NWK_UpdateKeyReq
 */
typedef struct _RF4CE_NWK_UpdateKeyReqParams_t
{
    uint8_t pairingRef;                             /*!< The reference into the local pairing table of the entry whose key is to be updated. */
    uint8_t newLinkKey[RF4CE_SECURITY_KEY_LENGTH];  /*!< The security link key to replace the key in the pairing table. */
} RF4CE_NWK_UpdateKeyReqParams_t;

/**//**
 * \brief RF4CE NWK NLME-UPDATE-KEY.confirm parameters type.
 * \ingroup RF4CE_NWK_UpdateKeyConf
 */
typedef struct _RF4CE_NWK_UpdateKeyConfParams_t
{
    uint8_t status;      /*!< The status of the request to update the security link key. */
    uint8_t pairingRef;  /*!< The reference into the local pairing table of the entry whose key is to be updated. */
} RF4CE_NWK_UpdateKeyConfParams_t;

/**//**
 * \brief RF4CE NWK NLME-UPDATE-KEY.request type declaration.
 * \ingroup RF4CE_NWK_UpdateKeyReq
 */
typedef struct _RF4CE_NWK_UpdateKeyReqDescr_t RF4CE_NWK_UpdateKeyReqDescr_t;

/**//**
 * \brief RF4CE NWK NLME-UPDATE-KEY confirmation function type.
 * \ingroup RF4CE_NWK_UpdateKeyConf
 */
typedef void (*RF4CE_NWK_UpdateKeyConfCallback_t)(RF4CE_NWK_UpdateKeyReqDescr_t *req, RF4CE_NWK_UpdateKeyConfParams_t *conf);

/**//**
 * \brief RF4CE NWK NLME-UPDATE-KEY.request type.
 * \ingroup RF4CE_NWK_UpdateKeyReq
 */
typedef struct _RF4CE_NWK_UpdateKeyReqDescr_t
{
#ifndef _HOST_
    SYS_QueueElement_t queueElement;             /*!< Service field. */
#else
    void *context;
#endif /* _HOST_ */
    RF4CE_NWK_UpdateKeyReqParams_t params;    /*!< Request data filled by the sender. */
    RF4CE_NWK_UpdateKeyConfCallback_t callback;  /*!< Callback to inform sender on the result. */
} RF4CE_NWK_UpdateKeyReqDescr_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Initiates asynchronous procedure to update the security key.
 \ingroup RF4CE_NWK_Functions

 \param[in] request - pointer to the structure that contains a pointer to the request structure.
 \return Nothing
 ****************************************************************************************/
void RF4CE_NWK_UpdateKeyReq(RF4CE_NWK_UpdateKeyReqDescr_t *request);

#endif /* _RF4CE_NWK_UPDATE_KEY_H */

/* eof bbRF4CENWKUpdateKey.h */