/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bdsp_common_priv_include.h"

BDBG_MODULE(bdsp_common_mm_priv);

void BDSP_P_CalculateDescriptorMemory(
	unsigned *pMemReqd
)
{
    unsigned MemoryRequired = 0;

    BDBG_ENTER(BDSP_P_CalculateDescriptorMemory);
    *pMemReqd = 0;

	MemoryRequired = (BDSP_MAX_DESCRIPTORS * sizeof(BDSP_AF_P_sCIRCULAR_BUFFER));
    *pMemReqd = MemoryRequired;

	BDBG_MSG(("Descriptor Memory = %d (%d KB) (%d MB)",MemoryRequired, (MemoryRequired/1024), (MemoryRequired/(1024*1024))));
    BDBG_LEAVE(BDSP_P_CalculateDescriptorMemory);
}

void BDSP_P_CalculateInterTaskMemory(
    unsigned  *pMemReqd,
    unsigned   numchannels
)
{
    unsigned MemoryRequired = 0;
    BDBG_ENTER(BDSP_P_CalculateDescriptorMemory);
    *pMemReqd = 0;

    MemoryRequired = (numchannels      * BDSP_AF_P_INTERTASK_BUFFER_PER_CHANNEL)+
              (BDSP_AF_P_MAX_TOC       * BDSP_AF_P_TOC_BUFFER_SIZE)+
              (BDSP_AF_P_MAX_METADATA  * BDSP_AF_P_METADATA_BUFFER_SIZE)+
              (BDSP_AF_P_MAX_OBJECTDATA*BDSP_AF_P_OBJECTDATA_BUFFER_SIZE);
    BDSP_SIZE_ALIGN(MemoryRequired);

    *pMemReqd = MemoryRequired;
    BDBG_MSG(("InterTask Memory( %d channels) = %d",numchannels, MemoryRequired));
    BDBG_LEAVE(BDSP_P_CalculateDescriptorMemory);
}

BERR_Code BDSP_MMA_P_AllocateAlignedMemory(BMMA_Heap_Handle memHandle,
	uint32_t size,
	BDSP_MMA_Memory *pMemory,
	BDSP_MMA_Alignment alignment
	)
{
	pMemory->hBlock= BMMA_Alloc(memHandle, size, alignment, NULL);
	if (NULL == pMemory->hBlock)
	{
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}
	pMemory->pAddr = BMMA_Lock(pMemory->hBlock);
	if (NULL == pMemory->pAddr)
	{
		BMMA_Free(pMemory->hBlock);
		pMemory->hBlock = NULL;
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}
	pMemory->offset = BMMA_LockOffset(pMemory->hBlock);
	if (0 == pMemory->offset)
	{
		BMMA_Unlock(pMemory->hBlock, pMemory->pAddr);
		pMemory->pAddr = NULL;
		BMMA_Free(pMemory->hBlock);
		pMemory->hBlock = NULL;
		return BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	}
	/*BDBG_MSG(("BLOCK = %x \t PADDR = %p ""OFFSET = "BDBG_UINT64_FMT,pMemory->hBlock, pMemory->pAddr, BDBG_UINT64_ARG(pMemory->offset)));*/
	return BERR_SUCCESS;
}

void BDSP_MMA_P_FreeMemory(BDSP_MMA_Memory *pMemory)
{
	if ( pMemory->hBlock )
	{
		if(pMemory->offset)
		{
			BMMA_UnlockOffset(pMemory->hBlock, pMemory->offset);
			pMemory->offset = 0;
		}
		if (pMemory->pAddr)
		{
			BMMA_Unlock(pMemory->hBlock, pMemory->pAddr);
			pMemory->pAddr = NULL;
		}
		BMMA_Free(pMemory->hBlock);
		pMemory->hBlock = NULL;
	}
}

BERR_Code BDSP_MMA_P_CopyDataToDram (
    BDSP_MMA_Memory *dest,
    void *src,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;
    if (src == NULL)
    {
        BKNI_Memset(dest->pAddr, 0, size);
        BDSP_MMA_P_FlushCache((*dest), size);
    }
    else
    {
        BKNI_Memcpy(dest->pAddr, src , size);
        BDSP_MMA_P_FlushCache((*dest), size);
    }
    return ret;
}

BERR_Code BDSP_MMA_P_CopyDataToDram_isr (
    BDSP_MMA_Memory *dest,
    void *src,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;
    if (src == NULL)
    {
        BKNI_Memset(dest->pAddr, 0, size);
        BDSP_MMA_P_FlushCache_isr((*dest), size);
    }
    else
    {
        BKNI_Memcpy(dest->pAddr, src , size);
        BDSP_MMA_P_FlushCache_isr((*dest), size);
    }
    return ret;
}
BERR_Code BDSP_MMA_P_CopyDataFromDram (
    void *dest,
    BDSP_MMA_Memory *src,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;

	BDSP_MMA_P_FlushCache((*src), size);
    BKNI_Memcpy(dest,src->pAddr,size);
    return ret;
}

BERR_Code BDSP_MMA_P_CopyDataFromDram_isr(
    void *dest,
    BDSP_MMA_Memory *src,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;

	BDSP_MMA_P_FlushCache_isr((*src), size);
    BKNI_Memcpy(dest,src->pAddr,size);
    return ret;
}

void* BDSP_MMA_P_GetVirtualAddressfromOffset(
	BDSP_P_FwBuffer *pBuffer,
	dramaddr_t       offset
)
{
	void *pAddr = NULL;
	BDBG_ENTER(BDSP_MMA_P_GetVirtualAddressfromOffset);

	if(offset <= (pBuffer->Buffer.offset+pBuffer->ui32Size) ||
		(offset >= pBuffer->Buffer.offset))
	{
		pAddr = BDSP_MMA_P_OffsetToVirtual(pBuffer->Buffer.pAddr, pBuffer->Buffer.offset, offset);
	}
	else
	{
		BDBG_ERR(("BDSP_Raaga_P_GetVirtualAddressfromOffset: Offset("BDSP_MSG_FMT") provided is outside the MMA block:Offset("BDSP_MSG_FMT"), size = %d",
					BDSP_MSG_ARG(offset), BDSP_MSG_ARG(pBuffer->Buffer.offset), pBuffer->ui32Size));
	}
	BDBG_LEAVE(BDSP_MMA_P_GetVirtualAddressfromOffset);
	return pAddr;
}

dramaddr_t BDSP_MMA_P_GetOffsetfromVirtualAddress(
	BDSP_P_FwBuffer *pBuffer,
	void       		*pAddr
)
{
	dramaddr_t offset = 0;
	BDBG_ENTER(BDSP_MMA_P_GetOffsetfromVirtualAddress);

	if((pAddr <= (void *)((uint8_t *)pBuffer->Buffer.pAddr+pBuffer->ui32Size)) ||
		(pAddr >= pBuffer->Buffer.pAddr))
	{
		offset = BDSP_MMA_P_VirtualToOffset(pBuffer->Buffer.pAddr, pBuffer->Buffer.offset, pAddr);
	}
	else
	{
		BDBG_ERR(("BDSP_Raaga_P_GetOffsetfromVirtualAddress: pAddr(%p) provided is outside the MMA block:pAddr(%p), size = %d",
					pAddr, pBuffer->Buffer.pAddr, pBuffer->ui32Size));
	}
	BDBG_LEAVE(BDSP_MMA_P_GetOffsetfromVirtualAddress);
	return offset;
}

BERR_Code BDSP_P_RequestMemory(
    BDSP_P_MemoryPool *pMemoryPool,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_P_RequestMemory);
	if(ui32Size)
	{
	    if((pMemoryPool->ui32UsedSize + ui32Size) > pMemoryPool->ui32Size)
	    {
	        BDBG_ERR(("BDSP_P_RequestMemory: Cannot Assign the requested Size"));
	        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	        goto end;
	    }

	    *pMemory = pMemoryPool->Memory;
	    pMemory->offset = pMemory->offset + pMemoryPool->ui32UsedSize;
	    pMemory->pAddr  = (void *)((uint8_t *)pMemory->pAddr + pMemoryPool->ui32UsedSize);

	    pMemoryPool->ui32UsedSize += ui32Size;
	}
end:
    BDBG_LEAVE(BDSP_P_RequestMemory);
    return errCode;
}

BERR_Code BDSP_P_ReleaseMemory(
    BDSP_P_MemoryPool *pMemoryPool,
    uint32_t ui32Size,
    BDSP_MMA_Memory *pMemory
)
{
    BERR_Code errCode = BERR_SUCCESS;

    BDBG_ENTER(BDSP_P_ReleaseMemory);
	if(ui32Size)
	{
	    if(pMemoryPool->ui32UsedSize < ui32Size)
	    {
	        BDBG_ERR(("BDSP_P_ReleaseMemory: Trying the release memory more than used"));
	        errCode = BERR_TRACE(BERR_OUT_OF_DEVICE_MEMORY);
	        goto end;
	    }

	    pMemoryPool->ui32UsedSize -= ui32Size;
		BKNI_Memset(pMemory, 0, sizeof(BDSP_MMA_Memory));
	}
end:
    BDBG_LEAVE(BDSP_P_ReleaseMemory);
    return errCode;
}
