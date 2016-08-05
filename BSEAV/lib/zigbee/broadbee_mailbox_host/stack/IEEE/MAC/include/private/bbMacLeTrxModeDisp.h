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
*
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacLeTrxModeDisp.h $
*
* DESCRIPTION:
*   MAC-LE Transceiver Mode Dispatcher interface.
*
* $Revision: 10263 $
* $Date: 2016-02-29 18:03:06Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_TRX_MODE_DISP_H
#define _BB_MAC_LE_TRX_MODE_DISP_H


/************************* INCLUDES *****************************************************/
#include "bbMacBasics.h"            /* Basic MAC set. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration for the \c command parameter of request to the MAC-LE Transceiver
 *  Mode Dispatcher.
 * \details This enumeration is used by the MAC-LE Transceiver Mode Dispatcher to
 *  distinguish requests to switch the transceiver state received from different units of
 *  the MAC:
 *  - persistent assignments to hold transceiver in RX_ON state or to drop it into TRX_OFF
 *    state when idle are originated by the MAC-PIB on changes in the macRxOnWhenIdle
 *    attribute;
 *  - timed commands to hold transceiver in RX_ON state when idle for the specified period
 *    of time are originated by the MLME-RX-ENABLE.request Processor;
 *  - timed commands to left transceiver in RX_ON state for the period determined by the
 *    MAC-PIB attribute macMaxFrameTotalWaitTime are originated by the MAC-LE Real-Time
 *    Dispatcher when the ACK frame with FramePending subfield set to one is received on
 *    the Data Request MAC Command frame (this is used only for ZigBee PRO context).
 */
typedef enum _MacLeTrxModeCmd_t
{
    MAC_TRX_MODE_CMD_PERSIST_OFF = 0,       /*!< Persistent command to drop transceiver into TRX_OFF state when idle
                                                from the MAC-PIB attribute macRxOnWhenIdle. */

    MAC_TRX_MODE_CMD_PERSIST_ON  = 1,       /*!< Persistent command to hold transceiver in RX_ON state when idle from
                                                the MAC-PIB attribute macRxOnWhenIdle. */

    MAC_TRX_MODE_CMD_TIMED_REQ   = 2,       /*!< Timed command from the MLME-RX-ENABLE.request. */

#if defined(_MAC_CONTEXT_ZBPRO_)
    MAC_TRX_MODE_CMD_TIMED_FSM   = 3,       /*!< Timed command from the MAC-LE Real-Time Dispatcher FSM. */
#endif

} MacLeTrxModeCmd_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts the specified command to switch the transceiver mode when idle.
 * \param[in]   __givenContextId    Identifier of the specified MAC Context.
 * \param[in]   command             Command to switch the transceiver mode when idle:
 *  - PERSIST_OFF   persistent command to drop transceiver into TRX_OFF state when idle
 *                  from the MAC-PIB attribute macRxOnWhenIdle,
 *  - PERSIST_ON    persistent command to hold transceiver in RX_ON state when idle from
                    the MAC-PIB attribute macRxOnWhenIdle,
 *  - TIMED_REQ     timed command from the MLME-RX-ENABLE.request,
 *  - TIMED_FSM     timed command from the MAC-LE Real-Time Dispatcher FSM.
 *
 * \param[in]   timeshift           Timeshift to hold transceiver in the RX_ON state when
 *  idle for the TIMED_REQ command type, in symbol quotients. This parameter is omitted
 *  and ignored for commands of other types.
 * \details
 *  Call this function to assign new transceiver mode for the idle state.
 * \note
 *  This function shall be called either in the context of interrupt request or from an
 *  atomic section.
*****************************************************************************************/
MAC_PRIVATE void macLeTrxModeDispAcceptCmd(MAC_WITH_GIVEN_CONTEXT(
                                           const MacLeTrxModeCmd_t     command,
                                           const HAL_Symbol__Tshift_t  timeshift));


#endif /* _BB_MAC_LE_TRX_MODE_DISP_H */
