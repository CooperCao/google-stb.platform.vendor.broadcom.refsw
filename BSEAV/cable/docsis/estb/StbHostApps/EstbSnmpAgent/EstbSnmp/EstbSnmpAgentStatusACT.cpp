//****************************************************************************
//
// Copyright (c) 2007-2012 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized License, Broadcom grants no license
// (express or implied), right to use, or waiver of any kind with respect to
// the Software, and Broadcom expressly reserves all rights in and to the
// Software and all intellectual property rights therein.  IF YOU HAVE NO
// AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
// AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
// SOFTWARE.
//
// Except as expressly set forth in the Authorized License,
//
// 1.     This program, including its structure, sequence and organization,
// constitutes the valuable trade secrets of Broadcom, and you shall use all
// reasonable efforts to protect the confidentiality thereof, and to use this
// information only in connection with your use of Broadcom integrated circuit
// products.
//
// 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
// "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
// OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
// RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
// IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
// A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
// ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
// THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
//
// 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
// OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
// INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
// RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
// EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
// WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
// FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
//
//**************************************************************************
//  $Id$
//
//  Filename:       EstbSnmpAgentStatusACT.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This class implements the ACT that allows the some other thread to
//      notify us about major changes in state.  It then translates these
//      events into the format that can be consumed by the SnmpAgent Ctl thread
//      so that it can act on the event.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbSnmpAgentStatusACT.h"

// The events for which I can be notified.
//#include "EstbStatusEventCodes.h"

#include "OctetBuffer.h"
#include "MessageLog.h"

#if (BCM_EVD_INTERFACE_INCLUDED)
// Event list to be filled in by the client.
#include "EcmStatusEventCodes.h"
#include "EstbEvents.h"
#endif

// The object to whom I relay the events.
#include "EstbSnmpAgent.h"

#include "SnmpLog.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


// Initializing Constructor.  You must specify the pointer to the SnmpAgent
// Ctl thread; this is the object that will be called when HandleEvent()
// is called.
//
// Parameters:
//      pAgent - pointer to the SnmpAgent.
//
// Returns:  N/A
//
EstbSnmpAgentCmDocsisStatusACT::EstbSnmpAgentCmDocsisStatusACT(EstbSnmpAgent *pAgent, BcmOctetBuffer *pMsgObuf )
  :
    BcmMsgACT( pMsgObuf ),
    pfAgent(pAgent)
{
    // Nothing else to do.
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
EstbSnmpAgentCmDocsisStatusACT::~EstbSnmpAgentCmDocsisStatusACT()
{
    // Clean up the protected and private members; delete memory/objects, and
    // set pointers to NULL.
    pfAgent = NULL;
}


// HandleEvent - in Asynchronous Completion Token pattern, this function
//      can be used by the Initiator object (or it's BcmCallbackHandler
//      helper object) to dispatch a specific completion handler that
//      processes the response from an asynchronous operation.
//
// Parameters:
//      event_code - integer event code which identifies the asynchronous
//          event type which occurred.  
//
// Returns:  Nothing.
//
void EstbSnmpAgentCmDocsisStatusACT::HandleEvent(const BcmCompletionEvent &event_code)
{
    // Make sure the pointer is reasonably valid (avoids race conditions in
    // deleting this object).
    if (pfAgent != NULL)
    {
        // Check the event code, and vector to the public method in the EstbSnmpAgent
        // object that handles it
        switch (event_code)
        {
			#if (BCM_EVD_INTERFACE_INCLUDED)
			case (kEventLogEvent):
			{
				cout << " EstbSnmpAgentCmDocsisStatusACT::*kEventLogEvent**" << endl;
				// for this application, BcmMsgACT::pfMsgParam is used to store an 
				// uint32 param...
				uint32 eventId = (uint32)pfMsgParam;		
				EstbLogEvent(eventId);
			}
			break;

			case (kEventDsgUpstreamChange):
			{
				cout << " EstbSnmpAgentCmDocsisStatusACT::*kEventDsgUpstreamChange**" << endl;
				// for this application, BcmMsgACT::pfMsgParam is used to store an 
				// bool param...
				bool upstreamState = (bool)((uint32)pfMsgParam & 0x0000FFFF);		

				if(upstreamState)
					pfAgent->EstbConfigFileNewEvent();
			}
			break;
			#endif

			/*
            case BcmEstbStatusEventCodes::kEstbTodInitOk:
                printf("MOE*** EstbSnmpAgentCmDocsisStatusACT::HandleEvent kEstbTodInitOk\n");
                pfAgent->SystemTimeChangeEvent();
                break;

            case BcmEstbStatusEventCodes::kEstbConfigFileNew:
                printf("MOE*** EstbSnmpAgentCmDocsisStatusACT::HandleEvent kEstbConfigFileNew\n");
                pfAgent->EstbConfigFileNewEvent();
                break;

            case BcmEstbStatusEventCodes::kEstbIsOperational:
                printf("MOE*** EstbSnmpAgentCmDocsisStatusACT::HandleEvent kEstbIsOperational\n");
                pfAgent->InterfaceOperationalEvent();
                break;
            
            case BcmEstbStatusEventCodes::kEstbIsShuttingDown:
                printf("MOE*** EstbSnmpAgentCmDocsisStatusACT::HandleEvent kEstbIsShuttingDown\n");
                pfAgent->ShutdownPendingEvent();
                break;
				*/
            
            default:
                printf("MOE*** EstbSnmpAgentCmDocsisStatusACT::HandleEvent Unknown event\n");
                // Shouldn't happen; we haven't subscribed for other events.
                break;
        }
    }
}


