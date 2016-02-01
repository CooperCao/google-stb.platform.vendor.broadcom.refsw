//****************************************************************************
//
// Copyright (c) 2005-2008 Broadcom Corporation
//
// This program is the proprietary software of Broadcom Corporation and/or
// its licensors, and may only be used, duplicated, modified or distributed
// pursuant to the terms and conditions of a separate, written license
// agreement executed between you and Broadcom (an "Authorized License").
// Except as set forth in an Authorized Licens, Broadcom grants no license
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
//  Filename:       DsgTunnelSettingsList.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgEstbTunnelSettingsList 
//
//****************************************************************************

#include "DsgEstbTunnelSettingsList.h"
#include "DsgEstbTunnelSettings.h"
//#include "map.h"
#include "MessageLog.h"

BcmDsgEstbTunnelSettingsList::BcmDsgEstbTunnelSettingsList()
  :
    fTunnelSetsList()
{
}


BcmDsgEstbTunnelSettingsList::BcmDsgEstbTunnelSettingsList( const BcmDsgEstbTunnelSettingsList& rhs )
  :
    fTunnelSetsList()
{
    // For STL containers, the Diab compiler doesn't support copy construction
    // in initializer lists.  Thus, we use the default constructor in the list
    // above, then use copy semantics here.
    fTunnelSetsList = rhs.fTunnelSetsList;
}


BcmDsgEstbTunnelSettingsList::~BcmDsgEstbTunnelSettingsList()
{
}


BcmDsgEstbTunnelSettingsList& BcmDsgEstbTunnelSettingsList::operator=( const BcmDsgEstbTunnelSettingsList& rhs )
{

    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fTunnelSetsList = rhs.fTunnelSetsList;
    return *this;
}

ostream& BcmDsgEstbTunnelSettingsList::PrintDetailed( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    // consider flow settings
    ostrm 
        << "\nBcmDsgEstbTunnelSettingsList:"
        << endl;

    // consider classifier settings
    if( !empty() )
    {
        TunnelSettingsListConstIterator tunnelSet_it = begin();
        TunnelSettingsListConstIterator tunnelSet_end = end();
        while( tunnelSet_it != tunnelSet_end )
        {
            // found classifier settings.
            ostrm << (tunnelSet_it->second) << endl;

            // increment iterator.
            ++tunnelSet_it;
        }   
    }
#endif

    return ostrm;
}



ostream& BcmDsgEstbTunnelSettingsList::Print( ostream& ostrm ) const
{
    return PrintDetailed( ostrm );
}

/*
ostream& BcmDsgEstbTunnelSettingsList::PrintSummary( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm 
        << "\n========================="
        << "\nBcmDsgTunnelSettings:\n"
        << "\nfTunnelAddress = " << fTunnelAddress << endl;

    // consider classifier settings
    if( !fClassifierSetsList.empty() )
    {
        ostrm 
            << "\nclassifiers:\n";

        ClassifierSetsListConstIterator cls_it = fClassifierSetsList.begin();
        ClassifierSetsListConstIterator cls_end = fClassifierSetsList.end();
        while( cls_it != cls_end )
        {
            // found classifier settings.
            ostrm 
                << "Classifier id=" 
                    << (void *) ((*cls_it).CmtsClassifierId())
                << endl;

            // increment iterator.
            ++cls_it;
        }   
    }

#endif

    return ostrm;
}
*/


bool BcmDsgEstbTunnelSettingsList::operator==(
  const BcmDsgEstbTunnelSettingsList& rhs ) const
{
    if( fTunnelSetsList != rhs.fTunnelSetsList )
    {
        return false;
    }

    return true;
}

void BcmDsgEstbTunnelSettingsList::Reset()
{
    if( !fTunnelSetsList.empty() )
    {
        fTunnelSetsList.clear();
    }
}

bool BcmDsgEstbTunnelSettingsList::add( const BcmDsgEstbTunnelSettings& dsgTunnelSettings )
{
#if (CABLECARD_SUPPORT)
    // Always add to the list so that ResetEcmTunnels() will remove all tunnels
    cout <<"Add tunnel " << dsgTunnelSettings.TunnelId() << " to TunnelList." << endl;
#else
	if( find(dsgTunnelSettings) )
	{
		cout <<"DUPLICATE TUNNEL. Not valid." << endl;
		return false;
	}
#endif

	// else matching element does not exist.  insert dsgTunnelSettings into the container.     
	fTunnelSetsList.insert( TunnelSettingsList::value_type(dsgTunnelSettings.TunnelAddress(), dsgTunnelSettings));
    
	return true;
}

TunnelSettingsListIterator BcmDsgEstbTunnelSettingsList::find(uint32 tunnelId)
{
  TunnelSettingsListIterator tunnelSet_it = begin();

  for( tunnelSet_it; tunnelSet_it!= end(); tunnelSet_it++)
  {
      if(tunnelSet_it->second.TunnelId() == tunnelId ) 
          break;
  }

  return tunnelSet_it;
}

BcmMacAddress BcmDsgEstbTunnelSettingsList::remove( const uint32& tunnelId )
{
	BcmMacAddress removedTunnelAddress(kAllZerosMac);
    TunnelSettingsListIterator tunnelSet_it = find (tunnelId);
    
    if (tunnelSet_it != end())
    {
        cout <<"Removing Tunnel"
		     << tunnelSet_it->second <<" ID = " << tunnelId << endl;

        // store the address to return
        removedTunnelAddress = tunnelSet_it->second.TunnelAddress();
#if (SNMP)
        tunnelSet_it->second.FlushTunnelFilterTable();
#endif
        // remove the tunnel.
        fTunnelSetsList.erase(tunnelSet_it);
    }

	// else matching element does not exist.  
	return removedTunnelAddress;
}

// This method checks whether this tunnel is subset of another tunnel.
// We will do a simple check. If one tunnel specifies classifiers and the 
// other does not, then we will ignore the one that has classifiers. The reason
// is that if we allow both these tunnels to be opened than eCM filters may not 
// work properly depending on which rule is checked first.
bool BcmDsgEstbTunnelSettingsList::IsSubSetOrDefected(const TunnelSettingsListIterator& tunnelIt)
{
	if(tunnelIt->second.Defected() == true )
	{
		return true;
	}

	// Find all the tunnels with the same tunnel address as tunnelIt's.
	BcmMacAddress tunnelAddress = tunnelIt->second.TunnelAddress();
    TunnelSettingsListIterator rng_it = fTunnelSetsList.lower_bound( tunnelAddress );
    TunnelSettingsListIterator rng_end = fTunnelSetsList.upper_bound( tunnelAddress );

	while( rng_it != rng_end )
	{
		// Make sure we are not comparing tunnelIt with itself.
		// Note that we are comparing pointer value here not the object itself.
		if((rng_it != tunnelIt) && (!rng_it->second.Defected()))
		{
			// Now check if the other tunnel specifies classifiers and I am not
			if( !tunnelIt->second.fClassifierIdList.empty() && rng_it->second.fClassifierIdList.empty() )	
			{
				 // tunnelIt is a subset of the other tunnel. tunnelIt is defected.
				tunnelIt->second.Defected(true);
				return true;
			}
			else if(tunnelIt->second.empty() && !rng_it->second.empty() )
			{
				// The other tunnel is a subset of tunnelIt. The other tunnel is defected.
				rng_it->second.Defected(true);
			}
		}

		rng_it++;
	}

	return false;
}

bool BcmDsgEstbTunnelSettingsList::find(const BcmDsgEstbTunnelSettings& dsgTunnelSettings )
{
	BcmMacAddress newTunnelAddress = dsgTunnelSettings.TunnelAddress();
    TunnelSettingsListConstIterator rng_it = fTunnelSetsList.lower_bound( newTunnelAddress );
    TunnelSettingsListConstIterator rng_end = fTunnelSetsList.upper_bound( newTunnelAddress );

	while( rng_it != rng_end )
	{
        // found 
        if( (*rng_it).second == dsgTunnelSettings )
        {							
            return true;
        }
        ++rng_it;
    }

	return false;
}


