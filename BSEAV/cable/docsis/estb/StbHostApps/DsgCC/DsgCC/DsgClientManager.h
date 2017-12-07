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
//****************************************************************************
//
//  Filename:       DsgClientManager.h
//  Author:         Pinar Taskiran 
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgClientManager 
//
//****************************************************************************

#ifndef DSGCLIENTMANAGER_H
#define DSGCLIENTMANAGER_H

//********************** Include Files ***************************************

// 
#include "MacAddress.h"

#define USE_MMAP 1

#if defined(WIN32)
#include <map>
#else
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 4) || (__GNUC__ > 4))
#include <map>
#else
#include <multimap.h>
#endif
#endif

#include "DsgClient.h"
#include "Type43VendorSettings.h"

//********************** Global Types ****************************************

typedef multimap< uint32, BcmDsgClient *, less<uint32> > SubscriberMmap;

typedef SubscriberMmap::iterator SubscriberMmapIt;
typedef SubscriberMmap::const_iterator SubscriberMmapConstIt;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmEthernetPacket;
class BcmDsgClientId;
class BcmDsgClientCtlThread;
class BcmDsgEstbTunnelSettings;
class BcmMessageLogSettings;
class BcmDSGAppNameTable;

class BcmDsgClientManager 
{
public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmDsgClientManager();
    
    // Copy Constructor.  
    //
    BcmDsgClientManager( const BcmDsgClientManager& rhs );

    // Destructor.  
    //
    virtual ~BcmDsgClientManager();


    //
    // end functions required to be param for STL containers...

    // Reset() - Reset to initial conditions.
    //
    void Reset();

    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& Print( ostream& ostrm ) const;

    // PrintSummary() - Print summary of formatted object contents 
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& PrintSummary( ostream& ostrm ) const;

    // PrintDetailed() - Print detailed formatted object contents 
    //      to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
	ostream& PrintDetailed( ostream& ostrm ) const;

    //
    // same concept for classifier settings.
    int NumTunnelSets() const { return fSubscriberMmap.size(); }

    //
    // accessors
    //
    //
    // STL multimap member function wrappers...
    //
    // size() - return number of elements in the container.
    //
    unsigned int size() const { return fSubscriberMmap.size(); }
    //
    //
    // begin() - return iterator/const iterator pointing to first element 
    //      in container, according to the sort order.
    //
    SubscriberMmapIt begin() { return fSubscriberMmap.begin(); }
    SubscriberMmapConstIt begin() const { return fSubscriberMmap.begin(); }
    //
    //
    // end() - return iterator/const iterator pointing to last element 
    //      in container, according to the sort order.
    //
    SubscriberMmapIt end() { return fSubscriberMmap.end(); }
    SubscriberMmapConstIt end() const { return fSubscriberMmap.end(); }
    //
    //
    // add() - Add the specified BcmDsgEstbTunnelSettings object into
    //      the container.  
    //
    // Parameters:
    //      classifiefSetting - BcmDsgEstbTunnelSettings object which is to be added/replaced
    //          to the set.
    //
    // Returns:
    //      true if successful
    //
    bool add( const BcmDsgClient& client );

    // remove() - Remove the specified BcmDsgEstbTunnelSettings object from
    //      the container.  
    //
    // Parameters:
    //      tunnelId - unique id of the BcmDsgEstbTunnelSettings object which is to be removed
    //          
    //
    // Returns:
    //      tunnel address of the removed tunnel
    //
	bool remove( const uint32& port );
    //
    //
    // empty() - return true if zero elements are in the container.
    //
    bool empty() const { return fSubscriberMmap.empty(); }
    //
    //
    // clear() - erase all elements in the container.
    //
    void clear() { Reset(); }
    //
    //
    // erase() - erase element with the specified iuc key value.
    //
    void erase( uint8 iuc );

	// This method searches the client list by a unique port number
	// to find whether any client with an open tunnel has this specific 
	// port number.
    bool IsClientAssignedTunnel(uint32 &port);

	// This method searches the client list by a unique port number
	// and returns a pointer to the client.
	BcmDsgClient *FindByPort(uint32 &port);
    
    // Same as above except finds based on ID instead of MAC address.
    // Less efficient because we must iterate over the map instead of hashing.
    SubscriberMmapIt find(uint32 tunnelId);

	bool Subscribe( uint32 tunnelId, BcmDsgClient *pClient );
	bool UnSubscribe( uint32 tunnelId, BcmDsgClient *pClient );
	bool FindAndSetClients(  BcmDsgEstbTunnelSettings *pTunnel, BcmDsgClientId* pClientId, uint8 token, bool &duplicateClient, bool &openTunnel, bool &openSecretTunnel );
	bool SetOrSwitchClients( BcmDsgClientId* pClientId, uint8 token ); 
	bool OpenTunnelsForMacAddressTypeClients( void );
	bool MustFindSrcSubTable(void);
	bool UnsubscribeAll(void);
	bool SubscribeClients( BcmDsgEstbTunnelSettings *pTunnel );
	void Set( BcmDsgClientCtlThread *pDsgClientCtlThread );
	void ForwardToClient( uint32 tunnelId, uint8* pBuffer, uint16 length, BcmEthernetPacket &tunnelPacket, bool applyClassifierRules  );
	void ForwardBroadcastTunnelPktToClient( uint32 tunnelId, BcmOctetBuffer* pReassembledPacket, BcmEthernetPacket &tunnelPacket, bool applyClassifierRules  );
	bool ForwardVspData( BcmType43VendorSettings &vendorSettings );
	void SubscribeClientsWithClassifiers( BcmDsgEstbTunnelSettings *pTunnel );
	bool AllCaTunnelRequestsSatisfied();
	bool CaTunnelRequestPresent();

	// Performing action on the Client List.
	// This method is called when a new SNS Table is received.
	bool AssignClientIds( BcmDSGAppNameTable *pAppNameTable ); 

	// This method returns number of clients per tunnel.
	uint32 NumberOfClients(uint32 tunnelId);

    // These methods send notifications of various state changes to all clients
    void NotifyClientsOfAdvancedMode(int mode);
    void NotifyClientsOfTwoWayMode(int mode);
    void NotifyClientsOfTunnelsPresent(void);
    void NotifyClientsOfDsScanComplete(void);
    void NotifyClientsOfUcid2wayOK(uint8 ucid);
    void NotifyClientsOfDccDepart(uint8 init_type);
	void NotifyClientsOfTwoWayForwardingDisabled( uint8 twoWayForwardingLimitations );
	void UpdateStatusNotifyClients(void);

    //
	// Applies the specific classification rules and parameters to the packet
	// specified, returning true if the packet is "classified" as belonging to
	// the service flow, or false if the packet doesn't belong here.
	//
	// Parameters:
	//      ethernetPacket - the packet to be classified.
	//		pClient - client settings pointer
	//		applyClassifierRules - 
	// Returns:
	//      true if the packet matches all of the classifier rules.
	//      false if the packet does not match all of the classifier rules.
	//
	bool ApplyRules(BcmEthernetPacket &ethernetPacket, BcmDsgClient *pClient, bool applyClassifierRules );

protected:
    //
	// Applies the specific classification rules and parameters to the packet
	// specified, returning true if the packet is "classified" as belonging to
	// the service flow, or false if the packet doesn't belong here.
	//
	// Parameters:
	//      ethernetPacket - the packet to be classified.
	//		pClient - client settings pointer
	// Returns:
	//      true if the packet matches all of the classifier rules.
	//      false if the packet does not match all of the classifier rules.
	//
	bool ApplyClassifierRules(BcmEthernetPacket &ethernetPacket, BcmDsgClient *pClient ); 

protected:
    //
    // DeleteClassifierSets - Delete the classifier settings 
    //      of this object's ClassifierSettingsList data member object 
    //      which match the specified values.
    //
    // Parameters:
    //      mod_cls_sets - reference to BcmDsgClassifierSettings object which 
    //          contains id values of clasifier to be deleted.
    //
    //      log_sets - BcmMessageLogSettings used to control debug output.
    //
    // Returns:
    //      byte - "Confirmation code" result, as described in DOCSIS 
    //          RFI v1.1-IO5 spec App C.4.  A result == 0 is success.  
    //          Non-zero result is an error.
    //
    byte DeleteClassifierSets( const BcmDsgEstbTunnelSettings& mod_cls_sets, 
        BcmMessageLogSettings* log_sets );

// data members are public for now because it's a hassle to manipulate
// the various <xxxx>SetsList data members via accessor functions.
public:    

    // List of DSG Tunnel Settings objects. 
    SubscriberMmap fSubscriberMmap;
    //

	BcmDsgClientCtlThread *pfDsgClientCtlThread;

    // For controlling message log output.
    BcmMessageLogSettings fMessageLogSettings;

	uint32 fMacAddrClientCount;
	uint32 fCaClientCount;

private:

    // Assignment operator.  NOT IMPLEMENTED.
    BcmDsgClientManager& operator=( const BcmDsgClientManager& rhs );

    // Less than operator. NOT IMPLEMENTED.
	bool operator<( const BcmDsgClientManager& rhs ) const;

    // Equality operator. NOT IMPLEMENTED.
	bool operator==( const BcmDsgClientManager& rhs ) const;    
};

inline ostream& operator<<( ostream& ostrm, const BcmDsgClientManager& rhs )
{
    return rhs.Print( ostrm );
}

#endif



