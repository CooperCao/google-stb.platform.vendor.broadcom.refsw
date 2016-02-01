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
 * @file EventQueue.cpp
 *
 */

#include "EventQueue.h"

// RULES
//
// "WP" can only be modified by the event poster side.
// "RP" can only be modified by the event retriever side.
// "WP == RP" means queue is empty.
// "(WP+1) == RP" (with wrap-around handled) means queue is full.

CARD_PlaybackDevice::EventQueue::EventQueue(uint32_t maxEvents) : m_maxEvents(maxEvents), m_events(new Event[maxEvents])

{

//printf("%s(): \n", __FUNCTION__);

   // initialize event queue
   m_wp = 0;
   m_rp = 0;

//printf("%s()\n", __FUNCTION__);

}

CARD_PlaybackDevice::EventQueue::~EventQueue()
{

//printf("%s(): \n", __FUNCTION__);

    // delete event queue
    delete [] m_events;

//printf("%s()\n", __FUNCTION__);

}

bool CARD_PlaybackDevice::EventQueue::postEvent(CARD_PlaybackEventID eventID, uint64_t eventParam1)
{
    uint32_t next = m_wp + 1;

//printf("%s(): \n", __FUNCTION__);

    if(next >= m_maxEvents)
    {
        next = 0;
    }

    if(next == m_rp)
    {
        // event queue is full
        return false;
    }

    m_events[m_wp].m_eventID     = eventID;
    m_events[m_wp].m_eventParam1 = eventParam1;

    m_wp = next;

//printf("%s()\n", __FUNCTION__);

    return true;
}

bool CARD_PlaybackDevice::EventQueue::getEvent(CARD_PlaybackEventID &eventID, uint64_t &eventParam1)
{

//printf("%s(): \n", __FUNCTION__);

    if(m_rp == m_wp)
    {
        // event queue is empty

//printf("%s(): No events\n", __FUNCTION__);
//printf("%s()\n", __FUNCTION__);

        return false;
    }

    eventID     = m_events[m_rp].m_eventID;
    eventParam1 = m_events[m_rp].m_eventParam1;

    uint32_t next = m_rp + 1;

    if(next >= m_maxEvents)
    {
        next = 0;
    }

    m_rp = next;

//printf("%s()\n", __FUNCTION__);

    return true;
}

