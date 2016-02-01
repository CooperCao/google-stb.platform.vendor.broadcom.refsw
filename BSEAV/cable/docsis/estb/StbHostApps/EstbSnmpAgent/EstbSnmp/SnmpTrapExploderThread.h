//****************************************************************************
//
// Copyright (c) 2010 Broadcom Corporation
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
//  $Id$
//
//  Filename:       SnmpTrapExploderThread.h
//  Author:         Maurice Turcotte
//  Creation Date:  16 MAY 2010
//
//****************************************************************************
//  Description:
//      Thread for the main loop for the SNMP trap exploder.  Basically
//      this thread is responsible for receiving traps off the local loopback.
//      It them calls the Master Agent to send a corresponding trap in the 
//      correct format to all the desinations that have been configured.
//
//****************************************************************************

#ifndef SNMPTRAPEXPLODERTHREAD_H
#define SNMPTRAPEXPLODERTHREAD_H

//********************** Include Files ***************************************

#include "Thread.h"
#include "IpAddress.h"
#include "SnmpApi.h"
#include "BcmSocket.h"
#include <map>

//********************** Global Types ****************************************

// typedef map <int, BcmSocket *> SocketMap;

#define TRAP_PACKETPOINTER_MAGIC_NUMBER 0x70507472 // "pPtr"
typedef struct _TrapPacketPointer
{
  unsigned long Magic;
  unsigned long Length;
  unsigned char *pData;
} TrapPacketPointer;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************
class BcmSnmpTrapHandler;
class BcmSocketSet;
class BcmTrapHandlerDescriptor;

//********************** Class Declarations ***********************************

class BcmTrapHandlerDescriptor
{
public:
    BcmTrapHandlerDescriptor (BcmSnmpTrapHandler *pTrapHandler);
   ~BcmTrapHandlerDescriptor ();
   
    bool Reset ();
    bool Bind ();
   
protected:
    BcmSnmpTrapHandler *fpTrapHandler;
    BcmSocket *fpSocket;
    BcmIpAddress fBoundIp;
    int fBoundPort;
    unsigned long fPacketsOut;
    unsigned long fPacketsIn;
    
friend class BcmSnmpTrapExploderThread;
};

class BcmSnmpTrapExploderThread : public BcmThread
{
public:
    // Get the singleton instance of this class.
    static BcmSnmpTrapExploderThread &Singleton ();
    
    int MaxPacketSize () const; // get
    bool MaxPacketSize (int MaxSize); // set - returns true on OK, false on error
    
    // Print thread info.
    ostream &Print(ostream &outStream) const;

    // Add a trap hander for us to service.
    bool RegisterTrapHandler (BcmSnmpTrapHandler *pTrapHandler);
    
    // An trap handler's IP address, UDP port, etc. has changed.
    bool TrapHandlerStateChanged (BcmSnmpTrapHandler *pTrapHandler);
    
    // APIs to allow access to SNMP trap handlers.  In each case, return NULL if
    // not found.
    int NumTrapHandlers () const;
    BcmSnmpTrapHandler *GetTrapHandler (int Index) const;
    BcmSnmpTrapHandler *FindTrapHandlerByName (const BcmString &Name) const;
    BcmSnmpTrapHandler *FindTrapHandlerByIp (const BcmIpAddress &Ip) const;

    /*\
     *  These methods may be used by the trap handler core to get sockets to use for
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
    BcmSnmpTrapExploderThread(BcmOperatingSystem::ThreadPriority initialPriority = BcmOperatingSystem::kNormalPriority);

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
    virtual ~BcmSnmpTrapExploderThread();
    
    // Miscellaneous constants
    typedef enum
    {
      kMaxSnmpTrapHandlers = 2,
      kDefaultRcvBufSize = 4096,
      kSignalPort = 5502
    } SnmpTrapExploderThreadConstant;
    
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
    
    // Given a trap handler pointer, find his descriptor.
    BcmTrapHandlerDescriptor *FindDescriptor (const BcmSnmpTrapHandler *pTrapHandler) const;
    
    // Similar method, but use a socket object for the find.
    BcmTrapHandlerDescriptor *FindDescriptor (const BcmSocket *pSocket) const;
    BcmTrapHandlerDescriptor *FindDescriptor (int Socket) const;
    
    // Signals for use in Signal() and ProcessSignal() methods.  More may be
    // added if needed.
    typedef enum
    {
      kShutdown = 0,
      kUpdateTrapHandlers
    } SnmpTrapExploderThreadSignal;
    
    typedef struct 
    {
      SnmpTrapExploderThreadSignal Signal;
      void *pPayload;
    } SnmpTrapExploderThreadSignalPayload;
    
    // Signal our main loop to take action from above enum.
    bool Signal (SnmpTrapExploderThreadSignal Action, void *pPayload = NULL);
    
    // Process the signal and do what it says.
    bool ProcessSignal (SnmpTrapExploderThreadSignal Action, void *pPayload);
  
    // Scan all trap handlers and re-socket/re-bind if necessary.  Like if the IP
    // address or UDP port of the trap handler changes.
    int UpdateTrapHandlers ();
    
    // Called at the top of the main loop before we call Select().  Make
    // sure that all the sockets are set read in the socket set.
    void UpdateSocketSet ();
    
    // Handle internal "TrapPacketPointer" conversions for giganto-packets.
    // If the packet passed in pPkt is determined to be a TrapPacketPointer, then
    // the address of the pointed-to-pcaket will be returned and Length
    // will be updated with the length of the pointed-to-packet.  If it's not
    // a TrapPacketPointer, NULL is returned.
    unsigned char *ConvertFromTrapPacketPointer (const unsigned char *pPkt, int &Length) const;
    
protected:
    static BcmSnmpTrapExploderThread *fpSingleton;
    
    // Tells the thread when to exit (set in ProcessSignal(kShutdown)).
    bool fExitThread;
    
    BcmSocketSet *fpSocketSet;
    BcmSocket *fpSignalSocket;
    BcmTrapHandlerDescriptor *fpTrapHandlers[kMaxSnmpTrapHandlers];
    int fMaxPacketSize;
    int fNumTrapHandlers;
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
    BcmSnmpTrapExploderThread(const BcmSnmpTrapExploderThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmSnmpTrapExploderThread & operator = (const BcmSnmpTrapExploderThread &otherInstance);
};


//********************** Inline Method Implementations ***********************

inline ostream & operator << (ostream &outStream, const BcmSnmpTrapExploderThread &Thread)
{
  return Thread.Print(outStream);
}

#endif
