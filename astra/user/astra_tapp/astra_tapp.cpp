/***************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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

#include "astra_tapp.h"

// Static non-const data from AstraTapp class
tzioc_client_handle AstraTapp::hClient;
int AstraTapp::msgQ;
uint8_t AstraTapp::clientId;

#if ENABLE_PROFILING
#if ENABLE_ARM_PM_PROFILING
uint32_t AstraTapp::pmccntr0, AstraTapp::pmccntr1;
#else
struct timespec AstraTapp::tv0, AstraTapp::tv1;
#endif
#endif

void AstraTapp::init()
{
#if ENABLE_PROFILING
    profilingInit();
#endif

    // Open TZIOC client
    hClient = tzioc_client_open(
        "astra_tapp",
        &msgQ,
        &clientId);

    if (!hClient) {
        LOGE("failed to open TZIOC client");
        throw(-EIO);
    }

    LOGI("Astra test app initialized!");
}

void AstraTapp::deinit()
{
    // Close TZIOC client
    if (hClient) {
        tzioc_client_close(hClient);
        hClient = NULL;
    }

    LOGI("Astra test app deinitialized!");
}

void AstraTapp::run()
{
    LOGI("Astra test app running!");

#if (ENABLE_PROFILING & PROFILING_LEVEL_0)
    printf("\n");

    for (int rep = 0; rep <= 10; rep++) {

        profilingStart();

        unsigned int a = 10, c = 0;
        for (int i = 0; i < 1000 * rep; i++) {
            c = c + a + i;
        }

        profilingStop();

        printf("Cumulator profiling: rep %d, result %ud, ", rep, c);
        profilingPrint();
        printf("\n");
    }

    printf("\n");
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

    LOGI("Astra test app finished!");
}

void AstraTapp::msgProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct astra_test_msg_hdr *pAHdr =
        (astra_test_msg_hdr *)TZIOC_MSG_PAYLOAD(pHdr);

    switch (pAHdr->ucType) {
    case ASTRA_TEST_MSG_HELLO:
        helloProc(pHdr);
        break;
    case ASTRA_TEST_MSG_MEM_ALLOC:
        memAllocProc(pHdr);
        break;
    case ASTRA_TEST_MSG_MAP_PADDR:
        mapPaddrProc(pHdr);
        break;
    case ASTRA_TEST_MSG_MAP_PADDRS:
        mapPaddrsProc(pHdr);
        break;
    default:
        LOGW("unknown astra_tapp! msg %d", pHdr->ucType);
    }
}

void AstraTapp::helloProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct astra_test_msg_hdr *pAHdr =
        (astra_test_msg_hdr *)TZIOC_MSG_PAYLOAD(pHdr);
    struct astra_test_msg_hello_cmd *pCmd =
        (struct astra_test_msg_hello_cmd *)ASTRA_TEST_MSG_PAYLOAD(pAHdr);

    if (pHdr->ulLen != sizeof(*pAHdr) + sizeof(*pCmd)) {
        LOGE("invalid hello cmd received");
        return;
    }

    LOGD("hello cmd: %s", pCmd->greet);

    // Caution: reused the cmd buffer for rpy.
    struct astra_test_msg_hello_rpy *pRpy =
        (struct astra_test_msg_hello_rpy *)pCmd;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pAHdr) + sizeof(*pRpy);

    strcpy(pRpy->reply, "Reply from TZOS astra test app.");

    tzioc_msg_send(hClient, pHdr);
}

void AstraTapp::memAllocProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct astra_test_msg_hdr *pAHdr =
        (astra_test_msg_hdr *)TZIOC_MSG_PAYLOAD(pHdr);
    struct astra_test_msg_mem_alloc_cmd *pCmd =
        (struct astra_test_msg_mem_alloc_cmd *)ASTRA_TEST_MSG_PAYLOAD(pAHdr);

    if (pHdr->ulLen != sizeof(*pAHdr) + sizeof(*pCmd)) {
        LOGE("invalid hello cmd received");
        return;
    }

    LOGD("mem alloc cmd: offset 0x%x, size 0x%x",
         pCmd->offset, pCmd->size);

    // map physical address
    void *vaddr = (void *)tzioc_offset2vaddr(
        hClient,
        pCmd->offset);

    if (!vaddr) {
        LOGE("failed to convert offset to virtual address");
        return;
    }

    // calculate simple checksum
    uint32_t checksum = 0;
    for (uint32_t i = 0; i < pCmd->size; i += sizeof(uint32_t)) {
        checksum += *(uint32_t *)((uint32_t)vaddr + i);
    }

    LOGD("mem alloc cmd: checksum 0x%x", checksum);

    // Caution: reused the cmd buffer for rpy.
    struct astra_test_msg_mem_alloc_rpy *pRpy =
        (struct astra_test_msg_mem_alloc_rpy *)pCmd;

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pAHdr) + sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

void AstraTapp::mapPaddrProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct astra_test_msg_map_paddr_cmd *pCmd =
        (struct astra_test_msg_map_paddr_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid map paddr cmd received");
        return;
    }

    LOGD("map paddr cmd: paddr 0x%x, size 0x%x",
         pCmd->paddr, pCmd->size);

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
        checksum += *(uint32_t *)((uint32_t)vaddr + i);
    }

    // unmap physical address
    tzioc_unmap_paddr(
        hClient,
        pCmd->paddr,
        pCmd->size);

    // Caution: reused the cmd buffer for rpy.
    struct astra_test_msg_map_paddr_rpy *pRpy =
        (struct astra_test_msg_map_paddr_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

void AstraTapp::mapPaddrsProc(
    struct tzioc_msg_hdr *pHdr)
{
    struct astra_test_msg_map_paddrs_cmd *pCmd =
        (struct astra_test_msg_map_paddrs_cmd *)TZIOC_MSG_PAYLOAD(pHdr);

    if (pHdr->ulLen != sizeof(*pCmd)) {
        LOGE("invalid map paddrs cmd received");
        return;
    }

    LOGD("map paddrs cmd: count %d\n", pCmd->count);
    uint8_t count = pCmd->count;

    struct tzioc_mem_region regions[TZIOC_MEM_REGION_MAX];
    for (int idx = 0; idx < count; idx++) {
        LOGD("\t%d: paddr 0x%x, size 0x%x", idx,
             pCmd->paddrs[idx], pCmd->sizes[idx]);

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
            checksum += *(uint32_t *)((uint32_t)regions[idx].ulVaddr + i);
        }
    }

    // unmap physical addresses
    tzioc_unmap_paddrs(
        hClient,
        count,
        regions);

    // Caution: reused the cmd buffer for rpy.
    struct astra_test_msg_map_paddrs_rpy *pRpy =
        (struct astra_test_msg_map_paddrs_rpy *)TZIOC_MSG_PAYLOAD(pHdr);

    pHdr->ucDest = pHdr->ucOrig;
    pHdr->ucOrig = clientId;
    pHdr->ulLen  = sizeof(*pRpy);

    pRpy->checksum = checksum;

    tzioc_msg_send(hClient, pHdr);
}

int main(int argc, char **argv)
{
    try {
        AstraTapp::init();
        AstraTapp::run();
        AstraTapp::deinit();
    }
    catch (int exception) {
        LOGE("fatal error %d", exception);
        LOGI("Astra test app terminated abnormally");
        exit(exception);
    }
    return 0;
}
