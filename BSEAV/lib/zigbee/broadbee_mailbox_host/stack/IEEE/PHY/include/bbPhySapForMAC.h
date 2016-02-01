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
* FILENAME: $Workfile: trunk/stack/IEEE/PHY/include/bbPhySapForMAC.h $
*
* DESCRIPTION:
*   PHY-SAP interface for the MAC layer.
*
* $Revision: 3159 $
* $Date: 2014-08-05 19:11:02Z $
*
*****************************************************************************************/


#ifndef _BB_PHY_SAP_FOR_MAC_H
#define _BB_PHY_SAP_FOR_MAC_H


/************************* INCLUDES *****************************************************/
#include "bbPhySapPib.h"            /* PHY-PIB for PHY-SAP definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   PSDU length data type (the PPDU.PHR field data type), in octets.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, table 16.
 */
typedef uint8_t  PHY_PsduLength_t;


/**//**
 * \brief   Single octet data type.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.3, table 16.
 */
typedef uint8_t  PHY_Octet_t;


/**//**
 * \brief   Data type for the \c ppduLinkQuality (LQI) parameter returned by the
 *  PD-DATA.indication.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.3.1, table 8.
 */
typedef uint8_t  PHY_Lqi_t;


/**//**
 * \brief   Data type for the \c EnergyLevel parameter returned by the PLME-ED.confirm.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.4.1, table 11.
 */
typedef uint8_t  PHY_Ed_t;


/**//**
 * \brief   Value of the \c EnergyLevel parameter of the PLME-ED.confirm for the case when
 *  an unsuccessful \c status is returned.
 */
#define PHY_ED_CONF_UNSUCCESSFUL_VALUE  0xFF


/**//**
 * \brief   Enumeration for the \c timed parameter of the PD-DATA.request.
 * \details This enumeration is used for the non-standard parameter \c timed of the
 *  PD-DATA.request to instruct the PHY whether to start a requested transmission
 *  immediately, or after a specified delay (i.e., to perform timed transmission).
 */
typedef enum _PHY_DataReqTimed_t
{
    PHY_DATA_REQ_IMMEDIATE = 0,     /*!< Start transmission immediately. */

    PHY_DATA_REQ_TIMED     = 1,     /*!< Perform timed transmission. */

} PHY_DataReqTimed_t;


/**//**
 * \brief   Enumeration for the \c state parameter of the PLME-SET-TRX-STATE.request.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.8.1, table 15.
 */
typedef enum _PHY_SetTrxStateCmd_t
{
    PHY_CMD_FORCE_TRX_OFF = PHY_FORCE_TRX_OFF,      /*!< The transceiver is to be switched off immediately. */

    PHY_CMD_TRX_OFF       = PHY_TRX_OFF,            /*!< The transceiver is to be configured into the
                                                        transceiver disabled state (deferrable). */

    PHY_CMD_RX_ON         = PHY_RX_ON,              /*!< The transceiver is to be configured into the
                                                        receiver enabled state (deferrable). */

    PHY_CMD_TX_ON         = PHY_TX_ON,              /*!< The transceiver is to be configured into the
                                                        transmitter enabled state immediately. */
} PHY_SetTrxStateCmd_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   The PD-DATA.request primitive requests the transfer of an MPDU (i.e., PSDU)
 *  from the MAC sublayer to the local PHY entity.
 * \param[in]   timed       TRUE to perform timed transmission; FALSE to perform immediate
 *  transmittion.
 * \param[in]   startTime   The timestamp according to the Symbol Timer Counter, in symbol
 *  quotients, at which to start a timed transmission. This parameter has no effect if
 *  \p timed is set to FALSE (i.e., for the case of immediate transmission).
 * \note
 *  Standard parameters \c psduLength and \c psdu are omitted because PSDU assignment to
 *  the Radio hardware is performed by the MAC-LE directly by calling the HAL functions.
 * \note
 *  Non-standard parameters \p timed and \p startTime are used to instruct the PHY to
 *  perform immediate or timed transmission. This functionality is used by the MAC-LE to
 *  appoint transmission of an ACK frame.
 * \note
 *  In the case of timed transmission the HAL validates the value of the \p startTime
 *  parameter respectively to the current timestamp. If the \p startTime is too close to
 *  the current timestamp (i.e., if there is a potential possibility for the hardware to
 *  overrun the desired timestamp without commencing the transmission), or if the
 *  \p startTime is already in the past to the current timestamp, the transmission will be
 *  started with a least possible delay from the current moment. Be aware of assigning the
 *  \p startTime with values that makes <tt>(startTime - current_timestamp)<\tt> greater
 *  then 0x7FFFFFFF (if calculated as 32-bit unsigned integer), because in this case the
 *  timeshift of \p startTime respectively to the current time is considered to be
 *  negative as signed integer (i.e., the moment is in the past).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.1.
*****************************************************************************************/
void PHY_DataReq(const PHY_DataReqTimed_t timed, const HAL_SymbolTimestamp_t startTime);


/*************************************************************************************//**
 * \beief   The PD-DATA.confirm primitive confirms the end of the transmission of an MPDU
 *  (i.e., PSDU) from a local PHY entity to a peer PHY entity.
 * \param[in]   status      The result of the request to transmit a packet. This parameter
 *  may take one of the following values: SUCCESS, TRX_OFF, RX_ON, BUSY_TX.
 * \param[in]   startTime   The timestamp according to the Symbol Timer Counter, in symbol
 *  quotients, at which the transmission being confirmed has actually started. This
 *  parameter is assigned with zero and shall be ignored if \p status is not equal to
 *  SUCCESS.
 * \param[in]   endTime     The timestamp according to the Symbol Timer Counter, in symbol
 *  quotients, at which the transmission being confirmed has actually ended. This
 *  parameter is assigned with zero and shall be ignored if \p status is not equal to
 *  SUCCESS.
 * \note
 *  Non-standard parameters \p startTime and \p endTime are used by the MAC for the
 *  following:
 *  - \p startTime - is used by the MAC-FE for assigning the \c timestamp parameter of the
 *    MCPS-DATA.confirm;
 *  - \p endTime - is used by the MAC-LE for starting the Interframe Spacing timer and, in
 *    the case of an acknowledged transmission, for starting the Waiting for ACK timer.
 *
 * \details
 *  The moment of transmission start, contained in \p startTime, is bound to the onset of
 *  the PPDU.PHR field (i.e., to the fallout of the PPDU.SFR field) of the transmitted
 *  packet.
 * \details
 *  The moment of transmission end, contained in \p endTime, is bound to the fallout of
 *  the last symbol of the PPDU of the transmitted packet.
 * \details
 *  The MAC-LE shall provide implementation for this callback-handler. The PHY calls this
 *  function to signal the end of transmission of a packet or to issue an unsuccessful
 *  confirmation.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.2.
*****************************************************************************************/
PHY_PUBLIC void PHY_DataConf(const PHY_Status_t           status,
                             const HAL_SymbolTimestamp_t  startTime,
                             const HAL_SymbolTimestamp_t  endTime);


/*************************************************************************************//**
 * \beief   The PD-DATA.indication primitive indicates the transfer of an MPDU (i.e.,
 *  PSDU) from the PHY to the local MAC sublayer entity.
 * \param[in]   startTime   The timestamp according to the Symbol Timer Counter, in symbol
 *  quotients, at which the reception being indicated has actually started.
 * \param[in]   endTime     The timestamp according to the Symbol Timer Counter, in symbol
 *  quotients, at which the reception being indicated has actually ended.
 * \note
 *  Standard parameters \c psduLength, \c psdu, and \c ppduLinkQuality are omitted because
 *  PSDU and corresponding LQI are read out by the MAC-LE directly by calling the HAL
 *  functions.
 * \note
 *  Non-standard parameters \p startTime and \p endTime are used by the MAC for the
 *  following:
 *  - \p startTime - is used by the MAC-FE for assigning the \c timestamp parameter of the
 *    MCPS-DATA.indication and the MLME-POLL.indication;
 *  - \p endTime - is used by the MAC-LE for starting the Interframe Spacing timer in the
 *    case of the ACK frame reception on a previously transmitted frame, and for
 *    appointing the ACK frame transmission in the case of reception of a frame with
 *    Acknowledgment Request subfield set.
 *
 * \details
 *  The moment of reception start, contained in \p startTime, is bound to the onset of the
 *  PPDU.PHR field (i.e., to the fallout of the PPDU.SFR field) of the received packet.
 * \details
 *  The moment of reception end, contained in \p endTime, is bound to the fallout of  the
 *  last symbol of the PPDU of the received packet.
 * \details
 *  The MAC-LE shall provide implementation for this callback-handler. The PHY calls this
 *  function to signal the end of reception of a new packet.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.1.3.
*****************************************************************************************/
PHY_PUBLIC void PHY_DataInd(const HAL_SymbolTimestamp_t startTime, const HAL_SymbolTimestamp_t endTime);


/*************************************************************************************//**
 * \beief   The PLME-CCA.request primitive requests that the PLME perform a CCA.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.1, 6.9.9.
*****************************************************************************************/
PHY_PUBLIC void PHY_CcaReq(void);


/*************************************************************************************//**
 * \beief   The PLME-CCA.confirm primitive reports the results of a CCA.
 * \param[in]   status      The result of the request to perform a CCA. This parameter may
 *  take one of the following values: TRX_OFF, BUSY, IDLE.
 * \details
 *  The MAC-LE shall provide implementation for this callback-handler. The PHY calls this
 *  function to signal the end of a single CCA cycle or to issue an unsuccessful
 *  confirmation.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.2, 6.9.9.
*****************************************************************************************/
PHY_PUBLIC void PHY_CcaConf(const PHY_Status_t status);


#if defined(_ZBPRO_) || defined(RF4CE_TARGET)
/*************************************************************************************//**
 * \beief   The PLME-ED.request primitive requests that the PLME perform an ED
 *  measurement.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.3, 6.9.7.
*****************************************************************************************/
PHY_PUBLIC void PHY_EdReq(void);


/*************************************************************************************//**
 * \beief   The PLME-ED.confirm primitive reports the results of the ED measurement.
 * \param[in]   status          The result of the request to perform an ED measurement.
 *  This parameter may take one of the following values: SUCCESS, TRX_OFF, TX_ON.
 * \param[in]   energyLevel     The ED level for the current channel, from 0x00 to 0xFF.
 *  If \p status is set to SUCCESS, this is the ED level for the current channel.
 *  Otherwise, the value of this parameter is set to 0xFF and shall be ignored.
 * \details
 *  The MAC-LE shall provide implementation for this callback-handler. The PHY calls this
 *  function to signal the end of a single ED measurement or to issue an unsuccessful
 *  confirmation.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclauses 6.2.2.4, 6.9.7.
*****************************************************************************************/
PHY_PUBLIC void PHY_EdConf(const PHY_Status_t status, const PHY_Ed_t energyLevel);
#endif /* _ZBPRO_ || RF4CE_TARGET */


/*************************************************************************************//**
 * \beief   The PLME-SET-TRX-STATE.request primitive requests that the PHY entity change
 *  the internal operating state of the transceiver.
 * \param[in]   command     The new state in which to configure the transceiver. This
 *  parameter may take one of the following values: FORCE_TRX_OFF, TRX_OFF, RX_ON, TX_ON.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.7.
*****************************************************************************************/
PHY_PUBLIC void PHY_SetTrxStateReq(const PHY_SetTrxStateCmd_t command);


/*************************************************************************************//**
 * \beief   The PLME-SET-TRX-STATE.confirm primitive reports the result of a request to
 *  change the internal operating state of the transceiver.
 * \param[in]   status  The result of the request to change the state of the transceiver.
 * \details
 *  The MAC-LE shall provide implementation for this callback-handler. The PHY calls this
 *  function to issue confirmation on request to switch the transceiver state.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.8.
*****************************************************************************************/
PHY_PUBLIC void PHY_SetTrxStateConf(const PHY_Status_t status);


/*************************************************************************************//**
 * \brief   The PLME-SET-CHANNEL.request primitive attempts to set the pair of PHY-PIB
 *  attributes phyCurrentChannel and phyCurrentPage to the given values.
 * \param[in]   channelOnPage   The 16-bit plain value including the identifier of a
 *  channel page (in the MSB) and the identifier of a logical channel (in the LSB).
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.9.
*****************************************************************************************/
PHY_PUBLIC void PHY_SetChannelReq(const PHY_ChannelOnPagePlain_t channelOnPage);


/*************************************************************************************//**
 * \brief   The PLME-SET-CHANNEL.confirm primitive reports the results of the attempt to
 *  set the pair of PHY-PIB attributes phyCurrentChannel and phyCurrentPage.
 * \param[in]   status  The result of the request to change the state of the transceiver.
 *  This parameter may take one of the following values: SUCCESS, INVALID_PARAMETER.
 * \par     Documentation
 *  See IEEE 802.15.4-2006, subclause 6.2.2.10.
*****************************************************************************************/
PHY_PUBLIC void PHY_SetChannelConf(const PHY_Status_t status);


#endif /* _BB_PHY_SAP_FOR_MAC_H */