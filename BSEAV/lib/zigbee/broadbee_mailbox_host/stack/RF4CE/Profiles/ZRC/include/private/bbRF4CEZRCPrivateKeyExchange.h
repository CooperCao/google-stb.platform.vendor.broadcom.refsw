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
 * FILENAME: $Workfile: branches/dkiselev/ZRC2/stack/RF4CE/Profiles/GDP/include/private/bbRF4CEGDPPrivateKeyExchange.h $
 *
 * DESCRIPTION:
 *   This is the private header file for the RF4CE ZRC2 key exchange placeholder handler.
 *
 * $Revision: 2869 $
 * $Date: 2014-07-10 08:15:06Z $
 *
 ****************************************************************************************/
#ifndef _RF4CE_ZRC2_PRIVATE_KEY_EXCHANGE_H
#define _RF4CE_ZRC2_PRIVATE_KEY_EXCHANGE_H

/************************* INCLUDES ****************************************************/
#include "bbRF4CEZRCKeyExchange.h"
#include "bbRF4CEZRCConstants.h"

/************************* DEFINITIONS *************************************************/
/**//**
 * \brief RF4CE GDP Key Exchange subtype.
 */
typedef enum _RF4CE_GDP_KeyExchangeSubType_t
{
    RF4CE_GDP_KEY_EXCHANGE_SUBTYPE_CHALLENGE          = 0,
    RF4CE_GDP_KEY_EXCHANGE_SUBTYPE_CHALLENGE_RESPONSE = 1,
    RF4CE_GDP_KEY_EXCHANGE_SUBTYPE_RESPONSE           = 2,
    RF4CE_GDP_KEY_EXCHANGE_SUBTYPE_CONFIRM            = 3,
} RF4CE_GDP_KeyExchangeSubType_t;

/**//**
 * \brief RF4CE GDP Key Exchange task types.
 */
typedef enum _RF4CE_ZRC2_KeyExchangeTaskType_t
{
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_FREE = 0,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_RESPONSE,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_RESPONSE,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CONFIRM,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SEND_CONFIRM,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_REQUEST,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_SEND,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_RESPONSE_SEND,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_CHALLENGE_REQUEST,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_CHALLENGE_RESPONSE,
    RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_RESPONSE
} RF4CE_ZRC2_KeyExchangeTaskType_t;

/**//**
 * \brief RF4CE Key Exchange Client Side test
 *
 * Note: there are server side state (for your information only):
 *      RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE
 *      RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_SEND
 *      RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_CHALLENGE_RESPONSE
 *      RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_RESPONSE
 *      RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SEND_CONFIRM
 */
#define RF4CE_ZRC2_KEY_EXCHANGE_IS_CLIENT(state)                            \
    (RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_REQUEST == state ||         \
     RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_CHALLENGE_REQUEST == state || \
     RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CHALLENGE_RESPONSE == state ||        \
     RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_RESPONSE_SEND == state ||             \
     RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_SENDING_RESPONSE == state ||          \
     RF4CE_ZRC2_KEY_EXCHANGE_TASKTYPE_CONFIRM == state)

/**//**
 * \brief RF4CE GDP Key Exchange M size.
 */
#define RF4CE_ZRC2_KEY_EXCHANGE_M_SIZE 10

/**//**
 * \brief RF4CE GDP Security Key Half Length.
 */
#define RF4CE_SECURITY_KEY_HALF_LENGTH (RF4CE_SECURITY_KEY_LENGTH >> 1)

#define RF4CE_ZRC2_KEY_EXCHANGE_MESSAGE "RF4CE GDP"

/************************* TYPES *******************************************************/
/**//**
 * \brief The GDP Frame Header. // TODO Move it
 */
typedef uint8_t RF4CE_GDP_FrameHeader_t;

/**//**
 * \brief RF4CE GDP Key Exchange frame header.
 */
typedef struct PACKED _RF4CE_GDP_KeyExchangeFrameHeader_t
{
    RF4CE_GDP_FrameHeader_t frameControl;
    uint8_t subType;
} RF4CE_GDP_KeyExchangeFrameHeader_t;

/**//**
 * \brief RF4CE GDP Key Exchange Challenge frame.
 */
typedef struct PACKED _RF4CE_GDP_KeyExchangeChallengeFrame_t
{
    RF4CE_GDP_KeyExchangeFrameHeader_t header;
    uint16_t flags;
    uint8_t randA[RF4CE_SECURITY_KEY_HALF_LENGTH];
} RF4CE_GDP_KeyExchangeChallengeFrame_t;

/**//**
 * \brief RF4CE GDP Key Exchange Challenge Response frame.
 */
typedef struct PACKED _RF4CE_GDP_KeyExchangeChallengeResponseFrame_t
{
    RF4CE_GDP_KeyExchangeFrameHeader_t header;
    uint16_t flags;
    uint8_t randB[RF4CE_SECURITY_KEY_HALF_LENGTH];
    uint32_t tagB;
} RF4CE_GDP_KeyExchangeChallengeResponseFrame_t;

/**//**
 * \brief RF4CE GDP Key Exchange Response frame.
 */
typedef struct PACKED _RF4CE_GDP_KeyExchangeResponseFrame_t
{
    RF4CE_GDP_KeyExchangeFrameHeader_t header;
    uint32_t tagA;
} RF4CE_GDP_KeyExchangeResponseFrame_t;

/**//**
 * \brief RF4CE GDP Key Exchange Confirm frame.
 */
typedef struct PACKED _RF4CE_GDP_KeyExchangeConfirmFrame_t
{
    RF4CE_GDP_KeyExchangeFrameHeader_t header;
} RF4CE_GDP_KeyExchangeConfirmFrame_t;

/**//**
 * \brief RF4CE GDP Key Exchange message.
 */
typedef struct PACKED _RF4CE_ZRC2_KeyExchangeMessage_t
{
    uint8_t txtMessage[9];
    uint64_t dstAddr;
    uint64_t srcAddr;
    uint8_t key[RF4CE_SECURITY_KEY_LENGTH];
} RF4CE_ZRC2_KeyExchangeMessage_t;

/************************* FUNCTIONS PROTOTYPES ****************************************/
/************************************************************************************//**
 \brief Processes the input GDP key exchange data request packet.

 \param[in] indication - pointer to the indication structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_ZRC2_NWK_KeyExchangeIndication(RF4CE_NWK_DataIndParams_t *indication);

// TODO Description
void rf4ceZrc2KeyExchangeClearInitialKey(uint8_t pair);

#endif /* _RF4CE_ZRC2_PRIVATE_KEY_EXCHANGE_H */
