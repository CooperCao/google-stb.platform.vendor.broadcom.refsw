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
 *      PLME-SET-TRX-STATE and PLME-GET-TRX-STATE services data types definition.
 *
*******************************************************************************/

#ifndef _BB_PHY_SAP_TYPES_STATE_H
#define _BB_PHY_SAP_TYPES_STATE_H

/************************* INCLUDES ***********************************************************************************/
#include "bbPhySapDefs.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Enumeration of PHY state switching commands.
 * \note    PHY implements slightly different behavior depending on which of two PHY-SAP is used:
 *  - Simplified PHY-SAP, which is used by MAC layer, implements only three commands: RX_ON, TRX_OFF, TX_ON - and all of
 *      them act as FORCE commands. The dedicated FORCE_TRX_OFF is not supported by Simplified PHY-SAP; the TRX_OFF code
 *      shall be used both for deferrable TRX_OFF and for FORCE_TRX_OFF. Deferring of commands is not supported. Also,
 *      not all state transitions (i.e., combinations of current and target state) are allowed. The list of allowed
 *      transitions is given in description of the RF-SET-TRX-STATE primitive. In the case of failure execution of
 *      application is halted.
 *  - IEEE-compliant PHY-SAP, which may be used by a different project, implements all four commands: RX_ON, TRX_OFF,
 *      TX_ON, and FORCE_TRX_OFF. The first three commands act as deferrable commands, and only FORCE_TRX_OFF delivers
 *      forced action. Deferring, if necessary, is performed (simulated) by means of the PHY layer software. All state
 *      transitions are formally supported; and in the case of error one of failure statuses is confirmed (execution of
 *      application is not halted).
 *
 * \details The PLME-SET-TRX-STATE.request, as it is implemented in this PHY, accepts reduced set of commands. Indeed,
 *  all three implemented commands are performed as FORCE commands. Due to this reason there are no separate commands
 *  FORCE_TRX_OFF (0x03) and TRX_OFF (0x08) - the TRX_OFF (0x08) shall be used in both cases.
 * \note    There are additional restrictions which commands may be issued to the PHY when it persists in particular
 *  states.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.7, table 14.
 */
enum PHY_Cmd_t {
    PHY_CMD__FORCE_TRX_OFF  = PHY__FORCE_TRX_OFF,       /*!< The transceiver is to be switched off immediately. Not
                                                            implemented in the Simplified PHY-SAP. */
    PHY_CMD__RX_ON          = PHY__RX_ON,               /*!< The transceiver is to be configured into the receiver
                                                            enabled state. State is forced in the Simplified PHY-SAP;
                                                            command may be deferred in the IEEE-compliant PHY-SAP. */
    PHY_CMD__TRX_OFF        = PHY__TRX_OFF,             /*!< The transceiver is to be configured into the transceiver
                                                            disabled state. State is forced in the Simplified PHY-SAP;
                                                            command may be deferred in the IEEE-compliant PHY-SAP. */
    PHY_CMD__TX_ON          = PHY__TX_ON,               /*!< The transceiver is to be configured into the transmitter
                                                            enabled stated. State is forced in the Simplified PHY-SAP;
                                                            command may be deferred in the IEEE-compliant PHY-SAP. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_Cmd_t) == 1);
SYS_DbgAssertStatic(PHY_CMD__RX_ON == HAL_RADIO_CMD__RX_ON);
SYS_DbgAssertStatic(PHY_CMD__TRX_OFF == HAL_RADIO_CMD__TRX_OFF);
SYS_DbgAssertStatic(PHY_CMD__TX_ON == HAL_RADIO_CMD__TX_ON);

/*--------------------------------------------------------------------------------------------------------------------*/
/**//**
 * \brief   Enumeration of PHY state codes.
 * \details This PHY implements custom PLME-GET-TRX-STATE primitive that returns the current state of the PHY. The PHY
 *  state code returned may be one of conventional PHY codes or the UNDEFINED (0x00) state.
 * \note    This enumeration indicates the state of the PHY which in general correspond to the actual Radio hardware
 *  state. Hence, the state of the Radio hardware may differ during short periods from the PHY state until the PHY state
 *  is synchronized with the hardware state after corresponding Radio hardware interrupt servicing.
 * \note    Implementations on particular platforms may extend this enumeration with private codes - for example, for
 *  different phases of BUSY states (preparation and continuing), or different types of UNDEFINED state (initially
 *  undefined and in the middle of state switching).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.3, table 18.
 */
enum PHY_State_t {
    PHY_STATE__UNDEFINED    = PHY__BUSY,        /*!< The transceiver is currently switching its state. */
    PHY_STATE__BUSY_RX      = PHY__BUSY_RX,     /*!< The transceiver is currently receiving. */
    PHY_STATE__BUSY_TX      = PHY__BUSY_TX,     /*!< The transceiver is currently transmitting. */
    PHY_STATE__RX_ON        = PHY__RX_ON,       /*!< The transceiver is in the receiver enabled state. */
    PHY_STATE__TRX_OFF      = PHY__TRX_OFF,     /*!< The transceiver is in the transceiver disabled state. */
    PHY_STATE__TX_ON        = PHY__TX_ON,       /*!< The transceiver is in the transmitter enabled state. */
};
SYS_DbgAssertStatic(sizeof(enum PHY_State_t) == 1);
SYS_DbgAssertStatic(PHY_STATE__UNDEFINED == HAL_RADIO_STATE__UNDEFINED);
SYS_DbgAssertStatic(PHY_STATE__BUSY_RX == HAL_RADIO_STATE__BUSY_RX);
SYS_DbgAssertStatic(PHY_STATE__BUSY_TX == HAL_RADIO_STATE__BUSY_TX);
SYS_DbgAssertStatic(PHY_STATE__RX_ON == HAL_RADIO_STATE__RX_ON);
SYS_DbgAssertStatic(PHY_STATE__TRX_OFF == HAL_RADIO_STATE__TRX_OFF);
SYS_DbgAssertStatic(PHY_STATE__TX_ON == HAL_RADIO_STATE__TX_ON);

#endif /* _BB_PHY_SAP_TYPES_STATE_H */

/* eof bbPhySapTypesState.h */