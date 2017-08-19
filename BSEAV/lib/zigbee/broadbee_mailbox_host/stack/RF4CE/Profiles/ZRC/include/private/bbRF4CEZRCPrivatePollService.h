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
*       RF4CE GDP 2.0 Poll Service private interface.
*
*******************************************************************************/

#ifndef _BB_RF4CE_ZRC_PRIVATE_POLL_SERVICE_H
#define _BB_RF4CE_ZRC_PRIVATE_POLL_SERVICE_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCPollService.h"
#include "bbRF4CEZRCPrivateHeartbeat.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for the Poll Client configuration and state on the Poll Client side.
 */
typedef struct _Rf4ceGdp2PollClientData_t
{
    /* 32-bit data. */
    RF4CE_GDP2_PollNegotiationReqDescr_t    *pollNegotiationReqDescr;       /*!< Pointer to the parent Poll
                                                                                Negotiation request descriptor. */
    /* Structured data. */
    union                       /* Embedded descriptors used on different phases of the Poll Negotiation procedure. */
    {
        RF4CE_ZRC2_SetAttributesReqDescr_t   pushReqDescr;                  /*!< Push Attributes request descriptor. */

        SYS_TimeoutTask_t                    delayToPullTimerDescr;         /*!< Timer descriptor for delay to pull. */

        RF4CE_ZRC2_PullAttributesReqDescr_t  pullReqDescr;                  /*!< Pull Attributes request descriptor. */
    };

    /* Structured data. */
    RF4CE_GDP2_PollConfiguration_t           pollConfiguration;             /*!< Configuration of the Poll Client. */

    /* Structured data. */
    struct                      /* Embedded descriptors used for the Heartbeat polling. */
    {
        SYS_SchedulerTaskDescriptor_t        onTimeTaskDescr;               /*!< Time Based Polling task descriptor. */

        SYS_TimeoutTask_t                    onTimeTimerDescr;              /*!< Time Based Polling timer descriptor. */

        Rf4ceGdp2HeartbeatReqDescr_t         heartbeatReqDescr;             /*!< Heartbeat request descriptor. */
    };

    /* 8-bit data. */
    uint8_t                                  keyPressDowncounter;           /*!< Down-counter of Key Press events. */

    Bool8_t                                  pollNegotiationInProgress;     /*!< TRUE if the Poll Negotiation procedure
                                                                                is in progress for this Poll Client. */
} Rf4ceGdp2PollClientData_t;


/**//**
 * \brief   Number of Poll Clients on this node.
 * \details Poll Client is replicated for each pair node and due to this reason is equal
 *  to the number of pair nodes (i.e., the number of records in the RF4CE NWK Pairing
 *  Table).
 */
#define RF4CE_GDP2_APLC_MAX_POLL_CLIENTS    RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES


/**//**
 * \brief   Number of Poll Servers on this node.
 * \details Poll Server is replicated for each pair node and due to this reason is equal
 *  to the number of pair nodes (i.e., the number of records in the RF4CE NWK Pairing
 *  Table).
 */
#define RF4CE_GDP2_APLC_MAX_POLL_SERVERS    RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES


/**//**
 * \brief   Gets polling timeout for the specified pair node (the Poll Server).
 * \param[in]   pairingRef      Pairing reference of the linked Poll Server node.
 * \return  Value of the Polling Timeout field of the aplPollConfiguration attribute
 *  corresponding to the referenced pair node, in milliseconds.
 */
#define RF4CE_GDP2_GET_POLLING_TIMEOUT(pairingRef)\
        (rf4ceGdp2PollClientData[pairingRef].pollConfiguration.pollingTimeout)


/************************* PROTOTYPES ***************************************************/
/**//**
 * \brief   Memory for settings and current state of the Poll Client.
 * \details Poll Client is replicated for each pair node.
 * \details Initially this variable is filled with all zeroes. It corresponds to the
 *  'Disabled' polling method; and due to this reason polling for all pair nodes is
 *  initially disabled.
 */
extern Rf4ceGdp2PollClientData_t  rf4ceGdp2PollClientData[RF4CE_GDP2_APLC_MAX_POLL_CLIENTS];

/*
 * Validate the number of Poll Clients on this node.
 */
SYS_DbgAssertStatic(RF4CE_GDP2_APLC_MAX_POLL_CLIENTS == RF4CE_NWKC_MAX_PAIRING_TABLE_ENTRIES);


/*************************************************************************************//**
 * \brief   Performs reset of all the Poll Clients on this node.
 * \param[in]   setDefaultPib   TRUE if shall reset to the factory default configuration;
 *  FALSE if shall reset and then restore configuration from the NVM.
 * \details
 *  Call this function from the GDP2-RESET.request(SetDefaultPIB). If the \p setDefaultPib
 *  is set to TRUE, then all the Poll Clients on this node are stopped and initialized as
 *  disabled with the factory (empty) configuration; this factory configuration is then
 *  saved into the NVM. Otherwise, if the \p setDefaultPib is set to FALSE, all the Poll
 *  Clients are also stopped, but then initialized with parameters loaded from the NVM and
 *  left inactive until the GDP2-RESTART.request() on which enabled Poll Clients will
 *  become active.
*****************************************************************************************/
void rf4ceGdp2PollClientsReset(const bool setDefaultPib);


/*************************************************************************************//**
 * \brief   Performs restart of all the Poll Clients on this node.
 * \default
 *  Call this function from the GDP2-START.request(). On this request all the Poll Clients
 *  on this node are stopped and their current states are reset to initial as on the
 *  GDP2-RESET.request(), but except their configurations that are preserved here; then
 *  all enabled Poll Clients are activated. Those Poll Clients that have the On-Reset
 *  polling trigger enabled in their Heartbeat polling method configuration will poll
 *  their linked Poll Servers by issuing the Heartbeat GDP command.
*****************************************************************************************/
void rf4ceGdp2PollClientsRestart(void);


/*************************************************************************************//**
 * \brief   Configures the Poll Client according to the received Poll Configuration.
 * \param[in]   pairingRef              Pairing reference of the responding Poll Server.
 * \param[in]   aplPollConfiguration    Pointer to the structured object containing the
 *  aplPollConfiguration attribute value received from the Poll Server.
 * \details
 *  This function is called by the Pull Attributes Response GDP command indication
 *  processor on reception of such a command from the Poll Server that was interrogated
 *  by this node (the Poll Client) during the Poll Negotiation procedure. This function is
 *  called by the GDP internals only if this node is capable to act as a Poll Client
 *  according to its aplGDPCapabilities attribute, but the GDP internals do not validate
 *  the received aplPollConfiguration attribute value, and also formally this function may
 *  be called even if this node is not during the Poll Negotiation procedure with the
 *  referenced Poll Server - consequently, this function itself shall perform additional
 *  validations of the received configuration and decide whether to accept it or reject.
 *  The responding Poll Server node is referenced with the \p pairingRef, and this
 *  reference is supposed to coincide with the originally requested Poll Server. The
 *  received value of the aplPollConfiguration attribute is deserialized into the
 *  structured object that is pointed with the \p aplPollConfiguration. This object is
 *  temporarily created in the program stack; it must be processed completely prior to
 *  this callback function returns, or otherwise it must be copied into a different
 *  permanent location.
 * \details
 *  This function configures a new polling channel corresponding to the referenced pair
 *  node and (re-)starts or disables polling on it.
*****************************************************************************************/
void rf4ceGdp2PollNegotiationDoConfigurePollClient(const uint8_t  pairingRef,
                                                   RF4CE_GDP2_PollConfiguration_t *const aplPollConfiguration);


/*************************************************************************************//**
 * \brief   Handles the Key Press event for all the enabled Poll Clients on this node and
 *  fires the On-Key-Press trigger of the Heartbeat Poll Client.
 * \details
 *  This function is called by the Action Service of the ZRC 2.0 profile when a single Key
 *  Press event is detected (i.e., is reported to the Action Service by the application).
 *  On such an event this function recalculates Key Press counters of all active Heartbeat
 *  Poll Clients on this node and for those of them that reached the configured limit it
 *  starts the Heartbeat transaction to the corresponding Poll Server.
*****************************************************************************************/
// TODO: Link this function to the Action Service.
// TODO: Need to think on whether to transfer the pairingRef into this function if action
//          is related to particular pair node.
void rf4ceGdp2PollClientHandleOnKeyPressEvent(void);


#endif /* _BB_RF4CE_ZRC_POLL_SERVICE_H */

/* eof bbRF4CEZRCPrivatePollService.h */