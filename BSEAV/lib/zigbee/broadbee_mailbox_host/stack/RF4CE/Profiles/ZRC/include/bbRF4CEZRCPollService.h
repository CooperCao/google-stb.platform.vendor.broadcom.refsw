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
* FILENAME: $Workfile$
*
* DESCRIPTION:
*   RF4CE GDP 2.0 Poll Service interface.
*
* $Revision$
* $Date$
*
*****************************************************************************************/


#ifndef _BB_RF4CE_ZRC_POLL_SERVICE_H
#define _BB_RF4CE_ZRC_POLL_SERVICE_H


/************************* INCLUDES *****************************************************/
#include "bbRF4CEZRCAttributes.h"
#include "bbSysTimeoutTask.h"
#include "bbSysPayload.h"


/************************* DEFINITIONS **************************************************/
/**//**
 * \brief   Enumeration of Polling Methods.
 * \note    When a Poling Constraint Record is published with 'Disabled' (0x00) polling
 *  method identifier or one of reserved values (0x02-0xFF) by a prospective Poll Client
 *  in its aplPollConstraints, such a Record must be ignored by the prospective Poll
 *  Server.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclauses 6.2.5.1, 6.2.6.1,
 *  table 11.
 */
typedef enum _RF4CE_GDP2_PollingMethodId_t
{
    RF4CE_GDP2_POLLING_METHOD_DISABLED  = 0x00,         /*!< Polling method is disabled. */

    RF4CE_GDP2_POLLING_METHOD_HEARTBEAT = 0x01,         /*!< GDP Heartbeat based polling. */

} RF4CE_GDP2_PollingMethodId_t;


/**//**
 * \brief   Enumeration of Polling Triggers.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 7.5.1, table 18.
 */
typedef enum _RF4CE_GDP2_PollingTriggerId_t
{
    RF4CE_GDP2_POLLING_ON_GENERIC_ACTIVITY    = 0x00,       /*!< Generic activity polling. The poll is triggered on
                                                                discretion of the application. */

    RF4CE_GDP2_POLLING_ON_TIME                = 0x01,       /*!< Time based polling. The poll is triggered by a periodic
                                                                timer. Generated internally by the GDP layer. */

    RF4CE_GDP2_POLLING_ON_KEY_PRESS           = 0x02,       /*!< Poling on key press. The poll is triggered by
                                                                occurrence of a certain number of key presses. Generated
                                                                internally by the GDP layer by counting single key press
                                                                events from the application that is responsible for
                                                                support of the keyboard. */

    RF4CE_GDP2_POLLING_ON_PICK_UP             = 0x03,       /*!< Polling on pick up. The poll is triggered when the node
                                                                detects a user pickup. Generated by the application and
                                                                delivered to the GDP via the public API. */

    RF4CE_GDP2_POLLING_ON_RESET               = 0x04,       /*!< Polling on reset. The poll is triggered when a node
                                                                resets and performs a warm start. Generated internally
                                                                by the GDP layer. */

    RF4CE_GDP2_POLLING_ON_MICROPHONE_ACTIVITY = 0x05,       /*!< Polling on microphone activity. The poll is triggered
                                                                when the node detects microphone activity. Generated by
                                                                the application and delivered to the GDP via the public
                                                                API. */

    RF4CE_GDP2_POLLING_ON_OTHER_USER_ACTIVITY = 0x06,       /*!< Polling on other user activity. The poll is triggered
                                                                when the node detects any user activity that is not
                                                                addressed by the other triggers. Generated by the
                                                                application and delivered to the GDP via the public
                                                                API. */
} RF4CE_GDP2_PollingTriggerId_t;


/**//**
 * \brief   Structure for the Polling Trigger Capabilities field of the Polling Constraint
 *  Record field array of the aplPollConstraints attribute and for the Polling Trigger
 *  Configuration field of the aplPollConfiguration attribute.
 * \note    The Poll Client may perform additional polls at any moment in time using the
 *  allowed poll method - i.e., polling on the Generic Activity trigger. There is no bit
 *  in this set corresponding to the Generic Activity trigger, because this trigger is
 *  considered enabled by default - so, this trigger capability is not published by the
 *  prospective Poll Client in its aplPollConstraints (i.e., it is capable by default) and
 *  is not assigned by the prospective Poll Server to the Poll Client being configured in
 *  the aplPollConfiguration (i.e., it is enabled unconditionally).
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclauses 6.2.5.2, 6.2.6.2,
 *  7.5.1, figures 29, 31, table 18.
 */
typedef union _RF4CE_GDP2_PollingTriggersSet_t
{
    /* 16-bit data. */
    struct
    {
        uint16_t  onTime               : 1;     /*!< TRUE if capable/enabled to perform the time based polling. */

        uint16_t  onKeyPress           : 1;     /*!< TRUE if capable/enabled to perform polling on key press. */

        uint16_t  onPickUp             : 1;     /*!< TRUE if capable/enabled to perform polling on pick up. */

        uint16_t  onReset              : 1;     /*!< TRUE if capable/enabled to perform polling on reset. */

        uint16_t  onMicrophoneActivity : 1;     /*!< TRUE if capable/enabled to perform polling on microphone
                                                    activity. */

        uint16_t  onOtherUserActivity  : 1;     /*!< TRUE if capable/enabled to perform polling on other user
                                                    activity. */
    };

    BitField16_t  plain;                        /*!< Plain value. */

} RF4CE_GDP2_PollingTriggersSet_t;


/**//**
 * \brief   Returns the single-bit mask for the specified Polling Trigger.
 * \param[in]   pollingTriggerId        Identifier of the polling trigger.
 * \return  Single-bit mask corresponding to the \p pollingTriggerId.
 * \details Single-bit mask is evaluated as the numeric identifier of the Polling Trigger
 *  subtracted one. This shift in enumeration originates from the Generic Activity trigger
 *  that has identifier 0x00 but has no corresponding bit in the triggers set.
 */
#define RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(pollingTriggerId)\
        (BIT((pollingTriggerId) - 1))


/**//**
 * \brief   Binary mask of all enabled triggers.
 * \details Polling trigger Generic Activity is unmaskable and due to this reason no bit
 *  in the mask correspond to it.
 */
#define RF4CE_GSP2_POLLING_TRIGGERS_ALL\
        (\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_TIME) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_KEY_PRESS) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_PICK_UP) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_RESET) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_MICROPHONE_ACTIVITY) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_OTHER_USER_ACTIVITY)\
        )


/**//**
 * \brief   Binary mask of triggers enabled for direct firing from the user application.
 * \details Polling trigger Generic Activity is unmaskable and due to this reason no bit
 *  in the mask correspond to it.
 * \details Polling triggers On-Time and On-Key-Press are fired only by internal
 *  mechanisms of the Poll Service.
 */
#define RF4CE_GSP2_POLLING_TRIGGERS_FOR_USER_EVENTS\
        (\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_PICK_UP) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_RESET) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_MICROPHONE_ACTIVITY) |\
            RF4CE_GSP2_POLLING_TRIGGER_SINGLE_BIT_MASK(RF4CE_GDP2_POLLING_ON_OTHER_USER_ACTIVITY)\
        )


/**//**
 * \brief   Structure for the Polling Constraint Record single field of the
 *  aplPollConstraints attribute.
 * \note    This structure is not binary compatible with the serialized format of the
 *  aplPollConstraints attribute. When need to (de-)serialize this structure according to
 *  the format of the aplPollConstraint attribute as it is transferred via the media by
 *  the RF4CE GDP layer, one should copy an object of this structure field-by-field.
 * \details If the Polling on Key Pressed bit is set in the Polling Trigger Capabilities
 *  field, the value of the Maximum Polling Key Press Counter field must be greater or
 *  equal to the value of the Minimum Polling Key Press Counter field; otherwise the
 *  complete Polling Constraint Record shall be ignored by the Poll Server.
 * \details If the Time Based Polling bit is set in the Polling Trigger Capabilities
 *  field, the value of the Minimum Polling Time Interval field must be between 50 ms and
 *  3600000 ms, the value of the Maximum Polling Time Interval field must be between
 *  60000 ms and 86400000 ms and must be greater or equal to the value of the Minimum
 *  Polling Time Interval field; otherwise the complete Polling Constraint Record shall be
 *  ignored by the Poll Server.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.5, figure 28.
 */
typedef struct _RF4CE_GDP2_PollingConstraintRecord_t
{
    /* 32-bit data. */
    uint32_t                         minimumPollingTimeInterval;        /*!< Minimum polling time interval, in
                                                                            milliseconds, between 50 and 3600000. */

    uint32_t                         maximumPollingTimeInterval;        /*!< Maximum polling time interval, in
                                                                            milliseconds, between 60000 and 86400000. */
    /* 16-bit data. */
    RF4CE_GDP2_PollingTriggersSet_t  pollingTriggerCapabilities;        /*!< Polling trigger capabilities. */

    /* 8-bit data. */
    RF4CE_GDP2_PollingMethodId_t     pollingMethodIdentifier;           /*!< Polling method identifier. */

    uint8_t                          minimumPollingKeyPressCounter;     /*!< Minimum polling key press counter. */

    uint8_t                          maximumPollingKeyPressCounter;     /*!< Maximum polling key press counter. */

} RF4CE_GDP2_PollingConstraintRecord_t;
// TODO: Resolve potential conflict with the RF4CE_GDP2_PollConstraint_t defined in the bbRF4CEZRCAttributes.h


/**//**
 * \brief   Length of a single Poll Constraint Record.
 */
#define RF4CE_GDP2_POLL_CONSTRAINT_RECORD_LENGTH\
        (\
            1 /* Polling method identifier */ +\
            2 /* Polling trigger capabilities */ +\
            1 /* Minimum polling key press counter */ +\
            1 /* Maximum polling key press counter */ +\
            4 /* Minimum polling time interval */ +\
            4 /* Maximum polling time interval */\
        )


/**//**
 * \name    Limit values for the Polling Constraints.
 */
/**@{*/
#define RF4CE_GDP2_LOW_MINIMUM_POLLING_TIME_INTERVAL    50          /*!< Lowest allowed value for the Minimum polling
                                                                        time interval. */

#define RF4CE_GDP2_TOP_MINIMUM_POLLING_TIME_INTERVAL    3600000     /*!< Highest allowed value for the Minimum polling
                                                                        time interval. */

#define RF4CE_GDP2_LOW_MAXIMUM_POLLING_TIME_INTERVAL    60000       /*!< Lowest allowed value for the Maximum polling
                                                                        time interval. */

#define RF4CE_GDP2_TOP_MAXIMUM_POLLING_TIME_INTERVAL    86400000    /*!< Highest allowed value for the Maximum polling
                                                                        time interval. */
/**@}*/


/**//**
 * \brief   The maximum allowed value of the Number of Polling Methods Supported field of
 *  the aplPollConstraints attribute.
 * \note    The GDP 2.0 profile implements only the Heartbeat polling method. Consequently
 *  it is enough for this device to have only a single Polling Constraint Record at the
 *  most in its aplPollConstraints attribute.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclauses 6.2.5, 7.5.1,
 *  figure 27, table 18.
 */
#define RF4CE_GDP2_MAX_NUMBER_OF_POLLING_METHODS_SUPPORTED  1


/**//**
 * \brief   Structure for the aplPollConstraints attribute.
 * \note    This structure is not binary compatible with the serialized format of the
 *  aplPollConstraints attribute. When need to serialize this structure according to the
 *  format of the aplPollConstraint attribute as it is transferred via the media by the
 *  RF4CE GDP layer, one should copy an object of this structure field-by-field.
 * \note    This structure must not be used by the GDP 2.0 on the Poll Server side for
 *  representation of the aplPollConstraints attribute value received via the media from a
 *  device of other vendor, because potentially the number of Polling Constraint Records
 *  included into the attribute by such a device may exceed the maximum allowed value of
 *  the Number of Polling Methods Supported by this particular implementation of GDP 2.0.
 *  The received aplPollConstraints attribute value shall be processed by a custom
 *  function and searched for the first Polling Constraint Record in the records array
 *  describing the Heartbeat polling method with valid constraints.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.5, figure 27.
 */
typedef struct _RF4CE_GDP2_PollConstraints_t
{
    /* Structured data. */
    RF4CE_GDP2_PollingConstraintRecord_t  pollingConstraintRecords[RF4CE_GDP2_MAX_NUMBER_OF_POLLING_METHODS_SUPPORTED];
                                                                                /*!< Array of polling constraint
                                                                                    records. */
    /* 8-bit data. */
    uint8_t                               numberOfPollingMethodsSupported;      /*!< Number of polling methods
                                                                                    supported. May have value of either
                                                                                    zero or one. */
} RF4CE_GDP2_PollConstraints_t;
// TODO: Resolve existing conflict with the RF4CE_GDP2_PollConstraints_t defined in the bbRF4CEZRCAttributes.h


/**//**
 * \brief   Structure for the aplPollConfiguration attribute.
 * \note    This structure is not binary compatible with the serialized format of the
 *  aplPollConfiguration attribute. When need to (de-)serialize this structure according
 *  to the format of the aplPollConfiguration attribute as it is transferred via the media
 *  by the RF4CE GDP layer, one should copy an object of this structure field-by-field.
 * \details If the prospective Poll Server decides to prohibit its polling by the Poll
 *  Client currently performing the Poll Negotiation procedure, it shall set the Poling
 *  Method Identifier to 'Disabled' (0x00) in response for the value of the
 *  aplPollConfiguration attribute being pulled by the Poll Client. Note that if instead
 *  of this the Poll Server merely will not respond with the value for the
 *  aplPollConfiguration attribute for the Pull Attribute command from the prospective
 *  Poll Client, the Poll Negotiation procedure on the Poll Client will be accomplished
 *  with the failure status and thereafter the Poll Client may repeat the Poll Negotiation
 *  attempt again until it will be accomplished with the successful status (i.e.,
 *  endlessly because it will not ever accomplish with success in the described case).
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.6, figure 30.
 */
typedef struct _RF4CE_GDP2_PollConfiguration_t
{
    /* 32-bit data. */
    uint32_t                         pollingTimeInterval;               /*!< Polling time interval, in milliseconds. */

    /* 16-bit data. */
    RF4CE_GDP2_PollingTriggersSet_t  pollingTriggerConfiguration;       /*!< Polling trigger configuration. */

    /* 8-bit data. */
    RF4CE_GDP2_PollingMethodId_t     pollingMethodIdentifier;           /*!< Polling method identifier. */

    uint8_t                          pollingKeyPressCounter;            /*!< Polling key press counter. */

    uint8_t                          pollingTimeout;                    /*!< Polling timeout, in milliseconds, less then
                                                                            aplcMaxPollingTimeout (100 ms). */
} RF4CE_GDP2_PollConfiguration_t;


/**//**
 * \brief   Structure for parameters of the request to assign the aplPollConstraints
 *  attribute of this node.
 * \note    The GDP 2.0 profile implements only the Heartbeat polling method. Consequently
 *  it is enough for this device to have only a single Polling Constraint Record at the
 *  most in its aplPollConstraints attribute. By this reason only single Polling
 *  Constraint Record is transferred with parameters of the request.
 * \note    To prohibit polling on the Poll Client (only before it has performed the Poll
 *  Negotiation procedure) even if it has supportPollClient bit set to TRUE in its
 *  aplGDPCapabilities attribute, one shall set the polling method identifier to 'Disabled'
 *  (0x00); and consequently the Poll Server will disable polling to this Poll Client.
 */
typedef struct _RF4CE_GDP2_SetPollConstraintsReqParams_t
{
    /* Structured data. */
    RF4CE_GDP2_PollingConstraintRecord_t  pollConstraints;      /*!< Single Poll Constraints Record field value. */

} RF4CE_GDP2_SetPollConstraintsReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to assign the
 *  aplPollConstraints attribute of this node.
 */
typedef struct _RF4CE_GDP2_SetPollConstraintsConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} RF4CE_GDP2_SetPollConstraintsConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to assign the aplPollConstraints
 *  attribute of this node.
 */
typedef struct _RF4CE_GDP2_SetPollConstraintsReqDescr_t  RF4CE_GDP2_SetPollConstraintsReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  assign the aplPollConstraints attribute of this node.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 * \param[in]   confParams      Pointer to the confirmation parameters object.
 * \details
 *  This callback function shall be provided by the application of the prospective Poll
 *  Client that requests assignment of its aplPollConstraints attribute that will be
 *  pushed into the Poll Server during the Poll Negotiation procedure. It will be called
 *  by the GDP layer on completion of corresponding request. The request descriptor object
 *  being confirmed is pointed with the \p reqDescr; the confirmed request descriptor
 *  object may be dismissed or reused by the application just when this callback function
 *  is called. The confirmation parameters structured object is temporarily created in the
 *  program stack and is pointed here with the \p confParams; the parameters object must
 *  be processed completely prior to this callback function returns, or otherwise it must
 *  be copied into a different permanent location.
 */
typedef void (*RF4CE_GDP2_SetPollConstraintsConfCallback_t)(RF4CE_GDP2_SetPollConstraintsReqDescr_t   *reqDescr,
                                                            RF4CE_GDP2_SetPollConstraintsConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to assign the aplPollConstraints
 *  attribute of this node.
 */
struct _RF4CE_GDP2_SetPollConstraintsReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_SetPollConstraintsConfCallback_t  callback;      /*!< Entry point of the confirmation callback
                                                                    function. */
#ifndef _HOST_
    /* Structured data. */
    SYS_SchedulerTaskDescriptor_t                taskDescr;     /*!< Task descriptor service object. */
#else
	void *context;
#endif

    /* Structured data. */
    RF4CE_GDP2_SetPollConstraintsReqParams_t     params;        /*!< Request parameters structured object. */
};


/**//**
 * \brief   Structure for parameters of the request to initiate the Poll Negotiation
 *  procedure on the Poll Client.
 */
typedef struct _RF4CE_GDP2_PollNegotiationReqParams_t
{
    /* 8-bit data. */
    uint8_t  pairingRef;        /*!< Pairing reference of the prospective Poll Server node. */

    uint8_t  delayToPull;       /*!< Delay to pull the aplPollConfiguration from the prospective Poll Server, in 100 ms
                                    units (where zero means pull immediately), after the Generic Response is received on
                                    the former Push aplPollConstraints attribute. */

} RF4CE_GDP2_PollNegotiationReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to perform the Poll
 *  Negotiation procedure on the Poll Client.
 */
typedef struct _RF4CE_GDP2_PollNegotiationConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} RF4CE_GDP2_PollNegotiationConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to initiate the Poll Negotiation
 *  procedure on the Poll Client.
 */
typedef struct _RF4CE_GDP2_PollNegotiationReqDescr_t  RF4CE_GDP2_PollNegotiationReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  perform the Poll Negotiation procedure on the Poll Client.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 * \param[in]   confParams      Pointer to the confirmation parameters object.
 * \details
 *  This callback function shall be provided by the application of the prospective Poll
 *  Client that requests initiation of the Poll Negotiation procedure. It will be called
 *  by the GDP layer on completion of corresponding request. The request descriptor object
 *  being confirmed is pointed with the \p reqDescr; the confirmed request descriptor
 *  object may be dismissed or reused by the application just when this callback function
 *  is called. The confirmation parameters structured object is temporarily created in the
 *  program stack and is pointed here with the \p confParams; the parameters object must
 *  be processed completely prior to this callback function returns, or otherwise it must
 *  be copied into a different permanent location.
 */
typedef void (*RF4CE_GDP2_PollNegotiationConfCallback_t)(RF4CE_GDP2_PollNegotiationReqDescr_t   *reqDescr,
                                                         RF4CE_GDP2_PollNegotiationConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to initiate the Poll Negotiation
 *  procedure on the Poll Client.
 */
struct _RF4CE_GDP2_PollNegotiationReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_PollNegotiationConfCallback_t  callback;     /*!< Entry point of the confirmation callback function. */

#ifndef _HOST_
    /* Structured data. */
    SYS_SchedulerTaskDescriptor_t             taskDescr;    /*!< Task descriptor service object. */
#else
	void *context;
#endif

    /* Structured data. */
    RF4CE_GDP2_PollNegotiationReqParams_t     params;       /*!< Request parameters structured object. */
};


/**//**
 * \brief   Structure for parameters of the indication of the Poll Negotiation procedure
 *  started for this node as a Poll Server by a remote Poll Client.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.5,
 *  figures 27, 28.
 */
typedef struct _RF4CE_GDP2_PollNegotiationIndParams_t
{
    /* Structured data. */
    SYS_DataPointer_t  payload;         /*!< Descriptor of the payload containing the serialized value of the
                                            aplPollConstraints attribute of the Poll Client. */
    /* 8-bit data. */
    uint8_t            pairingRef;      /*!< Pairing reference of the prospective Poll Client node. */

} RF4CE_GDP2_PollNegotiationIndParams_t;


/**//**
 * \brief   Structure for parameters of the request to handle a user event and fire the
 *  corresponding Poll Trigger.
 * \details The \c pollingTriggerId field specifies the fired trigger that will be
 *  reported to the Poll Server in the Heartbeat GDP command. All triggers are valid in
 *  general except the Time Based Polling and the Key Press Polling (these two are
 *  generated internally by the Poll Client).
 * \details If polling shall be performed only for a single Poll Server, then the
 *  \c pairingRef shall be assigned to its pairing reference according to the RF4CE NWK
 *  Pairing Table. If polling shall be performed for all linked Poll Servers with the same
 *  trigger and in single request from the application, then the \c pairingRef shall be
 *  set to 0xFF (broadcast).
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 7.5.1, table 18.
 */
typedef struct _RF4CE_GDP2_PollClientUserEventReqParams_t
{
    /* 8-bit data. */
    RF4CE_GDP2_PollingTriggerId_t  pollingTriggerId;        /*!< Identifier of the Polling Trigger to be fired. */

    uint8_t                        pairingRef;              /*!< Pairing reference of the linked Poll Server node.
                                                                Assign to 0xFF (broadcast) if need to poll all the
                                                                linked Poll Servers on this user event. */
} RF4CE_GDP2_PollClientUserEventReqParams_t;


/**//**
 * \brief   Structure for parameters of the confirmation on request to handle a user event
 *  and fire the corresponding Poll Trigger.
 */
typedef struct _RF4CE_GDP2_PollClientUserEventConfParams_t
{
    /* 8-bit data. */
    RF4CE_ZRC2GDP2_Status_t  status;        /*!< Status to be confirmed. */

} RF4CE_GDP2_PollClientUserEventConfParams_t;


/**//**
 * \brief   Structure for descriptor of the request to handle a user event and fire the
 *  corresponding Poll Trigger.
 */
typedef struct _RF4CE_GDP2_PollClientUserEventReqDescr_t  RF4CE_GDP2_PollClientUserEventReqDescr_t;


/**//**
 * \brief   Template for callback handler-function of the confirmation on request to
 *  handle a user event and fire the corresponding Poll Trigger.
 * \param[in]   reqDescr        Pointer to the confirmed request descriptor.
 * \param[in]   confParams      Pointer to the confirmation parameters object.
 * \details
 *  This callback function shall be provided by the application of the Poll Client if it
 *  wishes to perform polling on user events - i.e., all events except Timer Based polling
 *  and Key Press polling (which are both performed automatically). This function will be
 *  called by the GDP layer on completion of corresponding request. The request descriptor
 *  object being confirmed is pointed with the \p reqDescr; the confirmed request
 *  descriptor object may be dismissed or reused by the application just when this
 *  callback function is called. The confirmation parameters structured object is
 *  temporarily created in the program stack and is pointed here with the \p confParams;
 *  the parameters object must be processed completely prior to this callback function
 *  returns, or otherwise it must be copied into a different permanent location.
 */
typedef void (*RF4CE_GDP2_PollClientUserEventConfCallback_t)(RF4CE_GDP2_PollClientUserEventReqDescr_t   *reqDescr,
                                                             RF4CE_GDP2_PollClientUserEventConfParams_t *confParams);


/**//**
 * \brief   Structure for descriptor of the request to handle a user event and fire the
 *  corresponding Poll Trigger.
 */
struct _RF4CE_GDP2_PollClientUserEventReqDescr_t
{
    /* 32-bit data. */
    RF4CE_GDP2_PollClientUserEventConfCallback_t  callback;         /*!< Entry point of the confirmation callback
                                                                        function. */
#ifndef _HOST_
    /* Structured data. */
    SYS_SchedulerTaskDescriptor_t                 taskDescr;        /*!< Task descriptor service object. */
#else
	void *context;
#endif

    /* Structured data. */
    RF4CE_GDP2_PollClientUserEventReqParams_t     params;           /*!< Request parameters structured object. */
};


/************************* PROTOTYPES ***************************************************/
/*************************************************************************************//**
 * \brief   Accepts request to assign the aplPollConstraints attribute of this node.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called by the application to enable or disable the Heartbeat
 *  polling method and assign the Heartbeat polling method constraints in the
 *  aplPollConstraints attribute of this node. The Heartbeat polling enabled status and
 *  constraints are specified with the request parameters included into the request
 *  descriptor object pointed with the \p reqDescr. The request is accomplished with the
 *  confirmation having a single parameter \c status that indicates either success or
 *  failure.
 * \note
 *  The Heartbeat polling method is the single polling method implemented. By this reason
 *  there is no practical need to support ability to configure a number of different
 *  polling methods in this request.
 * \note
 *  If need to disable the Heartbeat polling (prior to the Poll Negotiation procedure is
 *  performed) one shall set the Polling Method Identifier to 'Disabled' (0x00) in
 *  parameters of the request.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclause 6.2.5.
*****************************************************************************************/
void RF4CE_GDP2_SetPollConstraintsReq(RF4CE_GDP2_SetPollConstraintsReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Accepts request from the application to perform the Poll Negotiation procedure
 *  as a Polling Client.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called by the application to initiate the Poll Client activity
 *  on this node. On the request this node sends the Push Attributes GDP command with its
 *  aplPollConstraints attribute to the dedicated paired node (that is expected to become
 *  the Poll Server). The application of such a node, if it is capable to act as the Poll
 *  Server, decides if and how it must be polled by this Poll Client node and assigns the
 *  value of the aplPollConfiguration attribute for this node. This value then pulled by
 *  this node with the Pull Attributes GDP command after the specified delay. The pairing
 *  reference of the prospective Poll Server and the delay to interrogate the Poll Server
 *  for the Poll Configuration are specified with the request parameters included into the
 *  request descriptor object pointed with the \p reqDescr. The request is accomplished
 *  with the confirmation having a single parameter \c status that indicates either
 *  success or failure.
 * \details
 *  If the Poll Negotiation procedure has been completed, the SUCCESS status is confirmed.
 *  It means that eventually the prospective Poll Server has responded with the
 *  appropriate value for the local attribute aplPollConfiguration - i.e., this Poll
 *  Client has been configured. But still it does not mean that this node has become the
 *  Poll Client and will be performing poll of the dedicated Poll Server. If the
 *  prospective Poll Server specified originally in the request by the application decided
 *  not to be polled by this node, it assigns the poll method to 'Disabled' 0x00 in the
 *  aplPollConfiguration for this node (if it wishes to be polled, it assigns it to
 *  'Heartbeat' 0x01). In this case the Poll Negotiation procedure must be considered
 *  completed successfully but polling is disabled.
 * \par     Documentation
 *  See ZigBee RF4CE GDP 2.0 / ZigBee Document 13-0396r29ZB, subclauses 7.2.9.1, 6.2.6.1.
*****************************************************************************************/
void RF4CE_GDP2_PollNegotiationReq(RF4CE_GDP2_PollNegotiationReqDescr_t *const reqDescr);


/*************************************************************************************//**
 * \brief   Notifies the application layer of the prospective Poll Server about the Poll
 *  Negotiation procedure started by a Poll Client node.
 * \param[in]   indParam        Pointer to the indication parameters object.
 * \details
 *  This callback function shall be provided by the application of a device that is
 *  capable to act as a Poll Server. It will be called by the GDP layer on request to
 *  participate in the Poll Negotiation procedure initiated by a prospective Poll Client.
 *  This function is called by the Push Attributes GDP command indication processor on
 *  reception of such a command from a Poll Client that has started its Poll Negotiation
 *  procedure for this node if this command contains a valid value of the
 *  aplPollConstraints attribute being pushed. This function is called only if this node
 *  is capable to act as a Poll Server according to its aplGDPCapabilities attribute. The
 *  indication parameters object pointed with the \p indParams contains the \c pairingRef
 *  and \c payload that denotes respectively the local pairing reference of the remote
 *  node (according to the RF4CE NWK Pairing Table) and the received value of the
 *  aplPollConstraints attribute being pushed. The indication parameters object is
 *  temporarily created in the program stack; it must be processed completely prior to
 *  this callback function returns, or otherwise it must be copied into a different
 *  permanent location. The \c payload object must be dismissed by the application layer
 *  when there is no need to keep it further.
 * \details
 *  The GDP layer does not perform any analysis of the received aplPollConstraints
 *  attribute value. The application layer is responsible for that. On such an indication,
 *  the application shall analyze the received aplPollConstraints attribute value of the
 *  prospective Poll Client, decide if and how it shall perform polling and then assign
 *  the appropriate value to the aplPollConfiguration attribute of that node. This
 *  attribute will then be pulled from this node by the Poll Client after an application
 *  specific delay.
 * \details
 *  If the application decides that no polling shall be performed it should set the
 *  Polling Method Identifier field of the aplPollConfiguration to 'Disabled' 0x00.
 *  Otherwise the Poll Client may repeat it Poll Negotiation procedure later.
*****************************************************************************************/
void RF4CE_GDP2_PollNegotiationInd(RF4CE_GDP2_PollNegotiationIndParams_t *const indParams);


/*************************************************************************************//**
 * \brief   Accepts request from the application to handle a user event and fire the
 *  corresponding Poll Trigger.
 * \param[in]   reqDescr    Pointer to the request descriptor object.
 * \details
 *  This function shall be called by the application to fire one of the user event
 *  triggers on this node Poll Client(-s) and issue the Heartbeat GDP command(-s) to the
 *  linked Poll Servers(-s). The user event is specified with the \c params.pollTriggerId
 *  field of the \p reqDescr. The destination pair node (the Poll Server) is specified
 *  with the \c params.pairingRef field of the \p reqDescr. If all the active Poll Clients
 *  shall poll their linked Poll Servers on this user event, the \c pairingRef field must
 *  be set to the special invalid (broadcast) value 0xFF.
*****************************************************************************************/
void RF4CE_GDP2_PollClientUserEventReq(RF4CE_GDP2_PollClientUserEventReqDescr_t *const reqDescr);


#endif /* _BB_RF4CE_ZRC_POLL_SERVICE_H */
