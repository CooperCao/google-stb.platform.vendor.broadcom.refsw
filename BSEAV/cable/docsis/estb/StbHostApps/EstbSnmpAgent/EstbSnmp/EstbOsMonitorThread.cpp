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
//  Filename:       EstbOsMonitorThread.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  30 NOV 06
//
//****************************************************************************
//  Description:
//      Thread to allow event logging to take place outside the context of
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "EstbOsMonitorThread.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"
#include "SnmpApi.h"
#include "SnmpLog.h"
#include "EstbSnmpAgent.h"
#if (LOCAL_SNMP_AGENT_SUPPORT)
#include "LocalSnmpAgent.h"  
#endif
extern "C" {
#include "LnxInterface.h"
#include <sys/stat.h>
}


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


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
BcmEstbOsMonitorThread::BcmEstbOsMonitorThread(EstbSnmpAgent *pAgent, const char *pName, BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread(pName, false, initialPriority)
{
    gLogMessageRaw << "BcmEstbOsMonitorThread starting" << endl;
    fMessageLogSettings.SetModuleName("BcmEstbOsMonitorThread");

    fExitThread = false;
    fAbortOsMonitoring = false;
    
    pfThreadStoppedSemaphore = NULL;
    
    fpAgent = pAgent;

    fLastIpAddr = kBroadcastIp;
    
    if (pfOperatingSystem->BeginThread(this, 16385) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmEstbOsMonitorThread" ) 
            << "Failed to spawn my thread!" << endl;
        assert(0);
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
BcmEstbOsMonitorThread::~BcmEstbOsMonitorThread()
{
  fExitThread = true;
  
  // Send a message to the thread to wake it up and stop it.
  if (pfEstbOsMonitorQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmEstbOsMonitorThread::~BcmEstbOsMonitorThread - pfEstbOsMonitorQueue is NULL" << endl;
    return;
  }
  
  pfEstbOsMonitorQueue->Send(kStopThread);
  
  // Wait for the thread to exit
  WaitForThread();
}


void BcmEstbOsMonitorThread::StartMonitoring ()
{
  fAbortOsMonitoring = false;

  // Send ourselves an event to off the monitoring.
  if (pfEstbOsMonitorQueue)
    pfEstbOsMonitorQueue->Send(kStartMonitoring);
}


bool BcmEstbOsMonitorThread::StopThread ()
{
  SnmpLogStart << "Stopping OS Monitor thread..." << endl;

  // Wait for the thread to reset.  This means that this call will block
  // until the thread has finished.
  pfThreadStoppedSemaphore->Get();
  pfThreadStoppedSemaphore->Release();
  
  SnmpLogStart << "OS Monitor thread stopped OK." << endl;
  
  return true;
}

bool BcmEstbOsMonitorThread::ShouldAbortOsMonitoring () const
{
  return fAbortOsMonitoring;
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
bool BcmEstbOsMonitorThread::Initialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    
    pfEstbOsMonitorQueue = pfOperatingSystem->NewMessageQueue ("BcmEstbOsMonitorThread message queue");
    
    if (pfEstbOsMonitorQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmEstbOsMonitorThread failed to create its pfEstbOsMonitorQueue!" << endl;
      return false;
    }
    
    pfThreadStoppedSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 1, "Thread Stopped Semaphore");

    if (pfThreadStoppedSemaphore == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") << "Failed to create my semaphore!" << endl;

        return false;
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
void BcmEstbOsMonitorThread::Deinitialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // Get rid of the OS objects that I created.
    if (pfEstbOsMonitorQueue)
      delete pfEstbOsMonitorQueue;
      
    pfEstbOsMonitorQueue = NULL;
    
    if (pfThreadStoppedSemaphore)
      delete pfThreadStoppedSemaphore;
      
    pfThreadStoppedSemaphore = NULL;
}


// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmEstbOsMonitorThread::ThreadMain(void)
{
  uint32 tmpAddr = 0;
  BcmString intName;    // the name of the interface we will monitor
  char intNameString[ESTB_SNMP_INTERFACE_NAME_LEN] = "";
  BcmIpAddress newIpAddr;
  struct stat newStat;
  time_t configFileMod = 0;   // last time config file was mondified
  char configFilePath[] = "./estbconfigsettings.bin";
  int linuxScanTimer = 1;


    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;
    
    if (pfEstbOsMonitorQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmEstbOsMonitorThread::ThreadMain - pfEstbOsMonitorQueue is NULL" << endl;
      return;
    }

    intName = fpAgent->GetInterfaceName();
    if (intName == ESTB_SNMP_ANY_INTERFACE) {
      // if we are listening on all interfaces, just announce we're up!
      fpAgent->InterfaceOperationalEvent();
    } else {
#if (LOCAL_SNMP_AGENT_SUPPORT)
  // If we are listening on a particular interface (not ALL)
  // fire up the local agent to listen on 127.0.0.1 (loopback)
    if (!LocalSnmpAgent::Exists()) {
      BcmIpAddress MyAddress;
      BcmIpAddress MyNetmask;
      BcmIpAddress Router;
      MyAddress.Set(kSoftwareLoopbackIp);
      MyNetmask.Set(kBroadcastIp);
      Router.Set(kAllZerosIp);
      SnmpLogRaw << "listening on interface " << intName << 
                           " - spawning local agent " << endl;
      LocalSnmpAgent::Singleton().ConfigIp (MyAddress, MyNetmask, Router);
      LocalSnmpAgent::Singleton().Start ();
    }
#endif
      if (intName != "") {
        if (intName.length() > 0) {
          strcpy(intNameString, intName.c_str());  
        } else {
          SnmpLogRaw << "ERROR - BcmEstbOsMonitorThread::ThreadMain - no interface to monitor!!!" << endl;
        } 
      } 
    }
    memset(&configFileMod, 0, sizeof(configFileMod));
    memset(&newStat, 0, sizeof(newStat));

    while (1)
    {
      if (intName != ESTB_SNMP_ANY_INTERFACE) {
        if (GetIpStackAddress(&tmpAddr, intNameString) == 0) {
          newIpAddr.NetworkSet(tmpAddr);
          if (fLastIpAddr != newIpAddr) {
            fLastIpAddr = newIpAddr;
            SnmpLogRaw << "BcmEstbOsMonitorThread - detected new address " << fLastIpAddr << " for interface " << intName << endl;
            fpAgent->InterfaceOperationalEvent();
          }
        }
      }

      if (stat(configFilePath, &newStat) == 0) { 
        // config file found
        if (memcmp(&configFileMod, &newStat.st_mtime, sizeof(configFileMod)) != 0) {
          // and it's newer
          memcpy(&configFileMod, &newStat.st_mtime, sizeof(configFileMod));
          SnmpLogRaw << "BcmEstbOsMonitorThread - found new config file " << endl;
          fpAgent->EstbConfigFileNewEvent();
        }
      }

      // Just do this once every ten sleeps
      linuxScanTimer--;
      if (linuxScanTimer <= 0) {
        linuxScanTimer = 10;
        fpAgent->ScanLinuxTables();
      }
          
       BcmOperatingSystemFactory::ThreadSleep (1000);
       // SnmpLogRaw << "BcmEstbOsMonitorThread::ThreadMain - going back to sleep" << endl;

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    }

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


