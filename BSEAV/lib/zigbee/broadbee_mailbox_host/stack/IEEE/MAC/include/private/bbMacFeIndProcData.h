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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacFeIndProcData.h $
*
* DESCRIPTION:
*   MCPS-DATA.indication Processor interface.
*
* $Revision: 2722 $
* $Date: 2014-06-24 19:37:15Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_FE_IND_PROC_DATA_H
#define _BB_MAC_FE_IND_PROC_DATA_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the received Data frame MPDU Surrogate and issues the
 *  MCPS-DATA.indication to the ZigBee PRO and/or the RF4CE higher layers if needed.
 * \param[in]   mpduSurr    Pointer to the MPDU surrogate of the received Data frame.
 * \details
 *  This function is called by the MAC-FE Indications Dispatcher in the case of Data frame
 *  reception. The received frame is already filtered for MAC contexts and deserialized
 *  into the structured form of MPDU Surrogate. This function constructs the indication
 *  parameters structured object and issues the MCPS-DATA.indication to the ZigBee PRO
 *  and/or the RF4CE stacks higher layers according to results of frame filtering for MAC
 *  contexts and current MAC contexts states.
 * \details
 *  If a MAC context that was originally included by the MAC-LE Frame Parser into the set
 *  of destination contexts is currently performing Scanning, it is excluded from the set
 *  of destination contexts.
 * \details
 *  In the case when both MAC contexts shall issue indications to their higher layers, the
 *  indication parameters object and the MSDU payload of the received Data frame are
 *  duplicated in order to provide both Stacks with their own private copies of indication
 *  parameters and payloads.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 7.1.1.3, 7.5.2.1.1, 7.5.2.1.2.
*****************************************************************************************/
MAC_PRIVATE void macFeIndProcDataAcceptFrame(MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_FE_IND_PROC_DATA_H */