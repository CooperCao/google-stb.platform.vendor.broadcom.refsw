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
* FILENAME: $Workfile: trunk/stack/IEEE/MAC/include/private/bbMacLeRealTimeDisp.h $
*
* DESCRIPTION:
*   MAC-LE Real-Time Dispatcher interface.
*
* $Revision: 3455 $
* $Date: 2014-09-04 07:56:49Z $
*
*****************************************************************************************/


#ifndef _BB_MAC_LE_REAL_TIME_DISP_H
#define _BB_MAC_LE_REAL_TIME_DISP_H


/************************* INCLUDES *****************************************************/
#include "private/bbMacMpdu.h"      /* MAC MPDU definitions. */


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration for the \c trxModeWhenIdle parameter of the
 *  MAC-LE-SET-TRX-MODE.request.
 */
typedef enum _MacTrxModeWhenIdle_t
{
    MAC_TRX_MODE_TRX_OFF_WHEN_IDLE = FALSE,     /*!< Code for the TRX_OFF_WHEN_IDLE mode of the MAC-LE. */

    MAC_TRX_MODE_TRX_ON_WHEN_IDLE  = TRUE,      /*!< Code for the TRX_ON_WHEN_IDLE mode of the MAC-LE. */

} MacTrxModeWhenIdle_t;


/**//**
 * \brief   Structure for parameter of the MAC-LE-COMPLETE.response.
 */
typedef struct _MacLeRealTimeDispCompleteRespParams_t
{
    /* 32-bit data. */
    HAL_SymbolTimestamp_t  txStartTime;     /*!< Last PPDU transmission start timestamp. */

    /* 8-bit data. */
    MAC_Status_t           status;          /*!< Confirmation status. */

#if defined(_MAC_CONTEXT_ZBPRO_)
    MacMpduFramePending_t  ackWithFp;       /*!< Value of the received ACK frame FramePending subfield. */
#endif

#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
    PHY_Ed_t               edMaxValue;      /*!< Maximum value of the EnergyLevel measured. */
#endif

} MacLeRealTimeDispCompleteRespParams_t;


/**//**
 * \brief   Union for the structured parameters returned by the MAC-LE on the
 *  MAC-LE-COMPLETE.response, or the MPDU Surrogate structured object constructed by the
 *  MAC-LE during processing of the MAC-LE-RECEIVE.indication.
 */
typedef union _MacLeReturnedParams_t
{
    MacLeRealTimeDispCompleteRespParams_t  completeParams;      /*!< Parameters of the MAC-LE-COMPLETE.response. */

    MacMpduSurr_t                          rxIndMpduSurr;       /*!< MPDU Surrogate constructed within the
                                                                    MAC-LE-RECEIVE.indication. */
} MacLeReturnedParams_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Processes the MAC-LE-RESET.request from the MAC-FE.
 * \details
 *  Call this function to immediately switch (force) the MAC-LE into the TRX_OFF state.
 * \details
 *  This command is used to reset the MAC-LE, the PHY and the Radio HAL. It terminates all
 *  commenced activities and recalls the MAC-LE-RECEIVE.indication if it was issued to the
 *  MAC-FE but still was not processed by it. It also resets the MAC-LE to the
 *  RX_OFF_WHEN_IDLE mode.
 * \details
 *  This request has no confirmation and performed immediately.
 * \note
 *  The MAC-FE shall not issue this command if Transmission or ED Scan was started or
 *  Channel Switching was requested but has not been completed by the MAC-LE yet (i.e.,
 *  there was no MAC-LE-COMPLETE.indication from the MAC-LE to the MAC-FE).
 * \note
 *  There is the MAC-LE Transceiver Mode Resolver unit between the MAC-FE and the MAC-LE
 *  that is responsible for generating MAC-LE-RESET.request to the MAC-LE on the base of
 *  previously received commands from two MAC-FE contexts (in the case of dual-context
 *  MAC). The MAC-LE and the Radio Hardware Transceiver will not be actually reset and
 *  switched to TRX_OFF if there are two enabled MAC contexts and both of them need the
 *  Radio to be enabled when idle; RESET will be actually performed only in the case when
 *  a single context is enabled or both contexts enabled but only one or none of them
 *  needs the Radio to be in RX_ON when idle.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispResetReq(void);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-SET-TRX-MODE.request from the MAC-FE.
 * \param[in]   trxOnWhenIdle   Set to TRUE to switch the MAC-LE into the TRX_ON_WHEN_IDLE
 *  mode; set to FALSE to switch the MAC-LE into the TRX_OFF_WHEN_IDLE mode.
 * \details
 *  Call this function to instruct the MAC-LE to stay in the RX_ON or in the TRX_OFF state
 *  when it is idle (i.e., after Transmission, after ED Scan, after Channel Switching and
 *  after Indication of the last received PPDU).
 * \details
 *  If the MAC-LE is currently idle but in the different state it will be switched between
 *  RX_ON and TRX_OFF immediately according to the specified argument \p trxOnWhenIdle. In
 *  all other cases (i.e., when the MAC-LE is busy) the MAC-LE mode will be switched only
 *  on completion of the activity being currently performed.
 * \details
 *  This request has no confirmation. It is considered to be performed immediately, even
 *  if the MAC-LE state switching was deferred due to some activity on the MAC-LE, because
 *  this request switches not the current MAC-LE state but its mode for the idle state.
 * \note
 *  The MAC-LE state RX_ON is not equal completely to the transceiver RX_ON state. If the
 *  transceiver is currently in the BUSY_RX state (i.e., is receiving a new PPDU, but
 *  reception is not completed yet), the MAC-LE will be in the RX_ON state (i.e., it will
 *  be currently idle). Consequently the command to switch the MAC-LE to the
 *  TRX_OFF_WHEN_IDLE will be performed immediately and the PPDU being currently received
 *  will be discarded without acknowledgment and indication.
 * \note
 *  There is the MAC-LE Transceiver Mode Dispatcher unit between the MAC-FE and the MAC-LE
 *  that is responsible for generating MAC-LE-SET-TRX-MODE.request to the MAC-LE on the
 *  base of previously received commands from two MAC-FE contexts (in the case of
 *  dual-context MAC). The MAC-LE and the Radio Hardware Transceiver will stay in RX_ON
 *  mode when idle if at least one of two contexts needs the Radio to stay enabled when
 *  idle; and the MAC-LE and the Radio will be switched to TRX_OFF mode when idle only if
 *  both MAC contexts do not need the Radio to be enabled when idle.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispSetTrxModeReq(const MacTrxModeWhenIdle_t trxModeWhenIdle);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-RECEIVE.indication to the MAC-FE.
 * \details
 *  The MAC-LE calls this callback function provided by the MAC-FE when the MAC-LE has
 *  finished a new PPDU reception and acknowledging, and is ready to transfer the PPDU to
 *  the MAC-FE.
 * \details
 *  This function issues the RECEIVE signal to the MAC-FE to schedule the corresponding
 *  task to perform reading-out and parsing of the received PPDU and its indication to the
 *  higher layer or the Currently Active MAC-FE Request/Response Processor, and to
 *  finalize reception with the MAC-LE-RECEIVE.response to the MAC-LE.
 * \note
 *  The MAC-FE shall issue the MAC-LE-RECEIVE.response in order to free the MAC-LE from
 *  BUSY_RX state and to reenable the Radio Hardware Receiver.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispReceiveInd(void);


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Processes the MAC-LE-RECEIVE-BEACON.indication to the MAC-FE.
 * \details
 *  The MAC-LE calls this callback function provided by the MAC-FE when the MAC-LE has
 *  finished a new beacon reception, and is ready to transfer its PPDU to the MAC-FE.
 * \details
 *  This function is called instead of the MAC-LE-RECEIVE.indication just in the case when
 *  a beacon frame (note: only with empty beacon payload and simple 4-byte superframe
 *  descriptor) is received.
 * \details
 *  This function copies the received beacon PPDU from the pre-read software buffer into
 *  the the multy-frame software buffer for further processing by the MAC-FE and issues
 *  the BUFFER signal to the MAC-FE to schedule the corresponding task to perform
 *  reading-out and parsing of the received beacon and its indication to the higher layer
 *  and to the MLME-SCAN.request Processor if it is currently active.
 * \note
 *  For this indication unlike the MAC-LE-RECEIVE.indication the MAC-FE shall not issue
 *  the MAC-LE-RECEIVE.response after the received beacon processing, because the Radio
 *  Hardware Receiver was not locked.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispReceiveBeaconInd(void);
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-RECALL.indication to the MAC-FE.
 * \details
 *  The MAC-LE calls this callback function provided by the MAC-FE when the MAC-LE is
 *  performing the MAC-LE-RESET.request to recall the previously signaled PPDU reception
 *  if it was not processed by the MAC-FE yet.
 * \details
 *  This function recalls the RECEIVE signal from the MAC-FE to drop the last received
 *  PPDU. The MAC-LE-RECEIVE.response shall not be issued by the MAC-FE in this case.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispRecallInd(void);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-RECEIVE.response from the MAC-FE.
 * \param[out]  mpduSurr    Pointer to the structured MPDU Surrogate object provided by
 *  the MAC-FE to be filled by the MAC-LE with parameters of the received PPDU:
 *  - reception start timestamp of the received PPDU, in symbol quotients,
 *  - identifier of the logical channel at which the PPDU was received,
 *  - identifier of the channel page at which the PPDU was received.
 *
 *  This parameter is allowed to be NULL. In such a case no parameters of the received
 *  PPDU are transferred to the MAC-FE. The NULL value is specified by the MAC-FE in the
 *  case when it decides to reject the received PPDU without indication.
 * \details
 *  Call this function as response on the MAC-LE-RECEIVE.indication to request the MAC-LE
 *  to release the Frame RX Buffer for new reception and to transfer the start timestamp
 *  of the received PPDU and identifiers of the logical channel and the channel page on
 *  which the PPDU was received via the specified parameter pointed with the \p mpduSurr.
 * \details
 *  When processing the RECEIVE task scheduled by the MAC-LE-RECEIVE.indication, the
 *  MAC-FE Time-Task Dispatcher shall call the MAC-LE Frame Parser in order to perform
 *  reading-out of the received PPDU from the Frame RX Buffer, deserializing of the MPDU
 *  and filtering the received MPDU according to its format and address, then call this
 *  function to issue the MAC-LE-RECEIVE.response to the MAC-LE, and finally pass obtained
 *  MPDU Surrogate structured object (only in the case when the received MPDU was not
 *  rejected by the MAC-LE Frame Parser) to the MAC-FE Indications Dispatcher which will
 *  issue an indication to the higher layer or to the Currently Active MAC-FE
 *  Request/Response Processor.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispReceiveResp(MacMpduSurr_t *const mpduSurr);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-START-TX.request from the MAC-FE.
 * \details
 *  Call this function to request the MAC-LE to start Transmission.
 * \details
 *  If the MAC-LE is in the RX_ON state waiting for a new PPDU (i.e., the transceiver is
 *  in the RX_ON or BUSY_RX state), or in the TRX_OFF state, it will start preparing to
 *  Transmission immediately (and for the case of the BUSY_RX state it will terminate the
 *  new PPDU reception). If the MAC-LE has just received a PPDU and has issued the
 *  MAC-LE-RECEIVE.indication to the MAC-FE before getting this request, but the MAC-FE
 *  still did not perform reading-out of the received PPDU and did not issue the
 *  MAC-LE-RECEIVE.response to the MAC-LE, or the ACK frame still has not been
 *  transmitted, the MAC-LE will defer this Transmission request until the MAC-FE issues
 *  the MAC-LE-RECEIVE.response and the ACK frame is transmitted if needed.
 * \details
 *  On this request, as soon as the MAC-LE becomes idle, it will issue the
 *  MAC-LE-READY-TX.indication to the MAC-FE once to instigate the MAC-FE to compose and
 *  to write-in the PPDU to transmit and to issue the MAC-LE-READY-TX.response to the
 *  MAC-LE. Then the MAC-LE will wait for the IFS period (LIFS or SIFS), then it will
 *  perform CSMA-CA, transmission, waiting for acknowledgment, and retransmissions if
 *  needed. After that, the MAC-LE will issue the MAC-LE-COMPLETE.indication to the
 *  MAC-FE to signal the transmission completion; and if the received ACK frame has the
 *  FramePending field set to one, the MAC-LE will left the receiver in the RX_ON state
 *  automatically at least for the \c macMaxFrameTotalWaitTime period in order to wait for
 *  incoming PPDU (it is the case of the Data Request MAC Command frame). The MAC-FE will
 *  then perform the MAC-LE-COMPLETE.response with the structured object to be filled by
 *  the MAC-LE with parameters of the completed transmission:
 *  - transmission start timestamp of the last successful transmission attempt, in symbol
 *    quotients,
 *  - status of the completed transmission: SUCCESS, NO_ACK, CHANNEL_ACCESS_FAIL,
 *  - value of the FramePending field of the received ACK frame (equal to FALSE if no
 *    acknowledgment was initially requested for this transmission).
 *
 * \details
 *  This request has no confirmation, but the MAC-LE-COMPLETE.indication is issued by the
 *  MAC-LE to the MAC-FE when Transmission is completed; and results of the completed
 *  Transmission are transferred by the MAC-LE to the MAC-FE in the structured object
 *  specified by the MAC-FE in the consecutive MAC-LE-COMPLETE.response.
 * \note
 *  The MAC-FE shall not issue this command if another Transmission or an ED Scan, or
 *  Channel Switching was started but has not been completed yet.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispStartTxReq(void);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-READY-TX.indication to the MAC-FE.
 * \details
 *  The MAC-LE calls this callback function provided by the MAC-FE when the MAC-LE has
 *  finished all previous activities and now is ready to commence the requested
 *  Transmission sequence.
 * \details
 *  This function shall issue the READY_TX signal to the MAC-FE to schedule the
 *  corresponding task to perform composing and writing-in of the PPDU to be transmitted,
 *  and to specify certain parameters of the requested Transmission with the
 *  MAC-LE-READY-TX.response.
 * \note
 *  On platforms with the Radio Frame RX/TX Buffer shared between receiver and transmitter
 *  the MAC-LE Frame Composer will also store the PPDU image in the software backup buffer
 *  (only if the acknowledged transmission is being prepared with retries number greater
 *  then zero) to provide ability for the MAC-LE Real-Time Dispatcher to restore the
 *  content of the Frame TX Buffer when need to retransmit. This functionality is fully
 *  performed by the MAC-LE and therefore is transparent for the MAC-FE, i.e., the MAC-FE
 *  will not be activated (for example with repeated MAC-LE-READY-TX.indication) each time
 *  the Frame TX Buffer need to be restored for retransmission during single transaction.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispReadyTxInd(void);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-READY-TX.response from the MAC-FE.
 * \param[in]   phr         Value of the PPDU.PHR field of the PPDU to be transmitted;
 * \param[in]   ackReq      Value of the MPDU.MHR.FCF.AckRequest (AR) subfield;
 * \param[in]   seqNum      Value of the MPDU.MHR.SequenceNumber (DSN or BSN) field;
 * \param[in]   retries     Number of retransmission attempts to be performed.
 * \details
 *  Call this function as response on the MAC-LE-READY-TX.indication to request the MAC-LE
 *  to perform requested Transmission sequence with specified parameters.
 * \details
 *  When processing the READY_TX task scheduled by the MAC-LE-READY-TX.indication, the
 *  MAC-FE Time-Task Dispatcher shall construct the MPDU Surrogate structured object by
 *  calling the MPDU Surrogate Constructor assigned by the Currently Active MAC-FE
 *  Request/Response Processor, then pass this MPDU Surrogate to the MAC-LE Frame Composer
 *  in order to serialize the MPDU and perform writing-in of the PPDU to be transmitted
 *  into the Frame TX Buffer, and finally call this function.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispReadyTxResp(const PHY_PsduLength_t       phr,
                                              const MacMpduAckRequest_t    ackReq,
                                              const MAC_Dsn_t              seqNum,
                                              const MAC_MaxFrameRetries_t  retries);


#if !defined(_MAC_CONTEXT_RF4CE_CONTROLLER_)
/*************************************************************************************//**
 * \brief   Processes the MAC-LE-START-ED.request from the MAC-FE.
 * \param[in]   duration    Duration of the ED Scan to be performed, in symbol quotients.
 * \details
 *  Call this function to request the MAC-LE to start ED Scan (repeated ED measurements).
 * \details
 *  If the MAC-LE is in the RX_ON state waiting for a new PPDU (i.e., the transceiver is
 *  in the RX_ON or BUSY_RX state), or in the TRX_OFF state, it will start preparing to
 *  ED Scan immediately (and for the case of the BUSY_RX state it will terminate the new
 *  PPDU reception). If the MAC-LE has just received a PPDU and has issued the
 *  MAC-LE-RECEIVE.indication to the MAC-FE before getting this request, but the MAC-FE
 *  still did not perform reading-out of the received PPDU and did not issue the
 *  MAC-LE-RECEIVE.response to the MAC-LE, or the ACK frame still has not been
 *  transmitted, the MAC-LE will defer this ED Scan request until the MAC-FE issues the
 *  MAC-LE-RECEIVE.response and the ACK frame is transmitted if needed.
 * \details
 *  On this request, as soon as the MAC-LE becomes idle, it will switch on the Radio
 *  receiver, lock the Frame RX Buffer from new PPDUs reception and perform series of ED
 *  measurements, each for 8 symbols, for the specified scanning period. After that, the
 *  MAC-LE will issue the MAC-LE-COMPLETE.indication to the MAC-FE to signal the ED Scan
 *  completion. The MAC-FE will then perform the MAC-LE-COMPLETE.response with the
 *  structured object to be filled by the MAC-LE with parameters of the completed ED Scan:
 *  - the absolute maximum of all the EnergyLevel values measured serially each for
 *    8-symbols period during the ED Scan period.
 *
 * \details
 *  This request has no confirmation, but the MAC-LE-COMPLETE.indication is issued by the
 *  MAC-LE to the MAC-FE when ED Scan is completed; and results of the completed ED Scan
 *  are transferred by the MAC-LE to the MAC-FE in the structured object specified by the
 *  MAC-FE in the consecutive MAC-LE-COMPLETE.response.
 * \note
 *  The MAC-FE shall not issue this command if a Transmission or another ED Scan, or
 *  Channel Switching was started but has not been completed yet.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispStartEdReq(const HAL_SymbolTimeshift_t duration);
#endif /* ! _MAC_CONTEXT_RF4CE_CONTROLLER_ */


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-SET-CHANNEL.request from the MAC-FE.
 * \param[in]   channelOnPage   The 16-bit plain value of the Channel-on-Page object
 *  containing the Channel Page IEEE-index in its MSB and the Logical Channel index in its
 *  LSB to be switched to.
 * \details
 *  Call this function to request the MAC-LE to switch the channel (and the channel page).
 * \details
 *  If the MAC-LE is in the RX_ON state waiting for a new PPDU (i.e., the transceiver is
 *  in the RX_ON or BUSY_RX state), or in the TRX_OFF state, it will switch the channel
 *  immediately (and for the case of the BUSY_RX state it will naturally terminate the new
 *  PPDU reception due to CRC corruption). If the MAC-LE has just received a PPDU and has
 *  issued the MAC-LE-RECEIVE.indication to the MAC-FE before getting this request, but
 *  the MAC-FE still did not perform reading-out of the received PPDU and did not issue
 *  the MAC-LE-RECEIVE.response to the MAC-LE, or the ACK frame still has not been
 *  transmitted, the MAC-LE will defer this Channel Switch request until the MAC-FE issues
 *  the MAC-LE-RECEIVE.response and the ACK frame is transmitted if needed.
 * \details
 *  On this request, as soon as the MAC-LE becomes idle, it will validate the requested
 *  channel page and logical channel if they are actually implemented by the Radio
 *  Hardware. If the specified Channel-on-Page is not implemented, the INVALID_PARAMETER
 *  status will be confirmed to the MAC-FE. Otherwise, the MAC-LE will instigate the PHY
 *  to perform the Radio channel and channel page switching and then, when the PHY
 *  confirms switching, issue SUCCESS confirmation to the MAC-FE. In both case
 *  confirmation to the MAC-FE is issued in the following way: the MAC-LE issues the
 *  MAC-LE-COMPLETE.indication to the MAC-FE to signal the Channel Switching completion;
 *  the MAC-FE will then perform the MAC-LE-COMPLETE.response with the structured object
 *  to be filled by the MAC-LE with parameters of the completed Channel Switching:
 *  - status of the completed channel switching: SUCCESS, or INVALID_PARAMETER.
 *
 * \details
 *  This request has no confirmation, but the MAC-LE-COMPLETE.indication is issued by the
 *  MAC-LE to the MAC-FE when Channel Switching is completed; and results of the completed
 *  Channel Switching are transferred by the MAC-LE to the MAC-FE in the structured object
 *  specified by the MAC-FE in the consecutive MAC-LE-COMPLETE.response.
 * \note
 *  The MAC-FE shall not issue this command if a Transmission or an ED Scan, or another
 *  Channel Switching was started but has not been completed yet.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispSetChannelReq(const PHY_ChannelOnPagePlain_t channelOnPage);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-COMPLETE.indication to the MAC-FE.
 * \details
 *  The MAC-LE calls this callback function provided by the MAC-FE when the requested
 *  Transmission or ED Scan is completed.
 * \details
 *  This function shall issue the COMPLETE signal to the MAC-FE to schedule the
 *  corresponding task to perform reading-out of the report of completed Transmission or
 *  ED Scan, or Channel Switching with the MAC-LE-COMPLETE.response.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispCompleteInd(void);


/*************************************************************************************//**
 * \brief   Processes the MAC-LE-COMPLETE.response from the MAC-FE.
 * \param[out]  params      Pointer to the structured object provided by the MAC-FE to be
 *  filled by the MAC-LE with parameters of the last completed Transmission or ED Scan, or
 *  Channel Switching:
 *  - transmission start timestamp of the last successful transmission attempt, in symbol
 *    quotients. This argument is assigned only when Transmission is being confirmed by
 *    the MAC-LE,
 *  - status of the completed transmission: SUCCESS, NO_ACK, CHANNEL_ACCESS_FAIL; or
 *    status of attempt to switch the channel (and channel page): SUCCESS or
 *    INVALID_PARAMETER. If an ED Scan is being confirmed, this parameter is constantly
 *    assigned with SUCCESS and may be ignored by the MAC-FE,
 *  - value of the FramePending field of the received ACK frame (equal to FALSE if no
 *    acknowledgment was initially requested for this transmission). This argument is
 *    assigned only when Transmission is being confirmed by the MAC-LE,
 *  - the absolute maximum of all the EnergyLevel values measured serially each for
 *    8-symbols period during the ED Scan period. This argument is  assigned only when
 *    ED Scan is being confirmed by the MAC-LE.
 *
 * \details
 *  Call this function as response on the MAC-LE-COMPLETE.indication to request the MAC-LE
 *  to transfer parameters of the last completed Transmission or ED Scan, or Channel
 *  Switching via the specified structured object pointed with the \p params.
*****************************************************************************************/
MAC_PRIVATE void macLeRealTimeDispCompleteResp(MacLeRealTimeDispCompleteRespParams_t *const params);


#endif /* _BB_MAC_LE_REAL_TIME_DISP_H */