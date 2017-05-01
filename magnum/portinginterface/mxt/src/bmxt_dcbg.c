/******************************************************************************
 *  Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "bchp_pwr.h"

#include "bmxt_index.h"
#include "bmxt_index_dcbg.h"
#include "bmxt_rdb_mask_shift_dcbg.h"

#include "bhab.h"

#if 0
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif

BDBG_MODULE(bmxt_dcbg);
BDBG_OBJECT_ID(BMXT_Dcbg_Handle);

/* 1: record pre-debonding input, 2: record debonded output */
#define ISSY_DNP_RECORD_MODE 0

/* ISSY/DNP record requires other hacks in XPT:
- BXPT_DirecTv_SetParserBandMode(): BCHP_SET_FIELD_DATA(Reg, XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PKT_LENGTH, 192);
- XPT_RSBUFF_PACKET_LENGTH = 192 and XPT_XCBUFF_PACKET_LENGTH = 192, but these are automatically set on "#if BXPT_NUM_TSIO"
TODO: consider printing an error when PARSER_PKT_LENGTH is not what's expected
*/

typedef struct BMXT_P_DcbgHandle
{
    BDBG_OBJECT(BMXT_Dcbg_Handle)
    BMXT_Handle hMxt;
    unsigned index;
    uint32_t mapVector;
    BMXT_Dcbg_Settings settings;
    bool running;
} BMXT_P_DcbgHandle;

#define GET_BIT(val, bit)   ((val >> bit) & 1)
#define SET_BIT(val, bit)   (val |= (1 << bit))
#define CLEAR_BIT(val, bit) (val &= ~(1 << bit))

#define R(reg) (hMxt->platform.regoffsetsDcbg[reg] + hMxt->platform.regbaseDcbg)
#define STEP(res) (hMxt->platform.stepsizeDcbg[res])
#define EXIST(reg) (hMxt->platform.regoffsetsDcbg[reg] != BMXT_NOREG)

static uint32_t BMXT_RegRead32(BMXT_Handle hMxt, uint32_t addr)
{
    if (addr - hMxt->platform.regbase == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=R(0));
    BDBG_ASSERT(addr<=R(BCHP_DEMOD_XPT_FE_OP_PIPE_BAND0_ADD_ATS_CONSTANT_BINARY));

    return BMXT_RegRead32_common(hMxt, addr);
}

static void BMXT_RegWrite32(BMXT_Handle hMxt, uint32_t addr, uint32_t data)
{
    if (addr - hMxt->platform.regbase == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=R(0));
    BDBG_ASSERT(addr<=R(BCHP_DEMOD_XPT_FE_OP_PIPE_BAND0_ADD_ATS_CONSTANT_BINARY));

    BMXT_RegWrite32_common(hMxt, addr, data);
    return;
}

void BMXT_Dcbg_GetDefaultOpenSettings(BMXT_Dcbg_OpenSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

/* reset any registers that are touched by BMXT_Dcbg_Start */
static void BMXT_Dcbg_Reset(BMXT_Handle hMxt)
{
    uint32_t addr, val;
    unsigned i;

    BDBG_ASSERT(hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_CNTR0_CTRL]==32);
    BDBG_ASSERT(hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_PACING0_CTRL]==32);
    BDBG_ASSERT(hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL]==32);
    BDBG_ASSERT(hMxt->platform.numDcbg[BMXT_RESOURCE_DCBG0_CTRL]==8);

    BDBG_MSG(("DCBG reset"));

    /* DCBG bands programming */
    for (i=0; i<32; i++)
    {
        addr = R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1) + (i * 4);
        val = 0x80; /* HW reset value */
        BMXT_RegWrite32(hMxt, addr, val);
    }

    /* map-vector programming */
    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_DELTA_EN);
    BMXT_RegWrite32(hMxt, addr, 0);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_EXTRAPOLATE_EN);
    BMXT_RegWrite32(hMxt, addr, 0);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_EN);
    BMXT_RegWrite32(hMxt, addr, 0);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_MODE);
    BMXT_RegWrite32(hMxt, addr, 0);

    for (i=0; i<hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_CNTR0_CTRL]; i++) {
        addr = R(BCHP_DEMOD_XPT_FE_ISSY_CNTR0_CTRL) + (i * STEP(BMXT_RESOURCE_ISSY_CNTR0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0);
    }

    for (i=0; i<hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_PACING0_CTRL]; i++) {
        addr = R(BCHP_DEMOD_XPT_FE_ISSY_PACING0_CTRL) + (i * STEP(BMXT_RESOURCE_ISSY_PACING0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0);
    }

    for (i=0; i<hMxt->platform.numDcbg[BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL]; i++) {
        addr = R(BCHP_DEMOD_XPT_FE_ISSY_RATIO_CNTR0_CTRL) + (i * STEP(BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0);
        addr = R(BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL) + (i * STEP(BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0xC);
    }

    for (i=0; i<hMxt->platform.numDcbg[BMXT_RESOURCE_DCBG0_CTRL]; i++) {
        addr = R(BCHP_DEMOD_XPT_FE_DCBG0_MAP_VECTOR) + (i * STEP(BMXT_RESOURCE_DCBG0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0);
        addr = R(BCHP_DEMOD_XPT_FE_DCBG0_CTRL) + (i * STEP(BMXT_RESOURCE_DCBG0_CTRL));
        BMXT_RegWrite32(hMxt, addr, 0x200);
    }
    return;
}

unsigned BMXT_Dcbg_GetFreeIndex(BMXT_Handle hMxt)
{
    unsigned i;
    for (i=0; i<BMXT_MAX_NUM_DCBG; i++) {
        if (hMxt->dcbg[i] == NULL) { break; }
    }
    return i; /* return i even if >= BMXT_MAX_NUM_DCBG. BMXT_Dcbg_Open will handle it */
}

BERR_Code BMXT_Dcbg_Open(BMXT_Handle hMxt, BMXT_Dcbg_Handle *phDcbg, unsigned index, BMXT_Dcbg_OpenSettings *pSettings)
{
    BREG_Handle hReg;
    BMXT_Dcbg_Handle dcbg = NULL;
    unsigned i;

    BDBG_ASSERT(hMxt);
    BDBG_ASSERT(pSettings);
    BDBG_ASSERT(phDcbg);

    if (hMxt->platform.regoffsetsDcbg==NULL) {
        BDBG_ERR(("No DCBG support on this demod chip"));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (index >= BMXT_MAX_NUM_DCBG) {
        /* TODO: could check that BMXT_MAX_NUM_DCBG == BMXT_NUMELEM_DCBG_45308[BMXT_RESOURCE_DCBG0_CTRL] */
        BDBG_ERR(("DCBG index %u not supported", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (hMxt->dcbg[index]!=NULL) {
        BDBG_ERR(("DCBG index %u already opened", index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i=0; i<BMXT_MAX_NUM_DCBG; i++) {
        if (hMxt->dcbg[i]!=NULL) { break; }
    }
    if (i==BMXT_MAX_NUM_DCBG) {
        BMXT_Dcbg_Reset(hMxt);
    }

    hReg = hMxt->hReg;

    *phDcbg = NULL;
    dcbg = BKNI_Malloc(sizeof(*dcbg));
    if (dcbg == NULL) {
        return BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(dcbg, 0, sizeof(*dcbg));
    BDBG_OBJECT_SET(dcbg, BMXT_Dcbg_Handle);
    dcbg->hMxt = hMxt;
    dcbg->index = index;

    *phDcbg = dcbg;
    hMxt->dcbg[index] = dcbg;
    BDBG_MSG(("%u: Open", index));

    return BERR_SUCCESS;
}

void BMXT_Dcbg_Close(BMXT_Dcbg_Handle hDcbg)
{
    BREG_Handle hReg;
    unsigned i;

    BDBG_ASSERT(hDcbg);
    hReg = hDcbg->hMxt->hReg;

    if (hDcbg->running) {
        BDBG_WRN(("%u: Close without Stop. Forcing stop...", hDcbg->index));
        BMXT_Dcbg_Stop(hDcbg);
    }
    BDBG_ASSERT(!hDcbg->running);

    if (hDcbg->mapVector) {
        BDBG_WRN(("%u: Close without RemoveParsers on all parsers, %08x", hDcbg->index, hDcbg->mapVector));
        for (i=0; i<32; i++)
        {
            if (GET_BIT(hDcbg->mapVector, i)) {
                BDBG_WRN(("%u: Forcing RemoveParser on parser %u", hDcbg->index, i));
                BMXT_Dcbg_RemoveParser(hDcbg, i);
            }
        }
    }

    hDcbg->hMxt->dcbg[hDcbg->index] = NULL;
    BDBG_MSG(("%u: Close", hDcbg->index));
    BDBG_OBJECT_UNSET(hDcbg, BMXT_Dcbg_Handle);
    BKNI_Free(hDcbg);
}

BERR_Code BMXT_Dcbg_AddParser(BMXT_Dcbg_Handle hDcbg, unsigned parserNum)
{
    unsigned i, ibNum;
    BMXT_Handle hMxt;
    BREG_Handle hReg;
    BDBG_ASSERT(hDcbg);
    hMxt = hDcbg->hMxt;
    hReg = hDcbg->hMxt->hReg;

    if (parserNum >= hDcbg->hMxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (hDcbg->running) {
        BDBG_ERR(("%u: Cannot add parsers while DCBG is enabled", hDcbg->index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (GET_BIT(hDcbg->mapVector, parserNum)) {
        BDBG_WRN(("%u: Parser %u already added to DCBG", hDcbg->index, parserNum));
        return BERR_SUCCESS;
    }
    for (i=0; i<BMXT_MAX_NUM_DCBG; i++) {
        BMXT_Dcbg_Handle hmDcbg = hDcbg->hMxt->dcbg[i];
        if (i==hDcbg->index) { continue; }
        if (hmDcbg && GET_BIT(hmDcbg->mapVector, parserNum)) {
            BDBG_ERR(("%u: Parser %u already added to DCBG%u", hDcbg->index, parserNum, i)); /* cannot continue */
            return BERR_TRACE(BERR_INVALID_PARAMETER);
        }
    }

    ibNum = BMXT_P_GetParserInputBand(hMxt, parserNum);
    SET_BIT(hDcbg->mapVector, parserNum);
    BDBG_MSG(("%u: AddParser %u (IB%u), %08x", hDcbg->index, parserNum, ibNum, hDcbg->mapVector));

    return BERR_SUCCESS;
}

BERR_Code BMXT_Dcbg_RemoveParser(BMXT_Dcbg_Handle hDcbg, unsigned parserNum)
{
    BDBG_ASSERT(hDcbg);

    if (parserNum >= hDcbg->hMxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (hDcbg->running) {
        BDBG_ERR(("%u: Cannot remove parsers while DCBG is enabled", hDcbg->index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (GET_BIT(hDcbg->mapVector, parserNum)==0) {
        BDBG_WRN(("%u: Parser %u not part of DCBG", hDcbg->index, parserNum));
    }

    CLEAR_BIT(hDcbg->mapVector, parserNum);
    BDBG_MSG(("%u: RemoveParser %u, %08x", hDcbg->index, parserNum, hDcbg->mapVector));

    return BERR_SUCCESS;
}

BERR_Code BMXT_Dcbg_RemoveAllParsers(BMXT_Dcbg_Handle hDcbg)
{
    unsigned i;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_ASSERT(hDcbg);

    if (hDcbg->running) {
        BDBG_ERR(("%u: Cannot remove parsers while DCBG is enabled", hDcbg->index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i=0; i<32; i++) {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }
        rc = BMXT_Dcbg_RemoveParser(hDcbg, i);
        if (rc) {
            rc = BERR_TRACE(rc);
        }
    }

    return rc;
}

void BMXT_Dcbg_GetDefaultSettings(BMXT_Dcbg_Handle hDcbg, BMXT_Dcbg_Settings *pSettings)
{
    BDBG_ASSERT(hDcbg);
    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    return;
}

#define MIN_BANDS_PER_DCBG 2
#define MAX_BANDS_PER_DCBG 8

BERR_Code BMXT_Dcbg_Start(BMXT_Dcbg_Handle hDcbg, const BMXT_Dcbg_Settings *pSettings)
{
    BMXT_Handle hMxt;
    BREG_Handle hReg;
    uint32_t addr, val;
    unsigned i, priband, priband_ib, priband_mtsifTxSel, priband_virtualParserNum;
    BMXT_ParserConfig parserConfig;

    BDBG_ASSERT(hDcbg);
    hMxt = hDcbg->hMxt;
    hReg = hDcbg->hMxt->hReg;
    hDcbg->settings = *pSettings;
    priband = pSettings->primaryBand;

    if (hDcbg->running) {
        BDBG_ERR(("%u: already started", hDcbg->index));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    for (i=0, val=0; i<32; i++) {
        if (GET_BIT(hDcbg->mapVector, i)) { val++; }
    }
    if (!((val >= MIN_BANDS_PER_DCBG) && (val <= MAX_BANDS_PER_DCBG))) {
        BDBG_ERR(("%u: invalid number of bands (%u) in DCBG (map %08x)", hDcbg->index, val, hDcbg->mapVector));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    if (((hDcbg->mapVector >> priband) & 1)==0) {
        BDBG_ERR(("%u: invalid primary band %u (map %08x)", hDcbg->index, priband, hDcbg->mapVector));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    /* DCBG bands programming */
    for (i=0; i<32; i++)
    {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }

        addr = R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1) + (i * 4);
        val = BMXT_RegRead32(hMxt, addr);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_SEL, priband);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DCBG_PRI_BAND, priband);
#if (ISSY_DNP_RECORD_MODE==1)
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_SEL, i);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DCBG_PRI_BAND, i);
#endif
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_SNAPSHOT_EN, 1);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_CNTR_ARM, i==priband ? 1 : 0); /* note, HW will clear this bit */
#if (ISSY_DNP_RECORD_MODE==1)
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_CNTR_ARM, 1);
#endif
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_EXTRACT_ISSY_DNP, 1);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DCB_EN, 1);
#if ISSY_DNP_RECORD_MODE /* for both 1 and 2 */
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1, DVB_ISSY_RECEIVED_TX_EN, 1);
#endif
        BMXT_RegWrite32(hMxt, addr, val);
    }

    /* map-vector programming */
    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_DELTA_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val |= hDcbg->mapVector;
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_EXTRAPOLATE_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val |= hDcbg->mapVector;
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val |= hDcbg->mapVector;
#if (ISSY_DNP_RECORD_MODE==1)
    val &= ~(hDcbg->mapVector);
#endif
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_MODE);
    val = BMXT_RegRead32(hMxt, addr);
    val &= ~(hDcbg->mapVector); /* 0 for MOD300 output ATS */
    BMXT_RegWrite32(hMxt, addr, val);

    /* priband-only programming */
    BMXT_GetParserConfig(hMxt, priband, &parserConfig);
    priband_ib = parserConfig.InputBandNumber;
    priband_mtsifTxSel = parserConfig.mtsifTxSelect;
    priband_virtualParserNum = parserConfig.virtualParserNum;

    addr = R(BCHP_DEMOD_XPT_FE_ISSY_CNTR0_CTRL) + (priband * STEP(BMXT_RESOURCE_ISSY_CNTR0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_CNTR0_CTRL, SYMBOL_CLK_SEL, priband_ib);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_ISSY_PACING0_CTRL) + (priband * STEP(BMXT_RESOURCE_ISSY_PACING0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, PACING_LAG, 0); /* TODO: ask */
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, PACING_LAG_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, ERR_THRESH_LATE_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, ERR_THRESH_EARLY_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, AUTORESTART_ON_ERROR_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, FORCE_RESTART, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, FORCE_RESTAMP_ATS_MODE, 0); /* MOD300 */
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, FORCE_RESTAMP, 0); /* disable */
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, PACING_EN, 1);
    BMXT_RegWrite32(hMxt, addr, val);

#if (ISSY_DNP_RECORD_MODE==1)
    /* disable ISSY PACING for all bands */
    for (i=0; i<32; i++)
    {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }

        addr = R(BCHP_DEMOD_XPT_FE_ISSY_PACING0_CTRL) + (i * STEP(BMXT_RESOURCE_ISSY_PACING0_CTRL));
        val = BMXT_RegRead32(hMxt, addr);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, PACING_EN, 0);
        BMXT_RegWrite32(hMxt, addr, val);
    }
#endif

    addr = R(BCHP_DEMOD_XPT_FE_ISSY_RATIO_CNTR0_CTRL) + (priband * STEP(BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_RATIO_CNTR0_CTRL, RATIO_SYMBOL_CLK_SEL, priband_ib);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL) + (priband * STEP(BMXT_RESOURCE_ISSY_RATIO_CNTR0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL, DROP_ON_RATIO_INVALID, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL, CAPTURE_AT_RO_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL, CAPTURE_EN, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_RATIO_SNAPSHOT0_CTRL, SW_START, 1);
    BMXT_RegWrite32(hMxt, addr, val);

    /* set MTSIF_TX BAND_ID to priband's destination for all bands */
    for (i=0; i<32; i++) {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }
        BMXT_P_SetVirtualParserNum(hMxt, priband_mtsifTxSel, i, priband_virtualParserNum);
#if (ISSY_DNP_RECORD_MODE==1)
        /* this assumes that demodPB and hostPB are equal, but MXT has no way to verify this info because hostPBs are not explicitly specified for non-master frontends */
        BMXT_P_SetVirtualParserNum(hMxt, priband_mtsifTxSel, i, i);
#endif
    }

    /* DCBG programming */
    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_BO) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_BO, BO_COUNT, 0); /* not set since DCBG_BO_EN = 0. note that is per-DCBG, not per-band */
    BMXT_RegWrite32(hMxt, addr, val);

#if 0 /* for debug */
    addr = R(BCHP_DEMOD_XPT_FE_SLOT_MANAGEMENT_BAND_MAX_POSSIBLE_SLOT_ALLOCATION); /* accomodate large number of packets sent in one band at once */
    BMXT_RegWrite32(hMxt, addr, 0x800);
#endif
#if 0 /* for debug */
    addr = R(BCHP_DEMOD_XPT_FE_DCB_MISC_CFG); /* accomodate large skew */
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCB_MISC_CFG, MAX_ISSY_DELTA, 0x300000);
    BMXT_RegWrite32(hMxt, addr, val);
#endif

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_MAP_VECTOR) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = hDcbg->mapVector;
#if (ISSY_DNP_RECORD_MODE==1)
    val = 0;
#endif
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_CTRL) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_BO_EN, 0); /* 0 to use ISSY pacing */
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_AUTORECOVER_ON_ERROR, 0);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_REPORT_ERROR_ONLY, 1);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_PRI_BAND, priband);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_ENABLE, 1);
#if (ISSY_DNP_RECORD_MODE==1)
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_ENABLE, 0);
#endif
    BMXT_RegWrite32(hMxt, addr, val);

    /* enable all bands */
    for (i=0; i<32; i++) {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }
        BMXT_P_SetParserEnable(hMxt, i, true);
    }

    /* note, priband from modulator perspective is the frequency. for XPT, it's the PB. for user, it's the frontend handle */
    BDBG_MSG(("%u: Start: demod priband %u (IB%u), map %08x", hDcbg->index, priband, priband_ib, hDcbg->mapVector));

    hDcbg->running = true;
    return BERR_SUCCESS;
}

void BMXT_Dcbg_Stop(BMXT_Dcbg_Handle hDcbg)
{
    BMXT_Handle hMxt;
    BREG_Handle hReg;
    uint32_t addr, val;
    unsigned priband, i;
    BDBG_ASSERT(hDcbg);

    hMxt = hDcbg->hMxt;
    hReg = hDcbg->hMxt->hReg;
    priband = hDcbg->settings.primaryBand;

    BDBG_ASSERT(hDcbg);
    if (!hDcbg->running) {
        BDBG_WRN(("%u: already stopped", hDcbg->index));
        /* continue */
    }

    /* disable all bands and increment PARSER_VERSION */
    for (i=0; i<32; i++) {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }
        BMXT_P_SetParserEnable(hMxt, i, false);
        BMXT_P_ParserVersion(hMxt, i);
    }

    addr = R(BCHP_DEMOD_XPT_FE_ISSY_PACING0_CTRL) + (priband * STEP(BMXT_RESOURCE_ISSY_PACING0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_ISSY_PACING0_CTRL, PACING_EN, 0);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_CTRL) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_PRI_BAND, 0);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_ENABLE, 0);
    BMXT_RegWrite32(hMxt, addr, val);

    for (i=0; i<32; i++)
    {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }

        addr = R(BCHP_DEMOD_XPT_FE_BAND_DROP_TILL_LAST_SET);
        val = BMXT_RegRead32(hMxt, addr);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_BAND_DROP_TILL_LAST_SET, BAND_NUM, i);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_BAND_DROP_TILL_LAST_SET, SET, 1);
        BMXT_RegWrite32(hMxt, addr, val);
    }

    /* DCBG bands programming */
    for (i=0; i<32; i++)
    {
        if (GET_BIT(hDcbg->mapVector, i)==0) { continue; }

        addr = R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_DVB_CTRL1) + (i * 4);
        val = 0x80; /* HW reset value */
        BMXT_RegWrite32(hMxt, addr, val);
    }

    /* map-vector programming */
    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_DELTA_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val &= ~(hDcbg->mapVector);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ISSY_EXTRAPOLATE_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val &= ~(hDcbg->mapVector);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_EN);
    val = BMXT_RegRead32(hMxt, addr);
    val &= ~(hDcbg->mapVector);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_BAND_ATS_ADJUST_MODE);
    val = BMXT_RegRead32(hMxt, addr);
    val &= ~(hDcbg->mapVector);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_MAP_VECTOR) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = 0;
    BMXT_RegWrite32(hMxt, addr, val);

    hDcbg->running = false;
    return;
}

void BMXT_Dcbg_GetSettings(BMXT_Dcbg_Handle hDcbg, BMXT_Dcbg_Settings *pSettings)
{
    BDBG_ASSERT(hDcbg);
    BDBG_ASSERT(pSettings);
    /* TODO: reconsider */
#if 0
    pSettings->primaryBand = hDcbg->settings.primaryBand;
#endif
    return;
}

BERR_Code BMXT_Dcbg_SetSettings(BMXT_Dcbg_Handle hDcbg, const BMXT_Dcbg_Settings *pSettings)
{
    BDBG_ASSERT(hDcbg);
    BDBG_ASSERT(pSettings);
    /* TODO: reconsider */
#if 0
    if (hDcbg->running) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    hDcbg->settings.primaryBand = pSettings->primaryBand;
#endif
    return BERR_SUCCESS;
}

BERR_Code BMXT_Dcbg_GetStatus(BMXT_Dcbg_Handle hDcbg, BMXT_Dcbg_Status *pStatus)
{
    BMXT_Handle hMxt;
    BREG_Handle hReg;
    uint32_t addr, val;
    BDBG_ASSERT(hDcbg);

    hMxt = hDcbg->hMxt;
    hReg = hDcbg->hMxt->hReg;

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_STATUS) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    pStatus->locked = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_STATUS, DCBG_SEQ_STATE)==3;
    return BERR_SUCCESS;
}

static void BMXT_Dcbg_AtsIssySnapshot(BMXT_Dcbg_Handle hDcbg)
{
    uint32_t addr;
    BMXT_Handle hMxt = hDcbg->hMxt;
    if (hDcbg->mapVector==0) {
        BDBG_ERR(("Null mapvector for ATS/ISSY snapshot"));
        return;
    }

    addr = R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_PARSER31_ATS_ISSY_SNAPSHOT_CTRL1);
    BMXT_RegWrite32(hMxt, addr, hDcbg->mapVector);
    BDBG_WRN(("ATS/ISSY snapshot arm %08x", hDcbg->mapVector));
}

/* arm snapshot for first DCBG and dump registers */
static void BMXT_Dcbg_DumpAtsIssySnapshot(BMXT_Handle hMxt)
{
    unsigned i;
    uint32_t addr, val;
    const unsigned TIMEOUT = 10;
    BMXT_Dcbg_Handle hDcbg;
    for (i=0; i<BMXT_MAX_NUM_DCBG; i++) {
        hDcbg = hMxt->dcbg[i];
        if (hDcbg && hDcbg->mapVector) {
            BMXT_Dcbg_AtsIssySnapshot(hDcbg);
            break;
        }
    }
    if (i>=BMXT_MAX_NUM_DCBG) {
        BDBG_ERR(("No valid DCBG for ATS/ISSY snapshot"));
        return;
    }

    /* wait for valid */
    for (i=0; i<TIMEOUT; i++) {
        addr = R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_PARSER31_ATS_ISSY_SNAPSHOT_VALID);
        val = BMXT_RegRead32(hMxt, addr);
        if ((val & hDcbg->mapVector)==hDcbg->mapVector) { break; }
        BKNI_Sleep(10);
    }

    if (i>=TIMEOUT) {
        BDBG_ERR(("ATS/ISSY snapshot valid timeout (valid=%08x)", val));
        return;
    }

    BMXT_P_RegDump(hMxt);
}

static void BMXT_P_BlockoutToggle(BMXT_Handle hMxt)
{
    unsigned i;
    uint32_t addr, val;
    BMXT_Dcbg_Handle hDcbg;
    for (i=0; i<BMXT_MAX_NUM_DCBG; i++) {
        hDcbg = hMxt->dcbg[i];
        if (hDcbg && hDcbg->mapVector) {
            break;
        }
    }

    if (i>=BMXT_MAX_NUM_DCBG) {
        BDBG_ERR(("No valid DCBG"));
        return;
    }

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_BO) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_BO, BO_COUNT, 0xffff);
    BMXT_RegWrite32(hMxt, addr, val);

    addr = R(BCHP_DEMOD_XPT_FE_DCBG0_CTRL) + (hDcbg->index * STEP(BMXT_RESOURCE_DCBG0_CTRL));
    val = BMXT_RegRead32(hMxt, addr);
    if (BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_BO_EN)) {
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_BO_EN, 0);
        BMXT_RegWrite32(hMxt, addr, val);
        BDBG_WRN(("DCBG%u DCBG_BO_EN=%u", hDcbg->index, 0));
    }
    else {
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_DCBG0_CTRL, DCBG_BO_EN, 1);
        BMXT_RegWrite32(hMxt, addr, val);
        BDBG_WRN(("DCBG%u DCBG_BO_EN=%u", hDcbg->index, 1));
    }
    BMXT_P_RegDump(hMxt);
}
