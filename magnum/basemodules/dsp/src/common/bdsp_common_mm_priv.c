/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bdsp_common_mm_priv.h"

BDBG_MODULE(bdsp_common_mm_priv);

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

BERR_Code BDSP_MMA_P_MemWrite32(
    BDSP_MMA_Memory  *dest,
    uint32_t data
    )
{
	BERR_Code ret = BERR_SUCCESS;
	ret = BDSP_MMA_P_CopyDataToDram(dest, (void *)&data, 4);
	return ret;
}

BERR_Code BDSP_MMA_P_MemWrite32_isr(
    BDSP_MMA_Memory    *dest,
    uint32_t    data
    )
{
	BERR_Code ret = BERR_SUCCESS;
	ret = BDSP_MMA_P_CopyDataToDram_isr(dest, (void *)&data, 4);
	return ret;
}

uint32_t BDSP_MMA_P_MemRead32(
     BDSP_MMA_Memory  *src
    )
{
    uint32_t ui32ValRead;
	BDSP_MMA_P_CopyDataFromDram((void *)&ui32ValRead, src, 4);
	return ui32ValRead;
}

uint32_t BDSP_MMA_P_MemRead32_isr(
     BDSP_MMA_Memory  *src
    )
{
    uint32_t ui32ValRead;
	BDSP_MMA_P_CopyDataFromDram_isr((void *)&ui32ValRead, src, 4);
	return ui32ValRead;
}
