
//****************************************************************************
//
// Copyright (c) 2004-2009 Broadcom Corporation
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
//
//****************************************************************************
//
//  Filename:       DsgTunnelSettingsList.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgClientManager 
//
//****************************************************************************

#include "DsgClientManager.h"
#include "DsgClientRegistrar.h"

#include "DsgClient.h"
#include "DsgClientId.h"
#include "DsgClientCtlThread.h"

#include "EthernetPacket.h"

#include <map>
#include "MessageLog.h"

#ifdef APPNAME_SUPPORT
#include "DsgAppNameTable.h"
#endif

#if( CABLECARD_SUPPORT )
#include "DsgccCablecardIf.h"
#endif

BcmDsgClientManager::BcmDsgClientManager()
  :
    fSubscriberMmap(),
	pfDsgClientCtlThread(NULL),
	fMessageLogSettings("DsgClientManager", "Dsg Client Manager"),
	fMacAddrClientCount(0),
	fCaClientCount(0)
{
	// Get my msg log settings to register itself with its command table.
    fMessageLogSettings.Register();	
}

BcmDsgClientManager::~BcmDsgClientManager()
{
}

ostream& BcmDsgClientManager::PrintDetailed( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    // consider flow settings
    ostrm 
        << "\n CLIENT LIST:"
        << endl;

    // consider classifier settings
    if( !empty() )
    {
        SubscriberMmapConstIt tunnelSet_it = begin();
        SubscriberMmapConstIt tunnelSet_end = end();
        while( tunnelSet_it != tunnelSet_end )
        {
            // found classifier settings.
            ostrm << *(tunnelSet_it->second) << endl;

            // increment iterator.
            ++tunnelSet_it;
        }   
    }
#endif

    return ostrm;
}



ostream& BcmDsgClientManager::Print( ostream& ostrm ) const
{
    return PrintDetailed( ostrm );
}


void BcmDsgClientManager::Reset()
{
    if( !fSubscriberMmap.empty() )
    {
        fSubscriberMmap.clear();
    }

	fMacAddrClientCount = 0;
	fCaClientCount = 0;
}

SubscriberMmapIt BcmDsgClientManager::find(uint32 tunnelId)
{
  SubscriberMmapIt tunnelSet_it = begin();

  for( tunnelSet_it; tunnelSet_it!= end(); tunnelSet_it++)
  {
      if(tunnelSet_it->second->TunnelId() == tunnelId ) 
          break;
  }

  return tunnelSet_it;
}


bool BcmDsgClientManager::Subscribe( const uint32 tunnelId, BcmDsgClient *pClient )
{   
    bool dupe_subscriber = false;

    SubscriberMmapIt rng_it = fSubscriberMmap.lower_bound( tunnelId );
    SubscriberMmapIt rng_end = fSubscriberMmap.upper_bound( tunnelId );
  
	if( tunnelId == 0 )
	{
		//gInfoMsg(fMessageLogSettings, "Subscribe") << "Searching client list for unsubscribed clients." << endl;
	}
    while( rng_it != rng_end )
    {
        // found subscriber(s) for this event code.
        if( *(*rng_it).second == *pClient )
        {
            // dupe subscriber.  note it and break out of loop.
            dupe_subscriber = true;

			gInfoMsg(fMessageLogSettings, "Subscribe")
				<<"Subscribe::DUPLICATE CLIENT. Not subscribing." << endl;

            break;
        }
        ++rng_it;
    }
    
    if( !dupe_subscriber )
    {	
		pClient->TunnelId(tunnelId);

		fSubscriberMmap.insert( SubscriberMmap::value_type(tunnelId, pClient));

		// Update counters.
		if( pClient->IdType() == kMacAddressType ) 
			fMacAddrClientCount++;
		else if ( pClient->IdType() == kCaSystemType )
			fCaClientCount++;

		if( tunnelId )
			gInfoMsg(fMessageLogSettings, "Subscribe") <<"\nSubscribed client to tunnel [ ID = " << tunnelId << " ]" << endl;
	}
    
	return (!dupe_subscriber);
}

bool BcmDsgClientManager::UnSubscribe( const uint32 tunnelId, BcmDsgClient *pClient )
{
    if( !fSubscriberMmap.empty() )
    {

        SubscriberMmapIt rng_it = fSubscriberMmap.lower_bound( tunnelId );
        SubscriberMmapIt rng_end = fSubscriberMmap.upper_bound( tunnelId );
    
        while( rng_it != rng_end )
        {
            // found subscriber(s) for this event code.
            if( *(*rng_it).second == *pClient )
            {
                // matching subscriber found.  erase it and return
                // NOTE: we are assuming that no duplicate subscribers have
                // been added to fSubscriberMmap.  Subscribe() should prevent
                // duplicate subscribers from being added to fSubscriberMmap.
                // 
                // Since second element is now an object ptr, need to first delete it
				
				// Update counters.
				if( pClient->IdType() == kMacAddressType ) 
					fMacAddrClientCount--;
				else if ( pClient->IdType() == kCaSystemType )
					fCaClientCount--;

                fSubscriberMmap.erase( rng_it );

                return true;
            }
            ++rng_it;
        }
    }

	return false;
}

// Performing action on the Client List.
// This method is called when validating DCD.
// When DCD is found - DCD must be validated
// or DCD is found - SourceSubTable acquired - DCD must be validated again.
// This method passes this tunnel's token to its clients to hold so that SubscribeClients() method
// can identify which clients to subscibe to this tunnel.
bool BcmDsgClientManager::FindAndSetClients( BcmDsgEstbTunnelSettings *pTunnel, BcmDsgClientId* pClientId, uint8 token, bool &duplicateClient, bool &openTunnel, bool &openSecretTunnel )
{
	bool found = false;

	// Search the client list 
	// Try to find a match for the given client ID object.
	SubscriberMmapIt client_begin = begin();
	SubscriberMmapIt client_end = end();
	uint32 unsubscribed = 0;
//    SubscriberMmapIt client_begin = fSubscriberMmap.lower_bound( unsubscribed );
//    SubscriberMmapIt client_end = fSubscriberMmap.upper_bound( unsubscribed );

	while( client_begin != client_end )
	{
		// Only search the ones that do not receive any tunnel data.
//		if( client_begin->second.TunnelId() == 0 )
		// If client has ID in text format and it is not mapped to an ID number.
		// Can't process it.
		// There is no need to do this check because if text name is not mapped
		// the nthe client ID s will not match anyway. IsEqual operator check for that.
		//if( !((client_begin->IdType() == kApplicationNameType) && (!client_begin->Mapped)) )
		{
			// Search for a match.
			if( (*pClientId) == *(client_begin->second) ) 
			{
				// Check if this client is already assigned a tunnel ID.
				// If so, indicate that the tunnel of which we are processing
				// the client ID list now must be ignored. 
				if( (client_begin->second->TunnelId() != 0)
					&& (client_begin->second->TunnelId() != pTunnel->TunnelId())
				  )
				{
					duplicateClient = true;
					gInfoMsg(fMessageLogSettings, "FindAndSetClients")
						<< "Duplicate Client: This client is included in more than one DSG rule. This client is already assigned a tunnel id = "
						<< client_begin->second->TunnelId()
						<< endl;
					
					// Do not assign Token to dupe clients.
				}
				else
				{
					// Give the client a token as an identifier to hold. When we open the tunnel
					// we will know the tunnel ID. When we know the tunnel ID we will
					// subscribe the clients that hold this token.
					client_begin->second->Token(token);

					if( !(client_begin->second->fIpClassifierSetsSpecified) ||
						 (client_begin->second->fIpClassifierSetsSpecified && pTunnel->IsEqualClassifier(client_begin->second->fIpClassifierSets)))
					{
						openTunnel = true;
						gInfoMsg(fMessageLogSettings, "FindAndSetClients")
							<< "FindAndSetClients - openTunnel = true for  "
							<< *client_begin->second
							<< endl;
					}
					else
					{
						openSecretTunnel = true;
						gInfoMsg(fMessageLogSettings, "FindAndSetClients")
							<< "FindAndSetClients - openSecretTunnel = true for  "
							<< *client_begin->second
							<< endl;
					}

				
					// we are happy if we match at least one client.
					found = true;

					gInfoMsg(fMessageLogSettings, "FindAndSetClients")
						<< " Found " << client_begin->second->IdTypeName((uint32)client_begin->second->IdType()) 
						<< " type client for this tunnel" 
						<< endl;
				}
			}
		}

		// Try to find a match for the rest of the tunnels' clients.
		client_begin++;
	}

	return found;
}

// Performing action on the Client List.
bool BcmDsgClientManager::OpenTunnelsForMacAddressTypeClients( void ) 
{
	if( fMacAddrClientCount == 0 )
	{
		return false;
	}

	bool found = false;

	// Search the client list 
	// Try to find a match for the given client ID object.
	SubscriberMmapIt client_begin = begin();
	SubscriberMmapIt client_end = end();

	while( client_begin != client_end )
	{		
		// Only search the ones that do not receive any tunnel data.
		if( (client_begin->second->IdType() == kMacAddressType) ) 
		{
			if(client_begin->second->TunnelId() == 0)
			{			
				BcmMacAddress tunnelAddress = client_begin->second->ClientAddress();				
				uint32 tunnelId = pfDsgClientCtlThread->OpenTunnel(tunnelAddress);
				if( tunnelId != 0 )
				{
					found = true;

					// Give client its tunnel ID.
					client_begin->second->TunnelId(tunnelId);
				
					// We can insert before we erase because this is a multimap.
					// It is safe to copy the client_begin->second ptr to the new multimap location. 
					fSubscriberMmap.insert( SubscriberMmap::value_type( tunnelId, client_begin->second ) );
					
					// Update the key of the mulltimap entry.
					// Sorting is done based on the key.
					// client_begin->first = tunnelId; Since this is readonly
					// we have to remove and insert this client with the new tunnel ID.
					fSubscriberMmap.erase( client_begin++ );		
				}
				else 
				{										
					gAlwaysMsgNoFields(fMessageLogSettings) << "A tunnel cannot be opened for " << tunnelAddress << endl;					
					++client_begin;
				}
			}
			else
			{
				found = true;
				++client_begin;
			}
		}
		else
		{			
			// Try to find a match for the rest of the tunnels' clients.
			++client_begin;
		}
	}

	return found;
}

bool BcmDsgClientManager::MustFindSrcSubTable(void)
{
	// Check if the client list has any client listed
	// that provides its ID in text format.

	// Search the client list 
	// Try to find a match for the given client ID object.
	SubscriberMmapIt client_begin = begin();
	SubscriberMmapIt client_end = end();

	while( client_begin != client_end )
	{
		//if( (client_begin->second->IdType() == kApplicationNameType) && (!client_begin->second->NameMapped()) )
		if( client_begin->second->NameMapped() == false )
		{
			return true;
		}
		client_begin++;
	}

	return false;
}

// UnsubscribeAll
bool BcmDsgClientManager::UnsubscribeAll(void)
{
	BcmDsgClientRegistrarThread* pBcmDsgClientRegistrarThread = BcmDsgClientRegistrarThread::GetSingletonInstance();

	SubscriberMmapIt client_begin = begin();
	SubscriberMmapIt client_end = end();

	while( client_begin != client_end )
	{
		if( (client_begin->second->TunnelId() != 0) )
		{
			gInfoMsg(fMessageLogSettings, "UnsubscribeAll") << "UnsubscribeAll - \n" << client_begin->second 
			<< endl;
			// Unsubscribe
			uint8 reset = 0;
			client_begin->second->TunnelId(0);
			client_begin->second->Token(reset);
			client_begin->second->Active(false);
			client_begin->second->TunnelStatus(BcmDsgClient::kDsgccTunnelPending);

			pBcmDsgClientRegistrarThread->updateDsgClientTunnelStatus(client_begin->second);

			// We can insert before we erase because this is a multimap.
			// It is safe to copy the client_begin->second ptr to the new multimap location. 
			fSubscriberMmap.insert( SubscriberMmap::value_type( 0, client_begin->second ) );
				
			// Update the key of the mulltimap entry.
			// we have to remove and insert this client with the new tunnel ID.
			fSubscriberMmap.erase( client_begin++ );

		}
		else
		{
			++client_begin;
		}
	}

	return false;
}

// Subscribe
bool BcmDsgClientManager::SubscribeClients( BcmDsgEstbTunnelSettings *pTunnel )
{
//	SubscriberMmapIt client_begin = begin();
//	SubscriberMmapIt client_end = end();
	uint8 unsubscribed = 0;
    SubscriberMmapIt client_begin = fSubscriberMmap.lower_bound( unsubscribed );
    SubscriberMmapIt client_end = fSubscriberMmap.upper_bound( unsubscribed );

	// gInfoMsg(fMessageLogSettings, "SubscribeClients") <<"SubscribeClients - Searching unsubscribed/no_classifier client list for a match." << endl;

	while( (client_begin != client_end) )
	{
		//gInfoMsg(fMessageLogSettings, "SubscribeClients") <<"SubscribeClients - client's token = " << (uint32) client_begin->second->Token()
		//	 << " tunnel token = " << (uint32) pTunnel->Token() << endl;

		// Find the clients that hold this tunnel's token. Only process unsubscribed clients.
		// One tunnel per client.
		if( (client_begin->second->Token() == pTunnel->Token()) && (!client_begin->second->fIpClassifierSetsSpecified)) // && !client_begin->second->Active() )
		{		
			// Subscribe
			client_begin->second->TunnelId(pTunnel->TunnelId());
			
			client_begin->second->Token(unsubscribed);

			gInfoMsg(fMessageLogSettings, "SubscribeClients") <<" Subscribed client to tunnel [ ID = " << pTunnel->TunnelId() << " ]" << endl;
				 //<< (client_begin->second) << endl; 

			// We can insert before we erase because this is a multimap.
			// It is safe to copy the client_begin->second ptr to the new multimap location. 
			fSubscriberMmap.insert( SubscriberMmap::value_type( client_begin->second->TunnelId(), client_begin->second ) );
				
			// Update the key of the mulltimap entry.
			// we have to remove and insert this client with the new tunnel ID.
			fSubscriberMmap.erase( client_begin++ );
			
		}
		else
		{
			++client_begin;
		}
	}

	//gInfoMsg(fMessageLogSettings, "SubscribeClients") << "CLIENT LIST SIZE  = "<< size() << endl;
	return true;
}


void BcmDsgClientManager::Set( BcmDsgClientCtlThread *pDsgClientCtlThread )
{
	pfDsgClientCtlThread = pDsgClientCtlThread;
}




void BcmDsgClientManager::ForwardToClient( uint32 tunnelId, uint8* pBuffer, uint16 length, BcmEthernetPacket &tunnelPacket, bool applyClassifierRules  )
{
    CallTrace("BcmDsgClientManager", "ForwardToClient");

	// Check if the tunnel is broadcast and reassembly is needed.
	// IsBroadcastAndReassembled()

    if( !empty() )
    {
        SubscriberMmapIt rng_it = fSubscriberMmap.lower_bound( tunnelId );
        SubscriberMmapIt rng_end = fSubscriberMmap.upper_bound( tunnelId );

        while( rng_it != rng_end )
        {
            // found a subscriber. notify the subscriber.			
			if( ApplyRules(tunnelPacket, (*rng_it).second, applyClassifierRules) )
			{
				if ((*rng_it).second->ProcessTunnelPacket(tunnelPacket) < 0)
				{
					 // Client is unreachable, remove it.
					 if (pfDsgClientCtlThread)
					 {
						gInfoMsg(fMessageLogSettings, "ForwardToClient") << "BcmDsgClientManager::ForwardToClient- Can't send packet to client." << endl;
						pfDsgClientCtlThread->DeregisterClient( (*rng_it).second );
					 }
				}
			}
			
            ++rng_it;
        }
    }
}


// Performing action on the Client List.
// This method is called when UCID is received.
// This method processes previously subscribed clients and switches the 
// to a new tunnel if necessary.
bool BcmDsgClientManager::SetOrSwitchClients( BcmDsgClientId* pClientId, uint8 token ) 
{
	bool found = false;

	// Search the client list 
	// Try to find a match for the given client ID object.
//	SubscriberMmapIt client_begin = begin();
//	SubscriberMmapIt client_end = end();
	uint32 unsubscribed = 0;
    SubscriberMmapIt client_begin = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

	while( client_begin != client_end )
	{
		// Only search the ones that do not receive any tunnel data.
//		if( client_begin->second->TunnelId() == 0 )
		// If client has ID in text format and it is not mapped to an ID number.
		// Can't process it.
		// There is no need to do this check because if text name is not mapped
		// the nthe client ID s will not match anyway. IsEqual operator check for that.
		//if( !((client_begin->IdType() == kApplicationNameType) && (!client_begin->Mapped)) )
		
		gInfoMsg(fMessageLogSettings, "SetOrSwitchClients")
				<< *pClientId << *(client_begin->second)
				<< endl;

		// Search previously SUBSCRIBED clients for a match.
		if( (*pClientId) == *(client_begin->second) ) 
		{
			gInfoMsg(fMessageLogSettings, "SetOrSwitchClients")
					<< "Matched."
					<< endl;

			// Check if this client's tunnel matches any of the open tunnels' 
			// settings except the UCID.
			// Update the passed in clientID object. 
			// Point it to this client.
			// This client object stores the socket port number.

			// Give the client a token as an identifier to hold. When we open the tunnel
			// we will know the tunnel ID. When we know the tunnel ID we will
			// subscribe the clients that hold this token.
			client_begin->second->Token(token);
			
			if( client_begin->second->TunnelId() != 0 )
			{
				gInfoMsg(fMessageLogSettings, "SetOrSwitchClients")
					<< "Preparing the client to switch to the new tunnel." 
					<< endl;
   
				// this client is already assigned to a tunnel.
				// Now switch it to this tunnel.
				// We will revise the other tunnel after we are done with this
				// process. We will decide whether to close it or not.
				// Now insert this client back in as an unsubscribed client.
				// Since it holds the token, we will subscribe it to the tunnel
				// after the tunnel is opened and we know what the tunnel id is.
				client_begin->second->TunnelId(unsubscribed);

				// We can insert before we erase because this is a multimap.
				// It is safe to copy the client_begin->second ptr to the new multimap location. 
				fSubscriberMmap.insert( SubscriberMmap::value_type( unsubscribed, client_begin->second ) );
					
				// Update the key of the mulltimap entry.
				// we have to remove and insert this client with the new tunnel ID.
				fSubscriberMmap.erase( client_begin++ );					
			}
			else
			{
				client_begin++;
			}

			// we are happy if we match at least one client.
			found = true;

			gInfoMsg(fMessageLogSettings, "SetOrSwitchClients")
				<< "Found Client for this tunnel " //with token = " << (uint32)token 
				<< endl;
		}
		else
		{
			gInfoMsg(fMessageLogSettings, "SetOrSwitchClients")
				<< "This client's ID is not specified in this tunnel"
				<< endl;	

			// Try to find a match for the rest of the tunnels' clients.
			client_begin++;
		}	
	}

	return found;
}


// This method returns number of clients per tunnel.
uint32 BcmDsgClientManager::NumberOfClients(uint32 tunnelId)
{
	uint32 clients = 0;

    if( !empty() )
    {
        SubscriberMmapIt rng_it = fSubscriberMmap.lower_bound( tunnelId );
        SubscriberMmapIt rng_end = fSubscriberMmap.upper_bound( tunnelId );

        while( rng_it != rng_end )
        {			
			clients++;
			
            ++rng_it;
        }
    }

	return clients;
}

// This method sends a notification of a change in Advanced Mode to each client
void BcmDsgClientManager::NotifyClientsOfAdvancedMode(int mode) {

  SubscriberMmapIt client = fSubscriberMmap.begin();
  SubscriberMmapIt client_end = fSubscriberMmap.end();

  while( client != client_end ) {
	  client->second->advancedModeChangeNotification(mode);
    client++;
  }

}

// This method sends a notification of a change in Two-way Mode to each client
void BcmDsgClientManager::NotifyClientsOfTwoWayMode(int mode) {

  SubscriberMmapIt client = fSubscriberMmap.begin();
  SubscriberMmapIt client_end = fSubscriberMmap.end();

  while( client != client_end ) {
    client->second->twoWayModeChangeNotification(mode);
    client++;
  }

}

// This method sends a notification of tunnel present to each client
// 1) BASIC mode: first DSG packet is received
// 2) ADVANCE mode: first DCD message is received.
void BcmDsgClientManager::NotifyClientsOfTunnelsPresent(void)
{
    SubscriberMmapIt client = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

    while( client != client_end ) {
        client->second->tunnelPresenceChangeNotification(1); //non-zero is present
      client++;
    }
}

// This method sends a notification to each client after a complete downstream scan 
// as an indicator that the eCM,
// Advanced Mode: Has been unable to identify a downstream channel with a DCD message.
// Basic Mode: Has been unable to find a DSG tunnel with a wellknown MAC address.
void BcmDsgClientManager::NotifyClientsOfDsScanComplete(void)
{
    SubscriberMmapIt client = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

#if (CABLECARD_SUPPORT)
    // special case: cable unplugged(no clients) and in dsg Adv mode/Basic mode
    if( (client == client_end) /*&& (pfDsgClientCtlThread->Mode() == kDsgAdvanceMode)*/ )
    {
        BcmCableCardIfThread *pCableCardIf = BcmCableCardIfThread::GetSingletonInstance();
        if( pCableCardIf )
            pCableCardIf->sendDsScanCompleteToCableCard();
        return;
    }
#endif

    while( client != client_end ) {
        client->second->DsgDsScanCompleteNotification();
      client++;
    }
}
    
// This method sends a notification of to each client after
// eCM has established two-way communication and is providing 
// the Upstream Channel ID (UCID) of the upstream channel.
void BcmDsgClientManager::NotifyClientsOfUcid2wayOK(uint8 ucid)
{
    SubscriberMmapIt client = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

    while( client != client_end ) {
      client->second->DsgUcid2wayOKNotification(ucid);
      client++;
    }
}
    
// This method sends a notification to each client that 
// the eCM has transmitted a DCCRSP(Depart) on the existing upstream 
// channel and is preparing to switch to a new upstream or downstream channel
void BcmDsgClientManager::NotifyClientsOfDccDepart(uint8 init_type)
{
    SubscriberMmapIt client = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

    while( client != client_end ) {
      client->second->DsgDccDepartNotification(init_type);
      client++;
    }
}

// This method sends a notification to each client what type of eCM provisioning 
// limitations impact eCM 2-Way forwarding. Values below are bit fields that can 
// be ORed to indicate multiple conditions.
// 0x01  Network access disabled (NACO=0)
// 0x02  Max CPE limit exhausted
void BcmDsgClientManager::NotifyClientsOfTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations )
{
    SubscriberMmapIt client = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

    while( client != client_end ) {
      client->second->DsgTwoWayForwardingDisabledNotification( twoWayForwardingLimitations );
      client++;
    }
}

bool BcmDsgClientManager::ForwardVspData( BcmType43VendorSettings &vendorSettings )
{   
	BcmOctetBuffer tempBuffer(0);
	
	// Overlay the VSP data.
	tempBuffer.Overlay(vendorSettings.VendorSpecificData());
	
	// The Vendor Id 
	uint8 vendorId[3];
	
	vendorSettings.VendorId( vendorId[0], vendorId[1], vendorId[2]);

	// Place vendor id into a 32-bit variable to use it to search
	// the client list.
	uint32 clientVendorId;

    clientVendorId  = vendorId[0] << 16;
    clientVendorId |= vendorId[1] << 8;
    clientVendorId |= vendorId[2] << 0;

    SubscriberMmapIt client_begin = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

	while( client_begin != client_end )
	{
		// Look for a matching client ID.
		if( client_begin->second->Id() == clientVendorId ) 
		{
			// Forward VSP data to client.
			if (client_begin->second->TunnelPacket(tempBuffer.AccessBuffer(), tempBuffer.BytesUsed()) < 0)
			{
				 // Client is unreachable, remove it.
				 if (pfDsgClientCtlThread)
				 {
					gInfoMsg(fMessageLogSettings, "ForwardVspData") << "BcmDsgClientManager::ForwardToClient- Can't send packet to client." << endl;
					pfDsgClientCtlThread->DeregisterClient( client_begin->second );
				 }
			}
		}
		
		++client_begin;
	}

	return true;
}

// This method searches the client list by a unique port number
// to find whether any client with an open tunnel has this specific 
// port number.
bool BcmDsgClientManager::IsClientAssignedTunnel(uint32 &port)
{
    SubscriberMmapIt client_begin = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

	while( client_begin != client_end )
	{
		// Look for a matching client port.
		if( client_begin->second->ClientPort() == port ) 
		{
			// If the client is assigned a tunnel, then
			// the client's tunnel ID must be bigger than 0.
			if( client_begin->second->TunnelId() > 0 )
			{
				return true;
			}
			break;
		}
		
		++client_begin;
	}

	return false;
}

// This method searches the client list by a unique port number
// and returns a pointer to the client.
BcmDsgClient *BcmDsgClientManager::FindByPort(uint32 &port)
{
    SubscriberMmapIt client_begin = fSubscriberMmap.begin();
    SubscriberMmapIt client_end = fSubscriberMmap.end();

	while( client_begin != client_end )
	{
		// Look for a matching client port.
		if( client_begin->second->ClientPort() == port ) 
		{
			return client_begin->second;
		}
		
		++client_begin;
	}

	return 0;
}

#ifdef APPNAME_SUPPORT
// Performing action on the Client List.
// This method is called when a new SNS Table is received.
bool BcmDsgClientManager::AssignClientIds( BcmDSGAppNameTable *pAppNameTable ) 
{
	bool mapped = false;

	// Search the client list 
	// Try to find a match for the given client ID object.
	SubscriberMmapIt client_begin = begin();
	SubscriberMmapIt client_end = end();

	while( client_begin != client_end )
	{
		//gInfoMsg(fMessageLogSettings, "AssignClientIds") << client_begin->second << endl;

		// Search for application name type clients without an ID.
		if( !client_begin->second->NameMapped() ) 
		{	
			pAppNameTable->Dump();

			BcmString appName = client_begin->second->ClientName();
			// Use the SNS Table to map the name to an ID.
			client_begin->second->Id( pAppNameTable->AppId((char*)appName.c_str()) );

			// Flag if the name is mapped (i.e., not zero)
			if( client_begin->second->Id() != 0 )
			{
				// Not sure whether we should do this, but the client has to have
				// a type that is defined in the spec.
				client_begin->second->IdType( kApplicationType );

				gInfoMsg(fMessageLogSettings, "AssignClientIds") << "Client " << client_begin->second->ClientName() 
					 << "is assigned ID= " << client_begin->second->Id() << endl;
				mapped = true;
			}
			else
			{
				gInfoMsg(fMessageLogSettings, "AssignClientIds") << "Can't map  " << client_begin->second->ClientName() << endl;
			}
		}

		// Try to find a match for the rest of the tunnels' clients.
		client_begin++;
	}

	return mapped;
}
#endif

void BcmDsgClientManager::SubscribeClientsWithClassifiers( BcmDsgEstbTunnelSettings *pTunnel )
{
	uint8 unsubscribed = 0;
    SubscriberMmapIt client_begin = fSubscriberMmap.lower_bound( unsubscribed );
    SubscriberMmapIt client_end = fSubscriberMmap.upper_bound( unsubscribed );

	while( client_begin != client_end )
	{
		//gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers") <<"SubscribeClientsWithClassifiers - client's token = " << client_begin->second->Token()
		//	 << "tunnel token = " << pTunnel->Token() << endl;

		// Find the clients that hold this tunnel's token. Only process unsubscribed clients.
		// One tunnel per client.
		if( (client_begin->second->Token() == pTunnel->Token()) && (client_begin->second->fIpClassifierSetsSpecified) )
		{
			// gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers") << "tokens matched and client has classifier specified." << endl;

			// Process the ones that have a classifier
			// Check if this client specified a classifier.
			// If so, check if classifier matches one of tunnel classifiers.
			// If so, subscribe client now. We will apply the classifier to tunnel packets prior to forwarding
			// them to this client.
			if( pTunnel->IsEqualClassifier(client_begin->second->fIpClassifierSets) )
			{
				// Subscribe
				client_begin->second->TunnelId(pTunnel->TunnelId());
				client_begin->second->Token(unsubscribed);

				gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers") <<" Subscribed client to tunnel [ ID = " << pTunnel->TunnelId() << " ]" 
					 << (*client_begin->second) 
					 << "Tunnel data will be filtered prior to forwarding to the client." << endl; 

				pTunnel->fClientRequestsSubTunnelForwarding = true;

				// We can insert before we erase because this is a multimap.
				// It is safe to copy the client_begin->second ptr to the new multimap location. 
				fSubscriberMmap.insert( SubscriberMmap::value_type( client_begin->second->TunnelId(), client_begin->second ) );
					
				// Update the key of the mulltimap entry.
				// we have to remove and insert this client with the new tunnel ID.
				fSubscriberMmap.erase( client_begin++ );
			}
			else
			{
				// Reset client's token.
				client_begin->second->Token(unsubscribed);

				// Classifier does not match any of the tunnel classifiers. Do not subscribe 
				// this client to this tunnel. Open a new tunnel for this client. 
			
				gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers")
					<< "Client requested a classifier that does not exist in this tunnel's settings. We will open a secret tunnel." << endl;

				// OPEN A NEW TUNNEL
				//BcmDsgEstbTunnelSettings newTunnel;

				// Set the mac address.
				//newTunnel.TunnelAddress(pTunnel->TunnelAddress());

				// Form the classifier object.
				BcmDsgClassifierSettings fClsfrSettings;
				fClsfrSettings.fIpClassifierSetsSpecified = true;
				fClsfrSettings.fIpClassifierSets = client_begin->second->fIpClassifierSets;

				BcmMacAddress mac = pTunnel->TunnelAddress();
				uint32 tunnelId = pfDsgClientCtlThread->OpenTunnel(mac, &fClsfrSettings);
				if( tunnelId != 0 )
				{
					// Subscribe
					client_begin->second->TunnelId(tunnelId);						

					gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers") <<" Subscribed special client to secret tunnel [ ID = " << tunnelId << " ]" 
						 << (*client_begin->second) << endl; 

					// We can insert before we erase because this is a multimap.
					// It is safe to copy the client_begin->second ptr to the new multimap location. 
					fSubscriberMmap.insert( SubscriberMmap::value_type( client_begin->second->TunnelId(), client_begin->second ) );
						
					// Update the key of the mulltimap entry.
					// we have to remove and insert this client with the new tunnel ID.
					fSubscriberMmap.erase( client_begin++ );
				}
				else
				{
					++client_begin;
				}
			}			
		}
		else
		{
			++client_begin;
		}
	}

	//gInfoMsg(fMessageLogSettings, "SubscribeClientsWithClassifiers") << "CLIENT LIST SIZE  = "<< size() << endl;
	return;
}

// Applies the specific classification rules and parameters to the packet
// specified, returning true if the packet is "classified" as belonging to
// the service flow, or false if the packet doesn't belong here.
//
// Parameters:
//      ethernetPacket - the packet to be classified.
//
// Returns:
//      true if the packet matches all of the classifier rules.
//      false if the packet does not match all of the classifier rules.
//
bool BcmDsgClientManager::ApplyRules(BcmEthernetPacket &ethernetPacket, BcmDsgClient *pClient, bool applyClassifierRules )
{
	// If there are no classifiers just retrun
	if( !pClient->fIpClassifierSetsSpecified )
	{
		return true;
	}

	// If this tunnel is a private tunnel for a client then there is no need to apply classifier rules
	// because eCM aplies classifiers to private tunnel packets.
	if( !applyClassifierRules )
	{
		return true;
	}

    // Force the ethernet packet to parse the IP header so that we can use
    // the accessors.
    ethernetPacket.ParseIp();

    // PR2014 - Never classify TCP/UDP IP fragments (except for the first
    // fragment).  We don't keep any state information, so it isn't possible to
    // know whether or not the first fragment was classified here or elsewhere.
    // If it was classified elsewhere, then the remaining fragments certainly
    // don't belong here, so we need to pass them along to the primary flow.
    if (ethernetPacket.IsIpPktParsed() &&
        ethernetPacket.IpProtocolTypeTcpUdpParsed() &&
        (ethernetPacket.FragmentOffsetParsed() != 0))
    {
        #if DEBUG_CLASSIFIERS
        gInfoMsgNoFields(fMessageLogSettings)
            << "FAILED!  (Fragmented TCP/UDP IP Packet, and frag offset > 0; see PR2014)" << endl;
        #endif

        return false;
    }	

	// Go through the classifier list and apply the rules.
	//ClassifierSettingsListIterator cls_it = begin();
	
    // This produces "statement with no effect" warning.  Slight
    // rework:
    // for( cls_it; cls_it!= end(); cls_it++)
    //while (cls_it != end())
	{
		if( ApplyClassifierRules(ethernetPacket, pClient) == true )
		{
			return true;
		}
        
    //    cls_it++;
	}

	return false;
}

// Applies the specific classification rules and parameters to the packet
// specified, returning true if the packet is "classified" as belonging to
// the service flow, or false if the packet doesn't belong here.
//
// Parameters:
//      ethernetPacket - the packet to be classified.
//
// Returns:
//      true if the packet matches all of the classifier rules.
//      false if the packet does not match all of the classifier rules.
//
bool BcmDsgClientManager::ApplyClassifierRules(BcmEthernetPacket &ethernetPacket, BcmDsgClient *pClient ) 
{
    bool anyRulesApplied = false;

    #if DEBUG_CLASSIFIERS
    gInfoMsg(fMessageLogSettings, "ApplyRules")
        << "Applying classifier rules:\n";
    #endif
    
		//BcmDsgClassifierSettings fSettings = cls_it->second;

		// Check IP fields, if any were specified.
		//if (fSettings.fIpClassifierSetsSpecified)
		{
			anyRulesApplied = true;
        
			// Make sure that this is an IP packet.  If not, then it can't match
			// any IP rules.
			//
			// UNFINISHED:  There is some contention regarding this; some argue that
			// the rules don't apply, and therefore don't cause it to fail.  I think
			// that classifiers are meant to be inclusion filters, meaning if you
			// are looking for packets with certain IP field value in the packet,
			// then only IP packets with the specified value should be included.
			// All others must not be included.
			if (!ethernetPacket.IsIpPktParsed())
			{
				#if DEBUG_CLASSIFIERS
				gAlwaysMsgNoFields(fMessageLogSettings)
					<< "[22/23].9 FAILED!  (Not an IP packet)" << endl;
				#endif

				return false;
			}

			// Check the IP Source Address, if specified.
			if (pClient->fIpClassifierSets.SourceIpAddrSpecified())
			{
				#if DEBUG_CLASSIFIERS
				gAlwaysMsgNoFields(fMessageLogSettings)
					<< "[22/23].9.3/4 IP Src Addr & Mask; ";
				#endif

				// Do the comparison, with masking applied.
				if (!ethernetPacket.SourceIpAddressParsed().IsEqualWithMask(pClient->fIpClassifierSets.SourceIpAddr(),
																			pClient-> fIpClassifierSets.SourceIpMask()))
				{
					// Doesn't match!
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!" << endl;
					#endif

					return false;
				}
			}

			// Check the IP Destination Address, if specified.
			if (pClient->fIpClassifierSets.DestIpAddrSpecified())
			{
				#if DEBUG_CLASSIFIERS
				gAlwaysMsgNoFields(fMessageLogSettings)
					<< "[22/23].9.5/6 IP Dest Addr & Mask; ";
				#endif

				// Do the comparison, with masking applied.
				if (!ethernetPacket.DestIpAddressParsed().IsEqualWithMask(pClient->fIpClassifierSets.DestIpAddr(),
																		  pClient->fIpClassifierSets.DestIpMask()))
				{
					// Doesn't match!
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!" << endl;
					#endif

					return false;
				}
			}
    
			// Check the TCP/UDP Source/Destination Port, if this is a TCP or UDP
			// packet.  According to the strictest wording in the spec, this check
			// will always take place because the start and end port parameters are
			// not irrelevant if not specified.
			//
			// Instead, I have interprited the spec to mean "if neither of the
			// start/end TLVs are present, then comparison of the port number is
			// irrelevant".  Thus, I only do this check if at least one of the
			// source/dest start/end port numbers is specified.
			if (pClient->fIpClassifierSets.SourcePortStartSpecified() ||
				pClient->fIpClassifierSets.SourcePortEndSpecified())
			{
				#if DEBUG_CLASSIFIERS
				gAlwaysMsgNoFields(fMessageLogSettings)
					<< "[22/23].9.8/9 TCP/UDP Source Port; ";
				#endif

				// Force the ethernet packet to parse the TCP/UDP header so we can
				// use the accessors.
				ethernetPacket.ParseTcpUdp();

				// Now, according to my usual "inclusion filter" interpritation of
				// classifiers, if we are looking for a specific range of port
				// numbers, and this is not a TCP or UDP packet, then it must not
				// be included.
				if (!ethernetPacket.IpProtocolTypeTcpUdpParsed())
				{
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!  (Not TCP/UDP)" << endl;
					#endif

					return false;
				}

				// Now that I have determined that a Source Port exists, then I can
				// check the range.
				if ((ethernetPacket.SourceTcpUdpPortParsed() < pClient->fIpClassifierSets.SourcePortStart()) ||
					(ethernetPacket.SourceTcpUdpPortParsed() > pClient->fIpClassifierSets.SourcePortEnd()))
				{
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!" << endl;
					#endif

					return false;
				}
			}
			if (pClient->fIpClassifierSets.DestPortStartSpecified() ||
				pClient->fIpClassifierSets.DestPortEndSpecified())
			{
				#if DEBUG_CLASSIFIERS
				gAlwaysMsgNoFields(fMessageLogSettings)
					<< "[22/23].9.10/11 TCP/UDP Dest Port; ";
				#endif

				// Force the ethernet packet to parse the TCP/UDP header so we can
				// use the accessors.
				ethernetPacket.ParseTcpUdp();

				// Now, according to my usual "inclusion filter" interpritation of
				// classifiers, if we are looking for a specific range of port
				// numbers, and this is not a TCP or UDP packet, then it must not
				// be included.
				if (!ethernetPacket.IpProtocolTypeTcpUdpParsed())
				{
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!  (Not TCP/UDP)" << endl;
					#endif

					return false;
				}

				// Now that I have determined that a Source Port exists, then I can
				// check the range.
				if ((ethernetPacket.DestTcpUdpPortParsed() < pClient->fIpClassifierSets.DestPortStart()) ||
					(ethernetPacket.DestTcpUdpPortParsed() > pClient->fIpClassifierSets.DestPortEnd()))
				{
					#if DEBUG_CLASSIFIERS
					gAlwaysMsgNoFields(fMessageLogSettings)
						<< "FAILED!" << endl;
					#endif

					return false;
				}
			}
		
	}	
    // Finally, do whatever vendor-specific classifier checks that were
    // specified.  Since our eCM currently doesn't support any vendor-specific
    // classifiers, we'll ignore these.


    // If we get to the end without finding any mis-matches, then this
    // classifier wins, and returns true.
    //
    // NOTE:  There is some contention about this; various individuals have
    // argued that a classifier only matches if one or more of the rules have
    // been applied AND they match.  If no rules were specified, then nothing
    // will have been checked; should this be an automatic match or a mismatch?
    // There seems to be overwhelming support for it being a mismatch 
    // (especially now that there is explicit support for a catch-all
    // classifier), so this is what we have implemented.
    if (!anyRulesApplied)
    {
        #if DEBUG_CLASSIFIERS
        gAlwaysMsgNoFields(fMessageLogSettings)
            << "PASSING!  (No classifier rules applied)";
        #endif
    }
    else
    {
        #if DEBUG_CLASSIFIERS
        gAlwaysMsgNoFields(fMessageLogSettings)
            << "MATCHED!";
        #endif
    }

    #if DEBUG_CLASSIFIERS
    gAlwaysMsgNoFields(fMessageLogSettings) << endl;
    #endif

    return true;
}

bool BcmDsgClientManager::AllCaTunnelRequestsSatisfied()
{
	uint8 unsubscribed = 0;
    SubscriberMmapIt client_begin = fSubscriberMmap.lower_bound( unsubscribed );
    SubscriberMmapIt client_end = fSubscriberMmap.upper_bound( unsubscribed );

	while( client_begin != client_end )
	{
		if(client_begin->second->IdType() == kCaSystemType)
		{
			gInfoMsg(fMessageLogSettings, "AllCaTunnelRequestsSatisfied")				
				<< *client_begin->second <<"\n"
				<<" Tunnel request could not be satisfied."
				<< endl;
			return false;
		}

		client_begin++;
	}

	gInfoMsg(fMessageLogSettings, "AllCaTunnelRequestsSatisfied")				
		<<" All CA tunnel requests are satisfied or no CA tunnel request exists."
		<< endl;

	return true;
}

bool BcmDsgClientManager::CaTunnelRequestPresent()
{
	SubscriberMmapIt tunnelSet_it = begin();

	for( tunnelSet_it; tunnelSet_it!= end(); tunnelSet_it++)
	{
		if(tunnelSet_it->second->IdType() == kCaSystemType)
		{
			return true;
		}
	}
	return false;
}

void BcmDsgClientManager::UpdateStatusNotifyClients()
{
	BcmDsgClientRegistrarThread* pBcmDsgClientRegistrarThread = BcmDsgClientRegistrarThread::GetSingletonInstance();

	SubscriberMmapIt tunnelSet_it = begin();
	for( tunnelSet_it; tunnelSet_it!= end(); tunnelSet_it++)
	{
		// If the tunnel was previously open and is not found in the current VALID DCD
		// then tell client that its tunnel is NOT AVAILABLE now.
		if(tunnelSet_it->second->TunnelStatus() == BcmDsgClient::kDsgccTunnelPending)
		{
			 if(tunnelSet_it->second->TunnelId() == 0)
			{
				// Update status.
				tunnelSet_it->second->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelNotAvailable);

				// Since DsgClientRegistrar keeps a duplicate list if clients we must update its status as well 
				// because dsgcc_KeepAliveClient API returns client's tunnel status from DsgClientRegistrar client list.
				pBcmDsgClientRegistrarThread->updateDsgClientTunnelStatus(tunnelSet_it->second);
			}
			else
			{
				// Tunnel is open. No need to notify client. Just update status.
				tunnelSet_it->second->TunnelStatus(BcmDsgClient::kDsgccTunnelOpen);

				// Since DsgClientRegistrar keeps a duplicate list if clients we must update its status as well 
				// because dsgcc_KeepAliveClient API returns client's tunnel status from DsgClientRegistrar client list.
				pBcmDsgClientRegistrarThread->updateDsgClientTunnelStatus(tunnelSet_it->second);
			}
		}

		// If this is the first time client's tunnel request is being processed then ..
		if(tunnelSet_it->second->TunnelStatus() == BcmDsgClient::kDsgccTunnelRequestPending)
		{
			if(tunnelSet_it->second->TunnelId() != 0)
			{
				tunnelSet_it->second->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelOpen);
			}
			else
			{
				tunnelSet_it->second->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelNotAvailable);
			}

			// Since DsgClientRegistrar keeps a duplicate list if clients we must update its status as well 
			// because dsgcc_KeepAliveClient API returns client's tunnel status from DsgClientRegistrar client list.
			pBcmDsgClientRegistrarThread->updateDsgClientTunnelStatus(tunnelSet_it->second);
		}

		// If the tunnel as not available in the previous DCD and is now open then update status.
		if(tunnelSet_it->second->TunnelStatus() == BcmDsgClient::kDsgccTunnelNotAvailable)
		{
			if(tunnelSet_it->second->TunnelId() != 0)
			{
				tunnelSet_it->second->UpdateTunnelStatusSendNotification(BcmDsgClient::kDsgccTunnelOpen);

				// Since DsgClientRegistrar keeps a duplicate list if clients we must update its status as well 
				// because dsgcc_KeepAliveClient API returns client's tunnel status from DsgClientRegistrar client list.
				pBcmDsgClientRegistrarThread->updateDsgClientTunnelStatus(tunnelSet_it->second);
			}
		}
	}
}


void BcmDsgClientManager::ForwardBroadcastTunnelPktToClient( uint32 tunnelId, BcmOctetBuffer* pReassembledPacket, BcmEthernetPacket &tunnelPacket, bool applyClassifierRules  )
{
    CallTrace("BcmDsgClientManager", "ForwardBroadcastTunnelPktToClient");

	if( pReassembledPacket == NULL )
	{
		gInfoMsg(fMessageLogSettings, "ForwardBroadcastTunnelPktToClient") << "pReassembledPacket == NULL " <<endl;
		return;
	}
 
    if( !empty() )
    {
        SubscriberMmapIt rng_it = fSubscriberMmap.lower_bound( tunnelId );
        SubscriberMmapIt rng_end = fSubscriberMmap.upper_bound( tunnelId );

        while( rng_it != rng_end )
        {
            // found a subscriber. notify the subscriber.						
			if( ApplyRules(tunnelPacket, (*rng_it).second, applyClassifierRules) )
			{					
				if ((*rng_it).second->TunnelPacket(pReassembledPacket->AccessBuffer(), pReassembledPacket->BytesUsed()) < 0)
				{
					 // Client is unreachable, remove it.
					 if (pfDsgClientCtlThread)
					 {
						gInfoMsg(fMessageLogSettings, "ForwardBroadcastTunnelPktToClient") << "BcmDsgClientManager::ForwardToClient- Can't send packet to client." << endl;
						pfDsgClientCtlThread->DeregisterClient( (*rng_it).second );
					 }
				}
			}
			else
			{
				gInfoMsg(fMessageLogSettings, "ForwardBroadcastTunnelPktToClient") << "BT packet failed client classifier rules." << endl;
			}
			
            ++rng_it;
        }
    }
}
