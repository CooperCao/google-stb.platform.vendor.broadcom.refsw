/***************************************************************************
 *     (c)2007-2013 Broadcom Corporation
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
 **************************************************************************/
#include "nexus_display_module.h"
#include "nexus_power_management.h"
#include "bfmt.h"
BDBG_MODULE(nexus_hddvi_input);

struct NEXUS_HdDviInput {
    NEXUS_OBJECT(NEXUS_HdDviInput);
    NEXUS_HdDviInputSettings settings;
    NEXUS_VideoInputObject input;
    BAVC_SourceId sourceId;
    BAVC_VDC_HdDvi_Picture stHdDviPic;
    unsigned index;
    struct {
        NEXUS_HdDviInputHandle master, slave;
    } alias;
};

#if NEXUS_NUM_HDDVI_INPUTS
static const BFMT_VideoFmt g_autoDetectFormats[] = {
    BFMT_VideoFmt_eNTSC ,
    BFMT_VideoFmt_e1080i,
    BFMT_VideoFmt_e1080p,
    BFMT_VideoFmt_e720p,
    BFMT_VideoFmt_e480p,
    BFMT_VideoFmt_e1080p_30Hz
};

static NEXUS_HdDviInputHandle g_handle[NEXUS_NUM_HDDVI_INPUTS];

static void NEXUS_VideoInput_P_HdDviInputPictureCallback_isr(void *pvParm1, int iParm2,
      BAVC_Polarity ePolarity, BAVC_SourceState eSourceState, void **ppvPicture)
{
    NEXUS_HdDviInputHandle hdDviInput =(NEXUS_HdDviInputHandle)pvParm1;
    BSTD_UNUSED(iParm2);
    BSTD_UNUSED(ePolarity);
    BSTD_UNUSED(eSourceState);
    *ppvPicture = &hdDviInput->stHdDviPic;
    BDBG_MSG(("NEXUS_VideoInput_P_HdDviInputPictureCallback_isr"));
    return;
}
#endif

void NEXUS_HdDviInput_GetDefaultSettings( NEXUS_HdDviInputSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->autoFormat = true;
    pSettings->inputDataMode = 24; /* 24 bit */
    pSettings->format = NEXUS_VideoFormat_eNtsc;
    pSettings->colorSpace  = NEXUS_ColorSpace_eYCbCr422;
    pSettings->colorPrimaries  = NEXUS_ColorPrimaries_eSmpte_170M;
    pSettings->matrixCoef = NEXUS_MatrixCoefficients_eSmpte_170M;
    pSettings->transferCharacteristics = NEXUS_TransferCharacteristics_eSmpte_170M;
}

NEXUS_HdDviInputHandle NEXUS_HdDviInput_Open( unsigned index, const NEXUS_HdDviInputSettings *pSettings )
{
#if NEXUS_NUM_HDDVI_INPUTS
    NEXUS_HdDviInputHandle hdDviInput;
    NEXUS_HdDviInputSettings defaultSettings;
    NEXUS_HdDviInputHandle master = NULL;

    if (index >= NEXUS_ALIAS_ID && index-NEXUS_ALIAS_ID < NEXUS_NUM_HDDVI_INPUTS) {
        BDBG_MSG(("%d aliasing %d(%p)", index, index-NEXUS_ALIAS_ID, g_handle[index-NEXUS_ALIAS_ID]));
        index -= NEXUS_ALIAS_ID;
        master = g_handle[index];
        if (!master) {
            BDBG_ERR(("cannot alias %d because it is not opened", index));
            return NULL;
        }
        if (master->alias.slave) {
            BDBG_ERR(("%d already aliased", index));
            return NULL;
        }
    }

    if (index >= NEXUS_NUM_HDDVI_INPUTS) {
        BDBG_ERR(("Cannot open HdDviInput %d. NEXUS_NUM_HDDVI_INPUTS is %d.", index, NEXUS_NUM_HDDVI_INPUTS));
        return NULL;
    }

    if (!pSettings) {
        NEXUS_HdDviInput_GetDefaultSettings(&defaultSettings);
        pSettings = &defaultSettings;
    }

    hdDviInput = BKNI_Malloc(sizeof(*hdDviInput));
    if (!hdDviInput) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_HdDviInput, hdDviInput);

    if (master) {
        int rc;
        hdDviInput->alias.master = master;
        master->alias.slave = hdDviInput;
        rc = NEXUS_HdDviInput_SetSettings(hdDviInput, pSettings);
        if (rc) {
            BERR_TRACE(rc);
            goto error;
        }
        return hdDviInput;
    }

    NEXUS_VIDEO_INPUT_INIT(&hdDviInput->input, NEXUS_VideoInputType_eHdDvi, hdDviInput);
    hdDviInput->index = index;
    g_handle[index] = hdDviInput;
    hdDviInput->settings = *pSettings;

    if(pSettings->colorSpace == NEXUS_ColorSpace_eRgb)
    {
        if(pSettings->matrixCoef == NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB)
        {
            if(pSettings->colorPrimaries == NEXUS_ColorPrimaries_eItu_R_BT_709)
                hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e709RgbFullRange;
            else
                hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e601RgbFullRange;
        }
        else if(pSettings->matrixCoef == NEXUS_MatrixCoefficients_eHdmi_RGB)
        {
            if(pSettings->colorPrimaries == NEXUS_ColorPrimaries_eItu_R_BT_709)
                hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e709RgbLimitedRange;
            else
                hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e601RgbLimitedRange;
        }
        else
        {
            BDBG_ERR(("Mismatch color space and matrix coefficients settings "));
            goto error;
        }
    }
    else
    {
        if(pSettings->colorPrimaries == NEXUS_ColorPrimaries_eItu_R_BT_709)
            hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e709YCbCr;
        else
            hdDviInput->stHdDviPic.eCscMode = BAVC_CscMode_e601YCbCr;
    }
    hdDviInput->stHdDviPic.eColorSpace              = pSettings->colorSpace;
    hdDviInput->stHdDviPic.eMatrixCoefficients      = NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(pSettings->matrixCoef);
    hdDviInput->stHdDviPic.eTransferCharacteristics = NEXUS_P_TransferCharacteristics_ToMagnum_isrsafe(pSettings->transferCharacteristics);

    switch (index) {
    case 0: hdDviInput->sourceId = BAVC_SourceId_eHdDvi0; break;
#if NEXUS_NUM_HDDVI_INPUTS > 1
    case 1: hdDviInput->sourceId = BAVC_SourceId_eHdDvi1; break;
#endif
    /* add each one explicitly. don't assume BAVC_SourceId enums are contiguous */
    /* defensive programming below gives coverity false positive */
    /* coverity[dead_error_begin] */
    default: BERR_TRACE(NEXUS_INVALID_PARAMETER); goto error;
    }

    NEXUS_OBJECT_REGISTER(NEXUS_VideoInput, &hdDviInput->input, Open);
    return hdDviInput;

error:
    NEXUS_HdDviInput_Close(hdDviInput);
    return NULL;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(pSettings);
    BERR_TRACE(NEXUS_NOT_SUPPORTED);
    return NULL;
#endif
}


static void NEXUS_HdDviInput_P_Finalizer( NEXUS_HdDviInputHandle hdDviInput )
{
    NEXUS_OBJECT_ASSERT(NEXUS_HdDviInput, hdDviInput);

    /* This input is in the Display module, so we can auto-disconnect & shutdown */
    if (hdDviInput->input.destination) {
        NEXUS_VideoWindow_RemoveInput(hdDviInput->input.destination, &hdDviInput->input);
        BDBG_ASSERT(!hdDviInput->input.destination);
    }
    if (hdDviInput->input.ref_cnt) {
        NEXUS_VideoInput_Shutdown(&hdDviInput->input);
        BDBG_ASSERT(!hdDviInput->input.ref_cnt);
    }

    NEXUS_OBJECT_DESTROY(NEXUS_HdDviInput, hdDviInput);
    BKNI_Free(hdDviInput);
}

static void NEXUS_HdDviInput_P_Release( NEXUS_HdDviInputHandle hdDviInput )
{
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
#if NEXUS_NUM_HDDVI_INPUTS
    if (hdDviInput->alias.master) {
        /* if slave, immediately unlink from master and verify that finalizer is no-op */
        hdDviInput->alias.master->alias.slave = NULL;
        BDBG_ASSERT(!hdDviInput->input.destination);
        BDBG_ASSERT(!hdDviInput->input.ref_cnt);
    }
    else {
        /* if possible master, close any slaves */
        unsigned i;
        NEXUS_OBJECT_UNREGISTER(NEXUS_VideoInput, &hdDviInput->input, Close);
        for (i=0;i<NEXUS_NUM_HDDVI_INPUTS;i++) {
            if (g_handle[i] && g_handle[i]->alias.master == hdDviInput) {
                /* slave is no longer callable by client */
                NEXUS_HdDviInput_Close(g_handle[i]);
            }
        }
        BDBG_ASSERT(g_handle[hdDviInput->index] == hdDviInput);
        g_handle[hdDviInput->index] = NULL;
    }
#endif
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_HdDviInput, NEXUS_HdDviInput_Close);


#define RESOLVE_ALIAS(handle) do {(handle) = ((handle)->alias.master?(handle)->alias.master:(handle));}while(0)

void NEXUS_HdDviInput_GetSettings( NEXUS_HdDviInputHandle hdDviInput, NEXUS_HdDviInputSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    RESOLVE_ALIAS(hdDviInput);
    *pSettings = hdDviInput->settings;
}

#if NEXUS_NUM_HDDVI_INPUTS
static NEXUS_Error NEXUS_HdDviInput_P_ApplyVdcSettings(NEXUS_VideoInput_P_Link *link, const NEXUS_HdDviInputSettings *pSettings)
{
    BERR_Code rc;
    BVDC_HdDvi_Settings vdcSettings;
    BFMT_VideoFmt formatVdc;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;
    unsigned  i;

    BDBG_MSG((" > NEXUS_HdDviInput_P_ApplyVdcSettings"));

    rc = BVDC_Source_SetAutoFormat(link->sourceVdc, pSettings->autoFormat, (void *)g_autoDetectFormats, sizeof(g_autoDetectFormats)/sizeof(*g_autoDetectFormats));
    if(rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    if(!pSettings->autoFormat && pSettings->format != NEXUS_VideoFormat_eUnknown) {
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(pSettings->format, &formatVdc);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
        rc = BVDC_Source_SetVideoFormat(link->sourceVdc, formatVdc);
        if (rc!=BERR_SUCCESS) {return BERR_TRACE(rc);}
    }

    (void)BVDC_Source_GetHdDviConfiguration(link->sourceVdc, &vdcSettings);
    vdcSettings.bEnableDe = pSettings->enableDe;
    vdcSettings.stFmtTolerence.ulWidth = pSettings->formatDetectionTolerance.width;
    vdcSettings.stFmtTolerence.ulHeight = pSettings->formatDetectionTolerance.height;

    switch (pSettings->inputDataMode) {
    case 24: vdcSettings.eInputDataMode = BVDC_HdDvi_InputDataMode_e24Bit; break;
    case 30: vdcSettings.eInputDataMode = BVDC_HdDvi_InputDataMode_e30Bit; break;
    case 36: vdcSettings.eInputDataMode = BVDC_HdDvi_InputDataMode_e36Bit; break;
    default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    BVDC_Source_SetInputPort(link->sourceVdc, !pSettings->external);
    BVDC_Source_SetHVStart(link->sourceVdc, pSettings->startPosition.enabled,
        pSettings->startPosition.horizontal, pSettings->startPosition.vertical);
    vdcSettings.bOverrideMux = pSettings->custom.enabled;
    for(i = 0; i < BVDC_MAX_HDDVI_MUX_COUNT; i++)
       vdcSettings.aucMux[i] = pSettings->custom.data[i];

    rc = BVDC_Source_SetHdDviConfiguration(link->sourceVdc, &vdcSettings);
    if (rc) return BERR_TRACE(rc);

    rc = BVDC_ApplyChanges(video->vdc);
    if ( rc )
    {
        return BERR_TRACE(rc);
    }
    BDBG_MSG((" < NEXUS_HdDviInput_P_ApplyVdcSettings"));
    return 0;
}
#endif

NEXUS_Error NEXUS_HdDviInput_SetSettings( NEXUS_HdDviInputHandle hdDviInput, const NEXUS_HdDviInputSettings *pSettings )
{
#if NEXUS_NUM_HDDVI_INPUTS
    NEXUS_VideoInput_P_Link *link;

    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    RESOLVE_ALIAS(hdDviInput);
    BDBG_MSG((" > NEXUS_HdDviInput_SetSettings"));
    hdDviInput->settings = *pSettings;

    link = hdDviInput->input.destination;
    if (!link) return 0; /* not connected */
    BDBG_MSG((" < NEXUS_HdDviInput_SetSettings"));
    return NEXUS_HdDviInput_P_ApplyVdcSettings(link, &hdDviInput->settings);
#else
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    hdDviInput->settings = *pSettings;
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_VideoInput NEXUS_HdDviInput_GetConnector( NEXUS_HdDviInputHandle hdDviInput )
{
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    RESOLVE_ALIAS(hdDviInput);
    return &hdDviInput->input;
}

NEXUS_Error NEXUS_HdDviInput_GetStatus( NEXUS_HdDviInputHandle hdDviInput, NEXUS_HdDviInputStatus *pStatus )
{
    BFMT_AspectRatio vdcAspectRatio;
    BFMT_VideoFmt vdcVideoFormat;
    NEXUS_VideoInput_P_Link *link;
    BERR_Code rc;

    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    RESOLVE_ALIAS(hdDviInput);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    link = hdDviInput->input.destination;
    if (!link) return 0; /* not connected */

    BDBG_ASSERT(link->sourceVdc);

    rc = BVDC_Source_GetAspectRatio(link->sourceVdc, &vdcAspectRatio);
    if (rc) return BERR_TRACE(rc);
    pStatus->aspectRatio = NEXUS_P_AspectRatio_FromMagnum_isrsafe(vdcAspectRatio);

    rc = BVDC_Source_GetVideoFormat(link->sourceVdc, &vdcVideoFormat);
    if (rc) return BERR_TRACE(rc);
    pStatus->videoFormat = NEXUS_P_VideoFormat_FromMagnum_isrsafe(vdcVideoFormat);

    return 0;
}

#if NEXUS_NUM_HDDVI_INPUTS
static NEXUS_Error
NEXUS_VideoInput_P_ConnectHdDviInput(NEXUS_VideoInput_P_Link *link)
{
    BERR_Code rc ;
    NEXUS_HdDviInputHandle hdDviInput;

    BDBG_ASSERT(link->input);
    BDBG_ASSERT(link->input->type == NEXUS_VideoInputType_eHdDvi);
    hdDviInput = link->input->source;
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    BDBG_MSG((" < NEXUS_VideoInput_P_ConnectHdDviInput"));
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHdDviInput, true);
    rc = BVDC_Source_InstallPictureCallback(link->sourceVdc,NEXUS_VideoInput_P_HdDviInputPictureCallback_isr,
                                       (void *)link->input->source, (BAVC_SourceId_eHdDvi0 == hdDviInput->sourceId) ? 0 : 1);
    if (rc) return BERR_TRACE(rc);
    BDBG_MSG((" > NEXUS_VideoInput_P_ConnectHdDviInput"));
    return NEXUS_HdDviInput_P_ApplyVdcSettings(link, &hdDviInput->settings);
}

static void
NEXUS_VideoInput_P_DisconnectHdDviInput(NEXUS_VideoInput_P_Link *link)
{
    NEXUS_HdDviInputHandle hdDviInput;

    BDBG_ASSERT(link->input);
    BDBG_ASSERT(link->input->type == NEXUS_VideoInputType_eHdDvi);
    hdDviInput = link->input->source;
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);
    BDBG_MSG((" > NEXUS_VideoInput_P_DisconnectHdDviInput"));
    NEXUS_PowerManagement_SetCoreState(NEXUS_PowerManagementCore_eHdDviInput, false);
    return;
}

NEXUS_VideoInput_P_Link *
NEXUS_VideoInput_P_OpenHdDviInput(NEXUS_VideoInput input)
{
    NEXUS_VideoInput_P_Iface iface;
    NEXUS_VideoInput_P_Link *link;
    NEXUS_VideoInput_P_LinkData data;
    NEXUS_HdDviInputHandle hdDviInput;
    BDBG_MSG((" > NEXUS_VideoInput_P_OpenHdDviInput"));
    BDBG_ASSERT(input->type == NEXUS_VideoInputType_eHdDvi);
    hdDviInput = input->source;
    BDBG_OBJECT_ASSERT(hdDviInput, NEXUS_HdDviInput);

    iface.connect = NEXUS_VideoInput_P_ConnectHdDviInput;
    iface.disconnect = NEXUS_VideoInput_P_DisconnectHdDviInput;
    NEXUS_VideoInput_P_LinkData_Init(&data, hdDviInput->sourceId);
    link = NEXUS_VideoInput_P_CreateLink(input, &data, &iface);
    if(!link) {
        return NULL;
    }
    BDBG_MSG((" < NEXUS_VideoInput_P_OpenHdDviInput"));
    return link;
}
#endif

/*
Other possible VDC HdDvi functions to be added:
BVDC_Source_SetAutoFormat
BVDC_Source_SetVideoFormat
BVDC_Window_SetUserPanScan
BVDC_Source_OverrideAspectRatio
*/

