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

#define BMXT_WAKE_R(reg) (handle->platform.regoffsetsWakeup ? (handle->platform.regoffsetsWakeup[reg] + handle->platform.regbaseWakeup) : 0)

static uint32_t BMXT_Wake_P_RegRead32(BMXT_Handle handle, uint32_t addr)
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
    BDBG_ASSERT(addr>=BMXT_WAKE_R(0));
    BDBG_ASSERT(addr<=(BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE)+4*200));

    return BMXT_RegRead32_common(handle, addr);
}

static void BMXT_Wake_P_RegWrite32(BMXT_Handle handle, uint32_t addr, uint32_t data)
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
    BDBG_ASSERT(addr>=BMXT_WAKE_R(0));
    BDBG_ASSERT(addr<=(BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE)+4*200));

    BMXT_RegWrite32_common(handle, addr, data);
    return;
}

void BMXT_Wakeup_GetSettings(BMXT_Handle handle, BMXT_Wakeup_Settings *pSettings)
{
    uint32_t reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);

    reg = BMXT_Wake_P_RegRead32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
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

    reg = BMXT_Wake_P_RegRead32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, PKT_LENGTH, pSettings->PacketLength);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, ERROR_INPUT_IGNORE, pSettings->ErrorInputIgnore);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL_MSB, (pSettings->InputBand >> 4) & 0x1);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, INPUT_SEL, pSettings->InputBand & 0xf);
    BMXT_Wake_P_RegWrite32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_CTRL), reg);

    return 0;
}

void BMXT_Wakeup_ClearInterruptToPMU(BMXT_Handle handle)
{
    uint32_t reg;
    BDBG_ASSERT(handle);

    reg = BMXT_Wake_P_RegRead32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_STATUS));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_STATUS, PKT_FOUND, 0);
    BMXT_Wake_P_RegWrite32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_STATUS), reg);
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
        case 0: arrayBase = BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE0_i_ARRAY_BASE); break;
        case 1: arrayBase = BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE1_i_ARRAY_BASE); break;
        case 2: arrayBase = BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE2_i_ARRAY_BASE); break;
        case 3: arrayBase = BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_PKT_TYPE3_i_ARRAY_BASE); break;
    }

    for (i=0; i<BMXT_WAKEUP_MAX_PACKET_SIZE; i++) {
        reg = BMXT_Wake_P_RegRead32(handle, arrayBase + (i*4));
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_BYTE, pFilter[i].CompareByte);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK_TYPE, pFilter[i].MaskType);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_PKT_TYPE0_i, COMPARE_MASK, pFilter[i].Mask);
        BMXT_Wake_P_RegWrite32(handle, arrayBase + (i*4), reg);
    }

    return 0;
}

void BMXT_Wakeup_Enable(BMXT_Handle handle, bool enable)
{
    uint32_t reg;
    BDBG_ASSERT(handle);

    reg = BMXT_Wake_P_RegRead32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_CTRL));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_WAKEUP_CTRL, PKT_DETECT_EN, enable ? 1 : 0);
    BMXT_Wake_P_RegWrite32(handle, BMXT_WAKE_R(BCHP_DEMOD_XPT_WAKEUP_CTRL), reg);
}
