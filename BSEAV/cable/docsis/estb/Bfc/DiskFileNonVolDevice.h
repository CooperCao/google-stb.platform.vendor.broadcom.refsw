//****************************************************************************
//
// Copyright © 2007 Broadcom Corporation
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
//  $Id$
//
//  Filename:       DiskFileNonVolDevice.h
//  Author:         David Pullen
//  Creation Date:  Feb 24, 2000
//
//****************************************************************************
//  Description:
//      This is the wrapper object for using a file on disk as the NvRam.  It
//      uses standard C file I/O routines, so it should be relatively portable,
//      so long as your C runtime environment provides this support.
//
//      Note that the settings are independant of the device to which they are
//      being stored, allowing the same application settings to be used on
//      multiple target platforms and devices!
//
//      This class provides the common API, as well as some helper methods, for
//      all of the NonVol Devices.
//
//      All NonVolDevices will have the following 8-byte header layout:
//
//        +--------+----------+--------------------+
//        | Length | Checksum | {Settings data...} |
//        +--------+----------+--------------------+
//             4         4          Length - 8
//
//      The Checksum and Length fields are calculated during Write(), and are
//      stored in network order.
//
//      The Length field tells how many bytes of header and app data are
//      contained.  This field must always come first, and will always be at
//      least 8.
//
//****************************************************************************

#ifndef DISKFILENONVOLDEVICE_H
#define DISKFILENONVOLDEVICE_H

//********************** Include Files ***************************************

// My base class.
#include "NonVolDevice.h"

#include "MessageLog.h"

//********************** Global Types ****************************************

//********************** Global Constants ************************************

//********************** Global Variables ************************************

//********************** Class Declaration ***********************************


class BcmDiskFileNonVolDevice : public BcmNonVolDevice
{
public:

    // Initializing Constructor.  Initializes the state of the object.
    //
    // Parameters:
    //      pDynamicFilename - the name of the file to be read/written for the
    //                         dynamic settings section.
    //      pPermanentFilename - the name of the file to be read/written for the
    //                           permanent settings section.
    //
    // Returns:  N/A
    //
    BcmDiskFileNonVolDevice(const char *pDynamicFilename,
                            const char *pPermanentFilename);

    // Destructor.  Frees up any memory/objects allocated, cleans up internal
    // state.
    //
    // Parameters:  N/A
    //
    // Returns:  N/A
    //
    virtual ~BcmDiskFileNonVolDevice();

protected:

    // This is where the derived class writes the buffer to the device.  Note
    // that the derived class may append 0's to the end of the buffer as
    // needed in order to enforce alignment or size restrictions as required by
    // the hardware.
    //
    // Parameters:
    //      outputBuffer - the buffer that should be written.
    //      section - the section that should be written.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (usually hardware failure).
    //
   virtual bool WriteFile(BcmOctetBuffer &outputBuffer,
						 char *filename);

   // This is where the derived class handles robustness before the buffer
   // is written to the device.
   //
   // Parameters:
   //      outputBuffer - the buffer that should be written.
   //      section      - the section that should be written.
   //
   // Returns:
   //      true if successful.
   //      false if there was a problem (usually hardware failure).
   //
    virtual bool WriteImpl(BcmOctetBuffer &outputBuffer,
                           BcmNonVolSettings::NonVolSection section = BcmNonVolSettings::kDynamicSection);

    // This is where the derived class reads data from the device into the
    // buffer.
    //
    // Parameters:
    //      inputBuffer - the buffer into which the data should be read.
    //      section - the section that should be read.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (usually hardware failure).
    //
    virtual bool ReadFile(BcmOctetBuffer &inputBuffer,
                          char *filename);

    // This is where the derived class handles robustness before the buffer
    // is read from the device.
    // buffer.
    //
    // Parameters:
    //      inputBuffer - the buffer into which the data should be read.
    //      section     - the section that should be read.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (usually hardware failure).
    //
    virtual bool ReadImpl(BcmOctetBuffer &inputBuffer,
                          BcmNonVolSettings::NonVolSection section = BcmNonVolSettings::kDynamicSection);

    // This is where the derived class clears the data from the device.
    //
    // This MUST be implemented by the derived class.
    //
    // Parameters:
    //      section - the section to be cleared.
    //
    // Returns:
    //      true if successful.
    //      false if there was a problem (checksum failed, hardware failure,
    //          the settings could not parse the data, etc.).
    //
    virtual bool ClearDeviceImpl(BcmNonVolSettings::NonVolSection section);

private:

    // The names of the files that were given to me in the constructor.
    char *pfDynamicFilename;
    char *pfPermanentFilename;
    char *pfDynamicFilenameBackup;
    char *pfPermanentFilenameBackup;

private:

    // Default Constructor.  Not supported.
    //
    // Parameters:  None.
    //
    // Returns:  N/A
    //
    BcmDiskFileNonVolDevice(void);

    // Copy Constructor.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:  N/A
    //
    BcmDiskFileNonVolDevice(const BcmDiskFileNonVolDevice &otherInstance);

    // Assignment operator.  Not supported.
    //
    // Parameters:
    //      otherInstance - reference to the object to copy.
    //
    // Returns:
    //      A reference to "this" so that operator = can be chained.
    //
    BcmDiskFileNonVolDevice & operator = (const BcmDiskFileNonVolDevice &otherInstance);

};


//********************** Inline Method Implementations ***********************

#endif

