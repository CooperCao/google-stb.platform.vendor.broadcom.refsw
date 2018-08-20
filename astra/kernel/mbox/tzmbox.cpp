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

#include "plat_config.h"
#include "kernel.h"
#include "pgtable.h"
#include "arm/gic.h"
#include "interrupt.h"
#include "libfdt.h"
#include "parse_utils.h"
#include "lib_printf.h"
#include "tzioc.h"
#include "tzmbox.h"

/* Header files shared with monitor */
#include "brcmstb_mbox.h"
#include "astra_msg.h"

// Static non-const data from TzMBox class
SpinLock TzMBox::lock;
uint64_t TzMBox::mboxAddr;
uint32_t TzMBox::mboxSize;
uint8_t TzMBox::mboxSgi;
struct brcmstb_mbox *TzMBox::pmbox;

ISRStatus TzMBox::mboxISR(int intrNum) {
    UNUSED(intrNum);
    proc();
    return IntrDone;
}

void TzMBox::init(void *devTree)
{
    // Parse the 'tzmbox' node in the device tree
    //
    // Example node:
    //
    // mailbox {
    //    compatible = "brcm,mailbox";
    //    reg = <0x0 0x7c000000 0x0 0x1000>; // mailbox memory
    //    irq = <0xe>;                       // mailbox IRQ (SGI)
    // }
    //
    // Note:
    //
    //     The 'reg' property of the 'mailbox' node uses the defines of
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

    // Get mailbox node
    int nodeOffset = fdt_subnode_offset(devTree, 0, "mailbox");
    if (nodeOffset < 0) {
        printf("No mailbox node found in the device tree\n");
        return;
    }

    // Parse 'reg' property
    const struct fdt_property *fpMemRanges =
        fdt_get_property(devTree, nodeOffset, "reg", &propLen);

    if ((!fpMemRanges) || (propLen != adByteSize + szByteSize))
        kernelHalt("Invalid memory range found in device tree mailbox node.");

    const char *rangeData = fpMemRanges->data;
    mboxAddr = (uintptr_t)((adCellSize == 1) ?
         parseInt(rangeData, adByteSize) :
         parseInt64(rangeData, adByteSize));
    rangeData += adByteSize;

    mboxSize = (uint32_t)((szCellSize == 1) ?
         parseInt(rangeData, szByteSize) :
         parseInt64(rangeData, szByteSize));

    printf("TzMBox mailbox at phys address 0x%x, size 0x%x\n",
           (unsigned int)mboxAddr, (unsigned int)mboxSize);

    // Parse 'irq' property
    const struct fdt_property *fpIrq =
        fdt_get_property(devTree, nodeOffset, "irq", &propLen);

    if ((!fpIrq) || (propLen != sizeof(int)))
        kernelHalt("Invalid irq property found in device tree mailbox node.");

    mboxSgi = parseInt((void *)fpIrq->data, propLen);

    printf("TzMBox SGI %d\n", (unsigned int)mboxSgi);

    // Map mailbox memory
    PageTable *kernPageTable = PageTable::kernelPageTable();

    void *pageStart = PAGE_START_4K(mboxAddr);
    void *virtPageStart = ARCH_MBOX_BASE ? (void *)(ARCH_MBOX_BASE) : pageStart;
    void *virtPageEnd   = (void *)((uintptr_t)virtPageStart + mboxSize - 1);
    kernPageTable->mapPageRange(
        virtPageStart,           // virtual start
        virtPageEnd,             // virtual end
        pageStart,               // physical start
        MAIR_DEVICE,             // device memory
        MEMORY_ACCESS_RW_KERNEL, // kernel read/write
        true,                    // never execute
        false,                   // non-shared memory
        false);                  // secure

    // Shared memory could be mapped to a different virtual address
    pmbox = (struct brcmstb_mbox *)(uintptr_t)virtPageStart;
    printf("TzMBox mailbox at virt address 0x%zx\n",
           (size_t)pmbox);

    if (!pmbox)
        kernelHalt("Failed to map shared memory");

    // Enable IRQ and register ISR
    GIC::intrEnable(mboxSgi);
    Interrupt::isrRegister(mboxSgi, mboxISR);

    // Init spinlock
    spinLockInit(&lock);
}

void TzMBox::proc()
{
    if (!pmbox || !(pmbox->status & 0x1))
        return;

    brcmstb_msg_hdr_t *phdr =
        (brcmstb_msg_hdr_t *)&pmbox->msg_hdr;

    info_msg("MBox msg received: 0x%x", (unsigned)phdr->id);

    brcmstb_msg_payload_t *payload =
        (brcmstb_msg_payload_t *)pmbox->msg_payload;

    if (phdr->prot != BRCMSTB_MSG_PROT_ASTRA) {
        payload->status = BRCMSTB_MSG_NOT_SUPPORTED;
        goto REPLY;
    }

    switch (phdr->id) {
    case ASTRA_MSG_VERSION:
    case ASTRA_MSG_ATTRIBUTES:
    case ASTRA_MSG_NSG_ATTRIBUTES:
    case ASTRA_MSG_CPU_OFF:
    case ASTRA_MSG_CPU_SUSPEND:
    case ASTRA_MSG_CPU_RESUME:
    case ASTRA_MSG_SYSTEM_OFF:
        payload->status = BRCMSTB_MSG_SUCCESS;
        break;
    case ASTRA_MSG_SYSTEM_SUSPEND:
        astra_msg_system_suspend_t *psystem_suspend;
        uint64_t smem_addr;
        uint32_t smem_size;

        psystem_suspend = (astra_msg_system_suspend_t *)payload;
        smem_addr = TzIoc::smemStart;
        smem_size = TzIoc::smemSize;

        psystem_suspend->status = BRCMSTB_MSG_SUCCESS;
        psystem_suspend->smem_addr_hi = smem_addr >> 32;
        psystem_suspend->smem_addr_lo = smem_addr & 0xffffffff;
        psystem_suspend->smem_size = smem_size;
        break;
    default:
        payload->status = BRCMSTB_MSG_NOT_SUPPORTED;
        break;
    }

 REPLY:
    ARCH_SPECIFIC_MBOX_REPLY;
}
