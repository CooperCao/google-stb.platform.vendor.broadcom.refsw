/***************************************************************************
 * 	Copyright (c) 2003-2005, Broadcom Corporation
 * 	All Rights Reserved
 * 	Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT	BETWEEN THE USER AND BROADCOM.	YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * This file was based upon: /vobs/magnum/syslib/pvrlib/bpvrlib_rec.c@@Hydra_Software_Devel/9
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
#include "bpvrlib_dynrec.h"
#include "bmem.h"

#include "bint.h"
#include "bchp_int_id_xpt_int.h"

BDBG_MODULE(BPVRlib_Rec);

#define BPVRLIB_REC_P_ADD_TO_FREE_LIST_ISR(recHandle, p_descEntry) \
	do{ \
		BLST_SQ_INSERT_TAIL(&((recHandle)->recMasterHandle->freeDescList), p_descEntry, link); \
		(recHandle)->recMasterHandle->numFreeDesc++; \
	}while(0)

#define BPVRLIB_REC_P_ADD_TO_FREE_LIST( recHandle, p_descEntry ) \
	do{ \
		BKNI_EnterCriticalSection(); \
		BPVRLIB_REC_P_ADD_TO_FREE_LIST_ISR(recHandle, p_descEntry); \
		BKNI_LeaveCriticalSection(); \
	}while(0)

#define BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST_ISR( recHandle, p_descEntry ) \
		do{ \
			p_descEntry = BLST_SQ_FIRST(&((recHandle)->recMasterHandle->freeDescList)); \
			BLST_SQ_REMOVE_HEAD( &((recHandle)->recMasterHandle->freeDescList), link ); \
			recHandle->recMasterHandle->numFreeDesc--; \
		}while(0)

#define BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST( recHandle, p_descEntry ) \
	do{ \
		BKNI_EnterCriticalSection(); \
		BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST_ISR(recHandle, p_descEntry); \
		BKNI_LeaveCriticalSection(); \
	}while(0)

#define BPVRLIB_REC_P_ADD_TO_HARDWARE_ISR( err, recHandle, p_descEntry ) \
		do { \
			BXPT_SetLastDescriptorFlag(p_descEntry->p_desc); \
			BLST_SQ_INSERT_TAIL( &recHandle->hwDescList, p_descEntry, link ); \
			recHandle->stats.numInHwDesc++; \
			BDBG_MSG(("Adding %s descriptor",(recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd")); \
			err = BXPT_Record_AddDescriptors(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, p_descEntry->p_desc, p_descEntry->p_desc); \
		}while(0)

#define BPVRLIB_REC_P_ADD_TO_HARDWARE( err, recHandle, p_descEntry ) \
	do { \
		BKNI_EnterCriticalSection(); \
		BPVRLIB_REC_P_ADD_TO_HARDWARE_ISR( err, recHandle, p_descEntry ); \
		BKNI_LeaveCriticalSection(); \
	}while(0)

typedef struct BPVRlib_Rec_P_DescEntry
{
	BLST_SQ_ENTRY(BPVRlib_Rec_P_DescEntry) link;
	BXPT_PvrDescriptor *p_desc;
} BPVRlib_Rec_P_DescEntry;

typedef BLST_SQ_HEAD(BPVRlib_Rec_P_DescList, BPVRlib_Rec_P_DescEntry) BPVRlib_Rec_P_DescList;

typedef struct BPVRlib_Rec_P_MasterHandle
{
	unsigned					usageCount;			/* Number of channels currently allocated */
	uint8_t 					*p_bfr; 			/* Pointer to ring buffer */
	bool						bfrAllocated;		/* false (buffer pointer passed in), true (buffer allocated internally) */
	size_t						bfrSize;			/* Size of ring buffer */
	uint8_t 					*p_descBfr; 		/* Buffer used for descriptor allocation */
	BPVRlib_Rec_P_DescEntry		*p_descEntries; 	/* Buffer used to store descriptor list entries */
	BPVRlib_Rec_P_DescList		freeDescList; 		/* List containing all free descriptors */
	uint32_t					numDesc;			/* Num of descriptors used for Record */
	uint32_t					numFreeDesc;		/* Num of free descriptors available */
	BPVRlib_Rec_CopyMemoryCb 	copyMemCb;			/* Callback used to copy data into the ring buffer */
	BXPT_ChanType 				chanType;			/* Channel type */
	BMEM_Handle 				memHandle;			/* Mem handle */
	BXPT_Handle 				xptHandle;			/* XPT handle */
	BINT_Handle					intHandle;			/* BINT handle */
} BPVRlib_Rec_P_MasterHandle;

typedef struct BPVRlib_Rec_P_Handle
{
	BPVRlib_Rec_MasterHandle 	recMasterHandle; 	/* Pointer to the master handle */
	BXPT_Record_Handle  		xptRecHandle;		/* XPT Record Handle */
	unsigned					chanNum;			/* Channel number */
	bool						isInProgress;		/* Record is in progress */
	bool						allRecordDataFound;	/* Set when last bits of record data is added to ring buffer */
	BKNI_EventHandle 			doneEvent;			/* Event used to signal when the record is done */
	BKNI_EventHandle 			descEvent;			/* Event used to signal when a descriptor is done */
	BPVRlib_Rec_P_DescList 		hwDescList;	  		/* List containing descriptors that are loading in hardware */
	BPVRlib_Rec_P_DescList 		compDescList;		/* List containing descriptors that are loading in hardware */
	uint32_t					curCompDescOffset;	/* Byte offset that has been read from our current completed descriptor */
	uint32_t					numBytesRead;		/* Number of bytes that have been read, but not re-added into hardware */
	uint32_t					timeoutValue;		/* Max number of msec to wait for an event to occur */
	BPVRlib_Rec_Stats			stats;				/* Statistics */
	BINT_CallbackHandle 		hRecInt;			/* cb Handle for rec interrupt */
	BINT_CallbackHandle 		hDoneInt;			/* cb Handle for done interrupt */
	uint32_t					minDesc;			/* Minimum number of descriptors to use with this channel */
} BPVRlib_Rec_P_Handle;

static void BPVRlib_Rec_P_Record_isr( void * pParm1, int parm2 );
static BERR_Code BPVRlib_Rec_P_CreateIntCallbacks( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_EnableInterrupts( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_DisableInterrupts( BPVRlib_Rec_Handle recHandle );
static BERR_Code BPVRlib_Rec_P_GetDescSize( BPVRlib_Rec_Handle recHandle, BXPT_PvrDescriptor *p_desc, size_t *p_descSize );

BERR_Code BPVRlib_Rec_Open(
	BPVRlib_Rec_MasterHandle *pRecMasterHandle, 	/* [out] context handle */
	BXPT_Handle xptHandle,				/* XPT handle */
	BMEM_Handle memHandle,				/* Mem handle */
	BINT_Handle intHandle,				/* Int handle */
	BXPT_ChanType chanType,				/* Record channel type */
	uint8_t *p_bfr, 					/* pointer to buffer that should be used for record buffer, 0 to allocate the buffer internally */
	size_t bfrSize,						/* size of record record buffer (in bytes) */
	uint32_t numDesc,					/* number of descriptors to use (minimum of 2 per record channel to be used) */
	BPVRlib_Rec_CopyMemoryCb copyMemCb	/* function pointer for memory copies (optional) */
	)
{
	BERR_Code		err = BERR_SUCCESS;
	uint32_t		i;
	BXPT_PvrDescriptor	*p_desc;
	BPVRlib_Rec_MasterHandle recHandle;
	size_t descSize = bfrSize / numDesc;
	size_t bfrOffset;

	if( ((bfrSize / numDesc) % 4) || bfrSize == 0 )
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER) ;
		BDBG_ERR(("pvr_recordAllocate(): (buffer size / number of descriptors) must be multiple of 32 bits!"));
		goto done;
	}

	recHandle = BKNI_Malloc( sizeof(BPVRlib_Rec_P_MasterHandle) );
	if( recHandle == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto done;
	}

	BKNI_Memset( recHandle, 0, sizeof(BPVRlib_Rec_P_MasterHandle) );

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
	recHandle->xptHandle	= xptHandle;
	recHandle->memHandle	= memHandle;
	recHandle->chanType		= chanType;
	recHandle->xptHandle	= xptHandle;
	recHandle->intHandle	= intHandle;

	/* Init our lists */
	BLST_SQ_INIT( &recHandle->freeDescList );

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

	bfrOffset = 0;
	/* Assign a descriptor to each descriptor list entry and add them to the free list */
	for( i = 0; i < numDesc; i++ )
	{
		BDBG_ASSERT( bfrOffset + descSize <= bfrSize );
		recHandle->p_descEntries[i].p_desc = &p_desc[i];

		err = BXPT_Record_CreateDesc(recHandle->xptHandle, recHandle->p_descEntries[i].p_desc, p_bfr+bfrOffset, descSize, true, NULL);
		if( err != BERR_SUCCESS )
		{
			goto error;
		}
		BDBG_WRN(("Created descriptor, %08X, %ld", recHandle->p_descEntries[i].p_desc->BufferStartAddr, recHandle->p_descEntries[i].p_desc->BufferLength));

		BLST_SQ_INSERT_TAIL(&recHandle->freeDescList, &recHandle->p_descEntries[i], link);
		recHandle->numFreeDesc++;
		bfrOffset += descSize;
	}
	
done:
	*pRecMasterHandle = recHandle;
	return err;

error:
	BPVRlib_Rec_Close( recHandle );
	return err;
}

BERR_Code BPVRlib_Rec_Close( 
				BPVRlib_Rec_MasterHandle recHandle 		/* context handle */
				)
{
	if( recHandle->usageCount )
	{
		return BERR_TRACE(BERR_UNKNOWN);
	}

	if( recHandle->p_descBfr != NULL ) BMEM_Free( recHandle->memHandle, recHandle->p_descBfr );
	if( recHandle->bfrAllocated == true ) BMEM_Free( recHandle->memHandle, recHandle->p_bfr );
	if( recHandle->p_descEntries != NULL ) BKNI_Free( recHandle->p_descEntries );

	BKNI_Free(recHandle);

	return BERR_SUCCESS;
}

BERR_Code BPVRlib_Rec_ChannelOpen(
	BPVRlib_Rec_Handle *pRecChanHandle, 	/* [out] context handle */
	BPVRlib_Rec_MasterHandle recMasterHandle, /* master handle context */
	BXPT_Record_Handle xptRecHandle,	/* XPT record handle */
	unsigned chanNum,						/* record channel number */
	uint32_t minDesc					/* minimum number of descriptors that should be always allocated for this record channel (0 is auto) */
	)
{
	BPVRlib_Rec_Handle recHandle;
	BERR_Code err;
	BXPT_Capability xptCaps;

	BXPT_GetCapability(recMasterHandle->xptHandle, &xptCaps );

	if( chanNum >= xptCaps.MaxRecords )
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	recHandle = BKNI_Malloc( sizeof(BPVRlib_Rec_P_Handle) );
	if( recHandle == NULL )
	{
		err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
		goto done;
	}

	BKNI_Memset( recHandle, 0, sizeof(BPVRlib_Rec_P_Handle) );

	recHandle->chanNum = chanNum;
	recHandle->xptRecHandle = xptRecHandle;
	recHandle->recMasterHandle = recMasterHandle;
	recHandle->timeoutValue = -1;

	if( minDesc == 0 )
	{
		minDesc = 3;
	}	

	recHandle->minDesc = minDesc;

	/* Init our lists */
	BLST_SQ_INIT( &recHandle->compDescList );
	BLST_SQ_INIT( &recHandle->hwDescList );

	err = BKNI_CreateEvent(&recHandle->doneEvent);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}
	err = BKNI_CreateEvent(&recHandle->descEvent);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}
	err = BPVRlib_Rec_P_CreateIntCallbacks(recHandle);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	BKNI_EnterCriticalSection();
	recHandle->recMasterHandle->usageCount++;
	BKNI_LeaveCriticalSection();
	
done:
	*pRecChanHandle = recHandle;
	return err;

}

BERR_Code BPVRlib_Rec_ChannelClose(
	BPVRlib_Rec_Handle recHandle
	)
{
	if( recHandle->isInProgress )
	{
		return BERR_TRACE(BERR_UNKNOWN);
	}

	BKNI_EnterCriticalSection();
	recHandle->recMasterHandle->usageCount--;
	BKNI_LeaveCriticalSection();	

	if( recHandle->doneEvent ) BKNI_DestroyEvent(recHandle->doneEvent);
	if( recHandle->descEvent ) BKNI_DestroyEvent(recHandle->descEvent);

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

	if( recHandle->isInProgress )
	{
		return BERR_TRACE(BERR_UNKNOWN);
	}

	recHandle->numBytesRead 			= 0;
	recHandle->curCompDescOffset 		= 0;
	recHandle->stats.numDescAdded		= 0;
	recHandle->stats.numDescCompleted	= 0;
	recHandle->stats.numOverFlows		= 0;
	recHandle->stats.numBufferBlocks	= 0;
	recHandle->stats.numTimeouts		= 0;
	recHandle->stats.maxBfrDepth		= 0;
	recHandle->stats.bfrDepth			= 0;

	for( p_descEntry = BLST_SQ_FIRST(&recHandle->compDescList); p_descEntry; p_descEntry = BLST_SQ_FIRST(&recHandle->compDescList) )
	{
		BLST_SQ_REMOVE_HEAD(&recHandle->compDescList, link);
		recHandle->stats.numCompDesc--;
		BPVRLIB_REC_P_ADD_TO_FREE_LIST( recHandle, p_descEntry );
	}

	/* Now load our minimum number of descriptors into hardware */
	while( recHandle->stats.numInHwDesc < recHandle->minDesc )
	{
		BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST( recHandle, p_descEntry );
		if( p_descEntry == NULL )
		{
			BDBG_WRN(("Record started with only %ld descriptors (minimum is supposed to be %ld", recHandle->stats.numInHwDesc, recHandle->minDesc));
			break;
		}

		BDBG_WRN(("Adding descriptor, %08X, %ld", p_descEntry->p_desc->BufferStartAddr, p_descEntry->p_desc->BufferLength));
	
		BPVRLIB_REC_P_ADD_TO_HARDWARE( err, recHandle, p_descEntry );
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
	}

	err = BPVRlib_Rec_P_EnableInterrupts( recHandle );
	if( err != BERR_SUCCESS )
	{
		goto done;
	}

	if( recHandle->recMasterHandle->chanType == BXPT_ChanType_eRecord )
	{
		err = BXPT_Record_StartChannel(recHandle->xptRecHandle);
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
	}
	else
	{
		err = BXPT_Record_StartScdChannel(recHandle->xptRecHandle, BXPT_IndexTable_eStartCode4Word);
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
	BINT_Id recInt;
	BINT_Id doneInt;

	if( recHandle->recMasterHandle->chanType == BXPT_ChanType_eRecord )
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
		}
	}

	err = BINT_CreateCallback(&recHandle->hRecInt, recHandle->recMasterHandle->intHandle, recInt, BPVRlib_Rec_P_Record_isr, recHandle, 0);
	if( err != BERR_SUCCESS )
	{
		goto done;
	}
	err = BINT_CreateCallback(&recHandle->hDoneInt, recHandle->recMasterHandle->intHandle, doneInt, BPVRlib_Rec_P_Record_isr, recHandle, 1);
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

	if( recHandle->recMasterHandle->copyMemCb )
	{
		(*recHandle->recMasterHandle->copyMemCb)( p_bfr, p_srcBfr, srcBfrSize );
	}
	else
	{
		err = BERR_TRACE(BERR_NOT_INITIALIZED);
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


BERR_Code BPVRlib_Rec_P_GetDescSize( BPVRlib_Rec_Handle recHandle, BXPT_PvrDescriptor *p_desc, size_t *p_descSize )
{
	BERR_Code err = BERR_SUCCESS;
	uint32_t physHwWriteAddr;
	uint32_t descSize;

	descSize = p_desc->BufferLength;

	/* If recording has ended we may have a final partial buffer on our complete list */
	if( !recHandle->isInProgress )
	{
		err = BXPT_Record_GetCurrentBufferAddress(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, &physHwWriteAddr);
		if( err != BERR_SUCCESS )
		{
			goto done;
		}

		if( (p_desc->BufferStartAddr < physHwWriteAddr) && 
			((p_desc->BufferStartAddr+p_desc->BufferLength) > physHwWriteAddr) )
		{
			/* Recording is done, so include last bits of data that was recorded by hardware */
			descSize = physHwWriteAddr - p_desc->BufferStartAddr;
		}
	}

	*p_descSize = descSize - recHandle->curCompDescOffset;
	
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
	BPVRlib_Rec_P_DescEntry *p_descEntry;

	*pp_bfr = NULL;
	*p_bfrSize = 0;

	if( !recHandle->isInProgress )
	{
		/* Do not allow blocking when the record is not actively in progress */
		block = 0;
	}

	while( 1 )
	{		
		BKNI_EnterCriticalSection();
		p_descEntry = BLST_SQ_FIRST(&recHandle->compDescList);
		BKNI_LeaveCriticalSection();

		if( p_descEntry )
		{
			err = BMEM_ConvertOffsetToAddress( recHandle->recMasterHandle->memHandle,
				p_descEntry->p_desc->BufferStartAddr+recHandle->curCompDescOffset, 
				(void **)pp_bfr );
			if( err != BERR_SUCCESS )
			{
				break;
			}
			err = BPVRlib_Rec_P_GetDescSize( recHandle, p_descEntry->p_desc, p_bfrSize );
			break;
		}
		else if( !recHandle->isInProgress )
		{
			/* Don't wait for more data, as no more will ever come in! */
			BDBG_MSG(("All data complete"));
			break;
		}
		else if( block )
		{
			recHandle->stats.numBufferBlocks++;
			BDBG_MSG(("Waiting for %s desc event: Line: %ld, free: %d, hwd: %d, comp: %d", (recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", __LINE__, recHandle->recMasterHandle->numFreeDesc, recHandle->stats.numInHwDesc, recHandle->stats.numCompDesc));
			err = BKNI_WaitForEvent( recHandle->descEvent, recHandle->timeoutValue);
			if( err != BERR_SUCCESS )
			{
				if( err == BERR_TIMEOUT )
				{
					recHandle->stats.numTimeouts++;
					BDBG_WRN(("pvr_recordGetDataRequest(): Timeout waiting for descriptor to become available"));
					break;
				}
				else
				{
					BDBG_ERR(("pvr_recordGetDataRequest(): Fatal error waiting for descriptor to become available"));
					break;
				}
			}
			else
			{
				BDBG_MSG(("Got %s desc event", (recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd"));
			}
		}
		else
		{
			break;
		}
	}

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

	BSTD_UNUSED(block);

	*p_overflow = false;

	if( recHandle->isInProgress )
	{
		BXPT_Record_ChannelStatus status;
		
		err = BXPT_Record_GetChannelStatus(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, &status);
		if( err != BERR_SUCCESS )
		{
			goto done;
		}

		if( status.Finished )
		{
			recHandle->stats.numOverFlows++;
			BDBG_WRN(("pvr_recordUpdateReadPointer(): Record channel %ld overflow detected!", recHandle->chanNum));
			*p_overflow = true;
		}
	}

	BKNI_EnterCriticalSection();
	p_descEntry = BLST_SQ_FIRST(&recHandle->compDescList);
	BKNI_LeaveCriticalSection();
	if( p_descEntry )
	{
		uint32_t bfrSize;
		err = BPVRlib_Rec_P_GetDescSize( recHandle, p_descEntry->p_desc, &bfrSize );
		if( err != BERR_SUCCESS )
		{
			goto done;
		}
		
		if( bytesRead > bfrSize )
		{
			err = BERR_TRACE(BERR_INVALID_PARAMETER);
			goto done;
		}
		else if( bytesRead == bfrSize )
		{
			BKNI_EnterCriticalSection();
			BLST_SQ_REMOVE_HEAD(&recHandle->compDescList, link);
			recHandle->stats.numCompDesc--;
			BKNI_LeaveCriticalSection();

			recHandle->curCompDescOffset = 0;

			if( *p_overflow )
			{
				/* 
				When we overflow we want to make sure to process as much data from the
				record channel as possible before starting up the record process.  This
				should help the system settle after it hits an overflow condition
				(otherwise it might get into a scenario when it constantly keeps 
				overflowing and has a hard time recovering).
				*/
				if( recHandle->stats.numCompDesc > 0 )
				{
					BPVRLIB_REC_P_ADD_TO_FREE_LIST( recHandle, p_descEntry );
				}
				else
				{
					/*
					We have now processed all data from this channel.
					Add the minimum descriptors if possible to the record channel.
					*/
					do
					{
						BPVRLIB_REC_P_ADD_TO_HARDWARE( err, recHandle, p_descEntry );
						if( recHandle->stats.numInHwDesc < recHandle->minDesc )
						{
							BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST( recHandle, p_descEntry );
						}
						else
						{
							p_descEntry = NULL;
						}
					} while( p_descEntry );
				}
			}
			else if( recHandle->stats.numInHwDesc < recHandle->minDesc && recHandle->isInProgress )
			{
				/*
				We want to reuse this buffer for this playback channel since we don't have
				our minimum number of descriptors allocated to the channel.  However, we
				need to protect ourselves since the interrupt routine might also be adding
				descriptors (thus the need for critical sections).
				*/
				BPVRLIB_REC_P_ADD_TO_HARDWARE( err, recHandle, p_descEntry );
			}
			else
			{
				BPVRLIB_REC_P_ADD_TO_FREE_LIST( recHandle, p_descEntry );
			}
		}
		else
		{
			recHandle->curCompDescOffset += bytesRead;
		}
	}
	else
	{
		err = BERR_TRACE(BERR_INVALID_PARAMETER);
		goto done;
	}

	recHandle->stats.bfrDepth -= bytesRead;
	recHandle->numBytesRead += bytesRead;

done:
	return err;
}

void BPVRlib_Rec_P_Record_isr( void * pParm1, int parm2 )
{
	BERR_Code err;
	uint32_t physHwWriteAddr;
	uint32_t descSize;
	bool lastDescriptorFound = false;
	BPVRlib_Rec_P_DescEntry *p_descEntry;
	BPVRlib_Rec_Handle recHandle = (BPVRlib_Rec_Handle)pParm1;

	BDBG_MSG(("Got %s %s Interrupt", (recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", parm2?"Done":"Desc" ));

	while( 1 )
	{
		p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList);
		if( p_descEntry == NULL )
		{
			BDBG_MSG(("No hardware descriptors left..."));
			break;
		}

		err = BXPT_Record_GetCurrentBufferAddress(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, &physHwWriteAddr);
		BDBG_ASSERT(err == BERR_SUCCESS);

		BDBG_MSG(("Desc Bfr Addr: %08X, Size: %ld", p_descEntry->p_desc->BufferStartAddr, p_descEntry->p_desc->BufferLength));
		BDBG_MSG(("Cur Write Addr: %08X", physHwWriteAddr));

		if( p_descEntry->p_desc->BufferStartAddr == physHwWriteAddr )
		{
			/* We need to check the finished status.  If it is set, then this descriptor is complete! */
			BXPT_Record_ChannelStatus status;
			
			err = BXPT_Record_GetChannelStatus(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, &status);
			BDBG_ASSERT(err == BERR_SUCCESS);
			
			if( !status.Finished )
			{
				BDBG_MSG(("Hardware descriptor still in progress..."));
				break;
			}
		}
		else if( (p_descEntry->p_desc->BufferStartAddr < physHwWriteAddr) && 
			((p_descEntry->p_desc->BufferStartAddr+p_descEntry->p_desc->BufferLength) > physHwWriteAddr) )
		{
			/* If this is the done interrupt, this descriptor is partially complete, so don't break */
			if( parm2 == 0 )
			{
				BDBG_MSG(("Hardware descriptor still in progress..."));
				break;
			}
			else
			{
				BDBG_MSG(("Found last descriptor..."));
				lastDescriptorFound = true;
			}
		}
		
		BDBG_MSG(("Hardware descriptor complete..."));

		/* This descriptor is complete (or partially complete if the record channel is finished) */
		BLST_SQ_REMOVE_HEAD(&recHandle->hwDescList, link);
		recHandle->stats.numInHwDesc--;

		BLST_SQ_INSERT_TAIL(&recHandle->compDescList, p_descEntry, link);		
		recHandle->stats.numCompDesc++;

		BPVRlib_Rec_P_GetDescSize(recHandle, p_descEntry->p_desc, &descSize);
		recHandle->stats.bfrDepth += descSize;
		recHandle->stats.numBytesRecorded += descSize;

		if( lastDescriptorFound )
		{
			break;
		}
	}

	if( recHandle->stats.bfrDepth > recHandle->stats.maxBfrDepth )
	{
		recHandle->stats.maxBfrDepth = recHandle->stats.bfrDepth;
	}

	if( parm2 ) /* done interrupt */
	{
		recHandle->isInProgress = false;

		BDBG_MSG(("Setting doneEvent for chan %d...", recHandle->chanNum));
		BKNI_SetEvent(recHandle->doneEvent);
	}
	else
	{
		BERR_Code err;
		
		/* Add free descriptors to record if possible */
		for( p_descEntry = BLST_SQ_FIRST(&recHandle->recMasterHandle->freeDescList); p_descEntry && (recHandle->stats.numInHwDesc < recHandle->minDesc); p_descEntry = BLST_SQ_FIRST(&recHandle->recMasterHandle->freeDescList) )
		{
			BPVRLIB_REC_P_REMOVE_FROM_FREE_LIST_ISR(recHandle, p_descEntry);
			BPVRLIB_REC_P_ADD_TO_HARDWARE_ISR(err, recHandle, p_descEntry);
		}	
	}
	
	BDBG_MSG(("Setting descEvent (%#x) for chan %d...", (unsigned)recHandle->descEvent, recHandle->chanNum));
	BKNI_SetEvent(recHandle->descEvent);
}

BERR_Code BPVRlib_Rec_Stop( 
				BPVRlib_Rec_Handle recHandle	/* context handle */
				)
{
	BPVRlib_Rec_P_DescEntry *p_descEntry;
	BERR_Code err = BERR_SUCCESS;

	BXPT_Record_ChannelStatus status;
	
	err = BXPT_Record_GetChannelStatus(recHandle->xptRecHandle, recHandle->recMasterHandle->chanType, &status);
	if( err != BERR_SUCCESS )
	{
		return err;
	}
	
	if( status.Finished )
	{
		BDBG_ERR(("BPVRlib_Rec_Stop(): Overflow must be cleared before stopping record channel!"));
		return BERR_TRACE(BERR_UNKNOWN);
	}

	/* isInProgress is automatically cleared when the record finishes */
	if( recHandle->recMasterHandle->chanType == BXPT_ChanType_eRecord )
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
		BDBG_MSG(("Waiting for %s done event: Line: %ld, free: %d, hwd: %d, comp: %d", (recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd", __LINE__, recHandle->recMasterHandle->numFreeDesc, recHandle->stats.numInHwDesc, recHandle->stats.numCompDesc));
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
		else
		{
			BDBG_MSG(("Got %s done event", (recHandle->recMasterHandle->chanType==BXPT_ChanType_eRecord)?"Rec":"Scd"));
		}
	}

	BDBG_MSG(("Disabling Interrupts"));

	err = BPVRlib_Rec_P_DisableInterrupts(recHandle);
	if( err != BERR_SUCCESS )
	{
		BDBG_ERR(("BPVRlib_Rec_Stop(): Error disabling interrupts"));
	}

	for( p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList); p_descEntry; p_descEntry = BLST_SQ_FIRST(&recHandle->hwDescList) )
	{
		/* Since record is finished we can remove any left over descriptors to the free list */
		BLST_SQ_REMOVE_HEAD(&recHandle->hwDescList, link);
		recHandle->stats.numInHwDesc--;

		BPVRLIB_REC_P_ADD_TO_FREE_LIST( recHandle, p_descEntry );
	}

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
	*p_bfrDepth = recHandle->stats.bfrDepth;
}

BERR_Code BPVRlib_Rec_GetStats( 
				BPVRlib_Rec_Handle recHandle,	/* context handle */
				BPVRlib_Rec_Stats *p_stats 		/* [out] this structure is filled with statistics */
				)
{
	p_stats->numBytesRecorded	= recHandle->stats.numBytesRecorded;
	p_stats->numBytesRemoved	= recHandle->numBytesRead;
	p_stats->numDesc			= recHandle->recMasterHandle->numDesc;
	p_stats->numFreeDesc		= recHandle->recMasterHandle->numFreeDesc;
	p_stats->numInHwDesc		= recHandle->stats.numInHwDesc;
	p_stats->numCompDesc		= recHandle->stats.numCompDesc;
	p_stats->numDescAdded		= recHandle->stats.numDescAdded;
	p_stats->numDescCompleted	= recHandle->stats.numDescCompleted;
	p_stats->numOverFlows		= recHandle->stats.numOverFlows;
	p_stats->numBufferBlocks	= recHandle->stats.numBufferBlocks;
	p_stats->numTimeouts		= recHandle->stats.numTimeouts;
	p_stats->maxBfrDepth		= recHandle->stats.maxBfrDepth;
	p_stats->bfrDepth			= recHandle->stats.bfrDepth;
	p_stats->descSize			= recHandle->recMasterHandle->bfrSize/recHandle->recMasterHandle->numDesc;

	return BERR_SUCCESS;
}


