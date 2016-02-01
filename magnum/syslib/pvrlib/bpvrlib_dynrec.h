/***************************************************************************
 *     Copyright (c) 2002, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

/*================== Module Overview =====================================

This module implements the record functionality for pvr using a list
of complete, free, and hardware buffers.  Free buffers can be shared between
all record channels in order to handle peaks in bandwidth (since it is
less likely that all record channels will peak at the same time).  This
allows more efficient memory utilization than having static buffers for
each channel.

It is only possible to include either the standard ring buffer based record
or this dynamic implementation (which is why they share the same API).

The following general overview describes the record algorithm:

 0.0 Master Handle Creation
   - A master record handle is created (it is then used for subsequent record
     channel open calls)
   - Size of entire record buffer is passed as parameter (same for all record channels)
   
 1.0 Open
   - Resources (events, lists, etc) are created.
 
 1.1 Configure Record
   - These should be performed outside the record syslib
   - Data transport PIDs are enabled and configured
   - Record parameters are configured (including decryption/encryption, etc)
   - Record pid channel bitmap is enabled
   - SCT record parameters should be set
 
 1.2 Start Record (SCT record channel should be started first if required)
   - Free buffers are added to record hardware and hardware list
   - Record hardware is started
 
 1.3 Get Data (not re-entrant for same record channel)
   - Record buffer address and size is obtained from buffer complete list
   (this step may block if there is no data available)
   (blocking is disabled if the record channel is stopped)
   - Data is copied to specified buffer
   - Buffer complete list is updated (descriptor is added to free list)
   - The amount of data copied from record channel is returned
   - Overflow status is returned
 
 (Alternate to 1.3)
 1.3.1 Request to Get Data (steps 1.3.x not re-entrant for same record channel)
   - Record buffer address and size is obtained from buffer complete list
   (this step may block if there is no space available)
   (blocking is disabled if the record channel is stopped)
   - Overflow status is returned
 
 1.3.2 Data is copied from specified buffer by calling code
 
 1.3.3 Complete list is updated
   - Buffer complete list is updated (descriptor is added to free list)
 
 1.4 Descriptor complete ISR()
   - First descriptor is removed from hardware list and checked for completion
   - If complete, descriptor is added to complete list
   - Check if next descriptor has also completed (repeat process if necessary)
   - Set event to notify of data arrival
 
 1.5 Stop Record
   - Record hardware is stopped
   - Wait for record done interrupt to occur if we are stopping a transport record channel
   - Partially complete descriptor is moved to complete list (with special flag
     specifying how much of the buffer is occupied)
   - All left over descriptors are moved from hw to the free list
   - Any blocking calls are released (future calls are not allowed to block)
   - Additional get data function call should be called to get remaining
	  data from record.
 
 1.6 Free
   - Free descriptor buffer
   - Free record buffer manager
 
========================================================================*/
#ifndef __BPVRLIB_DYNREC_H__
#define __BPVRLIB_DYNREC_H__

#include "bxpt_record.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 
Summary:
This callback is compatible with the standard 'c' memcpy routine.
It is used to copy data between a user buffer and the ring
buffer during a pvr_recordGetData() function call.
*/
typedef void *(*BPVRlib_Rec_CopyMemoryCb)( void *, const void *, size_t ); 

/* 
Summary:
This handle is used to store the context of a pvr record instance.
*/
typedef struct BPVRlib_Rec_P_Handle *BPVRlib_Rec_Handle;

/* 
Summary:
This handle is used to store the context of a pvr record channel.
*/
typedef struct BPVRlib_Rec_P_MasterHandle *BPVRlib_Rec_MasterHandle;

/* 
Summary:
This structure is used to store various statistics that can
be used to monitor and debug a record that is in progress.
Note: All stats are reset when the record is started or flushed! 
*/
typedef struct
{
	size_t		numBytesRecorded;	/* Number of bytes recorded by hardware */
	size_t		numBytesRemoved;	/* Number of bytes removed from the record buffer by software */
	size_t		numFreeDesc;		/* Current number of free descriptors (for all record channels) */
	uint32_t	numInHwDesc;		/* Current number of descriptors being processed by hardware */
	uint32_t	numCompDesc;		/* Current number of complete descriptors */
	uint32_t	numDesc;			/* Total number of descriptors (for all record channels) */
	uint32_t	numDescAdded;		/* Number of descriptors added in to hardware */
	uint32_t	numDescCompleted;	/* Number of descriptors completed by hardware */
	uint32_t	numOverFlows;		/* Number of times the hardware has reached the end of its descriptor list */
	uint32_t	numBufferBlocks;	/* Number of times blocking has occurred due to buffer resources */
	uint32_t	numTimeouts;		/* Number of times blocking has failed due to a timeout */
	size_t		maxBfrDepth;		/* Maximum number of bytes stored in buffer */
	size_t		bfrDepth;			/* Current number of valid bytes stored in the buffer */
	size_t		descSize;			/* Size of each descriptor */
} BPVRlib_Rec_Stats;

/* 
	The BPVRlib_Rec_Events structure is used to return various events used by the record module.
	Those events could be latter useful when application useed nonblocking flavor of record
*/
typedef struct BPVRlib_Rec_Events
{
	BKNI_EventHandle	descEvent;		/* Event used to signal when descriptor is added to free list */
	BKNI_EventHandle	finishedEvent;	/* Event used to signal when the record has finished */
} BPVRlib_Rec_Events;


/****************************************************************
Summary:
This function allocates the resources required for a record.  

See also:
BPVRlib_Rec_Close(), BPVRlib_Rec_Start(), BPVRlib_Rec_GetData()
****************************************************************/
BERR_Code BPVRlib_Rec_Open(
	BPVRlib_Rec_MasterHandle *pRecMasterHandle, 	/* [out] context handle */
	BXPT_Handle xptHandle,				/* XPT handle */
	BMEM_Handle memHandle,				/* Mem handle */
	BINT_Handle intHandle,				/* Int handle */
	BXPT_ChanType chanType,				/* Record channel type */
	uint8_t *p_bfr, 					/* pointer to buffer that should be used for record buffer, 0 to allocate the buffer internally */
	size_t bfrSize,						/* size of record record buffer (in bytes). Should be multiple of descSize */
	uint32_t numDesc,					/* number of descriptors to use (minimum of 3 per record channel that will be used) */
	BPVRlib_Rec_CopyMemoryCb copyMemCb	/* function pointer for memory copies (optional) */
	);

/****************************************************************
Summary:
This function frees any resources used by a record
channel.
****************************************************************/
BERR_Code BPVRlib_Rec_Close( 
	BPVRlib_Rec_MasterHandle recMasterHandle 		/* context handle */
	);

/****************************************************************
Summary:
This function allocates the resources required for a record channel.  

Description:
This includes a buffer, descriptor list, etc.
The pvrMemCopyCb parameter only needs to be valid
if the BPVRlib_Rec_GetData() function is going to be
used.

The buffer used by this specific record channel may be shared by 
other record channels.

See also:
BPVRlib_Rec_Close(), BPVRlib_Rec_Start(), BPVRlib_Rec_GetData()
****************************************************************/
BERR_Code BPVRlib_Rec_ChannelOpen(
	BPVRlib_Rec_Handle *pRecChanHandle, 	/* [out] context handle */
	BPVRlib_Rec_MasterHandle recMasterHandle, /* master handle context */
	BXPT_Record_Handle xptRecHandle,	/* XPT record handle */
	unsigned chanNum,						/* record channel number */
	uint32_t minDesc					/* minimum number of descriptors that should be always allocated for this record channel (0 is auto) */
	);


/****************************************************************
Summary:
This function frees any resources used by a record
channel.
****************************************************************/
BERR_Code BPVRlib_Rec_ChannelClose( 
	BPVRlib_Rec_Handle recChanHandle 		/* context handle */
	);

/****************************************************************
Summary:
This function returns events used by the record module
****************************************************************/
BERR_Code BPVRlib_Rec_GetEvents( 
	BPVRlib_Rec_Handle recHandle,	/* context handle*/
	BPVRlib_Rec_Events *events    /* events would be returned in this structure */ 
	);


/****************************************************************
Summary:
This function starts a record channel.
****************************************************************/
BERR_Code BPVRlib_Rec_Start( 
	BPVRlib_Rec_Handle recChanHandle		/* context handle */
	);

/****************************************************************
Summary:
This function checks if any data has been recorded
into the record buffer.  If not, it can optionally block
until data is recorded, or return immediately.  When
data is in the record buffer, it is copied (using the
memcpy funtion pointer passed in pvr_recordAllocate)
to the destination buffer.  The function always returns
when ANY amount of data is available, ie the destination
buffer will not always be filled, so make sure to
check the *p_bfrSize when this function returns!
****************************************************************/
BERR_Code BPVRlib_Rec_GetData( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	uint8_t *p_bfr,						/* [out,sizeof(*p_bfrSize)] pointer to destination buffer */
	size_t *p_bfrSize,					/* [in,out] size of destination buffer (in bytes), returns number of bytes actually copied */ 
	bool block,							/* false (do not block), true (block until data is available) */
	bool *p_overflow					/* [out] returns 1 if record overflow has occurred */
	);

/****************************************************************
Summary:
This function checks if any data has been recorded
into the record buffer.  If not, it can optionally block
until data is recorded, or return immediately.  When
data is in the record buffer, the address and size 
of the valid data is returned.  It is up to the
calling code to copy data from this location and
update the read pointer for the record channel
using the BPVRlib_Rec_UpdateReadPointer() function.
****************************************************************/
BERR_Code BPVRlib_Rec_GetDataRequest( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	uint8_t **pp_bfr, 					/* [out] returns pointer to the start of valid data */
	size_t *p_bfrSize,					/* [out] returns size of the valid data (in bytes) */ 
	bool block							/* 0 (do not block), 1 (block until data is available) */
	);

/****************************************************************
Summary:
This function updates the read pointer for the
record record buffer.  It is used to notify the record
buffer when data has been copied out of the buffer and
this space can be re-used for new record data.
****************************************************************/
BERR_Code BPVRlib_Rec_UpdateReadPointer( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	size_t bytesRead,					/* number of bytes that have been read from ring buffer */
	bool block, 						/* not currently used */
	bool *p_overflow					/* [out] returns true if record overflow has occurred */
	);

/****************************************************************
Summary:
This function stops a record channel and waits for
the record done interrupt to occur.  After the stop
function is called, don't forget to remove the last 
remains of the record from the record buffer!

This function will return an error if there is any overflow
pending in the record channel (overflow must be handled
before the record can be stopped!)
****************************************************************/
BERR_Code BPVRlib_Rec_Stop( 
	BPVRlib_Rec_Handle recChanHandle	/* context handle */
	);

/****************************************************************
Summary:
This function returns the number of valid bytes in the
record buffer (already recorded by hardware).
****************************************************************/
void BPVRlib_Rec_GetBufferDepth( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	size_t *p_bfrDepth					/* [out] number of valid bytes in the record buffer */ 
	);

/****************************************************************
Summary:
This function sets the maximum timeout value allowed when
waiting for buffer space or descriptors to become available.
INFINITE is used by default and is the recommended value.
****************************************************************/
void BPVRlib_Rec_SetTimeout( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	uint32_t numMsec					/* timeout value in msec */
	);

/****************************************************************
Summary:
This function returns the current timeout value
used when waiting for buffer space or descriptors
to become available.
****************************************************************/
void BPVRlib_Rec_GetTimeout( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	uint32_t *p_numMsec 				/* [out] returns current timeout value in msec */
	);

/****************************************************************
Summary:
This function returns various record channel statistics.
****************************************************************/
BERR_Code BPVRlib_Rec_GetStats( 
	BPVRlib_Rec_Handle recChanHandle,	/* context handle */
	BPVRlib_Rec_Stats *p_stats 			/* [out] this structure is filled with statistics */
	);

#ifdef __cplusplus
}
#endif

#endif

