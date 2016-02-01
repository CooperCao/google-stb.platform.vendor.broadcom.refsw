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
//  Filename:       DsgTunnelSettings.cpp
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

#include "DsgTunnelSettings.h"
#include "TLV.h"
#include "TLVCodes.h"

#include <map>
#include "MessageLog.h"

//********************** Local Constants *************************************

// Set this to 0 to remove classifier debug stuff.  This will reduce code size
// and speed up performance.
#define DEBUG_CLASSIFIERS 0

//********************** Local Variables *************************************

static unsigned int gInstanceNumber = 0;

//********************** Class Method Implementations ************************


BcmDsgTunnelSettings::BcmDsgTunnelSettings() :
    fTunnelAddress(),
	fTunnelId(0),
    fClassifierSetsList(),
    fMessageLogSettings("BcmDsgTunnelSettings")
{
    // Set up my message log settings.
    char instanceName[64];

    sprintf(instanceName, "DsgTunnelSettings: %d", gInstanceNumber);
    gInstanceNumber++;
    
    fName = instanceName;

    fMessageLogSettings.SetInstanceName(fName.c_str());
    fMessageLogSettings.SetModuleName(fName.c_str());

    //fMessageLogSettings.Register();
}

BcmDsgTunnelSettings::~BcmDsgTunnelSettings()
{
    // Clean up my message log settings.
    //fMessageLogSettings.Deregister();
}

BcmDsgTunnelSettings::BcmDsgTunnelSettings( const BcmDsgTunnelSettings& rhs )
  :
    fTunnelAddress(),
    fClassifierSetsList()
{
    // For STL containers, the Diab compiler doesn't support copy construction
    // in initializer lists.  Thus, we use the default constructor in the list
    // above, then use copy semantics here.
    fTunnelAddress = rhs.fTunnelAddress;
	fTunnelId = rhs.fTunnelId;
    fClassifierSetsList = rhs.fClassifierSetsList;

	fMessageLogSettings = rhs.fMessageLogSettings;
}

// Assignment operator.  
//
BcmDsgTunnelSettings& BcmDsgTunnelSettings::operator=( const BcmDsgTunnelSettings& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fTunnelAddress = rhs.fTunnelAddress;
	fTunnelId = rhs.fTunnelId;
    fClassifierSetsList = rhs.fClassifierSetsList;

	fMessageLogSettings = rhs.fMessageLogSettings;

    return *this;
}

ostream& BcmDsgTunnelSettings::PrintDetailed( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    // consider flow settings
    ostrm 
        << "\nTunnel Settings:"
		<< " " << (fTunnelId==0?"CLOSED":"OPEN")
        << "\n Tunnel Address = " << fTunnelAddress << ", ID = " << fTunnelId;
        

#endif

    return DerivedPrint( ostrm );
}



ostream& BcmDsgTunnelSettings::Print( ostream& ostrm ) const
{
    return PrintDetailed( ostrm );
}


ostream& BcmDsgTunnelSettings::PrintSummary( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << "\n========================="
        << "\nBcmDsgTunnelSettings:\n"
        << "\nfTunnelAddress = " << fTunnelAddress << ", ID = " << fTunnelId << endl;

    // consider classifier settings
    if( !fClassifierSetsList.empty() )
    {
        ostrm 
            << "\nclassifiers:\n";

        ClassifierSettingsListConstIterator cls_it = fClassifierSetsList.begin();
        ClassifierSettingsListConstIterator cls_end = fClassifierSetsList.end();
        while( cls_it != cls_end )
        {
            // found classifier settings.
            ostrm 
                << "Classifier id=" 
                << (int) ((cls_it->second).CmtsClassifierId())
                << endl;

            // increment iterator.
            ++cls_it;
        }   
    }

#endif

    return ostrm;
}

ostream& BcmDsgTunnelSettings::DerivedPrint( ostream& ostrm ) const
{
    // default implementation simply prints a newline and flushes the
    // output stream.
    ostrm << endl;
    return ostrm;
}

//
BcmMacAddress BcmDsgTunnelSettings::TunnelAddress() const
{
	return fTunnelAddress;
}

void BcmDsgTunnelSettings::TunnelAddress( BcmMacAddress &tunnelAddress )
{
	fTunnelAddress = tunnelAddress;
}

//
uint32 BcmDsgTunnelSettings::TunnelId() const
{
	return fTunnelId;
}
void BcmDsgTunnelSettings::TunnelId( uint32 &tunnelId )
{
	fTunnelId = tunnelId;
}


bool BcmDsgTunnelSettings::add( const BcmDsgClassifierSettings& classifierSetting )
{
	uint16 newClassifierId = classifierSetting.CmtsClassifierId();

	ClassifierSettingsListConstIterator cls_it = begin();
	
    // This produces "statement with no effect" warning.  Slight
    // rework:
    // for( cls_it; cls_it!= end(); cls_it++)
    while (cls_it != end())
	{
		if(cls_it->second.CmtsClassifierId() == newClassifierId ) 
		{
			cout
				<<"A Classifier with this Id"
				 << "[" << newClassifierId << "]" << "already exists in the list."<< endl;
			return false;
		}
        
        cls_it++;
    }

	// else matching element does not exist.  insert classifierSetting into the container.     
	fClassifierSetsList.insert( ClassifierSettingsList::value_type((byte)newClassifierId, classifierSetting));

	return true;
}

void BcmDsgTunnelSettings::Reset()
{
	fTunnelAddress = kAllZerosMac;

    if( !fClassifierSetsList.empty() )
    {
        fClassifierSetsList.clear();
    }
}










