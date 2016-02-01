/***************************************************************************
 *     Copyright (c) 2002, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * original file: \SetTop\syslib\pvrlib\venom2\ringpbmgr.h@@Irvine_HDDemo_Devel\20
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#ifndef __BPVRLIB_P_RINGMGR__
#define __BPVRLIB_P_RINGMGR__

#include "bkni.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*BPVRlib_P_RingMgrCb)( uint32_t channel );

/*
 * This structure is used to track all internal variables in the ring buffer
 * manager
 */
typedef struct BPVRlib_P_RingMgr_Impl {
	unsigned		channel;				/* this is returned by notify callback */
	uint8_t			*p_bfr;					/* ring buffer virtual (CPU) address */
	size_t			bufferSize;				/* size of ring buffer */
	uint32_t		writeWrapCount;			/* number of times the write count has wrapped */
	size_t			writeOffset;			/* write offset (end of valid data) */
	uint32_t		hwWriteWrapCount;		/* number of times the hwwrite count has wrapped */
	size_t			hwWriteOffset;			/* number of bytes loaded into hardware dma (optional use) */
	uint32_t		readWrapCount;			/* number of times the read count has wrapped */
	size_t			readOffset;				/* read offset (start of valid data) */
	volatile bool	resetInProgress;/* set when the reset function is called until the next add request is called */
	volatile bool	abortInProgress;/* set when the abort function is called until the next reset */
	BKNI_EventHandle	notifyEvent;			/* used to wake blocked calls when the read or write pointers are updated */
	BPVRlib_P_RingMgrCb	emptyToNotEmptyCb;		/* callback that is called when the buffer goes from empty to not empty */
	BPVRlib_P_RingMgrCb	fullToNotFullCb;		/* callback that is called when the buffer goes from full to not full */
} BPVRlib_P_RingMgr_Impl, *BPVRlib_P_RingMgr_Handle;

/****************************************************************
Summary:
This function creates a ring buffer manager.  It allocates
any resources required by the ring buffer manager.
When this call completes the ring buffer manager is
placed in the "reset" state.
the playback manager (NULL if error)
****************************************************************/
BERR_Code BPVRlib_P_RingMgr_Open( 
 			BPVRlib_P_RingMgr_Handle *rbmgr, 			/* [out] returns context */
			uint32_t channel,							/* returned as parameter by callback functions */
			BPVRlib_P_RingMgrCb emptyToNotEmptyCb,		/* optional callback function to be called when the ring buffer transitions from empty to not empty */
			BPVRlib_P_RingMgrCb fullToNotFullCb,		/* optional callback function to be called when the ring buffer transitions from full to not full */
			uint8_t *p_bfr,								/* pointer to the buffer that the ring buffer manager should use */
			size_t bfrSize							/* the size of the buffer described by the p_bfr pointer */
			);

/****************************************************************
Summary:
This function resets a ring buffer manager.  This includes
reseting any of the buffer offsets and wrap counts.
It also wakes any blocked call pending by the ring buffer manager.
After a reset, the rpbmgr_AddDataRequest() function
must be called to remove the reset condition.
****************************************************************/
void BPVRlib_P_RingMgr_Reset( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			);

/****************************************************************
Summary:
This function frees any resources used by a ring buffer
manager. 
****************************************************************/
void BPVRlib_P_RingMgr_Close( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			);

/****************************************************************
Summary:
This function requests to add data into the ring buffer manager
If there is no free space available this function will
block (if required) until free space becomes available.
****************************************************************/
BERR_Code BPVRlib_P_RingMgr_AddDataRequest( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			uint8_t **pp_bfr,						/* returns pointer of starting buffer address of free space */
			size_t *p_count,					/* returns size of the free space (in bytes) */
			bool block,							/* false (do not block), true (block until free space is available) */
			uint32_t timeout						/* max num of msec to block */
			);

/****************************************************************
Summary:
This function increments the ring buffer write pointer
It is used in conjuntion with the rpbmgr_AddDataRequest()
to add data into the ring buffer.
****************************************************************/
BERR_Code BPVRlib_P_RingMgr_UpdateWritePointer( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t count						/* number of bytes to increment the write pointer */
			);

/****************************************************************
Summary:
This function requests to remove valid data from the ring buffer manager
If there is no valid data available this function will
block (if required) until valid data becomes available.
****************************************************************/
BERR_Code BPVRlib_P_RingMgr_RemoveDataRequest( 
			BPVRlib_P_RingMgr_Handle rbmgr,				/* handle of ring buffer manager */
			uint8_t **pp_bfr,				/* returns pointer of starting buffer address of valid data */
			size_t *p_count,			/* returns size of the valid data (in bytes) */
			bool block,					/* false (do not block), true (block until free space is available) */
			uint32_t timeout				/* max num of msec to block */
			);

/****************************************************************
Summary:
This function increments the ring buffer read pointer.
It is used in conjuntion with the rpbmgr_RemoveDataRequest()
to remove valid data from the ring buffer.
****************************************************************/
BERR_Code BPVRlib_P_RingMgr_UpdateReadPointer( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t count						/* number of bytes to increment the read pointer */
			);

/****************************************************************
Summary:
This function gets the amount of data that has been
added into the ring buffer, but not placed in the
hardware dma engine.
****************************************************************/
void BPVRlib_P_RingMgr_GetHwWriteDifference( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			uint8_t **pp_bfr,						/* returns pointer of starting buffer address of data that needs to be added into hardware */
			size_t *p_count						/* returns size of the data that needs to be added into hardware */
			);

/****************************************************************
Summary:
This function increments the ring buffer hardware write pointer.
It is used in conjuntion with the rpbmgr_GetHwWriteDifference()
to track what data from the ring buffer has
been added into the hardware.
****************************************************************/
void BPVRlib_P_RingMgr_UpdateHwWriteCount( 
			BPVRlib_P_RingMgr_Handle rbmgr,	/* handle of ring buffer manager */
			size_t count					/* number of bytes to increment the hw write pointer */
			);

/****************************************************************
Summary:
This function returns the number of free bytes available
in the ring buffer.
****************************************************************/
void BPVRlib_P_RingMgr_GetNumFreeBytes( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t *p_numFreeBytes				/* returns number of bytes available in ring buffer */
			);

/****************************************************************
Summary:
This function wakes any blocked call pending by the ring buffer manager.
****************************************************************/
void BPVRlib_P_RingMgr_Abort( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			);

#ifdef __cplusplus
}
#endif

#endif /* __PLAYBACKMGR_H__ */
