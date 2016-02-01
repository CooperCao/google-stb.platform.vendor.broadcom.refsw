//****************************************************************************
//
// Copyright (c) 2005-2010 Broadcom Corporation
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
//  Filename:       DsgEstbTunnelSettingsList.h
//  Author:         Pinar Taskiran 
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgEstbTunnelSettingsList 
//
//****************************************************************************

#ifndef DSGESTBTUNNELSETTINGSLIST_H
#define DSGESTBTUNNELSETTINGSLIST_H

//********************** Include Files ***************************************

#include "MacAddress.h"
#include "DsgEstbTunnelSettings.h"	// KB:  Added for new version of toolchain - see comment below also.

#if defined(WIN32)
#include <map>
#else
#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <map>
#else
#include <multimap.h>
#endif
#endif

// KB:  New version of toolchain didn't like this forward declaration.
//      Added the appropriate include file instead.
// class BcmDsgEstbTunnelSettings;

//********************** Global Types ****************************************
typedef multimap< BcmMacAddress, BcmDsgEstbTunnelSettings, less<BcmMacAddress> > TunnelSettingsList;
typedef TunnelSettingsList::iterator TunnelSettingsListIterator;
typedef TunnelSettingsList::const_iterator TunnelSettingsListConstIterator;

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************

class BcmMessageLogSettings;

class BcmDsgEstbTunnelSettingsList 
{
public:
    // begin functions required to be param for STL containers...
    //
    
    // Constructor. 
    //
    BcmDsgEstbTunnelSettingsList();
    
    // Copy Constructor.  
    //
    BcmDsgEstbTunnelSettingsList( const BcmDsgEstbTunnelSettingsList& rhs );

    // Destructor.  
    //
    virtual ~BcmDsgEstbTunnelSettingsList();

    // Assignment operator.  
    //
    BcmDsgEstbTunnelSettingsList& operator=( const BcmDsgEstbTunnelSettingsList& rhs );

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
	bool operator<( const BcmDsgEstbTunnelSettingsList& rhs ) const;

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
	bool operator==( const BcmDsgEstbTunnelSettingsList& rhs ) const;    
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
    int NumTunnelSets() const { return fTunnelSetsList.size(); }

    //
    // accessors
    //
    //
    // STL multimap member function wrappers...
    //
    // size() - return number of elements in the container.
    //
    unsigned int size() const { return fTunnelSetsList.size(); }
    //
    //
    // begin() - return iterator/const iterator pointing to first element 
    //      in container, according to the sort order.
    //
    TunnelSettingsListIterator begin() { return fTunnelSetsList.begin(); }
    TunnelSettingsListConstIterator begin() const { return fTunnelSetsList.begin(); }
    //
    //
    // end() - return iterator/const iterator pointing to last element 
    //      in container, according to the sort order.
    //
    TunnelSettingsListIterator end() { return fTunnelSetsList.end(); }
    TunnelSettingsListConstIterator end() const { return fTunnelSetsList.end(); }
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
    bool add( const BcmDsgEstbTunnelSettings& dsgTunnelSettings );

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
	BcmMacAddress remove( const uint32& tunnelId );
    //
    //
    // empty() - return true if zero elements are in the container.
    //
    bool empty() const { return fTunnelSetsList.empty(); }
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

	inline TunnelSettingsListIterator find(const BcmMacAddress &tunnelAddress);
    
    // Same as above except finds based on ID instead of MAC address.
    // Less efficient because we must iterate over the map instead of hashing.
    TunnelSettingsListIterator find(uint32 tunnelId);
	bool find(const BcmDsgEstbTunnelSettings& dsgTunnelSettings );

	bool IsSubSetOrDefected(const TunnelSettingsListIterator& tunnelIt);

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
    TunnelSettingsList fTunnelSetsList;
    //
};

inline ostream& operator<<( ostream& ostrm, const BcmDsgEstbTunnelSettingsList& rhs )
{
    return rhs.Print( ostrm );
}

// Use this for DSG BASIC mode.
inline TunnelSettingsListIterator BcmDsgEstbTunnelSettingsList::find(const BcmMacAddress &tunnelAddress)
{
	TunnelSettingsListIterator tunnelSet_it = fTunnelSetsList.find(tunnelAddress);

	return tunnelSet_it;
}
#endif


