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
 *      MLME-POLL.indication Processor interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_IND_PROC_POLL_H
#define _BB_MAC_FE_IND_PROC_POLL_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Accepts a dispatched Data Request MAC Command frame MPDU surrogate, processes it,
    schedules the pending frame transmission if any persists in the MAC-FE Transactions
    Queue, and issues the MLME-POLL.indication to the ZigBee PRO higher layer if needed.
  \param    mpduSurr
    Pointer to the MPDU surrogate of the received Data Request MAC Command frame.
  \details
    A received MPDU is filtered and deserialized into the structured form called MPDU
    surrogate by the MAC-LE, then it is routed by the MAC-FE Indications Dispatcher to
    this MAC-FE Indication Processor according to the frame type and the command
    identifier. Then this MAC-FE Indication Processor verifies that the received frame is
    addressed to the ZigBee PRO context, that it is properly constructed by its
    originator, and that it is allowed to execute and/or indicate received MAC Commands,
    and then constructs and issues the request to the MAC-FE Transactions Dispatcher and
    finally constructs and issues the MLME-POLL.indication to the ZigBee PRO higher layer.
  \note
    The MLME-POLL.indication is issued even if there is no pending data in the MAC-FE
    Transactions Queue for the received Data Request MAC Command originator.
  \note
    If there is a data pending in the MAC-FE Transactions Queue, the corresponding
    Request/Response (the first of matched ones) is moved from the head of MAC-FE
    Transactions Queue to the head of MAC-FE Main Requests Queue to be processed as soon
    as possible; then MLME-POLL.indication is issued in the same context; and only then
    in the context of MAC-FE Requests Dispatcher task the extracted Request/Response
    processing is started.
  \par  Documentation
    See IEEE 802.15.4-2006, subclauses 7.1.1.3, 7.1.16.3, 7.3.4, 7.5.3.1, 7.5.5.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcPollAcceptFrame(MacMpduSurr_t *mpduSurr);


#endif /* _BB_MAC_FE_IND_PROC_POLL_H */

/* eof bbMacFeIndProcPoll.h */