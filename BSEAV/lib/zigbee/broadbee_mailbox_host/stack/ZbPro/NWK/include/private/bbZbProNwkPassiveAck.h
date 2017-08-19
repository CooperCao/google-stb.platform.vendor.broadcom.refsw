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
*       Network Passive Ack definitions and declaration.
*
*******************************************************************************/

#ifndef _BB_ZB_PRO_NWK_PASSIVE_ACK_H
#define _BB_ZB_PRO_NWK_PASSIVE_ACK_H

/************************* INCLUDES *****************************************************/
#include "bbZbProNwkConfig.h"
#include "private/bbZbProNwkFrame.h"
#include "private/bbZbProNwkCommonPrivate.h"

/************************* DEFINITIONS **************************************************/

#define NWK_PASSIVE_ACK_DESCR_INITIALIZER { \
                                              .queue = SYS_QUEUE_INITIALIZER \
                                          }


/**//**
 * \brief In this structure the node stores information about passive acks
 *        (rebroadcasts) that were received from neighbors.
 */
typedef struct _ZbProNwkPassiveAckEntry_t
{
    SYS_QueueElement_t        queueElement;                         /*!< Service field. */
    ZBPRO_NWK_NwkAddr_t       orgAddr;                              /*!< Short address of an originator of
                                                                         a broadcast transmission. */
    ZbProNwkSequenceNumber_t  seqNum;                               /*!< NWK packet sequence number. */
    BITMAP_DECLARE(map, ZBPRO_NWK_NEIGHBOR_TABLE_SIZE);             /*!< A bitmap which contain information
                                                                         about received passive acks.
                                                                         Each bit of this mask corresponds
                                                                         to one neighbor in neighbor table. */
} ZbProNwkPassiveAckEntry_t;

/**//**
 * \brief Internal memory of Passive Ack module.
 */
typedef struct _ZbProNwkPassiveAckDescr_t
{
    SYS_QueueDescriptor_t       queue;      /*!< Queue of active Passive Acks. */
} ZbProNwkPassiveAckDescr_t;

/************************* FUNCTION PROTOTYPES ******************************************/
/************************************************************************************//**
    \brief Initializes a Passive Ack structure.
    \param[in] entry - pointer to the Passive Ack entry.
    \param[in] prevHopAddr - network address of the previous hop.
    \param[in] orgAddr - network address of the originator of the frame.
    \param[in] seqNum - sequence number of the incoming packet.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkInitPassiveAck(ZbProNwkPassiveAckEntry_t *const entry,
                                        const ZBPRO_NWK_NwkAddr_t prevHopAddr,
                                        const ZBPRO_NWK_NwkAddr_t orgAddr,
                                        const ZbProNwkSequenceNumber_t seqNum);

/************************************************************************************//**
    \brief Updates the Passive Ack structure with the received broadcast parameters.
    \param[in] prevHopAddr - network address of the previous hop.
    \param[in] orgAddr - network address of the originator of the frame.
    \param[in] seqNum - sequence number of the incoming packet.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkPassiveAckReceived(const ZBPRO_NWK_NwkAddr_t prevHopAddr,
                                            const ZBPRO_NWK_NwkAddr_t orgAddr,
                                            const ZbProNwkSequenceNumber_t seqNum);

/************************************************************************************//**
    \brief Checks that all expected rebroadcast were received.
    \param[in] entry - pointer to the Passive Ack entry.
    \return 'true' if all expected neighbors received original broadcast packet
             otherwise 'false'.
 ***************************************************************************************/
NWK_PRIVATE bool zbProNwkIsPassiveAckDone(ZbProNwkPassiveAckEntry_t *const entry);

/************************************************************************************//**
    \brief Removes passive ack from handling.
    \param[in] entry - pointer to the Passive Ack entry.
 ***************************************************************************************/
NWK_PRIVATE void zbProNwkRemovePassiveAck(ZbProNwkPassiveAckEntry_t *const entry);

/************************************************************************************//**
    \brief Passive Ack handler reset routine
****************************************************************************************/
NWK_PRIVATE void zbProNwkPassiveAckReset(void);

#endif /* _BB_ZB_PRO_NWK_TX_STATUS_H */

/* eof bbZbProNwkPassiveAck.h */
