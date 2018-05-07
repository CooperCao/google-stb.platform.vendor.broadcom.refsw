/******************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
 *****************************************************************************/
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "nexus_simple_decoder_module.h"

#include "nexus_simple_audio_priv.h"
#include "nexus_simple_decoder_impl.h"
#if NEXUS_HAS_AUDIO
#include "nexus_audio.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decoder_primer.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_init.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "priv/nexus_audio_mixer_priv.h"
#include "priv/nexus_audio_output_priv.h"
#endif
#include "nexus_client_resources.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "priv/nexus_hdmi_output_priv.h"
#endif
#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#endif
#include "priv/nexus_simple_audio_playback_priv.h"

BDBG_MODULE(nexus_simple_audio_decoder);

#if NEXUS_HAS_AUDIO
static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddOutputs( NEXUS_SimpleAudioDecoderHandle handle );
static void NEXUS_SimpleAudioDecoder_P_RemoveOutputs( NEXUS_SimpleAudioDecoderHandle handle );
static NEXUS_Error nexus_simpleaudiodecoder_p_start(NEXUS_SimpleAudioDecoderHandle handle);
static void nexus_simpleaudiodecoder_p_stop( NEXUS_SimpleAudioDecoderHandle handle);
static NEXUS_Error nexus_simpleaudiodecoder_p_start_resume(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToStart);
static NEXUS_Error nexus_simpleaudiodecoder_apply_mixerinput_settings( NEXUS_AudioInputHandle hInput, NEXUS_AudioMixerHandle hMixer, NEXUS_SimpleAudioDecoderSelector selector, const NEXUS_SimpleAudioDecoderSettings *pSettings );

static const char * const g_selectorStr[NEXUS_SimpleAudioDecoderSelector_eMax] = {"primary", "secondary", "description"};
#endif
static NEXUS_AudioDecoderHandle nexus_p_get_decoder(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector);
static const char * const g_state[state_max] = {"stopped", "started", "suspended", "priming"};

#define NEXUS_SIMPLEAUDIODECODER_PRIMARY            ((unsigned)1<<0)
#define NEXUS_SIMPLEAUDIODECODER_SECONDARY          ((unsigned)1<<1)
#define NEXUS_SIMPLEAUDIODECODER_DESCRIPTION        ((unsigned)1<<2)
#define NEXUS_SIMPLEAUDIODECODER_ALL                ((unsigned)-1)

#if NEXUS_HAS_AUDIO
#define SUSPEND_DECODER     0x01
#define SUSPEND_PLAYBACK    0x02
#define SUSPEND_MIXER       0x04
#define SUSPEND_PERSISTENT  0x08
#define SUSPEND_ALL (SUSPEND_DECODER|SUSPEND_PLAYBACK|SUSPEND_MIXER|SUSPEND_PERSISTENT)
static void NEXUS_SimpleAudioDecoder_P_Suspend(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToSuspend);
#endif

#define CONNECTED(handle) ((handle)->serverSettings.primary || (handle)->serverSettings.secondary || (handle)->serverSettings.description)
#define SIMPLEMASTER(server, handle) ((server)->masterServerSettings.masterHandle == (handle))

/**
server functions
**/

void NEXUS_SimpleAudioDecoder_GetDefaultServerSettings( NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

static struct nexus_simpleaudiodecoder_defaultsettings {
    NEXUS_AudioDecoderSettings settings;
    NEXUS_AudioDecoderTrickState trickState;
    NEXUS_AudioDecoderCodecSettings codecSettings[NEXUS_AudioCodec_eMax];
} *g_default;

void NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(NEXUS_AudioDecoderHandle audio)
{
#if NEXUS_HAS_AUDIO
    unsigned i;
#endif
    if (!g_default) {
        g_default = BKNI_Malloc(sizeof(*g_default));
        if (!g_default) return;
    }
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoder_GetSettings(audio, &g_default->settings);
    NEXUS_AudioDecoder_GetTrickState(audio, &g_default->trickState);
    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        NEXUS_AudioDecoder_GetCodecSettings(audio, i, &g_default->codecSettings[i]);
    }
#else
    BSTD_UNUSED(audio);
#endif
}

void NEXUS_SimpleAudioDecoderModule_P_UnloadDefaultSettings(void)
{
    if (g_default) {
        BKNI_Free(g_default);
        g_default = NULL;
    }
}

#if NEXUS_HAS_AUDIO
/* framework for the future to address ordering as needed */
static unsigned nexus_simpleaudiodecoder_pp_first(void)
{
    return NEXUS_AudioPostProcessing_eAmbisonic;
}

static unsigned nexus_simpleaudiodecoder_pp_last(void)
{
    return NEXUS_AudioPostProcessing_eMax - 1;
}

static unsigned nexus_simpleaudiodecoder_pp_prev(unsigned j)
{
    return (j>0) ? j-1 : NEXUS_AudioPostProcessing_eMax;
}

static unsigned nexus_simpleaudiodecoder_pp_next(unsigned j)
{
    return (j<NEXUS_AudioPostProcessing_eMax) ? j+1 : NEXUS_AudioPostProcessing_eMax;
}

static bool nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed(NEXUS_SimpleAudioDecoderHandle handle)
{
    if ( handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic ) {
        return true;
    }

    return false;
}
#endif

static void nexus_simpleaudiodecoder_p_set_default_settings(NEXUS_SimpleAudioDecoderHandle handle)
{
    unsigned i;
    if (g_default) {
        handle->settings.primary = g_default->settings;
        handle->settings.secondary = g_default->settings;
        handle->settings.description = g_default->settings;
        handle->trickState = g_default->trickState;
    }
    for(i=0;i<NEXUS_SimpleAudioDecoderSelector_eMax;i++) {
        unsigned j;
        for(j=0;j<NEXUS_AudioPostProcessing_eMax;j++) {
            handle->decoders[i].processor[j] = NULL;
            BKNI_Memset( &handle->decoders[i].processorSettings[j], 0, sizeof(handle->decoders[i].processorSettings[j]) );
        }
    }
}

static BLST_S_HEAD(NEXUS_SimpleAudioDecoderServer_P_List, NEXUS_SimpleAudioDecoderServer) g_NEXUS_SimpleAudioDecoderServers;

NEXUS_SimpleAudioDecoderHandle nexus_simple_audio_decoder_p_first(void)
{
    NEXUS_SimpleAudioDecoderServerHandle server;
    for (server = BLST_S_FIRST(&g_NEXUS_SimpleAudioDecoderServers); server; server = BLST_S_NEXT(server, link)) {
        NEXUS_SimpleAudioDecoderHandle decoder = BLST_S_FIRST(&server->decoders);
        if (decoder) return decoder;
    }
    return NULL;
}

NEXUS_SimpleAudioDecoderHandle nexus_simple_audio_decoder_p_next(NEXUS_SimpleAudioDecoderHandle handle)
{
    NEXUS_SimpleAudioDecoderHandle next;
    next = BLST_S_NEXT(handle, link);
    if (!next) {
        NEXUS_SimpleAudioDecoderServerHandle server;
        for (server = BLST_S_NEXT(handle->server, link); server; server = BLST_S_NEXT(server, link)) {
            next = BLST_S_FIRST(&server->decoders);
            if (next) break;
        }
    }
    return next;
}

NEXUS_SimpleAudioDecoderServerHandle NEXUS_SimpleAudioDecoderServer_Create(void)
{
    NEXUS_SimpleAudioDecoderServerHandle handle;
    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) return NULL;
    NEXUS_OBJECT_INIT(NEXUS_SimpleAudioDecoderServer, handle);
    BLST_S_INSERT_HEAD(&g_NEXUS_SimpleAudioDecoderServers, handle, link);
    return handle;
}

static void NEXUS_SimpleAudioDecoderServer_P_Finalizer( NEXUS_SimpleAudioDecoderServerHandle handle )
{
    BLST_S_REMOVE(&g_NEXUS_SimpleAudioDecoderServers, handle, NEXUS_SimpleAudioDecoderServer, link);
    NEXUS_OBJECT_DESTROY(NEXUS_SimpleAudioDecoderServer, handle);
    BKNI_Free(handle);
}

NEXUS_OBJECT_CLASS_MAKE(NEXUS_SimpleAudioDecoderServer, NEXUS_SimpleAudioDecoderServer_Destroy);

NEXUS_SimpleAudioDecoderHandle NEXUS_SimpleAudioDecoder_Create( NEXUS_SimpleAudioDecoderServerHandle server, unsigned index, const NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    NEXUS_SimpleAudioDecoderHandle handle;
    NEXUS_Error rc;

    /* verify API macros aren't below nexus_platform_features.h */
#if NEXUS_NUM_SPDIF_OUTPUTS
    BDBG_CASSERT(NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS >= NEXUS_NUM_SPDIF_OUTPUTS);
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    BDBG_CASSERT(NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS >= NEXUS_NUM_HDMI_OUTPUTS);
#endif

    /* find dup */
    for (handle=BLST_S_FIRST(&server->decoders); handle; handle=BLST_S_NEXT(handle, link)) {
        if (handle->index == index) {
            BERR_TRACE(NEXUS_INVALID_PARAMETER);
            return NULL;
        }
    }

    handle = BKNI_Malloc(sizeof(*handle));
    if (!handle) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    NEXUS_OBJECT_INIT(NEXUS_SimpleAudioDecoder, handle);
    handle->server = server;
    handle->index = index;
    handle->stcIndex = -1;

    nexus_simpleaudiodecoder_p_set_default_settings(handle);

    BLST_S_INSERT_HEAD(&server->decoders, handle, link);
    NEXUS_OBJECT_REGISTER(NEXUS_SimpleAudioDecoder, handle, Create);
    /* now a valid object */

    /* this will get set during the connect sequence. Until then, we are not valid and not ready to run. */
    handle->serverSettings.type = NEXUS_SimpleAudioDecoderType_eMax;

    if (pSettings) {
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(server, handle, pSettings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }
    else {
        NEXUS_SimpleAudioDecoderServerSettings *settings;
        settings = BKNI_Malloc(sizeof(*settings));
        if(!settings) { (void)BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); goto error; }

        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(settings);
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(server, handle, settings);

        BKNI_Free(settings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    return handle;

error:
    NEXUS_SimpleAudioDecoder_Destroy(handle);
    return NULL;
}

#if NEXUS_HAS_AUDIO
static void nexus_simpleaudiodecoder_freeprocessing( NEXUS_SimpleAudioDecoderHandle handle )
{
    unsigned i, j;
    for(i=0;i<NEXUS_SimpleAudioDecoderSelector_eMax;i++) {
        for(j=nexus_simpleaudiodecoder_pp_last();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_prev(j)) {
            if ( handle->decoders[i].processor[j] ) {
                NEXUS_AudioProcessor_RemoveAllInputs(handle->decoders[i].processor[j]);
                NEXUS_AudioProcessor_Close(handle->decoders[i].processor[j]);
                handle->decoders[i].processor[j] = NULL;
                BKNI_Memset( &handle->decoders[i].processorSettings[j], 0, sizeof(handle->decoders[i].processorSettings[j]) );
            }
        }
    }
}

/* stop or clear so that we're not holding anything open */
static void NEXUS_SimpleAudioDecoder_P_ReleaseResources( NEXUS_SimpleAudioDecoderHandle handle )
{
    NEXUS_SimpleAudioDecoderSelector selector;
    if (handle->clientStarted || handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || handle->playback.state == state_started) {
        NEXUS_SimpleAudioDecoder_Stop(handle);
    }
    nexus_simpleaudiodecoder_freeprocessing(handle);

    NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
    for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
        NEXUS_AudioDecoderHandle decoder = nexus_p_get_decoder(handle, selector);
        if (decoder) {
            NEXUS_AudioDecoder_Clear_priv(decoder);
        }
    }
    NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);

    NEXUS_SimpleAudioDecoder_SetStcChannel(handle, NULL);
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].primer) {
        NEXUS_AudioDecoderPrimer_Destroy(handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].primer);
        handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].primer = NULL;
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].primer) {
        NEXUS_AudioDecoderPrimer_Destroy(handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].primer);
        handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].primer = NULL;
    }
    BDBG_ASSERT(!handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].primer);
}
#endif

static void NEXUS_SimpleAudioDecoder_P_Release( NEXUS_SimpleAudioDecoderHandle handle )
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioDecoder, handle);
    if (handle->acquired) {
        NEXUS_SimpleAudioDecoder_Release(handle);
    }
#if NEXUS_HAS_AUDIO
    NEXUS_SimpleAudioDecoder_P_ReleaseResources(handle); /* SimpleAudioDecoder may be used without Acquire */
#endif
    NEXUS_OBJECT_UNREGISTER(NEXUS_SimpleAudioDecoder, handle, Destroy);
    return;
}

static NEXUS_AudioDecoderHandle nexus_p_get_decoder(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector)
{
    switch (selector) {
    case NEXUS_SimpleAudioDecoderSelector_ePrimary: return handle->serverSettings.primary;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary: return handle->serverSettings.secondary;
    case NEXUS_SimpleAudioDecoderSelector_eDescription: return handle->serverSettings.description;
    default: return NULL;
    }
}

static struct NEXUS_SimpleAudioDecoder_P_CodecSettings *nexus_p_get_decoder_codec_settings(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector)
{
    switch (selector) {
    default:
    case NEXUS_SimpleAudioDecoderSelector_ePrimary: return &handle->codecSettings.primary;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary: return &handle->codecSettings.secondary;
    case NEXUS_SimpleAudioDecoderSelector_eDescription: return &handle->codecSettings.description;
    }
}

static void NEXUS_SimpleAudioDecoder_P_Finalizer( NEXUS_SimpleAudioDecoderHandle handle )
{
    unsigned i;
    NEXUS_SimpleAudioDecoderSelector selector;
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioDecoder, handle);
    /* no linking */
    if (handle->displayEncode.slave) {
        BDBG_MSG(("finalizer unlinking slave %p from master %p", (void*)handle->displayEncode.slave, (void*)handle));
        handle->displayEncode.slave->displayEncode.master = NULL;
    }
    if (handle->displayEncode.master) {
        BDBG_MSG(("finalizer unlinking master %p from slave %p", (void*)handle->displayEncode.master, (void*)handle));
        handle->displayEncode.master->displayEncode.slave = NULL;
    }
    BLST_S_REMOVE(&handle->server->decoders, handle, NEXUS_SimpleAudioDecoder, link);
#if NEXUS_HAS_AUDIO
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
        if (handle->serverSettings.spdif.outputs[i]) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_SpdifOutput, handle->serverSettings.spdif.outputs[i]);
        }
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
        if (handle->serverSettings.hdmi.outputs[i]) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->serverSettings.hdmi.outputs[i]);
        }
    }
#endif
    if(handle->serverSettings.simplePlayback) {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_SimpleAudioPlaybackServer, handle->serverSettings.simplePlayback);
    }
    for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
        struct NEXUS_SimpleAudioDecoder_P_CodecSettings *pCodecSettings;
        pCodecSettings = nexus_p_get_decoder_codec_settings(handle, selector);
        for(i=0;i<NEXUS_AudioCodec_eMax;i++) {
            if (pCodecSettings->codecSettings[i]) {
                BKNI_Free(pCodecSettings->codecSettings[i]);
            }
        }
    }
    if ( SIMPLEMASTER(handle->server, handle) ) {
        handle->server->masterServerSettings.masterHandle = NULL;
    }

    NEXUS_OBJECT_DESTROY(NEXUS_SimpleAudioDecoder, handle);
    BKNI_Free(handle);
    return;
}

NEXUS_OBJECT_CLASS_MAKE_WITH_RELEASE(NEXUS_SimpleAudioDecoder, NEXUS_SimpleAudioDecoder_Destroy);

void NEXUS_SimpleAudioDecoder_GetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->server != server) {BERR_TRACE(NEXUS_INVALID_PARAMETER); return;}
    *pSettings = handle->serverSettings;
}

#if NEXUS_HAS_AUDIO
static void NEXUS_SimpleAudioDecoder_P_GetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->server != server) {BERR_TRACE(NEXUS_INVALID_PARAMETER); return;}
    *pSettings = server->masterServerSettings;
}

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_ApplyCodecSettings(NEXUS_AudioDecoderHandle decoder, const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *codecSettings)
{
    NEXUS_Error rc;
    unsigned i;

    for(i=0;i<NEXUS_AudioCodec_eMax;i++) {
        if(codecSettings->codecSettings[i]) {
            rc = NEXUS_AudioDecoder_SetCodecSettings(decoder, codecSettings->codecSettings[i]);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_SimpleAudioDecoder_P_RestoreDecoder(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector)
{
    NEXUS_AudioDecoderHandle decoder;
    decoder = nexus_p_get_decoder(handle, selector);
    if (decoder && g_default) {
        unsigned i;
        const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *pCodecSettings;
        pCodecSettings = nexus_p_get_decoder_codec_settings(handle, selector);
        for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
            if (pCodecSettings->codecSettings[i] && g_default) {
                (void)NEXUS_AudioDecoder_SetCodecSettings(decoder, &g_default->codecSettings[i]);
            }
        }
        (void)NEXUS_AudioDecoder_SetSettings(decoder, &g_default->settings);
    }
}

/* Check if there is a primary decoder running AC4 */
static bool nexus_p_check_for_ac4_master(NEXUS_SimpleAudioDecoderHandle handle)
{
    int i;

    if (handle->startSettings.primary.codec == NEXUS_AudioCodec_eAc4)
    {
        if (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) {
            return true;
        }
        else {
            if (handle->startSettings.master) {
                return true;
            }
        }
    }

    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        if (handle->serverSettings.persistent[i].decoder != NULL) {
            NEXUS_AudioDecoderStatus decoderStatus;
            bool mixerMaster = false;
            NEXUS_AudioDecoder_GetStatus(handle->serverSettings.persistent[i].decoder, &decoderStatus);
            if (decoderStatus.codec == NEXUS_AudioCodec_eAc4) {
                NEXUS_AudioInputHandle connector = NEXUS_AudioDecoder_GetConnector(handle->serverSettings.persistent[i].decoder, NEXUS_AudioDecoderConnectorType_eMultichannel);
                NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
                mixerMaster = NEXUS_AudioMixer_IsInputMaster_priv(handle->serverSettings.mixers.multichannel, connector);
                NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
                if (mixerMaster) {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool nexus_p_check_for_presentation_reconfig(NEXUS_SimpleAudioDecoderHandle handle, bool alternatePresentationAvailable)
{
    bool mainConnection = false;

    NEXUS_SimpleAudioDecoder_P_GetServerSettings(handle->server, handle, &handle->masterSettings);

    if ( handle->masterSettings.dac.output && handle->masterSettings.dac.input) {
        NEXUS_AudioInput_IsConnectedToOutput(handle->masterSettings.dac.input, NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output), &mainConnection);
        if ( mainConnection && alternatePresentationAvailable && handle->masterSettings.dac.presentation == NEXUS_AudioPresentation_eAlternateStereo ) {
            return true;
        }
        else if (!mainConnection && (!alternatePresentationAvailable || handle->masterSettings.dac.presentation == NEXUS_AudioPresentation_eMain)) {
            return true;
        }
    }
    if ( handle->masterSettings.i2s[0].output && handle->masterSettings.i2s[0].input) {
        NEXUS_AudioInput_IsConnectedToOutput(handle->masterSettings.i2s[0].input, NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output), &mainConnection);
        if ( mainConnection && alternatePresentationAvailable && handle->masterSettings.i2s[0].presentation == NEXUS_AudioPresentation_eAlternateStereo ) {
            return true;
        }
        else if (!mainConnection && (!alternatePresentationAvailable || handle->masterSettings.i2s[0].presentation == NEXUS_AudioPresentation_eMain)) {
            return true;
        }
    }
    if ( handle->masterSettings.i2s[1].output && handle->masterSettings.i2s[1].input) {
        NEXUS_AudioInput_IsConnectedToOutput(handle->masterSettings.i2s[1].input, NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output), &mainConnection);
        if ( mainConnection && alternatePresentationAvailable && handle->masterSettings.i2s[1].presentation == NEXUS_AudioPresentation_eAlternateStereo ) {
            return true;
        }
        else if (!mainConnection && (!alternatePresentationAvailable || handle->masterSettings.i2s[1].presentation == NEXUS_AudioPresentation_eMain)) {
            return true;
        }
    }
    return false;
}

static void nexus_p_check_for_presentation_change(NEXUS_SimpleAudioDecoderHandle handle, bool *changed)
{
    bool alternatePresentationAvailable = false;
    bool reconfigRequired = false;
    alternatePresentationAvailable = nexus_p_check_for_ac4_master(handle);

    reconfigRequired = nexus_p_check_for_presentation_reconfig(handle, alternatePresentationAvailable);

    *changed = reconfigRequired;
    return;
}

static NEXUS_AudioInputHandle nexus_p_get_alternate_presentation_input(NEXUS_SimpleAudioDecoderHandle handle)
{
    int i;
    if (handle->startSettings.primary.codec == NEXUS_AudioCodec_eAc4)
    {
        if (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) {
            return NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, NEXUS_AudioConnectorType_eAlternateStereo);
        }
        else {
            if (handle->startSettings.master) {
                return NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, NEXUS_AudioConnectorType_eAlternateStereo);
            }
        }
    }

    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        if (handle->serverSettings.persistent[i].decoder != NULL) {
            NEXUS_AudioDecoderStatus decoderStatus;
            bool mixerMaster;
            NEXUS_AudioDecoder_GetStatus(handle->serverSettings.persistent[i].decoder, &decoderStatus);
            if (decoderStatus.codec == NEXUS_AudioCodec_eAc4) {
                NEXUS_AudioInputHandle connector = NEXUS_AudioDecoder_GetConnector(handle->serverSettings.persistent[i].decoder, NEXUS_AudioDecoderConnectorType_eMultichannel);
                NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
                mixerMaster = NEXUS_AudioMixer_IsInputMaster_priv(handle->serverSettings.mixers.multichannel, connector);
                NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
                if (mixerMaster) {
                    return NEXUS_AudioDecoder_GetConnector(handle->serverSettings.persistent[i].decoder, NEXUS_AudioConnectorType_eAlternateStereo);
                }
            }
        }
    }
    return NULL;
}

/* Check change in server settings and only suspend/resume and remove/re-add outputs if required.
We need to avoid reconfig for change in enabled or stcIndex, or if setting input[] == NULL if disabled.
All other changes require reconfig. */
static bool nexus_p_check_reconfig(NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderServerSettings *pNewSettings)
{
    unsigned i;
    const NEXUS_SimpleAudioDecoderServerSettings *pCurrentSettings = &handle->serverSettings;

    if ((pNewSettings->primary != pCurrentSettings->primary ||
        pNewSettings->secondary != pCurrentSettings->secondary ||
        pNewSettings->description != pCurrentSettings->description) && handle->acquired &&
        pNewSettings->type == NEXUS_SimpleAudioDecoderType_eDynamic) return true;
    /* If HDMI input is enabled we do not want to change any output setting just save them*/
    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        if (pNewSettings->spdif.input[i] != pCurrentSettings->spdif.input[i] && !handle->hdmiInput.handle) return true;
        if (pNewSettings->hdmi.input[i] != pCurrentSettings->hdmi.input[i] && !handle->hdmiInput.handle) return true;
    }
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
        if (pNewSettings->spdif.outputs[i] != pCurrentSettings->spdif.outputs[i]) return true;
    }
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
        if (pNewSettings->hdmi.outputs[i] != pCurrentSettings->hdmi.outputs[i]) return true;
    }

    if (pNewSettings->capture.output != pCurrentSettings->capture.output) return true;

    if (nexus_p_check_for_ac4_master(handle)) {
        if (pCurrentSettings->dac.output) {
            if (pNewSettings->dac.presentation != pCurrentSettings->dac.presentation) return true;
        }
        if (pCurrentSettings->i2s[0].output) {
            if (pNewSettings->i2s[0].presentation != pCurrentSettings->i2s[0].presentation) return true;
        }
        if (pCurrentSettings->i2s[1].output) {
            if (pNewSettings->i2s[1].presentation != pCurrentSettings->i2s[1].presentation) return true;
        }
    }

    return false;
}

static void NEXUS_SimpleAudioDecoder_P_SetServerSettings(NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    if ( server->masterServerSettings.masterHandle == NULL ||
         handle->serverSettings.masterHandle == handle ) {
        BKNI_Memcpy(&server->masterServerSettings, pSettings, sizeof(NEXUS_SimpleAudioDecoderServerSettings));
    }
    return;
}
#endif

NEXUS_Error NEXUS_SimpleAudioDecoder_SetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
#if NEXUS_HAS_AUDIO
    unsigned i;
    NEXUS_Error rc;
    bool configOutputs;
    bool suspendedPersistents[NEXUS_MAX_AUDIO_DECODERS];

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->server != server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    /* eMax -> valid and valid -> eMax are ok. Anything else is not allowed. */
    if ( handle->serverSettings.type != NEXUS_SimpleAudioDecoderType_eMax &&
         handle->serverSettings.type != pSettings->type &&
         pSettings->type != NEXUS_SimpleAudioDecoderType_eMax) {
        BDBG_ERR(("Simple Audio Decoder type is not allowed to change on the fly."));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    #if BDBG_DEBUG_BUILD
    if ( handle->serverSettings.type != pSettings->type ) {
        BDBG_MSG(("Simple decoder setting type to: %s", (pSettings->type==NEXUS_SimpleAudioDecoderType_ePersistent)?"Persistent":(pSettings->type==NEXUS_SimpleAudioDecoderType_eStandalone)?"Standalone":"Dynamic"));
    }
    #endif

    if ( pSettings->type < NEXUS_SimpleAudioDecoderType_eMax) {
        configOutputs = nexus_p_check_reconfig(handle, pSettings);

        if (pSettings->primary==NULL) {
            NEXUS_SimpleAudioDecoder_P_RestoreDecoder(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary);
        }
        if (pSettings->secondary==NULL) {
            NEXUS_SimpleAudioDecoder_P_RestoreDecoder(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary);
        }
        if (pSettings->description==NULL) {
            NEXUS_SimpleAudioDecoder_P_RestoreDecoder(handle, NEXUS_SimpleAudioDecoderSelector_eDescription);
        }
    }
    else {
        configOutputs = false;
    }
    if (configOutputs) {
        NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);
    }
    BDBG_MSG(("NEXUS_SimpleAudioDecoder_SetServerSettings %p: configOutputs %d", (void*)handle, configOutputs));

    /* before storing to handle->serverSettings, we must ACQUIRE/RELEASE as needed */
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
        if (pSettings->spdif.outputs[i] && !handle->serverSettings.spdif.outputs[i]) {
            NEXUS_OBJECT_ACQUIRE(handle, NEXUS_SpdifOutput, pSettings->spdif.outputs[i]);
        }
        else if (!pSettings->spdif.outputs[i] && handle->serverSettings.spdif.outputs[i]) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_SpdifOutput, handle->serverSettings.spdif.outputs[i]);
        }
        else {
            BDBG_ASSERT(pSettings->spdif.outputs[i] == handle->serverSettings.spdif.outputs[i]);
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
        if (pSettings->hdmi.outputs[i] && !handle->serverSettings.hdmi.outputs[i]) {
            NEXUS_OBJECT_ACQUIRE(handle, NEXUS_HdmiOutput, pSettings->hdmi.outputs[i]);
        }
        else if (!pSettings->hdmi.outputs[i] && handle->serverSettings.hdmi.outputs[i]) {
            NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiOutput, handle->serverSettings.hdmi.outputs[i]);
        }
        else {
            BDBG_ASSERT(pSettings->hdmi.outputs[i] == handle->serverSettings.hdmi.outputs[i]);
        }
    }
#endif
    if(pSettings->simplePlayback) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_SimpleAudioPlaybackServer, pSettings->simplePlayback);
    }
    if(handle->serverSettings.simplePlayback) {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_SimpleAudioPlaybackServer, handle->serverSettings.simplePlayback);
    }

    /* Copy suspended persistents */
    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        suspendedPersistents[i] = handle->serverSettings.persistent[i].suspended;
    }

    handle->serverSettings = *pSettings;

    if ( pSettings->type == NEXUS_SimpleAudioDecoderType_eMax ){
        /* this means we aren't ready for primetime yet. save settings and return. */
        return NEXUS_SUCCESS;
    }

    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        handle->serverSettings.persistent[i].suspended = suspendedPersistents[i];
    }

    if (!g_default && handle->serverSettings.primary) {
        /* this is added for backward compat with the risk that we wipe out client settings already made */
        NEXUS_SimpleAudioDecoderHandle d;
        NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(handle->serverSettings.primary);
        for (d=BLST_S_FIRST(&server->decoders); d; d=BLST_S_NEXT(d, link)) {
            nexus_simpleaudiodecoder_p_set_default_settings(d);
        }
    }

    if (handle->stcChannel)
    {
        /* notify ssc that something may have changed */
        NEXUS_SimpleStcChannel_SetAudio_priv(handle->stcChannel, handle);
    }

    NEXUS_SimpleAudioDecoder_P_SetServerSettings(server, handle, &handle->serverSettings);

    if (configOutputs) {
        rc = NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */

        if (handle->clientStarted) {
            rc = NEXUS_SimpleAudioDecoder_SetTrickState(handle, &handle->trickState);
            if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        }
    }
#else
    BSTD_UNUSED(server);
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif

    return 0;
}

#if NEXUS_HAS_AUDIO
static bool NEXUS_SimpleAudioDecoder_P_IsPriming(NEXUS_SimpleAudioDecoderHandle handle)
{
    return
        handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_priming ||
        handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_priming ||
        handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_priming;
}
#endif

void NEXUS_SimpleAudioDecoder_GetStcStatus_priv(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleStcChannelDecoderStatus * pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioDecoder, handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->connected = CONNECTED(handle);
    pStatus->stc.index = handle->stcIndex;
    NEXUS_SimpleEncoder_GetStcStatus_priv(handle->encoder.handle, &pStatus->encoder);
}

NEXUS_Error NEXUS_SimpleAudioDecoder_MoveServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle src, NEXUS_SimpleAudioDecoderHandle dest )
{
#if NEXUS_HAS_AUDIO
    int rc;
    unsigned i;
    NEXUS_SimpleAudioPlaybackServerHandle dest_simplePlayback;

    BDBG_OBJECT_ASSERT(src, NEXUS_SimpleAudioDecoder);
    BDBG_OBJECT_ASSERT(dest, NEXUS_SimpleAudioDecoder);
    if (src->server != server || dest->server != server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    if (dest->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || dest->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || dest->playback.state == state_started) {
        /* When moving settings (resources) destination should be idle */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    /* verify that destination doesn't hold any resources */
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
        if (dest->serverSettings.spdif.outputs[i]) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }

#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
        if (dest->serverSettings.hdmi.outputs[i]) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
    }
#endif

    BDBG_MSG(("MoveServerSettings %p -> %p", (void *)src, (void *)dest));
    if (src->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || src->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || src->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started || dest->playback.state == state_started) {
        nexus_simpleaudiodecoder_p_stop(src);
    }

    dest->trickState = src->trickState;

    /* don't copy/modify serverSettings.simplePlayback */
    dest_simplePlayback = dest->serverSettings.simplePlayback;
    dest->serverSettings = src->serverSettings;
    dest->serverSettings.simplePlayback = dest_simplePlayback;

    dest->stcIndex = src->stcIndex;
    dest->currentSpdifInput = src->currentSpdifInput;
    dest->currentHdmiInput = src->currentHdmiInput;
    dest->currentCaptureInput = src->currentCaptureInput;
    dest->mixers.connected = src->mixers.connected;
    dest->mixers.suspended = src->mixers.suspended;
    dest->encoder = src->encoder;
    if (dest->encoder.displayEncode && !src->displayEncode.slave) {
        BDBG_MSG(("link"));
        src->displayEncode.slave = dest;
        dest->displayEncode.master = src;
    }
    else if (src->displayEncode.master) {
        BDBG_MSG(("unlink"));
        src->displayEncode.master = NULL;
        dest->displayEncode.slave = NULL;
    }

    if (!dest->hdmiInput.handle && src->hdmiInput.handle)
    {
        NEXUS_SimpleAudioDecoder_P_RemoveOutputs(src);
        dest->currentHdmiInput = NULL;
        NEXUS_SimpleAudioDecoder_P_AddOutputs(dest);
    }

    src->serverSettings.primary = NULL;
    src->serverSettings.secondary = NULL;
    src->serverSettings.description = NULL;
    src->currentSpdifInput = NULL;
    src->currentHdmiInput = NULL;
    src->currentCaptureInput = NULL;
    src->stcIndex = -1;

    if (src->stcChannel) {
        /* source has lost its decoders just above here */
        NEXUS_SimpleStcChannel_SetAudio_priv(src->stcChannel, src);
    }
    if (dest->stcChannel) {
        /* dest may have gained some decoders if source had some before */
        NEXUS_SimpleStcChannel_SetAudio_priv(dest->stcChannel, dest);
    }

    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
        if (dest->serverSettings.spdif.outputs[i]) {
            NEXUS_OBJECT_ACQUIRE(dest, NEXUS_SpdifOutput, dest->serverSettings.spdif.outputs[i]);
            NEXUS_OBJECT_RELEASE(src, NEXUS_SpdifOutput, dest->serverSettings.spdif.outputs[i]);
            src->serverSettings.spdif.outputs[i] = NULL;
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
        if (dest->serverSettings.hdmi.outputs[i]) {
            NEXUS_OBJECT_ACQUIRE(dest, NEXUS_HdmiOutput, dest->serverSettings.hdmi.outputs[i]);
            NEXUS_OBJECT_RELEASE(src, NEXUS_HdmiOutput, dest->serverSettings.hdmi.outputs[i]);
            src->serverSettings.hdmi.outputs[i] = NULL;
        }
    }
#endif
    dest->serverSettings.masterHandle = dest;
    NEXUS_SimpleAudioDecoder_P_SetServerSettings(server, dest, &dest->serverSettings);
    src->serverSettings.masterHandle = NULL;

    if (CONNECTED(dest)) {
        rc = NEXUS_SimpleAudioDecoder_SetSettings(dest, &dest->settings);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
        if (dest->clientStarted) {
            rc = nexus_simpleaudiodecoder_p_start(dest);
            if (rc) {
                rc = BERR_TRACE(rc); /* fall through */
            }
            else {
                rc = NEXUS_SimpleAudioDecoder_SetTrickState(dest, &dest->trickState);
                if (rc) {rc = BERR_TRACE(rc);} /* fall through */
            }
        }

        rc = NEXUS_SimpleAudioDecoder_Resume(dest);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    }
#else
    BSTD_UNUSED(server);
    BSTD_UNUSED(src);
    BSTD_UNUSED(dest);
#endif

    return 0;
}

/**
client functions
**/

NEXUS_SimpleAudioDecoderHandle NEXUS_SimpleAudioDecoder_Acquire( unsigned index )
{
    NEXUS_SimpleAudioDecoderHandle handle;
    NEXUS_Error rc;

    for (handle=nexus_simple_audio_decoder_p_first(); handle; handle = nexus_simple_audio_decoder_p_next(handle)) {
        BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
        if (handle->index == index) {
            if (handle->acquired) {
                BERR_TRACE(NEXUS_NOT_AVAILABLE);
                return NULL;
            }

            rc = NEXUS_CLIENT_RESOURCES_ACQUIRE(simpleAudioDecoder,IdList,index);
            if (rc) { rc = BERR_TRACE(rc); return NULL; }

            handle->acquired = true;
            return handle;
        }
    }
    return NULL;
}

void NEXUS_SimpleAudioDecoder_Release( NEXUS_SimpleAudioDecoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
#if NEXUS_HAS_AUDIO
    /* IPC handle validation will only allow this call if handle is owned by client.
    For non-IPC used, acquiring is not required, so acquired boolean is not checked in any other API. */
    if (!handle->acquired) {
        BERR_TRACE(NEXUS_NOT_AVAILABLE);
        return;
    }

    NEXUS_SimpleAudioDecoder_P_ReleaseResources(handle);
    nexus_simpleaudiodecoder_p_set_default_settings(handle);

    handle->acquired = false;
    NEXUS_CLIENT_RESOURCES_RELEASE(simpleAudioDecoder,IdList,handle->index);
#endif
    return;
}

void NEXUS_SimpleAudioDecoder_GetDefaultStartSettings( NEXUS_SimpleAudioDecoderStartSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
#if NEXUS_HAS_AUDIO
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->primary);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->secondary);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->description);
    pSettings->secondary.secondaryDecoder = true;
    pSettings->description.secondaryDecoder = true;
#endif
    NEXUS_CallbackDesc_Init(&pSettings->passthroughBuffer.dataCallback);
}

#if NEXUS_HAS_AUDIO
static bool nexus_p_is_compressed_output(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioInputHandle audioInput)
{
    NEXUS_AudioDecoderConnectorType i;
    for (i=0;i<NEXUS_AudioDecoderConnectorType_eMax;i++) {
        if ((handle->serverSettings.primary && NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, i) == audioInput) ||
            (handle->serverSettings.secondary && NEXUS_AudioDecoder_GetConnector(handle->serverSettings.secondary, i) == audioInput) ||
            (handle->serverSettings.passthroughPlayback && NEXUS_AudioPlayback_GetConnector(handle->serverSettings.passthroughPlayback) == audioInput)) {
            switch (i) {
            case NEXUS_AudioConnectorType_eCompressed:
            case NEXUS_AudioConnectorType_eCompressed4x:
            case NEXUS_AudioConnectorType_eCompressed16x:
                return true;
            default:
                return false;
            }
        }
    }
    return false;
}

static NEXUS_Error nexus_p_update_static_outputs( NEXUS_SimpleAudioDecoderHandle handle )
{
    NEXUS_AudioPresentation presentation = NEXUS_AudioPresentation_eMain;
    NEXUS_Error rc;
    bool resume = false;
    NEXUS_SimpleAudioDecoderHandle masterHandle = NULL;

    NEXUS_SimpleAudioDecoder_P_GetServerSettings(handle->server, handle, &handle->masterSettings);

    masterHandle = handle->masterSettings.masterHandle;

    if (masterHandle && !masterHandle->mixers.suspended) {
        rc = NEXUS_SimpleAudioDecoder_Suspend(masterHandle);
        if (rc) {return BERR_TRACE(rc);}
        resume = true;
    }

    if (nexus_p_check_for_ac4_master(handle)){
        presentation = NEXUS_AudioPresentation_eAlternateStereo;
    }

    if ( handle->masterSettings.dac.output ) {
        if ( handle->masterSettings.dac.presentation == NEXUS_AudioPresentation_eAlternateStereo &&
             presentation == NEXUS_AudioPresentation_eAlternateStereo ) {
            NEXUS_AudioInputHandle input;

            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output));
            input = nexus_p_get_alternate_presentation_input(handle);
            if (input) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output), input);
            }
            else {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output), handle->masterSettings.dac.input);
            }
            if (rc) { return BERR_TRACE(rc); }
        }
        else if (handle->masterSettings.dac.input) {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output));
            BDBG_MSG(("%p: add input %p -> dac", (void *)handle, (void *)handle->masterSettings.dac.input));
            rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(handle->masterSettings.dac.output), handle->masterSettings.dac.input);
            if (rc) { return BERR_TRACE(rc); }
        }
    }
    if ( handle->masterSettings.i2s[0].output ) {
        if (handle->masterSettings.i2s[0].presentation == NEXUS_AudioPresentation_eAlternateStereo &&
            presentation == NEXUS_AudioPresentation_eAlternateStereo ) {
            NEXUS_AudioInputHandle input;

            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output));
            input = nexus_p_get_alternate_presentation_input(handle);
            if (input) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output), input);
            }
            else {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output), handle->masterSettings.i2s[0].input);
            }
            if (rc) { return BERR_TRACE(rc); }
        }
        else if (handle->masterSettings.i2s[0].input) {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output));
            BDBG_MSG(("%p: add input %p -> i2s0", (void *)handle, (void *)handle->masterSettings.i2s[0].input));
            rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[0].output), handle->masterSettings.i2s[0].input);
            if (rc) { return BERR_TRACE(rc); }
        }
    }
    if ( handle->masterSettings.i2s[1].output ) {
        if (handle->masterSettings.i2s[1].presentation == NEXUS_AudioPresentation_eAlternateStereo &&
            presentation == NEXUS_AudioPresentation_eAlternateStereo) {
            NEXUS_AudioInputHandle input;

            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output));
            input = nexus_p_get_alternate_presentation_input(handle);
            if (input) {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output), input);
            }
            else {
                rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output), handle->masterSettings.i2s[1].input);
            }
            if (rc) { return BERR_TRACE(rc); }
        }
        else if (handle->masterSettings.i2s[1].input) {
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output));
            BDBG_MSG(("%p: add input %p -> i2s0", (void *)handle, (void *)handle->masterSettings.i2s[1].input));
            rc = NEXUS_AudioOutput_AddInput(NEXUS_I2sOutput_GetConnector(handle->masterSettings.i2s[1].output), handle->masterSettings.i2s[1].input);
            if (rc) { return BERR_TRACE(rc); }
        }
    }
    if (resume) {
        rc = NEXUS_SimpleAudioDecoder_Resume(masterHandle);
        if (rc) {return BERR_TRACE(rc);}
    }
    return BERR_SUCCESS;
}

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddOutputs( NEXUS_SimpleAudioDecoderHandle handle )
{
    NEXUS_AudioInputHandle spdifInput, hdmiInput, captureInput;
    unsigned i;
    NEXUS_Error rc;
    NEXUS_AudioCodec primaryCodec, secondaryCodec;
    bool presentationChanged = false;

    NEXUS_GetAudioCapabilities(&handle->audioCapabilities);

    if (handle->encoder.handle) {
        return 0;
    }

    if (handle->startSettings.passthroughBuffer.enabled && NULL == handle->serverSettings.passthroughPlayback) {
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if (handle->clientStarted) {
        primaryCodec = handle->startSettings.primary.codec;
        secondaryCodec = handle->serverSettings.secondary && handle->startSettings.secondary.pidChannel ? handle->startSettings.secondary.codec : NEXUS_AudioCodec_eUnknown;
    }
    else {
        primaryCodec = secondaryCodec = NEXUS_AudioCodec_eUnknown;
    }

    nexus_p_check_for_presentation_change(handle, &presentationChanged);

    /* determine inputs */
    spdifInput = NULL;
    hdmiInput = NULL;
    captureInput = NULL;
    if (handle->startSettings.passthroughBuffer.enabled) {
        NEXUS_AudioInput passthroughInput;

        passthroughInput = NEXUS_AudioPlayback_GetConnector(handle->serverSettings.passthroughPlayback);

        /* Send to SPDIF if sampling rate is supported */
        spdifInput = (handle->startSettings.passthroughBuffer.sampleRate <= 48000) ? passthroughInput : NULL;
        /* Send to HDMI if it supports compressed AC3 or AC3+ */
        hdmiInput = nexus_p_is_compressed_output(handle, handle->serverSettings.hdmi.input[NEXUS_AudioCodec_eAc3]) ||
                    nexus_p_is_compressed_output(handle, handle->serverSettings.hdmi.input[NEXUS_AudioCodec_eAc3Plus]) ? passthroughInput : NULL;
    } else {
        if (secondaryCodec != NEXUS_AudioCodec_eUnknown) {
            spdifInput = handle->serverSettings.spdif.input[secondaryCodec];
        }
        if (!spdifInput) {
            spdifInput = handle->serverSettings.spdif.input[primaryCodec];
        }
#if NEXUS_HAS_HDMI_OUTPUT
        if (secondaryCodec != NEXUS_AudioCodec_eUnknown) {
            hdmiInput = handle->serverSettings.hdmi.input[secondaryCodec];
        }
        if (!hdmiInput) {
            hdmiInput = handle->serverSettings.hdmi.input[primaryCodec];
        }
#endif
        if (secondaryCodec != NEXUS_AudioCodec_eUnknown) {
            captureInput = handle->serverSettings.capture.input[secondaryCodec];
        }
        if (!captureInput) {
            captureInput = handle->serverSettings.capture.input[primaryCodec];
        }
    }

#if NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_HDMI_OUTPUT
    if (handle->hdmiInput.handle) {
        /* for now, always passthrough hdmi input audio. this is lower latency than doing DSP passthrough. */
        if (handle->serverSettings.hdmi.outputs[0]) {
            hdmiInput = NEXUS_AudioInputCapture_GetConnector(handle->hdmiInput.inputCapture);
            spdifInput = handle->serverSettings.spdif.input[NEXUS_AudioCodec_eUnknown];
        }
    }
#endif

    /* if outputs are the same, don't change. this avoids needless glitches when starting/stopping decode
       but if we were supsended already we need to ensure pcm stereo outputs resume */
    if (handle->currentSpdifInput == spdifInput &&
        handle->currentHdmiInput == hdmiInput &&
        handle->currentCaptureInput == captureInput &&
        !handle->suspended) {
        /* Spdif, HDMI, capture may not have changed due to being a persisent decoder.
           Need to look through all decoders.  Something may require it. */
        if (presentationChanged) {
            rc = nexus_p_update_static_outputs(handle);
            if (rc) {return BERR_TRACE(rc);}
        }
        return 0;
    }

    /* always remove because of default configuration */
    NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);

    if (spdifInput) {
        for (i=0;i<handle->audioCapabilities.numOutputs.spdif;i++) {
            if (handle->serverSettings.spdif.outputs[i]) {
                BDBG_MSG(("%p: add input %p -> spdif%d", (void *)handle, (void *)spdifInput, i));
                rc = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(handle->serverSettings.spdif.outputs[i]), spdifInput);
                if (rc) { rc = BERR_TRACE(rc); goto error_start;}
            }
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (hdmiInput) {
        for (i=0;i<handle->audioCapabilities.numOutputs.hdmi;i++) {
            if (handle->serverSettings.hdmi.outputs[i]) {
                BDBG_MSG(("%p: add input %p -> hdmi%d", (void *)handle, (void *)hdmiInput, i));
                rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(handle->serverSettings.hdmi.outputs[i]), hdmiInput);
                if (rc) { rc = BERR_TRACE(rc); goto error_start;}
            }
        }
    }
#endif
    if (captureInput) {
        if (handle->serverSettings.capture.output) {
            BDBG_MSG(("%p: add input %p -> capture", (void *)handle, (void *)captureInput));
            rc = NEXUS_AudioOutput_AddInput(NEXUS_AudioCapture_GetConnector(handle->serverSettings.capture.output), captureInput);
            if (rc) { rc = BERR_TRACE(rc); goto error_start;}
        }
        else {
            captureInput = NULL;
        }
    }

    rc = nexus_p_update_static_outputs(handle);
    if (rc) {rc = BERR_TRACE(rc);}

    rc = NEXUS_SimpleAudioDecoder_Resume(handle);
    if (rc) {rc = BERR_TRACE(rc);}

    handle->currentSpdifInput = spdifInput;
    handle->currentSpdifInputCompressed = handle->currentSpdifInput && nexus_p_is_compressed_output(handle, handle->currentSpdifInput);
    handle->currentHdmiInput = hdmiInput;
    handle->currentHdmiInputCompressed = handle->currentHdmiInput && nexus_p_is_compressed_output(handle, handle->currentHdmiInput);
    handle->currentCaptureInput = captureInput;
    return 0;

error_start:
    /* don't remove outputs here */
    return rc;
}

static void nexus_p_check_decoder(NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_AudioDecoder_GetConnector(audioDecoder, 0);
}

static void NEXUS_SimpleAudioDecoder_P_RemoveOutputs( NEXUS_SimpleAudioDecoderHandle handle )
{
    unsigned i;
    int rc;

    NEXUS_GetAudioCapabilities(&handle->audioCapabilities);

    if (handle->encoder.handle) {
        return;
    }
    /* Suspend playbacks first as suspending the decoder will cause the
       decoder to detach from the mixer but if anything is running it can't */
    rc = NEXUS_SimpleAudioDecoder_Suspend(handle);
    if (rc) {
        BERR_TRACE(rc);
        return;
    }

    if (handle->serverSettings.primary) {
        nexus_p_check_decoder(handle->serverSettings.primary);
    }
    if (handle->serverSettings.secondary) {
        nexus_p_check_decoder(handle->serverSettings.secondary);
    }

    if (handle->currentSpdifInput) {
        for (i=0;i<handle->audioCapabilities.numOutputs.spdif;i++) {
            if (handle->serverSettings.spdif.outputs[i]) {
                BDBG_MSG(("%p: remove input %p -> spdif%d", (void *)handle, (void *)handle->currentSpdifInput, i));
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(handle->serverSettings.spdif.outputs[i]));
            }
        }
        handle->currentSpdifInput = NULL;
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (handle->currentHdmiInput) {
        for (i=0;i<handle->audioCapabilities.numOutputs.hdmi;i++) {
            if (handle->serverSettings.hdmi.outputs[i]) {
                BDBG_MSG(("%p: remove input %p -> hdmi%d", (void *)handle, (void *)handle->currentHdmiInput, i));
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(handle->serverSettings.hdmi.outputs[i]));
            }
        }
        handle->currentHdmiInput = NULL;
    }
#endif

    if (handle->currentCaptureInput) {
        if (handle->serverSettings.capture.output)
        {
            BDBG_MSG(("%p: remove input %p -> capture", (void *)handle, (void *)handle->currentCaptureInput));
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(handle->serverSettings.capture.output));
        }
        handle->currentCaptureInput = NULL;
    }

    /* can't call NEXUS_AudioInput_Shutdown on handle->serverSettings.spdif/hdmi.input[] because it could be stereo connector */
}

static bool nexus_p_simpleaudiodecoder_is_decoder_connected(NEXUS_AudioDecoderHandle audioDecoder)
{
    NEXUS_AudioDecoderConnectorType i;
    for (i=0;i<NEXUS_AudioDecoderConnectorType_eMax;i++) {
        bool connected;
        NEXUS_AudioInputHandle audioInput = NEXUS_AudioDecoder_GetConnector(audioDecoder, i);
        if (audioInput) {
            NEXUS_AudioInput_HasConnectedOutputs(audioInput, &connected);
            if (connected) {
                return true;
            }
        }
    }
    return false;
}

static bool nexus_p_simpleaudiodecoder_is_playback_connected(NEXUS_AudioPlaybackHandle playback)
{
    bool connected=false;

    if ( playback ) {
        NEXUS_AudioInput_HasConnectedOutputs(NEXUS_AudioPlayback_GetConnector(playback), &connected);
    }

    return connected;
}
#endif

NEXUS_Error NEXUS_SimpleAudioDecoder_SetStcChannel( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleStcChannelHandle stcChannel )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (stcChannel == handle->stcChannel) {
        return 0;
    }
    if (handle->stcChannel) {
        NEXUS_SimpleStcChannel_SetAudio_priv(handle->stcChannel, NULL);
        NEXUS_OBJECT_RELEASE(handle, NEXUS_SimpleStcChannel, handle->stcChannel);
    }
    if (stcChannel) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_SimpleStcChannel, stcChannel);
        NEXUS_SimpleStcChannel_SetAudio_priv(stcChannel, handle);
    }
    handle->stcChannel = stcChannel;
    return 0;
}

NEXUS_SimpleStcChannelHandle NEXUS_SimpleAudioDecoder_P_GetStcChannel( NEXUS_SimpleAudioDecoderHandle handle )
{
    return handle->stcChannel;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_Start( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderStartSettings *pSettings )
{
    NEXUS_Error cumulativeRc = NEXUS_SUCCESS;
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc = NEXUS_SUCCESS;
    bool bypass_p_start = false;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);

    if (handle->clientStarted) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    /* make sure we have *some* valid input to start */
    if (!pSettings->primary.pidChannel && !pSettings->secondary.pidChannel && !pSettings->description.pidChannel &&
        !pSettings->primary.input && !pSettings->secondary.input && !pSettings->description.input &&
        !pSettings->passthroughBuffer.enabled) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (!CONNECTED(handle)) {
        if (handle->serverSettings.disableMode != NEXUS_SimpleDecoderDisableMode_eSuccess) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        bypass_p_start = (pSettings->primer.pcm || pSettings->primer.compressed)?false:true;
    }

    if (pSettings->primary.pidChannel) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pSettings->primary.pidChannel);
    }
    if (pSettings->secondary.pidChannel) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pSettings->secondary.pidChannel);
    }
    if (pSettings->description.pidChannel) {
        NEXUS_OBJECT_ACQUIRE(handle, NEXUS_PidChannel, pSettings->description.pidChannel);
    }
    handle->clientStarted = true;
    handle->startSettings = *pSettings;

    /* If we are not configured for audio description,
       the primary decoder mixing mode should be configured
       for NEXUS_AudioDecoderMixingMode_eSoundEffects */
    if (!handle->serverSettings.capabilities.ms11 && !handle->serverSettings.capabilities.ms12) {
    if (!handle->serverSettings.description && handle->startSettings.primary.mixingMode == NEXUS_AudioDecoderMixingMode_eDescription) {
        handle->startSettings.primary.mixingMode = NEXUS_AudioDecoderMixingMode_eSoundEffects;
        }
    }

    if (!bypass_p_start) {
        cumulativeRc = rc = nexus_simpleaudiodecoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleAudioDecoder_Stop(handle);
            BERR_TRACE(rc); /* fall through */
        }
        cumulativeRc |= rc = NEXUS_SimpleAudioDecoder_Resume(handle);
        if (rc) {rc = BERR_TRACE(rc);}
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pSettings);
#endif
    return cumulativeRc;
}

#if NEXUS_HAS_AUDIO
/**
Assign stcChannel in the following order:
1) user SimpleStcChannel, if video set the StcChannel
2) server StcChannel
3) user StcChannel
**/
static void copy_start_settings(NEXUS_AudioDecoderStartSettings *pStartSettings, NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector)
{
    switch (selector) {
    default:
    case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        *pStartSettings = handle->startSettings.primary;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary:
        *pStartSettings = handle->startSettings.secondary.pidChannel?handle->startSettings.secondary:handle->startSettings.primary;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eDescription:
        *pStartSettings = handle->startSettings.description;
        break;
    }

    if (handle->stcChannel) {
        /* if doing audio/video sync, and if audio and video were each given a separate stcIndex, we still must
        share a stcChannel, so always prefer video's stcChannel. */
        if (handle->encoder.handle && nexus_simpleencoder_p_nonRealTime(handle->encoder.handle)) {
            /* audio does not share STC with video in NRT */
            pStartSettings->stcChannel = NULL;
        }
        else {
            pStartSettings->stcChannel = NEXUS_SimpleStcChannel_GetServerStcChannel_priv(handle->stcChannel, NEXUS_SimpleDecoderType_eVideo);
        }
        if (!pStartSettings->stcChannel) {
            pStartSettings->stcChannel = NEXUS_SimpleStcChannel_GetServerStcChannel_priv(handle->stcChannel, NEXUS_SimpleDecoderType_eAudio);
        }
    }
}

static NEXUS_AudioInputHandle nexus_simpleaudiodecoder_get_current_decoder_connection(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType, bool traverse)
{
    NEXUS_AudioInputHandle connector = NULL;
    NEXUS_AudioDecoderHandle decoder = NULL;

    switch ( selector ) {
    default:
        break;
    case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        decoder = handle->serverSettings.primary;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eDescription:
        decoder = handle->serverSettings.description;
        break;
    }

    if ( decoder ) {
        connector = NEXUS_AudioDecoder_GetConnector(decoder, connectorType);
    }

    switch ( connectorType ) {
    case NEXUS_AudioConnectorType_eMultichannel:
        BDBG_MSG(("%s: selector %u, multich decoder connector %p", BSTD_FUNCTION, selector, (void*)connector));
        break;
    case NEXUS_AudioConnectorType_eStereo:
        BDBG_MSG(("%s: selector %u, stereo decoder connector %p", BSTD_FUNCTION, selector, (void*)connector));
        break;
    default:
        connector = NULL;
        break;
    }

    /* look for terminal connector from this decoder output */
    if ( connector ) {
        unsigned j;
        NEXUS_AudioInputHandle tail;
        /* upmixing / downmixing connectors need to grab the right connector from the decoder */
        if ( handle->decoders[selector].processor[NEXUS_AudioPostProcessing_eAmbisonic] ) {
            tail = NEXUS_AudioDecoder_GetConnector(decoder, handle->decoders[selector].processorConnectorType[NEXUS_AudioPostProcessing_eAmbisonic]);
            if ( connectorType != handle->decoders[selector].processorConnectorType[NEXUS_AudioPostProcessing_eAmbisonic] ) {
                BDBG_MSG(("%s: forcing decoder %s connector to %s for Ambisonic", BSTD_FUNCTION,
                          connectorType==NEXUS_AudioConnectorType_eMultichannel?"multichannel":"stereo",
                          handle->decoders[selector].processorConnectorType[NEXUS_AudioPostProcessing_eAmbisonic]==NEXUS_AudioConnectorType_eMultichannel?"multichannel":"stereo"));
            }
        }
        else {
            tail = connector;
        }

        /* traverse through post processing */
        if ( traverse ) {
            for ( j = nexus_simpleaudiodecoder_pp_first(); j < NEXUS_AudioPostProcessing_eMax; j = nexus_simpleaudiodecoder_pp_next(j) )
            {
                if ( handle->decoders[selector].processor[j] != NULL ) {
                    bool connected = false;
                    NEXUS_AudioInputHandle next = NULL;

                    switch ( j ) {
                    /* upmixing/downmixing post processors */
                    case NEXUS_AudioPostProcessing_eAmbisonic:
                        next = NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], connectorType);
                        break;
                    /* non-upmixing/downmixing post processors */
                    default:
                        if ( handle->decoders[selector].processorConnectorType[j] == connectorType ) {
                            next = NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], connectorType);
                        }
                        break;
                    }

                    if ( next != NULL ) {
                        NEXUS_AudioInput_IsConnectedToInput(tail, next, &connected);
                        BDBG_MSG(("%s: next valid. connected to %s tail %d", BSTD_FUNCTION,
                                  connectorType==NEXUS_AudioConnectorType_eMultichannel?"multichannel":"stereo",
                                  connected));
                    }

                    if ( connected ) {
                        tail = NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], connectorType);
                    }
                }
            }
        }
        return tail;
    }

    return NULL;
}

static NEXUS_Error nexus_simpleaudiodecoder_connect_processing(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType)
{
    NEXUS_AudioInputHandle tail;

    /* get_current_decoder_connection may override the connector type from the decoder if needed */
    tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType, false);
    BDBG_MSG(("Connect decoder[sel=%u] connection (%s) %p", selector, (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)tail));

    if ( tail )
    {
        unsigned j;
        /* connect post processing graph */
        for(j=nexus_simpleaudiodecoder_pp_first();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_next(j)) {
            if ( handle->decoders[selector].processor[j] != NULL ) {
                bool supported = false;

                switch ( j ) {
                /* upmixing/downmixing post processors */
                case NEXUS_AudioPostProcessing_eAmbisonic:
                    supported = true;
                    break;
                /* non-upmixing/downmixing post processors */
                default:
                    supported = ( handle->decoders[selector].processorConnectorType[j] == connectorType );
                    break;
                }
                if ( supported ) {
                    bool alreadyConnected = false;
                    NEXUS_AudioInput_IsConnectedToInput(tail, NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], connectorType), &alreadyConnected);
                    /* in the case of downmixing or upmixing processors, it may have already been added. */
                    if ( !alreadyConnected ) {
                        NEXUS_AudioProcessor_AddInput(handle->decoders[selector].processor[j], tail);
                    } else {
                        BDBG_MSG(("  (already connected)"));
                    }
                    tail = NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], connectorType);
                    BDBG_MSG(("  --> pp (%u) connection (%s) %p", j, (connectorType==NEXUS_AudioConnectorType_eMultichannel)?"multichannel":"stereo",(void*)tail));
                }
            }
        }
    }

    return NEXUS_SUCCESS;
}

static NEXUS_Error nexus_simpleaudiodecoder_connect_mixer(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType)
{
    NEXUS_Error rc;
    NEXUS_AudioInputHandle tail;
    NEXUS_AudioMixerHandle mixer = NULL;

    switch ( connectorType ) {
    case NEXUS_AudioConnectorType_eStereo:
        mixer = handle->serverSettings.mixers.stereo;
        break;
    case NEXUS_AudioConnectorType_eMultichannel:
        mixer = handle->serverSettings.mixers.multichannel;
        break;
    default:
        break;
    }

    tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType, true);

    BDBG_MSG(("current decoder tail (%s) %p selector %d", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)tail, selector));
    if ( mixer && tail ) {
        BDBG_MSG(("--> mixer (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer));
        rc = NEXUS_AudioMixer_AddInput(mixer, tail);
        if ( rc != NEXUS_SUCCESS ) {
            return BERR_TRACE(rc);
        }

        if (handle->serverSettings.type != NEXUS_SimpleAudioDecoderType_ePersistent) {
            if ( selector == NEXUS_SimpleAudioDecoderSelector_ePrimary ) {
                NEXUS_AudioMixerSettings mixerSettings;
                NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
                BDBG_MSG(("     setting mixer (%s) %p master to tail %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer, (void*)tail));
                mixerSettings.master = tail;
                rc = NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
                if ( rc != NEXUS_SUCCESS ) {
                    return BERR_TRACE(rc);
                }
            }
        }
        else {
            if (selector == NEXUS_SimpleAudioDecoderSelector_ePrimary) {
                NEXUS_AudioMixerSettings mixerSettings;
                NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
                if ( mixerSettings.mixUsingDsp && handle->startSettings.master )
                {
                    BDBG_MSG(("     setting mixer (%s) %p master to tail %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer, (void*)tail));
                    mixerSettings.master = tail;
                }
                rc = NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
                if ( rc != NEXUS_SUCCESS ) {
                    return BERR_TRACE(rc);
                }
            }
        }

        if ( handle->serverSettings.capabilities.ms12 &&
             (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_ePersistent ||
              handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) ) {
            /* use MS12 fade through downstream mixer. */
            if ( connectorType == NEXUS_AudioConnectorType_eMultichannel ) {
                rc = nexus_simpleaudiodecoder_apply_mixerinput_settings(
                    tail, mixer, selector,
                    &handle->settings);
                if ( rc != NEXUS_SUCCESS ) {
                    return BERR_TRACE(rc);
                }
            }
        }
    }
    #if 0
    else {
        BDBG_ERR(("Invalid mixer (%p) or tail (%p)", mixer, tail));
        return BERR_TRACE(BERR_NOT_AVAILABLE);
    }
    #endif

    return NEXUS_SUCCESS;
}

static void nexus_simpleaudiodecoder_disconnect_processing(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType)
{
    /* we only support PP on stereo path for now */
    if (connectorType == NEXUS_AudioConnectorType_eStereo) {
        /* clean up old post processing and mixing graph */
        if ( handle->serverSettings.mixers.stereo != NULL ) {
            unsigned j;
            for(j=nexus_simpleaudiodecoder_pp_last();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_prev(j)) {
                if ( handle->decoders[selector].processor[j] != NULL ) {
                    BDBG_MSG(("remove input to processor %p", (void*)NEXUS_AudioProcessor_GetConnectorByType(handle->decoders[selector].processor[j], NEXUS_AudioConnectorType_eStereo)));
                    NEXUS_AudioProcessor_RemoveAllInputs(handle->decoders[selector].processor[j]);
                }
            }
        }
    }
}

static void nexus_simpleaudiodecoder_disconnect_mixer(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType)
{
    NEXUS_AudioInputHandle tail;
    NEXUS_AudioMixerHandle mixer = NULL;
    NEXUS_Error rc;

    /* connect to primary display */
    switch ( connectorType ) {
    case NEXUS_AudioConnectorType_eStereo:
        mixer = handle->serverSettings.mixers.stereo;
        break;
    case NEXUS_AudioConnectorType_eMultichannel:
        mixer = handle->serverSettings.mixers.multichannel;
        break;
    default:
        break;
    }

    tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType, true);

    if ( mixer && tail ) {
        BDBG_MSG(("remove input to mixer (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer));
        NEXUS_AudioMixer_RemoveInput(mixer, tail);
        if ( selector == NEXUS_SimpleAudioDecoderSelector_ePrimary ) {
            NEXUS_AudioMixerSettings mixerSettings;
            NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
            if ( mixerSettings.mixUsingDsp && mixerSettings.master == tail ) {
                BDBG_MSG(("     setting mixer (%s) %p master to NULL", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer));
                mixerSettings.master = NULL;
            }
            rc = NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);
            if ( rc != NEXUS_SUCCESS ) {
                BERR_TRACE(rc);
                return;
            }
        }
    }
    #if 0
    else {
        BDBG_ERR(("Invalid mixer (%p) or tail (%p)", mixer, tail));
        BERR_TRACE(BERR_NOT_AVAILABLE);
    }
    #endif
}

static NEXUS_Error nexus_simpleaudiodecoder_connect_downstream(NEXUS_SimpleAudioDecoderHandle handle)
{
    NEXUS_Error rc;
    unsigned i;

    /* Standalone decoders do not connect to the server display */
    if ( handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eStandalone ) {
        return NEXUS_SUCCESS;
    }

    if (handle->mixers.connected) {
        return NEXUS_SUCCESS;
    }

    for ( i = 0; i < NEXUS_SimpleAudioDecoderSelector_eMax; i++ ) {
        switch ( i ) {
        case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        case NEXUS_SimpleAudioDecoderSelector_eDescription:
            rc = nexus_simpleaudiodecoder_connect_processing(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_processing(handle, i, NEXUS_AudioConnectorType_eStereo);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_mixer(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_mixer(handle, i, NEXUS_AudioConnectorType_eStereo);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            break;
        default:
            break;
        }
    }

    handle->mixers.connected = true;
    return NEXUS_SUCCESS;
}

static void nexus_simpleaudiodecoder_disconnect_downstream(NEXUS_SimpleAudioDecoderHandle handle)
{
    unsigned i;

    if (!handle->mixers.connected) {
        return;
    }

    for (i=0;i<NEXUS_SimpleAudioDecoderSelector_eMax;i++) {
        switch ( i ) {
        case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        case NEXUS_SimpleAudioDecoderSelector_eDescription:
            nexus_simpleaudiodecoder_disconnect_mixer(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            nexus_simpleaudiodecoder_disconnect_mixer(handle, i, NEXUS_AudioConnectorType_eStereo);
            nexus_simpleaudiodecoder_disconnect_processing(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            nexus_simpleaudiodecoder_disconnect_processing(handle, i, NEXUS_AudioConnectorType_eStereo);
            break;
        default:
            break;
        }
    }
    handle->mixers.connected = false;
}

typedef enum NEXUS_SimpleAudioDecoderStartOp
{
    NEXUS_SimpleAudioDecoderStartOp_eStart,
    NEXUS_SimpleAudioDecoderStartOp_eResume,
    NEXUS_SimpleAudioDecoderStartOp_eStopPrimerAndStartDecoder,
    NEXUS_SimpleAudioDecoderStartOp_eMax
} NEXUS_SimpleAudioDecoderStartOp;

typedef enum NEXUS_SimpleAudioDecoderStopOp
{
    NEXUS_SimpleAudioDecoderStopOp_eStop,
    NEXUS_SimpleAudioDecoderStopOp_eSuspend,
    NEXUS_SimpleAudioDecoderStopOp_eStopDecoderAndStartPrimer,
    NEXUS_SimpleAudioDecoderStopOp_eMax
} NEXUS_SimpleAudioDecoderStopOp;

static int nexus_simpleaudiodecoder_do_start(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderPrimerHandle primer,
    const NEXUS_AudioDecoderStartSettings * pStartSettings,
    NEXUS_SimpleAudioDecoderStartOp op)
{
    int rc = 0;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_AudioConnectorType connectorType = handle->serverSettings.syncConnector;
#endif

    if (handle->stcChannel)
    {
#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SimpleStcChannelSettings stcSettings;
        NEXUS_AudioDecoderSimpleSettings simpleSettings;
#endif
#if NEXUS_HAS_ASTM
        NEXUS_SimpleStcChannel_AddAstmAudio_priv(handle->stcChannel, decoder);
#endif
#if NEXUS_HAS_SYNC_CHANNEL
        NEXUS_SimpleStcChannel_AddSyncAudio_priv(handle->stcChannel, NEXUS_AudioDecoder_GetConnector(decoder, connectorType));
        NEXUS_SimpleStcChannel_GetSettings(handle->stcChannel, &stcSettings);
        NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
        NEXUS_AudioDecoder_GetSimpleSettings_priv(decoder, &simpleSettings);
        switch (stcSettings.sync)
        {
            case NEXUS_SimpleStcChannelSyncMode_eAudioAdjustmentConcealment:
                simpleSettings.gaThreshold = 44; /* 42 ms for longest video frame + 2 ms audio PTS jitter allowance */
                break;
            default:
                simpleSettings.gaThreshold = 0;
                break;
        }
        rc = NEXUS_AudioDecoder_SetSimpleSettings_priv(decoder, &simpleSettings);
        NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
        if (rc) BERR_TRACE(rc); /* keep going */
#endif
    }

    switch (op)
    {
        case NEXUS_SimpleAudioDecoderStartOp_eStart:
            rc = NEXUS_AudioDecoder_Start(decoder, pStartSettings);
            break;
        case NEXUS_SimpleAudioDecoderStartOp_eResume:
            rc = NEXUS_AudioDecoder_Resume(decoder);
            break;
        case NEXUS_SimpleAudioDecoderStartOp_eStopPrimerAndStartDecoder:
            rc = NEXUS_AudioDecoderPrimer_StopPrimerAndStartDecode(primer, decoder);
            break;
        default:
            break;
    }

    return rc;
}

static int nexus_simpleaudiodecoder_do_stop(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioDecoderHandle decoder,
    NEXUS_AudioDecoderPrimerHandle primer,
    NEXUS_SimpleAudioDecoderStopOp op)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_AudioConnectorType connectorType = handle->serverSettings.syncConnector;
#endif

    if ( decoder ) {
        switch (op)
        {
        case NEXUS_SimpleAudioDecoderStopOp_eStop:
            NEXUS_AudioDecoder_Stop(decoder);
            break;
        case NEXUS_SimpleAudioDecoderStopOp_eSuspend:
            rc = NEXUS_AudioDecoder_Suspend(decoder);
            break;
        case NEXUS_SimpleAudioDecoderStopOp_eStopDecoderAndStartPrimer:
            rc = NEXUS_AudioDecoderPrimer_StopDecodeAndStartPrimer(primer, decoder);
            break;
        default:
            break;
        }

        if (handle->stcChannel)
        {
            #if NEXUS_HAS_ASTM
            NEXUS_SimpleStcChannel_RemoveAstmAudio_priv(handle->stcChannel, decoder);
            #endif
            #if NEXUS_HAS_SYNC_CHANNEL
            NEXUS_SimpleStcChannel_RemoveSyncAudio_priv(handle->stcChannel, NEXUS_AudioDecoder_GetConnector(decoder, connectorType));
            #endif
        }
    }

    return rc;
}

static int nexus_simpleaudiodecoder_change_state_pb(NEXUS_SimpleAudioDecoderHandle handle,
    enum nexus_simpleaudiodecoder_state desired_state)
{
    NEXUS_Error rc;
    enum nexus_simpleaudiodecoder_state orgstate;
    NEXUS_AudioPlaybackHandle playback;

    orgstate = handle->playback.state;
    playback = handle->serverSettings.passthroughPlayback;

    /* adjust desired state */
    if (!playback ||
        ((desired_state == state_started || desired_state == state_suspended) && !nexus_p_simpleaudiodecoder_is_playback_connected(playback)))
    {
        desired_state = state_stopped;
    }

    /* N/A for passthrough playback */
    BDBG_ASSERT(desired_state != state_priming);

    switch (desired_state) {
    case state_suspended:
        if (orgstate == state_started) {
            rc = NEXUS_AudioPlayback_Suspend(playback);
            if (rc) {
                /* no error, just stop */
                NEXUS_AudioPlayback_Stop(playback);
                handle->playback.state = state_stopped;
            }
            else {
                handle->playback.state = state_suspended;
            }
        }
        break;

    case state_started:
        if (orgstate == state_suspended) {
            NEXUS_AudioPlayback_Resume(playback);
            handle->playback.state = state_started;
        }
        else {
            NEXUS_AudioPlaybackStartSettings startSettings;

            NEXUS_AudioPlayback_GetDefaultStartSettings(&startSettings);
            startSettings.compressed = true;
            startSettings.bitsPerSample = 16;
            startSettings.signedData = true;
            startSettings.stereo = true;
            startSettings.sampleRate = handle->startSettings.passthroughBuffer.sampleRate;
            startSettings.dataCallback = handle->startSettings.passthroughBuffer.dataCallback;
            rc = NEXUS_AudioPlayback_Start(playback, &startSettings);
            if (rc) return BERR_TRACE(rc);

            handle->playback.state = state_started;
        }
        break;

    case state_stopped:
        if (orgstate != state_stopped) {
            NEXUS_AudioPlayback_Stop(playback);
        }
        handle->playback.state = state_stopped;
        break;

    default:
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    BDBG_MSG(("change_state playback %p: %s->%s->%s", (void*)playback, g_state[orgstate], g_state[desired_state], g_state[handle->playback.state]));
    return 0;
}

static int nexus_simpleaudiodecoder_change_state(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    enum nexus_simpleaudiodecoder_state desired_state)
{
    NEXUS_AudioDecoderHandle decoder;
    NEXUS_AudioDecoderPrimerHandle *pPrimer;
    int rc;
    const NEXUS_AudioDecoderSettings *pDecoderSettings;
    const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *pCodecSettings;
    enum nexus_simpleaudiodecoder_state *pstate;
    enum nexus_simpleaudiodecoder_state orgstate;
    bool wants_priming;

    switch (selector) {
    case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        pDecoderSettings = &handle->settings.primary;
        wants_priming = handle->startSettings.primer.pcm;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary:
        pDecoderSettings = &handle->settings.secondary;
        wants_priming = handle->startSettings.primer.compressed;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eDescription:
        pDecoderSettings = &handle->settings.description;
        wants_priming = false;
        break;
    default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    decoder = nexus_p_get_decoder(handle, selector);
    pCodecSettings = nexus_p_get_decoder_codec_settings(handle, selector);
    pPrimer = &handle->decoders[selector].primer;
    pstate = &handle->decoders[selector].state;

    orgstate = *pstate;
    if (*pstate == state_priming) {
        BDBG_ASSERT(*pPrimer);
    }

    /* fix such that we check for potential connections too... */
    BDBG_MSG(("change state decoder %p, selector %d, orgstate %d, desired_state %d", (void*)decoder, selector, orgstate, desired_state));
    /* adjust desired state */
    if (!decoder ||
        ((desired_state == state_started || desired_state == state_suspended) && !nexus_p_simpleaudiodecoder_is_decoder_connected(decoder)))
    {
        BDBG_MSG(("  decoder %p selector %d has no outputs connected. demoting desired_state %d to stopped", (void*)decoder, selector, desired_state));
        desired_state = state_stopped;
    }
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if (wants_priming) {
        if (!handle->stcChannel) {
            BDBG_WRN(("%p: cannot prime without stcChannel", (void*)handle));
            wants_priming = false;
        }
        else if (!NEXUS_SimpleStcChannel_P_ActiveVideo(handle->stcChannel)) {
            /* audio priming is only useful if lipsynced video is active */
            wants_priming = false;
        }
        if (wants_priming) {
            if (desired_state == state_stopped) {
                desired_state = state_priming;
            }
            if (!*pPrimer) {
                *pPrimer = NEXUS_AudioDecoderPrimer_Create(NULL);
                if (!*pPrimer) return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
            }
        }
    }
#else
    /* no primer for RAP because of limited use cases and resources */
    BDBG_ASSERT(desired_state != state_priming);
    wants_priming = false;
#endif

    switch (desired_state) {
    case state_suspended:
        if (*pstate == state_started) {
            BDBG_MSG(("  SUSPEND selector %d", selector));
            rc = nexus_simpleaudiodecoder_do_stop(handle, decoder, NULL, NEXUS_SimpleAudioDecoderStopOp_eSuspend);
            if (rc) {
                /* no error, just stop */
                nexus_simpleaudiodecoder_do_stop(handle, decoder, NULL, NEXUS_SimpleAudioDecoderStopOp_eStop);
                *pstate = state_stopped;
            }
            else {
                *pstate = state_suspended;
            }
        }
        break;

    case state_started:
        if (*pstate == state_suspended) {
            BDBG_MSG(("  RESUME selector %d", selector));
            rc = nexus_simpleaudiodecoder_do_start(handle, decoder, NULL, NULL, NEXUS_SimpleAudioDecoderStartOp_eResume);
            if (rc) return BERR_TRACE(rc);
            *pstate = state_started;
        }
        else if (*pstate == state_priming) {
            rc = nexus_simpleaudiodecoder_do_start(handle, decoder, *pPrimer, NULL, NEXUS_SimpleAudioDecoderStartOp_eStopPrimerAndStartDecoder);
            if (rc) return BERR_TRACE(rc);
            *pstate = state_started;
            /* defer primer destroy until release */
        }
        else {
            NEXUS_AudioDecoderStartSettings startSettings;
            bool nonRealTime = nexus_simpleencoder_p_nonRealTime(handle->encoder.handle);
            bool skip = false;

            if (selector == NEXUS_SimpleAudioDecoderSelector_ePrimary) {
                static NEXUS_AudioCapabilities g_caps;
                static bool g_read_caps = false;
                if (!g_read_caps) {
                    NEXUS_GetAudioCapabilities(&g_caps);
                    g_read_caps = true;
                }
                if (!g_caps.dsp.codecs[handle->startSettings.primary.codec].decode && !handle->hdmiInput.handle) {
                    skip = true;
                }
            }
            else {
                if (nonRealTime) {
                    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                }
            }

            if (!skip) {
                rc = NEXUS_AudioDecoder_SetSettings(decoder, pDecoderSettings);
                if (rc) return BERR_TRACE(rc);
                rc = NEXUS_SimpleAudioDecoder_P_ApplyCodecSettings(decoder, pCodecSettings);
                if (rc) return BERR_TRACE(rc);

                copy_start_settings(&startSettings, handle, selector);
                if (startSettings.pidChannel || startSettings.input) {
                    startSettings.nonRealTime = nonRealTime;

                    if (wants_priming) {
                        /* if you ever want to prime, you must start the primer first then pass off to the
                        regular decoder so the primer's rave context is used. */
                        rc = NEXUS_AudioDecoderPrimer_Start(*pPrimer, &startSettings);
                        if (rc) return BERR_TRACE(rc);
                        rc = nexus_simpleaudiodecoder_do_start(handle, decoder, *pPrimer, NULL, NEXUS_SimpleAudioDecoderStartOp_eStopPrimerAndStartDecoder);
                        if (rc) return BERR_TRACE(rc);
                    }
                    else {
                        BDBG_MSG(("  START selector %d", selector));
                        rc = nexus_simpleaudiodecoder_do_start(handle, decoder, NULL, &startSettings, NEXUS_SimpleAudioDecoderStartOp_eStart);
                        if (rc) return BERR_TRACE(rc);
                    }
                    *pstate = state_started;
                }
            }
        }
        break;

    case state_priming:
        if (*pstate == state_started || *pstate == state_suspended) {
            if (decoder) {
                rc = nexus_simpleaudiodecoder_do_stop(handle, decoder, *pPrimer, NEXUS_SimpleAudioDecoderStopOp_eStopDecoderAndStartPrimer);
                if (!rc) {
                    *pstate = state_priming;
                }
                else {
                    BDBG_MSG(("  STOP selector %d", selector));
                    nexus_simpleaudiodecoder_do_stop(handle, decoder, NULL, NEXUS_SimpleAudioDecoderStopOp_eStop);
                    *pstate = state_stopped;
                }
            }
            else {
                *pstate = state_stopped;
            }
        }
        else if (*pstate == state_stopped) {
            NEXUS_AudioDecoderStartSettings startSettings;
            copy_start_settings(&startSettings, handle, selector);
            if (startSettings.stcChannel) {
                rc = NEXUS_AudioDecoderPrimer_Start(*pPrimer, &startSettings);
                if (!rc) {
                    *pstate = state_priming;
                }
            }
        }
        break;

    case state_stopped:
        if (*pstate == state_priming) {
            NEXUS_AudioDecoderPrimer_Stop(*pPrimer);
        }
        else if (*pstate != state_stopped) {
            BDBG_MSG(("  STOP selector %d", selector));
            nexus_simpleaudiodecoder_do_stop(handle, decoder, NULL, NEXUS_SimpleAudioDecoderStopOp_eStop);
        }
        *pstate = state_stopped;
        break;

    default:
        return BERR_TRACE(NEXUS_UNKNOWN);
    }

    BDBG_MSG(("change_state %s %p: %s->%s->%s", g_selectorStr[selector], (void*)decoder, g_state[orgstate], g_state[desired_state], g_state[*pstate]));
    return 0;
}
#endif

NEXUS_Error NEXUS_SimpleAudioDecoder_Suspend(NEXUS_SimpleAudioDecoderHandle handle)
{
#if NEXUS_HAS_AUDIO
    NEXUS_SimpleAudioDecoder_P_Suspend(handle, SUSPEND_ALL);
#else
    BSTD_UNUSED(handle);
#endif
    return NEXUS_SUCCESS;
}

#if NEXUS_HAS_AUDIO
static bool nexus_p_is_connected_to_a_mixer(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioInputHandle input)
{
    bool connected = false;
    bool anyConnected = false;
    if (handle->serverSettings.mixers.persistent) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(handle->serverSettings.mixers.persistent),
                                            &connected);
        anyConnected |= connected;
    }
    if (handle->serverSettings.mixers.stereo) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(handle->serverSettings.mixers.stereo),
                                            &connected);
        anyConnected |= connected;
    }
    if (handle->serverSettings.mixers.multichannel) {
        NEXUS_AudioInput_IsConnectedToInput(input,
                                            NEXUS_AudioMixer_GetConnector(handle->serverSettings.mixers.multichannel),
                                            &connected);
        anyConnected |= connected;
    }
    return anyConnected;
}

static void NEXUS_SimpleAudioDecoder_P_SuspendPersistentDecoders(NEXUS_SimpleAudioDecoderHandle handle)
{
    int i, j;
    bool running;
    bool found;
    NEXUS_Error rc = NEXUS_SUCCESS;

    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        found = false;
        if (handle->serverSettings.persistent[i].decoder != NULL) {
            for (j = 0; j < NEXUS_AudioConnectorType_eMax && !found; j++)
            {
                NEXUS_AudioInput_IsRunning(NEXUS_AudioDecoder_GetConnector(handle->serverSettings.persistent[i].decoder, j), &running);
                if (running && nexus_p_is_connected_to_a_mixer(handle, NEXUS_AudioDecoder_GetConnector(handle->serverSettings.persistent[i].decoder, j))) {
                    rc = NEXUS_AudioDecoder_Suspend(handle->serverSettings.persistent[i].decoder);
                    if (rc) {
                        BERR_TRACE(rc);
                    }
                    else {
                    handle->serverSettings.persistent[i].suspended = true;
                    }
                    found = true;
                }
            }
        }
    }
    return;
}

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_ResumePersistentDecoders(NEXUS_SimpleAudioDecoderHandle handle)
{
    int i;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_Error cumulativeRc = NEXUS_SUCCESS;

    for (i = 0; i < NEXUS_MAX_AUDIO_DECODERS; i++) {
        if (handle->serverSettings.persistent[i].decoder != NULL && handle->serverSettings.persistent[i].suspended) {
            rc = NEXUS_AudioDecoder_Resume(handle->serverSettings.persistent[i].decoder);
            if (rc) {
                BERR_TRACE(rc);
                cumulativeRc |= rc;
            }
            else {
                handle->serverSettings.persistent[i].suspended = false;
            }
        }
    }
    return cumulativeRc;
}

static void NEXUS_SimpleAudioDecoder_P_Suspend(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToSuspend)
{
    int rc;

    if (whatToSuspend & SUSPEND_DECODER) {
        bool done = false;
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
            rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, state_suspended);
            if (!rc) done = true;
        }
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
            rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary, state_suspended);
            if (!rc) done = true;
        }
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
            rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eDescription, state_suspended);
            if (!rc) done = true;
        }
        if (handle->playback.state == state_started) {
            rc = nexus_simpleaudiodecoder_change_state_pb(handle, state_suspended);
            if (!rc) done = true;
        }
        if (done) {
            nexus_simpleaudiodecoder_disconnect_downstream(handle);
            handle->suspended = true;
        }
    }

    if (whatToSuspend & SUSPEND_PERSISTENT) {
       NEXUS_SimpleAudioDecoder_P_SuspendPersistentDecoders(handle);
    }

    if (whatToSuspend & SUSPEND_PLAYBACK && handle->serverSettings.simplePlayback) {
        NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.simpleAudioPlayback);
        NEXUS_SimpleAudioPlaybackServer_Suspend_priv(handle->serverSettings.simplePlayback);
        NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.simpleAudioPlayback);
    }

    if ((whatToSuspend & SUSPEND_MIXER) && nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed(handle)) {
        if (!handle->mixers.suspended) {
            if (handle->serverSettings.mixers.persistent) {
                BDBG_MSG(("Stop persistent mixer."));
                NEXUS_AudioMixer_Stop(handle->serverSettings.mixers.persistent);
            }
            if (handle->serverSettings.mixers.stereo) {
                BDBG_MSG(("Stop stereo mixer."));
                NEXUS_AudioMixer_Stop(handle->serverSettings.mixers.stereo);
            }
            if (handle->serverSettings.mixers.multichannel) {
                BDBG_MSG(("Stop multichannel mixer."));
                NEXUS_AudioMixer_Stop(handle->serverSettings.mixers.multichannel);
            }
            handle->mixers.suspended = true;
        }
    }
    return;
}
#endif

NEXUS_Error NEXUS_SimpleAudioDecoder_Resume(NEXUS_SimpleAudioDecoderHandle handle)
{
    int rc = NEXUS_SUCCESS;
#if NEXUS_HAS_AUDIO
    if ( !NEXUS_GetEnv("audio_mixer_start_disabled") ) {
        if (handle->mixers.suspended && nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed(handle)) {
            if (handle->serverSettings.mixers.stereo) {
                BDBG_MSG(("Start stereo mixer."));
                rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.stereo);
                if (rc != NEXUS_SUCCESS) {
                    BERR_TRACE(rc);
                    goto suspend_stereo;
                }
            }
            if (handle->serverSettings.mixers.multichannel) {
                BDBG_MSG(("Start multichannel mixer."));
                rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.multichannel);
                if (rc != NEXUS_SUCCESS) {
                    BERR_TRACE(rc);
                    goto suspend_multichannel;
                }
            }
            if (handle->serverSettings.mixers.persistent) {
                BDBG_MSG(("Start persistent mixer."));
                rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.persistent);
                if (rc != NEXUS_SUCCESS) {
                    BERR_TRACE(rc);
                    goto suspend_persistent;
                }
            }
            handle->mixers.suspended = false;
        }
    }

    rc = NEXUS_SimpleAudioDecoder_P_ResumePersistentDecoders(handle);
    if (rc != NEXUS_SUCCESS) {
        BERR_TRACE(rc);
    }

    if (handle->serverSettings.simplePlayback) {
        NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.simpleAudioPlayback);
        NEXUS_SimpleAudioPlaybackServer_Resume_priv(handle->serverSettings.simplePlayback);
        NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.simpleAudioPlayback);
    }

    if (handle->suspended) {
        if (CONNECTED(handle)) {
            rc = nexus_simpleaudiodecoder_p_start_resume(handle, NEXUS_SIMPLEAUDIODECODER_ALL);
            if (!rc) handle->suspended = false;
        }
    }
#else
    BSTD_UNUSED(handle);
#endif
    return rc;
#if NEXUS_HAS_AUDIO
suspend_persistent:
        if (handle->serverSettings.mixers.multichannel) {
            NEXUS_AudioMixer_Stop(handle->serverSettings.mixers.multichannel);
        }
suspend_multichannel:
        if (handle->serverSettings.mixers.stereo) {
            NEXUS_AudioMixer_Stop(handle->serverSettings.mixers.stereo);
        }
suspend_stereo:
        return rc;
#endif
}

#if NEXUS_HAS_AUDIO
static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddEncoderOutputs(NEXUS_SimpleAudioDecoderHandle handle);

/*
audio connects to SimpleEncoder in two different configurations: transcode and display encode.

In both cases, the filter graph is: AudioDecoder -> DSP AudioMixer -> AudioEncoder -> AudioOutputMux

AudioOutputMux must be started before AudioMixer. AudioDecoder may be started before or after AudioMixer.

Display encode has a "slave" decoder because audio decode may be switched during encode as clients share the audio decoder.
This cannot happen during transcode because the audio decoder is dedicated.
*/
NEXUS_Error nexus_simpleaudiodecoder_p_add_encoder(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderHandle slave,
    NEXUS_AudioMuxOutputHandle muxOutput, NEXUS_AudioCodec codec, bool passthrough, unsigned sampleRate,
    NEXUS_SimpleEncoderHandle encoder, NEXUS_AudioMixerHandle audioMixer, bool displayEncode)
{
    int rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);

    BDBG_MSG(("nexus_simpleaudiodecoder_p_add_encoder %p %p %p samplerate:%uHz", (void *)handle, (void *)slave, (void *)audioMixer, sampleRate));
    if (handle->encoder.handle) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (!audioMixer) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    handle->encoder.handle = encoder;
    handle->encoder.muxOutput = muxOutput;
    handle->encoder.codec = codec;
    handle->encoder.passthrough = passthrough;
    handle->encoder.sampleRate = sampleRate;
    handle->encoder.displayEncode = displayEncode;
    handle->encoder.audioMixer = audioMixer;

    {
        if (slave) {
            rc = NEXUS_SimpleAudioDecoder_Suspend(slave);
            if (rc) return BERR_TRACE(rc);
            /* give the master the decoder, just temporarily */
            handle->serverSettings.primary = slave->serverSettings.primary;
        }
        else {
            if (displayEncode) {
                rc = NEXUS_SimpleAudioDecoder_Suspend(handle);
                if (rc) return BERR_TRACE(rc);
            }
        }
        rc = NEXUS_SimpleAudioDecoder_P_AddEncoderOutputs(handle);
        if (rc) return BERR_TRACE(rc);

        if (slave) {
            /* get into already-swapped state */
            slave->encoder = handle->encoder;
            slave->displayEncode.master = handle;
            handle->displayEncode.slave = slave;
            handle->serverSettings.primary = NULL;
            rc = NEXUS_SimpleAudioDecoder_Resume(slave);
            if (rc) {rc = BERR_TRACE(rc);}
        }
        else {
            rc = NEXUS_SimpleAudioDecoder_Resume(handle);
            if (rc) {rc = BERR_TRACE(rc);}
        }
    }

    return 0;
}

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddEncoderOutputs(NEXUS_SimpleAudioDecoderHandle handle)
{
    int rc;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioInputHandle decoderInput;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state != state_started && handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state != state_started && handle->playback.state != state_started);

    if (!handle->serverSettings.primary) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    decoderInput = handle->encoder.passthrough ?
        NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, NEXUS_AudioDecoderConnectorType_eCompressed) :
        NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, NEXUS_AudioDecoderConnectorType_eStereo);

    if (!handle->encoder.displayEncode) {
        rc = NEXUS_AudioMixer_AddInput(handle->encoder.audioMixer, decoderInput);
        if (rc) return BERR_TRACE(rc);
        NEXUS_AudioMixer_GetSettings(handle->encoder.audioMixer, &mixerSettings);
        mixerSettings.master = decoderInput;
        if(handle->encoder.sampleRate) {
            mixerSettings.outputSampleRate = handle->encoder.sampleRate;
            BDBG_MSG(("audio mixer sampleRate = %u Hz", mixerSettings.outputSampleRate));
        }
        rc = NEXUS_AudioMixer_SetSettings(handle->encoder.audioMixer, &mixerSettings);
        if (rc) return BERR_TRACE(rc);
    }

    if (handle->encoder.passthrough) {
        /* Just do a passthrough */
        BDBG_MSG(("%p: start passthrough decode for encode for %d", (void *)handle, handle->encoder.codec));
        rc = NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(handle->encoder.muxOutput),
            NEXUS_AudioMixer_GetConnector(handle->encoder.audioMixer));
        if (rc) return BERR_TRACE(rc);
    }
    else {
        /* Transcode */
        NEXUS_AudioEncoderSettings settings;

        BDBG_MSG(("%p: start transcode from %d to %d", (void *)handle, handle->startSettings.primary.codec, handle->encoder.codec));
        NEXUS_AudioEncoder_GetDefaultSettings(&settings);
        settings.codec = handle->encoder.codec;
        handle->encoder.audioEncoder = NEXUS_AudioEncoder_Open(&settings);
        if (!handle->encoder.audioEncoder) return BERR_TRACE(NEXUS_NOT_AVAILABLE);

        rc = NEXUS_AudioEncoder_AddInput(handle->encoder.audioEncoder,
            NEXUS_AudioMixer_GetConnector(handle->encoder.audioMixer));
        if (rc) {rc = BERR_TRACE(rc); goto connect_decoder_to_encoder;}

        rc = NEXUS_AudioOutput_AddInput(
            NEXUS_AudioMuxOutput_GetConnector(handle->encoder.muxOutput),
            NEXUS_AudioEncoder_GetConnector(handle->encoder.audioEncoder));
        if (rc) {rc = BERR_TRACE(rc); goto connect_encoder_to_mux;}
    }

    return 0;

connect_encoder_to_mux:
connect_decoder_to_encoder:
    NEXUS_AudioEncoder_Close(handle->encoder.audioEncoder);
    handle->encoder.audioEncoder = NULL;
    return rc;
}

void nexus_simpleaudiodecoder_p_remove_encoder(NEXUS_SimpleAudioDecoderHandle handle)
{
    NEXUS_SimpleAudioDecoderHandle slave;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!handle->encoder.handle) {
        /* encoder and decoder stop can come in either order, so don't fail if already stopped. */
        return;
    }
    slave = handle->displayEncode.slave ? handle->displayEncode.slave : handle;
    BDBG_MSG(("nexus_simpleaudiodecoder_p_remove_encoder %p owner %p", (void *)handle, (void *)slave));

    if (slave) {
        NEXUS_SimpleAudioDecoder_P_Suspend(slave, SUSPEND_DECODER);
    }

    if (!handle->encoder.displayEncode) {
        if (handle->encoder.audioMixer) {
            NEXUS_AudioMixer_RemoveAllInputs(handle->encoder.audioMixer);
        }
    }
    if (handle->encoder.muxOutput) {
        NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioMuxOutput_GetConnector(handle->encoder.muxOutput));
    }
    if (handle->encoder.audioEncoder) {
        NEXUS_AudioEncoder_RemoveAllInputs(handle->encoder.audioEncoder);
        NEXUS_AudioEncoder_Close(handle->encoder.audioEncoder);
    }
    BKNI_Memset(&handle->encoder, 0, sizeof(handle->encoder));
    if (slave) {
        BKNI_Memset(&slave->encoder, 0, sizeof(slave->encoder));
        NEXUS_SimpleAudioDecoder_Resume(slave);
    }
}

NEXUS_Error nexus_simpleaudiodecoder_p_encoder_set_codec_settings(NEXUS_SimpleAudioDecoderHandle handle,
    const NEXUS_AudioEncoderCodecSettings *pSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(NULL != pSettings);

    if (!handle->encoder.audioEncoder) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(pSettings->codec >= NEXUS_AudioCodec_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_AudioEncoder_SetCodecSettings(handle->encoder.audioEncoder, pSettings);
    if (rc){return BERR_TRACE(rc);}

    return rc;
}

/* start/resume must be combined because "resume" turns into "start" if outputs weren't present before suspend
but are available after. also, if outputs aren't present at resume, we must stop to prevent playback from
getting bandhold. */
static NEXUS_Error nexus_simpleaudiodecoder_p_start_resume(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToStart)
{
    int rc;
    BDBG_MSG(("start/resume %x", whatToStart));
    rc = nexus_simpleaudiodecoder_connect_downstream(handle);
    if (rc) return BERR_TRACE(rc);
    if ( handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state != state_started && (whatToStart & NEXUS_SIMPLEAUDIODECODER_PRIMARY))
    {
        BDBG_MSG(("start/resume primary"));
        rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, state_started);
        if (rc) {
            nexus_simpleaudiodecoder_disconnect_downstream(handle);
            return BERR_TRACE(rc);
        }
    }
    if ( handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state != state_started && (whatToStart & NEXUS_SIMPLEAUDIODECODER_SECONDARY))
    {
        BDBG_MSG(("start/resume secondary"));
        rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary, state_started);
        if (rc) { /* roll back */
            if (whatToStart & NEXUS_SIMPLEAUDIODECODER_PRIMARY)
            {
                nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state);
            }
            nexus_simpleaudiodecoder_disconnect_downstream(handle);
            return BERR_TRACE(rc);
        }
    }
    if ( handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state != state_started && (whatToStart & NEXUS_SIMPLEAUDIODECODER_DESCRIPTION))
    {
        BDBG_MSG(("start/resume description"));
        rc = nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eDescription, state_started);
        if (rc) { /* roll back */
            if (whatToStart & NEXUS_SIMPLEAUDIODECODER_PRIMARY)
            {
                nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state);
            }
            if (whatToStart & NEXUS_SIMPLEAUDIODECODER_SECONDARY)
            {
                nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary, handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state);
            }
            nexus_simpleaudiodecoder_disconnect_downstream(handle);
            return BERR_TRACE(rc);
        }
    }
    rc = nexus_simpleaudiodecoder_change_state_pb(handle, state_started);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

static NEXUS_Error nexus_simpleaudiodecoder_p_start(NEXUS_SimpleAudioDecoderHandle handle)
{
    const NEXUS_SimpleAudioDecoderStartSettings *pSettings = &handle->startSettings;
    NEXUS_Error rc;

    BDBG_MSG(("%p: start primary[pidch %p, codec %d, decoder %p] secondary[pidch %p, codec %d, decoder %p]",
        (void *)handle,
        (void *)pSettings->primary.pidChannel, pSettings->primary.codec, (void *)handle->serverSettings.primary,
        (void *)pSettings->secondary.pidChannel, pSettings->secondary.codec, (void *)handle->serverSettings.secondary));

    if (!handle->encoder.handle && CONNECTED(handle))
    {
        rc = NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
        if (rc) return BERR_TRACE(rc);
    }

#if NEXUS_HAS_HDMI_INPUT
    if (handle->hdmiInput.handle) {
        NEXUS_AudioInputCaptureStartSettings inputCaptureStartSettings;

        if (handle->encoder.handle) {
            BDBG_MSG(("%p: start encode from HDMI input %p", (void *)handle, (void*)handle->hdmiInput.handle));
            handle->startSettings.primary.input = NEXUS_HdmiInput_GetAudioConnector(handle->hdmiInput.handle);
        }
        else {
            BDBG_MSG(("%p: start passthrough from HDMI input %p", (void *)handle, (void*)handle->hdmiInput.handle));
            NEXUS_AudioInputCapture_GetDefaultStartSettings(&inputCaptureStartSettings);
            inputCaptureStartSettings.input = NEXUS_HdmiInput_GetAudioConnector(handle->hdmiInput.handle);
            rc = NEXUS_AudioInputCapture_Start(handle->hdmiInput.inputCapture, &inputCaptureStartSettings);
            if (rc) return BERR_TRACE(rc);
            goto start_encoder;
        }
    }
#endif

    if (handle->encoder.muxOutput) {
        /* this will restart the audio mux output only if we're in 'programChange' mode */
        rc = nexus_simpleencoder_p_start_audiomux(handle->encoder.handle);
        if (rc) {rc = BERR_TRACE(rc); goto error_start;}
    }

    rc = nexus_simpleaudiodecoder_p_start_resume(handle, NEXUS_SIMPLEAUDIODECODER_ALL);
    if (rc) {rc = BERR_TRACE(rc); goto error_start;}

#if NEXUS_HAS_HDMI_INPUT
start_encoder:
#endif
    if (handle->encoder.handle) {
        rc = nexus_simpleencoder_p_start_audio(handle->encoder.handle);
        if (rc) return BERR_TRACE(rc);
    }

    return 0;

error_start:
    {
        NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);
        NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
        return rc;
    }
}
#endif

void NEXUS_SimpleAudioDecoder_Stop( NEXUS_SimpleAudioDecoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
#if NEXUS_HAS_AUDIO
    if (!handle->clientStarted) return;

    handle->clientStarted = false;
    if (handle->hdmiInput.handle) {
        NEXUS_SimpleAudioDecoder_StopHdmiInput(handle);
    }
    if (handle->startSettings.primary.pidChannel) {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->startSettings.primary.pidChannel);
    }
    if (handle->startSettings.secondary.pidChannel) {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->startSettings.secondary.pidChannel);
    }
    if (handle->startSettings.description.pidChannel) {
        NEXUS_OBJECT_RELEASE(handle, NEXUS_PidChannel, handle->startSettings.description.pidChannel);
    }
    NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&handle->startSettings);

    nexus_simpleaudiodecoder_p_stop(handle);
#endif
}

#if NEXUS_HAS_AUDIO
static void nexus_simpleaudiodecoder_p_stop( NEXUS_SimpleAudioDecoderHandle handle)
{
    bool presentationChanged = false;
    BDBG_MSG(("nexus_simpleaudiodecoder_p_stop %p", (void *)handle));
#if NEXUS_HAS_HDMI_INPUT
    if (handle->hdmiInput.handle) {
        if (!handle->encoder.handle) {
            NEXUS_AudioInputCapture_Stop(handle->hdmiInput.inputCapture);
            return;
        }
    }
#endif
    if (handle->encoder.muxOutput) {
        NEXUS_AudioMuxOutputSettings settings;
        NEXUS_Error rc;

        NEXUS_AudioMuxOutput_GetSettings(handle->encoder.muxOutput, &settings);
        nexus_simpleencoder_p_beginProgramChange(handle->encoder.handle, &settings.sendEos);
        rc = NEXUS_AudioMuxOutput_SetSettings(handle->encoder.muxOutput, &settings);
        if (rc) BERR_TRACE(rc); /* keep going */
        NEXUS_AudioMuxOutput_Stop(handle->encoder.muxOutput);

        /* !!settings.sendEos means programChange in progress */
        if (handle->encoder.audioMixer && !settings.sendEos) {
            NEXUS_AudioMixer_Stop(handle->encoder.audioMixer);
        }
    }

    /* first, stop decode */
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, state_stopped);
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary, state_stopped);
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eDescription, state_stopped);
    nexus_simpleaudiodecoder_change_state_pb(handle, state_stopped);

    nexus_p_check_for_presentation_change(handle, &presentationChanged);

    if ((handle->currentSpdifInput || handle->currentHdmiInput || presentationChanged) && handle->acquired) {
        NEXUS_SimpleAudioDecoder_P_Suspend(handle, SUSPEND_DECODER);
        nexus_simpleaudiodecoder_disconnect_downstream(handle);
        (void)NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
    }
    else {
        nexus_simpleaudiodecoder_disconnect_downstream(handle);
    }
}
#endif

/* AudioPrimer should only be active if video is active */
void NEXUS_SimpleAudioDecoder_AdjustAudioPrimerToVideo_priv(NEXUS_SimpleAudioDecoderHandle handle, bool videoActive)
{
#if NEXUS_HAS_AUDIO
    bool priming = NEXUS_SimpleAudioDecoder_P_IsPriming(handle);
    if (videoActive) {
        if (handle->clientStarted && !CONNECTED(handle) && !priming && (handle->startSettings.primer.pcm || handle->startSettings.primer.compressed)) {
            nexus_simpleaudiodecoder_p_start_resume(handle, NEXUS_SIMPLEAUDIODECODER_ALL);
        }
    }
    else {
        if (priming) {
            nexus_simpleaudiodecoder_p_stop(handle);
        }
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(videoActive);
#endif
}

void NEXUS_SimpleAudioDecoder_GetSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    *pSettings = handle->settings;
    return;
}

#if NEXUS_HAS_AUDIO
static NEXUS_Error nexus_simpleaudiodecoder_apply_mixerinput_settings( NEXUS_AudioInputHandle hInput, NEXUS_AudioMixerHandle hMixer, NEXUS_SimpleAudioDecoderSelector selector, const NEXUS_SimpleAudioDecoderSettings *pSettings )
{
    NEXUS_Error rc;

    if ( hMixer && hInput ) {
        NEXUS_AudioMixerInputSettings inputSettings;
        rc = NEXUS_AudioMixer_GetInputSettings(hMixer, hInput, &inputSettings);
        if ( rc != NEXUS_SUCCESS ) {
            BDBG_ERR(("Failed to get audio mixer input settings."));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }

        if ( pSettings->processorSettings[selector].fade.connected ) {
            inputSettings.fade.level = pSettings->processorSettings[selector].fade.settings.level;
            inputSettings.fade.duration = pSettings->processorSettings[selector].fade.settings.duration;
            inputSettings.fade.type = pSettings->processorSettings[selector].fade.settings.type;
            BDBG_MSG(("new fade level %d, duration %d, type %d", inputSettings.fade.level, inputSettings.fade.duration, inputSettings.fade.type));
        }
        else {
            inputSettings.fade.level = 100;
            inputSettings.fade.duration = 0;
            inputSettings.fade.type = 0;
        }

        rc = NEXUS_AudioMixer_SetInputSettings(hMixer, hInput, &inputSettings);
        if ( rc != NEXUS_SUCCESS ) {
            BDBG_ERR(("Failed to set audio mixer input settings."));
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }

        return NEXUS_SUCCESS;
    }

    return NEXUS_INVALID_PARAMETER;
}

static NEXUS_Error nexus_simpleaudiodecoder_apply_processor_settings( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderSettings *pSettings )
{
    unsigned i;
    NEXUS_Error rc;
    bool pcmRunning = false;

    if ( pSettings->processorSettings[NEXUS_SimpleAudioDecoderSelector_eSecondary].fade.connected ||
         pSettings->processorSettings[NEXUS_SimpleAudioDecoderSelector_eSecondary].karaokeVocal.connected ) {
        BDBG_ERR(("Processing not supported for eSecondary (Passthrough)."));
        return BERR_TRACE(BERR_NOT_SUPPORTED);
    }

    if ( handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started ||
         handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started ) {
        pcmRunning = true;
    }

    if ( handle->serverSettings.capabilities.ms12 &&
         (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_ePersistent ||
          handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) ) {
        /* use MS12 fade through downstream mixer. */
        if ( handle->serverSettings.mixers.multichannel && pcmRunning ) {
            if ( handle->mixers.connected ) {
                if ( handle->serverSettings.primary ) {
                    rc = nexus_simpleaudiodecoder_apply_mixerinput_settings(
                        NEXUS_AudioDecoder_GetConnector(handle->serverSettings.primary, NEXUS_AudioConnectorType_eMultichannel),
                        handle->serverSettings.mixers.multichannel,
                        NEXUS_SimpleAudioDecoderSelector_ePrimary,
                        pSettings);
                    if ( rc != NEXUS_SUCCESS ) {
                        BDBG_ERR(("Failed to apply audio mixer settings."));
                        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    }
                }
                if ( handle->serverSettings.description ) {
                    rc = nexus_simpleaudiodecoder_apply_mixerinput_settings(
                        NEXUS_AudioDecoder_GetConnector(handle->serverSettings.description, NEXUS_AudioConnectorType_eMultichannel),
                        handle->serverSettings.mixers.multichannel,
                        NEXUS_SimpleAudioDecoderSelector_eDescription,
                        pSettings);
                    if ( rc != NEXUS_SUCCESS ) {
                        BDBG_ERR(("Failed to apply audio mixer settings."));
                        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
                    }
                }
            }
        }
    }
    else if ( handle->serverSettings.capabilities.ms11 &&
         (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_ePersistent ||
          handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) ) {
        /* no post processing support currently */
    }
    else {
        /* no MS support or eStandalone, allow stereo post processors with direct connections */
        for ( i = 0; i < NEXUS_SimpleAudioDecoderSelector_eMax; i++ ) {
            if ( i != NEXUS_SimpleAudioDecoderSelector_eSecondary ) {
                unsigned j;
                bool wasConnected, connected;
                for ( j = nexus_simpleaudiodecoder_pp_first(); j < NEXUS_AudioPostProcessing_eMax; j = nexus_simpleaudiodecoder_pp_next(j) ) {
                    NEXUS_AudioProcessorSettings processorSettings;
                    bool connect = false;
                    bool disconnect = false;
                    NEXUS_AudioConnectorType connectorType = NEXUS_AudioConnectorType_eStereo;

                    /* populate type specific data */
                    BKNI_Memset(&processorSettings, 0, sizeof(processorSettings));
                    switch ( j ) {
                    case NEXUS_AudioPostProcessing_eFade:
                        wasConnected = handle->settings.processorSettings[i].fade.connected;
                        connected = pSettings->processorSettings[i].fade.connected;
                        BKNI_Memcpy(&processorSettings.settings.fade, &pSettings->processorSettings[i].fade.settings, sizeof(processorSettings.settings.fade));
                        break;
                    case NEXUS_AudioPostProcessing_eKaraokeVocal:
                        wasConnected = handle->settings.processorSettings[i].karaokeVocal.connected;
                        connected = pSettings->processorSettings[i].karaokeVocal.connected;
                        BKNI_Memcpy(&processorSettings.settings.karaokeVocal, &pSettings->processorSettings[i].karaokeVocal.settings, sizeof(processorSettings.settings.karaokeVocal));
                        break;
                    case NEXUS_AudioPostProcessing_eAmbisonic:
                        wasConnected = handle->settings.processorSettings[i].ambisonic.connected;
                        connected = pSettings->processorSettings[i].ambisonic.connected;
                        if ( pSettings->processorSettings[i].ambisonic.connectorType == NEXUS_AudioConnectorType_eMultichannel ||
                             pSettings->processorSettings[i].ambisonic.connectorType == NEXUS_AudioConnectorType_eStereo ) {
                            connectorType = pSettings->processorSettings[i].ambisonic.connectorType;
                        }
                        BKNI_Memcpy(&processorSettings.settings.ambisonic, &pSettings->processorSettings[i].ambisonic.settings, sizeof(processorSettings.settings.ambisonic));
                        break;
                    default:
                        continue;
                        break; /* unreachable */
                    }

                    if ( wasConnected != connected ) {
                        if ( connected ) {
                            BDBG_MSG(("Connecting type %d processing", j));
                            connect = true;
                        }
                        else {
                            BDBG_MSG(("Disconnecting type %d processing", j));
                            disconnect = true;
                        }
                    }

                    /* check if we are running */
                    if ( ( connect || disconnect ) && pcmRunning ) {
                        BDBG_ERR(("Changes to processor 'connected' fields are not allowed while a pcm decoder is running."));
                        BDBG_ERR(("Attempted to %s processor for decode selector %u, processor type %u", connect?"connect":"disconnect", i, j));
                        return BERR_TRACE(BERR_NOT_SUPPORTED);
                    }

                    /* Create / Destroy Processor */
                    if ( connect )
                    {
                        NEXUS_AudioProcessorOpenSettings openSettings;
                        NEXUS_AudioProcessor_GetDefaultOpenSettings(&openSettings);
                        openSettings.type = (NEXUS_AudioPostProcessing) j;
                        handle->decoders[i].processor[j] = NEXUS_AudioProcessor_Open(&openSettings);
                        if ( handle->decoders[i].processor[j] == NULL ) {
                            BERR_TRACE(BERR_OUT_OF_SYSTEM_MEMORY);
                        }
                        handle->decoders[i].processorConnectorType[j] = connectorType;
                    }
                    else if ( disconnect )
                    {
                        if ( handle->decoders[i].processor[j] )
                        {
                            NEXUS_AudioProcessor_RemoveAllInputs(handle->decoders[i].processor[j]);
                            NEXUS_AudioProcessor_Close(handle->decoders[i].processor[j]);
                            handle->decoders[i].processor[j] = NULL;
                            handle->decoders[i].processorConnectorType[j] = NEXUS_AudioConnectorType_eStereo;
                        }
                    }

                    /* Configure Settings */
                    if ( connected )
                    {
                        NEXUS_AudioProcessorSettings procSettings;
                        NEXUS_AudioProcessor_GetSettings(handle->decoders[i].processor[j], &procSettings);
                        BKNI_Memcpy(&procSettings.settings, &processorSettings.settings, sizeof(procSettings.settings));
                        rc = NEXUS_AudioProcessor_SetSettings(handle->decoders[i].processor[j], &procSettings);
                        if ( rc != NEXUS_SUCCESS ) {
                            BDBG_ERR(("Failed to set Processor settings for selector %u, Processor type %u", i, j));
                            if ( connect && !wasConnected ) {
                                NEXUS_AudioProcessor_Close(handle->decoders[i].processor[j]);
                                handle->decoders[i].processor[j] = NULL;
                            }
                            return BERR_TRACE(rc);
                        }
                        BKNI_Memcpy(&handle->decoders[i].processorSettings[j].settings, &processorSettings.settings, sizeof(handle->decoders[i].processorSettings[j].settings));
                    }
                }
            }
        }
    }

    return NEXUS_SUCCESS;
}
#endif

NEXUS_Error NEXUS_SimpleAudioDecoder_SetSettings( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderSettings *pSettings )
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(pSettings);

    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    nexus_simpleaudiodecoder_apply_processor_settings(handle, pSettings);

    if (handle->serverSettings.primary) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.primary, &pSettings->primary);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->serverSettings.secondary) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.secondary, &pSettings->secondary);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->serverSettings.description) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.description, &pSettings->description);
        if (rc) return BERR_TRACE(rc);
    }
#endif

    if (&handle->settings != pSettings) {
        handle->settings = *pSettings;
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetStatus( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioDecoderStatus *pStatus )
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetStatus(handle->serverSettings.primary, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetStatus(handle->serverSettings.secondary, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else if (handle->playback.state == state_started) {
        NEXUS_AudioPlaybackStatus pbStatus;
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
        NEXUS_AudioPlayback_GetStatus(handle->serverSettings.passthroughPlayback, &pbStatus);
        pStatus->started = pbStatus.started;
        pStatus->fifoDepth = pbStatus.queuedBytes;
        pStatus->numBytesDecoded = pbStatus.playedBytes;
    }
    else
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pStatus);
#endif
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetCombinedStatus(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderStatus *pStatus
    )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);

    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

#if NEXUS_HAS_AUDIO
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetStatus(handle->serverSettings.primary, &pStatus->status[NEXUS_SimpleAudioDecoderSelector_ePrimary]);
        if (rc) return BERR_TRACE(rc);
    }
    else if (handle->playback.state == state_started) {
        NEXUS_AudioPlaybackStatus pbStatus;
        NEXUS_AudioPlayback_GetStatus(handle->serverSettings.passthroughPlayback, &pbStatus);
        pStatus->status[NEXUS_SimpleAudioDecoderSelector_ePrimary].started = pbStatus.started;
        pStatus->status[NEXUS_SimpleAudioDecoderSelector_ePrimary].fifoDepth = pbStatus.queuedBytes;
        pStatus->status[NEXUS_SimpleAudioDecoderSelector_ePrimary].numBytesDecoded = pbStatus.playedBytes;
    }

    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetStatus(handle->serverSettings.secondary, &pStatus->status[NEXUS_SimpleAudioDecoderSelector_eSecondary]);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
        rc = NEXUS_AudioDecoder_GetStatus(handle->serverSettings.description, &pStatus->status[NEXUS_SimpleAudioDecoderSelector_eDescription]);
        if (rc) return BERR_TRACE(rc);
    }
#else
    BSTD_UNUSED(rc);
#endif
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetProcessorStatus( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector, NEXUS_AudioPostProcessing type, NEXUS_AudioProcessorStatus *pStatus )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if ( selector >= NEXUS_SimpleAudioDecoderSelector_eMax ) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail ) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
#if NEXUS_HAS_AUDIO
    pStatus->type = NEXUS_AudioPostProcessing_eMax;
    if (handle->decoders[selector].state == state_started) {
        /* requested decoder is indeed started. Try to locate processor */
        if ( handle->serverSettings.capabilities.ms12 &&
            (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_ePersistent ||
             handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) ) {
            if ( type == NEXUS_AudioPostProcessing_eFade &&
                 handle->settings.processorSettings[selector].fade.connected ) {
                NEXUS_AudioDecoderHandle decoder = NULL;
                NEXUS_AudioMixerHandle mixer = handle->serverSettings.mixers.multichannel;

                switch (selector) {
                default:
                    break;
                case NEXUS_SimpleAudioDecoderSelector_ePrimary:
                    decoder = handle->serverSettings.primary;
                    break;
                case NEXUS_SimpleAudioDecoderSelector_eDescription:
                    decoder = handle->serverSettings.description;
                    break;
                }

                if ( mixer && decoder ) {
                    NEXUS_AudioInputHandle input;
                    NEXUS_AudioMixerInputStatus inputStatus;
                    NEXUS_Error rc;

                    input = NEXUS_AudioDecoder_GetConnector(decoder, NEXUS_AudioConnectorType_eMultichannel);
                    rc = NEXUS_AudioMixer_GetInputStatus(mixer, input, &inputStatus);
                    if ( rc != NEXUS_SUCCESS ) {
                        return rc;
                    }

                    pStatus->type = NEXUS_AudioPostProcessing_eFade;
                    pStatus->status.fade.active = inputStatus.fade.active;
                    pStatus->status.fade.level = inputStatus.fade.level;
                    pStatus->status.fade.remaining = inputStatus.fade.remaining;
                    return NEXUS_SUCCESS;
                }
            }
        }
        else if ( handle->serverSettings.capabilities.ms11 &&
         (handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_ePersistent ||
          handle->serverSettings.type == NEXUS_SimpleAudioDecoderType_eDynamic) ) {
            /* currently no support for post processing in MS11 mode */
        }
        else { /* legacy mode - decoder attached post processing */
            unsigned j;
            for(j=nexus_simpleaudiodecoder_pp_first();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_next(j)) {
                NEXUS_AudioProcessorSettings processorSettings;
                if ( handle->decoders[selector].processor[j] != NULL ) {
                    NEXUS_AudioProcessor_GetSettings(handle->decoders[selector].processor[j], &processorSettings);
                    if ( processorSettings.type == type ) {
                        /* found it */
                        NEXUS_AudioProcessor_GetStatus(handle->decoders[selector].processor[j], pStatus);
                        if ( pStatus->type == NEXUS_AudioPostProcessing_eMax )
                        {
                            return NEXUS_NOT_AVAILABLE;
                        }
                        return NEXUS_SUCCESS;
                    }
                }
            }
        }
    }
#else
    BSTD_UNUSED(type);
#endif
    return NEXUS_NOT_AVAILABLE;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetPresentationStatus( NEXUS_SimpleAudioDecoderHandle handle, unsigned presentationIndex, NEXUS_AudioDecoderPresentationStatus *pStatus )
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetPresentationStatus(handle->serverSettings.primary, presentationIndex, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_GetPresentationStatus(handle->serverSettings.secondary, presentationIndex, pStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else
    {
        BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(presentationIndex);
    BSTD_UNUSED(pStatus);
#endif
    return 0;
}

void NEXUS_SimpleAudioDecoder_Flush( NEXUS_SimpleAudioDecoderHandle handle )
{
#if NEXUS_HAS_AUDIO
    NEXUS_SimpleAudioDecoderSelector selector;
#endif
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
#if NEXUS_HAS_AUDIO
    for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
        if (handle->decoders[selector].state == state_started) {
            NEXUS_AudioDecoder_Flush(nexus_p_get_decoder(handle, selector));
        }
        else if (handle->decoders[selector].state == state_priming) {
            NEXUS_AudioDecoderPrimer_Flush(handle->decoders[selector].primer);
        }
    }
    if (handle->playback.state == state_started) {
        NEXUS_AudioPlayback_Flush(handle->serverSettings.passthroughPlayback);
    }
#endif
}

void NEXUS_SimpleAudioDecoder_GetTrickState( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioDecoderTrickState *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    *pSettings = handle->trickState;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetTrickState( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_AudioDecoderTrickState *pSettings )
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;
    NEXUS_SimpleAudioDecoderSelector selector;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->playback.state != state_stopped) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
        if (handle->decoders[selector].state == state_started) {
            rc = NEXUS_AudioDecoder_SetTrickState(nexus_p_get_decoder(handle, selector), pSettings);
            if (rc) return BERR_TRACE(rc);
        }
    }
    if (handle->trickState.forceStopped != pSettings->forceStopped) {
        NEXUS_SimpleAudioDecoderSelector selector;
        for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
            switch (handle->decoders[selector].state) {
            case state_priming:
                if (pSettings->forceStopped) {
                    NEXUS_AudioDecoderPrimer_Stop(handle->decoders[selector].primer);
                    handle->decoders[selector].state = state_priming_trick;
                }
                break;
            case state_priming_trick:
                if (!pSettings->forceStopped) {
                    NEXUS_AudioDecoderStartSettings startSettings;
                    copy_start_settings(&startSettings, handle, selector);
                    if (startSettings.stcChannel) {
                        NEXUS_AudioDecoderPrimer_Start(handle->decoders[selector].primer, &startSettings);
                    }
                    handle->decoders[selector].state = state_priming;
                }
                break;
            default:
                break;
            }
        }
    }
#endif

    handle->trickState = *pSettings;
    return 0;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_Advance( NEXUS_SimpleAudioDecoderHandle handle, uint32_t pts )
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;
    NEXUS_SimpleAudioDecoderSelector selector;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->playback.state != state_stopped) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    for (selector=0;selector<NEXUS_SimpleAudioDecoderSelector_eMax;selector++) {
        if (handle->decoders[selector].state == state_started) {
            rc = NEXUS_AudioDecoder_Advance(nexus_p_get_decoder(handle, selector), pts);
            if (rc) return BERR_TRACE(rc);
        }
    }
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(pts);
#endif
    return 0;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetCodecSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector, const NEXUS_AudioDecoderCodecSettings *pSettings)
{
#if NEXUS_HAS_AUDIO
    NEXUS_Error rc;
    struct NEXUS_SimpleAudioDecoder_P_CodecSettings *pCodecSettings;
#endif

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(pSettings);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
#if NEXUS_HAS_AUDIO
    if(pSettings->codec >= NEXUS_AudioCodec_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
#endif
    if(selector >= NEXUS_SimpleAudioDecoderSelector_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

#if NEXUS_HAS_AUDIO
    pCodecSettings = nexus_p_get_decoder_codec_settings(handle, selector);

    if (!pCodecSettings->codecSettings[pSettings->codec]) {
        pCodecSettings->codecSettings[pSettings->codec] = BKNI_Malloc(sizeof(*pSettings));
        if (!pCodecSettings->codecSettings[pSettings->codec]) {
            return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        }
    }
    if (handle->decoders[selector].state == state_started) {
        rc = NEXUS_AudioDecoder_SetCodecSettings(nexus_p_get_decoder(handle, selector), pSettings);
        if (rc) return BERR_TRACE(rc);
    }
    *pCodecSettings->codecSettings[pSettings->codec] = *pSettings;
#endif
    return NEXUS_SUCCESS;
}

void NEXUS_SimpleAudioDecoder_GetCodecSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector, NEXUS_AudioCodec codec, NEXUS_AudioDecoderCodecSettings *pSettings)
{
    const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *pCodecSettings;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(pSettings);

    if(codec >= NEXUS_AudioCodec_eMax) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    if(selector >= NEXUS_SimpleAudioDecoderSelector_eMax) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    pCodecSettings = nexus_p_get_decoder_codec_settings(handle, selector);

    if (pCodecSettings->codecSettings[codec]) {
        *pSettings = *pCodecSettings->codecSettings[codec];
    }
    else if (g_default) {
        *pSettings = g_default->codecSettings[codec];
    }
    return;
}

BDBG_FILE_MODULE(nexus_simple_decoder_proc);

void NEXUS_SimpleDecoderModule_P_PrintAudioDecoder(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_SimpleAudioDecoderHandle handle;

    for (handle=nexus_simple_audio_decoder_p_first(); handle; handle=nexus_simple_audio_decoder_p_next(handle)) {
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("audio %u(%p)", handle->index, (void *)handle));
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  acquired %d, clientStarted %d, primary=%s, secondary=%s, description=%s, passthroughBuffer=%s",
            handle->acquired, handle->clientStarted,
            g_state[handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state], g_state[handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state], g_state[handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state], g_state[handle->playback.state]));
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  primary %p, secondary %p, description %p, stcIndex %d",
            (void *)handle->serverSettings.primary,
            (void *)handle->serverSettings.secondary,
            (void *)handle->serverSettings.description,
            handle->stcIndex));
        if (handle->encoder.audioEncoder) {
            BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  encoder %p: codec %d", (void *)handle->encoder.handle, handle->encoder.codec));
        }
        if (handle->hdmiInput.handle) {
            BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  hdmiInput %p", (void *)handle->hdmiInput.handle));
        }
    }
#endif
}

void NEXUS_SimpleAudioDecoder_GetStcIndex( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, int *pStcIndex )
{
    if (server == handle->server) {
        *pStcIndex = handle->stcIndex;
    }
    else {
        *pStcIndex = 0;
    }
}

void NEXUS_SimpleAudioDecoder_SetStcIndex( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, int stcIndex )
{
    if (server == handle->server) {
        handle->stcIndex = stcIndex;
    }
}

NEXUS_Error NEXUS_SimpleAudioDecoder_StartHdmiInput( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_HdmiInputHandle hdmiInput, const NEXUS_SimpleAudioDecoderStartSettings *pStartSettings )
{
#if NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_AUDIO
    int rc = NEXUS_SUCCESS;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->hdmiInput.handle) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    NEXUS_OBJECT_ACQUIRE(handle, NEXUS_HdmiInput, hdmiInput);
    handle->hdmiInput.handle = hdmiInput;
    handle->hdmiInput.inputCapture = NEXUS_AudioInputCapture_Open(0, NULL); /* TODO: NEXUS_ANY_ID */
    handle->clientStarted = true;
    if (pStartSettings) {
        handle->startSettings = *pStartSettings;
    }
    else {
        NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&handle->startSettings);
    }
    if (CONNECTED(handle) && (!handle->encoder.audioEncoder || handle->encoder.displayEncode)) {
        rc = NEXUS_SimpleAudioDecoder_Suspend(handle);
        if (rc) {
            return BERR_TRACE(rc);
        }
    }
    if (CONNECTED(handle))
    {
        rc = nexus_simpleaudiodecoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleAudioDecoder_StopHdmiInput(handle);
            BERR_TRACE(rc); /* fall through */
        }
    }
    if (CONNECTED(handle) && (!handle->encoder.audioEncoder || handle->encoder.displayEncode)) {
        rc = NEXUS_SimpleAudioDecoder_Resume(handle);
        if (rc) {rc = BERR_TRACE(rc);}
    }
    return rc;
#else
    BSTD_UNUSED(handle);
    BSTD_UNUSED(hdmiInput);
    BSTD_UNUSED(pStartSettings);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

void NEXUS_SimpleAudioDecoder_StopHdmiInput( NEXUS_SimpleAudioDecoderHandle handle )
{
#if NEXUS_HAS_HDMI_INPUT && NEXUS_HAS_AUDIO
    bool restart = false;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!handle->hdmiInput.handle) {
        return;
    }
    if (handle->clientStarted ) {
        NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);
        restart = true;
    }
    nexus_simpleaudiodecoder_p_stop(handle);
    NEXUS_OBJECT_RELEASE(handle, NEXUS_HdmiInput, handle->hdmiInput.handle);
    handle->hdmiInput.handle = NULL;
    handle->clientStarted = false;
    if (handle->hdmiInput.inputCapture) {
        NEXUS_AudioInputCapture_Close(handle->hdmiInput.inputCapture);
        handle->hdmiInput.inputCapture = NULL;
    }
    if (restart) {
        NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
    }
#else
    BSTD_UNUSED(handle);
#endif

}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetPassthroughBuffer(
    NEXUS_SimpleAudioDecoderHandle handle,
    void **pBuffer, /* [out] attr{memory=cached} pointer to memory mapped region that is ready for passthrough data */
    size_t *pSize   /* [out] total number of writeable, contiguous bytes which buffer is pointing to */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
#if NEXUS_HAS_AUDIO
    if ( handle->serverSettings.passthroughPlayback ) {
        return NEXUS_AudioPlayback_GetBuffer(handle->serverSettings.passthroughPlayback, pBuffer, pSize);
    }
#endif
    *pBuffer = NULL;
    *pSize = 0;
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_PassthroughWriteComplete(
    NEXUS_SimpleAudioDecoderHandle handle,
    size_t amountWritten            /* The number of bytes written to the buffer */
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
#if NEXUS_HAS_AUDIO
    if ( handle->serverSettings.passthroughPlayback ) {
        return NEXUS_AudioPlayback_WriteComplete(handle->serverSettings.passthroughPlayback, amountWritten);
    }
#else
    BSTD_UNUSED(amountWritten);
#endif
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_AddOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType
    )
{
    NEXUS_AudioDecoderHandle decoder;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(output != NULL);

    if ( handle->serverSettings.type != NEXUS_SimpleAudioDecoderType_eStandalone ) {
        BDBG_ERR(("Outputs can only be directly added for eStandalone Simple Audio Decoders. Other types connect to the primary display."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (selector == NEXUS_SimpleAudioDecoderSelector_eDescription) {
        BDBG_ERR(("Invalid Selector. Only ePrimary or eSecondary (Passthrough) are valid for Standalone Simple Audio Decoders."));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    decoder = nexus_p_get_decoder(handle, selector);
    if ( !decoder ) {
        BDBG_ERR(("No primary decoder in this Simple Decoder. Cannot add output."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

#if NEXUS_HAS_AUDIO
    return BERR_TRACE(NEXUS_AudioOutput_AddInput(output, NEXUS_AudioDecoder_GetConnector(decoder, connectorType)));
#else
    BSTD_UNUSED(connectorType);
    return NEXUS_NOT_SUPPORTED;
#endif
}

NEXUS_Error NEXUS_SimpleAudioDecoder_RemoveOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(output != NULL);

    BSTD_UNUSED(handle);

#if NEXUS_HAS_AUDIO
    return BERR_TRACE(NEXUS_AudioOutput_RemoveAllInputs(output));
#else
    return NEXUS_NOT_SUPPORTED;
#endif
}
