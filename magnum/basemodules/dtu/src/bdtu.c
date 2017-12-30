/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#include "bstd.h"
#include "bkni.h"
#include "bdtu.h"
#include "bchp_common.h"

BDBG_MODULE(BDTU);

#ifdef BCHP_MEMC_DTU_MAP_STATE_0_REG_START
#include "bchp_memc_dtu_map_state_0.h"
#include "bchp_memc_dtu_config_0.h"
#ifdef BCHP_MEMC_DTU_MAP_STATE_1_REG_START
#include "bchp_memc_dtu_map_state_1.h"
#include "bchp_memc_dtu_config_1.h"
#endif
#ifdef BCHP_MEMC_DTU_MAP_STATE_2_REG_START
#include "bchp_memc_dtu_map_state_2.h"
#include "bchp_memc_dtu_config_2.h"
#endif

#define BDBG_MSG_TRACE(X) /* BDBG_WRN(X) */

BDBG_OBJECT_ID(BDTU);

struct BDTU
{
    BDBG_OBJECT(BDTU)
    BDTU_CreateSettings createSettings;
    uint32_t arrayBase;
    uint32_t arrayEnd;
    uint32_t arrayElementSize;
};

#define BDTU_REG(HANDLE, BP) (HANDLE->arrayBase + ((BP) * HANDLE->arrayElementSize/8))
#define _2MB 0x200000

void BDTU_GetDefaultCreateSettings( BDTU_CreateSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BDTU_Create( BDTU_Handle *pHandle, const BDTU_CreateSettings *pSettings )
{
    BDTU_Handle handle = NULL;
    BERR_Code rc = BERR_SUCCESS;
    unsigned i;

    if (!pSettings) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* Some sanity on the settings.. shouldn't ever fail */
    for (i = 0; i < BCHP_MAX_MEMC_REGIONS; i++) {
        if ((pSettings->memoryLayout.region[i].addr & (_2MB-1)) || (pSettings->memoryLayout.region[i].size & (_2MB-1))) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BKNI_Memset(handle, 0, sizeof(*handle));
    BDBG_OBJECT_SET(handle, BDTU);
    handle->createSettings = *pSettings;
    *pHandle = handle;

    switch (pSettings->memoryLayout.memcIndex) {
        case 0:
            handle->arrayEnd = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_END + 1;
            handle->arrayBase = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_BASE;
            handle->arrayElementSize = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_ELEMENT_SIZE;
            break;
#ifdef BCHP_MEMC_DTU_MAP_STATE_1_REG_START
        case 1:
            handle->arrayEnd = BCHP_MEMC_DTU_MAP_STATE_1_MAP_STATEi_ARRAY_END + 1;
            handle->arrayBase = BCHP_MEMC_DTU_MAP_STATE_1_MAP_STATEi_ARRAY_BASE;
            handle->arrayElementSize = BCHP_MEMC_DTU_MAP_STATE_1_MAP_STATEi_ARRAY_ELEMENT_SIZE;
            break;
#endif
#ifdef BCHP_MEMC_DTU_MAP_STATE_2_REG_START
        case 2:
            handle->arrayEnd = BCHP_MEMC_DTU_MAP_STATE_2_MAP_STATEi_ARRAY_END + 1;
            handle->arrayBase = BCHP_MEMC_DTU_MAP_STATE_2_MAP_STATEi_ARRAY_BASE;
            handle->arrayElementSize = BCHP_MEMC_DTU_MAP_STATE_2_MAP_STATEi_ARRAY_ELEMENT_SIZE;
            break;
#endif
        default:
            BDBG_ERR(("Invalid memcIndex provided"));
            rc = BERR_INVALID_PARAMETER;
            break;
    }

    if ((rc != BERR_SUCCESS) && handle)
    {
        BDTU_Destroy(handle);
    }

    return rc;
}

void BDTU_Destroy( BDTU_Handle handle )
{
    BDBG_OBJECT_ASSERT(handle, BDTU);
    BDBG_OBJECT_DESTROY(handle, BDTU);
    BKNI_Free(handle);
}

static BERR_Code bdtu_p_convert_addr_to_region(BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned *region)
{
    unsigned i;
    BSTD_DeviceOffset end;

    if (!region) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *region = BCHP_MAX_MEMC_REGIONS;

    if (addr & (_2MB-1)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i = 0;i < BCHP_MAX_MEMC_REGIONS;i++) {
        if (!handle->createSettings.memoryLayout.region[i].size) {
            continue;
        }

        end = handle->createSettings.memoryLayout.region[i].addr + handle->createSettings.memoryLayout.region[i].size;
        end -= _2MB;

        if((addr >= handle->createSettings.memoryLayout.region[i].addr) && (addr <= end)) {
            *region = i;
            break;
        }
    }

    if (*region == BCHP_MAX_MEMC_REGIONS)
    {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

static BERR_Code bdtu_p_convert_addr_to_bp(BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned *bp, unsigned *region)
{
    unsigned i;
    BERR_Code rc;
    unsigned rgn;

    if (!bp) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    *bp = 0;

    rc = bdtu_p_convert_addr_to_region(handle, addr, &rgn);
    if (rc != BERR_SUCCESS) {
        return BERR_TRACE(rc);
    }

    for (i = 0; i < rgn; i++) {
        *bp += (handle->createSettings.memoryLayout.region[i].size/_2MB);
    }

    addr -= handle->createSettings.memoryLayout.region[rgn].addr;
    *bp += (addr/_2MB);

    if (*bp >= handle->arrayEnd) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if(region) {
        *region = rgn;
    }

    return rc;
}

static void bdtu_p_print_page(BDTU_Handle handle, unsigned bp, unsigned region, uint32_t val)
{
    BSTD_DeviceOffset ba = (bp * _2MB) + handle->createSettings.memoryLayout.region[region].addr;
    int i; /* Must be signed */

    for (i=(region-1);i>=0;i--) {
        ba -= handle->createSettings.memoryLayout.region[i].size;
    }

    BDBG_WRN(("MEMC[%d][%d] BP %04u ("BDBG_UINT64_FMT"): VALID=%u, DEVICE_PAGE=%04u, OWNER_ID=%u, OWNED=%u, SCRUBBING=%u",
        handle->createSettings.memoryLayout.memcIndex, region, bp, BDBG_UINT64_ARG(ba),
        BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID),
        BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, DEVICE_PAGE),
        BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNER_ID),
        BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNED),
        BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, SCRUBBING)
        ));
}

void BDTU_GetDefaultRemapSettings( BDTU_RemapSettings *pSettings )
{
    if (!pSettings) {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }

    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static BERR_Code BDTU_P_Remap( BDTU_Handle handle, BSTD_DeviceOffset orgPhysAddr, BSTD_DeviceOffset fromPhysAddr, BSTD_DeviceOffset toPhysAddr )
{
    unsigned from_bp, to_bp, dp;
    unsigned from_region, to_region, dp_region;
    uint32_t val;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    BDBG_MSG_TRACE(("remap DA " BDBG_UINT64_FMT ", from BA " BDBG_UINT64_FMT " to BA " BDBG_UINT64_FMT, BDBG_UINT64_ARG(orgPhysAddr), BDBG_UINT64_ARG(fromPhysAddr), BDBG_UINT64_ARG(toPhysAddr)));
    rc = bdtu_p_convert_addr_to_bp(handle, fromPhysAddr, &from_bp, &from_region);
    if (rc) return BERR_TRACE(rc);
    rc = bdtu_p_convert_addr_to_bp(handle, toPhysAddr, &to_bp, &to_region);
    if (rc) return BERR_TRACE(rc);
    rc = bdtu_p_convert_addr_to_bp(handle, orgPhysAddr, &dp, &dp_region);
    if (rc) return BERR_TRACE(rc);

    /* validate existing/from mapping. Must be Valid, DP must match, also unmap will NOT work if owned. */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, from_bp));
    /* bdtu_p_print_page(handle, from_bp, from_region, val); */
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ((val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT != dp) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNED_MASK) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* validate to mapping. Must NOT be valid (ATM non-valid cannot be owned) */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, to_bp));
    /* bdtu_p_print_page(handle, to_bp, to_region, val); */
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* unmap and check */
    val = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_UNMAP;
    BREG_Write32(handle->createSettings.reg, BDTU_REG(handle, from_bp), val);
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, from_bp));
    /* bdtu_p_print_page(handle, from_bp, from_region, BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, from_bp))); */
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK) {
        BDBG_ERR(("old bp %u not unmapped", from_bp));
        bdtu_p_print_page(handle, from_bp, from_region, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }

    /* map and check */
    val = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_MAP |
        (dp << BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT);
    BREG_Write32(handle->createSettings.reg, BDTU_REG(handle, to_bp), val);
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, to_bp));
    /* bdtu_p_print_page(handle, to_bp, to_region, BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, to_bp))); */
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        BDBG_ERR(("new bp %u not mapped", to_bp));
        bdtu_p_print_page(handle, to_bp, to_region, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }
    if ((val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT != dp) {
        BDBG_ERR(("new bp %u not mapped to dp %u", to_bp, dp));
        bdtu_p_print_page(handle, to_bp, to_region, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }

    return BERR_SUCCESS;
}

BERR_Code BDTU_Remap( BDTU_Handle handle, const BDTU_RemapSettings *pSettings )
{
    unsigned i;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    for (i = 0;i<BDTU_REMAP_LIST_TOTAL && (pSettings->list[i].orgPhysAddr != BDTU_INVALID_ADDR);i++) {
        rc = BDTU_P_Remap(handle, pSettings->list[i].orgPhysAddr, pSettings->list[i].fromPhysAddr, pSettings->list[i].toPhysAddr);
        if (rc) { BERR_TRACE(rc); goto error; }
    }
    return BERR_SUCCESS;
error:
    /* try to remap back to what we had */
    while (i--) {
        /* declare a local rc so the original failure is preserved and returned */
        BERR_Code rc = BDTU_P_Remap(handle, pSettings->list[i].orgPhysAddr, pSettings->list[i].toPhysAddr, pSettings->list[i].fromPhysAddr);
        if (rc) { BERR_TRACE(rc); } /* keep going */
    }
    return rc;
}

BERR_Code BDTU_ReadOriginalAddress( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BSTD_DeviceOffset *orgPhysAddr )
{
    unsigned bp;
    unsigned region;
    unsigned dp;
    uint32_t val;
    BERR_Code rc;
    BSTD_DeviceOffset addr;

    BDBG_OBJECT_ASSERT(handle, BDTU);
    rc = bdtu_p_convert_addr_to_bp(handle, physAddr, &bp, &region);
    if (rc) return BERR_TRACE(rc);

    /* validate existing mapping */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bp));
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        return BERR_INVALID_PARAMETER;
    }
    dp = (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT;

    /* convert from DP to BA */
    addr = (BSTD_DeviceOffset)dp * _2MB;
    for (region=0;region<BCHP_MAX_MEMC_REGIONS;region++) {
        if (addr < handle->createSettings.memoryLayout.region[region].size) break;
        addr -= handle->createSettings.memoryLayout.region[region].size;

    }
    if (region == BCHP_MAX_MEMC_REGIONS) return BERR_TRACE(BERR_INVALID_PARAMETER);
    *orgPhysAddr = handle->createSettings.memoryLayout.region[region].addr + addr;

    return BERR_SUCCESS;
}

void BDTU_PrintMap( BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned size )
{
    unsigned i;
    unsigned bp;
    unsigned region;
    BSTD_DeviceOffset end;
    BSTD_DeviceOffset curr = addr;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    if(bdtu_p_convert_addr_to_bp(handle, addr, &bp, &region) != BERR_SUCCESS) {
       BERR_TRACE(BERR_INVALID_PARAMETER);
       return;
    }

    end = handle->createSettings.memoryLayout.region[region].addr + handle->createSettings.memoryLayout.region[region].size - 1;
    for (i = 0; i < size; i += _2MB) {
          if (curr > end) {
            region++;
            if ((region >= BCHP_MAX_MEMC_REGIONS) || (!handle->createSettings.memoryLayout.region[region].size)) {
                BDBG_ERR(("Exceeded MEMC. Stopping at 0x%x", i));
                return;
            }
            curr = handle->createSettings.memoryLayout.region[region].addr;
            end = curr + handle->createSettings.memoryLayout.region[region].size - 1;
        }

        bdtu_p_print_page(handle, bp, region, BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bp)));
        bp++;
        curr += _2MB;
    }
}

BERR_Code BDTU_Own( BDTU_Handle handle, BSTD_DeviceOffset addr, bool own)
{
    BERR_Code rc;
    unsigned bp;
    uint32_t val;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    rc = bdtu_p_convert_addr_to_bp(handle, addr, &bp, NULL);
    if (rc) return BERR_TRACE(rc);

    /* Can only own if the mapping is valid */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bp));
    if (!BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID)) {
        return BERR_NOT_INITIALIZED;
    }

    val = own ? BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_SETOWN : BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_UNSETOWN;
    BREG_Write32(handle->createSettings.reg, BDTU_REG(handle, bp), val);

    if (own) {
        /* validate ownership */
        val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bp));
        if (own != BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNED)) {
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }

        if (BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNER_ID) != handle->createSettings.ownerId) {
            /* race condition with another caller setting OWN on the same BP may cause mismatch */
            return BERR_TRACE(BERR_NOT_AVAILABLE);
        }
    }
    /* can't validate unownership. someone else may have grabbed it. */

    return BERR_SUCCESS;
}

BERR_Code BDTU_Scrub( BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned size )
{
    BERR_Code rc;
    unsigned bp, bpTmp;
    unsigned region, regionTmp;
    unsigned i;
    uint32_t val;
    BDTU_PageInfo info;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    if(size & (_2MB-1)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    rc = bdtu_p_convert_addr_to_bp(handle, addr, &bp, &region);
    if (rc) return BERR_TRACE(rc);

    rc = bdtu_p_convert_addr_to_bp(handle, addr + size - _2MB, &bpTmp, &regionTmp);
    if (rc) return BERR_TRACE(rc);

    /* For simplicity, make sure no regions are crossed */
    /* Probably wouldn't make much sense from a usage point of view to cross anyway */
    if (region != regionTmp) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i=0;i<size/_2MB;i++) {
        bpTmp = bp + i;

        val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bpTmp));

        info.valid = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID);
        if(!info.valid)
        {
            /* Scrubbing an invalid page is NA */
            continue;
        }

        info.owned = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNED);
        info.ownerID = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNER_ID);
        if(info.owned && (info.ownerID != handle->createSettings.ownerId))
        {
            BDBG_ERR(("Cannot scrub a page owned by someone else"));
            rc = BERR_TRACE(BERR_NOT_AVAILABLE);
            goto error;
        }

        BREG_Write32(handle->createSettings.reg, BDTU_REG(handle, bpTmp), BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_SCRUB);
    }

    for (i=0;i<size/_2MB;i++) {
        /* because we scrub all then check, we can afford a busy wait. it should be very fast. */
        /* If a page was skipped due to not being valid, the below check is still ok */
        uint32_t val;
        bpTmp = bp + i;
        do {
            val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bpTmp));
        } while (BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, SCRUBBING));
    }

error:
    return rc;
}

BERR_Code BDTU_ReadInfo( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BDTU_PageInfo *info)
{
    unsigned bp;
    unsigned region;
    BERR_Code rc;
    uint32_t val;
    int i; /* Must be signed */

    BDBG_OBJECT_ASSERT(handle, BDTU);

    if(!info) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    rc = bdtu_p_convert_addr_to_bp(handle, physAddr, &bp, &region);
    if (rc) return BERR_TRACE(rc);

    BKNI_Memset(info, 0, sizeof(*info));
    info->deviceOffset = BDTU_INVALID_ADDR;

    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(handle, bp));

    info->valid = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID);

    if(info->valid) {
        info->owned = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNED);
        info->scrubbing = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, SCRUBBING);
        info->ownerID = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNER_ID);
        info->deviceOffset = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, DEVICE_PAGE);
        info->deviceOffset *= _2MB;
        info->deviceOffset += handle->createSettings.memoryLayout.region[region].addr;
        for (i=(region-1);i>=0;i--) {
            info->deviceOffset -= handle->createSettings.memoryLayout.region[i].size;
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BDTU_GetStatus( BDTU_Handle handle, BDTU_Status *pStatus )
{
    uint32_t val;

    BDBG_OBJECT_ASSERT(handle, BDTU);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    switch(handle->createSettings.memoryLayout.memcIndex)
    {
        case 0:
            val = BREG_Read32(handle->createSettings.reg, BCHP_MEMC_DTU_CONFIG_0_TRANSLATE);
            break;
#ifdef BCHP_MEMC_DTU_MAP_STATE_1_REG_START
        case 1:
            val = BREG_Read32(handle->createSettings.reg, BCHP_MEMC_DTU_CONFIG_1_TRANSLATE);
            break;
#endif
#ifdef BCHP_MEMC_DTU_MAP_STATE_2_REG_START
        case 2:
            val = BREG_Read32(handle->createSettings.reg, BCHP_MEMC_DTU_CONFIG_2_TRANSLATE);
            break;
#endif
        default:
            return BERR_TRACE(BERR_UNKNOWN);
    }

    val = BCHP_GET_FIELD_DATA(val, MEMC_DTU_CONFIG_0_TRANSLATE, ENABLE);
    switch(val)
    {
        case BCHP_MEMC_DTU_CONFIG_0_TRANSLATE_ENABLE_UNSET:
            pStatus->state = BDTU_State_eUnset;
            break;
        case BCHP_MEMC_DTU_CONFIG_0_TRANSLATE_ENABLE_ENABLE:
            pStatus->state = BDTU_State_eEnabled;
            break;
        case BCHP_MEMC_DTU_CONFIG_0_TRANSLATE_ENABLE_DISABLE:
            pStatus->state = BDTU_State_eDisabled;
            break;
        default:
            return BERR_TRACE(BERR_UNKNOWN);
    }

    return BERR_SUCCESS;
}

#else
#include "bdtu_stub.c"
#endif
