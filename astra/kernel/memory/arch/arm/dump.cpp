/******************************************************************************
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
 *****************************************************************************/
#include "lib_printf.h"
#include "lib_string.h"
#include "pgtable.h"

struct ttblk_t {
    uint32_t start_addr;
    int mem_attr;
    int access_perms;
    int sw_bits;
    bool no_execute;
};

typedef struct coredump_memregion {
    unsigned int startAddr,endAddr, offset,flags;
} coredump_memregion;

static int buf_offset;

static inline void printblk(struct ttblk_t *cb, uint32_t va, void *mem)
{
    if(mem)
    {
        coredump_memregion dump;
        if(cb->sw_bits != 0x3) {
            dump.startAddr = cb->start_addr;
            dump.endAddr = (va - 1);
            dump.offset = 0;
            if(cb->access_perms == MEMORY_ACCESS_RW_USER)
                dump.flags = 0x3 | (!(cb->no_execute)<<3);
            else if (cb->access_perms == MEMORY_ACCESS_RO_USER)
                dump.flags = 0x1 | (!(cb->no_execute)<<3);
            else
                dump.flags = 0x0;

            memcpy((void *)((int)mem+buf_offset),(void *)&dump,sizeof(coredump_memregion));
            buf_offset += sizeof(coredump_memregion);
        }
    }
    else
    {
        printf("   %s block %08lx-%08lx  SW:%d MA:0x%x AP:0x%x %s\n",
           (cb->mem_attr & 0x7) ? "MEM" : "I/O",
           (unsigned long)cb->start_addr, (unsigned long)(va - 1),
           cb->sw_bits,
           cb->mem_attr, cb->access_perms,
           (cb->no_execute) ? "No Execute" : "Execute OK");
    }
}


static void handle_block_valid(struct ttblk_t *cb, uint32_t va, int sw, int attr,
                               int ap, int nx,void *mem)
{
    if ((cb->start_addr == 0xFFFFFFFF) ||
        (cb->mem_attr != attr) || (cb->access_perms != ap) || (cb->sw_bits != sw)) {
        if (cb->start_addr != 0xFFFFFFFF) {
            printblk(cb, va,mem);
        }
        cb->sw_bits = sw;
        cb->start_addr = va;
        cb->mem_attr = attr;
        cb->access_perms = ap;
        cb->no_execute = (nx == 1);
    }
}

static void handle_block_invalid(struct ttblk_t *cb, uint32_t va, void *mem)
{
    if (cb->start_addr != 0xFFFFFFFF) {
        printblk(cb, va,mem);
        cb->start_addr = 0xFFFFFFFF;
    }
}

void PageTable::DumpPageTableInfo(void * mem)
{

    const uint64_t *l1pt = topLevelDir;
    struct ttblk_t curblk;

    buf_offset =0;
    memset(&curblk, 0, sizeof(ttblk_t));
    curblk.start_addr = 0xffffffff;

    printf("Dumping page table @ %p\n", l1pt);
    for (int l1_idx = 0; l1_idx <= L1_PAGE_TABLE_SLOT(0xFFFFFFFF); l1_idx++) {
        uint64_t l1e = l1pt[l1_idx];
        uint32_t l1va = (l1_idx << 30);
        switch (l1e & 0x3) {
            case 0x3:
                {
                    uint64_t l1pa = l1e & L2_BLOCK_ADDR_MASK;
                    uint32_t l1pa_u8 = (uint32_t) (l1pa >> 32);
                    uint32_t l1pa_l32 = (uint32_t) (l1pa & 0xFFFFFFFF);
                    //int l1mattr = (l1e >> 2) & 0xF;
                    //int l1ap = (l1e >> 6) & 0x3;
                    uint64_t *l2_block;
                    /* printf("Found L1 table entry %3d VA: %08x, PA: 0x%02x%08x\n", */
                    /*        l1_idx, l1va, l1pa_u8, l1pa_l32); */
                    if (l1pa_u8) {
                        printf("Can't handle 40-bit addresses yet!\n");
                        break;
                    }
                    l2_block = (uint64_t *) (TzMem::physToVirt((void *) l1pa_l32));
                    for (int l2_idx = 0; l2_idx < L2_PAGE_NUM_ENTRIES; l2_idx++) {
                        uint64_t l2e = l2_block[l2_idx];
                        uint32_t l2va = l1va | (l2_idx << L2_BLOCK_SHIFT);
                        uint64_t l2pa = l2e & L3_BLOCK_ADDR_MASK;
                        uint32_t l2pa_u8 = (uint32_t) (l2pa >> 32);
                        uint32_t l2pa_l32 = (uint32_t) (l2pa & 0xFFFFFFFF);
                        int l2mattr = (l2e >> 2) & 0xF;
                        int l2ap = (l2e >> 6) & 0x3;
                        int l2nx = (l2e >> 54) & 0x1;
                        int l2sw = (l2e >> 55) & 0xF;
                        if (!(l2e & 1)) {
                            handle_block_invalid(&curblk, l2va, mem);

                            continue;
                        }
                        if (l2pa_u8) {
                            printf
                                ("WARNING: Found 40-bit L2 entry %3ld VA: %08lx, PA: 0x%02lx%08lx but ",
                                 (long)l2_idx, (long)l2va, (long)l2pa_u8, (long)l2pa_l32);
                            break;
                        }
                        if (l2e & 0x2) {
                            uint64_t *l3_block =
                                (uint64_t *) (TzMem::physToVirt((void *) l2pa_l32));
                            for (int l3_idx = 0; l3_idx < L3_PAGE_NUM_ENTRIES;
                                 l3_idx++) {
                                uint64_t l3e = l3_block[l3_idx];
                                uint32_t l3va =
                                    l2va | (l3_idx << L3_BLOCK_SHIFT);
                                uint64_t l3pa = l3e & L3_PHYS_ADDR_MASK;
                                uint32_t l3pa_u8 = (uint32_t) (l3pa >> 32);
                                int l3mattr = (l3e >> 2) & 0xF;
                                int l3ap = (l3e >> 6) & 0x3;
                                int l3nx = (l3e >> 54) & 0x1;
                                int sw = (l3e >> 55) & 0xF;
                                /* uint32_t l3pa_l32 = (uint32_t)(l3pa & 0xFFFFFFFF); */
                                if (!(l3e & 1)) {
                                    handle_block_invalid(&curblk, l3va, mem);
                                    continue;
                                }
                                /* printf("Found L3 block entry %3d VA: %08x, PA: 0x%02x%08x\n", */
                                /*        l3_idx, l3va, l3pa_u8, l3pa_l32); */
                                handle_block_valid(&curblk, l3va, sw, l3mattr, l3ap,
                                                   l3nx, mem);
                                if (l3pa_u8) {
                                    printf
                                        ("Warning! Can't handle 40-bit addresses yet!\n");
                                    break;
                                }
                                if (!(l3e & 0x2)) {
                                    printf
                                        ("ERROR: Unexpected L3 entry %3d val=0x%08x%08x!\n",
                                         l1_idx, (uint32_t) (l3e >> 32),
                                         (uint32_t) (l3e & 0xffffffff));
                                }
                            }
                        } else {
                            /* printf("Found L2 block entry %3d VA: %08x, PA: 0x%02x%08x\n", */
                            /*        l2_idx, l2va, l2pa_u8, l2pa_l32); */
                            handle_block_valid(&curblk, l2va, l2sw, l2mattr, l2ap,
                                               l2nx,mem);
                        }
                    }
                }
                break;

            case 0x0:
                handle_block_invalid(&curblk, l1va,mem);
                break;

            default:
                printf("Unexpected L1 entry %3d VA: %08x val=0x%08x%08x\n",
                       l1_idx, l1va, (uint32_t) (l1e >> 32),
                       (uint32_t) (l1e & 0xffffffff));
                break;
        }
    }

}

void PageTable::dump()
{
    DumpPageTableInfo(NULL);
}

void PageTable::dumpInMem(void * mem)
{
    DumpPageTableInfo(mem);
}
