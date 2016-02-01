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
//  Filename:       DsgEstbTunnelSettings.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Sep 01, 2004
//
//****************************************************************************
//  Description:
//      Class BcmDsgEstbTunnelSettings encapsulates the tunnel address and 
//      the related classifier settings associated with a particular
//      DSG Tunnel.  
//
//****************************************************************************

#include "DsgEstbTunnelSettings.h"
#include "TLV.h"
#include "DcdTlvCodes.h"

#include <map>
#include "MessageLog.h"

#include "DsgClientId.h"
#include "DsgClient.h"

//********************** Local Constants *************************************

// Set this to 0 to remove classifier debug stuff.  This will reduce code size
// and speed up performance.
#define DEBUG_CLASSIFIERS 0
typedef UcidList::iterator UcidListIterator;
typedef UcidList::const_iterator UcidListConstIterator;
//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


BcmDsgEstbTunnelSettings::BcmDsgEstbTunnelSettings() :	
	BcmDsgTunnelSettings(),
	//fUcidRange(0),
	fUcidList(),
	fClientIdList(),
	fRuleId(0),
	fRulePriority(0),
	fDefected( false ),
	fClassifierIdList(),
	fRuleIdSpecified(false),
	fRulePrioritySpecified(false),
	fUcidRangeSpecified(false),
	fClientIdSetsSpecified(false),
	fTunnelAddressSpecified(false),
	fClassifierIdSpecified(false),
	fVendorParamsSpecified(false),
	fUcidCertified(false),
	fToken(0),
	fClientRequestsSubTunnelForwarding(false),
	fIsClassifiedPrivateTunnel(false),
	fHasBroadcastFlow(false)
{
}


BcmDsgEstbTunnelSettings::BcmDsgEstbTunnelSettings( const BcmDsgEstbTunnelSettings& rhs )
  :
    BcmDsgTunnelSettings(rhs),
	fUcidList(),
	fClientIdList(),
	fClassifierIdList()
{
	fRuleId = rhs.fRuleId;
    fRulePriority = rhs.fRulePriority;
    //fUcidRange = rhs.fUcidRange;
	fUcidList = rhs.fUcidList;
    fClientIdList = rhs.fClientIdList;
	fClassifierIdList = rhs.fClassifierIdList;

	fDefected = rhs.fDefected;
	fRuleIdSpecified = rhs.fRuleIdSpecified;
	fRulePrioritySpecified = rhs.fRulePrioritySpecified;
	fUcidRangeSpecified = rhs.fUcidRangeSpecified;
	fClientIdSetsSpecified = rhs.fClientIdSetsSpecified;
	fTunnelAddressSpecified = rhs.fTunnelAddressSpecified;
	fClassifierIdSpecified = rhs.fClassifierIdSpecified;
	fVendorParamsSpecified = rhs.fVendorParamsSpecified;
	fVendorSettings = rhs.fVendorSettings;
	fUcidCertified = rhs.fUcidCertified;
	fToken = rhs.fToken;
	fClientRequestsSubTunnelForwarding = rhs.fClientRequestsSubTunnelForwarding;
	fIsClassifiedPrivateTunnel = rhs.fIsClassifiedPrivateTunnel;
	fHasBroadcastFlow = rhs.fHasBroadcastFlow;
}


BcmDsgEstbTunnelSettings::~BcmDsgEstbTunnelSettings()
{
    // Clean up my message log settings.
    //fMessageLogSettings.Deregister();
}

// Assignment operator.  
//
BcmDsgEstbTunnelSettings& BcmDsgEstbTunnelSettings::operator=( const BcmDsgEstbTunnelSettings& rhs )
{

    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    // call base class version.
    BcmDsgTunnelSettings::operator=( rhs );


	fRuleId = rhs.fRuleId;
    fRulePriority = rhs.fRulePriority;
    //fUcidRange = rhs.fUcidRange;
	fUcidList = rhs.fUcidList;	
    fClientIdList = rhs.fClientIdList;
	fClassifierIdList = rhs.fClassifierIdList;

	fDefected = rhs.fDefected;
	fRuleIdSpecified = rhs.fRuleIdSpecified;
	fRulePrioritySpecified = rhs.fRulePrioritySpecified;
	fUcidRangeSpecified = rhs.fUcidRangeSpecified;
	fClientIdSetsSpecified = rhs.fClientIdSetsSpecified;
	fTunnelAddressSpecified = rhs.fTunnelAddressSpecified;
	fClassifierIdSpecified = rhs.fClassifierIdSpecified;
	fVendorParamsSpecified = rhs.fVendorParamsSpecified;
	fVendorSettings = rhs.fVendorSettings;
	fUcidCertified = rhs.fUcidCertified;
	fToken = rhs.fToken;
	fClientRequestsSubTunnelForwarding = rhs.fClientRequestsSubTunnelForwarding;
	fIsClassifiedPrivateTunnel = rhs.fIsClassifiedPrivateTunnel;
	fHasBroadcastFlow = rhs.fHasBroadcastFlow;

    return *this;
}

ostream& BcmDsgEstbTunnelSettings::DerivedPrint( ostream& ostrm ) const
{    

#if !(BCM_REDUCED_IMAGE_SIZE)

    // consider flow settings
    ostrm 
        //<< "\n Ucid Range = " << fUcidRange 
		<< "\n Client requests Sub-Tunnel Forwarding = " << (fClientRequestsSubTunnelForwarding ? "true" : "false")
		<< "\n Is Classified Private Tunnel = " << (fIsClassifiedPrivateTunnel ? "true" : "false") 
		<< "\n Ucid Certified = " << (fUcidCertified ? "true" : "false")
		<< "\n Last Token = " << (uint32)fToken
		<< "\n Defected = " << (fDefected ?"true":"false")

        << "\n Number of Client IDs = " << fClientIdList.size() << endl;
		ClientIdListConstIterator clientId_it = fClientIdList.begin();
		ClientIdListConstIterator clientId_end = fClientIdList.end();
		while( clientId_it != clientId_end )
		{
			ostrm 
				<<" " << *clientId_it;
			++clientId_it;
		}

	ostrm 
        << " Number of UCIDs = " << fUcidList.size() << " List = [";

    // consider classifier settings
    if( !fUcidList.empty() )
    {
        UcidListConstIterator ucid_it = fUcidList.begin();
        UcidListConstIterator ucid_end = fUcidList.end();
        while( ucid_it != ucid_end )
        {
			ostrm 
				<<" " << (uint32) *ucid_it;			

            // increment iterator.
            ++ucid_it;
        }   
    }

    ostrm <<" ]" << endl;

	ostrm 
        << " Number of Classifiers = " << fClassifierSetsList.size() << " List = [";

    // consider classifier settings
    if( !fClassifierSetsList.empty() )
    {
        ClassifierSettingsListConstIterator cls_it = fClassifierSetsList.begin();
        ClassifierSettingsListConstIterator cls_end = fClassifierSetsList.end();
        while( cls_it != cls_end )
        {
            // found classifier settings.
            ostrm <<" " << (uint32)(cls_it->first) ;

            // increment iterator.
            ++cls_it;
        }   
    }
	
	ostrm <<" ]" << endl;

	ostrm 
        << " List of Classifiers = " << endl;

    //  classifier settings
    if( !fClassifierSetsList.empty() )
    {
        ClassifierSettingsListConstIterator cls_it = fClassifierSetsList.begin();
        ClassifierSettingsListConstIterator cls_end = fClassifierSetsList.end();
        while( cls_it != cls_end )
        {
            // found classifier settings.
            ostrm << (cls_it->second) << endl;

            // increment iterator.
            ++cls_it;
        }   
    }    

#endif

    return ostrm;
}


bool BcmDsgEstbTunnelSettings::operator==(
  const BcmDsgEstbTunnelSettings& rhs ) const
{
    if( fTunnelAddress != rhs.fTunnelAddress )
    {
        return false;
    }

	//cout <<"Step 1(out of 4): MAC address matched!";

	if( fClassifierIdSpecified )
	{
		if( fClassifierIdList != rhs.fClassifierIdList )
		{
			return false;
		}
	}
	//cout <<"Step 2(out of 4): Classifier ID List settings matched! ";

	if( fUcidRangeSpecified )
	{
		if( fUcidList != rhs.fUcidList )
		{
			cout <<" UCID List does not match! ";
			return false;
		}
	}
	//cout <<"Step 3(out of 4): UCID List settings matched! ";

	if( fClientIdSetsSpecified )
	{
		//#warning "fClientIdSetsSpecified - UNFINISHED"
		//if( fClientIdList != rhs.fClientIdList )
		{
		//	return false;
		}
	}

	//  classifier settings
    if( !fClassifierSetsList.empty() )
    {
		if( fClassifierSetsList != rhs.fClassifierSetsList )
		{
			cout <<" ClassifierSets List does not match! ";
			return false;
		}
    }

	//cout <<"Step 4(out of 4): ClassifierSets List settings matched! ";

	// UNFINISHED - PT
	//#warning"DsgTunnelSettings - Comparison Operator is not finished."

    return true;
}



void BcmDsgEstbTunnelSettings::Reset()
{
	fTunnelAddress = kAllZerosMac;

    if( !fClassifierSetsList.empty() )
    {
        fClassifierSetsList.clear();
    }

    if( !fUcidList.empty() )
    {
        fUcidList.clear();
    }

    if( !fClientIdList.empty() )
    {
        fClientIdList.clear();
    }

	if( !fClassifierIdList.empty() )
    {
        fClassifierIdList.clear();
    }

	fClientRequestsSubTunnelForwarding = false;
	fIsClassifiedPrivateTunnel = false;
	fHasBroadcastFlow = false;
}

// PARSE DSG RULE SETTINGS
byte BcmDsgEstbTunnelSettings::ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset,
    BcmMessageLogSettings* log_sets )
{
    TLVTable tlvTable[] =
    {
        // settings common to DCD top level.
		{ kDsgRuleId,		 1,	kTLV_uint8, &fRuleId, false, true, (char *) "Rule Id", &fRuleIdSpecified },

        { kDsgRulePriority,	 1,	kTLV_uint8, &fRulePriority, false, true, (char *) "Rule Priority", &fRulePrioritySpecified },

		//{ kDsgUcidRange,	 2,	kTLV_uint16, &fUcidRange, false, true, "\tDSG UCID Range", &fUcidRangeSpecified },

		{ kDsgUcidRange , kUnknownLength, kTLV_nested, NULL, true, false, (char *) "Ucid List", NULL },   

		{ kDsgClientIdSets , kUnknownLength, kTLV_nested, NULL, true, false, (char *) "Client Id", NULL },   

        { kDsgTunnelAddress, 6,	kTLV_MacAddress, &fTunnelAddress, false, true, (char *) "Tunnel Address", &fTunnelAddressSpecified },

        { kDsgClassifierId,	 kUnknownLength, kTLV_nested, NULL, true, true, (char *) "Classifier Id", &fClassifierIdSpecified },  
		
        { kDsgVendorSpec,		kUnknownLength,  kTLV_nested, NULL, true, false, (char *) "Vendor Specific", &fVendorParamsSpecified },

        // This must always be the last one in the table.
		kEndOfTLVTable
    };

	//cout <<"BcmDsgEstbTunnelSettings::ParseFrom" << endl;
    // Algorithm:
    //    Set up table to look for the top level Type codes.
    //    while top level type found...
    //      ...create new buffer, referring to the V bytes following top TL,
    //          going for length-1 bytes.
    //      create target object.  have it parse its TLVs.
    //      store target object.
    //    endwhile
    //


    // Parse the TLVs until end of input or special case exit.
    TLVParserResult result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);

    while( result == kTLVParserStopParsingFound )
    {
        // parser found a mutant type.  examine it.
        //
        // Note that this is set up such that if multiple instances of these
        // child types are encountered, then they will be applied to the
        // settings that have already been parsed.  The next effect is that
        // they will be treated as though they came across in one big lump.
        uint8 typeField, lengthField;

        if (!octetBuffer.NetworkRead(offset, typeField) ||
            !octetBuffer.NetworkRead(offset, lengthField))
        {
            return kRejMsgSyntaxError;
        }

        switch( typeField )
        {
            case kDsgUcidRange:
                {
					// create temporary octet buf and offset.
					BcmOctetBuffer tmp_obuf(0);
					unsigned int tmp_offset = 0;

                    // load an octet buf with the V bytes of the parent 
                    // (a.k.a. the child TLV's).
					// Make sure to include type and length fields
					// when passing it to the child buffer below.
					// That is why child buffer points to (offset-2).
					offset = offset - 1;
                    tmp_obuf.Overlay( octetBuffer, offset, (lengthField + 1) );
                    tmp_offset = 0;

					byte confCode;
					confCode = ParseUcidList( tmp_obuf, tmp_offset, log_sets );

                    if (confCode == kConfOk)              
                    {                    
                        // update offset of top level buffer.
                        offset += tmp_offset;
                    }
                    else
                    {
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Dsg Rule UCID TLVs" << endl;
                        }

                        return kRejOther;
                    }
                }
                break;


            case kDsgClientIdSets:
                {
					// create temporary octet buf and offset.
					BcmOctetBuffer tmp_obuf(0);
					unsigned int tmp_offset = 0;

                    // load an octet buf with the V bytes of the parent 
                    // (a.k.a. the child TLV's).
                    tmp_obuf.Overlay( octetBuffer, offset, lengthField );
                    tmp_offset = 0;
                    
                    // have parent obj load the kids from tempBuffer
                    if( ParseClientId( tmp_obuf, tmp_offset, log_sets ))                    
                    {
                        // all is well.  note that settings have been specified. 
						// This also means at least one client ID exists in the list.
                        fClientIdSetsSpecified = true;
                    
                        // update offset of top level buffer.
                        offset += tmp_offset;
                    }
                    else
                    {
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Dsg Rule Client ID TLVs" << endl;
                        }

                        fClientIdSetsSpecified = false;
                        return kRejOther;
                    }
                }
                break;

            case kDsgClassifierId:
                {
					uint16 classifierId;
					if (!octetBuffer.NetworkRead(offset, classifierId))
					{
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Dsg Rule Classifier ID TLVs" << endl;
                        }
						return kRejMsgSyntaxError;
					}

					// all is well.  note that settings have been specified.
					// Note that there could be multiple classifier IDs in a rule.
					fClassifierIdSpecified = true;

					// Insert into the channel list.
					fClassifierIdList.push_back(classifierId);
                }
                break;

            case kDsgVendorSpec:
                {
                    BcmOctetBuffer tempBuffer(0);
                    unsigned int tempOffset;
                    byte confCode;

                    // Overlay the V portion of this parent TLV.
                    tempBuffer.Overlay(octetBuffer, offset, lengthField);
                    tempOffset = 0;

                    // Parse the values.
                    confCode = fVendorSettings.ParseFrom(tempBuffer, tempOffset, log_sets);

                    if (confCode == kConfOk)
                    {
                        // Update the offset of the top level buffer.
                        offset += tempOffset;

						fVendorParamsSpecified = true;
                    }					
                }
                break;

        }

        // start the parser once again
        
        // Parse the TLVs until end of input or special case exit.
        result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);
    }
	
    // Check for an error during parsing.  Do we skip over unrecognized type
    // codes, or do we error?????  We should skip over rather than error; this
    // is done for us by the parser.
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...
        if (log_sets)
        {
            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse basic DSG Rule TLVs" << endl;
        }

        return kRejMsgSyntaxError;
    }

    /*if( !strict_checking )
    {
        // special case: client has specified that parsed values are NOT to
        // be validated w/strict error checking rules.
        return kConfOk;
    }

    if( !err_set_list.empty() )
    {
        // at least one error set is present.  
        // add rx_err_set_list to the errorEncodingList param passed to 
        // this function.
        return errorEncodingList.AddQosErrorEncoding( *this, err_set_list );
    }*/

	// Check if the tunnel is good to go.
	if( !MustHaveSetsSpecified() )
	{
        if (log_sets)
        {
			gErrorMsg(*log_sets, "ParseFrom") << "At least one must-have DSG Rule TLV is missing." << endl;
		}
		return kRejectRequiredSettingNotPresent;
	}

	/*
	// Test UCID Range
	if(fUcidRangeSpecified)
	{
		uint8 ucidLow = (fUcidRange >> 8);
		uint8 ucidHigh = fUcidRange & 0xFF;
		
		if(ucidLow >= ucidHigh)
		{
			if (log_sets)
			{
				gErrorMsg(*log_sets, "ParseFrom") << "ucidLow >= ucidHigh" << endl;
			}
			return kRejOutOfRangeError;
		}
	}*/

    // NOTE:  It is assumed that some higher-layer entity is going to verify
    // that the required things are present, and that the combination of
    // settings that are present is allowed...

	//cout <<"BcmDsgEstbTunnelSettings::ParseFrom returns kConfOk" << endl;
    return kConfOk;
}	


byte BcmDsgEstbTunnelSettings::ParseUcidList( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets )
{
//	bool fUcidRangeSpecified = false;
	TLVParserResult result = kTLVParserError;

	uint8 lengthField =0;
	if( !obuf.Read(offset, lengthField) )
	{
		return kRejMsgSyntaxError;
	}

    gInfoMsgNoFields(*log_sets) 
        << " UCID List: " << endl;

	while(lengthField--)
	{
		uint8 ucid = 0;
		if( !obuf.Read(offset, ucid) )
		{
			return kRejMsgSyntaxError;
		}
			
        gInfoMsgNoFields(*log_sets) 
            <<"\t" << (uint32) ucid << endl;

		fUcidList.push_back( ucid );
	}

	// Check if any ucid value is specified.
	if(!fUcidList.empty())
	{
		fUcidRangeSpecified = true;
	}

	return kConfOk;
}

bool BcmDsgEstbTunnelSettings::ParseClientId( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets )
{
	// obuf holds only the client ID list TLVs.
	// Client ID Type= 0x04 and Length is not included
	// in this obuf. Those fields are read before this
	// buffer is passed here.
	//
	// an example of obuf would be: 
	// 0x03, 0x02, 0x00, 0x01,
	// 0x04, 0x02, 0x00, 0x02

	// The size of obuf is obuf.BytesUsed()
	while( offset < obuf.BytesUsed() )
	{
		// Now process each client ID TLV independently. 
		// Read type and length
		uint8 typeField, lengthField;

		if (!obuf.NetworkRead(offset, typeField) ||
			!obuf.NetworkRead(offset, lengthField))
		{
			return false;
		}

		// Now that we know the length of this client ID value
		// create temporary octet buf and offset to pass this buffer 
		// to be processed in the DsgClientId object.
		BcmOctetBuffer tmp_obuf(0);
		unsigned int tmp_offset = 0;

		// load an octet buf with the V bytes of the parent 
		// (a.k.a. the child TLV's).
		// Make sure to include type and length fields
		// when passing it to the child buffer below.
		// That is why child buffer points to (offset-2).
		offset = offset - 2;
		tmp_obuf.Overlay( obuf, offset, (lengthField + 2) );
		//tmp_offset = 0;

		// create obj.
		BcmDsgClientId dsgClientId; 
    
		// have parent obj load the kids from tempBuffer
		if( dsgClientId.ParseFrom( tmp_obuf, tmp_offset, log_sets ) == kConfOk )           
		{                    
			// all is well.  store parent obj.
			fClientIdList.push_back( dsgClientId );
			cout << dsgClientId.IdTypeName(dsgClientId.IdType()) <<" Client ID = " << (uint32) dsgClientId.Id() << endl;

			if(dsgClientId.IdType() == kBroadcastType)
			{
				fHasBroadcastFlow = true;
			}

			// update offset of top level buffer.
			offset += tmp_offset;
		}
		else
		{
			// error.
			if (log_sets)
			{
				gErrorMsg(*log_sets, "BcmDsgEstbTunnelSettings::ParseClientId") 
				 << " Failed to parse Dsg Rule Client ID TLVs" << endl;
			}

			return false;
		}
	}

	return true;
}

// this method checks whether Tunnel Address and at least one
// CLient ID is specified. If not, what good is this tunnel?
const bool BcmDsgEstbTunnelSettings::MustHaveSetsSpecified(void) const
{
    if (fClientIdSetsSpecified &&
        fTunnelAddressSpecified )
    {
        return true;
    }

    return false;
}

// DCD object needs to know whether this tunnel has any classifier ID specified.
const bool BcmDsgEstbTunnelSettings::ClassifierIdSpecified(void) const
{
	return fClassifierIdSpecified;
}

// DCD object needs to know whether this tunnel has any classifier ID specified.
const bool BcmDsgEstbTunnelSettings::VendorParamsSpecified(void) const
{
	return fVendorParamsSpecified;
}

// Use this to filter the DSG Rules that specify a UCID range.
// Apply our UCID value to the UCID Range of a DSG Rule.
const bool BcmDsgEstbTunnelSettings::TestUcid(uint8 ucid, bool ucidAvailable)
{
	// Do not open the tunnel if it specifies a UCID range but we do not have any UCID.
	if( !ucidAvailable && fUcidRangeSpecified )
	{
		return false;
	}

	// Do not open the tunnel if we have a UCID but it does not fall in the specified range.
	if( ucidAvailable )
	{
		if(fUcidRangeSpecified)
		{

			UcidListIterator ucid_it = fUcidList.begin();
			UcidListIterator ucid_it_end = fUcidList.end();
			while( ucid_it != ucid_it_end )
			{
				// This will compare type and client ID.
				if( *ucid_it == ucid )
				{
					// Matched ucid value.
					fUcidCertified = true; 
					return true;
				}
				++ucid_it;
			}

			// UCID does not match any of the values in the list.
			return false;

			/*
			uint8 ucidLow = (fUcidRange >> 8);
			uint8 ucidHigh = fUcidRange & 0xFF;

			// Check if given UCID falls in this range.
			if( (ucidLow <= ucid) && (ucid <= ucidHigh) )
			{
				fUcidCertified = true; 
				return true;
			}
			return false;*/
		}
	}
	// This rule does not specify a Ucid Range.
	// Always return true.
	fUcidCertified = true; 
	return true;
}

BcmDsgClientId* BcmDsgEstbTunnelSettings::FindClient(BcmDsgClient *pClient)
{
	ClientIdListIterator clientId_it = fClientIdList.begin();
	ClientIdListIterator clientId_end = fClientIdList.end();
	while( clientId_it != clientId_end )
	{
		// This will compare type and client ID.
		if( *clientId_it == *pClient )
		{
			// Matched client.
			return &(*clientId_it);
		}
		++clientId_it;
	}

	return NULL;
}
/*
bool BcmDsgEstbTunnelSettings::AddToForwardingList(ClientIdListIterator &clientIterator, BcmDsgClient *pClient)
{
	clientIterator->fClientPort = pClient->fClientPort;
	clientIterator->fActive = true;

	// sort the list from active to not-active
}
*/

const bool BcmDsgEstbTunnelSettings::HasScte65BroadcastData(void) 
{
	ClientIdListIterator clientId_it = fClientIdList.begin();
	ClientIdListIterator clientId_end = fClientIdList.end();
	while( clientId_it != clientId_end )
	{
		// This will compare type and client ID.
		if( clientId_it->IsBroadcastSCTE65() )
		{
			return true;
		}
		++clientId_it;
	}

	return false;
}

bool BcmDsgEstbTunnelSettings::WriteTo( BcmOctetBuffer &octetBuffer ) const
{
	ClassifierSettingsListConstIterator cls_it = begin();
	ClassifierSettingsListConstIterator cls_end = end();
	while( cls_it != cls_end)
	{
		if( (cls_it->second.WriteTo(octetBuffer)) == false )
		{
			return false;
		}

		cls_it++;
	}

	return true;
}

void BcmDsgEstbTunnelSettings::Token(uint8 &token)
{
	fToken = token;
}

uint8 &BcmDsgEstbTunnelSettings::Token(void)
{
	return fToken;
}

// This function searches tunnel's classifier settings list for a matching element for the specified argument.
const bool BcmDsgEstbTunnelSettings::IsEqualClassifier(const BcmIpClassifierSettings& ipClassifierSets) const
{
	ClassifierSettingsListConstIterator cls_it = begin();
	
    while (cls_it != end())
	{
		if(cls_it->second.fIpClassifierSetsSpecified ) 
		{
			if (cls_it->second.fIpClassifierSets == ipClassifierSets)
			{
				cout
					<<"Client's classifier " << ipClassifierSets << " matched tunnel's classifier "
					<< cls_it->second.fIpClassifierSets << endl;

				return true;
			}
		}
        
        cls_it++;
    }

	// else matching element does not exist. 
	cout
		<<"Client's classifier did not match any of the tunnel's classifiers " << endl;

	return false;
}

