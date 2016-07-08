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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeIndProcBeacon.h $
*
* DESCRIPTION:
*   MLME-BEACON-NOTIFY.indication Processor interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_IND_PROC_BEACON_H
#define _BB_MAC_FE_IND_PROC_BEACON_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* VALIDATIONS **************************************************/
#if defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
# error This header is not for the RF4CE-Controller build.
#endif


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts a dispatched Beacon frame MPDU Surrogate, processes it, passes the
 *  received Beacon to the active MLME-SCAN.request Processor if it is currently active
 *  (in the case of the Active Scan only), and issues the MLME-BEACON-NOTIFY.indication to
 *  ZigBee PRO and/or RF4CE higher layers if needed.
 * \param[in]   mpduSurr    Pointer to the MPDU Surrogate of the received Beacon frame.
 * \details
 *  A received MAC Frame is filtered and deserialized into the structured form called MPDU
 *  Surrogate by the MAC-LE, then it is routed by the MAC-FE Indications Dispatcher to
 *  this MAC-FE Indication Processor according to the MAC Frame type. Then this MAC-FE
 *  Indication Processor discovers which of two MAC Contexts, ZigBee PRO and/or RF4CE,
 *  shall process this Beacon, forks the beacon payload if the Beacon must be processed by
 *  both contexts, verifies that the frame is properly constructed by its originator, and
 *  that it is allowed to indicate received Beacon frames, and then constructs and issues
 *  the MLME-BEACON-NOTIFY.indication to the ZigBee PRO and/or RF4CE higher layers. If
 *  there is an Active Scan being performed by one of the MAC Contexts, the received
 *  Beacon is delivered also to the currently active MLME-SCAN.request Processor just
 *  before issuing the MLME-BEACON-NOTIFY.indication.
 * \note
 *  In the case when the received Beacon frame may be delivered to both MAC Contexts (and
 *  they are both enabled), the MLME-BEACON-NOTIFY.indication primitive is issued for the
 *  ZigBee PRO Stack first and only then for the RF4CE Stack, but in the context of the
 *  same task.
 * \par  Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.5.1, 7.2.2.1, 7.5.1.1, 7.5.2.1.2, 7.5.4.1.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcBeaconAcceptFrame(MacMpduSurr_t *const mpduSurr);


/*************************************************************************************//**
 * \brief   Accepts a dispatched Beacon Request MAC Command frame MPDU Surrogate,
 *  processes it, and schedules Beacons transmission by currently enabled MAC Contexts.
 * \param[in]   mpduSurr    Pointer to the MPDU surrogate of the received Beacon Request
 *  MAC Command frame. This parameter is provided only in two cases: (1) for the
 *  dual-context MAC, and (2) for Debug build.
 * \details
 *  A received MAC Frame is filtered and deserialized into the structured form called MPDU
 *  Surrogate by the MAC-LE, then it is routed by the MAC-FE Indications Dispatcher to
 *  this MAC-FE Indication Processor according to the MAC Frame type and the MAC Command
 *  identifier. Then this MAC-FE Indication Processor discovers which of two MAC Contexts,
 *  ZigBee PRO and/or RF4CE, shall process this Beacon Request, verifies that the frame is
 *  properly constructed by its originator, and that it is allowed to execute received MAC
 *  Commands, and then instructs the MAC-FE Requests Dispatcher to schedule beacons
 *  transmission by MAC Contexts for ZigBee PRO and/or RF4CE.
 * \par  Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.3.7, 7.5.1.1, 7.5.2.1.
*****************************************************************************************/
#if defined(_MAC_DUAL_CONTEXT_) || defined(_DEBUG_)
MAC_PRIVATE void macFeIndProcBeaconReqAcceptFrame(MacMpduSurr_t *const mpduSurr);
#else
MAC_PRIVATE void macFeIndProcBeaconReqAcceptFrame(void);
#endif


#endif /* _BB_MAC_FE_IND_PROC_BEACON_H */