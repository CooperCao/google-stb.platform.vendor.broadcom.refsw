//****************************************************************************
//
// Copyright (c) 2001-2008 Broadcom Corporation
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
//  Filename:       ProgramStoreDeviceDriver.h
//  Author:         David Pullen
//  Creation Date:  Dec 11, 2001
//
//****************************************************************************
//  Description:
//      This is the second incarnation of the Program Store device driver.  The
//      purpose of this driver is to handle storing a new application to one of
//      two slots.  This driver interfaces to the flash driver, which is the
//      media on which we store the images/programs.
//
//      The driver supports a special mode on some flash devices that allows a
//      single large image to span the storage regions for both images.  This
//      is only possible if image1 and image2 are in adjacent regions in the
//      flash device.
//
//****************************************************************************

#ifndef PROGRAMSTOREDEVICEDRIVER_H
#define PROGRAMSTOREDEVICEDRIVER_H

#if defined __cplusplus
extern "C" {
#endif

// Visual SlickEdit gets confused by the open { above, and forces the code that
// follows to be indented.  This close } satisfies the editor and makes life
// easier.  Because it's inside the #if 0, the compiler will ignore it.
#if 0
}
#endif

//********************** Include Files ***************************************

// For the portable types (uint32, etc).
#include "bcmtypes.h"

//********************** Global Types ****************************************

typedef struct
{
   uint16 usSignature;     // the unique signature may be specified as a command
                           // line option:  The default is: 0x3350

   uint16 usControl;       // Control flags; the 8 lsbs indicate the compression
                           // type that is used.  Other bits are reserved for
                           // feature specifications.
#define  PGM_CONTROL_COMPRESSION_BITS                   0x0007
#define  PGM_CONTROL_FEATURE_BITS                       0xfff8
   
#define  PGM_CONTROL_COMPRESSED_IMAGE_NO_COMPRESSION    0x0000
#define  PGM_CONTROL_COMPRESSED_IMAGE_LZRW1_KH          0x0001
#define  PGM_CONTROL_COMPRESSED_IMAGE_MINILZO           0x0002
#define  PGM_CONTROL_COMPRESSED_IMAGE_RESERVED          0x0003  // Skipped
#define  PGM_CONTROL_COMPRESSED_IMAGE_NRV2D99           0x0004
#define  PGM_CONTROL_COMPRESSED_IMAGE_LZMA              0x0005

   uint16 usMajorRevision; // Major SW Program Revision
   uint16 usMinorRevision; // Minor SW Program Revision
                           // From a command line option this is specified as xxx.xxx
                           // for the Major.Minor revision (note: Minor Revision is 3 digits)

   uint32 ulcalendarTime;  // calendar time of this build (expressed as seconds since Jan 1 1970)


   uint32 ulfilelength; // length of Program portion of file (after compressed, if compressed)

   uint32 ulProgramLoadAddress; // Address where program should be loaded (virtual, uncached)

   #define PROGRAM_STORE_MAX_FILENAME_LENGTH 64
   
   char cFilename[PROGRAM_STORE_MAX_FILENAME_LENGTH];     // NULL terminated filename only (not pathname)

   uint16 usHcs;     // 16-bit crc Header checksum (CRC_CCITT) over the header [usSignature through cFilename]
   uint16 reserved;  // reserved
   uint32 ulcrc;     // CRC-32 of Program portion of file (following the header)

} BcmProgramHeader;

//********************** Global Constants ************************************

// Constants for selecting an image.
typedef enum
{
    kProgramStoreImage1 = 1,
    kProgramStoreImage2 = 2
} ProgramStoreImage;

// Result codes returned by various functions.
typedef enum
{
    kProgramStoreDriverFailure = 0,
    kProgramStoreDriverSuccess = 1
} ProgramStoreDriverResult;

// Used to select oversize image enable/disable.
typedef enum
{
    kProgramStoreDriverOversizeImageDisable = 0,
    kProgramStoreDriverOversizeImageEnable = 1
} ProgramStoreOversizeImageSelect;

// PR1744
// Used to select incompatible image enable/disable.  This determines the amount
// of error checking that is done on the ProgramStore header.
typedef enum
{
    kProgramStoreDriverIncompatibleImageDisable = 0,
    kProgramStoreDriverIncompatibleImageEnable = 1
} ProgramStoreIncompatibleImageSelect;

// PR1744
// The reasons why a ProgramStore header might not be valid.
typedef enum
{
    kProgramStoreHeaderIsValid = 0,     // No problems detected.
    kProgramStoreHeaderHcsFailed,
    kProgramStoreHeaderImageTooBig,
    kProgramStoreHeaderSignatureMismatch,
    kProgramStoreHeaderCompressionTypeNotSupported,
    kProgramStoreHeaderCrcFailed
} ProgramStoreInvalidHeaderReason;

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************


// This initializes the driver, letting it allocate resources, etc.  This
// function MUST be called before calling the rest of the functions, after the
// OS, C runtime library and flash driver are initialized.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void ProgramStoreDriverInit(void);

// This deinitializes the driver, releasing resources.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void ProgramStoreDriverDeinit(void);

// Returns the maximum size (in bytes) of the image that can be stored to the
// device.  Because image1 and image2 can be different sizes, you must query
// the max size for the image that you are going to store to.
//
// Parameters:
//      image - the image to be queried.
//
// Returns:
//      The max size of the specified image (in bytes).
//
unsigned int ProgramStoreDriverMaxImageSize(ProgramStoreImage image);

// Returns the filename associated with the specified image.  This is often
// used to determine if the image that is currently stored is different from
// the image that is about to be loaded.
//
// This must be called before you open the image for writing; that destroys
// the image, and the filename can't be queried.
//
// Parameters:
//      image - the image to be queried.
//
// Returns:
//      Pointer to a string containing the image name.
//      If the filename couldn't be queried for some reason, this returns NULL.
//
const char *ProgramStoreDriverImageName(ProgramStoreImage image);

const char *ProgramStoreDriverFileName(ProgramStoreImage image);
uint16 ProgramStoreDriverSignature(ProgramStoreImage image);
uint16 ProgramStoreDriverControl(ProgramStoreImage image);
uint16 ProgramStoreDriverMajorRev(ProgramStoreImage image);
uint16 ProgramStoreDriverMinorRev(ProgramStoreImage image);
uint32 ProgramStoreDriverCalendarTime(ProgramStoreImage image);
uint32 ProgramStoreDriverFileLength(ProgramStoreImage image);
uint16 ProgramStoreDriverHCS(ProgramStoreImage image);
uint32 ProgramStoreDriverCRC(ProgramStoreImage image);


// On some flash devices, there is a special case where image1 and image2 are
// adjacent.  In this case, it is possible to use both image slots to store a
// single image.  This function checks to see if this is possible.
//
// This is primarily for debugging purposes, to load a large image on a small
// flash device.  This will cause both image1 and image2 to be overwritten, so
// if there is a power failure, then the system will become inoperable.
//
// Parameters:  None.
//
// Returns:
//      kProgramStoreDriverSuccess if an oversize image can be stored to
//          image1/image2.
//      kProgramStoreDriverFailure if an oversize image can't be stored because
//          the images aren't adjacent.
//
ProgramStoreDriverResult ProgramStoreDriverOversizeImageAllowed(void);

// This enables storing an oversize image to image1.  After calling this
// function, calls to ProgramStoreDriverMaxImageSize(kProgramStoreImage1) will
// return the maximum size for image1 + image2, and you will be allowed to write
// to both image slots with one image.
//
// This setting has no effect on operations involving image2.
//
// If oversize images aren't supported (ProgramStoreDriverOversizeImageAllowed()
// returns kProgramStoreDriverFailure), then calling this function has no effect.
//
// Parameters:
//      select - kProgramStoreDriverOversizeImageEnable to enable oversize
//               image mode, kProgramStoreDriverOversizeImageDisable to disable.
//
// Returns:  Nothing.
//
void ProgramStoreDriverSetOversizeImage(ProgramStoreOversizeImageSelect select);

// This enables/disables extra error checking on the ProgramStore header.  By
// default, the error checking code requires that the image be completely
// valid and compatible with system capabilities (compression type, signature,
// CRC, HCS, etc.).
//
// If incompatible images are enabled (by specifying
// kProgramStoreDriverIncompatibleImageEnable), then some forms of error
// checking are skipped.  The image must still be valid (CRC, HCS, etc), but
// images targeted for other platforms will be allowed.
//
// NOTE - This does NOT guarantee that the image will be executed.  The
// bootloader may decide that it can't decompress the image or that it doesn't
// like the signature.
//
// This is for PR1744.
//
// Parameters:
//      select - kProgramStoreDriverIncompatibleImageEnable to disable full
//               error checking and to allow incompatible but valid images to
//               be loaded; kProgramStoreDriverIncompatibleImageDisable to
//               require the image to be fully valid and compatible.
//
// Returns:  Nothing.
//
void ProgramStoreDriverSetIncompatibleImage(ProgramStoreIncompatibleImageSelect select);

// Accessor for the above setting.
ProgramStoreIncompatibleImageSelect ProgramStoreDriverIncompatibleImageEnabled(void);

// This function checks the specified image to see if it is valid (i.e. could
// be executed).  It does an HCS check on the header, and then a CRC on the
// program data to ensure validity.
//
// Parameters:
//      image - the image to be checked.
//
// Returns:
//      kProgramStoreDriverSuccess if the image is valid.
//      kProgramStoreDriverFailure if the image is not valid, or if there was
//          some other problem (couldn't open flash driver, etc.).
//
ProgramStoreDriverResult ProgramStoreDriverIsImageValid(ProgramStoreImage image);

// This function checks the specified ProgramStore header to see if it is
// valid; it does an HCS check on the header, checks the signature, image
// length, etc.
//
// This is for PR1744.
//
// PR10061 - you can now pass in NULL for pProgramStoreHeader; this causes the
// header check to be performed against the one that's already stored in the
// specified image slot.
//
// Parameters:
//      pProgramStoreHeader - pointer to the ProgramStore header to check.  This
//                            can be NULL.
//      image - the image slot that it would be stored to (for size comparison).
//
// Returns:
//      kProgramStoreDriverSuccess if the header is valid.
//      kProgramStoreDriverFailure if the header is not valid.
//
ProgramStoreDriverResult ProgramStoreDriverIsHeaderValid(const BcmProgramHeader *pProgramStoreHeader,
                                                         ProgramStoreImage image);

ProgramStoreDriverResult CombinedProgramStoreDriverIsHeaderValid(const BcmProgramHeader *pProgramStoreHeader);

// If ProgramStoreDriverIsHeaderValid() returns kProgramStoreDriverFailure,
// then you can call this function to determine exactly why.
//
// This is for PR1744.
//
// Parameters:  None.
//
// Returns:
//      One of the ProgramStoreInvalidHeaderReason values.
//
ProgramStoreInvalidHeaderReason ProgramStoreDriverGetHeaderInvalidReason(void);

// This causes the specified image to be corrupted such that it won't be
// executed.  The header and several blocks of data are overwritten, and the
// CRC will fail, preventing it from being run.
//
// If you call this for both images, then the system will become inoperable.
//
// Parameters:
//      image - the image to be clobbered.
//
// Returns:
//      kProgramStoreDriverSuccess if the image was successfully clobbered.
//      kProgramStoreDriverFailure if there was a problem (couldn't open flash
//          driver, write failed, etc).
//
ProgramStoreDriverResult ProgramStoreDriverClobberImage(ProgramStoreImage image);

// This causes the specified source image to be copied to the specified
// destination image, essentially making both image storage slots identical.
//
// This will only operate if the source image is deemed to be valid and the
// destination image slot can hold the image.
//
// This is needed for PR2127.
//
// Parameters:
//      sourceImage - the image to be copied.
//      destinationImage - the image that the source should be copied to.
//
// Returns:
//      kProgramStoreDriverSuccess if the image was successfully copied.
//      kProgramStoreDriverFailure if there was a problem (couldn't open flash
//          driver, write failed, source not valid, destination too small,
//          etc.).
//
ProgramStoreDriverResult ProgramStoreDriverCopyImage(ProgramStoreImage sourceImage,
                                                     ProgramStoreImage destinationImage);

// Opens the device for writing.  This prepares the specified image number for
// writing, erasing the first flash block for the image.
//
// Parameters:
//      image - the image to be opened for writing.
//
// Returns:
//      kProgramStoreDriverSuccess if the image was opened, and the first flash
//          block was erased.
//      kProgramStoreDriverFailure if there was a problem (couldn't open flash
//          driver, erase failed, etc).
//
ProgramStoreDriverResult ProgramStoreDriverOpen(ProgramStoreImage image);

// Writes a buffer of data to the image, appending it to any data that was
// previously written.  It will erase flash blocks as needed during the write.
//
// Parameters:
//      pBuffer - pointer to the buffer of data to be written.
//      numberOfBytes - the number of bytes of data to be written.
//
// Returns:
//      kProgramStoreDriverSuccess if the buffer was written successfully.
//      kProgramStoreDriverFailure if there was a problem (flash driver not
//          opened, write failed, image too big, etc.).
//
ProgramStoreDriverResult ProgramStoreDriverWrite(const uint8 *pBuffer, 
                                                 unsigned int numberOfBytes);

// Closes the device/image, locking it in.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void ProgramStoreDriverClose(void);


#if defined __cplusplus
}
#endif

#endif


