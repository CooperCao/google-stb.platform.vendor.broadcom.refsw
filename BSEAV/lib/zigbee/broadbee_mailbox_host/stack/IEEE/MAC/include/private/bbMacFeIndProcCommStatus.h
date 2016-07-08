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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   MLME-COMM-STATUS.indication Processor interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_IND_PROC_COMM_STATUS_H
#define _BB_MAC_FE_IND_PROC_COMM_STATUS_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_) && !defined(MAILBOX_UNIT_TEST)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the MPDU Surrogate of the received MAC Secured frame and issues the
 *  MLME-COMM-STATUS.indication to the ZigBee PRO and/or RF4CE higher layers.
 * \param[in]   mpduSurr    Pointer to the MPDU surrogate of the received Data frame.
 * \details
 *  This function is called by the MAC-FE Indications Dispatcher in the case of MAC
 *  Secured frame reception. The received frame is already filtered for MAC contexts and
 *  deserialized into the structured form of MPDU Surrogate. This function constructs the
 *  indication parameters structured object and issues the MLME-COMM-STATUS.indication to
 *  the ZigBee PRO and/or RF4CE stacks higher layers according to results of frame
 *  filtering for MAC contexts and current MAC contexts states.
 * \note
 *  For the case when the MLME-COMM-STATUS.indication is issued following the
 *  MLME-ASSOCIATE.response or the MLME-ORPHAN.response the callback function specified in
 *  the response descriptor is called (just as a kind of confirmation on response) instead
 *  of issuing common MLME-COMM-STATUS.indication.
 * \details
 *  If a MAC context that was originally included by the MAC-LE Frame Parser into the set
 *  of destination contexts is currently performing Scanning, it is excluded from the set
 *  of destination contexts, and the MLME-COMM-STATUS.indication is not issued to it.
 * \details
 *  In the case when both MAC contexts shall issue indications to their higher layers, the
 *  indication parameters object of the received MAC Secured frame is duplicated in order
 *  to provide each Stack with its own private copy of indication parameters.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.12.1, 7.5.2.1.1, 7.5.2.1.2.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcCommStatusAcceptFrame(MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_FE_IND_PROC_COMM_STATUS_H */