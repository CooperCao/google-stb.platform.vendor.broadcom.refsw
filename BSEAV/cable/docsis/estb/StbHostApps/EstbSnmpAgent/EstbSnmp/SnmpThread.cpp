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

// My api and definitions...
#include "SnmpThread.h"
#include "OperatingSystemFactory.h"
#include "BcmSnmpAgent.h"
#include "SnmpLog.h"
#include "SnmpApi.h"

#include "SocketFactory.h"
#include "SocketSet.h"

// For link status duties
#include "ifBridge.h"
#include "SystemTime.h"


// How often do we poll for linkUp/linkDown traps?
#define LINK_CHECK_TIMEOUT_MS 1000

BcmSnmpThread* BcmSnmpThread::fpSingleton = NULL;

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************

BcmAgentDescriptor::BcmAgentDescriptor (BcmSnmpAgent *pAgent)
{
  // Store the agent
  fpAgent = pAgent;

  fpSocket = NULL;
  fBoundIp = kBroadcastIp;
  fBoundPort = 0;
  fPacketsIn = 0;
  fPacketsOut = 0;
}

BcmAgentDescriptor::~BcmAgentDescriptor ()
{
  Reset();

  if (fpSocket)
  {
    delete fpSocket;
    fpSocket = NULL;
  }
  
  fpAgent = NULL;
}

bool BcmAgentDescriptor::Reset ()
{
  SnmpLogThd << "BcmAgentDescriptor::Reset" << endl;

  if (fpSocket->Close() == 0)
  {
    SnmpLogThd << "  Closed socket OK." << endl;
  }
  else
  {
    SnmpLogThd << "  Socket close failed." << endl;
  }
    
  fBoundIp = kBroadcastIp;
  fBoundPort = 0;
  
  return true;
}
   
bool BcmAgentDescriptor::Bind ()
{
  SnmpLogThd << "BcmAgentDescriptor::Bind " << fpAgent->GetName() << endl;

  if ((fpAgent->GetUdpPort() == fBoundPort) 
  &&  (fpAgent->GetIpAddr() == fBoundIp))
  {
    SnmpLogThd << "  IP and port the same, nothing to do." << endl;
    return true;
  }
  
  // Close our socket and reset our state.  
  Reset ();
  
  if (fpAgent->GetIpAddr() != kBroadcastIp)
  {
    // Re-open the socket.  Again, note the use of OpenSnmpSocket to do this
    // which will get our socket registered with the thread's map.
    delete fpSocket;
    fpSocket = BcmSnmpThread::Singleton().OpenSnmpSocket(fpAgent->GetIpAddr());
    
    if (fpSocket == NULL)
    {
      SnmpLogThd << "  Failed to create socket." << endl;
      return false;
    }
    else
    {
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

    
    // Bind to the agent's IP address and UDP port.
    fBoundIp = fpAgent->GetIpAddr();
    fBoundPort = fpAgent->GetUdpPort();
    if (fpSocket->Bind (fBoundIp, fBoundPort) == 0)
    {
      SnmpLogThd << "  Bound socket to " << fBoundIp << ":" << fBoundPort << " OK." << endl;
    }
    else
    {
      SnmpLogThd << "  Bind socket to " << fpAgent->GetIpAddr() << ":" << fpAgent->GetUdpPort() << " failed." << endl;
      fpSocket->Close();
      fBoundIp = kBroadcastIp;
      fBoundPort = 0;
      return false;
    }
  }
  else
  {
    SnmpLogThd << " IP is 0.0.0.0, not binding socket." << endl;
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
BcmSnmpThread::BcmSnmpThread(BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread("SNMP Thread", false, initialPriority)
{
    SnmpLogThd << "Creating BcmSnmpThread at " << (void *)this << endl;
    
    fMessageLogSettings.SetModuleName("BcmSnmpThread");

    fExitThread = false;
    
    // NULL out all our agent pointers.
    fNumAgents = 0;
    memset (fpAgents, 0, sizeof(fpAgents));
    
    fpSocketSet = NULL;
    fpSignalSocket = NULL;
    
    MaxPacketSize (kDefaultRcvBufSize);

/*  Don't start the thread until we get our first active agent bound to a real
    IP address.  This is helpful for new OS's where the IP stack may not be
    quite really up yet and we don't have support for fancy things like
    127.0.0.1 loopback, etc. yet.
        
    // 6k stack should be sufficient...
    if (pfOperatingSystem->BeginThread(this, 6144) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmSnmpThread" ) 
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
BcmSnmpThread::~BcmSnmpThread()
{
  Signal(kShutdown);
  
  // Wait for the thread to exit
  WaitForThread();
}


BcmSnmpThread &BcmSnmpThread::Singleton()
{
  if (fpSingleton == NULL)
    fpSingleton = new BcmSnmpThread();
    
  return *fpSingleton;
}


ostream &BcmSnmpThread::Print(ostream &outStream) const
{
#if (!BCM_REDUCED_IMAGE_SIZE)
  int Loop;
  SocketMap::const_iterator SockPos;

  switch (GetState())
  {
    case BcmOperatingSystem::kNotStarted:
      outStream << "SNMP Thread (not started)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kInitializing:
      outStream << "SNMP Thread (initializing)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kRunning:
      outStream << "SNMP Thread (running)" << endl;
      outStream << "---------------------" << endl;
      break;
      
    case BcmOperatingSystem::kSuspended:
      outStream << "SNMP Thread (suspended)" << endl;
      outStream << "-----------------------" << endl;
      break;
      
    case BcmOperatingSystem::kDeinitializing:
      outStream << "SNMP Thread (de-initializing)" << endl;
      outStream << "----------------------------" << endl;
      break;
      
    case BcmOperatingSystem::kTerminated:
      outStream << "SNMP Thread (terminated)" << endl;
      outStream << "-----------------------" << endl;
      break;
      
    case BcmOperatingSystem::kInoperable:
      outStream << "SNMP Thread (inoperable)" << endl;
      outStream << "-------------------------" << endl;
      break;
      
    default:
      outStream << "SNMP Thread (unknown state)" << endl;
      outStream << "---------------------------" << endl;
      break;
  }
  
  outStream << "MAX PACKET SIZE = " << MaxPacketSize() << endl;

  if (fpSignalSocket)
  {
    outStream << "SIGNAL (127.0.0.1:" << (int)kSignalPort << ")" << endl;
    outStream << "   Socket = " << fpSignalSocket->SocketDescriptor() << endl;
    outStream << "   IsSet = " << fpSocketSet->IsSetRead (fpSignalSocket) << endl;
    
    // Check to see if this socket is mapped
    SockPos = fSockets.find(fpSignalSocket->SocketDescriptor());
    if (SockPos == fSockets.end())
      outStream << "   Mapped = false" << endl;
    else
      outStream << "   Mapped = true" << endl;
  }
  
  outStream << "AGENTS (max = " << (int) kMaxSnmpAgents << ")" << endl;
  
  for (Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if (fpAgents[Loop])
    {
      outStream << Loop << ": "  << fpAgents[Loop]->fpAgent->GetName() << endl;
      outStream << "   Bound to " << fpAgents[Loop]->fBoundIp << ":" << fpAgents[Loop]->fBoundPort << endl;
      outStream << "   Socket = " << fpAgents[Loop]->fpSocket->SocketDescriptor() << endl;
      if (fpSocketSet)
        outStream << "   IsSet = " << fpSocketSet->IsSetRead (fpAgents[Loop]->fpSocket) << endl;
        
      // Check to see if this socket is mapped
      SockPos = fSockets.find(fpAgents[Loop]->fpSocket->SocketDescriptor());
      if (SockPos == fSockets.end())
        outStream << "   Mapped = false" << endl;
      else
        outStream << "   Mapped = true" << endl;
        
      outStream << "   Packets In  = " << fpAgents[Loop]->fPacketsIn << endl;
      outStream << "   Packets Out = " << fpAgents[Loop]->fPacketsOut << endl;
    }
  }
#endif
  return outStream;
}

int BcmSnmpThread::MaxPacketSize () const
{
  return fMaxPacketSize;
}

bool BcmSnmpThread::MaxPacketSize (int MaxSize)
{
  fMaxPacketSize = AgentSetMaxPacketSize(MaxSize);
  
  if (fMaxPacketSize != MaxSize)
    return false;
     
  return true;
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
bool BcmSnmpThread::Initialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    BcmIpAddress tmpIp;
    int tmpPort;
    
    // Create our socket set
    fpSocketSet = new BcmSocketSet ("SNMP Socket Set");
    SnmpLogThd << "  Socket set created at " << (void *)fpSocketSet << endl;
    
    // Create our signal socket, it's on 127.0.0.1 using a private port.
    fpSignalSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SnmpLogThd << "  Got signal socket at " << (void *)fpSignalSocket << " = " << fpSignalSocket->SocketDescriptor() << endl;
    
    tmpIp = kSoftwareLoopbackIp;
    tmpPort = kSignalPort;
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
void BcmSnmpThread::Deinitialize(void)
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
      
    // Delete any agent descriptors.
    for (int Loop=0; Loop < kMaxSnmpAgents; Loop++)
    {
      if (fpAgents[Loop])
      {
        delete fpAgents[Loop];
        fpAgents[Loop] = NULL;
      }
    }
}


BcmAgentDescriptor *BcmSnmpThread::FindDescriptor (const BcmSnmpAgent *pAgent) const
{
  for (int Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if ((fpAgents[Loop] != NULL)
    &&  (fpAgents[Loop]->fpAgent == pAgent))
      return fpAgents[Loop];
  }
  
  return NULL;
}


BcmAgentDescriptor *BcmSnmpThread::FindDescriptor (const BcmSocket *pSocket) const
{
  for (int Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if ((fpAgents[Loop] != NULL)
    &&  (fpAgents[Loop]->fpSocket == pSocket))
      return fpAgents[Loop];
  }
  
  return NULL;
}


BcmAgentDescriptor *BcmSnmpThread::FindDescriptor (int Socket) const
{
  for (int Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if ((fpAgents[Loop] != NULL)
    &&  (fpAgents[Loop]->fpSocket->SocketDescriptor() == Socket))
      return fpAgents[Loop];
  }
  
  return NULL;
}


bool BcmSnmpThread::RegisterAgent (BcmSnmpAgent *pAgent)
{
  // First see if we already have this agent.
  if (FindDescriptor (pAgent))
  {
    SnmpLogThd << "  Agent already registered." << endl;
    return true;
  }

  SnmpLogThd << "SNMP thread registering agent " << pAgent->GetName() << endl;
  
  for (int Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if (fpAgents[Loop] == NULL)
    {
      // Store the agent
      SnmpLogThd << "  Found a spot at index " << Loop << endl;
      fpAgents[Loop] = new BcmAgentDescriptor (pAgent);
      fNumAgents++;
      
      // Get the agent a socket.  Note that we can't do this in the 
      // BcmAgentDescriptor any more because we're using OpenSnmpSocket
      // which needs the agent to already be in the fpAgents array in order
      // to resolve the IP stack instance to use.  We need to use 
      // OpenSnmpSocket so that the socket for our agent winds up in our
      // socket map so we can properly manage him.
      fpAgents[Loop]->fpSocket = OpenSnmpSocket(pAgent->GetIpAddr());
      SnmpLogThd << "BcmAgentDescriptor for " << pAgent->GetName() << " got socket " << fpAgents[Loop]->fpSocket->SocketDescriptor() << endl;
      
      // If we are running, signal our main loop to update agents
      if (GetState() != BcmOperatingSystem::kNotStarted)
        Signal(kUpdateAgents);
      // Otherwise, call the operation directly.
      else
        UpdateAgents();
        
      return true;
    }
  }
  
  SnmpLogThd << "  No room in the inn!" << endl;
  return false;
}


bool BcmSnmpThread::AgentStateChanged (BcmSnmpAgent *pAgent)
{
  // If we are running, signal our main loop to update agents
  if (GetState() != BcmOperatingSystem::kNotStarted)
    Signal(kUpdateAgents);
  // Otherwise, call the operation directly.
  else
    UpdateAgents();
    
  return true;
}


int BcmSnmpThread::NumAgents () const
{
  return fNumAgents;
}

BcmSnmpAgent *BcmSnmpThread::GetAgent (int Index) const
{
  if (fpAgents[Index] == NULL)
    return NULL;
    
  return fpAgents[Index]->fpAgent;
}

BcmSnmpAgent *BcmSnmpThread::FindAgentByIf (int Interface) const
{
  int Loop;
  BcmSnmpAgent *pAgent = NULL;
  
  // Special case: if Interface == -1, just return the first agent.
  if (Interface == -1)
    return GetAgent(0);
  
  for (Loop = 0; Loop < NumAgents(); Loop++)
  {
    pAgent = GetAgent(Loop);
    if ((pAgent) && (pAgent->GetIfNum() == Interface))
      return pAgent;
  }
  
  return NULL;
}

BcmSnmpAgent *BcmSnmpThread::FindAgentByName (const BcmString &Name) const
{
  int Loop;
  BcmSnmpAgent *pAgent = NULL;
  
  for (Loop = 0; Loop < NumAgents(); Loop++)
  {
    pAgent = GetAgent(Loop);
    if ((pAgent) && (pAgent->GetName() == Name))
      return pAgent;
  }
  
  return NULL;
}

BcmSnmpAgent *BcmSnmpThread::FindAgentByIp (const BcmIpAddress &Ip) const
{
  int Loop;
  BcmSnmpAgent *pAgent = NULL;
  
  // zero ip address will return the first Agent
  if (Ip == BcmIpAddress(0,0,0,0))
    return GetAgent(0);
    
  for (Loop = 0; Loop < NumAgents(); Loop++)
  {
    pAgent = GetAgent(Loop);
    if ((pAgent) && (pAgent->GetIpAddr() == Ip))
      return pAgent;
  }
  
  return NULL;
}

BcmSnmpAgent *BcmSnmpThread::FindAgentBySubnet (const BcmIpAddress &Subnet) const
{
  int Loop;
  BcmSnmpAgent *pAgent = NULL;
  
  for (Loop = 0; Loop < NumAgents(); Loop++)
  {
    pAgent = GetAgent(Loop);
    if ((pAgent) && (pAgent->GetSubnetMask() == Subnet))
      return pAgent;
  }
  
  return NULL;
}


bool BcmSnmpThread::Signal (SnmpThreadSignal Action, void *pPayload)
{
  SnmpThreadSignalPayload Payload;
  int NumBytes = 0;
  
  Payload.Signal = Action;
  Payload.pPayload = pPayload;
  
  // Create a socket and send a buffer to the loopback IP (127.0.0.1) on the 
  // signal port.
  BcmSocket *pSocket = BcmSocketFactory::NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  SnmpLogThd << "BcmSnmpThread::Signal got socket " << pSocket->SocketDescriptor() << endl;
  
  NumBytes = pSocket->Sendto ((char *)&Payload, sizeof(SnmpThreadSignalPayload), 0, kSoftwareLoopbackIp, (short)kSignalPort);
  SnmpLogThd << "  Sent " << NumBytes << " bytes." << endl;
  
  delete pSocket;
  
  // Yield the processor here just for a bit.  Give our thread a chance
  // to process this signal.
  BcmOperatingSystemFactory::ThreadSleep (100);

  return true;
}


bool BcmSnmpThread::ProcessSignal (SnmpThreadSignal Action, void *pPayload)
{
  bool Processed = true;
  
  SnmpLogThd << "BcmSnmpThread::ProcessSignal " << (int) Action << ", " << pPayload << endl;

  switch (Action)
  {
    case kShutdown:
      fExitThread = true;
      break;
      
    case kUpdateAgents:
      UpdateAgents();
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
void BcmSnmpThread::ThreadMain(void)
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
        // So don't process anything, but do take this opportunity to check
        // link up/down status for all managed interfaces.
        CheckLinkStatus ();
        
        // No need to do anything else below, just continue our loop.
        continue;
      }
      
      // Just in case we're getting a lot of SNMP traffic and we're never
      // hitting the timeout above, check the current system time and
      // if it's been longer than our timeout value since we last checked,
      // go ahead and check link status now.
      if (SystemTimeMS() > (fLastLinkCheck + LINK_CHECK_TIMEOUT_MS))
        CheckLinkStatus();
        
      /*\
       *  Process any received SNMP packets that might have caused our select
       *  call to unblock.  Do this first before checking for signals.  In the
       *  unlikely event that we have both SNMP packets to process and a
       *  signal, this will ensure that we get the packets off the wire
       *  before we go mucking around with the agent sockets, shutting down, etc.
      \*/
      
      // Did any of the agents fire?
      for (Loop = 0; Loop < kMaxSnmpAgents; Loop++)
      {
        if (fpAgents[Loop] != NULL)
        {
          if (fpSocketSet->IsSetRead (fpAgents[Loop]->fpSocket))
          {
            unsigned long SrcIp, DstIp;
            
            SnmpLogThd << fpAgents[Loop]->fpAgent->GetName() << " socket fired" << endl;
            
            fpAgents[Loop]->fpAgent->GetIpAddr().Get(DstIp);
            
            // Give the agent an opportunity to read the data off the wire
            // if he wants to.  If he returns -1 that means he wants us to
            // read the data and toss it over to him.  Some agent cores (eg
            // Net-SNMP) find it more convenient to read the packet themselves,
            // whereas others (eg WindManage) find it easier to just get a 
            // buffer.  This approach accomodates either preference.
            if (AgentInformPacketAvailable (DstIp, 
                                            fpAgents[Loop]->fpAgent->GetUdpPort(), 
                                            fpAgents[Loop]->fpSocket->SocketDescriptor()) == -1)
            {
              // NOTE: this buffer must be freed by the agent in the
              // AgentProcessSnmpPacket call.  Some agents may not copy the
              // buffer, so we must not free it here.
              unsigned char *pRcvBuf = new unsigned char[fMaxPacketSize];
            
              // Read the buffer off the wire.
              BytesRead = RecvfromSnmpSocket (fpAgents[Loop]->fpSocket->SocketDescriptor(), pRcvBuf, fMaxPacketSize, FromIp, FromPort);
              
              // Call the agent to deal with this packet.
              FromIp.Get(SrcIp);
              AgentProcessSnmpPacket (pRcvBuf, BytesRead, fpAgents[Loop]->fpSocket->SocketDescriptor(),
                                      SrcIp, DstIp, 
                                      FromPort, fpAgents[Loop]->fpAgent->GetUdpPort());
            }
            else
            {
              SnmpLogThd << "  agent core read packet off the wire." << endl;
              fpAgents[Loop]->fPacketsIn++;
            }
          }
        }
      }
      
      /*\
       *  Check for a signal on the signal socket and process it if we have
       *  one.
      \*/
      
      if (fpSocketSet->IsSetRead(fpSignalSocket))
      {
        SnmpThreadSignalPayload Payload;
        
        memset (&Payload, 0, sizeof (SnmpThreadSignalPayload));
        
        // Read the buffer off the wire.
        BytesRead = fpSignalSocket->Recvfrom((char *)&Payload, (int) sizeof(SnmpThreadSignalPayload), 0, FromIp, FromPort);
        SnmpLogThd << "Signal socket fired, " << BytesRead << " bytes on port " << FromPort << " from " << FromIp << "." << endl;
        
        ProcessSignal (Payload.Signal, Payload.pPayload);
      }
      
      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    } // while (1)

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


int BcmSnmpThread::UpdateAgents ()
{
  int ActiveAgents = 0, Loop;
  
  SnmpLogThd << "BcmSnmpThread::UpdateAgents" << endl;
  
  for (Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    // Look for agents which need rebinding and rebind them.
    if (fpAgents[Loop] != NULL)
    {
      if ((fpAgents[Loop]->fBoundPort != fpAgents[Loop]->fpAgent->GetUdpPort())
      ||  (fpAgents[Loop]->fBoundIp != fpAgents[Loop]->fpAgent->GetIpAddr()))
      {
        SnmpLogThd << "  rebinding " << fpAgents[Loop]->fpAgent->GetName() << endl;
      
        fpAgents[Loop]->Bind();
        if ((fpAgents[Loop]->fBoundIp != kBroadcastIp)
        &&  (fpAgents[Loop]->fBoundPort != 0))
          ActiveAgents++;
      }
    }
  }
  
  // If this is our first active agent, kick off the thread.
  if ((GetState() == BcmOperatingSystem::kNotStarted)
  &&  (ActiveAgents > 0))
  {
    SnmpLogRaw << "BcmSnmpThread starting thread operation." << endl;
    
    // Need more stack for the agent -try 14k for now...
    //if (pfOperatingSystem->BeginThread(this, 14*1024) == false)
    // Need a little more.
    if (pfOperatingSystem->BeginThread(this, 64*1024) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmSnmpThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
    }
  }
  
  return ActiveAgents;
}


void BcmSnmpThread::UpdateSocketSet ()
{
  int Loop;

  //SnmpLogThd << "Resetting socket set." << endl;

  fpSocketSet->ClearAll();
  
  if (fpSignalSocket->SocketDescriptor() > 0)
    fpSocketSet->SetRead (fpSignalSocket);
    
  for (Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if (fpAgents[Loop])
    {
      if ((fpAgents[Loop]->fBoundIp != kBroadcastIp)
      &&  (fpAgents[Loop]->fBoundPort != 0))
        fpSocketSet->SetRead (fpAgents[Loop]->fpSocket);
    }
  }
}

void BcmSnmpThread::CheckLinkStatus ()
{
  int Loop;
  ifEntryBridge *pIfEntry;
  BcmSnmpAgent *pAgent;

  // Iterate through agents.
  for (Loop = 0; Loop < kMaxSnmpAgents; Loop++)
  {
    if (fpAgents[Loop])
    {
      pAgent = fpAgents[Loop]->fpAgent;
      
      //SnmpLogThd << "Checking link status for " << pAgent->GetName() << endl;
    
      pIfEntry = pAgent->IfBridge().FirstIfEntry();
      
      // Iterate through ifEntries
      while (pIfEntry)
      {
        // Check ifOperStatus on each ifEntry.  If it has changed, the 
        // BcmNiHalIf object will handle sending a trap.
        pIfEntry->Get_ifOperStatus();
      
        pIfEntry = pAgent->IfBridge().NextIfEntry(pIfEntry->Get_ifIndex());
      }
    }
  }
  
  // Remember when we did this.
  fLastLinkCheck = SystemTimeMS();
}

#if (SNMP_SUPPORT == 3)
bool BcmSnmpThread::RegisterV3Agent (BcmSnmpV3Agent *pAgent)
{
  BcmAgentDescriptor *pDescr = FindDescriptor (pAgent);
  
  if (pDescr)
  {
    pDescr->fpV3Agent = pAgent;
    return true;
  }
  else if (RegisterAgent (pAgent) == false)
    return false;
    
  pDescr = FindDescriptor (pAgent);
  if (pDescr == NULL)
    return false;
    
  pDescr->fpV3Agent = pAgent;
  return true;
}

BcmSnmpV3Agent *BcmSnmpThread::FindV3Agent (const BcmSnmpAgent *pAgent) const
{
  BcmAgentDescriptor *pDescr = FindDescriptor (pAgent);
  
  if ((pDescr != NULL) && (pDescr->fpV3Agent == pAgent))
    return pDescr->fpV3Agent;
  
  return NULL;
}

BcmSnmpV3Agent *BcmSnmpThread::GetV3Agent (int Index) const
{
  BcmSnmpAgent *pAgent = GetAgent (Index);
  return FindV3Agent (pAgent);
}

BcmSnmpV3Agent *BcmSnmpThread::FindV3AgentByIf (int Interface) const
{
  BcmSnmpAgent *pAgent = FindAgentByIf (Interface);
  return FindV3Agent (pAgent);
}

BcmSnmpV3Agent *BcmSnmpThread::FindV3AgentByName (const BcmString &Name) const
{
  BcmSnmpAgent *pAgent = FindAgentByName (Name);
  return FindV3Agent (pAgent);
}

BcmSnmpV3Agent *BcmSnmpThread::FindV3AgentByIp (const BcmIpAddress &Ip) const
{
  BcmSnmpAgent *pAgent = FindAgentByIp (Ip);
  return FindV3Agent (pAgent);
}

BcmSnmpV3Agent *BcmSnmpThread::FindV3AgentBySubnet (const BcmIpAddress &Subnet) const
{
  BcmSnmpAgent *pAgent = FindAgentBySubnet (Subnet);
  return FindV3Agent (pAgent);
}

#endif


// Open a socket.  If a port is specified, then the socket will be bound
// to that UDP port on the specified IP address.  Thus this socket is
// usable as a 'server' socket in a subsequent recvfrom call.  If no
// port is specified, then the IP address argument will be used to determine
// the IP stack with which this socket will be associated.  If no match
// can be found, then the socket will be created without specifying an IP 
// stack and the caller will just have to take his chances.
BcmSocket *BcmSnmpThread::OpenSnmpSocket (const BcmIpAddress &Ip, int Port)
{
  BcmSnmpAgent *pAgent = FindAgentByIp (Ip);
  int IpStack = 0;
  BcmIpAddress tmpIp;
  
  
  SnmpLogThd << "Opening socket for " << Ip << ":" << Port << endl;
  
  if (pAgent)
  {
    IpStack = pAgent->GetIfNum();
    SnmpLogThd << "  Matched IP address to IP stack " << IpStack << endl;
    
    // If, by chance, the requested socket is for the port that this agent
    // is already bound to, then just return his socket.  This is in fact
    // how we often handle informs, we just send from the main agent socket,
    // so we wind up with a convenient listener.
    if (Port)
    {
      BcmAgentDescriptor *pDescriptor = FindDescriptor (pAgent);
      if (pDescriptor)
      {
        if ((pDescriptor->fBoundPort == Port)
        &&  (pDescriptor->fpSocket))
        {
          SnmpLogThd << "  Socket is for IP/Port already in use by agent " << pAgent->GetName() << endl;
          SnmpLogThd << "  Returning socket " << pDescriptor->fpSocket->SocketDescriptor() << endl;
          return pDescriptor->fpSocket;
        }
      }
    }
  }
  else
    SnmpLogThd << "  Couldn't match IP address, using IP stack 0." << endl;
  
  BcmSocket *pSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP, IpStack);
  
  if (pSocket == NULL)
  {
    SnmpLogThd << "  NULL socket!" << endl;
    return NULL;
  }

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
      SnmpLogThd << "Bind Failed <"<<ret<<">"<<endl;
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
int BcmSnmpThread::CloseSnmpSocket (int SocketDescriptor)
{
  SocketMap::iterator Pos = fSockets.find(SocketDescriptor);
  
  
  SnmpLogThd << "BcmSnmpThread::CloseSnmpSocket for socket " << SocketDescriptor << endl;
  
  if (Pos == fSockets.end())
  {
    SnmpLogThd << "  No match!" << endl;
    return -1;
  }
    
  BcmSocket *pSocket = (*Pos).second;
  
  // Make sure this socket doesn't belong to one of our agents.  If it
  // does, don't close it but return 0 so the agent core doesn't try to
  // close it either.
  BcmAgentDescriptor *pDescriptor = FindDescriptor (pSocket);
  if (pDescriptor)
  {
    SnmpLogThd << "  (socket belongs to " << pDescriptor->fpAgent->GetName() << ", only pretending to close...)" << endl;
    return 0;
  }
  
  SnmpLogThd << "  Found socket, closing and deleting." << endl;
  
  fSockets.erase (Pos);
  pSocket->Close();
  
  delete pSocket;
  
  return 0;
}


// Handle internal "PacketPointer" conversions for giganto-packets.
// If the packet passed in pPkt is determined to be a PacketPointer, then
// the address of the pointed-to-pcaket will be returned and Length
// will be updated with the length of the pointed-to-packet.  If it's not
// a PacketPointer, NULL is returned.
unsigned char *BcmSnmpThread::ConvertFromPacketPointer (const unsigned char *pPkt, int &Length) const
{
  PacketPointer *pPktPointer;
  
  // If our length isn't right, then this is obviously not a packet pointer
  // and we can return.
  if (Length != sizeof (PacketPointer))
    return NULL;
    
  // Cast as a PacketPointer
  pPktPointer = (PacketPointer *) pPkt;
  
  // Check magic number.
  if (pPktPointer->Magic != PACKETPOINTER_MAGIC_NUMBER)
    return NULL;
    
  // OK, we got ourselves a packet-pointer.
  SnmpLogThd << "  PacketPointer detected." << endl;
  
  Length = pPktPointer->Length;
  return pPktPointer->pData;
}


int BcmSnmpThread::RecvfromSnmpSocket (int Socket, unsigned char *pRcvBuf, int BufSize, BcmIpAddress &FromIp, unsigned short &FromPort) const
{
  int BytesRead;
  
  SnmpLogThd << "BcmSnmpThread::RecvfromSnmpSocket into " << BufSize << " byte buffer @ " << inHex(pRcvBuf) << endl;
  
  // First, see if the socket in question is one of ours.
  BcmAgentDescriptor *pDescriptor = FindDescriptor (Socket);
  
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
    
    pPktPointer = ConvertFromPacketPointer (pRcvBuf, BytesRead);
    if (pPktPointer)
    {
      int CopyBytes = BytesRead;
    
      SnmpLogThd << "  Converting PacketPointer." << endl;
      
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


int BcmSnmpThread::SendtoSnmpSocket (int Socket, const unsigned char *pBuf, int BufSize, const BcmIpAddress &ToIp, unsigned short &ToPort) const
{
  int BytesWrote;
  
  
  SnmpLogThd << "BcmSnmpThread::SendtoSnmpSocket from " << BufSize << " byte buffer @ " << inHex(pBuf) << endl;
  
  // First, see if the socket in question is one of ours.
  BcmAgentDescriptor *pDescriptor = FindDescriptor (Socket);
  
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
