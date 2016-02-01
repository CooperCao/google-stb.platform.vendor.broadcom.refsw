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
//  Filename:       DcdMsg.cpp
//  Author:         Pinar Taskiran
//  Creation Date:  Dec 2, 2004
//
//****************************************************************************
//  Description:
//      This is the implementation of the DCD MAC
//      Management Message.
//
//****************************************************************************

//********************** Include Files ***************************************

// My api and definitions...
#include "DcdMsg.h"

// For spewing some of my values out as TLVs...
#include "TLV.h"
#include "DcdTlvCodes.h"

#include "MessageLogSettings.h"
#include "DsgEstbTunnelSettingsList.h"
#include "DsgConfigSettings.h"

// For memset, NULL, etc.
#include <string.h>
#include <stdlib.h>

#ifndef TLVCODES_H
char* spec = "";
char* no_spec = " (NOT specified)";
#endif
//********************** Local Types *****************************************

//********************** Local Constants *************************************
#define kDefaultConfigChangeCount 0x5A
//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


BcmDcdMsg::BcmDcdMsg(BcmDsgEstbTunnelSettingsList* pTunnelManager, ClassifierSettingsList* pClassifierList, 
					 BcmDsgConfigSettings* pConfigSettings )
  :
    fConfigurationChangeCount( kDefaultConfigChangeCount ),
    fNumberOfFragments( 0 ),
    fFragmentSequenceNumber( 0 ),
	pfTunnelManager(NULL),
	pfClassifierList(NULL),
	pfConfigSettings(NULL),
	fPacketClassificationSetsSpecified(false),
	fPacketClassificationSetsRequired(false),
	fDsgRuleSetsSpecified(false),
	fDsgConfigurationSetsSpecified(false),
	pfDcdBuffer(NULL),
	fMessageLogSettings("BcmDcdMsg", "DCD")

{
	// Store list pointers to store DCD settings.
    pfTunnelManager = pTunnelManager;
	pfClassifierList = pClassifierList;
	pfConfigSettings = pConfigSettings;

	// Get my msg log settings to register itself with its command table.
    fMessageLogSettings.Register();	
}

BcmDcdMsg::~BcmDcdMsg()
{
	if(pfDcdBuffer)
        delete pfDcdBuffer;
}


byte BcmDcdMsg::ParseFrom( const BcmOctetBuffer &octetBuffer,
    unsigned int &offset )
{
    // first read binary encoded sid and response code.
    if( !octetBuffer.NetworkRead( offset, fConfigurationChangeCount ) 
        || !octetBuffer.NetworkRead( offset, fNumberOfFragments ) 
		|| !octetBuffer.NetworkRead( offset, fFragmentSequenceNumber )
	  )
    {
        return kRejMsgSyntaxError;
    }

    return kConfOk;
}

            
ostream& BcmDcdMsg::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    // print derived class info.
    ostrm 		 
            << setw( 30 ) <<"Configuration Change Count: " << dec << (uint32) fConfigurationChangeCount <<"\n"
            << setw( 30 ) <<"       Number Of Fragments: " << dec << (uint32) fNumberOfFragments <<"\n"
            << setw( 30 ) <<"  Fragment Sequence Number: " << dec << (uint32) fFragmentSequenceNumber 
            << "\n";
#endif

    return ostrm;
}


byte BcmDcdMsg::ParseTLVBuffer(const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, BcmMessageLogSettings* log_sets, bool parse_only )
{
    BcmOctetBuffer tempBuffer(0);
    unsigned int tempOffset;
    byte conf_code;

    ErrorSetList err_set_list;
	//BcmMessageLogSettings msg_log_sets;
	//BcmMessageLogSettings* log_sets = &msg_log_sets;
	log_sets = &fMessageLogSettings;
	bool strict_checking = true;

	// Reset settings before starting to parse.
	fPacketClassificationSetsSpecified = false;
	fPacketClassificationSetsRequired = false;
	fDsgRuleSetsSpecified = false;
	fDsgConfigurationSetsSpecified = false;

    TLVTable tlvTable[] =
    {
        // settings common to DCD top level.
		{ kDsPacketClassificationSets,	kUnknownLength,	kTLV_nested, NULL, true,  false, (char *) "PACKET CLASSIFICATION", NULL },
        { kDsgRuleSets,					kUnknownLength,	kTLV_nested, NULL, true,  false, (char *) "DSG RULE", NULL }, 
		{ kDsgConfigurationSets,		kUnknownLength,	kTLV_nested, NULL, true,  false, (char *) "DSG CONFIGURATION", NULL }, 
        // This must always be the last one in the table.
		kEndOfTLVTable
    };

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
            case kDsPacketClassificationSets:
                {
                    // have parent obj load the kids from tempBuffer
                    conf_code = ParseDsPacketClassificationSets( octetBuffer, 
                        offset, lengthField, err_set_list, log_sets, strict_checking, parse_only);

                    if( conf_code == kConfOk )
                    {
                        // all is well.  note that settings have been specified. 
						// this also means that at least one good classifier exists.
                        fPacketClassificationSetsSpecified = true;
					}
					else               
                    {
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Packet Classification TLVs" << endl;
                        }
                        fPacketClassificationSetsSpecified = false;
                        return kRejOther;
                    }

                }
                break;

            case kDsgRuleSets:
                {
                    // load an octet buf with the V bytes of the parent 
                    // (a.k.a. the child TLV's).
                    tempBuffer.Overlay( octetBuffer, offset, lengthField );
                    tempOffset = 0;
                    
                    // have parent obj load the kids from tempBuffer
                    conf_code = ParseDsgRuleSets( tempBuffer, tempOffset, log_sets, parse_only );
                    if( conf_code == kConfOk )
                    {
                        // all is well.  note that settings have been specified. 
						// this also means that at least one good tunnel exists.
                        fDsgRuleSetsSpecified = true;
                    
                        // update offset of top level buffer.
                        offset += tempOffset;						
                    }
                    else
                    {
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Dsg Rule TLVs" << endl;
                        }
                        fDsgRuleSetsSpecified = false;
                        return conf_code;
                    }
                }
                break;

            case kDsgConfigurationSets:
                {
                    // load an octet buf with the V bytes of the parent 
                    // (a.k.a. the child TLV's).
                    tempBuffer.Overlay( octetBuffer, offset, lengthField );
                    tempOffset = 0;
                    
					// If the caller wants to "only" parse the TLV buffer
					// without affecting Dsg Client Control Thread's
					// tunnel, classifier and config settings, then
					// create a BcmDsgConfigSettings object on the stack
					// so that the TLV parser can do its job.
					BcmDsgConfigSettings *pConfigSettings = NULL;
					if( parse_only )
					{
						BcmDsgConfigSettings configSettings;
						pConfigSettings = &configSettings;
					}
					else
					{
						pConfigSettings = pfConfigSettings;
					}

                    // have parent obj load the kids from tempBuffer
                    conf_code = pConfigSettings->ParseFrom( tempBuffer, 
                        tempOffset, log_sets, strict_checking );
                    if( conf_code == kConfOk )
                    {
                        // all is well.  note that settings have been specified. 
                        fDsgConfigurationSetsSpecified = pfConfigSettings->AnythingSpecified();
                    
                        // update offset of top level buffer.
                        offset += tempOffset;
                    }
                    else
                    {
                        // error.
                        if (log_sets)
                        {
                            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse Dsg Configuration TLVs" << endl;
                        }

                        fDsgConfigurationSetsSpecified = false;
                        return conf_code;
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
            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse basic Classifier TLVs" << endl;
        }

        return kRejMsgSyntaxError;
    }

    if( !strict_checking )
    {
        // special case: client has specified that parsed values are NOT to
        // be validated w/strict error checking rules.
        return kConfOk;
    }

    // At least one of the DSG Rule or DSG COnfiguration settings must be present
	// Classifier setting is not required unless a DSG Rule specifies the ID.
    if (!fDsgRuleSetsSpecified && !fDsgConfigurationSetsSpecified)
    {
		return kRejectRequiredSettingNotPresent;
    }
	else if ( fDsgRuleSetsSpecified ) 
	{
		if(fPacketClassificationSetsRequired && !fPacketClassificationSetsSpecified)
		{
			// There is at least one Rule that requires a Classifier Setting 
			// but no Classifier TLVs are specified.
			return kRejectRequiredSettingNotPresent;
		}
	}


/* ADDITIONAL ERROR CHECKING. COME BACK LATER FOR THIS

    if( !err_set_list.empty() )
    {
        // at least one error set is present.  
        // add rx_err_set_list to the errorEncodingList param passed to 
        // this function.
        return errorEncodingList.AddQosErrorEncoding( *this, err_set_list );
    }
*/
    // NOTE:  It is assumed that some higher-layer entity is going to verify
    // that the required things are present, and that the combination of
    // settings that are present is allowed...

    return kConfOk;
}

// PARSE DSG RULE SETTINGS
byte BcmDcdMsg::ParseDsPacketClassificationSets(const BcmOctetBuffer &obuf,
    unsigned int &offset, uint8 length, ErrorSetList& err_set_list, 
    BcmMessageLogSettings* log_sets, bool strict_checking, bool parse_only)
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tmp_obuf(0);
    unsigned int tmp_offset = 0;
    
    // load an octet buf with the V bytes of the parent 
    // (a.k.a. the child TLV's).
    tmp_obuf.Overlay( obuf, offset, length );
    tmp_offset = 0;
    
    // create parent obj.
    BcmDsgClassifierSettings classifierSetting; 

    // have parent obj load the kids from tempBuffer
    byte confCode = classifierSetting.ParseFrom( tmp_obuf, tmp_offset, err_set_list, log_sets, strict_checking );
	if( confCode == kConfOk )
    {
        // all is well.  store parent obj.
		// insert classifierSetting into the container.    
		if(!parse_only)
			pfClassifierList->insert( ClassifierSettingsList::value_type((byte)(classifierSetting.CmtsClassifierId()), classifierSetting));
        
        // update offset of top level buffer.
        offset += tmp_offset;        
    }
    else
    {
        // error.
        if (log_sets)
        {
            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse DsPacketClassificationSets TLVs" << endl;
        }
    }

    return confCode;
}

// PARSE DSG RULE SETTINGS
byte BcmDcdMsg::ParseDsgRuleSets( const BcmOctetBuffer &obuf, unsigned int &offset,
    BcmMessageLogSettings* log_sets, bool parse_only )
{
    // create temporary octet buf and offset.
    //BcmOctetBuffer tmp_obuf(0);
    //unsigned int tmp_offset = 0;

    // create parent obj.
    BcmDsgEstbTunnelSettings dsgTunnelSettings; 
    
    // have parent obj load the kids from tempBuffer
    byte confCode = dsgTunnelSettings.ParseFrom( obuf, offset, log_sets );
	if( confCode == kConfOk )
    {
        // all is well.  store parent obj.
		// Just add the tunnel to the list. Multiple tunnels
		// with the same MAC address may exist.
		if(!parse_only)
			pfTunnelManager->fTunnelSetsList.insert( TunnelSettingsList::value_type(dsgTunnelSettings.TunnelAddress(), dsgTunnelSettings));
		//cout <<"BcmDcdMsg::ParseDsgRuleSets returns kConfOk" << endl;

		// Check if this DSG rule specified any classifier.
		if(dsgTunnelSettings.ClassifierIdSpecified())
		{
			// This means this DCD must specify at least one Classifier Setting.
			fPacketClassificationSetsRequired = true;
		}
    }

    return confCode;
}

// This method should only be called after all DCD fragments are received.
// DCD fragments are parsed as they are received. Tunnel Settings List 
// and Classifier Settings List are filled in with DCD contents.
// After the DCD is completed, these two lists must be merged for final processing.
bool BcmDcdMsg::FinalizeDcdParsing(void)
{
	bool dcdValid = false;

    TunnelSettingsListIterator tunnelSet_it = pfTunnelManager->begin();
    TunnelSettingsListIterator tunnelSet_end = pfTunnelManager->end();
    while( tunnelSet_it != tunnelSet_end )
    {
		{
			// For every tunel that has a classifier ID list that is not empty,
			// go find the related settings for that classifier ID and insert the
			// settings into this tunnel's Classifier list.
			
			// Let's make it easy to read the code.
			const ClassifierIdList *pClsIdList = &(tunnelSet_it->second.fClassifierIdList);

			if( pClsIdList->empty() )
			{
				gLogMessageRaw << "*****Classifier ID List is EMPTY *****"  
				 << endl;
				dcdValid = true;
			}
			if( !pClsIdList->empty() )
			{
				ClsListConstIterator cls_it = pClsIdList->begin();
				ClsListConstIterator cls_end = pClsIdList->end();
				while( cls_it != cls_end)
				{
					// Go find the settings for this Classifier ID.
					// *cls_it = classifier ID
					ClassifierSettingsListConstIterator clsSet_it = pfClassifierList->find(*cls_it);
					if( clsSet_it != pfClassifierList->end() )
					{
						// Insert it into this tunnel's Classifier Settings List.
						// (clsSet_it->first = 16-bit ID) (clsSet_it->second = Settings Object)
						tunnelSet_it->second.add( clsSet_it->second );

						//gLogMessageRaw << "Adding Classifier with ID: " 
						//	 << *cls_it  << endl;
					}
					else
					{
						gLogMessageRaw << "Error: Classifier Settings for this Classifier ID: "
							 << *cls_it << "are not found in the DCD. "<< endl;

						// Is this an error case? I guess this tunnel is not usable
						// Maybe we should check after we finish processing the classifier
						// ID list. If the classifier settings list of this tunnel is still
						// empty, then we should mark this tunnel as "DEFECTED".
						// When we start opening tunnels using the registered client information,
						// we should only use the tunnels that are NOT marked DEFECTED.
						// So, we do not stop here and return error.
					}
            
					// increment iterator.
					++cls_it;
				}   

				// Now we are done processing the Classifier ID list for this tunnel.
				// Check whether there is at least one classifier setting is found.
				if(tunnelSet_it->second.empty())
				{
					// Mark this tunnel DEFECTED.
					// Should we not even keep these tunnels in the list? 
					// We will never open them.
					tunnelSet_it->second.Defected( true );
					gLogMessageRaw << "DEFECTED" << endl;
				}
				else
				{
					// Note that at least one tunnel is usable.
					dcdValid = true;
				}
			}
		}

        // increment iterator.
        ++tunnelSet_it;
    }  

	// Process only if this tunnel is not subset of another tunnel or 
	// is not defected. If is subset of another tunnel then it must 
	// be marked defected.
    tunnelSet_it = pfTunnelManager->begin();
    tunnelSet_end = pfTunnelManager->end();
    while( tunnelSet_it != tunnelSet_end )
    {
		if( pfTunnelManager->IsSubSetOrDefected(tunnelSet_it) )
		{
			gLogMessageRaw << "This tunnel will be ignored.INVALID tunnel settings.\n" << tunnelSet_it->second << endl;
		}

		++tunnelSet_it;
	}

	// This will return false if all tunnels are defected and
	// dcdValid never set to true.
	return dcdValid;
}

void BcmDcdMsg::OctetBuffer(BcmOctetBuffer *pDcdBuffer)
{
	if(pfDcdBuffer == NULL)
	{
		pfDcdBuffer = new BcmOctetBuffer(pDcdBuffer->BytesUsed());
	}

	if(pfDcdBuffer)
	{
		// Copy the packet data.
		*pfDcdBuffer = *pDcdBuffer;	
	}
}

const BcmOctetBuffer * BcmDcdMsg::OctetBuffer(void)
{
	if(pfDcdBuffer)
	{
		return pfDcdBuffer;
	}

	return NULL;
}

void BcmDcdMsg::PrintTLVs(void)
{
	#if !(BCM_REDUCED_IMAGE_SIZE)
	if(pfDcdBuffer != NULL)
	{
		unsigned int offset = 0;
		//BcmMessageLogSettings log_sets;

		// This will display first three fields of DCD.
		ParseFrom((const BcmOctetBuffer &)*(pfDcdBuffer), offset);

		// print derived class info.
		gLogMessageRaw 
            << setw( 30 ) <<"Configuration Change Count: " << dec << (uint32) fConfigurationChangeCount <<"\n"
            << setw( 30 ) <<"       Number Of Fragments: " << dec << (uint32) fNumberOfFragments <<"\n"
            << setw( 30 ) <<"  Fragment Sequence Number: " << dec << (uint32) fFragmentSequenceNumber 
            << "\n";


		// Note that last argument of ParseTLVBuffer() indicates that we only want to 
		// see the buffer parsed. If it is set to false (parse_only = false) then 
		// ParseTLVBuffer() will not only parse the buffer but also fill the tunnel 
		// settings, classifier settings and dsg config settings of the Client Controller thread.
		ParseTLVBuffer((const BcmOctetBuffer &)*(pfDcdBuffer), offset, &fMessageLogSettings, true);
	}
	#endif
}
