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
//  Filename:       DsgTunnelSettings.h
//  Author:         Pinar Taskiran 
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgTunnelSettings encapsulates the tunnel address and 
//      the related classifier settings associated with a particular
//      DSG Tunnel.  
//
//****************************************************************************

#ifndef DSGTUNNELSETTINGS_H
#define DSGTUNNELSETTINGS_H

//********************** Include Files ***************************************

// My base class.
#include <iostream>
#include "DsgClassifierSettings.h"
#include "MacAddress.h"
#include <map>
#include "MessageLog.h"

#include "BcmString.h"

//********************** Global Types ****************************************
typedef map< byte, BcmDsgClassifierSettings, less<byte> > ClassifierSettingsList;
typedef ClassifierSettingsList::iterator ClassifierSettingsListIterator;
typedef ClassifierSettingsList::const_iterator ClassifierSettingsListConstIterator;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************
class BcmMessageLogSettings;

class BcmDsgTunnelSettings 
{

public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmDsgTunnelSettings();
    
    // Copy Constructor.  
    //
    BcmDsgTunnelSettings( const BcmDsgTunnelSettings& rhs );

    // Destructor.  
    //
    virtual ~BcmDsgTunnelSettings();

    // Assignment operator.  
    //
    BcmDsgTunnelSettings& operator=( const BcmDsgTunnelSettings& rhs );

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
	bool operator<( const BcmDsgTunnelSettings& rhs ) const;

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
	bool operator==( const BcmDsgTunnelSettings& rhs ) const;    
    //
    // end functions required to be param for STL containers...

    // Reset() - Reset to initial conditions.
    //
    virtual void Reset();

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
	virtual ostream& PrintDetailed( ostream& ostrm ) const;

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

    //
    // same concept for classifier settings.
    int NumClassifierSets() const { return fClassifierSetsList.size(); }

    //
    BcmMacAddress TunnelAddress() const;
    void TunnelAddress( BcmMacAddress &tunnelAddress );

    //
    uint32 TunnelId() const;
    void TunnelId( uint32 &tunnelId );

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

    //
    // accessors
    //
    //
    // STL multimap member function wrappers...
    //
    // size() - return number of elements in the container.
    //
    unsigned int size() const { return fClassifierSetsList.size(); }
    //
    //
    // begin() - return iterator/const iterator pointing to first element 
    //      in container, according to the sort order.
    //
    ClassifierSettingsListIterator begin() { return fClassifierSetsList.begin(); }
    ClassifierSettingsListConstIterator begin() const { return fClassifierSetsList.begin(); }
    //
    //
    // end() - return iterator/const iterator pointing to last element 
    //      in container, according to the sort order.
    //
    ClassifierSettingsListIterator end() { return fClassifierSetsList.end(); }
    ClassifierSettingsListConstIterator end() const { return fClassifierSetsList.end(); }
    //
    //
    // add() - Add the specified BcmDsgClassifierSettings object into
    //      the container.  
    //
    // Parameters:
    //      classifiefSetting - BcmDsgClassifierSettings object which is to be added/replaced
    //          to the set.
    //
    // Returns:
    //      iterator which points to the new set element.
    //
    bool add( const BcmDsgClassifierSettings& classifiefSetting );
    //
    //
    // empty() - return true if zero elements are in the container.
    //
    bool empty() const { return fClassifierSetsList.empty(); }
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

protected:

// data members are public for now because it's a hassle to manipulate
// the various <xxxx>SetsList data members via accessor functions.
public:    

	BcmMacAddress fTunnelAddress;

	// eSTB will use this id to close this tunnel.
	uint32 fTunnelId;

    // List of DSG Tunnel Classifier settings objects. 
    ClassifierSettingsList fClassifierSetsList;

    // For logging information.
    BcmMessageLogSettings fMessageLogSettings;
    
    //
    BcmString fName;
};
//********************** Inline Method Implementations ***********************

inline ostream& operator<<( ostream& ostrm, const BcmDsgTunnelSettings& rhs )
{
    return rhs.Print( ostrm );
}

#endif
