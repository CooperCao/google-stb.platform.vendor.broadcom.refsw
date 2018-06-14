/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
//****************************************************************************
//
//  Filename:       EstbIpThread.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  March 10, 2008
//
//****************************************************************************
//  Description:
//      This class is the helper thread for the main DOCSIS Control thread.  It
//      handles the operation of all of the IP state, running the DHCP helper,
//      Time Of Day, and TFTPing the config file.
//
//      Additionally, it has the helper routine to handle software upgrades
//      asynchronously (TFTPing the s/w image, processing it, storing it in
//      flash, etc.).
//
//****************************************************************************

//********************** Include Files ***************************************

#include "typedefs.h"

#include "MutexSemaphore.h"
#ifdef STUB
#include "EventLog.h"
#include "CmEvents.h"
#endif
// My api and definitions...
#include "EstbIpThread.h"   

// OS stuff.
#include "CountingSemaphore.h"
#include "Event.h"
#include "EventSet.h"
#include "MessageQueue.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"
#include "Timer.h"

#ifdef STUB
// IP Helper stuff.
#include "IpHelperFactory.h"
#include "Tftp.h"
#include "DocsisTimeOfDayThread.h"      // PR396
#endif

#include "IpStackManager.h"
#include "BaseIpHalIf.h"

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
#include "DhcpClientIf.h"
#include "DhcpClientId.h"
#include "IpHelperDhcpClientEventACT.h"
#endif
#include "IpHelperDhcpV6ClientEventACT.h"

#if (BFC_INCLUDE_DHCPV6_SUPPORT)
// DHCPv6 options
#include "DhcpV6OptionsList.h"
#include "DhcpV6RapidCommitOption.h"
#include "DhcpV6ReconfigureAcceptOption.h"
#include "DhcpV6CableLabsVendorSpecificInfoOption.h"
#include "DhcpV6DocsisOptionRequestOption.h"
#include "DhcpV6DocsisCableLabsClientConfigOption.h"
#include "DhcpV6DocsisTlv5EncodingOption.h"
#include "DhcpV6DocsisDeviceIdentifierOption.h"
#include "DhcpV6CableLabsVendorSpecificInfoOption.h"
#include "DhcpV6DocsisTimeProtocolServersOption.h"
#include "DhcpV6DocsisCmVendorClassOption.h"
#include "DhcpV6DocsisTimeOffsetOption.h"
#include "DhcpV6DocsisTftpServerAddressesOption.h"
#include "DhcpV6DocsisCfgFileNameOption.h"
#include "DhcpV6DocsisDeviceTypeOption.h"
#include "DhcpV6DocsisEmbeddedComponentsListOption.h"
#include "DhcpV6DocsisDeviceSerialNumberOption.h"
#include "DhcpV6DocsisHardwareVersionNumberOption.h"
#include "DhcpV6DocsisSoftwareVersionNumberOption.h"
#include "DhcpV6DocsisBootRomVersionNumberOption.h"
#include "DhcpV6DocsisOuiOption.h"
#include "DhcpV6DocsisModelNumberOption.h"
#include "DhcpV6DocsisVendorNameOption.h"
#include "DhcpV6DnsServersOption.h"
#include "DhcpV6DomainListOption.h"
#endif

// Other stuff.
#include "OctetBuffer.h"
#include "IpAddressList.h"

#ifdef STUB
#include "CmDocsisNonVolSettings.h"
#include "CmDocsis30NonVolSettings.h"

#include "NonVolDevice.h"
#include "ProgramStoreDevice.h"


#if (SNMP_SUPPORT)
// PR6147
#include "CmSnmpAgent.h"
#include "docsCableDeviceBridge.h"
// PTASKIRAN _ ESTB SNMP AGENT SUPPORT #include "CmSnmpNonVolSettings.h"
#endif

// Secure Download stuff.
#ifdef INCLUDE_SECURE_DOWNLOAD
    #include "CmSecureDownload.h"
#endif

// For s/w dload checking support.
#if (BFC_CM_VENDOR_SUPPORT)
    #include "VendorCmApplicationBase.h"
#endif

#if (BCM_IPC_INCLUDED)
#include "ProgramStoreDeviceDriver.h"
#include "InterProcessComm.h"
#endif
#endif //#ifdef STUB

//#include "CmDocsisStatusEventCodes.h"
#include "MergedNonVolDefaults.h"

#include <assert.h>
#include <string.h>

//#if (BCM_IPV6_SUPPORT)
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
#include "NeighborDiscoveryThread.h"
#include "DefaultRouterList.h"
#endif
#include "DhcpV6ClientNeighborDiscoveryACT.h"

#if (BFC_INCLUDE_DHCPV6_SUPPORT)
#include "EstbDhcpV6Defaults.h"
#include "Portable.h"
#include <net/if.h> // PTMOD REMOVE
#endif

#if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
#include "EstbEvents.h"
#endif

#if (BCM_EVD_INTERFACE_INCLUDED)
#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
#include "EcmEventDispatcherThread.h"

#include "DhcpV6ClientEventDispatcherACT.h"
#include "EstbClientDefaultEventList.h"
#endif 

#if (BCM_RPC_INTERFACE_INCLUDED)
// Client's RPC interface.
#include "EvDispatcherClientIf.h"
#endif

#endif

#include "EcmStatusEventCodes.h"
#include "DhcpV6ClientThread.h"
//********************** Local Types *****************************************
//********************** Local Constants *************************************

// The DOCSIS spec requires that we wait at least 10 minutes between attempts
// to TFTP.  The current consensus is that this is only for software upgrades,
// not for config file.
#define kTimeBetweenTftpTries (10 * kOneMinute)

// The spec is silent on how long we should wait between attempts to get the
// config file, but this seems consistent.
#define kTimeBetweenConfigTftpTries (10 * kOneMinute)

// Docsis3.0 defines for TFTP
#define kDocsis30_TftpBackoffStartMS (1*1000)
#define kDocsis30_TftpBackoffEndMS (16*1000)
#define kDocsis30_TftpBackoffExponent 2
#define kDocsis30_TftpRequestRetries 16

// Docsis2.0 defines for TFTP
#define kDocsis20_TftpBackoffStartMS (10*kOneMinute)
#define kDocsis20_TftpBackoffEndMS (10*kOneMinute)
#define kDocsis20_TftpBackoffExponent 1
#define kDocsis20_TftpRequestRetries 4

#if !BCM_REDUCED_IMAGE_SIZE
// Commonly used string for error messages.
//static const char *pCantLoadString = "  Can't load the software image!";
#endif

//********************** Local Variables *************************************
// The stack I'm running on.
// I need to declare this variable here and set it in the constructor so that
// I can access this value in static functions declared below.
static unsigned int ipStackNumber = 0;

static BcmEstbIpThread *pThisEstbIpThread = NULL;
// Keep track ether V4 or V6 dhcp client is running: 1 dhcpv4, 2 dhcpv6
unsigned int dhcpVersion = 0;
//********************** Local Functions *************************************

// Dummy/default implementation for the image validation callout function.
// This one will be used unless somebody overrides it by registering their
// own callout.
static unsigned int DefaultValidateImageCallout(const unsigned char *pBuffer,
                                                unsigned int numberOfBytes)
{
    #if (BFC_CM_VENDOR_SUPPORT)

    // If vendor support is included, call the vendor class to check the image.
    return BcmVendorCmApplicationBase::IsImageHeaderOk(pBuffer, numberOfBytes);

    #else

    // The default implementation doesn't do any additional validation, and
    // always returns true (1) so that the image will be accepted.
    return 1;

    #endif
}

//********************** Extern Functions *************************************
extern "C"
{
    extern uint32 SetInterfaceFlags(uint32 ipAddress, char *pInterfaceName, uint32 flags);
}

#if(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
extern void dhcpV6EvdClientLogEvent(unsigned int eventCode);
extern void initializeDhcpV6ClientRpcInterfaceAndSubscribeToEvents(void);
#endif
//********************** Class Method Implementations ************************


// Default Constructor.  Starts the thread, and creates all of the objects
// that it uses.
//
// Note that you will generally want to set the thread priority lower than
// the priority of the Docsis Control/Management thread so that the TFTP
// process doesn't block it from running.
//
// Parameters:
//      initialPriority - the priority at which you want this thread to run.
//
// Returns:  N/A
//
BcmEstbIpThread::BcmEstbIpThread(BcmOperatingSystem::ThreadPriority initialPriority, unsigned int stackNumber) :
    BcmThread("EstbIpThread", false, initialPriority),
        fIpState(kIpNotStarted),
        fIpV6State(kIpV6NotStarted),
        fSwUpdateState(kSwNotStarted),
        fThreadIsRunning(false),
        fAbortSwUpdate(false),
        fTimeOfDayCompleted(false),

        pfConfigFileBuffer(NULL),
        fTftpServerIpAddressBackoffPairList(),
        fTftpBlockSize(0),
        fCurrentConfigFileIp(),
        fConfigFileFilename(128),

        pfSwUpdateFilename(NULL),
        fSwUpdateStartedByManagement(false),
        fImageNumber(0),
        fSecureDownload(true),
//      pfSettingsToStore(NULL),

        // These will be created in the thread constructor.
        pfWaitForAbortSemaphore(NULL),
        pfEventSet(NULL),
        pfMessageQueue(NULL),
        pfTodHelperDoneEvent(NULL),     // PR2182
        pfTftpTimer(NULL),
        pfRouterSolicitationTimer(NULL),
        pfSWDnldResetMaxDelayTimer(NULL),
        pfPostCallDelayTimer(NULL),

        fModemCapabilities(255),
        fModemCapabilitiesBinary(0),

        // This is created when RegisterWithIpStack() is called.
        pfDhcpAct(NULL),
        pfDhcpV6Act(NULL),
        pfDhcpV6PacketAct(NULL),    // PR13544

        pfEventPublisherMutex(NULL),

        fOption43Callout(NULL),
//      fPacketCableDhcpOptionNumber( kDhcpPacketCableServers ),

        fSWDownloadOKPending(false),
        fMaxResetDelayTimerActive(false),
        fMaxDelayTimerFirstExpiry(false),
        fPostCallResetDelayTimerActive(false),
        fMaxResetDelay(0),
        fPostCallResetDelay(0),
        fPostCallDelayPendingTime(0),
        fESafeImpact(none),
        fValidateImageCallout(DefaultValidateImageCallout),
        fPreferredIpVersionIsIPv6(false),
        fDualStackOperationEnabled(false),
        fAlternameIpManagementModeEnabled(false),
        fRouterSolicitationCount(0),
        pfDomainList(NULL)

        #if (BCM_EVD_INTERFACE_INCLUDED)
        ,pfDhcpV6EventDispatcherACT( NULL )
        ,fLastEventIdLogged(0)
        #endif
{
    // Override the class name given by my parent.
    fMessageLogSettings.SetModuleName("BcmEstbIpThread");

    // Query nonvol settings to see which IP stack number we're supposed to use.
    // By default, use stack 1.
    fStackNumber = 1;
#ifdef STUB
    BcmCmDocsisNonVolSettings *pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();

    if (pSettings != NULL)
    {
        fStackNumber = pSettings->IpStackNumber();
    }
#else

    fStackNumber = stackNumber;
    ipStackNumber = fStackNumber;

    pThisEstbIpThread = this;

    pfDomainList = new BcmOctetBuffer(256);
#endif
#if (NOT_INCLUDED_IN_ESTB_DHCP) //(BFC_INCLUDE_DHCPV4_SUPPORT)  
    {
        // PR396 - create the DOCSIS-specific ToD thread.
        pfTodThread = new BcmDocsisTimeOfDayThread(initialPriority);

        if (pfTodThread == NULL)
        {
            gFatalErrorMsg(fMessageLogSettings, "BcmEstbIpThread") 
                << "Failed to create my ToD helper object!" << endl;

            assert(0);
        }

        // Set the IP stack number for the ToD thread.
        pfTodThread->SetIpStackNumber(fStackNumber);
    }

    {
        pfTftpHelper = BcmIpHelperFactory::NewTftp();

        if (pfTftpHelper == NULL)
        {
            gFatalErrorMsg(fMessageLogSettings, "BcmEstbIpThread") 
                << "Failed to create my TFTP helper object!" << endl;

            assert(0);
        }

        // Set the IP stack number for the TFTP helper.
        pfTftpHelper->SetIpStackNumber(fStackNumber);
    }
#endif // #if (BFC_INCLUDE_DHCPV4_SUPPORT)  
    // Spawn my thread.  PR1640 - bumped the stack size up to 6k
    if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
    {
       gFatalErrorMsg(fMessageLogSettings, "BcmEstbIpThread") << "Failed to spawn my thread!" << endl;

        assert(0);
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmEstbIpThread::~BcmEstbIpThread()
{
    CallTrace("BcmEstbIpThread", "~BcmEstbIpThread");

    // Tell the thread to stop processing (if it is doing so) and exit.
    if (pfMessageQueue != NULL)
    {
        gInfoMsg(fMessageLogSettings, "~BcmEstbIpThread") 
            << "Signalling the thread to exit..." << endl;

        pfMessageQueue->Send(kExitThread);
    }

    // Wait for the thread to exit.
    WaitForThread(); 

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // Delete all of the objects that I created (the OS objects are deleted by
    // the thread destructor).
    delete pfTodThread;
    pfTodThread = NULL;

    delete pfTftpHelper;
    pfTftpHelper = NULL;
#endif

#if (BFC_INCLUDE_DHCPV4_SUPPORT) 

    delete pfDhcpAct;
    pfDhcpAct = NULL;
#endif
    delete pfDhcpV6Act;
    pfDhcpV6Act = NULL;

    // PR13544
    delete pfDhcpV6PacketAct;
    pfDhcpV6PacketAct = NULL;

#if (BCM_EVD_INTERFACE_INCLUDED)
    delete pfDhcpV6EventDispatcherACT;
#endif
}


// This must be called after the IP stacks have been installed and
// initialized.  I need to register myself with the stack so I can do DHCP,
// etc.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::RegisterWithIpStack(void)
{
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
    BcmDhcpClientLease *pLease;

    if (pfDhcpAct != NULL)
    {
        gWarningMsg(fMessageLogSettings, "RegisterWithIpStack")
            << "Already registered with the IP stack; ignoring..." << endl;

        return;
    }

    pLease = GetLease();

    if (pLease != NULL)
    {
        // Create my ACT.
        pfDhcpAct = new BcmIpHelperDhcpClientEventACT(this);

        // Register my act with this lease so that I get called with
        // events.
        pLease->Subscribe(BcmDhcpClientLease::kEventLeaseBound, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventLeaseFailed, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventNoOffers, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventNoAck, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventAckMissingPreferredFields, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventBadAckMissingRequiredFields, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventBadOfferMissingRequiredFields, pfDhcpAct);
        pLease->Subscribe(BcmDhcpClientLease::kEventLeaseLost, pfDhcpAct);

        // PR5736 - we also need to subscribe to the "renewing" event so that
        // we can notify clients about when this happens.
        pLease->Subscribe(BcmDhcpClientLease::kEventRenewing, pfDhcpAct);
    }
    else
    {
        gErrorMsg(fMessageLogSettings, "RegisterWithIpStack")
            << "Can't find the lease associated with my IP stack!  Can't configure the DHCP client!" << endl;

        return;
    }
#endif
    // UNFINISHED - hack for DHCPv6; this will change.
    #if (BFC_INCLUDE_DHCPV6_SUPPORT)   //(BCM_IPV6_SUPPORT)
    {
        // Find my IP stack and then find the lease.
        BcmBaseIpHalIf *pIpHalIf;

        pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

        pfDhcpV6Act = new BcmIpHelperDhcpV6ClientEventACT(this);

        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressAcquiredTentative, pfDhcpV6Act);
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAcquisitionFailed, pfDhcpV6Act);
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressAcquiredSuccess, pfDhcpV6Act);
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressLost, pfDhcpV6Act);   // PR13041
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressRenewing, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressRenewed, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventNoDhcpReplyReceived, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventDuplicateAddressDetected, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventMissingRequiredOptions, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventNoResponseToRenew, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventReconfigureReceived, pfDhcpV6Act); 
        pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAddressReleased, pfDhcpV6Act); 

        // PR13544
        {
            pfDhcpV6PacketAct = new BcmCmDhcpV6MsgACT();
    
            pfDhcpV6PacketAct->DhcpV6ClientIf(pIpHalIf->DhcpV6ClientIf());
    
            pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventAdvertisePacketReceived, pfDhcpV6PacketAct);
            pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventReplyPacketReceived, pfDhcpV6PacketAct);
            pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventReplyPacketReceived_Renewing, pfDhcpV6PacketAct);
            pIpHalIf->DhcpV6ClientIf()->Subscribe(BcmDhcpV6ClientIf::kEventReplyPacketReceived_Rebinding, pfDhcpV6PacketAct);
        }

        // Register for DHCPv6 Advertise / Reply
        BcmDhcpV6ClientIf * pDhcpV6ClientIf = pIpHalIf->DhcpV6ClientIf();
        
        if(pDhcpV6ClientIf != NULL)
        {
            // register for notification of Advertise / Reply DhcpV6 messages
            pDhcpV6ClientIf->RegisterDhcpV6AdvertiseReplyCallout(CmDhcpV6AdvertiseReplyCallout);
        }
        else
        {
            gLogMessageRaw << "pDhcpV6ClientIf == NULL" << endl;
        }
    }
    #endif
}

/*
// Allows clients to subscribe for one or more of the events that I publish.
//
// Parameters:
//      eventCode - the event code for the status event of interest (see
//                  CmDocsisStatusEventCodes.h).
//      pAct - pointer to the ACT that is to be called when publishing
//             an event.  This is how the client is notified that the event
//             has occurred.
//
// Returns:
//      true if the client was successfully subscribed for the event.
//      false if the client could not be subscribed for some reason, or
//          if I don't publish the specified event.
//
bool BcmEstbIpThread::SubscribeEventNote(unsigned int eventCode,
                                             BcmCompletionHandlerACT *pAct)
{
    CallTrace("BcmEstbIpThread", "SubscribeEventNote");

    // Make sure I publish this event.
    switch (eventCode)
    {
        case BcmCmDocsisStatusEventCodes::kEstbDhcpInitStarted:
        case BcmCmDocsisStatusEventCodes::kEstbDhcpv6IpAddressAcquiredTentative:
        case BcmCmDocsisStatusEventCodes::kEstbDhcpInitOk:
        case BcmCmDocsisStatusEventCodes::kEstbDhcpInitFailed:
        case BcmCmDocsisStatusEventCodes::kEstbDhcpRenewStarting: // PR5736
        case BcmCmDocsisStatusEventCodes::kEstbDhcpRenewSuccess:  // PR5736
        case BcmCmDocsisStatusEventCodes::kEstbDhcpRenewFailed:   // PR2182 - new event
        case BcmCmDocsisStatusEventCodes::kCmTodInitStarted:
        case BcmCmDocsisStatusEventCodes::kCmTodInitOk:
        case BcmCmDocsisStatusEventCodes::kCmTodInitFailed:
        case BcmCmDocsisStatusEventCodes::kCmTftpInitStarted:
        case BcmCmDocsisStatusEventCodes::kCmTftpInitOk:
        case BcmCmDocsisStatusEventCodes::kCmTftpInitFailed:
        case BcmCmDocsisStatusEventCodes::kCmSwDownloadStarted:
        case BcmCmDocsisStatusEventCodes::kCmSwDownloadOk:
        case BcmCmDocsisStatusEventCodes::kCmSwDownloadFailed:
            break;

        default:
            gErrorMsg(fMessageLogSettings, "SubscribeEventNote")
                << "I don't publish event " << eventCode << "!  Client can't subscribe!" << endl;
            return false;
    }

    if (!pfEventPublisherMutex->Lock())
    {
        gErrorMsg(fMessageLogSettings, "SubscribeEventNote")
            << "Failed to lock my mutex!" << endl;

        return false;
    }

    // Should I check for duplicates?  For now, don't.
    fDocsisStatusEventPublisher.Subscribe(eventCode, pAct);

    if (!pfEventPublisherMutex->Unlock())
    {
        gWarningMsg(fMessageLogSettings, "SubscribeEventNote")
            << "Failed to unlock my mutex!" << endl;
    }

    return true;
}


// Allows clients to unsubscribe from being notified for the event
// specified.  The parameters MUST be the same as the ones specified in the
// call to SubscribeEventNote; this pair of parameters is necessary to
// uniquely identify a client.
//
// Parameters:
//      eventCode - the event code for the status event of interest (see
//                  CmDocsisStatusEventCodes.h).
//      pAct - pointer to the ACT that is to be called when publishing
//             an event.  This is how the client is notified that the event
//             has occurred.
//
// Returns:
//      true if the client was successfully unsubscribed for the event.
//      false if the client could not be unsubscribed for some reason.
//
bool BcmEstbIpThread::UnSubscribeEventNote(unsigned int eventCode,
                                               BcmCompletionHandlerACT *pAct)
{
    CallTrace("BcmEstbIpThread", "UnSubscribeEventNote");

    if (!pfEventPublisherMutex->Lock())
    {
        gErrorMsg(fMessageLogSettings, "UnSubscribeEventNote")
            << "Failed to lock my mutex!" << endl;

        return false;
    }

    // Should I check to see if the client had previously subscribed?  For now,
    // don't.
    fDocsisStatusEventPublisher.UnSubscribe(eventCode, pAct);

    if (!pfEventPublisherMutex->Unlock())
    {
        gWarningMsg(fMessageLogSettings, "UnSubscribeEventNote")
            << "Failed to unlock my mutex!" << endl;
    }

    return true;
}
*/

// Sets the Option43 callout function.
//
// Parameters:
//      callout - the address of the function to call.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::SetOption43Callout(Option43Callout callout)
{
    fOption43Callout = callout;
}


// Sets the image validation callout function.
//
// Parameters:
//      callout - the address of the function to call.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::SetValidateImageCallout(ValidateImageCallout callout)
{
    if (callout == NULL)
    {
        fValidateImageCallout = DefaultValidateImageCallout;
    }
    else
    {
        fValidateImageCallout = callout;
    }
}

#ifdef STUB
// Sets the modem capabilites string that I need to send in DHCP packets
// as part of the Vendor Class Id option.  The capabilities should be the
// full TLV, including the parent type (5) and length, plus all sub-TLVs,
// in binary format.  I will convert them as needed to ASCII-hex data.
//
// Parameters:
//      modemCapabilites - buffer containing the full modem capabilities.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::SetCapabilities(const BcmOctetBuffer &modemCapabilities)
{
    unsigned int numBytes;

    fModemCapabilities.Empty();

    // Convert from binary to ASCII-hex.
    numBytes = modemCapabilities.ConvertToAsciiHex((char *) fModemCapabilities.AccessBuffer(),
                                                   fModemCapabilities.BufferSize());

    fModemCapabilities.SetBytesUsed(numBytes);

    // Store this in binary format for DHCPv6.
    fModemCapabilitiesBinary = modemCapabilities;
}

// Sets the modem capabilites object instance that is used when encoding
// option 43 vendor specific sub-options.
//
// Parameters:
//      modemCapabilites - buffer containing the full modem capabilities.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::SetCapabilitiesObject(const BcmModemCapabilities &modemCapabilities)
{
    fModemCapabilitiesObject = modemCapabilities;
}
#endif
// This is what signals the thread to begin IP initialization.  It starts
// doing DHCP to get its IP settings, then kicks off Time Of Day for the
// ToD server specified in the DHCP Bind, then tries to TFTP the config
// file specified in the DHCP Bind into the buffer that is specified.
//
// Clients will be notified when DHCP/ToD/TFTP start and finish (or stop
// due to errors) via the Publish/Subscribe interface.
//
// Parameters:
//      pConfigFileBuffer - the buffer into which you want the config file
//                          to be stored.  It should be large enough to hold
//                          the entire config file; if it is too small, then
//                          this will cause a failure to be reported.  This
//                          can be NULL if you don't want the config file.
//      numberOfTftpTries - the number of times to try TFTP of the
//                          config file before declaring failure.  DOCSIS
//                          requires this to be 3, but I'll allow some
//                          flexibility.
//
// Returns:
//      true if the IP Initialization process was started.
//      false if there was a problem (thread not running, invalid state,
//          etc).
//
bool BcmEstbIpThread::StartIpInit(BcmOctetBuffer *pConfigFileBuffer,
                                      bool preferredIpVersionIsIPv6,
                                      bool dualStackOperationEnabled,
                                      bool alternameIpManagementModeEnabled,
                                      DocsisMode docsisMode )
{
    CallTrace("BcmEstbIpThread", "StartIpInit");

    printf("\n== Starting IP Initialization ==\n");

/*    #if (BCM_IPV6_SUPPORT)

    BcmCmDocsis30NonVolSettings * pDocsis30Settings = BcmCmDocsis30NonVolSettings::GetSingletonInstance();
    if( pDocsis30Settings->EnableIPv6Support() == false )
    {
        gLogMessageRaw << "IPv6 globally disabled in non-vol" << endl;
    }
    else 
    {
        if( pDocsis30Settings->OverrideIpInitializationParamatersTLV() == true )
        {
            gLogMessageRaw << "Overriding MDD message IP init parameters with non-vol settings" << endl;
            preferredIpVersionIsIPv6 = pDocsis30Settings->PreferredIpVersionIsIPv6();
            dualStackOperationEnabled = pDocsis30Settings->DualStackOperationEnabled();
            alternameIpManagementModeEnabled = pDocsis30Settings->AlternameIpManagementModeEnabled();
        }

        if( preferredIpVersionIsIPv6 )
        {
            printf("Preferred IP version is IPv6\n");
        }
        else
        {
            printf("Preferred IP version is IPv4\n");
        }

            printf("Dual-stack operation is %d\n", (int)dualStackOperationEnabled);
            printf("Alternate IP management mode is %d\n", (int)alternameIpManagementModeEnabled);
    }
    #endif
*/

    fPreferredIpVersionIsIPv6 = preferredIpVersionIsIPv6;
    fDualStackOperationEnabled = dualStackOperationEnabled;
    fAlternameIpManagementModeEnabled = alternameIpManagementModeEnabled;

    // Do some quick validation.
    if (fThreadIsRunning == false)
    {
        gErrorMsg(fMessageLogSettings, "StartIpInit") 
            << "Thread is not running!" << endl;

        return false;
    }
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
    if (pfDhcpAct == NULL)
    {
        gErrorMsg(fMessageLogSettings, "StartIpInit")
            << "We have not registered with the IP stack; somebody didn't initialize us correctly!" << endl;
    }
#endif
    if (fIpState != kIpNotStarted)
    {
        gErrorMsg(fMessageLogSettings, "StartIpInit") 
            << "Can't start (incorrect state)!  You must call StopAndReset() first to reset my state!" << endl;

        return false;
    }

    // Store the client's parameters.
    pfConfigFileBuffer = pConfigFileBuffer;

    // Update my state.
    fTimeOfDayCompleted = false;

    unsigned int event=0;

    // kick off either IPv4 or IPv6 IP address acquisition
    if( preferredIpVersionIsIPv6 )
    {
        gLogMessageRaw <<"Starting IPv6 DHCP client! "<< endl;
        dhcpVersion = 2;
        event = kStartIpV6Init;
    }
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
    else
    {
        gLogMessageRaw <<"Starting IPv4 DHCP client! "<< endl;
        fIpState = kIpDhcpInProgress;
        dhcpVersion = 1;
        event = kStartIpV4Init;
    }
#endif
    // Tell my thread to start.
    if (pfMessageQueue->Send(event) == false)
    {
        // Reset my state.
        fIpState = kIpNotStarted;

        pfConfigFileBuffer = NULL;

        gErrorMsg(fMessageLogSettings, "StartIpInit") 
            << "Failed to send message to my thread!" << endl;

        return false;
    }

    return true;
}


// This allows the client to restart TFTP of the config file.  It is
// possible for the config file to be loaded successfully, but for the
// client not to like what's in the file, in which case we need to go
// through the whole "wait a while and retry" process, i.e. treat it the
// same as if the file doesn't exist.
//
// Parameters:
//      pConfigFileBuffer - the buffer into which you want the config file
//                          to be stored.  It should be large enough to hold
//                          the entire config file; if it is too small, then
//                          this will cause a failure to be reported.  This
//                          can be NULL if you don't want the config file.
//                          Of course, if you don't want the config file,
//                          then why are you calling this method?
//
// Returns:
//      true if the TFTP read was restarted again.
//      false if there was a problem (thread not running, invalid state,
//          etc).
//
bool BcmEstbIpThread::RestartTftpConfig(BcmOctetBuffer *pConfigFileBuffer)
{
    return true;
}


// This allows the client to abort any operation that is currently running
// (DHCP, ToD, TFTP config file, s/w dload) before it has completed, and
// will reset the state to an initial state (clearing the IP address, etc).
// This is often used when a T4 timeout is detected or when we deregister
// from the CMTS.
//
// This operates synchronously; i.e. the thread will stop operation and
// reset state before this method returns.
//
// Parameters:  None.
//
// Returns:
//      true if the thread was stopped and reset successfully.
//      false if it wasn't running, or if it couldn't be stopped for some
//          reason.
//
bool BcmEstbIpThread::StopAndReset(void)
{
    CallTrace("BcmEstbIpThread", "StopAndReset");

    // Do some quick validation.
    if (fThreadIsRunning == false)
    {
        gErrorMsg(fMessageLogSettings, "StopAndReset")
            << "Thread is not running!" << endl;

        return false;
    }

    // PR6491 - there is a race condition where this can be called just after
    // DHCP completes but just before ToD/TFTP starts, in which case the call
    // to pfTftpHelper->Stop() is too early and won't make us stop properly.
    // If we're in the state where DHCP is in progress, let's piss away a little
    // time and see if it moves on to starting TFTP.  If not, then DHCP probably
    // won't complete and we'll stop it in its tracks.  If it does, then we
    // will stop it after TFTP starts, which gets us to bail out more quickly.
    unsigned int waitCount;

    waitCount = 0;
    while ((fIpState == kIpDhcpInProgress) && (waitCount < 50))
    {
        BcmOperatingSystemFactory::ThreadSleep(10);

        waitCount++;
    }
#if (BFC_INCLUDE_DHCPV4_SUPPORT)

    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    // It is possible that we are "stuck" doing a TFTP transfer; in other words,
    // the physical link might be down, and the TFTP client might be retrying.
    // If so, it will continue to retry until it times out, which can take more
    // than a minute.  Since we know it is supposed to fail/abort, lets tell
    // the TFTP client to give it up and stop.  We have to do this in the
    // context of the calling thread, since my thread is the one that's blocked
    // with the TFTP retries.
    pfTftpHelper->Stop();
    #endif

    // If we are doing a s/w update, then make it stop.  I have to do this
    // before I try to send a message to myself, because the thread might be
    // busy loading the image and won't service the message queue until it
    // gives up.
    if (fIpState == kIpDoingSwUpdate)
    {
        StopSoftwareUpdate();
    }
#endif
    // Tell my thread to shut down all other processes and reset state.
    if (pfMessageQueue->Send(kStopAndReset) == false)
    {
        gErrorMsg(fMessageLogSettings, "StopAndReset")
            << "Failed to send message to my thread!" << endl;

        return false;
    }

    // Wait for the thread to signal that it has done this; this makes the
    // method synchronous.
    pfWaitForAbortSemaphore->Get();

    return true;
}


// IsDhcpBound() - Returns true if DHCP initialization has reached "bound"
// state.
//
// Parameters:  None.
//
// Returns:
//      true if the DHCP client has acquired an IP address.
//      false if the DHCP client has not acquired an IP address.
//
bool BcmEstbIpThread::IsDhcpBound(void) const
{
    bool ret_val = false;
    BcmBaseIpHalIf *pIpHalIf;

    CallTrace("BcmEstbIpThread", "IsDhcpBound");

    pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    if (pIpHalIf != NULL)
    {
        // UNFINISHED - is this accurate?  Maybe not...
        ret_val = pIpHalIf->IpAddressConfigured();
    }
    
    return ret_val;
}

#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
// Helper method to get the DHCP lease object for the stack that we are
// controlling.
//
// Parameters:  None.
//
// Returns:
//      A pointer to the lease object (or NULL if not found).
//
BcmDhcpClientLease *BcmEstbIpThread::GetLease(void) const
{
    CallTrace("BcmEstbIpThread", "GetLease");

    // Find my IP stack and then find the lease.
    BcmBaseIpHalIf *pIpHalIf;

    pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    if (pIpHalIf == NULL)
    {
        gErrorMsg(fMessageLogSettings, "GetLease")
            << "Can't find IP stack " << fStackNumber 
            << "!  Can't find lease!" << endl;

        return NULL;
    }

    if (pIpHalIf->DhcpClientIf() == NULL)
    {
        gErrorMsg(fMessageLogSettings, "GetLease")
            << "Can't find DhcpClientIf for IP stack " << fStackNumber 
            << "!  Can't find lease!" << endl;

        return NULL;
    }

    BcmDhcpClientId clientId(pIpHalIf->MacAddress());

    return pIpHalIf->DhcpClientIf()->FindLease(clientId);
}
#endif

#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
// Helper to return the current IP address of our stack.
//
// Parameters:  None.
//
// Returns:
//      The current IP address.
//
const BcmIpAddress &BcmEstbIpThread::CurrentIpAddress(void) const
{
    BcmDhcpClientLease *pLease = GetLease();

    if (pLease == NULL)
    {
        return kAllZerosIp;
    }

    return pLease->ServerLeaseSettings().YourIpAddress();
}
#endif

// Helper to return the TFTP server associated with our DHCP lease.
//
// Parameters:  None.
//
// Returns:
//      The IP address of our TFTP server.
//
const BcmIpAddress &BcmEstbIpThread::CurrentTftpServerAddress(void) const
{
    return fCurrentConfigFileIp;
}


// Helper to return the (config file, not dload) TFTP filename.
//
// Parameters:  None.
//
// Returns:
//      The (config file, not dload) TFTP filename.
//
const BcmString &BcmEstbIpThread::CurrentTftpServerFilename(void) const
{
    static BcmString filename(fConfigFileFilename);
    return filename;
}


// Allows the client to query the time offset from the TOD exchange.
//
// Parameters:  None.
//
// Returns:
//      The system clock offset induced by the last ToD exchange.
//      Could be positive, negative, or zero.
//
int BcmEstbIpThread::GetTodLastDelta(void)
{
    return SystemTimeGetLastDelta();
}


// This method checks the name of the existing software image and compares
// it to the one specified, returning false if the names are the different.
// In some cases we don't want to begin a software update if the existing
// image is the same as what is being requested.
//
// Parameters:
//      pFilename - the name of the software image that would be updated.
//
// Returns:
//      true if the filenames are the same.
//      false if the filenames are different.
//
bool BcmEstbIpThread::SoftwareImagesAreTheSame(const char *pFilename)
{
    return false;
}


// Begins the process of downloading a new software image for the CM.  This
// is done using TFTP from the specified server, and is performed
// asynchronously.
//
// This can only be called in the IpState is kIpSuccess, since we must have
// our IP parameters in order to run.
//
// Parameters:
//      pFilename - a string containing the name of the file to TFTP.  This
//                  will usually come from the config file, but it can also
//                  come from other sources (user input, etc.).
//      ipAddress - the IP address of the TFTP server.  This will also
//                  typically come from DHCP or the config file, but can
//                  come from other sources.
//      startedByManagement - tells me who started the update, management
//                            (SNMP/console) or provisioning (config file).
//      imageNumber - the image number to be updated.  0 = default.
//      secureDownload - whether we are downloading a secure image or not
//
// Returns:
//      true if the software update was started successfully.
//      false if there was a problem (thread not running, invalid state,
//          invalid parameters, etc.).
//
bool BcmEstbIpThread::StartSoftwareUpdate(const char *pFilename,
                                              const BcmIpAddress &ipAddress,
                                              bool startedByManagement,
                                              unsigned int imageNumber,
                                              bool secureDownload)
{

    return true;
}


// This allows the client to abort the software update process.
//
// Parameters:  None.
//
// Returns:
//      true if the software update was stopped.
//      false if it couldn't be stopped for some reason.
//
bool BcmEstbIpThread::StopSoftwareUpdate(void)
{

    return true;
}


// Called by the DHCP Client Event ACT to let me process events.
//
// Parameters:
//      eventCode - the event that occurred.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::ProcessDhcpEvent(const BcmCompletionEvent &eventCode)
{
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
    CallTrace("BcmEstbIpThread", "ProcessDhcpEvent");

    if (fThreadIsRunning == false)
    {
        gErrorMsg(fMessageLogSettings, "ProcessDhcpEvent")
            << "Thread is no longer running!" << endl;

        return;
    }

    BcmDhcpClientLease *pLease = GetLease();

    if (pLease == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ProcessDhcpEvent")
            << "Can't find lease object associated with my IP stack!  Can't handle event!" << endl;

        return;
    }

    // I need to handle some events synchronously so that I can query the
    // state of the lease before it transitions.
    switch (eventCode)
    {
        case BcmDhcpClientLease::kEventNoOffers:
            // Techincally speaking, this isn't quite right, but it's close
            // enough.  This gets logged every time we time out waiting for
            // offers.  It may take us a while before we stop retrying, though,
            // so you will get multiple of these before DHCP totally fails.

            // Log event "Discover sent, No offer received"
            // PTMOD - LOG EVENT CmLogEvent(kD01_0);
            #if (BCM_STANDALONE_DHCP_CLIENT)
                #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
                DhcpV6LogEvent(kEE_D00100);
                #endif
            #else
            CmLogEvent(kD01_0);
            #endif
            break;

        case BcmDhcpClientLease::kEventNoAck:
            // Techincally speaking, this isn't quite right, but it's close
            // enough.  This gets logged every time we time out waiting for
            // acks.  It may take us a while before we stop retrying, though,
            // so you will get multiple of these before DHCP totally fails.

            // Log event "Request sent, No response"
            // PTMOD - LOG EVENT CmLogEvent(kD02_0);
            #if (BCM_STANDALONE_DHCP_CLIENT)
                #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
                DhcpV6LogEvent(kEE_D00200);
                #endif
            #else
            CmLogEvent(kEE_D00200);
            #endif
            break;

        case BcmDhcpClientLease::kEventAckMissingPreferredFields:
            // Log a different event depending on the state of things.
            if (fIpState == kIpDhcpInProgress)
            {
                // Log event "Non-critical field invalid in response"
                // PTMOD - LOG EVENT CmLogEvent(kD03_0);
            }
            else
            {
                if (pLease->State() == BcmDhcpClientLease::kStateRenewing)
                {
                    // Same event, but in renewing state.
                    // PTMOD - LOG EVENT CmLogEvent(kD103_0);
                }
                else
                {
                    // Same event, but in rebinding state.
                    // PTMOD - LOG EVENT CmLogEvent(kD104_0);
                }
            }
            break;

        case BcmDhcpClientLease::kEventBadOfferMissingRequiredFields:
        case BcmDhcpClientLease::kEventBadAckMissingRequiredFields:
            // Log a different event depending on the state of things.
            if (fIpState == kIpDhcpInProgress)
            {
                // Log event "Critical field invalid in response"
                // PTMOD - LOG EVENT CmLogEvent(kD03_1);
            #if (BCM_STANDALONE_DHCP_CLIENT)
                #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
                DhcpV6LogEvent(kEE_D00301);
                #endif
            #else
            CmLogEvent(kD03_1);
            #endif
            }
            else
            {
                if (pLease->State() == BcmDhcpClientLease::kStateRenewing)
                {
                    // Same event, but in renewing state.
                    // PTMOD - LOG EVENT CmLogEvent(kD103_1);
                #if (BCM_STANDALONE_DHCP_CLIENT)
                    #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
                    DhcpV6LogEvent(kEE_D00301);
                    #endif
                #else
                CmLogEvent(kD03_1);
                #endif
                }
                else
                {
                    // Same event, but in rebinding state.
                    // PTMOD - LOG EVENT CmLogEvent(kD104_1);
                #if (BCM_STANDALONE_DHCP_CLIENT)
                    #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
                    DhcpV6LogEvent(kEE_D00301);
                    #endif
                #else
                CmLogEvent(kD03_1);
                #endif
                }
            }
            break;

        case BcmDhcpClientLease::kEventRenewing:
            // PR5736 - Notify clients that the lease is starting to renew.
            //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewStarting);
            PublishEvent(kEstbDhcpRenewStarting);
            break;

        default:
            // All other events are handled async.
            pfMessageQueue->Send(kProcessDhcpEvent, (void *) eventCode);
            break;
    }
#endif
}


void BcmEstbIpThread::ProcessDhcpV6Event(const BcmCompletionEvent &eventCode)
{
    CallTrace("BcmEstbIpThread", "ProcessDhcpV6Event");

    if (fThreadIsRunning == false)
    {
        gErrorMsg(fMessageLogSettings, "ProcessDhcpV6Event")
            << "Thread is no longer running!" << endl;

        return;
    }

    // I need to handle some events synchronously 
    switch (eventCode)
    {

        case BcmDhcpV6ClientIf::kEventAddressRenewing:
            // Notify clients that the lease is starting to renew.   This must be done
            // synchronously - the DocsisCtlThread uses this event to modify the forwarding
            // rules so that the multicast Rebind packet is not visible on the LAN. 
            //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewStarting);
            PublishEvent(kEstbDhcpRenewStarting);
            break;

        default:
            // All other events are handled async.
            pfMessageQueue->Send(kProcessDhcpV6Event, (void *) eventCode);
            break;
    }
}


// Thread constructor - this is the first method called after the thread has
// been spawned, and is where the thread should create all OS objects.  This
// has to be done here, rather than in the object's constructor, because
// some OS objects must be created in the context of the thread that will
// use them.  The object's constructor is still running in the context of
// the thread that created this object.
//
// Parameters:  None.
//
// Returns:
//      true if successful and ThreadMain() should be called.
//      false if there was a problem (couldn't create an OS object, etc.)
//          and ThreadMain() should not be called.
//
bool BcmEstbIpThread::Initialize(void)
{
    CallTrace("BcmEstbIpThread", "Initialize");

    // Create all of the OS objects that I use.
    pfWaitForAbortSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 0, "EstbIpThread abort-wait semaphore");

    if (pfWaitForAbortSemaphore == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") 
            << "Failed to create semaphore!" << endl;

        return false;
    }

    pfEventSet = pfOperatingSystem->NewEventSet("EstbIpThread event set");

    if (pfEventSet == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") 
            << "Failed to create event set!" << endl;

        return false;
    }

    pfMessageQueue = pfOperatingSystem->NewMessageQueue("EstbIpThread message queue");

    if (pfMessageQueue == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create message queue!" << endl;

        return false;
    }

    // PR2182 - split these out to 2 events.
    pfTodHelperDoneEvent = pfOperatingSystem->NewEvent("EstbIpThread ToD Helper Done event");

    if (pfTodHelperDoneEvent == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") 
            << "Failed to create ToD Helper Done event!" << endl;

        return false;
    }

    pfTftpTimer = pfOperatingSystem->NewTimer("EstbIpThread TFTP retry timer");

    if (pfTftpTimer == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create TFTP retry timer!" << endl;

        return false;
    }

    pfRouterSolicitationTimer = pfOperatingSystem->NewTimer("Router solicitation timer");

    if (pfRouterSolicitationTimer == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create router solicitation timer!" << endl;

        return false;
    }

    pfEventPublisherMutex = pfOperatingSystem->NewMutexSemaphore("EstbIpThread EventPublisher mutex");

    if (pfEventPublisherMutex == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create EventPublisher mutex!" << endl;

        return false;
    }

    pfSWDnldResetMaxDelayTimer = pfOperatingSystem->NewTimer("EstbIpThread SW Download reset delay timer");

    if (pfSWDnldResetMaxDelayTimer == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create SW Download reset delay timer!" << endl;

        return false;
    }
    
    pfPostCallDelayTimer = pfOperatingSystem->NewTimer("EstbIpThread SW Download reset delay timer");

    if (pfPostCallDelayTimer == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize")
            << "Failed to create Post Call reset delay timer!" << endl;

        return false;
    }

    // Add all of my objects to the event set.
    pfEventSet->Add(*pfMessageQueue);
    pfEventSet->Add(pfTodHelperDoneEvent);      // PR2182
    pfEventSet->Add(*pfTftpTimer);
    pfEventSet->Add(*pfRouterSolicitationTimer);
    pfEventSet->Add(*pfSWDnldResetMaxDelayTimer);
    pfEventSet->Add(*pfPostCallDelayTimer);

    // Signal that the thread has officially started and is available to take
    // events.
    fThreadIsRunning = true;

    return true;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::ThreadMain(void)
{
    bool result;
    bool timeToExit = false;

    #if(BCM_EVD_INTERFACE_INCLUDED)
        // Initialize RPC interface
        #if(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
        initializeDhcpV6ClientRpcInterfaceAndSubscribeToEvents();
        #else
        SubscribeToEvents();
        #endif
    #endif

    // Keep going until told to exit.  This is handled by servicing the message
    // queue; when we receive a command to exit, we will break out of the loop.
    while (1)
    {
        // Wait for an event to occur.
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

        // Service the message queue as the first priority.
        if (pfEventSet->Occurred(*pfMessageQueue))
        {
            unsigned int command = 0;
            void *pDummy;

            // It's possible that multiple commands were sent, so I'll try to
            // process them all.
            while (pfMessageQueue->Receive(command, pDummy, BcmMessageQueue::kTimeout) &&
                   !timeToExit)
            {
                switch (command)
                {
                    case kExitThread:
                        timeToExit = true;

                        // Make sure everything is shut down before exiting.
                        StopAndResetSync(false);
                        break;

                    case kStartIpV4Init:
                        StartDhcp();
                        break;

#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT) //(BCM_IPV6_SUPPORT)
                    case kStartIpV6Init:
                        InitiateDuplicateAddressDetectionOnLinkLocalAddress();
                        break;
#endif
                    case kObtainRouterAdvertisement:
                        {
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT) //(BCM_IPV6_SUPPORT)
                            if( fIpV6State == kDuplicateAddressDetectionOfLinkLocalAddressInProgress )
                            {

                                // sleep 1 second to give the IP stack a chance to do it's DAD and configure the address
                                // then clear out the default router list in the ND thread so we will solicit. 
                                // If the stack receives a RA before link-local is configured, it rejects it. 

                                fIpV6State = kRouterAdvertisementCollectionInProgress;
#if !(BCM_STANDALONE_DHCP_CLIENT)
                                BcmOperatingSystemFactory::ThreadSleep(1000);
                                BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
                                BcmNeighborDiscoveryThread * pNeighborDiscoveryThread = pIpHalIf->NeighborDiscoveryThread();
                                pNeighborDiscoveryThread->Reset();
#endif
                            }

                            ObtainRouterAdvertisement();
#endif
                        }
                        break;

#if (BFC_INCLUDE_DHCPV6_SUPPORT)
                    case kPerformDhcpV6:
                        StartDhcpV6();
                        break;

                    case kProcessDhcpV6Event:
                        {
                            unsigned int eventValue = (unsigned int) pDummy;
                            BcmDhcpV6ClientIf::DhcpV6ClientLeaseEvent event = (BcmDhcpV6ClientIf::DhcpV6ClientLeaseEvent) eventValue;

                            HandleDhcpV6Event(event);
                        }
                        break;
#endif

                    case kRestartConfigFileTftp:
                        TftpConfigFile();
                        break;

                    case kStopAndReset:
                        StopAndResetSync(true);
                        break;

                    case kStartSoftwareDload:
                        //TftpSoftwareImage();
                        break;

                    case kStopSoftwareDload:
                        //StopTftpSoftwareImage();
                        break;

                    case kProcessDhcpEvent:
                        {
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
                            unsigned int eventValue = (unsigned int) pDummy;
                            BcmDhcpClientLease::DhcpClientLeaseEvent event = (BcmDhcpClientLease::DhcpClientLeaseEvent) eventValue;

                            HandleDhcpEvent(event);
#endif
                        }
                        break;

                    case kStartPostCallTimer:   // PR11436
                        {
                            // have to do this in within our task tread
                            pfPostCallDelayTimer->Start( fPostCallResetDelay * 1000 );
                            fPostCallResetDelayTimerActive = true;
                        }
                        break;
                    
                    case kStopPostCallTimer:   // PR11436
                        {
                           // have to do this in within our task tread
                           pfPostCallDelayTimer->Stop();
                           fPostCallResetDelayTimerActive = false;
                        }
                        break;
                    
                    default:
                        gWarningMsg(fMessageLogSettings, "ThreadMain")
                            << "Received unrecognized command (" << command
                            << ") in the message queue!  Ignoring..." << endl;
                        break;
                }
            }

            // If we received a command to exit, then break out of the main
            // loop.
            if (timeToExit)
            {
                break;
            }
        }

        // See if the ToD helper has told me that it has finished whatever it
        // was doing.
        if (pfEventSet->Occurred(pfTodHelperDoneEvent))
        {
            // This is the ToD helper telling me that ToD is done, and I need
            // to check its state for success/fail.
            HandleTodComplete();
        }

        // See if the TFTP timer expired.
        if (pfEventSet->Occurred(*pfTftpTimer))
        {
            // Restart the TFTP, depending on our current state.
            if (fIpState == kIpTftpInProgress)
            {
                TftpConfigFile();
            }
            else
            {
                gWarningMsg(fMessageLogSettings, "ThreadMain")
                    << "TFTP Timer expired, but I wasn't expecting it!  Ignoring..." << endl;
            }
        }

        // See if the TFTP timer expired.
        if (pfEventSet->Occurred(*pfRouterSolicitationTimer))
        {
            gAlwaysMsgNoFields(fMessageLogSettings)
                << "Router Solicitation Timer expired!" << endl;
            // Restart the TFTP, depending on our current state.
            if (fIpV6State == kRouterAdvertisementCollectionInProgress)
            {
                gAlwaysMsgNoFields(fMessageLogSettings)
                    << "Current state is kRouterAdvertisementCollectionInProgress" << endl;
                pfMessageQueue->Send( kObtainRouterAdvertisement );
            }
            else
            {
                gWarningMsg(fMessageLogSettings, "ThreadMain")
                    << "RS Timer expired, but I wasn't expecting it!  Ignoring..." << endl;
            }
        }

        
        // See if the SW Download reset delay timer expired.
        if (pfEventSet->Occurred(*pfSWDnldResetMaxDelayTimer))
        {
            // Restart the TFTP, depending on our current state.
            if (fMaxResetDelayTimerActive)
            {
                HandleMaxDelayTimer();
            }
            else
            {
                gWarningMsg(fMessageLogSettings, "ThreadMain")
                    << "SW Download reset max delay timer expired, but I wasn't expecting it!  Ignoring..." << endl;
            }
        }
        
        // See if the SW Download reset delay timer expired.
        if (pfEventSet->Occurred(*pfPostCallDelayTimer))
        {
            // Use this timer to process soft timers of DHCPv6 thread.
            // Grab my IP stack.
            BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
            pIpHalIf->DhcpV6ClientIf()->DhcpV6ClientThread()->ResyncTimers();
            pfPostCallDelayTimer->Restart();
#if 0
            // Restart the TFTP, depending on our current state.
            if (fPostCallResetDelayTimerActive)
            {
                HandlePostCallDelayTimer();
            }
            else
            {
                gWarningMsg(fMessageLogSettings, "ThreadMain")
                    << "Post Call delay timer expired, but I wasn't expecting it!  Ignoring..." << endl;
            }
#endif
        }
    }

    // Signal that the thread has stopped and will no longer take events.
    fThreadIsRunning = false;
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::Deinitialize(void)
{
    CallTrace("BcmEstbIpThread", "Deinitialize");

    // Delete all of the OS objects that I created.
    delete pfWaitForAbortSemaphore;
    pfWaitForAbortSemaphore = NULL;

    if (pfEventSet != NULL)
    {
        pfEventSet->RemoveAll();
    }

    delete pfEventSet;
    pfEventSet = NULL;

    delete pfMessageQueue;
    pfMessageQueue = NULL;

    // PR2182 - split this out to 2 events.
    delete pfTodHelperDoneEvent;
    pfTodHelperDoneEvent = NULL;

    delete pfTftpTimer;
    pfTftpTimer = NULL;
    
    delete pfRouterSolicitationTimer;
    pfRouterSolicitationTimer = NULL;

    delete pfSWDnldResetMaxDelayTimer;
    pfSWDnldResetMaxDelayTimer = NULL;
    
    delete pfPostCallDelayTimer;
    pfPostCallDelayTimer = NULL;

    delete pfEventPublisherMutex;
    pfEventPublisherMutex = NULL;
}


/*
// Causes the specified event to be published (within the context of the
// mutex semaphore).
//
// Parameters:
//      event - the event to be published.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::PublishEvent(unsigned int event)
{
    CallTrace("BcmEstbIpThread", "PublishEvent");

    gInfoMsg(fMessageLogSettings, "PublishEvent")
        << "Publishing event id " << event << endl;

    if (!pfEventPublisherMutex->Lock())
    {
        gErrorMsg(fMessageLogSettings, "PublishEvent")
            << "Failed to lock my mutex!" << endl;

        return;
    }

    fDocsisStatusEventPublisher.PublishEvent(event);

    if (!pfEventPublisherMutex->Unlock())
    {
        gWarningMsg(fMessageLogSettings, "PublishEvent")
            << "Failed to unlock my mutex!" << endl;
    }
}
*/

// Append eSAFE-specific sub options to option 43 buffer.
//
// Parameters:  Option43 octet buffer.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::AppendApplicationSpecificOption43SubOptions(BcmOctetBuffer &opt43)
{
}

// Append ESAFE name to option 43 suboption 3 if DSG is enabled.
//
// Parameters:  Sub-option octet buffer.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::AppendApplicationSpecificEsafe(BcmOctetBuffer &subOpt3)
{
}

// Initiate DAD on the link-local address before starting DHCPv6
//
// Parameters:
//      none
//
// Returns:  Nothing.
//
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
void BcmEstbIpThread::InitiateDuplicateAddressDetectionOnLinkLocalAddress()
{
    // Lightweight ACT class used for duplicate address detection on our link-local
    // address
    class BcmDupDetectNeighborDiscoveryACT : public BcmNeighborDiscoveryACT
    {
    public:
        BcmEstbIpThread * pfDocsisIpThread;
        BcmDupDetectNeighborDiscoveryACT( BcmEstbIpThread * pDocsisIpThread ) : pfDocsisIpThread(pDocsisIpThread){}
        virtual ~BcmDupDetectNeighborDiscoveryACT(){}

        // NOTE:  Keep in mind, we are in the context of the NeighborDiscoverThread 
        // in HandleEvent()
        virtual void HandleEvent( const BcmCompletionEvent &event_code )
        {   
            bool result = false;

            if( event_code == BcmNeighborDiscoveryACT::kNeighborDiscoverySucceded )
                result = true;

            pfDocsisIpThread->DuplicateAddressDetectionOnLinkLocalAddressComplete( result );

            // self-destruct upon completion of request
            delete this;
        }
    };

    fIpV6State = kDuplicateAddressDetectionOfLinkLocalAddressInProgress;

    BcmDupDetectNeighborDiscoveryACT * pACT = new BcmDupDetectNeighborDiscoveryACT(this);

    fIpV6State = kDuplicateAddressDetectionOfLinkLocalAddressInProgress;

    BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
    BcmIpV6Address linkLocalIpAddress; 
    pIpHalIf->GetIpv6LinkLocalAddress( linkLocalIpAddress );  
    pACT->DestinationIpAddress( linkLocalIpAddress );

    gLogMessageRaw << "Initiating DAD on Link-local address " << linkLocalIpAddress << endl;
    {

        //gLogMessageRaw
        //  <<" Removing and adding Link-Local address to force the kernel to start the DAD process. \n" <<endl;

        // This will actually try to add this address to the interface even if the interface is already configured with the
        // same address.
        pIpHalIf->RemoveIpv6Address(linkLocalIpAddress);
        BcmOperatingSystemFactory::ThreadSleep(1000);
        pIpHalIf->AddIpv6LinkLocalAddress();

        // sleep 6 seconds to allow the IP stack to send the MLD message -
        // I'm not sure why the stack waits 5-7 seconds to join for the link-local address.
        //BcmOperatingSystemFactory::ThreadSleep(4000);
    }

    // The time between retransmissions of Neighbor Solicitation messages to a neighbor when
    // resolving the address or when probing the reachability of a neighbor.
    //
    // kick off neighbor discovery on link-local address.  The result will be picked up in HandleEvent above. 
    pIpHalIf->PerformDuplicateAddressDetection(pACT);
}
#endif


// Result of link-local address DAD
//
// Parameters:
//      none
//
// Returns:  Nothing.
//
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
void BcmEstbIpThread::DuplicateAddressDetectionOnLinkLocalAddressComplete( bool duplicateDetected )
{
    if( fIpV6State == kDuplicateAddressDetectionOfLinkLocalAddressInProgress )
    {
        if( duplicateDetected == true )
        {
            printf("ERROR:  Duplicate detected.  Manual intervention required.\n");

            #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
            // log event kEE_D01301 "Link-Local address failed DAD"
            DhcpV6LogEvent(kEE_D01301);
            #endif

            // Remove the link-local address now that a duplicate was detected
            BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
            BcmIpV6Address linkLocalAddress;
            pIpHalIf->GetIpv6LinkLocalAddress( linkLocalAddress );
            pIpHalIf->RemoveIpv6Address( linkLocalAddress );

            IpV6AddressAcquisitionFailed();
        }
        else
        {
            printf("No duplicates found.  Adding link-local address to IP stack.\n");

            // Kick off router advertisement collection
            pfMessageQueue->Send( kObtainRouterAdvertisement );
        }   
    }
}
#endif

// Obtain router advertisement
//
// Parameters:
//      none
//
// Returns:  Nothing.
//
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT) //(BCM_IPV6_SUPPORT)

void BcmEstbIpThread::ObtainRouterAdvertisement( void )
{ 
    BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
    BcmNeighborDiscoveryThread * pNeighborDiscoveryThread = pIpHalIf->NeighborDiscoveryThread();

    // if we have received a router advertisement, proceed to DHCPv6
    if( pNeighborDiscoveryThread->DefaultRouterList().IsEmpty() == false )
    {
        // reset my counter
        fRouterSolicitationCount = 0;

        // Kick off DHCPv6
        pfMessageQueue->Send( kPerformDhcpV6 );
    }

    // else, if not RAs yet, solicit one
    else if( fRouterSolicitationCount < MAX_RTR_SOLICITATIONS )
    {
        fRouterSolicitationCount++;
//#warning "PTASKIRAN-DHCPv6 MODS- STUB - BcmEstbIpThread::ObtainRouterAdvertisement"
#if !(BCM_STANDALONE_DHCP_CLIENT)
        printf("Sending IPv6 router solicitation number %d\n", fRouterSolicitationCount);

        // request the neighbor discovery thread send a router solicitation
        pNeighborDiscoveryThread->SendRouterSolicitation();

        pfRouterSolicitationTimer->Start(RTR_SOLICITATION_INTERVAL_MS);
#else
        printf("Obtaining router advertisement\n");
        printf("Default RouterList Is Empty. Starting Router Solicitation Timer! \n");
        //pfRouterSolicitationTimer->Start(MAX_INITIAL_RTR_ADVERT_INTERVAL *1000);
        // I changed this back to router solicitation timer value because
        // IP stack will send a router solicitation as soon as we add the link local address.
        pfRouterSolicitationTimer->Start(RTR_SOLICITATION_INTERVAL_MS);
#endif
        
    }

    // else, max number of solicits has been exceeded.  we failed. 
    else
    {
        cout <<"Failed to receive router advertisement" << endl;

        // reset my counter
        fRouterSolicitationCount = 0;

        #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
        // log event "DHCP failed - RS sent, no RA received"
        DhcpV6LogEvent(kEE_D01200);
        #endif

        // Without a router advertisement, we are supposed to fail IP initialization
        IpV6AddressAcquisitionFailed();
    }
}
#else
#warning "PTASKIRAN-DHCPv6 MODS- STUB - BcmEstbIpThread::ObtainRouterAdvertisement"
void BcmEstbIpThread::ObtainRouterAdvertisement( void )
{ 

        // reset my counter
        fRouterSolicitationCount = 0;

        // Kick off DHCPv6
        pfMessageQueue->Send( kPerformDhcpV6 );

}
#endif

// IPv6 IP address acquisition failed at one of a number of possible points
//
// Parameters:
//      none
//
// Returns:  Nothing.
//
#if (BFC_INCLUDE_DHCPV6_SUPPORT)//(BCM_IPV6_SUPPORT)
void BcmEstbIpThread::IpV6AddressAcquisitionFailed()
{
    printf("IPv6 address acquisition failed...\n");

    // reset IPv6 state
    fIpV6State = kIpV6NotStarted;

    if( fIpState != kIpSuccess )
    {
        // if the preferred method was IPv6 and that failed 
        // AND alternate IP management mode was enabled, kick off IPv4
        // IP address acquisition
        if( fPreferredIpVersionIsIPv6 && fAlternameIpManagementModeEnabled )
        {
            printf("Docsis altername management mode is enabled...\n");

            fIpState = kIpDhcpInProgress;

            if (pfMessageQueue->Send( kStartIpV4Init ) == false)
            {
                // Reset my state.
                fIpState = kIpNotStarted;

                pfConfigFileBuffer = NULL;

                gErrorMsg(fMessageLogSettings, "StartIpInit") 
                    << "Failed to send message to my thread!" << endl;
            }
        }
        else 
        {
            printf("Docsis alternate management mode is disabled.  Giving up...\n");

            fIpState = kIpFailedDhcp;
            
            PublishEvent(kEstbDhcpInitFailed, fLastEventIdLogged);
            //#warning " PTASKIRAN- DHCPv6 MODS - CALL ObtainRouterAdvertisement () in BcmEstbIpThread::IpV6AddressAcquisitionFailed to restart DHCPv6"
            //ObtainRouterAdvertisement();
        }
    }
}
#endif

// Kicks off DHCPv6.  Takes nonvol settings into account.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
#if (BFC_INCLUDE_DHCPV6_SUPPORT) //(BCM_IPV6_SUPPORT)
void BcmEstbIpThread::StartDhcpV6(void)
{
    BcmBaseIpHalIf *pIpHalIf;
    bool result;

    // Grab my IP stack.
    pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    if(pIpHalIf)
        gAlwaysMsgNoFields(fMessageLogSettings) << "Starting IP Initialization with DHCPv6 for " <<pIpHalIf->Description() << endl;

    //printf(" Interface = %s  [IF number = %d] [Stack number = %d]\n", pIpHalIf->Description(), pIpHalIf->InterfaceNumber(), fStackNumber);

    // Update my state and publish event if we are the primary IP version
    if( fIpState == kIpNotStarted )
    {
        //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitStarted);
        PublishEvent(kEstbDhcpInitStarted);

        fIpState = kIpDhcpInProgress;
    }

    // Set the DHCPv6 options that will be sent to the server.
    {
        gAlwaysMsgNoFields(fMessageLogSettings) << "Generating DHCPv6 packet with these options: " << endl;
        BcmDhcpV6OptionsList clientOptions;

        uint16 dhcpOptionPolicy[kDefault_NumberOfOptions] = kDefault_DhcpPolicy;
        for( uint32 i=0; i<kDefault_NumberOfOptions; i++ )
        {
            switch(dhcpOptionPolicy[i])
            {
                case kDhcpV6OptionRapidCommit:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "RapidCommit" << endl;
                        // Include the Rapid Commit option.
                        BcmDhcpV6RapidCommitOption rapidCommitOption;
                        clientOptions.Add(rapidCommitOption);
                    }
                    break;

                case kDhcpV6OptionReconfigureAccept:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "ReconfigureAccept" << endl;
                        // Include a reconfigure accept option.
                        BcmDhcpV6ReconfigureAcceptOption reconfigureAcceptOption;
                        clientOptions.Add(reconfigureAcceptOption);
                    }
                    break;

                case kDhcpV6OptionVendorSpecificInfo:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "VendorSpecificInfo" << endl;
                        AddCableLabsVendorSpecificInfoOption(clientOptions);
                    }
                    break;

                case kDhcpV6OptionVendorClass:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "VendorClass" << endl;

                        BcmDhcpV6VendorClassOption vendorClassOption;
                        vendorClassOption.EnterpriseNumber(4491);   // vendor-specific enterprise number

                        BcmOctetBuffer vendorClassData(16);
                        vendorClassData.AddToEnd(kDefault_VendorClassString, false); // vendor-specific class
                        vendorClassOption.Add(vendorClassData);

                        //gAlwaysMsgNoFields(fMessageLogSettings)
                        //  << vendorClassData.AccessBuffer() << endl;

                        clientOptions.Add(vendorClassOption);
                    }
                    break;

                case kDhcpV6OptionRequestOption:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "RequestOption" << endl;

                        // Configure the options that I want to request from the server.
                        BcmDhcpV6OptionRequestOption optionRequestOption;
                        //optionRequestOption.Add(23); //OPTION_DNS_SERVERS (23)
                        //optionRequestOption.Add(24); //OPTION_DOMAIN_LIST (24)
                        //clientOptions.Add(optionRequestOption);

                        uint16 dhcpOptionRequestOptionPolicy[kDefault_NumberOfOptionRequestOptions] = kDefault_DhcpOptionRequestOptionPolicy;
                        for( uint32 i=0; i<kDefault_NumberOfOptionRequestOptions; i++ )
                        {
                            optionRequestOption.Add(dhcpOptionRequestOptionPolicy[i]);
                        }
                        clientOptions.Add(optionRequestOption);
                    }
                    break;

                default:
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings)
                            << "Option code ot recognized" << endl;
                    }


            }
        }

        pIpHalIf->DhcpV6ClientIf()->SetClientOptions(clientOptions);
    }

    result = pIpHalIf->DhcpV6ClientIf()->AcquireAddresses();

    // See if there was a problem.
    if (result == false)
    {
        // Alert the media...
        gErrorMsg(fMessageLogSettings, "StartDhcpV6") 
            << "Failed to start DHCPv6!" << endl;

        // update state and publish event only if we are the primary IP version
        if( fIpState == kIpDhcpInProgress )
        {
            fIpState = kIpFailedDhcp;
            
            PublishEvent(kEstbDhcpInitFailed, fLastEventIdLogged);
        }
    }

    // Temp workaround to process soft timers used in DHCP thread.
    pfPostCallDelayTimer->Start(1000);
}
#endif

// Kicks off DHCP.  Takes nonvol settings into account.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::StartDhcp()
{
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  

    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    BcmCmDocsisNonVolSettings *pSettings;
    #endif
    BcmBaseIpHalIf *pIpHalIf;

    CallTrace("BcmEstbIpThread", "StartDhcp");

    // Alert the media...
    printf("Starting IPv4 Initialization with DHCP...\n");

    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitStarted);
    PublishEvent(kEstbDhcpInitStarted);

    // Grab my IP stack.
    pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    // Get a pointer to the lease for this stack.  We always use the MAC address
    // as the client id.
    BcmDhcpClientLease *pLease = GetLease();

    if (pLease == NULL)
    {
        gErrorMsg(fMessageLogSettings, "StartDhcp")
            << "Can't find lease object associated with my IP stack!  Can't start DHCP!" << endl;
    
        fIpState = kIpFailedDhcp;

        //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitFailed);
        PublishEvent(kEstbDhcpInitFailed);

        return;
    }

    // Make sure the IP state is cleared (ARP and routing tables, etc.).
    StopAndResetSync(false);

    // Update my state again; it was cleared by StopAndResetSync().
    fIpState = kIpDhcpInProgress;

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // Check the non-vol settings to see if we are supposed to bypass DHCP.  If
    // so, then load the DHCP settings with non-vol values, and cause events to
    // occur that simulate the successful completion of DHCP.
    pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
    if (((pSettings != NULL) && (pSettings->DhcpEnable() == false)) )
    {
        bool result;

        gAlwaysMsgNoFields(fMessageLogSettings) 
            << "DHCP is disabled in non-vol settings; using non-vol IP parameters..." << endl;

        // Configure the lease for static operation.
        pLease->ServerLeaseSettings().Reset();

        // Load the settings up from nonvol.
        pLease->ServerLeaseSettings().YourIpAddress(pSettings->IpAddress());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpSubnetMask, 
                                                       pSettings->SubnetMask());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpRouter, 
                                                       pSettings->Router());
        pLease->ServerLeaseSettings().ServerIpAddress(pSettings->TftpServer());
        pLease->ServerLeaseSettings().FileName(pSettings->ConfigFilename());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpTimeServer, 
                                                       pSettings->TimeServer());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpLogServer, 
                                                       pSettings->LogServer());

        // Lock in the lease.
        result = pIpHalIf->DhcpClientIf()->ConfigureStaticLease(pLease->ClientId());

        if (!result)
        {
            gErrorMsg(fMessageLogSettings, "StartDhcp")
                << "Failed to configure static lease!" << endl;
        }
    }
    else
#endif
    {
        SetupAndInitiateDhcpV4(pLease);
    }
#endif
}

// Kicks off DHCP after DHCPv6 has completed and CM is already operational. 
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::StartDhcpAsNonPreferredIpVersion()
{
#if (BFC_INCLUDE_DHCPV4_SUPPORT) 

    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    BcmCmDocsisNonVolSettings *pSettings;
    #endif
    BcmBaseIpHalIf *pIpHalIf;

    CallTrace("BcmEstbIpThread", "StartDhcp");

    // Alert the media...
    printf("Starting IPv4 Initialization with DHCP...\n");

    // Grab my IP stack.
    pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    // Get a pointer to the lease for this stack.  We always use the MAC address
    // as the client id.
    BcmDhcpClientLease *pLease = GetLease();

    if (pLease == NULL)
    {
        gErrorMsg(fMessageLogSettings, "StartDhcp")
            << "Can't find lease object associated with my IP stack!  Can't start DHCP!" << endl;

        return;
    }
    
    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    // Check the non-vol settings to see if we are supposed to bypass DHCP.  If
    // so, then load the DHCP settings with non-vol values, and cause events to
    // occur that simulate the successful completion of DHCP.
    pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
    if ((pSettings != NULL) && (pSettings->DhcpEnable() == false))
    {
        bool result;

        gAlwaysMsgNoFields(fMessageLogSettings) 
            << "DHCP is disabled in non-vol settings; using non-vol IP parameters..." << endl;

        // Configure the lease for static operation.
        pLease->ServerLeaseSettings().Reset();

        // Load the settings up from nonvol.
        pLease->ServerLeaseSettings().YourIpAddress(pSettings->IpAddress());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpSubnetMask, 
                                                       pSettings->SubnetMask());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpRouter, 
                                                       pSettings->Router());
        pLease->ServerLeaseSettings().ServerIpAddress(pSettings->TftpServer());
        pLease->ServerLeaseSettings().FileName(pSettings->ConfigFilename());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpTimeServer, 
                                                       pSettings->TimeServer());
        pLease->ServerLeaseSettings().AddReplaceOption(kDhcpLogServer, 
                                                       pSettings->LogServer());

        // Lock in the lease.
        result = pIpHalIf->DhcpClientIf()->ConfigureStaticLease(pLease->ClientId());

        if (!result)
        {
            gErrorMsg(fMessageLogSettings, "StartDhcp")
                << "Failed to configure static lease!" << endl;
        }
    }
    else
    #endif
    {
        SetupAndInitiateDhcpV4(pLease);
    }
#endif
}
#if (BFC_INCLUDE_DHCPV4_SUPPORT)  
// Handles the specified DHCP event (sent to us by the DHCP lease object
// via our ACT).
//
// Parameters:
//      pLease - pointer to our lease
//
// Returns:  Nothing.
//
void BcmEstbIpThread::SetupAndInitiateDhcpV4(BcmDhcpClientLease *pLease)
{
    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    BcmCmDocsisNonVolSettings *pSettings;
    pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
    #endif

    BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

    bool result;
    BcmOctetBuffer buffer(1024);
    
    // These are the option values that we will request from the server
    // via the parameter request list, and are also used when setting the
    // preferred fields.

    //Option value based on OpenCable host spec
    uint8 optionsListValues[] =
    {
        kDhcpSubnetMask,
        kDhcpRouter,
        kDhcpDomainNameServer,
        kDhcpDomainName,
        kDhcpDefaultIpTTL,
        kDhcpIpAddressLeaseTime,
        kDhcpServerIdentifier
    };

    //Option value required by OpenCable Host devices
    uint8 requiredOptionsListValues[] =
    {
        kDhcpRouter,
        kDhcpSubnetMask,
        kDhcpRouter,
        kDhcpIpAddressLeaseTime,
        kDhcpServerIdentifier
    };

    BcmOctetBuffer requiredOptionsList(requiredOptionsListValues,
                                       sizeof(requiredOptionsListValues), 
                                       sizeof(requiredOptionsListValues), 
                                       false);    
    
    // Wipe out any existing settings.
    pLease->ClientLeaseSettings().Reset();
    
    if (pfConfigFileBuffer != NULL)
    {
        // As a DOCSIS IP Helper, I know that there are certain fields
        // required in the DHCP Settings in order for it to be valid.  Make
        // sure the DHCP client will enforce this.
        //
        // PR1788 - ECN RFI-N-01100 changes the required fields so that only
        // the first 3 are mandatory.
        pLease->SetRequiredFields(BcmDhcpClientLease::kYourAddr |
                                  BcmDhcpClientLease::kServerAddr |
                                  BcmDhcpClientLease::kFileName, requiredOptionsList );
    }
    else
    {
        // If the config file buffer was not specified, then IP init was
        // started by the console.  In this case, we aren't doing the
        // DOCSIS thing, we're just getting our IP address, usually over
        // ethernet, so all we really need is the IP address.
        //pLease->SetRequiredFields(BcmDhcpClientLease::kYourAddr);
        // Added requiredOptionList for OpenCable Host device
        pLease->SetRequiredFields(BcmDhcpClientLease::kYourAddr,requiredOptionsList);
    }
    
    // Set up the parameter request list if compiled for Docsis2.0 
    #if( !BFC_MULPI_SUPPORT )
    {
        buffer.AddToEnd(optionsListValues, sizeof(optionsListValues));
    
        // Add the PacketCable servers option number.
        #if (EMTA_SUPPORT)
        buffer.AddToEnd(fPacketCableDhcpOptionNumber);
        #endif
    
        // Set the parameter request list option.
        pLease->ClientLeaseSettings().AddReplaceOption(kDhcpParamRequestList,
                                                       buffer);
    }
    #endif

    // Set the fields that we would like to have; this is used when
    // determining whether a given offer is better than another one.
    pLease->SetPreferredFields(BcmDhcpClientLease::kYourAddr |
                               BcmDhcpClientLease::kServerAddr |
                               BcmDhcpClientLease::kFileName,
                               buffer);

    #if (NOT_INCLUDED_IN_ESTB_DHCP)
    // if more than one downstream receiver is supported then assume 
    // that the CM supports downstream bonding.  for pre-docsis 3.0
    // deployment we will advertise "docsis2.0" and include an
    // option 43 vendor specific sub-option for number of ds rx chans.
    uint8 numDsRxSupported = fModemCapabilitiesObject.fNumDsRxSupported;
    #endif
    
#if 1
/* UNFINISHED - ESTB DHCPv4 CONFIGURATION, some value need retrieve from SNMP 
agent when this dhcp client is put into Open Cable Host app. Now hardcode them. 
***********************************************************************  */
    // Set option 60 - Vendor Class Id.  This must be one of the DOCSIS
    // strings, with our modem capabilities converted to ASCII 
    {
        buffer.Empty();

        // Opencable Host 2.1
        buffer.AddToEnd("OpenCable2.0", false);
        pLease->ClientLeaseSettings().AddReplaceOption(kDhcpVendorClassIdentifier,
                                                       buffer);
    }

    // Set option 61 Client identifier option.  I 
    {
        buffer.Empty();
        buffer.NetworkAddToEnd((uint8) 7);
        buffer.NetworkAddToEnd((uint8) 1);

        // BcmOctetBuffer optionField(64);
        uint8 macAddress[6];
        //Need add mac address of ipHalif //
        pIpHalIf->MacAddress().Get(macAddress[0],macAddress[1],macAddress[2],macAddress[3],macAddress[4],macAddress[5]);
        buffer.NetworkAddToEnd((uint8) macAddress[0]);
        buffer.NetworkAddToEnd((uint8) macAddress[1]);
        buffer.NetworkAddToEnd((uint8) macAddress[2]);

        buffer.NetworkAddToEnd((uint8) macAddress[3]);
        buffer.NetworkAddToEnd((uint8) macAddress[4]);
        buffer.NetworkAddToEnd((uint8) macAddress[5]);
        // Opencable Host 2.1
        //buffer.AddToEnd(optionField, false);
        pLease->ClientLeaseSettings().AddReplaceOption(kDhcpClientIdentifier,
                                                       buffer);
    }

    // PR6147 - Option 43 handling code.  In addition to the old Option43
    // callout, we now need to do some eSTB stuff.
    {
        buffer.Empty();
        // Generate sub-option 2; this is always the string "ECM".
        {
            buffer.NetworkAddToEnd((uint8) 2);
            buffer.NetworkAddToEnd((uint8) 4);
            buffer.AddToEnd("ESTB", false);
        }
        // Generate sub-option 3; this is built based on which SAFEs
        // are embedded.
        {
            BcmOctetBuffer subOpt3(64);
            subOpt3.AddToEnd("ECM:ESTB", false);
            buffer.NetworkAddToEnd((uint8) 3);
            buffer.NetworkAddToEnd((uint8) subOpt3.BytesUsed());
            buffer.Append(subOpt3);
        }    
        // Generate sub-option 4; this comes from docsDevSerialNumber.
        // This is a manufactured value, and always exists in the SNMP
        // nonvol, so I can just pull it from there.
        {
        //   CmSnmpNonVolSettings *pSettings = CmSnmpNonVolSettings::GetSingletonInstance();
        // Give a temp value, later get from SNMP 
            buffer.NetworkAddToEnd((uint8) 4);
            buffer.NetworkAddToEnd((uint8) 9);
            buffer.AddToEnd("123456789", false);
        }
    
        // Generate sub-option 5; this comes from sysDescr, and is the
        // hardware rev part of that string.
        {
            BcmString::size_type startOffset, endOffset;
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("HW_REV: ");
            //startOffset += 8;
    
            // Find the semicolon that signals the end of the value.
            //endOffset = sysDescr.find_first_of(';', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 5);
            buffer.NetworkAddToEnd((uint8) 5);
            buffer.AddToEnd("ABCDE", false);
        }
    
        // Generate sub-option 6; this is also from sysDescr, and is the
        // s/w version field.
        {
            BcmString::size_type startOffset, endOffset;
    
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("SW_REV: ");
            startOffset += 8;
    
            // Find the semicolon that signals the end of the value.
            //endOffset = sysDescr.find_first_of(';', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 6);
            //buffer.NetworkAddToEnd((uint8) (endOffset - startOffset));
            //buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);
            buffer.NetworkAddToEnd((uint8) 1);
            buffer.AddToEnd("1", false);
        }
    
        // Generate sub-option 7; this is also from sysDescr, and is the
        // boot rom version field.
        {
            BcmString::size_type startOffset, endOffset;
    
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("BOOTR: ");
            startOffset += 7;
    
            // Find the semicolon that signals the end of the value.
            //endOffset = sysDescr.find_first_of(';', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 7);
            //buffer.NetworkAddToEnd((uint8) (endOffset - startOffset));
            //buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);
            buffer.NetworkAddToEnd((uint8) 1);
            buffer.AddToEnd("1", false);
        }
    
        // Generate sub-option 8; this is the vendor id/OUI, converted
        // from hex to ASCII.
        {
            // This constant contains the vendor OUI, in partial MAC
            // address form, but as a string.  I need to skip around
            // the colons in the middle.
            const char *pVendorOui = kCustomerVendorId;
    
            buffer.NetworkAddToEnd((uint8) 8);
            buffer.NetworkAddToEnd((uint8) 6);
            buffer.NetworkAddToEnd(pVendorOui[0]);
            buffer.NetworkAddToEnd(pVendorOui[1]);
            buffer.NetworkAddToEnd(pVendorOui[3]);
            buffer.NetworkAddToEnd(pVendorOui[4]);
            buffer.NetworkAddToEnd(pVendorOui[6]);
            buffer.NetworkAddToEnd(pVendorOui[7]);
        }
    
        // Generate sub-option 9; this is also from sysDescr, and is the
        // model number field.
        {
            BcmString::size_type startOffset, endOffset;
    
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("MODEL: ");
            startOffset += 7;
    
            // Find the ">" that signals the end of the value.  This one
            // is different from the others.
            //endOffset = sysDescr.find_first_of('>', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 9);
            //buffer.NetworkAddToEnd((uint8) (endOffset - startOffset));
            //buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);
            buffer.NetworkAddToEnd((uint8) 1);
            buffer.AddToEnd("9", false);
        }
    
        // Generate sub-option 10; this is also from sysDescr, and is
        // the vendor name field.
        {
            BcmString::size_type startOffset, endOffset;
    
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("VENDOR: ");
            startOffset += 8;
    
            // Find the semicolon that signals the end of the value.
            //endOffset = sysDescr.find_first_of(';', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 10);
            //buffer.NetworkAddToEnd((uint8) (endOffset - startOffset));
            //buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);
            buffer.NetworkAddToEnd((uint8) 2);
            buffer.AddToEnd("10", false);
        }

        // Generate sub-option 54; this is also from sysDescr, and is
        // the vendor name field.
        {
            BcmString::size_type startOffset, endOffset;
    
            // Find the keyword we are looking for, then skip ahead to
            // the interesting data.
            //startOffset = sysDescr.find("VENDOR: ");
            startOffset += 8;
    
            // Find the semicolon that signals the end of the value.
            //endOffset = sysDescr.find_first_of(';', startOffset);
    
            buffer.NetworkAddToEnd((uint8) 54);
            //buffer.NetworkAddToEnd((uint8) (endOffset - startOffset));
            //buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);
            buffer.NetworkAddToEnd((uint8) 2);
            buffer.AddToEnd("54", false);
        }
            
        // If a client registered a callout with me for Option43, then call
        // it and add the buffer to the lease.
        if (fOption43Callout != NULL)
        {
            // Create a temporary buffer so that the client code can store
            // their values.
            unsigned int numberOfBytes = 0;
            BcmOctetBuffer calloutBuffer(512);
    
            // See if the client has anything interesting to add.
            fOption43Callout(calloutBuffer.AccessBuffer(), &numberOfBytes);
    
            // Set the number of bytes they claim to have consumed.
            calloutBuffer.SetBytesUsed(numberOfBytes);
    
            // Append this to my Option43 buffer, which may have data
            // corresponding to the eDOCSIS requirements, or which may be
            // empty.
            buffer.Append(calloutBuffer);
        }
    
        // If we have any Option43 data from the code above, add it to the
        // lease.
        if (buffer.BytesUsed() > 0)
        {
            pLease->ClientLeaseSettings().AddReplaceOption(kDhcpVendorSpecInfo,
                                                           buffer);
        }
    }
#else
    // Alert the media...
    gErrorMsg(fMessageLogSettings, "StartDhcp") 
        << "eSTB DHCPv4 options are not configured!!!!" << endl;
    
#endif
    
// Kick off DHCP, with a short initial delay.  I can get away with this
    // because the DOCSIS startup is sufficiently randomized.
    result = pIpHalIf->DhcpClientIf()->AcquireLease(pLease->ClientId(), true);
    
    // See if there was a problem.
    if (result == false)
    {
        fIpState = kIpFailedDhcp;
    
        // Alert the media...
        gErrorMsg(fMessageLogSettings, "StartDhcp") 
            << "Failed to start DHCP!" << endl;
    
        //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitFailed);
        PublishEvent(kEstbDhcpInitFailed);
    }
}
#endif //#if (BFC_INCLUDE_DHCPV4_SUPPORT)  

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
// Handles the specified DHCP event (sent to us by the DHCP lease object
// via our ACT).
//
// Parameters:
//      event - the event that occurred.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::HandleDhcpEvent(BcmDhcpClientLease::DhcpClientLeaseEvent event)
{
    CallTrace("BcmEstbIpThread", "HandleDhcpEvent");

    switch (event)
    {
        case BcmDhcpClientLease::kEventLeaseBound:
            if (fIpState == kIpDhcpInProgress)
            {
                // Change the fields that are required.
                //
                // PR1788 - ECN RFI-N-01100 states that after DHCP is complete
                // the first time, only the CM IP address is required.
                GetLease()->SetRequiredFields(BcmDhcpClientLease::kYourAddr);

                // Alert the media...
                printf("DHCP completed successfully!\n");

                //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitOk);
                PublishEvent(kEstbDhcpInitOk);

                // if APM is enabled, and DHCPv6 failed to complete
                if( fAlternameIpManagementModeEnabled && (fIpV6State != kIpV6Success) ) 
                {
                    // log event kD12_5 "Primary address failed, secondary active"
                    // PTMOD - LOG EVENT CmLogEvent(kD12_5);
                }

                #if (NOT_INCLUDED_IN_ESTB_DHCP)
                bool fakeTod = false;

                BcmDhcpClientLease *pLease = GetLease();
   
                BcmCmDocsisNonVolSettings *pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
                if ( (pSettings->TimeOfDayEnable() == false) )
                {
                    gAlwaysMsgNoFields(fMessageLogSettings) 
                        << "Skipping Time Of Day (disabled by non-vol settings)." << endl;

                    fakeTod = true;
                }
                // Also check to see if there is a ToD server specified; if not, then we
                // need to fake it out.
                //
                // PR1788 - ECN RFI-N-01100 
                else if (pLease->ServerLeaseSettings().OptionIsPresent(kDhcpTimeServer) == false)
                {
                    gAlwaysMsgNoFields(fMessageLogSettings)
                        << "Time server not specified!" << endl;

                    fakeTod = true;
                }   

                int32 timeOffset = 0;
                BcmIpAddressList timeServers;

                pLease->ServerLeaseSettings().GetOption(kDhcpTimeServer, timeServers);
                pLease->ServerLeaseSettings().GetOption(kDhcpTimeOffset, timeOffset);
                
                // Kick off Time Of Day.  This runs asynchronously; I don't
                // wait for it to complete.
                StartTod(fakeTod, timeServers, timeOffset);

                // Now start TFTP of the config file.
                //PublishEvent(BcmCmDocsisStatusEventCodes::kCmTftpInitStarted);
            
                // Update my state.
                fIpState = kIpTftpInProgress;
                SetupIPv4ConfigFileTftpParameters();
                TftpConfigFile();
                #endif
            }
            else if (fIpState == kIpSuccess)
            { 
                // PR 8120: initialize newTimeOffset to 0 in case UTC offset 
                // option is not present in DHCP lease settings.  Otherwise we 
                // will wind up with an uninitialized value which may be 
                // something bizarre.
                int32 newTimeOffset = 0, oldTimeOffset = 0;

                gLogMessageRaw << "DHCP lease renewed successfully!" << endl;

                // PR5736 - tell anyone who cares that the renewal completed
                // successfully.
                //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewSuccess);
                PublishEvent(kEstbDhcpRenewSuccess);

                #if (NOT_INCLUDED_IN_ESTB_DHCP)
                oldTimeOffset = pfTodThread->GetUtcOffset();
                GetLease()->ServerLeaseSettings().GetOption(kDhcpTimeOffset, newTimeOffset);

                // Check to see if the UTC offset value changed; if yes, then
                // we need to redo ToD so that the system time is changed to
                // reflect this.
                if (oldTimeOffset != newTimeOffset)
                {
                    // Store the new value.
                    pfTodThread->SetUtcOffset(newTimeOffset);

                    // If ToD hasn't been acquired, then all we need to do is
                    // store the new tiem offset value.
                    if (pfTodThread->GetTodState() == BcmTimeOfDayThread::kAcquired)
                    {
                        gLogMessageRaw
                            << "The UTC Time Offset value changed!  Old value = "
                            << oldTimeOffset << "; new value = " << newTimeOffset
                            << "\nRe-doing Time Of Day to update system clock." << endl;

                        // PTMOD - LOG EVENT CmLogEvent( kD106_0, "UTC Time Offset");

                        // Rather than calling the StartTod() helper (which
                        // does too much work, makes too many changes, and
                        // publishes too many events), we will just reconfigure
                        // the thread, stop it, and start it.
                        pfTodThread->Stop();
                    
                        // DOCSIS requires us to use UDP.
                        pfTodThread->GetTimeOfDay(BcmTimeOfDayThread::kProtocolUdp);
                    }
                }
                #endif
            }
            break;

        case BcmDhcpClientLease::kEventLeaseFailed:
    
            // IPv6 good 
            if( fIpV6State == kIpV6Success )
            {
                // DHCP fails with IPv6 OK
                if( fIpState == kIpDhcpInProgress ) 
                {
                    // Alert the media...
                    gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                        << "DHCP failed!  DHCPv6 still OK - not reiniting MAC" << endl;

                    // log event "Primary address acquired, secondary failed"
                    // PTMOD - LOG EVENT CmLogEvent( kD12_4 );
                }
                else
                {
                    // Alert the media...
                    gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                        << "DHCP renew failed!  DHCPv6 still OK - not reiniting MAC" << endl;
                }
            }
            else
            {
                if( fIpState == kIpDhcpInProgress ) 
                {
                    // Alert the media...
                    gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                        << "DHCP failed!" << endl;
    
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitFailed);
                    PublishEvent(kEstbDhcpInitFailed, fLastEventIdLogged);
                }

                // IPv6 failed or was never used - DHCP was good and then failed
                else if( fIpState == kIpSuccess ) 
                {
                    // Alert the media...
                    gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                        << "DHCP renew failed!" << endl;
    
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewFailed);
                    PublishEvent(kEstbDhcpRenewFailed);
                }
            }

            fIpState = kIpFailedDhcp;

            break;

        case BcmDhcpClientLease::kEventLeaseLost:
            {
                // We failed to renew our lease.
                fIpState = kIpFailedDhcp;
                
                // Alert the media...
                gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                    << "DHCP renew failed!" << endl;
    
                // If a CM is configured to use DPM and the CM fails to renew one of its IP
                // addresses, the CM MUST note the event in the Local Event Log. Failure to
                // renew an IP address when the other IP address is active is not a
                // critical event.  In this case, after noting the failure in the Local
                // Event Log, the CM MUST continue to operate with the remaining IP
                // address. On the other hand, failure to renew an IP address is a critical
                // event when the other IP address has already expired. When a CM operating
                // in DPM fails to successfully renew its only remaining IP address, the CM
                // MUST reinitialize the MAC as defined in Section 10.2.
                bool publishDhcpRenewFailed = true;
                #if (BCM_IPV6_SUPPORT )
                if( (fDualStackOperationEnabled == true) && (fIpV6State != kFailedDhcpV6) )
                {
                    gErrorMsg(fMessageLogSettings, "HandleDhcpEvent")
                        << "DPM is enabled and IPv6 is operational.  Not reinitializing MAC!" << endl;
                     publishDhcpRenewFailed = false;
                }
                #endif
                if( publishDhcpRenewFailed == true ) 
                {
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewFailed);
                    PublishEvent(kEstbDhcpRenewFailed);
                }
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
                //Need restart DHCP if v4 version is running.
                if(dhcpVersion == 1)
                {
                    gLogMessageRaw <<"DHCP V4 re-enter initial state." << endl;
                    if (pfMessageQueue->Send( kStartIpV4Init ) == false)
                    {
                        // Reset my state.
                        fIpState = kIpNotStarted;
                        gErrorMsg(fMessageLogSettings, "StartIpInit") 
                            << "Failed to send message to my thread!" << endl;
                     }
                }
#endif 
            }
            break;

        default:
            gWarningMsg(fMessageLogSettings, "HandleDhcpEvent")
                << "Unexpected event received from DHCP client ("
                << (int) event << "); ignoring..." << endl;
            break;
    }
}
#endif //#if (BFC_INCLUDE_DHCPV4_SUPPORT)

#if (BFC_INCLUDE_DHCPV6_SUPPORT)//(BCM_IPV6_SUPPORT)
void BcmEstbIpThread::HandleDhcpV6Event(BcmDhcpV6ClientIf::DhcpV6ClientLeaseEvent event)
{
    CallTrace("BcmEstbIpThread", "HandleDhcpV6Event");

    switch (event)
    {
        case BcmDhcpV6ClientIf::kEventAddressAcquiredTentative:
            {
                // re-publish this event
                //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpv6IpAddressAcquiredTentative);
                PublishEvent(kEstbDhcpv6IpAddressAcquiredTentative);
            }
            break;

        case BcmDhcpV6ClientIf::kEventAddressAcquiredSuccess:
            {
                // Alert the media...
                printf("DHCPv6 completed successfully!\n");

                fIpV6State = kIpV6Success;

                BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

                // save the IA so we can detect a change when we renew
                BcmDhcpV6IaAddress iaAddress;
                pIpHalIf->DhcpV6ClientIf()->IA().Get(0, iaAddress);
                fIpV6Address = iaAddress.IpV6Address();

                // only publish this event and TFTP config file if IPv4 has not already done so. 
                if( fIpState == kIpDhcpInProgress )
                {
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpInitOk);
                    PublishEvent(kEstbDhcpInitOk);

                    bool fakeTod = false;

                    int32 timeOffset = 0;
                    BcmIpAddressList timeServers;
#ifdef STUB
                    BcmCmDocsisNonVolSettings *pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();
                    if ( pSettings->TimeOfDayEnable() == false )
                    {
                        gAlwaysMsgNoFields(fMessageLogSettings) 
                            << "Skipping Time Of Day (disabled by non-vol settings)." << endl;

                        fakeTod = true;
                    }
#else
//#warning "PTASKIRAN-DHCPv6 MODS - STUB " 
                    if(1)
                        fakeTod = true;                   
#endif
                    // Also check to see if there is a ToD server specified; if not, then we
                    // need to fake it out.
                    //
                    // PR1788 - ECN RFI-N-01100 
                    else
                    {
                        BcmDhcpV6CableLabsVendorSpecificInfoOption clVendorOptions;
                        BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

                        gLogMessageRaw
                            << "Looking for ToD server/offset in DHCPv6 options..." << endl;

                        // Pull out the cablelabs vendor specific options
                        if( pIpHalIf->DhcpV6ClientIf()->Options().Get(clVendorOptions) == true )
                        {
                            // Get the ToD server IPv6 address suboption.
                            BcmDhcpV6DocsisTimeProtocolServersOption docsisTimeProtocolServersOption;
                            if (clVendorOptions.Options().Get( docsisTimeProtocolServersOption ) == false)
                            {
                                gWarningMsg(fMessageLogSettings, "HandleDhcpV6Event")
                                    << "Failed to get ToD server IP address!" << endl;

                                fakeTod = true;
                            }
                            else
                            {
                                // add the IP addresses(s) in the option into the list
                                unsigned int i;
                                for( i=0; i < docsisTimeProtocolServersOption.NumberOfIpV6Addresses(); i++ ) 
                                {
                                    timeServers.Add( docsisTimeProtocolServersOption.Get( i ) );
                                }
                            }

                            // Get the Time Offset value suboption.
                            BcmDhcpV6DocsisTimeOffsetOption docsisTimeOffsetOption;
                            if (clVendorOptions.Options().Get(docsisTimeOffsetOption) == false)
                            {
                                gWarningMsg(fMessageLogSettings, "HandleDhcpV6Event")
                                    << "Failed to get Time Offset value!" << endl;
                            }
                            else
                            {
                                timeOffset = docsisTimeOffsetOption.Value();
                            }
                        }
                        else
                        {
                            gWarningMsg(fMessageLogSettings, "HandleDhcpV6Event")
                                << "Failed to get CL Vendor-Specific options!" << endl;

                            fakeTod = true;
                        }
                    }   

                    // sleep 2 seconds to allow the IP stack to do it's DAD thing and actually install 
                    // the address
                    BcmOperatingSystemFactory::ThreadSleep(2000);

                    // Kick off Time Of Day.  This runs asynchronously; I don't
                    // wait for it to complete.
                    StartTod(fakeTod, timeServers, timeOffset);

                    // PR11866 - **************** END CHANGES *****************

                    // Now start TFTP of the config file.
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kCmTftpInitStarted);
                
                    // Update my state.
                    fIpState = kIpTftpInProgress;
#ifdef STUB
                    SetupIPv6ConfigFileTftpParameters();
#endif
                    TftpConfigFile();
                }
            }
            break;

        case BcmDhcpV6ClientIf::kEventAcquisitionFailed:
            {
                // Alert the media...
                gErrorMsg(fMessageLogSettings, "HandleDhcpV6Event")
                    << "DHCPv6 failed to complete!" << endl;

                fIpV6State = kFailedDhcpV6;

                // DHCPv6 failed.  Take appropriate action based on APM mode. 
                IpV6AddressAcquisitionFailed();
            }
            break;

        // PR13041
        case BcmDhcpV6ClientIf::kEventAddressLost:
            {
                // We failed to renew our lease.
                fIpV6State = kFailedDhcpV6;
                
                // Alert the media...
                gErrorMsg(fMessageLogSettings, "HandleDhcpV6Event")
                    << "DHCPv6 renew failed!" << endl;

                // log event "kD102_0 "DHCP REBIND sent - No response for v6"
                // PTMOD - LOG EVENT CmLogEvent(kD102_0, "v6");
    
                // If a CM is configured to use DPM and the CM fails to renew one of its IP
                // addresses, the CM MUST note the event in the Local Event Log. Failure to
                // renew an IP address when the other IP address is active is not a
                // critical event.  In this case, after noting the failure in the Local
                // Event Log, the CM MUST continue to operate with the remaining IP
                // address. 
                bool publishDhcpRenewFailed = true;
                #if (BCM_IPV6_SUPPORT )
                if( (fDualStackOperationEnabled == true) && (fIpState != kIpFailedDhcp) )
                {
                    gErrorMsg(fMessageLogSettings, "HandleDhcpV6Event")
                        << "DPM is enabled and IPv4 is operational.  Not reinitializing MAC!" << endl;
                     publishDhcpRenewFailed = false;

                     // log event kD107_0 "Primary lease failed, IPv4 fallback initiated"
                     // PTMOD - LOG EVENT CmLogEvent(kD107_0);
                }
                // According to Open Cable host requirement, restart IPV6 address acquire.
                printf(" According to Open Cable host requirement, restart IPV6 address acquire\n");
                if (pfMessageQueue->Send( kStartIpV6Init ) == false)
                {
                    gErrorMsg(fMessageLogSettings, "HandleDhcpV6Event")
                        << "DHCP V6 restart failed.  Not reinitializing MAC!" << endl;
                }

                #endif
                if( publishDhcpRenewFailed == true ) 
                {
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewFailed);
                    PublishEvent(kEstbDhcpRenewFailed);
                }
            }
            break;

        case BcmDhcpV6ClientIf::kEventAddressRenewed:
            {
                // tell anyone who cares that the renewal completed successfully.
                //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewSuccess);
                PublishEvent(kEstbDhcpRenewSuccess);

                // It is possible during the DHCPv6 renew operation that the CM will receive 
                // updated fields in the DHCP Reply message.  If the CM IPv6 Management Address 
                // (IA_NA option) is different in the DHCP Reply than the current value used by 
                // the CM, the CM MUST follow one of the following two:
                //   Change to using the new IPv6 Management Address without reinitializing the CM, or
                //   Reinitialize MAC
                // 
                BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
                BcmDhcpV6IaAddress iaAddress;
                pIpHalIf->DhcpV6ClientIf()->IA().Get(0, iaAddress);

                // check for change in address
                if( fIpV6Address != iaAddress.IpV6Address() ) 
                {
                    gErrorMsg(fMessageLogSettings, "HandleDhcpV6Event")
                        << "Renew address " << iaAddress.IpV6Address() << " is different than " 
                        << fIpV6Address << ".  Treating like failed renew." << endl;
        
                    //PublishEvent(BcmCmDocsisStatusEventCodes::kEstbDhcpRenewFailed);
                    PublishEvent(kEstbDhcpRenewFailed);

                    // log event "DHCP Renew - lease parameters <P1> modified" 
                    // PTMOD - LOG EVENT CmLogEvent( kD106_0, "IA Address");
                }
#if 0
// PTMOD - TFTP-TOD refs. commented out
                // Check to see if the UTC offset value changed; if yes, then
                // we need to redo ToD so that the system time is changed to
                // reflect this.
                int32 newTimeOffset = 0, oldTimeOffset = 0;
                oldTimeOffset = pfTodThread->GetUtcOffset();

                BcmDhcpV6CableLabsVendorSpecificInfoOption clVendorOptions;

                // Pull out the cablelabs vendor specific options, which contain time offset
                if( pIpHalIf->DhcpV6ClientIf()->Options().Get(clVendorOptions) == true )
                {
                    bool timeOptionsChanged = false;

                    // If the Time Protocol Servers option in the DHCP Reply is different than 
                    // the current value used by the CM, the CM MUST update the time server 
                    // address with the new value. This will cause the CM to use the new 
                    // address(es) on future ToD requests (if any).
                    BcmDhcpV6DocsisTimeProtocolServersOption docsisTimeProtocolServersOption;
                    if (clVendorOptions.Options().Get( docsisTimeProtocolServersOption ) == true)
                    {
                        BcmIpAddressList currentTodServers = pfTodThread->GetTodServerIpAddress();

                        if( docsisTimeProtocolServersOption.NumberOfIpV6Addresses() != currentTodServers.Size() )
                        {
                            timeOptionsChanged = true;

                            // log event "DHCP Renew - lease parameters <P1> modified" 
                            // PTMOD - LOG EVENT CmLogEvent( kD106_0, "Time Protocol Servers");
                        }
                        else
                        {
                            // same number of servers in old and new.   Now compare them. 
                            unsigned int i;
                            for( i = 0; i < currentTodServers.Size(); i++ ) 
                            {
                                BcmIpV6Address oldAddress = currentTodServers[i].GetIpV6Address();

                                // check for exactly one instance of each of the old servers in the new server list
                                if( docsisTimeProtocolServersOption.Count( oldAddress ) != 1 ) 
                                {
                                    timeOptionsChanged = true;

                                    gLogMessageRaw
                                        << "The Time Server IPs changed!  Re-doing Time Of Day to update system clock." << endl;

                                    // log event "DHCP Renew - lease parameters <P1> modified" 
                                    // PTMOD - LOG EVENT CmLogEvent( kD106_0, "Time Protocol Servers");
                                }
                            }
                        }

                        // update time server if there was a change
                        if( timeOptionsChanged ) 
                        {
                            BcmIpAddressList timeServers;

                            // add the IP addresses(s) in the option into the list
                            unsigned int i;
                            for( i=0; i < docsisTimeProtocolServersOption.NumberOfIpV6Addresses(); i++ ) 
                            {
                                timeServers.Add( docsisTimeProtocolServersOption.Get( i ) );
                            }
                            pfTodThread->SetTodServerIpAddress(timeServers);
                        }
                    }

                    BcmDhcpV6DocsisTimeOffsetOption docsisTimeOffsetOption;

                    if (clVendorOptions.Options().Get(docsisTimeOffsetOption) == false)
                    {
                        gWarningMsg(fMessageLogSettings, "HandleDhcpV6Event")
                            << "Failed to get Time Offset value!" << endl;
                    }
                    else
                    {
                        newTimeOffset = docsisTimeOffsetOption.Value();
    
                        if (oldTimeOffset != newTimeOffset)
                        {
                            // Store the new value.
                            pfTodThread->SetUtcOffset(newTimeOffset);
        
                            // If ToD hasn't been acquired, then all we need to do is
                            // store the new tiem offset value.
                            if (pfTodThread->GetTodState() == BcmTimeOfDayThread::kAcquired)
                            {
                                gLogMessageRaw
                                    << "The UTC Time Offset value changed!  Old value = "
                                    << oldTimeOffset << "; new value = " << newTimeOffset
                                    << "\nRe-doing Time Of Day to update system clock." << endl;

                                // log event "DHCP Renew - lease parameters <P1> modified" 
                                // PTMOD - LOG EVENT CmLogEvent( kD106_0, "UTC Time Offset");

                                timeOptionsChanged = true;
                            }
                        }
                    }

                    if( timeOptionsChanged == true ) 
                    {
                        // Rather than calling the StartTod() helper (which
                        // does too much work, makes too many changes, and
                        // publishes too many events), we will just reconfigure
                        // the thread, stop it, and start it.
                        pfTodThread->Stop();

                        // DOCSIS requires us to use UDP.
                        pfTodThread->GetTimeOfDay(BcmTimeOfDayThread::kProtocolUdp);
                    }
                }
#endif // END of PTMOD - TFTP-TOD refs. commented out
            }
        break;

    case BcmDhcpV6ClientIf::kEventNoDhcpAdvertiseReceived:
        {
            #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
            // log event kEE_D01202 "DHCP failed - DHCP Solicit sent, No DHCP Advertise received"
            DhcpV6LogEvent(kEE_D01202);
            #endif
        }
        break;

    case BcmDhcpV6ClientIf::kEventNoDhcpReplyReceived:
        {
            #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
            // log event kEE_D01203 "DHCP failed - DHCP Request sent, No DHCP REPLY received"
            DhcpV6LogEvent(kEE_D01203);
            #endif
        }
        break;

    case BcmDhcpV6ClientIf::kEventDuplicateAddressDetected:
        {
            #if (BCM_ESTB_CLIENT_SNMP_INTERFACE_INCLUDED)
            // log event kEE_D01302 "DHCP lease address failed DAD"
            DhcpV6LogEvent(kEE_D01302);
            #endif
        }
        break;

    case BcmDhcpV6ClientIf::kEventMissingRequiredOptions:
        {
            // log event kD03_1 "DHCP FAILED - Response doesn't contain ALL the valid options"
            // PTMOD - LOG EVENT CmLogEvent(kD03_1);
        }
        break;

    case BcmDhcpV6ClientIf::kEventNoResponseToRenew:
        {
            // log event kD101_0 "DHCP RENEW sent - No response for v6"
            // PTMOD - LOG EVENT CmLogEvent(kD101_0, "v6");
        }

        break;    

    case BcmDhcpV6ClientIf::kEventReconfigureReceived:
        {
            // log event kD105_0 "DHCP Reconfigure received
            // PTMOD - LOG EVENT CmLogEvent(kD105_0);
        }
        break;

    default:
        break;
    }
}
#endif


// Kicks off the Time Of Day thread.  The ToD parameters are pulled from
// the DHCP settings.
//
// Parameters:  
//
//      fakeTod - indicates we should fake out TOD
//      todServerIp - either an IPv4 or IPv6 address of the time server
//      timeOffset - time offset of TOD obtained from DHCP
//
// Returns:  Nothing.
//
void BcmEstbIpThread::StartTod( bool fakeTod, BcmIpAddressList & timeServers, int32 timeOffset )
{
#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    CallTrace("BcmEstbIpThread", "StartTod");

    // The DOCSIS spec requires that there be 3 retries per period, and that the
    // periods are at least 5 minutes apart.  It is silent as to how much time
    // there should be between each retry, so I have selected 2 seconds as a
    // reasonable value.
    //
    // In other words, we will send a ToD request every 2 seconds, until the
    // number of retries has been exceeded, then we'll wait the period time
    // before sending more ToD requests.
    #define kRetryPeriod (5 * kOneMinute)
    #define kNumberOfRetries 3
    #define kTimeBetweenRetries (2 * kOneSecond)
    
    bool result;

    // Reset this state.
    fTimeOfDayCompleted = false;

    // Alert the media...
    //PublishEvent(BcmCmDocsisStatusEventCodes::kCmTodInitStarted);

    printf("Starting Time Of Day...\n");

    if (!fakeTod)
    {
        // PR 8120: initialize timeOffset to 0 in case UTC offset option is not
        // present in DHCP lease settings.  Otherwise we will wind up with
        // an uninitialized value which may be something bizarre.
        // Set the ToD parameters based on the DHCP results.
        pfTodThread->SetTodServerIpAddress(timeServers);
        pfTodThread->SetUtcOffset(timeOffset);

        // This is what the ToD thread uses to tell me when it is done.
        // PR2182 - event was changed.
        pfTodThread->SetNotificationEvent(pfTodHelperDoneEvent);

        #if (!BCM_IPV6_SUPPORT)
        // Docsis 2.0 requires....
        // "The specific timeout for Time of Day Requests is implementation dependent.
        // However, for each server defined the CM MUST NOT exceed more than 3 Time of
        // Day requests in any 5 minute period. At minimum, the CM MUST issue at least
        // 1 Time of Day request per 5 minute period for each server specified until
        // local time is established."
        // 
        pfTodThread->SetRetryParameters( 1000 * 300,  // 300 seconds per period
                                         2,           // 1 try, and 2 retries per period
                                         1000 * 1,    // 1 second between retries with exponential backoff
                                         1            // 1 retry per server per interval
                                       );
        #endif

        // DOCSIS requires us to use UDP.
        result = pfTodThread->GetTimeOfDay(BcmTimeOfDayThread::kProtocolUdp);

        // Note that failure of this isn't critical to the operation of the rest of
        // IP initialization.
        if (result == false)
        {
            // Alert the media...
            gErrorMsg(fMessageLogSettings, "StartTod") 
                << "Failed to start ToD!" << endl;

            //PublishEvent(BcmCmDocsisStatusEventCodes::kCmTodInitFailed);

            // Note: OSS spec version I04 removes event kD04_0.  Need to make a
            // proprietary event for this, because kD04_1 doesn't really fit well.
            // CmLogEvent(kD04_0);
        }
        else
        {
            // Since Tod is our first unicast exchange with the CMTS/router/ToD
            // server after DHCP, an ARP is required before transmission of the
            // ToD frame. This delay will allow for the ARP to complete.  The
            // Cisco CMTS seems to be one that  has a longer RTT for ARP
            // handling.
            //
            // PR2824 - Looks like MOT beats Cisco; give more time for this to
            // complete.
            pfOperatingSystem->Sleep(300 * kOneMillisecond);
        }
    }
    else
    {
        // PR1788 - ECN RFI-N-01100 says set ToD to this value if not present.
        gLogMessageRaw << "Forcing Time Of Day to accept Jan 1, 1970..." << endl;

        // Force the Time of day to revert to Jan 1, 1970.  The hard coded value
        // represents Jan 1, 1970, 0h00s, per RFC868.  The UTC time offset is
        // 0.
        SetTimeOfDay(2208988800UL, 0);

        // Simulate the completion of ToD so that it will do all the stuff
        // associated with setting ToD.
        // PR2182 - event was changed.
        pfTodHelperDoneEvent->Send();
    }
#endif //#if (BFC_INCLUDE_DHCPV4_SUPPORT)
}


// Handles the completion of ToD, checking the status, and doing whatever
// cleanup is necessary.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::HandleTodComplete(void)
{
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    CallTrace("BcmEstbIpThread", "HandleTodComplete");

    // When we started the ToD thread, we told it to run forever, so if it is
    // telling us that it finished, this means that it actually acquired time
    // of day.  Currently, there is no failure reporting mechanism.
    //
    // Update my state.
    fTimeOfDayCompleted = true;

    // Alert the media...
    printf("Time Of Day completed...\n");

    //PublishEvent(BcmCmDocsisStatusEventCodes::kCmTodInitOk);
#endif // #if (BFC_INCLUDE_DHCPV4_SUPPORT)
}


// Stops the basic IP state machine (not the s/w upgrade state machine),
// and resets everything to an initial state.
//
// Parameters:
//      releaseSemaphore - true if the semaphore should be released (meaning
//                         that this is called because the client thread is
//                         blocked); false if this is being called internally
//                         and the semaphore should not be released.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::StopAndResetSync(bool releaseSemaphore)
{
    CallTrace("BcmEstbIpThread", "StopAndResetSync");

    if (releaseSemaphore)
    {
        gAlwaysMsgNoFields(fMessageLogSettings) 
            << "Stopping DHCP (client requested)..." << endl;
    }

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // TFTP is done synchronously in my thread, so there's nothing to stop,
    // except the timer.
    pfTftpTimer->Stop();
#endif

    // stop router solicitations
    pfRouterSolicitationTimer->Stop();

#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    // Stop ToD.
    pfTodThread->Stop();
    pfTodThread->SetNotificationEvent(NULL);
    fTimeOfDayCompleted = false;
#endif

#if (BFC_INCLUDE_DHCPV4_SUPPORT)
    // Stop DHCP if necessary.
    BcmDhcpClientLease *pLease;

    pLease = GetLease();

    if (pLease->State() != BcmDhcpClientLease::kStateInit)
    {
        BcmBaseIpHalIf *pIpHalIf;
    
        pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

        pIpHalIf->DhcpClientIf()->ReleaseLease(pLease->ClientId());
    }
#endif // #if (BFC_INCLUDE_DHCPV4_SUPPORT)
    {
        BcmBaseIpHalIf *pIpHalIf;
    
        pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);

        // Remove the link-local address
        BcmIpV6Address linkLocalAddress;
        pIpHalIf->GetIpv6LinkLocalAddress( linkLocalAddress );
        pIpHalIf->RemoveIpv6Address( linkLocalAddress );
    }

    // Clear any possible pending events, which are now irrelevant.
    // PR2182 - events were split out.
    pfEventSet->Occurred(pfTodHelperDoneEvent);
    pfEventSet->Occurred(*pfTftpTimer);
    pfEventSet->Occurred(*pfRouterSolicitationTimer);

    // Set my state.
    fIpState = kIpNotStarted;

    // reset the neighbor discovery thread to clear out router entries and neighbor cache
    // entries.  
    //#if (BCM_IPV6_SUPPORT)
#if (BCM_IPV6_NEIGHBOR_DISCOVERY_SUPPORT)
    BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
    BcmNeighborDiscoveryThread * pNeighborDiscoveryThread = pIpHalIf->NeighborDiscoveryThread();
    pNeighborDiscoveryThread->Reset();

    // reset IPv6 state
    fIpV6State = kIpV6NotStarted;
    fRouterSolicitationCount = 0;
#endif
    // PR12294 - release any leased IPv6 addresses here.
//  BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
    pIpHalIf->DhcpV6ClientIf()->ReleaseAddresses();

    fDnsServerList.Reset();
    if( pfDomainList )
        pfDomainList->Empty();

    //#endif

    if (releaseSemaphore)
    {
        // This operation was made synchronous - the caller was blocked waiting
        // for me to release the semaphore.
        pfWaitForAbortSemaphore->Release();
    }
}

// Does the TFTP of the config file.  The parameters are pulled from the
// DHCP settings.
//
// Parameters:  none
//              
// Returns:  Nothing.
//
void BcmEstbIpThread::TftpConfigFile()
{

}

// Sets up the config file name, TFTP servers list, and backoff/retry settings.  
// This is based on the Docsis version (Docsis 2.0 or Docsis3.0), the 
// DHCPv4 or DHCPv6 lease settings, and possibly the the non-vol settings
// for test modes like "force config file".
//
// Parameters:  none
//
// Returns:  none
//
bool BcmEstbIpThread::SetupIPv4ConfigFileTftpParameters()
{
    bool result = true;
#if (NOT_INCLUDED_IN_ESTB_DHCP) 
    BcmIpAddressList tftpServerIpAddressList;
    BcmCmDocsisNonVolSettings *pSettings = BcmCmDocsisNonVolSettings::GetSingletonInstance();

    BcmDhcpClientLease *pLease = GetLease();

    // clear the serverIP/backoff list
    fTftpServerIpAddressBackoffPairList.clear();

    if (pSettings->ForceConfigFileForDhcpEnable() == true)
    {
        // DHCP at this point but force us to use the config file specified in non-vol.
        gAlwaysMsgNoFields(fMessageLogSettings)
            << "FORCE the use of non-vol specified config file!..." << endl;

        tftpServerIpAddressList.Add( pSettings->TftpServer() );

        // use non-vol config file name
        fConfigFileFilename.Empty();
        fConfigFileFilename.AddToEnd(pSettings->ConfigFilename());
    }
    else
    {
        if( pLease->ServerLeaseSettings().TftpServerIpAddressList().Size() != 0 ) 
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Using option 125 for TFTP server." << endl;
    
            // Add all TFTP server IPv6 addresses into the list
            unsigned int i;
            for( i=0; i <  pLease->ServerLeaseSettings().TftpServerIpAddressList().Size(); i++ )
            {
                BcmIpAddress tftpServerIp( pLease->ServerLeaseSettings().TftpServerIpAddressList()[i] );   
    
                tftpServerIpAddressList.Add( tftpServerIp );
            }
        }
        else
        {
            gAlwaysMsgNoFields(fMessageLogSettings) 
                << "Using siaddr or option 66 for TFTP server." << endl;

            tftpServerIpAddressList.Add(  pLease->ServerLeaseSettings().ServerIpAddress() );
        }

        // get the config file name from the lease settings
        fConfigFileFilename.Empty();
        fConfigFileFilename.AddToEnd(pLease->ServerLeaseSettings().FileName());
    }

    // Now setup the TFTP server IP / backoff timer pair list.
    // This is a list of IP addresses paired with the time you should wait before retrying 
    // on the next element on the list. 
    // 
    // The CM MUST use an adaptive timeout between retries based on a binary
    // exponential backoff with an initial backoff value of TFTP Backoff Start and
    // final backoff value of TFTP Backoff End as defined in Annex B.  
    // If the CM is unable to establish a connection on the first attempt, 
    // the CM MUST retry requesting up to TFTP Request Retries.
    unsigned int tftpBackoffStartMS;
    unsigned int tftpBackoffEndMS;
    unsigned int tftpBackoffExponent;
    unsigned int tftpRequestRetries;
    unsigned int serverIndex = 0;

    #if( BFC_MULPI_SUPPORT )
    tftpBackoffStartMS  = kDocsis30_TftpBackoffStartMS;
    tftpBackoffEndMS    = kDocsis30_TftpBackoffEndMS;
    tftpBackoffExponent = kDocsis30_TftpBackoffExponent;
    tftpRequestRetries  = kDocsis30_TftpRequestRetries;
    #else
    tftpBackoffStartMS  = kDocsis20_TftpBackoffStartMS;
    tftpBackoffEndMS    = kDocsis20_TftpBackoffEndMS;
    tftpBackoffExponent = kDocsis20_TftpBackoffExponent;
    tftpRequestRetries  = kDocsis20_TftpRequestRetries;
    #endif

    for( serverIndex = 0; serverIndex < tftpServerIpAddressList.Size(); serverIndex++) 
    {
        unsigned int backoffMS = tftpBackoffStartMS;
        while( tftpRequestRetries-- )
        {
            tTftpServerIpAddressBackoffPair addressBackoffPair( tftpServerIpAddressList[serverIndex], backoffMS );
            if( backoffMS < tftpBackoffEndMS ) 
                backoffMS *= tftpBackoffExponent;
            fTftpServerIpAddressBackoffPairList.push_back( addressBackoffPair );
        }
    }

    printf("Starting Tftp of configuration file...\n");
#endif // #if (BFC_INCLUDE_DHCPV4_SUPPORT)
    return result;
}

// Returns the config file name
//
// Parameters:  None.
//
// Returns:  pointer to config file name
//
const char * BcmEstbIpThread::GetConfigFileName()
{
    return (char *)fConfigFileFilename.AccessBuffer();
}

// Reset IP state to it's initial value. This would normally be
// kIpSuccess, for applications other than docsis it may be something
// else.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::RestoreApplicationSpecificIpState(void)
{
    fIpState = kIpSuccess;
}
#if 0 // PTASKIRAN _ ESTB SNMP AGENT SUPPORT(SNMP_SUPPORT)
// Reset docsDevSwAdminStatus to it's initial value. This would normally be
// CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE, for applications
// other than docsis it may be something else.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::RestoreApplicationSpecificSwAdminStatus(CmSnmpNonVolSettings *pSnmpSettings)
{
    pSnmpSettings->DocsDevSwAdminStatus(CONSTVAL_DOCSDEVSWADMINSTATUS_ALLOWPROVISIONINGUPGRADE);
}
#endif
// Do application specific program store driver configuration.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::ApplicationSpecificProgramStoreDriverSettings(void)
{
}

// Do application specific tftp configuration.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbIpThread::ApplicationSpecificTftpInitialization(void)
{
}




// HandleMaxDelayTimer: 
//    Called when Max delay timer expires, this timer is started after SW download completes
//    Timer is only started if emta image and emtaMaxDelaytime MIB has non-zero value.
//
// Parameters:  None
//
// Returns:     None
//
void BcmEstbIpThread::HandleMaxDelayTimer( void )
{

}

// Handle Post call delay timer expiry: 
//    Called when Post Call delay timer expires, this timer is started after a phone call terminates.
//    timer is only started if emta image and emtaPostCallCompletiontime MIB has non-zero value
//
// Parameters:  None
//
// Returns:     None
//
void BcmEstbIpThread::HandlePostCallDelayTimer( void )
{
   // if MaxDelay Timer is Active, Cancel it because phone call has ended and the Post call 
   // reset delay timer has expired
   if( fMaxResetDelayTimerActive )
   {
      pfSWDnldResetMaxDelayTimer->Stop();
      fMaxResetDelayTimerActive = false;
   }
   fPostCallResetDelayTimerActive = false;
   
   // we have now satisfied all SW Download delay timers, its OK to send Docsis ctrl thread
   // the event to proceed with the reset.
   if( fSWDownloadOKPending )
   {
       fSWDownloadOKPending = false;
       //PublishEvent(BcmCmDocsisStatusEventCodes::kCmSwDownloadOk);
   }

}

// UpdateSWDownloadResetDelayParams:
// This is called only by EMTA IF to tell us of active call and SW download delay timers.
//
// Parameters:  impact: significant, none
//              uint32 MaxResetDelay
//              uint32 PostCallResetDelay
//
// Returns:     None
//
void BcmEstbIpThread::UpdateSWDownloadResetDelayParams(int impact, uint32 MaxResetDelay, uint32 PostCallResetDelay )
{
   // Store off timer values so IP thread knows emta exist and Download reset delay is in effect.
   fMaxResetDelay = MaxResetDelay;
   fPostCallResetDelay = PostCallResetDelay;
   
   // if phone call is being activated
   if( impact == significant )
   {
      fESafeImpact = significant;
      
      // if Post Call Reset Delay Timer is Active, Cancel it because another phone call has 
      // started before Post Call delay timer expires.
      if( fPostCallResetDelayTimerActive )
      {
         // stop the post call delay timer
         // If IP thread is not busy downloading, send event to stop the Post Call timer
         if( fIpState != kIpDoingSwUpdate )
         {
            // start the Post call completion timer
            if (pfMessageQueue->Send(kStopPostCallTimer) == false)
            {
                 gErrorMsg(fMessageLogSettings, "UpdateSWDownloadResetDelayParams")
                     << "Failed to send message to my thread!" << endl;
            }
         }
         else
         {
            // Because IP thread is busy downloding SW, it will not process kStopPostCallTimer
            // we reset PostCallDelayPendingTime so SWDownload complete will not start the timer.
            fPostCallDelayPendingTime = 0;
         }
      }
      fPostCallDelayPendingTime = 0;
   }
   else if( (impact == none) && (fESafeImpact == significant) ) // if phone call being terminated
   {
      fESafeImpact = none;

      if( (fMaxResetDelay != 0) && (fPostCallResetDelay != 0) )
      {
         if( fIpState != kIpDoingSwUpdate )
         {
            // start the Post call completion timer
            if (pfMessageQueue->Send(kStartPostCallTimer) == false)
            {
                 gErrorMsg(fMessageLogSettings, "UpdateSWDownloadResetDelayParams")
                     << "Failed to send message to my thread!" << endl;
            }
            fPostCallDelayPendingTime = 0;
         }
         else
         {
            // If the IP thread is busy doing SW download, kStartPostCallTimer event will
            // not get processed right away, so we have to mark current time, so 
            // we can start the timer at the end of the SW download.
            fPostCallDelayPendingTime = SystemTimeMS();
         }
      }
      else if( fMaxResetDelay != 0 )
      {
         // if SW Download OK event is pending send it so we reset.
         // No need to stop the max delay timer as we are resetting the board anyways.
         if( fSWDownloadOKPending )
         {
            fSWDownloadOKPending = false;
            //PublishEvent(BcmCmDocsisStatusEventCodes::kCmSwDownloadOk);
         }
      }
   }
}



void BcmEstbIpThread::AddCableLabsVendorSpecificInfoOption(BcmDhcpV6OptionsList &clientOptions)
{
        // Include Cable Labs Vendor Specific Information Options
        {
            BcmDhcpV6CableLabsVendorSpecificInfoOption cableLabsVendorSpecificInfoOption;
#ifdef STUB// (SNMP_SUPPORT)
            BcmString sysDescr = CmSnmpAgent::Singleton().sysDescr();
#else
            //"BFC cablemodem reference design <<HW_REV: V1.0; VENDOR: Broadcom; BOOTR: 0.1.1e; SW_REV: 4.4.1; MODEL: BCM93255eSTB>>";
            BcmString sysDescr = kDefault_EstbSysDescr;
#endif
            
            // Include the CL_OPTION_ORO.
            /*{
                BcmDhcpV6DocsisOptionRequestOption docsisOptionRequestOption;

                // Append option codes that I want to get back.
                docsisOptionRequestOption.Add(kDhcpV6OptionTftpServerAddresses);
                docsisOptionRequestOption.Add(kDhcpV6OptionConfigFileName);
                docsisOptionRequestOption.Add(kDhcpV6OptionSyslogServerAddresses);
                docsisOptionRequestOption.Add(kDhcpV6OptionRfc868TodServers);
                docsisOptionRequestOption.Add(kDhcpV6OptionTimeOffset);

                #if (EMTA_SUPPORT)
                docsisOptionRequestOption.Add(kDhcpV6CableLabsClientConfiguration);
                #endif

                cableLabsVendorSpecificInfoOption.OptionsModify().Add(docsisOptionRequestOption);
            }*/

            // Include the CL_OPTION_DEVICE_TYPE
            {
                BcmDhcpV6DocsisDeviceTypeOption deviceTypeOption;
    
                BcmOctetBuffer buffer(16);
    
                // Finish with the string "docsis2.0" or "docsis3.0".
                // 
                buffer.AddToEnd("ESTB", false);
                deviceTypeOption.Value( buffer );
    
                // Add to parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( deviceTypeOption );
            }

            // Include the CL_OPTION_EMBEDDED_COMPONENTS_LIST
            {
                BcmDhcpV6DocsisEmbeddedComponentsListOption embeddedComponentsOption;
                BcmOctetBuffer embeddedComponents(64);

                embeddedComponents.AddToEnd("ECM", false);

                #if (BCM_DSG_SUPPORT)
                AppendApplicationSpecificEsafe(embeddedComponents);
                #endif

                // Add the string to the option
                embeddedComponentsOption.Value( embeddedComponents );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( embeddedComponentsOption );
            }

            // Include the CL_OPTION_DEVICE_SERIAL_NUMBER
            {
                BcmDhcpV6DocsisDeviceSerialNumberOption deviceSerialNumberOption;
                BcmOctetBuffer buffer;
#ifdef STUB // (SNMP_SUPPORT)
                CmSnmpNonVolSettings *pSettings = CmSnmpNonVolSettings::GetSingletonInstance();
                buffer.AddToEnd(pSettings->DocsDevSerialNumber(), false);
#else
                buffer.AddToEnd(kDefault_DeviceSerialNumber, false);
#endif
                // Add the serial number to the option data
                deviceSerialNumberOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( deviceSerialNumberOption );
            }

            // Include the CL_OPTION_HARDWARE_VERSION_NUMBER
            {
                BcmDhcpV6DocsisHardwareVersionNumberOption hardwareVersionNumberOption;
                BcmOctetBuffer buffer;

                BcmString::size_type startOffset, endOffset;
    
                // Find the keyword we are looking for, then skip ahead to
                // the interesting data.
                startOffset = sysDescr.find("HW_REV: ");
                startOffset += 8;
    
                // Find the semicolon that signals the end of the value.
                endOffset = sysDescr.find_first_of(';', startOffset);
    
                buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);

                hardwareVersionNumberOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( hardwareVersionNumberOption );
            }

            // Include the CL_OPTION_SOFTWARE_VERSION_NUMBER
            {
                BcmDhcpV6DocsisSoftwareVersionNumberOption softwareVersionNumberOption;
                BcmOctetBuffer buffer;

                BcmString::size_type startOffset, endOffset;
    
                // Find the keyword we are looking for, then skip ahead to
                // the interesting data.
                startOffset = sysDescr.find("SW_REV: ");
                startOffset += 8;
    
                // Find the semicolon that signals the end of the value.
                endOffset = sysDescr.find_first_of(';', startOffset);
    
                buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);

                softwareVersionNumberOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( softwareVersionNumberOption );
            }

            // Include the CL_OPTION_BOOT_ROM_VERSION
            {
                BcmDhcpV6DocsisBootRomVersionNumberOption bootRomVersionNumberOption;
                BcmOctetBuffer buffer;

                BcmString::size_type startOffset, endOffset;
    
                // Find the keyword we are looking for, then skip ahead to
                // the interesting data.
                startOffset = sysDescr.find("BOOTR: ");
                startOffset += 7;
    
                // Find the semicolon that signals the end of the value.
                endOffset = sysDescr.find_first_of(';', startOffset);
    
                buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);

                bootRomVersionNumberOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( bootRomVersionNumberOption );
            }

            // Include the CL_OPTION_VENDOR_OUI
            {
                BcmDhcpV6DocsisOuiOption ouiOption;
                BcmOctetBuffer buffer;

                // This constant contains the vendor OUI, in partial MAC
                // address form, but as a string.  I need to skip around
                // the colons in the middle.
                const char *pVendorOui = kCustomerVendorId;
    
                buffer.NetworkAddToEnd(pVendorOui[0]);
                buffer.NetworkAddToEnd(pVendorOui[1]);
                buffer.NetworkAddToEnd(pVendorOui[3]);
                buffer.NetworkAddToEnd(pVendorOui[4]);
                buffer.NetworkAddToEnd(pVendorOui[6]);
                buffer.NetworkAddToEnd(pVendorOui[7]);

                ouiOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( ouiOption );
            }

            // Include the CL_OPTION_MODEL_NUMBER
            {
                BcmDhcpV6DocsisModelNumberOption modelNumberOption;
                BcmOctetBuffer buffer;

                BcmString::size_type startOffset, endOffset;
    
                // Find the keyword we are looking for, then skip ahead to
                // the interesting data.
                startOffset = sysDescr.find("MODEL: ");
                startOffset += 7;
    
                // Find the ">" that signals the end of the value.  This one
                // is different from the others.
                endOffset = sysDescr.find_first_of('>', startOffset);
    
                buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);

                modelNumberOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( modelNumberOption );
            }

            // Include the CL_OPTION_VENDOR_NAME
            {
                BcmDhcpV6DocsisVendorNameOption vendorNameOption;
                BcmOctetBuffer buffer;

                BcmString::size_type startOffset, endOffset;
    
                // Find the keyword we are looking for, then skip ahead to
                // the interesting data.
                startOffset = sysDescr.find("VENDOR: ");
                startOffset += 8;
    
                // Find the semicolon that signals the end of the value.
                endOffset = sysDescr.find_first_of(';', startOffset);
    
                buffer.AddToEnd(sysDescr.substr(startOffset, endOffset - startOffset).c_str(), false);

                vendorNameOption.Value( buffer );

                // Add the option to the parent option.
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( vendorNameOption );
            }

            // Include the DOCSIS Device Identifier option.
            {
                BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(fStackNumber);
                BcmDhcpV6DocsisDeviceIdentifierOption docsisDeviceIdentifierOption;
                docsisDeviceIdentifierOption.Replace(0, pIpHalIf->MacAddress());
                cableLabsVendorSpecificInfoOption.OptionsModify().Add( docsisDeviceIdentifierOption );
            }

            // Add to parent option.
            clientOptions.Add( cableLabsVendorSpecificInfoOption );
        }
}

#if (BFC_INCLUDE_DHCPV6_SUPPORT)
// This is the callout function that is registered with the DHCPv6 Client
// When an advertise/reply packet is received, the eSTB will inspect it.
// If everything checks out then we will return true so the lease can be accepted
//
//
// \param
//      dhcpPacket - the received DHCPv6 packet.
// \param
//      SolicitNumTries - the number of Solicit messages this DHCPv6 Client has sent
//
// \retval
//      true if the advertise/reply is acceptable.
// \retval
//      false if the advertise/reply is not acceptable, and should not be selected or
//          should be NAKed.
//
bool BcmEstbIpThread::CmDhcpV6AdvertiseReplyCallout(const BcmDhcpV6Packet &dhcpPacket, unsigned int solicitNumTries)
{
    bool result = true;

    BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(ipStackNumber);
    if( pIpHalIf == NULL )
    {
        gLogMessageRaw << "CmDhcpV6AdvertiseReplyCallout: pIpHalIf is NULL" << endl;
        return false;
    }

    // check the message type
    if (dhcpPacket.MessageType() == kDhcpV6Advertise)
    {
        //if( solicitNumTries < 3 )
        {
            BcmDhcpV6ReconfigureAcceptOption reconfigureAcceptOption;

            // If OPTION_RECONF_ACCEPT option in absent from the Advertise message, 
            // the eSTB MUST discard the message and wait for other Advertise messages.
            if( dhcpPacket.Options().Get(reconfigureAcceptOption) )
            {                
                gLogMessageRaw << "OPTION_RECONF_ACCEPT present in DHCPv6 Advertise." << endl;
            }
            else
            {
                gLogMessageRaw << "Missing OPTION_RECONF_ACCEPT option in DHCPv6 Advertise. " << endl;
                //result = false;
            }
        }

        {
            // Get the DNS server IPv6 address suboption.
            BcmDhcpV6DnsServersOption dnsServersOption;

            if( dhcpPacket.Options().Get(dnsServersOption) == true )
            {                
                gLogMessageRaw << "OPTION_DNS_SERVERS present in DHCPv6 Advertise." << endl;

                //BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(ipStackNumber);

                unsigned int serverIndex = 0;
                for( serverIndex = 0; serverIndex < dnsServersOption.NumberOfIpV6Addresses(); serverIndex++)
                {
                    if( !pThisEstbIpThread->DnsServerList().IsPresent(dnsServersOption.Get( serverIndex )) )
                    {
                        gLogMessageRaw << dnsServersOption.Get( serverIndex ) << endl;
                        pThisEstbIpThread->DnsServerList().Add( dnsServersOption.Get( serverIndex ) );

                        int retVal = 0;
                        retVal = pIpHalIf->AddIpv6DnsServer( dnsServersOption.Get( serverIndex ));
                        //gLogMessageRaw << "AddIpv6DnsServer - returned " << retVal << endl;
                        //retVal = pIpHalIf->RemoveIpv6DnsServer( dnsServersOption.Get( serverIndex ));
                        //gLogMessageRaw << "RemoveIpv6DnsServer - returned " << retVal << endl;
                    }
                }
            }
            else
            {
                gLogMessageRaw << "Missing OPTION_DNS_SERVERS option in DHCPv6 Advertise." << endl;                
            }
        }

        {
            // Get the DNS server IPv6 address suboption.
            BcmDhcpV6DomainListOption domainListOption;

            if( dhcpPacket.Options().Get(domainListOption) == true )
            {                
                gLogMessageRaw << "OPTION_DOMAIN_LIST present in DHCPv6 Advertise." << endl;

                //BcmBaseIpHalIf *pIpHalIf = BcmIpStackManager::GetSingleton().Find(ipStackNumber);

                unsigned int domainListSize = domainListOption.DataSize();
                BcmOctetBuffer domainList = domainListOption.Value();
                unsigned int domainLength;
                unsigned int offset = 0;

                BcmOctetBuffer domainName(0);

                while( domainListSize > offset )
                {
                    domainLength = strlen((const char*)(domainList.AccessBuffer() + offset));
                    domainName.Overlay( domainList, offset, domainLength );

                    //gLogMessageRaw << domainList << domainName << domainLength << endl;
                    if(!pThisEstbIpThread->DomainListIsPresent(domainName))
                    {
                        pThisEstbIpThread->DomainListAdd(domainName);

                        int retVal = 0;
                        retVal = pIpHalIf->AddIpv6DomainName( (const char*) domainName.AccessBuffer() );
                        //gLogMessageRaw << "AddIpv6DomainName - returned " << retVal << endl;
                        //retVal = pIpHalIf->RemoveIpv6DomainName( (const char*) domainName.AccessBuffer() );
                        //gLogMessageRaw << "RemoveIpv6DomainName - returned " << retVal << endl;
                    }
                    offset += domainLength + 1;
                }

                //BcmOctetBuffer testDomainName;
                //testDomainName.NetworkAddToEnd(".example.com", false);
                //pThisEstbIpThread->DomainListRemove(testDomainName);
            }
            else
            {
                gLogMessageRaw << "Missing OPTION_DNS_SERVERS option in DHCPv6 Advertise." << endl;                
            }
        }
    }
    else if(dhcpPacket.MessageType() == kDhcpV6Reply)
    {
        //if( solicitNumTries < 3 )
        {
            BcmDhcpV6ReconfigureAcceptOption reconfigureAcceptOption;

            // Check if the Reconfigure Accept Option exists in this message.
            if( dhcpPacket.Options().Get(reconfigureAcceptOption) )
            {                
                gLogMessageRaw << "OPTION_RECONF_ACCEPT present in DHCPv6 Reply." << endl;
            }
            else
            {
                gLogMessageRaw << "Missing OPTION_RECONF_ACCEPT option in DHCPv6 Reply." << endl;
                //result = false;
            }
        }
    }

    return result;
}

BcmIpAddressList & BcmEstbIpThread::DnsServerList(void)
{
    return fDnsServerList;
}
 
void BcmEstbIpThread::This( BcmEstbIpThread* pThis)
{
    pThisEstbIpThread = pThis;
}

bool BcmEstbIpThread::DomainListIsPresent(BcmOctetBuffer& newDomainName)
{
    unsigned int domainListSize = pfDomainList->BufferSize();
    //BcmOctetBuffer domainList = domainListOption.Value();
    unsigned int domainLength;
    unsigned int offset = 0;

    BcmOctetBuffer domainName(0);

    while( domainListSize > offset )
    {
        domainLength = strlen((const char*)(pfDomainList->AccessBuffer() + offset));
        domainName.Overlay( *pfDomainList, offset, domainLength );

        if( domainName == newDomainName )
        {
            return true;
        }
        //gLogMessageRaw << domainList << domainName << domainLength << endl;

        offset += domainLength + 1;
    }

    return false;
}

bool BcmEstbIpThread::DomainListAdd(BcmOctetBuffer& newDomainName)
{
    bool retVal = pfDomainList->Append( newDomainName, 0, newDomainName.BufferSize(), true);
    if( retVal )
        pfDomainList->NullTerminate();

    //gLogMessageRaw << *pfDomainList << endl;

    return retVal;
}

bool BcmEstbIpThread::DomainListRemove(BcmOctetBuffer& domainName)
{
    unsigned int domainListSize = pfDomainList->BufferSize();
    BcmOctetBuffer tempBuff(domainListSize);
    //BcmOctetBuffer domainList = domainListOption.Value();
    unsigned int domainLength;
    unsigned int offset = 0;
    bool domainFound = false;

    BcmOctetBuffer overlayDomainName(0);

    while( domainListSize > offset )
    {
        domainLength = strlen((const char*)(pfDomainList->AccessBuffer() + offset));
        overlayDomainName.Overlay( *pfDomainList, offset, domainLength );

        if( overlayDomainName != domainName )
        {
            bool result = tempBuff.Append( overlayDomainName, 0, overlayDomainName.BufferSize(), true);
            if( result )
                tempBuff.NullTerminate();

            offset += domainLength + 1;
        }
        else
        {
            domainFound = true;

            // Found the one we want to remove. Skip over it and copy the rest.
            offset += domainLength + 1;
            //bool result =
            tempBuff.Append( *pfDomainList, offset, (domainListSize-offset), true);

            gLogMessageRaw <<"DomainListRemove " << tempBuff << endl;

            break;
        }
        //gLogMessageRaw << domainList << domainName << domainLength << endl;
    }

    *pfDomainList = tempBuff;
    gLogMessageRaw <<"DomainListRemove "  << *pfDomainList
        <<"domainName = " << domainName << endl;


    return domainFound;
}


#endif

#if (BCM_EVD_INTERFACE_INCLUDED)
#if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
void BcmEstbIpThread::SubscribeToEvents(void)
{
    // SUBSCRIBE TO EVENTS
    // Now subscribe to events.
    unsigned int i=0;
    unsigned int eventList[kDefault_NumOfEvents] = kDefault_EventList;

    // create my ACT & hook it up to Event Dispatcher
    // the ACT will relay interesting state changes from the Event Dispatcher.
    pfDhcpV6EventDispatcherACT = new BcmDhcpV6ClientEventDispatcherACT (this);

    BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();

    for( i=0; i<kDefault_NumOfEvents; i++ )
    {
        if(pEvDispatcher)
            pEvDispatcher->SubscribeMsgNote (*(eventList+i), pfDhcpV6EventDispatcherACT);

        cout <<" ESTB DHCPv6 Client: SubscribeMsgNote event " << (*(eventList+i)) << endl;
    }
}
#endif

void BcmEstbIpThread::DhcpV6LogEvent(uint32 Id)
{
    #if(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
    {
        dhcpV6EvdClientLogEvent(Id);
    }
    #else
    {
        fLastEventIdLogged = Id;
        cout << "DHCPv6 Client LOG EVENT ! " << fLastEventIdLogged << endl;

        //uint32 msgParam = kEE_D120;
        BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();
        pEvDispatcher->PublishMsg( kEventLogEvent, (void*) &Id );
        // This should be replaced with EstbLogEvent if EstbSnmpAgent is included in the same process.
    }
    #endif
}

void EstbDhcpV6LogEvent(uint32 Id)
{
    if(pThisEstbIpThread)
    {
        pThisEstbIpThread->DhcpV6LogEvent(Id);
    }
}
#endif

void BcmEstbIpThread::PublishEvent(unsigned int event, uint32 value)
{
    #if (BCM_EVD_INTERFACE_INCLUDED)
    #if !(BCM_EVD_RPCCL_INTERFACE_INCLUDED)
    //uint8 dummy = 0;
    BcmEcmEventDispatcherThread *pEvDispatcher = BcmEcmEventDispatcherThread::GetSingletonInstance();
    cout <<" BcmEstbIpThread: Publishing event= " << event <<" value = " << value << endl;
    pEvDispatcher->PublishMsg( event, (void*) &value );
    #endif
    #endif
}
