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
//  $Id$
//
//  Filename:       SnmpThread.h
//  Author:         Kevin O'Neal
//  Creation Date:  June 5, 2003
//
//****************************************************************************
//  Description:
//      Thread for the main loop for any 3rd party SNMP agent.  Basically
//      this thread is responsible for receiving packets off the wire and
//      tossing them over to the agent for processing.  Also has some secondary
//      purposes, but that's the main one.
//
//****************************************************************************

#ifndef SNMPTHREAD_H
#define SNMPTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"
#include "IpAddress.h"
#include "SnmpApi.h"
#include "BcmSocket.h"
#include <map>

//********************** Global Types ****************************************

typedef map <int, BcmSocket *> SocketMap;

#define PACKETPOINTER_MAGIC_NUMBER 0x70507472 // "pPtr"
typedef struct _PacketPointer
{
  unsigned long Magic;
  unsigned long Length;
  unsigned char *pData;
} PacketPointer;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
class BcmSnmpAgent;
class BcmSnmpV3Agent;
class BcmSocketSet;
class BcmAgentDescriptor;

//********************** Class Declarations ***********************************

class BcmAgentDescriptor
{
public:
    BcmAgentDescriptor (BcmSnmpAgent *pAgent);
   ~BcmAgentDescriptor ();
   
    bool Reset ();
    bool Bind ();
   
protected:
    BcmSnmpAgent *fpAgent;
    BcmSnmpV3Agent *fpV3Agent;
    BcmSocket *fpSocket;
    BcmIpAddress fBoundIp;
    int fBoundPort;
    unsigned long fPacketsIn;
    unsigned long fPacketsOut;
    
friend class BcmSnmpThread;
};

class BcmSnmpThread : public BcmThread
{
public:
    // Get the singleton instance of this class.
    static BcmSnmpThread &Singleton ();
    
    int MaxPacketSize () const; // get
    bool MaxPacketSize (int MaxSize); // set - returns true on OK, false on error
    
    // Print thread info.
    ostream &Print(ostream &outStream) const;

    // Add an agent for us to handle.
    bool RegisterAgent (BcmSnmpAgent *pAgent);
    
    // An agent's IP address, UDP port, etc. has changed.
    bool AgentStateChanged (BcmSnmpAgent *pAgent);
    
    // APIs to allow access to SNMP agents.  In each case, return NULL if
    // not found.
    int NumAgents () const;
    BcmSnmpAgent *GetAgent (int Index) const;
    BcmSnmpAgent *FindAgentByIf (int Interface) const;
    BcmSnmpAgent *FindAgentByName (const BcmString &Name) const;
    BcmSnmpAgent *FindAgentByIp (const BcmIpAddress &Ip) const;
    BcmSnmpAgent *FindAgentBySubnet (const BcmIpAddress &Subnet) const;

#if (SNMP_SUPPORT == 3)    
    bool RegisterV3Agent (BcmSnmpV3Agent *pAgent);
    BcmSnmpV3Agent *GetV3Agent (int Index) const;
    BcmSnmpV3Agent *FindV3Agent (const BcmSnmpAgent *pAgent) const;
    BcmSnmpV3Agent *FindV3AgentByIf (int Interface) const;
    BcmSnmpV3Agent *FindV3AgentByName (const BcmString &Name) const;
    BcmSnmpV3Agent *FindV3AgentByIp (const BcmIpAddress &Ip) const;
    BcmSnmpV3Agent *FindV3AgentBySubnet (const BcmIpAddress &Subnet) const;
#endif

    /*\
     *  These methods may be used by the agent core to get sockets to use for
     *  traps and informs.
    \*/
    
    // Open a socket.  If a port is specified, then the socket will be bound
    // to that UDP port on the specified IP address.  Thus this socket is
    // usable as a 'server' socket in a subsequent recvfrom call.  If no
    // port is specified, then the IP address argument will be used to determine
    // the IP stack with which this socket will be associated.  If no match
    // can be found, then the socket will be created without specifying an IP 
    // stack and the caller will just have to take his chances.
    BcmSocket *OpenSnmpSocket (const BcmIpAddress &Ip, int Port = 0);
    
    // Close the specified socket.
    int CloseSnmpSocket (int SocketDescriptor);
    
    // Read data off the wire.  Bytes read is returned if successful, -1 on 
    // failure.  Data is shoved into pBuf, source address and port are
    // returned in FromIp and FromPort, respectively.
    int RecvfromSnmpSocket (int Socket, unsigned char *pBuf, int BufSize, BcmIpAddress &FromIp, unsigned short &FromPort) const;
    
    // Write data to the wire.  Bytes sent is returned if successful, -1 on
    // failure.
    int SendtoSnmpSocket (int Socket, const unsigned char *pBuf, int BufSize, const BcmIpAddress &ToIp, unsigned short &ToPort) const;
    
protected:
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
    BcmSnmpThread(BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kNormalPriority);

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
    virtual ~BcmSnmpThread();
    
    // Miscellaneous constants
    typedef enum
    {
      kMaxSnmpAgents = 8,
      kDefaultRcvBufSize = 4096,
      kSignalPort = 5502
    } SnmpThreadConstant;
    
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
    
    // Given an agent pointer, find his descriptor.
    BcmAgentDescriptor *FindDescriptor (const BcmSnmpAgent *pAgent) const;
    
    // Similar method, but use a socket object for the find.
    BcmAgentDescriptor *FindDescriptor (const BcmSocket *pSocket) const;
    BcmAgentDescriptor *FindDescriptor (int Socket) const;
    
    // Signals for use in Signal() and ProcessSignal() methods.  More may be
    // added if needed.
    typedef enum
    {
      kShutdown = 0,
      kUpdateAgents
    } SnmpThreadSignal;
    
    typedef struct 
    {
      SnmpThreadSignal Signal;
      void *pPayload;
    } SnmpThreadSignalPayload;
    
    // Signal our main loop to take action from above enum.
    bool Signal (SnmpThreadSignal Action, void *pPayload = NULL);
    
    // Process the signal and do what it says.
    bool ProcessSignal (SnmpThreadSignal Action, void *pPayload);
  
    // Scan all agents and re-socket/re-bind if necessary.  Like if the IP
    // address or UDP port of the agent changes.
    int UpdateAgents ();
    
    // Called at the top of the main loop before we call Select().  Make
    // sure that all the sockets are set read in the socket set.
    void UpdateSocketSet ();
    
    // Iterate through all agents and send any linkUp/linkDown traps if
    // any interface's ifOperStatus has changed.  Move responsibility for
    // this up from the drivers to the app.
    void CheckLinkStatus ();
    
    // Handle internal "PacketPointer" conversions for giganto-packets.
    // If the packet passed in pPkt is determined to be a PacketPointer, then
    // the address of the pointed-to-pcaket will be returned and Length
    // will be updated with the length of the pointed-to-packet.  If it's not
    // a PacketPointer, NULL is returned.
    unsigned char *ConvertFromPacketPointer (const unsigned char *pPkt, int &Length) const;
    
protected:
    static BcmSnmpThread *fpSingleton;
    
    // Tells the thread when to exit (set in ProcessSignal(kShutdown)).
    bool fExitThread;
    
    BcmSocketSet *fpSocketSet;
    BcmSocket *fpSignalSocket;
    BcmAgentDescriptor *fpAgents[kMaxSnmpAgents];
    int fMaxPacketSize;
    int fNumAgents;
    unsigned long fLastLinkCheck;
    SocketMap fSockets;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmSnmpThread(const BcmSnmpThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmSnmpThread & operator = (const BcmSnmpThread &otherInstance);
};


//********************** Inline Method Implementations ***********************

inline ostream & operator << (ostream &outStream, const BcmSnmpThread &Thread)
{
  return Thread.Print(outStream);
}

#endif
