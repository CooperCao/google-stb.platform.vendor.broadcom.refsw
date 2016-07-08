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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeIndProcAssociate.h $
*
* DESCRIPTION:
*   MLME-ASSOCIATE.indication Processor interface.
*
* $Revision: 1308 $
* $Date: 2014-01-31 16:07:51Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_IND_PROC_ASSOCIATE_H
#define _BB_MAC_FE_IND_PROC_ASSOCIATE_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* VALIDATIONS **************************************************/
#if !defined(_MAC_CONTEXT_ZBPRO_)
# error This file requires the MAC Context for ZigBee PRO to be included into the build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
  \brief
    Accepts a dispatched Association Request MAC Command frame MPDU surrogate, processes
    it, and issues the MLME-ASSOCIATE.indication to the ZigBee PRO higher layer if needed.
  \param    mpduSurr
    Pointer to the MPDU surrogate of the received Association Request MAC Command frame.
  \details
    A received MPDU is filtered and deserialized into the structured form called MPDU
    surrogate by the MAC-LE, then it is routed by the MAC-FE Indications Dispatcher to
    this MAC-FE Indication Processor according to the frame type and the command
    identifier. Then this MAC-FE Indication Processor verifies that the received frame is
    addressed to the ZigBee PRO context, that it is properly constructed by its
    originator, that it is allowed to indicate received MAC Commands, and that the
    ZigBee PRO NWK layer allows association, and then instructs the MAC-FE Transactions
    Dispatcher to flush the MAC-FE Pending Transactions Queue by the received request
    originator 64-bit source address and finally constructs and issues the
    MLME-ASSOCIATE.indication to the ZigBee PRO higher layer.
  \par  Documentation
    See IEEE 802.15.4-2006, subclauses 7.1.3.2, 7.3.1, 7.5.3.1.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcAssociateReqAcceptFrame(MacMpduSurr_t *mpduSurr);


/*************************************************************************************//**
  \brief
    Accepts a dispatched Association Response MAC Command frame MPDU surrogate, processes
    it, and passes the received coordinator response to the active MLME-ASSOCIATE.request
    Processor if it is currently active.
  \param    mpduSurr
    Pointer to the MPDU surrogate of the received Association Response MAC Command frame.
  \details
    A received MPDU is filtered and deserialized into the structured form called MPDU
    surrogate by the MAC-LE, then it is routed by the MAC-FE Indications Dispatcher to
    this MAC-FE Indication Processor according to the frame type and the command
    identifier. Then this MAC-FE Indication Processor verifies that the received frame is
    addressed to the ZigBee PRO context, that it is properly constructed by its
    originator, that it is allowed to execute and/or indicate received MAC Commands, and
    that the MLME-ASSOCIATE.request Processor is currently active, and then passes the
    received coordinator response to the active MLME-ASSOCIATE.request Processor.
  \par  Documentation
    See IEEE 802.15.4-2006, subclauses 7.1.3.1, 7.1.3.3, 7.3.2, 7.5.3.1.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcAssociateRespAcceptFrame(MacMpduSurr_t *mpduSurr);


#endif /* _BB_MAC_FE_IND_PROC_ASSOCIATE_H */