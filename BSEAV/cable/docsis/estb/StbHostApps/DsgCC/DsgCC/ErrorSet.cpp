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

//
// ErrorSet.cpp
//
#include "ErrorSet.h"
#include "OctetBuffer.h"
#include "TLV.h"
#include "TLVCodes.h"
#include <string.h>
#include "MessageLogSettings.h"

BcmErrorSet::BcmErrorSet()
  :
    fErrorType( kMaxErrorTypeLength ),
    fErrorCode( 0 ),
    fErrorStringSpecified( false )
{
    fErrorString[ 0 ] = 0;
}



BcmErrorSet::BcmErrorSet( byte err_type, byte err_code, const char* err_str )
  :
    fErrorType( kMaxErrorTypeLength ),
    fErrorCode( err_code )
{
    fErrorType.AddToEnd(err_type);

    if( err_str )
    {
        strcpy( fErrorString, err_str );
        fErrorStringSpecified = true;
    }
    else
    {
        fErrorString[ 0 ] = 0;
        fErrorStringSpecified = false;
    }
}


BcmErrorSet::BcmErrorSet( byte err_type, byte err_child_type, byte err_code, 
                          const char* err_str )
  :
    fErrorType( kMaxErrorTypeLength ),
    fErrorCode( err_code )
{
    fErrorType.AddToEnd(err_type);
    fErrorType.AddToEnd(err_child_type);

    if( err_str )
    {
        strcpy( fErrorString, err_str );
        fErrorStringSpecified = true;
    }
    else
    {
        fErrorString[ 0 ] = 0;
        fErrorStringSpecified = false;
    }
}


BcmErrorSet::BcmErrorSet( const BcmErrorSet& rhs )
  :
    fErrorType( rhs.fErrorType ),
    fErrorCode( rhs.fErrorCode ),
    fErrorStringSpecified( rhs.fErrorStringSpecified )
{
    if( fErrorStringSpecified )
    {
        strcpy( fErrorString, rhs.fErrorString );
    }
    else
    {
        fErrorString[ 0 ] = 0;
    }
}


BcmErrorSet& BcmErrorSet::operator=( const
  BcmErrorSet& rhs )
{
    if( this == &rhs )
    {
        // special case: assignment to self
        return *this;
    }

    fErrorType = rhs.fErrorType;
    fErrorCode = rhs.fErrorCode;
    fErrorStringSpecified = rhs.fErrorStringSpecified;

    if( fErrorStringSpecified )
    {
        strcpy( fErrorString, rhs.fErrorString );
    }
    else
    {
        fErrorString[ 0 ] = 0;
    }

    return *this;
}

bool BcmErrorSet::operator<( const BcmErrorSet& rhs ) const
{
    if( fErrorType < rhs.fErrorType )
    {
        return true;
    }
    else if( rhs.fErrorType < fErrorType )
    {
        return false;
    }
    else if( fErrorCode < fErrorCode )
    {
        return true;
    }
    else
    {
        return false;
    }    
}


ostream& BcmErrorSet::Print( ostream& ostrm ) const
{

//#warning "BcmErrorSet COMPILER ERROR FIX"

#if 0
#if !(BCM_REDUCED_IMAGE_SIZE)

    ostrm << "\nError Param Set:"
          << "\n  appendix C error type: " << fErrorType
          << "\n             error code: " << (void *) fErrorCode
          << " (" << ConfirmationCodeName( fErrorCode ) << ") "

          << "\n           error string: "
          << ( fErrorStringSpecified ? (char *)fErrorString : "none" )

          << endl;
#endif
#endif

    return ostrm;
}





byte BcmErrorSet::ParseFrom( const BcmOctetBuffer &octetBuffer,
    unsigned int &offset, BcmMessageLogSettings* log_sets )
{
    // input for TLV parser.  
    TLVTable tlvTable[] =
    {
        { kErrorType,   kUnknownLength, kTLV_OctetBuffer, &fErrorType,   false, false, (char *) "setting Type w/error", NULL },
        { kErrorCode,                1, kTLV_uint8,       &fErrorCode,   false, false, (char *) "error code", NULL },
        { kErrorString, kUnknownLength, kTLV_string,      &fErrorString, false, false, (char *) "error string", &fErrorStringSpecified },

        // This must always be the last one in the table.
        kEndOfTLVTable
    };

    // Parse the TLVs until end of input or special case exit.
    TLVParserResult result = ParseTLVs(octetBuffer, offset, tlvTable, log_sets);

    // Check for an error during parsing.  
    if( result >= kTLVParserError )
    {
        // Log this to the debugger...
        return kRejMsgSyntaxError;
    }

    return kConfOk;
}


bool BcmErrorSet::WriteTo( BcmOctetBuffer &octetBuffer, 
    BcmMessageLogSettings* log_sets ) const
{
    if( !GenerateTLV( octetBuffer, kErrorType, fErrorType, log_sets, 
            "setting Type w/error" ) ||
        !GenerateTLV( octetBuffer, kErrorCode, fErrorCode, log_sets, 
            "error code" ) )
    {
        return false;
    }

    if( fErrorStringSpecified )
    {
        if( !GenerateTLV( octetBuffer, kErrorString, fErrorString, log_sets, 
            "error string" ) )
        {
            return false;
        }
    }

    return true;
}

