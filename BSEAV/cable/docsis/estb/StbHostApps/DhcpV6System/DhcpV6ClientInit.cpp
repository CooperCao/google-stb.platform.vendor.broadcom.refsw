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
//
//  Filename:       DhcpV6ClientInit.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Nov 19, 2007
//
//****************************************************************************
//  Description:

//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DhcpV6ClientInit.h"

#include "OperatingSystemFactory.h"

#if defined(TARGETOS_vxWorks)
    #include "vxIpHalIf.h"
#elif defined(TARGETOS_Linux)
    #include "LnxIpStackIf.h"	
	#include <net/if.h>	
#elif defined(PSOS)
    #include "PsosIpHalIf.h"
#elif defined(TARGETOS_eCos)
    #include "ecosIpHalIf.h"
#else
    #include "BaseIpHalIf.h"
#endif

#include "EstbIpThread.h"
#include "DhcpV6ClientSnoopThread.h" 
#if (BFC_INCLUDE_DHCPV4_SUPPORT)
#include "DhcpV4ClientSnoopThread.h"
#endif
#include "IpHalIfFactory.h"

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

//********************** Extern Functions *************************************
extern "C"
{
	extern uint32 SetInterfaceFlags(uint32 ipAddress, char *pInterfaceName, uint32 flags);
}

//********************** Class Method Implementations ************************

void DhcpClientInit(bool preferredIpVersionIsIPv6, bool dualStackOperationEnabled)
{
    BcmLnxIpStackIf *pHal;
    //BcmBaseIpHalIf *pHal;
    pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->First();
    while (pHal)
    {
		////////////////////////////////
		//BcmString ifEth0Descr = "eth0";
		//BcmString ifEth1Descr = "eth1";
		//BcmString thisIfDescr = pHal->Description();		
		////////////////////////////////

		uint32 flags = (IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST);

		if( SetInterfaceFlags(0, (char*)pHal->Description(), flags) != 0 )
		{
			gLogMessageRaw << "Failed to set interface flags for" << pHal->Description() << endl;
		}

		//gLogMessageRaw << "##############flags =  " << inHex(flags) << endl;
		if (((BcmBaseIpHalIf*) pHal)->GetInterfaceFlags(&flags) )
		{

			if( flags & (IFF_UP | IFF_BROADCAST | IFF_RUNNING | IFF_MULTICAST) )
				//( (thisIfDescr == ifEth1Descr) ) 
			{
				// Is interface IPv6 capable
				// Get IPv6 Link Local address.
				//BcmIpV6Address linkLocalAddress;
				//if(ptr->linkaddrcount)
				//	linkLocalAddress.Set((uint8*)ptr->linkaddr);

				if( pHal->Ipv6LinkLocalAddressConfigured() )
				{
					// Get IPv6 Link Local address.
					BcmIpV6Address linkLocalAddress;
					pHal->GetIpv6LinkLocalAddress(linkLocalAddress);				

					// Create transmit socket.
					pHal->CreateNetworkTxSocket(linkLocalAddress, kDhcpV6UdpClientPort, true, true);
				}
				else
				{
					gLogMessageRaw << "IPv6 Link Local Address is not configured for " << pHal->Description() << endl;
				}
                #if (BFC_INCLUDE_DHCPV4_SUPPORT)
				// Create Transmit socket for IPV4 dhcp client
				((BcmLnxIpStackIf*) pHal)->CreateV4NetworkTxSocket();
                #endif
			}
			else
			{
				gLogMessageRaw << pHal->Description() << " cannot be configured. Required interface flags are not enabled." << inHex(flags) << endl;
			}
		}
      
		pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->Next(pHal);
    }    

	// Start DHCP Helper thread that will listen to IPv6 sockets.
#if 1
	BcmDhcpV6ClientSnoopThread *pDhcpV6ClientSnoopThread = new BcmDhcpV6ClientSnoopThread(BcmOperatingSystem::kNormalPriority) ;
	if (pDhcpV6ClientSnoopThread == NULL)
	{
		cout //gFatalErrorMsg( fMessageLogSettings, "BcmDhcpV6System" ) 
		<< "Failed to create my DHCP Helper Thread object!" << endl;
		assert(0);
	}

	// Give BcmDhcpV6ClientSnoopThread a chance to initialize since it will setup NetworkRxSocket()
	// for the network interface.
	BcmOperatingSystemFactory::ThreadSleep(1000);
    #if (BFC_INCLUDE_DHCPV4_SUPPORT)
    // Start DHCP V4 Helper thread that will listen to eth1 packet sockets.
	BcmDhcpV4ClientSnoopThread *pDhcpV4ClientSnoopThread = new BcmDhcpV4ClientSnoopThread(BcmOperatingSystem::kNormalPriority) ;
	if (pDhcpV4ClientSnoopThread == NULL)
	{
		cout //gFatalErrorMsg( fMessageLogSettings, "BcmDhcpV6System" ) 
		<< "Failed to create my DHCP v4 Helper Thread object!" << endl;
		assert(0);
	}

	// Give BcmDhcpV6ClientSnoopThread a chance to initialize since it will setup NetworkRxSocket()
	// for the network interface.
	BcmOperatingSystemFactory::ThreadSleep(1000);
    #endif

#endif
	// Start an IP thread for each active interface.
    pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->First();
    while (pHal)
    {
		// This must be NetworkTxSocket
		if(pHal->NetworkRxSocket() > 0
			#if (BFC_INCLUDE_DHCPV4_SUPPORT)
			|| pHal->NetworkV4RxSocket() > 0) 
			#else
			)
			#endif
		{			
			#if 1
				//
				BcmEstbIpThread *pIpHelperThd = new BcmEstbIpThread(BcmOperatingSystem::kNormalPriority, pHal->InterfaceNumber());
				if (pIpHelperThd == NULL)
				{
					cout //gFatalErrorMsg( fMessageLogSettings, "BcmDhcpV6System" ) 
					<< "Failed to create my IP helper object!" << endl;
					assert(0);
				}

				BcmEstbIpThread::DocsisMode docsisMode = BcmEstbIpThread::kDocsis20IpInitMode;

				// Make the ESTB IP Helper thread register with the IP stack now that
				// it has been created and initialized.
				pIpHelperThd->RegisterWithIpStack();

				//if( fCmMultiDsHelper.CmDsMacMddSetsState() == BcmCmDsPrimaryChan::kMddSetsOk )
				//{
				//    docsisMode = BcmEstbIpThread::kDocsis30IpInitMode;
				//}
			#endif
			#if 1
				// Start IPV4 initialzation. Hard code this value for now. Later this value should
				// be set by Open calbe host app. after it parsing TLV from eCM to decide which 
				// one should be started. 
				// bool preferredIpVersionIsIPv6 = false;
				// Start IPV6 initialzation.
				//bool preferredIpVersionIsIPv6 = true;
				//bool dualStackOperationEnabled = false;
				bool alternameIpManagementModeEnabled = false;

				// delegate ip initialization to the ip helper.
				if ( !pIpHelperThd->StartIpInit( NULL, //pfCmCfgObuf, 
												  preferredIpVersionIsIPv6, 
												  dualStackOperationEnabled, 
												  alternameIpManagementModeEnabled,
												  docsisMode ) )
				{
					// error: IP helper failed to start.  very unlikely...
					cout //gErrorMsg(fMessageLogSettings, "BcmDhcpV6System")
					<< "pIpHelperThd->StartIpInit() failed.  restarting." << endl;

					assert( 0 );
				}
			//#else
				//gLogMessageRaw << "Skipping DuplicateAddressDetectionOnLinkLocalAddress \n"
				//			   << "Skipping ObtainRouterAdvertisement\n" 
				//			   << endl;
/*				
				// Here we are skipping a couple of initial steps in the IP thread.
				// Kernel sets up the Link Local address. We should verify that it is set before we proceed
				// with DHCPv6.
				{         
					// Remember GetIpv6LinkLocalAddress() cooks up the address based on MAC address and other
					// prefix values. It does not read it from HAL.
					BcmIpV6Address LinkLocal;
					pHal->GetIpv6LinkLocalAddress ( LinkLocal );
        
					// During initialization, we detected interfaces and created HAL If objects.
					// We set HAL If object's fIpv6LinkLocalAddress to the value we got from the detected 
					// interface settings  which kernel set
					// Now we need to call AddIpv6LinkLocalAddress to populate certain object settings.
					gLogMessageRaw << "LinkLocal = " << LinkLocal <<" pHal->Ipv6LinkLocalAddress() = " << pHal->Ipv6LinkLocalAddress()<<endl;
					if ( LinkLocal == pHal->Ipv6LinkLocalAddress() )
					{
						gLogMessageRaw << "LinkLocalAddress is already configured. \n" <<endl;
						// This will actually try to add this address to the interface even if the interface is already configured with the 
						// same address.
						//pHal->AddIpv6LinkLocalAddress();						
					}
				}
*/				
				//pIpHelperThd->ObtainRouterAdvertisement();
			
			BcmOperatingSystemFactory::ThreadSleep(10000);
		
			//	pEth0IpHalIf->DhcpV6ClientIf()->ReleaseAddresses();
			#endif
		}
		pHal = (BcmLnxIpStackIf*) BcmIpHalIfFactory::GetSingleton()->Next(pHal);
    }    
}