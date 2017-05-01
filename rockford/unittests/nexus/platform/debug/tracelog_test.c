/******************************************************************************
* Copyright (C) 2016-2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

BDBG_MODULE(tracelog_test);

#include "../dbg/breg_application.h"
#include "../dbg/memc_msa.h"
#include "../dbg/memc_tracelog.h"

#define BLOCK_COUNT 4
static void tracelog_TestOneMemmc(BREG_Handle reg, unsigned memc, NEXUS_HeapHandle heap)
{
    NEXUS_MemoryBlockHandle memBlock[BLOCK_COUNT];
    NEXUS_Addr unsecureMemory[BLOCK_COUNT];
    const unsigned blockSize = 4096;
    void *mem[BLOCK_COUNT];
    void *ptr[BLOCK_COUNT];
    unsigned i;
    NEXUS_Error rc;
    int result;
    tracelog_Status status;
    tracelog_StartSettings start;
    tracelog_IoFilter io;
    uint32_t pattern0=0xDA7ADA7A;
    uint32_t pattern1=0xBEEFBEEF;

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

    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);
    tracelog_Reset(reg, memc);


    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);
    tracelog_GetDefaultIoFilter(&io);
    TRACELOG_SET_IO_RANGE(&io, MEMC_GEN_0);
    tracelog_EnableIoFilter(reg, memc, 0, &io, tracelog_FilterMode_eCapture);
    tracelog_Start(reg, memc, NULL);
    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);

    BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_RD_DATA7);
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA7, pattern0);
    BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA7, pattern1);
    BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA7);

    tracelog_GetStatus(reg, memc, &status);
    tracelog_PrintStatus(&status);
    tracelog_Stop(reg, memc);

    tracelog_PrintLog(reg, memc, NULL);

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

    if(!tracelog_Supported()) {
        BDBG_LOG(("TRACELOG is not supported on this platform"));
        goto done;
    }

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
                            tracelog_TestOneMemmc(&reg, status.memcIndex, heap);
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

done:
    NEXUS_Platform_Uninit();
    return 0;
}
