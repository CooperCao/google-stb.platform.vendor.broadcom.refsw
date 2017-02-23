/***************************************************************************
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
 ***************************************************************************/

#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <syscalls.h>

#include "tzioc_tapp.h"

extern "C" long syscall(long, ...);

// Static non-const data from TziocTapp class
tzioc_client_handle TziocTapp::hClient;
int TziocTapp::msgQ;
uint8_t TziocTapp::clientId;

#if ENABLE_PROFILING
#if ENABLE_ARM_PM_PROFILING
uint32_t TziocTapp::pmccntr0[NUM_PROFILING_LEVELS];
uint32_t TziocTapp::pmccntr1[NUM_PROFILING_LEVELS];
#else
struct timespec TziocTapp::tv0[NUM_PROFILING_LEVELS];
struct timespec TziocTapp::tv1[NUM_PROFILING_LEVELS];
#endif
#endif

void TziocTapp::init()
{
#if ENABLE_PROFILING
    profilingInit();
#endif

    // Open TZIOC client
    hClient = tzioc_client_open(
        "tzioc_tapp",
        &msgQ,
        &clientId);

    if (!hClient) {
        LOGE("failed to open TZIOC client");
        throw(-EIO);
    }

    LOGI("TZIOC test app initialized!");
}

void TziocTapp::deinit()
{
    // Close TZIOC client
    if (hClient) {
        tzioc_client_close(hClient);
        hClient = NULL;
    }

    LOGI("TZIOC test app deinitialized!");
}

void TziocTapp::run()
{
    LOGI("TZIOC test app running!");

#if (ENABLE_PROFILING & PROFILING_LEVEL_0)
    // Example of profiling
    LOGI("Profiling level 0 starting...");
    profilingStart(0);

    // Code to be profiled

    profilingStop(0);
    LOGI("Profiling level 0 stopped");

    // Print profiling result
    profilingPrint(0);
#endif

#if (ENABLE_TRACELOG)
    // Example of tracelog
    LOGI("Tracelog starting...");
    syscall(EXT_tracelog_inval); /* invalidate */
    syscall(EXT_tracelog_start); /* start */

    // Log events
    syscall(EXT_tracelog_add, 0x01, 0);

    syscall(EXT_tracelog_stop);
    LOGI("Tracelog stopped");
#endif

    while (1) {
        static uint8_t msg[TZIOC_MSG_SIZE_MAX];
        struct tzioc_msg_hdr *pHdr = (struct tzioc_msg_hdr *)msg;

        // Receive incoming msg
        int err = tzioc_msg_receive(
            hClient,
            pHdr,
            TZIOC_MSG_SIZE_MAX,
            -1);

        if (err) {
            if (err != -EINTR) {
                LOGE("error receiving TZIOC msg, err %d", err);
            }
            continue;
        }

        // Process incoming msg
        msgProc(pHdr);
    }

    LOGI("TZIOC test app finished!");
}

void TziocTapp::msgProc(
    struct tzioc_msg_hdr *pHdr)
{
    switch (pHdr->ucType) {
    case TZIOC_TEST_MSG_HELLO:
        helloProc(pHdr);
        break;
    case TZIOC_TEST_MSG_MEM_ALLOC:
        memAllocProc(pHdr);
        break;
    case TZIOC_TEST_MSG_MAP_PADDR:
        mapPaddrProc(pHdr);
        break;
    case TZIOC_TEST_MSG_MAP_PADDRS:
        mapPaddrsProc(pHdr);
        break;
    default:
        LOGW("unknown tzioc_tapp msg %d", pHdr->ucType);
    }
}

void TziocTapp::helloProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct tzioc_test_msg_hello_cmd *pCmd =
        (struct tzioc_test_msg_hello_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid hello cmd received");
        return;
    }

    LOGD("hello cmd: %s", pCmd->greet);

    // Caution: reused the cmd buffer for rpy.
    struct tzioc_test_msg_hello_rpy *pRpy =
        (struct tzioc_test_msg_hello_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    strcpy(pRpy->reply, "Reply from TZOS TZIOC test app.");

    tzioc_msg_send(hClient, pHdr);
}

void TziocTapp::memAllocProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct tzioc_test_msg_mem_alloc_cmd *pCmd =
        (struct tzioc_test_msg_mem_alloc_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid mem alloc cmd received");
        return;
    }

    LOGD("mem alloc cmd: offset 0x%zx, size 0x%x",
         (size_t)pCmd->offset, pCmd->size);

    // map physical address
    void *vaddr = (void *)tzioc_offset2vaddr(
        hClient,
        pCmd->offset);

    if ((uintptr_t)vaddr == (uintptr_t)-1) {
        LOGE("failed to convert offset to virtual address");
        return;
    }

    // calculate simple checksum
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < pCmd->size; i += sizeof(uint32_t)) {
        checksum += *(uint32_t *)((uintptr_t)vaddr + i);
    }

    // Caution: reused the cmd buffer for rpy.
    struct tzioc_test_msg_mem_alloc_rpy *pRpy =
        (struct tzioc_test_msg_mem_alloc_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

void TziocTapp::mapPaddrProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct tzioc_test_msg_map_paddr_cmd *pCmd =
        (struct tzioc_test_msg_map_paddr_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid map paddr cmd received");
        return;
    }

    LOGD("map paddr cmd: paddr 0x%zx, size 0x%x",
         (size_t)pCmd->paddr, pCmd->size);

    // map physical address
    void *vaddr = tzioc_map_paddr(
        hClient,
        pCmd->paddr,
        pCmd->size,
        pCmd->flags);

    if (!vaddr) {
        LOGE("failed to map physical address");
        return;
    }

    // calculate simple checksum
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < pCmd->size; i += sizeof(uint32_t)) {
        checksum += *(uint32_t *)((uintptr_t)vaddr + i);
    }

    // unmap physical address
    tzioc_unmap_paddr(
        hClient,
        pCmd->paddr,
        pCmd->size);

    // Caution: reused the cmd buffer for rpy.
    struct tzioc_test_msg_map_paddr_rpy *pRpy =
        (struct tzioc_test_msg_map_paddr_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

void TziocTapp::mapPaddrsProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct tzioc_test_msg_map_paddrs_cmd *pCmd =
        (struct tzioc_test_msg_map_paddrs_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid map paddrs cmd received");
        return;
    }

    LOGD("map paddrs cmd: count %d\n", pCmd->count);
    uint8_t count = pCmd->count;

    struct tzioc_mem_region regions[TZIOC_MEM_REGION_MAX];
    for (int idx = 0; idx < count; idx++) {
        LOGD("\t%d: paddr 0x%zx, size 0x%x", idx,
             (size_t)pCmd->paddrs[idx], pCmd->sizes[idx]);

        regions[idx].ulPaddr = pCmd->paddrs[idx];
        regions[idx].ulSize  = pCmd->sizes[idx];
        regions[idx].ulFlags = pCmd->flags[idx];
    }
    LOGD("\n");

    // map physical addresses
    int err = tzioc_map_paddrs(
        hClient,
        count,
        regions);

    if (err) {
        LOGE("failed to map physical addresses");
        return;
    }

    // calculate simple checksum
    uint32_t checksum = 0;
    for (int idx = 0; idx < count; idx++) {
        for (uint32_t i = 0; i < regions[idx].ulSize; i += sizeof(uint32_t)) {
            checksum += *(uint32_t *)((uintptr_t)regions[idx].ulVaddr + i);
        }
    }

    // unmap physical addresses
    tzioc_unmap_paddrs(
        hClient,
        count,
        regions);

    // Caution: reused the cmd buffer for rpy.
    struct tzioc_test_msg_map_paddrs_rpy *pRpy =
        (struct tzioc_test_msg_map_paddrs_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

int main(int argc, char **argv)
{
    try {
        TziocTapp::init();
        TziocTapp::run();
        TziocTapp::deinit();
    }
    catch (int exception) {
        LOGE("fatal error %d", exception);
        LOGI("TZIOC test app terminated abnormally");
        exit(exception);
    }
    catch (...) {
        LOGI("Unhandled exception");
        exit(-1);
    }
    return 0;
}
