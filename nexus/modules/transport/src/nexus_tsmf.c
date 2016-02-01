/***************************************************************************
 *     (c)2003-2013 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *
 * $brcm_Workfile: $
 * $brcm_Revision: $
 * $brcm_Date: $
 *
 * Module Description:
 *
 * Revision History:
 *
 * $brcm_Log: $
 * 
 ***************************************************************************/
#include "nexus_transport_module.h"
#include "nexus_playpump_impl.h"
#include "priv/nexus_tsmf_priv.h"
#if NEXUS_HAS_TSMF
#include "bxpt_tsmf.h"
#endif

BDBG_MODULE(nexus_tsmf);

#if NEXUS_HAS_TSMF

struct NEXUS_Tsmf
{
    NEXUS_OBJECT(NEXUS_Tsmf);
    BLST_S_ENTRY(NEXUS_Tsmf) link;
    unsigned index;
    NEXUS_TsmfOpenSettings openSettings;
    NEXUS_TsmfSettings settings;
    bool pendingSetSettings; /* set on NEXUS_Tsmf_SetSettings(). cleared on NEXUS_TuneXyz() */
};

NEXUS_Tsmf_P_State g_NEXUS_Tsmf_P_State;

/* establishes TSMF -> PB mapping on host (i.e. set output of TSMF). 
   uses the currently-saved PB settings and TSMF settings */
NEXUS_Error NEXUS_Tsmf_SetOutput_priv(void *parserBandHandle)
{
    NEXUS_ParserBandHandle parserBand = parserBandHandle;
    NEXUS_Error rc;
    NEXUS_ASSERT_MODULE();

    BDBG_ASSERT(parserBand->settings.sourceType == NEXUS_ParserBandSourceType_eTsmf);

    if (parserBand->settings.sourceTypeSettings.tsmf==NULL) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    NEXUS_OBJECT_ASSERT(NEXUS_Tsmf, parserBand->settings.sourceTypeSettings.tsmf);

    if (parserBand->settings.sourceTypeSettings.tsmf->settings.sourceType == NEXUS_TsmfSourceType_eMtsif) {
        return NEXUS_SUCCESS; /* demod TSMF output is handled in frontend code */
    }
    else {
        unsigned parserIndex = parserBand->hwIndex;
        unsigned tsmfIndex = parserBand->settings.sourceTypeSettings.tsmf->index;
        unsigned enabled = parserBand->settings.sourceTypeSettings.tsmf->settings.enabled;

        BDBG_MSG(("TSMF%u -> PB%u (enable %u)", tsmfIndex, parserIndex, enabled));
        rc = BXPT_TSMF_SetParserConfig(pTransport->xpt, parserIndex, tsmfIndex, enabled);
        if (rc) return BERR_TRACE(rc);
    }
    return NEXUS_SUCCESS;
}

/* read settings from tsmf so they can be applied to the demod */
void NEXUS_Tsmf_ReadSettings_priv(NEXUS_TsmfHandle tsmf, struct NEXUS_MtsifParserBandSettings *pSettings)
{
    pSettings->tsmf.valid = true;
    pSettings->tsmf.hwIndex = tsmf->index;
    pSettings->tsmf.pending = tsmf->pendingSetSettings;
    tsmf->pendingSetSettings = false; /* read and clear */
    pSettings->tsmf.settings = tsmf->settings;
}

void NEXUS_Tsmf_GetDefaultOpenSettings(NEXUS_TsmfOpenSettings *pOpenSettings)
{
    BKNI_Memset(pOpenSettings, 0, sizeof(*pOpenSettings));
}

NEXUS_TsmfHandle NEXUS_Tsmf_Open(unsigned index, const NEXUS_TsmfOpenSettings* pOpenSettings)
{
    NEXUS_Error rc;
    NEXUS_TsmfHandle handle, prev, curr;
    NEXUS_TsmfOpenSettings openSettings;
    if (pOpenSettings==NULL) {
        NEXUS_Tsmf_GetDefaultOpenSettings(&openSettings);
    }

    for (handle=BLST_S_FIRST(&g_NEXUS_Tsmf_P_State.handles); handle; handle=BLST_S_NEXT(handle, link)) {
        if (handle->index==index) {
            BDBG_ERR(("TSMF index %u already opened", index));
            rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }
  
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }

    BKNI_Memset(handle, 0, sizeof(*handle));
    NEXUS_OBJECT_INIT(NEXUS_Tsmf, handle);

    handle->index = index;
    handle->openSettings = pOpenSettings ? *pOpenSettings : openSettings;
    handle->settings.sourceType = NEXUS_TsmfSourceType_eMtsif;

#if 0
    BLST_S_INSERT_HEAD(&g_NEXUS_Tsmf_P_State.handles, handle, link);
#else
    for (prev=NULL, curr=BLST_S_FIRST(&g_NEXUS_Tsmf_P_State.handles); curr; curr=BLST_S_NEXT(curr, link)) {
        prev = curr;
    }
    if (prev) {
        BLST_S_INSERT_AFTER(&g_NEXUS_Tsmf_P_State.handles, prev, handle, link);
    }
    else {
        BLST_S_INSERT_HEAD(&g_NEXUS_Tsmf_P_State.handles, handle, link);
    }
#endif
    
    return handle;
}

static void NEXUS_Tsmf_P_Finalizer(NEXUS_TsmfHandle handle)
{
    BLST_S_REMOVE(&g_NEXUS_Tsmf_P_State.handles, handle, NEXUS_Tsmf, link);
    NEXUS_OBJECT_DESTROY(NEXUS_Tsmf, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_Tsmf, NEXUS_Tsmf_Close);

void NEXUS_Tsmf_GetSettings(NEXUS_TsmfHandle handle, NEXUS_TsmfSettings *pSettings)
{
    NEXUS_Error rc;
    unsigned tsmfIndex = handle->index;
    
    BDBG_OBJECT_ASSERT(handle, NEXUS_Tsmf);
    BDBG_ASSERT(NULL != pSettings);

    if (handle->settings.sourceType==NEXUS_TsmfSourceType_eMtsif) {
        goto done;
    }
    rc = BXPT_TSMF_GetFldVerifyConfig(pTransport->xpt, tsmfIndex, (BXPT_TSMFFldVerifyConfig *)(&(handle->settings.fieldVerifyConfig)));
    if (rc) {rc = BERR_TRACE(rc);} /* keep going */

done:
    *pSettings = handle->settings;
}

NEXUS_Error NEXUS_Tsmf_SetSettings(NEXUS_TsmfHandle handle, const NEXUS_TsmfSettings *pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned tsmfIndex = handle->index;
    bool changed = false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_Tsmf);
    BDBG_ASSERT(pSettings);

    BDBG_CASSERT(sizeof(NEXUS_TsmfFieldVerifyConfig) == sizeof(BXPT_TSMFFldVerifyConfig));
    BDBG_CASSERT(NEXUS_TsmfVersionChangeMode_eFrameChangeVer == (NEXUS_TsmfVersionChangeMode)BXPT_TSMFVersionChgMode_eFrameChgVer);

    if (pSettings->sourceType==NEXUS_TsmfSourceType_eMtsif) {
        /* defer to frontend */
        handle->pendingSetSettings = true;
        goto done;
    }

    rc = BXPT_TSMF_SetFldVerifyConfig(pTransport->xpt, tsmfIndex, (const BXPT_TSMFFldVerifyConfig *)&(pSettings->fieldVerifyConfig));
    if (rc) {rc = BERR_TRACE(rc);} /* keep going */

    if ((pSettings->enabled!=handle->settings.enabled) || (pSettings->semiAutomaticMode!=handle->settings.semiAutomaticMode)) {
        changed = true;
       
        if (pSettings->enabled) {
            BXPT_TSMF_InputSel inputSelect = BXPT_TSMF_InputSel_eIB0;

            /* convert to XPT enum */
            if (pSettings->sourceType==NEXUS_TsmfSourceType_eInputBand) {
                inputSelect = BXPT_TSMF_InputSel_eIB0 + (pSettings->sourceTypeSettings.inputBand - NEXUS_InputBand_e0);
            }
            else if (pSettings->sourceType==NEXUS_TsmfSourceType_eRemux) {
                if (pSettings->sourceTypeSettings.remux->index > 0) {
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED); /* if HW supports, must extend enum */
                }
                inputSelect = BXPT_TSMF_InputSel_eRMX;
            }

            /* specify the TSMF input (and other params). the TSMF output is set via NEXUS_ParserBand_SetSettings or NEXUS_Frontend_TuneXyz */
            BDBG_MSG(("IB%u -> TSMF%u", inputSelect, tsmfIndex));
            if (pSettings->semiAutomaticMode) {
                rc = BXPT_TSMF_EnableSemiAutoMode(pTransport->xpt, inputSelect, tsmfIndex, pSettings->slotMapLo, pSettings->slotMapHi, pSettings->relativeTsNum);
                if (rc) return BERR_TRACE(rc);
            }
            else {
                rc = BXPT_TSMF_EnableAutoMode(pTransport->xpt, inputSelect, tsmfIndex, pSettings->relativeTsNum);
                if (rc) return BERR_TRACE(rc);
            }
        }
        else {
            BDBG_MSG(("TSMF%u disabled", tsmfIndex));
            rc = BXPT_TSMF_DisableTsmf(pTransport->xpt, tsmfIndex);
            if (rc) return BERR_TRACE(rc);
        }
    }

done:
    handle->settings = *pSettings;

    /* if output already connected (TSMF -> PB), then reapply */
    if (changed && pSettings->sourceType!=NEXUS_TsmfSourceType_eMtsif) {
        unsigned i;
        for (i=0; i<NEXUS_NUM_PARSER_BANDS; i++) {
            NEXUS_ParserBandSettings *pbSettings;
            if (!pTransport->parserBand[i]) continue;
            pbSettings = &pTransport->parserBand[i]->settings;
            if (pbSettings->sourceType == NEXUS_ParserBandSourceType_eTsmf && pbSettings->sourceTypeSettings.tsmf != NULL) 
            {
                NEXUS_TsmfHandle pbTsmf = pbSettings->sourceTypeSettings.tsmf;
                NEXUS_OBJECT_ASSERT(NEXUS_Tsmf, pbTsmf);
                if (pbTsmf == handle) {
                    NEXUS_Tsmf_SetOutput_priv(pTransport->parserBand[i]);
                }
            }
        }
    }

    return rc;
}

#else /* NEXUS_HAS_TSMF */

void NEXUS_Tsmf_GetDefaultOpenSettings(NEXUS_TsmfOpenSettings *pOpenSettings)
{
    BSTD_UNUSED(pOpenSettings);
}

struct NEXUS_Tsmf
{
    NEXUS_OBJECT(NEXUS_Tsmf);
};

NEXUS_TsmfHandle NEXUS_Tsmf_Open(unsigned index, const NEXUS_TsmfOpenSettings* pOpenSettings)
{
    BSTD_UNUSED(index);
    BSTD_UNUSED(pOpenSettings);
    return NULL;
}

static void NEXUS_Tsmf_P_Finalizer(NEXUS_TsmfHandle handle)
{
    BSTD_UNUSED(handle);
}

void NEXUS_Tsmf_GetSettings(NEXUS_TsmfHandle handle, NEXUS_TsmfSettings *pSettings)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
}

NEXUS_Error NEXUS_Tsmf_SetSettings(NEXUS_TsmfHandle handle, const NEXUS_TsmfSettings *pSettings)
{
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
    return 0;
}
NEXUS_OBJECT_CLASS_MAKE(NEXUS_Tsmf, NEXUS_Tsmf_Close);
#endif
