/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/
//****************************************************************************
//  $Id$
//
//  Filename:       DiskFileNonVolDevice.cpp
//  Author:         David Pullen
//  Creation Date:  Feb 10, 2000
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

//********************** Include Files ***************************************

// My api and definitions...
#include "DiskFileNonVolDevice.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

// This include file is only supported by Linux.
#if TARGETOS_Linux
#include <unistd.h>
#endif

//********************** Local Types *****************************************

//********************** Local Constants *************************************

//********************** Local Variables *************************************

//********************** Class Method Implementations ************************


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
BcmDiskFileNonVolDevice::BcmDiskFileNonVolDevice(const char *pDynamicFilename,
                                                 const char *pPermanentFilename):
    // This matches what we have on the target.
    BcmNonVolDevice(1024 * 128, 1024 * 128)
{
    fMessageLogSettings.SetModuleName("BcmDiskFileNonVolDevice");
    fMessageLogSettings.SetInstanceName("NonVol Device");
    fMessageLogSettings.Register();

    // Quick validation of the filenames.
    if ((pDynamicFilename == NULL) ||
        (strlen(pDynamicFilename) == 0))
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmDiskFileNonVolDevice")
            << "Dynamic Filename parameter is NULL or empty string!" << endl;

        assert(0);
    }
    if ((pPermanentFilename == NULL) ||
        (strlen(pPermanentFilename) == 0))
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmDiskFileNonVolDevice")
            << "Permanent Filename parameter is NULL or empty string!" << endl;

        assert(0);
    }
    
    // Do a deep copy of the filenames.
    pfDynamicFilename = new char[strlen(pDynamicFilename) + 1];
    pfPermanentFilename = new char[strlen(pPermanentFilename) + 1];
    pfDynamicFilenameBackup = new char[strlen(pDynamicFilename) + 1 + 4];
    pfPermanentFilenameBackup = new char[strlen(pPermanentFilename) + 1 + 4];

    // We can work without the backups, so only fail if no main filenames.
    if ((pfDynamicFilename == NULL) ||
        (pfPermanentFilename == NULL))
    {
        gFatalErrorMsg(fMessageLogSettings, "BcmDiskFileNonVolDevice")
            << "Failed to allocate memory to store the filenames!" << endl;

        assert(0);
    }

    strcpy(pfDynamicFilename, pDynamicFilename);
    strcpy(pfPermanentFilename, pPermanentFilename);

    if ((pfDynamicFilenameBackup != NULL) &&
        (pfPermanentFilenameBackup != NULL))
    {
        strcpy(pfDynamicFilenameBackup, pDynamicFilename);
        strcpy(pfPermanentFilenameBackup, pPermanentFilename);
        strcat(pfDynamicFilenameBackup, ".bak");
        strcat(pfPermanentFilenameBackup, ".bak");
    }
}


// Destructor.  Frees up any memory/objects allocated, cleans up internal
// state.
//
// Parameters:  N/A
//
// Returns:  N/A
//
BcmDiskFileNonVolDevice::~BcmDiskFileNonVolDevice()
{
    fMessageLogSettings.Deregister();

    // Free the memory I created.
    delete [] pfDynamicFilename;
    pfDynamicFilename = NULL;

    delete [] pfPermanentFilename;
    pfPermanentFilename = NULL;

    delete [] pfDynamicFilenameBackup;
    pfDynamicFilenameBackup = NULL;

    delete [] pfPermanentFilenameBackup;
    pfPermanentFilenameBackup = NULL;
}

// This is where the derived class writes the buffer to the device.
//
// Parameters:
//      outputBuffer - the buffer that should be written.
//      filename     - the filename that should be written.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually hardware failure).
//
bool BcmDiskFileNonVolDevice::WriteFile(BcmOctetBuffer &outputBuffer,
                                        char *filename)
{
    FILE *pFile;
    bool result = true;
    size_t bytesWritten;

    // Quick sanity check.
    if (filename == NULL)
    {
        gErrorMsg(fMessageLogSettings, "WriteFile")
            << "Filename string is NULL!" << endl;

        return false;
    }

    // Open the file for writing in binary mode.  If it doesn't exist, then it
    // should be created.
    pFile = fopen(filename, "wb");

    if (pFile == NULL)
    {
        gErrorMsg(fMessageLogSettings, "WriteFile")
            << "Failed to open file '" << filename << "' for writing!" << endl;

        return false;
    }

    // Write the buffer to the file.
    bytesWritten = fwrite(outputBuffer.AccessBuffer(), 1, outputBuffer.BytesUsed(), pFile);

    if (bytesWritten != outputBuffer.BytesUsed())
    {
        gErrorMsg(fMessageLogSettings, "WriteFile")
            << "Failed to write all of the bytes!  Only wrote " << bytesWritten << " of " << outputBuffer.BytesUsed()
            << " bytes to file '" << filename << "'!" << endl;

        result = false;
    }

    // Close the file.
    fclose(pFile);

    return result;
}

// This is where the derived class writes the buffer to the device.
//
// Parameters:
//      outputBuffer - the buffer that should be written.
//      section - the section that should be written.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually hardware failure).
//
bool BcmDiskFileNonVolDevice::WriteImpl(BcmOctetBuffer &outputBuffer,
                                        BcmNonVolSettings::NonVolSection section)
{
    bool result = true;
    char *pFilename;
    char *pFilenameBack;
    BcmOctetBuffer *testBuffer;
    unsigned int offset;
    uint32 settingsLength;
    uint32 checksum;

    // Set up the filename based on the section parameter.
    switch (section)
    {
        case BcmNonVolSettings::kDynamicSection:
            pFilename     = pfDynamicFilename;
            pFilenameBack = pfDynamicFilenameBackup;
            break;

        case BcmNonVolSettings::kPermanentSection:
            pFilename     = pfPermanentFilename;
            pFilenameBack = pfPermanentFilenameBackup;
            break;

        default:
            pFilename = NULL;
            pFilenameBack = NULL;
            break;
    }
    // We need to make sure that the main file is not corrupt before
    // we copy it to the backup file.  All the crap around rename does
    // that.

    // If either of the filenames is NULL, don't worry about a backup.
    if ((pFilename != NULL) && (pFilenameBack != NULL))
    {
        // Get a buffer to read the main file.
        if (section == BcmNonVolSettings::kDynamicSection)
           testBuffer = new BcmOctetBuffer(fPermanentSizeBytes);
        else
           testBuffer = new BcmOctetBuffer(fDynamicSizeBytes);

        result = ReadFile(*testBuffer, pFilename);

        // If the read failed, no sense even trying to back it up.
        if (result && section == BcmNonVolSettings::kDynamicSection)
        {
            // Get the length and checksum values.
            offset = 0;
            if ((testBuffer->NetworkRead(offset, settingsLength)  != false) &&
                (testBuffer->NetworkRead(offset, checksum)  != false))
            {
                // Last, make sure the checksum is correct.
                if (CalculateChecksum(*testBuffer) == 0)
                {
                    // rename does a sync(), so we don't have to.
                    rename(pFilename, pFilenameBack);
                }
            }
        }
        // No longer need the buffer.
        delete testBuffer;
    }

    result = WriteFile(outputBuffer, pFilename);
    sync();

    return result;
}

// This is where the derived class reads data from the device into the
// buffer.
//
// Parameters:
//      inputBuffer - the buffer into which the data should be read.
//      filename    - the filename that should be read.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually hardware failure).
//
bool BcmDiskFileNonVolDevice::ReadFile(BcmOctetBuffer &inputBuffer,
                                       char *filename)
{
    FILE *pFile;
    bool result = true;
    size_t bytesRead;

    // Quick sanity check.
    if (filename == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ReadFile")
            << "Filename string is NULL!" << endl;

        return false;
    }

    // Open the file for reading in binary mode.  If it doesn't exist, then
    // this should fail.
    pFile = fopen(filename, "rb");

    if (pFile == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ReadFile")
            << "Failed to open file '" << filename << "' for reading!" << endl;

        return false;
    }

    //KB_TODO: COMPARE TO ORIGINAL BFC FILE
    // Read the buffer from the file.
    bytesRead = fread(inputBuffer.AccessBuffer(), 1, inputBuffer.BufferSize(), pFile);

    if (!feof(pFile))
    {
        gErrorMsg(fMessageLogSettings, "ReadFile")
            << "Buffer too small to hold the entire file!" << endl;

        result = false;
    }

    // Finally, make the buffer think that the correct number of bytes have been
    // consumed.
    inputBuffer.SetBytesUsed(bytesRead);

    // Close the file. MRT PR7318
    fclose(pFile);

    return result;
}



// This is where the derived class handles robustness before the buffer
// is read from the device.
//
// Parameters:
//      inputBuffer - the buffer into which the data should be read.
//      section     - the section that should be read.
//
// Returns:
//      true if successful.
//      false if there was a problem (usually hardware failure).
//
bool BcmDiskFileNonVolDevice::ReadImpl(BcmOctetBuffer &inputBuffer,
                                       BcmNonVolSettings::NonVolSection section)
{
    bool result = true;
    char *pFilename;
    char *pFilenameBack;
    unsigned int offset;
    uint32 settingsLength;
    uint32 checksum;

    // Set up the filename based on the section parameter.
    switch (section)
    {
        case BcmNonVolSettings::kDynamicSection:
            pFilename     = pfDynamicFilename;
            pFilenameBack = pfDynamicFilenameBackup;
            break;

        case BcmNonVolSettings::kPermanentSection:
            pFilename     = pfPermanentFilename;
            pFilenameBack = pfPermanentFilenameBackup;
            break;

        default:
            pFilename = NULL;
            pFilenameBack = NULL;
            break;
    }

    // Try to read the main file first.
    result = ReadFile(inputBuffer, pFilename);
    if (result)
    {
        // Get the length and checksum values.
        offset = 0;
        if ((inputBuffer.NetworkRead(offset, settingsLength)  == false) ||
            (inputBuffer.NetworkRead(offset, checksum)  == false))
        {
            result = false;
        }
        else
        {
            // Last, make sure the checksum is correct.
            if (CalculateChecksum(inputBuffer) != 0)
            {
                result = false;
            }
        }
    }

    // Only check the backup file if the main file was suspect.
    if(!result)
    {
        gLogMessageRaw << "Could not read settings from Non-Vol Device... Trying to restore from backup!" << endl;

        // Try to read the backup file.
        result = ReadFile(inputBuffer, pFilenameBack);
    }
    return result;
}


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
bool BcmDiskFileNonVolDevice::ClearDeviceImpl(BcmNonVolSettings::NonVolSection section)
{
    char *pFilename;
    int result;

    // Set up the filename based on the section parameter.
    switch (section)
    {
        case BcmNonVolSettings::kDynamicSection:
            pFilename = pfDynamicFilename;
            break;

        case BcmNonVolSettings::kPermanentSection:
            pFilename = pfPermanentFilename;
            break;

        default:
            pFilename = NULL;
            break;
    }

    // Quick sanity check.
    if (pFilename == NULL)
    {
        gErrorMsg(fMessageLogSettings, "ClearDeviceImpl")
            << "Filename string is NULL!" << endl;

        return false;
    }

    result = remove(pFilename);

    if (result != 0)
    {
        gErrorMsg(fMessageLogSettings, "ClearDeviceImpl")
            << "Unable to delete the file '" << pFilename << "'!" << endl;

        return false;
    }

    return true;
}
