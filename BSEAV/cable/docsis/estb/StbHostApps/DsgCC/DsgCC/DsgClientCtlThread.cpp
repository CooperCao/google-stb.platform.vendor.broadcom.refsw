/******************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ******************************************************************************/
//****************************************************************************
//
//  Filename:       DsgClientCtlThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  November 22, 2004
//
//****************************************************************************

//********************** Include Files ***************************************

#include "EstbRemoteInterfaceStub.h"
#include "EstbNonVolDefaults.h"

#include "EnetPkt.h"
#include "zone.h"

// My api and definitions...
#include "DsgClientCtlThread.h"
#include "EstbRemoteIfHelperThread.h"
#include "DsgClientRegistrar.h"
#include "DsgEstbTunnelSettingsList.h"
#include "DsgConfigSettings.h"
#include "DsgClientId.h"
#include "DsgClient.h"

// OS stuff.
#include "Event.h"
#include "EventSet.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "MessageQueue.h"

#include "MessageLog.h"

// IP Helper stuff.

// Other stuff.
#include <string.h>
#include "OctetBuffer.h"

#include <iostream>
#include <assert.h>

//#include "DcdTlvCodes.h"
#include "DcdMsg.h"
#include "DsgClientManager.h"

#ifdef APPNAME_SUPPORT
#include "DsgAppNameTable.h"
#endif

#include "EthernetPacket.h"

#if ( BCM_CONSOLE_SUPPORT )
#include "DsgCommandTable.h"
#endif

#if ( BCM_NONVOL_SUPPORT )
#if (BCM_DSG_SINGLE_PROCESS_INTERFACE)
#include "DsgNonVolSettings.h"
#else
#include "DsgccNonVolSettings.h"
#endif
#include "EstbTestNonVolSettings.h"
#endif

#if defined (BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
#include "EstbRemoteIfDataHandlerThread.h"
#endif

#if( CABLECARD_SUPPORT )
#include "DsgccCablecardIf.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************
// ConfigurationChangeCount is a modulo 8-bit number
// In between reseting CCC & the arrival of eCM notification of "SCAN SUCCESS, Tunnels Present",
// do not forward new DCD to cablecard. Otherwise, this will mess up eCM state machine.
#define kDefaultConfigChangeCount 0xFFFF
#define kProcessConfigChangeCount 0xAAAA
#define kDefaultDsgMode			  kDsgAdvanceMode
#define kDefaultTwoWayMode		  true

// DSG tunnel status
enum
{
	kFail				= 0,
	kDsgScanningTunnel	= 1,
	kDsgTunnelPresent	= 2,
	kDsgTunnelAbsent	= 3,
	kDsgNotStarted		= 4
};

typedef enum
{
    kOther = 1,
    kNotReady = 2,
    kNotSynchronized = 3, // kDsPhyNotLocked
    kPhySynchronized = 4, // kDsPhyOkUsPending
    kUsParametersAcquired = 5, // kUcdsOk
    kRangingComplete = 6, // kRngFirstSuccess
    kIpComplete = 7, // kDhcpInitOk
    kTodEstablished = 8, // kTimeOfDayInitOk
    kSecurityEstablished = 9, // kBpkmInitOk
    kParamTransferComplete = 10, // kCmCfgFileDloadOk
    kRegistrationComplete = 11, // kDocsisRegOk
    kOperational = 12,
    kAccessDenied = 13
} CmStatusValue;

//********************** Global Variables *************************************
// Set to NULL. Will be set up by constructor.
BcmDsgClientCtlThread *BcmDsgClientCtlThread::pfSingletonInstance = NULL;
uint32 gTunnelDebugLevel = 0;

//********************** Local Variables *************************************
static bool justStarted = true;
static bool IfIsDown = false;
static bool FirstTimeRegBasicMode = true;   //Fix to pass DSG-02 test 1.2

//********************** Local Functions *************************************

//********************** Class Method Implementations ************************


BcmDsgClientCtlThread::BcmDsgClientCtlThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("DsgClientCtlThread", false, initialPriority),
    fTimeToExit( false ),
    fShutdownExitsApp( true ),
    pfClientExitSemaphore( NULL ),
    fThreadIsRunning( false ),
    pfEventSet( NULL ),
    pfRxQueue( NULL ),
    pfDestructEvent( NULL ),
	fDsgMode(kDefaultValue_dsgMode),
	fEstbDefaultTwoWayMode(kDefaultTwoWayMode),
	fEcmTwoWayMode(false),
    pfDsgClientRegistrarThread(NULL),
	fLinkUp(false),
	fDsgCcState(kInitial),
	fNumberOfClients( 0 ),
	fCurrentUcid( 0 ),
	fUcidAvailable( false ),
	fConfigSettings(),
	fToken( 1 ),
	pfAppNameTable(NULL),
	pfNoDcd(NULL),
	fBroadcastTunnelMac(kAllZerosMac),
	fBroadcastTunnelId(0),
	fWaitingForSrcNameSubTable(false),
	#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	pfEstbRemoteIfHelperThread(NULL),
	#endif
	#if defined (BCM_DSG_DUAL_PROCESSOR_INTERFACE)
	pfInterfaceKeepAliveTimer(NULL),
	#endif
	pfNoSnsTableTimer(NULL),
	fNumberOfDsLockAttempts(kNumberOfFullScansPerDsgMode),
	pfnDcdPassThroughHandler(NULL),
	pfDcdMsg(NULL),
	fLastConfigChangeCount( kDefaultConfigChangeCount ),
	fFirstDcd(true),
	pfDsgBroadcastTunnelPktReassembler(NULL)
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmDsgClientCtlThread");

	// Create DCD object to hold DCD message.
	pfDcdMsg = new BcmDcdMsg( &fDsgTunnelManager, &fClassifierSetsList, &fConfigSettings );

	// Initialize DCD settings.
	ResetDcdSettings();

    // Set up the singleton.
    if (pfSingletonInstance != NULL)
    {
        gWarningMsg(fMessageLogSettings, "BcmDsgClientCtlThread")
            << "Singleton pointer is not NULL!  There are multiple BcmDsgClientCtlThread!  Leaving the singleton pointer alone..." << endl;
    }
    else
    {
        gInfoMsg(fMessageLogSettings, "BcmDsgClientCtlThread")
            << "Setting up singleton pointer." << endl;

        pfSingletonInstance = this;
    }

	#if (BCM_NONVOL_SUPPORT)
	#if (BCM_DSG_SINGLE_PROCESS_INTERFACE)
	pfDsgccNonVolSettings = BcmEstbNonVolSettings::GetSingletonInstance();
	if (pfDsgccNonVolSettings == NULL)
	{
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgClientCtlThread" )
            << "EstbNonVolSettings Singleton is NULL!" << endl;
		assert(0);
	}
	#else
	pfDsgccNonVolSettings = BcmDsgccNonVolSettings::GetSingletonInstance();
	if (pfDsgccNonVolSettings == NULL)
	{
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgClientCtlThread" )
            << "DsgccNonVolSettings Singleton is NULL!" << endl;
		assert(0);
	}
	#endif

	fDsgMode = pfDsgccNonVolSettings->DsgMode();

		// Temporarily included only for linux builds that use the dsgcc's built-in command line client.
	        // Used primarily for ATP testing
		#if (TARGETOS_Linux) //&& !(CLIENT_INTERFACE_SUPPORT)

		BcmEstbTestNonVolSettings* pEstbTestNonVolSettings = BcmEstbTestNonVolSettings::GetSingletonInstance();
		if (pEstbTestNonVolSettings == NULL)
		{
			gFatalErrorMsg( fMessageLogSettings, "BcmDsgClientCtlThread" )
				<< "Estb TEST NonVolSettings Singleton is NULL!" << endl;
			assert(0);
		}

		// Get my client manager pointer.
		pfDsgClientManager = pEstbTestNonVolSettings->DsgClientManager();
		#else
			// Create client list manager which will own the list.
			pfDsgClientManager = &fDsgClientManager; //new BcmDsgClientManager(this);
		#endif

	#else
		// Create client list manager which will own the list.
		pfDsgClientManager = &fDsgClientManager; //new BcmDsgClientManager(this);
	#endif

#if ( BCM_CONSOLE_SUPPORT )
    // Register myself with the command table.
    GetCommandTableSingleton().AddInstance("DSG Client Control Thread", this);
#endif

	pfDsgClientManager->Set(this);

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)

	// ATTENTION - This is moved to CmAppMain.cpp
	// Set up eSTB Remote Interface Helper Thread
	// Create my helper thread.
    //pfEstbRemoteIfHelperThread = new BcmEstbRemoteIfHelperThread (BcmOperatingSystem::kNormalPriority, this);

    #if defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
        pfEstbRemoteIfDataHandlerThread = new BcmEstbRemoteIfDataHandlerThread (BcmOperatingSystem::kNormalPriority, this);
    #endif
#endif

    // Set up DSG Registrar thread
    pfDsgClientRegistrarThread = new BcmDsgClientRegistrarThread (BcmOperatingSystem::kNormalPriority, this);

	// If client settings were stored to nonvol, populate DSG Registrar Thread object's
	// client list. This must be done here because BcmEstbTestNonVolSettings object is already created
	// and populated pfDsgClientManager's list with client settings. Yes, DsgClientManager object and
	// DsgClientRegistrarThread object each has its own list of client settings pointers which point to
	// the same client setting memory.
	#if (TARGETOS_Linux)
	if( pfDsgClientRegistrarThread )
	{
		SubscriberMmapIt client_begin = pfDsgClientManager->begin();
		SubscriberMmapIt client_end = pfDsgClientManager->end();

		while( client_begin != client_end )
		{
			cout <<"ADDING " <<endl;
			if( !pfDsgClientRegistrarThread->populateDsgClientList(client_begin->second))
				{
						gErrorMsg(fMessageLogSettings, "ReadFromImpl")
							<< "Failed to register new Client." << endl;
				}

			client_begin++;
		}
	}
	pfDsgBroadcastTunnelPktReassembler = new BcmDsgBroadcastTunnelPktReassembler();

	#endif

    // Spawn my thread.
#if defined(TARGETOS_vxWorks)
    if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
#else
    if (pfOperatingSystem->BeginThread(this, 32 * 1024) == false)
#endif
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmDsgClientCtlThread" )
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
}


BcmDsgClientCtlThread::~BcmDsgClientCtlThread()
{
    gFuncEntryExitMsg( fMessageLogSettings, "~BcmDsgClientCtlThread" ) << "Entering..." << endl;

#if ( BCM_CONSOLE_SUPPORT )
    GetCommandTableSingleton().RemoveInstance(this);
#endif

    gInfoMsg( fMessageLogSettings, "~BcmDsgClientCtlThread" )
        << "Signalling the thread to exit..." << endl;

    fTimeToExit = true;

    if (pfDestructEvent != NULL)
    {
        pfDestructEvent->Send();
    }

	if (pfAppNameTable)
	{
		delete pfAppNameTable;
		pfAppNameTable = NULL;
	}

    // Wait for the thread to exit.
    WaitForThread();

    // Same for the client's semaphore.
    pfClientExitSemaphore = NULL;

    gFuncEntryExitMsg(fMessageLogSettings, "~BcmDsgClientCtlThread") << "Exiting..." << endl;
}

bool BcmDsgClientCtlThread::Initialize(void)
{
    gFuncEntryExitMsg( fMessageLogSettings, "Initialize" ) << "Entering..." << endl;

    // Create all of the OS objects that I use.
    pfEventSet = pfOperatingSystem->NewEventSet( "BcmDsgClientCtlThread event set" );
    if (pfEventSet == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create event set!" << endl;
        return false;
    }

    pfRxQueue = pfOperatingSystem->NewMessageQueue( "BcmDsgClientCtlThread rx msg queue" );
    if (pfRxQueue == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create rx msg queue!" << endl;
        return false;
    }

    pfDestructEvent = pfOperatingSystem->NewEvent( "BcmDsgClientCtlThread Destruct event" );
    if (pfDestructEvent == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create Destruct event!" << endl;
        return false;
    }

    pfNoDcd = pfOperatingSystem->NewTimer( "NoDcd" );
    if (pfNoDcd == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create NoDcd timer!" << endl;
        return false;
    }

    pfInterfaceKeepAliveTimer = pfOperatingSystem->NewTimer( "KeepAlive" );
    if (pfInterfaceKeepAliveTimer == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create pfInterfaceKeepAliveTimer timer!" << endl;
        return false;
    }

	#ifdef APPNAME_SUPPORT
    pfNoSnsTableTimer = pfOperatingSystem->NewTimer( "SNS Table Timer" );
    if (pfNoSnsTableTimer == NULL)
    {
        gErrorMsg( fMessageLogSettings, "Initialize" )
            << "Failed to create pfNoSnsTableTimer timer!" << endl;
        return false;
    }
	#endif

    // Add all of my objects to the event set.
    pfEventSet->Add( *pfRxQueue );
    pfEventSet->Add(pfDestructEvent);
    pfEventSet->Add(*pfNoDcd);
    pfEventSet->Add(*pfInterfaceKeepAliveTimer);

	#ifdef APPNAME_SUPPORT
    pfEventSet->Add(*pfNoSnsTableTimer);
	#endif

    // set all internal state vars to initial state.
    if (!IsInitialState())
    {
		Reset();
    }

    gFuncEntryExitMsg( fMessageLogSettings, "Initialize" ) << "Exiting..." << endl;

    // Signal that the thread has officially started and is available to take
    // events.
    fThreadIsRunning = true;

    return true;
}

void BcmDsgClientCtlThread::ThreadMain(void)
{
    bool result;

    gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Entering..." << endl;

    bool rx_msg_pending = false;

	cout <<"DSG-CC is waiting for clients to register." << endl;

    // Keep going until told to exit.
    while (fTimeToExit == false)
    {
        if( !rx_msg_pending )
        {
            // normal case: rx msgs are NOT pending.
            // --> wait for an event to occur.
            result = pfEventSet->Wait();

            // Since I'm waiting forever for any event to occur, if this returns
            // with false, then there was a big problem that's likely to persist
            // forever.
            if (result == false)
            {
                gErrorMsg(fMessageLogSettings, "ThreadMain")
                    << "Failed to wait for the event set!" << endl;
                break;
            }
        }
        else
        {
            // else special case: rx msg servicing iteration limit reached.
            // rx msgs may be pending in our q.
            // --> sleep 30 millisec to give up cpu.
            pfOperatingSystem->Sleep( 1000 );

            // --> wait 30 millisec for any events.
            result = pfEventSet->Wait( BcmEventSet::kAny, BcmEvent::kTimeout, 30 );
        }

        // See if we were awakened in order to exit.
        if (fTimeToExit)
        {
			gAlwaysMsgNoFields(fMessageLogSettings) <<"fTimeToExit = true" << endl;
            break;
        }

        // Check to see which event occurred, and process it.
        //
        // first check our rx msg queue.
        if( pfEventSet->Occurred( *pfRxQueue ) || rx_msg_pending )
        {
            // delegate rx msg event processing.
            rx_msg_pending = RxMsgEvent();
        }

        if (pfEventSet->Occurred( *pfNoDcd ))
        {
			NoDcdEvent();
        }

		#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
        if (pfEventSet->Occurred( *pfInterfaceKeepAliveTimer ))
        {
			// Start hunting channels if we have clients waiting and we just started.
			if( KeepAlive() )
			{
				if( justStarted && (pfDsgClientManager->size() > 0) && (fDsgMode != kDsgDisabledMode))
				{
					justStarted = false;
					gAlwaysMsgNoFields(fMessageLogSettings) <<"Just started. I have clients. " << endl;
					HuntForDsgChannel();
				}
			}
        }
		#endif
		
		#ifdef APPNAME_SUPPORT
        if (pfEventSet->Occurred( *pfNoSnsTableTimer ))
        {
				NoGoodSnsTableTimerEvent();
        }
		#endif

        if (pfEventSet->Occurred( pfDestructEvent ))
        {
            //gInfoMsg( fMessageLogSettings, "ThreadMain" )
				cout
                << "Stopping DSG-CC..." << endl;

            fTimeToExit = true;
        }
    }

    // Signal that the thread has stopped and will no longer take events.
    fThreadIsRunning = false;

    // alert the media...
//    fDocsisStatusEventPublisher.PublishEvent( BcmCmDocsisStatusEventCodes::kCmIsShuttingDown );

    // If the client gave me a semaphore, then post it.
    //
    // this needs to be changed to an ACT object that the client
    // registers for!
    if (pfClientExitSemaphore != NULL)
    {
        pfClientExitSemaphore->Release();
    }

    gFuncEntryExitMsg( fMessageLogSettings, "ThreadMain" ) << "Exiting..." << endl;
}


bool BcmDsgClientCtlThread::RxMsgEvent()
{
    CallTrace("BcmDsgClientCtlThread", "RxMsgEvent");

    // Keep processing packets until there are no more, or until we have
    // iterated too many times (don't consume all of the CPU time!).

    unsigned int iterations = 0;
    unsigned int msg_code;
    void *buf = NULL;

    while( pfRxQueue->NumberOfMessages() > 0 )
    {
        // Get the packet at the front of the queue.  I have to use
        // a temporary void * variable and then cast it because the
        // compiler creates a temporary for you, which causes your
        // return value to be NULL.
        pfRxQueue->Receive( msg_code, buf );

		//gAlwaysMsgNoFields(fMessageLogSettings) <<"\n MSG_CODE = " << (uint32)msg_code << endl;

        switch (msg_code)
        {
			// THE FOLLOWING MESSAGES ARE RECEIVED FROM eCM.
			case kEventDsgRxDCD:
				{
					BcmOctetBuffer* pDcd = (BcmOctetBuffer *)buf;
					if (pDcd)
					{
						gInfoMsg( fMessageLogSettings, "RxMsgEvent" ) << *pDcd << endl;
					}
					SyncDsgRxDCD( pDcd );
					delete pDcd;
				}
				break;

			case kEventDsgScanSuccess:
				{
					SyncDsgScanSuccess();
				}
                break;

			case kEventDsgFullScanFail:
				{
					SyncDsgFullScanFail();
				}
                break;

			case kEventDsgUpstreamChange:
				{
					SyncDsgUpstreamChange((bool)buf);
				}
                break;

			case kEventDsgUcid:
				{
					//uint32 ucid = (uint32) buf;
					SyncDsgUcid((uint32) buf);  // ucid
				}
                break;

			case kEventDsgTimerEvent:
				{
					//uint32 timer = (uint32) buf;
					SyncDsgTimerEvent((uint32) buf);  // timer
				}
                break;

			case kEventDsgDccInitType:
				{
					//uint32 dccType = (uint32) buf;
					SyncDsgDccInitType((uint32) buf);
				}
				break;

			case kEventDsgTwoWayForwardingDisabled:
				{
					uint32 param = (uint32) buf;
					SyncDsgTwoWayForwardingDisabled((uint8) param);
				}
				break;

			case kRegisterClient:
				{
					cout <<"\nDSG-CC received new client registration request."<< endl;
					SyncRegisterClient((BcmDsgClient *)buf);
				}
                break;

			case kDeregisterClient:
				{
					SyncDeregisterClient((BcmDsgClient *)buf);
				}
                break;

			case kStart:
				{
					SyncStart();
				}
				break;
			
			#ifdef APPNAME_SUPPORT
			case kSourceSubTable:
				{
					SyncProcessSourceSubTable((BcmDSGAppNameTable *)buf);
				}
				break;
			#endif

            #if 0   //obsolete
            case kSetDsgOtherConfig:
                {
                    ApplyDcdConfigSettings();
                }
                break;
            #endif

			#if ( BCM_CONSOLE_SUPPORT )
			case kTestDsgSetDSGMode:
			case kTestDsgEnableDocsis:
			case kTestDsgSetTimer:
			case kTestDsgGetTimer :
			case kTestDsgSetChannelList:
			case kTestDsgResetChannelList:
			case kTestDsgScan:
			case kTestDsgStatus:
			case kTestDsgDocsisStatus:
				{
					HandleTestEvent(msg_code, buf);
				}
				break;
			#endif

            case kEventResetKeepAliveTimer:
                {
                    pfInterfaceKeepAliveTimer->Stop();
                    pfEventSet->Occurred( *pfInterfaceKeepAliveTimer );
                    pfInterfaceKeepAliveTimer->Start((uint32)buf);
                    cout << "Call KeepAlive in " << (int)buf << " ms" << endl;
                    IfIsDown = true;
                }
            break;

			case kEventDsgEcmMacReinit:
				{				
					SyncDsgEcmMacReinitEvent();
				}
				break;

            default:
                gWarningMsg(fMessageLogSettings, "RxMsgEvent")
                    << "Received unknown message code (" << msg_code
                    << ")!  Ignoring..." << endl;
                break;
        }

        // Kick out if we have iterated too many times.
        iterations++;
        if( iterations >= 8 )
        {
		    if( !IsInitialState() )
		    {
		    	// not in initial state --> the DOCSIS app is running
		    	// and we may be hogging the CPU.  give up control.
                gWarningMsg(fMessageLogSettings, "RxMsgEvent")
                    << "iteration limit reached.  giving up control..." << endl;

                // return value of true indicates iteration limit reached.
               	return true;
		    }
		    // else the DOCSIS app is not running yet.  --> we will not
		    // worry about hogging the CPU.
        }
    }
    // return value false indicates that rx msg q has been emptied.
    return false;
}

void BcmDsgClientCtlThread::Start( void )
{
    // serialize the function call by sending kStart to our msg q.
    SerializePublicInterfaceCall( kStart );
}

void BcmDsgClientCtlThread::SyncStart( void )
{
    #if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)

    // Check if Link up.
    if( KeepAlive() == true )
    {
        // Set mode.
        Mode( fDsgMode );

        #if( CABLECARD_SUPPORT )
        
        // For cablecard, default not to scan. But...
        // if in cablecard dsg advance mode, even no client registered, start the scanning...
        //if( justStarted && (pfnDcdPassThroughHandler || (pfDsgClientManager->size()>0)) && (fDsgMode != kDsgDisabledMode) )
        if( (pfnDcdPassThroughHandler || (pfDsgClientManager->size()>0)) && (fDsgMode != kDsgDisabledMode) )
        {
            //justStarted = false;
            HuntForDsgChannel();
        }

        #else
        
        // Change to always start Scanning in any mode
        //if( (fDsgMode != kDsgDisabledMode) && (pfDsgClientManager->size()>0) )
        if( fDsgMode != kDsgDisabledMode )
            HuntForDsgChannel();
        else
            DsgScan(true);

        #endif
    }
    else
        gWarningMsg(fMessageLogSettings, "SyncStart")
            << "SyncStart Failed!" << endl;

    #else
    
    // Set mode.
    Mode( fDsgMode );

    if( justStarted && (fDsgMode == kDsgDisabledMode))
    {
        justStarted = false;

        // Configure eCM using eCM API.
        Mode( fDsgMode );
        DsgScan(true);
    }

    #endif

}

/*****************************************************************************************
  Local Dsg Client functions published for Dsg Host(eCM) to call into my Dsg Client(eSTB)
******************************************************************************************/

/*****************************************************************************************
 * DsgScanSuccess
 *
 * The eCM calls this function to notify the eSTB that the eCM has locked on a downstream with
 * DSG data. For Basic Mode, this means that the eCM has received traffic on at least one of
 * the configured DSG tunnels. For Advanced Mode, this means that the eCM has received a DCD
 * message.
 *
 * Parameters:  N/A
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgScanSuccess( void )
{

    // serialize the function call by sending kEventDsgScanSuccess to our msg q.
    SerializePublicInterfaceCall( kEventDsgScanSuccess );
}

void BcmDsgClientCtlThread::SyncDsgScanSuccess( void )
{
	gAlwaysMsgNoFields(fMessageLogSettings) <<
	"\n\n**********************************************************" << endl <<
	" eSTB received notification: SCAN SUCCESS, Tunnels Present" << endl <<
	"**********************************************************" << endl;

	#if( CABLECARD_SUPPORT )
    // Setup to start forward new DCD
    // Commnet out the following if line: Fix same CCC in different DS feq corner case.
    //if( fConfigChangeCount == kDefaultConfigChangeCount )
        fConfigChangeCount = kProcessConfigChangeCount;

    #else   //obsolete basic mode
	// eCM sends this notification if:
	// 1) BASIC mode: first DSG packet is received
	// 2) ADVANCE mode: first DCD message is received.
	//    eCM sends DCD message after this notification.
	if( fDsgMode == kDsgBasicMode )
	{
		// Change state.
		State(kOperational);

		// PR9041: Reset number Of Ds Lock Attempts
		fNumberOfDsLockAttempts = kNumberOfFullScansPerDsgMode;
    
		#if defined(CLIENT_INTERFACE_SUPPORT) 
		pfDsgClientManager->NotifyClientsOfTunnelsPresent();
		#endif
	}
    #endif

    #if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	if( DsgSetRIAdr() == 0 )
	{
		gAlwaysMsgNoFields(fMessageLogSettings)
			<<"DsgSetRIAdr - function failure" << endl;
	}
    #endif
}


/*****************************************************************************************
 * DsgFullScanFail
 *
// The eCM calls this function to notify the eSTB that the eCM has scanned every downstream in
// its spectrum (or channel list) without finding suitable DSG traffic.
//
// Parameters:  N/A
//
// Returns:  N/A
//
 *
 * Parameters:  N/A
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgFullScanFail( void )
{
    // serialize the function call by sending kEventDsgFullScanFail to our msg q.
    SerializePublicInterfaceCall( kEventDsgFullScanFail );
}

void BcmDsgClientCtlThread::SyncDsgFullScanFail( void )
{
	gAlwaysMsgNoFields(fMessageLogSettings) <<
	"\n\n************************************************" << endl <<
	" eSTB received notification: FULL SCAN FAILED   " << endl <<
	"************************************************" << endl;

	// PR13042
	//DsgResetChannelList();

#if( CABLECARD_SUPPORT )
	// Handle Full scan fail notification differently for cablecard mode.
	// Let Cablecard decide what to do next if full scan fail
	if( pfnDcdPassThroughHandler )
	{
		BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
		if( pCableCardIf )
			pCableCardIf->CloseAllCableCardClients( this );

		gAlwaysMsgNoFields (fMessageLogSettings) << "Resetting Tunnel Settings & notify cablecard client." << endl;
		Reset();

		#if defined(CLIENT_INTERFACE_SUPPORT)
		pfDsgClientManager->NotifyClientsOfDsScanComplete();
		#endif

		return;
	}
#endif

	// If we failed in Basic mode, there is nothing to do.
	// Just reset everything and go to initial state.
	if( fDsgMode == kDsgBasicMode )
	{
		Reset();

		// PR9041: Do not switch mode until all retries are exhausted.
		fNumberOfDsLockAttempts--;

		// Continue in BASIC mode only if MAC-ADDRESS type clients are still registered.
		if( (pfDsgClientManager->fMacAddrClientCount == 0) ||
			(fNumberOfDsLockAttempts == 0))
		{
			// All MAC-ADDRESS type clients disappeared.
			// Switch back to ADVANCE mode if there any other type client is registered.			
			Mode( kDsgAdvanceMode );
			// If All MAC-ADDRESS type clients are disappeared
			// all tunnels must be closed since we were scanning in
			// BASIC mode. Reset tunnels anyway.
			ResetTunnels();

			fNumberOfDsLockAttempts = kNumberOfFullScansPerDsgMode;			
		}

		// And hunt
		HuntForDsgChannel();

		return;
	}

	gAlwaysMsgNoFields (fMessageLogSettings) << "IN ADVANCE MODE. Resetting Tunnel Settings." << endl;

	fNumberOfDsLockAttempts--;

	// Clear tunnel settings.
	ResetTunnels();

	#if defined(CLIENT_INTERFACE_SUPPORT)
	pfDsgClientManager->NotifyClientsOfDsScanComplete();
	#endif
	
	// Remove BASIC mode from scan logic.
	#if 0 
	// eCM indicates that it has scanned the complete spectrum without
	// finding any DCDs (if in advanced mode)
	// Now if all clients currently registered provided their IDs
	// in the form of MAC addresses, then we will switch to the BASIC
	// mode and scan again.
	// What if clients do not have IDs in the form of MAC addresses
	// OR even if they have what if we cannot find any channels?

	// if( at least one Client IDs are MAC address type )
	if( (pfDsgClientManager->fMacAddrClientCount > 0) && (fNumberOfDsLockAttempts == 0) )
	{
		gAlwaysMsgNoFields (fMessageLogSettings) << "\nAt least 1 client's id is a MAC ADDRESS."
			<< "\nSwitching to BASIC MODE. " << endl;

		Mode( kDsgBasicMode );

		fNumberOfDsLockAttempts = kNumberOfFullScansPerDsgMode;

		// And hunt
		HuntForDsgChannel();
		return;
	}

	gAlwaysMsgNoFields (fMessageLogSettings) << "None of the clients has MAC address type id."  << endl;
	#endif
	// Cannot switch to basic mode either.
	// Try again
	if( fNumberOfDsLockAttempts == 0 )
	{
		// Reset number of attempts.
		fNumberOfDsLockAttempts = kNumberOfFullScansPerDsgMode;
	}

	// Go to initial state.
	Reset();

	// And hunt
	HuntForDsgChannel();
	// Once switched to BASIC mode, DSG-CC does not switch the
	// eCM back to ADVANCED mode (except upon reset).
	// It will keep scanning downstream until it finds a tunnel
	// or all clients disappear. When full scan fails and there are no clients
	// registered we will not scan.
}

/*****************************************************************************************
 * DsgRxDCD
 *
// This method will send DCD msg encapsulated with UDP packet to MA via Halif interface.
// Will wait for packet sent completion
//
// Parameters:
//      pData is the Dcd Mac msg pointer including the header
//      msg_len is the entire mac msg length
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgRxDCD( int len, uint8 * dcd )
{
	// Only process if we are in ADVANCE mode and not in initial state.
	if( (fDsgMode != kDsgAdvanceMode) || (IsInitialState()) )
	{
		gAlwaysMsg( fMessageLogSettings, "DsgRxDCD" )
			<< "Cannot process this DCD. "
			<< "State = " << StateTypeName(fDsgCcState)
			<< "Mode = " << (uint32)fDsgMode << endl;

		return;
	}

    // Wrap the message in an Octet buffer so that the receiving entity gets it
    // in a more usable form.
    BcmOctetBuffer *pOctetBuffer;

    // Wrap a BcmOctetBuffer object around the raw buffer.
    pOctetBuffer = new BcmOctetBuffer(len);

    // Copy the packet data.
    if( pOctetBuffer->AddToEnd((uint8 *) dcd, len) )
	{
		pOctetBuffer->SetBytesUsed(len);
	}
    // serialize the function call by sending kTestDsgRxDCD to our msg q.
	// Delete this memory when this DCD is processed.
    SerializePublicInterfaceCall( kEventDsgRxDCD, pOctetBuffer );
}

const bool BcmDsgClientCtlThread::IsNewDCD( const BcmDcdMsg &dcdMsg )
{
	// ToDo:Create a DCD-MSG class
	uint16 ConfigChangeCount = 0;
	uint8 NumFragments, FragSeqNum;

	#if( CABLECARD_SUPPORT )
    // In between reseting CCC & the arrival of eCM notification of "SCAN SUCCESS, Tunnels Present",
    // do not forward new DCD to cablecard. Otherwise, this will mess up eCM state machine.
    if( fConfigChangeCount == kDefaultConfigChangeCount )
    {
        gAlwaysMsgNoFields (fMessageLogSettings) << ">>>>Wait for SCAN SUCCESS notification..." << endl;
        // Wait for eCM notification SCAN SUCCESS to arrive
        return false;
    }
    #endif

	ConfigChangeCount = dcdMsg.ConfigurationChangeCount();
	NumFragments = dcdMsg.NumberOfFragments();
	FragSeqNum = dcdMsg.FragmentSequenceNumber();

	// Is this a new DCD?
	if( ConfigChangeCount != fConfigChangeCount )
	{
		gInfoMsgNoFields( fMessageLogSettings )
			<< "ConfigChangeCount " << (uint32) ConfigChangeCount <<" != fConfigChangeCount " << (uint32) fConfigChangeCount << endl;

		// Yes!
		// All currently open tunnels are invalid now.
		#if( CABLECARD_SUPPORT )
		{
			BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
			if( pCableCardIf )
				pCableCardIf->CloseAllCableCardClients( this );
		}
		#else
		// PR9105
		ResetEcmTunnels();
		ResetTunnels();
		#endif

		ResetDcdSettings();

		fConfigChangeCount = ConfigChangeCount;
		fNumberOfFragments = NumFragments;

		if( fNumberOfFragments > 1 ) //if( FragSeqNum != NumFragments )
		{
			// Update reassembly state.
			if( IsNewFragment(FragSeqNum) )
			{
				gAlwaysMsgNoFields (fMessageLogSettings) <<"New Fragment." << endl;
			}
		}
		
		return true;
	}

	// DCD Configuration Change Count did not change
	// But maybe this is a new fragment.
	// Is it fragmented at all?
	if( NumFragments > 1 ) //if( FragSeqNum != NumFragments )
	{
		// Move into DCD reassembly state.
		if( IsNewFragment(FragSeqNum) == true )
		{
			gAlwaysMsgNoFields (fMessageLogSettings) <<"New Fragment." << endl;
			return true;
		}
	}

	// Old DCD
	return false;
}

void BcmDsgClientCtlThread::SyncDsgRxDCD( BcmOctetBuffer* octetBuffer )
{
	// Stop DCD reassembly timer if running.
	pfNoDcd->Stop();
	unsigned int offset = 0;
	// Create a Dcd Msg object to help me parse the buffer.
	//fDcdMsg( &fDsgTunnelManager, &fClassifierSetsList, &fConfigSettings );
	byte confCode = pfDcdMsg->ParseFrom((*octetBuffer), offset);

	if(confCode != kConfOk)
	{
		// What to do here? Decide based on our state. HuntForDsgChannel();
		return;
	}

	// Now we have parsed enough to make a decision.
	if( IsNewDCD(*pfDcdMsg) == false )
	{
		// DSG-CC does not care about this DCD now.
		// Drop it.
		gInfoMsg( fMessageLogSettings, "SyncDsgRxDCD" ) <<"Old DCD.\n" << *pfDcdMsg << endl;
		return;
	}

	// This is a new DCD.
	// Lets print and parse-only the rest of the message.
    gAlwaysMsgNoFields (fMessageLogSettings) <<"New DCD. (length=" << octetBuffer->BytesUsed() << ")\n" << *pfDcdMsg << endl;

	// If anyone else consume the DCD, just return
	// OC-SP-CCIF2.0-I01-050331 : Do this after detects a change in CCC field in the DCD msg.
	if( DcdPassThroughHandler( (*octetBuffer) ) == true )
		return;

	// Store new DCD buffer.
	pfDcdMsg->OctetBuffer(octetBuffer);
	//ResetTunnels();

	// This is a new DCD.
	// Lets parse the rest of the message.
	confCode = pfDcdMsg->ParseTLVBuffer((*octetBuffer), offset, &fMessageLogSettings, false);

	if(confCode != kConfOk)
	{
		HuntForDsgChannel();
		return;
	}

	fParsedFragments++;

	// Check if we have a complete DCD.
	if( IsDcdComplete() == false )
	{
		// If this is the first DCD fragment
		// start the Incomplete DCD timer. When the search is
		// underway for a suitable downstream, the "incomplete
		// DCD timeout" is 10 seconds.
		if( fDsgCcState == kHuntingForDsgChannel )
		{
			// Start "Incomplete DCD" timer.
			pfNoDcd->Start( kNoDcdPeriodMillisec );
		}

		// Switch to DCD reassembly mode.
		// Wait for next fragment.
		State(kReassemblingDcd);

		return;
	}

	//cout << *this << endl;

	// We have a complete DCD to process.
	// Now populate tunnel settings list with classifier settings.
	// I will decouple DcdTlvParser later.
	if( pfDcdMsg->FinalizeDcdParsing() == false )
	{
		gAlwaysMsgNoFields (fMessageLogSettings) <<" Invalid DCD." << endl;

		// DCD not valid. Hunt.
		HuntForDsgChannel();
		return;
	}

	// If this is a DCD update (i.e. DSG-CC is in operational state) do not update the state.
	if( fDsgCcState < kValidatingDcd )
	{
		// Change state.
		State(kValidatingDcd);

		// If this is the first DCD and no tunnel request is present then don't do anything until the first
		// tunnel request comes.
		if( pfDsgClientManager->empty() )
		{
			gAlwaysMsgNoFields (fMessageLogSettings) <<" Received DCD but no tunnel request present yet. Waiting for a tunnel request..." << endl;
			return;
		}
	}

	// CORNER CASE: Request clarification.
	// What happens if a non-CA tunnel request comes before any CA tunnel request is received.
	/*if( !pfDsgClientManager->CaTunnelRequestPresent() )
	{
		cout				
			<<" *** No CA tunnel request present. Can't proceed with DCD validation. Waiting for a CA tunnel request..***"
			<< endl;
		// NOTIFY CLIENT - DSGCC_TUNNEL_PENDING
		return;
	}		*/

	//gAlwaysMsgNoFields(fMessageLogSettings) << *this <<endl;

	// Validate DCD, i.e. is any of our clients
	// interested in these tunnels.
	if( (ValidateDcdAndOpenTunnels() == false) ) //&& pfDsgClientManager->CaTunnelRequestPresent())
	{
		if( !fWaitingForSrcNameSubTable)
		  {

			// PR13043:When DSG-CC receives a new DCD that contains DSG channel TLVs,
			// it should set/update eCM's DSG channel list regardless of whether
			// it can find the requested tunnels or not.
			// While hunting for a DSG channel, DSG-CC treats every DCD as a new DCD.
			// In operational mode, DSG-CC processes DCDs only with a new config change count(CCC).
			//
			// While hunting for a DSG channel, DSG-CC might get a DCD that only contains a DSG channel list.
			// DSG-CC should only process the DSG channel list if it gets a DCD with a different CCC, otherwise
			// if it sets the same channel list every time it finds the same DCD eCM will get stuck trying
			// the first channel in the DSG channel list because when DSG channel list is updated channel list
			// index is set to point to the head of the list.
			if( pfDcdMsg->ConfigurationChangeCount() != fLastConfigChangeCount )
			{
				fLastConfigChangeCount = pfDcdMsg->ConfigurationChangeCount();
				ApplyDcdConfigSettings();
			}

			// DCD not valid. Hunt.
			// NOTIFY CLIENT - DSGCC_TUNNEL_PENDING
			HuntForDsgChannel();
			return;
		  }
	}

	fLastConfigChangeCount = pfDcdMsg->ConfigurationChangeCount();
	ApplyDcdConfigSettings();
	return;

	// Move to operational state.
	// State(kOperational); // Now doing this in ValidateDcdAndOpenTunnels
}

void BcmDsgClientCtlThread::ApplyDcdConfigSettings( void)
{
	//if( fConfigSettings.AnythingSpecified())
	{
		uint16 timeout = 0;
		fConfigSettings.Tdsg1TimeoutSpecified(timeout);
		DsgSetTimer(1, timeout);
				
		fConfigSettings.Tdsg2TimeoutSpecified(timeout);
		DsgSetTimer(2, timeout);

		fConfigSettings.Tdsg3TimeoutSpecified(timeout);
		DsgSetTimer(3, timeout);

		fConfigSettings.Tdsg4TimeoutSpecified(timeout);
		DsgSetTimer(4, timeout);

		// Now set channel if any specified.
		if(fConfigSettings.ChannelListSpecified())
		{
			uint32 bufferSize = fConfigSettings.ChannelListSize() * sizeof(uint32);
			BcmOctetBuffer channelList(bufferSize);
			fConfigSettings.WriteTo(channelList);
            cout << fConfigSettings << endl;

			DsgResetChannelList();
			DsgSetChannelList((uint16)fConfigSettings.ChannelListSize(), (uint32*) channelList.AccessBuffer() );
		}

		// Send Vendor Specific Data to VSP clients.
		// Please note that VSP may be stored in a DSG Rule as well. We parse and store any VSP
		// data when we parse the DCD. Currently, we only send VSP that is specified in Config Settings
		// to clients.
		if(fConfigSettings.VendorParamsSpecified())
		{
			pfDsgClientManager->ForwardVspData(fConfigSettings.VendorSettings());
		}
	}
}

BcmDcdMsg* BcmDsgClientCtlThread::DcdMsg(void)
{
	return pfDcdMsg;
}
/*****************************************************************************************
 * DsgUpstreamChange
 *
// The eCM will call this function to notify the eSTB when DOCSIS registration is complete
// (upstream_state = true) or when the upstream is lost (upstream_state = false).
//
// Parameters:
//      us_state -  true (2-way mode)
//                  false (1-way mode)
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgUpstreamChange( bool us_state )
{
    // serialize the function call by sending kTestDsgRxDCD to our msg q.
    SerializePublicInterfaceCall( kEventDsgUpstreamChange, (void*) us_state );
}

void BcmDsgClientCtlThread::SyncDsgUpstreamChange( bool us_state )
{
	if (us_state)
	{
		fEcmTwoWayMode = true;

		cout << //gAlwaysMsgNoFields(fMessageLogSettings) <<
		"\n\n***********************************************************************" << endl <<
		" eSTB received notification: DOCSIS REG COMPLETED. *** TWO_WAY MODE ***" << endl <<
		"***********************************************************************" << endl;

		CheckNewEstbPrimaryIpV4Address(true);
	}
    else
	{
		fEcmTwoWayMode = false;

		cout << //gAlwaysMsgNoFields(fMessageLogSettings) <<
		"\n\n*********************************************" << endl <<
		" eSTB received notification: ONE_WAY MODE ***" << endl <<
		"*********************************************" << endl;
	}

	#if defined(CLIENT_INTERFACE_SUPPORT)
    // tell clients all about it
	pfDsgClientManager->NotifyClientsOfTwoWayMode(fEcmTwoWayMode);
	#endif

}

/*****************************************************************************************
 * DsgUcid
 *
// When the eCM establishes DOCSIS registration, it will provide its Upstream Channel ID to
// the eSTB using this function.
//
// Parameters:
//      uint8 uc_id
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgUcid( uint8 uc_id  )
{
	uint32 ucid = (uint32) uc_id;
    // serialize the function call by sending kTestDsgUcid to our msg q.
    SerializePublicInterfaceCall( kEventDsgUcid, (void*) ucid );

}

void BcmDsgClientCtlThread::SyncDsgUcid( uint8 uc_id  )
{

	cout << //gAlwaysMsgNoFields(fMessageLogSettings) <<
	"\n\n*********************************************" << endl <<
	" eSTB received notification: UCID = 0x" << hex << (uint32)uc_id  << endl <<
	"*********************************************" << endl;

	fCurrentUcid = uc_id;
	fUcidAvailable = true;

    // For cablecard mode: ucid only apply to Host tunnels only.
    // Moved process ucid into CablecardClient object in DsgCableCardClient.cpp
    #if( CABLECARD_SUPPORT == 0 )
	if(fDsgMode == kDsgAdvanceMode )
	{
		ProcessUcid();
	}
    #endif

	#if defined(CLIENT_INTERFACE_SUPPORT)
	pfDsgClientManager->NotifyClientsOfUcid2wayOK(fCurrentUcid);
	#endif

}

/*****************************************************************************************
 * DsgTimerEvent
 *
// The eCM will call this function to notify the eSTB that a relevant timer event has occurred.
// The value event will be encoded as follows:
// Value	Event	Meaning
//  1	Tdsg1 timeout	Did not find DSG info on current channel - continuing scan
//  2	Tdsg2 timeout	DSG info lost from current channel - starting Tdsg4
//  3	Tdsg3 timeout	Beginning attempt to establish DOCSIS two way connection
//  4	Tdsg4 timeout	DSG info gone from current channel - starting downstream scan
//  5	Tdsg4 stop	    DSG info regained on current channel
//
// Parameters:
//      uint8 tim_event - 1-5 encosed above
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
void BcmDsgClientCtlThread::DsgTimerEvent( uint8 tm_event )
{
	uint32 tmEvent = (uint32) tm_event;
	// serialize the function call by sending kTestDsgTimerEvent to our msg q.
    SerializePublicInterfaceCall( kEventDsgTimerEvent, (void*) tmEvent );
}

void BcmDsgClientCtlThread::SyncDsgTimerEvent( uint8 tm_event )
{
	cout << //gAlwaysMsgNoFields(fMessageLogSettings) <<
	"\n\n*************************************************" << endl <<
	" eSTB received notification: TIMER EVENT type = " << (uint32) tm_event << endl  << DsgTimerEventTypeName(tm_event)<< endl <<
	"*************************************************" << endl;

	// If this is a Tdsg4 Timeout, then DSG information in the current
	// DS channel is lost.
	// eCM is starting downstream scan. We should clean up.
	//
	if( (tm_event == kTdsg4Timeout) || (tm_event == kTdsg1Timeout) )
	{
		// Reset settings applicable to the current mode.
		// Close tunnels only if we are in
		// Advanced mode.
		if( fDsgMode == kDsgAdvanceMode )
		{
            #if( CABLECARD_SUPPORT )
            {
                BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
                if( pCableCardIf )
                    pCableCardIf->CloseAllCableCardClients( this );
            }
            #else
			ResetTunnels();
            #endif
			ResetDcdSettings();
		}

		fUcidAvailable = false;
		fCurrentUcid = 0;
	}

    return;
}

/*****************************************************************************************
 * DsgTunnelPkt
 *
// The eCM will call this function to give the eSTB a DSG tunnel packet.
//
// Parameters:
//      ETHERNET_PACKET *pData
//
// Returns:
//      true - the eSTB accepted the packet
//      false - the eSTB could not accept the packet for some reason
//              (in this case, the eCM will discard the packet)
 *
 * Returns:  N/A
 *
 *
 *****************************************************************************************/
// process in the same context of the calling routine
void BcmDsgClientCtlThread::SyncDsgTunnelPkt( ETHERNET_PACKET *pkt )
{
    return;
}

bool BcmDsgClientCtlThread::DsgTunnelPkt( ETHERNET_PACKET *pkt )
{
    // ATTENTION: Remember do not release actual packet buffer
    // that is received by the socket.
    unsigned char *pBufData = (unsigned char *) pkt->pDataBufferHead->pvStart;
    uint16 pktlen = pkt->pDataBufferHead->uiLen;     //get ethernet pkt length

    // Reconstruct the ethernet header.
    // Remember eCM overwrites the first 4 bytes of the packet
    // with the Tunnel Id.
    // This reconstruction is optional. I am doing this to display the ethernet
    // header correctly when debug is on.
    uint32 tunnelId;

    // Read the tunnel id.
    memcpy( &tunnelId, pBufData, sizeof(uint32));

#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE)
    tunnelId = NetworkToHost(tunnelId);
#endif

    // Use Tunnel Settings Table to find the Tunnel MAC address
    // and set it.
    // memcpy( pBufData, &(tunnelSettingsList[tunnelId-1].tunnelMacAddress), 6);
    // UNFINISHED _ #warning "RECONSTRUCT TUNNEL MAC ADDRESS"

#if ( BCM_CONSOLE_SUPPORT )
    char *progressChar[] =
    {
        (char *) "> ", (char *) ">>", (char *) " >", (char *) " <", (char *) "<<", (char *) "< "
    };
    static unsigned int tick = 0;

    // tunnel packet debugging
    if (gTunnelDebugLevel & 0x1)
    {
        gAlwaysMsgNoFields(fMessageLogSettings) <<"\r\t\t\t\t\t\t\t\t\tT " << progressChar[tick] << "\r" << endl;
        tick++;
        if (tick >= (sizeof(progressChar) / sizeof(char *)))
            tick = 0;
    }

    if( gTunnelDebugLevel & 0x2 )
    {
        gAlwaysMsgNoFields(fMessageLogSettings)
             << " Rcvd Tunnel [ " << tunnelId << " ] Pkt with length = " << pktlen << endl;
    }

    if (gTunnelDebugLevel & 0x4)
    {
        BcmEthernetPacket packet;
        packet.SetPacket(pkt);
        // Clear a bunch of stuff normally set in cmhal but not set here
        // since we don't use cmhal. This prevents seg-faults in the packet
        // display code.
        memset(&(packet.PacketStruct()->pDataBufferHead->cmHalPhsInfo),
           0, sizeof(packet.PacketStruct()->pDataBufferHead->cmHalPhsInfo));

        if( packet.ParseIp() == true )
        {
            gAlwaysMsgNoFields(fMessageLogSettings)  << packet << endl;
        }
    }
#endif

#ifdef APPNAME_SUPPORT
    // First, check for Broadcast Tunnel
    if(fWaitingForSrcNameSubTable)
    {
        if (tunnelId == fBroadcastTunnelId)
        {
            if( gTunnelDebugLevel & 0x4 )
            {
                gAlwaysMsgNoFields(fMessageLogSettings)
                    << " Rcvd BroadcastTunnel [ " << tunnelId << " ] Pkt with length = " << pktlen << endl;
            }

            // This will parse the pkt contents, create the AppName Table
            // and send a message to DSG-CC Thread.
            ProcessSourceSubTable(pkt);
        }
    }
#endif

#if( CABLECARD_SUPPORT )
    // Resolve rare dsgcc crash due to a tunnel packet queued in the socket receive thread after eCM reset notification is received & all tunnels are closed
    {
        BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
        if( pCableCardIf && (pCableCardIf->NumOpenTunnels()==0))
        {
            gAlwaysMsgNoFields(fMessageLogSettings) << "Dropping pkt! Receive Tunnel pkt after all tunnels are closed!" << endl;
            return false;   //caller will free pkt
        }
    }
#endif

    BcmEthernetPacket tunnelPacket;
    TunnelSettingsListIterator tunnelIt= fDsgTunnelManager.find(tunnelId);
    bool applyClassifierRules = false;
    bool broadcastTunnelPkt = false;
    if( tunnelIt != fDsgTunnelManager.end() )
    {
        #ifdef DCAS_BT_SUPPORT
        if( tunnelIt->second.HasBroadcastFlow() )
        {
            broadcastTunnelPkt = true;
        }
        #endif

        applyClassifierRules = tunnelIt->second.fClientRequestsSubTunnelForwarding;
        tunnelPacket.SetPacket(pkt);
        // Clear a bunch of stuff normally set in cmhal but not set here
        // since we don't use cmhal. This prevents seg-faults in the packet
        // display code.
        memset(&(tunnelPacket.PacketStruct()->pDataBufferHead->cmHalPhsInfo),
           0, sizeof(tunnelPacket.PacketStruct()->pDataBufferHead->cmHalPhsInfo));

        if( tunnelPacket.ParseIp() )
        {
            #if ( BCM_CONSOLE_SUPPORT )
            if (gTunnelDebugLevel & 0x8)
                gAlwaysMsgNoFields(fMessageLogSettings)  << tunnelPacket << endl;
            #endif
        }

        #ifdef DCAS_BT_SUPPORT	
        // Process DSG PACKET.
        if( broadcastTunnelPkt )
        {
            uint32 result;
            BcmOctetBuffer* pReassembledPacket = NULL;
            result = pfDsgBroadcastTunnelPktReassembler->ProcessBtFragment(tunnelPacket, &pReassembledPacket);
            if(result == BcmDsgBroadcastTunnelPktReassemblyBuffer::kNotFragmented)
            {
                gAlwaysMsgNoFields(fMessageLogSettings) <<"Forward BT Packet to client" << *pReassembledPacket << endl;
                // Forward to client.
                pfDsgClientManager->ForwardToClient(tunnelId, pBufData, pktlen, tunnelPacket, applyClassifierRules);
            }
            else if(result == BcmDsgBroadcastTunnelPktReassemblyBuffer::kAllFragmentsReceived)
            {
                gAlwaysMsgNoFields(fMessageLogSettings) <<"Forward BT re-assembled Packet to client" << *pReassembledPacket << endl;

                // Forward to client.
                pfDsgClientManager->ForwardBroadcastTunnelPktToClient(tunnelId, pReassembledPacket, tunnelPacket, applyClassifierRules);

                // Now delete the buffer that was holding Reassembled Packet. 
                delete pReassembledPacket;
            }
        }
        else
        #endif

        {
            // Forward to client.
            pfDsgClientManager->ForwardToClient(tunnelId, pBufData, pktlen, tunnelPacket, applyClassifierRules);
        }

    }
    else
    {
        gAlwaysMsgNoFields(fMessageLogSettings) << "Dropping pkt! Receive Tunnel pkt with invalid Tunnel ID " << tunnelId << endl;
    }

    // ATTENTION: Remember do not release actual packet buffer
    // that is received by the socket.
    // Done with the DSG packet. Release it.
    if( DsgFreeEthernetPkt( pkt ) == false )
    {
        gAlwaysMsgNoFields(fMessageLogSettings) <<"Dsg packet free error!"<< endl;
        return false;
    }

    return true;
}

/*****************************************************************************************/
// When the eCM sends a "DCC-RSP (Depart)" message, the eCM must also send a
// "DCC Depart, Initialization Type <IT>" (where IT = "DCC Initialization Type")
// message to the Client Controller.
//
// In the case of a pre-registration downstream channel change initiated by a RNG-RSP 
// message or a CONFIG file, eCM will use the existing API 
// "void DsgDccInitType( uint8 dccInitType )" to notify the DSG-CC. It is DSG-CC's 
// responsibility to test the dccInitType value before taking the appropriate action.
//
//
// Parameters:
//      uint8 dccInitType
//
//		The table below shows the values to define eCM's pre-registration and post-registration
//		downstream channel change targeting context.
//
//		Values 0-4 are define the initialization type values in post-registration 
//		DCC-RSP (Depart) message.
//
//		Values 5 and above define the pre-registration event type.
//
//		DCC Initialization Type Codes:
//
//			kDccUsStratResetUs				= 0,
//			kDccUsStratRngInitial			= 1,
//			kDccUsStratRngStation			= 2,
//			kDccUsStratRngInitialOrStation	= 3,
//			kDccUsStratUseDirectly			= 4,
//
//			reserved						= 5,
//			kRngRspOverride					= 6,
//			reserved						= 7,
//			kConfigFileOverride				= 8
//
// Returns:  N/A
//
/*****************************************************************************************/
void BcmDsgClientCtlThread::DsgDccInitType( uint8 dccInitType )
{
	uint32 dccType = (uint32) dccInitType;
	// serialize the function call by sending kTestDsgTimerEvent to our msg q.
    SerializePublicInterfaceCall( kEventDsgDccInitType, (void*) dccType );
}

void BcmDsgClientCtlThread::SyncDsgDccInitType( uint8 dccInitType )
{
	if( dccInitType > 4 )
	{
		gAlwaysMsgNoFields(fMessageLogSettings) <<
		"\n\n********************************************************************" << endl <<
		" eSTB received notification: PRE-REG DS Channel Change (Event type = " << (int)dccInitType <<" )" << endl <<
		"************************************************************************" << endl;
	}
	else
	{
		gAlwaysMsgNoFields(fMessageLogSettings) <<
		"\n\n*********************************************" << endl <<
		" eSTB received notification: DCC Depart, Init Type = " << (int)dccInitType  << endl <<
		"*********************************************" << endl;
	}

	#if defined(CLIENT_INTERFACE_SUPPORT)
	pfDsgClientManager->NotifyClientsOfDccDepart(dccInitType);
	#endif

	#if( CABLECARD_SUPPORT )
	if( (fDsgMode == kDsgAdvanceMode) && (dccInitType==0) )
	{
		BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
		if( pCableCardIf )
			pCableCardIf->CloseAllCableCardClients( this );
		ResetDcdSettings();
        cout << "DCC Init Type 0: CloseAllCableCardClients & Reset CCC/DSG_DIR version!" << endl;
	}
	#endif
}

void BcmDsgClientCtlThread::DsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations )
{
	uint32 twoWayForwarding = (uint32) twoWayForwardingLimitations;
	// serialize the function call by sending kTestDsgTimerEvent to our msg q.
    SerializePublicInterfaceCall( kEventDsgTwoWayForwardingDisabled, (void*) twoWayForwarding );
}

void BcmDsgClientCtlThread::SyncDsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations )
{
	gAlwaysMsgNoFields(fMessageLogSettings) <<
	"\n\n*********************************************" << endl <<
	" eSTB received notification: Disabled-Forwarding-Type 0x" << hex << twoWayForwardingLimitations << dec << endl <<
	"*********************************************" << endl;

	#if defined(CLIENT_INTERFACE_SUPPORT)
	pfDsgClientManager->NotifyClientsOfTwoWayForwardingDisabled(twoWayForwardingLimitations);
	#endif
}

void BcmDsgClientCtlThread::State(DsgCcStatusValue state)
{
	gAlwaysMsgNoFields(fMessageLogSettings)
					<<"\nChanging State from [ " << StateTypeName(fDsgCcState)
					<< " ] to [ " << StateTypeName(state) <<" ]	"
					<< ((fWaitingForSrcNameSubTable)?"Sub State = WAITING FOR SCTE-65 SOURCE SUBTABLE":"")
					<< endl;

	fDsgCcState = state;
}

const char* BcmDsgClientCtlThread::StateTypeName( uint32 value ) const
{
	char* names[] =
	{
		(char *) "INITIAL STATE", 				// = 1 - 
		(char *) "HUNTING FOR DSG CHANNEL",		// = 2
		(char *) "REASSEMBLING DCD",				// = 3
		(char *) "VALIDATING DCD",				// = 4
		(char *) "OPERATIONAL"					// = 5
	};

    return names[ value ];
}

const char* BcmDsgClientCtlThread::DsgStateTypeName( uint32 value ) const
{
	char* names[] =
	{
		(char *) "",
		(char *) "Scanning for DSG info",			// = 1
		(char *) "DSG info present",                 // = 2
		(char *) "DSG info lost, trying to recover", // = 3
		(char *) "DSG not started"                   // = 4
	};

	return names[ value ];
}

const char* BcmDsgClientCtlThread::DsgAPIRetCodeTypeName( uint32 value)
{
    char* names[] =
    {
		(char *) "DSG_API_STATUS_FAIL",			// API call successful
		(char *) "DSG_API_STATUS_SUCCESS",	    //API call failed
		(char *) "DSG_API_STATUS_DOCSIS_ONLY_TEST_MODE"
    };

	if( value == kDocsisOnlyTestMode )
		value = 2;

    return names[ value ];
}

const char* BcmDsgClientCtlThread::DsgTimerEventTypeName( uint32 value)
{
    char* names[] =
    {
		(char *) "",
		(char *) "TDSG1 TIMEOUT - DSG info NOT FOUND - Continuing to scan...",
		(char *) "TDSG2 TIMEOUT - DSG info LOST - Starting Tdsg4 timer",
		(char *) "TDSG3 TIMEOUT - Attempting to establish DOCSIS two way connection",
		(char *) "TDSG4 TIMEOUT - DSG info LOST - Starting downstream scan",
		(char *) "DSG info RECOVERED"
    };

    return names[ value ];
}

const char* BcmDsgClientCtlThread::CmInitStateTypeName( uint32 value )
{
	char* names[] =
	{
		(char *) "",
		(char *) "Initial State", //"Other",       // = 1 - I changed this one so that it makes sense to the diag info user.
		(char *) "Not Ready",                      // = 2
		(char *) "Not Synchronized",               // = 3
		(char *) "PHY Synchronized",               // = 4
		(char *) "Upstream Parameters Acquired",   // = 5
		(char *) "Ranging Complete",               // = 6
		(char *) "Ip Complete",                    // = 7
		(char *) "TOD Established",                // = 8
		(char *) "Security Established",           // = 9
		(char *) "Parameter Transfer Complete",    // = 10
		(char *) "Registration Complete",          // = 11
		(char *) "Operational",                    // = 12
		(char *) "Access Denied"                   // = 13
	};

    return names[ value ];
}

const char* BcmDsgClientCtlThread::DsgModeTypeName( uint32 value) const
{
    char* names[] =
    {
		(char *) "DISABLED",
		(char *) "BASIC",
		(char *) "ADVANCE"
    };

    return names[ value ];
}

// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDsgClientCtlThread::Deinitialize(void)
{
    CallTrace("BcmDsgClientCtlThread", "Deinitialize");

    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // stop our helper threads.
#if 0
    if( pfEstbRemoteIfHelperThread )
    {
        // Reset IP helper.
        delete pfEstbRemoteIfHelperThread;
		pfEstbRemoteIfHelperThread = NULL;
    }

#endif

    // Delete all of the OS objects that I created.
    if( pfEventSet != NULL )
    {
        pfEventSet->RemoveAll();
    }

    delete pfNoDcd;
    pfNoDcd = NULL;

    delete pfInterfaceKeepAliveTimer;
    pfInterfaceKeepAliveTimer = NULL;

	#ifdef APPNAME_SUPPORT
    delete pfNoSnsTableTimer;
    pfNoSnsTableTimer = NULL;
	#endif

    delete pfDestructEvent;
    pfDestructEvent = NULL;

    delete pfRxQueue;
    pfRxQueue = NULL;

    delete pfEventSet;
    pfEventSet = NULL;

    gFuncEntryExitMsg( fMessageLogSettings, "Deinitialize" ) << "Exiting..." << endl;
}

bool BcmDsgClientCtlThread::SerializePublicInterfaceCall( unsigned int msg_code, void *pmsg )
{
    CallTrace("BcmDsgClientCtlThread", "SerializePublicInterfaceCall");

    // assume failure.
    bool msg_send_result = false;

    // serialize the function call by sending the msg to our msg q.
    if( pfRxQueue != NULL )
    {
        // Don't let the depth of the message queue grow in an unbounded
        // way; this could suck up a lot of memory.
        if (pfRxQueue->NumberOfMessages() < 32)
        {
			//cout <<"Sending to msg queue msg = " << msg_code << endl;
            msg_send_result = pfRxQueue->Send( msg_code, pmsg );
        }
        else
        {
            gWarningMsg(fMessageLogSettings, "SerializePublicInterfaceCall")
                << "Too many messages in my queue; discarding request to queue new message!" << endl;

            // send a "service the message queue" event to our event set,
            // so we can process some messages!
            /* This is commented out to resolve timeout problem that occurs
			   when waiting on recvfrom().
			BcmEvent* pevent = pfRxQueue->GetEvent();
            if( pevent )
            {
                pevent->Send();
            } */
        }
    }

    return msg_send_result;
}

// NO DCD EVENT
void BcmDsgClientCtlThread::NoDcdEvent()
{
    CallTrace("BcmDsgClientCtlThread", "NoDcdEvent");

	gAlwaysMsgNoFields (fMessageLogSettings) <<"Incomplete Dcd Timer Expired." <<endl;
	HuntForDsgChannel();
}

bool BcmDsgClientCtlThread::IsInitialState() const
{
	if( fDsgCcState == kInitial )
	{
		return true;
	}

    return false;
}

void BcmDsgClientCtlThread::Reset()
{
	gAlwaysMsgNoFields (fMessageLogSettings) <<"Resetting" << endl;

	State(kInitial);

	// Do not reset client IDs here.

	// Close tunnels only if we are in
	// Advanced mode.
	if( fDsgMode == kDsgAdvanceMode )
	{
		ResetTunnels();
	}

	ResetDcdSettings();
	fConfigSettings.Reset();

	fUcidAvailable = false;
	fCurrentUcid = 0;

	#ifdef APPNAME_SUPPORT
	fWaitingForSrcNameSubTable = false;

	// Stop SNS Timer
	pfNoSnsTableTimer->Stop();
	#endif
}

// Returns the pointer to the singleton program store device.  Most objects
// in the system will use this method rather than being passed a pointer to
// the object.  The singleton pointer will be set up in the base-class
// constructor.
//
// NOTES:  This can return NULL if a singleton has not been set up for the
//         system, so you must check for this condition.
//
//         You must not delete this object!
//
//         You should not store a pointer to the object that is returned,
//         since it may be deleted and replaced with a new one.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the program store device that should be used by the
//      system.
//
BcmDsgClientCtlThread *BcmDsgClientCtlThread::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmDsgClientCtlThread::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }
    return pfSingletonInstance;
}

bool BcmDsgClientCtlThread::DsgStartScan ( void )
{
	// For now, just warn if no clients are registered.
	/*if( pfDsgClientManager->empty() && (pfnDcdPassThroughHandler==NULL) )
	{
		gWarningMsg(fMessageLogSettings, "DsgStartScan")
						<< "Can't start scan. No client is registered. " << endl;
		return false;
	}
	else*/
	{
		// if we are in basic mode make sure that tunnels are
		// open for MAC address type clients.
		if( fDsgMode == kDsgBasicMode )
		{
			if( pfDsgClientManager->OpenTunnelsForMacAddressTypeClients() == false )
			{
				gWarningMsg(fMessageLogSettings, "DsgStartScan")
					<< "Can't start scan. [ DSG Mode = Basic mode ] but no MAC address type client is registered"
					<< endl;
				return false;
			}
		}
	}

	// Change state.
	State( kHuntingForDsgChannel ) ;

	uint32 status = DsgScan(true);

	return status;
}

// Accessors to get/set the DSG global enable bit.
//      mode =
//      enum
//      {
//  	    kDsgDisabledMode = 0,
//  	    kDsgBasicMode,
//  	    kDsgAdvanceMode
//      };
//
// Parameters:
//
//
// Returns:
//
const int BcmDsgClientCtlThread::Mode ( const uint32& mode )
{
	gAlwaysMsgNoFields (fMessageLogSettings) << "Switching DSG Mode from "
											 << DsgModeTypeName(fDsgMode)
											 << " to " << DsgModeTypeName(mode) << endl;
	// PR9091
	if (mode != fDsgMode)
		ResetTunnels();

	#if defined(CLIENT_INTERFACE_SUPPORT)
    // If it changed, tell clients all about it
    if (mode != fDsgMode)
      pfDsgClientManager->NotifyClientsOfAdvancedMode(mode);
	#endif

	fDsgMode = (uint8)mode;

	// Only store to nonvol if DSG-CC is a standalone application.
	// This means it has its own nonvol space.
	#if ( defined(BCM_NONVOL_SUPPORT) && defined(BCM_STANDALONE_DSGCC) )
		pfDsgccNonVolSettings->DsgMode(fDsgMode);
	#endif

	return DsgSetDSGMode( mode );
}

const uint32 BcmDsgClientCtlThread::Mode(void)
{
	#if (BCM_NONVOL_SUPPORT)
	fDsgMode = pfDsgccNonVolSettings->DsgMode();
	#endif

	return (uint32)fDsgMode;
}

// DCD Processing
// Depending on the Total Number Of Fragments the Fragment Mask
// may be a an array of 1-8 uint32 values. Number of Fragments field
// in the DCD message is defined as an 8-bit unsigned integer.
// Based on that, a DCD message can consist of maximum of 255 fragments.
void BcmDsgClientCtlThread::UpdateFragmentMask(uint32 fragmentSeqNumber )
{
	// Find which element of the fragment mask array
	// this fragment sequence number falls into.
	// Each element is uint32. 2^5 = 32
    uint32 fragmentMaskArrayIndex = fragmentSeqNumber >> 5;
    uint32 fragmentBitIndex = 0x1;
    uint32 fragmentBitIndexNum = (fragmentSeqNumber ^ ( fragmentMaskArrayIndex << 5 ));

	fragmentBitIndexNum = fragmentBitIndexNum -1;
	fragmentBitIndex = fragmentBitIndex << fragmentBitIndexNum;
	fFragmentMaskArray[fragmentMaskArrayIndex] |= fragmentBitIndex;
}

// Checks if this fragment already received.
// Updates the mask if this is a new fragment.
const bool BcmDsgClientCtlThread::IsNewFragment(const uint32& fragmentSeqNumber)
{
	// Find which element of the fragment mask array
	// this fragment sequence number falls into.
	// Each element is uint32. 2^5 = 32
    uint32 fragmentMaskArrayIndex = fragmentSeqNumber >> 5;
    uint32 fragmentBitIndex = 0x1;
    uint32 fragmentBitIndexNum = (fragmentSeqNumber ^ ( fragmentMaskArrayIndex << 5 ));

	fragmentBitIndexNum = fragmentBitIndexNum -1;
	fragmentBitIndex = fragmentBitIndex << fragmentBitIndexNum;

	gInfoMsg (fMessageLogSettings, "IsNewFragment") 
		<<"fragmentMaskArrayIndex =  " << fragmentMaskArrayIndex 
		<<"\nfFragmentMaskArray[" << fragmentMaskArrayIndex <<"]=" << hex << fFragmentMaskArray[fragmentMaskArrayIndex] 
		<<"\nfragmentBitIndex = 0x" << hex << fragmentBitIndex 
		<< endl;

	// Check if this bit is already set.
	if( (fFragmentMaskArray[fragmentMaskArrayIndex] & (fragmentBitIndex)) == false )
	{
		// Update the mask
		fFragmentMaskArray[fragmentMaskArrayIndex] |= fragmentBitIndex;

		// Return true indicating that this DCD must be parsed now.

		gAlwaysMsgNoFields (fMessageLogSettings) << "**** New fragment: mask=  " << hex << fFragmentMaskArray[fragmentMaskArrayIndex] << endl;
		return true;
	}

	return false;
}

// Checks if all fragments are received.
const bool BcmDsgClientCtlThread::IsDcdComplete(void) const
{
	if( fParsedFragments == fNumberOfFragments )
	{
		return true;
	}

	return false;
}

void BcmDsgClientCtlThread::ResetDcdSettings(void)
{
	fConfigChangeCount = kDefaultConfigChangeCount;
	fNumberOfFragments = 0;
	fParsedFragments = 0;

	memset(fFragmentMaskArray, 0, sizeof(fFragmentMaskArray));

	if(pfNoDcd)
	{
		pfNoDcd->Stop();
	}
}

//
//
// ValidateDcdAndOpenTunnels() - Subroutine which parses and validates
//      the DCD buffer.
//
// Returns:
//      bool - False is returned if any errors are detected by parser
//          or validation tests.
//
bool BcmDsgClientCtlThread::ValidateDcdAndOpenTunnels()
{
	bool dcdValidated = false;
	bool atLeastOneTunnelIsOpen = false;

    TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
    TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
    while( tunnelSet_it != tunnelSet_end )
    {
		// Do not process defected tunnels.
		if(tunnelSet_it->second.Defected() == false)
		{
			bool openTunnel = false;
			bool openSecretTunnel = false;
			bool ucidCertified = false;
			bool duplicateClient = false;

			// Apply UCID test.
			ucidCertified = tunnelSet_it->second.TestUcid( fCurrentUcid, fUcidAvailable);
			if(ucidCertified)
			{
				gInfoMsg(fMessageLogSettings, "ValidateDcdAndOpenTunnels")
					<<"\nTunnel is UCID Certified." << endl;
				//openTunnel = true;

				// gAlwaysMsgNoFields (fMessageLogSettings) <<(tunnelSet_it->second) << endl;
				// Now check if any client wants us to open this channel.
				// Remember this method will only search client IDs that do
				// not already point to a client object. If we are here
				// because a new DCD is received , then no client ID could be
				// pointing to any client object. If we are here because  we received
				// Source SubTable, then this tunnel might already be open for
				// some clients. If so, do not try to open the tunnel. even
				// if you do so eCM will not process it.
				if( ProcessClientList( &(tunnelSet_it->second), duplicateClient, openTunnel, openSecretTunnel )  == true )
				{
					// Check if this tunnel specifies a client ID that another tunnel already specified.
					// If one of this tunnel's clients is already assigned a tunnel id then we will
					// ignore this tunnel. At this point we know that this tunnel is UCID certified and ready
					// to go if we find a client for it unless that client is already specified by another
					// tunnel.
					if(duplicateClient == true)
					{
						//if( tunnelSet_it->second.TunnelId() == 0 )
						{
							//openTunnel = false;
							//openSecretTunnel = false;
							gAlwaysMsgNoFields (fMessageLogSettings) <<"Tunnel specifies Duplicate Client ID" << endl;
						}
					}
				}
				else
				{
					gInfoMsg(fMessageLogSettings, "ValidateDcdAndOpenTunnels")
						<<"No client requested this tunnel's data." << endl;
				}
			}

			gInfoMsg(fMessageLogSettings, "ValidateDcdAndOpenTunnels") <<"ValidateDcdAndOpenTunnels - openTunnel = " << (openTunnel?"TRUE":"FALSE") <<" openSecretTunnel = " << (openSecretTunnel?"TRUE":"FALSE")  << endl;

			// Tunnel MUST pass both Client and UCID tests to be opened.
			if( (openTunnel || openSecretTunnel) && ucidCertified)
			{
				// If we can open at least one tunnel, DCD is valid.
				dcdValidated = true;

				uint32 tunnelId = tunnelSet_it->second.TunnelId();
				if( tunnelId == 0 )
				{
					if( openTunnel )
						tunnelId = ActivateTunnel(&(tunnelSet_it->second));

					if( (tunnelId != 0) && openTunnel )
					{
						gInfoMsg(fMessageLogSettings, "ValidateDcdAndOpenTunnels")
							<<"Subscribing client(s) to tunnel  " << tunnelId << endl;

						pfDsgClientManager->SubscribeClients(&(tunnelSet_it->second));
						atLeastOneTunnelIsOpen = true;
					}

					pfDsgClientManager->SubscribeClientsWithClassifiers(&(tunnelSet_it->second));
					gInfoMsg(fMessageLogSettings, "ValidateDcdAndOpenTunnels") << *pfDsgClientManager << endl;
				}
				else
				{
					atLeastOneTunnelIsOpen = true;
					gAlwaysMsgNoFields (fMessageLogSettings) <<"Tunnel is already open." << endl;
					pfDsgClientManager->SubscribeClients(&(tunnelSet_it->second));
					pfDsgClientManager->SubscribeClientsWithClassifiers(&(tunnelSet_it->second));
				}
			}
		}

		// increment iterator.
		++tunnelSet_it;
	}

#if 0
	// Check if all CA tunnel requests are satisfied
	// Note that partial-match is not supported yet.
	if( pfDsgClientManager->AllCaTunnelRequestsSatisfied() == false )
	{
		gAlwaysMsgNoFields (fMessageLogSettings) <<"At least one CA tunnel request could not be satisfied." << endl;
		dcdValidated = false;
	}
#endif
	if( dcdValidated )
	{
		State( kOperational);

		// PR9041: Reset number Of Ds Lock Attempts
		fNumberOfDsLockAttempts = kNumberOfFullScansPerDsgMode;

		// NOTIFY CLIENT(s) - some tunnels may have not been opened. DSGCC_TUNNEL_NOT_AVAILABLE OR DSGCC_SUCCESS
		pfDsgClientManager->UpdateStatusNotifyClients();
	}

	#ifdef APPNAME_SUPPORT
	// Do we need to open the broadcast channel?
	// If we could not open any tunnels check if tuning to the broadcast channel
	// will help. Open the broadcast channel that carries SCTE 65 data stream.
	if( (!fWaitingForSrcNameSubTable) && (pfDsgClientManager->MustFindSrcSubTable() == true) )
	{
		if( (dcdValidated == false) && ( OpenBroadcastChannel() == true ) )
		{
			// If we totally rely on the Source SubTable to open tunnels
			// Then we will stay in this state until we get the
			// Source SubTable.
			fWaitingForSrcNameSubTable = true;

			// Start SNS Table timer
			pfNoSnsTableTimer->Start(kNoGoodSnsTablePeriodMillisec);

			gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Must find SourceName SubTable\n"
				<<"Waiting for SNS Table for new client."<< endl;
		}
	}
	#endif

	// At this point DSG-CC could be in one of these states:
	// 1) At least one tunnel is opened. No client provided its ID in text format.
	//    We will move to the operational state.
	//
	// 2) At least one tunnel is opened. At least one client provided its ID
	//		in text format. We will try to open the broadcast tunnel that carries SCTE 65.
	//		We will move to the operational state. If we opened the broadcast tunnel
	//		when Source SubTable comes in
	//		we will try to translate the application names to client IDs and
	//		try to open tunnels for those clients. If we cannot open any tunnel,
	//		we will ignore these clients. Other clients are happy.
	//		We will move to the operational state.
	//
	// 3) No tunnel is open. At least one client provided its ID
	//		in text format. We will try to open the broadcast tunnel
	//		that carries SCTE 65. If we cannot open the broadcast tunnel
	//		either we will go back to hunting for ds channel state.
	//		If we opened the broadcast tunnel, we will move to waiting
	//		for Source SubTable state. When Source SubTable comes in
	//		we will try to translate the application names to client IDs and
	//		try to open tunnels for those clients. If we cannot open any tunnel,
	//      we will move to hunting for ds channel state.

	// 4) No tunnel is open. No client provided its ID in text format.
	//		Hunt for new ds channel.

	// At least one open tunnel is good enough to validate the DCD.
	gAlwaysMsgNoFields(fMessageLogSettings) <<"DCD is " << (dcdValidated?"":"NOT ") <<"VALID." << endl;

	if( atLeastOneTunnelIsOpen == false )
	{
		gAlwaysMsgNoFields(fMessageLogSettings) <<"eCM failed to open any tunnel. " << endl;
		dcdValidated = false;
	}

	return dcdValidated;
}

void BcmDsgClientCtlThread::HuntForDsgChannel()
{
	cout
		<<"Hunt For New Dsg Channel. Resetting settings." << endl;

	// Obviously we are starting
	justStarted = false;

	// Do not reset client IDs here.

	// Close tunnels only if we are in
	// Advanced mode.
	if( fDsgMode == kDsgAdvanceMode )
	{
		ResetTunnels();

		if( pfAppNameTable )
		{
			delete pfAppNameTable;
			pfAppNameTable = NULL;
		}
	}

	ResetDcdSettings();
	fConfigSettings.Reset();

	fUcidAvailable = false;
	fCurrentUcid = 0;

	#ifdef APPNAME_SUPPORT
	// SNS Table stuff;
	fWaitingForSrcNameSubTable = false;

	// Stop SNS Table timer
	pfNoSnsTableTimer->Stop();
	#endif

	// Configure eCM with default values.
	// Default Mode is ADVANCE TWO-WAY mode.
	cout
		<<"\nConfiguring eCM [ Dsg Mode = "
		<< DsgModeTypeName(fDsgMode) << " ]  [ Two-Way Mode = " << (uint32) fEstbDefaultTwoWayMode <<" ]" << endl;
	Mode( fDsgMode );
	DsgEnableDOCSIS ( fEstbDefaultTwoWayMode );

	DsgStartScan();

}

// PR9105:
// There are certain cases when eCM does not realize
// it need to reset its tunnel list.
// Call this method before calling ResetTunnels(),
// otherwise eCM's tunnels will stay open
// unless it realizes (i.e. starts scanning, T4Timeout)
// itself.
void BcmDsgClientCtlThread::ResetEcmTunnels(void)
{
	TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
	TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
	while( tunnelSet_it != tunnelSet_end )
	{
		// Close open tunnels.
		if( tunnelSet_it->second.TunnelId() != 0 )
		{
            cout << "==>Close eCM tunnel " << (int)tunnelSet_it->second.TunnelId() << endl;
			DsgCloseTunnel(tunnelSet_it->second.TunnelId());
		}

		++tunnelSet_it;
	}
}

void BcmDsgClientCtlThread::ResetTunnels(void)
{
	// PR9091:
	// eCM will close the tunnels itself.
	// Do not close eCM's tunnels.
	/*
	TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
	TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
	while( tunnelSet_it != tunnelSet_end )
	{
		// Close open tunnels.
		if( tunnelSet_it->second.TunnelId() != 0 )
		{
			DsgCloseTunnel(tunnelSet_it->second.TunnelId());
		}

		++tunnelSet_it;
	}*/

	fClassifierSetsList.clear();
	fDsgTunnelManager.Reset();

	fConfigSettings.Reset();

	#if( CABLECARD_SUPPORT )
	// For cablecard, all the clients are removed
	pfDsgClientManager->Reset();
	BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
	if( pCableCardIf )
		pCableCardIf->ResetDsgDirVersion();
	#else
	// Remove the client->tunnel link.
	// this does not erase clients.
	// ResetClientTunnelIds();
	pfDsgClientManager->UnsubscribeAll();
	#endif

	// Reset token generator.
	fToken = 1;
}


// DSG-CC/ Client API
// Public method to be called by Client Helper Thread.
void BcmDsgClientCtlThread::RegisterClient( const BcmDsgClient *pClient )
{
    // serialize the function call by sending kRegisterClient to our msg q.
	//
	// We are sending a pointer
    SerializePublicInterfaceCall( kRegisterClient, (void*) pClient);
}

// SyncRegisterClient -
//
// Actions to be performed by DSG-CC thread.
// If this is not the first client registering,
// then the following might be happening based on the
// state:
//
// state != operational
// 1) We might be in process of reassembling DCD,
// or populating tunnel settings table. Tunnel settings
// table is not ready to search for yet. In this case
// we should not do anything here. We already inserted
// the client ID to the client ID table. This client ID
// table will be used to open tunnels when the population
// of the tunnel settings table is completed.
//
// // state = Dsg Channel Found
// 2) Tunnel settings table is populated/channel validated.
// Search tunnel settings to find the desired tunnel for
// this client.
// If the desired tunnel is found:
// 1) Check whether this tunnel can be opened (UCID test, etc)
// 2) Add the client ID to the desired tunnel's forwarding list.
// 3) Open tunnel if it is not already open.
//
// CORNER CASE: What happens if the desired tunnel is not found
// and we already have other happy clients.
void BcmDsgClientCtlThread::SyncRegisterClient(BcmDsgClient *pNewClient)
{
	// Add this client ID to our client list.
	// For now, Client Registrar and DSG-CC will keep separate
	// client lists.

	gAlwaysMsgNoFields(fMessageLogSettings)
		<<"Registering new ClientPort " << pNewClient->ClientPort() << endl;

	switch (fDsgMode)
	{
		case kDsgBasicMode:
			RegisterBasicModeClient(pNewClient);
			break;

		case kDsgAdvanceMode:
#if 0   //Change to let CableCardIf call direct (CABLECARD_SUPPORT)
			if( pfnDcdPassThroughHandler )  //check for cablecard client
				RegisterCableCardAdvancedModeClient((BcmDsgCableCardClient *)pNewClient);
			else
#endif
			RegisterAdvancedModeClient(pNewClient);
			break;

		case kDsgDisabledMode:
			{
				gErrorMsg( fMessageLogSettings, "SyncRegisterClient" )
					<<"Can't accept client registration. Dsg Mode  = " << DsgModeTypeName(fDsgMode)  << endl;
			}
			break;

		default:
			break;
	}
}

// Public method to be called by Client Helper Thread.
void BcmDsgClientCtlThread::DeregisterClient( const BcmDsgClient *pClient )
{
    // serialize the function call by sending kTestDsgRxDCD to our msg q.
	// Delete this memory when this DCD is processed.
    SerializePublicInterfaceCall( kDeregisterClient, (void*) pClient);
}

// Actions to be performed by DSG-CC thread.
void BcmDsgClientCtlThread::SyncDeregisterClient(BcmDsgClient *pClient)
{

	gAlwaysMsgNoFields(fMessageLogSettings)
		<<"SyncDeregisterClient- Deregistering client." << endl;

	// If we actually got a tunnel then unsubscribe
	uint32 tunnelId = pClient->TunnelId();

	if( pfDsgClientManager->UnSubscribe( tunnelId, pClient) == false )
	{
		gErrorMsg( fMessageLogSettings, "SyncDeregisterClient" )
			<<"Failed to remove client from the list. \n" << endl;
		return;
	}

	// Tell Registrar to delete client
	if(pfDsgClientRegistrarThread)
		pfDsgClientRegistrarThread->deleteDsgClient( pClient);

	// Check if we should close its tunnel.
	// Do not close the tunnel if there are other clients of this tunnel.
	if (tunnelId != 0)
	{
		if( pfDsgClientManager->NumberOfClients(pClient->TunnelId()) != 0 )
		{
			return;
		}

		TunnelSettingsListIterator tunnelIt= fDsgTunnelManager.find(tunnelId);
		if(tunnelIt->second.fIsClassifiedPrivateTunnel)
		{
			gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Client of a secret tunnel deregistered. Removing tunnel settings" << endl;

			// Tell eCM to close the tunnel.
			DsgCloseTunnel(tunnelId);

			// Remove it from my tunnel list.
			fDsgTunnelManager.remove(tunnelId);
		}
		else
		{
			// No other client is waiting for this tunnel.
			// Close it.
			CloseTunnel(tunnelId);
		}

		if( pfDsgClientManager->empty() == true )
		{
			gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Last client is deregistered. No tunnel should be open now."
				<<"What do we do now? Current State = "
				<< StateTypeName(fDsgCcState)
				<< *pfDsgClientManager << endl;
		}
	}
}

#if ( BCM_CONSOLE_SUPPORT )
// This static method allows an object (including myself) to get access to
// the singleton command table for controlling objects of my type.  This
// helps avoid order of initialization problems by ensuring that the
// object is created and initialized on-demand.
//
// Parameters:  None.
//
// Returns:
//      A reference to the command table singleton for my class.
//
BcmDsgCommandTable &BcmDsgClientCtlThread::GetCommandTableSingleton(void)
{
    static BcmDsgCommandTable gCommandTable;

    return gCommandTable;
}
#endif

ostream& BcmDsgClientCtlThread::Print( ostream& ostrm ) const
{
#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm
		<< setw(30)
        << "\n     Dsg Mode : " << DsgModeTypeName(fDsgMode)
        << "\n Two-way Mode : " << (fEcmTwoWayMode?"ENABLED":"DISABLED")
		<< "\n        State : " << StateTypeName(fDsgCcState)
		<< "\n    Sub State : " << ((fWaitingForSrcNameSubTable)?"WAITING FOR SCTE-65 SOURCENAME SUBTABLE":"n/a");

	// Print DSG Config Settings
	ostrm << fConfigSettings << endl;

#endif
	return ostrm;
}

void BcmDsgClientCtlThread::PrintClassifierList( void ) const
{
#if !(BCM_REDUCED_IMAGE_SIZE)

    gAlwaysMsgNoFields(fMessageLogSettings)
        << "\n========================="
        << "\nClassifier Settings List:\n";

    // consider classifier settings
    if( fClassifierSetsList.empty() )
    {
		gAlwaysMsgNoFields(fMessageLogSettings)  << "Empty" << endl;
		return;
	}

    ClassifierSettingsListConstIterator cls_it = fClassifierSetsList.begin();
    ClassifierSettingsListConstIterator cls_end = fClassifierSetsList.end();
    while( cls_it != cls_end )
    {
        // found classifier settings.
        gAlwaysMsgNoFields(fMessageLogSettings) << (cls_it->second) << endl;

        // increment iterator.
        ++cls_it;
    }

#endif

}

// Processing tunnel's client ID list.
bool BcmDsgClientCtlThread::ProcessClientList( BcmDsgEstbTunnelSettings *pTunnel, bool &duplicateClient, bool &openTunnel, bool &openSecretTunnel )
{
	bool found = false;

	gInfoMsg(fMessageLogSettings, "ProcessClientList")
		<<"\n Searching client for Tunnel "
		<< "MAC = [ " << pTunnel->TunnelAddress() << " ] "
		;

	// Processing tunnel's client ID list.
	ClientIdListIterator clientId_it = pTunnel->fClientIdList.begin();
	ClientIdListIterator clientId_end = pTunnel->fClientIdList.end();
	while( clientId_it != clientId_end )
	{
		gInfoMsg(fMessageLogSettings, "ProcessClientList") << *clientId_it << endl;

		// Search the client list for this client
		// FindAndSetClients() might find more than one client with the same ID.
		// Two different clients may want to receive the same tunnel data.
		// If found let the client hold this token.
		found = pfDsgClientManager->FindAndSetClients( pTunnel, (&(*clientId_it)), fToken, duplicateClient, openTunnel, openSecretTunnel );

		clientId_it++;
	}

	pTunnel->Token( fToken );
	fToken++;

	return found;
}


const bool BcmDsgClientCtlThread::OpenBroadcastChannel(void)
{
	bool openTunnel = false;

	// Just look for that tunnel with SCTE 65 broadcast id.
	// For now I will open the first one I find.
    TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
    TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
    while( tunnelSet_it != tunnelSet_end )
    {
		if( (fBroadcastTunnelMac == kAllZerosMac) || (tunnelSet_it->first == fBroadcastTunnelMac))
		{

			// Find the tunnel with  SCTE 65 data.
			if( tunnelSet_it->second.HasScte65BroadcastData() )
			{
				// Return if its is already open.
				if( tunnelSet_it->second.TunnelId() != 0 )
				{
					gAlwaysMsgNoFields(fMessageLogSettings) <<"Broadcast Tunnel is already OPEN "<< endl;
					return true;
				}

				openTunnel = true;
			}

			if( openTunnel )
			{
				// Load the tunnel mac address into "t_mac_adr" type mac variable.
				t_mac_adr mac;
				tunnelSet_it->first.Get(mac.macaddress[0], mac.macaddress[1], mac.macaddress[2],
					mac.macaddress[3], mac.macaddress[4], mac.macaddress[5]);

				BcmOctetBuffer octetBuffer;
				// Always in advance mode if we are processing a DCD.
				if( fDsgMode == kDsgAdvanceMode )
				{
					ClassifierSettingsListConstIterator cls_it = tunnelSet_it->second.begin();
					ClassifierSettingsListConstIterator cls_end = tunnelSet_it->second.end();
					while( cls_it != cls_end)
					{
						cls_it->second.WriteTo(octetBuffer);

						cls_it++;
					}
				}

				// For now I will open the first one I find.
				// Make the API call.
				DsgTunnelSettings tunnelSettings;
				tunnelSettings.pTunnelAddress = &mac;
				tunnelSettings.clientIdType = kBroadcastType;
				tunnelSettings.clientIdValue = kBroadcastSCTE65;
				tunnelSettings.numOfClassifiers = tunnelSet_it->second.size();
				tunnelSettings.pClassifierList = (t_dsg_classifier*) octetBuffer.AccessBuffer();

				fBroadcastTunnelId = DsgOpenTunnel(&tunnelSettings); 

				if( fBroadcastTunnelId != 0 )
				{
					// Set tunnel id. We will use this to close tunnels.
					tunnelSet_it->second.TunnelId(fBroadcastTunnelId);
					gAlwaysMsgNoFields(fMessageLogSettings) <<"Broadcast Tunnel Id is " <<fBroadcastTunnelId << endl;
				}

				return true;
			}
		}

        // increment iterator.
        ++tunnelSet_it;
    }

	gAlwaysMsgNoFields(fMessageLogSettings) <<"FAILED to open Broadcast Tunnel"<< endl;
	return false;
}

// This will create a new tunnel settings object and insert it to the list
// once the tunnel is opened.
const uint32 BcmDsgClientCtlThread::OpenTunnel(BcmMacAddress &tunnelAddress, BcmDsgClassifierSettings* pClassifierSetting )
{
	// Do not add the same tunnel address twice in basic mode.
	if(fDsgMode == kDsgBasicMode) 
	{
		TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.find( tunnelAddress );
		if( tunnelSet_it != fDsgTunnelManager.end() )
		{
			// In BASIC mode, all tunnels in the tunnel list must have a non-zero tunnel ID
			// because DSG-CC inserts only tunnels with non-zero ID to the list. 
			if( tunnelSet_it->second.TunnelId() == 0 )
				gWarningMsg(fMessageLogSettings, "OpenTunnel") <<"In BASIC mode, tunnel list should not have an entry [" << tunnelAddress << "] with tunnel ID=0!" << endl;
			return tunnelSet_it->second.TunnelId();
		}
	}
	
	// No tunnel with this address exists. So create a new one.
	BcmDsgEstbTunnelSettings newTunnel;

	// Set the mac address. Remember, at this point we failed to find a DCD
	// AND at least one client registered with MAC ADDRESS ID type.
	// In basic mode, eCM checks only MAC ADDRESS of the DSG packet
	// to lock on a downstream.
	newTunnel.TunnelAddress(tunnelAddress);

	if( pClassifierSetting )
	{
		// This tunnel's MAC address is included in the DCD but non of its classifiers match client-specified classifier.
		// We have to open a private tunnel.
		newTunnel.fIsClassifiedPrivateTunnel = true;

		// Add classifier to the tunnel.
		if( newTunnel.add(*pClassifierSetting) == false )
		{
			gErrorMsg(fMessageLogSettings, "OpenTunnel") <<" Failed to add classifier to tunnel. Will not open the tunnel. "  << endl;
			return 0;
		}
	}

	// Do not open duplicate tunnels. 
	uint32 tunnelId = 0;
	if( !fDsgTunnelManager.find(newTunnel) )
	{
		tunnelId = ActivateTunnel(&newTunnel);
	}

	if( tunnelId != 0)
	{
		gAlwaysMsgNoFields (fMessageLogSettings) <<"eCM opened tunnel [ " << tunnelAddress <<" ] tunnel ID =  " << tunnelId << endl;
		// Set tunnel id. We will use this to close tunnels.
		// newTunnel.TunnelId(tunnelId);

		if( !fDsgTunnelManager.add(newTunnel) )
		{
			gErrorMsg(fMessageLogSettings, "OpenTunnel") <<" Failed to add tunnel to tunnel list. Telling eCM to close the tunnel."  << endl;
			DsgCloseTunnel(tunnelId);
			return 0;
		}

		return tunnelId;
	}
	else
	{
		gErrorMsg(fMessageLogSettings, "OpenTunnel") <<"Failed to open tunnel for " << tunnelAddress << endl;
	}
	
	return 0;
}


// Open a tunnel that exists.
const uint32 BcmDsgClientCtlThread::ActivateTunnel( BcmDsgEstbTunnelSettings *pTunnel)
{
	// Load the tunnel mac address into "t_mac_adr" type mac variable.
	t_mac_adr mac;
	pTunnel->TunnelAddress().Get(mac.macaddress[0], mac.macaddress[1], mac.macaddress[2],
		mac.macaddress[3], mac.macaddress[4], mac.macaddress[5]);

	BcmOctetBuffer octetBuffer;
	if( fDsgMode == kDsgAdvanceMode )
	{
		// Write classifier settings to the buffer.
		pTunnel->WriteTo(octetBuffer);
	}
	//cout << octetBuffer << endl;

	// Make the API call.
	// Note that if we are in basic mode or the tunnel does not have any classifiers, pTunnel.size() is zero
	// and octetBuffer.AccessBuffer() is NULL.

	DsgTunnelSettings tunnelSettings;
	tunnelSettings.pTunnelAddress = &mac;

	// Now get the client ID type and value.
	// This tunnel settings object is designed to hold a list of client ID type/values
	// but we will get the head of the list for now.
	ClientIdListIterator clientId_it = pTunnel->fClientIdList.begin();
	ClientIdListIterator clientId_end = pTunnel->fClientIdList.end();
	if( clientId_it != clientId_end )
	{
		tunnelSettings.clientIdType = clientId_it->IdType();
		tunnelSettings.clientIdValue = clientId_it->Id();
		//cout << "TYPE:  " << clientId_it->IdType()   << "VALUE: " << tunnelSettings.clientIdValue << endl;

		t_mac_adr macAddressClientIdVal;
		clientId_it->ClientAddress().Get(macAddressClientIdVal.macaddress[0], macAddressClientIdVal.macaddress[1], macAddressClientIdVal.macaddress[2],
			macAddressClientIdVal.macaddress[3], macAddressClientIdVal.macaddress[4], macAddressClientIdVal.macaddress[5]);
		
		tunnelSettings.pMacAddressClientIdVal = &macAddressClientIdVal;
	}

	tunnelSettings.numOfClassifiers = pTunnel->size();
	tunnelSettings.pClassifierList = (t_dsg_classifier*) octetBuffer.AccessBuffer();		

	uint32 tunnelId = 0;
	tunnelId = DsgOpenTunnel(&tunnelSettings);

	// Set tunnel id. We will use this to close tunnels.
	if( tunnelId != 0)
	{
		pTunnel->TunnelId(tunnelId);
	}

	return tunnelId;
}


// UCID is received. At this point, we must have processed and validated a DCD
// (if in advanced mode). All tunnels in the tunnel list that do not specify a UCID
// are already processed. Now thta we have a UCID we can process the rest of the tunnels
// that specify a UCID.
const uint32 BcmDsgClientCtlThread::ProcessUcid(void)
{

    TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
    TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
    while( tunnelSet_it != tunnelSet_end )
    {
		bool openTunnel = false;
		bool ucidCertified = false;

		// Only process the ones that specify a UCID.
		if(tunnelSet_it->second.UcidCertified() == false)
		{
			//gAlwaysMsgNoFields (fMessageLogSettings) <<"Tunnel is not UCID Certified." << endl;

			// Apply UCID test.
			ucidCertified = tunnelSet_it->second.TestUcid( fCurrentUcid, fUcidAvailable);
			if(ucidCertified)
			{
				gAlwaysMsgNoFields (fMessageLogSettings)
                    <<"Tunnel ID" << tunnelSet_it->second.TunnelId() << " is now UCID Certified with new UCID = " << (int)fCurrentUcid << endl;

				// gAlwaysMsgNoFields (fMessageLogSettings) <<(tunnelSet_it->second) << endl;
				// Now check if any client wants us to open this channel.
				// Remember this method will only search client IDs that do
				// not already point to a client object. If we are here
				// because a new DCD is received , then no client ID could be
				// pointing to any client object. If we are here because  we received
				// Source SubTable, then this tunnel might already be open for
				// some clients. If so, do not try to open the tunnel. even
				// if you do so eCM will not process it.
				if( ProcessAllClients( &(tunnelSet_it->second) ) == true )
				{
					if( tunnelSet_it->second.TunnelId() == 0 )
					{
						openTunnel = true;
					}
					else
					{
						// This cannot happen??
						gAlwaysMsgNoFields (fMessageLogSettings) <<"ERROR - Tunnel is already open." << endl;
					}
				}
				else
				{
					gInfoMsg(fMessageLogSettings, "ProcessUcid")
						<<"No client requested this tunnel's data." << endl;
				}
			}
            else
                gAlwaysMsgNoFields (fMessageLogSettings)
                     <<"Tunnel ID" << tunnelSet_it->second.TunnelId() << " is not UCID Certified." << endl;

			// Tunnel MUST pass both Client and UCID tests to be opened.
			if( openTunnel && ucidCertified)
			{
				uint32 tunnelId = ActivateTunnel(&(tunnelSet_it->second));

				if( tunnelId != 0 )
				{

					cout
						<<"Subscribing client(s) to tunnel  " << tunnelId << endl;
					pfDsgClientManager->SubscribeClients(&(tunnelSet_it->second));

					pfDsgClientManager->SubscribeClientsWithClassifiers(&(tunnelSet_it->second));
				}
			}
		} // end UcidCertified

		tunnelSet_it++;
	} // end while

	// Update tunnel list.
	// Check if any previously open tunnel lost all of its clients because we switched the clients
	// to a new tunnel with UCID. If so, close the tunnel. No client is waiting for its data.
	CloseDeadTunnels();
	return 0;
}


// Processing tunnel's client ID list.
bool BcmDsgClientCtlThread::ProcessAllClients( BcmDsgEstbTunnelSettings *pTunnel )
{
	bool found = false;

	gInfoMsg(fMessageLogSettings, "ProcessAllClients")
		<<"\n Searching client for Tunnel "
		<< "MAC = [ " << pTunnel->TunnelAddress() << " ] "
		<< endl;

	// Processing tunnel's client ID list.
	ClientIdListIterator clientId_it = pTunnel->fClientIdList.begin();
	ClientIdListIterator clientId_end = pTunnel->fClientIdList.end();
	while( clientId_it != clientId_end )
	{
		gInfoMsg(fMessageLogSettings, "ProcessAllClients")  << *clientId_it << endl;
		//if ( clientId_it->ClientPointerList().empty() ||   )
		{
			// Search the client list for this client
			// FindAndSetClients() might find more than one client with the same ID.
			// Two different clients may want to receive the same tunnel data.
			// If found let the client hold this token.
			if( pfDsgClientManager->SetOrSwitchClients( (&(*clientId_it)), fToken ))
			{
				found = true;
			}
		}

		clientId_it++;
	}

	pTunnel->Token( fToken );
	fToken++;

	return found;
}

// Close tunnels that do not serve any client.
bool BcmDsgClientCtlThread::CloseDeadTunnels( void)
{
    TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
    TunnelSettingsListIterator tunnelSet_end = fDsgTunnelManager.end();
    while( tunnelSet_it != tunnelSet_end )
    {
		if( (tunnelSet_it->second.TunnelId() != 0) && (tunnelSet_it->second.TunnelId() != fBroadcastTunnelId) )
		{
			// Search client list by tunnel id. If the search returns no clients
			// then close the tunnel.
			if( pfDsgClientManager->find(tunnelSet_it->second.TunnelId()) == pfDsgClientManager->end() )
			{
				gWarningMsg(fMessageLogSettings, "CloseDeadTunnels")
					<< "Closing DEAD Tunnel ID = " << tunnelSet_it->second.TunnelId() << endl;

				DsgCloseTunnel(tunnelSet_it->second.TunnelId());

				// Remove this tunnel setting if it is a non-DCD (secret tunnel) entry.
				if(tunnelSet_it->second.fIsClassifiedPrivateTunnel)		
				{
					fDsgTunnelManager.remove(tunnelSet_it->second.TunnelId());
				}
				else
				{
					// Set the tunnel id to zero after eCM closes it.
					uint32 closed = 0;
					tunnelSet_it->second.TunnelId(closed);
				}
			}
		}

		tunnelSet_it++;
	}
	return 0;
}

bool BcmDsgClientCtlThread::CloseTunnel(const uint32& id)
{
	if( DsgCloseTunnel(id) == true )
	{
		// Search client list by tunnel id. Warn if search finds
		// clients
		if( pfDsgClientManager->find(id) != pfDsgClientManager->end() )
		{
		    gWarningMsg(fMessageLogSettings, "CloseTunnel")
			<< "Closing tunnel ID = " << id <<"but it has active clients. " << endl;
		}
		else
		{
			// Remove tunnel completely if in basic mode.
			if( fDsgMode == kDsgBasicMode )
			{
				// Now remove it from the list
				fDsgTunnelManager.remove(id);
			}
			else
			{
				// No clients left for this tunnel.
				TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.find(id);
				if( tunnelSet_it != fDsgTunnelManager.end() )
				{
					// Make sure to reset the tunnel id.
					uint32 closed = 0;
					tunnelSet_it->second.TunnelId(closed);
				}
			}
		}
		return true;
	}

	return false;
}

void BcmDsgClientCtlThread::RegisterBasicModeClient(BcmDsgClient *pNewClient)
{
	bool openTunnel = false;
	bool found = false;

	// Return immediately if the client is not MAC-ADDRESS type.
	// We cannot open tunnels for any other type clients in BASIC mode.
	if( pNewClient->IdType() != kMacAddressType )
	{
		gErrorMsg( fMessageLogSettings, "RegisterBasicModeClient" )
				<<"We register only MAC-ADDRESS type clients in DSG BASIC mode.\n"
				<< endl;
		return;
	}

	// Read client's ID. This is the tunnel address.
	BcmMacAddress mac = pNewClient->ClientAddress();

	uint32 tunnelId = 0;
	TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.find(mac);
	if(tunnelSet_it != fDsgTunnelManager.end())
	{
		// Tunnel is in the list
		// Is it open?
		if( tunnelSet_it->second.TunnelId() != 0 )
		{
			found = true;
			tunnelId = tunnelSet_it->second.TunnelId();
		}
		else
		{
			gErrorMsg( fMessageLogSettings, "RegisterBasicModeClient" )
				<< "Tunnel is in the list but is closed. This should not happen in DSG BASIC mode." << endl;
		}
	}

	// Open the tunnel if not open.
	if( found == false)
	{
		tunnelId = OpenTunnel(mac);
		if( tunnelId == 0 )
		{
			gErrorMsg( fMessageLogSettings, "RegisterBasicModeClient" )
				<< "Tunnel requested by the client cannot be opened!" << endl;
			//return;
		}
	}

	// Add client to the list, subscribe if tunnel is open.
	if( pfDsgClientManager->Subscribe( tunnelId, pNewClient) == false )
	{
		gErrorMsg( fMessageLogSettings, "RegisterBasicModeClient" )
			<<"Failed to subscribe client. \n" << endl;
		return;
	}

	// Return if a tunnel cannot be opened.
	if( tunnelId == 0 )
	{
		return;
	}

	// Start scanning if this is the first client.
	if(pfDsgClientManager->size() == 1 )
	{
		// If this is the first client
		// then we must be in the initial state.
		if( !(fDsgCcState == kInitial) )
		{
			gWarningMsg( fMessageLogSettings, "RegisterBasicModeClient" )
				<< "First client registered and we are not in initial state!!" << endl;
		}
    
        // 09/29/07: During DSG-02 test 1.2, the test requires that the current basic mode 
        // MAC address be replaced with a new MAC address.  When this change occurs, 
        // the device is supposed to remain on the same downstream without re-initializing.
        // Added local var FirstTimeRegBasicMode to only scan once.
        if( FirstTimeRegBasicMode )
        {
            FirstTimeRegBasicMode = false;
    		HuntForDsgChannel();
        }

		return;
	}
}

void BcmDsgClientCtlThread::RegisterAdvancedModeClient(BcmDsgClient *pNewClient)
{
	if( !TestAndProcessApplicationNameTypeClient(pNewClient) )
	{
		return;
	}

	if(fDsgCcState < kOperational)
	{
		// Insert it to the list now. It will be processed
		// when tunnel info is present.
		uint32 unsubscribed = 0;
		pfDsgClientManager->Subscribe( unsubscribed, pNewClient);
	}

	// No DCD is present or DCD is present but no tunnels could be opened yet.
	if(fDsgCcState < kValidatingDcd)
	{
		gAlwaysMsgNoFields(fMessageLogSettings)
					<<"Inserted new tunnel request to the list now. It will be processed when DCD is present"<< endl;
		return;
	}
	// DCD is already received but not validated yet because no tunnel requests were pending at the time.
	else if(fDsgCcState == kValidatingDcd)
	{
		if( (ValidateDcdAndOpenTunnels() == false))// && pfDsgClientManager->CaTunnelRequestPresent())
		{
			if( !fWaitingForSrcNameSubTable) 
			{
				// PR13043:When DSG-CC receives a new DCD that contains DSG channel TLVs,
				// it should set/update eCM's DSG channel list regardless of whether
				// it can find the requested tunnels or not.
				// While hunting for a DSG channel, DSG-CC treats every DCD as a new DCD.
				// In operational mode, DSG-CC processes DCDs only with a new config change count(CCC).
				//
				// While hunting for a DSG channel, DSG-CC might get a DCD that only contains a DSG channel list.
				// DSG-CC should only process the DSG channel list if it gets a DCD with a different CCC, otherwise
				// if it sets the same channel list every time it finds the same DCD eCM will get stuck trying
				// the first channel in the DSG channel list because when DSG channel list is updated channel list
				// index is set to point to the head of the list.
				if( pfDcdMsg->ConfigurationChangeCount() != fLastConfigChangeCount )
				{
					fLastConfigChangeCount = pfDcdMsg->ConfigurationChangeCount();
					ApplyDcdConfigSettings();
				}

				// DCD not valid. Hunt.
				HuntForDsgChannel();
				return;
			}
		}

		// Tunnel found in the DCD. Notify the client.
		pNewClient->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelOpen); 

		fLastConfigChangeCount = pfDcdMsg->ConfigurationChangeCount();
		ApplyDcdConfigSettings();
		return;
	}

	// Process tunnel request.
	bool openTunnel = false;
	bool found = false;
	bool openSecretTunnel = false;
	uint32 tunnelId = 0;

	// Now search for a tunnel that serves this client.
	//
	TunnelSettingsListIterator tunnelSet_it = fDsgTunnelManager.begin();
	while( tunnelSet_it != fDsgTunnelManager.end())
	{
		BcmDsgClientId* pClientId = tunnelSet_it->second.FindClient(pNewClient);
		if( pClientId != NULL )
		{
			// we have a match for client.
			// Open this tunnel if it is not already open.
			// If the tunnel ID has a value other than zero then the tunnel is open.
			tunnelId = tunnelSet_it->second.TunnelId();
			if( tunnelId == 0  )
			{
				// At this point we know that
				if(tunnelSet_it->second.fUcidCertified == true)
				{
					if( pNewClient->fIpClassifierSetsSpecified && (!tunnelSet_it->second.IsEqualClassifier(pNewClient->fIpClassifierSets)) )
					{
						// Secret tunnel was an requirement from DCAS, which is obsoleted.
						//openSecretTunnel = true;
					}
					else
					{
						openTunnel = true;
						// Found the matching classifier, exiting the while loop.
						break;
					}
				}
			}
			else
			{
				gInfoMsgNoFields(fMessageLogSettings)
						<< "Tunnel is already open ID = " << tunnelId << endl;

				if( !(pNewClient->fIpClassifierSetsSpecified) ||
					 (pNewClient->fIpClassifierSetsSpecified && tunnelSet_it->second.IsEqualClassifier(pNewClient->fIpClassifierSets)))
				{
					found = true;

					// Update the client 
					pNewClient->TunnelId(tunnelId);

					gAlwaysMsgNoFields(fMessageLogSettings)  <<"Subscribed client to tunnel [ ID = " << tunnelId << " ]" << endl; 
					break;
				}
			}
		}
		// continue to search.
		tunnelSet_it++;
	}
	
	// Done searching for a valid tunnel. Now process the results.
	if( openTunnel == true )
	{
		tunnelId = ActivateTunnel(&(tunnelSet_it->second));

		if( tunnelId == 0 )
		{
			gErrorMsg( fMessageLogSettings, "RegisterAdvancedModeClient" ) 
				<< "Tunnel requested by the client cannot be activated!" << endl;
			//return;
		}
		else
		{
			// Tunnel is open now.
			pNewClient->TunnelId(tunnelId);	
		}
	}

	// Secret tunnel was an requirement from DCAS, which is obsoleted.
#if 0
	if( openSecretTunnel == true )
	{	
		BcmDsgClassifierSettings fClsfrSettings;
		fClsfrSettings.fIpClassifierSetsSpecified = true;
		fClsfrSettings.fIpClassifierSets = pNewClient->fIpClassifierSets;

		BcmMacAddress mac = tunnelSet_it->second.TunnelAddress();
		tunnelId = OpenTunnel(mac, &fClsfrSettings);

		if( tunnelId == 0 )
		{
			gErrorMsg( fMessageLogSettings, "RegisterAdvancedModeClient" ) 
				<< "Tunnel requested by the client with classifier cannot be opened!" << endl;
			return;
		}
		else
		{
			// Tunnel is open now.
			pNewClient->TunnelId(tunnelId);	
		}
	}
#endif
	// Tell the client its tunnel's id if we found one for it.
	// If we found a tunnel for this client then it was either
	// already open or not open and we opened it.
	// If we cannot open a tunnel for this client, we will still keep the tunnel request to fulfill later.
	if( pfDsgClientManager->Subscribe( pNewClient->TunnelId(), pNewClient) == false )
	{
		gErrorMsg( fMessageLogSettings, "RegisterAdvancedModeClient" )
			<<"Failed to subscribe client to tunnel = \n" << (tunnelSet_it->second) << endl;
		return;
	}

	if( (!openTunnel && !found && !openSecretTunnel) || (tunnelId == 0) )
	{	
		gWarningMsgNoFields(fMessageLogSettings)			
			<<"Cannot find an existing tunnel or open a new one for this client!" << endl;		

		if( pNewClient->IdType() == kCaSystemType )
		{
			gAlwaysMsgNoFields(fMessageLogSettings) 
				<< "CA tunnel is not found in the DCD." << endl;

			// If the requested CA tunnel is not found, the DSG-CC 
			// shall request the eCM to scan for a different DCD.
			HuntForDsgChannel();
		}
		else
		{
			gAlwaysMsgNoFields(fMessageLogSettings) 
				<< "NOTIFY CLIENT - DSGCC_TUNNEL_NOT_AVAILABLE" << endl;

			pNewClient->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelNotAvailable); 
		}
	}
	else
	{
		gAlwaysMsgNoFields(fMessageLogSettings) 
			<< "NOTIFY CLIENT - DSGCC_TUNNEL_OPEN" << endl;

		pNewClient->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelOpen); 
	}
}

#if (CABLECARD_SUPPORT)
void BcmDsgClientCtlThread::RegisterCableCardAdvancedModeClient(BcmDsgCableCardClient *pNewClient)
{
	uint32 tunnelId;
    BcmDsgEstbTunnelSettings *pTunnel = &pNewClient->CableCardTunnelSetting();

	// Copy the client ID type and value into CableCardTunnelSetting object
	ClientIdListIterator clientId_it = pTunnel->fClientIdList.begin();
	ClientIdListIterator clientId_end = pTunnel->fClientIdList.end();
	if( clientId_it != clientId_end )
	{
		clientId_it->IdType( pNewClient->IdType() );
		clientId_it->Id( pNewClient->Id() );
		//cout << clientId_it->IdTypeName(clientId_it->IdType()) << "(TYPE: " << clientId_it->IdType() << ")   ID: " << clientId_it->Id() << endl;
		BcmMacAddress macAddressClientIdVal = pNewClient->ClientAddress();
		clientId_it->ClientAddress( macAddressClientIdVal );
	}

	// This function is called from cablcardIf object which always remove all open tunnels prior arriving here.
	// It is save to just open the tunnel
	tunnelId = ActivateTunnel(&pNewClient->CableCardTunnelSetting());

	if( tunnelId )
	{
            // Add object & tunnel mac address to the tunnel list
		fDsgTunnelManager.add(pNewClient->CableCardTunnelSetting());

		gAlwaysMsgNoFields(fMessageLogSettings) 
			<< "NOTIFY CLIENT - DSGCC_TUNNEL_OPEN" << endl;

		pNewClient->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelOpen); 
	}
	else
	{
		gErrorMsg( fMessageLogSettings, "RegisterCableCardAdvancedModeClient" )
			<< "Tunnel requested by the client cannot be opened!" << endl;
		return;
	}

	// Add client to the list, subscribe if tunnel is open.
	if( pfDsgClientManager->Subscribe( tunnelId, pNewClient) == false )
	{
		gErrorMsg( fMessageLogSettings, "RegisterCableCardAdvancedModeClient" )
			<<"Failed to subscribe client. \n" << endl;
		return;
	}
}

void BcmDsgClientCtlThread::CableCardEnableTwoWayMode(uint8 enable)
{
    bool benable = (enable)?true:false;

    DsgEnableDOCSIS (benable);
    fEcmTwoWayMode = fEstbDefaultTwoWayMode = benable;
}

#endif

void BcmDsgClientCtlThread::HandleTestEvent( const uint32 &msg_code, void* buf )
{
#if ( BCM_CONSOLE_SUPPORT )

    uint32 result = 0;
	bool retVal = 0;
    uint32 event = (uint32) buf;

	switch(msg_code)
	{
        case kTestDsgSetDSGMode:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"\n eSTB is calling DsgSetDSGMode() "<< endl;

				 result = Mode((uint8) event);

				 // Decode result.
				 if( result == kFuncInvalid )
				 {
					 gErrorMsg( fMessageLogSettings, "HandleTestEvent" ) <<"DsgSetDSGMode() returned kFuncInvalid\n" << endl;
				 }
				 else
				 {
					 //fDsgMode = (uint8) buf;
					 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"DsgSetDSGMode() returned " << DsgAPIRetCodeTypeName(result) << endl;
				 }
			}
            break;

        case kTestDsgEnableDocsis:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"eSTB is calling DsgEnableDOCSIS()" << endl;

				 retVal = DsgEnableDOCSIS ((uint8) event);
				 fEstbDefaultTwoWayMode = (uint8) event;
				 fEcmTwoWayMode = fEstbDefaultTwoWayMode;
				 // Decode result.
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"DsgEnableDOCSIS() returned " << DsgAPIRetCodeTypeName((uint32) retVal) << endl;
			}
            break;

        case kTestDsgSetChannelList:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"\n eSTB is calling DsgSetChannelList() "<< endl;
				
				 result = DsgSetChannelList((uint16)1, &event );

				 // Decode result.
				 if( result == kFuncInvalid )
				 {
					 gErrorMsg( fMessageLogSettings, "HandleTestEvent" ) <<"DsgSetChannelList() returned kFuncInvalid\n" << endl;
				 }
				 else
				 {
					 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"DsgSetChannelList() returned " << DsgAPIRetCodeTypeName(result) << endl;
				 }
			}
            break;

		case kTestDsgResetChannelList:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"\n eSTB is calling DsgResetChannelList() "<< endl;
				 retVal = DsgResetChannelList();

				 // Decode result.
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"DsgResetChannelList() returned " <<DsgAPIRetCodeTypeName((uint32) retVal) << endl;
			}
            break;

		case kTestDsgScan:
			{
				 if( (fDsgMode == kDsgDisabledMode) || !((bool) event) )
				 {
					 DsgScan((bool) event);
				 }
				 else
				 {

					 HuntForDsgChannel();
				 }
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"\n ***** eSTB is calling DsgScan() ******" << endl;

			}
            break;


		case kTestDsgStatus:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<"\n eSTB is calling DsgStatus() \n\n" << endl;

				 result = DsgStatus();

				 // Decode result.
				 if( result == 0 ) // kFail
				 {
					 gErrorMsg( fMessageLogSettings, "HandleTestEvent" ) <<"Function failure." << endl;
				 }
				 else
				 {
					gAlwaysMsgNoFields (fMessageLogSettings) << DsgStateTypeName(result) << endl;
				 }
			}
            break;

		case kTestDsgDocsisStatus:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<" eSTB is calling DsgDocsisStatus() " << endl;

				 result = DsgDocsisStatus();

				 // Decode result.
				 if( result == 0 ) // kFail
				 {
					 gErrorMsg( fMessageLogSettings, "HandleTestEvent" ) <<"Function failure." << endl;
				 }
				 else
				 {
					gAlwaysMsgNoFields (fMessageLogSettings) << CmInitStateTypeName(result) << endl;
				 }
			}
            break;

		case kTestDsgSetTimer:
			{
				 gInfoMsg(fMessageLogSettings, "HandleTestEvent") <<" eSTB is calling DsgSetTimer() " << endl;

				 uint8 timerID = ((uint32) buf) >> 16;

				 result = DsgSetTimer(timerID, ((uint32)buf) & 0xFFFF);

				 // Decode result.
				 if( result == 0 ) // kFail
				 {
					 gErrorMsg( fMessageLogSettings, "HandleTestEvent" ) <<"Function failure." << endl;
				 }
				 else
				 {
					gAlwaysMsgNoFields (fMessageLogSettings) << CmInitStateTypeName(result) << endl;
				 }
			}
            break;

		default:
			gErrorMsg( fMessageLogSettings, "HandleTestEvent" )
				<< "Messge code " << msg_code << "not recognized." << endl;
			break;
	}
#endif // #if ( BCM_CONSOLE_SUPPORT )
}

bool BcmDsgClientCtlThread::KeepAlive( void )
{
#if ( defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) && (!defined ECM_STUB_API))

	static uint16 LastHearBeatCount = 0;
	uint32 ret_val;
    uint16 EcmHearBeatCount=0;

    if( IfIsDown )
    {
        // bring interface up
		ConfigureEcmInterface();
        // Schedule next heartbeat check.
        pfInterfaceKeepAliveTimer->Start(kRemoteInterfaceKeepAlivePeriod);
        IfIsDown = false;

        // Return true for the link is up
        return true;
    }
    
   	// Get eCM's HeartBeat Count
    ret_val = DsgGetEcmHeartBeat( &EcmHearBeatCount );

	// Try to establish the link if it is down.
	if (ret_val == kFuncFail)
	{
		fLinkUp = false;
		gAlwaysMsgNoFields(fMessageLogSettings) << "KeepAlive: ERROR - eCM-Link is down!" << endl;

		// Refresh the settings in case they are lost.
		ConfigureEcmInterface();

		// Do not block trying to restore. Schedule a new try.
		pfInterfaceKeepAliveTimer->Start(1000);

		// Return false if the link is down
		return false;
	}

	// Link is restored if we reached here..
	if(!fLinkUp)
	{
		fLinkUp = true;
		gAlwaysMsgNoFields(fMessageLogSettings) << "KeepAlive: eCM-Link is Up!" << endl;

		// Check if any client deregistered while the link is down
		// and the tunnel stayed open because eSTb could not communicate to eCM.
		CloseDeadTunnels();
	}

	// informational print when ESTB get a new ip address.
	PrintNewEstbPrimaryIpAddress();

	// inform eCM when eSTB gets a new ip address.
	CheckNewEstbPrimaryIpV4Address(false);

	// printf("LastHearBeatCount %d, EcmHearBeatCount %d\n", LastHearBeatCount, EcmHearBeatCount );
	// Check if eCM has reset.
	if( LastHearBeatCount > EcmHearBeatCount )
	{
		gAlwaysMsgNoFields(fMessageLogSettings) <<"eCM RESET. Reconfiguring eCM." << endl;

		// OC-ATP 14.1.1 step 65: Moved to EstbRemoteInterfaceStub.c
		//#if (CABLECARD_SUPPORT)
		//BcmIndicateEcmReset();
		//#endif

		// Set RI address
		if( DsgSetRIAdr() != kFuncFail)
		{
			if( fDsgMode == kDsgDisabledMode )
			{
				justStarted = false;
				// Configure eCM to current mode
				Mode( fDsgMode );
				//DsgScan(true);    // do not start Docsis scan as this could be OOB mode.
			}
			else
			{
				// Reset tunnels regardless of the mode
				// because eCM has just reset.
				//if( DsgDocsisStatus() == kOther )
				//	ResetTunnels();

				if(!IsInitialState())
					HuntForDsgChannel();
			}

			CheckApiVersion();
		}
        else
            gAlwaysMsgNoFields(fMessageLogSettings) << "Cannot reconfig - eCM-Link still down!" << endl;
	}

    // update Count and check for roll over. Reserved 0 for detecting roll over reset
    LastHearBeatCount = EcmHearBeatCount;
    if( LastHearBeatCount == 0xffff )   
        LastHearBeatCount = 1;

	// Schedule next heartbeat check.
	pfInterfaceKeepAliveTimer->Start(kRemoteInterfaceKeepAlivePeriod);

#endif

	// Return true for the link is up
    return true;
}

// Install Handler to allow other entity to take over and process the DCD
// This function will only be called when an Advance mode DSG Cablecard is inserted
//
// Parameters:
//		PFN_DSG_DCD_BYPASS_HANDLER
//
// Returns:
//
void BcmDsgClientCtlThread::InstallDcdPassThroughHandler( PFN_DSG_DCD_BYPASS_HANDLER dcdbypasshandler )
{
	pfnDcdPassThroughHandler = dcdbypasshandler;
}

//
// Handler to allow other entity to consume the DCD
//
// Parameters:
//		BcmOctetBuffer &
//
// Returns:
//		false - nobody wants DCD, Process DCD internally (Default behavior)
//		true  - Other enitity is processing the DCD, do not process DCD internally
//
bool BcmDsgClientCtlThread::DcdPassThroughHandler( const BcmOctetBuffer &octetBuffer )
{
	unsigned int offset = 0;
    static BcmOctetBuffer *pfDcdBuffer = NULL;

	// if nobody interested in processing DCD, just return
	if( pfnDcdPassThroughHandler == NULL )
		return false;

    // print out DCD here. Cannot ParseTLVBuffer twice
    if( pfDcdMsg->ParseTLVBuffer((octetBuffer), offset, &fMessageLogSettings, true) != kConfOk )
        gAlwaysMsgNoFields (fMessageLogSettings) <<"Parse TLV error! Bad DCD!" << endl;

    // somebody wants to process DCD
    if( fNumberOfFragments == 1 )
    {
        // Normal case - non-fragment
        // OC-SP-CCIF2.0-I20-091211 : pass in complete DCD w/o the first 3 bytes
        pfnDcdPassThroughHandler( octetBuffer.AccessBuffer()+3, octetBuffer.BytesUsed()-3 );
    }
    else
    {
        // OC-SP-CCIF2.0-I20-091211 : REQ2385 If the Host receives the DCD message from the eCM in 2 or more DCD fragments, 
        // the Host SHALL combine all DCD fragments, remove the DOCSIS MAC management header and the three header bytes 
        // (Configuration Change Count, Number of Fragments and Fragment Sequence Number) from each of the fragments, 
        // and send just the TLVs to the Card in the send_DCD_info() APDU.
        if(pfDcdBuffer == NULL)
        {
            //create a separate buffer to hold the complete fragmented DCD
            pfDcdBuffer = new BcmOctetBuffer(octetBuffer.BytesUsed());
        }

        if(pfDcdBuffer)
        {
            // Copy/append the packet data, minus the first 3 bytes
            pfDcdBuffer->AddToEnd(octetBuffer.AccessBuffer()+3, octetBuffer.BytesUsed()-3, true) ;	
        }

        fParsedFragments++;
        if( fParsedFragments == fNumberOfFragments )
        {
            gAlwaysMsgNoFields (fMessageLogSettings) <<"!!!Got all " << fNumberOfFragments << " fragments DCD. Sending " << pfDcdBuffer->BytesUsed() << " bytes to cablecard." << endl;
            // OC-SP-CCIF2.0-I20-091211 : pass in complete DCD w/o the first 3 bytes
            pfnDcdPassThroughHandler( pfDcdBuffer->AccessBuffer(), pfDcdBuffer->BytesUsed() );
            delete pfDcdBuffer;     // it is safe to delete after above function call.
            pfDcdBuffer = NULL;
        }
    }
	return true;
}

// This function is used exclusively by CablecardIf
void BcmDsgClientCtlThread::SetDsgConfigSetting( BcmDsgConfigSettings &cfgset )
{
    // copy to local object
    fConfigSettings = cfgset;

    // serialize the function call by sending kStart to our msg q.
    //SerializePublicInterfaceCall( kSetDsgOtherConfig );
	gAlwaysMsgNoFields(fMessageLogSettings) << "==ApplyDcdConfigSettings" << endl;
    ApplyDcdConfigSettings();
}

uint8 BcmDsgClientCtlThread::Ucid( void )
{
	return fCurrentUcid;  
}



bool BcmDsgClientCtlThread::TestAndProcessApplicationNameTypeClient(BcmDsgClient *pNewClient)
{
#ifdef APPNAME_SUPPORT
	// If this client has only the Application Name, we need to check
	// and see if we can resolve the name in the App Name table
	// If not, just leave it and we will resolve it when the table
	// comes in.
	if (pNewClient->IdType() == kApplicationNameType)
	{
		if( pfAppNameTable != NULL )
		{
			BcmString appName = pNewClient->ClientName();

			// Use the SNS Table to map the name to an ID.
			pNewClient->Id( pfAppNameTable->AppId((char*)appName.c_str()) );
		}

		if( (pNewClient->Id() == 0) || (pfAppNameTable == NULL) )
		{
			// Name cannot be mapped to an id.
			// Insert it to the list now. It will be processed
			// when when SNS table is received.
			uint32 unsubscribed = 0;
			pfDsgClientManager->Subscribe( unsubscribed, pNewClient);

			if( OpenBroadcastChannel() == true )
			{
				// Indicate that we are waiting for SNS table.
				// Note that this state is a parallel state to
				// DSG-CC state. DSG-CC can be in operational state
				// and be waiting for SNS table at the same time.

				if( !fWaitingForSrcNameSubTable )
				{
					fWaitingForSrcNameSubTable = true;

					// Start SNS Table timer
					pfNoSnsTableTimer->Start(kNoGoodSnsTablePeriodMillisec);
				}

				gAlwaysMsgNoFields(fMessageLogSettings)
					<<"Waiting for SNS Table for new client."<< endl;
			}

			// If we could not open a broadcast channel then there
			// is nothing we can do for this client as long as we have
			// at least one other happy client.

			return false;
		}
		else
		{
			// Not sure whether we should do this, but the client has to have
			// a type that is defined in the spec.
			pNewClient->IdType( kApplicationType );
		}
	}
#endif
	return true;
}

#ifdef APPNAME_SUPPORT
// Public method to be called by Estb Helper Thread.
// Assumption: Helper thread is giving me a complete table.
void BcmDsgClientCtlThread::ProcessSourceSubTable( ETHERNET_PACKET *pkt )
{
	BcmEthernetPacket packet;
	packet.SetPacket(pkt);

	if( packet.ParseTcpUdp() == false )
	{
		return;
	}

	BcmDSGAppNameTable *pAppNameTable = new BcmDSGAppNameTable((unsigned char *) (packet.UdpPdu() + 4), (int) (packet.UdpLengthBytes() - 12));

    // serialize the function call by sending kSourceSubTable to our msg q.
	// Delete this memory when this DCD is processed.
    SerializePublicInterfaceCall( kSourceSubTable, (void*) pAppNameTable);
}

// Actions to be performed by DSG-CC thread.
void BcmDsgClientCtlThread::SyncProcessSourceSubTable(BcmDSGAppNameTable *pNewAppNameTable)
{
	// We MUST be in one of these states.
	//
	// 1) Operational State:
	//		At least one tunnel is open. At least one client provided its ID
	//		in text format.
	//
	//		Translate the application names to client IDs and
	//		try to open tunnels for those clients. If we cannot open any tunnel,
	//		we will ignore these clients. Other clients are happy.
	//		We will stay in the operational state.
	//
	// 2) Waiting for Source SubTable State.
	//		No tunnel is open. At least one client provided its ID
	//		in text format.

	//		Translate the application names to client IDs and
	//		try to open tunnels for those clients. If we cannot open any tunnel,
	//      we will move to hunting for ds channel state, else we will
	//		move to operational state.

	// Search the Client List for clients with IDs in text format.
	// Translate the application names to client IDs

	// Go through the

	if( fWaitingForSrcNameSubTable == false )
	{
		gAlwaysMsgNoFields(fMessageLogSettings)
				<< "Not waiting for SNS Table. Ignoring! " << endl;
		delete pNewAppNameTable;
		return;
	}

	if( ( fDsgCcState == kValidatingDcd ) || ( fDsgCcState == kOperational ) )
	{
		if( AssignClientIdsAndOpenTunnels( pNewAppNameTable ) == false )
		{
			gAlwaysMsgNoFields(fMessageLogSettings)
				<< "This SNS Table is no good!" << endl;

			delete pNewAppNameTable;

			// Continue to wait for a new table
			return;
		}

		gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Found Valid SNS Table." << endl;

		fWaitingForSrcNameSubTable = false;

		// Stop SNS Table timer
		pfNoSnsTableTimer->Stop();

		// Close broadcast tunnel if no client is subscribed to it.
		// Search client list by tunnel id. Warn if search finds
		// clients
		if( pfDsgClientManager->find(fBroadcastTunnelId) == pfDsgClientManager->end() )
		{
			gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Closing Broadcast Tunnel with MAC Address = "
				<< fBroadcastTunnelMac << endl;

			CloseTunnel(fBroadcastTunnelId);
			fBroadcastTunnelId = 0;
			fBroadcastTunnelMac = kAllZerosMac;
		}

		// Keep the table. Discard the old one.
		delete pfAppNameTable;
		pfAppNameTable = pNewAppNameTable;
	}
}

// Returns true if at least one tunnel can be open for
// a Client with ID = Application Name
const bool BcmDsgClientCtlThread::AssignClientIdsAndOpenTunnels(BcmDSGAppNameTable *pNewAppNameTable)
{
	if( pfDsgClientManager->AssignClientIds( pNewAppNameTable ) )
	{
		return ValidateDcdAndOpenTunnels();
	}

	return false;
}

void BcmDsgClientCtlThread::NoGoodSnsTableTimerEvent(void)
{
	gAlwaysMsgNoFields(fMessageLogSettings)
		<<" SnsTable Timer Expired \n "<< endl;

	if(!fWaitingForSrcNameSubTable)
	{
		gAlwaysMsgNoFields(fMessageLogSettings)
			<<"Ignoring NoSnsTableTimerEvent.. "<< endl;
		return;
	}

	if( ( fDsgCcState == kValidatingDcd ) || ( fDsgCcState == kOperational ) )
	{
		// If we are relying on the SNS Table to find a good
		// DSG downstream then we need to move on.
		// Obviuosly, we do not find any on this
		// downstream or the one we find does not map the
		// app name we are looking for..
		if( fDsgCcState == kValidatingDcd )
		{
			HuntForDsgChannel();
			return;
		}

		// Done with SNS Table
		fWaitingForSrcNameSubTable = false;

		// We are in Operational state. At least one client is happy.
		// At this point there is at least one AppName client
		// that we cannot assign a Client ID and therefore we
		// cannot open a tunnel for it.

		// Close broadcast tunnel if no client is subscribed to it.
		// Search client list by tunnel id. Warn if search finds
		// clients
		if( pfDsgClientManager->find(fBroadcastTunnelId) == pfDsgClientManager->end() )
		{
			gAlwaysMsgNoFields(fMessageLogSettings)
				<<"Closing Broadcast Tunnel with MAC Address = "
				<< fBroadcastTunnelMac << endl;

			CloseTunnel(fBroadcastTunnelId);
			fBroadcastTunnelId = 0;
			fBroadcastTunnelMac = kAllZerosMac;
		}
	}
}
#endif

// This callback function will notify eSTB of eCM's reset status.
//
// Parameters:
//      uint8 eCmResetType - 1: kEcmReset (eCM reset is detected via heartbeat )
//		                     2: kEcmDocsDevResetIssued   (eCM is about to reset due to docsDevReset)
//							 3: kEcmSwUpgradeResetIssued (eCM upgraded its sw and is about to reset)
// 
// Returns:  N/A
void BcmDsgClientCtlThread::DsgEcmResetIndication(uint8 eCmResetType)
{
	switch(eCmResetType)
	{
		case kEcmReset:
			{
				gAlwaysMsgNoFields(fMessageLogSettings) << "DSG-CC received eCM reset notification.\n";
			}
			break;

		case kEcmDocsDevResetIssued:
		{
			#if (CABLECARD_SUPPORT)
			// OC-ATP 14.1.1 step 65:
			BcmIndicateEcmReset();
			#endif

			gAlwaysMsgNoFields(fMessageLogSettings) << "eCM is resetting due to docsDevReset. Probe interface in 1 second...\n";

			// serialize the function call by sending kEventDsgScanSuccess to our msg q.
			SerializePublicInterfaceCall( kEventResetKeepAliveTimer, (void*)1000 );
		}
		break;

		case kEcmSwUpgradeResetIssued:
			{
				gAlwaysMsgNoFields(fMessageLogSettings) << "eCM is resetting due to successful software upgrade.\n";
			}
			break;

		default:
			gAlwaysMsgNoFields(fMessageLogSettings) << "eCM reset notification code not recognized.\n";
			break;
	}
}


void BcmDsgClientCtlThread::DsgEcmMacReinitEvent( void )
{
	// serialize the function call by sending kTestDsgTimerEvent to our msg q.
    SerializePublicInterfaceCall( kEventDsgEcmMacReinit );
}

void BcmDsgClientCtlThread::SyncDsgEcmMacReinitEvent( void )
{
	gAlwaysMsgNoFields(fMessageLogSettings) <<
"\n\n*********************************************" << endl << 
	" eSTB received notification: eCM MAC RE-INIT " << endl <<
	"*********************************************" << endl;

	fDsgCcState = kHuntingForDsgChannel;
	{
		if( fDsgMode == kDsgAdvanceMode )
		{
			ResetTunnels();
			ResetDcdSettings();
		}

		fUcidAvailable = false;
		fCurrentUcid = 0;
	}
}
