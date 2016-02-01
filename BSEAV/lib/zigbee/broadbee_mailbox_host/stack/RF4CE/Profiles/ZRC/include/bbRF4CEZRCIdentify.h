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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   RF4CE GDP 2.0 Identify sub-type of Client Notification command processor interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RF4CE_ZRC_IDENTIFY_H
#define _BB_RF4CE_ZRC_IDENTIFY_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCClientNotification.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Structure for Identify flags field.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11.1.1,
 *  figure 21.
 */
typedef union _RF4CE_GDP2_IdentifyFlags_t
{
    /* 8-bit data. */
    struct
    {
        Bool8_t  stopOnAction : 1;      /*!< Stop-on-Action flag. When set to TRUE, indicates that the identify client
                                            shall stop the identify action when it detects user interaction; FALSE to
                                            continue until the new designation from the server. */

        Bool8_t  flashLight   : 1;      /*!< Flash-Light flag. */

        Bool8_t  makeSound    : 1;      /*!< Make-Sound flag. */

        Bool8_t  vibrate      : 1;      /*!< Vibrate flag. */
    };

    BitField8_t  plain;                 /*!< Plain value. */

} RF4CE_GDP2_IdentifyFlags_t;


/**//**
 * \brief   Structure for parameters of the request to initiate the Identification
 *  Capabilities Announcement procedure on the Identify Client.
 */
typedef struct _RF4CE_GDP2_IdentifyCapAnnounceReqParams_t
{
    /* 8-bit data. */
    uint8_t  pairingRef;        /*!< Pairing reference of the prospective Identify Server node. */

} RF4CE_GDP2_IdentifyCapAnnounceReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to perform the
 *  Identification Capabilities Announcement procedure on the Identify Client.
 */
typedef struct _RF4CE_GDP2_IdentifyCapAnnounceConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} RF4CE_GDP2_IdentifyCapAnnounceConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to initiate the Identification
 *  Capabilities Announcement procedure on the Identify Client.
 */
typedef struct _RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t  RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  perform the Identification Capabilities Announcement procedure on the Identify Client.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 *  the application just when this callback function is called. The confirmation
 *  parameters structured object is temporarily created in the program stack and is
 *  pointed here with the \p confParams; the parameters object must be processed
 *  completely prior to this callback function returns, or otherwise it must be copied
 *  into a different permanent location.
 */
typedef void (*RF4CE_GDP2_IdentifyCapAnnounceConfCallback_t)(RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t   *reqDescr,
                                                             RF4CE_GDP2_IdentifyCapAnnounceConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to initiate the Identification
 *  Capabilities Announcement procedure on the Identify Client.
 */
struct _RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_IdentifyCapAnnounceConfCallback_t  callback;         /*!< Entry point of the confirmation callback
                                                                        function. */
#ifndef _HOST_
    /* Structured data. */
    union                       /* Embedded descriptors used on different phases of the Poll Negotiation procedure. */
    {
        SYS_SchedulerTaskDescriptor_t             taskDescr;        /*!< Task descriptor service object. */

        RF4CE_ZRC2_SetAttributesReqDescr_t        pushReqDescr;     /*!< Push Attributes request descriptor. */
    };
#endif

    /* Structured data. */
    RF4CE_GDP2_IdentifyCapAnnounceReqParams_t     params;           /*!< Request parameters structured object. */

}; /* RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t */


/**//**
 * \brief   Structure for parameters of the indication of the Identification
 *  Capabilities Announcement procedure started for this node as an Identify Server by a
 *  remote Identify Client.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.12, table 12.
 */
typedef struct _RF4CE_GDP2_IdentifyCapAnnounceIndParams_t
{
    /* 8-bit data. */
    uint8_t                     pairingRef;                 /*!< Pairing reference of the prospective Poll Client
                                                                node. */

    /* Structured / 8-bit data. */
    RF4CE_GDP2_IdentifyFlags_t  identifyCapabilities;       /*!< Specifies all the actions the Identify Client is
                                                                capable to perform on request to identify itself. Bit #0
                                                                'stopOnAction' is reserved and set to zero; it is
                                                                assumed that the Identify Client is capable to stop any
                                                                action. */
} RF4CE_GDP2_IdentifyCapAnnounceIndParams_t;


/**//**
 * \brief   Structure for parameters of the request to issue the Identify sub-type of the
 *  Client Notification GDP command.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11.1.1, table 6.
 */
typedef struct _RF4CE_GDP2_IdentifyReqParams_t
{
    /* 32-bit data. */
    uint32_t                    clientNotificationTimeout;      /*!< Timeout to be polled by the Poll Client, in
                                                                    milliseconds. */
    /* 16-bit data. */
    uint16_t                    identifyTime;                   /*!< The time for which the Identify Client shall
                                                                    perform the identify action, in seconds. A value of
                                                                    zero indicates to the Identify Client to stop the
                                                                    identify action. */
    /* 8-bit data. */
    RF4CE_GDP2_IdentifyFlags_t  identifyFlags;                  /*!< Specifies all the actions the Identify Client shall
                                                                    take to identify itself. */

    uint8_t                     pairingRef;                     /*!< Pairing reference of the linked Identify Client
                                                                    node. */
} RF4CE_GDP2_IdentifyReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to issue the Identify
 *  sub-type of the Client Notification GDP command.
 */
typedef RF4CE_GDP2_ClientNotificationConfParams_t  RF4CE_GDP2_IdentifyConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to issue the Identify sub-type of the
 *  Client Notification GDP command.
 */
typedef struct _RF4CE_GDP2_IdentifyReqDescr_t  RF4CE_GDP2_IdentifyReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  issue the Identify sub-type of the Client Notification GDP command.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 * \param[in]   confParams      Pointer to the confirmation parameters object.
 * \details
 *  This callback function shall be provided by the application on the Identify Server
 *  side. This function will be called by the GDP layer on completion of corresponding
 *  request. The request descriptor object being confirmed is pointed with the
 *  \p reqDescr; the confirmed request descriptor object may be dismissed or reused by the
 *  Identify Service just when this callback function is called. The confirmation
 *  parameters structured object is temporarily created in the program stack and is
 *  pointed here with the \p confParams; the parameters object must be processed
 *  completely prior to this callback function returns, or otherwise it must be copied
 *  into a different permanent location.
 */
typedef void (*RF4CE_GDP2_IdentifyConfCallback_t)(RF4CE_GDP2_IdentifyReqDescr_t   *reqDescr,
                                                  RF4CE_GDP2_IdentifyConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to issue the Identify sub-type of the
 *  Client Notification GDP command.
 */
struct _RF4CE_GDP2_IdentifyReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_IdentifyConfCallback_t        callback;                  /*!< Entry point of the confirmation callback
                                                                            function. */
#ifndef _HOST_
    /* Structured data. */
    RF4CE_GDP2_ClientNotificationReqDescr_t  clientNotificationReq;     /*!< Embedded Client Notification request
                                                                            descriptor. */
#endif

    /* Structured data. */
    RF4CE_GDP2_IdentifyReqParams_t           params;                    /*!< Request parameters structured object. */

}; /* RF4CE_GDP2_IdentifyReqDescr_t */


/**//**
 * \brief   Structure for parameters of the indication of a received Identify sub-type of
 *  the Client Notification GDP command.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 5.11.1.1,
 *  figure 21.
 */
typedef struct _RF4CE_GDP2_IdentifyIndParams_t
{
    /* 16-bit data. */
    uint16_t                    identifyTime;       /*!< The time for which the Identify Client shall perform the
                                                        identify action, in seconds. A value of zero indicates to the
                                                        Identify Client to stop the identify action. */
    /* 8-bit data. */
    RF4CE_GDP2_IdentifyFlags_t  identifyFlags;      /*!< Specifies all the actions the Identify Client shall take to
                                                        identify itself. */

    uint8_t                     pairingRef;         /*!< Pairing reference of the linked Identify Server node. */

} RF4CE_GDP2_IdentifyIndParams_t;


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 *  informed by these means what identify activities it may request the Identify Client to
 *  perform. The request is accomplished with the confirmation having a single parameter
 *  \c status that indicates either success or failure.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclauses 7.2.9.3, 6.2.12.
*****************************************************************************************/
void RF4CE_GDP2_IdentifyCapAnnounceReq(RF4CE_GDP2_IdentifyCapAnnounceReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Notifies the application layer of the prospective Identify Server about the
 *  Identification Capabilities Announcement procedure started by an Identify Client node.
 * \param[in]   indParam        Pointer to the indication parameters object.
 *  The GDP layer does not perform any analysis of the received
 *  aplIdentificationCapabilities attribute value. The application layer is responsible
 *  for that. On such an indication, the application shall analyze the received
 *  aplIdentificationCapabilities attribute value of the prospective Identify Client,
 *  decide if and how it shall request identify actions on the Identify Client.
*****************************************************************************************/
void RF4CE_GDP2_IdentifyCapAnnounceInd(RF4CE_GDP2_IdentifyCapAnnounceIndParams_t *const indParams);


/*************************************************************************************//**
 * \brief   Accepts request from the application to issue the Identify sub-type of the
 *  Client Notification GDP command.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called by the Identify Server application when it needs the
 *  Identify Client node to start or stop identifying itself or change the way of
 *  identification.
*****************************************************************************************/
void RF4CE_GDP2_IdentifyReq(RF4CE_GDP2_IdentifyReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Notifies the application layer of the Identify Client about reception of a
 *  Identify sub-type of the Client Notification GDP command from a linked Identify
 *  Server.
 * \param[in]   indParam        Pointer to the indication parameters object.
 * \details
 *  This callback function shall be provided by the application of a device that is
 *  capable to act as an Identify Client. It will be called by the GDP layer during the
 *  Heartbeat polling being performed by this node (an Identify Client) on each reception
 *  of the Client Notification GDP command of the Identify sub-type from the polled
 *  Identify Server.
 * \note
 *  The Identify Client GDP profile does not implement internal timeout timer for issuing
 *  the artificial 'stop identify itself' Identify indication on the last received
 *  Identify Time timeout expiration. Application shall support such a timer internally
 *  and stop its identify action itself. The reason is that the GDP profile may be reset
 *  asynchronously at arbitrary moment of time and in this case it would not be able to
 *  send the artificial 'stop identify' indication to the application.
 * \note
 *  The Client Notification GDP command indication is not issued to the recipient
 *  application if the Identify indication is issued instead of it.
*****************************************************************************************/
void RF4CE_GDP2_IdentifyInd(RF4CE_GDP2_IdentifyIndParams_t *const indParams);


#endif /* _BB_RF4CE_ZRC_IDENTIFY_H */
