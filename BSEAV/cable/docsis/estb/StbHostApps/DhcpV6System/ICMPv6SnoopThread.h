//****************************************************************************
//
// Copyright (c) 2008 Broadcom Corporation
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
//
//  Filename:       ICMPv6SnoopThread.h
//  Author:         Karthik Balasubramanian
//  Creation Date:  Mar 31, 2008
//
//****************************************************************************
//  Description:

//
//****************************************************************************

#ifndef ICMPV6SNOOPTHREAD_H
#define ICMPV6SNOOPTHREAD_H

//********************** Include Files ***************************************
#include "pcap.h"
#include "Thread.h"
#include "EthernetPacket.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

//********************** Forward Declarations ********************************

class BcmNeighborDiscoveryThread;


class BcmIcmpV6SnoopThread: public BcmThread
{
public:

	// Constructor.  Takes a string and uses it as the packet filtering criterion.  Also
	// requires a callback function.  This is the function that will be called when a packet
	// matching the filtering criterion is received.
	//
	// Parameters:
	//      initialPriority - the priority that you want the thread to have.
	//
	// Returns:  N/A
	//
	BcmIcmpV6SnoopThread(char *snoopThreadName, char *ethInterface, BcmNeighborDiscoveryThread * pNeighborDiscoveryThread,
						 BcmOperatingSystem::ThreadPriority initialPriority);

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
    virtual ~BcmIcmpV6SnoopThread();

	//void SetEstbIpAddress(BcmIpAddress &eStbIpAddress);

    static BcmIcmpV6SnoopThread *GetSingletonInstance(void);

	inline BcmNeighborDiscoveryThread * GetNeighborDiscoveryThread();

	void ParseRouterAdvertisement(BcmEthernetPacket & pPacket);


    //void Icmpv6Handler(u_char *notUsed, const struct pcap_pkthdr* pktHdr, const u_char* capturedPacket);

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

	BcmNeighborDiscoveryThread * pfNeighborDiscoveryThread;

protected:
    // Tells the thread when to exit (set in the destructor).
    bool fExitThread;

private:

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmIcmpV6SnoopThread(const BcmIcmpV6SnoopThread &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmIcmpV6SnoopThread & operator = (const BcmIcmpV6SnoopThread &otherInstance);

private:
	pcap_t* pcapDeviceDescr;
	char fEthInterface[128];
};


//********************** Inline Method Implementations ***********************
BcmNeighborDiscoveryThread * BcmIcmpV6SnoopThread::GetNeighborDiscoveryThread()
{
	return pfNeighborDiscoveryThread;
}

#endif



