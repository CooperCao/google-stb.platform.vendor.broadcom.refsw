//****************************************************************************
//
// Copyright (c) 2007-2010 Broadcom Corporation
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
//
//  Filename:       EstbSnmpAgentStatusACT.h
//  Author:         Maurice Turcotte
//  Creation Date:  12 MAY 06
//
//****************************************************************************
//  Description:
//      This class implements the ACT that allows some other thread to
//      notify us about major changes in state.  It then translates these
//      events into the format that can be consumed by the Estb SNMP Agent
//      so that it can act on the event.
//
//****************************************************************************

#ifndef ESTBSNMPAGENTSTATUSACT_H
#define ESTBSNMPAGENTSTATUSACT_H

//********************** Include Files ***************************************

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#else
#include <iostream.h>
#endif
#include "MsgACT.h"
#include "OctetBuffer.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class EstbSnmpAgent;

//********************** Class Declaration ***********************************


class EstbSnmpAgentCmDocsisStatusACT : public BcmMsgACT
{
public:

    // Initializing Constructor.  You must specify the pointer to the SnmpAgent
    // Ctl thread; this is the object that will be called when HandleEvent()
    // is called.
    //
    // Parameters:
    //      pAgent - pointer to the SnmpAgent.
    //
    // Returns:  N/A
    //
    EstbSnmpAgentCmDocsisStatusACT(EstbSnmpAgent *pAgent, BcmOctetBuffer *pMsgObuf = NULL );

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~EstbSnmpAgentCmDocsisStatusACT();

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
    virtual void HandleEvent(const BcmCompletionEvent &event_code);

protected:

    EstbSnmpAgent *pfAgent;

private:

    // Various required methods that aren't supported.
    EstbSnmpAgentCmDocsisStatusACT(void);
    EstbSnmpAgentCmDocsisStatusACT(const EstbSnmpAgentCmDocsisStatusACT &);
    EstbSnmpAgentCmDocsisStatusACT & operator = (const EstbSnmpAgentCmDocsisStatusACT &);

};


//********************** Inline Method Implementations ***********************

#endif


