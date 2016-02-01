//****************************************************************************
//
// Copyright (c) 2000-2010 Broadcom Corporation
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
//  $Id: ErrorSet.h 1.8 2001/07/09 22:12:34Z cliffd Release $
//

#ifndef ERRORSET_H
#define ERRORSET_H

#include "typedefs.h"
#include "OctetBuffer.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#else
#include <iomanip.h>
#endif

class BcmMessageLogSettings;

/**# :[Description = "Class BcmErrorSet models the 'Error Parameter Set'
encoding used in DOCSIS v1.1 QoS flow, classifier and PHS settings."]
[Level = Analysis] */
class BcmErrorSet
{
public:
    // begin functions required to be param for STL containers...
    //

    // Default constructor.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmErrorSet();

    // Constructor.  This one is meant to be used for Flow and PHS settings.
    //
    // Parameters:
    //      err_type - the type code from the TLV that is being errored.
    //      err_code - a conf code from appendix C describing the error.
    //      err_str - optional string explaining the error.
    //
	BcmErrorSet( byte err_type, byte err_code, const char* err_str = 0 );

    // Alternate constructor; for Classifier error encodings, there can be
    // multiple levels of type codes from the TLV that are being errored,
    // e.g. 9.2 is for the IP Protocol TLV.  In this case, you would specify
    // 9 as the err_type parameter, and 2 as the err_child_type parameter.
    //
    // Note that, technically, the spec allows any number of type codes, but
    // realistically you can only specify 2, since that's the number of levels
    // there are.  If they change this in the future, then we will need to
    // change the API to handle this.
    //
    // Parameters:
    //      err_type - the type code from the TLV that is being errored.
    //      err_child_type - the child type code from the parent type TLV that
    //                       is being errored.
    //      err_code - a conf code from appendix C describing the error.
    //      err_str - optional string explaining the error.
    //
    BcmErrorSet( byte err_type, byte err_child_type, byte err_code, const char* err_str = 0 );

    // Copy constructor.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmErrorSet( const BcmErrorSet& rhs );

    // Destructor.
    //
    // Note:
    //      Needed to be type param for STL container.
    //
    ~BcmErrorSet() {}

    // Assignment operator.
    //
    // Parameters:
    //      rhs - source object, a.k.a. right hand side.
    //
    // Returns:
    //      reference to this object
    //
    // Note:
    //      Needed to be type param for STL container.
    //
	BcmErrorSet& operator=( const BcmErrorSet& rhs );

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
	bool operator<( const BcmErrorSet& rhs ) const;

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
	bool operator==( const BcmErrorSet& rhs ) const
	{
	    return ((fErrorType == rhs.fErrorType)
            && (fErrorCode == rhs.fErrorCode)
            && (fErrorStringSpecified == rhs.fErrorStringSpecified)
            && (fErrorString == rhs.fErrorString));
	}
    // end functions required to be param for STL containers...


    // Print formatted object contents to specified ostream.
    //
    // Parameters:
    //      ostream& ostrm - reference to the destination output stream.
    //
    // Returns:
    //      reference to the destination output stream.
    //
    ostream& Print( ostream& ostrm ) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //
    //      offset - offset into input buffer at which to begin parsing.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      "Confirmation code" result, as described in DOCSIS RFI v1.1-IO5
    //      spec App C.4.  A result == 0 is success.  Non-zero result is an
    //      error.
    //
    byte ParseFrom( const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
        BcmMessageLogSettings* log_sets = NULL );

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 26 specific sub-type TLV
    // attributes, but not the parent type 26 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //
    //      log_sets - pointer to BcmMessageLogSettings object which
    //          specifies debug printing options.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
    bool WriteTo( BcmOctetBuffer &octetBuffer, BcmMessageLogSettings* log_sets = NULL ) const;

    // accessor for private fErrorCode member
    inline byte ErrorCode () const { return fErrorCode; }

//private:

    // type code of TLV setting which is in error.  Type x.1   no default.  
    // assume 0, which is NOT a valid type code.  value must be specified!
    // In most cases, this will be a single byte, but in some cases it can be
    // variable length (for Classifiers).
    //
    // For now, we will assume that this can only be 2 bytes at most.
    BcmOctetBuffer fErrorType;

    enum
    {
        kMaxErrorTypeLength = 2
    };

    // error code from App. C.4.  Type x.2   default == 0 == ok (which 
    // is incorrect for an error encoding!)  non-zero value must be specified!
    byte fErrorCode;

    // error message string.  Type x.3  optional.
    char fErrorString[ 250 ];
    bool fErrorStringSpecified;
};

inline ostream& operator<<( ostream& ostrm, const BcmErrorSet& rhs )
{
    return rhs.Print( ostrm );
}

#endif

