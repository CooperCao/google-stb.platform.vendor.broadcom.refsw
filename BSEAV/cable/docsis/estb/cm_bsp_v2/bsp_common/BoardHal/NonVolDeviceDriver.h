//****************************************************************************
//
//  Copyright (c) 2002  Broadcom Corporation
//  All Rights Reserved
//  No portions of this material may be reproduced in any form without the
//  written permission of:
//          Broadcom Corporation
//          16215 Alton Parkway
//          Irvine, California 92618
//  All information contained in this document is Broadcom Corporation
//  company private, proprietary, and trade secret.
//
//****************************************************************************
//  $Id: NonVolDeviceDriver.h 1.4 2004/08/11 14:17:04Z dpullen Release $
//
//  Filename:       NonVolDeviceDriver.h
//  Author:         David Pullen
//  Creation Date:  Jan 2, 2002
//
//****************************************************************************
//  Description:
//      This is the second (possibly third?) incarnation of the NonVol device
//      driver.  The purpose of this driver is to handle storing and reading
//      Dynamic and Permanent nonvol settings to a device, providing data
//      integrity and redundant backup support when possible.  This driver
//      interfaces to the flash driver, which is the media on which we store
//      the NonVol settings.
//
//      This driver only deals with raw buffers of data; it doesn't interprit
//      the settings in any way.
//
//      The driver manages 2 sections (essentially 2 files).  These are mapped
//      to different flash blocks, so that each can be written and erased
//      independently, which keeps a power failure from corrupting both.
//
//      Each flash block is further subdivided into segments.  The driver will
//      append segments during each write until the block is filled, at which
//      point it will erase the block and start over for the next write.  The
//      size of the segment is somewhat dynamic, growing as needed to accomodate
//      the amount of storage that is needed to write the settings, up to the
//      max size allowed for the section.
//
//      There is a control word at the end of the flash block that tells the
//      current segment size and contains a bitmap of the segments that have
//      been written.  If the segment size changes, then the flash block is
//      erased and the entire section is written.  If the segment size doesn't
//      change, then a new segment is written to the next available slot.  The
//      segment bitmask contains a 0 if a segment slot has been written to, and
//      a 1 if the segment slot is available for writing.
//
//      Diagram of a nonvol section in flash:
//
//      +----------+----------+---------+----------+-----+
//      | Segment1 | Segment2 |  . . .  | SegmentN | Ctl |
//      +----------+----------+---------+----------+-----+
//         N bytes    N bytes              N bytes  8 bytes
//
//      Diagram of the Control Word:
//
//      +-----------------+----------------+----------------+
//      | SegmentSize (N) |   Reserved     | SegmentBitmask |
//      +-----------------+----------------+----------------+
//            4 bytes           2 bytes         2 bytes
//
//****************************************************************************

#ifndef NONVOLDEVICEDRIVER_H
#define NONVOLDEVICEDRIVER_H

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

//********************** Global Constants ************************************

// The two sections that can be read/written.  Typically, Permanent settings
// are only written during manufacturing, but Dynamic settings can be written
// at any time during normal operation of the system.
typedef enum
{
    kNonVolDriverDynamicSection = 0,
    kNonVolDriverPermanentSection = 1
} NonVolDriverSection;

// Result codes returned by various functions.
typedef enum
{
    kNonVolDriverFailure = 0,
    kNonVolDriverSuccess = 1
} NonVolDriverResult;

//********************** Global Variables ************************************

//********************** Forward Declarations ********************************


// This function is called by the flash driver during initialization, when it
// is placing the nonvol regions.  Permanent nonvol can wind up being placed
// in a couple of different locations; if valid settings are stored somewhere,
// then we want to use them, otherwise we pick the best location based on
// the size and flash device geometry.
//
// The flash driver will select a location and size, and call this function so
// that we can check various aspects (main the control word) to see if it look
// reasonably valid.  If so, the flash driver will select this location when
// placing the region.
//
// This method can be called prior to calling NonVolDriverInit().
//
// This was added for PR3958.
//
// Parameters:
//      pFlashMemory - pointer to the beginning of flash memory that the nonvol
//                     might be stored in.  This should be the beginning of a
//                     flash block.
//      numberOfBytes - the number of bytes past the memory pointer that would
//                      be assigned to the nonvol region.
//
// Returns:
//      kNonVolDriverSuccess if this appears to be valid nonvol settings.
//      kNonVolDriverFailure if this isn't valid nonvol settings.
//
NonVolDriverResult NonVolDriverIsValid(const uint8 *pFlashMemory,
                                       unsigned int numberOfBytes);

// This initializes the driver, letting it allocate resources, etc.  This
// function MUST be called before calling the rest of the functions, after the
// OS, C runtime library and flash driver are initialized.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void NonVolDriverInit(void);

// This deinitializes the driver, releasing resources.
//
// Parameters:  None.
//
// Returns:  Nothing.
//
void NonVolDriverDeinit(void);

// Returns the maximum number of bytes that can be written to (or read from)
// the specified section of nonvol.  This will usually be half the size of a
// flash block (so that we can store 2 copies of the settings, giving some
// fail-safe redundancy).  This can be used to size buffers, etc.
//
// Parameters:
//      section - the section to query.
//
// Returns:
//      The max number of bytes for the section.
//
unsigned int NonVolDriverMaxSizeBytes(NonVolDriverSection section);

// Returns the current segment size for the specified section of nonvol.  This
// can be used to size buffers, etc., and corresponds to the maximum number of
// bytes of data that will be read, or that can be written without causing the
// segment size to increase.
//
// Note that this can return 0 if nonvol has not yet been initialized or if
// the control word has been corrupted.
//
// Parameters:
//      section - the section to query.
//
// Returns:
//      The segment size for the section of nonvol.
//
unsigned int NonVolDriverSegmentSizeBytes(NonVolDriverSection section);

// Reads a segment of nonvol data from the specified section in flash memory to
// the buffer specified.  It will read the smaller of the current segment size
// or the specified buffer size.  It always reads the most recently written
// segment, as specified by the segment bitmask in the control word.
//
// There is no indication as to the number of bytes of valid data that were
// previously written to the segment, so there is no indication of how many
// valid bytes were read.  The client must embed this information in the data
// if it is desired.
//
// Parameters:
//      section - the section to read.
//      pBuffer - pointer to a buffer into which the data should be copied.
//      bufferSize - the maximum number of bytes that can be copied into the
//                   buffer.
//
// Returns:
//      kNonVolDriverSuccess if a segment was found and the data was copied
//          into the buffer (possibly truncated because the buffer size was
//          smaller than the segment size).
//      kNonVolDriverFailure if no segments were found (i.e. the bitmask is all
//          1s, or the segment size is invalid or corrupted).
//
NonVolDriverResult NonVolDriverRead(NonVolDriverSection section,
                                    uint8 *pBuffer, unsigned int bufferSize);

// Writes the buffer of data to the specified section in flash memory.  If
// possible, it will simply append the data as the next segment.  If the flash
// block is full (no segments available), or if the numberOfBytes is larger than
// the current segment size, then it will erase the flash block and write the
// segment.
//
// After the data is written, it will verify the data, retrying the write as
// needed.
//
// Parameters:
//      section - the section to write.
//      pBuffer - pointer to a buffer of data to be written to the device.
//      numberOfBytes - the number of bytes to write.
//
// Returns:
//      kNonVolDriverSuccess if the data was written successfully.
//      kNonVolDriverFailure if the data could not be written correctly.
//
NonVolDriverResult NonVolDriverWrite(NonVolDriverSection section,
                                     const uint8 *pBuffer, 
                                     unsigned int numberOfBytes);

// Forces the flash block(s) associated with the specified section to be erased.
// The existing data is wiped out, and must be written manually.
//
// This is generally only used for debugging and/or manufacturing purposes to
// restore the flash device to a "virgin" state.  This is the only way to force
// the segment size to be reduced if it has grown; the next write will set the
// segment size based on the amount of data that is written.
//
// Parameters:
//      section - the section to be cleared.
//
// Returns:
//      kNonVolDriverSuccess if the section was cleared.
//      kNonVolDriverFailure if the section was not cleared for some reason.
//
NonVolDriverResult NonVolDriverClear(NonVolDriverSection section);


#if defined __cplusplus
}
#endif

#endif


