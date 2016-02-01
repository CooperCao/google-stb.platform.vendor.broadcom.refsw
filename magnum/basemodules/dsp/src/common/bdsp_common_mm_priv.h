/******************************************************************************
 * (c) 2006-2016 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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
 *
 *****************************************************************************/
#ifndef BDSP_COMMON_MM_PRIV_H_
#define BDSP_COMMON_MM_PRIV_H_

#include "bdsp_common_priv_include.h"

#define BDSP_Read32(hReg, addr)             BREG_Read32(hReg, addr)
#define BDSP_Write32(hReg, addr, data)      BREG_Write32(hReg, addr, data)
#define BDSP_Read32_isr(hReg, addr)         BREG_Read32_isr(hReg, addr)
#define BDSP_Write32_isr(hReg, addr, data)  BREG_Write32_isr(hReg, addr, data)

BERR_Code BDSP_P_CopyDataToDram (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    );
BERR_Code BDSP_P_CopyDataToDram_isr (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    );
BERR_Code BDSP_P_CopyDataFromDram (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    );
BERR_Code BDSP_P_CopyDataFromDram_isr (
    BMEM_Handle hHeap,
    void *data,
    void *memAdr,
    uint32_t size
    );

void BDSP_P_MemWrite32(
    BMEM_Handle hHeap,
    void    *memAddress,
    uint32_t    data
    );

void BDSP_P_MemWrite32_isr(
    BMEM_Handle hHeap,
    void    *memAddress,
    uint32_t    data
    );


uint32_t BDSP_P_MemRead32(
        BMEM_Handle hHeap,
        void    *memAddress
    );

uint32_t BDSP_P_MemRead32_isr(
        BMEM_Handle hHeap,
        void    *memAddress
    );

void BDSP_P_WriteToOffset(BMEM_Handle hHeap,
                                void        *pSrc,
                                uint32_t    ui32DestAddr,
                                uint32_t    ui32Size );

void BDSP_P_ReadFromOffset(BMEM_Handle    hHeap,
                                        uint32_t ui32SrcOffset,
                                        void * pDest,
                                        uint32_t ui32size );

/***************************************************************************************************
#Use following calls to assign memory which will be accessed by BDSP device ;It may be the location that only BDSP device reads or writes
#into or both Host and BDSP device access it. System allocates cache memory pointer whenever you allocate using BDSP_MEM_P_AllocateAlignedMemory.
#So any write into that address needs to be flushed for it to be seen by BDSP device as it is a different process. Unless the BDSP code does a flush
#one can't guarantee  that BDSP device will see the values written by BDSP code into that location. So always flush after a write to the memory which
#will be read by BDSP device.

#Cache address needs to be converted to Virtual offset address for BDSP device to be abe to read from it.
#BDSP needs address to be Cached address for it to Read or write into.

#In BDSP: Write and Read requires cached address
#In BDSP device interface: Write or Read into DRAM requries virtual offset.

***************************************************************************************************/
/***************************************************************************************
# BDSP_MEM_P_AllocateAlignedMemory has been split the following way to use the debug
utilities provided by Vladimir. Whenever an allocated chunk of memory doesn't get freed
bmem utility prints the line number where it was allocated to indicate the allocation
which has not been freed. To make use of that BMEM_AllocAligned has to be present where the
allocation happens. If the allocation fails then the return value is going to be NULL which gets
checked inside the BDSP_P_ConvertAllocToCache. BDSP_P_ConvertAllocToCache should never be called
independently. It will be called internally inside BDSP_MEM_P_AllocateAlignedMemory only

***************************************************************************************************/
void * BDSP_P_ConvertAllocToCache(BMEM_Handle mem, void * ptr);

#define BDSP_MEM_P_AllocateAlignedMemory(memHandle, size, align, boundary)\
        BDSP_P_ConvertAllocToCache(memHandle, BMEM_AllocAligned(memHandle,size,align, boundary) )


/***************************************************************************************

#Use BDSP_MEM_P_ConvertOffsetToCacheAddr or BDSP_MEM_P_ConvertAddressToOffset
#to convert between the cache address and offset whenever required
#In BDSP: Write and Read requires cached address
#In BDSP device interface: Write or Read into DRAM requries virtual offset.

*****************************************************************************************/
BERR_Code BDSP_MEM_P_ConvertOffsetToCacheAddress(
    BMEM_Handle    hHeap,
    uint32_t       ui32Offset,
    void **ppCachedAddr
    );

#ifdef HEAP_DBG


#define BDSP_MEM_P_FlushCache( heap, addr, size) \
    do{ \
        BERR_Code macroErr;\
        macroErr = BMEM_FlushCache(heap, addr,size); \
        if(macroErr != BERR_SUCCESS){ \
            BERR_TRACE(BERR_INVALID_PARAMETER);\
            BDBG_ASSERT(0);\
            }\
        }while(0)

#define BDSP_MEM_P_FlushCache_isr( heap, addr, size) \
            do{ \
                BERR_Code macroErr;\
                macroErr = BMEM_FlushCache_isr(heap, addr,size); \
                if(macroErr != BERR_SUCCESS){ \
                    BERR_TRACE(BERR_INVALID_PARAMETER);\
                    BDBG_ASSERT(0);\
                    }\
                }while(0)

/* mptr should have been assigned to NULL after freeing the pointer but bdsp uses uint32_t for mptr, in some places,
 and assigning NULL, a void *, raises warnings. So assigning to 0 for now. This will avoid double freeing of pointer */

#define BDSP_MEM_P_FreeMemory(memHandle, mptr)\
                    do{ \
                        BERR_Code macroErr;\
                        macroErr = BMEM_FreeCached((BMEM_Handle)memHandle, (void *)mptr); \
                        if(macroErr != BERR_SUCCESS){ \
                            BERR_TRACE(BERR_INVALID_PARAMETER);\
                            BDBG_ASSERT(0);\
                            }\
                        mptr = 0;\
                        }while(0)

#define BDSP_MEM_P_ConvertAddressToOffset( heap,addr,offset)\
                        do{ \
                        BERR_Code macroErr;\
                        macroErr = BMEM_ConvertAddressToOffset(heap, addr, offset);\
                        if(macroErr != BERR_SUCCESS){ \
                            BERR_TRACE(BERR_INVALID_PARAMETER);\
                            BDBG_ASSERT(0);\
                            }\
                        }while(0)

#define BDSP_MEM_P_ConvertAddressToOffset_isr(heap, addr, offset)\
                        do{ \
                        BERR_Code macroErr;\
                        macroErr = BMEM_ConvertAddressToOffset_isr(heap, addr, offset);\
                        if(macroErr != BERR_SUCCESS){ \
                            BERR_TRACE(BERR_INVALID_PARAMETER);\
                            BDBG_ASSERT(0);\
                            }\
                        }while(0)

#define BDSP_MEM_P_ConvertOffsetToCacheAddr(hHeap, ulOffset, ppvAddress) \
                        do{ \
                            BERR_Code macroErr;\
                            macroErr = BDSP_MEM_P_ConvertOffsetToCacheAddress(hHeap, ulOffset, ppvAddress);\
                            if(macroErr != BERR_SUCCESS){ \
                                BERR_TRACE(BERR_INVALID_PARAMETER);\
                                BDBG_ASSERT(0);\
                            }\
                        }while(0)

#elif defined(HEAP_DBG_NO_ASSERT)

#define BDSP_MEM_P_FlushCache( heap, addr, size) \
        do{ \
            BERR_Code macroErr;\
            macroErr = BMEM_FlushCache(heap, addr,size); \
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
        }while(0)

#define BDSP_MEM_P_FlushCache_isr( heap, addr, size) \
        do{ \
            BERR_Code macroErr;\
            macroErr = BMEM_FlushCache_isr(heap, addr,size); \
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
        }while(0)

/* mptr should have been assigned to NULL after freeing the pointer but bdsp uses uint32_t for mptr, in some places,
 and assigning NULL, a void *, raises warnings. So assigning to 0 for now. This will avoid double freeing of pointer */

#define BDSP_MEM_P_FreeMemory(memHandle, mptr)\
        do{ \
            BERR_Code macroErr;\
            macroErr = BMEM_FreeCached((BMEM_Handle)memHandle, (void *)mptr); \
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
            mptr = 0;\
        }while(0)

#define BDSP_MEM_P_ConvertAddressToOffset( heap,addr,offset)\
        do{ \
            BERR_Code macroErr;\
            macroErr = BMEM_ConvertAddressToOffset(heap, addr, offset);\
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
        }while(0)


#define BDSP_MEM_P_ConvertAddressToOffset_isr(heap, addr, offset)\
        do{ \
            BERR_Code macroErr;\
            macroErr = BMEM_ConvertAddressToOffset_isr(heap, addr, offset);\
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
        }while(0)

#define BDSP_MEM_P_ConvertOffsetToCacheAddr(hHeap, ulOffset, ppvAddress) \
        do{ \
            BERR_Code macroErr;\
            macroErr = BDSP_MEM_P_ConvertOffsetToCacheAddress(hHeap, ulOffset, ppvAddress);\
            if(macroErr != BERR_SUCCESS){ \
                BERR_TRACE(BERR_INVALID_PARAMETER);\
            }\
        }while(0)

#else
#define BDSP_MEM_P_ConvertAddressToOffset BMEM_ConvertAddressToOffset

#define BDSP_MEM_P_ConvertAddressToOffset_isr BMEM_ConvertAddressToOffset_isr

/************************************************************************************
#Use BDSP_MEM_P_FlushCache BDSP_MEM_P_FlushCache_isr to Flush the cache
#after writing into the cache address or before reading from the cache address.
**************************************************************************************/

#define BDSP_MEM_P_FlushCache BMEM_FlushCache

#define BDSP_MEM_P_FlushCache_isr BMEM_FlushCache_isr

/*******************
#Use BDSP_MEM_P_FreeMemory to free a memory allocated using BDSP_MEM_P_AllocateAlignedMemory.
********************/
/* mptr should have been assigned to NULL after freeing the pointer but bdsp uses uint32_t for mptr, in some places,
 and assigning NULL, a void *, raises warnings. So assigning to 0 for now. This will avoid double freeing of pointer */

#define BDSP_MEM_P_FreeMemory(memHandle, mptr) \
        do{\
            (void )BMEM_FreeCached((BMEM_Handle)(memHandle), (void *)mptr);\
            mptr = 0;\
        }while(0)


#define BDSP_MEM_P_ConvertOffsetToCacheAddr(hHeap, ulOffset, ppvAddress) \
            BDSP_MEM_P_ConvertOffsetToCacheAddress(hHeap, ulOffset, ppvAddress)

#endif

#endif /*BDSP_COMMON_MM_PRIV_H_*/
