/******************************************************************************
 * Broadcom Proprietary and Confidential. (c) 2016 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed pursuant
 * to the terms and conditions of a separate, written license agreement executed
 * between you and Broadcom (an "Authorized License").  Except as set forth in
 * an Authorized License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and Broadcom
 * expressly reserves all rights in and to the Software and all intellectual
 * property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use all
 *    reasonable efforts to protect the confidentiality thereof, and to use
 *    this information only in connection with your use of Broadcom integrated
 *    circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
 *    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
 *    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
 *    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 *****************************************************************************/
#include "nexus_base.h"
#include "nexus_display_module.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output_hdcp.h"
#endif
BDBG_MODULE(nexus_video_output);

#if NEXUS_HAS_RFM
#include "priv/nexus_rfm_priv.h"
#endif

static const NEXUS_SvideoOutputSettings NEXUS_SvideoOutputDefaultSettings = {
    NEXUS_VideoDac_eNone,
    NEXUS_VideoDac_eNone
};

static const NEXUS_CompositeOutputSettings NEXUS_CompositeOutputDefaultSettings = {
    NEXUS_VideoDac_eNone
};

static const NEXUS_Ccir656OutputSettings NEXUS_Ccir656OutputDefaultSettings = {
    {0}
};

static BERR_Code NEXUS_ComponentOutput_P_Connect(void *output,  NEXUS_DisplayHandle display);
static BERR_Code NEXUS_CompositeOutput_P_Connect(void *output,  NEXUS_DisplayHandle display);
static BERR_Code NEXUS_SvideoOutput_P_Connect(void *output,  NEXUS_DisplayHandle display);
static BERR_Code NEXUS_Ccir656Output_P_Connect(void *output,  NEXUS_DisplayHandle display);

void
NEXUS_VideoOutputs_P_Init(void)
{
    NEXUS_DisplayModule_State *state = &g_NEXUS_DisplayModule_State;
    unsigned i;
    BSTD_UNUSED(i);
    BSTD_UNUSED(state);
#if NEXUS_NUM_COMPONENT_OUTPUTS
    for(i=0;i<sizeof(state->outputs.component)/sizeof(state->outputs.component[0]);i++) {
        BDBG_OBJECT_INIT(&state->outputs.component[i], NEXUS_ComponentOutput);
        NEXUS_ComponentOutput_GetDefaultSettings(&state->outputs.component[i].cfg);
        state->outputs.component[i].opened = false;
        NEXUS_VIDEO_OUTPUT_INIT(&state->outputs.component[i].output, NEXUS_VideoOutputType_eComponent, &state->outputs.component[i]);
    }
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    for(i=0;i<sizeof(state->outputs.composite)/sizeof(state->outputs.composite[0]);i++) {
        BDBG_OBJECT_INIT(&state->outputs.composite[i], NEXUS_CompositeOutput);
        state->outputs.composite[i].cfg = NEXUS_CompositeOutputDefaultSettings;
        state->outputs.composite[i].opened = false;
        state->outputs.composite[i].dacOutputType = BVDC_DacOutput_eComposite;
        NEXUS_VIDEO_OUTPUT_INIT(&state->outputs.composite[i].output, NEXUS_VideoOutputType_eComposite, &state->outputs.composite[i]);
    }
#endif
#if NEXUS_NUM_SVIDEO_OUTPUTS
    for(i=0;i<sizeof(state->outputs.svideo)/sizeof(state->outputs.svideo[0]);i++) {
        BDBG_OBJECT_INIT(&state->outputs.svideo[i], NEXUS_SvideoOutput);
        state->outputs.svideo[i].cfg = NEXUS_SvideoOutputDefaultSettings;
        state->outputs.svideo[i].opened = false;
        NEXUS_VIDEO_OUTPUT_INIT(&state->outputs.svideo[i].output, NEXUS_VideoOutputType_eSvideo, &state->outputs.svideo[i]);
    }
#endif
#if NEXUS_NUM_656_OUTPUTS
    for(i=0;i<sizeof(state->outputs.ccir656)/sizeof(state->outputs.ccir656[0]);i++) {
        BDBG_OBJECT_INIT(&state->outputs.ccir656[i], NEXUS_Ccir656Output);
        state->outputs.ccir656[i].cfg = NEXUS_Ccir656OutputDefaultSettings;
        state->outputs.ccir656[i].opened = false;
        NEXUS_VIDEO_OUTPUT_INIT(&state->outputs.ccir656[i].output, NEXUS_VideoOutputType_eCcir656, &state->outputs.ccir656[i]);
    }
#endif
    return;
}
#if NEXUS_NUM_HDMI_DVO

#if NEXUS_NUM_HDMI_DVO > 1
#error Currently, only one HDMI output is supported
#endif

#include "nexus_hdmi_dvo.h"
#include "priv/nexus_hdmi_dvo_priv.h"

static void NEXUS_VideoOutput_P_HdmiDvoRateChange_isr(NEXUS_DisplayHandle display, void *pParam)
{
    NEXUS_HdmiDvoHandle hdmiDvo = pParam;
    BDBG_ASSERT(NULL != hdmiDvo);
    BDBG_MSG(("> NEXUS_VideoOutput_P_HdmiDvoRateChange_isr"));
    NEXUS_HdmiDvo_VideoRateChange_isr(hdmiDvo, &display->hdmiDvo.rateInfo);
    BDBG_MSG(("< NEXUS_VideoOutput_P_HdmiDvoRateChange_isr"));
    return;
}

static BERR_Code NEXUS_VideoOutput_P_SetHdmiDvoFormat(void *output, NEXUS_DisplayHandle display, NEXUS_VideoFormat format, NEXUS_DisplayAspectRatio aspectRatio)
{
    BERR_Code rc;
    NEXUS_HdmiDvoHandle hdmiDvo = output;
    BFMT_VideoFmt videoFmt;
    BVDC_Display_HdmiSettings stVdcHdmiSettings;
#if NEXUS_HAS_HDMI_1_3
    NEXUS_HdmiOutputSettings settings;
    BAVC_HDMI_BitsPerPixel colorDepth;
#endif
    BDBG_MSG(("> NEXUS_VideoOutput_P_SetHdmiDvoFormat"));
    BDBG_ASSERT(NULL != hdmiDvo);

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(format, &videoFmt);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiDvo);
    NEXUS_HdmiDvo_GetColorimetry_priv(hdmiDvo,&g_NEXUS_DisplayModule_State.hdmiDvo.colorimetry) ;
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiDvo);

    /* Enable HDMI port in the VDC */
    rc = BVDC_Display_GetHdmiSettings(display->displayVdc, &stVdcHdmiSettings);
    stVdcHdmiSettings.ulPortId      = BVDC_Hdmi_0;
    stVdcHdmiSettings.eMatrixCoeffs = g_NEXUS_DisplayModule_State.hdmiDvo.colorimetry;
    rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &stVdcHdmiSettings);
    if (rc) return BERR_TRACE(rc);

    /* Additional step to configure deep color mode */
#if NEXUS_HAS_HDMI_1_3
    /* Get color depth settings */
    NEXUS_HdmiDvo_GetSettings(hdmiDvo, &settings);
    colorDepth = NEXUS_P_HdmiDvoColorDepth_ToMagnum(settings.colorDepth);
    rc = BVDC_Display_SetHdmiColorDepth(display->displayVdc, colorDepth);
    if (rc) return BERR_TRACE(rc);
#endif

    NEXUS_HdmiDvo_SetDisplayParams_priv(hdmiDvo,videoFmt,g_NEXUS_DisplayModule_State.hdmiDvo.colorimetry,aspectRatio);
    BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
    g_NEXUS_DisplayModule_State.hdmiDvo.display = display;
    g_NEXUS_DisplayModule_State.hdmiDvo.hdmiDvo = hdmiDvo;
    g_NEXUS_DisplayModule_State.hdmiDvo.aspectRatio = aspectRatio;
    g_NEXUS_DisplayModule_State.hdmiDvo.format = format;
    BDBG_MSG(("< NEXUS_VideoOutput_P_SetHdmiDvoFormat"));
    return 0;
}

static NEXUS_Error NEXUS_VideoOutput_P_HdmiDvoFormatChange(void *output, NEXUS_DisplayHandle display, NEXUS_VideoFormat format, NEXUS_DisplayAspectRatio aspectRatio, bool _3dOrientationChange)
{
    NEXUS_Error errCode= NEXUS_SUCCESS;
	BSTD_UNUSED(_3dOrientationChange);
    BDBG_MSG(("> NEXUS_VideoOutput_P_HdmiDvoFormatChange"));
    (void)NEXUS_VideoOutput_P_SetHdmiDvoFormat(output, display, format, aspectRatio);
    BDBG_MSG(("< NEXUS_VideoOutput_P_HdmiDvoFormatChange"));
    return errCode;
}


static NEXUS_Error NEXUS_HdmiDvo_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    BVDC_Display_HdmiSettings stVdcHdmiSettings;
    NEXUS_HdmiDvoHandle hdmiDvo=output;
    NEXUS_Error rc=NEXUS_SUCCESS;
    BDBG_MSG(("> NEXUS_HdmiDvo_P_Disconnect"));
    BDBG_ASSERT(NULL != hdmiDvo);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    BVDC_Display_GetHdmiSettings(display->displayVdc, &stVdcHdmiSettings);
    stVdcHdmiSettings.ulPortId      = BVDC_Hdmi_0;
    stVdcHdmiSettings.eMatrixCoeffs = BAVC_MatrixCoefficients_eUnknown;
    BVDC_Display_SetHdmiSettings(display->displayVdc, &stVdcHdmiSettings);
    BDBG_MSG(("< NEXUS_HdmiDvo_P_Disconnect"));
    return rc;
}

static NEXUS_Error NEXUS_HdmiDvo_P_Connect(void *output,  NEXUS_DisplayHandle display)
{
    NEXUS_Error rc=NEXUS_SUCCESS;
    NEXUS_HdmiDvoHandle hdmiDvo=output;
    BDBG_MSG(("> NEXUS_HdmiDvo_P_Connect"));
    BDBG_ASSERT(NULL != hdmiDvo);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);
    (void)NEXUS_VideoOutput_P_SetHdmiDvoFormat(output, display, display->cfg.format, display->cfg.aspectRatio);

    BKNI_EnterCriticalSection();
    BDBG_ASSERT(NULL == display->hdmiDvo.rateChangeCb_isr);
    display->hdmiDvo.rateChangeCb_isr = NEXUS_VideoOutput_P_HdmiDvoRateChange_isr;
    display->hdmiDvo.pCbParam = hdmiDvo;
    if ( display->hdmiDvo.rateInfoValid )
    {
        NEXUS_VideoOutput_P_HdmiDvoRateChange_isr(display, hdmiDvo);
    }
    BKNI_LeaveCriticalSection();
    BDBG_MSG(("< NEXUS_HdmiDvo_P_Connect"));
    return rc;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenHdmiDvo(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;
    BDBG_MSG(("> NEXUS_VideoOutput_P_OpenHdmiDvo"));
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(output->type == NEXUS_VideoOutputType_eHdmiDvo);
    iface.connect = NEXUS_HdmiDvo_P_Connect;
    iface.disconnect = NEXUS_HdmiDvo_P_Disconnect;
    iface.formatChange = NEXUS_VideoOutput_P_HdmiDvoFormatChange;
    BDBG_MSG(("< NEXUS_VideoOutput_P_OpenHdmiDvo"));
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, false);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eDvo;
    }
    return link;
}

#endif

void
NEXUS_ComponentOutput_GetDefaultSettings(NEXUS_ComponentOutputSettings *pSettings)
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    pSettings->type = NEXUS_ComponentOutputType_eYPrPb;
}

NEXUS_ComponentOutputHandle
NEXUS_ComponentOutput_Open(unsigned index, const NEXUS_ComponentOutputSettings *cfg)
{
    BERR_Code rc;
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_ComponentOutputHandle output;
    NEXUS_DisplayModule_State *state = &g_NEXUS_DisplayModule_State;
    NEXUS_ComponentOutputSettings defaultSettings;

    BSTD_UNUSED(rc);

    if(!cfg) {
        NEXUS_ComponentOutput_GetDefaultSettings(&defaultSettings);
        cfg = &defaultSettings;
    }
    if(index>=sizeof(state->outputs.component)/sizeof(state->outputs.component[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output = &state->outputs.component[index];
    if(output->opened) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output->opened = true;
    output->index = index;
    output->cfg = *cfg;
    NEXUS_OBJECT_REGISTER(NEXUS_VideoOutput, &output->output, Open);
    NEXUS_OBJECT_SET(NEXUS_ComponentOutput, output);
    return output;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(cfg);
    rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    return NULL;
#endif
}

static void
NEXUS_ComponentOutput_P_Finalizer(NEXUS_ComponentOutputHandle output)
{
    NEXUS_DisplayHandle display;
    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, output);

    /* auto-remove from any Display */
    display = NEXUS_VideoOutput_P_GetDisplay(&output->output);
    if (display) {
        NEXUS_Display_RemoveOutput(display, &output->output);
    }

    BDBG_OBJECT_UNSET(output, NEXUS_ComponentOutput);
    output->opened = false;
    return;
}

static void
NEXUS_ComponentOutput_P_Release(NEXUS_ComponentOutputHandle output)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoOutput, &output->output, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_ComponentOutput, NEXUS_ComponentOutput_Close);

NEXUS_VideoOutput
NEXUS_ComponentOutput_GetConnector(NEXUS_ComponentOutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, output);

    return &output->output;
}

void
NEXUS_ComponentOutput_GetSettings(NEXUS_ComponentOutputHandle output,  NEXUS_ComponentOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, output);
    BDBG_ASSERT(pSettings);
    *pSettings = output->cfg;
    return;
}

NEXUS_Error
NEXUS_ComponentOutput_SetSettings(NEXUS_ComponentOutputHandle output, const NEXUS_ComponentOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, output);
    output->cfg = *pSettings;
    if (output->output.destination) {
        NEXUS_Error rc;
        NEXUS_VideoOutput_P_Link *link = output->output.destination;
        NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

        rc = nexus_videooutput_p_connect(link);
        if (rc) return BERR_TRACE(rc);
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return BERR_SUCCESS;
}

void
NEXUS_SvideoOutput_GetDefaultSettings(NEXUS_SvideoOutputSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_SvideoOutputDefaultSettings;
    return;
}

NEXUS_SvideoOutputHandle
NEXUS_SvideoOutput_Open(unsigned index, const NEXUS_SvideoOutputSettings *cfg)
{
    BERR_Code rc;
#if NEXUS_NUM_SVIDEO_OUTPUTS
    NEXUS_SvideoOutputHandle  output;
    NEXUS_DisplayModule_State *state = &g_NEXUS_DisplayModule_State;
    BSTD_UNUSED(rc);

    if(!cfg) {
        cfg  = &NEXUS_SvideoOutputDefaultSettings;
    }
    if(index>=sizeof(state->outputs.svideo)/sizeof(state->outputs.svideo[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output = &state->outputs.svideo[index];
    if(output->opened) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output->opened = true;
    output->cfg = *cfg;
    NEXUS_OBJECT_REGISTER(NEXUS_VideoOutput, &output->output, Open);
    NEXUS_OBJECT_SET(NEXUS_SvideoOutput, output);
    return output;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(cfg);
    rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    return NULL;
#endif
}

static void
NEXUS_SvideoOutput_P_Finalizer(NEXUS_SvideoOutputHandle output)
{
    NEXUS_DisplayHandle display;
    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, output);

    /* auto-remove from any Display */
    display = NEXUS_VideoOutput_P_GetDisplay(&output->output);
    if (display) {
        NEXUS_Display_RemoveOutput(display, &output->output);
    }

    BDBG_OBJECT_UNSET(output, NEXUS_SvideoOutput);
    output->opened = false;
    return;
}

static void
NEXUS_SvideoOutput_P_Release(NEXUS_SvideoOutputHandle output)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoOutput, &output->output, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SvideoOutput, NEXUS_SvideoOutput_Close);

NEXUS_VideoOutput
NEXUS_SvideoOutput_GetConnector(NEXUS_SvideoOutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, output);
    return &output->output;
}

void
NEXUS_SvideoOutput_GetSettings(NEXUS_SvideoOutputHandle output,  NEXUS_SvideoOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, output);
    BDBG_ASSERT(pSettings);
    *pSettings = output->cfg;
    return;
}

NEXUS_Error
NEXUS_SvideoOutput_SetSettings(NEXUS_SvideoOutputHandle output, const NEXUS_SvideoOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, output);
    output->cfg = *pSettings;
    if (output->output.destination) {
        NEXUS_Error rc;
        NEXUS_VideoOutput_P_Link *link = output->output.destination;
        NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

        rc = nexus_videooutput_p_connect(link);
        if (rc) return BERR_TRACE(rc);
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return BERR_SUCCESS;
}

void
NEXUS_CompositeOutput_GetDefaultSettings(NEXUS_CompositeOutputSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_CompositeOutputDefaultSettings;
    return;
}


NEXUS_CompositeOutputHandle
NEXUS_CompositeOutput_Open(unsigned index, const NEXUS_CompositeOutputSettings *cfg)
{
    BERR_Code rc;
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_CompositeOutputHandle  output;
    NEXUS_DisplayModule_State *state = &g_NEXUS_DisplayModule_State;
    BSTD_UNUSED(rc);

    if(!cfg) {
        cfg  = &NEXUS_CompositeOutputDefaultSettings;
    }

    if(index>=sizeof(state->outputs.composite)/sizeof(state->outputs.composite[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output = &state->outputs.composite[index];
    if(output->opened) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output->opened = true;
    output->cfg = *cfg;
    NEXUS_OBJECT_REGISTER(NEXUS_VideoOutput, &output->output, Open);
    NEXUS_OBJECT_SET(NEXUS_CompositeOutput, output);
    return output;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(cfg);
    rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    return NULL;
#endif
}

static void
NEXUS_CompositeOutput_P_Finalizer(NEXUS_CompositeOutputHandle output)
{
    NEXUS_DisplayHandle display;
    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, output);

    /* auto-remove from any Display */
    display = NEXUS_VideoOutput_P_GetDisplay(&output->output);
    if (display) {
        NEXUS_Display_RemoveOutput(display, &output->output);
    }

    BDBG_OBJECT_UNSET(output, NEXUS_CompositeOutput);
    output->opened = false;
    return;
}

static void
NEXUS_CompositeOutput_P_Release(NEXUS_CompositeOutputHandle output)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoOutput, &output->output, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_CompositeOutput, NEXUS_CompositeOutput_Close);

NEXUS_VideoOutput
NEXUS_CompositeOutput_GetConnector(NEXUS_CompositeOutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, output);
    return &output->output;
}

void
NEXUS_CompositeOutput_GetSettings(NEXUS_CompositeOutputHandle output,  NEXUS_CompositeOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, output);
    BDBG_ASSERT(pSettings);
    *pSettings = output->cfg;
    return;
}

NEXUS_Error
NEXUS_CompositeOutput_SetSettings(NEXUS_CompositeOutputHandle output, const NEXUS_CompositeOutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, output);
    output->cfg = *pSettings;
    if (output->output.destination) {
        NEXUS_Error rc;
        NEXUS_VideoOutput_P_Link *link = output->output.destination;
        NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

        rc = nexus_videooutput_p_connect(link);
        if (rc) return BERR_TRACE(rc);
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return BERR_SUCCESS;
}

void
NEXUS_Ccir656Output_GetDefaultSettings(NEXUS_Ccir656OutputSettings *pSettings)
{
    BDBG_ASSERT(pSettings);
    *pSettings = NEXUS_Ccir656OutputDefaultSettings;
    return;
}


NEXUS_Ccir656OutputHandle
NEXUS_Ccir656Output_Open(unsigned index, const NEXUS_Ccir656OutputSettings *cfg)
{
    BERR_Code rc;
#if NEXUS_NUM_656_OUTPUTS
    NEXUS_Ccir656OutputHandle  output;
    NEXUS_DisplayModule_State *state = &g_NEXUS_DisplayModule_State;
    BSTD_UNUSED(rc);

    if(!cfg) {
        cfg  = &NEXUS_Ccir656OutputDefaultSettings;
    }
    if(index>=sizeof(state->outputs.ccir656)/sizeof(state->outputs.ccir656[0])) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output = &state->outputs.ccir656[index];
    if(output->opened) {
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        return NULL;
    }
    output->opened = true;
    output->cfg = *cfg;
    NEXUS_OBJECT_REGISTER(NEXUS_VideoOutput, &output->output, Open);
    NEXUS_OBJECT_SET(NEXUS_Ccir656Output, output);
    return output;
#else
    BSTD_UNUSED(index);
    BSTD_UNUSED(cfg);
    rc = BERR_TRACE(BERR_INVALID_PARAMETER);
    return NULL;
#endif
}

static void
NEXUS_Ccir656Output_P_Finalizer(NEXUS_Ccir656OutputHandle output)
{
    NEXUS_DisplayHandle display;
    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, output);

    /* auto-remove from any Display */
    display = NEXUS_VideoOutput_P_GetDisplay(&output->output);
    if (display) {
        NEXUS_Display_RemoveOutput(display, &output->output);
    }

    BDBG_OBJECT_UNSET(output, NEXUS_Ccir656Output);
    output->opened = false;
    return;
}

static void
NEXUS_Ccir656Output_P_Release(NEXUS_Ccir656OutputHandle output)
{
    NEXUS_OBJECT_UNREGISTER(NEXUS_VideoOutput, &output->output, Close);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_Ccir656Output, NEXUS_Ccir656Output_Close);

NEXUS_VideoOutput
NEXUS_Ccir656Output_GetConnector(NEXUS_Ccir656OutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, output);
    return &output->output;
}

void
NEXUS_Ccir656Output_GetSettings(NEXUS_Ccir656OutputHandle output,  NEXUS_Ccir656OutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, output);
    BDBG_ASSERT(pSettings);
    *pSettings = output->cfg;
    return;
}

NEXUS_Error
NEXUS_Ccir656Output_SetSettings(NEXUS_Ccir656OutputHandle output, const NEXUS_Ccir656OutputSettings *pSettings)
{
    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, output);
    output->cfg = *pSettings;
    if (output->output.destination) {
        NEXUS_Error rc;
        NEXUS_VideoOutput_P_Link *link = output->output.destination;
        NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

        rc = nexus_videooutput_p_connect(link);
        if (rc) return BERR_TRACE(rc);
        rc = NEXUS_Display_P_ApplyChanges();
        if (rc) return BERR_TRACE(rc);
    }
    return BERR_SUCCESS;
}

BDBG_OBJECT_ID(NEXUS_VideoOutput_P_Link);

static unsigned nexus_videooutput_p_convertdac(NEXUS_VideoDac dac)
{
    unsigned dacVdc;
    switch(dac) {
    default:
    case NEXUS_VideoDac_eNone:
        dacVdc = 0;
        break;
    case NEXUS_VideoDac_e0:
        dacVdc = BVDC_Dac_0;
        break;
    case NEXUS_VideoDac_e1:
        dacVdc = BVDC_Dac_1;
        break;
    case NEXUS_VideoDac_e2:
        dacVdc = BVDC_Dac_2;
        break;
    case NEXUS_VideoDac_e3:
        dacVdc = BVDC_Dac_3;
        break;
    case NEXUS_VideoDac_e4:
        dacVdc = BVDC_Dac_4;
        break;
    case NEXUS_VideoDac_e5:
        dacVdc = BVDC_Dac_5;
        break;
    case NEXUS_VideoDac_e6:
        dacVdc = BVDC_Dac_6;
        break;
    }
    return dacVdc;
}

/* Any display with component/composite/svideo/rfm output is an "analog display" and requires analog HW resources.
Component requires a separate analog HW resource; composite/svideo/rfm on the same display share the same resource.
These may not exist on HW-limited systems. After adding up all current analog resources,
can this display support the new analog output? If not, Nexus will print a BDBG_WRN and return NEXUS_SUCCESS.
This allows generic test apps to keep working on these systems.
This code does not re-enable a previously muted analog output when an analog output on another display is disabled. */
static bool nexus_videooutput_p_allow_analog_display(NEXUS_VideoOutputType outputType, NEXUS_DisplayHandle display)
{
    unsigned d, num = 0;
    for (d=0;d<sizeof(g_NEXUS_DisplayModule_State.displays)/sizeof(g_NEXUS_DisplayModule_State.displays[0]);d++) {
        bool analog[2] = {false, false};
        NEXUS_VideoOutput_P_Link *otherLink;
        NEXUS_DisplayHandle otherDisplay = g_NEXUS_DisplayModule_State.displays[d];
        if (!otherDisplay || otherDisplay->timingGenerator == NEXUS_DisplayTimingGenerator_eEncoder) continue;
        for (otherLink=BLST_D_FIRST(&otherDisplay->outputs);otherLink;otherLink=BLST_D_NEXT(otherLink, link)) {
            switch (otherLink->output->type) {
            case NEXUS_VideoOutputType_eComponent:
                analog[0] = true;
                break;
            case NEXUS_VideoOutputType_eSvideo:
            case NEXUS_VideoOutputType_eComposite:
            case NEXUS_VideoOutputType_eRfm:
                analog[1] = true;
                break;
            default:
                break;
            }
        }
        if (display == otherDisplay) {
            analog[(outputType == NEXUS_VideoOutputType_eComponent) ? 0 : 1] = true;
        }

        if (!analog[0] && !analog[1] && !g_NEXUS_DisplayModule_State.vdcCapabilities.bAlgStandAlone) {
            /* if !bAlgStandAlone then "no analog" still consumes an analog */
            num++;
        }
        else {
            if (analog[0]) num++;
            if (analog[1]) num++;
        }
    }
    if (num > g_NEXUS_DisplayModule_State.vdcCapabilities.ulNumAlgPaths) {
        BDBG_WRN(("muting %s output because of limited analog paths", g_videoOutputStr[outputType]));
        return false;
    }
    else {
        return true;
    }
}

static BERR_Code
NEXUS_VideoOutput_P_SetDac(NEXUS_VideoOutput_P_Link *link, NEXUS_DisplayHandle display, bool connect)
{
    BERR_Code rc;
    unsigned i, j;

    if ((connect == false) && (link->dacsConnected == false))
		return NEXUS_SUCCESS;

    if (connect && !nexus_videooutput_p_allow_analog_display(link->output->type, display)) return NEXUS_SUCCESS;

    if (connect) {
        /* Search all other outputs on the display. If a dac is already in use it's likely a DAC-limited chip (for example,
        only 3 DAC's shared between component and composite).
        We don't want to fail and require all apps to change, and we don't want to have to choose component or composite at compile time.
        Instead, issue a BDBG_WRN and return NEXUS_SUCCESS. Give preference to component. */
        NEXUS_VideoOutput_P_Link *otherLink;
        unsigned d;
        for (d=0;d<sizeof(g_NEXUS_DisplayModule_State.displays)/sizeof(g_NEXUS_DisplayModule_State.displays[0]);d++) {
            if (!g_NEXUS_DisplayModule_State.displays[d]) continue;
            for (otherLink=BLST_D_FIRST(&g_NEXUS_DisplayModule_State.displays[d]->outputs);otherLink;otherLink=BLST_D_NEXT(otherLink, link)) {
                if (otherLink == link || !otherLink->dacsConnected) continue;
                for (i=0;i<NEXUS_P_MAX_DACS;i++) {
                    if (!otherLink->dac[i].dac) continue;
                    for (j=0;j<NEXUS_P_MAX_DACS;j++) {
                        if (otherLink->dac[i].dac == link->dac[j].dac) {
                            NEXUS_VideoOutput_P_Link *muteLink = link;
                            /* give preference to component */
                            if (link->output->type == NEXUS_VideoOutputType_eComponent) {
                                NEXUS_VideoOutput_P_SetDac(otherLink, otherLink->display, false);
                                muteLink = otherLink;
                                rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
                                if (rc) {rc = BERR_TRACE(rc);} /* report */
                            }
                            BDBG_WRN(("DAC %u already in use. Muting %s output.", otherLink->dac[i].dac - NEXUS_VideoDac_e0,
                                g_videoOutputStr[muteLink->output->type]));
                            if (link->output->type != NEXUS_VideoOutputType_eComponent) {
                                return NEXUS_SUCCESS;
                            }
                            else {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    for (i=0;i<NEXUS_P_MAX_DACS;i++) {
        if (!link->dac[i].dac) continue;
        BDBG_MSG(("%s DAC %d for output %s on display%d", connect?"set":"unset", link->dac[i].dac - NEXUS_VideoDac_e0, g_videoOutputStr[link->output->type],
            display->index));
        rc = BVDC_Display_SetDacConfiguration(display->displayVdc, nexus_videooutput_p_convertdac(link->dac[i].dac), connect ? link->dac[i].type : BVDC_DacOutput_eUnused);
        if (rc) {
            /* unwind so that the function is transactional */
            for (j=0;j<i;j++) {
                (void)BVDC_Display_SetDacConfiguration(display->displayVdc, nexus_videooutput_p_convertdac(link->dac[j].dac), BVDC_DacOutput_eUnused);
            }
            return BERR_TRACE(rc);
        }
    }
    link->dacsConnected = connect;
    return NEXUS_SUCCESS;
}

static BERR_Code
NEXUS_CompositeOutput_P_Connect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_CompositeOutputHandle composite=output;
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, composite);
    link = composite->output.destination;

    BKNI_Memset(link->dac, 0, sizeof(link->dac));
    link->dac[0].dac = composite->cfg.dac;
    link->dac[0].type = composite->dacOutputType;

    rc = NEXUS_VideoOutput_P_SetDac(link, display, true);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); }
    return rc;
}

static BERR_Code
NEXUS_CompositeOutput_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_CompositeOutputHandle composite = output;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, composite);
    link = composite->output.destination;

    rc = NEXUS_VideoOutput_P_SetDac(link, display, false);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); }
    return rc;
}

static BERR_Code
NEXUS_SvideoOutput_P_Connect(void *output,  NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_SvideoOutputHandle svideo=output;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, svideo);
    link = svideo->output.destination;

    BKNI_Memset(link->dac, 0, sizeof(link->dac));
    link->dac[0].dac = svideo->cfg.dacY;
    link->dac[0].type = BVDC_DacOutput_eSVideo_Luma;
    link->dac[1].dac = svideo->cfg.dacC;
    link->dac[1].type = BVDC_DacOutput_eSVideo_Chroma;

    rc = NEXUS_VideoOutput_P_SetDac(link, display, true);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_dac; }
err_dac:
    return rc;
}

static BERR_Code
NEXUS_SvideoOutput_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_SvideoOutputHandle svideo=output;
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, svideo);
    link = svideo->output.destination;

    rc = NEXUS_VideoOutput_P_SetDac(link, display, false);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_dac; }
err_dac:
    return rc;
}

static BERR_Code
NEXUS_ComponentOutput_P_Connect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_ComponentOutputHandle component=output;
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, component);
    link = component->output.destination;

    BKNI_Memset(link->dac, 0, sizeof(link->dac));
    switch(component->cfg.type) {
    case NEXUS_ComponentOutputType_eYPrPb:
        link->dac[0].dac = component->cfg.dacs.YPrPb.dacY;
        link->dac[0].type = BVDC_DacOutput_eY;
        link->dac[1].dac = component->cfg.dacs.YPrPb.dacPr;
        link->dac[1].type = BVDC_DacOutput_ePr;
        link->dac[2].dac = component->cfg.dacs.YPrPb.dacPb;
        link->dac[2].type = BVDC_DacOutput_ePb;
        break;
    case NEXUS_ComponentOutputType_eRGB:
        link->dac[0].dac = component->cfg.dacs.RGB.dacGreen;
        link->dac[0].type = (component->cfg.dacs.RGB.dacHSync != NEXUS_VideoDac_eNone || component->cfg.dacs.RGB.noSync) ? BVDC_DacOutput_eGreen_NoSync : BVDC_DacOutput_eGreen;
        link->dac[1].dac = component->cfg.dacs.RGB.dacRed;
        link->dac[1].type = BVDC_DacOutput_eRed;
        link->dac[2].dac = component->cfg.dacs.RGB.dacBlue;
        link->dac[2].type = BVDC_DacOutput_eBlue;
        if (component->cfg.dacs.RGB.dacHSync != NEXUS_VideoDac_eNone && !component->cfg.dacs.RGB.noSync) {
            link->dac[3].dac = component->cfg.dacs.RGB.dacHSync;
            link->dac[3].type = BVDC_DacOutput_eHsync;
        }
        break;
    default:
        rc = BERR_TRACE(BERR_INVALID_PARAMETER);
        goto err_dac;
    }
    rc = NEXUS_VideoOutput_P_SetDac(link, display, true);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_dac; }
    rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eComponent, 1 << component->index, component->cfg.mpaaDecimationEnabled);
    if (rc) return BERR_TRACE(rc);

err_dac:
    return rc;
}


static BERR_Code
NEXUS_ComponentOutput_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_ComponentOutputHandle component=output;
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, component);
    link = component->output.destination;
    if (component->cfg.mpaaDecimationEnabled) {
        component->cfg.mpaaDecimationEnabled = false;
        rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eComponent, 1 << component->index, component->cfg.mpaaDecimationEnabled);
        if (rc) {
            rc = BERR_TRACE(rc); /* fall through */
        }
        else {
            /* two ApplyChanges are required: this one and the standard one */
            rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
            if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        }
    }

    rc = NEXUS_VideoOutput_P_SetDac(link, display, false);
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); goto err_dac; }

err_dac:
    return rc;
}

static BERR_Code
NEXUS_Ccir656Output_P_Connect(void *output,  NEXUS_DisplayHandle display)
{
    NEXUS_Ccir656OutputHandle ccir656=output;
    BERR_Code rc = BERR_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, ccir656);

    rc = BVDC_Display_Set656Configuration(display->displayVdc, BVDC_Itur656Output_0, true); /* enable 656 out */
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); }

    return rc;
}

static BERR_Code
NEXUS_Ccir656Output_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_Ccir656OutputHandle ccir656=output;
    BERR_Code rc = BERR_SUCCESS;

    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, ccir656);

    rc = BVDC_Display_Set656Configuration(display->displayVdc, BVDC_Itur656Output_0, false); /* disable 656 out */
    if(rc!=BERR_SUCCESS) { rc=BERR_TRACE(rc); }

    return rc;
}

NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_CreateLink(NEXUS_VideoOutputHandle output, const NEXUS_VideoOutput_P_Iface *iface, bool sdOnly)
{
    NEXUS_VideoOutput_P_Link *link;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(iface);

    link = BKNI_Malloc(sizeof(*link));
    if(!link) {
        BERR_Code rc = BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
        BSTD_UNUSED(rc);
        goto err_alloc;
    }
    BKNI_Memset(link, 0, sizeof(*link));
    BDBG_OBJECT_SET(link, NEXUS_VideoOutput_P_Link);
    link->iface = *iface;
    link->output = output;
    link->display = NULL;
    link->sdOnly = sdOnly;
    link->displayOutput = 0; /* no unknown value */
    link->connected = false;
    output->destination = link;
    return link;
err_alloc:
    return NULL;
}

/* this function is called from NEXUS_Display_Close and removes the link from the list */
void
NEXUS_VideoOutput_P_DestroyLink(NEXUS_VideoOutput_P_Link *link)
{
    BDBG_OBJECT_DESTROY(link, NEXUS_VideoOutput_P_Link);
    BKNI_Free(link);
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenComposite(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;
    NEXUS_CompositeOutputHandle compositeOutput = (NEXUS_CompositeOutputHandle)output->source;

    NEXUS_OBJECT_ASSERT(NEXUS_CompositeOutput, compositeOutput);
    iface.connect = NEXUS_CompositeOutput_P_Connect;
    iface.disconnect = NEXUS_CompositeOutput_P_Disconnect;
    iface.formatChange = NULL;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, true);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eComposite;
    }
    return link;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenSvideo(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;

    NEXUS_OBJECT_ASSERT(NEXUS_SvideoOutput, (NEXUS_SvideoOutputHandle)output->source);
    iface.connect = NEXUS_SvideoOutput_P_Connect;
    iface.disconnect = NEXUS_SvideoOutput_P_Disconnect;
    iface.formatChange = NULL;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, true);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eSVideo;
    }
    return link;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenComponent(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;

    NEXUS_OBJECT_ASSERT(NEXUS_ComponentOutput, (NEXUS_ComponentOutputHandle)output->source);
    iface.connect = NEXUS_ComponentOutput_P_Connect;
    iface.disconnect = NEXUS_ComponentOutput_P_Disconnect;
    iface.formatChange = NULL;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, false);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eComponent;
    }
    return link;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenCcir656(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;
    NEXUS_OBJECT_ASSERT(NEXUS_Ccir656Output, (NEXUS_Ccir656OutputHandle)output->source);
    iface.connect = NEXUS_Ccir656Output_P_Connect;
    iface.disconnect = NEXUS_Ccir656Output_P_Disconnect;
    iface.formatChange = NULL;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, true);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_e656;
    }
    return link;
}

#if NEXUS_HAS_HDMI_OUTPUT
#include "priv/nexus_hdmi_output_priv.h"

static void
NEXUS_VideoOutput_P_HdmiRateChange_isr(NEXUS_DisplayHandle display, void *pParam)
{
    NEXUS_HdmiOutputHandle hdmi = pParam;
    BDBG_ASSERT(NULL != hdmi);
    NEXUS_HdmiOutput_VideoRateChange_isr(hdmi, &display->hdmi.rateInfo);
}

static BERR_Code
NEXUS_VideoOutput_P_HdmiNum_ToMagnum(unsigned index, uint32_t *ulHdmi)
{
    BERR_Code rc = BERR_SUCCESS;
    switch(index) {
    case 0: *ulHdmi = BVDC_Hdmi_0; break;
#if NEXUS_NUM_HDMI_OUTPUTS > 1
    case 1: *ulHdmi = BVDC_Hdmi_1; break;
#endif
    default: rc = BERR_TRACE(BERR_INVALID_PARAMETER); break;
    }
    return rc;
}


/* Set HDMI settings to a legal default configuration; avoids errors due to changed formats */
static void NEXUS_VideoOutput_P_SetDefaultHdmiSettings(NEXUS_DisplayHandle display)
{
    BVDC_Display_HdmiSettings displayHdmiSettings ;
    BERR_Code rc ;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BVDC_Display_GetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
    /* disable data to HDMI core */
    displayHdmiSettings.ulPortId      = display->hdmi.vdcIndex;
    displayHdmiSettings.eMatrixCoeffs = BAVC_MatrixCoefficients_eUnknown;

    /* color depth */
    rc = BVDC_Display_SetHdmiColorDepth(display->displayVdc, BAVC_HDMI_BitsPerPixel_e24bit) ;
    if (rc) {
        BERR_TRACE(rc) ;
        BVDC_AbortChanges(video->vdc) ;
        goto done ;
    }

    /* color space */
    displayHdmiSettings.eColorComponent = BAVC_Colorspace_eYCbCr444 ;
    rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
    if (rc) {
        BERR_TRACE(rc) ;
        BVDC_AbortChanges(video->vdc) ;
        goto done ;
    }

    rc = BVDC_ApplyChanges(video->vdc);

done:
    return ;
}

NEXUS_Error NEXUS_VideoOutput_P_SetHdrSettings(void *output,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInputDrmInfoFrame)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHandle hdmiOutput = output;

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        rc = NEXUS_HdmiOutput_SetInputDrmInfoFrame_priv(hdmiOutput, pInputDrmInfoFrame);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    if (rc) { BERR_TRACE(rc); goto error; }

error:
    return rc;
}

/*
three entry points:
1) NEXUS_HdmiOutput_SetSettings
        where display->cfg is unchanged, called via notifyDisplay callback then NEXUS_VideoOutput_P_SetHdmiSettings
2) NEXUS_Display_AddOutput
        see NEXUS_VideoOutput_P_ConnectHdmi,
        also called via NEXUS_VideoOutput_P_SetHdmiSettings but with a forced format change
3) NEXUS_Display_SetSettings
        where NEXUS_HdmiOutput_SetSettings is unchanged, but new format is passed as params
*/
static BERR_Code
NEXUS_VideoOutput_P_ApplyHdmiSettings(void *output, NEXUS_DisplayHandle display,
    /* the following params could be current or new display settings. only use display->cfg to test for change. */
    NEXUS_VideoFormat format, NEXUS_DisplayAspectRatio aspectRatio, bool _3dOrientationChange)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmiOutput = output;
    NEXUS_HdmiOutputSettings settings;

    BFMT_VideoFmt videoFmt, hdmiFmt;
    BFMT_AspectRatio aspectRatioVdc;
    bool hdmiFormatChange;
    BVDC_Display_DvoSettings dvoSettings;
    bool reconnect_outputs = false;
    BAVC_MatrixCoefficients magnumMatrixCoefficients;
    NEXUS_MatrixCoefficients nexusMatrixCoefficients;
    BAVC_ColorRange magnumColorRange;
    NEXUS_ColorRange nexusColorRange;
    bool colorimetry_supported;
    BVDC_Display_HdmiSettings displayHdmiSettings ;
    NEXUS_VideoEotf eotf;
    bool doneHdmiSettings = false;

    NEXUS_HdmiOutputVideoSettings requested ;
    NEXUS_HdmiOutputVideoSettings preferred ;
    bool overrideRequestedSettings = false ;
    NEXUS_CallbackDesc notifyDisplay;

    if (display->hdmi.rateChangeCb_isr == NULL)
    {
        BDBG_MSG(("HDMI output is disconnected from display. Settings not applied to HDMI device")) ;
        return NEXUS_SUCCESS;
    }
    BDBG_ASSERT(display->hdmi.outputNotify == hdmiOutput);

    NEXUS_HdmiOutput_GetSettings(hdmiOutput, &settings);

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        NEXUS_HdmiOutput_ReadFormatChangeStatus_priv(hdmiOutput, &hdmiFormatChange); /* read and clear */

    if (display->hdmi.forceFormatChange) {
        hdmiFormatChange = true;
        display->hdmi.forceFormatChange = false;
    }
    else if (format != display->cfg.format || aspectRatio != display->cfg.aspectRatio || _3dOrientationChange) {
        hdmiFormatChange = true;
    }

    if (settings.overrideMatrixCoefficients)
    {
        /* overrride supported colorimetry with requested colorimetry */
        colorimetry_supported = true ;
        BDBG_MSG(("User Override of default colorimetry with colorimetry matrix %d",
            settings.matrixCoefficients)) ;
        magnumMatrixCoefficients = NEXUS_P_MatrixCoefficients_ToMagnum_isrsafe(settings.matrixCoefficients) ;
        if (magnumMatrixCoefficients == BAVC_MatrixCoefficients_eUnknown)
        {
            BDBG_ERR(("Override colorimetry %d is UNKNOWN; Default to BT 709",
                settings.matrixCoefficients)) ;
            magnumMatrixCoefficients = BAVC_MatrixCoefficients_eItu_R_BT_709 ;
        }
    }
    else
    {
        NEXUS_HdmiOutputColorimetryParameters colorimetryParameters ;

        BKNI_Memset(&colorimetryParameters, 0, sizeof(NEXUS_HdmiOutputColorimetryParameters)) ;

        colorimetryParameters.xvYccEnabled = display->cfg.xvYccEnabled ;
        colorimetryParameters.format = format ;

        /* NEXUS_HdmiOutput_GetColorimetry_priv returns bool for connected */
        colorimetry_supported = NEXUS_HdmiOutput_GetColorimetry_priv(hdmiOutput,
                &colorimetryParameters, &magnumMatrixCoefficients) ;
    }

    NEXUS_HdmiOutput_GetEotf_priv(hdmiOutput, &eotf);

    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);

    /* convert Coefficients from magnum to nexus for use in the remainder of function */
    nexusMatrixCoefficients =
        NEXUS_P_MatrixCoefficients_FromMagnum_isrsafe(magnumMatrixCoefficients) ;

    BDBG_MSG(("hdmi Format Change: %s Colorimetry Supported: %s  Colorimetry: %d",
        hdmiFormatChange ? "Yes" : "No",
        colorimetry_supported ? "Yes" : "No", nexusMatrixCoefficients)) ;

    if (settings.overrideColorRange)
    {
        /* overrride supported colorRange with requested colorRange */
        BDBG_MSG(("User Override of default colorRange with colorRange %d", settings.colorRange)) ;
        magnumColorRange = NEXUS_P_ColorRange_ToMagnum_isrsafe(settings.colorRange) ;
    }
    else
    {
        magnumColorRange =
               ((magnumMatrixCoefficients == BAVC_MatrixCoefficients_eHdmi_Full_Range_YCbCr)
            || (magnumMatrixCoefficients == BAVC_MatrixCoefficients_eDvi_Full_Range_RGB))
            ? BAVC_ColorRange_eFull : BAVC_ColorRange_eLimited;
    }
    nexusColorRange = NEXUS_P_ColorRange_FromMagnum_isrsafe(magnumColorRange) ;

    /* certain settings, like color depth and color space, ought to trigger a format change */
    if (hdmiFormatChange) {
        NEXUS_HdmiOutputStatus *hdmiOutputStatus;
        bool aspectRatioChangeOnly;
        bool hdmiMasterMode ;
        hdmiOutputStatus = &g_NEXUS_DisplayModule_State.functionData.NEXUS_VideoOutput_P_SetHdmiFormat.hdmiOutputStatus;
        rc = NEXUS_HdmiOutput_GetStatus(hdmiOutput, hdmiOutputStatus);
        if (rc) return BERR_TRACE(rc);

        /* If there is no HDMI Rx connected to the Tx; disable the HDMI Output */
        /* reset VDC HDMI settings to values compatible with analog settings */

        /* 7563[5] has a clockgen bug which requires fixed usage of displays and timing generators. */
        /* This is handled using the DISPLAY_OPEN_REQUIRES_HDMI_OUTPUT define and g_NEXUS_DisplayModule_State.requiredOutput. */
        /* Calling this on start up on 7563[5] causes the video window creation to fail, or errors when we pretend to remove HDMI. */

        /* Also do not reset the matrixCoefficients if HDMI is in master mode */
        hdmiMasterMode = display->timingGenerator == NEXUS_DisplayTimingGenerator_eHdmiDvo ;

        if (!hdmiOutputStatus->connected
        && !g_NEXUS_DisplayModule_State.requiredOutputSystem && !hdmiMasterMode)
        {
            NEXUS_VideoOutput_P_SetDefaultHdmiSettings(display) ;
            BDBG_WRN(("No HDMI Rx connected.. settings will not be applied")) ;

            /* HDMI settings will not be updated, but continue to disable data to HDMI */
            /* new HDMI settings will be applied when Rx device is connected  */

            magnumMatrixCoefficients = BAVC_MatrixCoefficients_eUnknown ;
        }

        if (hdmiOutputStatus->connected)
        {
            BAVC_HDMI_BitsPerPixel colorDepth;

            /* If only aspect ratio is changing, set the aspectRatioChangeOnly flag for later use in hdmi_output module*/
            aspectRatioChangeOnly = ((display->cfg.aspectRatio != aspectRatio) && (display->cfg.format == format) && (!_3dOrientationChange));
            NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
            rc = NEXUS_HdmiOutput_P_PreFormatChange_priv(hdmiOutput, aspectRatioChangeOnly);
            NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
            if (rc) {
                rc = BERR_TRACE(rc);
                /* Keep going, they asked for it... */
            }

            if (!colorimetry_supported)
            {
                BDBG_WRN(("Requested video format/colorimetry %d/%d is not supported by the attached receiver",
                    format, nexusMatrixCoefficients)) ;
                /* Keep going, they asked for it... */
            }

            /* Additional step to configure deep color mode */
            requested.videoFormat = format ;
            requested.colorDepth = settings.colorDepth ;
            requested.colorSpace = settings.colorSpace ;
            BKNI_Memset(&preferred, 0, sizeof(NEXUS_HdmiOutputVideoSettings)) ;

            NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
            rc = NEXUS_HdmiOutput_ValidateVideoSettings_priv(hdmiOutput, &requested, &preferred) ;
            NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
            if (rc)
            {
                BDBG_WRN(("Unable to validate Color Space/Color Depth Settings; No Change")) ;
                return rc ;
            }

            overrideRequestedSettings =
                (requested.videoFormat != preferred.videoFormat)
            || (requested.colorSpace != preferred.colorSpace)
            || (requested.colorDepth != preferred.colorDepth) ;

            colorDepth = NEXUS_P_HdmiColorDepth_ToMagnum_isrsafe(preferred.colorDepth);
            rc = BVDC_Display_SetHdmiColorDepth(display->displayVdc, colorDepth);
            if (rc) return BERR_TRACE(rc);

            BVDC_Display_GetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
            displayHdmiSettings.ulPortId      = display->hdmi.vdcIndex;
            displayHdmiSettings.eMatrixCoeffs = magnumMatrixCoefficients;
            displayHdmiSettings.eColorComponent = NEXUS_P_ColorSpace_ToMagnum_isrsafe(preferred.colorSpace) ;
            displayHdmiSettings.eColorRange = magnumColorRange;
            displayHdmiSettings.eEotf = NEXUS_P_VideoEotf_ToMagnum_isrsafe(eotf);
            rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
            if (rc) return BERR_TRACE(rc);
            doneHdmiSettings = true;
        }
    }

    rc = NEXUS_P_DisplayAspectRatio_ToMagnum(aspectRatio, format, &aspectRatioVdc);
    if (rc) {
        aspectRatioVdc = BFMT_AspectRatio_eUnknown; /* don't proceed with uninitialized value. */
    }

    rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(format, &videoFmt);
    if (rc) {
        videoFmt = BFMT_VideoFmt_eNTSC; /* don't proceed with uninitialized value. */
    }

    if (!doneHdmiSettings) {
        BVDC_Display_GetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
        displayHdmiSettings.ulPortId      = display->hdmi.vdcIndex;
        displayHdmiSettings.eMatrixCoeffs = magnumMatrixCoefficients;
        displayHdmiSettings.eColorRange   = magnumColorRange;
        displayHdmiSettings.eEotf         = NEXUS_P_VideoEotf_ToMagnum_isrsafe(eotf);
        rc = BVDC_Display_SetHdmiSettings(display->displayVdc, &displayHdmiSettings) ;
        if (rc) return BERR_TRACE(rc);
    }

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    {
        NEXUS_HdmiOutputDisplaySettings stHdmiOutputDisplaySettings ;
        rc = NEXUS_HdmiOutput_GetDisplaySettings_priv(hdmiOutput, &stHdmiOutputDisplaySettings) ;
        if (rc) return BERR_TRACE(rc);

        if (overrideRequestedSettings) {
            stHdmiOutputDisplaySettings.colorDepth = preferred.colorDepth ;
            stHdmiOutputDisplaySettings.colorSpace = preferred.colorSpace ;
        }
        stHdmiOutputDisplaySettings.overrideMatrixCoefficients = settings.overrideMatrixCoefficients ;
        stHdmiOutputDisplaySettings.eColorimetry = nexusMatrixCoefficients ;
        stHdmiOutputDisplaySettings.overrideColorRange = settings.overrideColorRange ;
        stHdmiOutputDisplaySettings.colorRange = nexusColorRange ;

        NEXUS_HdmiOutput_SetDisplaySettings_priv(hdmiOutput, &stHdmiOutputDisplaySettings) ;
    }
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);

    rc = BVDC_Display_SetHdmiSyncOnly(display->displayVdc, settings.syncOnly);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eHdmi, 1, settings.mpaaDecimationEnabled);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    BVDC_Display_GetDvoConfiguration(display->displayVdc, &dvoSettings);
    dvoSettings.stSpreadSpectrum.bEnable = settings.spreadSpectrum.enable;
    dvoSettings.stSpreadSpectrum.ulFrequency  = settings.spreadSpectrum.frequency ;
    dvoSettings.stSpreadSpectrum.ulDelta = settings.spreadSpectrum.delta ;
    rc = BVDC_Display_SetDvoConfiguration(display->displayVdc, &dvoSettings);
    if (rc) {rc = BERR_TRACE(rc); goto error;}

    /* Check if any hdmi upscale change or hdmi format change */
    if (display->hdmi.outputFormat != settings.outputFormat || hdmiFormatChange) {
        if(settings.outputFormat != NEXUS_VideoFormat_eUnknown)
        {
            reconnect_outputs = true;
        }
        nexus_display_p_disconnect_outputs(display, &display->cfg, settings.outputFormat);

        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(settings.outputFormat, &hdmiFmt);
        if (rc) {
            hdmiFmt = BFMT_VideoFmt_eMaxCount;
        }

        rc = BVDC_Display_SetHdmiFormat(display->displayVdc, hdmiFmt, videoFmt);
        if (rc) {rc = BERR_TRACE(rc); goto error;}

        display->hdmi.outputFormat = settings.outputFormat;
    }

    rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    if (reconnect_outputs) {
        nexus_display_p_connect_outputs(display);
        rc = BVDC_ApplyChanges(g_NEXUS_DisplayModule_State.vdc);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    /* if upscale, use upscale format to set HDMI instead of videoFmt */
    if(display->hdmi.outputFormat != NEXUS_VideoFormat_eUnknown) {
        rc = NEXUS_P_VideoFormat_ToMagnum_isrsafe(display->hdmi.outputFormat, &hdmiFmt);
    }
    else {
        hdmiFmt = videoFmt;
    }

    /* post-ApplyChanges format change */
    NEXUS_CallbackHandler_PrepareCallback(display->hdmi.outputNotifyDisplay, notifyDisplay);

    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
        (void)NEXUS_HdmiOutput_SetDisplayParams_priv(hdmiOutput, hdmiFmt, magnumMatrixCoefficients, aspectRatioVdc,
            display->timingGenerator == NEXUS_DisplayTimingGenerator_eHdmiDvo,
            hdmiFormatChange ? &notifyDisplay : NULL) ;

        if (hdmiFormatChange)
        {
            (void)NEXUS_HdmiOutput_P_PostFormatChange_priv(hdmiOutput);
        }
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);

    return NEXUS_SUCCESS;

error:
    NEXUS_Module_Lock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    NEXUS_HdmiOutput_RestorePrevSettings_priv(hdmiOutput);
    NEXUS_Module_Unlock(g_NEXUS_DisplayModule_State.modules.hdmiOutput);
    BDBG_ASSERT(rc);
    return rc;
}

/* called as a result of NEXUS_HdmiOutput_SetSettings */
void NEXUS_VideoOutput_P_SetHdmiSettings(void *context)
{
    NEXUS_Error rc;
    NEXUS_DisplayHandle display = context;
    /* call NEXUS_VideoOutput_P_ApplyHdmiSettings with current display settings */
    rc = NEXUS_VideoOutput_P_ApplyHdmiSettings(display->hdmi.outputNotify, display, display->cfg.format, display->cfg.aspectRatio, false);
    if (rc) BERR_TRACE(rc);
}

static BERR_Code
NEXUS_VideoOutput_P_ConnectHdmi(void *output,  NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmi = output;
    NEXUS_HdmiOutputStatus *hdmiOutputStatus;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BDBG_ASSERT(NULL != hdmi);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if(display->hdmi.rateChangeCb_isr)
    {
        /* already connected */
        return BERR_SUCCESS;
    }

    /* make sure HDMI Core is enabled; TMDS clock ON and data OFF until properly configured */
    BDBG_MSG(("Add HDMI to nexus display...")) ;
    NEXUS_Module_Lock(video->modules.hdmiOutput) ;
        rc = NEXUS_HdmiOutput_P_SetTmdsSignalData(hdmi, false) ;
        if (!rc)
        {
            rc = NEXUS_HdmiOutput_P_SetTmdsSignalClock(hdmi, true) ;
            if (rc) BERR_TRACE(rc) ;
        }
        else
        {
            BERR_TRACE(rc) ;
        }
    NEXUS_Module_Unlock(video->modules.hdmiOutput) ;
    if (rc) return rc ;

    hdmiOutputStatus = &g_NEXUS_DisplayModule_State.functionData.NEXUS_VideoOutput_P_SetHdmiFormat.hdmiOutputStatus;
    rc = NEXUS_HdmiOutput_GetStatus(hdmi, hdmiOutputStatus);
    if (rc) return BERR_TRACE(rc);
    rc = NEXUS_VideoOutput_P_HdmiNum_ToMagnum(hdmiOutputStatus->index, &display->hdmi.vdcIndex);
    if (rc) return BERR_TRACE(rc);

    BKNI_EnterCriticalSection();
    display->hdmi.rateChangeCb_isr = NEXUS_VideoOutput_P_HdmiRateChange_isr;
    display->hdmi.vsync_isr = NEXUS_HdmiOutput_P_Vsync_isr;
    display->hdmi.pCbParam = hdmi;
    if ( display->hdmi.rateInfoValid )
    {
        NEXUS_VideoOutput_P_HdmiRateChange_isr(display, hdmi);
    }
    BKNI_LeaveCriticalSection();

    /* On NEXUS_Display_AddOutput, we need to apply settings just like a NEXUS_HdmiOutput_SetSettings. */
    BDBG_ASSERT(!display->hdmi.outputNotify);
    display->hdmi.outputNotify = hdmi;
    display->hdmi.forceFormatChange = true;
    NEXUS_VideoOutput_P_SetHdmiSettings(display);

    return BERR_SUCCESS;
}

static BERR_Code
NEXUS_VideoOutput_P_DisconnectHdmi(void *output, NEXUS_DisplayHandle display)
{
    BERR_Code rc;
    NEXUS_HdmiOutputHandle hdmi = output;
    NEXUS_HdmiOutputSettings settings;
    const NEXUS_DisplayModule_State *video= &g_NEXUS_DisplayModule_State;

    BDBG_ASSERT(NULL != hdmi);
    NEXUS_OBJECT_ASSERT(NEXUS_Display, display);

    if(g_NEXUS_DisplayModule_State.updateMode != NEXUS_DisplayUpdateMode_eAuto) {rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);}

    /* HDMI is being removed from the Display; */
    /* reset VDC HDMI settings to values compatible with analog settings */
    NEXUS_VideoOutput_P_SetDefaultHdmiSettings(display) ;

    BKNI_EnterCriticalSection();
        BDBG_ASSERT(NULL != display->hdmi.rateChangeCb_isr);
        display->hdmi.rateChangeCb_isr = NULL;
        display->hdmi.vsync_isr = NULL;
        display->hdmi.pCbParam = NULL;
    BKNI_LeaveCriticalSection();
    display->hdmi.outputNotify = NULL;

    (void) NEXUS_HdmiOutput_GetSettings(hdmi, &settings);
    if (settings.mpaaDecimationEnabled) {
        settings.mpaaDecimationEnabled = false;
        rc = BVDC_Display_SetMpaaDecimation(display->displayVdc, BVDC_MpaaDeciIf_eHdmi, 1, settings.mpaaDecimationEnabled);
        if (rc) {
            rc = BERR_TRACE(rc); /* fall through */
        }
        else {
            /* two ApplyChanges are required: this one and the standard one */
            rc = BVDC_ApplyChanges(video->vdc);
            if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        }
    }

    /* Turn off the transmitter itself */
    BDBG_MSG(("Remove HDMI from nexus display...")) ;
    NEXUS_Module_Lock(video->modules.hdmiOutput);
    rc = NEXUS_HdmiOutput_Disconnect_priv(hdmi);
    NEXUS_Module_Unlock(video->modules.hdmiOutput);
    if (rc) {BERR_TRACE(rc);} /* fall through */

    return BERR_SUCCESS;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenHdmi(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BDBG_ASSERT(output->type == NEXUS_VideoOutputType_eHdmi);

    iface.connect = NEXUS_VideoOutput_P_ConnectHdmi;
    iface.disconnect = NEXUS_VideoOutput_P_DisconnectHdmi;
    iface.formatChange = NEXUS_VideoOutput_P_ApplyHdmiSettings;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, false);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eDvo;
    }
    return link;
}
#endif

#if NEXUS_HAS_RFM
static BERR_Code
NEXUS_Rfm_P_Connect(void *output,  NEXUS_DisplayHandle display)
{
    NEXUS_RfmHandle rfm = output;
    NEXUS_RfmConnectionSettings rfmConnectionSettings;
    unsigned index;
    BERR_Code rc;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    NEXUS_Module_Lock(video->modules.rfm);
    NEXUS_Rfm_GetIndex_priv(rfm, &index);
    NEXUS_Module_Unlock(video->modules.rfm);

    if (!nexus_videooutput_p_allow_analog_display(NEXUS_VideoOutputType_eRfm, display)) return NEXUS_SUCCESS;

    rc = BVDC_Display_SetRfmConfiguration(display->displayVdc, BVDC_Rfm_0 + index,  BVDC_RfmOutput_eCVBS, 0);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Module_Lock(video->modules.rfm);
    NEXUS_Rfm_GetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    rfmConnectionSettings.videoEnabled = true;
    rfmConnectionSettings.videoFormat = display->cfg.format;
    rc = NEXUS_Rfm_SetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    NEXUS_Module_Unlock(video->modules.rfm);

    return rc;
}

static BERR_Code
NEXUS_Rfm_P_Disconnect(void *output, NEXUS_DisplayHandle display)
{
    NEXUS_RfmHandle rfm = output;
    NEXUS_RfmConnectionSettings rfmConnectionSettings;
    unsigned index;
    BERR_Code rc;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;

    NEXUS_Module_Lock(video->modules.rfm);
    NEXUS_Rfm_GetIndex_priv(rfm, &index);
    NEXUS_Module_Unlock(video->modules.rfm);

    rc = BVDC_Display_SetRfmConfiguration(display->displayVdc, BVDC_Rfm_0 + index,  BVDC_RfmOutput_eUnused, 0);
    if (rc) return BERR_TRACE(rc);

    NEXUS_Module_Lock(video->modules.rfm);
    NEXUS_Rfm_GetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    rfmConnectionSettings.videoEnabled = false;
    rc = NEXUS_Rfm_SetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    NEXUS_Module_Unlock(video->modules.rfm);

    return rc;
}

static BERR_Code
NEXUS_Rfm_P_FormatChange(void *output, NEXUS_DisplayHandle display, NEXUS_VideoFormat format, NEXUS_DisplayAspectRatio aspectRatio, bool _3dOrientationChange)
{
    NEXUS_RfmHandle rfm = output;
    NEXUS_RfmConnectionSettings rfmConnectionSettings;
    BERR_Code rc;
    const NEXUS_DisplayModule_State *video = &g_NEXUS_DisplayModule_State;
    BSTD_UNUSED(display);
    BSTD_UNUSED(aspectRatio);
	BSTD_UNUSED(_3dOrientationChange);

    NEXUS_Module_Lock(video->modules.rfm);
    NEXUS_Rfm_GetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    rfmConnectionSettings.videoEnabled = true;
    rfmConnectionSettings.videoFormat = format;
    rc = NEXUS_Rfm_SetConnectionSettings_priv(rfm, &rfmConnectionSettings);
    NEXUS_Module_Unlock(video->modules.rfm);

    return rc;
}

static NEXUS_VideoOutput_P_Link *
NEXUS_VideoOutput_P_OpenRfm(NEXUS_VideoOutputHandle output)
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_VideoOutput_P_Iface iface;
    iface.connect = NEXUS_Rfm_P_Connect;
    iface.disconnect = NEXUS_Rfm_P_Disconnect;
    iface.formatChange = NEXUS_Rfm_P_FormatChange;
    link = NEXUS_VideoOutput_P_CreateLink(output, &iface, true);
    if (link) {
        link->displayOutput = BVDC_DisplayOutput_eComposite;
    }
    return link;
}
#endif

NEXUS_VideoOutput_P_Link *
NEXUS_P_VideoOutput_Link(NEXUS_VideoOutputHandle output)
{
    BERR_Code rc;
    BDBG_ASSERT(output->destination==NULL);
    switch(output->type) {
#if NEXUS_NUM_HDMI_DVO
    case NEXUS_VideoOutputType_eHdmiDvo:
        return NEXUS_VideoOutput_P_OpenHdmiDvo(output);
#endif
    case NEXUS_VideoOutputType_eComposite:
        return NEXUS_VideoOutput_P_OpenComposite(output);
    case NEXUS_VideoOutputType_eComponent:
        return NEXUS_VideoOutput_P_OpenComponent(output);
    case NEXUS_VideoOutputType_eSvideo:
        return NEXUS_VideoOutput_P_OpenSvideo(output);
#if NEXUS_HAS_HDMI_OUTPUT
    case NEXUS_VideoOutputType_eHdmi:
        return NEXUS_VideoOutput_P_OpenHdmi(output);
#endif
#if NEXUS_HAS_RFM
    case NEXUS_VideoOutputType_eRfm:
        return NEXUS_VideoOutput_P_OpenRfm(output);
#endif
    case NEXUS_VideoOutputType_eCcir656:
        return NEXUS_VideoOutput_P_OpenCcir656(output);
    default:
        rc=BERR_TRACE(NEXUS_NOT_SUPPORTED);
        return NULL;
    }
}

void
NEXUS_VideoOutput_Shutdown(NEXUS_VideoOutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    if (output->destination) {
        NEXUS_DisplayHandle display = NEXUS_VideoOutput_P_GetDisplay(output);
        if (display) {
            NEXUS_Display_RemoveOutput(display, output);
        }
    }
    return;
}

NEXUS_DisplayHandle NEXUS_VideoOutput_P_GetDisplay(NEXUS_VideoOutputHandle output)
{
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);

    if (output->destination) {
        NEXUS_VideoOutput_P_Link *link = output->destination;
        NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);
        return link->display;
    }
    return NULL;
}

static unsigned nexus_videooutput_p_dac_index(const NEXUS_VideoOutput_P_Link *link, NEXUS_VideoDac dac)
{
    unsigned i;
    for (i=0;i<NEXUS_P_MAX_DACS;i++) {
        if (link->dac[i].dac == dac && dac != NEXUS_VideoDac_eNone) break;
    }
    return i;
}

void NEXUS_VideoOutput_GetVfFilter( NEXUS_VideoOutputHandle output, NEXUS_VideoDac dac, NEXUS_VideoOutputVfFilter *pFilter )
{
    BERR_Code rc;
    bool override;
    NEXUS_VideoOutput_P_Link *link;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);

    BKNI_Memset(pFilter, 0, sizeof(*pFilter));
    if (!output->destination) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto done;
    }
    link = output->destination;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

    i = nexus_videooutput_p_dac_index(link, dac);
    if (i == NEXUS_P_MAX_DACS) {
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto done;
    }

    rc = BVDC_Display_GetVfFilter(link->display->displayVdc, link->displayOutput, link->dac[i].type, &override, pFilter->filterRegs, NEXUS_MAX_VF_FILTER_ENTRIES, &(pFilter->SUM_OF_TAPS));
    if (rc) {
        rc = BERR_TRACE(rc);
        pFilter->numEntries = 0;
    }
    /* TODO: VDC does not return actual number populated, so we just report back the max. */
    pFilter->numEntries = NEXUS_MAX_VF_FILTER_ENTRIES;
done:
    return;
}

NEXUS_Error NEXUS_VideoOutput_SetVfFilter( NEXUS_VideoOutputHandle output, NEXUS_VideoDac dac, const NEXUS_VideoOutputVfFilter *pFilter )
{
    BERR_Code rc;
    NEXUS_VideoOutput_P_Link *link;
    unsigned i;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    if (!output->destination) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    link = output->destination;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

    i = nexus_videooutput_p_dac_index(link, dac);
    if (i == NEXUS_P_MAX_DACS) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (pFilter) {
        rc = BVDC_Display_SetVfFilter(link->display->displayVdc, link->displayOutput, link->dac[i].type, true, (uint32_t*)pFilter->filterRegs, pFilter->numEntries, pFilter->SUM_OF_TAPS);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        rc = BVDC_Display_SetVfFilter(link->display->displayVdc, link->displayOutput, link->dac[i].type, false, NULL, 0, 0);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

void NEXUS_VideoOutput_GetSettings( NEXUS_VideoOutputHandle output, NEXUS_VideoOutputSettings *pSettings )
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    if (!output->destination) {
        BKNI_Memset(pSettings, 0, sizeof(*pSettings));
        return;
    }
    link = output->destination;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);
    *pSettings = link->settings;
}

NEXUS_Error NEXUS_VideoOutput_SetSettings( NEXUS_VideoOutputHandle output, const NEXUS_VideoOutputSettings *pSettings )
{
    NEXUS_VideoOutput_P_Link *link;
    NEXUS_Error rc;
    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    if (!output->destination) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    link = output->destination;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

    rc = BVDC_Display_SetMuteMode(link->display->displayVdc, link->displayOutput, pSettings->mute?BVDC_MuteMode_eConst:BVDC_MuteMode_eDisable);
    if (rc) return BERR_TRACE(rc);

     rc = NEXUS_Display_P_ApplyChanges();
    if (rc) return BERR_TRACE(rc);

    link->settings = *pSettings;
    return 0;
}

NEXUS_Error nexus_p_bypass_video_output_connect(NEXUS_VideoOutput_P_Link *link, const NEXUS_DisplaySettings *pSettings, NEXUS_VideoFormat hdmiOutputFormat)
{
    if (link->sdOnly && !NEXUS_P_VideoFormat_IsSd(pSettings->format)) {
        BDBG_WRN(("disabling %s output (%p) because it is not HD capable", g_videoOutputStr[link->output->type], (void *)link->output));
        return true;
    }
    else if (link->output->type != NEXUS_VideoOutputType_eHdmi && link->output->type != NEXUS_VideoOutputType_eHdmiDvo && link->display->timingGenerator == NEXUS_DisplayTimingGenerator_eHdmiDvo) {
        BDBG_WRN(("disabling %s output (%p) because display does not support analog", g_videoOutputStr[link->output->type], (void *)link->output));
        return true;
    }
    else if (link->output->type != NEXUS_VideoOutputType_eHdmi && link->output->type != NEXUS_VideoOutputType_eHdmiDvo && (NEXUS_P_VideoFormat_IsNotAnalogOutput(pSettings->format) || NEXUS_P_VideoFormat_IsNotAnalogOutput(hdmiOutputFormat))) {
        BDBG_WRN(("disabling %s output (%p) because display format does not support 4K", g_videoOutputStr[link->output->type], (void *)link->output));
        return true;
    }
    /* don't bypass */
    return false;
}

NEXUS_Error nexus_videooutput_p_connect(NEXUS_VideoOutput_P_Link *link)
{
    NEXUS_DisplayHandle display;
    NEXUS_Error rc;

    display = NEXUS_VideoOutput_P_GetDisplay(link->output);
    if (!display) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (!link->connected && nexus_p_bypass_video_output_connect(link, &link->display->cfg, display->hdmi.outputFormat)) {
        return 0;
    }

    rc = link->iface.connect(link->output->source, display);
    if (rc) return BERR_TRACE(rc);

    link->connected = true;
    return 0;
}

void nexus_videooutput_p_disconnect(NEXUS_VideoOutput_P_Link *link)
{
    if (link->connected) {
        NEXUS_DisplayHandle display = NEXUS_VideoOutput_P_GetDisplay(link->output);
        BDBG_ASSERT(display);
        link->iface.disconnect(link->output->source, display);
        link->connected = false;
    }
}

NEXUS_Error NEXUS_VideoOutput_GetStatus( NEXUS_VideoOutputHandle output, NEXUS_VideoOutputStatus *pStatus )
{
    BERR_Code rc = NEXUS_SUCCESS;
    NEXUS_VideoOutput_P_Link *link;
    unsigned i;
    bool found_one = false;

    NEXUS_OBJECT_ASSERT(NEXUS_VideoOutput, output);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (!output->destination) {
        return rc;
    }

    link = output->destination;
    NEXUS_OBJECT_ASSERT(NEXUS_Display, link->display);

    /* Aggregate by looking at all DACS with this precedence: connected -> disconnected -> unknown.
    if any DAC is unknown or there are no dacs, the state is unknown;
    else if any DAC is disconnected, the state is disconnected;
    else it is connected. */
    pStatus->connectionState = NEXUS_VideoOutputConnectionState_eConnected;
    for (i=0;i<NEXUS_P_MAX_DACS;i++) {
        if(link->dac[i].dac && link->dac[i].dac < NEXUS_VideoDac_eMax && link->dacsConnected) {
            found_one = true;
            BDBG_CASSERT(BVDC_MAX_DACS == NEXUS_VideoDac_eMax - NEXUS_VideoDac_e0);
            switch (link->display->dacStatus[link->dac[i].dac - NEXUS_VideoDac_e0]) {
            case BVDC_DacConnectionState_eUnknown:
                pStatus->connectionState = NEXUS_VideoOutputConnectionState_eUnknown;
                i = NEXUS_P_MAX_DACS; /* terminal */
                break;
            case BVDC_DacConnectionState_eDisconnected:
                pStatus->connectionState = NEXUS_VideoOutputConnectionState_eDisconnected;
                /* keep going. could still be eUnknown. */
                break;
            default: break;
            }
        }
    }
    if (!found_one) {
        pStatus->connectionState = NEXUS_VideoOutputConnectionState_eUnknown;
    }
    return rc;
}

/* End of file */
