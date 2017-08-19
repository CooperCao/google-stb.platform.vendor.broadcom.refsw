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
enum test {
    test_ePrint, test_eRegisterWrite, test_eRegisterRead, test_eMax
};

struct test_state {
    enum test test;
    bool completed;
    uint32_t pattern0;
    uint32_t pattern1;
};

static bool tracelog_P_TestRegister(void * state_, unsigned count, unsigned n, const struct tracelog_P_Log16 *log16, const struct tracelog_P_Log128 *log128)
{
    struct test_state *state = state_;

    if(log16==NULL) {
        return true;
    }
    BDBG_ASSERT(log128);
    BSTD_UNUSED(count);
    if(state->test==test_eRegisterWrite) {
        if(n==1 || n==2) {
            BDBG_ASSERT(log16->valid);
            BDBG_ASSERT(log16->write);
            BDBG_ASSERT(log16->io);
            BDBG_ASSERT(!log16->data.io._64bit);
            if(n==1) {
                BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_WR_DATA7);
                BDBG_ASSERT(log128->data.io.data == state->pattern0);
            } else {
                BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_WR_DATA6);
                BDBG_ASSERT(log128->data.io.data == state->pattern1);
#if !(BCHP_REGISTER_HAS_64_BIT && defined(BCHP_MEMC_GEN_0_MSA_CMD_ADDR))
                state->completed = true;
#endif
            }
        }
#if BCHP_REGISTER_HAS_64_BIT && defined(BCHP_MEMC_GEN_0_MSA_CMD_ADDR)
        if(n==5 || n==6) {
            uint64_t pattern64 = (state->pattern0)*(uint64_t)8;
            BDBG_ASSERT(log16->valid);
            BDBG_ASSERT(log16->write);
            BDBG_ASSERT(log16->io);
            BDBG_ASSERT(log16->data.io._64bit);
            BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_CMD_ADDR);
            if(n==5) {
                BDBG_ASSERT(log128->data.io.data == (uint32_t)pattern64);
            } else {
                BDBG_ASSERT(log128->data.io.data == (uint32_t)(pattern64>>32));
                state->completed = true;
            }
        }
#endif
    } else if(state->test==test_eRegisterRead) {
        if(n==0 || n==3) {
            BDBG_ASSERT(log16->valid);
            BDBG_ASSERT(!log16->write);
            BDBG_ASSERT(log16->io);
            BDBG_ASSERT(!log16->data.io._64bit);
            if(n==0) {
                BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_RD_DATA7);
            } else {
                BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_WR_DATA7);
#if TRACELOG_IO_RD_DATA_SUPPORTED
                BDBG_ASSERT(log128->data.io.data == state->pattern0);
#endif
#if !(BCHP_REGISTER_HAS_64_BIT && defined(BCHP_MEMC_GEN_0_MSA_CMD_ADDR))
                state->completed = true;
#endif
            }
        }
#if BCHP_REGISTER_HAS_64_BIT && defined(BCHP_MEMC_GEN_0_MSA_CMD_ADDR)
        if(n==8 || n==9) {
            uint64_t pattern64 = (state->pattern0)*(uint64_t)8;
            BDBG_ASSERT(log16->valid);
            BDBG_ASSERT(!log16->write);
            BDBG_ASSERT(log16->io);
            BDBG_ASSERT(log16->data.io._64bit);
            BDBG_ASSERT(log128->data.io.addr == BCHP_PHYSICAL_OFFSET+BCHP_MEMC_GEN_0_MSA_CMD_ADDR);
            if(n==8) {
                BDBG_ASSERT(log128->data.io.data == (uint32_t)pattern64);
            } else {
                BDBG_ASSERT(log128->data.io.data == (uint32_t)(pattern64>>32));
                state->completed = true;
            }
        }
#endif
    }
    return true;
}

static void tracelog_TestOneMemc(BREG_Handle reg, unsigned memc, NEXUS_HeapHandle heap)
{
    enum test test;
    static const char * const testName[] = {
        "Print", "RegisterWrite", "RegisterRead"
    };
    NEXUS_MemoryBlockHandle buffer;
    void *bufferPtr;
    NEXUS_Addr bufferAddr;
    tracelog_MemoryBuffer tracelogBuffer;
    const unsigned bufferSize = 1024*256;
    NEXUS_MemoryBlockHandle memBlock[BLOCK_COUNT];
    NEXUS_Addr memBlockAddr[BLOCK_COUNT];
    const unsigned blockSize = 4096;
    void *ptr[BLOCK_COUNT];
    unsigned i;
    NEXUS_Error rc;
    tracelog_Status status;
    uint32_t pattern0=0xDA7ADA7A;
    uint32_t pattern1=0xBEEFBEEF;

    BDBG_LOG(("Testing TRACELOG MEMC%u %s Buffer", memc, heap?"External":"Internal"));

    rc = tracelog_GetStatus(reg, memc, &status);
    if(rc==BERR_NOT_SUPPORTED) {
        BDBG_LOG(("TRACELOG is not supported"));
        return;
    }
    BDBG_ASSERT(rc==BERR_SUCCESS);

    if(heap) {
        BDBG_MSG(("Allocating MEMC%u", memc));
        buffer = NEXUS_MemoryBlock_Allocate(heap, bufferSize, TRACELOG_MIN_BUFFER_ALIGNMENT, NULL);
        BDBG_ASSERT(buffer);
        rc = NEXUS_MemoryBlock_Lock(buffer, &bufferPtr);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);
        rc = NEXUS_MemoryBlock_LockOffset(buffer, &bufferAddr);
        BDBG_ASSERT(rc==NEXUS_SUCCESS);

        for(i=0;i<BLOCK_COUNT;i++) {
            memBlock[i] = NEXUS_MemoryBlock_Allocate(heap, blockSize, 0, NULL);
            BDBG_ASSERT(memBlock[i]);
        }

        BDBG_MSG(("Lock memc:%u", memc));
        for(i=0;i<BLOCK_COUNT;i++) {
            rc = NEXUS_MemoryBlock_Lock(memBlock[i], &ptr[i]);
            BDBG_ASSERT(rc==NEXUS_SUCCESS);
            rc = NEXUS_MemoryBlock_LockOffset(memBlock[i], &memBlockAddr[i]);
            BDBG_ASSERT(rc==NEXUS_SUCCESS);
            BDBG_MSG(("memc:%u memory[%u] at " BDBG_UINT64_FMT "(%p)", memc, i, BDBG_UINT64_ARG(memBlockAddr[i]), ptr[i]));
        }
    }

    if(heap==NULL) {
        msa_Reset(reg, memc);
        tracelog_PrintStatus(&status);
    }


    if(heap==NULL) {
        rc=tracelog_GetStatus(reg, memc, &status);
        BDBG_ASSERT(rc==BERR_SUCCESS);
        tracelog_PrintStatus(&status);
    }
    tracelog_GetDefaultMemoryBuffer(&tracelogBuffer);
    if(heap!=NULL) {
        tracelogBuffer.base = bufferAddr;
        tracelogBuffer.ptr = bufferPtr;
        tracelogBuffer.size = bufferSize;
    }
    for(test=test_ePrint;test<test_eMax;test++) {
        unsigned i;
        tracelog_IoFilter io;
        tracelog_MemoryFilter mem;
        tracelog_CalibrationData calibrationData;

        for(i=0;i<4;i++) {
            tracelog_DisableFilter(reg, memc, 0);
            tracelog_DisableFilter(reg, memc, 1);
        }

        BDBG_LOG(("TRACELOG MEMC%u %s %u/%u", memc, testName[test], test,test_eMax-1));
        rc=tracelog_Reset(reg, memc);
        BDBG_ASSERT(rc==BERR_SUCCESS);

        if(test == test_ePrint || test==test_eRegisterWrite || test==test_eRegisterRead) {
            tracelog_GetDefaultIoFilter(&io);
            TRACELOG_SET_IO_RANGE(&io, MEMC_GEN_0);
            rc=tracelog_EnableIoFilter(reg, memc, 0, &io, tracelog_FilterMode_eCapture);
            BDBG_ASSERT(rc==BERR_SUCCESS);
        }
        if(heap && test == test_ePrint) {
            tracelog_GetDefaultMemoryFilter(&mem);
            mem.match.address.enabled = true;
            mem.match.address.low = memBlockAddr[0];
            mem.match.address.high = memBlockAddr[0]+blockSize;
            tracelog_EnableMemoryFilter(reg, memc, 1, &mem, tracelog_FilterMode_eCapture);
        }

        if(heap==NULL) {
            rc=tracelog_Start(reg, memc, NULL);
            BDBG_ASSERT(rc==BERR_SUCCESS);
        } else {
            tracelog_StartSettings startSettings;

            tracelog_GetDefaultStartSettings(&startSettings);
            startSettings.buffer = tracelogBuffer;
            rc=tracelog_Start(reg, memc, &startSettings);
            BDBG_ASSERT(rc==BERR_SUCCESS);
        }

        if(heap==NULL) {
            rc=tracelog_GetStatus(reg, memc, &status);
            BDBG_ASSERT(rc==BERR_SUCCESS);
            tracelog_PrintStatus(&status);
        }
        if(test == test_ePrint || test==test_eRegisterWrite || test==test_eRegisterRead) {
            uint32_t data;
            BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_RD_DATA7);
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA7, pattern0);
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
            data = BREG_Read32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA7);
            BDBG_ASSERT(data==pattern0);
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
#if BCHP_REGISTER_HAS_64_BIT && defined(BCHP_MEMC_GEN_0_MSA_CMD_ADDR)
            {
                uint64_t data64;
                uint64_t pattern64 = pattern0*(uint64_t)8;
                BREG_Write64(reg, BCHP_MEMC_GEN_0_MSA_CMD_ADDR, pattern64);
                BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
                data64 = BREG_Read64(reg, BCHP_MEMC_GEN_0_MSA_CMD_ADDR);
                BDBG_ASSERT(pattern64==data64);
            }
#endif
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
            BREG_Write32(reg, BCHP_MEMC_GEN_0_MSA_WR_DATA6, pattern1);
        }
        if(heap) {
            *(uint8_t*)ptr[0] = (uint8_t)pattern0;
            NEXUS_FlushCache(ptr[0], blockSize);
        }

        if(1 || heap==NULL) {
            rc=tracelog_GetStatus(reg, memc, &status);
            BDBG_ASSERT(rc==BERR_SUCCESS);
            tracelog_PrintStatus(&status);
        }
        if(test==test_ePrint) {
            tracelog_Calibrate(reg, memc, &calibrationData);
        }
        rc=tracelog_Stop(reg, memc);
        BDBG_ASSERT(rc==BERR_SUCCESS);

        {
            const tracelog_MemoryBuffer *buffer;

            if(heap==NULL) {
                buffer = NULL;
            } else {
                NEXUS_FlushCache(bufferPtr, bufferSize);
                buffer = &tracelogBuffer;
            }

            if(test==test_ePrint) {
                rc=tracelog_PrintLog(reg, memc, buffer, &calibrationData);
                BDBG_ASSERT(rc==BERR_SUCCESS);
            } else if(test==test_eRegisterRead || test==test_eRegisterWrite) {
                struct test_state state;
                state.test = test;
                state.completed = false;
                state.pattern0 = pattern0;
                state.pattern1 = pattern1;
                tracelog_IterateLog(reg, memc, buffer, tracelog_P_TestRegister, &state);
                BDBG_ASSERT(state.completed);
            }
        }
    }

    BDBG_LOG(("memc:%u Done", memc));

    if(heap) {
        for(i=0;i<BLOCK_COUNT;i++) {
            NEXUS_MemoryBlock_UnlockOffset(memBlock[i]);
        }
        BDBG_MSG(("Freeing"));
        NEXUS_MemoryBlock_Unlock(buffer);
        NEXUS_MemoryBlock_UnlockOffset(buffer);
        NEXUS_MemoryBlock_Free(buffer);
        for(i=0;i<BLOCK_COUNT;i++) {
            NEXUS_MemoryBlock_Unlock(memBlock[i]);
            NEXUS_MemoryBlock_Free(memBlock[i]);
        }
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

    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);

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
                    tracelog_TestOneMemc(&reg, status.memcIndex, NULL);
                }
                if(!memcTested[status.memcIndex]) {
                    if( ((status.memoryType & NEXUS_MEMORY_TYPE_SECURE) != NEXUS_MEMORY_TYPE_SECURE) &&
                        ((status.memoryType&NEXUS_MEMORY_TYPE_APPLICATION_CACHED || status.memoryType&NEXUS_MEMORY_TYPE_ONDEMAND_MAPPED))
                      ) {
                        if(status.free>1*1024*1024) {
                            tracelog_TestOneMemc(&reg, status.memcIndex, heap);
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
