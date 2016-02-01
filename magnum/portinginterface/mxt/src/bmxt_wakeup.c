/***************************************************************************
 *     Copyright (c) 2003-2011, Broadcom Corporation
 *     All Rights Reserved
 *     Confidential Property of Broadcom Corporation
 *
 *  THIS SOFTWARE MAY ONLY BE USED SUBJECT TO AN EXECUTED SOFTWARE LICENSE
 *  AGREEMENT  BETWEEN THE USER AND BROADCOM.  YOU HAVE NO RIGHT TO USE OR
 *  EXPLOIT THIS MATERIAL EXCEPT SUBJECT TO THE TERMS OF SUCH AN AGREEMENT.
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Porting interface code for the data transport core.
 *
 * Revision History:
 *
 * $brcm_Log: $
 *
 ***************************************************************************/

#include "bstd.h"
#include "bchp.h"
#include "bkni.h"
#include "bmxt_priv.h"
#include "bmxt.h"
#include "bmxt_wakeup.h"

#include "bmxt_index_wakeup.h"
#include "bmxt_rdb_mask_shift.h"

#include "bhab.h"

BDBG_MODULE(bmxt_wakeup);

#define BMXT_WAKEUP_MAX_PACKET_SIZE  200
#define BMXT_WAKEUP_MAX_PACKET_TYPES 4 /* up to 4 packet types (0 through 3) */

#define R(reg) (handle->platform.regoffsetsWakeup ? (handle->platform.regoffsetsWakeup[reg] + handle->platform.regbaseWakeup) : 0)

static uint32_t BMXT_RegRead32(BMXT_Handle handle, uint32_t addr)
{
    if (handle->platform.regbaseWakeup==0) {
        BDBG_ERR(("No DEMOD_XPT_WAKEUP support on this frontend platform"));
        return 0;
    }
    if (addr - handle->platform.regbaseWakeup == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=R(0));
    BDBG_ASSERT(addr<=R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE));

    return BMXT_RegRead32_common(handle, addr);
}

static void BMXT_RegWrite32(BMXT_Handle handle, uint32_t addr, uint32_t data)
{
    if (handle->platform.regbaseWakeup==0) {
        BDBG_ERR(("No DEMOD_XPT_WAKEUP support on this frontend platform"));
        return;
    }
    if (addr - handle->platform.regbaseWakeup == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=R(0));
    BDBG_ASSERT(addr<=R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE));

    BMXT_RegWrite32_common(handle, addr, data);
    return;
}

void BMXT_Wakeup_GetSettings(BMXT_Handle handle, BMXT_Wakeup_Settings *pSettings)
{
    uint32_t reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);

    reg = BMXT_RegRead32(handle, R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
    pSettings->InputBand = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL);
    pSettings->InputBand |= BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL_MSB) << 4;
    pSettings->PacketLength = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, PKT_LENGTH);
    pSettings->ErrorInputIgnore = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE);
}

BERR_Code BMXT_Wakeup_SetSettings(BMXT_Handle handle, const BMXT_Wakeup_Settings *pSettings)
{
    uint32_t reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);

    if (pSettings->PacketLength > BMXT_WAKEUP_MAX_PACKET_SIZE) {
        BDBG_ERR(("PacketLength %u is larger than max (%u)", pSettings->PacketLength, BMXT_WAKEUP_MAX_PACKET_SIZE));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    reg = BMXT_RegRead32(handle, R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, PKT_LENGTH, pSettings->PacketLength);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE, pSettings->ErrorInputIgnore);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL_MSB, (pSettings->InputBand >> 4) & 0x1);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL, pSettings->InputBand & 0xf);
    BMXT_RegWrite32(handle, R(BCHP_DEMOD_XPT_WAKEUP_CTRL), reg);

    return 0;
}

void BMXT_Wakeup_ClearInterruptToPMU(BMXT_Handle handle)
{
    BKNI_EnterCriticalSection();
    BMXT_Wakeup_ClearInterruptToPMU_isr(handle);
    BKNI_LeaveCriticalSection();
}

void BMXT_Wakeup_ClearInterruptToPMU_isr(BMXT_Handle handle)
{
    uint32_t reg;
    BDBG_ASSERT(handle);

    reg = BMXT_RegRead32(handle, R(BCHP_DEMOD_XPT_WAKEUP_STATUS));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_STATUS, PKT_FOUND, 0);
    BMXT_RegWrite32(handle, R(BCHP_DEMOD_XPT_WAKEUP_STATUS), reg);
}

BERR_Code BMXT_Wakeup_SetPacketFilterBytes(BMXT_Handle handle, unsigned packetType, const BMXT_Wakeup_PacketFilter *pFilter)

{
    uint32_t reg, arrayBase;
    unsigned i;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pFilter);

    if (packetType >= BMXT_WAKEUP_MAX_PACKET_TYPES) {
        BDBG_ERR(("PacketType %u is larger than max (%u)", packetType, BMXT_WAKEUP_MAX_PACKET_TYPES));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    switch (packetType) {
        default:
        case 0: arrayBase = R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE); break;
        case 1: arrayBase = R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE1_i_ARRAY_BASE); break;
        case 2: arrayBase = R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE2_i_ARRAY_BASE); break;
        case 3: arrayBase = R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE); break;
    }

    for (i=0; i<BMXT_WAKEUP_MAX_PACKET_SIZE; i++) {
        reg = BMXT_RegRead32(handle, arrayBase + (i*4));
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_BYTE, pFilter[i].CompareByte);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK_TYPE, pFilter[i].MaskType);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK, pFilter[i].Mask);
        BMXT_RegWrite32(handle, arrayBase + (i*4), reg);
    }

    return 0;
}

void BMXT_Wakeup_Enable(BMXT_Handle handle, bool enable)
{
    uint32_t reg;
    BDBG_ASSERT(handle);

    reg = BMXT_RegRead32(handle, R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, PKT_DETECT_EN, enable ? 1 : 0);
    BMXT_RegWrite32(handle, R(BCHP_DEMOD_XPT_WAKEUP_CTRL), reg);
}
