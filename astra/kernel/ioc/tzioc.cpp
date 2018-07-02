/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 ******************************************************************************/


#include "astra_version.h"

#include "tzioc.h"
#include "tzioc_msg.h"
#include "tzioc_mem.h"
#include "tzioc_client.h"
#include "tzioc_ioctl.h"
#include "tzioc_sys_msg.h"

#include "plat_config.h"
#include "kernel.h"
#include "pgtable.h"
#include "hwtimer.h"
#include "arm/gic.h"
#include "libfdt.h"
#include "parse_utils.h"
#include "lib_printf.h"
#include "vuartfops.h"
#include "tracelog.h"

// work-around c++ compiler problem with errno
#include <bits/errno.h>

// Static non-const data from TzIoc class
uintptr_t TzIoc::smemStart;
uintptr_t TzIoc::smemSize;
uint32_t TzIoc::sysIrq;
struct tzioc_client *TzIoc::psysClient;
SpinLock TzIoc::lock;
struct tzioc_shared_mem *TzIoc::psmem = NULL;
bool TzIoc::peerUp = false;
uint64_t TzIoc::tzStartTicks = 0;
uint64_t TzIoc::nwStartTicks = 0;

void TzIoc::init(void *devTree)
{
    // Parse the 'tzioc' node in the device tree
    //
    // Example node:
    //
    // tzioc {
    //    compatible = "brcm,tzioc";
    //    reg = <0x0 0x7c000000 0x0 0x2000000>; // shared memory
    //    irq = <0xf>;                          // system IRQ (SGI)
    // }
    //
    // Note:
    //
    //     The 'reg' property of the 'tzioc' node uses the defines of
    //     #address-cells and #size-cells of the parent node.

    // Get parent node
    int parentOffset = 0;
    int propLen;

    // Parse #address-cells property of parent node
    unsigned long adCellSize;
    const struct fdt_property *fpAdCells =
        fdt_get_property(devTree, parentOffset, "#address-cells", &propLen);

    if ((!fpAdCells) || (propLen < sizeof(int)))
        adCellSize = 1;
    else
        adCellSize = parseInt((void *)fpAdCells->data, propLen);

    // Parse #size-cells property of parent node
    unsigned long szCellSize;
    const struct fdt_property *fpSzCells =
        fdt_get_property(devTree, parentOffset, "#size-cells", &propLen);

    if ((!fpSzCells) || (propLen < sizeof(int)))
        szCellSize = 1;
    else
        szCellSize = parseInt((void *)fpSzCells->data, propLen);

    int adByteSize = adCellSize * sizeof(int);
    int szByteSize = szCellSize * sizeof(int);

    // Get tzioc node
    int nodeOffset = fdt_subnode_offset(devTree, 0, "tzioc");
    if (nodeOffset < 0) {
        kernelHalt("No tzioc node found in the device tree");
    }

    // Parse 'reg' property
    const struct fdt_property *fpMemRanges =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);

    if ((!fpMemRanges) || (propLen != adByteSize + szByteSize))
        kernelHalt("Invalid memory range found in device tree tzioc node.");

    const char *rangeData = fpMemRanges->data;
    smemStart = (uintptr_t)((adCellSize == 1) ?
         parseInt(rangeData, adByteSize) :
         parseInt64(rangeData, adByteSize));
    rangeData += adByteSize;

    smemSize = (uint32_t)((szCellSize == 1) ?
         parseInt(rangeData, szByteSize) :
         parseInt64(rangeData, szByteSize));

    printf("TzIoc shared memory at phys address 0x%x, size 0x%x\n",
           (unsigned int)smemStart, (unsigned int)smemSize);

    // Parse 'irq' property
    const struct fdt_property *fpIrq =
        fdt_get_property(devTree, nodeOffset, "irq", &propLen);

    if ((!fpIrq) || (propLen != sizeof(int)))
        kernelHalt("Invalid irq property found in device tree tzioc node.");

    sysIrq = parseInt((void *)fpIrq->data, propLen);

    printf("TzIoc system IRQ %d\n", (unsigned int)sysIrq);

    // Map shared memory
    PageTable *kernPageTable = PageTable::kernelPageTable();

    void *pageStart = PAGE_START_4K(smemStart);
    void *virtPageStart = ARCH_SHMEM_BASE ? (void *)(ARCH_SHMEM_BASE) : pageStart;
    void *virtPageEnd   = (void *)((uintptr_t)virtPageStart + smemSize - 1);
    kernPageTable->mapPageRange(
        virtPageStart,             // virtual start
        virtPageEnd,               // virtual end
        pageStart,             // physical start
        MAIR_MEMORY,           // cached memory
        MEMORY_ACCESS_RW_USER, // user read/write
        true,                  // never execute
        true,                  // shared memory
        true);                 // non-secure

    // Shared memory could be mapped to a different virtual address
    psmem = (struct tzioc_shared_mem *)(uintptr_t)virtPageStart;
    printf("TzIoc shared memory virt address 0x%zx\n",
           (size_t)psmem);

    if (!psmem)
        kernelHalt("Failed to map shared memory");

    // Init shared memory
    memset(psmem, 0, sizeof(*psmem));
    psmem->ulMagic = ASTRA_VERSION_WORD;
    psmem->secure = false;

    // Init spinlock
    spinLockInit(&lock);

    // Init ioctl module
    TzIocIoctl::init(devTree);

    // Init msg module
    TzIocMsg::init(devTree);

    // Init mem module
    TzIocMem::init(devTree);

    // Init client module
    TzIocClient::init(devTree);

    // Open system client
    psysClient = TzIocClient::kernelClientOpen(
        "tzioc_system",
        NULL, // sys msgs dispatched directly
        0); // private data not used

    if (!psysClient)
        kernelHalt("Failed to open TzIoc system client");

    printf("TzIoc initialized\n");
}

void TzIoc::proc()
{
    // Return if not initialized
    if (!psmem) return;

    // Update cpu time
    if (arm::smpCpuNum() == 0) {
        tzStartTicks = TzHwCounter::timeNow();
        psmem->ullNwUsecs += TzHwCounter::ticksToUs(tzStartTicks - nwStartTicks);
    }

    // Process incoming msgs
    while (1) {
        static uint8_t msg[TZIOC_MSG_SIZE_MAX];
        struct tzioc_msg_hdr *pHdr = (struct tzioc_msg_hdr *)msg;

        // Get received msg
        int err = TzIocMsg::receive(
            psysClient,
            pHdr,
            TZIOC_MSG_PAYLOAD(pHdr),
            TZIOC_MSG_PAYLOAD_MAX);

        if (err == -ENOMSG)
            break;

        if (err) {
            printf("Error receiving msg, err %d\n", err);
            continue;
        }

        // Dispatch received msg
        if (pHdr->ucDest == TZIOC_CLIENT_ID_SYS) {
            err = sysMsgProc(pHdr);
            if (err) {
                printf("Failed to process sys msg\n");
            }
        }
#if TZIOC_MSG_ECHO
        else if (pHdr->ucDest == TZIOC_CLIENT_ID_MAX) {
            err = echoMsgProc(pHdr);
            if (err) {
                printf("Failed to process echo msg\n");
            }
        }
#endif
        else {
            struct tzioc_client *pClient =
                TzIocClient::clientFindById(pHdr->ucDest);

            if (!pClient) {
                printf("Unknown msg dest %d\n", pHdr->ucDest);
                continue;
            }

            if (pClient->kernel) {
                // Call kernel msg processing func directly
                pClient->msgProc(pHdr, pClient->privData);
            }
            else {
                // Send to user msg queue
                TzTask *pTask = (TzTask *)(uintptr_t)pClient->task;
                pTask->mqSend(
                    pClient->msgQ,
                    (const char *)pHdr,
                    sizeof(*pHdr) + pHdr->ulLen,
                    0, -1);
            }
        }
    }
}

void TzIoc::notify()
{
    // Return if not initialized
    if (!psmem) return;

    if (peerUp) {
        // printf("TzIoc system notify\n");
        GIC::sgiGenerate(1, sysIrq);
    }

    // Update cpu time
    if (arm::smpCpuNum() == 0) {
        nwStartTicks = TzHwCounter::timeNow();
        // Skip counting if start ticks NOT initialized
        psmem->ullTzUsecs += (tzStartTicks) ?
            TzHwCounter::ticksToUs(nwStartTicks - tzStartTicks) : 0;
    }
}

void TzIoc::cleanupTask(TzTask *pTask)
{
    // Return if not initialized
    if (!psmem) return;

    struct tzioc_client *pClient;
    while ((pClient = TzIocClient::clientFindByTask(pTask))) {
        TzIocClient::userClientClose(pClient);
    }
}

int TzIoc::sysMsgProc(
    struct tzioc_msg_hdr *pHdr)
{
    int err = 0;

    printf("TzIoc system msg processing\n");

    switch (pHdr->ucType) {
    // System up cmd
    case SYS_MSG_UP:
        if (pHdr->ucOrig != TZIOC_CLIENT_ID_SYS ||
            pHdr->ulLen != 0) {
            printf("Invalid system up cmd received\n");
            err = -EINVAL;
            break;
        }

        err = TzIocMsg::send(
            psysClient,
            pHdr, NULL);

        if (err) {
            printf("Failed to send system up rpy\n");
            break;
        }

        peerUp = true;
        printf("TzIoc peer is up\n");

        // Notify other modules
        VuartFops::peerUp();
        TraceLog::peerUp();
        break;

    // System down cmd
    case SYS_MSG_DOWN:
        if (pHdr->ucOrig != TZIOC_CLIENT_ID_SYS ||
            pHdr->ulLen != 0) {
            printf("Invalid system down cmd received\n");
            err = -EINVAL;
            break;
        }

        err = TzIocMsg::send(
            psysClient,
            pHdr, NULL);

        if (err) {
            printf("Failed to send system down rpy\n");
            break;
        }

        peerUp = false;
        printf("TzIoc peer is down\n");

        // Notify other modules
        VuartFops::peerDown();
        TraceLog::peerDown();
        break;

    default:
        printf("Unknown system msg %d\n", pHdr->ucType);
        err = -ENOENT;
    }
    return err;
}

#if TZIOC_MSG_ECHO
int TzIoc::echoMsgProc(
    struct tzioc_msg_hdr *pHdr)
{
    printf("TzIoc echo msg processing\n");

    uint8_t id = pHdr->ucOrig;
    pHdr->ucOrig = pHdr->ucDest;
    pHdr->ucDest = id;

    int err = TzIocMsg::send(
        psysClient,
        pHdr,
        TZIOC_MSG_PAYLOAD(pHdr));

    if (err) {
        printf("Failed to send echo msg\n");
    }
    return err;
}
#endif

int TzIoc::ioctl(uint32_t fd, unsigned long cmd, uintptr_t arg)
{
    // Return if not initialized
    if (!psmem) return -ENODEV;

    if (fd != TZIOC_DEV_DEFAULT_FD) {
        printf("Invalid fd 0x%x in ioctl call\n", (unsigned int)fd);
        return -EBADF;
    }

    if (cmd >= TZIOC_IOCTL_LAST) {
        printf("Invalid cmd 0x%x in ioctl call\n", (unsigned int)cmd);
        return -EINVAL;
    }

    return TzIocIoctl::doIoctl(cmd, (void *)arg);
}

// Export functions to common code

#ifdef __cplusplus
extern "C" {
#endif

uintptr_t _tzioc_offset2addr(uintptr_t ulOffset)
{
    return TzIoc::offset2addr(ulOffset);
}

uintptr_t _tzioc_addr2offset(uintptr_t ulAddr)
{
    return TzIoc::addr2offset(ulAddr);
}

#ifdef __cplusplus
}
#endif
