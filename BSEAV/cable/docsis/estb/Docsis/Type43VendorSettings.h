//****************************************************************************
//
// Copyright (c) 2001-2010 Broadcom Corporation
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
//  $Id: Type43VendorSettings.h 1.4 2001/11/08 00:33:52Z cliffd Release $
//
//  Filename:       Type43VendorSettings.h
//  Author:         David Pullen
//  Creation Date:  September 20, 2001
//
//****************************************************************************
//  Description:
//      This class wraps the Type 43 TLVs that you can encounter in various
//      settings in DOCSIS (Classifiers, PHS rules, Service Flows, etc).  It
//      doesn't attempt to interprit the contents of the settings (since this
//      is vendor-specific by definition), but it provides storage for it and
//      helper methods to generate and parse the data in the correct format
//      when commanded to do so.
//
//      The TLV data beyond the required Vendor Id is stored in raw form in an
//      octet buffer.  It is the client's responsibility to generate and/or
//      parse this data and make use of it.
//
//      See section C.1.1.17 of the DOCSIS 1.1 spec (Vendor Specific
//      Information) for more info.
//
//****************************************************************************

#ifndef TYPE43VENDORSETTINGS_H
#define TYPE43VENDORSETTINGS_H

//********************** Include Files ***************************************

#include "OctetBuffer.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iostream>
#include <list>
#else
#include <iostream.h>
#include <list.h>
#endif

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************

class BcmMessageLogSettings;

//********************** Class Declaration ***********************************


class BcmType43VendorSettings
{
public:

    // Initializing/Default Constructor.
    //
    // If you are creating this object to parse settings that were received,
    // then just leave the vendor id bytes defaulted (0's).  They will be
    // overwritten by whatever is parsed from the data.
    //
    // If you are creating this object to send the settings, then you must
    // specify the Vendor Id value to be encoded.  For example, if your MAC
    // Address is 00:10:18:00:11:22, then your Vendor Id is 00:10:18.
    //
    // Parameters:
    //      vendorId0 - byte 0 of the Vendor Id.
    //      vendorId1 - byte 1 of the Vendor Id.
    //      vendorId2 - byte 2 of the Vendor Id.
    //
    // Returns:  N/A
    //
    BcmType43VendorSettings(uint8 vendorId0 = 0, uint8 vendorId1 = 0,
                            uint8 vendorId2 = 0);

    // Copy Constructor.  Initializes the state of this object to match that
    // of the instance passed in.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmType43VendorSettings(const BcmType43VendorSettings &otherInstance);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmType43VendorSettings();

    // Assignment operator.  Copies the state of the instance passed in so that
    // this object is "identical".
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmType43VendorSettings & operator = (const BcmType43VendorSettings &otherInstance);

    // Comparison operator.
    //
    // Parameters:
    //      otherInstance - reference to the object to compare against.
    //
    // Returns:
    //      1 if the objects are equivalent.
    //      0 of the object don't compare.
    //
    int operator == (const BcmType43VendorSettings &otherInstance) const;

    // Inequality operator.
    inline int operator != (const BcmType43VendorSettings &otherInstance) const
    {
        return !(operator == (otherInstance));
    }

    // Less than operator.
    //
    // Parameters:
    //      otherInstance - comparison object, a.k.a. right hand side.
    //
    // Returns:
    //      bool - true if objects are equal, else false.
    //
    // Note:
    //      Some compilers (like Borland C++ v5.02) require operator<
    //      for lists even if functions which require it are never
    //      called.  
    //
	bool operator<( const BcmType43VendorSettings &otherInstance ) const
    {
        // this is a dummy implementation to placate Borland and Watcom!!!
        return false;
    }

    // Resets the object to the default state (empties the buffer of data, but
    // leaves the Vendor Id alone).
    //
    // Parameters:  None.
    //
    // Returns:  Nothing.
    //
    void Reset(void);

    // Accessor for the Vendor Id.
    //
    // Parameters:
    //      vendorId0 - reference to the variable into which byte 0 should be
    //                  written.
    //      vendorId1 - reference to the variable into which byte 1 should be
    //                  written.
    //      vendorId2 - reference to the variable into which byte 2 should be
    //                  written.
    //
    // Returns:  Nothing.
    //
    void VendorId(uint8 &vendorId0, uint8 &vendorId1, uint8 &vendorId2) const;

    // Accessor to store the Vendor Specific data.  When setting new data, any
    // data already in the buffer will be overwritten.  Also, if there is too
    // much data, then the original data is left alone and the method returns
    // false.
    //
    // Parmaeters:
    //      vendorData - the data to be stored.
    //
    // Returns:
    //      true if the data was stored.
    //      false if there is too much data to store.
    //
    bool VendorSpecificData(const BcmOctetBuffer &vendorData);

    // Accessor to read the Vendor Specific data that was previously stored or
    // parsed.
    //
    // Parameters:  None.
    //
    // Returns:
    //      A reference to the vendor specific data.
    //
    const BcmOctetBuffer &VendorSpecificData(void) const;

    // Does formatted printing to the specified ostream.
    //
    // Parameters:
    //      ostrm - the ostream to print to.
    //
    // Returns:
    //      A reference to the ostream.
    //
    virtual ostream &Print(ostream &ostrm) const;

    // This method causes the object to write itself out to the octet buffer in
    // TLV format.  This function writes the type 43 specific sub-type TLV
    // attributes, but not the parent type 43 attribute.  The caller
    // of this function must handle the parent TLV.
    //
    // Parameters:
    //      octetBuffer - the buffer into which the TLVs should be written.
    //      pMessageLogSettings - the message log settings object to be used
    //                            for debug logging.
    //
    // Returns:
    //      true if the TLVs were written correctly.
    //      false if there was a problem (not enough room in the buffer, etc.).
    //
	bool WriteTo(BcmOctetBuffer &octetBuffer,
                 BcmMessageLogSettings *pMessageLogSettings = NULL) const;

	// Load object with the decoded TLV attributes contained in the specified
    // input buffer.
    //
    // Parameters:
    //      octetBuffer - input buffer.
    //      offset - offset into input buffer at which to begin parsing.
    //      pMessageLogSettings - the message log settings object to be used
    //                            for debug logging.
    //
    // Returns:
    //      byte - "Confirmation code" result, as described in DOCSIS RFI 
    //          v1.1-IO5 spec App C.4.  A result == 0 is success.  Non-zero 
    //          result is an error.
    //
    // Note:
    // It is assumed that the parent T and L codes have already been read
    // from the input buffer.  The starting offset for subsequent processing
    // will be the T code of the first sub-type.
    //
    byte ParseFrom(const BcmOctetBuffer &octetBuffer, unsigned int &offset, 
                   BcmMessageLogSettings *pMessageLogSettings = NULL);

protected:

    // The TLV data to be written (or that was parsed).
    BcmOctetBuffer fVendorSpecificData;

private:

    // The Vendor Id specified in the constructor.
    uint8 fVendorId[3];

};


//********************** Global Types ****************************************

// Have to define these here because the compiler needs the whole class
// definition before we can use it.
typedef list<BcmType43VendorSettings> BcmType43VendorSettingsList;
typedef BcmType43VendorSettingsList::iterator BcmType43VendorSettingsListIterator;
typedef BcmType43VendorSettingsList::const_iterator BcmType43VendorSettingsListConstIterator;

//********************** Inline Method Implementations ***********************


inline ostream & operator << (ostream ostrm, const BcmType43VendorSettings &rhs)
{
    return rhs.Print(ostrm);
}


#endif


