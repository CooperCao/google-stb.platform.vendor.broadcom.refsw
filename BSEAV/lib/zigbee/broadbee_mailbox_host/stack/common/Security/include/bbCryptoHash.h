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
* FILENAME: $Workfile: branches/ext_xhuajun/MailboxIntegration/stack/common/Security/include/bbCryptoHash.h $
*
* DESCRIPTION:
*   Cryptographic hash function.
*
* $Revision: 1832 $
* $Date: 2014-03-19 07:10:11Z $
*
*****************************************************************************************/


#ifndef _BB_CRYPTO_HASH_H
#define _BB_CRYPTO_HASH_H


/************************* INCLUDES *****************************************************/
#include "bbSysBasics.h"            /* Basic system environment set. */
#include "bbSysMemMan.h"            /* Memory manager. */
#include "bbSysPayload.h"            /* Payload. */

/************************* DEFINITIONS **************************************************/
/**//**
 * \brief Hash size.
 */
#define SECURITY_AES_MMO_HASH_SIZE         16

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Starts hash generation.

 \param[in] payload - pointer to the requested payload to be hashed.
 \param[in] hash - pointer to the buffer of SECURITY_AES_MMO_HASH_SIZE bytes which
                   receives calculated hash.
 \return Nothing.
 ****************************************************************************************/
void Security_CbbHash(SYS_DataPointer_t *payload, uint8_t *hash);

/************************************************************************************//**
 \brief Starts hash generation.

 \param[in] payload - pointer to the requested payload to be hashed.
 \param[in] payloadSize - size of the payload to be hashed.
 \param[in] hash - pointer to the buffer of SECURITY_AES_MMO_HASH_SIZE bytes which
                   receives calculated hash.
 \return Nothing.
 ****************************************************************************************/
void Security_CbbHashVoid(void *payload, uint32_t payloadSize, uint8_t *hash);

/************************************************************************************//**
 \brief Keyed hash function for message authentication.

 \param[in, out] key - pointer to the input and output key.
 \param[in] m - the value.
 \return Nothing.
 ****************************************************************************************/
void Security_CbbHashForMessageAuth(uint8_t *key, uint8_t m);

#endif /* _BB_CRYPTO_HASH_H */