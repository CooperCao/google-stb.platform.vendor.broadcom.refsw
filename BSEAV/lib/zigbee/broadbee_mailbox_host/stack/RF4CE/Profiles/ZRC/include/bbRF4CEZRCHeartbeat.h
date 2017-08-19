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
*       RF4CE GDP 2.0 Heartbeat command processor interface.
*
*******************************************************************************/

#ifndef _BB_RF4CE_ZRC_HEARTBEAT_H
#define _BB_RF4CE_ZRC_HEARTBEAT_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCPollService.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the Heartbeat GDP command indication.
 * \ingroup RF4CE_GDP2_HeartbeatInd
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 7.5.1, table 18.
 */
typedef struct _RF4CE_GDP2_HeartbeatIndParams_t
{
    /* 8-bit data. */
    RF4CE_GDP2_PollingTriggerId_t  pollingTriggerId;        /*!< Identifier of the fired Polling Trigger. */
    uint8_t                        pairingRef;              /*!< Pairing reference of the linked Poll Client node. */
} RF4CE_GDP2_HeartbeatIndParams_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Notifies the application layer of the Poll Server about reception of a
 *  Heartbeat GDP command from a linked Poll Client.
 * \ingroup RF4CE_ZRC_Functions
 * \param[in]   indParams       Pointer to the indication parameters object.
 * \details
 *  This callback function shall be provided by the application of a device that is
 *  capable to act as a Poll Server. It will be called by the GDP layer during Heartbeat
 *  polling being performed by a Poll Client on each reception of the Heartbeat GDP
 *  command if this node is capable to act as a Poll Server. The originator node (the Poll
 *  Client) pairing reference and the fired trigger identifier are reported in the
 *  indication parameters \p indParams.
*****************************************************************************************/
void RF4CE_GDP2_HeartbeatInd(RF4CE_GDP2_HeartbeatIndParams_t *const indParams);


#endif /* _BB_RF4CE_ZRC_HEARTBEAT_H */

/* EOF bbRF4CEZRCHeartbeat.h */