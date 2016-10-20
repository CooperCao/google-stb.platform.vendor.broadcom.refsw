/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bdsp_common_mm_priv.h"

BDBG_MODULE(bdsp_common_mm_priv);

BERR_Code BDSP_P_CopyDataToDram (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;

    if (data==NULL)
    {
        BKNI_Memset(memAdr, 0, size);
        BDSP_MEM_P_FlushCache(hHeap,memAdr,size);
    }
    else
    {
        BKNI_Memcpy(memAdr, data , size);
        BDSP_MEM_P_FlushCache(hHeap,memAdr,size);
    }
    return ret;
}

BERR_Code BDSP_P_CopyDataFromDram (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;

    BDSP_MEM_P_FlushCache(hHeap,memAdr,size);
    BKNI_Memcpy(data,memAdr,size);
    return ret;
}

BERR_Code BDSP_P_CopyDataFromDram_isr (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    )
{
    BERR_Code    ret=BERR_SUCCESS;

    BDSP_MEM_P_FlushCache_isr(hHeap,memAdr,size);
    BKNI_Memcpy(data,memAdr,size);
    return ret;
}

BERR_Code BDSP_P_CopyDataToDram_isr (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    )
{
    BERR_Code   ret=BERR_SUCCESS;

     if (data==NULL)
     {
         BKNI_Memset(memAdr, 0, size);
         BDSP_MEM_P_FlushCache_isr(hHeap,memAdr,size);
     }
     else
     {
         BKNI_Memcpy(memAdr, data , size);
         BDSP_MEM_P_FlushCache_isr(hHeap,memAdr,size);
     }

     return ret;
}

void BDSP_P_MemWrite32(
    BMEM_Handle hHeap,
    void    *memAddress,
    uint32_t    data
    )
{
    BDSP_P_CopyDataToDram(hHeap,(void *)&data,memAddress,4);
}

void BDSP_P_MemWrite32_isr(
    BMEM_Handle hHeap,
    void    *memAddress,
    uint32_t    data
    )
{
    BDSP_P_CopyDataToDram_isr(hHeap,(void *)&data,memAddress,4);
}

uint32_t BDSP_P_MemRead32_isr(
        BMEM_Handle hHeap,
        void        *memAddress
    )
{
    uint32_t ui32ValRead;
    BDSP_P_CopyDataFromDram_isr(hHeap,(void *)&ui32ValRead,memAddress,4);
    return ui32ValRead;
}

void BDSP_P_WriteToOffset(BMEM_Handle hHeap,
                                void        *pSrc,
                                uint32_t    ui32DestAddr,
                                uint32_t    ui32Size )
{
    void        *pDest;

    /*  Need to convert the physical address to virtual address to access   DRAM */
    BDSP_MEM_P_ConvertOffsetToCacheAddr(hHeap,
                                    ui32DestAddr,
                                    ((void**)(&pDest)));

    BDSP_P_CopyDataToDram(hHeap, pSrc, pDest, ui32Size);
}

void BDSP_P_ReadFromOffset(BMEM_Handle    hHeap,
                                        uint32_t ui32SrcOffset,
                                        void * pDest,
                                        uint32_t ui32size )
{

    void *pSrc;

    BDSP_MEM_P_ConvertOffsetToCacheAddr(hHeap,
                                ui32SrcOffset,
                                (void **)&pSrc
                            );

    BDSP_MEM_P_FlushCache(hHeap, pSrc, ui32size);

    /*Read the DRAM to local structure */
    BDSP_P_CopyDataFromDram(hHeap, pDest, pSrc, ui32size);
}

/***************************************************************************************************
Summary:
#Use BDSP_MEM_P_AllocateAlignedMemory to assign memory which will be accessed by Raaga;It may be the location that only Raaga reads or writes
#into or both Host and Raaga access it. System allocates cache memory pointer whenever you allocate using BDSP_MEM_P_AllocateAlignedMemory.
#So any write into that address needs to be flushed for it to be seen by Raaga as it is a different processor. Unless the BDSP code does a flush
#one can't guarantee  that Raaga will see the values written by BDSP code into that location. So always flush after a write to the memory which
#will be read by Raaga.

#Cache address needs to be converted to Virtual offset address for DSP to be abe to read from it.
#BDSP needs address to be Cached address for it to Read or write into.

#In BDSP: Write and Read requires cached address
#In Raaga: Write or Read into DRAM requries virtual offset.

--Usage of BDSP_MEM_P_AllocateAlignedMemory function
#define BDSP_MEM_P_AllocateAlignedMemory(memHandle, size, align, boundary) \
                 BDSP_P_ConvertAllocToCache(memHandle, BMEM_AllocAligned(memHandle,size,align, boundary) )

Do Not use BDSP_P_ConvertAllocToCache independently in BDSP code. It will be called inside the
BDSP_MEM_P_AllocateAlignedMemory only for now. Check other definition in BDSP_MEM_P* function calls
in the file bdsp_raaga_mm_priv.h to choose the functions required for bmem access.

***************************************************************************************************/


void * BDSP_P_ConvertAllocToCache(BMEM_Handle memHandle, void * ptr)
{
        void * pCache = NULL;
        BERR_Code err;

        if(ptr == NULL){
                goto exit;
        }

        err = BMEM_ConvertAddressToCached(memHandle, ptr, &pCache);
        if(err != BERR_SUCCESS){
                pCache = NULL;
                BDBG_ERR((" %s: Issue with ptr=%p being tried to convert to cache", __FUNCTION__, ptr));
        }

exit:
        return pCache;

}
