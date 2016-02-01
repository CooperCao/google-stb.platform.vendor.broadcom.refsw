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
//  $Id: Type43VendorSettings.cpp 1.4 2004/09/23 13:24:31Z dpullen Release $
//
//  Filename:       Type43VendorSettings.cpp
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

//********************** Include Files ***************************************

// My api and definitions...
#include "Type43VendorSettings.h"

#include "MessageLog.h"

#include "TLV.h"
#include "TLVCodes.h"

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 4)
#include <iomanip>
#else
#include <iomanip.h>
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


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
BcmType43VendorSettings::BcmType43VendorSettings(uint8 vendorId0,
                                                 uint8 vendorId1,
                                                 uint8 vendorId2) :
    // The maximum amount of Vendor Specific data that can be stored is 255
    // (the parent length is a single byte, which can have a value of 0..255),
    // minus the length of the Vendor Id TLV, which is 5 bytes.  Making the
    // buffer exactly this size gives the client some feedback early on as to
    // whether or not there is too much data.
    fVendorSpecificData(255 - 5)
{
    // Copy the vendor id passed in (may be defaulted to 0's).
    fVendorId[0] = vendorId0;
    fVendorId[1] = vendorId1;
    fVendorId[2] = vendorId2;
}


// Copy Constructor.  Initializes the state of this object to match that
// of the instance passed in.
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:  N/A
//
BcmType43VendorSettings::BcmType43VendorSettings(const BcmType43VendorSettings &otherInstance) :
    // The maximum amount of Vendor Specific data that can be stored is 255
    // (the parent length is a single byte, which can have a value of 0..255),
    // minus the length of the Vendor Id TLV, which is 5 bytes.  Making the
    // buffer exactly this size gives the client some feedback early on as to
    // whether or not there is too much data.
    fVendorSpecificData(255 - 5)
{
    // Usually, you can just call the assignment operator to do the work.
    *this = otherInstance;
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmType43VendorSettings::~BcmType43VendorSettings()
{
    // Nothing to do.
}


// Assignment operator.  Copies the state of the instance passed in so that
// this object is "identical".
//
// Parameters:
//      otherInstance - reference to the object to copy.
//
// Returns:
//      A reference to "this" so that operator = can be chained.
//
BcmType43VendorSettings & BcmType43VendorSettings::operator = (const BcmType43VendorSettings &otherInstance)
{
    // Copy protected and private members of otherInstance to my members.
    // Create memory (using new) if necessary; don't copy any of it's memory
    // pointers - copy the contents of the memory it points to.
    fVendorSpecificData = otherInstance.fVendorSpecificData;

    fVendorId[0] = otherInstance.fVendorId[0];
    fVendorId[1] = otherInstance.fVendorId[1];
    fVendorId[2] = otherInstance.fVendorId[2];

    return *this;
}


// Comparison operator.
//
// Parameters:
//      otherInstance - reference to the object to compare against.
//
// Returns:
//      1 if the objects are equivalent.
//      0 of the object don't compare.
//
int BcmType43VendorSettings::operator == (const BcmType43VendorSettings &otherInstance) const
{
    if ((fVendorId[0] == otherInstance.fVendorId[0]) &&
        (fVendorId[1] == otherInstance.fVendorId[1]) &&
        (fVendorId[2] == otherInstance.fVendorId[2]) &&
        (fVendorSpecificData == otherInstance.fVendorSpecificData))
    {
        // If we got this far, then we compared.
        return 1;
    }

    return 0;
}


// Resets the object to the default state (empties the buffer of data, but
// leaves the Vendor Id alone).
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void BcmType43VendorSettings::Reset(void)
{
    fVendorSpecificData.Empty();
}


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
void BcmType43VendorSettings::VendorId(uint8 &vendorId0, uint8 &vendorId1,
                                       uint8 &vendorId2) const
{
    vendorId0 = fVendorId[0];
    vendorId1 = fVendorId[1];
    vendorId2 = fVendorId[2];
}


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
bool BcmType43VendorSettings::VendorSpecificData(const BcmOctetBuffer &vendorData)
{
    // Check the amount of data that is being stored; if too much, then bail.
    if (vendorData.BytesUsed() > fVendorSpecificData.BufferSize())
    {
        return false;
    }

    // Clear the existing data, and append the new data.
    fVendorSpecificData.Empty();
    return fVendorSpecificData.Append(vendorData);
}


// Accessor to read the Vendor Specific data that was previously stored or
// parsed.
//
// Parameters:  None.
//
// Returns:
//      A reference to the vendor specific data.
//
const BcmOctetBuffer &BcmType43VendorSettings::VendorSpecificData(void) const
{
    return fVendorSpecificData;
}


// Does formatted printing to the specified ostream.
//
// Parameters:
//      ostrm - the ostream to print to.
//
// Returns:
//      A reference to the ostream.
//
ostream &BcmType43VendorSettings::Print(ostream &ostrm) const
{
    #if (!BCM_REDUCED_IMAGE_SIZE)
    {
        ostrm
            << "Type 43 Vendor Settings:\n\n"

            << setw(10) << "Vendor Id: " << hex << setw(2) << (int) fVendorId[0] << ':' 
                                                << setw(2) << (int) fVendorId[1] << ':' 
                                                << setw(2) << (int) fVendorId[2] << dec << '\n'

            << setw(9) << "Vendor Data: (" << fVendorSpecificData.BytesUsed() 
                                           << " of " << fVendorSpecificData.BufferSize() 
                                           << " bytes)\n" 
                                           << fVendorSpecificData;
    }
    #endif

    return ostrm;
}


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
bool BcmType43VendorSettings::WriteTo(BcmOctetBuffer &octetBuffer,
                                      BcmMessageLogSettings *pMessageLogSettings) const
{
    // First, I must write the Vendor Id TLV; this must appear first in the
    // list.
    if (!GenerateTLV(octetBuffer, kVendorId, fVendorId, 3, pMessageLogSettings,
                     "Vendor Id"))
    {
        return false;
    }

    // The remainder of the data is assumed to be properly encoded (TLV or
    // otherwise) in the octet buffer, so I just need to append it.
    return octetBuffer.Append(fVendorSpecificData);
}


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
byte BcmType43VendorSettings::ParseFrom(const BcmOctetBuffer &octetBuffer,
                                        unsigned int &offset,
                                        BcmMessageLogSettings *pMessageLogSettings)
{
    // This isn't really a good candidate for doing the generic TLV parsing
    // stuff.  The spec says that the first TLV must be the Vendor Id; then
    // we store the rest of the data in raw form.
    //
    // The TLV Parser engine would allow the Vendor Id to appear in any order,
    // and it wouldn't be able to store the rest of the data without trying to
    // do some sort of parent level TLV parsing.

    uint8 typeCode, length;

    // Read the T and L parts of the TLV.
    if (!octetBuffer.NetworkRead(offset, typeCode) ||
        !octetBuffer.NetworkRead(offset, length))
    {
        if (pMessageLogSettings != NULL)
        {
            gErrorMsg(*pMessageLogSettings, "ParseFrom(Type43 Vendor Specific)")
                << "Buffer contains partial TLV!" << endl;
        }

        return kRejMsgSyntaxError;
    }

    // Make sure they match what we are expecting.
    if ((typeCode != kVendorId) || (length != 3))
    {
        if (pMessageLogSettings != NULL)
        {
            gErrorMsg(*pMessageLogSettings, "ParseFrom(Type43 Vendor Specific)")
                << "First TLV isn't Vendor Id!" << endl;
        }

        return kRejRequiredParamNotPresent;
    }

    // Try to parse the Vendor Id bytes.
    if (!octetBuffer.Read(offset, fVendorId, 3))
    {
        if (pMessageLogSettings != NULL)
        {
            gErrorMsg(*pMessageLogSettings, "ParseFrom(Type43 Vendor Specific)")
                << "Buffer contains partial TLV!  Failed to read Vendor Id bytes." << endl;
        }

        return kRejMsgSyntaxError;
    }

    // If that worked, then the remainder of the buffer contains data that just
    // needs to be stored.
    fVendorSpecificData.Empty();
    if (!octetBuffer.Read(offset, fVendorSpecificData, 
                          octetBuffer.BytesUsed() - offset))
    {
        if (pMessageLogSettings != NULL)
        {
            gErrorMsg(*pMessageLogSettings, "ParseFrom(Type43 Vendor Specific)")
                << "Failed to read remaining Vendor Specific data!" << endl;
        }

        return kRejMsgSyntaxError;
    }

    return kConfOk;
}


