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

/*******************************************************************************
 *
 * DESCRIPTION:
 *      Events system implementation.
 *
*******************************************************************************/

/************************* INCLUDES ****************************************************/
#include "bbSysEvent.h"

/************************* IMPLEMENTATION **********************************************/
#ifdef MAILBOX_STACK_SIDE
/**//**
 * \brief Event module descriptor type.
 */
typedef struct _SYS_EventSystemDescr_t
{
    SYS_QueueDescriptor_t       subscribersQueue;
    SYS_EventHandlerParams_t          hostEvent;
} SYS_EventSystemDescr_t;

/**//**
 * \brief Event module descriptor.
 */
static SYS_EventSystemDescr_t eventsSystemDescr;

/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 */
void SYS_EventInit(void)
{
    memset(&eventsSystemDescr, 0, sizeof(eventsSystemDescr));
}

/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 */
void SYS_EventRaise(SYS_EventNotifyParams_t *const event)
{
    SYS_QUEUE_ITERATION(&eventsSystemDescr.subscribersQueue, seek)
    {
        SYS_EventHandlerParams_t *eventHandler = GET_PARENT_BY_FIELD(SYS_EventHandlerParams_t, elem, seek);
        if (BITMAP_ISSET(eventHandler->subscribedEventsMap, event->id))
            eventHandler->eventNtfy(event);
    }
}

/**//**
 * \brief Register event handlers into event system. Can used only by mailbox.
 */
void sysEventSubscribeHostHandler(SYS_EventHandlerMailParams_t *const eventHandler)
{
    /* NOTE: Bitmap always should be on the top position. */
    SYS_DbgAssertStatic(0 == offsetof(SYS_EventHandlerParams_t, subscribedEventsMap));

    eventsSystemDescr.hostEvent.eventNtfy = SYS_EventNtfy;
    memcpy(&eventsSystemDescr.hostEvent.subscribedEventsMap, &eventHandler->subscribedEventsMap,
        sizeof(eventsSystemDescr.hostEvent.subscribedEventsMap));
    SYS_EventSubscribe(&eventsSystemDescr.hostEvent);
}

/**//**
 * \brief Register event handlers into event system. Can used only by stack side.
 */
void SYS_EventSubscribe(SYS_EventHandlerParams_t *const eventHandler)
{
    SYS_QueueRemoveQueueElement(&eventsSystemDescr.subscribersQueue, &eventHandler->elem);
    SYS_QueuePutQueueElementToHead(&eventsSystemDescr.subscribersQueue, &eventHandler->elem);
}


/**//**
 * \brief Entry point to the event module. Can used only by stack side.
 * \note Different from previous one - in that function some usefull data could be included into the event-object.
 * \note It is not a macross because of its code size.
 * \param eventId [in] - ID of the event.
 * \param dataPtr [in] - pointer to data, which must be copied to event-object.
 * \param dataSize [in] - size of data in bytes.
 */
void SYS_RAISE_EVENT_WITH_DATA(SYS_EventId_t eventId, void * dataPtr, uint8_t dataSize)
{
    SYS_EventNotifyParams_t event = { .id = eventId };
    SYS_DbgAssert(dataSize <= SYS_EVENT_MAX_DATA_SIZE, HALT_SYS_RAISE_EVENT_WITH_DATA_TooBigData);
    memcpy(event.data, dataPtr, dataSize);
    SYS_EventRaise(&event);
}

#endif

/* eof bbSysEvent.c */