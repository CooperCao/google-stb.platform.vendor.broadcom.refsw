//****************************************************************************
//
//  Copyright (c) 2000  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92619
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       DsgClientRegistrarThread.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  Nov 22, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DsgClientRegistrar.h"
#include "DsgClientCtlThread.h"

//#include "DsgClient.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"

#include "SystemTime.h"

#include "BcmSocket.h"
#include "SocketFactory.h"
#include <errno.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#if defined(WIN32)
#include <winsock.h>
#else
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <unistd.h>
#endif
#include <errno.h>

#ifdef DSG_RPC_INCLUDED
#include "dsgccClientRegistration_rpcServer.h"
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************
//********************** Class Method Implementations ************************

// Set to NULL. Will be set up by constructor.
BcmDsgClientRegistrarThread *BcmDsgClientRegistrarThread::pfSingletonInstance = NULL;

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
BcmDsgClientRegistrarThread::BcmDsgClientRegistrarThread( BcmOperatingSystem::ThreadPriority initialPriority, BcmDsgClientCtlThread* pDsgClientCtlThread) :
BcmThread("DsgClientRegistrarace Thread", false, initialPriority),
fEstbIpAddress(kAllZerosIp)
{
    CallTrace("BcmDsgClientRegistrarThread", "BcmDsgClientRegistrarThread");

    fMessageLogSettings.SetModuleName("BcmDsgClientRegistrarThread");

    // Set some clean default values.
    fExitThread = false;

	if( pDsgClientCtlThread == NULL )
	{
		gFatalErrorMsg(fMessageLogSettings, "BcmDsgClientRegistrarThread") << "pDsgClientCtlThread is NULL!" << endl;
	}
	else
	{
		pfDsgClientCtlThread = pDsgClientCtlThread;
	}

    // Just kick off the thread.  PR1613 - VxWorks uses almost 4k of stack for
    // some reason; not sure why.  Give it 6k so we have lots of margin.

#if defined(TARGETOS_vxWorks) || defined(WIN32)
	if (pfOperatingSystem->BeginThread(this, 16 * 1024) == false)
#else
	if (pfOperatingSystem->BeginThread(this, 8 * 1024) == false)
#endif
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmDsgClientRegistrarThread") << "Failed to spawn my thread!" << endl;

        assert(0);
    }

    // Set myself as the singleton.
    if (pfSingletonInstance == NULL)
    {
        gInfoMsg(fMessageLogSettings, "BcmDsgClientRegistrarThread")
            << "Setting up the singleton pointer." << endl;
        pfSingletonInstance = this;
    }
    else
    {
        gWarningMsg(fMessageLogSettings, "BcmDsgClientRegistrarThread")
            << "Singleton pointer is not NULL!  There are multiple instances!  Leaving the singleton pointer alone..." << endl;
    }

}


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
BcmDsgClientRegistrarThread::~BcmDsgClientRegistrarThread()
{
    CallTrace("BcmDsgClientRegistrarThread", "~BcmDsgClientRegistrarThread");

    fExitThread = true;

    // Make sure the thread has exited.
    WaitForThread();    
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
BcmDsgClientRegistrarThread *BcmDsgClientRegistrarThread::GetSingletonInstance(void)
{
    if (pfSingletonInstance == NULL)
    {
        gLogMessageRaw
            << "BcmDsgClientRegistrarThread::GetSingletonInstance:  WARNING - the singleton instance is NULL, and someone is accessing it!" << endl;
    }

    return pfSingletonInstance;
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
bool BcmDsgClientRegistrarThread::Initialize(void)
{
    int i;

    CallTrace("BcmDsgClientRegistrarThread", "Initialize");

    fNumDsgClients = 0;
    for (i = (MAXNUMDSGCLIENTS - 1); i >= 0; i--)
      dsgClientList[i] = NULL;

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
void BcmDsgClientRegistrarThread::ThreadMain(void)
{
    CallTrace("BcmDsgClientRegistrarThread", "ThreadMain");

	HandleRegistrations();
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmDsgClientRegistrarThread::Deinitialize(void)
{
    CallTrace("BcmDsgClientRegistrarThread", "Deinitialize");

    // Get rid of the OS objects that I created.
    purgeDsgClientList();
}

// This method sets up the RPC interface so clients
// can register.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void 
BcmDsgClientRegistrarThread::HandleRegistrations(void)
{
#if defined (DSG_RPC_INCLUDED)    
    if(!initializeRPCinterface()) {
		gFatalErrorMsg(fMessageLogSettings, "BcmDsgClientRegistrarThread") 
			<< "Failed to initialize the RPC interface!" << endl;
    }
#endif
}


int
BcmDsgClientRegistrarThread::purgeDsgClientList() {

  while (fNumDsgClients)
    delete dsgClientList[--fNumDsgClients];
  return 1;
}

int
BcmDsgClientRegistrarThread::findDsgClientEntry(BcmDsgClient *pClient) {
  int i;

  for (i = fNumDsgClients; i;)
    if ((dsgClientList[--i])->ClientPort() == pClient->ClientPort())
     if ((dsgClientList[i])->IdType() == (pClient->IdType() & 0xffff))
        return i;
  return -1;
}

int
BcmDsgClientRegistrarThread::findDsgClientEntry(struct dsgClientRegInfo *regInfo) {
  int i;

  for (i = fNumDsgClients; i;)
    if ((dsgClientList[--i])->ClientPort() == regInfo->clientPort)
      if ((dsgClientList[i])->IdType() == (regInfo->idType & 0xffff))
        return i;
  return -1;
}

    // This method gets the status of a client. The
    //
    // Parameters:  struct dsgClientRegInfo.
    //
    // Returns: the client controller's view of the status 
    // of the client. This is a bitmask. The bits are:
    // 1 - Client registered - 1 = registered, 0 = not registered
    // 2 - DSG Operational - 1 = operational, 0 = not operational
    // 4 - DSG Mode - 1 = advanced mode, 0 = basic mode
    // 8 - Tunnel Open - 1 = open, 0 = not open
    // 16 - CM Upstream up - 1 = up, 0 = not up
    // all other bits are always zero
    //
int BcmDsgClientRegistrarThread::statusDsgClientEntry(struct dsgClientRegInfo *regInfo) {
  int i;
  int retVal = 0;

  i = findDsgClientEntry(regInfo);
  if (i < 0)
    return (retVal);

  retVal |= DSGCLIENTSTATUSREGISTERED;	// Found client, 'registered' bit on
  // printf ("DSGCLIENTSTATUSREGISTERED retVal = %d\n", retVal);

  if (pfDsgClientCtlThread == NULL) {
    printf ("BcmDsgClientRegistrarThread::statusDsgClientEntry() pfDsgClientCtlThread == NULL!!!\n");
    return (retVal);			// No Client Ctl thread, just return now
  }

  // Check if DSG is operational
  if (pfDsgClientCtlThread->State() == BcmDsgClientCtlThread::kOperational) {
    retVal |= DSGCLIENTSTATUSOPERATIONAL; // In advanced mode, turn bit on
    // printf ("DSGCLIENTSTATUSOPERATIONAL retVal = %d\n", retVal);
  }

  // Check DSG Mode
  if (pfDsgClientCtlThread->Mode() == kDsgAdvanceMode) {
    retVal |= DSGCLIENTSTATUSADVANCEMODE; // In advanced mode, turn bit on
    // printf ("DSGCLIENTSTATUSADVANCEMODE retVal = %d\n", retVal);
  }

  // Check DSG in Two-way mode
  if (pfDsgClientCtlThread->TwoWayMode() == true) {
    retVal |= DSGCLIENTSTATUSTWOWAYMODE; // In two way mode, turn bit on
    // printf ("DSGCLIENTSTATUSTWOWAYMODE retVal = %d\n", retVal);
  }

  // Check DSG Tunnel Open for this client
  uint32 port = regInfo->clientPort;
  if (pfDsgClientCtlThread->DsgClientManager().IsClientAssignedTunnel(port) == true) {
    retVal |= DSGCLIENTSTATUSTUNNELOPEN; // Tunnel open, turn bit on
    // printf ("DSGCLIENTSTATUSTUNNELOPEN retVal = %d\n", retVal);
  }

  uint32 tunnelStatus = dsgClientList[i]->TunnelStatus();
  tunnelStatus <<= 16;
  retVal |= tunnelStatus;
  //printf ("Client keepalive retVal = %08x\n", retVal);

  return (retVal);			// return with the whole bit mask

}


int
BcmDsgClientRegistrarThread::addDsgClient(struct dsgClientRegInfo *regInfo) {
  BcmDsgClient *newClient = 0;

  if (fNumDsgClients >= MAXNUMDSGCLIENTS)
    return DSGCC_MAX_CLIENT_REACHED;

  if(pfDsgClientCtlThread == NULL)
    return DSGCC_ERROR;

  newClient = new BcmDsgClient(regInfo);
  if (newClient == NULL)
    return DSGCC_INVALID_PARAM;

  dsgClientList[fNumDsgClients++] = newClient;

  if(pfDsgClientCtlThread)
    pfDsgClientCtlThread->RegisterClient(newClient);

  return DSGCC_SUCCESS;
}

int
BcmDsgClientRegistrarThread::addDsgClientWithClassifier(struct dsgClientRegInfo *regInfo, dsg_classifier* dsgClassifier) 
{
  BcmDsgClient *newClient = 0;

  if (fNumDsgClients >= MAXNUMDSGCLIENTS)
    return DSGCC_MAX_CLIENT_REACHED;

  if(pfDsgClientCtlThread == NULL)
    return DSGCC_ERROR;

  newClient = new BcmDsgClient(regInfo, dsgClassifier);
  if (newClient == NULL)
    return DSGCC_INVALID_PARAM;

  dsgClientList[fNumDsgClients++] = newClient;

  if(pfDsgClientCtlThread)
    pfDsgClientCtlThread->RegisterClient(newClient);

  return DSGCC_SUCCESS;
}

int
BcmDsgClientRegistrarThread::populateDsgClientList(BcmDsgClient* pNewClient) {
  if (pNewClient == NULL)
    return DSGCC_INVALID_PARAM;

  if (fNumDsgClients >= MAXNUMDSGCLIENTS)
    return DSGCC_MAX_CLIENT_REACHED;

  dsgClientList[fNumDsgClients++] = pNewClient;

  return DSGCC_SUCCESS;
}

// This is a deletion from the client itself (there is regInfo), but
// we need to let the Client Control Thread do things in the right 
// order.
int
BcmDsgClientRegistrarThread::deleteDsgClient(struct dsgClientRegInfo *regInfo) {
  int i;
  BcmDsgClient *toBeDeleted;

  i = findDsgClientEntry(regInfo);

  if (i > -1) {
    toBeDeleted = dsgClientList[i];
    if(pfDsgClientCtlThread)
      pfDsgClientCtlThread->DeregisterClient(toBeDeleted);
    return DSGCC_SUCCESS;
  }
  return DSGCC_ERROR;
}

// This is a deletion from the client control thread (there is a BcmDsgClient object)
int
BcmDsgClientRegistrarThread::deleteDsgClient(BcmDsgClient *pClient) {
  int i;
  BcmDsgClient *toBeDeleted;

  i = findDsgClientEntry(pClient);

  if (i > -1) {
    toBeDeleted = dsgClientList[i];
    fNumDsgClients--;
    dsgClientList[i] = dsgClientList[fNumDsgClients];
    delete toBeDeleted;
    return DSGCC_SUCCESS;
  }
  return DSGCC_ERROR;
}

void
BcmDsgClientRegistrarThread::dumpDsgClientList() {
  //int i;

  // printf("\n\n%d clients in the list...\n", fNumDsgClients);
  //   delete *toBeDeleted;
  // for (i=0; i < fNumDsgClients; i++) {
  //   printf("Client entry %d on port %d\n", i, dsgClientList[i]->clientPort);
  // }
}

int
BcmDsgClientRegistrarThread::updateDsgClientTunnelStatus(BcmDsgClient *pClient) {

#if 0
	int i = findDsgClientEntry(pClient);

	if( i < 0 )
	{
		return DSGCC_ERROR;
	}

	// update tunnel status
	dsgClientList[i]->TunnelStatus(pClient->TunnelStatus());
#endif
	return DSGCC_SUCCESS;
}
