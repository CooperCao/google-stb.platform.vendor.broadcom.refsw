/***************************************************************************
 *     Copyright (c) 2002-2007, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * This module implements the playback functionality for pvr using an intermediate
 * ring buffer.  It features a programmable buffer location, buffer size, and number
 * of descriptors.  It also supports a constant stream by dynamically adding buffers
 * into the transport playback hardware.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/


 /*================== Module Overview =====================================
 
 The following general overview describes the playback algorithm:
  
  1.0 Allocate
  
	- The number of descriptors and buffer address are passed as parameters
	- The ring buffer manager is allocated
	- Array of descriptors is allocated (all descriptors are added to free list)
  
  1.1 Start Playback
  
	- Playback resync variable is set (used to reset sync detect engine)
	- Configure playback hardware based upon sync mode
	- Playback hardware is started
  
  1.2 Add Data (not re-entrant for same playback channel)
  
	- Buffer address and size is requested from the ring buffer manager
	(this step may block if there is no space available)
	- Data is copied into specified buffer
	- Ring buffer write offset is updated
	- Data is commited to hardware
	- In the case of no blocking, the amount of data NOT copied is returned
  
  (Alternate 1.2)
  1.2.1 Request to Add Data (steps 1.2.x not re-entrant for same playback channel)
  
	- Free buffer address and size is requested from the ring buffer manager
	(this step may block if there is no space available)
  
  1.2.2 Data is copied to specified buffer by calling code
  
  1.2.3 Write pointer is updated
  
	- Ring buffer write offset is updated
	- Data is commited to hardware
  
  1.3 Data is Commited To Hardware
  
	- Availability of free descriptor is checked
	(this step may block if there is no space available)
	- Descriptor is obtained from the free list
	- Buffer address and size is programmed into descriptor
	- Descriptor is added to hardware list
	- Descriptor is programmed into playback hardware
  
  1.4 Hardware Data Complete
  
	- First descriptor is removed from hardware list
	- Event is triggered signaling removal of descriptor
	- Ring buffer read offset is updated
	- Event is triggered signaling freeing of data from ring buffer
	- Descriptor is added to free list
	- Check if next descriptor has also completed (repeat process if necessary)
  
  1.5 Stop Playback
  
	- Playback hardware is stopped
	- Ring buffer manager is reset
	- All descriptors are moved from hw to the free list
  
  1.6 Free
  
	- Free descriptor buffer
	- Free ring buffer manager
	
================== End Module Overview =================================*/
 
#ifndef BPVRLIB_PLAY_H__
#define BPVRLIB_PLAY_H__

#include "bxpt_playback.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
	This callback is compatible with the standard 'c' memcpy routine.
	It is used to copy data between a user buffer and the ring
	buffer during a BPVRlib_Play_AddData() function call.
*/
typedef void *(*BPVRlib_Play_MemCopyCb)( void *, const void *, size_t ); 

/* 
Summary:
This handle is used to store the context of a pvr record.
*/
typedef struct BPVRlib_Play_P_Handle *BPVRlib_Play_Handle;

/* 
Summary:
This enumeration is used to describe the various types of blocking that can
be used when performing playback with this module.
*/
typedef enum
{
	BPVRlib_Play_BlockMode_eNone, /* Do not block */
	BPVRlib_Play_BlockMode_eFreeBuffer, /* Block until free space is available in the buffer */
	BPVRlib_Play_BlockMode_eFreeDescriptor, /* Block until a descriptor is available (in addition to free space) */
	BPVRlib_Play_BlockMode_eNoCommit /* Do not commit the buffer to the hardware until later */
} BPVRlib_Play_BlockMode;

/* 
	The BPVRlib_Play_Stats structure is used to store various statistics that can
	be used to monitor and debug a playback that is in progress.
	Note: All stats are reset when the playback is started or flushed! 
*/
typedef struct
{
	size_t		numBytesPlayed;		/* Number of bytes played back through hardware */
	size_t		numBytesAdded;		/* Number of bytes added into ring buffer */
	size_t		bfrSize;			/* Total buffer size */
	size_t		readOffset;			/* Current read offset */
	size_t		writeOffset;		/* Current write offset */
	size_t		hwWriteOffset;		/* Current hw write offset (committed to hardware) */
	size_t		numBytesNotCommitted;	/* Current number of bytes in the ring buffer that are not committed into hardware */
	size_t		numBytesScatGat;	/* Current number of bytes in scatter gather hardware list */
	uint32_t	numFreeDesc;		/* Current number of free descriptors */
	uint32_t	numInHwDesc;		/* Current number of descriptors being processed by hardware */
	uint32_t	numScatGatDesc;		/* Current number of descriptors in hardware on scatter gather list */
	uint32_t	numDesc;			/* Total number of descriptors */
	uint32_t	numDescAdded;		/* Number of descriptors added in to hardware */
	uint32_t	numDescCompleted;	/* Number of descriptors completed by hardware */
	uint32_t	numUnderFlows;		/* Number of times the hardware has reached the end of its descriptor list */
	uint32_t	numBufferBlocks;	/* Number of times blocking has occurred due to buffer resources */
	uint32_t	numDescBlocks;		/* Number of times blocking has occurred due to descriptor resources */
	uint32_t	numTimeouts;		/* Number of times blocking has failed due to a timeout */
	uint32_t	numMultiDescDone;	/* Number of times multiple descriptors have been completed in a single call */
} BPVRlib_Play_Stats;

/* 
	The BPVRlib_Play_Events structure is used to return various events used by playback module.
	Those events could be used by an application for non-blocking playback.
*/
typedef struct BPVRlib_Play_Events
{
	BKNI_EventHandle	descEvent;		/* Event used to signal when descriptor is added to free list */
	BKNI_EventHandle	finishedEvent;	/* Event used to signal when the playback has finished */
} BPVRlib_Play_Events;

/****************************************************************
Summary:
This function allocates the resources required for a playback.  This 
includes a ring buffer manager, descriptor list, etc.  The pvrMemCopyCb 
parameter only needs to be valid if the pvr_recordAddData() function is 
going to be used.  
****************************************************************/
BERR_Code BPVRlib_Play_Open(
	BPVRlib_Play_Handle *pPlayHandle, 	/* [out] context handle */
	BXPT_Handle xptHandle,				/* XPT handle */
	BXPT_Playback_Handle xptPlayHandle,	/* XPT playback handle */
	BMEM_Handle memHandle,				/* Mem handle */
	BINT_Handle intHandle,				/* Int handle */
	unsigned channelNum,				/* playback channel number */
	uint8_t *p_bfr,						/* pointer to buffer that should be used for playback ring buffer, 0 to allocate the buffer internally */
	size_t bfrSize,					/* size of playback ring buffer (in bytes) */
	uint32_t numDesc,					/* number of descriptors to use  */
	BPVRlib_Play_MemCopyCb pvrMemCopyCb			/* function pointer for memory copies (optional) */
	);

/****************************************************************
Summary:
This function frees any resources used by a playback channel.
****************************************************************/
BERR_Code BPVRlib_Play_Close( 
	BPVRlib_Play_Handle playHandle 		/* context handle */
	);

/****************************************************************
Summary:
This function returns events used by playback module
Description:
The events returned by this function cannot be used along with any blocking mode
other than BPVRlib_Play_BlockMode_eNone.
****************************************************************/
BERR_Code BPVRlib_Play_GetEvents( 
	BPVRlib_Play_Handle playHandle,	/* context handle*/
	BPVRlib_Play_Events *events    /* events would be returned in this structure */ 
	);


/****************************************************************
Summary:
This function starts a playback.
****************************************************************/
BERR_Code BPVRlib_Play_Start( 
	BPVRlib_Play_Handle playHandle		/* context handle */
	);

/****************************************************************
Summary:
This function checks if there is any room in the playback ring buffer and 
copies data from the source buffer.  If there is no space available in the 
ring buffer this function can optionally block until space is available 
and all data has been copied.  It used the pvrMemCopyCb function passed in 
the BPVRlib_Play_Open() function to perform the memory copy.  
****************************************************************/
BERR_Code BPVRlib_Play_AddData( 
	BPVRlib_Play_Handle playHandle,		/* context handle */
	uint8_t *p_bfr,		/* pointer to source buffer */
	size_t *p_bfrSize,	/* size of source buffer (in bytes) */
	BPVRlib_Play_BlockMode block /* block mode */
	);

/****************************************************************
Summary:
This function checks if there is any room in the playback
ring buffer. If there is no space available in the ring buffer this
function can optionally block until space is available.

Description:
This function will return BERR_TIMEOUT if there is no buffer
space available OR no descriptors available.  It is possible
to add additional data to the buffer without having descriptors
available.  If this type of operation is desired, the p_bfrSize 
variable should be checked for a non-zero value that signifies
that buffer space is available.
****************************************************************/
BERR_Code BPVRlib_Play_AddDataRequest( 
	BPVRlib_Play_Handle playHandle,	/* context */
	uint8_t **pp_bfr,		/* returns pointer to free space in ring buffer */
	size_t *p_bfrSize,	/* returns size of free space (in bytes) */
	BPVRlib_Play_BlockMode block /* block mode */
	);

/****************************************************************
Summary:
This function updates the write pointer of the ring buffer.
This function will automatically call the 
BPVRlib_Play_CommitBuffersToHardware() (with the
block parameter passed through) after the 
write pointer has been updated.
****************************************************************/
BERR_Code BPVRlib_Play_UpdateWritePointer( 
	BPVRlib_Play_Handle playHandle,	/* context */
	size_t bfrSize,		/* number of bytes copied into ring buffer */
	BPVRlib_Play_BlockMode block /* block mode */
	);

/****************************************************************
Summary:
This function updates the write pointer of the ring buffer but
will program the playback hardware to skip the specified number of 
bytes starting at the last ring buffer location returned by
BPVRlib_Play_AddDataRequest().  The specified bfrSize is 
INCLUSIVE of the skipSize.

This function requires a descriptor to be available so the block 
mode is recommended to be set as 
BPVRlib_Play_BlockMode_eFreeDescriptor.  This function is
usefull when reading blocks of data with specific alignment
requirements since an extra memory copy is not needed.
****************************************************************/
BERR_Code BPVRlib_Play_UpdateWritePointerWithSkip( 
	BPVRlib_Play_Handle playHandle, /* context */
	size_t bfrSize, 	/* number of bytes copied into ring buffer */
	size_t skipSize,	/* number of bytes to skip at the current ring buffer address when programming hardware */
	BPVRlib_Play_BlockMode block /* block mode */
	);

/****************************************************************
Summary:
This function check if descriptors are available 
for loading into the playback hardware.  If
not the function can block until one becomes available.
Once a descriptor is available, any new data is 
added into the playback hardware.
If blocking is enabled, this function will not return
until all data has been committed into hardware.
****************************************************************/
BERR_Code BPVRlib_Play_CommitBuffersToHardware( 
	BPVRlib_Play_Handle playHandle,	/* context */
	BPVRlib_Play_BlockMode block	/* block mode */
	);

/****************************************************************
Summary:
This function flushes any data currently in the playback ring buffer.
****************************************************************/
BERR_Code BPVRlib_Play_Flush( 
	BPVRlib_Play_Handle playHandle	/* context */
	);

/****************************************************************
Summary:
This function stops a playback.  It also moves all
descriptors back onto the free list and resets to
ring buffer to be ready for a new playback.
****************************************************************/
BERR_Code BPVRlib_Play_Stop( 
	BPVRlib_Play_Handle playHandle	/* context */
	);

/***********************************************************/ /**
Summary:
This function returns the number of valid bytes in the
ring buffer (currently being played back by the hardware).
****************************************************************/
BERR_Code BPVRlib_Play_GetBufferDepth( 
	BPVRlib_Play_Handle playHandle,	/* context */
	size_t *p_bfrDepth	/* returns number of valid bytes in the ring buffer */
	);

/***********************************************************/ /**
Summary:
This function waits for the playback channel to finish.
Once the playback is done it will return.  If the playback
is stopped or flushed it will return immediately.
****************************************************************/
BERR_Code BPVRlib_Play_WaitForFinished( 
	BPVRlib_Play_Handle playHandle	/* context */
	);

/***********************************************************/ /**
Summary:
This function sets the maximum timeout value allowed when
waiting for buffer space or descriptors to become available.
INFINITE is used by default and is the recommended value.
****************************************************************/
BERR_Code BPVRlib_Play_SetTimeout( 
	BPVRlib_Play_Handle playHandle,	/* context */
	uint32_t numMsec		/* timeout value in msec */
	);

/***********************************************************/ /**
Summary:
This function returns the current timeout value
used when waiting for buffer space or descriptors
to become available.
****************************************************************/
BERR_Code BPVRlib_Play_GetTimeout( 
	BPVRlib_Play_Handle playHandle,	/* context */
	uint32_t *p_numMsec	/* returns current timeout value */
	);

/***********************************************************/ /**
Summary:
This function returns various playback channel statistics.
****************************************************************/
BERR_Code BPVRlib_Play_GetStats( 
	BPVRlib_Play_Handle playHandle,	/* context */
	BPVRlib_Play_Stats *p_stats	/* function fills this structure with statistics */ 
	);

/***********************************************************/ /**
Summary:
This function creates a NULL packet.  It is created in place starting
at the location specified.  Therefore, the buffer must be at least
188 bytes, or memory will be overwritten.
****************************************************************/
void BPVRlib_Play_GenerateTsNullPacket( 
	uint8_t *pkt			/* Pointer to buffer that will contain null packet (must be at least 188 bytes) */
	);

/***********************************************************/ /**
Summary:
This function is an alternate way of adding data to a playback channel
on a buffer by buffer basis.  This allows scatter gather playback
to be accomplished by loading specified buffers directly into the playback
channel.

Description:
The calling code needs to track the number of buffers 
added to playback and use the BPVRlib_Play_IsBufferComplete() function
to determine when buffers have been completed.  Alternatively,
the numScatGatDesc stat obtained by the BPVRlib_Play_GetStats() 
function can be used to detect when buffers have completed.

Buffers added using this function will be directly accessed by the
playback hardware.  Therefore, buffers must be contiguous and be
covertable using the BMEM_ConvertAddressToOffset() function.  
This function will not flush the CPU cache (or any other cache
in the system) so if necessary this should be done before calling
this function. The calling function cannot reuse the specified 
buffer until playback is done using it.

This function can be called and used at the same time the ring
buffer is being used.  The data will be queued up in the order
that the various functions were called.

This function requires all data added to the ring buffer to be
committed to hardware and requires a free descriptor to be
available.  The block parameter can be used to wait for these
requirements to be satisfied before the function returns.
****************************************************************/
BERR_Code BPVRlib_Play_AddBuffer( 
	BPVRlib_Play_Handle playHandle, 	/* context handle */
	const uint8_t *p_bfr,	/* pointer to source buffer. must be covertable with the BMEM_ConvertAddressToOffset() function */
	const size_t bfrSize,	/* size of buffer (in bytes) */
	BPVRlib_Play_BlockMode block /* block mode */
	);

/***********************************************************/ /**
Summary:
This function is used to detect when scatter gather buffers have
been completed by the playback hardware since the last call to
BPVRlib_Play_IsBufferComplete().

Description:
After scatter gather buffers have been added to the playback
channel using the BPVRlib_Play_AddBuffer() this function can
be used to determine when they have completed.

For example, if two buffers where added, this function would be
called until both of those buffers were reported complete.

The block parameter can be used to block until one or more buffers
have been completed.
****************************************************************/
BERR_Code BPVRlib_Play_IsBufferComplete( 
	BPVRlib_Play_Handle playHandle, /* context handle */
	unsigned *p_bfrsCompleted,	/* number of scatter gather buffers completed since last call to BPVRlib_Play_BufferComplete() */
	BPVRlib_Play_BlockMode block /* block mode */
	);

#ifdef __cplusplus
}
#endif

#endif

