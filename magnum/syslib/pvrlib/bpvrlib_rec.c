/***************************************************************************
 * 	Copyright (c) 2003-2006, Broadcom Corporation
 * 	All Rights Reserved
 * 	Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT	BETWEEN THE USER AND BROADCOM.	YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * This file was based upon: /SetTop/syslib/pvrlib/7115/pvrlib_rec.c@@Irvine_BSEAVSW_Devel/28
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 *************************************************************/

#include "bstd.h"
#include "blst_squeue.h"
#include "bkni.h"
#include "bpvrlib_p_ringmgr.h"
#include "bpvrlib_rec.h"
#include "bmem.h"

#include "bint.h"
#include "bchp_int_id_xpt_int.h"

BDBG_MODULE(BPVRlib_Rec);

/* 
TODO: 
1. Modify transport PI to use single function for both type of record channels 
2. Move open settings to a structure
3. Move enable and disable interrupts to XPT PI
*/

typedef struct BPVRlib_Rec_P_DescEntry
{
	BLST_SQ_ENTRY(BPVRlib_Rec_P_DescEntry) link;
	BXPT_PvrDescriptor *p_desc;
} BPVRlib_Rec_P_DescEntry;

typedef BLST_SQ_HEAD(BPVRlib_Rec_P_DescList, BPVRlib_Rec_P_DescEntry) BPVRlib_Rec_P_DescList;

typedef struct BPVRlib_Rec_P_Handle
{
	volatile bool		isInProgress;			/* Record is in progress */
	bool				allRecordDataFound;		/* Set when last bits of record data is added to ring buffer */
	BPVRlib_P_RingMgr_Handle		rbmgr;					/* Pointer to allocated ring buffer manager */
	uint8_t				*p_bfr;					/* Pointer to ring buffer */
	bool				bfrAllocated;			/* false (buffer pointer passed in), true (buffer allocated internally) */
	size_t			bfrSize;				/* Size of ring buffer */
	uint8_t				*p_descBfr;				/* Buffer used for descriptor allocation */
	BPVRlib_Rec_P_DescEntry		*p_descEntries;	/* Buffer used to store descriptor list entries */
	uint32_t			numDesc;				/* Num of descriptors used for Record */
	BKNI_EventHandle 	doneEvent;				/* Event used to signal when the record is done */
	BKNI_EventHandle 	descEvent;				/* Event used to signal when a descriptor is done */
	BPVRlib_Rec_P_DescList freeDescList; /* List containing all free descriptors */
	BPVRlib_Rec_P_DescList hwDescList;	  /* List containing descriptors that are loading in hardware */
	BPVRlib_Rec_CopyMemoryCb copyMemCb;		/* Callback used to copy data into the ring buffer */
	uint32_t			numBytesRead;			/* Number of bytes that have been read, but not re-added into hardware */
	uint32_t			timeoutValue;			/* Max number of msec to wait for an event to occur */
	BPVRlib_Rec_Stats	stats;					/* Statistics */
	BXPT_ChanType 		chanType;			/* Channel type */
	unsigned			chanNum;				/* Channel number */
	BMEM_Handle 		memHandle;				/* Mem handle */
	BXPT_Handle 		xptHandle;				/* XPT handle */
	BXPT_Record_Handle  xptRecHandle;			/* XPT Record Handle */
	BINT_Handle			intHandle;				/* BINT handle */
	BINT_CallbackHandle hRecInt;				/* cb Handle for rec interrupt */
	BINT_CallbackHandle hDoneInt;				/* cb Handle for done interrupt */
	BXPT_IndexTable		indexType;				/* index type to record */
} BPVRlib_Rec_P_Handle;

static void BPVRlib_Rec_P_isr( void * pParm1, int parm2 );
static void BPVRlib_Rec_P_UpdateHwDescriptors( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_CreateIntCallbacks( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_EnableInterrupts( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_DisableInterrupts( BPVRlib_Rec_Handle recHandle );

BERR_Code BPVRlib_Rec_Open(
	BPVRlib_Rec_Handle *pRecHandle,		/* [out] context handle */
	BXPT_Handle xptHandle,				/* XPT handle */
	BXPT_Record_Handle xptRecHandle,	/* XPT Record handle */
	BMEM_Handle memHandle,				/* Mem handle */
	BINT_Handle intHandle,				/* Int handle */
	BXPT_ChanType chanType,	/* Record channel type */
	unsigned chanNum,						/* record channel number */
	uint8_t *p_bfr,						/* pointer to buffer that should be used for record ring buffer, 0 to allocate the buffer internally */
	size_t bfrSize,					/* size of record ring buffer (in bytes) */
	uint32_t numDesc,					/* number of descriptors to use  */
	BPVRlib_Rec_CopyMemoryCb copyMemCb	/* function pointer for memory copies (optional) */
				)
{
	BERR_Code		err = BERR_SUCCESS;
	uint32_t		i;
	BXPT_PvrDescriptor	*p_desc;
	BPVRlib_Rec_Handle recHandle = NULL;
	BXPT_Capability xptCaps;

	BXPT_GetCapability( xptHandle, &xptCaps );

	if( chanNum >= xptCaps.MaxRecords )
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	if( ((bfrSize / numDesc) % 4) || bfrSize == 0 )
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		BDBG_ERR(("pvr_recordAllocate(): (buffer size / number of descriptors) must be multiple of 32 bits!"));
		goto done;
	}

	recHandle = BKNI_Malloc( sizeof(BPVRlib_Rec_P_Handle) );
	if( recHandle == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto done;
	}

	BKNI_Memset( recHandle, 0, sizeof(BPVRlib_Rec_P_Handle) );

	if( p_bfr == NULL )
	{
		p_bfr = BMEM_Alloc( memHandle, bfrSize );
		if( p_bfr == NULL )
		{
			err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
			goto error;
		}
		recHandle->bfrAllocated = true;
	}

	/* Save parameters */
	recHandle->p_bfr		= p_bfr;
	recHandle->bfrSize		= bfrSize;
	recHandle->numDesc		= numDesc;
	recHandle->copyMemCb	= copyMemCb;
	recHandle->timeoutValue	= 0xFFFFFFFF;
	recHandle->xptHandle	= xptHandle;
	recHandle->xptRecHandle = xptRecHandle;
	recHandle->memHandle	= memHandle;
	recHandle->chanType		= chanType;
	recHandle->chanNum 		= chanNum;
	recHandle->xptHandle	= xptHandle;
	recHandle->intHandle	= intHandle;
	recHandle->indexType	= BXPT_IndexTable_eStartCode4Word;

	/* Init our lists */
	BLST_SQ_INIT( &recHandle->freeDescList );
	BLST_SQ_INIT( &recHandle->hwDescList );

	/* Descriptors must be 16 byte aligned so allocate an extra 15 bytes */
	recHandle->p_descBfr = BMEM_AllocAligned( recHandle->memHandle, (sizeof(BXPT_PvrDescriptor)*numDesc), 4 /* 16 byte aligned */, 0 );
	p_desc = (BXPT_PvrDescriptor *)recHandle->p_descBfr;
	if( p_desc == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
		goto error;		
	}

	/* Allocate storage for our descriptor entries */
	recHandle->p_descEntries = BKNI_Malloc( sizeof(BPVRlib_Rec_P_DescEntry) * numDesc );
	if( recHandle->p_descEntries == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto error;		
	}

	/* Assign a descriptor to each descriptor list entry and add them to the free list */
	for( i = 0; i < numDesc; i++ )
	{
		recHandle->p_descEntries[i].p_desc = &p_desc[i];
		BLST_SQ_INSERT_HEAD(&recHandle->freeDescList, &recHandle->p_descEntries[i], link);
		recHandle->stats.numFreeDesc++;
	}

	err = BPVRlib_P_RingMgr_Open( &recHandle->rbmgr, chanNum, 0, 0, p_bfr, bfrSize );
	if( err != BERR_SUCCESS )
	{
		goto error;
	}
	err = BKNI_CreateEvent(&recHandle->doneEvent);
	if( err != BERR_SUCCESS )
	{
		goto error;
	}
	err = BKNI_CreateEvent(&recHandle->descEvent);
	if( err != BERR_SUCCESS )
	{
		goto error;
	}
	err = BPVRlib_Rec_P_CreateIntCallbacks(recHandle);
	if( err != BERR_SUCCESS )
	{
		goto error;
	}

done:
	*pRecHandle = recHandle;
	return err;

error:
	BPVRlib_Rec_Close( recHandle );
	return err;
}

BERR_Code BPVRlib_Rec_Close(
				BPVRlib_Rec_Handle recHandle 		/* context handle */
				)
{
	if( recHandle->isInProgress )
	{
		return BERR_TRACE(BERR_UNKNOWN);
	}

	if (recHandle->hRecInt) BINT_DestroyCallback(recHandle->hRecInt);
	if (recHandle->hDoneInt) BINT_DestroyCallback(recHandle->hDoneInt);

	if( recHandle->rbmgr != NULL ) BPVRlib_P_RingMgr_Close( recHandle->rbmgr );
	if( recHandle->p_descBfr != NULL ) BMEM_Free( recHandle->memHandle, recHandle->p_descBfr );
	if( recHandle->bfrAllocated == true ) BMEM_Free( recHandle->memHandle, recHandle->p_bfr );
	if( recHandle->p_descEntries != NULL ) BKNI_Free( recHandle->p_descEntries );
	if( recHandle->doneEvent != NULL ) BKNI_DestroyEvent(recHandle->doneEvent);
	if( recHandle->descEvent != NULL ) BKNI_DestroyEvent(recHandle->descEvent);

	BKNI_Free(recHandle);

	return BERR_SUCCESS;
}

BERR_Code 
BPVRlib_Rec_GetEvents(BPVRlib_Rec_Handle recHandle, BPVRlib_Rec_Events *events)
{
	events->descEvent = recHandle->descEvent;
	events->finishedEvent = recHandle->doneEvent;
	return BERR_SUCCESS;
}

BERR_Code BPVRlib_Rec_Start( 
				BPVRlib_Rec_Handle recHandle		/* context handle */
				)
{
	BERR_Code		err = BERR_SUCCESS;
	BPVRlib_Rec_P_DescEntry *p_descEntry;
	size_t bfrSize;
    uint32_t bfrOffset;
	uint8_t *p_bfr;

	if( recHandle->isInProgress )
	{
		return BERR_TRACE(BERR_UNKNOWN);
	}

	BPVRlib_P_RingMgr_Reset( recHandle->rbmgr );
	recHandle->numBytesRead = 0;

	recHandle->stats.numDescAdded		= 0;
	recHandle->stats.numDescCompleted	= 0;
	recHandle->stats.numOverFlows		= 0;
	recHandle->stats.numBufferBlocks	= 0;
	recHandle->stats.numTimeouts		= 0;
	recHandle->stats.maxBfrDepth		= 0;

	/* Move all of the descriptors from the hardware list to the free list */
	while( !BLST_SQ_EMPTY(&recHandle->hwDescList) )
	{
		p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList);
		BLST_SQ_REMOVE_HEAD(&recHandle->hwDescList, link);
		recHandle->stats.numInHwDesc--;
		BLST_SQ_INSERT_HEAD(&recHandle->freeDescList, p_descEntry, link);
		recHandle->stats.numFreeDesc++;
	}

	bfrOffset = 0;

	/* We know this divides evenly because we checked it as part of the allocate routine */
	bfrSize = recHandle->bfrSize / recHandle->numDesc;

	/* Now load all of our descriptors into hardware */
	while( !BLST_SQ_EMPTY(&recHandle->freeDescList) )
	{
		BDBG_ASSERT( bfrOffset + bfrSize <= recHandle->bfrSize );

		p_descEntry = BLST_SQ_FIRST(&recHandle->freeDescList);
		BLST_SQ_REMOVE_HEAD( &recHandle->freeDescList, link );
		recHandle->stats.numFreeDesc--;
		if( p_descEntry == NULL )
		{
			err = BERR_TRACE(BERR_UNKNOWN);
			goto done;
		}

		err = BXPT_Record_CreateDesc(recHandle->xptHandle, p_descEntry->p_desc, recHandle->p_bfr+bfrOffset, bfrSize, true, NULL);
		BLST_SQ_INSERT_TAIL( &recHandle->hwDescList, p_descEntry, link );
		recHandle->stats.numInHwDesc++;

		if( err != BERR_SUCCESS )
		{
			goto done;
		}

		BDBG_MSG(("Adding %s descriptor",(recHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd")); 
		err = BXPT_Record_AddDescriptors(recHandle->xptRecHandle, recHandle->chanType, p_descEntry->p_desc, p_descEntry->p_desc);

		if( err != BERR_SUCCESS )
		{
			goto done;
		}

		bfrOffset += bfrSize;
	}

	BDBG_ASSERT( bfrOffset == recHandle->bfrSize );

	/* We need to kick start the ring buffer manager */
	err = BPVRlib_P_RingMgr_AddDataRequest( recHandle->rbmgr, &p_bfr, &bfrSize, 0, 0 );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	err = BPVRlib_Rec_P_EnableInterrupts( recHandle );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	if( recHandle->chanType == BXPT_ChanType_eRecord )
	{
		err = BXPT_Record_StartChannel(recHandle->xptRecHandle);
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
	}
	else
	{
		err = BXPT_Record_StartScdChannel(recHandle->xptRecHandle, recHandle->indexType);
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
	}

	/*
	It is okay to set these flags after the record hardware has started for the following reasons:
	These flags are only touched by non-isr functions (which must be serialed with this function, 
	or when the done interrupt is set by the hardware.  The done interrupt	will never be set 
	until after the record channel is stopped.
	*/
	recHandle->isInProgress = true;
	recHandle->allRecordDataFound = false;

done:
	return err;
}

BERR_Code BPVRlib_Rec_P_CreateIntCallbacks( BPVRlib_Rec_Handle recHandle )
{
	BERR_Code err = BERR_SUCCESS;

	/* For each record channel we need to enable the record and done interrupts */
	BINT_Id recInt = BCHP_INT_ID_REC0_INT;
	BINT_Id doneInt = BCHP_INT_ID_REC0_DONE;

	if( recHandle->chanType == BXPT_ChanType_eRecord )
	{
		switch( recHandle->chanNum )
		{
			case 0:
				recInt = BCHP_INT_ID_REC0_INT;
				doneInt = BCHP_INT_ID_REC0_DONE;
				break;
			case 1:
				recInt = BCHP_INT_ID_REC1_INT;
				doneInt = BCHP_INT_ID_REC1_DONE;
				break;
			case 2:
				recInt = BCHP_INT_ID_REC2_INT;
				doneInt = BCHP_INT_ID_REC2_DONE;
				break;
			default:
				err = BERR_TRACE(BERR_UNKNOWN);
				goto done;
		}
	}
	else
	{
		switch( recHandle->chanNum )
		{
			case 0:
				recInt = BCHP_INT_ID_SCD0_INT;
				doneInt = BCHP_INT_ID_SCD0_DONE;
				break;
			case 1:
				recInt = BCHP_INT_ID_SCD1_INT;
				doneInt = BCHP_INT_ID_SCD1_DONE;
				break;
			case 2:
				recInt = BCHP_INT_ID_SCD2_INT;
				doneInt = BCHP_INT_ID_SCD2_DONE;
				break;
			default:
				err = BERR_TRACE(BERR_UNKNOWN);
				goto done;
		}
	}

	err = BINT_CreateCallback(&recHandle->hRecInt, recHandle->intHandle, recInt, BPVRlib_Rec_P_isr, recHandle, 0);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}
	err = BINT_CreateCallback(&recHandle->hDoneInt, recHandle->intHandle, doneInt, BPVRlib_Rec_P_isr, recHandle, 1);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

done:
	return err;
}

BERR_Code BPVRlib_Rec_P_EnableInterrupts( BPVRlib_Rec_Handle recHandle )
{
	BERR_Code err = BERR_SUCCESS;

	err = BINT_EnableCallback( recHandle->hDoneInt );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	err = BINT_EnableCallback( recHandle->hRecInt );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

done:
	return err;
}

BERR_Code BPVRlib_Rec_GetData(
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				uint8_t *p_bfr,				/* [out,sizeof(*p_bfrSize)] pointer to destination buffer */
				size_t *p_bfrSize,		/* [in,out] size of destination buffer (in bytes), returns number of bytes actually copied */
				bool block,					/* false (do not block), true (block until data is available) */
				bool *p_overflow			/* [out] returns 1 if record overflow has occurred */
				)
{
	BERR_Code err = BERR_SUCCESS;
	uint8_t *p_srcBfr;
	size_t srcBfrSize;

	*p_overflow = false;

	err = BPVRlib_Rec_GetDataRequest( recHandle, &p_srcBfr, &srcBfrSize, block );

	if( srcBfrSize > *p_bfrSize )
	{
		srcBfrSize = *p_bfrSize;
	}

	/* Make sure to init this variable before returing due to errors */
	*p_bfrSize = 0;

	if( srcBfrSize == 0 || err != BERR_SUCCESS )
	{
		goto done;
	}

	if( recHandle->copyMemCb )
	{
		(*recHandle->copyMemCb)( p_bfr, p_srcBfr, srcBfrSize );
	}
	else
	{
		err = BERR_NOT_INITIALIZED;
		BDBG_ERR(("pvr_recordGetData(): No memory copy callback installed!  Unable to add data."));
		goto done;
	}

	err = BPVRlib_Rec_UpdateReadPointer( recHandle, srcBfrSize, block, p_overflow );
	if( err != BERR_SUCCESS )
	{
		BDBG_WRN(("BPVRlib_Rec_GetData(): No blocking requirement caused write pointer update to timeout"));
		goto done;
	}

	*p_bfrSize = srcBfrSize;

done:
	return err;
}

BERR_Code BPVRlib_Rec_GetDataRequest( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				uint8_t **pp_bfr, 				/* [out] returns pointer to the start of valid data */
				size_t *p_bfrSize,			/* [out] returns size of the valid data (in bytes) */ 
				bool block						/* 0 (do not block), 1 (block until data is available) */
				)
{
	BERR_Code err = BERR_SUCCESS;

	if( !recHandle->isInProgress )
	{
		/* Do not allow blocking when the record is not actively in progress */
		block = 0;
	}

	while( 1 )
	{
		BPVRlib_Rec_P_UpdateHwDescriptors( recHandle );

		if( (err = BPVRlib_P_RingMgr_RemoveDataRequest( recHandle->rbmgr, pp_bfr, p_bfrSize, 0, recHandle->timeoutValue )) != BERR_SUCCESS )
		{
			/* A timeout isn't really an error as it will occur when we stop the record */
			if( recHandle->isInProgress || err != BERR_TIMEOUT )
			{
				BDBG_ERR(("BPVRlib_Rec_GetDataRequest(): Error removing data from ring buffer"));
			}
			goto done;
		}

		if( !recHandle->isInProgress )
		{
			/* Don't wait for more data, as no more will ever come in! */
			BDBG_MSG(("BPVRlib_Rec_GetDataRequest(): Record no longer in progress"));
			goto done;
		}

		if( *p_bfrSize == 0 && block )
		{
			recHandle->stats.numBufferBlocks++;
			BDBG_MSG(("Waiting for %s event: Line: %ld, fd: %d, hwd: %d", (recHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", __LINE__, recHandle->stats.numFreeDesc, recHandle->stats.numInHwDesc));
			err = BKNI_WaitForEvent( recHandle->descEvent, recHandle->timeoutValue);
			if( err != BERR_SUCCESS )
			{
				if( err == BERR_TIMEOUT )
				{
					recHandle->stats.numTimeouts++;
					BDBG_WRN(("pvr_recordGetDataRequest(): Timeout waiting for descriptor to become available"));
					goto done;
				}
				else
				{
					BDBG_ERR(("pvr_recordGetDataRequest(): Fatal error waiting for descriptor to become available"));
					goto done;
				}
			}
		}
		else
		{
			break;
		}
	}


done:
	return err;
}

BERR_Code BPVRlib_Rec_UpdateReadPointer( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				size_t bytesRead,				/* number of bytes that have been read from ring buffer */
				bool block,						/* not currently used */
				bool *p_overflow				/* [out] returns true if record overflow has occurred */
				)
{	
	BPVRlib_Rec_P_DescEntry *p_descEntry;
	BERR_Code err = BERR_SUCCESS;
	BXPT_Record_ChannelStatus status;

	BSTD_UNUSED(block);

	err = BXPT_Record_GetChannelStatus(recHandle->xptRecHandle, recHandle->chanType, &status);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	if( status.Finished )
	{
		recHandle->stats.numOverFlows++;
		BDBG_WRN(("pvr_recordUpdateReadPointer(): Record channel %ld overflow detected!", recHandle->chanNum));
		*p_overflow = true;
	} else {
		*p_overflow = false;
	}

	err = BPVRlib_P_RingMgr_UpdateReadPointer( recHandle->rbmgr, bytesRead );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	recHandle->numBytesRead += bytesRead;

	if( !recHandle->isInProgress )
	{
		/* Since we are not actively recording, simply return as we don't need to load any buffers into the hardware */
		goto done;
	}

	/* Check if we've removed enough data to allow us to reload a descriptor (or two) */
	while( recHandle->numBytesRead >= recHandle->bfrSize/recHandle->numDesc )
	{
		/* By definition a descriptor should be on the free list */
		BDBG_ASSERT( !BLST_SQ_EMPTY(&recHandle->freeDescList) );

		/* Remove the first free descriptor */
		p_descEntry = BLST_SQ_FIRST(&recHandle->freeDescList);
		BLST_SQ_REMOVE_HEAD( &recHandle->freeDescList, link );
		recHandle->stats.numFreeDesc--;

		/* We need to remove the next descriptor pointer before reloading it! */
		BXPT_SetLastDescriptorFlag(p_descEntry->p_desc);

		/* Add our descriptor entry to the hardware list */
		BLST_SQ_INSERT_TAIL( &recHandle->hwDescList, p_descEntry, link );
		recHandle->stats.numInHwDesc++;

		/* Add this buffer into the hardware */
		BDBG_MSG(("Adding %s descriptor",(recHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd"));
		BDBG_MSG(("0x%08X 0x%08X 0x%08X 0x%08X",((uint32_t*)(p_descEntry->p_desc))[0], ((uint32_t*)(p_descEntry->p_desc))[1], ((uint32_t*)(p_descEntry->p_desc))[2], ((uint32_t*)(p_descEntry->p_desc))[3]));
		BXPT_Record_AddDescriptors(recHandle->xptRecHandle, recHandle->chanType, p_descEntry->p_desc, p_descEntry->p_desc);

		recHandle->stats.numDescAdded++;
		recHandle->numBytesRead -= recHandle->bfrSize/recHandle->numDesc;
	}

done:
	return err;
}

void BPVRlib_Rec_P_isr( void * pParm1, int parm2 )
{
	BPVRlib_Rec_Handle recHandle = (BPVRlib_Rec_Handle)pParm1;

	BDBG_MSG(("Got %s %s Interrupt", (recHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", parm2?"Done":"Desc" ));

	BKNI_SetEvent(recHandle->descEvent);

	if( parm2 ) /* done interrupt */
	{
		recHandle->isInProgress = false;
		BKNI_SetEvent(recHandle->doneEvent);
	}
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+ INPUTS:	chanType - ePvrRecord or ePvrScd channel
+			chanNum - record channel number
+ OUTPUTS:	None
+ RETURNS:	None
+ FUNCTION: This function updates the ring buffer write pointer with
+			the amount of data completed in the descriptor.  It also
+			searches the descriptor list for all completed descriptors
+			in case more than one has completed since the last interrupt.
+			This function is also called after the record done interrupt occurs
+			to update the ring buffer with the last partial amount
+			of data that was recorded though a single recursion
+			call.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void BPVRlib_Rec_P_UpdateHwDescriptors( BPVRlib_Rec_Handle recHandle )
{
	bool	descIsComplete = true;
	BPVRlib_Rec_P_DescEntry *p_descEntry;
	uint8_t *p_bfr;
	size_t bfrSize;
	uint32_t descSize;
	uint32_t physHwWriteAddr;
	size_t bufferDepth;

	while( descIsComplete )
	{
		if( recHandle->allRecordDataFound )
		{
			descIsComplete = false;	/* exit the while loop */
			continue;
		}

		/* Make sure we have entries in the hardware list */
		if( BLST_SQ_EMPTY(&recHandle->hwDescList) )
		{
			BDBG_MSG(("BPVRlib_Rec_P_UpdateHwDescriptors(): Hardware descriptor list is empty"));
			descIsComplete = false;	/* exit the while loop */
			continue;
		}

		BXPT_Record_GetCurrentBufferAddress(recHandle->xptRecHandle, recHandle->chanType, &physHwWriteAddr);

		/* Check if the head descriptor is complete */
		p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList);

		descSize = p_descEntry->p_desc->BufferLength;

		if( (p_descEntry->p_desc->BufferStartAddr <= physHwWriteAddr)
			&& ((p_descEntry->p_desc->BufferStartAddr+p_descEntry->p_desc->BufferLength) > physHwWriteAddr) )
		{
			BXPT_Record_ChannelStatus status;
			
			BXPT_Record_GetChannelStatus(recHandle->xptRecHandle, recHandle->chanType, &status);
			
			if( status.Finished && p_descEntry->p_desc->BufferStartAddr == physHwWriteAddr )
			{
				/* We are completely full and in an overflow condition */
				descIsComplete = true;
			}
			else if( recHandle->isInProgress )
			{
				/* Hardware is currently processing this descriptor */
				descIsComplete = false;
			}
			else
			{
				BDBG_MSG(("BPVRlib_Rec_P_UpdateHwDescriptors(): All record data has been stored"));
				/* Recording is done, so include last bits of data that was recorded by hardware */
				descSize = physHwWriteAddr - p_descEntry->p_desc->BufferStartAddr;
				recHandle->allRecordDataFound = true;
			}
		}

		if( !descIsComplete )
		{
			/* exit the while loop because there are no more complete descriptors */
			continue;
		}

		/* Remove the first descriptor from the hardware list */
		p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList);
		BLST_SQ_REMOVE_HEAD( &recHandle->hwDescList, link );
		recHandle->stats.numInHwDesc--;

		recHandle->stats.numDescCompleted++;

		/* Place the descriptor back on the free list */
		BLST_SQ_INSERT_TAIL( &recHandle->freeDescList, p_descEntry, link );
		recHandle->stats.numFreeDesc++;

		BPVRlib_P_RingMgr_AddDataRequest( recHandle->rbmgr, &p_bfr, &bfrSize, 0, 0 );

		/* Sanity check */
		BDBG_ASSERT( bfrSize >= descSize );
		BDBG_ASSERT( (BMEM_ConvertAddressToOffset( recHandle->memHandle, p_bfr, &physHwWriteAddr ), physHwWriteAddr == p_descEntry->p_desc->BufferStartAddr) );

		BPVRlib_P_RingMgr_UpdateWritePointer( recHandle->rbmgr, descSize );
	}
	
	/* Store our max buffer depth size */
	BPVRlib_Rec_GetBufferDepth(recHandle, &bufferDepth);
	if( bufferDepth > recHandle->stats.maxBfrDepth )
	{
		recHandle->stats.maxBfrDepth = bufferDepth;
	}
}

BERR_Code BPVRlib_Rec_Stop( 
				BPVRlib_Rec_Handle recHandle	/* context handle */
				)
{
	BERR_Code err = BERR_SUCCESS;

	/* isInProgress is automatically cleared when the record finishes */
	if( recHandle->chanType == BXPT_ChanType_eRecord )
	{
		err = BXPT_Record_StopChannel(recHandle->xptRecHandle);
		if( err != BERR_SUCCESS )
		{
			BDBG_ERR(("BPVRlib_Rec_Stop(): Timeout waiting for record to finish"));
		}
	}
	else
	{
		err = BXPT_Record_StopScdChannel(recHandle->xptRecHandle);
		if( err != BERR_SUCCESS )
		{
			BDBG_ERR(("BPVRlib_Rec_Stop(): Timeout waiting for record to finish"));
		}
	}

	if( recHandle->isInProgress )
	{
		BDBG_MSG(("Waiting for %s event: Line: %ld, fd: %d, hwd: %d", (recHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", __LINE__, recHandle->stats.numFreeDesc, recHandle->stats.numInHwDesc));
		err = BKNI_WaitForEvent(recHandle->doneEvent, recHandle->timeoutValue);
		if( err != BERR_SUCCESS )
		{
			if( err == BERR_TIMEOUT )
			{
				recHandle->stats.numTimeouts++;
				BDBG_ERR(("BPVRlib_Rec_Stop(): Timeout waiting for record to finish"));
			}
			else
			{
				BDBG_ERR(("BPVRlib_Rec_Stop(): Fatal error waiting for record to finish"));
			}
		}
	}

	err = BPVRlib_Rec_P_DisableInterrupts(recHandle);
	if( err != BERR_SUCCESS )
	{
		BDBG_ERR(("BPVRlib_Rec_Stop(): Error disabling interrupts"));
	}

	/* Wake up a task that might be waiting for record data! */
	BKNI_SetEvent(recHandle->descEvent);

	return err;
}


BERR_Code BPVRlib_Rec_P_DisableInterrupts( BPVRlib_Rec_Handle recHandle )
{
		BERR_Code err = BERR_SUCCESS;
		
		err = BINT_DisableCallback( recHandle->hDoneInt );
		if( err != BERR_SUCCESS )
		{
			goto done;
		}

		err = BINT_DisableCallback( recHandle->hRecInt );
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
	
	done:
		return err;
}


void BPVRlib_Rec_SetTimeout( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				uint32_t numMsec				/* timeout value in msec */
				)
{
	recHandle->timeoutValue = numMsec;
}

void BPVRlib_Rec_GetTimeout( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				uint32_t *p_numMsec 			/* [out] returns current timeout value in msec */
				)
{
	*p_numMsec = recHandle->timeoutValue;
}

void BPVRlib_Rec_GetBufferDepth( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				size_t *p_bfrDepth			/* [out] number of valid bytes in the ring buffer */ 
				)
{
	BPVRlib_P_RingMgr_GetNumFreeBytes( recHandle->rbmgr, p_bfrDepth );
	*p_bfrDepth = recHandle->bfrSize - *p_bfrDepth;
}

BERR_Code BPVRlib_Rec_GetStats(
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				BPVRlib_Rec_Stats *p_stats 		/* [out] this structure is filled with statistics */
				)
{
	BPVRlib_Rec_P_UpdateHwDescriptors( recHandle );
	
	p_stats->numBytesRecorded	= (recHandle->rbmgr->bufferSize * recHandle->rbmgr->writeWrapCount) + recHandle->rbmgr->writeOffset;
	p_stats->numBytesRemoved	= (recHandle->rbmgr->bufferSize * recHandle->rbmgr->readWrapCount) + recHandle->rbmgr->readOffset;
	p_stats->bfrSize			= recHandle->rbmgr->bufferSize;
	p_stats->readOffset		= recHandle->rbmgr->readOffset;
	p_stats->writeOffset		= recHandle->rbmgr->hwWriteOffset;
	p_stats->numDesc			= recHandle->numDesc;
	p_stats->numFreeDesc		= recHandle->stats.numFreeDesc;
	p_stats->numInHwDesc		= recHandle->stats.numInHwDesc;
	p_stats->numDescAdded		= recHandle->stats.numDescAdded;
	p_stats->numDescCompleted	= recHandle->stats.numDescCompleted;
	p_stats->numOverFlows		= recHandle->stats.numOverFlows;
	p_stats->numBufferBlocks	= recHandle->stats.numBufferBlocks;
	p_stats->numTimeouts		= recHandle->stats.numTimeouts;
	p_stats->maxBfrDepth		= recHandle->stats.maxBfrDepth;

	return BERR_SUCCESS;
}

void BPVRlib_Rec_SetIndexType( 
	BPVRlib_Rec_Handle recHandle,	/* context handle */
	BXPT_IndexTable indexType		/* type of index data to record (default is BXPT_IndexTable_eStartCode4Word) */
	)
{
	recHandle->indexType	= indexType;
}

