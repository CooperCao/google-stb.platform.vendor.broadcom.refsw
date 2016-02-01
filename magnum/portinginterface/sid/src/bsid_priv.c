/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 *
 ******************************************************************************/

#include "bstd.h"
#include "berr.h"
#include "bkni.h"
#include "bkni_event_group.h"
#include "bpxl.h"

#include "bsid.h"
#include "bsid_priv.h"
#include "bsid_platform.h"
#include "bsid_msg.h"

BDBG_MODULE(BSID_PRIV);
BDBG_FILE_MODULE(BSID_MEM);

/* max time (in milliseconds) to wait on Channel Sync before erroring out */
#define BSID_SYNC_MAX_WAITING_TIME                                     5000

/******************************************************************************
* Function name: BSID_P_AllocateLock
*
* Comments:
* alignment is the required alignment of the memory in bytes
******************************************************************************/
BERR_Code BSID_P_AllocLock(BMMA_Heap_Handle hMma, BSID_LinearBuffer * buf, uint32_t size, uint32_t alignment, const char *str)
{
#ifndef BDBG_DEBUG_BUILD
    BSTD_UNUSED(str);
#endif
    if (buf == NULL)
    {
        BDBG_ERR(("Buffer is null"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

    buf->ui32_Size = size;

    buf->hBlock = BMMA_Alloc(hMma, buf->ui32_Size, alignment, NULL);
    if (buf->hBlock == NULL)
    {
        BDBG_ERR(("BMMA_Alloc failed"));
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    buf->pv_CachedAddr = BMMA_Lock(buf->hBlock);
    if (buf->pv_CachedAddr == NULL)
    {
        BMMA_Free(buf->hBlock);
        buf->hBlock = NULL;
        BDBG_ERR(("BMMA_Lock failed"));
        return BERR_TRACE(BERR_UNKNOWN);
    }

   /* NOTE: This really returns an offset not an address.
            This call cannot fail, and 0 is a valid offset */
    buf->ui32_PhysAddr = BMMA_LockOffset(buf->hBlock);
    BDBG_MODULE_MSG(BSID_MEM, ("Alloc %s mem: addr %p off " BDBG_UINT64_FMT " size %d",
    str, buf->pv_CachedAddr, BDBG_UINT64_ARG(buf->ui32_PhysAddr), buf->ui32_Size));

    return BERR_TRACE((BERR_SUCCESS));
}

/******************************************************************************
* Function name: BSID_P_FreeUnlock
*
* Comments:
*
******************************************************************************/
void BSID_P_FreeUnlock(BSID_LinearBuffer * buf)
{
    if (buf != NULL && buf->hBlock != NULL)
    {
        if (buf->pv_CachedAddr != NULL)
        {
            BMMA_UnlockOffset(buf->hBlock, buf->ui32_PhysAddr);
            BMMA_Unlock(buf->hBlock, buf->pv_CachedAddr);
            buf->pv_CachedAddr = NULL;
        }

        BMMA_Free(buf->hBlock);
        buf->hBlock = NULL;
    }
}


/******************************************************************************
* Function name: BSID_P_ResetDmaInfo
*
* Comments:
*     Clear the contents of the DMA chunk shared memory so that FW does not
*     see "sticky" or bogus information.
*
******************************************************************************/
void BSID_P_ResetDmaInfo(BSID_Handle hSid)
{
    BSID_LinearBuffer *sInpDmaMemory = &hSid->sFwHwConfig.sInpDmaMemory;
    BSID_DmaChunkInfo *pDmaInfo = (BSID_DmaChunkInfo *)sInpDmaMemory->pv_CachedAddr;
    BMMA_Block_Handle hInpDmaBlock = sInpDmaMemory->hBlock;

    BKNI_Memset(pDmaInfo, 0x0, sizeof(BSID_DmaChunkInfo));

    BMMA_FlushCache(hInpDmaBlock, (void *)pDmaInfo, sizeof(BSID_DmaChunkInfo));
}


/******************************************************************************
* Function name: BSID_P_SetFwHwDefault
*
* Comments:
*
******************************************************************************/
BERR_Code  BSID_P_SetFwHwDefault(
    BSID_Handle hSid,
    BSID_Settings ps_DefSettings
)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_LinearBuffer *sCodeMemory   = &hSid->sFwHwConfig.sCodeMemory;
    BSID_LinearBuffer *sMbxMemoryCmd = &hSid->sFwHwConfig.sMbxMemory.sCmd;
    BSID_LinearBuffer *sMbxMemoryRsp = &hSid->sFwHwConfig.sMbxMemory.sRsp;
    BSID_LinearBuffer *sInpDmaMemory = &hSid->sFwHwConfig.sInpDmaMemory;
    BSID_LinearBuffer *sDataMemory   = &hSid->sFwHwConfig.sDataMemory;

    BDBG_ENTER(BSID_P_SetFwHwDefault);

    hSid->sFwHwConfig.ui16_JPEGHorizAndVerFilt = ps_DefSettings.ui16_JPEGHorizAndVerFilt;
    hSid->sFwHwConfig.ui8_AlphaValue           = ps_DefSettings.ui8_AlphaValue;
    hSid->sFwHwConfig.b_EndianessSwap          = ps_DefSettings.b_EndianessSwap;
    hSid->sFwHwConfig.bSelfTest                = ps_DefSettings.b_SelfTest;
    /* NOTE: currently, only unified memory is supported */
    hSid->sFwHwConfig.eMemoryMode              = BSID_MemoryMode_eUnifiedMemory;

    /* code memory block */
    retCode = BSID_P_AllocLock(hSid->hMma, sCodeMemory, BSID_ARC_CODE_SIZE, BSID_ARC_CODE_ALIGN_BYTES, "Code");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Code memory alloc failed"));
        BSID_P_ResetFwHwDefault(hSid);
        BDBG_LEAVE(BSID_P_SetFwHwDefault);
        return BERR_TRACE(retCode);
    }

    /* mbx memory block */
    retCode = BSID_P_AllocLock(hSid->hMma, sMbxMemoryCmd, BSID_MBX_MEMORY_SIZE, BSID_MEM_ALIGN_BYTES, "Mbx ");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("MBX memory alloc failed"));
        BSID_P_ResetFwHwDefault(hSid);
        BDBG_LEAVE(BSID_P_SetFwHwDefault);
        return BERR_TRACE(retCode);
    }

    /* mbx memory block */
    retCode = BSID_P_AllocLock(hSid->hMma, sMbxMemoryRsp, BSID_MBX_MEMORY_SIZE, BSID_MEM_ALIGN_BYTES, "Mbx ");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("MBX memory alloc failed"));
        BSID_P_ResetFwHwDefault(hSid);
        BDBG_LEAVE(BSID_P_SetFwHwDefault);
        return BERR_TRACE(retCode);
    }

    /* input dma memory block */
    retCode = BSID_P_AllocLock(hSid->hMma, sInpDmaMemory, BSID_INPUT_DMA_MEMORY_SIZE, BSID_MEM_ALIGN_BYTES, "iDma");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("iDMA memory alloc failed"));
        BSID_P_ResetFwHwDefault(hSid);
        BDBG_LEAVE(BSID_P_SetFwHwDefault);
        return BERR_TRACE(retCode);
    }

    /* data memory block */
    retCode = BSID_P_AllocLock(hSid->hMma, sDataMemory, BSID_DATA_MEMORY_SIZE, BSID_MEM_ALIGN_BYTES, "Data");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("Data memory alloc failed"));
        BSID_P_ResetFwHwDefault(hSid);
        BDBG_LEAVE(BSID_P_SetFwHwDefault);
        return BERR_TRACE(retCode);
    }

    /* reset the content of the dma info structure to avoid triggering the fw with bogus values */
    BSID_P_ResetDmaInfo(hSid);

    BDBG_LEAVE(BSID_P_SetFwHwDefault);

    return BERR_TRACE(BERR_SUCCESS);
}

/******************************************************************************
* Function name: BSID_P_ResetFwHwDefault
*
* Comments:
*
******************************************************************************/
void  BSID_P_ResetFwHwDefault(
    BSID_Handle hSid
)
{
    /* NOTE: We dont need to check pointers before making these calls,
             since FreeUnlock will only free/unlock if the corresponding
             resource was initially allocated */

    /* code memory block */
    BSID_P_FreeUnlock(&hSid->sFwHwConfig.sCodeMemory);

    /* mbx memory block */
    BSID_P_FreeUnlock(&hSid->sFwHwConfig.sMbxMemory.sCmd);

    /* mbx memory block */
    BSID_P_FreeUnlock(&hSid->sFwHwConfig.sMbxMemory.sRsp);

    /* input dma memory block */
    BSID_P_FreeUnlock(&hSid->sFwHwConfig.sInpDmaMemory);

    /* data memory block */
    BSID_P_FreeUnlock(&hSid->sFwHwConfig.sDataMemory);
}


/******************************************************************************
* Function name: BSID_P_Watchdog_isr
*
* Comments:
*     Interrupt Handler for watchdog ISR
*     Generally just invokes a callback to notify the user of the event
*     Typically they would then invoke BSID_ProcessWatchdog() to cleanup
*
******************************************************************************/
void BSID_P_Watchdog_isr(void *pContext, int iParam)
{
   BSID_Handle hSid = (BSID_Handle)pContext;
   uint32_t uiSIDStatus = 0;
   uint32_t uiArcPC = 0;

   BSTD_UNUSED(iParam);

   hSid->bWatchdogOccurred = true;

   BSID_P_ReadSIDStatus_isr(hSid->hReg, &uiSIDStatus, &uiArcPC);

   BDBG_ERR(("Watchdog w/ SID HW status code: %08x! (ARC PC = %08x)", uiSIDStatus, uiArcPC));

   /* Watchdog callback sends back user's info and any internal data necessary */
   if (NULL != hSid->fWatchdogCallback_isr)
   {
      /* add any additional info necessary to callback data */
      (hSid->fWatchdogCallback_isr)((const BSID_WatchdogCallbackData *)&hSid->pWatchdogCallbackData);
   }
}

/******************************************************************************
* Function name: BSID_P_CreateChannelMemory
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_CreateChannelMemory(BSID_ChannelHandle hSidCh)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_Handle hSid = hSidCh->hSid;
    BSID_LinearBuffer *sMemory = NULL;
    uint32_t ui32_Size;

    /* allocate channel request queue */
    sMemory = &hSidCh->sReqQueue.sBuf;
    ui32_Size = sizeof(BSID_CommandQueueHeader) + (hSidCh->ui32_QueueTrueDepth * sizeof(BSID_ReqQueueEntry));
    retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "ChReqQ");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("ChReqQ memory alloc failed"));
        BSID_P_DestroyChannelMemory(hSidCh);
        return BERR_TRACE(retCode);
    }

    /* allocate channel release queue */
    sMemory = &hSidCh->sRelQueue.sBuf;
    ui32_Size = sizeof(BSID_CommandQueueHeader) + (hSidCh->ui32_QueueTrueDepth * sizeof(BSID_RelQueueEntry));
    retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "ChRelQ");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("ChRelQ memory alloc failed"));
        BSID_P_DestroyChannelMemory(hSidCh);
        return BERR_TRACE(retCode);
    }

    /* allocate channel data queue */
    sMemory = &hSidCh->sDataQueue.sBuf;
    ui32_Size = sizeof(BSID_CommandQueueHeader) + (hSidCh->ui32_QueueTrueDepth * sizeof(BSID_DataQueueEntry));
    retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "ChDataQ");
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("ChDataQ memory alloc failed"));
        BSID_P_DestroyChannelMemory(hSidCh);
        return BERR_TRACE(retCode);
    }

    /* allocate metadata array */
    hSidCh->pMetadata = (BSID_P_MetadataEntry *)BKNI_Malloc(hSidCh->ui32_QueueTrueDepth * sizeof(BSID_P_MetadataEntry));
    if (NULL == hSidCh->pMetadata)
    {
        BDBG_ERR(("Metadata array allocation failed"));
        BSID_P_DestroyChannelMemory(hSidCh);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    /* check if we are dealing with a motion channel */
    if (hSidCh->e_ChannelType == BSID_ChannelType_eMotion)
    {
        unsigned int pbQueueSize;
        char str[10] = "";
        unsigned int buffer_index;
        /*
         * allocate rave report memory and playback buffer queue.
         */
        /* rave report memory */
        sMemory = &hSidCh->sRaveReport;
        ui32_Size = sizeof(BSID_RaveStatusReport);
        retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "RaveReport");
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("RaveReport memory alloc failed"));
            BSID_P_DestroyChannelMemory(hSidCh);
            return BERR_TRACE(retCode);
        }

        for (buffer_index = 0x0; buffer_index < hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber; buffer_index++)
        {
            BKNI_Snprintf(str, 10, "OutBuf[%d]", buffer_index);

            /* output buffer@index memory */
            sMemory = &hSidCh->a_OutputBuffer[buffer_index];
            ui32_Size = (hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputMaxWidth *
                         hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputMaxHeight *
                         (BPXL_BITS_PER_PIXEL(BPXL_eA8_Y8_Cb8_Cr8)>>3));
            retCode = BSID_P_AllocLock(hSidCh->hChMmaHeap, sMemory, ui32_Size, BSID_OUTPUT_BUFFER_ALIGNMENT_BYTES, str);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("Output memory alloc failed"));
                BSID_P_DestroyChannelMemory(hSidCh);
                return BERR_TRACE(retCode);
            }

            /* this structure will be used to pass data downstream */
            hSidCh->a_OutputBuffer[buffer_index].ui32_PhysAddr = sMemory->ui32_PhysAddr;
        }

        pbQueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;

        /* playback write queue */
        sMemory = &hSidCh->sPlaybackQueue.sWriteBuf;
        ui32_Size = sizeof(BSID_PlaybackWriteQueueHeader) + (pbQueueSize * sizeof(BSID_PlaybackWriteQueueEntry));
        retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "PbWrQ");
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("PbWrQ memory alloc failed"));
            BSID_P_DestroyChannelMemory(hSidCh);
            return BERR_TRACE(retCode);
        }

        /* playback read queue */
        sMemory = &hSidCh->sPlaybackQueue.sReadBuf;
        ui32_Size = sizeof(BSID_PlaybackReadQueueHeader) + (pbQueueSize * sizeof(BSID_PlaybackReadQueueEntry));
        retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "PbRdQ");
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("PbRdQ memory alloc failed"));
            BSID_P_DestroyChannelMemory(hSidCh);
            return BERR_TRACE(retCode);
        }

        /* playback state queue */
        sMemory = &hSidCh->sPlaybackQueue.sStateBuf;
        ui32_Size = pbQueueSize * sizeof(BSID_PlaybackQueueState);
        retCode = BSID_P_AllocLock(hSid->hMma, sMemory, ui32_Size, BSID_QUEUE_ALIGNMENT_BYTES, "PbStateQ");
        if (retCode != BERR_SUCCESS)
        {
            BDBG_ERR(("PbStateQ memory alloc failed"));
            BSID_P_DestroyChannelMemory(hSidCh);
            return BERR_TRACE(retCode);
        }
    } /* end: if motion channel */

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_DestroyChannelMemory
*
* Comments:
*
******************************************************************************/
void BSID_P_DestroyChannelMemory(BSID_ChannelHandle hSidCh)
{
    /* free channel request queue */
    BSID_P_FreeUnlock(&hSidCh->sReqQueue.sBuf);

    /* free channel request queue */
    BSID_P_FreeUnlock(&hSidCh->sRelQueue.sBuf);

    /* free channel data queue */
    BSID_P_FreeUnlock(&hSidCh->sDataQueue.sBuf);

    if (NULL != hSidCh->pMetadata)
    {
        BKNI_Free(hSidCh->pMetadata);
        hSidCh->pMetadata = NULL;
    }

    /* check if we are dealing with a motion channel */
    if (hSidCh->e_ChannelType == BSID_ChannelType_eMotion)
    {
        /* free output buffer */
        unsigned int buffer_index = 0x0;

        /* free output buffer */
        for (buffer_index = 0x0; buffer_index < hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber; buffer_index++)
        {
            BSID_P_FreeUnlock(&hSidCh->a_OutputBuffer[buffer_index]);
        }

        /* free rave report */
        BSID_P_FreeUnlock(&hSidCh->sRaveReport);

        /* free playback read queue */
        BSID_P_FreeUnlock(&hSidCh->sPlaybackQueue.sReadBuf);

        /* free playback write queue */
        BSID_P_FreeUnlock(&hSidCh->sPlaybackQueue.sWriteBuf);

        /* free playback state buffer */
        BSID_P_FreeUnlock(&hSidCh->sPlaybackQueue.sStateBuf);
    }
}

/******************************************************************************
* Function name: BSID_P_CreateChannelMemory
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_ResetChannelMemory(BSID_ChannelHandle hSidCh)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_LinearBuffer *sMemory = NULL;
    BSID_CommandQueueHeader *psQueueHeader = NULL;
    unsigned int buffer_index;

    /* reset request queue */
    sMemory = &hSidCh->sReqQueue.sBuf;
    psQueueHeader = (BSID_CommandQueueHeader *)sMemory->pv_CachedAddr;
    psQueueHeader->ui32_ReadIndex = psQueueHeader->ui32_WriteIndex = 0;
    BMMA_FlushCache(sMemory->hBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));

    /* reset release queue */
    sMemory = &hSidCh->sRelQueue.sBuf;
    psQueueHeader = (BSID_CommandQueueHeader *)sMemory->pv_CachedAddr;
    psQueueHeader->ui32_ReadIndex = psQueueHeader->ui32_WriteIndex = 0;
    BMMA_FlushCache(sMemory->hBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));

    /* reset data queue */
    sMemory = &hSidCh->sDataQueue.sBuf;
    psQueueHeader = (BSID_CommandQueueHeader *)sMemory->pv_CachedAddr;
    psQueueHeader->ui32_ReadIndex = psQueueHeader->ui32_WriteIndex = 0;
    BMMA_FlushCache(sMemory->hBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));

    /* reset the metadata queue */
    BKNI_Memset((void *)hSidCh->pMetadata, 0, hSidCh->ui32_QueueTrueDepth * sizeof(BSID_P_MetadataEntry));

    /* check if we are dealing with a motion channel */
    if (hSidCh->e_ChannelType == BSID_ChannelType_eMotion)
    {
        BSID_RaveStatusReport *pMemoryReport;
        BSID_PlaybackWriteQueueHeader *psPbWriteQueueHeader;
        BSID_PlaybackWriteQueueEntry *psPbWriteQueueEntry;
        BSID_PlaybackReadQueueHeader *psPbReadQueueHeader;
        BSID_PlaybackQueueState *psPbStateQueue;
        BSID_PlaybackQueueState *psPbStateQueueEntry;
        unsigned int pbQueueSize = hSidCh->sChSettings.u_ChannelSpecific.motion.ui32_OutputBuffersNumber;
        BXDM_Picture *pUniPicture;

        sMemory = &hSidCh->sRaveReport;
        pMemoryReport = (BSID_RaveStatusReport *)sMemory->pv_CachedAddr;
        BKNI_Memset((void *)pMemoryReport, 0x0, sizeof(BSID_RaveStatusReport));
        BMMA_FlushCache(sMemory->hBlock, (void *)pMemoryReport, sizeof(BSID_RaveStatusReport));
        /*
         * following are playbackQueue related
         */
        hSidCh->sPlaybackQueue.ui32_DisplayReadIndex = 0;
        hSidCh->sPlaybackQueue.ui32_HostDecodeReadIndex = 0;
        hSidCh->sPlaybackQueue.ui32_DecodeQueueFullnessCount = 0;
        hSidCh->sPlaybackQueue.ui32_DisplayQueueFullnessCount = 0;

        /* playback write queue */
        sMemory = &hSidCh->sPlaybackQueue.sWriteBuf;
        psPbWriteQueueHeader = (BSID_PlaybackWriteQueueHeader *)sMemory->pv_CachedAddr;
        psPbWriteQueueHeader->ui32_NewPictWriteIndex = 0;
        psPbWriteQueueHeader->ui32_ChannelState = BSID_ChannelState_eReady;

        BMMA_FlushCache(sMemory->hBlock, (void *)psPbWriteQueueHeader, sizeof(BSID_PlaybackWriteQueueHeader));

        for (buffer_index = 0x0; buffer_index < (pbQueueSize); buffer_index++)
        {
            pUniPicture = &hSidCh->a_DisplayBuffer[buffer_index];

            pUniPicture->stBufferInfo.bValid = true;
            pUniPicture->stPTS.uiValue = 0x0;
            pUniPicture->stPTS.bValid = true;
            pUniPicture->stFrameRate.eType = BXDM_Picture_FrameRateType_eUnknown;

            psPbWriteQueueEntry = (BSID_PlaybackWriteQueueEntry *)((uint8_t *)psPbWriteQueueHeader + sizeof(BSID_PlaybackWriteQueueHeader) + \
                                  (psPbWriteQueueHeader->ui32_NewPictWriteIndex * sizeof(BSID_PlaybackWriteQueueEntry)));

            BMMA_FlushCache(sMemory->hBlock, (void *)psPbWriteQueueEntry, sizeof(BSID_PlaybackWriteQueueEntry));

            BKNI_Memset((void *)psPbWriteQueueEntry, 0x00, sizeof(BSID_PlaybackWriteQueueEntry));

            psPbWriteQueueEntry->ui32_OutputBufferAddress = hSidCh->a_OutputBuffer[buffer_index].ui32_PhysAddr;

            BSID_INCR_INDEX(psPbWriteQueueHeader->ui32_NewPictWriteIndex, pbQueueSize);

            BMMA_FlushCache(sMemory->hBlock, (void *)psPbWriteQueueEntry, sizeof(BSID_PlaybackWriteQueueEntry));
            BMMA_FlushCache(sMemory->hBlock, (void *)psPbWriteQueueHeader, sizeof(BSID_PlaybackWriteQueueHeader));
        }

        psPbWriteQueueHeader->ui32_NewPictWriteIndex = 0x0;
        BMMA_FlushCache(sMemory->hBlock, (void *)psPbWriteQueueHeader, sizeof(BSID_PlaybackWriteQueueHeader));

        /* playback read queue */
        sMemory = &hSidCh->sPlaybackQueue.sReadBuf;
        psPbReadQueueHeader = (BSID_PlaybackReadQueueHeader *)sMemory->pv_CachedAddr;
        psPbReadQueueHeader->ui32_DecodedReadIndex = 0x0;
        BMMA_FlushCache(sMemory->hBlock, (void *)psPbReadQueueHeader, sizeof(BSID_PlaybackReadQueueHeader));

        /* playback state queue */
        sMemory = &hSidCh->sPlaybackQueue.sStateBuf;
        psPbStateQueue = (BSID_PlaybackQueueState *)sMemory->pv_CachedAddr;
        BMMA_FlushCache(sMemory->hBlock, (void *)psPbStateQueue, sizeof(BSID_PlaybackQueueState));

        for (buffer_index = 0x0; buffer_index < (pbQueueSize); buffer_index++)
        {
            psPbStateQueueEntry = (BSID_PlaybackQueueState *)((uint8_t *)psPbStateQueue + \
                                  (buffer_index * sizeof(BSID_PlaybackQueueState)));

            psPbStateQueueEntry->ui32_OutputState = BSID_OutputBufferState_eIdle;
            psPbStateQueueEntry->ps_UnifiedPicture = NULL;
            BMMA_FlushCache(sMemory->hBlock, (void *)psPbStateQueueEntry, sizeof(BSID_PlaybackQueueState));
        }
    } /* end: if motion channel */

    return BERR_TRACE(retCode);
}



void BSID_P_DestroyChannel(BSID_ChannelHandle hSidCh)
{
    BDBG_ASSERT(hSidCh);

    /* destroy device channel queue */
    BSID_P_DestroyChannelMemory(hSidCh);

    BSID_SET_CH_STATE(hSidCh, Close);

    /* destroy sync event */
    if (NULL != hSidCh->hSyncEvent)
    {
       BKNI_DestroyEvent(hSidCh->hSyncEvent);
       hSidCh->hSyncEvent = NULL;
    }

    /* destroy abort event */
    if (NULL != hSidCh->hAbortedEvent)
    {
       BKNI_DestroyEvent(hSidCh->hAbortedEvent);
       hSidCh->hAbortedEvent = NULL;
    }

    /* invalidate the device channel from the main device handle */
    hSidCh->hSid->ahChannel[hSidCh->ui32_ChannelNum] = NULL;

    /* free device channel handle */
    BKNI_Free(hSidCh);
    hSidCh = NULL;
}

/******************************************************************************
* Function name: BSID_P_IsChannelQueueFull
*
* Comments:
*
******************************************************************************/
bool BSID_P_IsChannelQueueFull(BSID_ChannelHandle hSidCh)
{
    BSID_CommandQueueHeader *psQueueHeader = (BSID_CommandQueueHeader *)hSidCh->sReqQueue.sBuf.pv_CachedAddr;
    BMMA_FlushCache(hSidCh->sReqQueue.sBuf.hBlock, (void *)psQueueHeader, sizeof(BSID_CommandQueueHeader));

    return (((psQueueHeader->ui32_WriteIndex + 1) % hSidCh->ui32_QueueTrueDepth) == psQueueHeader->ui32_ReadIndex);
}

/******************************************************************************
* Function name: BSID_P_MotionDecode
*   -
* Comments:
*   -
******************************************************************************/
BERR_Code BSID_P_MotionDecode(BSID_ChannelHandle hSidCh, const BSID_DecodeMotion *ps_MotionSettings)
{
    BERR_Code retCode = BERR_SUCCESS;

    BDBG_ENTER(BSID_P_MotionDecode);

    /* init rave, decode, xdm and channel data  */
    retCode = BSID_P_InitPlayback(hSidCh, ps_MotionSettings);
    if (retCode != BERR_SUCCESS)
    {
       BDBG_ERR(("BSID_InitPlayback failed"));
       BDBG_LEAVE(BSID_P_MotionDecode);
       return BERR_TRACE(retCode);
    }

    /* prepare decode command and send it to sid arc */
    retCode = BSID_P_SendCmdStartDecode(hSidCh);
    if (retCode != BERR_SUCCESS)
    {
        /* un-lock status of sid arc: sid arc status is idle */
        BDBG_LEAVE(BSID_P_MotionDecode);
        return BERR_TRACE(retCode);
    }

    /* NOTE: The following does nothing unless BSID_P_DEBUG_SAVE_BUFFER enabled
       (see bsid_dbg.h) */
    BSID_P_CreateSaveImagesThread(hSidCh);

    BDBG_LEAVE(BSID_P_MotionDecode);

    return BERR_TRACE(retCode);
}


/******************************************************************************
* Function name: BSID_P_AbortDecode
*
* Comments:
*    This must always succeed even if it means restarting the core (an
*    equivalent to a "watchdog")
******************************************************************************/
void BSID_P_AbortDecode(BSID_ChannelHandle hSidCh)
{
    int iAbortAttempts = 0;
    BERR_Code err = BERR_SUCCESS;

    BDBG_ENTER(BSID_P_AbortDecode);
    BKNI_ResetEvent(hSidCh->hAbortedEvent);

    do
    {
        BKNI_EnterCriticalSection();
        BSID_P_SetAbortDma_isr(hSidCh);
        BKNI_LeaveCriticalSection();
        hSidCh->bAbortInitiated = true;
        err = BKNI_WaitForEvent(hSidCh->hAbortedEvent, 100/*ms*/);
        if (BERR_TIMEOUT == err)
        {
           BDBG_WRN(("AbortDecode:: waiting for channel %d (%p) to respond [%u]", hSidCh->ui32_ChannelNum, (void *)hSidCh, iAbortAttempts));
           iAbortAttempts++;
        }
    }
    while ((BERR_TIMEOUT == err ) && (iAbortAttempts < 10));
    if (iAbortAttempts == 10)
    {
        BDBG_ERR(("AbortDecode:: Abort failed - invoking watchdog"));
        /* raise the equivalent of a watchdog interrupt so the caller can "reset" the SID */
        BKNI_EnterCriticalSection();
        BSID_P_Watchdog_isr(hSidCh->hSid, 0);
        BKNI_LeaveCriticalSection();
    }
    else
    {
        BDBG_WRN(("Decode Aborted!"));
    }

    hSidCh->bAbortInitiated = false;

    /* Clear the DMA info to avoid "sticky" abort status */
    BSID_P_ResetDmaInfo(hSidCh->hSid);

    BDBG_LEAVE(BSID_P_AbortDecode);
}


/******************************************************************************
* Function name: BSID_SuspendStillDecode
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_SuspendChannels(BSID_Handle hSid)
{
    BERR_Code retCode = BERR_SUCCESS;
    BKNI_EventGroupHandle hEventGroup = NULL;
    BKNI_EventHandle *pTriggeredEvents = NULL;
    unsigned numTriggeredEvents;
    unsigned int channel_index = 0;
    bool bAllChannelClosed = true;
    BSID_ChannelHandle hSidCh;

    retCode = BKNI_CreateEventGroup(&hEventGroup);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BKNI_CreateEventGroup failed with error 0x%x", retCode));
        return BERR_TRACE(retCode);
    }

    pTriggeredEvents = (BKNI_EventHandle *)BKNI_Malloc(sizeof(BKNI_EventHandle)*BSID_MAX_CHANNELS);
    if (pTriggeredEvents == NULL)
    {
        BDBG_ERR(("BKNI_Malloc failed"));
        BKNI_DestroyEventGroup(hEventGroup);
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }

    for (channel_index = 0; channel_index < BSID_MAX_CHANNELS; channel_index++)
    {
        hSidCh = hSid->ahChannel[channel_index];
        if (hSidCh != NULL)
        {
            bAllChannelClosed = false;
            BSID_SET_CH_STATE(hSidCh, Suspend);

            retCode = BKNI_AddEventGroup(hEventGroup, hSidCh->hSyncEvent);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BKNI_AddEventGroup failed with error 0x%x", retCode));
                BKNI_DestroyEventGroup(hEventGroup);
                BKNI_Free(pTriggeredEvents);
                return BERR_TRACE(retCode);
            }

            retCode = BSID_P_SendCmdSyncChannel(hSidCh);
            if (retCode != BERR_SUCCESS)
            {
                BDBG_ERR(("BSID_P_SendSyncChannel failed with error 0x%x", retCode));
                BKNI_DestroyEventGroup(hEventGroup);
                BKNI_Free(pTriggeredEvents);
                return BERR_TRACE(retCode);
            }
        }
    }

    if (!bAllChannelClosed)
    {
      /* wait for the required channel(s) to sync before close */
       retCode = BKNI_WaitForGroup(hEventGroup, BSID_SYNC_MAX_WAITING_TIME, pTriggeredEvents, BSID_MAX_CHANNELS, &numTriggeredEvents);
       /* NOTE: if this wait fails, it will attempt to proceed anyway.
          The subsequent send command may fail, but then we can clean up after that failed attempt */
       if (retCode != BERR_SUCCESS)
       {
           BDBG_ERR(("BKNI_WaitForGroup failed with error 0x%x ... attempting to close channels", retCode));
       }
       BDBG_MSG(("BKNI_WaitForGroup waited on %d events", numTriggeredEvents));

       for (channel_index = 0; channel_index < BSID_MAX_CHANNELS; channel_index++)
       {
           if (hSid->ahChannel[channel_index] != NULL)
           {
               retCode = BSID_P_SendCmdCloseChannel(hSid->ahChannel[channel_index]);
               /* NOTE: This assumes that if any CloseChannel Command fails, then
                  all subsequent close channel requests will also fail */
               if (retCode != BERR_SUCCESS)
               {
                   BDBG_ERR(("BSID_P_SendCmdCloseChannel failed with error 0x%x", retCode));
                   BKNI_DestroyEventGroup(hEventGroup);
                   BKNI_Free(pTriggeredEvents);
                   return BERR_TRACE(retCode);
               }
           }
       }
    }
    BKNI_DestroyEventGroup(hEventGroup);
    BKNI_Free(pTriggeredEvents);

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_P_ResumeChannel
*
* Comments:
*
* Reset the channel settings that can change
******************************************************************************/

BERR_Code BSID_P_ResetChannel(BSID_ChannelHandle hSidCh)
{
    hSidCh->ui32_SequenceNumber = 0x0;
    hSidCh->b_FlushPending      = false;
    BSID_SET_CH_STATE(hSidCh, Close);
    return BERR_SUCCESS;
}


/******************************************************************************
* Function name: BSID_P_ResumeChannel
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_ResumeChannel(BSID_ChannelHandle hSidCh)
{
    BERR_Code retCode = BERR_SUCCESS;

    BSID_P_ResetChannel(hSidCh);
    BSID_P_ResetChannelMemory(hSidCh);
    BKNI_ResetEvent(hSidCh->hSyncEvent);

    /* send open channel command to sid arc */
    retCode = BSID_P_SendCmdOpenChannel(hSidCh);
    if (retCode != BERR_SUCCESS)
    {
        BDBG_ERR(("BSID_P_SendCmdOpenChannel failed with error 0x%x", retCode));
        return BERR_TRACE(retCode);
    }

    /* channel is in open state */
    BSID_SET_CH_STATE(hSidCh, Ready);

    return retCode;
}

/******************************************************************************
* Function name: BSID_P_ResumeActiveChannels
*
* Comments:
*
******************************************************************************/
BERR_Code BSID_P_ResumeActiveChannels(BSID_Handle hSid)
{
    BERR_Code retCode = BERR_SUCCESS;
    BSID_ChannelHandle hSidCh;

    unsigned int channel_index = 0;

    for (channel_index = 0; (channel_index < BSID_MAX_CHANNELS) && (BERR_SUCCESS == retCode); channel_index++)
    {
        hSidCh = hSid->ahChannel[channel_index];
        if (hSidCh != NULL)
        {
            retCode = BSID_P_ResumeChannel(hSidCh);
        }
    }

    return BERR_TRACE(retCode);
}

/******************************************************************************
* Function name: BSID_IsStillOperationAllowed
*
* Comments:
*
******************************************************************************/
bool BSID_P_IsStillOperationAllowed(BSID_Handle hSid)
{
    unsigned int channel_index = 0;
    bool still_allowed = true;

    for (channel_index = 0; channel_index < BSID_MAX_CHANNELS; channel_index++)
    {
        if (hSid->ahChannel[channel_index] != NULL)
        {
            if (hSid->ahChannel[channel_index]->e_ChannelType == BSID_ChannelType_eMotion)
            {
                still_allowed = false;
                break;
            }
        }
    }

    return still_allowed;
}

/******************************************************************************
* Function name: BSID_IsMotionOperationAllowed
*
* Comments:
*
******************************************************************************/
bool BSID_P_IsMotionOperationAllowed(BSID_Handle hSid)
{
    unsigned int channel_index = 0;
    bool motion_allowed = true;

    for (channel_index = 0; channel_index < BSID_MAX_CHANNELS; channel_index++)
    {
        if (hSid->ahChannel[channel_index] != NULL)
        {
            if ((hSid->ahChannel[channel_index]->e_ChannelType == BSID_ChannelType_eStill) &&
                (hSid->ahChannel[channel_index]->e_ChannelState == BSID_ChannelState_eDecode))
            {
                motion_allowed = false;
                break;
            }
        }
    }

    return motion_allowed;
}

/******************************************************************************
* Function name: BSID_P_AnyChannelAvailable
*
* Comments:
*
******************************************************************************/
bool BSID_P_AnyChannelAvailable(BSID_Handle hSid, uint32_t *idleChannel)
{
    int32_t channelIndex;

    /* get first channel available */
    for (channelIndex = (BSID_MAX_CHANNELS - 1); channelIndex >= 0; channelIndex--)
    {
        if (hSid->ahChannel[channelIndex] == NULL)
        {
            *idleChannel = channelIndex;
            return true;
        }
    }
    return false;
}

/*******************
  End Of File
********************/
