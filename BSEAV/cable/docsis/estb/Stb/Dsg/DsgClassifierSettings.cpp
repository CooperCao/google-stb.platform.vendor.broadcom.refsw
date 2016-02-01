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
//****************************************************************************
//
//  Filename:       DsgClassifierSettings.cpp
//  Author:         Pinar Taskiran 
//				    (adapted from ClassifierSettings.cpp by Cliff Danielson)
//  Creation Date:  {unknown}
//
//****************************************************************************
//  Description:
//
//****************************************************************************

//********************** Include Files ***************************************

#include "DsgClassifierSettings.h"
#include "OctetBuffer.h"
#include "TLV.h"

#include "MessageLog.h"
#include <string.h>
#include "MessageLogSettings.h"

#include "DcdTlvCodes.h"

//********************** Local Types *****************************************

#if (BCM_STANDALONE_DSGCC)
const char *spec = "specified";
const char *no_spec = "not specified";
#endif
  
//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************

BcmDsgClassifierSettings::BcmDsgClassifierSettings( byte starting_type_code )
  :
    fCmtsClassifierId( 0 ),
	fClassifierIdSpecified(false),
    fRulePriority( 0 ),
    fRulePrioritySpecified( false ),
    fIpClassifierSets(),
    fIpClassifierSetsSpecified( false )
{
}


BcmDsgClassifierSettings::BcmDsgClassifierSettings( const BcmDsgClassifierSettings& rhs )
  :
    fCmtsClassifierId( rhs.fCmtsClassifierId ),
    fClassifierIdSpecified( rhs.fClassifierIdSpecified ),
    fRulePriority( rhs.fRulePriority ),
    fRulePrioritySpecified( rhs.fRulePrioritySpecified ),
    fIpClassifierSets( rhs.fIpClassifierSets ),
    fIpClassifierSetsSpecified( rhs.fIpClassifierSetsSpecified )
{
}


BcmDsgClassifierSettings& BcmDsgClassifierSettings::operator=( 
    const BcmDsgClassifierSettings& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fCmtsClassifierId = rhs.fCmtsClassifierId;
    fClassifierIdSpecified = rhs.fClassifierIdSpecified;
    fRulePriority = rhs.fRulePriority;
    fRulePrioritySpecified = rhs.fRulePrioritySpecified;
    fIpClassifierSets = rhs.fIpClassifierSets;
    fIpClassifierSetsSpecified = rhs.fIpClassifierSetsSpecified;

    return *this;
}




bool BcmDsgClassifierSettings::operator<( const BcmDsgClassifierSettings& rhs ) const
{
    // this is to pacify Borland C++.  it is doubtful that it will ever be
    // called.
    // first test cmts assigned sfid.

    // second test cmts assigned clasifier id.
    if( fCmtsClassifierId < rhs.fCmtsClassifierId )
    {
        return true;
    }
    else if( fCmtsClassifierId > rhs.fCmtsClassifierId )
    {
        return false;
    }

    // else...
    return false;
}

bool BcmDsgClassifierSettings::operator==(
  const BcmDsgClassifierSettings& rhs ) const
{
    // Note - only compare the values if they were specified.  If they weren't
    // specified, then the actual values are don't cares.
    if ((fClassifierIdSpecified != rhs.fClassifierIdSpecified) ||   
        (fRulePrioritySpecified != rhs.fRulePrioritySpecified) ||
        (fIpClassifierSetsSpecified != rhs.fIpClassifierSetsSpecified))
    {
        return false;
    }

    if (fClassifierIdSpecified &&
        (fCmtsClassifierId != rhs.fCmtsClassifierId))
    {
        return false;
    }

    if (fRulePrioritySpecified &&
        (fRulePriority != rhs.fRulePriority))
    {
        return false;
    }

    if (fIpClassifierSetsSpecified &&
        (fIpClassifierSets != rhs.fIpClassifierSets))
    {
        return false;
    }

    return true;
}


ostream& BcmDsgClassifierSettings::Print( ostream& ostrm ) const
{

#if !(BCM_REDUCED_IMAGE_SIZE)

    // print settings specific to this class.
    ostrm << "\ninter-galactic classifier settings:"

        << "\n     Classifier id: " << (void *) ((uint32)fCmtsClassifierId)
            << ( fClassifierIdSpecified ? spec : no_spec )

        << "\n          rule priority: " << (void *) ((uint32) fRulePriority)
            << ( fRulePrioritySpecified ? spec : no_spec )

      << "\n\n IP classifier settings: "
            << ( fIpClassifierSetsSpecified ? spec : no_spec )
            << fIpClassifierSets << endl;
        
#endif

    return ostrm;
}

byte BcmDsgClassifierSettings::ParseFrom(const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, ErrorSetList& err_set_list, 
    BcmMessageLogSettings* log_sets, bool strict_checking )
{
    // create temporary octet buf and offset.
    BcmOctetBuffer tempBuffer(0);
    unsigned int tempOffset;
    byte conf_code;

    TLVTable tlvTable[] =
    {
        // settings common to DCD top level.
		{ kClassifierId,	 2,	kTLV_uint16, &fCmtsClassifierId, false, true, "\tClassifier Id:", &fClassifierIdSpecified },
        { kRulePriority,	 1,	kTLV_uint8,  &fRulePriority, false, true, "\tClassifier Rule Priority", &fRulePrioritySpecified },
		{ kIpClassifierSets, kUnknownLength, kTLV_nested, NULL, true, true, "\tIP CLASSIFIER SETTINGS:", NULL },   

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

            case kIpClassifierSets:
                {
                    // load an octet buf with the V bytes of the parent 
                    // (a.k.a. the child TLV's).
                    tempBuffer.Overlay( octetBuffer, offset, lengthField );
                    tempOffset = 0;
                    
                    // have parent obj load the kids from tempBuffer
                    conf_code = fIpClassifierSets.ParseFrom( tempBuffer, 
                        tempOffset, err_set_list, log_sets, strict_checking );
					if( conf_code == kConfOk )
					{
						// all is well.  note that settings have been specified. 
						fIpClassifierSetsSpecified = fIpClassifierSets.AnythingSpecified();
    
						// update offset of top level buffer.
						offset += tempOffset;
					}
					else
					{
						// error.
						if (log_sets)
						{
							gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse IpClassifier TLVs" << endl;
						}
						fIpClassifierSetsSpecified = false;
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

    /*if( !err_set_list.empty() )
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
			gErrorMsg(*log_sets, "ParseFrom") << "At least one must-have DSG Classifier TLV is missing." << endl;
		}
		return kRejectRequiredSettingNotPresent;
	}

    // NOTE:  It is assumed that some higher-layer entity is going to verify
    // that the required things are present, and that the combination of
    // settings that are present is allowed...

    return kConfOk;
}	

// this method checks whether Tunnel Address and at least one
// CLient ID is specified. If not, what good is this tunnel?
bool BcmDsgClassifierSettings::MustHaveSetsSpecified(void) const
{
    if (fClassifierIdSpecified &&
        fIpClassifierSetsSpecified )
    {
        return true;
    }

    return false;
}

bool BcmDsgClassifierSettings::WriteTo( BcmOctetBuffer &octetBuffer ) const
{
    // this type is a mutant.  it should have (3) sub-type codes, but instead
    // gloms (3) value bytes after the parent TL.
    // here we will simply write the (3) binary encoded mutant child V bytes.
	uint8 portUsed = 0;
	uint8 unused = 0;

	uint32 source_ip_adr;
	fIpClassifierSets.SourceIpAddr().Get(source_ip_adr);

	uint32 source_ip_mask;
	fIpClassifierSets.SourceIpMask().Get(source_ip_mask);

	uint32 dest_ip_adr;
	fIpClassifierSets.DestIpAddr().Get(dest_ip_adr);

	if(fIpClassifierSets.DestPortStartSpecified() || fIpClassifierSets.DestPortEndSpecified() )
	{
		portUsed = 1;
	}

    if( !octetBuffer.NetworkAddToEnd( fCmtsClassifierId ) ||
        !octetBuffer.NetworkAddToEnd( fRulePriority ) ||
        !octetBuffer.NetworkAddToEnd( unused ) ||
		!octetBuffer.NetworkAddToEnd( (uint8)fIpClassifierSets.SourceIpAddrSpecified() ) ||
        !octetBuffer.NetworkAddToEnd( (uint8)fIpClassifierSets.SourceIpMaskSpecified() ) ||
        !octetBuffer.NetworkAddToEnd( fIpClassifierSets.DestIpAddrSpecified() ) ||
        !octetBuffer.NetworkAddToEnd( portUsed ) ||
        !octetBuffer.NetworkAddToEnd( source_ip_adr ) ||
        !octetBuffer.NetworkAddToEnd( source_ip_mask ) ||
        !octetBuffer.NetworkAddToEnd( dest_ip_adr ) ||
        !octetBuffer.NetworkAddToEnd( fIpClassifierSets.DestPortStart() ) ||
        !octetBuffer.NetworkAddToEnd( fIpClassifierSets.DestPortEnd()) 		
	  )
    {
        return false;
    }
    return true;
}

