/***************************************************************************
 *  Copyright (C) 2016 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 *****************************************************************/
#include "nexus_transport_module.h"
#include "bchp_xpt_fe.h"
#if NEXUS_TRANSPORT_EXTENSION_TSMF
#include "priv/nexus_tsmf_priv.h"
#endif
#include "priv/nexus_transport_priv.h"

#if BXPT_HAS_XCBUF
#include "bxpt_xcbuf.h"
#endif
#if BXPT_HAS_RSBUF
#include "bxpt_rsbuf.h"
#endif

BDBG_MODULE(nexus_parser_band);
BDBG_FILE_MODULE(nexus_flow_parser_band);

/* when we receive a parser band CC error interrupt, we can look up which pid channel had the error.
this is not exact, but the approximate count is useful. */
void NEXUS_ParserBand_P_CountCcErrors_isr(void)
{
    NEXUS_P_HwPidChannel *pidChannel;
    unsigned i;

#if NEXUS_PARSER_BAND_CC_CHECK
    uint32_t value[NEXUS_NUM_PID_CHANNELS/32];

    /* read & clear status immediately to maximize chance of catching the next one */
    for (i=0;i<NEXUS_NUM_PID_CHANNELS/32;i++) {
        uint32_t addr = BCHP_XPT_FE_PCC_ERROR0 + (i*4);
        value[i] = BREG_Read32(g_pCoreHandles->reg, addr);
        if (value[i]) {
            BREG_Write32(g_pCoreHandles->reg, addr, 0); /* clear status immediately to maximize chance of catching the next one */
        }
    }

    /* now apply to each pid channel */
    for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link)) {
        unsigned index = pidChannel->status.pidChannelIndex;
        if ((value[index/32]>>(index%32)) & 0x1) {
            pidChannel->status.continuityCountErrors++;
        }
    }
#else
    bool value[NEXUS_NUM_PID_CHANNELS];
    uint32_t Reg;

    /* read & clear status immediately to maximize chance of catching the next one */
    for (i = 0; i < NEXUS_NUM_PID_CHANNELS; i++)
    {
        uint32_t addr = BCHP_XPT_FULL_PID_PARSER_STATE_CONFIG_0_i_ARRAY_BASE + (i*4);

        Reg = BREG_Read32(g_pCoreHandles->reg, addr);
        value[ i ] = BCHP_GET_FIELD_DATA( Reg, XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PCC_ERROR ) ? true : false;
        if (value[i])
        {
            Reg &= ~BCHP_MASK( XPT_FULL_PID_PARSER_STATE_CONFIG_0_i, PCC_ERROR );
            BREG_Write32(g_pCoreHandles->reg, addr, Reg); /* clear status immediately to maximize chance of catching the next one */
        }
    }

    /* now apply to each pid channel */
    for (pidChannel = BLST_S_FIRST(&pTransport->pidChannels); pidChannel; pidChannel = BLST_S_NEXT(pidChannel, link))
    {
        unsigned index = pidChannel->status.pidChannelIndex;

        if ( value[index] )
        {
            pidChannel->status.continuityCountErrors++;
        }
    }
#endif
}

#if NEXUS_NUM_PARSER_BANDS

#if (BXPT_HAS_FIXED_RSBUF_CONFIG || BXPT_HAS_FIXED_XCBUF_CONFIG)
#define PARSER_BAND_HAS_MEMORY(i) (pTransport->settings.clientEnabled.parserBand[i].rave)
#else
#define PARSER_BAND_HAS_MEMORY(i) (true)
#endif

static NEXUS_Error NEXUS_ParserBand_P_SetInterrupts(NEXUS_ParserBandHandle parserBand, const NEXUS_ParserBandSettings *pSettings);
static NEXUS_Error NEXUS_ParserBand_P_Init(unsigned index);
static NEXUS_ParserBandHandle NEXUS_ParserBand_P_Open(unsigned index);

static int NEXUS_ParserBand_P_NextAvailable(unsigned *parserBand)
{
    unsigned i;
    for (i = 0; i < NEXUS_NUM_PARSER_BANDS; i++) {
        if (!pTransport->parserBand[i] && PARSER_BAND_HAS_MEMORY(i)) {
            *parserBand = i;
            return 0;
        }
    }
    return -1;
}

static NEXUS_ParserBandHandle NEXUS_ParserBand_P_ResolveAcquire(NEXUS_ParserBand band, bool acquire)
{
    NEXUS_ParserBandHandle out = NULL;

    if (band != NEXUS_ParserBand_eInvalid)
    {
        if (band < NEXUS_ParserBand_eMax)
        {
            unsigned index = band - NEXUS_ParserBand_e0; /* assumes continuous enums */

            if (index < NEXUS_NUM_PARSER_BANDS)
            {
                /* enum variant */
                out = pTransport->parserBand[index];
                if (!out && acquire) {
                    BDBG_MSG(("Allocating parser band %u", index));
                    out = NEXUS_ParserBand_P_Open(index);
                }
            }
            else
            {
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
            }
        }
        else if (band > NEXUS_ParserBand_eMax)
        {
            /* pointer variant */
            out = (NEXUS_ParserBandHandle)band;
            BDBG_OBJECT_ASSERT(out, NEXUS_ParserBand);
        }
        else /*if (band == NEXUS_ParserBand_eMax)*/
        {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    /* else == eInvalid -> return NULL */

    return out;
}

NEXUS_ParserBandHandle NEXUS_ParserBand_Resolve_priv(NEXUS_ParserBand band)
{
    return NEXUS_ParserBand_P_ResolveAcquire(band, true);
}

void NEXUS_ParserBand_P_GetSettings(NEXUS_ParserBandHandle band, NEXUS_ParserBandSettings *pSettings)
{
    BDBG_OBJECT_ASSERT(band, NEXUS_ParserBand);
    *pSettings = band->settings;
}

void NEXUS_ParserBand_GetSettings(NEXUS_ParserBand band, NEXUS_ParserBandSettings *pSettings)
{
    NEXUS_ParserBandHandle handle;

    handle = NEXUS_ParserBand_P_ResolveAcquire(band, false);

    if (handle) {
        NEXUS_ParserBand_P_GetSettings(handle, pSettings);
    }
    else if (band != NEXUS_ParserBand_eInvalid && band < NEXUS_ParserBand_eMax) {
        /* not opened, so return default settings */
        NEXUS_ParserBand_P_GetDefaultSettings(band - NEXUS_ParserBand_e0, pSettings);
    }
    else {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

NEXUS_Error NEXUS_ParserBand_P_SetSettings(NEXUS_ParserBandHandle parserBand, const NEXUS_ParserBandSettings *pSettings)
{
    BERR_Code rc;
    BXPT_ParserConfig parserCfg;
    unsigned prevMaxDataRate = 0;
    unsigned bandHwIndex;

    BDBG_OBJECT_ASSERT(parserBand, NEXUS_ParserBand);

    bandHwIndex = parserBand->hwIndex;

#if !NEXUS_PARSER_BAND_CC_CHECK
    if(pSettings->continuityCountEnabled != parserBand->settings.continuityCountEnabled && parserBand->pidChannels) {
        BDBG_WRN(("%u:continuityCountEnabled wouldn't get applied to aleady opened pids", bandHwIndex));
    }
#endif


    rc = BXPT_GetParserConfig(pTransport->xpt, bandHwIndex, &parserCfg);
    if (rc) {return BERR_TRACE(rc);}

    switch (pSettings->sourceType) {
    case NEXUS_ParserBandSourceType_eInputBand:
#if NEXUS_TRANSPORT_EXTENSION_TSMF
    case NEXUS_ParserBandSourceType_eTsmf: /* TSMF source requires inputband to be configured properly */
#endif
        BDBG_MODULE_MSG(nexus_flow_parser_band, ("connect PB%d to IB%lu", bandHwIndex, pSettings->sourceTypeSettings.inputBand));
        rc = BXPT_SetParserDataSource(pTransport->xpt, bandHwIndex, BXPT_DataSource_eInputBand, pSettings->sourceTypeSettings.inputBand);
        if (rc) {return BERR_TRACE(rc);}

        rc = NEXUS_InputBand_P_SetTransportType(pSettings->sourceTypeSettings.inputBand, pSettings->transportType);
        if (rc) {return BERR_TRACE(rc);}
        break;

#if BXPT_NUM_REMULTIPLEXORS
    case NEXUS_ParserBandSourceType_eRemux:
        BDBG_MODULE_MSG(nexus_flow_parser_band, ("connect PB%d to RMX%d", bandHwIndex, pSettings->sourceTypeSettings.remux->index));
        rc = BXPT_SetParserDataSource(pTransport->xpt, bandHwIndex, BXPT_DataSource_eRemuxFeedback, pSettings->sourceTypeSettings.remux->index);
        if (rc) {return BERR_TRACE(rc);}
        break;
#endif

    case NEXUS_ParserBandSourceType_eMtsif:
/* even if chip doesn't have MTSIF support, enum can be used as "no mapping" placeholder */
#if BXPT_NUM_MTSIF
        BDBG_MODULE_MSG(nexus_flow_parser_band, ("connect PB%d to mtsif %p", bandHwIndex, (void *)pSettings->sourceTypeSettings.mtsif));
#endif
        break; /* keep going with the rest of the function. the host PB settings still need to be set */

    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

#if NEXUS_PARSER_BAND_CC_CHECK
    parserCfg.ContCountIgnore = !pSettings->continuityCountEnabled;
    if (NEXUS_GetEnv("cont_count_ignore")) {
        parserCfg.ContCountIgnore = true;
    }
#endif
    parserCfg.ErrorInputIgnore = pSettings->teiIgnoreEnabled;
    parserCfg.AcceptNulls = pSettings->acceptNullPackets;
    parserCfg.TsMode = NEXUS_IS_DSS_MODE(pSettings->transportType) ? BXPT_ParserTimestampMode_eBinary : BXPT_ParserTimestampMode_eMod300;
    parserCfg.AcceptAdapt00 = pSettings->acceptAdapt00;
    parserCfg.ForceRestamping = pSettings->forceRestamping;

    rc = BXPT_SetParserConfig(pTransport->xpt, bandHwIndex, &parserCfg);
    if (rc) {return BERR_TRACE(rc);}

#if B_REFSW_DSS_SUPPORT
    rc = BXPT_DirecTv_SetParserBandMode(pTransport->xpt, bandHwIndex, NEXUS_IS_DSS_MODE(pSettings->transportType) ? BXPT_ParserMode_eDirecTv:BXPT_ParserMode_eMpeg);
    if (rc) {return BERR_TRACE(rc);}
#else
    if (NEXUS_IS_DSS_MODE(pSettings->transportType)) {
        BDBG_ERR(("DSS not supported. Compile with B_REFSW_DSS_SUPPORT=y to enable."));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#endif

    rc = NEXUS_ParserBand_P_SetInterrupts(parserBand, pSettings);
    if (rc) {return BERR_TRACE(rc);}

    prevMaxDataRate = parserBand->settings.maxDataRate;
    parserBand->settings = *pSettings;

#if NEXUS_TRANSPORT_EXTENSION_TSMF
    if (pSettings->sourceType==NEXUS_ParserBandSourceType_eTsmf)
    {
        /* now that we know which PB, complete the IB -> TSMF -> PB chain */
        NEXUS_Tsmf_SetOutput_priv(parserBand);
    }
#endif

#if (!NEXUS_HAS_LEGACY_XPT) /* 40nm platforms */
    if (prevMaxDataRate != pSettings->maxDataRate) {
        BDBG_WRN(("maxDataRate can only be changed via NEXUS_TransportModuleInternalSettings"));
        parserBand->settings.maxDataRate = prevMaxDataRate;
    }
#endif

    /* BXPT_ParserAllPassMode(false) has a side effect of restoring default RS/XC DataRate values. Therefore the order
    of the following calls is important. */
    rc = BXPT_ParserAllPassMode(pTransport->xpt, bandHwIndex, pSettings->allPass);
    if (rc) {return BERR_TRACE(rc);}

#if BXPT_HAS_XCBUF
    if(!pSettings->allPass)
    {
        rc = BXPT_Rave_SetRSXCDataRate( pTransport->rave[0].channel, BXPT_ParserType_eIb, bandHwIndex, pSettings->maxDataRate, NEXUS_IS_DSS_MODE(pSettings->transportType)?130:188);
        if (rc) {return BERR_TRACE(rc);}
    }
#endif

    /* settings.cableCard can cause enable to change */
    NEXUS_ParserBand_P_SetEnable(parserBand);

    return rc;
}

NEXUS_Error NEXUS_ParserBand_SetSettings(NEXUS_ParserBand band, const NEXUS_ParserBandSettings *pSettings)
{
    NEXUS_ParserBandHandle handle = NULL;

    handle = NEXUS_ParserBand_Resolve_priv(band);

    if (handle)
    {
        return NEXUS_ParserBand_P_SetSettings(handle, pSettings);
    }
    else
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

/*
Enable the parser or input band if there is one pid channel open
*/
void NEXUS_ParserBand_P_SetEnable(NEXUS_ParserBandHandle parserBand)
{
    BERR_Code rc;
    bool enabled;

    BDBG_OBJECT_ASSERT(parserBand, NEXUS_ParserBand);

    /* TODO: replace with enabled = (parserBand->_n_baseObject.ref_cnt > 1) and remove parserBand->refcnt */
    enabled = parserBand->refcnt?true:false;

    if (parserBand->settings.sourceType==NEXUS_ParserBandSourceType_eMtsif) {
        /* for MTSIF connections, the host PB must be disabled in order to prevent a host IB from feeding to this PB */
        enabled = false;
    }

    /* cableCard + allpass + non-Mtsif input forces a parser band on */
    if (!enabled && parserBand->settings.cableCard != NEXUS_CableCardType_eNone && parserBand->settings.allPass && parserBand->settings.sourceType!=NEXUS_ParserBandSourceType_eMtsif) {
        enabled = true;
    }

#if NEXUS_PARSER_BAND_CC_CHECK
    if (NEXUS_GetEnv("cont_count_ignore")) {
        unsigned bandHwIndex = parserBand->hwIndex;
        BXPT_ParserConfig parserCfg;
        /* SetSettings is not required to be called by the user, so set ContCountIgnore directly */
        rc = BXPT_GetParserConfig(pTransport->xpt, bandHwIndex, &parserCfg);
        if (!rc) {
            parserCfg.ContCountIgnore = true;
            (void)BXPT_SetParserConfig(pTransport->xpt, bandHwIndex, &parserCfg);
        }
    }
#endif

    /* make sure the refcnt didn't get decremented below zero */
    BDBG_ASSERT(parserBand->refcnt >= 0);
    rc = BXPT_SetParserEnable(pTransport->xpt, parserBand->hwIndex, enabled);
    if (rc) {rc=BERR_TRACE(rc);}

    parserBand->enabled = enabled;

    /* clear accumulated rsbuff/xcbuff overflow errors.
       use refcnt==0 instead of enabled, because host PB is disabled when using MTSIF */
    if (parserBand->refcnt==0) {
        unsigned ibp = parserBand->hwIndex;
        pTransport->overflow.rsbuff.ibp[ibp] = 0;
        pTransport->overflow.xcbuff.ibp2rave[ibp] = 0;
        pTransport->overflow.xcbuff.ibp2msg[ibp] = 0;
        pTransport->overflow.xcbuff.ibp2rmx0[ibp] = 0;
        pTransport->overflow.xcbuff.ibp2rmx1[ibp] = 0;
    }

#if !BXPT_NUM_RAVE_CONTEXTS
    {
        int refcnt = 0;
        int i;
        NEXUS_InputBand inputBand = parserBand->inputBand;
        for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
            if (pTransport->parserBand[i]->inputBand == inputBand) {
                refcnt += pTransport->parserBand[i]->refcnt;
            }
        }
        enabled = refcnt?true:false;
        rc = BXPT_EnableOutputBufferBand(pTransport->xpt, inputBand, enabled);
        if (rc) {BERR_TRACE(rc);}
        pTransport->inputBand[inputBand].enabled = enabled;
    }
#endif
}

static void NEXUS_ParserBand_P_CCError_isr(void *context, int param)
{
    NEXUS_ParserBandHandle handle = context;
    BSTD_UNUSED(param);
    handle->ccErrorCount++;
    NEXUS_ParserBand_P_CountCcErrors_isr();
    NEXUS_IsrCallback_Fire_isr(handle->ccErrorCallback);
}

static void NEXUS_ParserBand_P_TEIError_isr(void *context, int param)
{
    NEXUS_ParserBandHandle handle = context;
    BSTD_UNUSED(param);
    handle->teiErrorCount++;
    NEXUS_IsrCallback_Fire_isr(handle->teiErrorCallback);
}

static void NEXUS_ParserBand_P_LengthError_isr(void *context, int param)
{
    NEXUS_ParserBandHandle handle = context;
    BSTD_UNUSED(param);
    handle->lengthErrorCount++;
    NEXUS_IsrCallback_Fire_isr(handle->lengthErrorCallback);
}

static NEXUS_Error NEXUS_ParserBand_P_SetInterrupts(NEXUS_ParserBandHandle parserBand, const NEXUS_ParserBandSettings *pSettings)
{
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(parserBand, NEXUS_ParserBand);

    if (pSettings->ccError.callback) {
        if (!parserBand->ccErrorInt) {
            BDBG_WRN(("create cc callback %p", (void *)parserBand));
            rc = BINT_CreateCallback(&parserBand->ccErrorInt, g_pCoreHandles->bint, BXPT_INT_ID_PARSER_CONTINUITY_ERROR(parserBand->hwIndex), NEXUS_ParserBand_P_CCError_isr, parserBand, 0);
            if (rc) return BERR_TRACE(rc);
            rc = BINT_EnableCallback(parserBand->ccErrorInt);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (parserBand->ccErrorInt) {
        BINT_DisableCallback(parserBand->ccErrorInt);
        BINT_DestroyCallback(parserBand->ccErrorInt);
        parserBand->ccErrorInt = NULL;
    }

    if (pSettings->teiError.callback) {
        if (!parserBand->teiErrorInt) {
            BDBG_WRN(("create tei callback %p", (void *)parserBand));
            rc = BINT_CreateCallback(&parserBand->teiErrorInt, g_pCoreHandles->bint, BXPT_INT_ID_PARSER_TRANSPORT_ERROR(parserBand->hwIndex), NEXUS_ParserBand_P_TEIError_isr, parserBand, 0);
            if (rc) return BERR_TRACE(rc);
            rc = BINT_EnableCallback(parserBand->teiErrorInt);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (parserBand->teiErrorInt) {
        BINT_DisableCallback(parserBand->teiErrorInt);
        BINT_DestroyCallback(parserBand->teiErrorInt);
        parserBand->teiErrorInt = NULL;
    }

    if (pSettings->lengthError.callback) {
        if (!parserBand->lengthErrorInt) {
            BDBG_WRN(("create length callback %p", (void *)parserBand));
            rc = BINT_CreateCallback(&parserBand->lengthErrorInt, g_pCoreHandles->bint, BXPT_INT_ID_PARSER_LENGTH_ERROR(parserBand->hwIndex), NEXUS_ParserBand_P_LengthError_isr, parserBand, 0);
            if (rc) return BERR_TRACE(rc);
            rc = BINT_EnableCallback(parserBand->lengthErrorInt);
            if (rc) return BERR_TRACE(rc);
        }
    }
    else if (parserBand->lengthErrorInt) {
        BINT_DisableCallback(parserBand->lengthErrorInt);
        BINT_DestroyCallback(parserBand->lengthErrorInt);
        parserBand->lengthErrorInt = NULL;
    }

    NEXUS_IsrCallback_Set(parserBand->ccErrorCallback, &pSettings->ccError );
    NEXUS_IsrCallback_Set(parserBand->teiErrorCallback, &pSettings->teiError);
    NEXUS_IsrCallback_Set(parserBand->lengthErrorCallback, &pSettings->lengthError);

    return 0;
}

void NEXUS_ParserBand_P_GetDefaultSettings(unsigned index, NEXUS_ParserBandSettings * pSettings)
{
    /* set default state */
    BKNI_Memset(pSettings, 0, sizeof(NEXUS_ParserBandSettings));
    pSettings->transportType = NEXUS_TransportType_eTs;
    pSettings->continuityCountEnabled = true;
    /* HW may not support connection to IB, so default to disconnected MTSIF */
    pSettings->sourceType = NEXUS_ParserBandSourceType_eMtsif;
    pSettings->sourceTypeSettings.mtsif = NULL;
#if NEXUS_HAS_LEGACY_XPT
    BSTD_UNUSED(index);
    pSettings->maxDataRate = 25000000; /* 25 Mbps */
#else
    pSettings->maxDataRate = pTransport->settings.maxDataRate.parserBand[index];
#endif
    pSettings->forceRestamping = true;
}

#if NEXUS_NUM_PARSER_BANDS
static NEXUS_Error NEXUS_ParserBand_P_Init(unsigned index)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_ParserBandHandle parserBand;

    parserBand = BKNI_Malloc(sizeof(*parserBand));
    if (!parserBand) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    NEXUS_OBJECT_INIT(NEXUS_ParserBand, parserBand);
    pTransport->parserBand[index] = parserBand;

    NEXUS_ParserBand_P_GetDefaultSettings(index, &parserBandSettings);

    parserBand->hwIndex = index;
    parserBand->enumBand = NEXUS_ParserBand_e0 + index; /* assumes enums are continuous */
    parserBand->pidChannels = 0;
    parserBand->enabled = false;
    parserBand->settings = parserBandSettings;
    parserBand->teiErrorCallback = NEXUS_IsrCallback_Create(parserBand, NULL);
    parserBand->ccErrorCallback = NEXUS_IsrCallback_Create(parserBand, NULL);
    parserBand->lengthErrorCallback = NEXUS_IsrCallback_Create(parserBand, NULL);
    rc = NEXUS_ParserBand_P_SetSettings(parserBand, &parserBandSettings);
    return rc;
}

static void NEXUS_ParserBand_P_Uninit(NEXUS_ParserBandHandle parserBand)
{
    if(parserBand->teiErrorInt)
    {
        BINT_DisableCallback(parserBand->teiErrorInt);
        BINT_DestroyCallback(parserBand->teiErrorInt);
        parserBand->teiErrorInt = NULL;
    }
    if(parserBand->ccErrorInt)
    {
        BINT_DisableCallback(parserBand->ccErrorInt);
        BINT_DestroyCallback(parserBand->ccErrorInt);
        parserBand->ccErrorInt = NULL;
    }
    if(parserBand->lengthErrorInt)
    {
        BINT_DisableCallback(parserBand->lengthErrorInt);
        BINT_DestroyCallback(parserBand->lengthErrorInt);
        parserBand->lengthErrorInt = NULL;
    }
    NEXUS_IsrCallback_Destroy(parserBand->teiErrorCallback);
    NEXUS_IsrCallback_Destroy(parserBand->ccErrorCallback);
    NEXUS_IsrCallback_Destroy(parserBand->lengthErrorCallback);
    if (parserBand->refcnt) {
        BDBG_WRN(("parser band %u still in use", parserBand->hwIndex));
    }
    pTransport->parserBand[parserBand->hwIndex] = NULL;
    NEXUS_OBJECT_DESTROY(NEXUS_ParserBand, parserBand);
    BKNI_Free(parserBand);
}
#endif

void NEXUS_ParserBand_P_UninitAll(void)
{
#if NEXUS_NUM_PARSER_BANDS
    unsigned i;
    for (i=0;i<NEXUS_NUM_PARSER_BANDS;i++) {
        if (pTransport->parserBand[i]) {
            NEXUS_ParserBand_P_Uninit(pTransport->parserBand[i]);
        }
    }
#endif
}

static NEXUS_ParserBandHandle NEXUS_ParserBand_P_Open(unsigned index)
{
    NEXUS_ParserBandHandle handle = NULL;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_Error rc;

    if (index == NEXUS_ANY_ID)
    {
        rc = NEXUS_ParserBand_P_NextAvailable(&index);
        if (rc) {
            BERR_TRACE(rc);
            goto end;
        }
    }
    if (index >= NEXUS_NUM_PARSER_BANDS || pTransport->parserBand[index])
    {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto end;
    }

    rc = NEXUS_ParserBand_P_Init(index);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto end;
    }

    handle = pTransport->parserBand[index];

    NEXUS_ParserBand_P_GetDefaultSettings(index, &parserBandSettings);
    (void)NEXUS_ParserBand_P_SetSettings(handle, &parserBandSettings);

end:
    return handle;
}

NEXUS_ParserBand NEXUS_ParserBand_Open(unsigned index)
{
    return (NEXUS_ParserBand)NEXUS_ParserBand_P_Open(index);
}

static void NEXUS_ParserBand_P_Finalizer(NEXUS_ParserBandHandle band)
{
    NEXUS_OBJECT_ASSERT(NEXUS_ParserBand, band);

    NEXUS_PidChannel_CloseAll(band->hwIndex);

    /* this should disable, if no other references */
    NEXUS_ParserBand_P_SetEnable(band);
    NEXUS_ParserBand_P_Uninit(band);
}

static NEXUS_OBJECT_CLASS_MAKE(NEXUS_ParserBand, NEXUS_ParserBand_P_Close);

void NEXUS_ParserBand_Close(NEXUS_ParserBand parserBand)
{
    NEXUS_ParserBandHandle handle = NULL;

    handle = NEXUS_ParserBand_Resolve_priv(parserBand);

    if (handle)
    {
        NEXUS_ParserBand_P_Close(handle);
    }
    else
    {
        BDBG_ERR(("You may be attempting to close the enum variant of this resource.  Please ensure you are passing the resource returned when you called open."));
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
}

NEXUS_Error NEXUS_ParserBand_GetStatus(NEXUS_ParserBand parserBand, NEXUS_ParserBandStatus *pStatus)
{
    NEXUS_ParserBandHandle handle = NULL;

    handle = NEXUS_ParserBand_Resolve_priv(parserBand);

    if (!handle) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    BDBG_OBJECT_ASSERT(handle, NEXUS_ParserBand);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->index = handle->hwIndex;

    /* sum rsbuff overflows */
    if (handle->hwIndex < 32) {
        unsigned ibp = handle->hwIndex;
        if (handle->mpodBand) {
            pStatus->rsBufferStatus.overflowErrors += pTransport->overflow.rsbuff.mpodIbp[ibp];
        }
        else {
            pStatus->rsBufferStatus.overflowErrors += pTransport->overflow.rsbuff.ibp[ibp];
        }
    }

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_ParserBand_GetAllPassPidChannelIndex(
    NEXUS_ParserBand parserBand,
    unsigned *pHwPidChannel
    )
{
    NEXUS_ParserBandHandle handle = NULL;

    handle = NEXUS_ParserBand_P_ResolveAcquire(parserBand, false);
    if (handle)
    {
        /* On the front-end parsers, the PID channel number is the parser band number. There's little risk that it'll change. */
        *pHwPidChannel = BXPT_GET_IB_PARSER_ALLPASS_CHNL( handle->hwIndex );
    }
    else
    {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    return 0;
}

static bool is_mtsif(unsigned i, NEXUS_FrontendConnectorHandle connector, void **tsmf)
{
    const NEXUS_ParserBandSettings *ps;

    if (i >= NEXUS_NUM_PARSER_BANDS || !pTransport->parserBand[i]) {
        return false;
    }
    ps = &pTransport->parserBand[i]->settings;
    *tsmf = NULL;

    /* if connector is NULL, match any */
    if (ps->sourceType == NEXUS_ParserBandSourceType_eMtsif && (ps->sourceTypeSettings.mtsif == connector || !connector)) {
        return true;
    }
#if NEXUS_TRANSPORT_EXTENSION_TSMF
    if (ps->sourceType == NEXUS_ParserBandSourceType_eTsmf && ps->sourceTypeSettings.tsmf) {
        NEXUS_TsmfSettings settings;
        NEXUS_Tsmf_GetSettings(ps->sourceTypeSettings.tsmf, &settings);
        if (settings.sourceType == NEXUS_TsmfSourceType_eMtsif && (settings.sourceTypeSettings.mtsif==connector || !connector)) {
            *tsmf = ps->sourceTypeSettings.tsmf;
            return true;
        }
    }
#endif
    return false;
}

void NEXUS_ParserBand_GetMtsifConnections_priv( NEXUS_FrontendConnectorHandle connector, struct NEXUS_MtsifParserBandSettings *pSettings, unsigned numEntries, unsigned *pNumReturned)
{
    unsigned i, total = 0;
    NEXUS_ASSERT_MODULE();
    for (i=0;i < NEXUS_NUM_PARSER_BANDS && total < numEntries;i++) {
        void *tsmf = NULL;
        if (is_mtsif(i, connector, &tsmf)) {
            BKNI_Memset(&pSettings[total], 0, sizeof(pSettings[total]));
            pSettings[total].index = i;
            pSettings[total].settings = pTransport->parserBand[i]->settings;
#if NEXUS_TRANSPORT_EXTENSION_TSMF
            if (tsmf) {
                NEXUS_Tsmf_ReadSettings_priv(tsmf, &pSettings[total]);
            }
#endif
            total++;
        }
    }
    *pNumReturned = total;
}

void NEXUS_ParserBand_P_MtsifErrorStatus_priv(unsigned lengthError, unsigned transportError)
{
    unsigned i;
    NEXUS_ASSERT_MODULE();
    for (i=0;(lengthError || transportError) && i<NEXUS_NUM_PARSER_BANDS;i++) {
        void *tsmf;
        if (!is_mtsif(i, NULL, &tsmf)) continue;
        if ((lengthError & 1) || (transportError & 1)) {
            BKNI_EnterCriticalSection();
            if (lengthError & 1) {
                NEXUS_ParserBand_P_LengthError_isr(NEXUS_ParserBand_Resolve_priv(i), 0);
            }
            if (transportError & 1) {
                NEXUS_ParserBand_P_TEIError_isr(NEXUS_ParserBand_Resolve_priv(i), 0);
            }
            BKNI_LeaveCriticalSection();
        }
        lengthError >>= 1;
        transportError >>= 1;
    }
}

#else /* NEXUS_NUM_PARSER_BANDS */

NEXUS_ParserBandHandle NEXUS_ParserBand_Resolve_priv(NEXUS_ParserBand band)
{
    BSTD_UNUSED(band);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
}

void NEXUS_ParserBand_P_GetSettings(NEXUS_ParserBandHandle band, NEXUS_ParserBandSettings *pSettings)
{
    BSTD_UNUSED(band);
    BSTD_UNUSED(pSettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_ParserBand_GetSettings(NEXUS_ParserBand parserBand, NEXUS_ParserBandSettings *pSettings)
{
    BSTD_UNUSED(parserBand);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Parser Band not enabled on this chipset"));
    return;
}

NEXUS_Error NEXUS_ParserBand_SetSettings(NEXUS_ParserBand parserBand, const NEXUS_ParserBandSettings *pSettings)
{
    BSTD_UNUSED(parserBand);
    BSTD_UNUSED(pSettings);
    BDBG_WRN(("Parser Band not enabled on this chipset"));
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return 0;
}

NEXUS_ParserBand NEXUS_ParserBand_Open(unsigned index)
{
    BSTD_UNUSED(index);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NEXUS_ParserBand_eInvalid;
}

static void NEXUS_ParserBand_P_Finalizer(NEXUS_ParserBandHandle band)
{
    BSTD_UNUSED(band);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

static NEXUS_OBJECT_CLASS_MAKE(NEXUS_ParserBand, NEXUS_ParserBand_P_Close);

void NEXUS_ParserBand_Close(NEXUS_ParserBand parserBand)
{
    BSTD_UNUSED(parserBand);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    BSTD_UNUSED(&NEXUS_ParserBand_P_Close);
}

NEXUS_Error NEXUS_ParserBand_GetStatus(NEXUS_ParserBand parserBand, NEXUS_ParserBandStatus *pStatus)
{
    BSTD_UNUSED(parserBand);
    BSTD_UNUSED(pStatus);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_ParserBand_P_SetEnable(NEXUS_ParserBandHandle parserBand)
{
    BSTD_UNUSED(parserBand);
    BDBG_WRN(("Parser Band not enabled on this chipset"));
    return;
}

NEXUS_Error NEXUS_ParserBand_GetAllPassPidChannelIndex(
    NEXUS_ParserBand parserBand,
    unsigned *pHwPidChannel
    )
{
    BSTD_UNUSED(parserBand);
    BSTD_UNUSED(pHwPidChannel);
    BDBG_WRN(("Parser Band not enabled on this chipset"));
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
}

void NEXUS_ParserBand_GetMtsifConnections_priv( NEXUS_FrontendConnectorHandle connector, struct NEXUS_MtsifParserBandSettings *pSettings, unsigned numEntries, unsigned *pNumReturned)
{
    BSTD_UNUSED(connector);
    BSTD_UNUSED(pSettings);
    BSTD_UNUSED(numEntries);
    BSTD_UNUSED(pNumReturned);
}

void NEXUS_ParserBand_P_MtsifErrorStatus_priv(unsigned lengthError, unsigned transportError)
{
    BSTD_UNUSED(lengthError);
    BSTD_UNUSED(transportError);
}

#endif /* NEXUS_NUM_PARSER_BANDS */
