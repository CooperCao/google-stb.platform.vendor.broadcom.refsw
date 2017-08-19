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
*       ZDO Frequency Agility friend interface.
*
*******************************************************************************/

#ifndef _BB_ZBPRO_ZDO_FREQUENCY_AGILITY_H
#define _BB_ZBPRO_ZDO_FREQUENCY_AGILITY_H


/************************* INCLUDES *****************************************************/
#include "bbZbProNwkSapTypesIb.h"


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Handles event raised by NWK TX FSM on MCPS-DATA.confirm on unicast data
 *  transmission.
 * \param[in]   nwkTxTotal      Actual value of the nwkTxTotal attribute.
 * \details
 *  This function is called by the NWK TX FSM on each MCPS-DATA.confirm received on
 *  completed unicast transmission.
 * \details
 *  The 'unicast' is considered in terms of the MAC layer: (1) unicast 16-bit destination
 *  address (i.e., not 0xFFFF) or other addressing mode, and (2) unicast PAN Id (i.e.,
 *  also not 0xFFFF).
 * \details
 *  All the necessary NWK layer counters are updated to the moment of this function call.
 *  Namely, personal transmission failures counter of the destination neighbor is already
 *  incremented in the case of failure status returned by MAC, and the total NWK
 *  transmissions counter is also incremented. Consequently, this function may perform
 *  evaluation of transmissions error rate based on the NWK IB attributes.
 * \note
 *  This function shall return fast. To support this condition, this function:
 *  - must return if the \p nwkTxTotal is less then or equal to 20 (the threshold);
 *  - must not perform evaluation of transmissions error rate each time it is called.
 *      Instead of this, it shall perform calculations only if the following two
 *      conditions are met: (1) the specified period has elapsed from the moment of the
 *      last calculation that had come with positive result ('positive' means that the
 *      transmissions error rate is greater than the threshold), and (2) this function was
 *      called sufficient number of times since the moment of the last calculation ended
 *      with negative result, in order to allow the failure counters to reach the
 *      necessary minimum level to provide positive result of the error rate calculation;
 *  - it must not issue requests to the Stack directly. Instead of this, it shall schedule
 *      task-event with ZDO priority which handler will issue all the necessary requests
 *      to the Stack.
 * \par     Documentation
 *  See ZigBee Document 053474r20, annex E.
 */
void zbProZdoFrequencyAgilityMacDataConfHandler(const ZBPRO_NWK_NIB_TxTotal_t nwkTxTotal,
                                                const ZBPRO_NWK_NIB_TxTotal_t nwkTxFailures);


#endif /* _BB_ZBPRO_ZDO_FREQUENCY_AGILITY_H */

/* eof bbZbProZdoFrequencyAgility.h */