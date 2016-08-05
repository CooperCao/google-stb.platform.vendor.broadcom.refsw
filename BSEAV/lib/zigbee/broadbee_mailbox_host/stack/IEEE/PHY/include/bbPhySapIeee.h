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
*   PHY-SAP interface according to IEEE Std 802.15.4-2006.
*
* $Revision$
* $Date$
*
*****************************************************************************************/

#ifndef _BB_PHY_SAP_IEEE_H
#define _BB_PHY_SAP_IEEE_H

/************************* VALIDATIONS ********************************************************************************/
#if !defined(_PHY_SAP_IEEE_)
# error This header shall be compiled only for SoC_phyTest project.
#endif

/************************* INCLUDES ***********************************************************************************/
#include "bbPhySapForMac.h"

/************************* PROTOTYPES *********************************************************************************/
/**//**
 * \brief   The PD-DATA.request primitive requests the transfer of an MPDU (i.e., PSDU) from the MAC sublayer to the
 *  local PHY entity.
 * \note    Standard parameters \c psduLength and \c psdu are omitted because PSDU assignment to the Radio hardware is
 *  performed by the MAC directly by calling the HAL functions.
 * \note    PHY performs immediate transmission on this request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.1.
 */
void PHY_DataReq(void);

/**//**
 * \brief   The PD-DATA.request primitive requests the transfer of an MAC Acknowledgment (ACK) frame MPDU (i.e., PSDU)
 *  from the MAC sublayer to the local PHY entity.
 * \note    Standard parameters \c psduLength and \c psdu are omitted because PSDU assignment to the Radio hardware is
 *  performed by the MAC directly by calling the HAL functions.
 * \note    PHY performs timed transmission on this request. The ACK frame transmission is started in 12.375 symbols
 *  from the current moment.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.1.
 */
void PHY_AckReq(void);

/**//**
 * \brief   The PD-DATA.confirm primitive confirms the end of the transmission of an MPDU (i.e., PSDU) from a local PHY
 *  entity to a peer PHY entity.
 * \param[in]   status      The result of the request to transmit a packet. This parameter may take one of the following
 *  values: SUCCESS, TRX_OFF, RX_ON, BUSY_TX, and BUSY.
 * \param[in]   startTime   The timestamp according to the Symbol Timer Counter, in symbol quotients, at which the
 *  transmission being confirmed has actually started. This parameter is assigned with zero and shall be ignored if
 *  \p status is not equal to SUCCESS.
 * \param[in]   endTime     The timestamp according to the Symbol Timer Counter, in symbol quotients, at which the
 *  transmission being confirmed has actually ended. This parameter is assigned with zero and shall be ignored if
 *  \p status is not equal to SUCCESS.
 * \note    Non-standard parameters \p startTime and \p endTime are used by the MAC for the following:
 *  - \p startTime - is used by the MAC for assigning the \c timestamp parameter of the MCPS-DATA.confirm;
 *  - \p endTime - is used by the MAC for starting the Interframe Spacing timer and, in the case of an acknowledged
 *      transmission, for starting the Waiting for ACK timer.
 *
 * \details The moment of transmission start, contained in \p startTime, is bound to the onset of the PPDU.PHR field
 *  (i.e., to the cutoff of the PPDU.SFR field) of the transmitted packet.
 * \details The moment of transmission end, contained in \p endTime, is bound to the cutoff of the last symbol of the
 *  PPDU of the transmitted packet.
 * \details The MAC shall provide implementation for this callback-handler. The PHY calls this function to signal the
 *  end of transmission of a packet or to issue an unsuccessful confirmation.
 * \details BUSY status is confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.2.
 */
extern void PHY_DataConf(const PHY_Status_t status,
        const HAL_Symbol__Tstamp_t startTime,
        const HAL_Symbol__Tstamp_t endTime);

/**//**
 * \brief   The PD-DATA.indication primitive indicates the transfer of an MPDU (i.e., PSDU) from the PHY to the local
 *  MAC sublayer entity.
 * \param[in]   startTime   The timestamp according to the Symbol Timer Counter, in symbol quotients, at which the
 *  reception being indicated has actually started.
 * \param[in]   endTime     The timestamp according to the Symbol Timer Counter, in symbol quotients, at which the
 *  reception being indicated has actually ended.
 * \note    Standard parameters \c psduLength, \c psdu, and \c ppduLinkQuality are omitted because PSDU and
 *  corresponding LQI are read out by the MAC directly by calling the HAL functions.
 * \note    Non-standard parameters \p startTime and \p endTime are used by the MAC for the following:
 *  - \p startTime - is used by the MAC for assigning the \c timestamp parameter of the MCPS-DATA.indication and the
 *      MLME-POLL.indication;
 *  - \p endTime - is used by the MAC for starting the Interframe Spacing timer in the case of the ACK frame reception
 *      on a previously transmitted frame, and for appointing the ACK frame transmission in the case of reception of a
 *      frame with Acknowledgment Request subfield set.
 *
 * \details The moment of reception start, contained in \p startTime, is bound to the onset of the PPDU.PHR field (i.e.,
 *  to the cutoff of the PPDU.SFR field) of the received packet.
 * \details The moment of reception end, contained in \p endTime, is bound to the cutoff of the last symbol of the PPDU
 *  of the received packet.
 * \details The MAC shall provide implementation for this callback-handler. The PHY calls this function to signal the
 *  end of reception of a new packet.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.3.
 */
extern void PHY_DataInd(const HAL_Symbol__Tstamp_t startTime, const HAL_Symbol__Tstamp_t endTime);

/**//**
 * \brief   The PLME-SET-TRX-STATE.request primitive requests that the PHY entity change the internal operating state of
 *  the transceiver.
 * \param[in]   command     The new state in which to configure the transceiver. This parameter may take one of the
 *  following values: FORCE_TRX_OFF, TRX_OFF, RX_ON, TX_ON, BUSY.
 * \details BUSY status is confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.7.
 */
void PHY_SetTrxStateReq(const enum PHY_Cmd_t command);

/**//**
 * \brief   The PLME-SET-TRX-STATE.confirm primitive reports the result of a request to change the internal operating
 *  state of the transceiver.
 * \param[in]   status  The result of the request to change the state of the transceiver.
 * \details The MAC shall provide implementation for this callback-handler. The PHY calls this function to issue
 *  confirmation on request to switch the transceiver state.
 * \details BUSY status is confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.8.
 */
extern void PHY_SetTrxStateConf(const PHY_Status_t status);

/**//**
 * \brief   The PLME-CCA.request primitive requests that the PLME perform a CCA.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.1, 6.9.9.
 */
void PHY_CcaReq(void);

/**//**
 * \brief   The PLME-CCA.confirm primitive reports the results of a CCA.
 * \param[in]   status      The result of the request to perform a CCA. This parameter may take one of the following
 *  values: TRX_OFF, BUSY, IDLE.
 * \details The MAC shall provide implementation for this callback-handler. The PHY calls this function to signal the
 *  end of a single CCA cycle or to issue an unsuccessful confirmation.
 * \details BUSY status may also be confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.2, 6.9.9.
 */
extern void PHY_CcaConf(const PHY_Status_t status);

/**//**
 * \brief   The PLME-ED.request primitive requests that the PLME perform an ED measurement.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.3, 6.9.7.
 */
void PHY_EdReq(void);

/**//**
 * \beief   The PLME-ED.confirm primitive reports the results of the ED measurement.
 * \param[in]   status          The result of the request to perform an ED measurement. This parameter may take one of
 *  the following values: SUCCESS, TRX_OFF, TX_ON, BUSY.
 * \param[in]   energyLevel     The ED level for the current channel, from 0x00 to 0xFF. If \p status is set to SUCCESS,
 *  this is the ED level for the current channel. Otherwise, the value of this parameter is set to 0xFF and shall be
 *  ignored.
 * \details The MAC shall provide implementation for this callback-handler. The PHY calls this function to signal the
 *  end of a single ED measurement or to issue an unsuccessful confirmation.
 * \details BUSY status is confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.4, 6.9.7.
 */
extern void PHY_EdConf(const PHY_Status_t status, const PHY_ED_t energyLevel);

/**//**
 * \brief   The PLME-SET-CHANNEL.request primitive attempts to set the pair of PHY-PIB attributes phyCurrentPage and
 *  phyCurrentChannel to the given values.
 * \param[in]   pgch    The 8-bit packed value of channel page and logical channel identifiers.
 * \details The \p pgch represents particular channel on particular channel page in the following format:
 *  - bits 4..0 - Channel[4..0] - indicate the channel in the range from 0 to 26
 *  - bits 7..5 - Page[2..0] - indicate the channel page in the range from 0 to 7
 *
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.9.
 */
void PHY_SetChannelReq(const PHY_PageChannel_t pgch);

/**//**
 * \brief   The PLME-SET-CHANNEL.confirm primitive reports the results of the attempt to set the pair of PHY-PIB
 *  attributes phyCurrentPage and phyCurrentChannel.
 * \param[in]   status  The result of the request to change the state of the transceiver. This parameter may take one of
 *  the following values: SUCCESS, INVALID_PARAMETER, BUSY.
 * \details BUSY status is confirmed if Radio Driver is currently busy with processing of a previous request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.10.
 */
extern void PHY_SetChannelConf(const PHY_Status_t status);

#endif /* _BB_PHY_SAP_IEEE_H */
