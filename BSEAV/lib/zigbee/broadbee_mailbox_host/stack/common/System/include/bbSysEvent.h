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
 ******************************************************************************/
/*****************************************************************************
*
* FILENAME: $Workfile: $
*
* DESCRIPTION:
*   Events system declaration.
*
* $Revision: $
* $Date: $
*
****************************************************************************************/

#ifndef _SYS_EVENT_H
#define _SYS_EVENT_H

/************************* INCLUDES ****************************************************/
#include "bbSysPayload.h"
#include "bbSysQueue.h"

/************************* TYPES *******************************************************/
/**//**
 * \brief Events identifiers enumeration.
 */
typedef enum _SYS_EventId_t
{
    /* The device successfully changed itself short address. */
    ZBPRO_NETWORK_ADDRESS_CHANGED_EID   = 0x00U,
    /* The device successfully changed itself panId. */
    ZBPRO_NETWORK_PAN_ID_CHANGED_EID    = 0x01U,
    /* The device successfully changed itself channel. */
    ZBPRO_NETWORK_CHANNEL_CHANGED_EID   = 0x02U,
    /* The device has a new child. */
    ZBPRO_NEW_CHILD_EID                 = 0x03U,

    /* The device has left network */
    ZBPRO_LEAVE_EID                     = 0x04U,
    /* The child has been removed from network */
    ZBPRO_CHILD_REMOVED_EID             = 0x05U,
    ZBPRO_RECONECTED_EID                = 0x06U,
    /* Child is to be removed from network */
    ZBPRO_CHILD_REMOVING_EID            = 0x07U,

    /* Events for persist manager */
    ZBPRO_NWK_HOT_UPDATE                = 0x0AU,
    ZBPRO_APS_HOT_UPDATE                = 0x0BU,

    ZBPRO_ZDO_INIT_DONE                 = 0x0FU,
    /* RF4CE ZRC2 Attributes was initialized */
    RF4CE_ZRC2_INIT_DONE                = 0x10U,


    /* Can be used by application for test proposal. */
    APP_EVENT_00                        = 0x18U,
    APP_EVENT_01                        = 0x19U,
    APP_EVENT_02                        = 0x1AU,
    APP_EVENT_03                        = 0x1BU,
    APP_EVENT_04                        = 0x1CU,
    APP_EVENT_05                        = 0x1DU,
    APP_EVENT_06                        = 0x1EU,
    APP_EVENT_07                        = 0x1FU,
#ifdef _PHY_TEST_HOST_INTERFACE_
    RF4CE_CTRL_EVENT_PAIRING_DEVICE_PAIRED = 0x20U,
    RF4CE_CTRL_EVENT_PAIRING_DEVICE_NOT_FOUND = 0x21U,
    RF4CE_CTRL_EVENT_PAIRING_MULTIPLE_DEVICES_FOUND = 0x22U,
    RF4CE_CTRL_EVENT_PAIRING_DUPLICATE_PAIR_FOUND = 0x23U,
    RF4CE_CTRL_EVENT_UNPAIRING_DEVICE_UNPAIRED = 0x24U,
    RF4CE_CTRL_EVENT_VENDOR_FRAME_RECEIVED = 0x25U,
    RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_OK = 0x26U,
    RF4CE_CTRL_EVENT_VENDOR_FRAME_SEND_FAILED = 0x27U,
    RF4CE_CTRL_EVENT_BAD_VENDOR_FRAME = 0x28U,
    RF4CE_CTRL_EVENT_PAIRING_GENERAL_FAILURE = 0x29U,
    RF4CE_CTRL_EVENT_TEST_TRANSMIT_END = 0x2AU,
#endif
    LAST_EVENT_ID,
} SYS_EventIdEnum_t;
SYS_DbgAssertStatic(UINT8_MAX > LAST_EVENT_ID);
typedef uint8_t SYS_EventId_t;



#define SYS_EVENT_MAX_DATA_SIZE  16

/**//**
 * \brief Event parameters type.
 */
typedef struct _SYS_EventNotifyParams_t
{
    // TODO: The payload field can cause unpredictable behavior. Temporarily deleted.
    // SYS_DataPointer_t   payload;    /* !< Some addition attached data. */
    SYS_EventId_t       id;                          /* !< Event number. */
    uint8_t             data[SYS_EVENT_MAX_DATA_SIZE];   /* !< Data field. Could contains any usefull data. */
} SYS_EventNotifyParams_t;

/**//**
 * \brief Event notification function type.
 */
typedef void SYS_EventNtfy_t(SYS_EventNotifyParams_t *const event);

/**//**
 * \brief Event handler type.
 */
typedef struct _SYS_EventHandlerParams_t
{
    /* NOTE: There is a trick and the bitmap field should be in the first position to make this code work. */
    BITMAP_DECLARE(subscribedEventsMap, LAST_EVENT_ID); /* !< The bitmap of subscribed events.
                                            If appropriate bit is one, then eventNtfy function will be called.
                                            Please use BITMAP_SET macro for setting bits. */
#ifdef MAILBOX_STACK_SIDE
    SYS_EventNtfy_t     *eventNtfy;     /* !< The function to call then subscribed event has been raised */
    SYS_QueueElement_t  elem;           /* !< service field */
#endif
} SYS_EventHandlerParams_t;

/**//**
 * \brief Reduced version of SYS_EventHandlerParams_t. Used by mailbox.
 */
typedef struct _SYS_EventHandlerMailParams_t
{
    BITMAP_DECLARE(subscribedEventsMap, LAST_EVENT_ID);
} SYS_EventHandlerMailParams_t;

/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 */
void SYS_EventInit(void);

/**//**
 * \brief Subscribe handler to the given events list.
 */
void SYS_EventSubscribe(SYS_EventHandlerParams_t *const eventHandler);
void sysEventSubscribeHostHandler(SYS_EventHandlerMailParams_t *const eventHandler); /* NOTE: version for mailbox */

/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 */
void SYS_EventRaise(SYS_EventNotifyParams_t *const event);
#define SYS_RAISE_EVENT(eventId)                            \
    SYS_WRAPPED_BLOCK(                                      \
        SYS_EventNotifyParams_t event = { .id = eventId };  \
        SYS_EventRaise(&event);                             \
    )

/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 * \note Different from previous one - in that function some usefull data could be included into the event-object.
 * \note It is not a macross because of its code size.
 * \param eventId [in] - ID of the event.
 * \param dataPtr [in] - pointer to data, which must be copied to event-object.
 * \param dataSize [in] - size of data in bytes.
 */
void SYS_RAISE_EVENT_WITH_DATA(SYS_EventId_t eventId, void * dataPtr, uint8_t dataSize);

/**//**
 * \brief Notify application about some event. Shall be implemented into application/host side.
 * The easiest way to implementation is:
 *      void SYS_EventNtfy(SYS_EventNotifyParams_t *const event)
 *      {
 *          switch(event.id)
 *          {
 *              case SOME_EVENT_ID:
 *                  SOME_ACTION
 *                  break;
 *          }
 *      }
 */
void SYS_EventNtfy(SYS_EventNotifyParams_t *const event);

#endif /* _SYS_EVENT_H */