/******************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/
//
//****************************************************************************
//
//  Filename:       DsgClientCtlThread.h
//  Author:         Pinar Taskiran
//  Creation Date:  November 22, 2004
//
//****************************************************************************
//  Description:
//
//
//****************************************************************************
#ifndef DSGCLIENTCTLTHREAD_H
#define DSGCLIENTCTLTHREAD_H

//********************** Include Files ***************************************
#if defined(WIN32)
#include <map>
#else
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <map>
#else
#include <multimap.h>
#endif
#endif

#include "Thread.h"

#include "IpAddress.h"

#include "dsg_api.h"

#include "EnetPkt.h"
#include "zone.h"

#include "DcdMsg.h"
#include "DsgEstbTunnelSettingsList.h"
#include "DsgConfigSettings.h"
#include "DsgClientManager.h"
#include "DsgAppNameTable.h"

#if (CABLECARD_SUPPORT)
#include "DsgCableCardClient.h"
#endif

#include "DsgBroadcastTunnelPktReassembler.h"
//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEvent;
class BcmCountingSemaphore;
class BcmSocket;
class BcmMacAddress;
class BcmEstbRemoteIfHelperThread;
class BcmDsgClientId;
class BcmDsgClient;
class BcmDsgClientManager;
class BcmDsgClientRegistrarThread;
class BcmDsgCommandTable;
class BcmDSGAppNameTable;
#if (BCM_DSG_SINGLE_PROCESS_INTERFACE)
class BcmEstbNonVolSettings;
#else
class BcmDsgccNonVolSettings;
#endif

#if defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
class BcmEstbRemoteIfDataHandlerThread;
#endif

//********************** Class Declaration ***********************************


class BcmDsgClientCtlThread : public BcmThread
{
public:

    // Default Constructor.  Starts the thread and waits for it to initialze.
    // When the constructor returns, you can successfully manipulate the thread.
    // Note that it is the most-derived class who actually spawns the thread and
    // waits for it.
    //
    // For now, the thread name is hard-wired to be "Time Of Day Thread", and
    // it will not start suspended.  These may be changed if we find that it is
    // desireable.
    //
    // You can set the thread priority as desired (defaults to normal) so that
    // the thread consumes the appropriate amount of processor time.
    //
    // Parameters:
    //      initialPriority - the priority that you want the thread to have.
    //
    // Returns:  N/A
    //
	BcmDsgClientCtlThread(BcmOperatingSystem::ThreadPriority initialPriority);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Note that the event that was passed to us via SetNotificationEvent() will
    // not be deleted!  It is the owner's responsibility to do so!
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgClientCtlThread();

		// These are the "DSG-CC initialization state values"
    typedef enum
    {
        kInitial				= 0,
        kHuntingForDsgChannel	= 1,

		// These apply to Advanced mode only.
        kReassemblingDcd		= 2,
        kValidatingDcd			= 3,
		kOperational			= 4

    } DsgCcStatusValue;

	//
    //
    // IsInitialState() - returns true if all internal state vars are in
    //      their initial condition.
    //
    bool IsInitialState() const;

	// These functions must be provided by the eSTB
	// eCM expects these functions to be present
	void DsgScanSuccess( void );
	void DsgFullScanFail( void );
	void DsgRxDCD( int len, uint8 * dcd );
	void DsgUpstreamChange( bool us_state );
	void DsgUcid( uint8 uc_id  );
	void DsgTimerEvent( uint8 tm_event );
	bool DsgTunnelPkt( ETHERNET_PACKET *pkt );
	void DsgDccInitType( uint8 dccInitType );
	void DsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations );
	void DsgEcmMacReinitEvent(void);

	static BcmDsgClientCtlThread *GetSingletonInstance(void);
	BcmDsgClientManager &DsgClientManager( void)  {return *pfDsgClientManager;}

	void Start();
	const uint32 Mode(void);
	const bool &TwoWayMode(void) const {return fEcmTwoWayMode;}

	const uint32 OpenTunnel(BcmMacAddress &tunnelAddress, BcmDsgClassifierSettings* pClassifierSetting = NULL );

	// DSG-CC/ Client API
	// Public method to be called by Client Helper Thread.
	void RegisterClient( const BcmDsgClient *pClient );
	void RegisterBasicModeClient(BcmDsgClient *pNewClient);
	void RegisterAdvancedModeClient(BcmDsgClient *pNewClient);
	void DeregisterClient( const BcmDsgClient *pClient );

	DsgCcStatusValue State(void) const {return (DsgCcStatusValue)fDsgCcState;}

	// This callback function will notify eSTB of eCM's reset status.
	//
	// Parameters:
	//      uint8 eCmResetType - 1: kEcmReset (eCM reset is detected via heartbeat )
	//		                     2: kEcmDocsDevResetIssued   (eCM is about to reset due to docsDevReset)
	//							 3: kEcmSwUpgradeResetIssued (eCM upgraded its sw and is about to reset)
	// 
	// Returns:  N/A
    void DsgEcmResetIndication(uint8 eCmResetType);

    // PrintDetailed() - Print detailed formatted object contents
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;
	void PrintClassifierList( void ) const;

	void InstallDcdPassThroughHandler( PFN_DSG_DCD_BYPASS_HANDLER dcdbypasshandler );

#if (CABLECARD_SUPPORT)
	void CableCardEnableTwoWayMode(uint8 enable);

	// Allow BcmCableCardIfThread class to access some of my private functions
	friend class BcmCableCardIfThread;
private:
	void RegisterCableCardAdvancedModeClient(BcmDsgCableCardClient *pNewClient);
#else
protected:
#endif
	void ResetEcmTunnels(void);
	void ResetTunnels(void);
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
	const int Mode ( const uint32& mode );
	void SetDsgConfigSetting( BcmDsgConfigSettings &cfgset );

protected:
	void SyncDsgScanSuccess( void );
	void SyncDsgFullScanFail( void );
	void SyncDsgRxDCD( BcmOctetBuffer* octetBuffer );
	void SyncDsgUpstreamChange( bool us_state );
	void SyncDsgUcid( uint8 uc_id  );
	void SyncDsgTimerEvent( uint8 tm_event );
	void SyncDsgTunnelPkt( ETHERNET_PACKET *pkt );
	void SyncDsgDccInitType( uint8 dccInitType );
	void SyncDsgTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations );
	void SyncRegisterClient(BcmDsgClient *pClient);
	void SyncDsgEcmMacReinitEvent(void);

	void State(DsgCcStatusValue state);

	const char* StateTypeName( uint32 value ) const;
	const char* DsgStateTypeName( uint32 value ) const;
	const char* DsgAPIRetCodeTypeName( uint32 value);
	const char* DsgTimerEventTypeName( uint32 value);
	const char* CmInitStateTypeName( uint32 value ) ;
	const char* DsgModeTypeName( uint32 value) const;

	enum
	{
		kNoDcdPeriodMillisec			= 10000,
		kRemoteInterfaceKeepAlivePeriod = 10000,
		kNoGoodSnsTablePeriodMillisec	= 120000,	// 2 minutes

		kNumberOfFullScansPerDsgMode	= 3

	};

    //
    //
    // NoDcd() - Handle T1 timeout event (no UCD from CMTS).
    //
    void NoDcdEvent();
    //
	//
    // RestartNoDcd() - Clear pending T1 timer (no UCD) expiration event and
	//		restart T1.
    //
    void RestartNoDcd();

	// DCD Processing
	// Depending on the Total Number Of Fragments the Fragment Mask
	// may be a an array of 1-8 uint32 values. Number of Fragments field
	// in the DCD message is defined as an 8-bit unsigned integer.
	// Based on that, a DCD message can consist of maximum of 255 fragments.
	void UpdateFragmentMask(uint32 fragmentSeqNumber );

	const bool IsNewDCD( const BcmDcdMsg &dcdMsg );

	// Checks if this fragment already received.
	const bool IsNewFragment(const uint32& fragmentSeqNumber);

	// Checks if all fragments are received.
	const bool IsDcdComplete(void) const;

	void ResetDcdSettings(void);

	BcmDcdMsg* DcdMsg(void);

	void SyncDeregisterClient(BcmDsgClient *pClient);

	// Public method to be called by Estb Helper Thread.
	// Thisa method will construct the App Name Table..
	void ProcessSourceSubTable(ETHERNET_PACKET *pkt );

    BcmDsgEstbTunnelSettingsList &DsgTunnelManager( void)  {return fDsgTunnelManager;}

    inline void RegisterExitSemaphore(BcmCountingSemaphore *pSemaphore)
    {
        pfClientExitSemaphore = pSemaphore;
    }

	uint8 Ucid(void);

protected:

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
    virtual bool Initialize(void);

    // This is the main body of the thread's code.  This is the only method
    // that absolutely must be provided in the derived class (since there is no
    // reasonable default behavior).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void ThreadMain(void);

    // Thread destructor - this is the last method called when the thread is
    // exiting, and is where the thread should delete all of the OS objects that
    // it created.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    virtual void Deinitialize(void);

    // SerializePublicInterfaceCall() - Helper function called to serialize
    //      a public interface member function call via the BcmMessageQueue*
    //      owned by this object.
    //
    // Parameters:
    //      msg_code - message code which identifies the serialized
    //          public interface call.
    //
    //      pmsg - optional void* to data associated with the public
    //          interface call.
    //
    // Returns:
    //      true is returned if msg_code and pmsg were successfully added
    //      to the message queue.  otherwise false is returned and,
    //      if pmsg is nonzero, then pmsg is deleted.
    //
    bool SerializePublicInterfaceCall( unsigned int msg_code, void *pmsg = NULL );

    // RxMsgEvent() - helper function which handles received inter-process
    //      communications message events. It is called by ThreadMain().
    //
    // Returns:
    //      bool - return value false indicates that message queue is
    //          empty.  return value true indicates that internal
    //          iteration limit was reached.
    //
    bool RxMsgEvent();

    // SyncStart() - synchronized portion of Start().
    //
    void SyncStart();

    // SyncStop() - synchronized portion of Stop().
    //
    void SyncStop();

    // SyncRxMacMgtMsg() - synchronized portion of RxMacMgtMsg().
    //
    // Parameters:
    //      obuf - pointer to BcmOctetBuffer object which contains the msg.
	//			It is assumed that MAC mgt header starts at offset 0.
    //
	void SyncRxMacMgtMsg( BcmOctetBuffer* obuf );


    // Reset() - Reset internal state to initial conditions.
    //
    void Reset();

	enum
	{
		kStart				= 30,
		kSourceSubTable		,
		kRegisterClient		,
		kDeregisterClient	,
		kSetDsgOtherConfig  ,
		kEventResetKeepAliveTimer
	};

	// eCM API.
	bool DsgStartScan ( void );


protected:

    //
    //
    // ValidateDcdAndOpenTunnels() - Subroutine which parses and validates
    //      the DCD buffer.
    //
    // Returns:
    //      bool - False is returned if any errors are detected by parser
    //          or validation tests.
    //
    bool ValidateDcdAndOpenTunnels(void);

	void HuntForDsgChannel(void);

	void OpenTunnels(void);

	// Processing tunnel's client ID list.
	bool ProcessClientList( BcmDsgEstbTunnelSettings * pTunnel, bool &duplicateClient, bool &openTunnel, bool &openSecretTunnel );

	bool OpenTunnelsForMacAddressTypeClients( void ) ;

	// Actions to be performed by DSG-CC thread.
	void SyncProcessSourceSubTable(BcmDSGAppNameTable *pNewAppNameTable);

	// Returns true if at least one tunnel can be open for
	// a Client with ID = Application Name
	const bool AssignClientIdsAndOpenTunnels(BcmDSGAppNameTable *pNewAppNameTable);
	void NoGoodSnsTableTimerEvent(void);
	bool TestAndProcessApplicationNameTypeClient(BcmDsgClient *pNewClient);

	const bool OpenBroadcastChannel(void);

	void ApplyDcdConfigSettings( void);

	bool ResetClientTunnelIds(void);

	bool AddTestClientMacAddress(const BcmMacAddress &macAddress);

	const uint32 ActivateTunnel( BcmDsgEstbTunnelSettings *pTunnel);

	const uint32 ProcessUcid(void);
	bool ProcessAllClients( BcmDsgEstbTunnelSettings *pTunnel );
	bool CloseDeadTunnels( void);
	bool CloseTunnel(const uint32& id);

	inline void BcastTunnelId( const uint32 &bcastId) {fBroadcastTunnelId = bcastId;}
	inline uint32 &BcastTunnelId( void) {return fBroadcastTunnelId;}

	void HandleTestEvent( const uint32 &msg_code, void* buf );
	bool KeepAlive(void);

	bool DcdPassThroughHandler( const BcmOctetBuffer &octetBuffer );

protected:
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;

    // if true, object is being destructed and ThreadMain() needs to break out
    // of it's endless loop.
    bool fTimeToExit;

    // Determines the behavior of Shutdown(); if false, then Shutdown() is
    // handled by simply stopping then restarting the state machines.  Otherwise
    // Shutdown() is handled by exiting, which often causes the system to
    // reboot (this is the default behavior).
    bool fShutdownExitsApp;


    // This is the semaphore that was given to me in the call to Start().  If
    // not NULL, then I will post the semaphore once I exit my thread.
    BcmCountingSemaphore *pfClientExitSemaphore;

    // if true, thread is running and will execute events.
    bool fThreadIsRunning;

    // Container for all of my events.
    BcmEventSet *pfEventSet;

    // message queue for docsis mac mgt messages and serialized public interface
    // calls.
    BcmMessageQueue *pfRxQueue;

    // Called in destructor to indicate that we need to break out of ThreadMain()'s
    // main loop.
    //
    BcmEvent *pfDestructEvent;

    //
    // No DCD from CMTS.
    BcmTimer* pfNoDcd;

	// Keepalive timer.
	BcmTimer* pfInterfaceKeepAliveTimer;

	// Keepalive timer.
	BcmTimer* pfNoSnsTableTimer;

	// DCD processing
	uint16 fConfigChangeCount;
	uint8 fNumberOfFragments;
	uint8 fParsedFragments;

	uint16 fLastConfigChangeCount;

	uint32 fFragmentMaskArray[8];

	PFN_DSG_DCD_BYPASS_HANDLER pfnDcdPassThroughHandler;

public:

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
    static BcmDsgCommandTable &GetCommandTableSingleton(void);

    // Since the command handler code is in the command table, I need to allow
    // it full access to me.
    friend class BcmDsgCommandTable;
#endif

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgClientCtlThread(const BcmDsgClientCtlThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgClientCtlThread & operator = (const BcmDsgClientCtlThread &otherInstance);

private:

	// Begin data members which are specific to DSG Operation Mode.
    //
	uint8 fDsgMode;

	// Docsis enabled/disabled
	bool fEstbDefaultTwoWayMode;
	bool fEcmTwoWayMode;

	//
	// End data members which are specific to DSG Operation Mode.


#if defined(BCM_DSG_DUAL_PROCESSOR_INTERFACE) || defined(BCM_DSG_DUAL_PROCESS_INTERFACE)
	BcmEstbRemoteIfHelperThread * pfEstbRemoteIfHelperThread;
	#if defined(BCM_DSGCC_ECM_USE_SHARED_MEMORY_IPC)
	    BcmEstbRemoteIfDataHandlerThread * pfEstbRemoteIfDataHandlerThread;
	#endif
#endif

    BcmDsgClientRegistrarThread * pfDsgClientRegistrarThread;

	// This is the singleton instance of this class.
    static BcmDsgClientCtlThread *pfSingletonInstance;

    // Begin data members which are specific to DSG-CC state machine..
    //
    // see class enum for legal values for state variable.
	bool fLinkUp;
    int fDsgCcState;
	bool fWaitingForSrcNameSubTable;

	// Number of clients registered with DSG-CC to receive traffic.
	uint32 fNumberOfClients;

	// Upstream Channel ID (UCID).
	uint8 fCurrentUcid;
	bool fUcidAvailable;

	// Tunnel Settings List
	BcmDsgEstbTunnelSettingsList fDsgTunnelManager;

    // List of DSG Tunnel Classifier settings objects.
    ClassifierSettingsList fClassifierSetsList;

    // DSG Configuration Settings.
    BcmDsgConfigSettings fConfigSettings;

	BcmDsgClientManager fDsgClientManager;
	BcmDsgClientManager *pfDsgClientManager;
	BcmDsgBroadcastTunnelPktReassembler *pfDsgBroadcastTunnelPktReassembler;
	uint8 fToken;

	#if (BCM_NONVOL_SUPPORT)
	#if (BCM_DSG_SINGLE_PROCESS_INTERFACE)
	BcmEstbNonVolSettings *pfDsgccNonVolSettings;
	#else
	BcmDsgccNonVolSettings * pfDsgccNonVolSettings;
	#endif
	#endif

	// PR9041
	uint32 fNumberOfDsLockAttempts;

	// Pointer to DCD Message Object to store the last new DCD.
	BcmDcdMsg *pfDcdMsg;

	// First DCD flag.
	bool fFirstDcd;

public:
	// Data Members for Broadcast Tunnel/SCTE 65 data
	// Application Name mapping
    BcmDSGAppNameTable *pfAppNameTable;
    uint32 fBroadcastTunnelId;
	BcmMacAddress fBroadcastTunnelMac;
};


//********************** Inline Method Implementations ***********************
inline ostream& operator<<( ostream& ostrm, const BcmDsgClientCtlThread& rhs )
{
    return rhs.Print( ostrm );
}

#endif



