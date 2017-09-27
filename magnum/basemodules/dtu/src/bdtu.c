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
#define BDBG_MSG_TRACE(X) /* BDBG_WRN(X) */

BDBG_OBJECT_ID(BDTU);

struct BDTU
{
    BDBG_OBJECT(BDTU)
    BDTU_CreateSettings createSettings;
};

void BDTU_GetDefaultCreateSettings( BDTU_CreateSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BDTU_Create( BDTU_Handle *pHandle, const BDTU_CreateSettings *pSettings )
{
    BDTU_Handle handle;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    BKNI_Memset(handle, 0, sizeof(*handle));
    BDBG_OBJECT_SET(handle, BDTU);
    handle->createSettings = *pSettings;
    *pHandle = handle;

    return BERR_SUCCESS;
}

void BDTU_Destroy( BDTU_Handle handle )
{
    BDBG_OBJECT_ASSERT(handle, BDTU);
    BDBG_OBJECT_DESTROY(handle, BDTU);
    BKNI_Free(handle);
}

#define _2MB 0x200000

static BERR_Code bdtu_p_convert_physaddr_to_bp(BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned *bp)
{
    if (addr < handle->createSettings.physAddrBase || (addr & (_2MB-1))) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    addr -= handle->createSettings.physAddrBase;
    *bp = (addr - handle->createSettings.physAddrBase)/_2MB;
    if (*bp >= BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_END+1) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    return BERR_SUCCESS;
}

static bool bdtu_p_convert_devaddr_to_dp(BSTD_DeviceOffset addr, unsigned *dp)
{
    if (addr & (_2MB-1)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    *dp = addr/_2MB;
    if (*dp > BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    return BERR_SUCCESS;
}

static void bdtu_p_print_page(unsigned bp, uint32_t val)
{
    BDBG_MSG(("BP %u: VALID=%u, DEVICE_PAGE=%u, OWNER_ID=%u, OWNED=%u, SCRUBBING=%u",
        bp,
        (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_SHIFT,
        (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT,
        (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNER_ID_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNER_ID_SHIFT,
        (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNED_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNED_SHIFT,
        (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_SCRUBBING_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_SCRUBBING_SHIFT
        ));
}

/* Extend this macro to support multiple MEMC's */
#define BDTU_REG(BP) (BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_BASE + ((BP) * BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_ARRAY_ELEMENT_SIZE/8))

void BDTU_GetDefaultRemapSettings( BDTU_RemapSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

BERR_Code BDTU_P_Remap( BDTU_Handle handle, BSTD_DeviceOffset devAddr, BSTD_DeviceOffset fromPhysAddr, BSTD_DeviceOffset toPhysAddr )
{
    unsigned from_bp, to_bp, dp = 0;
    uint32_t val;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    BDBG_MSG_TRACE(("remap DA " BDBG_UINT64_FMT ", from BA " BDBG_UINT64_FMT " to BA " BDBG_UINT64_FMT, BDBG_UINT64_ARG(devAddr), BDBG_UINT64_ARG(fromPhysAddr), BDBG_UINT64_ARG(toPhysAddr)));
    rc = bdtu_p_convert_physaddr_to_bp(handle, fromPhysAddr, &from_bp);
    if (rc) return BERR_TRACE(rc);
    rc = bdtu_p_convert_physaddr_to_bp(handle, toPhysAddr, &to_bp);
    if (rc) return BERR_TRACE(rc);
    rc = bdtu_p_convert_devaddr_to_dp(devAddr, &dp);
    if (rc) return BERR_TRACE(rc);

    /* validate existing mapping */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(from_bp));
    /* bdtu_p_print_page(from_bp, val); */
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if ((val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT != dp) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNED_MASK) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(to_bp));
    /* bdtu_p_print_page(to_bp, val); */
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_OWNED_MASK) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* unmap and check */
    val = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_UNMAP;
    BREG_Write32(handle->createSettings.reg, BDTU_REG(from_bp), val);
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(from_bp));
    /* bdtu_p_print_page(from_bp, BREG_Read32(handle->createSettings.reg, BDTU_REG(from_bp))); */
    if (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK) {
        BDBG_ERR(("old bp %u not unmapped", from_bp));
        bdtu_p_print_page(from_bp, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }

    /* map and check */
    val = BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_MAP |
        (dp << BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT);
    BREG_Write32(handle->createSettings.reg, BDTU_REG(to_bp), val);
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(to_bp));
    /* bdtu_p_print_page(to_bp, BREG_Read32(handle->createSettings.reg, BDTU_REG(to_bp))); */
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        BDBG_ERR(("new bp %u not mapped", to_bp));
        bdtu_p_print_page(to_bp, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }
    if ((val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT != dp) {
        BDBG_ERR(("new bp %u not mapped to dp %u", to_bp, dp));
        bdtu_p_print_page(to_bp, val);
        return BERR_TRACE(BERR_UNKNOWN);
    }

    return BERR_SUCCESS;
}

BERR_Code BDTU_Remap( BDTU_Handle handle, const BDTU_RemapSettings *pSettings )
{
    unsigned i;
    BERR_Code rc;
    for (i=0;i<BDTU_REMAP_LIST_TOTAL && pSettings->list[i].devAddr;i++) {
        rc = BDTU_P_Remap(handle, pSettings->list[i].devAddr, pSettings->list[i].fromPhysAddr, pSettings->list[i].toPhysAddr);
        if (rc) { BERR_TRACE(rc); goto error; }
    }
    return BERR_SUCCESS;
error:
    /* try to remap back to what we had */
    while (i--) {
        /* declare a local rc so the original failure is preserved and returned */
        BERR_Code rc = BDTU_P_Remap(handle, pSettings->list[i].devAddr, pSettings->list[i].toPhysAddr, pSettings->list[i].fromPhysAddr);
        if (rc) { BERR_TRACE(rc); } /* keep going */
    }
    return rc;
}

BERR_Code BDTU_ReadDeviceAddress( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BSTD_DeviceOffset *devAddr )
{
    unsigned bp, dp;
    uint32_t val;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(handle, BDTU);
    rc = bdtu_p_convert_physaddr_to_bp(handle, physAddr, &bp);
    if (rc) return BERR_TRACE(rc);

    /* validate existing mapping */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp));
    if (!(val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_VALID_MASK)) {
        return BERR_INVALID_PARAMETER;
    }
    dp = (val & BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_MASK) >> BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_DEVICE_PAGE_SHIFT;
    *devAddr = dp * _2MB;

    return BERR_SUCCESS;
}

void BDTU_PrintMap( BDTU_Handle handle, BSTD_DeviceOffset addr, unsigned size )
{
    unsigned i;
    for (i=0;i<size;i+=_2MB) {
        unsigned bp = (addr+i)/_2MB;
        bdtu_p_print_page(bp, BREG_Read32(handle->createSettings.reg, BDTU_REG(bp)));
    }
}

BERR_Code BDTU_Own( BDTU_Handle handle, BSTD_DeviceOffset addr, bool own)
{
    BERR_Code rc;
    unsigned bp;
    uint32_t val;

    rc = bdtu_p_convert_physaddr_to_bp(handle, addr, &bp);
    if (rc) return BERR_TRACE(rc);

    /* Can only own if the mapping is valid */
    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp));
    if (!BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID)) {
        return BERR_NOT_INITIALIZED;
    }

    val = own ? BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_SETOWN : BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_UNSETOWN;
    BREG_Write32(handle->createSettings.reg, BDTU_REG(bp), val);

    if (own) {
        /* validate ownership */
        val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp));
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
    unsigned bp, i;
    uint32_t val;
    BDTU_PageInfo info;

    rc = bdtu_p_convert_physaddr_to_bp(handle, addr, &bp);
    if (rc) return BERR_TRACE(rc);

    for (i=0;i<size/_2MB;i++) {
        val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp+i));

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

        BREG_Write32(handle->createSettings.reg, BDTU_REG(bp+i), BCHP_MEMC_DTU_MAP_STATE_0_MAP_STATEi_COMMAND_CMD_SCRUB);
    }

    for (i=0;i<size/_2MB;i++) {
        /* because we scrub all then check, we can afford a busy wait. it should be very fast. */
        /* If a page was skipped due to not being valid, the below check is still ok */
        uint32_t val;
        do {
            val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp+i));
        } while (BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, SCRUBBING));
    }

error:
    return rc;
}

BERR_Code BDTU_ReadInfo( BDTU_Handle handle, BSTD_DeviceOffset physAddr, BDTU_PageInfo *info)
{
    unsigned bp;
    BERR_Code rc;
    uint32_t val;

    BDBG_OBJECT_ASSERT(handle, BDTU);

    if(!info) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    rc = bdtu_p_convert_physaddr_to_bp(handle, physAddr, &bp);
    if (rc) return BERR_TRACE(rc);

    BKNI_Memset(info, 0, sizeof(*info));

    val = BREG_Read32(handle->createSettings.reg, BDTU_REG(bp));

    info->valid = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, VALID);

    if(info->valid) {
        info->owned = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNED);
        info->scrubbing = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, SCRUBBING);
        info->ownerID = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, OWNER_ID);
        info->deviceOffset = BCHP_GET_FIELD_DATA(val, MEMC_DTU_MAP_STATE_0_MAP_STATEi, DEVICE_PAGE);
        info->deviceOffset *= _2MB;
    }

    return BERR_SUCCESS;
}

BERR_Code BDTU_GetStatus( BDTU_Handle handle, BDTU_Status *pStatus )
{
    uint32_t val;

    BDBG_OBJECT_ASSERT(handle, BDTU);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    val = BREG_Read32(handle->createSettings.reg, BCHP_MEMC_DTU_CONFIG_0_TRANSLATE);
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
