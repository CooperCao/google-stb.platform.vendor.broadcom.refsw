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
 * Module Description:
 * 
 * This module implements a generic ring buffer in software.
 * It includes function that allow the user add and remove data
 * from the ring buffer.  It supports notication of buffer
 * full to not full, and buffer empty to not empty states through
 * the use of callbacks.  It supports blocking function
 * calls when requesting to add and requesting to remove data
 * from the ring buffer.  It supports a programmable buffer
 * address and size, and does not actually require accessing any
 * memory inside the specified buffer.
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "bpvrlib_p_ringmgr.h"

BDBG_MODULE(BPVRlib_RingMgr);

BERR_Code BPVRlib_P_RingMgr_Open( 
				BPVRlib_P_RingMgr_Handle *rbmgr,			/* [out] returns context */
				uint32_t channel,							/* returned as parameter by callback functions */
				BPVRlib_P_RingMgrCb emptyToNotEmptyCb,	/* optional callback function to be called when the ring buffer transitions from empty to not empty */
				BPVRlib_P_RingMgrCb fullToNotFullCb,		/* optional callback function to be called when the ring buffer transitions from full to not full */
				uint8_t *p_bfr, 						/* pointer to the buffer that the ring buffer manager should use */
				size_t bfrSize							/* the size of the buffer described by the p_bfr pointer */
				)
{
	BERR_Code err = BERR_SUCCESS;
	
	*rbmgr = BKNI_Malloc( sizeof( BPVRlib_P_RingMgr_Impl ) );
	if( !*rbmgr )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		BDBG_ERR(("Unable to allocate ring buffer manager."));
		goto done;
	}

	/* This variable must be set to init the rpbmgr */
	(*rbmgr)->channel			= channel;
	(*rbmgr)->emptyToNotEmptyCb= emptyToNotEmptyCb;
	(*rbmgr)->fullToNotFullCb	= fullToNotFullCb;
	(*rbmgr)->bufferSize		= bfrSize;
	(*rbmgr)->p_bfr				= p_bfr;

	err = BKNI_CreateEvent(&((*rbmgr)->notifyEvent));
	if( err != BERR_SUCCESS )
	{
		BKNI_Free( *rbmgr );
		goto done;
	}
	BPVRlib_P_RingMgr_Reset( *rbmgr );

done:
	return err;
}

void BPVRlib_P_RingMgr_Reset( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			)
{
	/* Assign the proper channel */
	rbmgr->resetInProgress	= 1;
	rbmgr->abortInProgress	= 0;
	rbmgr->writeOffset		= 0;
	rbmgr->writeWrapCount	= 0;
	rbmgr->readOffset		= 0;
	rbmgr->readWrapCount	= 0;
	rbmgr->hwWriteOffset	= 0;
	rbmgr->hwWriteWrapCount	= 0;
	BKNI_SetEvent(rbmgr->notifyEvent);
}

void BPVRlib_P_RingMgr_Close( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			)
{
	BKNI_DestroyEvent(rbmgr->notifyEvent);
	BKNI_Free( rbmgr );
}

BERR_Code BPVRlib_P_RingMgr_AddDataRequest( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			uint8_t **pp_bfr,						/* returns pointer of starting buffer address of free space */
			size_t *p_count,					/* returns size of the free space (in bytes) */
			bool block,							/* false (do not block), true (block until free space is available) */
			uint32_t timeout						/* max num of msec to block */
			)
{
	BERR_Code err = BERR_SUCCESS;

	BDBG_MSG(("Request to add data to ring buffer with %s", block?"block":"no block"));

	rbmgr->resetInProgress = 0;
	*pp_bfr = (uint8_t *)rbmgr->p_bfr + rbmgr->writeOffset;

	while( 1 )
	{
		if( rbmgr->writeOffset == rbmgr->readOffset )
		{
			if( rbmgr->writeWrapCount == rbmgr->readWrapCount )
			{
				*p_count = rbmgr->bufferSize - rbmgr->writeOffset;
			}
			else
			{
				*p_count = 0;
			}
		}
		else if( rbmgr->writeOffset > rbmgr->readOffset )
		{
			*p_count = rbmgr->bufferSize - rbmgr->writeOffset;
		}
		else if( rbmgr->writeOffset < rbmgr->readOffset )
		{
			*p_count = rbmgr->readOffset - rbmgr->writeOffset;
		}

		if( *p_count == 0 && block )
		{
			BDBG_MSG(("Ring buffer full.  Waiting %ld for read offset to be updated", timeout));
			err = BKNI_WaitForEvent( rbmgr->notifyEvent, timeout );

			if( err != BERR_SUCCESS || rbmgr->resetInProgress || rbmgr->abortInProgress )
			{
				err = BERR_TRACE(BERR_TIMEOUT);
				break;
			}
		}
		else
		{
			break;
		}
	}

	return err;
}

BERR_Code BPVRlib_P_RingMgr_UpdateWritePointer( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t count						/* number of bytes to increment the write pointer */
			)
{
	BERR_Code err = BERR_SUCCESS;
	bool empty = false;

	if( rbmgr->resetInProgress )
	{
		err = BERR_TRACE(BERR_UNKNOWN);
		BDBG_WRN(("Reset or timeout occurred: rpbmgr_UpdateWritePointer"));
		goto done;
	}

	BDBG_MSG(("Incrementing write pointer by %ld bytes", count));

	if( (rbmgr->writeOffset == rbmgr->readOffset)
		&& (rbmgr->writeWrapCount == rbmgr->readWrapCount) )
	{
		empty = true;
	}

	if( (rbmgr->writeOffset+count) > rbmgr->bufferSize )
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		BDBG_ERR(("FATAL ERROR: Attempt to overflow ring buffer by %ld bytes!", (rbmgr->writeOffset+count)-rbmgr->bufferSize));
		goto done;
	}

	rbmgr->writeOffset += count;
	if( rbmgr->writeOffset >= rbmgr->bufferSize )
	{
		rbmgr->writeWrapCount++;
		rbmgr->writeOffset -= rbmgr->bufferSize;
	}
		
	BKNI_SetEvent( rbmgr->notifyEvent );

	if( empty )
	{
		if( rbmgr->emptyToNotEmptyCb )
			(*rbmgr->emptyToNotEmptyCb)( rbmgr->channel );
	}

done:
	return err;
}

BERR_Code BPVRlib_P_RingMgr_RemoveDataRequest( 
			BPVRlib_P_RingMgr_Handle rbmgr,				/* handle of ring buffer manager */
			uint8_t **pp_bfr,				/* returns pointer of starting buffer address of valid data */
			size_t *p_count,			/* returns size of the valid data (in bytes) */
			bool block,					/* false (do not block), true (block until free space is available) */
			uint32_t timeout				/* max num of msec to block */
			)

{
	BERR_Code err = BERR_SUCCESS;

	BDBG_MSG(("Requesting to remove data from ring buffer with %s", block?"block":"no block"));

	*pp_bfr = (uint8_t *)rbmgr->p_bfr + rbmgr->readOffset;

	while( 1 )
	{
		if( rbmgr->writeOffset == rbmgr->readOffset )
		{
			if( rbmgr->readWrapCount == rbmgr->writeWrapCount )
			{
				*p_count = 0;
			}
			else
			{
				*p_count = rbmgr->bufferSize - rbmgr->readOffset;
			}
		}
		else if( rbmgr->writeOffset > rbmgr->readOffset )
		{
			*p_count = rbmgr->writeOffset - rbmgr->readOffset;
		}
		else if( rbmgr->writeOffset < rbmgr->readOffset )
		{
			*p_count = rbmgr->bufferSize - rbmgr->readOffset;
		}

		if( *p_count == 0 && block )
		{
			BDBG_MSG(("Ring buffer empty.  Waiting %ld for write offset to be updated", timeout));
			err = BKNI_WaitForEvent( rbmgr->notifyEvent, timeout);

			if( err != BERR_SUCCESS || rbmgr->resetInProgress || rbmgr->abortInProgress )
			{
				err = BERR_TRACE(BERR_TIMEOUT);
				break;
			}
		}
		else
		{
			break;
		}
	}

	return err;
}

BERR_Code BPVRlib_P_RingMgr_UpdateReadPointer( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t count						/* number of bytes to increment the read pointer */
			)
{
	BERR_Code err = BERR_SUCCESS;
	bool full = false;

	if( rbmgr->resetInProgress )
	{
		BDBG_WRN(("Reset or timeout occurred: rpbmgr_UpdateReadPointer"));
		err = BERR_TRACE(BERR_UNKNOWN);
		goto done;
	}

	BDBG_MSG(("Incrementing read pointer by %ld bytes", count));

	if( (rbmgr->writeOffset == rbmgr->readOffset)
		&& (rbmgr->writeWrapCount != rbmgr->readWrapCount) )
	{
		full = true;
	}

	if( (rbmgr->readOffset+count) > rbmgr->bufferSize )
	{
		BDBG_ERR(("FATAL ERROR: Attempt to overflow ring buffer by %ld bytes!", (rbmgr->readOffset+count)-rbmgr->bufferSize));
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	rbmgr->readOffset += count;
	if( rbmgr->readOffset >= rbmgr->bufferSize )
	{
		rbmgr->readWrapCount++;
		rbmgr->readOffset -= rbmgr->bufferSize;
	}

	BKNI_SetEvent( rbmgr->notifyEvent );

	if( full )
	{
		if( rbmgr->fullToNotFullCb )
			(*rbmgr->fullToNotFullCb)( rbmgr->channel );
	}

done:
	return err;
}

void BPVRlib_P_RingMgr_GetNumFreeBytes( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			size_t *p_numFreeBytes				/* returns number of bytes available in ring buffer */
			)
{
	if( rbmgr->writeOffset == rbmgr->readOffset )
	{
		if( rbmgr->readWrapCount == rbmgr->writeWrapCount )
		{
			*p_numFreeBytes = rbmgr->bufferSize;
		}
		else
		{
			*p_numFreeBytes = 0;
		}
	}
	else if( rbmgr->writeOffset < rbmgr->readOffset )
	{
		*p_numFreeBytes = rbmgr->readOffset - rbmgr->writeOffset;
	}
	else if( rbmgr->writeOffset > rbmgr->readOffset )
	{
		*p_numFreeBytes = rbmgr->bufferSize - rbmgr->writeOffset;
		*p_numFreeBytes += rbmgr->readOffset;
	}
	else
	{
		BDBG_ERR(("FATAL ERROR: This should not be possible!"));
	}
}

void BPVRlib_P_RingMgr_Abort( 
			BPVRlib_P_RingMgr_Handle rbmgr				/* handle of ring buffer manager */
			)
{
	/* This function differs from a reset only in that it
	 * doesn't reset any of the data pointers
	 */
	rbmgr->abortInProgress = true;

	BKNI_SetEvent(rbmgr->notifyEvent);
}

void BPVRlib_P_RingMgr_GetHwWriteDifference( 
			BPVRlib_P_RingMgr_Handle rbmgr,		/* handle of ring buffer manager */
			uint8_t **pp_bfr,						/* returns pointer of starting buffer address of data that needs to be added into hardware */
			size_t *p_count						/* returns size of the data that needs to be added into hardware */
			)
{
	*p_count = 0;

	*pp_bfr = (uint8_t *)rbmgr->p_bfr + rbmgr->hwWriteOffset;

	if( rbmgr->hwWriteOffset == rbmgr->writeOffset )
	{
		if( rbmgr->hwWriteWrapCount == rbmgr->writeWrapCount )
		{
			*p_count = 0;
		}
		else
		{
			*p_count = rbmgr->bufferSize - rbmgr->hwWriteOffset;
		}
	}
	else
	{
		if( rbmgr->hwWriteOffset > rbmgr->writeOffset )
		{
			*p_count = rbmgr->bufferSize - rbmgr->hwWriteOffset;
		}
		else
		{
			*p_count = rbmgr->writeOffset - rbmgr->hwWriteOffset;
		}
	}
}	

void BPVRlib_P_RingMgr_UpdateHwWriteCount( 
			BPVRlib_P_RingMgr_Handle rbmgr,	/* handle of ring buffer manager */
			size_t count					/* number of bytes to increment the hw write pointer */
			)
{
	rbmgr->hwWriteOffset += count;

	if( rbmgr->hwWriteOffset >= rbmgr->bufferSize )
	{
		rbmgr->hwWriteWrapCount++;
		rbmgr->hwWriteOffset -= rbmgr->bufferSize;
	}
}

