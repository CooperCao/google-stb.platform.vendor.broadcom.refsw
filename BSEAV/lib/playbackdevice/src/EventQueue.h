/***************************************************************************
 *    (c)2005-2011 Broadcom Corporation
 * 
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 * 
 * Module Description:
 * 
 * Revision History:
 * 
 * $brcm_Log: $
 * 
 ***************************************************************************/
/**
 * @file EventQueue.h
 *
 */

#ifndef _EVENTQUEUE_H
#define _EVENTQUEUE_H

#include "PlaybackDevice.h"

/**
 * @class EventQueue EventQueue.h
 * @brief Implementation of event queue for posting/retrieving playback events.
 */
class CARD_PlaybackDevice::EventQueue
{
public:
    /**
     * Constructor.
     *
     * @param[in] maxEvents number of events the queue can hold. (NOTE: the actual
     *            capacity is in fact one entry less than this number)
     */
    EventQueue(uint32_t maxEvents);

    /**
     * Destructor.
     */
    virtual ~EventQueue();

    /**
     * Post an event into the queue. If the queue is full, false will be returned.
     *
     * @param[in] eventID the event identifier for the type of event to post.
     * @param[in] eventParam1 event parameter 1, actual meaning depends on the eventID.
     *
     * @return true if succeeded, otherwise false (queue is full).
     */
    bool postEvent(CARD_PlaybackEventID eventID, uint64_t eventParam1 = 0);

    /**
     * Get event (if any) from the queue (first posted event gets retrieved first).
     * If the queue is empty, false will be returned.
     *
     * @param[out] eventID the event identifier for the type of posted event.
     * @param[out] eventParam1 event parameter 1, actual meaning depends on the eventID.
     *
     * @return true if succeeded, otherwise false (queue is empty).
     */
    bool getEvent(CARD_PlaybackEventID &eventID, uint64_t &eventParam1);

private:

    struct Event
    {
        CARD_PlaybackEventID m_eventID;
        uint64_t  m_eventParam1;
    };

    uint32_t volatile m_wp;
    uint32_t volatile m_rp;
    uint32_t const m_maxEvents;
    Event* const m_events;
};

#endif // _EVENTQUEUE_H

