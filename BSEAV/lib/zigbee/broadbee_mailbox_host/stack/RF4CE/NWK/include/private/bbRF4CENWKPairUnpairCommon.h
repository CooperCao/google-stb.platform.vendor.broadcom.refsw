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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      RF4CE NWK Pair common pair/unpair handlers.
 *
*******************************************************************************/

#ifndef _RF4CE_NWK_PAIR_UNPAIR_COMMON_H_
#define _RF4CE_NWK_PAIR_UNPAIR_COMMON_H_

/************************* INCLUDES *****************************************************/
#include "bbSysTypes.h"
#include "bbSysFsm.h"
#include "bbSysTaskScheduler.h"
#include "bbRF4CENWKPair.h"

#include "bbRF4CEFrame.h"
#include "bbRF4CENWKNIBAttributes.h"

typedef struct _RF4CE_NWK_PairingOriginatorMem_t
{
    PairResponseFrameBody_t         pairBody;
    uint8_t                         pairVarPart[RF4CE_NWK_MAX_TOTAL_VARIABLE_LENGTH];
    uint32_t                        pairingData;
    SecurityKey_t                   oldKey;
    RF4CE_NWK_PairConfParams_t      pairConfirm;
    RF4CE_NWK_PairReqDescr_t    *   pairRequest;
    SYS_FSM_StateId_t               currentFsmState;
    SYS_FSM_EventId_t               lastEventId;
    Bool8_t                         needToWaitConf;
    Bool8_t                         needToWaitResp;
} RF4CE_NWK_PairingOriginatorMem_t;


typedef struct _RF4CE_NWK_PairingRecipientMem_t
{
    RF4CE_NWK_PairIndParams_t       pairIndication;
    SYS_DataPointer_t               keySeedFrame;
    SYS_FSM_StateId_t               currentFsmState;
    RF4CE_NLDE_DATA_Status_t        commStatus;
    SYS_FSM_EventId_t               lastEventId;
    Bool8_t                         keySeedFailed;
    Bool8_t                         needToSendPingResponse;
    Bool8_t                         pingRequestRecieved;
    uint32_t                        pingRequestRandomValue;
} RF4CE_NWK_PairingRecipientMem_t;


/************************* DEFINITIONS **************************************************/
typedef void (*rf4cenwkSetRequestConfirm)(MAC_SetReqDescr_t *req, MAC_SetConfParams_t *conf);
bool rf4cenwkMultychannelCheckAndRetransmit(uint8_t destLogicalChannel, rf4cenwkSetRequestConfirm conf);

/************************************************************************************//**
 \brief Pair Request Task Handler. Handles Request queue and dispatches calls.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairUnpairRequestHandler(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);


/************************************************************************************//**
 \brief Pair Request Task Handler.
 \param request - pointer to the request
 \return true if next queued request can be processed.
 ****************************************************************************************/
bool RF4CE_NWK_PairRequestHandler(RF4CE_NWK_PairReqDescr_t *request);

/************************************************************************************//**
 \brief Handler for the any timeout during the Pairing procedure on the Originator side.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairOriginatorTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);

#ifdef RF4CE_TARGET
/************************************************************************************//**
 \brief Handler for the any timeout during the Pairing procedure on the Originator side.

 \param[in] taskDescriptor - pointer to the current task descriptor structure.
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_PairRecipientTimeout(SYS_SchedulerTaskDescriptor_t *const taskDescriptor);
#endif

/************************************************************************************//**
 \brief Unpair Request Task Handler.

 \param request - pointer to the request
 \return true if next queued request can be processed.
 ****************************************************************************************/
bool RF4CE_NWK_UnpairRequestHandler(RF4CE_NWK_UnpairReqDescr_t *request, bool isSilently);

/************************************************************************************//**
 \brief Initialize function for the Pairing FSM on the Originator side
 ****************************************************************************************/
void RF4CE_NWK_PairingOriginatorReset(void);
#ifdef RF4CE_TARGET
/************************************************************************************//**
 \brief Initialize function for the Pairing FSM on the Recipient side
 ****************************************************************************************/
void RF4CE_NWK_PairingRecipientReset(void);
#endif /* RF4CE_TARGET */

/************************************************************************************//**
 \brief Restore saved Pairing reference for the current Binding/Validation process.

 \param[in] pairRef - current pairing reference (index in NWK pairing table).
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_RestorePairRefOriginator(uint8_t pairRef);

#ifdef RF4CE_TARGET
/************************************************************************************//**
 \brief Restore saved Pairing reference for the current Binding/Validation process.

 \param[in] pairRef - current pairing reference (index in NWK pairing table).
 \return Nothing.
 ****************************************************************************************/
void RF4CE_NWK_RestorePairRefRecipient(uint8_t pairRef);
#endif /* RF4CE_TARGET */


#endif /* _RF4CE_NWK_PAIR_UNPAIR_COMMON_H_ */

/* eof bbRF4CENWKPairUnpairCommon.h */