//****************************************************************************
//
//  Copyright (c) 2004  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16251 Laguna Canyon Road
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//
//  Filename:       DsgClient.h
//  Author:         Maurice Turcotte, Pinar Taskiran
//  Creation Date:  Dec 3, 2004
//
//****************************************************************************
//  Description:
//      This is the Client class.
//
//****************************************************************************

#ifndef DSGCLIENT_H
#define DSGCLIENT_H

//********************** Include Files ***************************************
#include "typedefs.h"

#include "OctetBuffer.h"
#include "MacAddress.h"
#include "BcmString.h"
#include "EnetPkt.h"

#include "BcmSocket.h"
#include "ClassifierSettings.h"

//#ifdef DSG_RPC_INCLUDED
//#include "dsgccClientRegistration_rpc.h"
//#else
#include "dsgcc_client_api.h"
//#endif
//********************** Global Types ****************************************


//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmEthernetPacket;

class BcmDsgClient
{
public:

    // Default Constructor.  Initializes the state of the object...
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDsgClient(void);

    // Registration Constructor.  Initializes the state of this object
    // to the registration information passed in.
    //
    // Parameters:
    //      struct dsgClientRegInfo - the registration info.
    //
    // Returns:  N/A
    //
    //BcmDsgClient(const struct dsgClientRegInfo *regInfo);

    // Registration Constructor.  Initializes the state of this object
    // to the registration information passed in.
    //
    // Parameters:
    //      struct dsgClientRegInfo - the registration info.
    //
    // Returns:  N/A
    //
    BcmDsgClient(const struct dsgClientRegInfo *regInfo, dsg_classifier* dsgClassifier = NULL);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDsgClient();

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDsgClient(const BcmDsgClient &otherInstance);

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDsgClient & operator = (const BcmDsgClient &otherInstance);

    // Equality operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator==( const BcmDsgClient& rhs ) const;  

    // misc accessors.
    uint32 Id() const { return fClientId; }
    void Id( const uint32 new_val ) { fClientId = new_val; }

    // misc accessors.
    bool Active() const { return fActive; }
	void Active( const bool new_val ) { fActive = new_val; }

    uint32 ClientProgNum() const { return fClientProgNum; }
    void ClientProgNum( const uint32 new_val ) { fClientProgNum = new_val; }

    uint32 ClientPort() const { return fClientPort; }
    void ClientPort( const uint32 new_val ) { fClientPort = new_val; }

    // misc accessors.
    uint32 IdType() const { return fIdType; }
    void IdType( const uint32 new_val ) { fIdType = new_val; }

    BcmMacAddress ClientAddress() const { return fClientAddr; }
	void ClientAddress(BcmMacAddress &id);
    const BcmString &ClientName() const { return fClientName; }    
	void ClientName(const BcmOctetBuffer &clientName);

    uint32 TunnelId() const { return fTunnelId; }
    void TunnelId( const uint32 new_val );

	uint32 TunnelStatus() const { return fTunnelStatus; }
	void TunnelStatus( const uint32 new_val );

	void Token(uint8 &token);
	uint8 &Token(void);

	bool NameMapped(void) const;

	ostream& Print( ostream& ostrm ) const;
	const char* IdTypeName( const uint32 value ) const;

	void CloseSocket(void);

    // These methods are available to notify the client object that
    // some status has changed. The client object will 
    // notify the actual client of the change.
	int SendNotification(int event, int value = 0);
	int UpdateTunnelStatusSendNotification(int status);

    virtual int twoWayModeChangeNotification(int mode);
    virtual int advancedModeChangeNotification(int mode);
    virtual int tunnelPresenceChangeNotification(int presence);
	virtual int TunnelPacket(const unsigned char *pkt, uint16 pktLen);
	int ProcessTunnelPacket(BcmEthernetPacket &ethernetPacket);
    virtual int DsgDsScanCompleteNotification(void);
    virtual int DsgUcid2wayOKNotification(uint8 ucid);
    virtual int DsgDccDepartNotification(uint8 init_type);
	virtual int DsgTwoWayForwardingDisabledNotification(uint8 twoWayForwardingLimitations);

	enum
	{
		// Tunnel status codes
		kDsgccTunnelRequestPending	= DSGCC_TUNNEL_REQUEST_PENDING,
		kDsgccTunnelOpen			= DSGCC_TUNNEL_OPEN,
		kDsgccTunnelNotAvailable	= DSGCC_TUNNEL_NOT_AVAILABLE,
		kDsgccTunnelPending			= DSGCC_TUNNEL_PENDING,

		// DSG status codes
		kTwoWayModeChange			= DSGCC_TWOWAYMODE_CHANGE,
		kAdvancedModeChange			= DSGCC_ADVANCEMODE_CHANGE
	};

	const char* TunnelStatusTypeName( uint32 value ) const;

private:

	// Client ID
	uint32 fClientId;
	uint32 fIdType;
	BcmMacAddress fClientAddr;
	BcmString fClientName;

	BcmSocket *pfSocket;
	#if defined(BCM_DSGCC_CLIENTS_USE_SHARED_MEMORY_IPC)
	    uint32 fSharedMemoryId;
	#endif

	bool fActive;
	uint32 fClientProgNum;
	uint32 fClientPort;

	// Id of the tunnel the client is subscribed to.
	// Value is zero if unsubscribed.
	uint32 fTunnelId;

	// Client identifier.
	uint8 fToken;

	// the forwarded packet count
	uint32 fPacketCount;
	uint32 fNumberOfOctets;

	uint32 fTunnelStatus;

public:
    // IP classifier settings. 
    BcmIpClassifierSettings fIpClassifierSets;
    bool fIpClassifierSetsSpecified;
	bool fRemoveUdpHeader;
};


//********************** Inline Method Implementations ***********************


inline ostream& operator<<( ostream& ostrm, const BcmDsgClient& rhs )
{
    return rhs.Print( ostrm );
}

#endif


