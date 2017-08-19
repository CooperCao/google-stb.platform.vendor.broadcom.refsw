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
 *      Friend interface of the APS key pair table.
 *
*******************************************************************************/

#ifndef _ZBPRO_APS_KEY_PAIR_FRIEND_H
#define _ZBPRO_APS_KEY_PAIR_FRIEND_H

/************************* INCLUDES ****************************************************/
#include "bbZbProSsp.h"
#include "bbZbProApsCommon.h"

/**************************** DEFINITIONS **********************************************/
/**//**
 * \brief Address field values of the Key Pair table which have special handling
 */
/* address specifies that this key is global and for the TC and other devices */
#define ZBPRO_APS_KEYPAIR_SPECKEYADDR_TC_GLOBAL             ZBPRO_APS_EXT_ADDRESS_ALL
/* address specifies that this key is an unique TC key but the TC address is not known at this moment */
#define ZBPRO_APS_KEYPAIR_SPECKEYADDR_TC_UNIQUE_UNRESOLVED  ZBPRO_APS_EXT_ADDRESS_ALL_RX_ON_WHEN_IDLE

/**//**
 * \brief Returns true if keyRef refers to a global key
 */
#define ZBPRO_APS_IS_GLOBAL_KEY(keyRef) (ZBPRO_APS_KEYPAIR_SPECKEYADDR_TC_GLOBAL == (keyRef)->deviceAddr)

/**//**
 * \brief Returns true if keyRef refers to an unresolved TC key
 */
#define ZBPRO_APS_IS_UNRESOLVED_TC_KEY(keyRef) \
    (ZBPRO_APS_KEYPAIR_SPECKEYADDR_TC_UNIQUE_UNRESOLVED == (keyRef)->deviceAddr)

/**************************** TYPES ****************************************************/
/**//**
 * \brief Key pair handle type definition.
 */
typedef uint16_t ZBPRO_APS_KeyPairHandle_t;

/**//**
 * \brief Key pair table entry definition.
 */
typedef struct _ZbProApsKeyPairReference_t
{
    ZbProSspKey_t               linkKey;
    ZBPRO_APS_ExtAddr_t         deviceAddr;
    ZbProSspFrameCnt_t          outFrameCnt;
    ZbProSspFrameCnt_t          inFrameCnt;
    ZBPRO_APS_KeyPairHandle_t   handle;
} ZbProApsKeyPairReference_t;


/************************************************************************************//**
    \brief Updates or Creates key pair reference.
    \param[in] addr - pointer to the Extended address.
    \param[in] newKeyValue - pointer to new Link Key value.
    \return Pointer to the Key Pair reference.
****************************************************************************************/
ZbProApsKeyPairReference_t *zbProApsIbUpdateKeyPair(const ZBPRO_APS_ExtAddr_t *const addr,
        const ZbProSspKey_t *const newKeyValue);

/************************************************************************************//**
    \brief Returns an appropriate key pair reference from key pair table.
    \param[in] addr - pointer to the Extended address.
    \return Pointer to the Key Pair reference.
****************************************************************************************/
ZbProApsKeyPairReference_t *zbProApsIbGetKeyPairByAddr(const ZBPRO_APS_ExtAddr_t *const addr);

/************************************************************************************//**
    \brief Returns key pair reference from key pair table with the specified address
    \param[in] addr - pointer to the Extended address.
    \return Pointer to the Key Pair reference.
****************************************************************************************/
ZbProApsKeyPairReference_t *zbProApsIbGetKeyPair(const ZBPRO_APS_ExtAddr_t *const addr);

/************************************************************************************//**
    \brief Returns next GLOBAL key pair reference from key pair table.
    \param[in] currentKeyRef - pointer to the current Key Pair reference.
    \return Pointer to the next Key Pair reference.
****************************************************************************************/
ZbProApsKeyPairReference_t *zbProApsIbGetNextGlobalKeyPair(ZbProApsKeyPairReference_t *currentKeyRef);

/************************************************************************************//**
    \brief Updates unresolved TC address Key Pair with the specified address
    \param[in] addr - extended address of the TC
****************************************************************************************/
void zbProApsIbKeyPairUpdateTcAddress(const ZBPRO_APS_ExtAddr_t addr);

#endif /* _ZBPRO_APS_KEY_PAIR_FRIEND_H */

/* eof bbZbProApsKeyPairFriend.h */
