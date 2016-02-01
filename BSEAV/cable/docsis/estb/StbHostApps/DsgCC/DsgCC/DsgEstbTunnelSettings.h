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
//  Filename:       DsgEstbTunnelSettings.h
//  Author:         Pinar Taskiran 
//  Creation Date:  Dec 27, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgEstbTunnelSettings encapsulates the tunnel address and 
//      the related classifier settings associated with a particular
//      DSG Tunnel.  
//
//****************************************************************************

#ifndef DSGESTBTUNNELSETTINGS_H
#define DSGESTBTUNNELSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include "DsgTunnelSettings.h"
#include "Type43VendorSettings.h"
#include "DsgClientId.h"
//********************** Global Types ****************************************

//typedef map< byte, BcmDsgClientId, less<byte> > ClientIdList;
typedef list<BcmDsgClientId> ClientIdList;
typedef ClientIdList::iterator ClientIdListIterator;
typedef ClientIdList::const_iterator ClientIdListConstIterator;

typedef list<uint8> UcidList;

// Pseudonym for the queue class type.
typedef list<uint16> ClassifierIdList;
typedef ClassifierIdList::iterator ClsIterator;
typedef ClassifierIdList::const_iterator ClsListConstIterator;
//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;
class BcmEthernetPacket;
class BcmDsgClient;

class BcmDsgEstbTunnelSettings : public BcmDsgTunnelSettings
{

public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmDsgEstbTunnelSettings();
    
    // Copy Constructor.  
    //
    BcmDsgEstbTunnelSettings( const BcmDsgEstbTunnelSettings& rhs );

    // Destructor.  
    //
    virtual ~BcmDsgEstbTunnelSettings();

    // Assignment operator.  
    //
    BcmDsgEstbTunnelSettings& operator=( const BcmDsgEstbTunnelSettings& rhs );

    // Less than operator.
    //
    // Parameters:
    //      rhs - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if this object is less than the comparison object.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  It is used for ordering lists.
    //
	bool operator<( const BcmDsgEstbTunnelSettings& rhs ) const;

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
	bool operator==( const BcmDsgEstbTunnelSettings& rhs ) const;    
    //
    // end functions required to be param for STL containers...

    //
    //
    // MostActiveFlowSets() - Assign the value of the most active
    //      BcmQosFlowSettings object owned by this object to the
    //      specified BcmQosFlowSettings object.
    //
    // Parameters:
    //      dest_flow_sets - reference to destination BcmQosFlowSettings 
    //          object.
    //
    // Returns:
    //      Returns true if active, admitted or provisioned flow sets exist.
    //      Otherwise returns false.
    //
    /*bool MostActiveFlowSets( BcmQosFlowSettings& dest_flow_sets ) const;*/


	byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset,
					 BcmMessageLogSettings* log_sets );
	const bool Defected() const { return fDefected; }
	void Defected( const bool new_val ) { fDefected = new_val; }

	// DCD object needs to know whether this tunnel has any classifier ID specified.
	const bool ClassifierIdSpecified(void) const;

	// DCD object needs to know whether this tunnel has any classifier ID specified.
	const bool VendorParamsSpecified(void) const;

	// Use this to filter the DSG Rules that specify a UCID range.
	// Apply our UCID value to the UCID Range of a DSG Rule.
	const bool TestUcid(uint8 ucid, bool ucidAvailable);

	BcmDsgClientId* FindClient(BcmDsgClient *pClient);

	const bool HasScte65BroadcastData(void);
	const bool HasBroadcastFlow() { return fHasBroadcastFlow;}

	bool WriteTo( BcmOctetBuffer &octetBuffer ) const;

	void Token(uint8 &token);
	uint8 &Token(void);

	const bool UcidCertified(void) const {return fUcidCertified;}

    // Reset() - Reset to initial conditions.
    //
    virtual void Reset();

	// IsEqualClassifier()
	const bool IsEqualClassifier(const BcmIpClassifierSettings& ipClassifierSets) const;

protected:

	byte ParseUcidList( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets );

	bool ParseClientId( const BcmOctetBuffer &obuf, unsigned int &offset,
	    BcmMessageLogSettings* log_sets );

	const bool MustHaveSetsSpecified(void) const;

    // DerivedPrint - This function is implemented by classes derived
    //      from BcmMacManagementMessage to print formatted derived 
    //      object contents to the specified ostream.  It is called 
    //      by BcmMacManagementMessage::Print() (a.k.a. the base class
    //      Print() function).
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
    virtual ostream& DerivedPrint( ostream& ostrm ) const;

public:    
    
	uint8 fRuleId;
	uint8 fRulePriority;

	//uint16 fUcidRange;
	// Consists of 8-bit UCID values.
	UcidList fUcidList;

	// Consists of ClientId objects
	ClientIdList fClientIdList;

	// List to store client IDs.
	ClassifierIdList fClassifierIdList;
	
	bool fDefected;

	bool fRuleIdSpecified;
	bool fRulePrioritySpecified;
	bool fUcidRangeSpecified;
	bool fClientIdSetsSpecified;
	bool fTunnelAddressSpecified;
	bool fClassifierIdSpecified;
	bool fVendorParamsSpecified;

	bool fUcidCertified;

	// Vendor Specific Parameters
	BcmType43VendorSettings fVendorSettings;

    // Client identifier.
    uint8 fToken;

	// These two variables cannot be true at the same time.
	bool fClientRequestsSubTunnelForwarding;
	bool fIsClassifiedPrivateTunnel;
	bool fHasBroadcastFlow;
};
//********************** Inline Method Implementations ***********************

#endif

