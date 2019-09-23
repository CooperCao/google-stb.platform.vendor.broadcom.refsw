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
 *
 * Module Description:
 *
 *****************************************************************************/
#include "nxserverlib_impl.h"
#if NEXUS_HAS_DISPLAY
#include "nexus_display_private.h"
#endif
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder_private.h"
#endif

/*#include <stdio.h>*/
/*#include <string.h>*/
/*#include <stdlib.h>*/
/*#include <errno.h>*/
/*#include <signal.h>*/
/*#include <pthread.h>*/
/*#include <unistd.h>*/
/*#include <fcntl.h>*/
/*#include <sys/poll.h>*/

/*#include <unistd.h>*/
/*#include <sys/types.h>*/
/*#include <sys/stat.h>*/
/*#include <fcntl.h>*/
/*#include <string.h>*/
#include "bstd.h"


BDBG_MODULE(nxserverlib_dynrng);
#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#if NEXUS_HAS_HDMI_OUTPUT
static const NEXUS_HdmiDynamicRangeMasteringInfoFrame unspecifiedHdmiDrmInfoFrame =
{
    NEXUS_VideoEotf_eMax, /* max indicates from input */
    {
        NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
        {
            {
                {
                    { -1, -1 }, /* red primary color chromaticity coordinates used by the mastering display.
                                 X and Y values range from 0 to 50000 and are fixed point representations
                                 of floating point values between 0.0 and 1.0, with a step of 0.00002 per tick. */
                    { -1, -1 }, /* green primary color chromaticity coordinates used by the mastering display.
                                 see description for red primary above for unit details */
                    { -1, -1 }, /* blue primary color chromaticity coordinates used by the mastering display.
                                 see description for red primary above for unit details */
                    { -1, -1 }, /* white point chromaticity coordinate used by the mastering display.
                                 see description for red primary above for unit details */
                    {
                        -1, /* 1 cd / m^2 */
                        -1 /* 0.0001 cd / m^2 */
                    } /* luminance range of the mastering display */
                },
                {
                    -1, /* 1 cd / m^2. This is the max light level used in any pixel across the entire stream */
                    -1 /* 1 cd / m^2. Averaging the light level spatially per picture as frmAvg,
                         this is the max value of frmAvg reached across the entire stream */
                }
            } /* type "1" metadata */
        } /* type-specific metadata settings */
    }
};

static const NEXUS_HdmiDynamicRangeMasteringInfoFrame failsafeHdmiDrmInfoFrame =
{
    NEXUS_VideoEotf_eSdr, /* eotf */
    {
        NEXUS_HdmiDynamicRangeMasteringStaticMetadataType_e1,
        {
            {
                {
                    { 0, 0 }, /* red primary color chromaticity coordinates used by the mastering display.
                                 X and Y values range from 0 to 50000 and are fixed point representations
                                 of floating point values between 0.0 and 1.0, with a step of 0.00002 per tick. */
                    { 0, 0 }, /* green primary color chromaticity coordinates used by the mastering display.
                                 see description for red primary above for unit details */
                    { 0, 0 }, /* blue primary color chromaticity coordinates used by the mastering display.
                                 see description for red primary above for unit details */
                    { 0, 0 }, /* white point chromaticity coordinate used by the mastering display.
                                 see description for red primary above for unit details */
                    {
                        0, /* 1 cd / m^2 */
                        0 /* 0.0001 cd / m^2 */
                    } /* luminance range of the mastering display */
                },
                {
                    0, /* 1 cd / m^2. This is the max light level used in any pixel across the entire stream */
                    0 /* 1 cd / m^2. Averaging the light level spatially per picture as frmAvg,
                         this is the max value of frmAvg reached across the entire stream */
                }
            } /* type "1" metadata */
        } /* type-specific metadata settings */
    }
};

static NEXUS_Error nxserverlib_dynrng_p_apply(struct b_video_dynrng *dynrng);

bool nxserverlib_dynrng_p_is_dolby_vision_active(struct b_session *session)
{
    NEXUS_DisplayHandle display = nxserverlib_session_p_get_primary_display(session);
    if (display) {
        NEXUS_DisplayStatus status;
        NEXUS_Display_GetStatus(display, &status);
        return status.dynamicRangeMode == NEXUS_VideoDynamicRangeMode_eDolbyVision;
    }
    return false;
}

static void nxserverlib_dynrng_p_display_hdr_info_changed(void * context, int param)
{
    struct b_video_dynrng * dynrng = context;
    nxserver_t server = nxserverlib_session_p_get_server(dynrng->session);
    NEXUS_DisplayPrivateStatus status;

    BSTD_UNUSED(param);

    BDBG_MSG(("nxserverlib_dynrng_p_display_hdr_info_changed"));

    nxserver_p_lock(server);
    NEXUS_Display_GetPrivateStatus(nxserverlib_session_p_get_primary_display(dynrng->session), &status);
    BKNI_Memcpy(&dynrng->input.metadata.typeSettings.type1.contentLightLevel, &status.infoFrame.metadata.typeSettings.type1.contentLightLevel, sizeof(dynrng->input.metadata.typeSettings.type1.contentLightLevel));
    BKNI_Memcpy(&dynrng->input.metadata.typeSettings.type1.masteringDisplayColorVolume, &status.infoFrame.metadata.typeSettings.type1.masteringDisplayColorVolume, sizeof(dynrng->input.metadata.typeSettings.type1.masteringDisplayColorVolume));
    dynrng->smdValid = true;
#if NEXUS_HAS_VIDEO_DECODER
    dynrng->dynamicMetadataType = status.infoFrame.metadata.type;
#endif
    nxserverlib_dynrng_p_apply(dynrng);
    nxserver_p_unlock(server);
}

int nxserverlib_dynrng_p_session_initialized(struct b_session * session)
{
    int rc = 0;
    NEXUS_DisplayPrivateSettings privateSettings;
    NEXUS_DisplayHandle display = nxserverlib_session_p_get_primary_display(session);
    struct b_video_dynrng * dynrng = &session->hdmi.dynrng;

    BDBG_MSG(("nxserverlib_dynrng_p_session_initialized"));

    dynrng->session = session;

    NEXUS_Display_GetPrivateSettings(display, &privateSettings);
    privateSettings.hdrInfoChanged.callback = nxserverlib_dynrng_p_display_hdr_info_changed;
    privateSettings.hdrInfoChanged.context = dynrng;
    rc = NEXUS_Display_SetPrivateSettings(display, &privateSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    BKNI_Memcpy(&dynrng->input, &failsafeHdmiDrmInfoFrame, sizeof(dynrng->input));
    dynrng->dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eDefault;

error:
    return rc;
}

#if NEXUS_HAS_VIDEO_DECODER
static void nxserverlib_dynrng_p_video_decoder_stream_changed(void * context, int param)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct b_video_dynrng * dynrng = context;
    NEXUS_VideoDecoderStreamInformation streamInfo;

    BDBG_MSG(("nxserverlib_dynrng_p_video_decoder_stream_changed"));

    BSTD_UNUSED(param);

    rc = NEXUS_VideoDecoder_GetStreamInformation(dynrng->mainDecoder, &streamInfo);
    if (!rc)
    {
        nxserver_t server = nxserverlib_session_p_get_server(dynrng->session);
        nxserver_p_lock(server);
        dynrng->input.eotf = streamInfo.eotf;
        dynrng->eotfValid = true;
        nxserverlib_dynrng_p_apply(dynrng);
        nxserver_p_unlock(server);
    }

}
#endif

int nxserverlib_dynrng_p_video_decoder_acquired(struct b_video_dynrng *dynrng, NEXUS_VideoDecoderHandle handle, NxClient_VideoWindowType type)
{
    int rc = 0;
#if NEXUS_HAS_VIDEO_DECODER
    BDBG_MSG(("nxserverlib_dynrng_p_video_decoder_acquired"));

    if (handle && type == NxClient_VideoWindowType_eMain && nxserverlib_session_p_get_hdmi_output(dynrng->session))
    {
        NEXUS_VideoDecoderPrivateSettings privateSettings;
        dynrng->mainDecoder = handle;
        NEXUS_VideoDecoder_GetPrivateSettings(handle, &privateSettings);
        privateSettings.streamChanged.callback = nxserverlib_dynrng_p_video_decoder_stream_changed;
        privateSettings.streamChanged.context = dynrng;
        rc = NEXUS_VideoDecoder_SetPrivateSettings(handle, &privateSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
error:
#else
    BSTD_UNUSED(dynrng);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(type);
#endif
    return rc;
}

void nxserverlib_dynrng_p_video_decoder_released(struct b_video_dynrng *dynrng, NEXUS_VideoDecoderHandle handle)
{
    BDBG_MSG(("nxserverlib_dynrng_p_video_decoder_released"));
    if (dynrng->mainDecoder && dynrng->mainDecoder == handle)
    {
        NEXUS_Error rc = NEXUS_SUCCESS;
        NEXUS_VideoDecoderPrivateSettings privateSettings;
        NEXUS_VideoDecoder_GetPrivateSettings(dynrng->mainDecoder, &privateSettings);
        privateSettings.streamChanged.callback = NULL;
        privateSettings.streamChanged.context = NULL;
        rc = NEXUS_VideoDecoder_SetPrivateSettings(dynrng->mainDecoder, &privateSettings);
        if (rc) { rc = BERR_TRACE(rc); }
        dynrng->mainDecoder = NULL;
    }
}

void nxserverlib_dynrng_p_hotplug_callback_locked(struct b_video_dynrng *dynrng)
{
    NEXUS_HdmiOutputExtraStatus extraStatus;
    NEXUS_HdmiOutput_GetExtraStatus(nxserverlib_session_p_get_hdmi_output(dynrng->session), &extraStatus);

    BDBG_MSG(("nxserverlib_dynrng_p_hotplug_callback_locked"));

    if (BKNI_Memcmp(&dynrng->extraStatus, &extraStatus, sizeof(dynrng->extraStatus))) {
        BKNI_Memcpy(&dynrng->extraStatus, &extraStatus, sizeof(dynrng->extraStatus));
        nxserverlib_dynrng_p_apply(dynrng);
    }
}

void nxserverlib_dyrnng_p_get_default_settings(NxClient_DisplaySettings * pSettings)
{
    BDBG_MSG(("nxserverlib_dyrnng_p_get_default_settings"));
    BKNI_Memcpy(&pSettings->hdmiPreferences.drmInfoFrame, &unspecifiedHdmiDrmInfoFrame, sizeof(pSettings->hdmiPreferences.drmInfoFrame));
    pSettings->hdmiPreferences.dynamicRangeMode = NEXUS_VideoDynamicRangeMode_eDefault;
}

static void nxserverlib_dynrng_p_set_selector(
    struct b_video_dynrng_selector * pSelector,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pUserInfoFrame,
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pUnspecifiedInfoFrame
)
{
    const NEXUS_ContentLightLevel * pUserCll;
    const NEXUS_MasteringDisplayColorVolume * pUserMdcv;
    const NEXUS_ContentLightLevel * pUnspecifiedCll;
    const NEXUS_MasteringDisplayColorVolume * pUnspecifiedMdcv;

    pUserMdcv = &pUserInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pUserCll = &pUserInfoFrame->metadata.typeSettings.type1.contentLightLevel;
    pUnspecifiedMdcv = &pUnspecifiedInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pUnspecifiedCll = &pUnspecifiedInfoFrame->metadata.typeSettings.type1.contentLightLevel;

    pSelector->eotf = pUserInfoFrame->eotf != pUnspecifiedInfoFrame->eotf
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.primaries.red = BKNI_Memcmp(&pUserMdcv->redPrimary, &pUnspecifiedMdcv->redPrimary, sizeof(pUserMdcv->redPrimary))
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.primaries.green = BKNI_Memcmp(&pUserMdcv->greenPrimary, &pUnspecifiedMdcv->greenPrimary, sizeof(pUserMdcv->greenPrimary))
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.primaries.blue = BKNI_Memcmp(&pUserMdcv->bluePrimary, &pUnspecifiedMdcv->bluePrimary, sizeof(pUserMdcv->bluePrimary))
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.whitePoint = BKNI_Memcmp(&pUserMdcv->whitePoint, &pUnspecifiedMdcv->whitePoint, sizeof(pUserMdcv->whitePoint))
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.luminance.max = pUserMdcv->luminance.max != pUnspecifiedMdcv->luminance.max
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->mdcv.luminance.min = pUserMdcv->luminance.min != pUnspecifiedMdcv->luminance.min
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->cll.max = pUserCll->max != pUnspecifiedCll->max
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
    pSelector->cll.maxFrameAverage = pUserCll->maxFrameAverage != pUnspecifiedCll->maxFrameAverage
        ? b_video_dynrng_source_user : b_video_dynrng_source_input;
}

static bool nxserverlib_dynrng_p_selector_has_user(const struct b_video_dynrng_selector * pSelector)
{
    return (pSelector->eotf == b_video_dynrng_source_user)
        || (pSelector->mdcv.primaries.red == b_video_dynrng_source_user)
        || (pSelector->mdcv.primaries.green == b_video_dynrng_source_user)
        || (pSelector->mdcv.primaries.blue == b_video_dynrng_source_user)
        || (pSelector->mdcv.whitePoint == b_video_dynrng_source_user)
        || (pSelector->mdcv.luminance.max == b_video_dynrng_source_user)
        || (pSelector->mdcv.luminance.min == b_video_dynrng_source_user)
        || (pSelector->cll.max == b_video_dynrng_source_user)
        || (pSelector->cll.maxFrameAverage == b_video_dynrng_source_user)
        || false;
}

static unsigned nxserverlib_dynrng_p_apply_impl(
    enum b_video_dynrng_source source,
    bool inputValid,
    size_t size,
    void * pOutput,
    const void * pInput,
    const void * pUser,
    const void * pFailsafe)
{
    unsigned changed = 0;
    const void * pSource = NULL;

    if (source == b_video_dynrng_source_input)
    {
        if (inputValid)
        {
            pSource = pInput;
        }
        else
        {
            pSource = pFailsafe;
        }
    }
    else
    {
        pSource = pUser;
    }

    if (pSource && BKNI_Memcmp(pOutput, pSource, size))
    {

        BKNI_Memcpy(pOutput, pSource, size);
        changed = 1;
    }
    /* else change nothing */

    return changed;
}

#if 0
void nxserverlib_dynrng_p_dump(const char * tag, const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInfoFrame)
{
    BDBG_ERR(("%s = %p", tag, (void *)pInfoFrame));
    BDBG_ERR(("eotf = %d", pInfoFrame->eotf));
    BDBG_ERR(("type: %d", pInfoFrame->metadata.type + 1));
    BDBG_ERR(("red: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.x, pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.redPrimary.y));
    BDBG_ERR(("green: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.x, pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.greenPrimary.y));
    BDBG_ERR(("blue: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.x, pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.bluePrimary.y));
    BDBG_ERR(("white: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.x, pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.whitePoint.y));
    BDBG_ERR(("luma: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.max, pInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume.luminance.min));
    BDBG_ERR(("cll: (%d, %d)", pInfoFrame->metadata.typeSettings.type1.contentLightLevel.max, pInfoFrame->metadata.typeSettings.type1.contentLightLevel.maxFrameAverage));
}
#endif

static NEXUS_Error nxserverlib_dynrng_p_apply_drmif(const struct b_video_dynrng * dynrng, const NxClient_DisplaySettings * pSettings)
{
    NEXUS_HdmiOutputExtraSettings hdmiOutputSettings;
    NEXUS_HdmiDynamicRangeMasteringInfoFrame * pOutputInfoFrame;
    NEXUS_ContentLightLevel * pOutputCll;
    NEXUS_MasteringDisplayColorVolume * pOutputMdcv;
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pInputInfoFrame;
    const NEXUS_ContentLightLevel * pInputCll;
    const NEXUS_MasteringDisplayColorVolume * pInputMdcv;
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pUserInfoFrame;
    const NEXUS_ContentLightLevel * pUserCll;
    const NEXUS_MasteringDisplayColorVolume * pUserMdcv;
    const NEXUS_HdmiDynamicRangeMasteringInfoFrame * pFailsafeInfoFrame;
    const NEXUS_ContentLightLevel * pFailsafeCll;
    const NEXUS_MasteringDisplayColorVolume * pFailsafeMdcv;
    NEXUS_HdmiOutputHandle output;
    NEXUS_Error rc = NEXUS_SUCCESS;
    unsigned changed = 0;

    BDBG_MSG(("nxserverlib_dynrng_p_apply_drmif"));

    output = nxserverlib_session_p_get_hdmi_output(dynrng->session);

    if (!output)
    {
        return NEXUS_SUCCESS;
    }

    /* if mode is not auto or default, do not force override drmif (track input on legacy chips allowed force override so we allow it here) */
    if (pSettings->hdmiPreferences.dynamicRangeMode != NEXUS_VideoDynamicRangeMode_eAuto && pSettings->hdmiPreferences.dynamicRangeMode != NEXUS_VideoDynamicRangeMode_eDefault)
    {
        return NEXUS_SUCCESS;
    }

    NEXUS_HdmiOutput_GetExtraSettings(output, &hdmiOutputSettings);

    pOutputInfoFrame = &hdmiOutputSettings.dynamicRangeMasteringInfoFrame;
    pOutputMdcv = &pOutputInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pOutputCll = &pOutputInfoFrame->metadata.typeSettings.type1.contentLightLevel;
    pInputInfoFrame = &dynrng->input;
    pInputMdcv = &pInputInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pInputCll = &pInputInfoFrame->metadata.typeSettings.type1.contentLightLevel;
    pUserInfoFrame = &pSettings->hdmiPreferences.drmInfoFrame;
    pUserMdcv = &pUserInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pUserCll = &pUserInfoFrame->metadata.typeSettings.type1.contentLightLevel;
    pFailsafeInfoFrame = &failsafeHdmiDrmInfoFrame;
    pFailsafeMdcv = &pFailsafeInfoFrame->metadata.typeSettings.type1.masteringDisplayColorVolume;
    pFailsafeCll = &pFailsafeInfoFrame->metadata.typeSettings.type1.contentLightLevel;

    /* EOTF */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.eotf,
            dynrng->eotfValid,
            sizeof(pOutputInfoFrame->eotf),
            &pOutputInfoFrame->eotf,
            &pInputInfoFrame->eotf,
            &pUserInfoFrame->eotf,
            &pFailsafeInfoFrame->eotf);
    /* MDCV.red */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.primaries.red,
            dynrng->smdValid,
            sizeof(pOutputMdcv->redPrimary),
            &pOutputMdcv->redPrimary,
            &pInputMdcv->redPrimary,
            &pUserMdcv->redPrimary,
            &pFailsafeMdcv->redPrimary);
    /* MDCV.green */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.primaries.green,
            dynrng->smdValid,
            sizeof(pOutputMdcv->greenPrimary),
            &pOutputMdcv->greenPrimary,
            &pInputMdcv->greenPrimary,
            &pUserMdcv->greenPrimary,
            &pFailsafeMdcv->greenPrimary);
    /* MDCV.blue */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.primaries.blue,
            dynrng->smdValid,
            sizeof(pOutputMdcv->bluePrimary),
            &pOutputMdcv->bluePrimary,
            &pInputMdcv->bluePrimary,
            &pUserMdcv->bluePrimary,
            &pFailsafeMdcv->bluePrimary);
    /* MDCV.white */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.whitePoint,
            dynrng->smdValid,
            sizeof(pOutputMdcv->whitePoint),
            &pOutputMdcv->whitePoint,
            &pInputMdcv->whitePoint,
            &pUserMdcv->whitePoint,
            &pFailsafeMdcv->whitePoint);
    /* MDCV.luma.max */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.luminance.max,
            dynrng->smdValid,
            sizeof(pOutputMdcv->luminance.max),
            &pOutputMdcv->luminance.max,
            &pInputMdcv->luminance.max,
            &pUserMdcv->luminance.max,
            &pFailsafeMdcv->luminance.max);
    /* MDCV.luma.min */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.mdcv.luminance.min,
            dynrng->smdValid,
            sizeof(pOutputMdcv->luminance.min),
            &pOutputMdcv->luminance.min,
            &pInputMdcv->luminance.min,
            &pUserMdcv->luminance.min,
            &pFailsafeMdcv->luminance.min);
    /* CLL */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.cll.max,
            dynrng->smdValid,
            sizeof(pOutputCll->max),
            &pOutputCll->max,
            &pInputCll->max,
            &pUserCll->max,
            &pFailsafeCll->max);
    /* FAL */
    changed +=
        nxserverlib_dynrng_p_apply_impl(
            dynrng->selector.cll.maxFrameAverage,
            dynrng->smdValid,
            sizeof(pOutputCll->maxFrameAverage),
            &pOutputCll->maxFrameAverage,
            &pInputCll->maxFrameAverage,
            &pUserCll->maxFrameAverage,
            &pFailsafeCll->maxFrameAverage);

    /* if something changed */
    if (changed)
    {
        /* override if any one of the DRMIF fields are set to user-specified */
        hdmiOutputSettings.overrideDynamicRangeMasteringInfoFrame = nxserverlib_dynrng_p_selector_has_user(&dynrng->selector);
        rc = NEXUS_HdmiOutput_SetExtraSettings(output, &hdmiOutputSettings);
        if (rc) { BERR_TRACE(rc); goto error; }
    }

error:
    return rc;
}

NEXUS_Error nxserverlib_dynrng_p_set_mode(struct b_video_dynrng *dynrng, const NxClient_DisplaySettings *pSettings)
{
    NEXUS_DisplaySettings displaySettings;
    NEXUS_DisplayHandle display;
    NEXUS_Error rc = NEXUS_SUCCESS;

    BDBG_MSG(("nxserverlib_dynrng_p_set_mode"));

    display = nxserverlib_session_p_get_primary_display(dynrng->session);

    if (!display)
    {
        return NEXUS_SUCCESS;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    displaySettings.dynamicRangeMode = pSettings->hdmiPreferences.dynamicRangeMode;
    rc = NEXUS_Display_SetSettings(display, &displaySettings);
    if (rc) { BERR_TRACE(rc); goto error; }

error:
    return rc;
}

static NEXUS_Error nxserverlib_dynrng_p_apply(struct b_video_dynrng *dynrng)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    NxClient_DisplaySettings oldSettings;

    BDBG_MSG(("nxserverlib_dynrng_p_apply"));

    NxClient_P_GetDisplaySettings(NULL, dynrng->session, &oldSettings);

    rc = nxserverlib_dynrng_p_set_mode(dynrng, &oldSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

    rc = nxserverlib_dynrng_p_apply_drmif(dynrng, &oldSettings);
    if (rc) { rc = BERR_TRACE(rc); goto error; }

error:
    return rc;
}

NEXUS_Error nxserverlib_dynrng_p_set_drmif(struct b_video_dynrng *dynrng, const NxClient_DisplaySettings *pSettings)
{
    BDBG_MSG(("nxserverlib_dynrng_p_settings_set_drmif"));
    nxserverlib_dynrng_p_set_selector(&dynrng->selector, &pSettings->hdmiPreferences.drmInfoFrame, &unspecifiedHdmiDrmInfoFrame);
    return nxserverlib_dynrng_p_apply_drmif(dynrng, pSettings);
}

#endif /* HDMI_OUTPUT */
