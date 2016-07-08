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
 * FILENAME: $Workfile: trunk/stack/RF4CE/NWK/include/bbRF4CENWKEncryptDecrypt.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE Network Layer component encryption handler.
 *
 * $Revision: 1980 $
 * $Date: 2014-03-31 08:52:48Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_NWK_ENCRYPT_DECRYPT_H
#define _RF4CE_NWK_ENCRYPT_DECRYPT_H

/************************* INCLUDES ****************************************************/
#include "bbSysTypes.h"
#include "bbRF4CENWKConstants.h"
#include "bbRF4CENWKEnums.h"
#include "bbSysMemMan.h"
#include "bbRF4CEFrame.h"
#include "bbMacSapForRF4CE.h"
#include "bbSecurity.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief Errors enumeration for data decryption.
 */
typedef enum _RF4CE_DecryptError_t
{
    RF4CE_DECRYPT_SUCCESS = 0,
    RF4CE_DECRYPT_ERROR_RECOVERABLE = 1,
    RF4CE_DECRYPT_ERROR_IRRECOVERABLE
} RF4CE_DecryptError_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Encrypts the outgoing packet.

 \param[in] payload - pointer to the payload to be encrypted.
 \param[in] callback - pointer to the callback function.
 \param[in] dstAddress - destination IEEE address.
 \param[in] key - pointer to the security key.
 \return true if encryption has been successfully started, false otherwise.
 ****************************************************************************************/
bool RF4CE_EncryptRequest(SYS_DataPointer_t *payload, Security_Callback_t callback, uint64_t dstAddress, uint8_t *key);

/************************************************************************************//**
 \brief Decrypts the incoming packet.

 \param[in] indication - pointer to the indication data.
 \param[in] callback - pointer to the callback function.
 \param[in] srcAddress - source IEEE address.
 \param[in] key - pointer to the security key.
 \return 0 on success, 1 - recoverable error, 2 - other error.
 ****************************************************************************************/
RF4CE_DecryptError_t RF4CE_DecryptIndication(MAC_DataIndParams_t *indication, Security_Callback_t callback, uint64_t srcAddress, uint8_t *key);

#endif /* _RF4CE_NWK_ENCRYPT_DECRYPT_H */