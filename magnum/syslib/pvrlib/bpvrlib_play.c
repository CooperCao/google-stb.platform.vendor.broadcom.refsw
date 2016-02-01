/***************************************************************************
 *     Copyright (c) 2002-2010, Broadcom Corporation
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
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/

#include "bstd.h"
#include "blst_squeue.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "bpvrlib_p_ringmgr.h"
#include "bpvrlib_play.h"
#include "bmem.h"

#include "bint.h"
#define B_HAS_RAVE 1


BDBG_MODULE(BPVRlib_Play);

typedef enum
{
    BPVRlib_Play_P_BufferModel_eRing,
    BPVRlib_Play_P_BufferModel_eScatterGather
} BPVRlib_Play_P_BufferModel;

typedef struct BPVRlib_Play_P_DescEntry
{
    BLST_SQ_ENTRY(BPVRlib_Play_P_DescEntry) link;
    BXPT_PvrDescriptor *p_desc;
    size_t  skipSize;
    BPVRlib_Play_P_BufferModel bufferModel;
} BPVRlib_Play_P_DescEntry;

typedef BLST_SQ_HEAD(BPVRlib_Play_P_DescList, BPVRlib_Play_P_DescEntry) BPVRlib_Play_P_DescList;

typedef struct BPVRlib_Play_P_Handle
{
    BXPT_Handle         xptHandle;      /* XPT handle */
    BXPT_Playback_Handle xptPlayHandle; /* XPT playback handle */
    BMEM_Handle         memHandle;      /* Mem handle */
    BINT_Handle         intHandle;      /* Int handle */
    BINT_CallbackHandle hPlayInt;       /* cb Handle for playback interrupt */
    volatile bool       isInProgress;   /* Playback is in progress */
    unsigned            channelNum;     /* Channel number */
    BPVRlib_P_RingMgr_Handle    rbmgr;  /* Allocated ring buffer manager */
    uint8_t             *p_bfr;         /* Pointer to ring buffer */
    bool                bfrAllocated;   /* 0 (buffer pointer passed in), 1 (buffer allocated internally) */
    size_t          bfrSize;        /* Size of ring buffer */
    uint8_t             *p_descBfr;     /* Buffer used for descriptor allocation */
    BPVRlib_Play_P_DescEntry    *p_descEntries; /* Buffer used to store descriptor list entries */
    uint32_t            numDesc;        /* Num of descriptors used for playback */
    BKNI_EventHandle    descEvent;      /* Event used to signal when descriptor is added to free list */
    BKNI_EventHandle    finishedEvent;  /* Event used to signal when the playback has finished */
    volatile bool       resync;         /* Used to set the resync bit in the transport playback hardware */
    BPVRlib_Play_P_DescList     freeDescList;   /* List containing all free descriptors */
    BPVRlib_Play_P_DescList     hwDescList;     /* List containing descriptors that are loading in hardware */
    BPVRlib_Play_MemCopyCb      pvrMemCopyCb;   /* Callback used to copy data into the ring buffer */
    uint32_t            totalDescRemoved; /* Total number of descriptors removed from hw list */
    uint32_t            timeoutValue;   /* Max number of msec to wait for an event to occur */
    uint32_t            maxDescSize;    /* Maximum number of bytes to add into hardware at one time */
    BPVRlib_Play_Stats  stats;          /* Statistics structure */
    unsigned            scatGatBfrsComplete;    /* Tracks the current number of scatter gather buffers that have completed */
} BPVRlib_Play_P_Handle;

static void BPVRlib_Play_P_isr( void *Parm1, int Parm2 );
static void BPVRlib_Play_P_UpdateHwDescriptors( BPVRlib_Play_Handle playHandle );
static BERR_Code BPVRlib_Play_P_WaitForFreeDescriptor( BPVRlib_Play_Handle playHandle, BPVRlib_Play_BlockMode block );
static bool BPVRlib_Play_P_IsActive( BPVRlib_Play_Handle playHandle );
static BERR_Code BPVRlib_Play_P_CreateIntCallbacks( BPVRlib_Play_Handle playHandle );
static BERR_Code BPVRlib_Play_P_EnableInterrupts( BPVRlib_Play_Handle playHandle );

BERR_Code BPVRlib_Play_Open(
    BPVRlib_Play_Handle *pPlayHandle,   /* [out] context handle */
    BXPT_Handle xptHandle,              /* XPT handle */
    BXPT_Playback_Handle xptPlayHandle, /* XPT playback handle */
    BMEM_Handle memHandle,              /* Mem handle */
    BINT_Handle intHandle,              /* Int handle */
    unsigned channelNum,                /* playback channel number */
    uint8_t *p_bfr,                     /* pointer to buffer that should be used for playback ring buffer, 0 to allocate the buffer internally */
    size_t bfrSize,                 /* size of playback ring buffer (in bytes) */
    uint32_t numDesc,                   /* number of descriptors to use  */
    BPVRlib_Play_MemCopyCb pvrMemCopyCb         /* function pointer for memory copies (optional) */
    )
{
    BERR_Code       err = BERR_SUCCESS;
    uint32_t        i;
    BXPT_PvrDescriptor  *p_desc;
    BPVRlib_Play_Handle playHandle = NULL;
    BXPT_Capability xptCaps;

    BXPT_GetCapability(xptHandle, &xptCaps );

    if( channelNum >= xptCaps.MaxPlaybacks )
    {
        err = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    playHandle = BKNI_Malloc( sizeof(BPVRlib_Play_P_Handle) );
    if( playHandle == NULL )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    BKNI_Memset( playHandle, 0, sizeof(BPVRlib_Play_P_Handle) );

    if( p_bfr == NULL )
    {
        p_bfr = BMEM_Alloc( memHandle, bfrSize );
        if( p_bfr == NULL )
        {
            err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
            goto error;
        }
        playHandle->bfrAllocated = true;
    }

    /* Save parameters */
    playHandle->p_bfr = p_bfr;
    playHandle->numDesc = numDesc;
    playHandle->pvrMemCopyCb = pvrMemCopyCb;
    playHandle->timeoutValue = BKNI_INFINITE;
    playHandle->maxDescSize = bfrSize / 2;
    playHandle->xptHandle   = xptHandle;
    playHandle->intHandle   = intHandle;
    playHandle->xptPlayHandle = xptPlayHandle;
    playHandle->memHandle   = memHandle;
    playHandle->channelNum  = channelNum;

    /* Init our lists */
    BLST_SQ_INIT( &playHandle->freeDescList );
    BLST_SQ_INIT( &playHandle->hwDescList );

    /* Descriptors must be 16 byte aligned so allocate an extra 15 bytes */
    playHandle->p_descBfr = BMEM_AllocAligned( playHandle->memHandle, (sizeof(BXPT_PvrDescriptor)*numDesc), 4 /* 16 byte aligned */, 0 );
    p_desc = (BXPT_PvrDescriptor *)playHandle->p_descBfr;
    if( p_desc == NULL )
    {
        err = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
        goto error;
    }

    /* Allocate storage for our descriptor entries */
    playHandle->p_descEntries = BKNI_Malloc( sizeof(BPVRlib_Play_P_DescEntry) * numDesc );
    if( playHandle->p_descEntries == NULL )
    {
        err = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    /* Assign a descriptor to each descriptor list entry and add them to the free list */
    for( i = 0; i < numDesc; i++ )
    {
        playHandle->p_descEntries[i].p_desc = &p_desc[i];
        BLST_SQ_INSERT_HEAD(&playHandle->freeDescList, &playHandle->p_descEntries[i], link);
        playHandle->stats.numFreeDesc++;
    }

    err = BPVRlib_P_RingMgr_Open( &playHandle->rbmgr, channelNum, 0, 0, p_bfr, bfrSize );
    if( err != BERR_SUCCESS )
    {
        goto error;
    }

    err = BKNI_CreateEvent(&playHandle->descEvent);
    if( err != BERR_SUCCESS )
    {
        goto error;
    }
    err = BKNI_CreateEvent(&playHandle->finishedEvent);
    if( err != BERR_SUCCESS )
    {
        goto error;
    }
    err = BPVRlib_Play_P_CreateIntCallbacks(playHandle);
    if( err != BERR_SUCCESS )
    {
        goto error;
    }

done:
    *pPlayHandle = playHandle;
    return err;

error:
    BPVRlib_Play_Close( playHandle );
    return err;
}

BERR_Code BPVRlib_Play_Close(
                BPVRlib_Play_Handle playHandle      /* context handle */
                )
{
    if( playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    if (playHandle->hPlayInt) BINT_DestroyCallback(playHandle->hPlayInt);

    if( playHandle->rbmgr != NULL ) BPVRlib_P_RingMgr_Close( playHandle->rbmgr );
    if( playHandle->p_descBfr != NULL ) BMEM_Free( playHandle->memHandle, playHandle->p_descBfr );
    if( playHandle->bfrAllocated == true ) BMEM_Free( playHandle->memHandle, playHandle->p_bfr );
    if( playHandle->p_descEntries != NULL ) BKNI_Free( playHandle->p_descEntries );
    if( playHandle->descEvent != NULL ) BKNI_DestroyEvent(playHandle->descEvent);
    if( playHandle->finishedEvent != NULL ) BKNI_DestroyEvent(playHandle->finishedEvent);

    BKNI_Free( playHandle );

    return BERR_SUCCESS;
}

BERR_Code BPVRlib_Play_GetEvents(BPVRlib_Play_Handle playHandle, BPVRlib_Play_Events *events)
{
    events->descEvent = playHandle->descEvent;
    events->finishedEvent = playHandle->finishedEvent;
    return BERR_SUCCESS;
}

BERR_Code BPVRlib_Play_Start( BPVRlib_Play_Handle playHandle )
{
    BERR_Code err = BERR_SUCCESS;

    if( playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BKNI_Memset( &playHandle->stats, 0, sizeof( BPVRlib_Play_Stats ) );
    playHandle->stats.numFreeDesc = playHandle->numDesc;
    playHandle->scatGatBfrsComplete = 0;

    err = BPVRlib_Play_P_EnableInterrupts( playHandle );
    if( err != BERR_SUCCESS )
    {
        goto done;
    }

    playHandle->resync = true;

    err = BXPT_Playback_StartChannel(playHandle->xptPlayHandle);
    if( err != BERR_SUCCESS )
    {
        goto done;
    }

    playHandle->isInProgress = true;

done:
    return err;
}

BERR_Code BPVRlib_Play_P_CreateIntCallbacks( BPVRlib_Play_Handle playHandle )
{
    BINT_Id playInt;

	playInt = BXPT_Playback_GetIntId(playHandle->xptPlayHandle, BXPT_PbInt_eDone);
    return BINT_CreateCallback(&playHandle->hPlayInt, playHandle->intHandle, playInt, BPVRlib_Play_P_isr, playHandle, 0);
}

BERR_Code BPVRlib_Play_P_EnableInterrupts( BPVRlib_Play_Handle playHandle )
{
    return BINT_EnableCallback( playHandle->hPlayInt );
}

BERR_Code BPVRlib_Play_AddData( BPVRlib_Play_Handle playHandle, uint8_t *p_bfr, size_t *p_bfrSize, BPVRlib_Play_BlockMode block )
{
    uint32_t numBytesCopied = 0;
    uint8_t *p_destBfr;
    size_t destBfrSize;
    BERR_Code err = BERR_SUCCESS;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    while( *p_bfrSize )
    {
        err = BPVRlib_Play_AddDataRequest( playHandle, &p_destBfr, &destBfrSize, block );
        BDBG_MSG(("BPVRlib_Play_AddData(): %d bytes available", destBfrSize));

        if( destBfrSize == 0 || err != BERR_SUCCESS )
        {
            if( err == BERR_TIMEOUT)
            {
                goto done;
            }
        }

        if( destBfrSize > *p_bfrSize )
        {
            destBfrSize = *p_bfrSize;
        }

        if( playHandle->pvrMemCopyCb )
        {
            (*playHandle->pvrMemCopyCb)( p_destBfr, p_bfr, destBfrSize );
        }
        else
        {
            err = BERR_TRACE(BERR_NOT_INITIALIZED);
            goto done;
        }

        BDBG_MSG(("BPVRlib_Play_AddData(): adding %d bytes", destBfrSize));
        err = BPVRlib_Play_UpdateWritePointer( playHandle, destBfrSize, block );
        if( err != BERR_SUCCESS )
        {
            goto done;
        }

        p_bfr += destBfrSize;
        numBytesCopied += destBfrSize;
        *p_bfrSize -= destBfrSize;
    }

    *p_bfrSize = numBytesCopied;

done:
    return err;
}

BERR_Code BPVRlib_Play_AddDataRequest( BPVRlib_Play_Handle playHandle, uint8_t **pp_bfr, size_t *p_bfrSize, BPVRlib_Play_BlockMode block )
{
    BERR_Code err = BERR_SUCCESS;

    *p_bfrSize = 0;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    while( true )
    {
        BPVRlib_Play_CommitBuffersToHardware( playHandle, BPVRlib_Play_BlockMode_eNone );

        if( block == BPVRlib_Play_BlockMode_eFreeDescriptor )
        {
            err = BPVRlib_Play_P_WaitForFreeDescriptor( playHandle, block );
            if( err != BERR_SUCCESS )
            {
                return err;
            }
        }
        else
        {
            BPVRlib_Play_P_UpdateHwDescriptors( playHandle );
        }

        if( (err = BPVRlib_P_RingMgr_AddDataRequest( playHandle->rbmgr, pp_bfr, p_bfrSize, 0, 0 )) != BERR_SUCCESS )
        {
            return err;
        }

        if( *p_bfrSize == 0 && block )
        {
            playHandle->stats.numBufferBlocks++;
            if (playHandle->timeoutValue==0) {
                return BERR_TIMEOUT;
            }
            BDBG_MSG(("Waiting event: Line: %ld.", __LINE__));
            err = BKNI_WaitForEvent( playHandle->descEvent, playHandle->timeoutValue);
            if( err != BERR_SUCCESS )
            {
                if( err == BERR_TIMEOUT)
                {
                    playHandle->stats.numTimeouts++;
                    BDBG_ERR(("BPVRlib_Play_AddDataRequest(): Timeout waiting for descriptor to become available"));
                    return err;
                }
                else
                {
                    BDBG_ERR(("BPVRlib_Play_AddDataRequest(): Error waiting for descriptor to become available"));
                    return err;
                }
            }

            if( !playHandle->isInProgress )
            {
                BDBG_ERR(("BPVRlib_Play_AddDataRequest(): Playback no longer in progress"));
                return err;
            }
        }
        else
        {
            break;
        }
    }

    /* Use return code to specify if free descriptor is available. */
    err = BPVRlib_Play_P_WaitForFreeDescriptor( playHandle, BPVRlib_Play_BlockMode_eNone );

    return err;
}

BERR_Code BPVRlib_Play_UpdateWritePointer( BPVRlib_Play_Handle playHandle, size_t bfrSize, BPVRlib_Play_BlockMode block )
{
    uint8_t *p_ringBfrAddr;
    size_t ringBfrSize;
    BERR_Code err = BERR_SUCCESS;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MSG(("Updating write pointer by %ld bytes for playback channel %ld. Block = %d", bfrSize, playHandle->channelNum, block));

    /* Make sure we really have space in the ring buffer */
    BPVRlib_P_RingMgr_AddDataRequest( playHandle->rbmgr, &p_ringBfrAddr, &ringBfrSize, 0 /* no block */, 0 /* no timeout*/ );

    /* Sanity check */
    BDBG_ASSERT( ringBfrSize >= bfrSize );

    /* Update the write pointer after we have the descriptor to keep an accurate
     * estimate of how much valid data is contained in the ring buffer.
     */
    BPVRlib_P_RingMgr_UpdateWritePointer( playHandle->rbmgr, bfrSize );

    /* There may be times when we want to add data to the buffer, but not
     * actually play it back until additional data is added.  Therefore,
     * do not commit the data to hardware in these cases. */
    if( block != BPVRlib_Play_BlockMode_eNoCommit )
    {
        err = BPVRlib_Play_CommitBuffersToHardware( playHandle, block );

        if( err == BERR_TIMEOUT && block != BPVRlib_Play_BlockMode_eFreeDescriptor )
        {
            /* This is not really an error since we didn't block */
            err = BERR_SUCCESS;
        }
    }

    return err;
}

BERR_Code BPVRlib_Play_UpdateWritePointerWithSkip( BPVRlib_Play_Handle playHandle, size_t bfrSize, size_t skipSize, BPVRlib_Play_BlockMode block )
{
    BPVRlib_Play_P_DescEntry *p_descEntry;
    uint8_t *p_ringBfrAddr;
    size_t ringBfrSize;
    BERR_Code err = BERR_SUCCESS;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MSG(("Updating write pointer by %ld bytes with skip %ld for playback channel %ld.", bfrSize, skipSize, playHandle->channelNum));

    /* All previously added data must be committed to hardware so we can properly program in the skip size */
    err = BPVRlib_Play_CommitBuffersToHardware( playHandle, block );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

    /* Make sure we really have space in the ring buffer */
    BPVRlib_P_RingMgr_AddDataRequest( playHandle->rbmgr, &p_ringBfrAddr, &ringBfrSize, 0 /* no block */, 0 /* no timeout*/ );

    /* Sanity check */
    BDBG_ASSERT( ringBfrSize >= bfrSize );

    err = BPVRlib_Play_P_WaitForFreeDescriptor( playHandle, block );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

    BDBG_ASSERT( !BLST_SQ_EMPTY(&playHandle->freeDescList) );

    /* Now that the data is in the ring buffer, add the data into hardware */
    /* Remove the first free descriptor */
    p_descEntry = BLST_SQ_FIRST( &playHandle->freeDescList );
    BLST_SQ_REMOVE_HEAD( &playHandle->freeDescList, link );
    playHandle->stats.numFreeDesc--;

    BDBG_ASSERT( p_descEntry );
    BDBG_ASSERT( p_descEntry->p_desc );

    /* Update the write pointer after we have the descriptor to keep an accurate
     * estimate of how much valid data is contained in the ring buffer.
     */
    BPVRlib_P_RingMgr_UpdateWritePointer( playHandle->rbmgr, bfrSize );

    BXPT_Playback_CreateDesc(playHandle->xptHandle, p_descEntry->p_desc, p_ringBfrAddr+skipSize, bfrSize-skipSize, true /* int enable */, playHandle->resync, NULL);
    p_descEntry->skipSize = skipSize;
    p_descEntry->bufferModel = BPVRlib_Play_P_BufferModel_eRing;

    playHandle->resync = false;

    /* Add our descriptor entry to the hardware list */
    BLST_SQ_INSERT_TAIL( &playHandle->hwDescList, p_descEntry, link );
    playHandle->stats.numInHwDesc++;

    /* Add this buffer into the hardware */
    BXPT_Playback_AddDescriptors( playHandle->xptPlayHandle, p_descEntry->p_desc, p_descEntry->p_desc );

    playHandle->stats.numDescAdded++;

    BPVRlib_P_RingMgr_UpdateHwWriteCount( playHandle->rbmgr, bfrSize );

    return err;

}

BERR_Code BPVRlib_Play_CommitBuffersToHardware( BPVRlib_Play_Handle playHandle, BPVRlib_Play_BlockMode block )
{
    BPVRlib_Play_P_DescEntry *p_descEntry;
    BERR_Code err = BERR_SUCCESS;
    uint8_t *p_bfrAddr;
    size_t bfrSize;

    BDBG_MSG(("Commiting buffers to hardware for playback channel %ld. Block = %d", playHandle->channelNum, block));

    BPVRlib_P_RingMgr_GetHwWriteDifference( playHandle->rbmgr, &p_bfrAddr, &bfrSize );

    while( bfrSize )
    {
        /* We don't want our descriptors to get too big, otherwise our buffer efficency will go down too low */
        if( bfrSize > playHandle->maxDescSize )
        {
            bfrSize = playHandle->maxDescSize;
        }

        err = BPVRlib_Play_P_WaitForFreeDescriptor( playHandle, block );
        if( err != BERR_SUCCESS )
        {
            return err;
        }

        BDBG_ASSERT( !BLST_SQ_EMPTY(&playHandle->freeDescList) );

        /* Now that the data is in the ring buffer, add the data into hardware */
        /* Remove the first free descriptor */
        p_descEntry = BLST_SQ_FIRST( &playHandle->freeDescList );
        BLST_SQ_REMOVE_HEAD( &playHandle->freeDescList, link );
        playHandle->stats.numFreeDesc--;

        BDBG_ASSERT( p_descEntry );
        BDBG_ASSERT( p_descEntry->p_desc );

        BXPT_Playback_CreateDesc(playHandle->xptHandle, p_descEntry->p_desc, p_bfrAddr, bfrSize, true /* int enable */, playHandle->resync, NULL);
        p_descEntry->skipSize = 0;
        p_descEntry->bufferModel = BPVRlib_Play_P_BufferModel_eRing;

        playHandle->resync = false;

        /* Add our descriptor entry to the hardware list */
        BLST_SQ_INSERT_TAIL( &playHandle->hwDescList, p_descEntry, link );
        playHandle->stats.numInHwDesc++;

        /* Add this buffer into the hardware */
        BXPT_Playback_AddDescriptors( playHandle->xptPlayHandle, p_descEntry->p_desc, p_descEntry->p_desc );

        playHandle->stats.numDescAdded++;

        BPVRlib_P_RingMgr_UpdateHwWriteCount( playHandle->rbmgr, bfrSize );
        BPVRlib_P_RingMgr_GetHwWriteDifference( playHandle->rbmgr, &p_bfrAddr, &bfrSize );
    }

    return err;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/ /**
This function check if descriptors are available
for loading into the playback hardware.  If
not the function can block until one becomes available.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
BERR_Code BPVRlib_Play_P_WaitForFreeDescriptor( BPVRlib_Play_Handle playHandle, BPVRlib_Play_BlockMode block )
{
    BERR_Code err = BERR_SUCCESS;

    BDBG_MSG(("Waiting for free descriptor on playback channel %ld. Block = %d", playHandle->channelNum, block));

    BPVRlib_Play_P_UpdateHwDescriptors( playHandle );

    /* Wait for a free descriptor to become available */
    while( BLST_SQ_EMPTY(&playHandle->freeDescList) )
    {
        if( block != BPVRlib_Play_BlockMode_eFreeDescriptor )
        {
            /* We cannot block, but there are no descriptors available, so we must return */
            return BERR_TIMEOUT;
        }

        playHandle->stats.numDescBlocks++;
        if (playHandle->timeoutValue==0) {
            return BERR_TIMEOUT;
        }

        BDBG_MSG(("Waiting event: Line: %ld.", __LINE__));
        err = BKNI_WaitForEvent( playHandle->descEvent, playHandle->timeoutValue);
        if( err != BERR_SUCCESS )
        {
            if( err == BERR_TIMEOUT )
            {
                playHandle->stats.numTimeouts++;
                BDBG_ERR(("BPVRlib_Play_P_WaitForFreeDescriptor(): Timeout waiting for descriptor to become available"));
                return err;
            }
            else
            {
                BDBG_ERR(("BPVRlib_Play_P_WaitForFreeDescriptor(): Error waiting for descriptor to become available"));
                return err;
            }
        }

        if( !playHandle->isInProgress )
        {
            BDBG_ERR(("BPVRlib_Play_P_WaitForFreeDescriptor(): Playback no longer in progress"));
            return err;
        }

        BPVRlib_Play_P_UpdateHwDescriptors( playHandle );
    }

    return err;
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/ /**
This function is called when a transport playback
interrupt is signalled.  It simply sets one (or two)
events to communicate to the task level code that a
descriptor has completed.
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void BPVRlib_Play_P_isr( void *Parm1, int Parm2 )
{
    BXPT_Playback_ChannelStatus status;
    BPVRlib_Play_Handle playHandle = (BPVRlib_Play_Handle)Parm1;

    BSTD_UNUSED(Parm2);

    BDBG_MSG(("Got interrupt for playback channel %ld.", playHandle->channelNum));

    BXPT_Playback_GetChannelStatus( playHandle->xptPlayHandle, &status );

    if( status.Finished )
    {
        playHandle->stats.numUnderFlows++;
        BKNI_SetEvent( playHandle->finishedEvent );
        BDBG_MSG(("BPVRlib_Play_P_isr(): Playback channel %ld underflowed!", playHandle->channelNum));
    }

    BKNI_SetEvent( playHandle->descEvent );
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/ /**
This function frees any data
(updates the ring buffer read pointer) that has now
completed playback through the hardware.  It also
places newly completed descriptors on the free list
and wakes any tasks that may be waiting for one to
free up (if the free list was previously empty).
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void BPVRlib_Play_P_UpdateHwDescriptors( BPVRlib_Play_Handle playHandle )
{
    uint32_t numDoneDesc = 0;
    bool    descInUse = false;
    uint32_t descSize;
    BPVRlib_Play_P_DescEntry *p_descEntry;

    BDBG_MSG(("Updating hardware descriptors for playback channel %ld.",playHandle->channelNum));

    while( !descInUse )
    {
        /* Make sure we have entries in the hardware list */
        if( BLST_SQ_EMPTY(&playHandle->hwDescList) )
        {
            BDBG_MSG(("BPVRlib_Play_P_UpdateHwDescriptors(): Hardware descriptor list is empty"));
            descInUse = true;   /* exit the while loop */
            continue;
        }

        /* Check if the head descriptor is complete */
        p_descEntry = BLST_SQ_FIRST( &playHandle->hwDescList );

        /* Sanity Check */
        BDBG_ASSERT(p_descEntry);
        BDBG_ASSERT(p_descEntry->p_desc);

        BXPT_Playback_CheckHeadDescriptor(playHandle->xptPlayHandle, p_descEntry->p_desc, &descInUse, &descSize );

        if( descInUse )
        {
            BDBG_MSG(("Head descriptor still in use."));
            /* exit the while loop because there are no more complete descriptors */
            continue;
        }

        /* Remove the first descriptor from the hardware list */
        p_descEntry = BLST_SQ_FIRST(&playHandle->hwDescList);
        BLST_SQ_REMOVE_HEAD( &playHandle->hwDescList, link );
        playHandle->stats.numInHwDesc--;

        BDBG_ASSERT( p_descEntry );
        BDBG_ASSERT( p_descEntry->p_desc );

        if( p_descEntry->bufferModel == BPVRlib_Play_P_BufferModel_eRing )
        {
            descSize += p_descEntry->skipSize;
            BDBG_MSG(("BPVRlib_Play_P_UpdateHwDescriptors(): Updating read pointer by %d bytes.", descSize));
            BPVRlib_P_RingMgr_UpdateReadPointer( playHandle->rbmgr, descSize );
        }
        else
        {
            playHandle->scatGatBfrsComplete++;
            playHandle->stats.numBytesScatGat -= descSize;
            playHandle->stats.numScatGatDesc--;
        }

        playHandle->stats.numDescCompleted++;

        numDoneDesc++;
        if( numDoneDesc == 2 )
        {
            playHandle->stats.numMultiDescDone++;
        }

        /* Place the descriptor back on the free list */
        BLST_SQ_INSERT_TAIL( &playHandle->freeDescList, p_descEntry, link );
        playHandle->stats.numFreeDesc++;
    }
}

BERR_Code BPVRlib_Play_Stop( BPVRlib_Play_Handle playHandle )
{
    BPVRlib_Play_P_DescEntry *p_descEntry;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MSG(("Stopping playback channel %ld.", playHandle->channelNum));

    playHandle->isInProgress = false;

    BXPT_Playback_StopChannel(playHandle->xptPlayHandle);

    /* Stopping the ring buffer will wake any threads waiting to feed more data into the ring buffer */
    BPVRlib_P_RingMgr_Reset( playHandle->rbmgr );

    /* Wake any threads waiting for a free descriptor */
    BKNI_SetEvent( playHandle->descEvent );
    BKNI_SetEvent( playHandle->finishedEvent );

    /* Move all of the descriptors from the hardware list to the free list */
    while( !BLST_SQ_EMPTY( &playHandle->hwDescList ) )
    {
        p_descEntry = BLST_SQ_FIRST(&playHandle->hwDescList);
        BLST_SQ_REMOVE_HEAD( &playHandle->hwDescList, link );
        playHandle->stats.numInHwDesc--;

        BDBG_ASSERT( p_descEntry );
        BDBG_ASSERT( p_descEntry->p_desc );

        BLST_SQ_INSERT_TAIL( &playHandle->freeDescList, p_descEntry, link );
        playHandle->stats.numFreeDesc++;
    }

    return BERR_SUCCESS;
}

BERR_Code BPVRlib_Play_Flush( BPVRlib_Play_Handle playHandle )
{
#if !B_HAS_RAVE
    BXPT_Playback_ChannelSettings settings;
    uint8_t *p_bfr;
    size_t bfrSize = 0;
#endif

    BERR_Code err = BERR_SUCCESS;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MSG(("Flushing playback channel %ld", playHandle->channelNum));

    err = BPVRlib_Play_Stop( playHandle );
    if( err != BERR_SUCCESS )
    {
        return err;
    }
    err = BPVRlib_Play_Start( playHandle );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

#if !B_HAS_RAVE
    err = BXPT_Playback_GetChannelSettings( playHandle->xptPlayHandle, &settings );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

    if( settings.SyncMode == BXPT_PB_SYNC_MPEG )
    {
        /* Insert a null packet to flush the video decoder once we start up again */
        if( (BPVRlib_Play_AddDataRequest( playHandle, &p_bfr, &bfrSize, BPVRlib_Play_BlockMode_eNone ) == BERR_SUCCESS)
            && (bfrSize >= settings.PacketLength) )
        {
            BPVRlib_Play_GenerateTsNullPacket( p_bfr );
            BPVRlib_Play_UpdateWritePointer( playHandle, settings.PacketLength, BPVRlib_Play_BlockMode_eNoCommit );
        }
        else
        {
            BDBG_ERR(("BPVRlib_Play_Flush(): Unable to send NULL packet!"));
        }
    }
#endif

    if( BPVRlib_Play_P_IsActive( playHandle ) )
    {
        BDBG_ERR(("BPVRlib_Play_Flush(): Playback active immediately after flush!!"));
        return BERR_UNKNOWN;
    }

    return err;
}

BERR_Code BPVRlib_Play_GetBufferDepth( BPVRlib_Play_Handle playHandle, size_t *p_bfrDepth )
{
    /* This will cause the state to be applied and the status to be up-to-date. */
    BPVRlib_Play_CommitBuffersToHardware( playHandle, BPVRlib_Play_BlockMode_eNone );
    BPVRlib_Play_P_UpdateHwDescriptors( playHandle );
    BPVRlib_P_RingMgr_GetNumFreeBytes( playHandle->rbmgr, p_bfrDepth );
    *p_bfrDepth = playHandle->rbmgr->bufferSize - *p_bfrDepth;
    return BERR_SUCCESS;
}

/***********************************************************/ /**
This function checks it the playback channel is active.
****************************************************************/
bool BPVRlib_Play_P_IsActive( BPVRlib_Play_Handle playHandle )
{
    BXPT_Playback_ChannelStatus status;

    BXPT_Playback_GetChannelStatus( playHandle->xptPlayHandle, &status );

    if( status.Run && !status.Finished )
        return true;
    else
        return false;
}

BERR_Code BPVRlib_Play_WaitForFinished( BPVRlib_Play_Handle playHandle )
{
    BERR_Code err = BERR_SUCCESS;

    if( !playHandle->isInProgress )
    {
        return BERR_TRACE(BERR_UNKNOWN);
    }

    BDBG_MSG(("Waiting for playback channel %ld to complete.", playHandle->channelNum));

    BPVRlib_Play_CommitBuffersToHardware( playHandle, BPVRlib_Play_BlockMode_eFreeDescriptor );

    while( BPVRlib_Play_P_IsActive( playHandle ) )
    {
        BDBG_MSG(("Waiting event: Line: %ld.", __LINE__));
        err = (playHandle->timeoutValue?BKNI_WaitForEvent( playHandle->finishedEvent, playHandle->timeoutValue):BERR_TIMEOUT);
        if( err != BERR_SUCCESS )
        {
            if( err == BERR_TIMEOUT)
            {
                playHandle->stats.numTimeouts++;
                BDBG_ERR(("BPVRlib_Play_WaitForFinished(): Timeout waiting for playback to finish"));
                return err;
            }
            else
            {
                BDBG_ERR(("BPVRlib_Play_WaitForFinished(): Error waiting for playback to finish"));
                return err;
            }
        }
    }

    return err;
}

BERR_Code BPVRlib_Play_SetTimeout( BPVRlib_Play_Handle playHandle, uint32_t numMsec )
{
    playHandle->timeoutValue = numMsec;

    return BERR_SUCCESS;
}

BERR_Code BPVRlib_Play_GetTimeout( BPVRlib_Play_Handle playHandle, uint32_t *p_numMsec )
{
    *p_numMsec = playHandle->timeoutValue;

    return BERR_SUCCESS;
}

BERR_Code BPVRlib_Play_GetStats( BPVRlib_Play_Handle playHandle, BPVRlib_Play_Stats *p_stats )
{
    uint8_t *dummy;
    size_t count;

    /* This will cause the state to be applied and the status to be up-to-date. */
    BPVRlib_Play_CommitBuffersToHardware( playHandle, BPVRlib_Play_BlockMode_eNone );
    BPVRlib_Play_P_UpdateHwDescriptors( playHandle );

    p_stats->numBytesPlayed = (playHandle->rbmgr->bufferSize * playHandle->rbmgr->readWrapCount) + playHandle->rbmgr->readOffset;
    p_stats->numBytesAdded      = (playHandle->rbmgr->bufferSize * playHandle->rbmgr->writeWrapCount) + playHandle->rbmgr->writeOffset;
    p_stats->bfrSize            = playHandle->rbmgr->bufferSize;
    p_stats->readOffset     = playHandle->rbmgr->readOffset;
    p_stats->writeOffset        = playHandle->rbmgr->writeOffset;
    p_stats->hwWriteOffset      = playHandle->rbmgr->hwWriteOffset;

    BPVRlib_P_RingMgr_GetHwWriteDifference( playHandle->rbmgr, &dummy, &count );
    p_stats->numBytesNotCommitted = count;

    p_stats->numFreeDesc        = playHandle->stats.numFreeDesc;
    p_stats->numInHwDesc        = playHandle->stats.numInHwDesc;
    p_stats->numDesc            = playHandle->numDesc;
    p_stats->numDescAdded       = playHandle->stats.numDescAdded;
    p_stats->numDescCompleted   = playHandle->stats.numDescCompleted;
    p_stats->numUnderFlows      = playHandle->stats.numUnderFlows;
    p_stats->numBufferBlocks    = playHandle->stats.numBufferBlocks;
    p_stats->numDescBlocks      = playHandle->stats.numDescBlocks;
    p_stats->numTimeouts        = playHandle->stats.numTimeouts;

        p_stats->numBytesScatGat        = playHandle->stats.numBytesScatGat;
        p_stats->numScatGatDesc         = playHandle->stats.numScatGatDesc;
        p_stats->numMultiDescDone       = playHandle->stats.numMultiDescDone;

    return BERR_SUCCESS;
}

void BPVRlib_Play_GenerateTsNullPacket( uint8_t *pkt )
{
    int i;

    pkt[0] = 0x47;   /* SYNC BYTE */
    pkt[1] = 0x1f;
    pkt[2] = 0xff;  /* PID */
    pkt[3] = 0x10;
    for(i=4; i<188; ++i)
    {
        pkt[i] = 0xff;
    }
}

BERR_Code BPVRlib_Play_AddBuffer( BPVRlib_Play_Handle playHandle,   const uint8_t *p_bfr, const size_t bfrSize, BPVRlib_Play_BlockMode block )
{
    BPVRlib_Play_P_DescEntry *p_descEntry;
    BERR_Code err = BERR_SUCCESS;

    BDBG_MSG(("Adding buffer to hardware for playback channel %ld. Block = %d", playHandle->channelNum, block));

    err = BPVRlib_Play_CommitBuffersToHardware( playHandle, block );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

    err = BPVRlib_Play_P_WaitForFreeDescriptor( playHandle, block );
    if( err != BERR_SUCCESS )
    {
        return err;
    }

    BDBG_ASSERT( !BLST_SQ_EMPTY(&playHandle->freeDescList) );

    /* Remove the first free descriptor */
    p_descEntry = BLST_SQ_FIRST( &playHandle->freeDescList );
    BLST_SQ_REMOVE_HEAD( &playHandle->freeDescList, link );
    playHandle->stats.numFreeDesc--;

    BDBG_ASSERT( p_descEntry );
    BDBG_ASSERT( p_descEntry->p_desc );

    BXPT_Playback_CreateDesc(playHandle->xptHandle, p_descEntry->p_desc, (uint8_t *)p_bfr, bfrSize, true /* int enable */, playHandle->resync, NULL);
    p_descEntry->skipSize = 0;
    p_descEntry->bufferModel = BPVRlib_Play_P_BufferModel_eScatterGather;

    playHandle->resync = false;

    /* Add our descriptor entry to the hardware list */
    BLST_SQ_INSERT_TAIL( &playHandle->hwDescList, p_descEntry, link );
    playHandle->stats.numInHwDesc++;
    playHandle->stats.numScatGatDesc++;
    playHandle->stats.numBytesScatGat += bfrSize;

    /* Add this buffer into the hardware */
    BXPT_Playback_AddDescriptors( playHandle->xptPlayHandle, p_descEntry->p_desc, p_descEntry->p_desc );

    playHandle->stats.numDescAdded++;

    return err;
}

BERR_Code BPVRlib_Play_IsBufferComplete( BPVRlib_Play_Handle playHandle, unsigned *p_bfrsCompleted, BPVRlib_Play_BlockMode block )
{
    BERR_Code err = BERR_SUCCESS;

    BPVRlib_Play_P_UpdateHwDescriptors( playHandle );

    if( block != BPVRlib_Play_BlockMode_eNone )
    {
        while( playHandle->scatGatBfrsComplete == 0 && playHandle->stats.numScatGatDesc != 0 )
        {
            BDBG_MSG(("Waiting event: Line: %ld.", __LINE__));
            err = BKNI_WaitForEvent( playHandle->descEvent, playHandle->timeoutValue);
            if( err != BERR_SUCCESS )
            {
                if( err == BERR_TIMEOUT )
                {
                    playHandle->stats.numTimeouts++;
                    BDBG_ERR(("BPVRlib_Play_IsBufferComplete(): Timeout waiting for descriptor to complete"));
                    return err;
                }
                else
                {
                    BDBG_ERR(("BPVRlib_Play_IsBufferComplete(): Error waiting for descriptor to complete"));
                    return err;
                }
            }
            BPVRlib_Play_P_UpdateHwDescriptors( playHandle );
        }
    }

    *p_bfrsCompleted = playHandle->scatGatBfrsComplete;
    playHandle->scatGatBfrsComplete = 0;

    return BERR_SUCCESS;
}

