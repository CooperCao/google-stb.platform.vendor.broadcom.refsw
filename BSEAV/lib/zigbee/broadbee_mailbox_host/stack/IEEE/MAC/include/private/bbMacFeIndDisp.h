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
 *      MAC-FE Indications Dispatcher interface.
 *
*******************************************************************************/

#ifndef _BB_MAC_FE_IND_DISP_H
#define _BB_MAC_FE_IND_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Dispatches the received MAC Frame indicated by the MAC-LE.
 * \param[in]   mpduSurr    Pointer to the MPDU Surrogate of the received MAC frame.
 * \details
 *  When called, routes the specified MPDU Surrogate to the corresponding MAC-FE
 *  Indication Processor according to the MPDU Surrogate Identifier (i.e., the received
 *  MAC Frame type and MAC Command identifier).
 * \details
 *  The called MAC-FE Indication Processor is responsible for subsequent analyzing of the
 *  received frame parameters and its indication to the higher layer(-s) or transferring
 *  the MPDU Surrogate to the currently active MAC-FE Request Processor or to the MAC-FE
 *  Transactions Dispatcher according to the MAC layer current state and received frame
 *  parameters. The received frame switching or forking between two MAC Contexts is
 *  applied by the called MAC-FE Indication Processor according to results of destination
 *  address filtering performed by the MAC-LE Frame Parser and stored within the indicated
 *  MPDU Surrogate parameters.
 * \note
 *  For the case of dual-context MAC the MAC-LE shall assign the mask of destination MAC
 *  Contexts for the received MAC Frame according to the actual (i.e., at the moment of
 *  indication read-out from the MAC-LE) result of address filtering, and MAC Contexts
 *  enabled/disabled current states.
 * \note
 *  For the RF4CE-Controller build, only Data frames are indicated by the MAC-LE to the
 *  MAC-FE (ACK frames are processed by the MAC-LE internally and not indicated, and
 *  frames of all other types except Data frame are merely rejected), so the MAC-FE
 *  Indication Dispatcher does not analyze the MPDU Surrogate identifier in the case of
 *  RF4CE-Controller build MAC configuration, but just transfers the MPDU Surrogate to the
 *  MCPS-DATA.indication Processor.
*****************************************************************************************/
MAC_PRIVATE void macFeIndDispAcceptMpdu(MacMpduSurr_t *const mpduSurr);


#endif /* _BB_MAC_FE_IND_DISP_H */

/* eof bbMacFeIndDisp.h */