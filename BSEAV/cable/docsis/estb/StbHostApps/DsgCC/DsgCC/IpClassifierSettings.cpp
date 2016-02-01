//****************************************************************************
//
//  Copyright (c) 1999, 2000, 2001  Broadcom Corporation
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
//  $Id$
//
//  Filename:       ClassifierSettings.cpp
//  Author:         Cliff Danielson
//  Creation Date:  {unknown}
//
//****************************************************************************
//  Description:
//
//****************************************************************************

//********************** Include Files ***************************************

#include "ClassifierSettings.h"
#include "OctetBuffer.h"
#include "TLV.h"
#include "TLVCodes.h"
#include "MessageLog.h"
#include <string.h>
#include "MessageLogSettings.h"

#include "DcdTlvCodes.h"
//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Local Functions *************************************
// Helper function that creates/adds a new Error Set object to the specified
// Error Set list with the specified TLV type code, conf code, and optional
// string.  It also logs meaningful information to the console.  
//
// This function is only used if errors are encountered in objects contained 
// by BcmClassifierSettings, such as BcmEthernetClassifierSettings, 
// BcmIpClassifierSettings and Bcm8021PqClassifierSettings.  Member function
// BcmClassifierSettings::AddErrorSet() is called for errors with subtypes
// which are not nested objects.
//
// It is only called during parsing of the classifier TLVs and validation of
// the values.
//
// Parameters:
//      pMsgLogSettings - used for logging to the console.
//      errorSetList - the Error Set list that the error set to to be added to.
//      typeCode - the parent TLV type code that is being errored.
//      childTypeCode - if non-0, this is the child TLV type code that is being
//                      errored.
//      pErrorString - optional error string giving more info about the error.
//
// Returns:
//      The conf code that was specified.
//
static byte AddErrorSetForContainedObject(BcmMessageLogSettings *pMsgLogSettings,
    ErrorSetList &errorSetList, byte typeCode, byte childTypeCode, byte confCode, 
    const char *pErrorString = NULL);
//********************** Class Method Implementations ************************


BcmIpClassifierSettings::BcmIpClassifierSettings()
  :
    fSourceIpAddr(),
    fSourceIpMask( 255, 255, 255, 255 ),
    fDestIpAddr(),
    fDestIpMask( 255, 255, 255, 255 )
{
    fTosRangeAndMaskSpecified = fProtocolTypeSpecified = fSourceIpAddrSpecified
      = fSourceIpMaskSpecified = fDestIpAddrSpecified = fDestIpMaskSpecified
      = fSourcePortStartSpecified = fSourcePortEndSpecified
      = fDestPortStartSpecified = fDestPortEndSpecified
      = false;

    memset( fTosRangeAndMask, 0, 3 );
    fProtocolType = 0;
    fSourcePortStart = 0;
    fSourcePortEnd = 65535;
    fDestPortStart = 0;
    fDestPortEnd = 65535;
}


BcmIpClassifierSettings::BcmIpClassifierSettings( 
    const BcmIpClassifierSettings& rhs )
  :
    fTosRangeAndMaskSpecified( rhs.fTosRangeAndMaskSpecified ),
    fProtocolType( rhs.fProtocolType ),
    fProtocolTypeSpecified( rhs.fProtocolTypeSpecified ),
    fSourceIpAddr( rhs.fSourceIpAddr ),
    fSourceIpAddrSpecified( rhs.fSourceIpAddrSpecified ),
    fSourceIpMask( rhs.fSourceIpMask ),
    fSourceIpMaskSpecified( rhs.fSourceIpMaskSpecified ),
    fDestIpAddr( rhs.fDestIpAddr ),
    fDestIpAddrSpecified( rhs.fDestIpAddrSpecified ),
    fDestIpMask( rhs.fDestIpMask ),
    fDestIpMaskSpecified( rhs.fDestIpMaskSpecified ),
    fSourcePortStart( rhs.fSourcePortStart ),
    fSourcePortStartSpecified( rhs.fSourcePortStartSpecified ),
    fSourcePortEnd( rhs.fSourcePortEnd ),
    fSourcePortEndSpecified( rhs.fSourcePortEndSpecified ),
    fDestPortStart( rhs.fDestPortStart ),
    fDestPortStartSpecified( rhs.fDestPortStartSpecified ),
    fDestPortEnd( rhs.fDestPortEnd ),
    fDestPortEndSpecified( rhs.fDestPortEndSpecified )
{
    memcpy( fTosRangeAndMask, rhs.fTosRangeAndMask, 3 );
}


BcmIpClassifierSettings& BcmIpClassifierSettings::operator=( 
    const BcmIpClassifierSettings& rhs )
{
    // special case: assignment to self can skip the work.
    if (this != &rhs)
    {
        memcpy( fTosRangeAndMask, rhs.fTosRangeAndMask, 3 );
        fTosRangeAndMaskSpecified = rhs.fTosRangeAndMaskSpecified;
        fProtocolType = rhs.fProtocolType;
        fProtocolTypeSpecified = rhs.fProtocolTypeSpecified;
        fSourceIpAddr = rhs.fSourceIpAddr;
        fSourceIpAddrSpecified = rhs.fSourceIpAddrSpecified;
        fSourceIpMask = rhs.fSourceIpMask;
        fSourceIpMaskSpecified = rhs.fSourceIpMaskSpecified;
        fDestIpAddr = rhs.fDestIpAddr;
        fDestIpAddrSpecified = rhs.fDestIpAddrSpecified;
        fDestIpMask = rhs.fDestIpMask;
        fDestIpMaskSpecified = rhs.fDestIpMaskSpecified;
        fSourcePortStart = rhs.fSourcePortStart;
        fSourcePortStartSpecified = rhs.fSourcePortStartSpecified;
        fSourcePortEnd = rhs.fSourcePortEnd;
        fSourcePortEndSpecified = rhs.fSourcePortEndSpecified;
        fDestPortStart = rhs.fDestPortStart;
        fDestPortStartSpecified = rhs.fDestPortStartSpecified;
        fDestPortEnd = rhs.fDestPortEnd;
        fDestPortEndSpecified = rhs.fDestPortEndSpecified;
    }

    return *this;
}


bool BcmIpClassifierSettings::operator==(
  const BcmIpClassifierSettings& rhs ) const
{
    // Note - only compare the values if they were specified.  If they weren't
    // specified, then the actual values are don't cares.
    if ((fTosRangeAndMaskSpecified != rhs.fTosRangeAndMaskSpecified) ||
        (fProtocolTypeSpecified != rhs.fProtocolTypeSpecified) ||
        (fSourceIpAddrSpecified != rhs.fSourceIpAddrSpecified) ||
        (fSourceIpMaskSpecified != rhs.fSourceIpMaskSpecified) ||
        (fDestIpAddrSpecified != rhs.fDestIpAddrSpecified) ||
        (fDestIpMaskSpecified != rhs.fDestIpMaskSpecified) ||
        (fSourcePortStartSpecified != rhs.fSourcePortStartSpecified) ||
        (fSourcePortEndSpecified != rhs.fSourcePortEndSpecified) ||
        (fDestPortStartSpecified != rhs.fDestPortStartSpecified) ||
        (fDestPortEndSpecified != rhs.fDestPortEndSpecified))
    {
        return false;
    }

    if (fTosRangeAndMaskSpecified &&
        ((fTosRangeAndMask[ 0 ] != rhs.fTosRangeAndMask[ 0 ]) ||
         (fTosRangeAndMask[ 1 ] != rhs.fTosRangeAndMask[ 1 ]) ||
         (fTosRangeAndMask[ 2 ] != rhs.fTosRangeAndMask[ 2 ])))
    {
        return false;
    }

    if (fProtocolTypeSpecified &&
        (fProtocolType != rhs.fProtocolType))
    {
        return false;
    }

    if (fSourceIpAddrSpecified &&
        (fSourceIpAddr != rhs.fSourceIpAddr))
    {
        return false;
    }

    if (fSourceIpMaskSpecified &&
        (fSourceIpMask != rhs.fSourceIpMask))
    {
        return false;
    }

    if (fDestIpAddrSpecified &&
        (fDestIpAddr != rhs.fDestIpAddr))
    {
        return false;
    }

    if (fDestIpMaskSpecified &&
        (fDestIpMask != rhs.fDestIpMask))
    {
        return false;
    }

    if (fSourcePortStartSpecified &&
        (fSourcePortStart != rhs.fSourcePortStart))
    {
        return false;
    }

    if (fSourcePortEndSpecified &&
        (fSourcePortEnd != rhs.fSourcePortEnd))
    {
        return false;
    }

    if (fDestPortStartSpecified &&
        (fDestPortStart != rhs.fDestPortStart))
    {
        return false;
    }

    if (fDestPortEndSpecified &&
        (fDestPortEnd != rhs.fDestPortEnd))
    {
        return false;
    }

    return true;
}

bool BcmIpClassifierSettings::operator!=(
  const BcmIpClassifierSettings& rhs ) const
{
    // Use the == operator.
    return (!(*this == rhs));
}

void BcmIpClassifierSettings::Reset()
{
    fTosRangeAndMaskSpecified = fProtocolTypeSpecified = fSourceIpAddrSpecified
      = fSourceIpMaskSpecified = fDestIpAddrSpecified = fDestIpMaskSpecified
      = fSourcePortStartSpecified = fSourcePortEndSpecified
      = fDestPortStartSpecified = fDestPortEndSpecified
      = false;
    memset( fTosRangeAndMask, 0, 3 );
    fProtocolType = 0;
    fSourceIpAddr.reset();
    BcmIpAddress def_mask( 255, 255, 255, 255 );
    fSourceIpMask = def_mask;
    fDestIpAddr.reset();
    fDestIpMask = def_mask;
    fSourcePortStart = 0;
    fSourcePortEnd = 65535;
    fDestPortStart = 0;
    fDestPortEnd = 65535;
}



ostream& BcmIpClassifierSettings::Print( ostream& ostrm ) const
{
    #if !(BCM_REDUCED_IMAGE_SIZE)

    // print settings specific to this class.
    // presumably some kids are present.
    ostrm 
        << "\n     ToS range and mask: " << inHex( fTosRangeAndMask[ 0 ] )
            << " " << inHex( fTosRangeAndMask[ 1 ] )
            << " " << inHex( fTosRangeAndMask[ 2 ] )
            << ( fTosRangeAndMaskSpecified ? spec : no_spec )

        << "\n          protocol type: "
            << inHex(fProtocolType) << " (" << fProtocolType << ")"
            << ( fProtocolTypeSpecified ? spec : no_spec )

        << "\n      source IP address: " << fSourceIpAddr
            << ( fSourceIpAddrSpecified ? spec : no_spec )

        << "\n         source IP mask: " << fSourceIpMask
            << ( fSourceIpMaskSpecified ? spec : no_spec )

        << "\n        dest IP address: " << fDestIpAddr
            << ( fDestIpAddrSpecified ? spec : no_spec )

        << "\n           dest IP mask: " << fDestIpMask
            << ( fDestIpMaskSpecified ? spec : no_spec )

        << "\n      source port start: " 
            << inHex(fSourcePortStart) << " (" << fSourcePortStart << ")"
            << ( fSourcePortStartSpecified ? spec : no_spec )

        << "\n        source port end: " 
            << inHex(fSourcePortEnd) << " (" << fSourcePortEnd << ")"
            << ( fSourcePortEndSpecified ? spec : no_spec )

        << "\n        dest port start: " 
            << inHex(fDestPortStart) << " (" << fDestPortStart << ")"
            << ( fDestPortStartSpecified ? spec : no_spec )

        << "\n          dest port end: " 
            << inHex(fDestPortEnd) << " (" << fDestPortEnd << ")"
            << ( fDestPortEndSpecified ? spec : no_spec )

        << endl;

    #endif
    
    return ostrm;
}

bool BcmIpClassifierSettings::WriteTo( BcmOctetBuffer &octetBuffer, 
    BcmMessageLogSettings* log_sets ) const
{
    if( fTosRangeAndMaskSpecified )
    {
        // fTosRangeAndMask is specified.  print it.
        // this data member is a byte array that is always (3) bytes
        // in length.
        if( !GenerateTLV( octetBuffer, kTosRangeAndMask, fTosRangeAndMask, 3, log_sets,
            "ToS range and mask" ) )
        {
            return false;
        }
    }

    if( fProtocolTypeSpecified )
    {
        // fProtocolType is specified.  print it.
        if( !GenerateTLV( octetBuffer, kProtocolType, fProtocolType, log_sets,
            "protocol type" ) )
        {
            return false;
        }
    }

    if( fSourceIpAddrSpecified )
    {
        // fSourceIpAddr is specified.  print it.
        if( !GenerateTLV( octetBuffer, kSourceIpAddr, fSourceIpAddr, log_sets,
            "source IP address" ) )
        {
            return false;
        }
    }

    if( fSourceIpMaskSpecified )
    {
        // fSourceIpMask is specified.  print it.
        if( !GenerateTLV( octetBuffer, kSourceIpMask, fSourceIpMask, log_sets,
            "source IP mask" ) )
        {
            return false;
        }
    }

    if( fDestIpAddrSpecified )
    {
        // fDestIpAddr is specified.  print it.
        if( !GenerateTLV( octetBuffer, kDestIpAddr, fDestIpAddr, log_sets,
            "dest IP address" ) )
        {
            return false;
        }
    }

    if( fDestIpMaskSpecified )
    {
        // fDestIpMask is specified.  print it.
        if( !GenerateTLV( octetBuffer, kDestIpMask, fDestIpMask, log_sets,
            "dest IP mask" ) )
        {
            return false;
        }
    }

    if( fSourcePortStartSpecified )
    {
        // fSourcePortStart is specified.  print it.
        if( !GenerateTLV( octetBuffer, kSourcePortStart, fSourcePortStart, log_sets,
            "source port start" ) )
        {
            return false;
        }
    }

    if( fSourcePortEndSpecified )
    {
        // fSourcePortEnd is specified.  print it.
        if( !GenerateTLV( octetBuffer, kSourcePortEnd, fSourcePortEnd, log_sets,
            "source port end" ) )
        {
            return false;
        }
    }

    if( fDestPortStartSpecified )
    {
        // fDestPortStart is specified.  print it.
        if( !GenerateTLV( octetBuffer, kDestPortStart, fDestPortStart, log_sets,
            "dest port start" ) )
        {
            return false;
        }
    }

    if( fDestPortEndSpecified )
    {
        // fDestPortEnd is specified.  print it.
        if( !GenerateTLV( octetBuffer, kDestPortEnd, fDestPortEnd, log_sets,
            "dest port end" ) )
        {
            return false;
        }
    }

    return true;
}


byte BcmIpClassifierSettings::ParseFrom( const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, ErrorSetList& err_set_list,
    BcmMessageLogSettings* log_sets, bool strict_checking )
{
    // input for TLV parser.  
    TLVTable tlvTable[] =
    {
        { kTosRangeAndMask, 3, kTLV_FixedByteArray, &fTosRangeAndMask, false, false, (char *) "ToS range and mask", &fTosRangeAndMaskSpecified },
        { kProtocolType,    2, kTLV_uint16,         &fProtocolType,    false, false, (char *) "protocol type",      &fProtocolTypeSpecified },
        { kSourceIpAddr,    4, kTLV_IpAddress,      &fSourceIpAddr,    false, false, (char *) "source IP address",  &fSourceIpAddrSpecified },
        { kSourceIpMask,    4, kTLV_IpAddress,      &fSourceIpMask,    false, false, (char *) "source IP mask",     &fSourceIpMaskSpecified },
        { kDestIpAddr,      4, kTLV_IpAddress,      &fDestIpAddr,      false, false, (char *) "dest IP address",    &fDestIpAddrSpecified },
        { kDestIpMask,      4, kTLV_IpAddress,      &fDestIpMask,      false, false, (char *) "dest IP mask",       &fDestIpMaskSpecified },
        { kSourcePortStart, 2, kTLV_uint16,         &fSourcePortStart, false, false, (char *) "source port start",  &fSourcePortStartSpecified },
        { kSourcePortEnd,   2, kTLV_uint16,         &fSourcePortEnd,   false, false, (char *) "source port end",    &fSourcePortEndSpecified },
        { kDestPortStart,   2, kTLV_uint16,         &fDestPortStart,   false, false, (char *) "dest port start",    &fDestPortStartSpecified },
        { kDestPortEnd,     2, kTLV_uint16,         &fDestPortEnd,     false, false, (char *) "dest port end",      &fDestPortEndSpecified },

        // This must always be the last one in the table.
        kEndOfTLVTable
    };

    // Parse the TLVs until end of input or special case exit.
    TLVParserResult result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);

    // Check for an error during parsing.  
    if( result >= kTLVParserError )
    {
        if (log_sets)
        {
            gErrorMsg(*log_sets, "ParseFrom") << "Failed to parse IP Classifier TLVs!  Message syntax error" << endl;
        }

        return kRejMsgSyntaxError;
    }

    if( !strict_checking )
    {
        // special case: client has specified that parsed values are NOT to
        // be validated w/strict error checking rules.
        return kConfOk;
    }

    // Do range checking on the values.
    if (fTosRangeAndMaskSpecified &&
        (fTosRangeAndMask[0] > fTosRangeAndMask[1]))
    {
        return AddErrorSetForContainedObject(log_sets, err_set_list, 
                                             kIpClassifierSets, 
                                             kTosRangeAndMask, 
                                             kRejUnrecognizedConfigSetting, 
                                            "ToS low > ToS high");
    }
    if (fProtocolTypeSpecified &&
        (fProtocolType > 257))
    {
        return AddErrorSetForContainedObject(log_sets, err_set_list, 
                                             kIpClassifierSets, 
                                             kProtocolType, 
                                             kRejUnrecognizedConfigSetting, 
                                             "IP Protocol value > 257");
    }
    if (fSourcePortStart > fSourcePortEnd)
    {
        return AddErrorSetForContainedObject(log_sets, err_set_list, 
                                             kIpClassifierSets, 
                                             kSourcePortStart, 
                                             kRejUnrecognizedConfigSetting, 
                                             "Source Port Start > End");
    }
    if (fDestPortStart > fDestPortEnd)
    {
        return AddErrorSetForContainedObject(log_sets, err_set_list, 
                                             kIpClassifierSets, 
                                             kDestPortStart, 
                                             kRejUnrecognizedConfigSetting, 
                                             "Destination Port Start > End");
    }

    // NOTE:  It is assumed that some higher-layer entity is going to verify
    // that the required things are present, and that the combination of
    // settings that are present is allowed...

    return kConfOk;
}


bool BcmIpClassifierSettings::AnythingSpecified(void) const
{
    if (fTosRangeAndMaskSpecified ||
        fProtocolTypeSpecified ||
        fSourceIpAddrSpecified ||
        fSourceIpMaskSpecified ||
        fDestIpAddrSpecified ||
        fDestIpMaskSpecified ||
        fSourcePortStartSpecified ||
        fSourcePortEndSpecified ||
        fDestPortStartSpecified ||
        fDestPortEndSpecified)
    {
        return true;
    }

    return false;
}

static byte AddErrorSetForContainedObject( BcmMessageLogSettings *pMsgLogSettings,
    ErrorSetList &errorSetList, byte typeCode, 
    byte childTypeCode, byte confCode, const char *pErrorString )
{
    gErrorMsg(*pMsgLogSettings, "ParseFrom")
        << "While parsing/validating TLVs:\n";

    if (childTypeCode == 0)
    {
        BcmErrorSet errorSet(typeCode, confCode, pErrorString);
        errorSetList.push_back( errorSet );

        gErrorMsgNoFields(*pMsgLogSettings) << errorSet;
    }
    else
    {
        BcmErrorSet errorSet(typeCode, childTypeCode, confCode, pErrorString);
        errorSetList.push_back( errorSet );

        gErrorMsgNoFields(*pMsgLogSettings) << errorSet;
    }

    return confCode;
}

