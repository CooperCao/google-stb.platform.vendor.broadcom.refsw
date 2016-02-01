//****************************************************************************
//
// Copyright (c) 2009-2012 Broadcom Corporation
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
//  Filename:       MocaMonitorThread.cpp
//  Author:         Maurice Turcotte
//  Creation Date:  07 DEC 09
//
//****************************************************************************
//  Description:
//      This thread monitors the MoCA driver periodically and
//      notifies interested parties.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "MocaMonitorThread.h"

// Other objects of interest.
#include "Event.h"
#include "Timer.h"
#include "CountingSemaphore.h"
#include "OperatingSystem.h"
#include "OperatingSystemFactory.h"
#include "SnmpApi.h"
#include "SnmpLog.h"
#include "EstbSnmpAgent.h"
#include "mocaBridge.h"
#include <stdint.h>
// #include "bcmmocacfg.h"
extern "C" {
#include "mocalib.h"
#include "LnxInterface.h"
#include <sys/stat.h>
#include <sys/time.h>
}


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************
void *mocaHandle = NULL;
EstbSnmpAgent *gAgent = NULL;
int gIfIndex = 0;

static pthread_cond_t pqos_cond;
static pthread_mutex_t pqos_mutex;



//********************** Class Method Implementations ************************


static int moca_start_event_loop(void * ctx, pthread_t * thread)
{
   int ret;

   pthread_mutex_init(&pqos_mutex, NULL);
   pthread_cond_init(&pqos_cond, NULL);

   ret = pthread_create(thread, NULL, (void* (*)(void*)) moca_event_loop, ctx);
   if (ret != 0) {
      fprintf( stderr, " pthread_create returned %d\n",
         ret);
      return(1);
   }

   /* Give the thread a chance to run */
   usleep(500);

   return(0);
}

static int moca_wait_for_event(void * ctx, pthread_t thread)
{
   struct timeval now;
   struct timespec timeout;
   int ret;

   gettimeofday(&now, NULL);
   timeout.tv_sec = now.tv_sec + 2; /* wait 2 seconds for pqos response */
   timeout.tv_nsec = now.tv_usec * 1000;
   pthread_mutex_lock(&pqos_mutex);
   ret = pthread_cond_timedwait(&pqos_cond, &pqos_mutex, &timeout);
   pthread_mutex_unlock(&pqos_mutex);

   // if (ret == ETIMEDOUT)
   //    cout << "Operation timed out." << endl;

   pthread_cond_destroy(&pqos_cond);
   pthread_mutex_destroy(&pqos_mutex);
   return(ret);
}

static void pqos_callback_return(void * arg)
{
   // cout << "  pqos_callback_return() called" << endl;
   moca_cancel_event_loop(arg);
   pthread_mutex_lock(&pqos_mutex);
   pthread_cond_signal(&pqos_cond);
   pthread_mutex_unlock(&pqos_mutex);
}


static void pqos_query_response_copy_cb(void *arg, struct moca_pqos_query_response *in)
{
   // cout << "  pqos_query_response_copy_cb() called" << endl;
   if (arg != NULL)
   {
      memcpy((struct moca_pqos_query_response *)arg, in,
         sizeof(struct moca_pqos_query_response));
   }

   pqos_callback_return(mocaHandle);
}

// static void pqos_list_response_cb(void *arg, struct moca_pqos_list_response *in)
static void FlowListResponseCB(void *arg, struct moca_pqos_list_response *in)
{
  int i, ret;
  uint32_t *flowid;
  uint32_t number_of_flows = 0;
  struct moca_pqos_query_request req;
  struct moca_gen_status gs;
  struct moca_gen_node_status gns;
  BcmMibBridge *pBridge;
  pthread_t pqos_thread;
  struct moca_pqos_query_response query_rsp;



  // cout << "  FlowListResponseCB called" << endl;

  if (mocaHandle == NULL) {
     pqos_callback_return(mocaHandle);
     return;
   }

   flowid = &in->flowid0_hi;

   for (i = 0; i < 32; i++)
   {
      if (flowid[i*2] != 0 || flowid[i*2+1] != 0)
      {
         // cout << "   Flow ID (" << i << "):     " << 
         //    ((flowid[i*2] >> 24) & 0xFF) << ":" <<
         //    ((flowid[i*2] >> 16) & 0xFF) << ":" <<
         //    ((flowid[i*2] >>  8) & 0xFF) << ":" <<
         //    ((flowid[i*2] >>  0) & 0xFF) << ":" <<
         //    ((flowid[i*2+1] >> 24) & 0xFF) << ":" <<
         //    ((flowid[i*2+1] >> 16) & 0xFF) << endl;

        memset(&req, 0, sizeof(req));

        /* get node bitmask */
        moca_get_gen_status(mocaHandle, &gs);

        /* set wave0Nodemask to all 1.1 nodes */
        for(i = 0; i < MOCA_MAX_NODES; i++)
        {
                if(! (gs.connected_nodes & (1 << i)))
                        continue;
                ret = moca_get_gen_node_status(mocaHandle, i, &gns);
                if (ret == 0)
                {
                        if ((gns.protocol_support >> 24) == MoCA_VERSION_11)
                                req.wave0Nodemask |= (1 << i);
                }
        }

        ret = moca_start_event_loop(mocaHandle, &pqos_thread);
        if (ret == 0) {
          moca_register_pqos_query_response_cb(mocaHandle,
                  &pqos_query_response_copy_cb, &query_rsp);

          req.flowid_hi = flowid[i*2]; 
          req.flowid_lo = flowid[i*2+1]; 
          moca_set_pqos_query_request(mocaHandle, &req);
          moca_wait_for_event(mocaHandle, pqos_thread);
        }

        if (query_rsp.responsecode == MOCA_L2_SUCCESS) {
          pBridge = new mocaIfFlowStatsEntryBridge (gIfIndex, (int) i + 1,
                                                  mocaHandle, &query_rsp, gAgent);
          if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
            delete (pBridge);
        }
         number_of_flows++;
      }
   }

   pqos_callback_return(mocaHandle);
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
BcmMocaMonitorThread::BcmMocaMonitorThread(EstbSnmpAgent *pAgent, const char *pName, BcmOperatingSystem::ThreadPriority initialPriority) :
    BcmThread(pName, false, initialPriority)
{
    gLogMessageRaw << "BcmMocaMonitorThread starting" << endl;
    fMessageLogSettings.SetModuleName("BcmMocaMonitorThread");

    fExitThread = false;
    fAbortOsMonitoring = false;
    
    pfThreadStoppedSemaphore = NULL;
    
    fpAgent = pAgent;

    fLastIpAddr = kAllZerosIp;
    
    if (pfOperatingSystem->BeginThread(this, 16385) == false)
    {
        gFatalErrorMsg( fMessageLogSettings, "BcmMocaMonitorThread" ) 
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
BcmMocaMonitorThread::~BcmMocaMonitorThread()
{
  fExitThread = true;
  
  // Send a message to the thread to wake it up and stop it.
  if (pfMocaMonitorQueue == NULL)
  {
    SnmpLogRaw << "ERROR - BcmMocaMonitorThread::~BcmMocaMonitorThread - pfMocaMonitorQueue is NULL" << endl;
    return;
  }
  
  pfMocaMonitorQueue->Send(kStopThread);
  
  // Wait for the thread to exit
  WaitForThread();
}


void BcmMocaMonitorThread::StartMonitoring ()
{
  fAbortOsMonitoring = false;

  // Send ourselves an event to off the monitoring.
  if (pfMocaMonitorQueue)
    pfMocaMonitorQueue->Send(kStartMonitoring);
}


bool BcmMocaMonitorThread::StopThread ()
{
  SnmpLogStart << "Stopping OS Monitor thread..." << endl;

  // Wait for the thread to reset.  This means that this call will block
  // until the thread has finished.
  pfThreadStoppedSemaphore->Get();
  pfThreadStoppedSemaphore->Release();
  
  SnmpLogStart << "OS Monitor thread stopped OK." << endl;
  
  return true;
}

bool BcmMocaMonitorThread::ShouldAbortOsMonitoring () const
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
bool BcmMocaMonitorThread::Initialize(void)
{
  BcmMibGroup *pGroup;

    gFuncEntryExitMsg(fMessageLogSettings, "Initialize") << "Entering..." << endl;
    
    pfMocaMonitorQueue = pfOperatingSystem->NewMessageQueue ("BcmMocaMonitorThread message queue");
    
    if (pfMocaMonitorQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmMocaMonitorThread failed to create its pfMocaMonitorQueue!" << endl;
      return false;
    }
    
    pfThreadStoppedSemaphore = BcmOperatingSystemFactory::NewCountingSemaphore(true, 1, 1, "Thread Stopped Semaphore");

    if (pfThreadStoppedSemaphore == NULL)
    {
        gErrorMsg(fMessageLogSettings, "Initialize") << "Failed to create my semaphore!" << endl;

        return false;
    }

  pGroup = new moca11Group ();
  fpAgent->AddGroup (pGroup);

  // Save local copies of some tables
  fpMocaIfConfigTable  = fpAgent->FindTable (BcmString ("mocaIfConfigTable"));
  fpMocaIfAccessTable  = fpAgent->FindTable (BcmString ("mocaIfAccessTable"));
  fpMocaIfStatusTable  = fpAgent->FindTable (BcmString ("mocaIfStatusTable"));
  fpMocaIfStatsTable  = fpAgent->FindTable (BcmString ("mocaIfStatsTable"));
  fpMocaIfFlowStatsTable  = fpAgent->FindTable (BcmString ("mocaIfFlowStatsTable"));
  fpMocaNodeTable  = fpAgent->FindTable (BcmString ("mocaNodeTable"));
  fpMocaMeshTable  = fpAgent->FindTable (BcmString ("mocaMeshTable"));
  fpMocaBridgeTable  = fpAgent->FindTable (BcmString ("mocaBridgeTable"));

  if (fpMocaIfConfigTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaIfConfigTable" << endl;
  if (fpMocaIfAccessTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaIfAccessTable" << endl;
  if (fpMocaIfStatusTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaIfStatusTable" << endl;
  if (fpMocaIfStatsTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaIfStatsTable" << endl;
  if (fpMocaIfFlowStatsTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaIfFlowStatsTable" << endl;
  if (fpMocaNodeTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaNodeTable" << endl;
  if (fpMocaMeshTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaMeshTable" << endl;
  if (fpMocaBridgeTable == NULL)
    SnmpLogRaw << "WARNING - Failed to find fpMocaBridgeTable" << endl;


    
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
void BcmMocaMonitorThread::Deinitialize(void)
{
    gFuncEntryExitMsg(fMessageLogSettings, "Deinitialize") << "Entering..." << endl;

    // Get rid of the OS objects that I created.
    if (pfMocaMonitorQueue)
      delete pfMocaMonitorQueue;
      
    pfMocaMonitorQueue = NULL;
    
    if (pfThreadStoppedSemaphore)
      delete pfThreadStoppedSemaphore;
      
    pfThreadStoppedSemaphore = NULL;
}


// This function scans the Linux Ports looking for MoCA devices
void BcmMocaMonitorThread::ScanDrivers () {
  ifEntryBridge *pIfEntry;
  char ifDescr[] = "eth1";
  BcmMibBridge *pBridge;
  struct moca_gen_status genStatus; 
  struct moca_init_time initTime; 
  struct moca_gen_node_status nodeStatus; 
  struct moca_pqos_list_request pqos;
  int nodeId, myNodeId;
  MAC_ADDRESS *pmacAddr;
  MAC_ADDRESS mac;
  pthread_t pqos_thread;
  int total_flows = 0;
  int ret;





  if (fpMocaIfConfigTable)
    fpMocaIfConfigTable->Flush();
  if (fpMocaIfAccessTable)
    fpMocaIfAccessTable->Flush();
  if (fpMocaIfStatusTable)
    fpMocaIfStatusTable->Flush();
  if (fpMocaIfStatsTable)
    fpMocaIfStatsTable->Flush();
  if (fpMocaIfFlowStatsTable)
    fpMocaIfFlowStatsTable->Flush();
  if (fpMocaNodeTable)
    fpMocaNodeTable->Flush();
  if (fpMocaMeshTable)
    fpMocaMeshTable->Flush();
  if (fpMocaBridgeTable)
    fpMocaBridgeTable->Flush();

  pIfEntry = fpAgent->IfBridge().FindIfEntry(ifDescr, strlen(ifDescr));

  if (pIfEntry) {

    // SnmpLogRaw << "BcmMocaMonitorThread::ScanDrivers() calling moca_open" << endl;
    mocaHandle = moca_open( ifDescr );
    gIfIndex = pIfEntry->Get_ifIndex();
    gAgent = fpAgent;

    // SnmpLogRaw << "BcmMocaMonitorThread::ScanDrivers() returned from moca_open" << endl;
    if (mocaHandle) {

      // mocaIfStatusTable
      pBridge = new mocaIfStatusEntryBridge (pIfEntry->Get_ifIndex(),
                                mocaHandle, fpAgent);
      if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
        delete (pBridge);

      // mocaIfStatsTable
      pBridge = new mocaIfStatsEntryBridge (pIfEntry->Get_ifIndex(),
                                mocaHandle, fpAgent);
      if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
            delete (pBridge);

      // mocaFlowStatsTable
      if (!moca_get_gen_status(mocaHandle, &genStatus)) {
        myNodeId = genStatus.node_id;
        // cout << "BcmMocaMonitorThread::ScanDrivers() got " << genStatus.connected_nodes << " connected nodes" << endl;
        for (nodeId = 0; genStatus.connected_nodes;
                      genStatus.connected_nodes >>= 1, nodeId++) {
          pmacAddr = NULL;
          if (genStatus.connected_nodes & 0x1) {
            if (genStatus.node_id == nodeId) {
              // cout << "  node ID " << nodeId << " is local" << endl;
              if (!moca_get_init_time(mocaHandle, &initTime)) {
                moca_u32_to_mac(mac, initTime.mac_addr_hi,
                    initTime.mac_addr_lo);
                pmacAddr = &mac;
              }
            } else {
              // cout << "  node ID " << nodeId << " is remote" << endl;
              if (!moca_get_gen_node_status(mocaHandle, nodeId, &nodeStatus)) {
                moca_u32_to_mac(mac, nodeStatus.eui_hi, nodeStatus.eui_lo);
                pmacAddr = &mac;
              }
            }
            // cout << "  MAC Address: " <<
            //     mac[0] << ":" <<
            //     mac[1] << ":" <<
            //     mac[2] << ":" <<
            //     mac[3] << ":" <<
            //     mac[4] << ":" <<
            //     mac[5] << endl;
            if (pmacAddr != NULL) {
              // cout << "  Ingress Node MAC Address: " <<
                // pmacAddr[0] << ":" <<
                // pmacAddr[1] << ":" <<
                // pmacAddr[2] << ":" <<
                // pmacAddr[3] << ":" <<
                // pmacAddr[4] << ":" <<
                // pmacAddr[5] << endl;

              ret = moca_start_event_loop(mocaHandle, &pqos_thread);
              if (ret == 0) {
                // cout << "  calling moca_register_pqos_list_response_cb()" << endl;
                moca_register_pqos_list_response_cb(mocaHandle, &FlowListResponseCB, (void *)&total_flows);
                memset(&pqos, 0x00, sizeof(pqos));
                pqos.wave0Nodemask = 0x1 << nodeId;
                pqos.flowStartIndex = 0;
                pqos.flowMaxReturn = 32;
                // cout << "  calling moca_set_pqos_list_request()" << endl;
                if (!moca_set_pqos_list_request(mocaHandle, &pqos)) {
                // cout << "  calling moca_wait_for_event()" << endl;
                  moca_wait_for_event(mocaHandle, pqos_thread);
                // cout << "  moca_wait_for_event() returned" << endl;
                }
              }
            }
          }
        }
      } 


      // cout << "BcmMocaMonitorThread::ScanDrivers() doing mocaMeshTable and mocaNodeTable" << endl;
      // mocaMeshTable and mocaNodeTable
      uint32_t srcNodeId = 0;
      uint32_t destNodeId = 0;
      uint32_t connectedNodes = 0;
      uint32_t srcConnectedNodes = 0;
      uint32_t destConnectedNodes = 0;

      if (!moca_get_gen_status(mocaHandle, &genStatus)) {
        connectedNodes = srcConnectedNodes = destConnectedNodes = genStatus.connected_nodes;
        // cout << " got " << genStatus.connected_nodes << " connected nodes" << endl;
        for (srcNodeId = 0; srcConnectedNodes;
             srcConnectedNodes >>= 1, srcNodeId++) {
          if (srcConnectedNodes & 0x1) {
            if (srcNodeId != myNodeId) {     // no entry for myself 
              // cout << "  adding mocaNodeTable for NodeId " << srcNodeId << endl;
              pBridge = new mocaNodeEntryBridge (pIfEntry->Get_ifIndex(), srcNodeId,
                                         mocaHandle, fpAgent);
              if (pBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR)
                delete (pBridge);
            }

            destConnectedNodes = connectedNodes;
            for (destNodeId = 0; destConnectedNodes;
                 destConnectedNodes >>= 1, destNodeId++) {
              if (destConnectedNodes & 0x1) {
                if (srcNodeId != destNodeId) {
                  // cout << " moca MESH node " << srcNodeId << " to node " << destNodeId << endl;
                  mocaMeshEntryBridge *pMeshBridge = 
                      new mocaMeshEntryBridge (pIfEntry->Get_ifIndex(), srcNodeId,
                                                     destNodeId, mocaHandle, fpAgent);
                  if (pMeshBridge->Get_mocaMeshTxRate() == 0) {
                    delete (pMeshBridge);
                    // cout << "     mocaMeshTxRate is 0 ... don't register this entry!" << endl;
                  } else if (pMeshBridge->Register (kDestroyMibObject) != SNMP_NO_ERROR) {
                    delete (pMeshBridge);
                  }
                }
              }
            }
          }
        }
      }

      // moca_close( mocaHandle ); mocaIfStatusEntryBridge will close it
      mocaHandle = NULL;
      gIfIndex = 0;
      gAgent = NULL;
    }


  }

}



// This is the main body of the thread's code.  This is the only method
// that absolutely must be provided in the derived class (since there is no
// reasonable default behavior).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmMocaMonitorThread::ThreadMain(void)
{
  int mocaScanTimer = 1;


    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Entering..." << endl;
    
    if (pfMocaMonitorQueue == NULL)
    {
      SnmpLogRaw << "ERROR - BcmMocaMonitorThread::ThreadMain - pfMocaMonitorQueue is NULL" << endl;
      return;
    }

    while (1)
    {
      // Just do this once every 60 sleeps
      // There could be 16 nodes and it could take 2 seconds
      // to get the info from each node, so we need at least
      // 32 seconds between scans, so 60 sounds good.
      mocaScanTimer--;
      if (mocaScanTimer <= 0) {
        mocaScanTimer = 60;
        ScanDrivers();
      }
          
       BcmOperatingSystemFactory::ThreadSleep (1000);
       // SnmpLogRaw << "BcmMocaMonitorThread::ThreadMain - going back to sleep" << endl;

      // If I'm being told to exit the thread, then do so.
      if (fExitThread)
        break;
    }

    gFuncEntryExitMsg(fMessageLogSettings, "ThreadMain") << "Exiting..." << endl;
}


