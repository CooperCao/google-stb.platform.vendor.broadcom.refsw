/******************************************************************************
* Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. , WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>

BDBG_MODULE(msa_test);

#include "../dbg/breg_application.h"
#include "../dbg/memc_msa.h"

static void msa_memcpy(BREG_Handle reg, unsigned memc, NEXUS_Addr dst_addr, NEXUS_Addr src_addr, size_t size)
{
    struct msa_data data;
    unsigned i;

    BDBG_ASSERT(dst_addr%sizeof(data.jw[0].data)==0);
    BDBG_ASSERT(src_addr%sizeof(data.jw[0].data)==0);
    BDBG_ASSERT(size%sizeof(data.jw[0].data)==0);
    for(i=0;i<size/sizeof(data.jw[0].data);i++) {
        msa_ExecuteRead(reg, memc, src_addr+i*sizeof(data.jw[0].data),sizeof(data.jw[0].data), &data);
        msa_PrepareWrite(dst_addr+i*sizeof(data.jw[0].data), sizeof(data.jw[0].data), &data);
        msa_ExecuteWrite(reg, memc, &data);
    }
    return;
}

static void msa_memset(BREG_Handle reg, unsigned memc, NEXUS_Addr addr, int c, size_t size)
{
    struct msa_data data;
    unsigned i;

    BDBG_ASSERT(addr%sizeof(data.jw[0].data)==0);
    BDBG_ASSERT(size%sizeof(data.jw[0].data)==0);
    BKNI_Memset(data.jw[0].data,c,sizeof(data.jw[0].data));
    for(i=0;i<size/sizeof(data.jw[0].data);i++) {
        msa_PrepareWrite(addr+i*sizeof(data.jw[0].data), sizeof(data.jw[0].data), &data);
        msa_ExecuteWrite(reg, memc, &data);
    }
    return;
}


static void msa_readTest(BREG_Handle reg, unsigned memc, NEXUS_Addr addr, unsigned offset)
{
    struct msa_data data;
    unsigned i;

    msa_ExecuteRead(reg, memc, addr+offset, sizeof(data.jw[0].data), &data);
    if(0) {
        for(i=0;i<sizeof(data.jw[0].data)/sizeof(data.jw[0].data[0]);i++) {
            BDBG_MSG(("%u %u %#x", offset, i, (unsigned)data.jw[0].data[i]));
        }
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint32_t)) {
        uint32_t d = msa_Fetch32(addr+i+offset, &data);
        BDBG_MSG(("32: " BDBG_UINT64_FMT " %u %u %#x", BDBG_UINT64_ARG(addr), offset, i, (unsigned)d));
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint8_t)) {
        uint8_t d = msa_Fetch8(addr+i+offset, &data);
        BDBG_MSG(("8: %u %u %#x", offset, i, (unsigned)d));
        BDBG_ASSERT(d == i+offset);
    }
    return;
}

static void msa_writeTest(BREG_Handle reg, unsigned memc, NEXUS_Addr addr, void *optr, unsigned offset, unsigned blockSize)
{
    BERR_Code rc;
    unsigned i;
    struct msa_data data;
    void *ptr;

    addr += offset;
    ptr = (uint8_t *)optr + offset;
    NEXUS_FlushCache(optr, blockSize);
    rc = msa_PrepareWrite(addr, sizeof(data.jw[0].data), &data);
    BDBG_ASSERT(rc==BERR_SUCCESS);
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint32_t)) {
        msa_Store32(addr+i, i+i*offset, &data);
    }
    rc = msa_ExecuteWrite(reg, memc, &data);
    BDBG_ASSERT(rc==BERR_SUCCESS);
    NEXUS_FlushCache(optr, blockSize);
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint32_t)) {
        uint32_t data = *(uint32_t *)(((uint8_t *)ptr)+i);
        BDBG_MSG(("JWORD:" BDBG_UINT64_FMT " %u: %u %#x(%#x)", BDBG_UINT64_ARG(addr), offset, i, data, i+i*offset));
        BDBG_ASSERT(data==i+i*offset);
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint32_t)) {
        *(uint32_t *)(((uint8_t *)ptr)+i) = i;
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint32_t)) {
        uint32_t test_data = i;
        unsigned j;

        NEXUS_FlushCache(optr, blockSize);
        test_data = ~test_data;
        BKNI_Memset(&data, 0xAA, sizeof(data));
        rc = msa_PrepareWrite(addr + i, sizeof(uint32_t), &data);
        BDBG_ASSERT(rc==BERR_SUCCESS);
        msa_Store32(addr+i, test_data, &data);
        rc = msa_ExecuteWrite(reg, memc, &data);
        BDBG_ASSERT(rc==BERR_SUCCESS);
        NEXUS_FlushCache(optr, blockSize);
        for(j=0;j<sizeof(data.jw[0].data);j+=sizeof(uint32_t)) {
            uint32_t data = *(uint32_t *)(((uint8_t *)ptr)+j);
            uint32_t expected = j;
            if(i==j) {
                expected = test_data;
                BDBG_MSG(("WORD: %u: %u/%u %#x(%#x)", offset, i, j, (unsigned)data, (unsigned)expected));
            }
            BDBG_ASSERT(data==expected);
        }
        *(uint32_t *)(((uint8_t *)ptr)+i) = i;
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint8_t)) {
        *(((uint8_t *)ptr)+i) = i;
    }
    for(i=0;i<sizeof(data.jw[0].data);i+=sizeof(uint8_t)) {
        uint8_t test_data = i;
        unsigned j;

        NEXUS_FlushCache(optr, blockSize);
        test_data = ~test_data;
        BKNI_Memset(&data, 0xAA, sizeof(data));
        rc = msa_PrepareWrite(addr + i, sizeof(uint8_t), &data);
        BDBG_ASSERT(rc==BERR_SUCCESS);
        msa_Store8(addr+i, test_data, &data);
        rc = msa_ExecuteWrite(reg, memc, &data);
        BDBG_ASSERT(rc==BERR_SUCCESS);
        NEXUS_FlushCache(optr, blockSize);
        for(j=0;j<sizeof(data.jw[0].data);j+=sizeof(uint8_t)) {
            uint8_t data = *(((uint8_t *)ptr)+j);
            uint8_t expected = j;
            if(i==j) {
                expected = test_data;
                BDBG_MSG(("BYTE: %u: %u/%u %#x(%#x)", offset, i, j, (unsigned)data, (unsigned)expected));
            }
            BDBG_ASSERT(data==expected);
        }
        *(((uint8_t *)ptr)+i) = i;
    }
    NEXUS_FlushCache(optr, blockSize);
}

#define BLOCK_COUNT 4
static void msa_TestOneMemmc(BREG_Handle reg, unsigned memc, NEXUS_HeapHandle heap)
{
    NEXUS_MemoryBlockHandle memBlock[BLOCK_COUNT];
    NEXUS_Addr unsecureMemory[BLOCK_COUNT];
    const unsigned blockSize = 4096;
    void *mem[BLOCK_COUNT];
    void *ptr[BLOCK_COUNT];
    unsigned i;
    NEXUS_Error rc;
    int result;
    struct msa_data data;

    BDBG_LOG(("Allocating memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        memBlock[i] = NEXUS_MemoryBlock_Allocate(heap, blockSize, 0, NULL);
        BDBG_ASSERT(memBlock[i]);
        mem[i] = BKNI_Malloc(blockSize);
        BDBG_ASSERT(mem[i]);
    }

    BDBG_LOG(("memset memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        BKNI_Memset(mem[i], i+1, blockSize);
    }
    BDBG_LOG(("Lock memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_Addr addr;
        rc = NEXUS_MemoryBlock_Lock(memBlock[i], &ptr[i]);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        rc = NEXUS_MemoryBlock_LockOffset(memBlock[i], &addr);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        BDBG_LOG(("mcmc:%u memory[%u] at " BDBG_UINT64_FMT "(%p)", memc, i, BDBG_UINT64_ARG(addr), ptr[i]));
        NEXUS_MemoryBlock_UnlockOffset(memBlock[i]);
    }

    BDBG_LOG(("Testing MSA memc:%u", memc));

    rc = msa_Reset(reg, memc);
    BDBG_ASSERT(rc==BERR_SUCCESS);

    for(i=0;i<BLOCK_COUNT;i++) {
        rc = NEXUS_MemoryBlock_LockOffset(memBlock[i], &unsecureMemory[i]);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
    }


    BDBG_LOG(("MSA:%u READ TESTS ...", memc));
    for(i=0;i<blockSize;i++) {
        ((uint8_t *)ptr[0])[i] = (uint8_t)i;
    }
    NEXUS_FlushCache(ptr[0], blockSize);
    msa_readTest(reg, memc, unsecureMemory[0], 0);
    msa_readTest(reg, memc, unsecureMemory[0], 16);
    msa_readTest(reg, memc, unsecureMemory[0], 32);

    BDBG_LOG(("MSA:%u WRITE TESTS ...", memc));

    msa_writeTest(reg, memc, unsecureMemory[0], ptr[0], 0, blockSize);
    msa_writeTest(reg, memc, unsecureMemory[0], ptr[0], 16, blockSize);
    msa_writeTest(reg, memc, unsecureMemory[0], ptr[0], 32, blockSize);

    BDBG_LOG(("memset memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i], blockSize);
        msa_memset(reg, memc, unsecureMemory[i], i+1, blockSize);
        BKNI_Memset(mem[i], i+1, blockSize);
    }

    BDBG_LOG(("compare after memset memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i], blockSize);
        result = BKNI_Memcmp(mem[i], ptr[i], blockSize);
        BDBG_ASSERT(result==0);
    }


    BDBG_LOG(("memset memc:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        msa_memset(reg, memc, unsecureMemory[i], i+1, blockSize);
        BKNI_Memset(mem[i], i+1, blockSize);
    }

    BDBG_LOG(("memcpy:%u", memc));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i], blockSize);
    }
    for(i=1;i<BLOCK_COUNT;i++) {
        msa_memcpy(reg, memc, unsecureMemory[i-1], unsecureMemory[i], blockSize);
    }
    BDBG_LOG(("compare after memcpy memc:%u", memc));
    for(i=1;i<BLOCK_COUNT;i++) {
        NEXUS_FlushCache(ptr[i-1], blockSize);
        result = BKNI_Memcmp(mem[i], ptr[i-1], blockSize);
        BDBG_ASSERT(result==0);
    }
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_MemoryBlock_UnlockOffset(memBlock[i]);
    }
    BDBG_LOG(("memc:%u Done", memc));


    BDBG_LOG(("Freeing"));
    for(i=0;i<BLOCK_COUNT;i++) {
        NEXUS_MemoryBlock_Unlock(memBlock[i]);
        NEXUS_MemoryBlock_Free(memBlock[i]);
        BKNI_Free(mem[i]);
    }
    return;
}

int main(int argc, const char *argv[])
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    struct BREG_Impl reg;
    bool memcTested[NEXUS_MAX_MEMC];
    bool memcFound[NEXUS_MAX_MEMC];
    unsigned i;
    unsigned memcCount;
    unsigned memcTestedCount;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);

    rc = REG_open(&reg);
    BDBG_ASSERT(rc==BERR_SUCCESS);

    NEXUS_Platform_GetConfiguration(&platformConfig);
    for(i=0;i<NEXUS_MAX_MEMC;i++) {
        memcTested[i] = false;
        memcFound[i] = false;
    }
    memcCount = 0;
    memcTestedCount = 0;
    for(i=0;i<NEXUS_MAX_HEAPS;i++) {
        NEXUS_HeapHandle heap = platformConfig.heap[i];
        if(heap) {
            NEXUS_MemoryStatus status;
            rc = NEXUS_Heap_GetStatus(heap, &status);
            if(rc==NEXUS_SUCCESS) {
                BDBG_ASSERT(status.memcIndex<NEXUS_MAX_MEMC);
                if(!memcFound[status.memcIndex]) {
                    memcCount++;
                    memcFound[status.memcIndex] = true;
                }
                if(!memcTested[status.memcIndex]) {
                    if( ((status.memoryType & NEXUS_MEMORY_TYPE_SECURE) != NEXUS_MEMORY_TYPE_SECURE) &&
                        ((status.memoryType&NEXUS_MEMORY_TYPE_APPLICATION_CACHED || status.memoryType&NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED))
                      ) {
                        if(status.free>1*1024*1024) {
                            msa_TestOneMemmc(&reg, status.memcIndex, heap);
                            memcTestedCount++;
                            memcTested[status.memcIndex] = true;
                        }
                    }
                }
            }
        }
    }
    BDBG_LOG(("Memory Controllers Found:%u Tested:%u", memcCount, memcTestedCount));

    REG_close(&reg);

    NEXUS_Platform_Uninit();
    return 0;
}
