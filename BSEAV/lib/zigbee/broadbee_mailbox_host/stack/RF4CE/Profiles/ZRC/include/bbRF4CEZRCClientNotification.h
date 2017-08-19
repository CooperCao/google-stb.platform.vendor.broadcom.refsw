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
*       RF4CE GDP 2.0 Client Notification command processor interface.
*
*******************************************************************************/

#ifndef _BB_RF4CE_ZRC_CLIENT_NOTIFICATION_H
#define _BB_RF4CE_ZRC_CLIENT_NOTIFICATION_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCAttributes.h"
#include "bbSysTimeoutTask.h"
#include "bbSysPayload.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of Client Notification Sub-Types.
 * \ingroup RF4CE_GDP2_ClientNotificationReq
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11, table 6.<br>
 *  See ZigBee RF4CE ZRC 2.0 / ZigBee Document 13-0442-23, subclause 6.6, table 12.
 */
typedef enum _RF4CE_ZRC2_ClientNotificationSubTypeId_t
{
    /* GDP 2.0 Sub-Types. */

    RF4CE_ZRC2_CLIENT_NOTIFY_IDENTIFY                 = 0x00,       /*!< Identify. */

    RF4CE_ZRC2_CLIENT_NOTIFY_REQUEST_POLL_NEGOTIATION = 0x01,       /*!< Request Poll Negotiation. */

    RF4CE_ZRC2_CLIENT_NOTIFY_GDP2_SUBTYPE_MAX         = 0x3F,       /*!< Maximum value reserved for GDP profile. */

    /* ZRC 2.0 Sub-Types. */

    RF4CE_ZRC2_CLIENT_NOTIFY_REQUEST_ACTION_MAPPING_NEGOTIATION      = 0x40,
            /*!< Request Action Mapping Negotiation. */

    RF4CE_ZRC2_CLIENT_NOTIFY_REQUEST_HOME_AUTOMATION_PULL            = 0x41,
            /*!< Request Home Automation Pull. */

    RF4CE_ZRC2_CLIENT_NOTIFY_REQUEST_SELECTIVE_ACTION_MAPPING_UPDATE = 0x42,
            /*!< Request Selective Action Mapping Update. */

} RF4CE_ZRC2_ClientNotificationSubTypeId_t;


/**//**
 * \brief   Structure for parameters of the request to issue the Client Notification GDP
 *  command.
 * \ingroup RF4CE_GDP2_ClientNotificationReq
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11, table 6.<br>
 *  See ZigBee RF4CE ZRC 2.0 / ZigBee Document 13-0442-23, subclause 6.6, table 12.
 */
typedef struct _RF4CE_GDP2_ClientNotificationReqParams_t
{
    /* 32-bit data. */
    uint32_t                                  timeout;                      /*!< Timeout to be polled by the Poll
                                                                                Client, in milliseconds. */
    /* Structured data. */
    SYS_DataPointer_t                         payload;                      /*!< Client Notification Payload field. */

    /* 8-bit data. */
    RF4CE_ZRC2_ClientNotificationSubTypeId_t  clientNotificationSubType;    /*!< Client Notification Sub-Type field. */

    uint8_t                                   pairingRef;                   /*!< Pairing reference of the linked Poll
                                                                                Client node. */
} RF4CE_GDP2_ClientNotificationReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to issue the Client
 *  Notification GDP command.
 * \ingroup RF4CE_GDP2_ClientNotificationConf
 */
typedef struct _RF4CE_GDP2_ClientNotificationConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} RF4CE_GDP2_ClientNotificationConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to issue the Client Notification GDP
 *  command.
 * \ingroup RF4CE_GDP2_ClientNotificationReq
 */
typedef struct _RF4CE_GDP2_ClientNotificationReqDescr_t  RF4CE_GDP2_ClientNotificationReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  issue the Client Notification GDP command.
 * \ingroup RF4CE_GDP2_ClientNotificationConf
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 * \param[in]   confParams      Pointer to the confirmation parameters object.
 * \details
 *  This callback function shall be provided by the application on the Poll Server side.
 *  This function will be called by the GDP layer on completion of corresponding request.
 *  The request descriptor object being confirmed is pointed with the \p reqDescr; the
 *  confirmed request descriptor object may be dismissed or reused by the Poll Service
 *  just when this callback function is called. The confirmation parameters structured
 *  object is temporarily created in the program stack and is pointed here with the
 *  \p confParams; the parameters object must be processed completely prior to this
 *  callback function returns, or otherwise it must be copied into a different permanent
 *  location.
 */
typedef void (*RF4CE_GDP2_ClientNotificationConfCallback_t)(RF4CE_GDP2_ClientNotificationReqDescr_t   *reqDescr,
                                                            RF4CE_GDP2_ClientNotificationConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to issue the Client Notification GDP
 *  command.
 * \ingroup RF4CE_GDP2_ClientNotificationReq
 */
struct _RF4CE_GDP2_ClientNotificationReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_ClientNotificationConfCallback_t  callback;                      /*!< Entry point of the confirmation
                                                                                    callback function. */
#ifndef _HOST_
    /* Structured data. */
    union
    {
        SYS_QueueElement_t                       pendingQueueElement;           /*!< Pending queue element. */

        RF4CE_NWK_RequestService_t               transmissionQueueElement;      /*!< Transmission queue element. */
    };

    SYS_SchedulerTaskDescriptor_t                taskDescr;                     /*!< Service task descriptor. */

    SYS_TimeoutTask_t                            onExpiredTimerDescr;           /*!< Expiration timer descriptor. */
#else
    void *context;
#endif

    /* Structured data. */
    RF4CE_GDP2_ClientNotificationReqParams_t     params;                        /*!< Request parameters structured
                                                                                    object. */
}; /* RF4CE_GDP2_ClientNotificationReqDescr_t */


/**//**
 * \brief   Structure for parameters of the indication of a received Client Notification
 *  GDP command.
 * \ingroup RF4CE_GDP2_ClientNotificationInd
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11, table 6.<br>
 *  See ZigBee RF4CE ZRC 2.0 / ZigBee Document 13-0442-23, subclause 6.6, table 12.
 */
typedef struct _RF4CE_GDP2_ClientNotificationIndParams_t
{
    /* Structured data. */
    SYS_DataPointer_t                         payload;                      /*!< Client Notification Payload field. */

    /* 8-bit data. */
    RF4CE_ZRC2_ClientNotificationSubTypeId_t  clientNotificationSubType;    /*!< Client Notification Sub-Type field. */

    uint8_t                                   pairingRef;                   /*!< Pairing reference of the linked Poll
                                                                                Server node. */
} RF4CE_GDP2_ClientNotificationIndParams_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts request from the application to issue the Client Notification GDP
 *  command.
 * \ingroup RF4CE_ZRC_Functions
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called by the Poll Server application when it needs to send
 *  data to the dedicated Poll Client.
*****************************************************************************************/
void RF4CE_GDP2_ClientNotificationReq(RF4CE_GDP2_ClientNotificationReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Notifies the application layer of the Poll Client about reception of a
 *  Client Notification GDP command from a linked Poll Server.
 * \ingroup RF4CE_ZRC_Functions
 * \param[in]   indParams       Pointer to the indication parameters object.
 * \details
 *  This callback function shall be provided by the application of a device that is
 *  capable to act as a Poll Client. It will be called by the GDP layer during the
 *  Heartbeat polling being performed by this node (a Poll Client) on each reception of
 *  the Client Notification GDP command from the polled Poll Server.
*****************************************************************************************/
void RF4CE_GDP2_ClientNotificationInd(RF4CE_GDP2_ClientNotificationIndParams_t *const indParams);


#endif /* _BB_RF4CE_ZRC_CLIENT_NOTIFICATION_H */

/* eof bbRF4CEZRCClientNotification.h */