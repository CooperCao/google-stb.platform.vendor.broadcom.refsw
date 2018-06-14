/******************************************************************************
 * Copyright (C) 2018 Broadcom.
 * The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to
 * the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied),
 * right to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 * THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 * IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 * THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 * OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#include "bstd.h"
#include "bchp.h"
#include "bkni.h"
#include "bmxt_priv.h"
#include "bmxt.h"
#include "bchp_pwr.h"

#include "bmxt_index.h"
#include "bmxt_rdb_mask_shift.h"

#include "bhab.h"

#if 0
#define BDBG_MSG_TRACE(x) BDBG_MSG(x)
#else
#define BDBG_MSG_TRACE(x)
#endif

BDBG_MODULE(bmxt_fe);

#define BMXT_R(reg) (handle->platform.regoffsets[reg] + handle->platform.regbase)
#define BMXT_STEP(res) (handle->platform.stepsize[res])
#define BMXT_EXIST(reg) (handle->platform.regoffsets[reg] != BMXT_NOREG)
#define BMXT_HAS_PID_TABLE() (handle->settings.chip!=BMXT_Chip_e45402)

#define BMXT_IS_3128_FAMILY() ((handle->settings.chip==BMXT_Chip_e3128) || (handle->settings.chip==BMXT_Chip_e3383) || (handle->settings.chip==BMXT_Chip_e4528) || (handle->settings.chip==BMXT_Chip_e4517) || (handle->settings.chip==BMXT_Chip_e3472))
#define BMXT_IS_4538_FAMILY() ((handle->settings.chip==BMXT_Chip_e4538) || (handle->settings.chip==BMXT_Chip_e3384) || (handle->settings.chip==BMXT_Chip_e4548) || (handle->settings.chip==BMXT_Chip_e7364) || (handle->settings.chip==BMXT_Chip_e7366) || (handle->settings.chip==BMXT_Chip_e7145) || (handle->settings.chip==BMXT_Chip_e45216))
#define BMXT_IS_45308_FAMILY() (BMXT_EXIST(BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_STATUS))

#define PARSER_OUTPUT_PIPE_SEL_TO_INDEX(pops)    (BMXT_IS_3128_FAMILY() ? pops-1 : pops)
#define PARSER_OUTPUT_PIPE_SEL_FROM_INDEX(index) (BMXT_IS_3128_FAMILY() ? index+1 : index)

#define BMXT_P_LEGACY_PARSER_BAND_ID_MAX 16 /* on pre-4538, MINI_PID_PARSER?_TO_PARSER?_BAND_ID.PARSER?_BAND_ID is a 4-bit number */

const char* const BMXT_CHIP_STR[] = {"3128", "3158", "3383", "3384", "3390", "3466", "3472", "4517", "4528", "4538", "4548", "45216", "45308", "45316", "45402", "7145", "7366", "7364"};
const char* const BMXT_PLATFORM_TYPE_STR[] = {"HAB", "RPC", "REG"};

#define VIRTUAL_HANDLE_REG_OFFSET 0x80000000 /* hard-coded for now */
static uint32_t BMXT_RegRead32(BMXT_Handle handle, uint32_t addr)
{
    if (addr - handle->platform.regbase == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return 0;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=BMXT_R(0));
    BDBG_ASSERT(addr<=BMXT_R(BCHP_DEMOD_XPT_FE_SPID_TABLE_i_ARRAY_BASE)+4*512);

    return BMXT_RegRead32_common(handle, addr);
}

static void BMXT_RegWrite32(BMXT_Handle handle, uint32_t addr, uint32_t data)
{
    if (addr - handle->platform.regbase == BMXT_NOREG) {
        BERR_TRACE(BERR_UNKNOWN);
        return;
    }

    BDBG_ASSERT(addr%4==0);
    BDBG_ASSERT(addr>=BMXT_R(0));
    BDBG_ASSERT(addr<=BMXT_R(BCHP_DEMOD_XPT_FE_SPID_TABLE_i_ARRAY_BASE)+4*512);

    BMXT_RegWrite32_common(handle, addr, data);
    return;
}

BERR_Code BMXT_GetDefaultSettings(BMXT_Settings *pSettings)
{
    unsigned i;

    BDBG_ASSERT(pSettings);
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));

    for (i=0; i<BMXT_MAX_NUM_MTSIF_TX; i++) {
        pSettings->MtsifTxCfg[i].TxInterfaceWidth = 8;
        pSettings->MtsifTxCfg[i].TxClockPolarity = 1;
        pSettings->MtsifTxCfg[i].Enable = false;
    }

    for (i=0; i<BMXT_MAX_NUM_MTSIF_RX; i++) {
        pSettings->MtsifRxCfg[i].RxInterfaceWidth = 8;
        pSettings->MtsifRxCfg[i].RxClockPolarity = 0;
        pSettings->MtsifRxCfg[i].Enable = false;
    }

    return BERR_SUCCESS;
}

static BERR_Code BMXT_Open_PreOpen(BMXT_Handle *pHandle, BCHP_Handle hChp, BREG_Handle hReg, const BMXT_Settings *pSettings)
{
    BMXT_Handle mxt = NULL;
    BERR_Code rc = BERR_SUCCESS;
    unsigned i;

    BDBG_ASSERT(pHandle);
    BDBG_ASSERT(pSettings);

    mxt = BKNI_Malloc(sizeof(BMXT_P_TransportData));
    if (mxt==NULL) {
        BDBG_ERR(("BKNI_Malloc() failed!"));
        rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        goto error;
    }

    BKNI_Memset(mxt, 0, sizeof(BMXT_P_TransportData));
    mxt->hChp = hChp;
    mxt->hReg = hReg;
    mxt->hHab = pSettings->hHab;
    mxt->hRpc = pSettings->hRpc;
    mxt->settings = *pSettings;

    BMXT_P_SetPlatform(mxt, 0, 0, 0);

    if (!pSettings->hVirtual) {
        switch (mxt->platform.type) {
            case BMXT_P_PlatformType_eHab:
                if (!mxt->hHab) {
                    BDBG_ERR(("BMXT_DefaultSettings.hHab must be set"));
                    rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error;
                }
                break;
            case BMXT_P_PlatformType_eRpc:
                if (!mxt->hRpc) {
                    BDBG_ERR(("BMXT_DefaultSettings.hRpc must be set"));
                    rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error;
                }
                break;
            case BMXT_P_PlatformType_eReg:
                if (!mxt->hReg || !mxt->hChp) {
                    BDBG_ERR(("BREG_Handle and BCHP_Handle arguments must be set"));
                    rc = BERR_TRACE(BERR_INVALID_PARAMETER); goto error;
                }
                break;
        }
    }

    BDBG_ASSERT(mxt->platform.num[BMXT_RESOURCE_IB0_CTRL] <= BMXT_MAX_NUM_INPUT_BANDS);
    BDBG_ASSERT(mxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1] <= BMXT_MAX_NUM_PARSERS);
    BDBG_ASSERT(mxt->platform.num[BMXT_RESOURCE_TSMF0_CTRL] <= BMXT_MAX_NUM_TSMF);
    BDBG_ASSERT(mxt->platform.num[BMXT_RESOURCE_MTSIF_RX0_CTRL1] <= BMXT_MAX_NUM_MTSIF_RX);
    BDBG_ASSERT(mxt->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1] <= BMXT_MAX_NUM_MTSIF_TX);

    BDBG_CASSERT(sizeof(BMXT_CHIP_STR)/sizeof(*BMXT_CHIP_STR)==BMXT_Chip_eMax);
    BDBG_MSG(("Open: %p, platform %s (%u), rev %u, type %s, regbase 0x%08x",
        (void*)mxt, BMXT_CHIP_STR[mxt->settings.chip], mxt->settings.chip, mxt->settings.chipRev, BMXT_PLATFORM_TYPE_STR[mxt->platform.type], mxt->platform.regbase));
    BDBG_MSG(("Open: resources: %u %u %u %u %u",
        mxt->platform.num[BMXT_RESOURCE_IB0_CTRL], mxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1],
        mxt->platform.num[BMXT_RESOURCE_TSMF0_CTRL], mxt->platform.num[BMXT_RESOURCE_MTSIF_RX0_CTRL1], mxt->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]));

    for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]; i++) {
        if (pSettings->MtsifTxCfg[i].TxInterfaceWidth != 8) {
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
    }

    for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MTSIF_RX0_CTRL1]; i++) {
        if (pSettings->MtsifRxCfg[i].RxInterfaceWidth != 8) {
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
    }

    if (pSettings->enablePidFiltering) {
        BMXT_Handle handle = mxt;
        if (BMXT_HAS_PID_TABLE()) {
            BDBG_WRN(("Open: enable demod pid filtering on %s", BMXT_CHIP_STR[mxt->settings.chip]));
        }
        else {
            BDBG_WRN(("Open: demod pid filtering not supported on %s", BMXT_CHIP_STR[mxt->settings.chip]));
        }
    }
#if 0
{
    /* manually trigger DEMOD_XPT_FE reset on 45308. no longer needed */
    BHAB_Handle hab = mxt->hHab;
    uint32_t addr = 0x7020410;
    uint32_t reg;
    BHAB_ReadRegister(hab, addr, &reg);
    reg |= (1 << 3);
    BHAB_WriteRegister(hab, addr, &reg);
    reg &= ~(1 << 3);
    BHAB_WriteRegister(hab, addr, &reg);
}
#endif

    *pHandle = mxt;
    return rc;

error:
    if (mxt) {
        BKNI_Free(mxt);
    }
    *pHandle = NULL;
    return rc;
}

static void BMXT_Open_PostOpen(BMXT_Handle mxt)
{
    uint32_t reg, addr;
    unsigned i;
    BMXT_Settings *pSettings = &mxt->settings;
    BMXT_Handle handle = mxt; /* for R() / STEP() macros */

#ifdef BCHP_PWR_RESOURCE_XPT_DEMOD
    if (mxt->platform.type==BMXT_P_PlatformType_eReg) {
        BCHP_PWR_AcquireResource(mxt->hChp, BCHP_PWR_RESOURCE_XPT_DEMOD);
    }
#endif

    /* SW7425-1655 */
    reg = BMXT_RegRead32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_ATS_COUNTER_CTRL));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_ATS_COUNTER_CTRL, INC_MUX_SEL, 1);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_ATS_COUNTER_CTRL, EXT_RESET_ENABLE, 1);
    BMXT_RegWrite32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_ATS_COUNTER_CTRL), reg);

    /* clear interrupt status regs */
    if (BMXT_EXIST(BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG)) { /* pre-45308 */
        BMXT_RegWrite32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG), 0);
        BMXT_RegWrite32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG), 0);
    }

    for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]; i++) {
        BDBG_ASSERT(pSettings->MtsifTxCfg[i].TxInterfaceWidth==8);

        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_CTRL1) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));

        reg = BMXT_RegRead32(mxt, addr);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, MTSIF_TX_IF_WIDTH, 3); /* 8-bit wide */
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, MTSIF_TX_CLOCK_POL_SEL, pSettings->MtsifTxCfg[i].TxClockPolarity);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, TX_ENABLE, pSettings->MtsifTxCfg[i].Enable==true ? 1 : 0);
        BMXT_RegWrite32(mxt, addr, reg);

        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_SECRET_WORD) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));
        BMXT_RegWrite32(mxt, addr, pSettings->MtsifTxCfg[i].Encrypt ? 0:0x829eecde);
    }

    for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MTSIF_RX0_CTRL1]; i++) {
        BDBG_ASSERT(pSettings->MtsifRxCfg[i].RxInterfaceWidth==8);

        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_RX0_CTRL1)+ (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_RX0_CTRL1));
        reg = BMXT_RegRead32(mxt, addr);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_IF_WIDTH, 3); /* 8-bit wide */
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_CLOCK_POL_SEL, pSettings->MtsifRxCfg[i].RxClockPolarity);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, PARSER_ENABLE, pSettings->MtsifRxCfg[i].Enable==true ? 1 : 0);
        BMXT_RegWrite32(mxt, addr, reg);

        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_RX0_SECRET_WORD) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_RX0_CTRL1));
        BMXT_RegWrite32(mxt, addr, pSettings->MtsifRxCfg[i].Decrypt ? 0:0x829eecde);
    }

    for (i=0; i<mxt->platform.num[BMXT_RESOURCE_TSMF0_CTRL]; i++) {
        BMXT_TSMFFldVerifyConfig tsmfConfig;
        BKNI_Memset(&tsmfConfig, 0, sizeof(tsmfConfig));
        BMXT_TSMF_SetFldVerifyConfig(mxt, i, &tsmfConfig);
        BMXT_TSMF_EnableAutoMode(mxt, 0, i, 0);
        BMXT_TSMF_EnableSemiAutoMode(mxt, 0, i, 0, 0, 0);
        BMXT_TSMF_DisableTsmf(mxt, i);
    }

#if 0
    for (i=0; i<256; i++) {
        BKNI_Printf("PID_TABLE %3u: %#x\n", i, BMXT_RegRead32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + 4*i));
    }
#endif

    if (handle->settings.enablePidFiltering==false) {
        for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]; i++) {
            uint32_t val = 0;
            unsigned pipeSel = PARSER_OUTPUT_PIPE_SEL_FROM_INDEX(0); /* default to TX0 */
            if (!BMXT_HAS_PID_TABLE()) { continue; }

            addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*i);
            reg = BMXT_RegRead32(mxt, addr);
            #if 0
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, SECURITY_OUTPUT_PIPE_SEL, 0);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, IGNORE_PID_VERSION, 0);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 0); /* not playback */
            #endif
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, pipeSel);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, i);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, MPOD_BYPASS_EN, 0);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, 1);
            #if 0
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER, 0);
            BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, PID_CHANNEL_PID, 0); /* don't care */
            #endif
            BDBG_MSG(("PID_TABLE %3u: %08x -> %08x", i, reg, val));
            BMXT_RegWrite32(mxt, addr, val);
        }

        /* for platforms with PARSER-level BAND_ID mapping (pre-4538), set correct MTSIF_TX defaults on possible virtual (not physical) parser's PID_TABLE entries */
        if (handle->platform.regoffsets[BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID] == BMXT_NOREG) {
            for (; i<BMXT_P_LEGACY_PARSER_BAND_ID_MAX; i++) {
                uint32_t val = 0;
                unsigned pipeSel = PARSER_OUTPUT_PIPE_SEL_FROM_INDEX(0); /* default to TX0 */
                if (!BMXT_HAS_PID_TABLE()) { continue; }

                addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*i);
                reg = BMXT_RegRead32(mxt, addr);
                BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, pipeSel);
                BMXT_RegWrite32(mxt, addr, val);
            }
        }
    }

    /* set IB, PB and IB->PB mapping defaults */
    {
        BMXT_InputBandConfig ibConfig;
        for (i=0; i<mxt->platform.num[BMXT_RESOURCE_IB0_CTRL]; i++) {
            BMXT_GetInputBandConfig(mxt, i, &ibConfig);
            ibConfig.DssMode = false;
            BMXT_SetInputBandConfig(mxt, i, &ibConfig);
        }
    }

    {
        BMXT_ParserConfig pbConfig;
        for (i=0; i<mxt->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]; i++) {
            BMXT_GetParserConfig(mxt, i, &pbConfig);
            pbConfig.ErrorInputIgnore = false;
            pbConfig.TsMode = BMXT_ParserTimestampMode_eMod300;
            if (mxt->settings.enablePidFiltering==false) {
                pbConfig.AcceptNulls = true;
                pbConfig.AllPass = true;
            }
            pbConfig.DssMode = false;
            pbConfig.InputBandNumber = i; /* default to 1-to-1 mapping */
            pbConfig.virtualParserNum = i; /* default to 1-to-1 mapping */
            pbConfig.Enable = false;
            BMXT_SetParserConfig(mxt, i, &pbConfig);
            BMXT_TSMF_SetParserConfig(mxt, i, 0, false);
        }
    }

#if 0 /* DEPRECATED */
    {
        BMXT_ParserBandMapping parserMapping;
        BMXT_GetParserMapping(mxt, &parserMapping);
        for (i=0; i<mxt->num.pb; i++) {
            parserMapping.FrontEnd[i].VirtualParserBandNum = i;
            parserMapping.FrontEnd[i].VirtualParserIsPlayback = false;
        }
        BMXT_SetParserMapping(mxt, &parserMapping);
    }
#endif

    return;
}

void BMXT_P_Inbuf_Pwr_Ctrl(BMXT_Handle handle, bool powerUp)
{
    static const unsigned BMXT_P_INPUF_MEM0_ONLY = 0xFFFFFFFE;
    uint32_t val;
    BMXT_RegWrite32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_CTRL), powerUp ? 0 : BMXT_P_INPUF_MEM0_ONLY);
    val = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_STATUS));
    BDBG_MSG(("INBUF powerup %u, STATUS %08x", (unsigned)powerUp, val));
}

BERR_Code BMXT_Open(BMXT_Handle *pHandle, BCHP_Handle hChp, BREG_Handle hReg, const BMXT_Settings *pSettings)
{
    BMXT_Handle mxt, handle;
    unsigned i;
    BERR_Code rc;
    uint32_t reg;

    rc = BMXT_Open_PreOpen(pHandle, hChp, hReg, pSettings);
    if (rc) {
        return rc;
    }

    mxt = handle = *pHandle;
    for (i=0; i<BMXT_ResourceType_eMax; i++) {
        if (mxt->platform.num[i]) { break; }
    }
    if (i==BMXT_ResourceType_eMax) {
        return rc; /* fake handle: create handle, but avoid any register access */
    }

    if (BMXT_IS_45308_FAMILY()) {
        reg = BMXT_RegRead32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_INBUF_MEM0_MEM31_PWR_DN_STATUS));
        BDBG_MSG(("45308_Open: PWR_DN_STATUS: %08x", reg));

        /* CRXPT-1195: disable dynamic power down and manually enable single instance of INBUF */
        reg = BMXT_RegRead32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_INBUF_MEM_PWR_DN_GLOBAL_CTRL));
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_INBUF_MEM_PWR_DN_GLOBAL_CTRL, INBUF_MEM_DYNAMIC_PWR_DN_EN, 0);
        BMXT_RegWrite32(mxt, BMXT_R(BCHP_DEMOD_XPT_FE_INBUF_MEM_PWR_DN_GLOBAL_CTRL), reg);
        BMXT_P_Inbuf_Pwr_Ctrl(mxt, false);
    }

    BMXT_Open_PostOpen(mxt);
    return rc;
}

void BMXT_Close(BMXT_Handle handle)
{
    unsigned i;
    BMXT_ParserConfig pbConfig;
    uint32_t addr, reg;
    BDBG_ASSERT(handle);

    for (i=0; i<handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]; i++) {
        BMXT_GetParserConfig(handle, i, &pbConfig);
        if (pbConfig.Enable) {
            pbConfig.Enable = false;
            BMXT_SetParserConfig(handle, i, &pbConfig);
        }
    }

    for (i=0; i<handle->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]; i++) {
        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_CTRL1) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));
        reg = BMXT_RegRead32(handle, addr);
        BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, TX_ENABLE, 0);
        BMXT_RegWrite32(handle, addr, reg);
    }

#if 0 /* prints unnecessary warnings on s3 standby */
    if (handle->settings.enablePidFiltering) {
        for (i=0; i<BMXT_MAX_NUM_PIDCHANNELS; i++) {
            addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*i);
            reg = BMXT_RegRead32(handle, addr);
            if (BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE)) {
                BDBG_WRN(("BMXT_Close: pidchannel %u left enabled", i));
            }
        }
    }
#endif

#ifdef BCHP_PWR_RESOURCE_XPT_DEMOD
    if (handle->platform.type==BMXT_P_PlatformType_eReg) {
        BCHP_PWR_ReleaseResource(handle->hChp, BCHP_PWR_RESOURCE_XPT_DEMOD);
    }
#endif

    BKNI_Free(handle);
}

BERR_Code BMXT_GetMtsifStatus(BMXT_Handle handle, BMXT_MtsifStatus *pStatus)
{
    unsigned i;
    uint32_t addr, val;
    static const unsigned SECRET_WORD = 0x829eecde;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BMXT_MtsifStatus));

    for (i=0; i<handle->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]; i++) {
        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_CTRL1) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));
        val = BMXT_RegRead32(handle, addr);
        pStatus->tx[i].interfaceWidth = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, MTSIF_TX_IF_WIDTH);
        pStatus->tx[i].clockPolarity = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, MTSIF_TX_CLOCK_POL_SEL);
        pStatus->tx[i].enabled = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_CTRL1, TX_ENABLE);
        if (!(BMXT_IS_3128_FAMILY() || BMXT_IS_4538_FAMILY())) {
            addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_STATUS) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));
            val = BMXT_RegRead32(handle, addr);
            pStatus->tx[i].encrypted = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_STATUS, MTSIF_TX_ENCRYPTION_ENABLE_STATUS);
        }
        else {
            /* the bit field doesn't exist, so take a best guess */
            addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_SECRET_WORD) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1));
            val = BMXT_RegRead32(handle, addr);
            pStatus->tx[i].encrypted = (val!=SECRET_WORD);
        }
    }
    for (i=0; i<handle->platform.num[BMXT_RESOURCE_MTSIF_RX0_CTRL1]; i++) {
        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_RX0_CTRL1) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_RX0_CTRL1));
        val = BMXT_RegRead32(handle, addr);
        pStatus->rx[i].interfaceWidth = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_IF_WIDTH);
        pStatus->rx[i].clockPolarity = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_CLOCK_POL_SEL);
        pStatus->rx[i].enabled = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, PARSER_ENABLE);
        if (!(BMXT_IS_3128_FAMILY() || BMXT_IS_4538_FAMILY())) {
            pStatus->rx[i].encrypted = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_RX0_CTRL1, MTSIF_RX_DECRYPTION_ENABLE_STATUS);
        }
        else {
            /* the bit field doesn't exist, so take a best guess */
            addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_RX0_SECRET_WORD) + (i*BMXT_STEP(BMXT_RESOURCE_MTSIF_RX0_CTRL1));
            val = BMXT_RegRead32(handle, addr);
            pStatus->rx[i].encrypted = (val!=SECRET_WORD);
        }
    }

    return BERR_SUCCESS;
}

BERR_Code BMXT_GetInputBandStatus(BMXT_Handle handle, BMXT_InputBandStatus *pStatus)
{
    unsigned i;
    uint32_t addr;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pStatus);

    BKNI_Memset(pStatus, 0, sizeof(BMXT_InputBandStatus));

    for (i=0; i<handle->platform.num[BMXT_RESOURCE_IB0_CTRL]; i++) {
        addr = BMXT_R(BCHP_DEMOD_XPT_FE_IB0_SYNC_COUNT) + (i*BMXT_STEP(BMXT_RESOURCE_IB0_CTRL));
        pStatus->syncCount[i] = BMXT_RegRead32(handle, addr);
    }

    return BERR_SUCCESS;
}

static BERR_Code BMXT_P_GetMtsifTxSelect(BMXT_Handle handle, unsigned parserNum, unsigned *mtsifTxNum)
{
    uint32_t reg, addr;
    if (!BMXT_HAS_PID_TABLE()) {
        *mtsifTxNum = 0;
        goto done;
    }
    if (handle->settings.enablePidFiltering) {
        *mtsifTxNum = handle->mtsifTxSel[parserNum];
        goto done;
    }

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*parserNum);
    reg = BMXT_RegRead32(handle, addr);
    if (BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT)!=parserNum) {
        BDBG_ERR(("Unable to get MTSIF_TX selection (PID_TABLE[%u])", parserNum));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    reg = PARSER_OUTPUT_PIPE_SEL_TO_INDEX(BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL));
    if (reg >= handle->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]) {
        BDBG_ERR(("Unexpected MTSIF_TX selection %u (PID_TABLE[%u])", reg, parserNum));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    *mtsifTxNum = reg;
done:
    return BERR_SUCCESS;
}

static BERR_Code BMXT_P_SetMtsifTxSelect(BMXT_Handle handle, unsigned parserNum, unsigned mtsifTxNum)
{
    uint32_t reg, addr;

    if (!BMXT_HAS_PID_TABLE()) {
        goto done;
    }
    handle->mtsifTxSel[parserNum] = mtsifTxNum;
    if (handle->settings.enablePidFiltering) {
        goto done;
    }

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*parserNum);
    reg = BMXT_RegRead32(handle, addr);
    if (BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT)!=parserNum) {
        /* assume that pid channel x and parser x have the same number. if not, we could traverse through
           the pid table and find out but it's simpler just to enforce this assumption */
        BDBG_ERR(("Unable to set MTSIF_TX selection (PID_TABLE[%u])", parserNum));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, PARSER_OUTPUT_PIPE_SEL_FROM_INDEX(mtsifTxNum));
    BMXT_RegWrite32(handle, addr, reg);
done:
    return BERR_SUCCESS;
}

unsigned BMXT_P_GetVirtualParserNum(BMXT_Handle handle, unsigned mtsifTxSelect, unsigned parserNum)
{
    uint32_t addr, val;
    unsigned virtualParserNum;
    BDBG_ASSERT(handle->platform.regoffsets[BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID] != BMXT_NOREG);

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID) + 4*(parserNum/4) + (BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1)*mtsifTxSelect);
    val = BMXT_RegRead32(handle, addr);

    switch (parserNum%4) {
        case 0: virtualParserNum = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND0_BAND_ID); break;
        case 1: virtualParserNum = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND1_BAND_ID); break;
        case 2: virtualParserNum = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND2_BAND_ID); break;
        case 3: virtualParserNum = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND3_BAND_ID); break;
    }
    return virtualParserNum;
}

void BMXT_P_SetVirtualParserNum(BMXT_Handle handle, unsigned mtsifTxSelect, unsigned parserNum, unsigned virtualParserNum)
{
    uint32_t addr, val;

    BDBG_ASSERT(handle->platform.regoffsets[BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID] != BMXT_NOREG);

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID) + 4*(parserNum/4) + (BMXT_STEP(BMXT_RESOURCE_MTSIF_TX0_CTRL1)*mtsifTxSelect);
    val = BMXT_RegRead32(handle, addr);
    switch (parserNum%4) {
        case 0: BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND0_BAND_ID, virtualParserNum); break;
        case 1: BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND1_BAND_ID, virtualParserNum); break;
        case 2: BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND2_BAND_ID, virtualParserNum); break;
        case 3: BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID, MTSIF_BAND3_BAND_ID, virtualParserNum); break;
    }
    BMXT_RegWrite32(handle, addr, val);
    return;
}

#define BMXT_USE_PARSER_VERSION 1

void BMXT_P_ParserVersion(BMXT_Handle handle, unsigned parserNum)
{
    uint32_t addr, val;
    unsigned version;
    if (BMXT_USE_PARSER_VERSION && BMXT_IS_45308_FAMILY()) {
        addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
        val = BMXT_RegRead32(handle, addr);
        version = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, PARSER_VERSION);
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, PARSER_VERSION, ++version);
        BMXT_RegWrite32(handle, addr, val);
        BDBG_MSG(("%u: PARSER_VERSION %u", parserNum, version));
    }
}

#define PARSER_INPUT_SEL_MASK (BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1_PARSER_INPUT_SEL_MASK >> BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1_PARSER_INPUT_SEL_SHIFT)
#define LEGACY_NUM_REMAP_PB 9 /* number of remappable parsers, where MTSIF_TX-level remapping is not available */

#define USE_PARSER_ENABLE_WORKAROUND 0 /* disable, since it causes issues with DCBG */

void BMXT_P_SetParserEnable(BMXT_Handle handle, unsigned parserNum, bool enable)
{
    uint32_t addr, val;
    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    val = BMXT_RegRead32(handle, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE, enable ? 1 : 0);

#if USE_PARSER_ENABLE_WORKAROUND
    if ((handle->settings.chip==BMXT_Chip_e45308) && (handle->settings.chipRev<BMXT_ChipRev_eC0)) {
        BMXT_P_ParserVersion(handle, parserNum); /* if PARSER_ENABLE = 0 before changing any settings is not possible, bump PARSER_VERSION prior to any changes */
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE, 1); /* never disable */
        BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB, enable ? 0 : 1);
    }
#endif
    BMXT_RegWrite32(handle, addr, val);
}

BERR_Code BMXT_GetParserConfig(BMXT_Handle handle, unsigned parserNum, BMXT_ParserConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (parserNum >= handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]) {
        BDBG_ERR(("GetParserConfig%u: parserNum out of range!", parserNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_Memset(pConfig, 0, sizeof(BMXT_ParserConfig));

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    Reg = BMXT_RegRead32(handle, RegAddr);

    pConfig->ErrorInputIgnore = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE);
    pConfig->TsMode = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE);
    pConfig->AcceptNulls = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD);
    pConfig->AllPass = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD);
    pConfig->DssMode = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PACKET_TYPE) ? true : false;
    pConfig->InputBandNumber = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL) +
        (BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB) ? (PARSER_INPUT_SEL_MASK+1) : 0);
    pConfig->Enable = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE);

#if USE_PARSER_ENABLE_WORKAROUND
    if ((handle->settings.chip==BMXT_Chip_e45308) && (handle->settings.chipRev<BMXT_ChipRev_eC0)) {
        if (BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB)) {
            pConfig->InputBandNumber = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL);
            pConfig->Enable = false;
        }
    }
#endif

    if (BMXT_IS_45308_FAMILY()) {
        RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
        Reg = BMXT_RegRead32(handle, RegAddr);
        pConfig->streamIdFilterEn = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, STREAM_ID_FILT_EN);
        pConfig->streamId = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, STREAM_ID);
    }

    /* defaults, before they're overridden below */
    pConfig->mtsifTxSelect = 0;
    pConfig->virtualParserNum = parserNum;

    /* 4538 and later */
    if (handle->platform.regoffsets[BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID] != BMXT_NOREG)
    {
        /* for MTSIF_TX-level mapping, query PID_TABLE to get correct MTSIF_TX first, then query BAND_ID */
        rc = BMXT_P_GetMtsifTxSelect(handle, parserNum, &pConfig->mtsifTxSelect);
        if (rc) {
            BDBG_ERR(("GetParserConfig%u: BMXT_P_GetMtsifTxSelect failure", parserNum));
            goto done;
        }
        pConfig->virtualParserNum = BMXT_P_GetVirtualParserNum(handle, pConfig->mtsifTxSelect, parserNum);
    }
    else { /* legacy */
        /* for PARSER-level mapping, query BAND_ID first, then query the correct PID_TABLE entry */
        if (parserNum >= LEGACY_NUM_REMAP_PB) {
            pConfig->virtualParserNum = parserNum;
        }
        else
        {
            RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID) + 4*(parserNum/4);
            Reg = BMXT_RegRead32(handle, RegAddr);
            switch (parserNum%4) {
                case 0: pConfig->virtualParserNum = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_BAND_ID); break;
                case 1: pConfig->virtualParserNum = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_BAND_ID); break;
                case 2: pConfig->virtualParserNum = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_BAND_ID); break;
                case 3: pConfig->virtualParserNum = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_BAND_ID); break;
            }
        }

        #if 1
        /* for PARSER-level mapping, there are some complications when used in conjunction with MTSIF_TX selection, so we limit to TX0.
           see SW7346-1802 */
        pConfig->mtsifTxSelect = 0;
        BSTD_UNUSED(BMXT_P_GetMtsifTxSelect);
        BSTD_UNUSED(BMXT_P_SetMtsifTxSelect);
        goto done;
        #else
        /* for PARSER-level BAND_ID mapping, we must get/set virtualParserNum's PID_TABLE entry, not the physical parser's */
        rc = BMXT_P_GetMtsifTxSelect(handle, pConfig->virtualParserNum, &pConfig->mtsifTxSelect);
        if (rc) {
            BDBG_ERR(("GetParserConfig%u: BMXT_P_GetMtsifTxSelect failure", parserNum));
            goto done;
        }
        #endif
    }

done:
    BDBG_MSG(("GetParserConfig%u: enable%u, IB%2u, BAND_ID%2u, MTSIF%u, DSS%u", parserNum, pConfig->Enable, pConfig->InputBandNumber, pConfig->virtualParserNum, (rc==BERR_SUCCESS) ? pConfig->mtsifTxSelect : (unsigned)-1, pConfig->DssMode));
    return rc;
}

BERR_Code BMXT_SetParserConfig(BMXT_Handle handle, unsigned parserNum, const BMXT_ParserConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;
    bool wasEnabled;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (parserNum >= handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]) {
        BDBG_ERR(("SetParserConfig%u: parserNum out of range!", parserNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (pConfig->mtsifTxSelect >= handle->platform.num[BMXT_RESOURCE_MTSIF_TX0_CTRL1]) {
        BDBG_ERR(("SetParserConfig%u: MTSIF_TX %u is out of range!", parserNum, pConfig->mtsifTxSelect));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    if (pConfig->virtualParserNum >= BMXT_MAX_NUM_PARSERS) { /* sanity check */
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto done;
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    Reg = BMXT_RegRead32(handle, RegAddr);
    wasEnabled = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ENABLE);
    BMXT_P_SetParserEnable(handle, parserNum, false); /* disable parser before changing any settings */

    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ERROR_INPUT_TEI_IGNORE, pConfig->ErrorInputIgnore);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TIMESTAMP_MODE, pConfig->TsMode);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, pConfig->AcceptNulls);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ALL_PASS_CTRL_PRE_MPOD, pConfig->AllPass);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PKT_LENGTH, pConfig->DssMode ? 130 : 188);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_PACKET_TYPE, pConfig->DssMode ? 1 : 0);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB, (pConfig->InputBandNumber > PARSER_INPUT_SEL_MASK) ? 1 : 0);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL, (pConfig->InputBandNumber & PARSER_INPUT_SEL_MASK));
    BMXT_RegWrite32(handle, RegAddr, Reg);

    if (BMXT_IS_45308_FAMILY()) {
        RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
        Reg = BMXT_RegRead32(handle, RegAddr);
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, STREAM_ID_FILT_EN, pConfig->streamIdFilterEn);
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, STREAM_ID, pConfig->streamId);
        BMXT_RegWrite32(handle, RegAddr, Reg);
    }

    switch(pConfig->mtsifTxSelect) {
        case 0:
#ifdef BCHP_PWR_RESOURCE_MTSIF_TX0
            if (handle->platform.type==BMXT_P_PlatformType_eReg) {
                if (pConfig->Enable && !wasEnabled) {
                    BCHP_PWR_AcquireResource(handle->hChp, BCHP_PWR_RESOURCE_MTSIF_TX0);
                }
                else if (!pConfig->Enable && wasEnabled) {
                    BCHP_PWR_ReleaseResource(handle->hChp, BCHP_PWR_RESOURCE_MTSIF_TX0);
                }
            }
#endif
            break;
        case 1:
#ifdef BCHP_PWR_RESOURCE_MTSIF_TX1
            if (handle->platform.type==BMXT_P_PlatformType_eReg) {
                if (pConfig->Enable && !wasEnabled) {
                    BCHP_PWR_AcquireResource(handle->hChp, BCHP_PWR_RESOURCE_MTSIF_TX1);
                }
                else if (!pConfig->Enable && wasEnabled) {
                    BCHP_PWR_ReleaseResource(handle->hChp, BCHP_PWR_RESOURCE_MTSIF_TX1);
                }
            }
#endif
            break;
        default:
            break;
    }

    /* 4538 and later */
    if (handle->platform.regoffsets[BCHP_DEMOD_XPT_FE_MTSIF_TX0_BAND0_BAND3_ID] != BMXT_NOREG)
    {
        rc = BMXT_P_SetMtsifTxSelect(handle, parserNum, pConfig->mtsifTxSelect);
        if (rc) {
            BDBG_ERR(("SetParserConfig%u: BMXT_P_SetMtsifTxSelect failure", parserNum));
            goto done;
        }

        BMXT_P_SetVirtualParserNum(handle, pConfig->mtsifTxSelect, parserNum, pConfig->virtualParserNum);
        BDBG_MSG(("BAND_ID mapping: MTSIF_TX: TX%u, parser%u", pConfig->mtsifTxSelect, parserNum));
    }
    else {
        if (parserNum >= LEGACY_NUM_REMAP_PB) {
            if (parserNum!=pConfig->virtualParserNum) {
                BDBG_WRN(("SetParserConfig%u: parser is not a remappable parser (%u)", parserNum, pConfig->virtualParserNum));
                goto post_map;
            }
        }
        if (pConfig->virtualParserNum >= BMXT_P_LEGACY_PARSER_BAND_ID_MAX) {
            BDBG_WRN(("SetParserConfig%u: parser cannot be remapped to parser %u >= %u", parserNum, pConfig->virtualParserNum, BMXT_P_LEGACY_PARSER_BAND_ID_MAX));
            goto post_map;
        }

        RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID) + 4*(parserNum/4);
        Reg = BMXT_RegRead32(handle, RegAddr);
        switch (parserNum%4) {
            case 0: BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER0_BAND_ID, pConfig->virtualParserNum); break;
            case 1: BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER1_BAND_ID, pConfig->virtualParserNum); break;
            case 2: BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER2_BAND_ID, pConfig->virtualParserNum); break;
            case 3: BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TO_PARSER3_BAND_ID, PARSER3_BAND_ID, pConfig->virtualParserNum); break;
        }
        BMXT_RegWrite32(handle, RegAddr, Reg);
        BDBG_MSG(("BAND_ID mapping: PARSER: parser%u, addr 0x%08x", parserNum, RegAddr));

        #if 1
        if (pConfig->mtsifTxSelect!=0) {
            BDBG_ERR(("SetParserConfig%u: MTSIF_TX selection change not supported on this chip", parserNum));
            rc = BERR_TRACE(BERR_NOT_SUPPORTED);
        }
        #else
        /* for PARSER-level BAND_ID mapping, we must get/set virtualParserNum's PID_TABLE entry, not the physical parser's */
        rc = BMXT_P_SetMtsifTxSelect(handle, pConfig->virtualParserNum, pConfig->mtsifTxSelect);
        if (rc) {
            BDBG_ERR(("SetParserConfig%u: BMXT_P_SetMtsifTxSelect failure", parserNum));
            goto done;
        }
        #endif
    }

#if 0 /* debug dump */
{
    unsigned i;
    for (i=0; i<handle->num.pb; i++) {
        Reg = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*i));
        BDBG_MSG(("PID_TABLE[%2u] %08x, MTSIF_TX%u, PB%2u, EN%u", i, Reg,
            PARSER_OUTPUT_PIPE_SEL_TO_INDEX(BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL)),
            BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT),
            BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE) ));
    }
}
#endif

post_map:
    /* don't restamp on master parserband in daisy-chain config */
    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL2, PARSER_TIMESTAMP_RESTAMP, pConfig->passthrough ? 0 : 1); /* reset value is 1 */
    BMXT_RegWrite32(handle, RegAddr, Reg);

    /* channel start or new settings while channel was running */
    if (pConfig->Enable) {
        BMXT_P_ParserVersion(handle, parserNum);
    }

    /* PARSER_ENABLE always done last */
    BMXT_P_SetParserEnable(handle, parserNum, pConfig->Enable);
    if (!pConfig->Enable) {
        /* disable TB-functionality */
        if (parserNum < handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1]) {
            RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1) + parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1);
            Reg = BMXT_RegRead32(handle, RegAddr);
            BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_ENABLE, 0);
            BMXT_RegWrite32(handle, RegAddr, Reg);
        }
    }

    /* channel stop */
    if (!pConfig->Enable && wasEnabled) {
        BMXT_P_ParserVersion(handle, parserNum);
    }

done:
    BDBG_MSG(("SetParserConfig%u: enable%u, IB%2u, BAND_ID%2u, MTSIF%u, DSS%u", parserNum, pConfig->Enable, pConfig->InputBandNumber, pConfig->virtualParserNum, pConfig->mtsifTxSelect, pConfig->DssMode));
    return rc;
}

BERR_Code BMXT_GetParserMapping(BMXT_Handle handle, BMXT_ParserBandMapping *pMapping)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pMapping);
    return BERR_TRACE(BERR_NOT_SUPPORTED); /* DEPRECATED */
}

BERR_Code BMXT_SetParserMapping(BMXT_Handle handle, const BMXT_ParserBandMapping *pMapping)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pMapping);
    return BERR_TRACE(BERR_NOT_SUPPORTED); /* DEPRECATED */
}

BERR_Code BMXT_GetInputBandConfig(BMXT_Handle handle, unsigned bandNum, BMXT_InputBandConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BDBG_ASSERT(pConfig);

    if (bandNum>=handle->platform.num[BMXT_RESOURCE_IB0_CTRL]) {
        BDBG_ERR(("BandNum %u is out of range!", bandNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    BKNI_Memset(pConfig, 0, sizeof(BMXT_InputBandConfig));
    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_IB0_CTRL) + (bandNum * BMXT_STEP(BMXT_RESOURCE_IB0_CTRL));
    Reg = BMXT_RegRead32(handle, RegAddr);
    pConfig->DssMode = (BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, IB_PKT_LENGTH) == 130);
    pConfig->ParallelInputSel = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL);
    if (BMXT_IS_45308_FAMILY()) {
        pConfig->streamIdExtractEn = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, STREAM_ID_EXTRACT_EN);
        pConfig->packetEndDetectEn = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, PKT_END_DETECT_EN);
    }

    return BERR_SUCCESS;
}

BERR_Code BMXT_SetInputBandConfig(BMXT_Handle handle, unsigned bandNum, const BMXT_InputBandConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BDBG_ASSERT(pConfig);

    if (bandNum>=handle->platform.num[BMXT_RESOURCE_IB0_CTRL]) {
        BDBG_ERR(("BandNum %u is out of range!", bandNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_IB0_CTRL) + (bandNum * BMXT_STEP(BMXT_RESOURCE_IB0_CTRL));
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, IB_PKT_LENGTH, pConfig->DssMode ? 130 : 188);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, IB_PARALLEL_INPUT_SEL, pConfig->ParallelInputSel ? 1 : 0);
    if (BMXT_IS_45308_FAMILY()) {
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, STREAM_ID_EXTRACT_EN, pConfig->streamIdExtractEn ? 1 : 0);
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, PKT_END_DETECT_EN, pConfig->packetEndDetectEn ? 1 : 0);
    }

    /* SWSTB-1353: C7 syncbyte corruption workaround */
    switch (handle->settings.chip) {
        case BMXT_Chip_e7366:
        case BMXT_Chip_e7364:
        case BMXT_Chip_e45216:
        case BMXT_Chip_e45308:
        case BMXT_Chip_e4538:
        case BMXT_Chip_e4548:
            BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_IB0_CTRL, IB_ERROR_INPUT_EN, 1); break;
        default: break;
    }

    BMXT_RegWrite32(handle, RegAddr, Reg);

    return BERR_SUCCESS;
}

uint32_t BMXT_ReadIntStatusRegister(BMXT_Handle handle, BMXT_IntReg intReg)
{
    uint32_t reg = 0, val = 0;

    switch (intReg) {
        case BMXT_IntReg_eFE_INTR_STATUS0:
            reg = BMXT_R(BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG);
            break;
        case BMXT_IntReg_eFE_INTR_STATUS1:
            reg = BMXT_R(BCHP_DEMOD_XPT_FE_INTR_STATUS1_REG);
            break;
        default:
            return BERR_TRACE(BERR_INVALID_PARAMETER);
            break;
    }

    if (BMXT_EXIST(BCHP_DEMOD_XPT_FE_INTR_STATUS0_REG)) {
        val = BMXT_RegRead32(handle, reg);
        BMXT_RegWrite32(handle, reg, 0);
    }
    else {
        val = 0;
    }
    return val;
}

void BMXT_ConfigPidChannel(BMXT_Handle handle, unsigned index, const BMXT_PidChannelSettings *pidChannelSettings)
{
    uint32_t addr, reg;
    if (index >= BMXT_MAX_NUM_PIDCHANNELS) {
        BERR_TRACE(BERR_INVALID_PARAMETER);
        return;
    }
    if (handle->settings.enablePidFiltering==false) {
        BDBG_ERR(("MXT not configured for pid filtering"));
        return;
    }
    if (!BMXT_HAS_PID_TABLE()) {
        BERR_TRACE(BERR_NOT_SUPPORTED);
        return;
    }

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_PID_TABLE_i_ARRAY_BASE) + (4*index);
    reg = BMXT_RegRead32(handle, addr);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, SECURITY_OUTPUT_PIPE_SEL, 0);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, IGNORE_PID_VERSION, 1); /* hard-coded for now */
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PLAYBACK_FE_SEL, 0);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PARSER_OUTPUT_PIPE_SEL, pidChannelSettings->mtsifTxSelect);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, INPUT_BAND_PARSER_PID_CHANNEL_INPUT_SELECT, pidChannelSettings->inputSelect);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, MPOD_BYPASS_EN, 0);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, PID_CHANNEL_ENABLE, pidChannelSettings->enable ? 1:0);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, ENABLE_HD_FILTER, 0);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_PID_TABLE_i, HD_FILT_DIS_PID_CHANNEL_PID, pidChannelSettings->pid);
    BMXT_RegWrite32(handle, addr, reg);
}

/****************************************************************
TSMF APIs
****************************************************************/

BERR_Code BMXT_TSMF_GetFldVerifyConfig(BMXT_Handle handle, unsigned tsmfNum, BMXT_TSMFFldVerifyConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;

    if (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]) {
        BDBG_ERR(("TSMFNum %u is out of range!", tsmfNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_CTRL) + tsmfNum * BMXT_STEP(BMXT_RESOURCE_TSMF0_CTRL);
    Reg = BMXT_RegRead32(handle, RegAddr);
    pConfig->CrcChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_CRC_CK_DIS);
    pConfig->RelTsStatusChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_STATUS_CK_DIS);
    pConfig->FrameTypeChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_FRAME_TYPE_CK_DIS);
    pConfig->RelTsModeChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_MODE_CK_DIS);
    pConfig->SyncChkFDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_SYNC_CK_DIS);
    pConfig->CCCkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_CC_CK_DIS);
    pConfig->AdapChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_ADAP_CK_DIS);
    pConfig->ScChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_SC_CK_DIS);
    pConfig->TsPriorChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_TS_PRIOR_CK_DIS);
    pConfig->PusiChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_PUSI_CK_DIS);
    pConfig->TeiChkDis = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_TEI_CK_DIS);
    pConfig->VersionChgMode = BCHP_GET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_VER_CHANGE_MODE);

    return rc;
}

BERR_Code BMXT_TSMF_SetFldVerifyConfig(BMXT_Handle handle, unsigned tsmfNum, const BMXT_TSMFFldVerifyConfig *pConfig)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;

    if (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]) {
        BDBG_ERR(("TSMFNum %u is out of range!", tsmfNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_CTRL) + tsmfNum * BMXT_STEP(BMXT_RESOURCE_TSMF0_CTRL);
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_CRC_CK_DIS, pConfig->CrcChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_STATUS_CK_DIS, pConfig->RelTsStatusChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_FRAME_TYPE_CK_DIS, pConfig->FrameTypeChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_MODE_CK_DIS, pConfig->RelTsModeChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_SYNC_CK_DIS, pConfig->SyncChkFDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_CC_CK_DIS, pConfig->CCCkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_ADAP_CK_DIS, pConfig->AdapChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_SC_CK_DIS, pConfig->ScChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_TS_PRIOR_CK_DIS, pConfig->TsPriorChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_PUSI_CK_DIS, pConfig->PusiChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_TEI_CK_DIS, pConfig->TeiChkDis);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_VER_CHANGE_MODE, pConfig->VersionChgMode);
    BMXT_RegWrite32(handle, RegAddr, Reg);

    return rc;
}

BERR_Code BMXT_TSMF_EnableAutoMode(BMXT_Handle handle, BMXT_TSMFInputSel input, unsigned tsmfNum, uint8_t relativeTsNum)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_MSG(("TSMF_EnableAutoMode: IB%u -> TSMF%u, rel%u", input, tsmfNum, relativeTsNum));
    if (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]) {
        BDBG_ERR(("TSMFNum %u is out of range!", tsmfNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_CTRL) + tsmfNum * BMXT_STEP(BMXT_RESOURCE_TSMF0_CTRL);
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_NO, relativeTsNum);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_INPUT_SEL, input);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_AUTO_EN, 1);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_ENABLE, 1);
    BMXT_RegWrite32(handle, RegAddr, Reg);

    return rc;
}

BERR_Code BMXT_TSMF_EnableSemiAutoMode(BMXT_Handle handle, BMXT_TSMFInputSel input, unsigned tsmfNum, uint32_t slotMapLo, uint32_t slotMapHi, uint8_t relativeTsNum)
{
    uint32_t Reg, RegAddr, RegOffset;
    BERR_Code rc = BERR_SUCCESS;
    if (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]) {
        BDBG_ERR(("TSMFNum %u is out of range!", tsmfNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }
    BDBG_MSG(("TSMF_EnableSemiAutoMode: IB%u -> TSMF%u, slot%u:%u, rel%u", input, tsmfNum, slotMapLo, slotMapHi, relativeTsNum));
    RegOffset = tsmfNum * BMXT_STEP(BMXT_RESOURCE_TSMF0_CTRL);

    BMXT_RegWrite32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_LO) + RegOffset, slotMapLo);
    BMXT_RegWrite32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_SLOT_MAP_HI) + RegOffset, slotMapHi);

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_CTRL) + RegOffset;
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_INPUT_SEL, input);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_AUTO_EN, 0);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_ENABLE, 1);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_REL_TS_NO, relativeTsNum);
    BMXT_RegWrite32(handle, RegAddr, Reg);

    return rc;
}

BERR_Code BMXT_TSMF_DisableTsmf(BMXT_Handle handle, unsigned tsmfNum)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_MSG(("TSMF_DisableTsmf: TSMF%u", tsmfNum));
    if (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]) {
        BDBG_ERR(("TSMFNum %u is out of range!", tsmfNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_TSMF0_CTRL) + tsmfNum * BMXT_STEP(BMXT_RESOURCE_TSMF0_CTRL);
    Reg = BMXT_RegRead32(handle, RegAddr);
    BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_TSMF0_CTRL, TSMF_ENABLE, 0);
    BMXT_RegWrite32(handle, RegAddr, Reg);

    return rc;
}

BERR_Code BMXT_TSMF_SetParserConfig(BMXT_Handle handle, unsigned parserNum, unsigned tsmfNum, bool enable)
{
    uint32_t Reg, RegAddr;
    BERR_Code rc = BERR_SUCCESS;
    BDBG_MSG(("TSMF_SetParserConfig: TSMF%u -> PB%u, enable%u", tsmfNum, parserNum, enable));

    if ((parserNum>=handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1]) || (tsmfNum>=handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return rc;
    }

    RegAddr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1);
    Reg = BMXT_RegRead32(handle, RegAddr);

    if (BMXT_IS_3128_FAMILY()) { /* 3128, 3383, 4528, 4517, 3472 */
        unsigned TSMF_SEL_MASK, TSMF_SEL_SHIFT, TSMF_EN_MASK, TSMF_EN_SHIFT;
        if (handle->settings.chip==BMXT_Chip_e3128 || (handle->settings.chip==BMXT_Chip_e3383 && handle->platform.num[BMXT_RESOURCE_TSMF0_CTRL]<=2)) {
            TSMF_SEL_MASK = 0x04000000;
            TSMF_SEL_SHIFT = 26;
            TSMF_EN_MASK = 0x02000000;
            TSMF_EN_SHIFT = 25;
        }
        else {
            TSMF_SEL_MASK = 0x07000000;
            TSMF_SEL_SHIFT = 24;
            TSMF_EN_MASK = 0x00800000;
            TSMF_EN_SHIFT = 23;
        }
        Reg &= ~( TSMF_SEL_MASK | TSMF_EN_MASK );
        Reg |=(
            ((tsmfNum << TSMF_SEL_SHIFT) & TSMF_SEL_MASK) |
            ((enable << TSMF_EN_SHIFT) & TSMF_EN_MASK));
    }
    else { /* all others */
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TSMF_SEL, tsmfNum);
        BCHP_SET_FIELD_DATA(Reg, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_TSMF_EN, enable);
    }

    BMXT_RegWrite32(handle, RegAddr, Reg);
    return rc;
}

/****************************************************************
Power APIs
****************************************************************/

void BMXT_GetPowerSaverSettings(BMXT_Handle handle, BMXT_PowerSaverSettings *pSettings)
{
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    *pSettings = handle->powerSettings;
}

BERR_Code BMXT_SetPowerSaverSettings(BMXT_Handle handle, const BMXT_PowerSaverSettings *pSettings)
{
    BMXT_Chip chip;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pSettings);
    chip = handle->settings.chip;

    if (chip == BMXT_Chip_e4538) {
        BERR_Code rc;
        uint8_t buf[4];
        buf[0] = 0x4E;
        buf[1] = pSettings->disableMtsif ? 1 : 0;
        rc = BHAB_SendHabCommand(handle->hHab, buf, 3, buf, 3, true, true, 3);
        if (rc!=BERR_SUCCESS) {
            return BERR_TRACE(rc);
        }
    }

    handle->powerSettings = *pSettings;
    return BERR_SUCCESS;
}

/****************************************************************
TBG APIs
****************************************************************/

BERR_Code BMXT_Tbg_GetParserConfig(BMXT_Handle handle, unsigned parserNum, BMXT_Tbg_ParserConfig *pConfig)
{
    uint32_t addr, reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (parserNum >= handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1]) {
        BDBG_ERR(("ParserNum %u is out of range!", parserNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1) + parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1);
    reg = BMXT_RegRead32(handle, addr);
    pConfig->latchSyncCount = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, LATCH_SYNC_COUNT_INTO_TIMESTAMP);
    pConfig->primaryBandNum = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TBG_PRI_BAND_NUM);
    pConfig->enable = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_ENABLE);
    /* pid-filtering with TBG */
    pConfig->unmappedMarkerPktAcceptEn = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_UNMAPPED_MARKER_PKT_ACCEPT_EN);
    pConfig->btpGenDis = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_BTP_GEN_DIS);
    pConfig->unmappedPidChNum = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_PARSER_PID_CH_NUM);
    return BERR_SUCCESS;
}

BERR_Code BMXT_Tbg_SetParserConfig(BMXT_Handle handle, unsigned parserNum, const BMXT_Tbg_ParserConfig *pConfig)
{
    uint32_t addr, reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (parserNum >= handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1]) {
        BDBG_ERR(("ParserNum %u is out of range!", parserNum));
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1) + parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1);
    BDBG_MSG(("TBG: PB%u: latch %u, priBand %u, enable %u ", parserNum, pConfig->latchSyncCount, pConfig->primaryBandNum, pConfig->enable));
    reg = BMXT_RegRead32(handle, addr);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, LATCH_SYNC_COUNT_INTO_TIMESTAMP, pConfig->latchSyncCount);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TBG_PRI_BAND_NUM, pConfig->primaryBandNum);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_ENABLE, pConfig->enable);
    /* pid-filtering with TBG */
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_UNMAPPED_MARKER_PKT_ACCEPT_EN, pConfig->unmappedMarkerPktAcceptEn);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_BTP_GEN_DIS, pConfig->btpGenDis);
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_MINI_PID_PARSER0_TB_CTRL1, TB_PARSER_PID_CH_NUM, pConfig->unmappedPidChNum);
    BMXT_RegWrite32(handle, addr, reg);

    return BERR_SUCCESS;
}

BERR_Code BMXT_Tbg_GetGlobalConfig(BMXT_Handle handle, BMXT_Tbg_GlobalConfig *pConfig)
{
    uint32_t reg;
    BMXT_Chip chip = handle->settings.chip;
    BHAB_Handle hab = handle->hHab;
    uint32_t temp_val, val;
    uint32_t JTAG_OTP_GENERAL_STATUS_0 = 0, statusOffset = 0;
    BERR_Code rc;

    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1]==0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    reg = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL1));
    pConfig->markerTag = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_TB_GLOBAL_CTRL1, TB_MARKER_TAG);

    reg = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL2));
    pConfig->markerPidValue = BCHP_GET_FIELD_DATA(reg, DEMOD_XPT_FE_TB_GLOBAL_CTRL2, MARKER_PID_VALUE);

    pConfig->markerFeedId = 0;

    if (handle->platform.type != BMXT_P_PlatformType_eHab) {
        goto done;
    }

    switch (chip) {
        case BMXT_Chip_e4538:
            JTAG_OTP_GENERAL_STATUS_0 = 0x90814;
            statusOffset = 3;
            break;
        case BMXT_Chip_e45216:
            JTAG_OTP_GENERAL_STATUS_0 = 0x6921014;
            statusOffset = 10;
            break;
        case BMXT_Chip_e45308:
        case BMXT_Chip_e45316:
            JTAG_OTP_GENERAL_STATUS_0 = 0x7021014;
            statusOffset = 10;
            break;
        default:
            break;
    }

    switch (chip) {
        case BMXT_Chip_e4538: /* STATUS_3 */
            reg = JTAG_OTP_GENERAL_STATUS_0 + 4*statusOffset;
            rc = BHAB_ReadRegister(hab, reg, &val);
            if (rc!=BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto done;
            }
            break;
        case BMXT_Chip_e45216:
        case BMXT_Chip_e45308:
        case BMXT_Chip_e45316: /* STATUS_10 and STATUS_11 */
            reg = JTAG_OTP_GENERAL_STATUS_0 + 4*statusOffset;
            rc = BHAB_ReadRegister(hab, reg, &temp_val);
            if (rc!=BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto done;
            }
            val = (temp_val & 0x03ffffff);

            reg = JTAG_OTP_GENERAL_STATUS_0 + 4*(statusOffset+1);
            rc = BHAB_ReadRegister(hab, reg, &temp_val);
            if (rc!=BERR_SUCCESS) {
                rc = BERR_TRACE(rc);
                goto done;
            }
            val |= ((temp_val & 0x3f) << 26);
            break;
        default:
            BERR_TRACE(BERR_INVALID_PARAMETER); goto done;
    }

    pConfig->markerFeedId = val;

done:
    return BERR_SUCCESS;
}

BERR_Code BMXT_Tbg_SetGlobalConfig(BMXT_Handle handle, const BMXT_Tbg_GlobalConfig *pConfig)
{
    uint32_t reg;
    BDBG_ASSERT(handle);
    BDBG_ASSERT(pConfig);

    if (handle->platform.num[BMXT_RESOURCE_MINI_PID_PARSER0_TB_CTRL1]==0) {
        return BERR_TRACE(BERR_INVALID_PARAMETER);
    }

    reg = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL1));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_TB_GLOBAL_CTRL1, TB_MARKER_TAG, pConfig->markerTag);
    BMXT_RegWrite32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL1), reg);

    reg = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL2));
    BCHP_SET_FIELD_DATA(reg, DEMOD_XPT_FE_TB_GLOBAL_CTRL2, MARKER_PID_VALUE, pConfig->markerPidValue);
    BMXT_RegWrite32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_TB_GLOBAL_CTRL2), reg);

    return BERR_SUCCESS;
}

/* DCBG helper */
unsigned BMXT_P_GetParserInputBand(BMXT_Handle handle, unsigned parserNum)
{
    uint32_t val, addr, rc;
    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    val = BMXT_RegRead32(handle, addr);
#if USE_PARSER_ENABLE_WORKAROUND
    if ((handle->settings.chip==BMXT_Chip_e45308) && (handle->settings.chipRev<BMXT_ChipRev_eC0)) {
        rc = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL);
    }
    else
#endif
    {
        rc = BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL) +
        (BCHP_GET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_INPUT_SEL_MSB) ? (PARSER_INPUT_SEL_MASK+1) : 0);
    }
    return rc;
}

void BMXT_P_SetParserAcceptNull(BMXT_Handle handle, unsigned parserNum, bool enable)
{
    uint32_t val, addr;
    addr = BMXT_R(BCHP_DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1) + (parserNum * BMXT_STEP(BMXT_RESOURCE_MINI_PID_PARSER0_CTRL1));
    val = BMXT_RegRead32(handle, addr);
    BCHP_SET_FIELD_DATA(val, DEMOD_XPT_FE_MINI_PID_PARSER0_CTRL1, PARSER_ACCEPT_NULL_PKT_PRE_MPOD, enable ? 1 : 0);
    BMXT_RegWrite32(handle, addr, val);
    return;
}

void BMXT_P_RegDump(BMXT_Handle handle)
{
#if 0 /* generic, simple for-loop */
    uint32_t i;
    uint32_t start = BMXT_R(BCHP_DEMOD_XPT_FE_FE_CTRL);
    uint32_t end = BMXT_R(BCHP_DEMOD_XPT_FE_SPID_TABLE_i_ARRAY_BASE); /* print upto, but not including SPID table */

    for (i=start; i<end; i+=4) {
        BKNI_Printf("%08x = %08x\n", i, BMXT_RegRead32(handle, i));
    }
#elif 1 /* chip-specific, manual printout */
    BMXT_P_RegDump_Fe(handle);
    BMXT_P_RegDump_Dcbg(handle);
    BMXT_P_RegDump_L2Intr(handle);
#else
    BMXT_P_RegDump_Quick(handle);
#endif
    handle->dumpIndex++;
}

#if 0
void BMXT_P_Debug(BMXT_Handle handle)
{
    uint32_t val;
    val = BMXT_RegRead32(handle, BMXT_R(BCHP_DEMOD_XPT_FE_MTSIF_RX0_PKT_BAND0_BAND31_DETECT));
    BDBG_WRN(("DEBUG: %08x", val));
}
#endif
