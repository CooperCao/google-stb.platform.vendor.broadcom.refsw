/***************************************************************************
 *     Copyright (c) 2007 Broadcom Corporation
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
 * PVR library, playback feeder module
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef BPVRLIB_FEED_H__
#define BPVRLIB_FEED_H__
#include "bxpt_playback.h"
#include "bavc_xpt.h"

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
The BPVRlib_Feed is replacement for BPVRlib_Play. And goal of this
replacement is to improve performance, it's done in several ways: 
	o reduce code complexity by removing unused functionality
	o support for batch operations (send multiple blocks at the time), thus spreading cost of function call across multiple data chunks 
	o effective use of cached memory to reduce latency of CPU to hardware communications.


Purpose of BPVRlib_Feed library is to send data from the memory using the playback block into the transport hardware.

BPVRlib_Feed library provides scalable API that could be used in wide range of application 
	o data from circular FIFO 
	o data from network buffers 
	o synchronous transfer from the user's memory 
	o data from the stream parsers library 
	o good use of resources at hand (MIPS cache, scatter/gather nature of playback hardware) 
	o Asynchronous/non-blocking execution

*************************************************************************/


/* type of BPVRlib_Feed instance */
typedef struct BPVRlib_Feed  *BPVRlib_Feed_Handle;

/*
Summary:
The settings needed for creating instance of BPVRlib_Feed 
*/
typedef struct BPVRlib_Feed_Settings {
        BXPT_Handle xptHandle;              /* XPT handle */
        BXPT_Playback_Handle xptPlayHandle; /* XPT playback handle */
        BMMA_Heap_Handle mmaHeap;
        BMMA_Block_Handle bufferBlock;  /* source data buffer block */
        unsigned bufferOffset;          /* offset into bufferBlock */
        BINT_Handle intHandle;              /* Int handle */
        bool useExtndedDesc;		        /* enable use of extended (8 word) descriprots if HW supports it,  this allows use of the BPVRlib_Feed_AddExtendedOffsetEntries API */
        size_t numDesc;                   /* number of descriptors to use  */
        void *applicationCnxt;             /* application specific context */
        int applicationNumber;           /* application specific number passed into the callback */
        void (*descAvaliable_isr)(void *applicationCnxt, int applicationNumber); /* function that is called when new descriptor becomes avaliable, e.g.  when BPVRlib_Feed_AddEntries/BPVRlib_Feed_OffsetAddEntries could be called and it will consume some data */
        void* (*offsetToCachedAddr) (uint64_t offset);
        void (*flushCache)(const void *pvAddr, size_t ulNumBytes);
} BPVRlib_Feed_Settings;

/*
Summary:
	status of BPVRlib_Feed instance
See also:
    BPVRlib_Feed_GetStatus
*/
typedef struct BPVRlib_Feed_Status {
        unsigned freeDesc;     /* number of avaliable descriptors, call to BPVRlib_Feed_AddEntries/BPVRlib_Feed_OffsetAddEntries will consume at least that many entries */
} BPVRlib_Feed_Status;

/*
Summary:
This structure describes single transfer unit for the feeder 
Description:
    This structure is used to describe transfer unit in terms of CPU virtual address
See also:
   o BPVRlib_Feed_OffsetEntry
   o BPVRlib_Feed_AddEntries
*/
typedef struct BPVRlib_Feed_Entry {
        void *addr; /* memory address such BMEM_Heap_ConvertAddressToOffset could be uses convert address to the device offset, for example an address returned by BMEM_Alloc */
        size_t len; /* length of memory block */
} BPVRlib_Feed_Entry;

/*
Summary:
	This structure describes single transfer unit for the feeder 
Description:
    This structure is used to describe transfer unit it terms of physicall device addresses (also known as device offset)
See also:
   o BPVRlib_Feed_Entry
   o BPVRlib_Feed_AddOffsetEntries
*/
typedef struct BPVRlib_Feed_OffsetEntry {
        uint32_t    offset; /* device offset of the memory region */
        size_t len; /* length of memory block */
} BPVRlib_Feed_OffsetEntry;

/*
Summary:
	This structure describes extended single transfer unit for the feeder 
Description:
    This structure is used to describe transfer unit it terms of physicall device addresses (also known as device offset)
See also:
   o BPVRlib_Feed_AddExtendedOffsetEntries
*/
typedef struct BPVRlib_Feed_ExtendedOffsetEntry {
    BPVRlib_Feed_OffsetEntry baseEntry;
    BAVC_TsMux_DescConfig flags;  
} BPVRlib_Feed_ExtendedOffsetEntry;

/*
Summary:
	fills-out configuration structure with default values */
void BPVRlib_Feed_GetDefaultSettings(
		BPVRlib_Feed_Settings *settings /* pointer to the configuration structure */
		);
/*
Summary:
	opens new instancne of BPVRlib_Feed_ feeder 
*/
BERR_Code BPVRlib_Feed_Open(
		BPVRlib_Feed_Handle *feed,  /* out pointer to the instance of BPVRlib_Feed */
		const BPVRlib_Feed_Settings *settings /* pointer to the configuration structure */
		);

/*
Summary:
	releases all resources allocated for the feeder
*/
void BPVRlib_Feed_Close(
		BPVRlib_Feed_Handle feed /* instance of BPVRlib_Feed */
		);

/*
Summary:
   Addes new data to the feeder.
Description:
  This function is used to add new data to the feeder.
  Pointer to BPVRlib_Feed_Entry need to be valid only during duration of the function call . 
*/
BERR_Code BPVRlib_Feed_AddEntries(
		BPVRlib_Feed_Handle feed, /* instance of BPVRlib_Feed */
		BMEM_Heap_Handle heap,  /* instance of BMEM_Heap_Handle that used to convert entries[i].addr to the device offset */
		const BPVRlib_Feed_Entry *entries, /* pointer to array of BPVRlib_Feed_Entry */ 
		size_t count,		/* number of elements in the array */ 
		size_t *nconsumed   /* out pointer to return number of consumed elements */
		);

/*
Summary:
   Addes new data to the feeder.
Description:
  This function is used to add new data to the feeder.
  Pointer to BPVRlib_Feed_Entry need to be valid only during duration of the function call. 
  This function shall be used if address can't be converted by using BMEM_Heap_Handle or user want's to
  remove overhead of calling BMEM_Heap_ConvertAddressToOffset for each address
*/
BERR_Code BPVRlib_Feed_AddOffsetEntries(
		BPVRlib_Feed_Handle feed, /* instance of BPVRlib_Feed */
		const BPVRlib_Feed_OffsetEntry *entries, /* pointer to array of BPVRlib_Feed_OffsetEntry */ 
		size_t count, /* number of elements in the array */ 
		size_t *nconsumed/* out pointer to return number of consumed elements */
		);

/*
Summary:
   Addes new data to the feeder.
Description:
  This function is used to add new data to the feeder.
  Pointer to BPVRlib_Feed_ExtendedOffsetEntry need to be valid only during duration of the function call. 
  This function shall be used if address can't be converted by using BMEM_Heap_Handle or user want's to
  remove overhead of calling BMEM_Heap_ConvertAddressToOffset for each address.
See also:
   o BPVRlib_Feed_AddOffsetEntries
   o BPVRlib_Feed_Settings
*/
BERR_Code BPVRlib_Feed_AddExtendedOffsetEntries(
		BPVRlib_Feed_Handle feed, /* instance of BPVRlib_Feed */
		const BPVRlib_Feed_ExtendedOffsetEntry *entries, /* pointer to array of BPVRlib_Feed_ExtendedOffsetEntries */ 
		size_t count, /* number of elements in the array */ 
		size_t *nconsumed/* out pointer to return number of consumed elements */
		);

/*
Summary:
   This function is used to monitor progress of transaction 
Description:
   This function returns number of completed entries since last call to BPVRlib_Feed_GetCompleted. Often 
   application would use this number recycle memory.
*/
BERR_Code BPVRlib_Feed_GetCompleted(
		BPVRlib_Feed_Handle feed, /* instance of BPVRlib_Feed */
		size_t *ncompleted	/* out pointer to return number of completed elements */
		);
/*
Summary:
	This function activates data feeding
*/
BERR_Code BPVRlib_Feed_Start(
		BPVRlib_Feed_Handle feed /* instance of BPVRlib_Feed */
		);

/*
Summary:
	This function deactivates data feeding.
Description:
	The BPVRlib_Feed_Stop function stops data feeding, all unsent entries are discarded.
*/
void BPVRlib_Feed_Stop(
		BPVRlib_Feed_Handle feed /* instance of BPVRlib_Feed */
		);
/*
Summary:
   This function returms statos of BPVRlib_Feed module
*/
void BPVRlib_Feed_GetStatus(
		BPVRlib_Feed_Handle feed, /* instance of BPVRlib_Feed */
		BPVRlib_Feed_Status *status /* out pointer to the  BPVRlib_Feed_Status structure */
		);
#ifdef __cplusplus
}
#endif

#endif /* BPVRLIB_FEED_H__ */

