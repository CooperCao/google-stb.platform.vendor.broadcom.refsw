/****************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 ****************************************************************************/

#include "libdspcontrol/CHIP.h"
#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
#  include "bstd_defs.h"
#  include "DSP_raaga_inttypes.h"
#else
#  include <inttypes.h>
#  include <stdint.h>
#endif

#include "libdspcontrol/DSP.h"
#include "libdspcontrol/DSPLOG.h"

#include "libfp/c_utils.h"

#include "libfp/src/c_utils_internal.h"

#include "DSP_raaga_octave.h"
#include "DSP_raaga_octave_atu.h"

#include "../UTIL_assert.h"
#include "../UTIL_rdb.h"



#define ATU_VIRTUAL_ADDR_GRANULARITY_LOG2   12
#define ATU_VIRTUAL_ADDR_GRANULARITY        (1 << ATU_VIRTUAL_ADDR_GRANULARITY_LOG2)
#define ATU_PHYSICAL_ADDR_GRANULARITY_LOG2  9
#define ATU_PHYSICAL_ADDR_GRANULARITY       (1 << ATU_PHYSICAL_ADDR_GRANULARITY_LOG2)


#define DSP_RAAGA_OCTAVE_ATU_DEBUG      0
#define BOOL2STR(b)                     ((b) ? "true" : "false")
#if FEATURE_IS(SW_HOST, RAAGA_MAGNUM)
static __attribute__((unused)) void DEBUG_PRINT(char *msg __attribute__((unused)), ...) { }
#else
#  define DEBUG_PRINT(msg, ...)         do { if(DSP_RAAGA_OCTAVE_ATU_DEBUG) DSPLOG_DEBUG("DSP_RAAGA_OCTAVE_ATU_DEBUG: " msg, ##__VA_ARGS__); } while(0)
#endif


static __unused
void compile_time_assertions(void)
{
    /* Ensure there is enough space in our struct for either revision of the
     * L2C's tables. */
    COMPILE_TIME_ASSERT(BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE_COUNT <= DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES);
    COMPILE_TIME_ASSERT(BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR_COUNT <= DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES);
    COMPILE_TIME_ASSERT(BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE_COUNT <= DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES);
    COMPILE_TIME_ASSERT(BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR_COUNT <= DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES);
}


static
unsigned get_num_atu_entries(DSP *dsp)
{
    return (dsp->atu_hw_revision == 0x0100) ?
                        BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE_COUNT :
                        BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE_COUNT;
}


void DSP_fetchAtuRevision(DSP *dsp)
{
    uint32_t revision = DSP_readSharedRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R0_REVISION);
    revision &= 0xFFFF;
    dsp->atu_hw_revision = revision;
}


void DSP_readAtuEntry(DSP *dsp, unsigned num, DSP_RAAGA_ATU_ENTRY *entry)
{
    uint64_t raw_virtual;
    uint32_t raw_physical;

    DEBUG_PRINT("DSP_readAtuEntry %u", num);

    if (dsp->atu_hw_revision == 0x0100)
    {
        raw_virtual = DSP_readShared64BitRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE(num));
        raw_physical = DSP_readSharedRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR(num));
    }
    else
    {
        raw_virtual = DSP_readShared64BitRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE(num));
        raw_physical = DSP_readSharedRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR(num));
    }
    DEBUG_PRINT("read ATU entry %u: raw_virtual=0x%016" PRIx64, num, raw_virtual);
    DEBUG_PRINT("read ATU entry %u: raw_physical=0x%08" PRIx32, num, raw_physical);

    {
        bool enabled           = RDB_64_GET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, TRANSLATION_VALID);
        bool pageing_en        = RDB_64_GET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, PAGEING_EN);
        uint32_t virtual_start = RDB_64_GET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, VIRTUAL_START);
        uint32_t virtual_end   = RDB_64_GET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, VIRTUAL_END);

        virtual_start = virtual_start << ATU_VIRTUAL_ADDR_GRANULARITY_LOG2;
        virtual_end = ((virtual_end + 1) << ATU_VIRTUAL_ADDR_GRANULARITY_LOG2) - 1;
        DEBUG_PRINT("decoded read ATU entry %u: "
                    "enabled=%s, pageing_en=%s, virtual_start=0x%08" PRIx32 ", virtual_end=0x%08" PRIx32,
                    num, BOOL2STR(enabled), BOOL2STR(pageing_en), virtual_start, virtual_end);
        ASSERT_MSG(pageing_en == false, "ATU pageing is not supported");

        entry->enabled = enabled;
        entry->virtual_start = virtual_start;
        entry->length = virtual_end - virtual_start + 1;
    }

    {
        uint64_t physical_start = RDB_GET_FIELD(raw_physical, BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR, BASE_ADDR);
        physical_start <<= ATU_PHYSICAL_ADDR_GRANULARITY_LOG2;
        DEBUG_PRINT("decoded read ATU entry %u: physical_start=0x%016" PRIx64, num, physical_start);

        entry->physical_start = physical_start;
    }
}


void DSP_writeAtuEntry(DSP *dsp, unsigned num, const DSP_RAAGA_ATU_ENTRY *entry)
{
    uint64_t raw_virtual = 0;
    uint32_t raw_physical = 0;

    DEBUG_PRINT("DSP_writeAtuEntry %u", num);
    DEBUG_PRINT("to be written decoded ATU entry %u: enabled=%s, "
                "virtual_start=0x%08" PRIx32 ", length=%" PRIu32 " (%" PRIu32 " KiB)",
                num, BOOL2STR(entry->enabled), entry->virtual_start,
                entry->length, entry->length / 1024);

    ASSERT(IS_MULTIPLE_OF_POW2(entry->virtual_start, ATU_VIRTUAL_ADDR_GRANULARITY));
    ASSERT(IS_MULTIPLE_OF_POW2(entry->physical_start, ATU_PHYSICAL_ADDR_GRANULARITY));
    ASSERT(IS_MULTIPLE_OF_POW2(entry->length, ATU_VIRTUAL_ADDR_GRANULARITY));

    {
        uint32_t enabled = entry->enabled ? 1 : 0;
        uint32_t pageing_en = 0;
        uint32_t virtual_start = entry->virtual_start;
        uint32_t virtual_end = entry->virtual_start + entry->length - 1;
        virtual_start = virtual_start >> ATU_VIRTUAL_ADDR_GRANULARITY_LOG2;
        virtual_end = virtual_end >>  ATU_VIRTUAL_ADDR_GRANULARITY_LOG2;

        RDB_64_SET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, TRANSLATION_VALID, (uint64_t) enabled);
        RDB_64_SET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, PAGEING_EN,        (uint64_t) pageing_en);
        RDB_64_SET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, VIRTUAL_START,     (uint64_t) virtual_start);
        RDB_64_SET_FIELD(raw_virtual, BFPSDK_RAAGA_DSP_L2C_ADDR_TRANSLATION_TABLE, VIRTUAL_END,       (uint64_t) virtual_end);
    }

    DEBUG_PRINT("to be written decoded ATU entry %u: physical_start=0x%016" PRIx64,
                num, entry->physical_start);

    {
        uint64_t physical_start = entry->physical_start;
        physical_start >>= ATU_PHYSICAL_ADDR_GRANULARITY_LOG2;

        RDB_SET_FIELD(raw_physical, BFPSDK_RAAGA_DSP_L2C_PHYSICAL_40B_BASE_ADDR, BASE_ADDR, physical_start);
    }

    DEBUG_PRINT("to be written ATU entry %u: raw_virtual=0x%016" PRIx64, num, raw_virtual);
    DEBUG_PRINT("to be written ATU entry %u: raw_physical=0x%08" PRIx32, num, raw_physical);
    if (dsp->atu_hw_revision == 0x0100)
    {
        DSP_writeSharedRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R0_PHYSICAL_40B_BASE_ADDR(num), raw_physical);
        DSP_writeShared64BitRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R0_ADDR_TRANSLATION_TABLE(num), raw_virtual);
    }
    else
    {
        DSP_writeSharedRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R1_PHYSICAL_40B_BASE_ADDR(num), raw_physical);
        DSP_writeShared64BitRegister(dsp, BFPSDK_RAAGA_DSP_L2C_R1_ADDR_TRANSLATION_TABLE(num), raw_virtual);
    }
}


void DSP_readAtu(DSP *dsp, DSP_RAAGA_ATU *atu)
{
    unsigned num_entries = get_num_atu_entries(dsp);
    DEBUG_PRINT("DSP_readAtu: reading ATU %u entries", num_entries);
    {
        unsigned i;
        for(i = 0; i < num_entries; i++)
            DSP_readAtuEntry(dsp, i, &atu->entries[i]);
        for( ; i < DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES; i++)
            atu->entries[i].enabled = false;
    }
}


void DSP_writeAtu(DSP *dsp, const DSP_RAAGA_ATU *atu)
{
    unsigned num_entries = get_num_atu_entries(dsp);

    DEBUG_PRINT("DSP_writeAtu: writing ATU %u entries", num_entries);
    {
        unsigned i;
        for(i = 0; i < num_entries; i++)
            DSP_writeAtuEntry(dsp, i, &atu->entries[i]);
    }
}


void DSP_createAtuIndex(const DSP_RAAGA_ATU *atu, DSP_RAAGA_ATU_INDEX *index)
{
    DEBUG_PRINT("DSP_createAtuIndex");

    {
        unsigned i;
        index->entries_count = 0;
        for(i = 0; i < DSP_RAAGA_ATU_CONFIG_MAX_ENTRIES; i++)
            if(atu->entries[i].enabled)
            {
                const DSP_RAAGA_ATU_ENTRY *entry = &(atu->entries[i]);
                index->entries[index->entries_count++] = entry;
                DEBUG_PRINT("index entry %u: virtual_start=0x%08" PRIx32 ", physical_start=0x%016"
                            PRIx64 ", length=%" PRIu32 " (%" PRIu32 " KiB), "
                            "(virtual_end=0x%08" PRIx32 ")",
                            i, entry->virtual_start, entry->physical_start, entry->length,
                            entry->length / 1024, entry->virtual_start + entry->length - 1);
            }
    }

    DEBUG_PRINT("created ATU cache with %u entries", index->entries_count);
}


DSP_RET DSP_atuVirtualToPhysical(const DSP_RAAGA_ATU_INDEX *index,
                                 DSP_ADDR virtual_addr, uint64_t *physical_addr,
                                 uint32_t *length)
{
    DEBUG_PRINT("DSP_atuVirtualToPhysical virtual_addr=0x%08" PRIx32 ", length=%" PRIu32,
                virtual_addr, length ? *length : 0);

    {
        unsigned i;
        for(i = 0; i < index->entries_count; i++)
        {
            const DSP_RAAGA_ATU_ENTRY *entry = index->entries[i];
            if(virtual_addr >= entry->virtual_start &&
               virtual_addr < entry->virtual_start + entry->length)
            {
                uint32_t virtual_offset = virtual_addr - entry->virtual_start;
                *physical_addr = entry->physical_start + virtual_offset;

                if(length)
                {
                    uint32_t remaining_range = entry->length - virtual_offset;
                    if(remaining_range < *length)
                        *length = remaining_range;
                }

                DEBUG_PRINT("DSP_atuVirtualToPhysical converted physical_addr=0x%016" PRIx64
                            ", length=%" PRIu32, *physical_addr, length ? *length : 0);

                return DSP_SUCCESS;
            }
        }
    }

    DEBUG_PRINT("DSP_atuVirtualToPhysical couldn't find a translation");

    return DSP_FAILURE;
}


DSP_RET DSP_atuPhysicalToVirtual(const DSP_RAAGA_ATU_INDEX *index,
                                 uint64_t physical_addr, DSP_ADDR *virtual_addr,
                                 uint32_t *length)
{
    DEBUG_PRINT("DSP_atuPhysicalToVirtual physical_addr=0x%016" PRIx64 ", length=%" PRIu32,
                physical_addr, length ? *length : 0);

    {
        unsigned i;
        for(i = 0; i < index->entries_count; i++)
        {
            const DSP_RAAGA_ATU_ENTRY *entry = index->entries[i];
            if(physical_addr >= entry->physical_start &&
               physical_addr < entry->physical_start + entry->length)
            {
                uint64_t physical_offset = physical_addr - entry->physical_start;
                uint64_t virtual = physical_offset + entry->virtual_start;
                ASSERT(virtual <= UINT32_MAX);
                *virtual_addr = (uint32_t) virtual;

                if(length)
                {
                    uint32_t remaining_range = entry->length - (uint32_t) physical_offset;
                    if(remaining_range < *length)
                        *length = remaining_range;
                }

                DEBUG_PRINT("DSP_atuPhysicalToVirtual converted virtual_addr=0x%08" PRIx32
                            ", length=%" PRIu32, *virtual_addr, length ? *length : 0);

                return DSP_SUCCESS;
            }
        }
    }

    DEBUG_PRINT("DSP_atuPhysicalToVirtual couldn't find a translation");

    return DSP_FAILURE;
}
