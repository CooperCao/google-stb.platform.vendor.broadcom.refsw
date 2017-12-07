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
//**************************************************************************
//  $Id$
//
//  Filename:       Proxy.cpp
//  Author:         Maurice Turcotte / Thomas Clack
//  Creation Date:  4 AUG 06
//
//****************************************************************************
//
//****************************************************************************

//********************** Include Files ***************************************

#include <stdio.h>
#include <string.h>
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <iostream>
#else
#include <iostream.h>
#endif
#include "SnmpLog.h"

// My api and definitions...
#include "MutexSemaphore.h"
#include "OperatingSystemFactory.h"
#include "Proxy.h"
#include "SocketFactory.h"
#include "SocketSet.h"
// #include "HalIf.h"
// #include "BaseIpHalIf.h"

// #include "IpStackManager.h"

#include "ocStbHostMib.h"
#include "ocHnMib.h"



// extern "C" 
// {
//     int bcmHalIpAddrGet(unsigned int *pIpAddr, unsigned char stackIfNum );
// }


//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Global instances ************************************

//********************** Class Method Implementations ************************


BcmSocket *BcmProxy::pProxySocket= NULL;
BcmSocketSet *BcmProxy::pSocketSet= NULL;
// BcmHalIf *BcmProxy::pHalIf= NULL;
BcmMutexSemaphore *BcmProxy::pMutex= NULL;
BcmMessageLogSettings BcmProxy::fMessageLogSettings; 

BcmIpAddress BcmProxy::fSourceIp;
unsigned char BcmProxy::fStackNum;
BcmProxySnmpAgent *BcmProxy::fpSubAgent = NULL;



// Default Constructor.  Initializes the state of the object...
//
// Parameters:  None.
//
// Returns:  N/A
//
BcmProxy::BcmProxy(void)
{
    //    CallTrace("BcmProxy", "BcmProxy");
    fMessageLogSettings.SetModuleName("BcmProxy");

}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmProxy::~BcmProxy()
{
//    CallTrace("BcmProxy", "~BcmProxy");
}


bool BcmProxy::Initialize(BcmSnmpAgent *masterAgent)
{
    BcmMibGroup *pGroup;

//     pHalIf = NULL;

    fStackNum = 1;
    fSourceIp = kAllZerosIp;

    // Create my mutex.  If this fails, bad things will happen.
    if ( pMutex == NULL )
    {
        pMutex = BcmOperatingSystemFactory::NewMutexSemaphore("BcmProxy's mutex");
    }

    if ( pMutex == NULL )
    {
        gErrorMsg(fMessageLogSettings, "BcmProxy")
        << "DID NOT CREATE MUTEX!." << endl; 
        //BAD!!!
        return false;
    }

    //  TODO: May want to open socket here instead .....


    fpSubAgent =  new BcmProxySnmpAgent ((char *) "Proxy Agent",masterAgent);

    // This is where we add the MIBs supported by the Proxy 
    pGroup = new ocStbHostMibObjectsGroup ();
    fpSubAgent->AddGroup (pGroup);

    pGroup = new ocHnMibObjectsGroup ();
    fpSubAgent->AddGroup (pGroup);

    
    return true;
}


bool BcmProxy::SendPacket( BcmString &SendMsg, BcmOctetBuffer *returnBuffer)
{
    BcmSocket *pSocket;
    BcmIpAddress DestIp(kSoftwareLoopbackIp);
    int DestPort = PROXY_DEST_PORT;

    // cout<<"SendPacket is staring......"<<endl;
    // Create a socket:
    pSocket = BcmSocketFactory::NewSocket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    
    if ( pSocket )
    {
        // TODO:  Might want to BIND our socket...... finish this

        // Bind to the SW loopback address on our source port.
        //  if (pSocket->Bind (kSoftwareLoopbackIp, PROXY_SRC_PORT) != 0)
        //  {
        //      cout << "Bind failure on socket " << pSocket->SocketDescriptor() << " to " << kSoftwareLoopbackIp << ":" << PROXY_SRC_PORT << endl;
        //      return 0;
        //  }
        //  cout<<"Bind succeded to "<< kSoftwareLoopbackIp << endl;

        
        // cout << "Sending proxy snmp packet to " << kSoftwareLoopbackIp << " : " << DestPort<< endl;
        if ( pSocket->Sendto ((char *)SendMsg.data(), SendMsg.length(), 0, kSoftwareLoopbackIp, DestPort) != (int)SendMsg.length() )
        {
            // If the sendto call fails, good chance it's because we are dealing with
            // a packet which is bigger than the OS / IP stack can handle.  For
            // eCos, this happens to be 8k.  So if that's the case, then try our
            // clever PacketPointer technique...
            SnmpLogRaw << "Sendto error!  Packet is probably too big.  " << endl;
            return 0;
        }
        // cout<<"packet was sent, bytes="<<SendMsg.length()<<endl;

    }
    return 1;
}
