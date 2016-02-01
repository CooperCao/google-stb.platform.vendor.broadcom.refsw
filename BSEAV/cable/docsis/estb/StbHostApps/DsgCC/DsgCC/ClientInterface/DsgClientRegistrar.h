//****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
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
//  Filename:       DsgClientRegistrar.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  Nov 22, 2004
//
//****************************************************************************
//  Description:

//
//****************************************************************************

#ifndef DSGCLIENTREGISTRAR_H
#define DSGCLIENTREGISTRAR_H

//********************** Include Files ***************************************

#include "Thread.h"

#include "IpAddress.h"
#include "IpAddressList.h"

#include "dsg_api.h"
#include "DsgClient.h"
//********************** Global Types ****************************************
// The bit mask constants for the status that is passed to external clients
enum
{
	DSGCLIENTSTATUSREGISTERED	= DSGCC_CLIENT_REGISTERED,
	DSGCLIENTSTATUSOPERATIONAL	= DSG_IS_OPERATIONAL,
	DSGCLIENTSTATUSADVANCEMODE	= DSG_IS_IN_ADVANCED_MODE,
	DSGCLIENTSTATUSTWOWAYMODE   = DSG_IS_IN_TWOWAY_MODE,
	DSGCLIENTSTATUSTUNNELOPEN	= DSGCC_CLIENT_TUNNEL_OPEN
};
//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmEvent;
class BcmCountingSemaphore;
class BcmSocket;
class BcmDsgClientCtlThread;
//********************** Class Declaration ***********************************


class BcmDsgClientRegistrarThread : public BcmThread
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
	BcmDsgClientRegistrarThread( BcmOperatingSystem::ThreadPriority initialPriority, BcmDsgClientCtlThread* pDsgClientCtlThread);

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
    virtual ~BcmDsgClientRegistrarThread();

	void SetEstbIpAddress(BcmIpAddress &eStbIpAddress);

    static BcmDsgClientRegistrarThread *GetSingletonInstance(void);

    // This method finds an entry in the client list. The
    // clientIdType, clientId (or Address or Name), and
    // clientPort must all match.
    //
    // Parameters:  struct dsgClientRegInfo.
    //
    // Returns: the index of the entry in the client list 
    // or -1 if search failed.
    //
    int findDsgClientEntry(struct dsgClientRegInfo *regInfo);
    int findDsgClientEntry(BcmDsgClient *pClient);


    // This method gets the status of a client. The
    //
    // Parameters:  struct dsgClientRegInfo.
    //
    // Returns: the client controller's view of the status
    // of the client. This is a bitmask. The bits are:
    // 1 - Client registered - 1 = registered, 0 = not registered
    // 2 - DSG Mode - 1 = advanced mode, 0 = basic mode
    // 4 - Tunnel Open - 1 = open, 0 = not open
    // 8 - CM Downstream up - 1 = up, 0 = not up
    // 16 - CM Upstream up - 1 = up, 0 = not up
    // all other bits are always zero
    //
    int statusDsgClientEntry(struct dsgClientRegInfo *regInfo);


    // This method allocates a new client object, fills
    // it in with the registration info, and adds it
    // to the client list.
    //
    // Parameters:  struct dsgClientRegInfo.
    //
    // Returns:  1 if successful, 0 if failed.
    //
    int addDsgClient(struct dsgClientRegInfo *regInfo);
	int addDsgClientWithClassifier(struct dsgClientRegInfo *regInfo, dsg_classifier* dsgClassifier = NULL); 
	int populateDsgClientList(BcmDsgClient* pNewClient);

    // This method removes a client from the 
    // client list, based on registration info from the RPC interface
    //
    // Parameters:  struct dsgClientRegInfo.
    //
    // Returns:  1 if successful, 0 if failed.
    //
    int deleteDsgClient(struct dsgClientRegInfo *regInfo);

    // This method removes a client from the 
    // client list, based on a BcmDsgClient object from the ClientCtl thread
    // Note that this can be the actual object or a copy.
    //
    // Parameters:  BcmDsgClient
    //
    // Returns:  1 if successful, 0 if failed.
    //
    int deleteDsgClient(BcmDsgClient *pClient);

	// Since DsgClientRegistrar keeps a duplicate list if clients we must update its status whenever 
	// client's tunnel  status changes because dsgcc_KeepAliveClient API returns client's tunnel 
	// status from DsgClientRegistrar client list.
	int	updateDsgClientTunnelStatus(BcmDsgClient *pClient) ;

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

protected:

    // This method sets up the RPC interface so clients
    // can register.
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void HandleRegistrations(void);


    // This method cleans out the client list.
    //
    // Parameters:  None.
    //
    // Returns:  1 if successful, 0 if failed.
    //
    int purgeDsgClientList();


    // This method prints out the contents of the client list.
    //
    // Parameters:  None.
    //
    // Returns:  None.
    //
    void dumpDsgClientList(void);


protected:
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;

	BcmSocket *pfSocket;

	BcmIpAddress fEcmIpAddress;
	BcmIpAddress fEstbIpAddress;

	BcmDsgClientCtlThread *pfDsgClientCtlThread;
    int fNumDsgClients;    
    BcmDsgClient *dsgClientList[MAXNUMDSGCLIENTS];

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgClientRegistrarThread(const BcmDsgClientRegistrarThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgClientRegistrarThread & operator = (const BcmDsgClientRegistrarThread &otherInstance);

private:

    // This is the singleton instance of this class.
    static BcmDsgClientRegistrarThread *pfSingletonInstance;


};


//********************** Inline Method Implementations ***********************


#endif



