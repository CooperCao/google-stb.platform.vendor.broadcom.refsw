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

#include "lib_printf.h"
#include "tracelog.h"
#include "tzioc_client.h"
#include "tzioc_mem.h"
#include "tzioc_msg.h"
#include "tzioc_sys_msg.h"


/* MEMC_TRACELOG :: VERSION :: MAJOR_REVISION [15:08] */
#define STB_MEMC_TRACELOG_VERSION_MAJOR_REVISION_MASK         0x0000ff00
#define STB_MEMC_TRACELOG_VERSION_MAJOR_REVISION_SHIFT        8

/* MEMC_TRACELOG :: FILTER_MODEi :: ADDRESS_ENABLE [05:05] */
#define STB_MEMC_TRACELOG_FILTER_MODEi_ADDRESS_ENABLE_MASK    0x00000020
#define STB_MEMC_TRACELOG_FILTER_MODEi_ADDRESS_ENABLE_SHIFT   5

/* MEMC_TRACELOG :: FILTER_MODEi :: SNOOP_IO_BUS [00:00] */
#define STB_MEMC_TRACELOG_FILTER_MODEi_SNOOP_IO_BUS_MASK      0x00000001
#define STB_MEMC_TRACELOG_FILTER_MODEi_SNOOP_IO_BUS_SHIFT     0

/* MEMC_TRACELOG :: CONTROL :: ENABLE_FILTERS_FOR_CAPTURE [27:24] */
#define STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_CAPTURE_MASK 0x0f000000
#define STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_CAPTURE_SHIFT 24

/* MEMC_TRACELOG :: CONTROL :: ENABLE_FILTERS_FOR_TRIGGER [19:16] */
#define STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_TRIGGER_MASK 0x000f0000
#define STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_TRIGGER_SHIFT 16

/* MEMC_TRACELOG :: CONTROL :: GISB_FULL_SNOOP [02:02] */
#define STB_MEMC_TRACELOG_CONTROL_GISB_FULL_SNOOP_MASK        0x00000004
#define STB_MEMC_TRACELOG_CONTROL_GISB_FULL_SNOOP_SHIFT       2

/* MEMC_TRACELOG :: CONTROL :: BUFFER_DRAM [01:01] */
#define STB_MEMC_TRACELOG_CONTROL_BUFFER_DRAM_MASK            0x00000002
#define STB_MEMC_TRACELOG_CONTROL_BUFFER_DRAM_SHIFT           1

/* MEMC_TRACELOG :: CONTROL :: FORMAT16 [00:00] */
#define STB_MEMC_TRACELOG_CONTROL_FORMAT16_MASK               0x00000001
#define STB_MEMC_TRACELOG_CONTROL_FORMAT16_SHIFT              0

/* MEMC_TRACELOG :: TRIGGER_MODE :: EVENT_MODE [10:08] */
#define STB_MEMC_TRACELOG_TRIGGER_MODE_EVENT_MODE_MASK        0x00000700
#define STB_MEMC_TRACELOG_TRIGGER_MODE_EVENT_MODE_SHIFT       8

/* MEMC_TRACELOG :: TRIGGER_MODE :: CAPTURE_MODE [02:00] */
#define STB_MEMC_TRACELOG_TRIGGER_MODE_CAPTURE_MODE_MASK      0x00000007
#define STB_MEMC_TRACELOG_TRIGGER_MODE_CAPTURE_MODE_SHIFT     0


bool TraceLog::enabled = false;
uint32_t TraceLog::tracelogBase = 0;
uint32_t TraceLog::commandReg = 0;
uint32_t TraceLog::sentinelBase = 0;
uint32_t TraceLog::sentinelSize = 0;
TzMem::PhysAddr TraceLog::traceBuffPaddr = 0;
TzMem::VirtAddr TraceLog::traceBuffVaddr = 0;


void TraceLog::init(void) {

    uint32_t versionReg = STB_REG_ADDR(STB_MEMC_TRACELOG_VERSION);

    if (!versionReg) {
        warn_msg("TraceLog is not supported");
        return;
    }

    if (REG_RD(versionReg) < 0x0200) {
        warn_msg("TraceLog with imcomptible HW");
        return;
    }

    tracelogBase = versionReg;
    commandReg = STB_REG_ADDR(STB_MEMC_TRACELOG_COMMAND);

    uint32_t sentinelStart = STB_REG_ADDR(STB_MEMC_SENTINEL_RANGE_START);
    uint32_t sentinelEnd = STB_REG_ADDR(STB_MEMC_SENTINEL_RANGE_END);

    sentinelBase = sentinelStart;
    sentinelSize = (sentinelEnd - sentinelStart) / 4 + 1;

    // Alloc trace buffer
    struct tzioc_client *pClient =
        TzIoc::TzIocClient::clientFindById(TZIOC_CLIENT_ID_SYS);

    if (!pClient) {
        err_msg("Failed to get TZIOC system client");
        return;
    }

    traceBuffVaddr = TzIoc::TzIocMem::alloc(pClient, PAGE_SIZE_4K_BYTES);

    if (!traceBuffVaddr) {
        err_msg("Failed to alloc TZIOC memory");
        return;
    }

    // TZIOC shared mem could be mapped to a different address
    traceBuffPaddr = (TzMem::PhysAddr)
        TzIoc::vaddr2paddr((uint32_t)traceBuffVaddr);

    // Setup trace buffer
    STB_REG_WR(STB_MEMC_TRACELOG_BUFFER_PTR, (uint32_t)traceBuffPaddr);
    STB_REG_WR(STB_MEMC_TRACELOG_BUFFER_PTR_EXT, 0);
    STB_REG_WR(STB_MEMC_TRACELOG_BUFFER_SIZE, PAGE_SIZE_4K_BYTES);

    // Setup filter 0 for Sentinel address range
    STB_REG_WR(STB_MEMC_TRACELOG_FILTER_ADDR_LOWERi_ARRAY_BASE,
        sentinelBase);

    STB_REG_WR(STB_MEMC_TRACELOG_FILTER_ADDR_UPPERi_ARRAY_BASE,
        sentinelEnd);

    STB_REG_WR(STB_MEMC_TRACELOG_FILTER_MODEi_ARRAY_BASE,
        ((1 << STB_MEMC_TRACELOG_FILTER_MODEi_SNOOP_IO_BUS_SHIFT) |
         (1 << STB_MEMC_TRACELOG_FILTER_MODEi_ADDRESS_ENABLE_SHIFT)));

    // Enable the filter 0
    STB_REG_WR(STB_MEMC_TRACELOG_CONTROL,
        ((1 << STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_CAPTURE_SHIFT) |
         (1 << STB_MEMC_TRACELOG_CONTROL_ENABLE_FILTERS_FOR_TRIGGER_SHIFT) |
         (0 << STB_MEMC_TRACELOG_CONTROL_GISB_FULL_SNOOP_SHIFT) |
         (1 << STB_MEMC_TRACELOG_CONTROL_BUFFER_DRAM_SHIFT) |
         (0 << STB_MEMC_TRACELOG_CONTROL_FORMAT16_SHIFT)));

    // Setup trigger
    STB_REG_WR(STB_MEMC_TRACELOG_TRIGGER_MODE,
        ((0 << STB_MEMC_TRACELOG_TRIGGER_MODE_EVENT_MODE_SHIFT) |
         (1 << STB_MEMC_TRACELOG_TRIGGER_MODE_CAPTURE_MODE_SHIFT)));

    enabled = true;
    printf("TraceLog initialized\n");
}

int TraceLog::peerUp(void) {
    int err = 0;

    if (!enabled) {
        printf("Tracelog is not enabled\n");
        return 0;
    }

    struct tzioc_client *pClient =
        TzIoc::TzIocClient::clientFindById(TZIOC_CLIENT_ID_SYS);

    if (!pClient) {
        err_msg("Failed to get TZIOC system client");
        return -EIO;
    }

    struct tzioc_msg_hdr hdr;
    struct sys_msg_tracelog_on_cmd cmd;

    hdr.ucOrig = TZIOC_CLIENT_ID_SYS;
    hdr.ucDest = TZIOC_CLIENT_ID_SYS;
    hdr.ucType = SYS_MSG_TRACELOG_ON;
    hdr.ulLen = sizeof(cmd);

    cmd.tracelogBase = tracelogBase;
    cmd.sentinelBase = sentinelBase;
    cmd.sentinelSize = sentinelSize;

    cmd.traceBuffPaddr = (uint32_t)traceBuffPaddr;
    cmd.traceBuffSize  = PAGE_SIZE_4K_BYTES;

    err = TzIoc::TzIocMsg::send(pClient, &hdr, (uint8_t *)&cmd);

    if (err) {
        printf("Failed to send tracelog cmd\n");
        return err;
    }
    return 0;
}

int TraceLog::peerDown(void) {
    /* Clean up after peer down */
    return 0;
}

void TraceLog::inval(void) {

    if (!enabled)
        return;

    uint32_t kvaStart = (uint32_t)traceBuffVaddr;
    uint32_t kvaLast  = (uint32_t)traceBuffVaddr + PAGE_SIZE_4K_BYTES - 1;

    register uint32_t mva;
    for (mva = kvaStart; mva < kvaLast; mva += CORTEX_A15_CACHE_LINE_SIZE) {
        // DCIMVAC - Invalidate data cache by MVA to PoC
        asm volatile ("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
    }
}

void TraceLog::dump(void) {

    if (!enabled)
        return;

    int entries = (STB_REG_RD(STB_MEMC_TRACELOG_BUFFER_WR_PTR) -
        (uint32_t)traceBuffPaddr) / 16;

    for (int i = 0; i < entries; i++) {
        uint32_t *data = (uint32_t *)((uint32_t)traceBuffVaddr + 16 * i);

        // Tracelog always write in blocks,
        // there may be fewer data.
        uint32_t valid = (data[3] >> 15) & 0x1;
        if (!valid) break;

        uint32_t event       = (data[0]);
        uint32_t index       = (data[1] - sentinelBase) / 4;
        uint32_t timestampLo = (data[2]);
        uint32_t timestampHi = (data[3] >> 16);

        printf("0x%04x%08x: event=0x%08x index=0x%08x\n",
               (unsigned int)(timestampHi & 0xFFFF),
               (unsigned int)timestampLo,
               (unsigned int)event,
               (unsigned int)index);
    }
}
