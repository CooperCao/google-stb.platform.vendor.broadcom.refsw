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
//  Filename:       SnmpTrapExploderThread.cpp
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

// My api and definitions...
#include "SnmpTrapExploderThread.h"
#include "OperatingSystemFactory.h"
#include "BcmSnmpTrapHandler.h"
#include "SnmpLog.h"
#include "SnmpApi.h"

#include "SocketFactory.h"
#include "SocketSet.h"
#include "BcmV2Glue.h"

// For link status duties
#include "ifBridge.h"
#include "SystemTime.h"

// Some of our trap types
#if SNMP_PROXY_SUPPORT
#include "ocStbHostTrap.h"
#endif

// How often do we poll for link response?
#define LINK_CHECK_TIMEOUT_MS 1000

#define TRAP_PORT 162
#define TRAP_EXPLODER_SIGNAL_PORT (kSignalPort + 1)

BcmSnmpTrapExploderThread* BcmSnmpTrapExploderThread::fpSingleton = NULL;

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

BcmTrapHandlerDescriptor::BcmTrapHandlerDescriptor (BcmSnmpTrapHandler *pTrapHandler)
{
  // Store the agent
  fpTrapHandler = pTrapHandler;
  fpSocket = NULL;
}

BcmTrapHandlerDescriptor::~BcmTrapHandlerDescriptor ()
{
  Reset();

  if (fpSocket)
  {
    delete fpSocket;
    fpSocket = NULL;
  }
  
  fpTrapHandler = NULL;
}

bool BcmTrapHandlerDescriptor::Reset ()
{
  SnmpLogThd << "BcmTrapHandlerDescriptor::Reset" << endl;

  if (fpSocket) 
    if (fpSocket->Close() == 0) {
      SnmpLogThd << "  Closed socket OK." << endl;
      delete fpSocket;
      fpSocket = NULL;
    } else {
    SnmpLogThd << "  Socket close failed." << endl;
    }
    
  fBoundIp = kBroadcastIp;
  fBoundPort = 0;
  
  return true;
}
   
bool BcmTrapHandlerDescriptor::Bind ()
{
  SnmpLogThd << "BcmTrapHandlerDescriptor::Bind for " << fpTrapHandler->GetName() << endl;

  if ((fBoundPort == fpTrapHandler->GetUdpTrapPort()) 
  &&  (fBoundIp == fpTrapHandler->GetIpAddr()))
  {
    SnmpLogThd << "  IP and port the same, nothing to do." << endl;
    return true;
  }
  
  // Close our socket and reset our state.  
  Reset ();
  
  // Re-open the socket.  Again, note the use of OpenSnmpSocket to do this
  // which will get our socket registered with the thread's map.
  // delete fpSocket;
  // fpSocket = BcmSnmpTrapExploderThread::Singleton().OpenSnmpSocket(fpTrapHandler->GetIpAddr(), fpTrapHandler->GetUdpTrapPort());
  fpSocket = BcmSnmpTrapExploderThread::Singleton().OpenSnmpSocket(fpTrapHandler->GetIpAddr(), 0);
    
  if (fpSocket == NULL) {
    SnmpLogThd << "  Failed to create socket." << endl;
    return false;
  } else {
    SnmpLogThd << "  Created socket " << fpSocket->SocketDescriptor() << " OK." << endl;
  }

        
  // This module uses the SO_REUSEADDR and SO_REUSEPORT socket options, if
  // the OS has these defined. If not, blast a compiler warning, call it
  // a success, and move on.

  // Set the REUSEPORT and REUSEADDR options.  This is particularly necessary
  // for informs, because the inform code will reuse this socket.
  int Optval = 1;
#ifdef SO_REUSEPORT
  fpSocket->Setsockopt (SOL_SOCKET, SO_REUSEPORT, (char *)&Optval, sizeof (int));
#else
//#warning SO_REUSEPORT not defined!  Skipping and hoping for the best...
#endif
#ifdef SO_REUSEADDR
  fpSocket->Setsockopt (SOL_SOCKET, SO_REUSEADDR, (char *)&Optval, sizeof (int));
#else
//#warning SO_REUSEADDR not defined!  Skipping and hoping for the best...
#endif

    
  // Bind to the IP address and trap port.
  fBoundIp = fpTrapHandler->GetIpAddr();
  fBoundPort = fpTrapHandler->GetUdpTrapPort(); 
  if (fpSocket->Bind (fBoundIp, fBoundPort) == 0) {
    SnmpLogThd << "  Bound socket to " << fBoundIp << ":" << fBoundPort << " OK." << endl;
  } else {
    SnmpLogThd << "  Bind socket to " << fpTrapHandler->GetIpAddr() << ":" << fpTrapHandler->GetUdpTrapPort() << " failed." << endl;
    fpSocket->Close();
    fBoundIp = kBroadcastIp;
    fBoundPort = 0;
    return false;
  }
  
  return true;
}



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
BcmSnmpTrapExploderThread::BcmSnmpTrapExploderThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("SNMP Trap Exploder Thread", false, initialPriority)
{
    SnmpLogThd << "Creating BcmSnmpTrapExploderThread at " << (void *)this << endl;
    
    fMessageLogSettings.SetModuleName("BcmSnmpTrapExploderThread");

    fExitThread = false;
    
    // NULL out all our agent pointers.
    fNumTrapHandlers = 0;
    memset (fpTrapHandlers, 0, sizeof(fpTrapHandlers));
    
    fpSocketSet = NULL;
    fpSignalSocket = NULL;
    
    fMaxPacketSize = kDefaultRcvBufSize;

/*  Don't start the thread until we get our first active agent, since
    there wouldn't be any trap destination.
        
    // 6k stack should be sufficient...
    if (pfOperatingSystem->BeginThread(this, 6144) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmSnmpTrapExploderThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
*/
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
BcmSnmpTrapExploderThread::~BcmSnmpTrapExploderThread()
{
  Signal(kShutdown);
  
  // Wait for the thread to exit
  WaitForThread();
}


BcmSnmpTrapExploderThread &BcmSnmpTrapExploderThread::Singleton()
{
  if (fpSingleton == NULL)
    fpSingleton = new BcmSnmpTrapExploderThread();
    
  return *fpSingleton;
}


ostream &BcmSnmpTrapExploderThread::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  int Loop;
  SocketMap::const_iterator SockPos;

  switch (GetState())
  {
    case BcmOperatingSystem::kNotStarted:
      outStream << "SNMP Trap Exploder Thread (not started)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kInitializing:
      outStream << "SNMP Trap Exploder Thread (initializing)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kRunning:
      outStream << "SNMP Trap Exploder Thread (running)" << endl;
      outStream << "---------------------" << endl;
      break;
      
    case BcmOperatingSystem::kSuspended:
      outStream << "SNMP Trap Exploder Thread (suspended)" << endl;
      outStream << "-----------------------" << endl;
      break;
      
    case BcmOperatingSystem::kDeinitializing:
      outStream << "SNMP Trap Exploder Thread (de-initializing)" << endl;
      outStream << "----------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kTerminated:
      outStream << "SNMP Trap Exploder Thread (terminated)" << endl;
      outStream << "-----------------------" << endl;
      break;
      
    case BcmOperatingSystem::kInoperable:
      outStream << "SNMP Trap Exploder Thread (inoperable)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    default:
      outStream << "SNMP Trap Exploder Thread (unknown state)" << endl;
      outStream << "---------------------------" << endl;
      break;
  }
  
  outStream << "MAX PACKET SIZE = " << fMaxPacketSize << endl;

  if (fpSignalSocket)
  {
    outStream << "SIGNAL (127.0.0.1:" << (int) TRAP_EXPLODER_SIGNAL_PORT << ")" << endl;
    outStream << "   Socket = " << fpSignalSocket->SocketDescriptor() << endl;
    outStream << "   IsSet = " << fpSocketSet->IsSetRead (fpSignalSocket) << endl;
    
    // Check to see if this socket is mapped
    SockPos = fSockets.find(fpSignalSocket->SocketDescriptor());
    if (SockPos == fSockets.end())
      outStream << "   Mapped = false" << endl;
    else
      outStream << "   Mapped = true" << endl;
  }
  
  outStream << "TRAP HANDLERS (max = " << (int) kMaxSnmpTrapHandlers << ")" << endl;
  
  for (Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if (fpTrapHandlers[Loop])
    {
      outStream << Loop << ": "  << fpTrapHandlers[Loop]->fpTrapHandler->GetName() << endl;
      outStream << "   Master Agent " << fpTrapHandlers[Loop]->fpTrapHandler->GetMasterAgentName() << endl;
      outStream << "   Bound to " << fpTrapHandlers[Loop]->fBoundIp << ":" << fpTrapHandlers[Loop]->fBoundPort << endl;
      outStream << "   Socket = " << fpTrapHandlers[Loop]->fpSocket->SocketDescriptor() << endl;
      if (fpSocketSet)
        outStream << "   IsSet = " << fpSocketSet->IsSetRead (fpTrapHandlers[Loop]->fpSocket) << endl;
        
      // Check to see if this socket is mapped
      SockPos = fSockets.find(fpTrapHandlers[Loop]->fpSocket->SocketDescriptor());
      if (SockPos == fSockets.end())
        outStream << "   Mapped = false" << endl;
      else
        outStream << "   Mapped = true" << endl;
        
      outStream << "   Packets In  = " << fpTrapHandlers[Loop]->fPacketsIn << endl;
      outStream << "   Packets Out = " << fpTrapHandlers[Loop]->fPacketsOut << endl;
    }
  }
#endif
  return outStream;
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
bool BcmSnmpTrapExploderThread::Initialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    BcmIpAddress tmpIp;
    int tmpPort;
    
    // Create our socket set
    fpSocketSet = new BcmSocketSet ("SNMP Trap Exploder Socket Set");
    SnmpLogThd << "Trap Exploder Socket set created at " << (void *)fpSocketSet << endl;
    
    // Create our signal socket, it's on 127.0.0.1 using a private port.
    fpSignalSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SnmpLogThd << "  Got signal socket at " << (void *)fpSignalSocket << " = " << fpSignalSocket->SocketDescriptor() << endl;
    
    tmpIp = kSoftwareLoopbackIp;
    tmpPort = TRAP_EXPLODER_SIGNAL_PORT;
    if (fpSignalSocket->Bind (tmpIp, tmpPort) == 0)
    {
      SnmpLogThd << "  Bound signal socket OK" << endl;
      
      if (fpSocketSet->SetRead (fpSignalSocket) == true)
        SnmpLogThd << "  Set signal socket to SocketSet OK." << endl;
      else
        SnmpLogThd << "  Set signal socket to SocketSet failed." << endl;
    }
    else
    {
      SnmpLogThd << "  Signal bind failed." << endl;
      fpSignalSocket->Close();
    }
    
    return true;
}


// Thread destructor - this is the last method called when the thread is
// exiting, and is where the thread should delete all of the OS objects that
// it created.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmSnmpTrapExploderThread::Deinitialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // Clear and delete our socket set.
    if (fpSocketSet)
    {
      fpSocketSet->ClearAll();
      delete fpSocketSet;
    }
    
    // Delete the signal socket
    if (fpSignalSocket)
    {
      delete fpSignalSocket;
      fpSignalSocket = NULL;
    }
      
    // Delete any trap handler descriptors.
    for (int Loop=0; Loop < kMaxSnmpTrapHandlers; Loop++)
    {
      if (fpTrapHandlers[Loop])
      {
        delete fpTrapHandlers[Loop];
        fpTrapHandlers[Loop] = NULL;
      }
    }
}


BcmTrapHandlerDescriptor *BcmSnmpTrapExploderThread::FindDescriptor (const BcmSnmpTrapHandler *pHandler) const
{
  for (int Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if ((fpTrapHandlers[Loop] != NULL)
    &&  (fpTrapHandlers[Loop]->fpTrapHandler == pHandler))
      return fpTrapHandlers[Loop];
  }
  
  return NULL;
}


BcmTrapHandlerDescriptor *BcmSnmpTrapExploderThread::FindDescriptor (const BcmSocket *pSocket) const
{
  for (int Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if ((fpTrapHandlers[Loop] != NULL)
    &&  (fpTrapHandlers[Loop]->fpSocket == pSocket))
      return fpTrapHandlers[Loop];
  }
  
  return NULL;
}


BcmTrapHandlerDescriptor *BcmSnmpTrapExploderThread::FindDescriptor (int Socket) const
{
  for (int Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if ((fpTrapHandlers[Loop] != NULL)
    &&  (fpTrapHandlers[Loop]->fpSocket->SocketDescriptor() == Socket))
      return fpTrapHandlers[Loop];
  }
  
  return NULL;
}


bool BcmSnmpTrapExploderThread::RegisterTrapHandler (BcmSnmpTrapHandler *pHandler)
{
  // First see if we already have this trap handler.
  if (FindDescriptor (pHandler))
  {
    SnmpLogThd << "  Trap Handler already registered!!!" << endl;
    return true;
  }

  SnmpLogThd << "Trap Exploder thread registering trap handler " << pHandler->GetName() << endl;
  
  for (int Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if (fpTrapHandlers[Loop] == NULL)
    {
      // Store the trap handler
      SnmpLogThd << "  Found a spot at index " << Loop << endl;
      fpTrapHandlers[Loop] = new BcmTrapHandlerDescriptor (pHandler);
      fNumTrapHandlers++;
      
      // Get the trap handler a socket.  Note that we can't do this in the 
      // BcmTrapHandlerDescriptor any more because we're using OpenSnmpSocket
      // which needs the agent to already be in the fpTrapHandlers array in order
      // to resolve the IP stack instance to use.  We need to use 
      // OpenSnmpSocket so that the socket for our agent winds up in our
      // socket map so we can properly manage him.
      // fpTrapHandlers[Loop]->fpSocket = OpenSnmpSocket(pHandler->GetIpAddr(), pHandler->GetUdpTrapPort());
      // SnmpLogThd << "BcmTrapHandlerDescriptor for " << pHandler->GetName() << " got socket " << fpTrapHandlers[Loop]->fpSocket->SocketDescriptor() << endl;
      
      // If we are running, signal our main loop to update agents
      if (GetState() != BcmOperatingSystem::kNotStarted)
        Signal(kUpdateTrapHandlers);
      // Otherwise, call the operation directly.
      else
        UpdateTrapHandlers();
        
      return true;
    }
  }
  
  SnmpLogThd << "  No room in the inn!" << endl;
  return false;
}


bool BcmSnmpTrapExploderThread::TrapHandlerStateChanged (BcmSnmpTrapHandler *pHandler)
{
  // If we are running, signal our main loop to update agents
  if (GetState() != BcmOperatingSystem::kNotStarted)
    Signal(kUpdateTrapHandlers);
  // Otherwise, call the operation directly.
  else
    UpdateTrapHandlers();
    
  return true;
}


int BcmSnmpTrapExploderThread::NumTrapHandlers () const
{
  return fNumTrapHandlers;
}

BcmSnmpTrapHandler *BcmSnmpTrapExploderThread::GetTrapHandler (int Index) const
{
  if (fpTrapHandlers[Index] == NULL)
    return NULL;
    
  return fpTrapHandlers[Index]->fpTrapHandler;
}

BcmSnmpTrapHandler *BcmSnmpTrapExploderThread::FindTrapHandlerByName (const BcmString &Name) const
{
  int Loop;
  BcmSnmpTrapHandler *pHandler = NULL;
  
  for (Loop = 0; Loop < NumTrapHandlers(); Loop++)
  {
    pHandler = GetTrapHandler(Loop);
    if ((pHandler) && (pHandler->GetName() == Name))
      return pHandler;
  }
  
  return NULL;
}

BcmSnmpTrapHandler *BcmSnmpTrapExploderThread::FindTrapHandlerByIp (const BcmIpAddress &Ip) const
{
  int Loop;
  BcmSnmpTrapHandler *pHandler = NULL;
  
  // zero ip address will return the first Handler
  if (Ip == BcmIpAddress(0,0,0,0))
    return GetTrapHandler(0);
    
  for (Loop = 0; Loop < NumTrapHandlers(); Loop++)
  {
    pHandler = GetTrapHandler(Loop);
    if ((pHandler) && (pHandler->GetIpAddr() == Ip))
      return pHandler;
  }
  
  return NULL;
}

bool BcmSnmpTrapExploderThread::Signal (SnmpTrapExploderThreadSignal Action, void *pPayload)
{
  SnmpTrapExploderThreadSignalPayload Payload;
  int NumBytes = 0;
  
  Payload.Signal = Action;
  Payload.pPayload = pPayload;
  
  // Create a socket and send a buffer to the loopback IP (127.0.0.1) on the 
  // signal port.
  BcmSocket *pSocket = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  SnmpLogThd << "BcmSnmpTrapExploderThread::Signal got socket " << pSocket->SocketDescriptor() << endl;
  
  NumBytes = pSocket->Sendto ((char *)&Payload, sizeof(SnmpTrapExploderThreadSignalPayload), 0, kSoftwareLoopbackIp, (short) TRAP_EXPLODER_SIGNAL_PORT);
  SnmpLogThd << "  Sent " << NumBytes << " bytes." << endl;
  
  delete pSocket;
  
  // Yield the processor here just for a bit.  Give our thread a chance
  // to process this signal.
  BcmOperatingSystemFactory::ThreadSleep (100);

  return true;
}


bool BcmSnmpTrapExploderThread::ProcessSignal (SnmpTrapExploderThreadSignal Action, void *pPayload)
{
  bool Processed = true;
  
  SnmpLogThd << "BcmSnmpTrapExploderThread::ProcessSignal " << (int) Action << ", " << pPayload << endl;

  switch (Action)
  {
    case kShutdown:
      fExitThread = true;
      break;
      
    case kUpdateTrapHandlers:
      UpdateTrapHandlers();
      break;
  
    default:
      Processed = false;
  }
  
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
void BcmSnmpTrapExploderThread::ThreadMain(void)
{
    BcmIpAddress FromIp;  
    uint16 FromPort;  
    int Loop, BytesRead = 0;

    
    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;
    
    while (1)
    {
      /*\
       *  Call Select, we will block here until a packet comes in on one
       *  of our sockets, either an SNMP socket or the internal signal
       *  socket.  Or, we will timeout to allow for a link status poll.
      \*/
      
      /*\
       *  Make sure our socket set has everyone listening.
      \*/
      
      UpdateSocketSet ();
      
      // Select returns 1 if we hear something and 0 if we timeout.
      // Use a timeout for oper status poll for linkUp/linkDown traps.
      if (fpSocketSet->Select(LINK_CHECK_TIMEOUT_MS) == 0)
      {
        // If our select call times out, we have not just received a packet.
        // No need to do anything else below, just continue our loop.
        continue;
      }
      
      /*\
       *  Process any received SNMP packets that might have caused our select
       *  call to unblock.  Do this first before checking for signals.  In the
       *  unlikely event that we have both SNMP packets to process and a
       *  signal, this will ensure that we get the packets off the wire
       *  before we go mucking around with the agent sockets, shutting down, etc.
      \*/
      
      // Did any of the agents fire?
      for (Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
      {
        if (fpTrapHandlers[Loop] != NULL)
        {
          if (fpSocketSet->IsSetRead (fpTrapHandlers[Loop]->fpSocket))
          {
            unsigned long SrcIp, DstIp;
            
            SnmpLogThd << fpTrapHandlers[Loop]->fpTrapHandler->GetName() << " socket fired" << endl;
            
            fpTrapHandlers[Loop]->fpTrapHandler->GetIpAddr().Get(DstIp);
            
            // read the data off the wire
            
            // NOTE: this buffer must be freed by the agent in the
            // HandlerProcessSnmpPacket call.  Some agents may not copy the
            // buffer, so we must not free it here.
            unsigned char *pRcvBuf = new unsigned char[fMaxPacketSize];
            
            // Read the buffer off the wire.
            BytesRead = RecvfromSnmpSocket (fpTrapHandlers[Loop]->fpSocket->SocketDescriptor(), pRcvBuf, fMaxPacketSize, FromIp, FromPort);
              
            SnmpLogThd << "  call agent to send the correct trap(s)." << endl;
            // Call the agent to deal with this packet.
            FromIp.Get(SrcIp);
            // AgentProcessSnmpPacket (pRcvBuf, BytesRead, fpTrapHandlers[Loop]->fpSocket->SocketDescriptor(),
            // SrcIp, DstIp, 
            // FromPort, fpTrapHandlers[Loop]->fpTrapHandler->GetUdpTrapPort());
            BcmSnmpAgent *pAgent = fpTrapHandlers[Loop]->fpTrapHandler->MasterAgent();

            int retval = GetSnmpV2cTrapType(pRcvBuf, BytesRead);
#if SNMP_PROXY_SUPPORT
            if (retval == BCMV2_TRAP_TYPE_OCSTBPANICDUMP) {
              BcmTrap *pTrap = new ocStbPanicDumpTrap(pAgent);
              pAgent->SendTrap(pTrap);
              delete (pTrap);
            }
#endif

            delete (pRcvBuf);       
          }
        }
      }
      
      /*\
       *  Check for a signal on the signal socket and process it if we have
       *  one.
      \*/
      
      if (fpSocketSet->IsSetRead(fpSignalSocket))
      {
        SnmpTrapExploderThreadSignalPayload Payload;
        
        memset (&Payload, 0, sizeof (SnmpTrapExploderThreadSignalPayload));
        
        // Read the buffer off the wire.
        BytesRead = fpSignalSocket->Recvfrom((char *)&Payload, (int) sizeof(SnmpTrapExploderThreadSignalPayload), 0, FromIp, FromPort);
        SnmpLogThd << "Signal socket fired, " << BytesRead << " bytes on port " << FromPort << " from " << FromIp << "." << endl;
        
        ProcessSignal (Payload.Signal, Payload.pPayload);
      }
      
      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    } // while (1)

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


int BcmSnmpTrapExploderThread::UpdateTrapHandlers ()
{
  int ActiveHandlers = 0, Loop;
  
  SnmpLogThd << "BcmSnmpTrapExploderThread::UpdateTrapHandlers" << endl;
  
  for (Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    // Look for agents which need rebinding and rebind them.
    if (fpTrapHandlers[Loop] != NULL)
    {
      if ((fpTrapHandlers[Loop]->fBoundPort != fpTrapHandlers[Loop]->fpTrapHandler->GetUdpTrapPort())
      ||  (fpTrapHandlers[Loop]->fBoundIp != fpTrapHandlers[Loop]->fpTrapHandler->GetIpAddr()))
      {
        SnmpLogThd << "  rebinding " << fpTrapHandlers[Loop]->fpTrapHandler->GetName() << endl;
      
        fpTrapHandlers[Loop]->Bind();
        if ((fpTrapHandlers[Loop]->fBoundIp != kBroadcastIp)
        &&  (fpTrapHandlers[Loop]->fBoundPort != 0))
          ActiveHandlers++;
      }
    }
  }
  
  // If this is our first active agent, kick off the thread.
  if ((GetState() == BcmOperatingSystem::kNotStarted)
  &&  (ActiveHandlers > 0))
  {
    SnmpLogRaw << "BcmSnmpTrapExploderThread starting thread operation." << endl;
    
    // Need more stack for the agent -try 14k for now...
    //if (pfOperatingSystem->BeginThread(this, 14*1024) == false)
    // Need a little more.
    if (pfOperatingSystem->BeginThread(this, 64*1024) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmSnmpTrapExploderThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
  }
  
  return ActiveHandlers;
}


void BcmSnmpTrapExploderThread::UpdateSocketSet ()
{
  int Loop;

  //SnmpLogThd << "Resetting socket set." << endl;

  fpSocketSet->ClearAll();
  
  if (fpSignalSocket->SocketDescriptor() > 0)
    fpSocketSet->SetRead (fpSignalSocket);
    
  for (Loop = 0; Loop < kMaxSnmpTrapHandlers; Loop++)
  {
    if (fpTrapHandlers[Loop])
    {
      if ((fpTrapHandlers[Loop]->fBoundIp != kBroadcastIp)
      &&  (fpTrapHandlers[Loop]->fBoundPort != 0))
        fpSocketSet->SetRead (fpTrapHandlers[Loop]->fpSocket);
    }
  }
}

// Open a socket.  If a port is specified, then the socket will be bound
// to that UDP port on the specified IP address.  Thus this socket is
// usable as a 'server' socket in a subsequent recvfrom call.  If no
// port is specified, then the IP address argument will be used to determine
// the IP stack with which this socket will be associated.  If no match
// can be found, then the socket will be created without specifying an IP 
// stack and the caller will just have to take his chances.
BcmSocket *BcmSnmpTrapExploderThread::OpenSnmpSocket (const BcmIpAddress &Ip, int Port)
{
  BcmSnmpTrapHandler *pHandler = FindTrapHandlerByIp (Ip);
  BcmIpAddress tmpIp;
  
  
  SnmpLogThd << "TrapExploderThread::OpenSnmpSocket for " << Ip << ":" << Port << endl;
  
  if (pHandler)
  {
    
    SnmpLogThd << "  For this IP address, we already have trap handler " << pHandler->GetName() << endl;
    // If, by chance, the requested socket is for the port that this
    // handler is already bound to, then just return his socket.
    if (Port)
    {
      BcmTrapHandlerDescriptor *pDescriptor = FindDescriptor (pHandler);
      if (pDescriptor)
      {
    SnmpLogThd << "  For this port, we already have a trap handler descriptor" << endl;
        if ((pDescriptor->fBoundPort == Port)
        &&  (pDescriptor->fpSocket))
        {
          SnmpLogThd << "  Socket is for IP/Port already in use by trap handler " << pHandler->GetName() << endl;
          SnmpLogThd << "  Returning socket " << pDescriptor->fpSocket->SocketDescriptor() << endl;
          return pDescriptor->fpSocket;
        }
      }
    }
  }
  
    SnmpLogThd << "  Calling NewSocket() for trap handler " << pHandler->GetName() << endl;
  BcmSocket *pSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP, (int) 0);
  
  if (pSocket == NULL)
  {
    SnmpLogThd << "  NULL socket!" << endl;
    return NULL;
  }

  SnmpLogThd << "  Got an new socket for trap handler " << pHandler->GetName() << endl;
// This module uses the SO_REUSEADDR and SO_REUSEPORT socket options, if
// the OS has these defined. If not, blast a compiler warning, call it
// a success, and move on.

    // Set the REUSEPORT and REUSEADDR options.  This is particularly necessary
    // for informs, because the inform code will reuse this socket.
    int Optval = 1;
#ifdef SO_REUSEPORT
    pSocket->Setsockopt (SOL_SOCKET, SO_REUSEPORT, (char *)&Optval, sizeof (int));
#else
//#warning SO_REUSEPORT not defined!  Skipping and hoping for the best...
#endif
#ifdef SO_REUSEADDR
    pSocket->Setsockopt (SOL_SOCKET, SO_REUSEADDR, (char *)&Optval, sizeof (int));
#else
//#warning SO_REUSEADDR not defined!  Skipping and hoping for the best...
#endif


  if (Port)
  {
    int ret = ( pSocket->Bind (tmpIp, Port)); 
    if (ret != 0) {
      SnmpLogThd << "   Bind Failed <"<<ret<<">"<<endl;
      delete pSocket;
      return NULL;
    }
  }
  SnmpLogThd << "  Got socket " << pSocket->SocketDescriptor() << endl;
  
  // Remember this socket in our map.
  fSockets[pSocket->SocketDescriptor()] = pSocket;
    
  return pSocket;
}

// Close the specified socket.
int BcmSnmpTrapExploderThread::CloseSnmpSocket (int SocketDescriptor)
{
  SocketMap::iterator Pos = fSockets.find(SocketDescriptor);
  
  
  SnmpLogThd << "BcmSnmpTrapExploderThread::CloseSnmpSocket for socket " << SocketDescriptor << endl;
  
  if (Pos == fSockets.end())
  {
    SnmpLogThd << "  No match!" << endl;
    return -1;
  }
    
  BcmSocket *pSocket = (*Pos).second;
  
  // Make sure this socket doesn't belong to one of our agents.  If it
  // does, don't close it but return 0 so the agent core doesn't try to
  // close it either.
  BcmTrapHandlerDescriptor *pDescriptor = FindDescriptor (pSocket);
  if (pDescriptor)
  {
    SnmpLogThd << "  (socket belongs to " << pDescriptor->fpTrapHandler->GetName() << ", only pretending to close...)" << endl;
    return 0;
  }
  
  SnmpLogThd << "  Found socket, closing and deleting." << endl;
  
  fSockets.erase (Pos);
  pSocket->Close();
  
  delete pSocket;
  
  return 0;
}


// Handle internal "TrapPacketPointer" conversions for giganto-packets.
// If the packet passed in pPkt is determined to be a TrapPacketPointer, then
// the address of the pointed-to-pcaket will be returned and Length
// will be updated with the length of the pointed-to-packet.  If it's not
// a TrapPacketPointer, NULL is returned.
unsigned char *BcmSnmpTrapExploderThread::ConvertFromTrapPacketPointer (const unsigned char *pPkt, int &Length) const
{
  TrapPacketPointer *pPktPointer;
  
  // If our length isn't right, then this is obviously not a packet pointer
  // and we can return.
  if (Length != sizeof (TrapPacketPointer))
    return NULL;
    
  // Cast as a TrapPacketPointer
  pPktPointer = (TrapPacketPointer *) pPkt;
  
  // Check magic number.
  if (pPktPointer->Magic != TRAP_PACKETPOINTER_MAGIC_NUMBER)
    return NULL;
    
  // OK, we got ourselves a packet-pointer.
  SnmpLogThd << "  TrapPacketPointer detected." << endl;
  
  Length = pPktPointer->Length;
  return pPktPointer->pData;
}


int BcmSnmpTrapExploderThread::RecvfromSnmpSocket (int Socket, unsigned char *pRcvBuf, int BufSize, BcmIpAddress &FromIp, unsigned short &FromPort) const
{
  int BytesRead;
  
  SnmpLogThd << "BcmSnmpTrapExploderThread::RecvfromSnmpSocket into " << BufSize << " byte buffer @ " << inHex(pRcvBuf) << endl;
  
  // First, see if the socket in question is one of ours.
  BcmTrapHandlerDescriptor *pDescriptor = FindDescriptor (Socket);
  
  if (pDescriptor == NULL)
  {
    SnmpLogThd << "  Couldn't find socket " << Socket << endl;
    return -1;
  }

  // Read the buffer off the wire.
  BytesRead = pDescriptor->fpSocket->Recvfrom((char *)pRcvBuf, BufSize, 0, FromIp, FromPort);
  pDescriptor->fPacketsIn++;
  SnmpLogThd << "  Received " << BytesRead << " bytes from " << FromIp << ":" << FromPort << BcmOctetBuffer (pRcvBuf, BytesRead, BytesRead, false) << endl;
  
  // Check for a packet-pointer and convert if necessary.  Only do 
  // this on packets received locally - from 127.0.0.1.  This is 
  // used to pass over internal SNMP packets which are bigger than
  // our OS / IP stack can handle - eg DOCSIS config file packets with
  // lots and lots of varbinds.
  if ((BytesRead != -1) && (FromIp == kSoftwareLoopbackIp))
  {
    unsigned char *pPktPointer = NULL;
    
    pPktPointer = ConvertFromTrapPacketPointer (pRcvBuf, BytesRead);
    if (pPktPointer)
    {
      int CopyBytes = BytesRead;
    
      SnmpLogThd << "  Converting TrapPacketPointer." << endl;
      
      if (CopyBytes > BufSize)
      {
        gLogMessageRaw << "WARNING: SNMP packet is larger than available buffer (" << CopyBytes << " vs. " << BufSize << ") - truncating!" << endl;
        BufSize = CopyBytes;
      }
      
      // Copy into destination buffer, up to size permitted.
      memcpy (pRcvBuf, pPktPointer, CopyBytes);
      SnmpLogThd << "  Converted to " << BytesRead << " bytes from " << FromIp << ":" << FromPort << BcmOctetBuffer (pRcvBuf, BytesRead, BytesRead, false) << endl;
      
      // Delete original packet.
      delete pPktPointer;
    }
  }
  
  return BytesRead;
}


int BcmSnmpTrapExploderThread::SendtoSnmpSocket (int Socket, const unsigned char *pBuf, int BufSize, const BcmIpAddress &ToIp, unsigned short &ToPort) const
{
  int BytesWrote;
  
  
  SnmpLogThd << "BcmSnmpTrapExploderThread::SendtoSnmpSocket from " << BufSize << " byte buffer @ " << inHex(pBuf) << endl;
  
  // First, see if the socket in question is one of ours.
  BcmTrapHandlerDescriptor *pDescriptor = FindDescriptor (Socket);
  
  if (pDescriptor == NULL)
  {
    SnmpLogThd << "  Couldn't find socket " << Socket << endl;
    return -1;
  }

  // Write the buffer to the wire.
  BytesWrote = pDescriptor->fpSocket->Sendto((char *)pBuf, BufSize, 0, ToIp, ToPort);
  pDescriptor->fPacketsOut++;
  SnmpLogThd << "  Sent " << BytesWrote << " bytes to " << ToIp << ":" << ToPort << BcmOctetBuffer ((unsigned char *)pBuf, BytesWrote, BytesWrote, false) << endl;

  return BytesWrote;
}
