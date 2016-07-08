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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   MAC Real Time Service Flowcharts interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_MAC_RTS_FLOWCHARTS_H
#define _BB_MAC_RTS_FLOWCHARTS_H

/************************* INCLUDES ***********************************************************************************/
#include "bbHalRadio.h"

/************************* DEFINITIONS ********************************************************************************/
/**//**
 * \brief   Enumeration of MAC RTS Data Confirmation statuses.
 */
enum MRTS_FC__Status_t {                                                                                                   // TODO: Move to definitions.
    MRTS_SUCCESS                    = 0x00,     /*!< Frame was transmitted successfully. */
    MRTS_CHANNEL_ACCESS_FAILURE     = 0xE1,     /*!< Channel access failure occurred during CSMA-CA. */
    MRTS_NO_ACK                     = 0xE9,     /*!< ACK was not received after attempt to transmit. */
};

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   Number of additional attempts to transmit frame if ACK was not received on the first attempt to transmit.
 * \details This value correspond to the macMaxFrameRetries attribute.
 */
extern uint8_t MRTS_FC__TX_Retries;                                                                                         // TODO: Think, how to make it a parameter of the MRTS_FC__DATA_req. // Eliminate after splitting retries and TX.

/**//**
 * \brief   Combined confirmation status reported by the MAC RTS.
 * \details This value correspond to the Status field of conventional MAC confirmation primitive with restriction to the
 *  scope of MAC RTS (Data, ED scan, Channel switch services).
 * \details As soon as MAC RTS may not be requested with two or more requests simultaneously, and each request is
 *  confirmed by the MAC RTS with exactly one confirmation per request, there is only one combined confirmation callback
 *  function and one stored status (the status of the last executed and being confirmed request of any type).
 */
extern enum MRTS_FC__Status_t  MRTS_FC__CONF_Status;

/**//**
 * \brief   Variable for saving result of Energy Detection scan (ED) over single channel.
 * \note    Flowcharts prohibit requesting two or more concurrent processes. Due to this reason results published by
 *  different processes may be stored in the shared memory space - i.e., in a union.
 */
extern union MRTS_FC__CONF_Params_t {
    uint32_t    edLevel;    /*!< The saved ED level for internal use by the ED scan flowchart, hardware-specific format.
                                During the ED scan period this value is updated after each single ED detection performed
                                by the PHY, and it keeps either the maximum observed ED level or the sum of all single
                                ED measurements (according to the ED scan configuration: maximum or average). */
} MRTS_FC__CONF_Params;

/**//**
 * \brief   Initializes MAC RTS Flowcharts.
 * \details This function must be called once on the application startup.
 */
void MRTS_FC__Init(void);

/**//**
 * \brief   Resets MAC RTS.
 * \note    This request has no confirmation.
 */
void MRTS_FC__RESET_req(void);

/**//**
 * \brief   Initiates MPDU transmission.
 */
void MRTS_FC__DATA_req(void);

/**//**
 * \brief   Handles confirmation of MPDU transmission.
 * \details This function must be provided by the higher-level layer. It will be called by MAC RTS when it finished
 *  transmission transaction. This function is called by the MAC-FE Dispatcher in the main execution context.
 */
extern void MRTS_FC__DATA_conf(void);

/**//**
 * \brief   Handles indication of received MPDU.
 * \details This function must be provided by the higher-level layer. It is called by the MAC-FE Dispatcher in the main
 *  execution context when corresponding task is fetched for execution.
 */
extern void MRTS_FC__DATA_ind(void);

/**//**
 * \brief   Initiates an Energy Detection (ED) scan by the MAC-LE.
 * \param[in]   duration    Duration of the ED Scan to be performed, in symbol fractions.
 */
void MRTS_FC__ED_req(const HAL_Symbol__Tshift_t duration);

/**//**
 * \brief   Initiates a Channel Switching by the MAC-LE.
 * \param[in]   pgch    The new page and channel to switch the transceiver to.
 */
void MRTS_FC__CHANNEL_req(const PHY_PageChannel_t pgch);

/**//**
 * \brief   Assigns the transceiver mode that will be set each time MAC becomes idle.
 * \param[in]   trxOnWhenIdle   TRUE means that MAC shall keep the radio in the RX_ON state while it's idle; FALSE means
 *  that MAC shall turn the radio to TRX_OFF for idle periods.
 * \note    This request has no confirmation.
 */
void MRTS_FC__MODE_req(const Bool8_t trxOnWhenIdle);

#endif /* _BB_MAC_RTS_FLOWCHARTS_H */
