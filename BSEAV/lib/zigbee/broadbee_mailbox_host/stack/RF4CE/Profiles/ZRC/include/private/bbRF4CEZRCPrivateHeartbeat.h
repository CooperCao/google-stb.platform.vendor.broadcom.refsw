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
/*****************************************************************************
 *
 * FILENAME: $Workfile: branches/dkiselev/ZRC2/stack/RF4CE/Profiles/ZRC/include/private/bbRF4CEZRCPrivateHeartbeat.h $
 *
 * DESCRIPTION:
 *   This is the header file for the RF4CE private ZRC heartbeat handler.
 *
 * $Revision: 4430 $
 * $Date: 2014-11-10 14:33:34Z $
 *
 ****************************************************************************************/


#ifndef _BB_RF4CE_ZRC_PRIVATE_HEARTBEAT_H
#define _BB_RF4CE_ZRC_PRIVATE_HEARTBEAT_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCHeartbeat.h"
#include "bbRF4CENWK.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for parameters of the request to issue the Heartbeat GDP command.
 * \details The \c pollingTriggerId field specifies the fired trigger that will be
 *  general except the Time Based Polling and the Key Press Polling (these two are
 *  generated internally by the Poll Client). The \c pairingRef must be a valid reference
 *  of the linked Poll Server to be polled.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 7.5.1, table 18.
 */
typedef struct _Rf4ceGdp2HeartbeatReqParams_t
{
    /* 8-bit data. */
    RF4CE_GDP2_PollingTriggerId_t  pollingTriggerId;        /*!< Identifier of the Polling Trigger to be fired. */

    uint8_t                        pairingRef;              /*!< Pairing reference of the linked Poll Server node. */

} Rf4ceGdp2HeartbeatReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to issue the Heartbeat
 *  GDP command.
 */
typedef struct _Rf4ceGdp2HeartbeatConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} Rf4ceGdp2HeartbeatConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to issue the Heartbeat GDP command.
 */
typedef struct _Rf4ceGdp2HeartbeatReqDescr_t  Rf4ceGdp2HeartbeatReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  issue the Heartbeat GDP command.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 *  the Poll Service just when this callback function is called. The confirmation
 *  parameters structured object is temporarily created in the program stack and is
 *  pointed here with the \p confParams; the parameters object must be processed
 *  completely prior to this callback function returns, or otherwise it must be copied
 *  into a different permanent location.
 */
typedef void (*Rf4ceGdp2HeartbeatConfCallback_t)(Rf4ceGdp2HeartbeatReqDescr_t   *const reqDescr,
                                                 Rf4ceGdp2HeartbeatConfParams_t *const confParams);


/**//**
 * \brief   Structure for descriptor of the request to issue the Heartbeat GDP command.
 */
struct _Rf4ceGdp2HeartbeatReqDescr_t
{
    /* 32-bit data. */
    Rf4ceGdp2HeartbeatConfCallback_t  callback;         /*!< Entry point of the confirmation callback function. */

#ifndef _HOST_
    /* Structured data. */
    RF4CE_NWK_RequestService_t        service;          /*!< Service field. */
#else
	void *context;
#endif

    /* Structured data. */
    Rf4ceGdp2HeartbeatReqParams_t     params;           /*!< Request parameters structured object. */
};


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts request from the application to issue the Heartbeat GDP command.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called internally by the Poll Client when need to perform next
 *  attempt to poll the dedicated Poll Server.
*****************************************************************************************/
void rf4ceGdp2HeartbeatReq(Rf4ceGdp2HeartbeatReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Handles the request to issue the Heartbeat GDP command.
 * \param[in]   queueElement        Pointer to the service field of the descriptor object
 *  of the original request to issue the Heartbeat GDP command.
 * \details
 *  This function shall be called internally by this node task scheduler when the request
 *  to issue the Heartbeat GDP command is selected for execution.
*****************************************************************************************/
void rf4ceGdp2HeartbeatHandler(SYS_QueueElement_t *queueElement);


/*************************************************************************************//**
 * \brief   Handles the polling timeout.
 * \param[in]   profileData     Pointer to the profile data structured object.
 * \details
 *  This function shall be called internally by this node profile timeout service on
 *  Polling Timeout expiration.
*****************************************************************************************/
void rf4ceGdp2HeartbeatTimeoutHandler(RF4CE_ZRC_ProfileData_t *profileData);


/*************************************************************************************//**
 * \brief   Handles the Generic Response GDP command received on the Heartbeat GDP
 *  command.
 * \param[in]   pairingRef                  Pairing reference of the responding node.
 * \param[in]   genericResponseStatus       Status indicated in the received Generic
 *  Response GDP command.
 * \details
 *  This function shall be called internally by this node command processor on reception
 *  of a Generic Response GDP command if it was not processed by other internal services.
*****************************************************************************************/
void rf4ceGdp2HeartbeatGenericResponseInd(uint8_t pairingRef, RF4CE_ZRC2GDP2_Status_t genericResponseStatus);


/*************************************************************************************//**
 * \brief   Handles the Client Notification GDP command received on the Heartbeat GDP
 *  command.
 * \param[in] dataIndParams         Pointer to the NLDE-DATA.indication parameters object.
 * \param[in] leaveReceiverOn       TRUE if necessary to leave the receiver switched on.
 * \details
 *  This function shall be called internally by this node command processor on reception
 *  of a Client Notification GDP command. Notice that this function is called even if this
 *  node is not able to act as a Poll Client.
*****************************************************************************************/
void rf4ceGdp2ClientNotificationInd(RF4CE_NWK_DataIndParams_t *dataIndParams, uint32_t dataPayloadLength,
        bool leaveReceiverOn);


/*************************************************************************************//**
 * \brief   Accepts internal request to switch the radio receiver.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
*****************************************************************************************/
void rf4cezrc2RXEnableReq(RF4CE_NWK_RXEnableReqDescr_t *reqDescr);


#endif /* _BB_RF4CE_ZRC_PRIVATE_HEARTBEAT_H */
