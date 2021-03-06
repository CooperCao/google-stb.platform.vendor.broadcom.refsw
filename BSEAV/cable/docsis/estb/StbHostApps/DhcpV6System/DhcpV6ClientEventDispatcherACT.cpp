//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//****************************************************************************
//
//  Filename:       DhcpV6ClientEventDispatcherACT.cpp
//  Author:         Pinar Taskiran-Cyr
//  Creation Date:  July 17, 2009
//
//****************************************************************************
#include "DhcpV6ClientEventDispatcherACT.h"
#include "OctetBuffer.h"
#include "MessageLog.h"

#if (BCM_EVD_INTERFACE_INCLUDED)

// Event list to be filled in by the client.
#include "EcmStatusEventCodes.h"

#endif

#include "EstbIpThread.h"

BcmDhcpV6ClientEventDispatcherACT::BcmDhcpV6ClientEventDispatcherACT( BcmEstbIpThread *pEstbIpThread, BcmOctetBuffer *pMsgObuf )
  :
    BcmMsgACT( pMsgObuf ),
	pfEstbIpThread(pEstbIpThread)
{
    CallTrace("BcmDhcpV6ClientEventDispatcherACT", "constructor");
}

BcmDhcpV6ClientEventDispatcherACT::~BcmDhcpV6ClientEventDispatcherACT()
{
    CallTrace("BcmDhcpV6ClientEventDispatcherACT", "destructor");
	pfEstbIpThread = NULL;
}


ostream& BcmDhcpV6ClientEventDispatcherACT::Print( ostream& ostrm ) const
{    
    // call base class version.
    BcmMsgACT::Print( ostrm );

    // print extra stuff specific to this class...

    return ostrm;
}


void BcmDhcpV6ClientEventDispatcherACT::HandleEvent( 
    const BcmCompletionEvent &event_code )
{
    CallTrace("BcmDhcpV6ClientEventDispatcherACT", "HandleEvent");
	
	if (pfEstbIpThread == NULL) 
	{
		return;
	}

	//cout << " BcmDhcpV6ClientEventDispatcherACT:****" << endl;
    // NOTE: base class BcmMsgACT doesn't know usage context wrt pfMsgParam,
    // so it can't apply print formatting beyond (void *) and it doesn't know
    // whether or not it needs to be deleted...  the context is determined
    // by event_code.
    switch( event_code )
    {     				
		case (kDhcpV6ClientStopAndReset):
		{				
			pfEstbIpThread->StopAndReset();
		}
		break;

		case (kDhcpV6ClientStartIpInit):
		{				
			bool dualStackOperationEnabled = (bool)pfMsgParam;

			bool preferredIpVersionIsIPv6 = true;
			//bool dualStackOperationEnabled = false;
			bool alternameIpManagementModeEnabled = false;
			BcmEstbIpThread::DocsisMode docsisMode = BcmEstbIpThread::kDocsis20IpInitMode;

			// delegate ip initialization to the ip helper.
			if ( !pfEstbIpThread->StartIpInit( NULL, //pfCmCfgObuf, 
											  preferredIpVersionIsIPv6, 
											  dualStackOperationEnabled, 
											  alternameIpManagementModeEnabled,
											  docsisMode ) )
			{
				// error: IP helper failed to start.  very unlikely...
				cout 
				<< "pIpHelperThd->StartIpInit() failed." << endl;
			}
		}
		break;

		case (kDhcpV4ClientStartIpInit):
		{				
			bool preferredIpVersionIsIPv6 = false;
			bool dualStackOperationEnabled = false;
			bool alternameIpManagementModeEnabled = false;
			BcmEstbIpThread::DocsisMode docsisMode = BcmEstbIpThread::kDocsis20IpInitMode;

			// delegate ip initialization to the ip helper.
			if ( !pfEstbIpThread->StartIpInit( NULL, //pfCmCfgObuf, 
											  preferredIpVersionIsIPv6, 
											  dualStackOperationEnabled, 
											  alternameIpManagementModeEnabled,
											  docsisMode ) )
			{
				// error: IP helper failed to start.  very unlikely...
				cout 
				<< "pIpHelperThd->StartIpInit() failed." << endl;
			}
		}
		break;

/*
		case BcmCmDocsisStatusEventCodes::kCmIsNotOperational:
		{
			pfEstbIpThread->CmIsNotOperational();			
		}
		break;
   
		case BcmCmDocsisStatusEventCodes::kCmPreRegConfigFileOk:
		{			
			pfEstbIpThread->CmPreRegConfigFileOk();			
		}
		break;

		case BcmCmDocsisStatusEventCodes::kCmValidManufCvcObtained:
		if( pfMsgParam )
		{		
			pfEstbIpThread->NewManufacturerCVC((BcmOctetBuffer*)pfMsgParam);
		}
		break;

		#if defined (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
		// We received notification that a reset is pending.  If we're in dual processor mode,
		// then we'll have to tell the other side that we're resetting.  This will let the DSG-CC
		// side know that the DSG interface (USB, ETH or PCI as the case may be), will be temporarily
		// unavailable, and prevent any illegal accesses over that interface.
		case BcmCmDocsisStatusEventCodes::kCmIsShuttingDown:
		{
			pfEstbIpThread->CmNotifyHostOfDocsDevResetNow();
			break;
		}
		#endif
*/
		default:
		{
			gLogMessageRaw << " BcmDhcpV6ClientEventDispatcherACT::HandleEvent - Unrecognized event code" 
						   << event_code << endl;
		}
		break;
    }
}

