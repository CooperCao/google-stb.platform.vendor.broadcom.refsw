/******************************************************************************
 * Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
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
#include "nexus_audio.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_decoder_primer.h"
#include "nexus_audio_decoder_trick.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_audio_init.h"
#include "priv/nexus_audio_decoder_priv.h"
#include "priv/nexus_audio_output_priv.h"
#include "nexus_client_resources.h"
#include "priv/nexus_pid_channel_priv.h"
#include "priv/nexus_stc_channel_priv.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "priv/nexus_hdmi_output_priv.h"
#endif
#if NEXUS_HAS_HDMI_INPUT
#include "priv/nexus_hdmi_input_priv.h"
#endif

BDBG_MODULE(nexus_simple_audio_decoder);

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddOutputs( NEXUS_SimpleAudioDecoderHandle handle );
static void NEXUS_SimpleAudioDecoder_P_RemoveOutputs( NEXUS_SimpleAudioDecoderHandle handle );
static NEXUS_Error nexus_simpleaudiodecoder_p_start(NEXUS_SimpleAudioDecoderHandle handle);
static void nexus_simpleaudiodecoder_p_stop( NEXUS_SimpleAudioDecoderHandle handle);
static NEXUS_Error nexus_simpleaudiodecoder_p_start_resume(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToStart);

static const char * const g_state[state_max] = {"stopped", "started", "suspended", "priming"};
static const char * const g_selectorStr[NEXUS_SimpleAudioDecoderSelector_eMax] = {"primary", "secondary", "description"};

#define NEXUS_SIMPLEAUDIODECODER_PRIMARY            ((unsigned)1<<0)
#define NEXUS_SIMPLEAUDIODECODER_SECONDARY          ((unsigned)1<<1)
#define NEXUS_SIMPLEAUDIODECODER_DESCRIPTION        ((unsigned)1<<2)
#define NEXUS_SIMPLEAUDIODECODER_ALL                ((unsigned)-1)

#define SUSPEND_DECODER     0x01
#define SUSPEND_PLAYBACK    0x02
#define SUSPEND_MIXER       0x04
#define SUSPEND_ALL (SUSPEND_DECODER|SUSPEND_PLAYBACK|SUSPEND_MIXER)
static NEXUS_Error NEXUS_SimpleAudioDecoder_P_Suspend(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToSuspend);

#define CONNECTED(handle) ((handle)->serverSettings.primary || (handle)->serverSettings.secondary || (handle)->serverSettings.description)

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
    struct NEXUS_SimpleAudioDecoder_P_CodecSettings codecSettings;
} *g_default;

void NEXUS_SimpleAudioDecoderModule_LoadDefaultSettings(NEXUS_AudioDecoderHandle audio)
{
    unsigned i;
    if (!g_default) {
        g_default = BKNI_Malloc(sizeof(*g_default));
        if (!g_default) return;
    }
    NEXUS_AudioDecoder_GetSettings(audio, &g_default->settings);
    NEXUS_AudioDecoder_GetTrickState(audio, &g_default->trickState);
    for (i=0;i<NEXUS_AudioCodec_eMax;i++) {
        NEXUS_AudioDecoder_GetCodecSettings(audio, i, &g_default->codecSettings.codecSettings[i]);
    }
}

void NEXUS_SimpleAudioDecoderModule_P_UnloadDefaultSettings(void)
{
    if (g_default) {
        BKNI_Free(g_default);
        g_default = NULL;
    }
}

/* framework for the future to address ordering as needed */
static unsigned nexus_simpleaudiodecoder_pp_first(void)
{
    return 0;
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

static bool nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed(void)
{
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    return true;
#else
    return false;
#endif
}

static void nexus_simpleaudiodecoder_p_set_default_settings(NEXUS_SimpleAudioDecoderHandle handle)
{
    unsigned i;
    for(i=0;i<NEXUS_AudioCodec_eMax;i++) {
        handle->codecSettings.primary.codecSettings[i].codec = NEXUS_AudioCodec_eUnknown;
        handle->codecSettings.secondary.codecSettings[i].codec = NEXUS_AudioCodec_eUnknown;
        handle->codecSettings.description.codecSettings[i].codec = NEXUS_AudioCodec_eUnknown;
    }
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

NEXUS_SimpleAudioPlaybackHandle nexus_simple_audio_playback_p_first(void)
{
    NEXUS_SimpleAudioDecoderServerHandle server;
    server = BLST_S_FIRST(&g_NEXUS_SimpleAudioDecoderServers);
    return server ? BLST_S_FIRST(&server->playbacks) : NULL;
}

NEXUS_SimpleAudioPlaybackHandle nexus_simple_audio_playback_p_next(NEXUS_SimpleAudioPlaybackHandle handle)
{
    NEXUS_SimpleAudioPlaybackHandle next;
    next = BLST_S_NEXT(handle, link);
    if (!next) {
        NEXUS_SimpleAudioDecoderServerHandle server;
        server = BLST_S_NEXT(handle->server, link);
        if (server) {
            next = BLST_S_FIRST(&server->playbacks);
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
        NEXUS_SimpleAudioDecoderServerSettings settings;
        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
        rc = NEXUS_SimpleAudioDecoder_SetServerSettings(server, handle, &settings);
        if (rc) { rc = BERR_TRACE(rc); goto error; }
    }

    return handle;

error:
    NEXUS_SimpleAudioDecoder_Destroy(handle);
    return NULL;
}

void nexus_simpleaudiodecoder_freeprocessing( NEXUS_SimpleAudioDecoderHandle handle )
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
    if (handle->clientStarted || handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || handle->playback.state == state_started) {
        NEXUS_SimpleAudioDecoder_Stop(handle);
    }
    nexus_simpleaudiodecoder_freeprocessing(handle);

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

static void NEXUS_SimpleAudioDecoder_P_Release( NEXUS_SimpleAudioDecoderHandle handle )
{
    NEXUS_SimpleAudioDecoderServerSettings settings;
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioDecoder, handle);
    if (handle->acquired) {
        NEXUS_SimpleAudioDecoder_Release(handle);
    }
    NEXUS_SimpleAudioDecoder_P_ReleaseResources(handle); /* SimpleAudioDecoder may be used without Acquire */

    /* call NEXUS_OBJECT_RELEASE on handles */
    NEXUS_SimpleAudioDecoder_GetServerSettings(handle->server, handle, &settings);
    settings.primary = settings.secondary = settings.description = NULL;
    settings.passthroughPlayback = NULL;
    BKNI_Memset(&settings.spdif, 0, sizeof(settings.spdif));
    BKNI_Memset(&settings.hdmi, 0, sizeof(settings.hdmi));
    (void)NEXUS_SimpleAudioDecoder_SetServerSettings(handle->server, handle, &settings);

    NEXUS_OBJECT_UNREGISTER(NEXUS_SimpleAudioDecoder, handle, Destroy);
    return;
}

static void NEXUS_SimpleAudioDecoder_P_Finalizer( NEXUS_SimpleAudioDecoderHandle handle )
{
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

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_ApplyCodecSettings(NEXUS_AudioDecoderHandle decoder, const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *codecSettings)
{
    NEXUS_Error rc;
    unsigned i;

    for(i=0;i<NEXUS_AudioCodec_eMax;i++) {
        if(codecSettings->codecSettings[i].codec!=NEXUS_AudioCodec_eUnknown) {
            rc = NEXUS_AudioDecoder_SetCodecSettings(decoder, &codecSettings->codecSettings[i]);
            if(rc!=NEXUS_SUCCESS) {return BERR_TRACE(rc);}
        }
    }
    return NEXUS_SUCCESS;
}

static void NEXUS_SimpleAudioDecoder_P_RestoreCodecSettings(NEXUS_AudioDecoderHandle decoder, const struct NEXUS_SimpleAudioDecoder_P_CodecSettings *codecSettings)
{
    unsigned i;
    for(i=0;i<NEXUS_AudioCodec_eMax;i++) {
        if(codecSettings->codecSettings[i].codec!=NEXUS_AudioCodec_eUnknown && g_default) {
            (void)NEXUS_AudioDecoder_SetCodecSettings(decoder, &g_default->codecSettings.codecSettings[i]);
        }
    }
}

static void NEXUS_SimpleAudioDecoder_P_RestorePrimaryDecoder(NEXUS_SimpleAudioDecoderHandle handle)
{
    if(handle->serverSettings.primary && g_default) {
        NEXUS_SimpleAudioDecoder_P_RestoreCodecSettings(handle->serverSettings.primary, &handle->codecSettings.primary);
        (void)NEXUS_AudioDecoder_SetSettings(handle->serverSettings.primary, &g_default->settings);
    }
}

static void NEXUS_SimpleAudioDecoder_P_RestoreSecondaryDecoder(NEXUS_SimpleAudioDecoderHandle handle)
{
    if(handle->serverSettings.secondary && g_default) {
        NEXUS_SimpleAudioDecoder_P_RestoreCodecSettings(handle->serverSettings.secondary, &handle->codecSettings.secondary);
        (void)NEXUS_AudioDecoder_SetSettings(handle->serverSettings.secondary, &g_default->settings);
    }
}

static void NEXUS_SimpleAudioDecoder_P_RestoreDescriptionDecoder(NEXUS_SimpleAudioDecoderHandle handle)
{
    if(handle->serverSettings.description && g_default) {
        NEXUS_SimpleAudioDecoder_P_RestoreCodecSettings(handle->serverSettings.description, &handle->codecSettings.description);
        (void)NEXUS_AudioDecoder_SetSettings(handle->serverSettings.description, &g_default->settings);
    }
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
        pNewSettings->description != pCurrentSettings->description) && handle->acquired) return true;
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

    return false;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderServerSettings *pSettings )
{
    unsigned i;
    NEXUS_Error rc;
    bool configOutputs;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->server != server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    /* eMax -> valid and valid -> eMax are ok. Anything else is not allowed. */
    if ( handle->serverSettings.type != NEXUS_SimpleAudioDecoderType_eMax &&
         handle->serverSettings.type != pSettings->type &&
         pSettings->type != NEXUS_SimpleAudioDecoderType_eMax) {
        BDBG_ERR(("Simple Audio Decoder type is not allowed to change on the fly."));
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    if ( pSettings->type == NEXUS_SimpleAudioDecoderType_eMax ){
        /* this means we aren't ready for primetime yet. save settings and return. */
        handle->serverSettings = *pSettings;
        return NEXUS_SUCCESS;
    }

    #if BDBG_DEBUG_BUILD
    if ( handle->serverSettings.type != pSettings->type ) {
        BDBG_MSG(("Simple decoder setting type to: %s", (pSettings->type==NEXUS_SimpleAudioDecoderType_ePersistent)?"Persistent":(pSettings->type==NEXUS_SimpleAudioDecoderType_ePersistent)?"Standalone":"Dynamic"));
    }
    #endif

    configOutputs = nexus_p_check_reconfig(handle, pSettings);

    if (pSettings->primary==NULL) {
        NEXUS_SimpleAudioDecoder_P_RestorePrimaryDecoder(handle);
    }
    if (pSettings->secondary==NULL) {
        NEXUS_SimpleAudioDecoder_P_RestoreSecondaryDecoder(handle);
    }
    if (pSettings->description==NULL) {
        NEXUS_SimpleAudioDecoder_P_RestoreDescriptionDecoder(handle);
    }

    if (configOutputs) {
        NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);
    }
    BDBG_MSG(("NEXUS_SimpleAudioDecoder_SetServerSettings %p: configOutputs %d", (void*)handle, configOutputs));

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

    handle->serverSettings = *pSettings;

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
        NEXUS_SimpleStcChannel_SetAudio_priv(handle->stcChannel, CONNECTED(handle) ? handle : NULL);
    }

    if (configOutputs) {
        rc = NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
        if (rc) {rc = BERR_TRACE(rc);} /* fall through */
    }

    return 0;
}

void NEXUS_SimpleAudioDecoder_GetStcStatus_priv(NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleStcChannelDecoderStatus * pStatus)
{
    NEXUS_OBJECT_ASSERT(NEXUS_SimpleAudioDecoder, handle);
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    pStatus->connected = CONNECTED(handle);
    pStatus->started = handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started; /* Playback does not involve stc channel */
    pStatus->stcIndex = handle->stcIndex;
    NEXUS_SimpleEncoder_GetStcStatus_priv(handle->encoder.handle, &pStatus->encoder);
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SwapServerSettings( NEXUS_SimpleAudioDecoderServerHandle server, NEXUS_SimpleAudioDecoderHandle src, NEXUS_SimpleAudioDecoderHandle dest )
{
    int rc;
    unsigned i;

    BDBG_OBJECT_ASSERT(src, NEXUS_SimpleAudioDecoder);
    BDBG_OBJECT_ASSERT(dest, NEXUS_SimpleAudioDecoder);
    if (src->server != server || dest->server != server) return BERR_TRACE(NEXUS_INVALID_PARAMETER);

    /* allows caller to be directionless swap, but internally to transfer from src -> dest */
    if (dest->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || dest->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || dest->playback.state == state_started) {
        NEXUS_SimpleAudioDecoderHandle temp = src;
        src = dest;
        dest = temp;
    }

    BDBG_MSG(("swap %p -> %p", (void *)src, (void *)dest));
    if (src->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started || src->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started || src->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started || dest->playback.state == state_started) {
        nexus_simpleaudiodecoder_p_stop(src);
    }

    dest->trickState = src->trickState;
    dest->serverSettings = src->serverSettings;
    dest->stcIndex = src->stcIndex;
    dest->currentSpdifInput = src->currentSpdifInput;
    dest->currentHdmiInput = src->currentHdmiInput;
    dest->currentCaptureInput = src->currentCaptureInput;
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
        NEXUS_SimpleStcChannel_SetAudio_priv(src->stcChannel, NULL);
    }
    if (dest->stcChannel) {
        /* dest may have gained some decoders if source had some before */
        NEXUS_SimpleStcChannel_SetAudio_priv(dest->stcChannel, CONNECTED(dest) ? dest : NULL);
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

    if (CONNECTED(dest)) {
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

        NEXUS_SimpleAudioDecoder_Resume(dest);
    }

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
    return;
}

void NEXUS_SimpleAudioDecoder_GetDefaultStartSettings( NEXUS_SimpleAudioDecoderStartSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->primary);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->secondary);
    NEXUS_AudioDecoder_GetDefaultStartSettings(&pSettings->description);
    pSettings->secondary.secondaryDecoder = true;
    pSettings->description.secondaryDecoder = true;
}

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

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_AddOutputs( NEXUS_SimpleAudioDecoderHandle handle)
{
    NEXUS_AudioInputHandle spdifInput, hdmiInput, captureInput;
    unsigned i;
    NEXUS_Error rc;
    NEXUS_AudioCodec primaryCodec, secondaryCodec;

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
#if NEXUS_NUM_AUDIO_CAPTURES
        if (secondaryCodec != NEXUS_AudioCodec_eUnknown) {
            captureInput = handle->serverSettings.capture.input[secondaryCodec];
        }
        if (!captureInput) {
            captureInput = handle->serverSettings.capture.input[primaryCodec];
        }
#endif
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

    /* if outputs are the same, don't change. this avoids needless glitches when starting/stopping decode */
    if (handle->currentSpdifInput == spdifInput && handle->currentHdmiInput == hdmiInput && handle->currentCaptureInput == captureInput) {
        return 0;
    }

    /* always remove because of default configuration */
    NEXUS_SimpleAudioDecoder_P_RemoveOutputs(handle);

    if (spdifInput) {
        for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
            if (handle->serverSettings.spdif.outputs[i]) {
                BDBG_MSG(("%p: add input %p -> spdif%d", (void *)handle, (void *)spdifInput, i));
                rc = NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(handle->serverSettings.spdif.outputs[i]), spdifInput);
                if (rc) { rc = BERR_TRACE(rc); goto error_start;}
            }
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (hdmiInput) {
        for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
            if (handle->serverSettings.hdmi.outputs[i]) {
                BDBG_MSG(("%p: add input %p -> hdmi%d", (void *)handle, (void *)hdmiInput, i));
                rc = NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(handle->serverSettings.hdmi.outputs[i]), hdmiInput);
                if (rc) { rc = BERR_TRACE(rc); goto error_start;}
            }
        }
    }
#endif

#if NEXUS_NUM_AUDIO_CAPTURES
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
#endif

    NEXUS_SimpleAudioDecoder_Resume(handle);

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

    if (handle->encoder.handle) {
        return;
    }
    /* Suspend playbacks first as suspending the decoder will cause the
       decoder to detach from the mixer but if anything is running it can't */
    NEXUS_SimpleAudioDecoder_Suspend(handle);

    if (handle->serverSettings.primary) {
        nexus_p_check_decoder(handle->serverSettings.primary);
    }
    if (handle->serverSettings.secondary) {
        nexus_p_check_decoder(handle->serverSettings.secondary);
    }

    if (handle->currentSpdifInput) {
        for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_SPDIF_OUTPUTS;i++) {
            if (handle->serverSettings.spdif.outputs[i]) {
                BDBG_MSG(("%p: remove input %p -> spdif%d", (void *)handle, (void *)handle->currentSpdifInput, i));
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(handle->serverSettings.spdif.outputs[i]));
            }
        }
        handle->currentSpdifInput = NULL;
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (handle->currentHdmiInput) {
        for (i=0;i<NEXUS_MAX_SIMPLE_DECODER_HDMI_OUTPUTS;i++) {
            if (handle->serverSettings.hdmi.outputs[i]) {
                BDBG_MSG(("%p: remove input %p -> hdmi%d", (void *)handle, (void *)handle->currentHdmiInput, i));
                NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(handle->serverSettings.hdmi.outputs[i]));
            }
        }
        handle->currentHdmiInput = NULL;
    }
#endif

#if NEXUS_NUM_AUDIO_CAPTURES
    if (handle->currentCaptureInput) {
        if (handle->serverSettings.capture.output)
        {
            BDBG_MSG(("%p: remove input %p -> capture", (void *)handle, (void *)handle->currentCaptureInput));
            NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioCapture_GetConnector(handle->serverSettings.capture.output));
        }
        handle->currentCaptureInput = NULL;
    }
#endif

    /* can't call NEXUS_AudioInput_Shutdown on handle->serverSettings.spdif/hdmi.input[] because it could be stereo connector */
}

static bool nexus_p_is_decoder_connected(NEXUS_AudioDecoderHandle audioDecoder)
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

static bool nexus_p_is_playback_connected(NEXUS_AudioPlaybackHandle playback)
{
    bool connected=false;

    if ( playback ) {
        NEXUS_AudioInput_HasConnectedOutputs(NEXUS_AudioPlayback_GetConnector(playback), &connected);
    }

    return connected;
}

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
        NEXUS_SimpleStcChannel_SetAudio_priv(stcChannel, CONNECTED(handle) ? handle : NULL);
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

    if (!bypass_p_start) {
        rc = nexus_simpleaudiodecoder_p_start(handle);
        if (rc) {
            NEXUS_SimpleAudioDecoder_Stop(handle);
            BERR_TRACE(rc); /* fall through */
        }
        NEXUS_SimpleAudioDecoder_Resume(handle);
    }
    return rc;
}

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
    NEXUS_AudioConnectorType connectorType)
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
        /* multichannel does not currently support post processing, return the connector here */
        if ( connector != NULL ) {
            return connector;
        }
        break;
    case NEXUS_AudioConnectorType_eStereo:
        BDBG_MSG(("%s: selector %u, stereo decoder connector %p", __FUNCTION__, selector, (void*)connector));
        /* look for terminal connector from this decoder output */
        if ( connector ) {
            unsigned j;
            NEXUS_AudioInputHandle tail = connector;
            for(j=nexus_simpleaudiodecoder_pp_first();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_next(j)) {
                if ( handle->decoders[selector].processor[j] != NULL ) {
                    bool connected = false;
                    NEXUS_AudioInput_IsConnectedToInput(tail, NEXUS_AudioProcessor_GetConnector(handle->decoders[selector].processor[j]), &connected);

                    if ( connected ) {
                        tail = NEXUS_AudioProcessor_GetConnector(handle->decoders[selector].processor[j]);
                    }
                }
            }
            return tail;
        }
        break;
    default:
        break;
    }
    return NULL;
}

static NEXUS_Error nexus_simpleaudiodecoder_connect_processing(NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType)
{
    NEXUS_AudioInputHandle tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType);

    BDBG_MSG(("Connect decoder connection (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)tail));

    /* we only support PP on stereo path for now */
    if ( connectorType == NEXUS_AudioConnectorType_eStereo && tail )
    {
        unsigned j;
        /* connect post processing graph */
        for(j=nexus_simpleaudiodecoder_pp_first();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_next(j)) {
            if ( handle->decoders[selector].processor[j] != NULL ) {
                NEXUS_AudioProcessor_AddInput(handle->decoders[selector].processor[j], tail);
                tail = NEXUS_AudioProcessor_GetConnector(handle->decoders[selector].processor[j]);
                BDBG_MSG(("--> pp (%u) connection (stereo) %p", j, (void*)tail));
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

    tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType);

    BDBG_MSG(("current decoder tail (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)tail));
    if ( mixer && tail ) {
        BDBG_MSG(("--> mixer (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer));
        rc = NEXUS_AudioMixer_AddInput(mixer, tail);
        if ( rc != NEXUS_SUCCESS ) {
            return BERR_TRACE(rc);
        }

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
                    BDBG_MSG(("remove input to processor %p", (void*)NEXUS_AudioProcessor_GetConnector(handle->decoders[selector].processor[j])));
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

    tail = nexus_simpleaudiodecoder_get_current_decoder_connection(handle, selector, connectorType);

    if ( mixer && tail ) {
        BDBG_MSG(("remove input to mixer (%s) %p", (connectorType == NEXUS_AudioConnectorType_eStereo)?"stereo":"multichannel", (void*)mixer));
        NEXUS_AudioMixer_RemoveInput(mixer, tail);
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

    for ( i = 0; i < NEXUS_SimpleAudioDecoderSelector_eMax; i++ ) {
        switch ( i ) {
        case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        case NEXUS_SimpleAudioDecoderSelector_eDescription:
            rc = nexus_simpleaudiodecoder_connect_processing(handle, i, NEXUS_AudioConnectorType_eStereo);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_mixer(handle, i, NEXUS_AudioConnectorType_eStereo);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_processing(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            rc = nexus_simpleaudiodecoder_connect_mixer(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            if ( rc != NEXUS_SUCCESS ) {
                return BERR_TRACE(rc);
            }
            break;
        default:
            break;
        }
    }

    return NEXUS_SUCCESS;
}

static void nexus_simpleaudiodecoder_disconnect_downstream(NEXUS_SimpleAudioDecoderHandle handle)
{
    unsigned i;

    for (i=0;i<NEXUS_SimpleAudioDecoderSelector_eMax;i++) {
        switch ( i ) {
        case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        case NEXUS_SimpleAudioDecoderSelector_eDescription:
            nexus_simpleaudiodecoder_disconnect_mixer(handle, i, NEXUS_AudioConnectorType_eStereo);
            nexus_simpleaudiodecoder_disconnect_processing(handle, i, NEXUS_AudioConnectorType_eStereo);
            nexus_simpleaudiodecoder_disconnect_mixer(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            nexus_simpleaudiodecoder_disconnect_processing(handle, i, NEXUS_AudioConnectorType_eMultichannel);
            break;
        default:
            break;
        }
    }
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
    int rc = 0;
#if NEXUS_HAS_SYNC_CHANNEL
    NEXUS_AudioConnectorType connectorType = handle->serverSettings.syncConnector;
#endif

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
        ((desired_state == state_started || desired_state == state_suspended) && !nexus_p_is_playback_connected(playback)))
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
        decoder = handle->serverSettings.primary;
        pPrimer = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].primer;
        pDecoderSettings = &handle->settings.primary;
        pCodecSettings = &handle->codecSettings.primary;
        pstate = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state;
        wants_priming = handle->startSettings.primer.pcm;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary:
        decoder = handle->serverSettings.secondary;
        pPrimer = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].primer;
        pDecoderSettings = &handle->settings.secondary;
        pCodecSettings = &handle->codecSettings.secondary;
        pstate = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state;
        wants_priming = handle->startSettings.primer.compressed;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eDescription:
        decoder = handle->serverSettings.description;
        pPrimer = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].primer;
        pDecoderSettings = &handle->settings.description;
        pCodecSettings = &handle->codecSettings.description;
        pstate = &handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state;
        wants_priming = false;
        break;
    default: return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    orgstate = *pstate;
    if (*pstate == state_priming) {
        BDBG_ASSERT(*pPrimer);
    }

    /* fix such that we check for potential connections too... */
    BDBG_MSG(("change state decoder %p, selector %d, orgstate %d, desired_state %d", (void*)decoder, selector, orgstate, desired_state));
    /* adjust desired state */
    if (!decoder ||
        ((desired_state == state_started || desired_state == state_suspended) && !nexus_p_is_decoder_connected(decoder)))
    {
        BDBG_MSG(("  decoder %p selector %d has no outputs connected. demoting desired_state %d to stopped", (void*)decoder, selector, desired_state));
        desired_state = state_stopped;
    }
#if NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA
    if (wants_priming) {
        if (desired_state == state_stopped) {
            desired_state = state_priming;
        }
        if (!*pPrimer) {
            *pPrimer = NEXUS_AudioDecoderPrimer_Create(NULL);
            if (!*pPrimer) return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        }
    }
#else
    /* no primer for RAP because of limited use cases and resources */
    BDBG_ASSERT(desired_state != state_priming);
    wants_priming = false;
#endif

    if (wants_priming && !handle->stcChannel) {
        BDBG_WRN(("%p: cannot prime without stcChannel", (void*)handle));
        wants_priming = false;
    }

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

NEXUS_Error NEXUS_SimpleAudioDecoder_Suspend(NEXUS_SimpleAudioDecoderHandle handle)
{
    return NEXUS_SimpleAudioDecoder_P_Suspend(handle, SUSPEND_ALL);
}

static NEXUS_Error NEXUS_SimpleAudioDecoder_P_Suspend(NEXUS_SimpleAudioDecoderHandle handle, unsigned whatToSuspend)
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

    if (whatToSuspend & SUSPEND_PLAYBACK) {
        NEXUS_SimpleAudioPlaybackHandle audioPlayback;
        for (audioPlayback = BLST_S_FIRST(&handle->server->playbacks); audioPlayback; audioPlayback = BLST_S_NEXT(audioPlayback, link)) {
            nexus_simpleaudioplayback_p_suspend(audioPlayback);
        }
    }

    if ((whatToSuspend & SUSPEND_MIXER) && nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed()) {
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

    return 0;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_Resume(NEXUS_SimpleAudioDecoderHandle handle)
{
    int rc;

    if (handle->mixers.suspended && nexus_simpleaudiodecoder_p_running_mixer_input_changes_allowed()) {
        handle->mixers.suspended = false;
        if (handle->serverSettings.mixers.stereo) {
            BDBG_MSG(("Start stereo mixer."));
            rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.stereo);
            if (rc != NEXUS_SUCCESS) {
                BERR_TRACE(rc);
            }
        }
        if (handle->serverSettings.mixers.multichannel) {
            BDBG_MSG(("Start multichannel mixer."));
            rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.multichannel);
            if (rc != NEXUS_SUCCESS) {
                BERR_TRACE(rc);
            }
        }
        if (handle->serverSettings.mixers.persistent) {
            BDBG_MSG(("Start persistent mixer."));
            rc = NEXUS_AudioMixer_Start(handle->serverSettings.mixers.persistent);
            if (rc != NEXUS_SUCCESS) {
                BERR_TRACE(rc);
            }
        }
    }

    {
        NEXUS_SimpleAudioPlaybackHandle audioPlayback;
        for (audioPlayback = BLST_S_FIRST(&handle->server->playbacks); audioPlayback; audioPlayback = BLST_S_NEXT(audioPlayback, link)) {
            nexus_simpleaudioplayback_p_resume(audioPlayback);
        }
    }

    if (handle->suspended) {
        if (CONNECTED(handle)) {
            rc = nexus_simpleaudiodecoder_p_start_resume(handle, NEXUS_SIMPLEAUDIODECODER_ALL);
            if (!rc) handle->suspended = false;
        }
    }

    return 0;
}

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
            NEXUS_SimpleAudioDecoder_Suspend(slave);
            /* give the master the decoder, just temporarily */
            handle->serverSettings.primary = slave->serverSettings.primary;
        }
        else {
            if (displayEncode)
            {
                NEXUS_SimpleAudioDecoder_Suspend(handle);
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
            NEXUS_SimpleAudioDecoder_Resume(slave);
        }
        else {
            NEXUS_SimpleAudioDecoder_Resume(handle);
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
    BDBG_ASSERT(!handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started && !handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started && !handle->playback.state == state_started);

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

void NEXUS_SimpleAudioDecoder_Stop( NEXUS_SimpleAudioDecoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);

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
}

static void nexus_simpleaudiodecoder_p_stop( NEXUS_SimpleAudioDecoderHandle handle)
{
    BDBG_MSG(("nexus_simpleaudiodecoder_p_stop %p", (void *)handle));
#if NEXUS_HAS_HDMI_INPUT
    if (handle->hdmiInput.handle) {
        if (!handle->encoder.handle) {
            NEXUS_AudioInputCapture_Stop(handle->hdmiInput.inputCapture);
            return;
        }
    }
#endif
    /* first, stop decode */
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_ePrimary, state_stopped);
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eSecondary, state_stopped);
    nexus_simpleaudiodecoder_change_state(handle, NEXUS_SimpleAudioDecoderSelector_eDescription, state_stopped);
    nexus_simpleaudiodecoder_change_state_pb(handle, state_stopped);

    if ((handle->currentSpdifInput || handle->currentHdmiInput) && handle->acquired) {
        NEXUS_SimpleAudioDecoder_P_Suspend(handle, SUSPEND_DECODER);
        nexus_simpleaudiodecoder_disconnect_downstream(handle);
        (void)NEXUS_SimpleAudioDecoder_P_AddOutputs(handle);
    }
    else
    {
        nexus_simpleaudiodecoder_disconnect_downstream(handle);
    }
}

void NEXUS_SimpleAudioDecoder_GetSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSettings *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    *pSettings = handle->settings;
    return;
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

    for ( i = 0; i < NEXUS_SimpleAudioDecoderSelector_eMax; i++ )
    {
        if ( i != NEXUS_SimpleAudioDecoderSelector_eSecondary ) {
            unsigned j;
            bool wasConnected, connected;
            for ( j = nexus_simpleaudiodecoder_pp_first(); j < NEXUS_AudioPostProcessing_eMax; j = nexus_simpleaudiodecoder_pp_next(j) ) {
                NEXUS_AudioProcessorSettings processorSettings;
                bool connect = false;
                bool disconnect = false;

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
                }
                else if ( disconnect )
                {
                    if ( handle->decoders[i].processor[j] )
                    {
                        NEXUS_AudioProcessor_RemoveAllInputs(handle->decoders[i].processor[j]);
                        NEXUS_AudioProcessor_Close(handle->decoders[i].processor[j]);
                        handle->decoders[i].processor[j] = NULL;
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

    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetSettings( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_SimpleAudioDecoderSettings *pSettings )
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(pSettings);

    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    nexus_simpleaudiodecoder_apply_processor_settings(handle, pSettings);

    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.primary, &pSettings->primary);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.secondary, &pSettings->secondary);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
        rc = NEXUS_AudioDecoder_SetSettings(handle->serverSettings.description, &pSettings->description);
        if (rc) return BERR_TRACE(rc);
    }

    handle->settings = *pSettings;
    return NEXUS_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetStatus( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioDecoderStatus *pStatus )
{
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
    if (handle->decoders[selector].state == state_started) {
        /* requested decoder is indeed started. Try to locate processor */
        unsigned j;
        for(j=nexus_simpleaudiodecoder_pp_first();j<NEXUS_AudioPostProcessing_eMax;j=nexus_simpleaudiodecoder_pp_next(j)) {
            NEXUS_AudioProcessorSettings processorSettings;
            if ( handle->decoders[selector].processor[j] != NULL ) {
                NEXUS_AudioProcessor_GetSettings(handle->decoders[selector].processor[j], &processorSettings);
                if ( processorSettings.type == type ) {
                    /* found it */
                    NEXUS_AudioProcessor_GetStatus(handle->decoders[selector].processor[j], pStatus);
                    return NEXUS_SUCCESS;
                }
            }
        }
    }
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

NEXUS_Error NEXUS_SimpleAudioDecoder_GetPresentationStatus( NEXUS_SimpleAudioDecoderHandle handle, unsigned presentationIndex, NEXUS_AudioDecoderPresentationStatus *pStatus )
{
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
    return 0;
}

void NEXUS_SimpleAudioDecoder_Flush( NEXUS_SimpleAudioDecoderHandle handle )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        NEXUS_AudioDecoder_Flush(handle->serverSettings.primary);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        NEXUS_AudioDecoder_Flush(handle->serverSettings.secondary);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
        NEXUS_AudioDecoder_Flush(handle->serverSettings.description);
    }
    if (handle->playback.state == state_started) {
        NEXUS_AudioPlayback_Flush(handle->serverSettings.passthroughPlayback);
    }
}

void NEXUS_SimpleAudioDecoder_GetTrickState( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_AudioDecoderTrickState *pSettings )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    *pSettings = handle->trickState;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetTrickState( NEXUS_SimpleAudioDecoderHandle handle, const NEXUS_AudioDecoderTrickState *pSettings )
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->playback.state != state_stopped) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_SetTrickState(handle->serverSettings.primary, pSettings);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_SetTrickState(handle->serverSettings.secondary, pSettings);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
        rc = NEXUS_AudioDecoder_SetTrickState(handle->serverSettings.description, pSettings);
        if (rc) return BERR_TRACE(rc);
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

    handle->trickState = *pSettings;
    return 0;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_Advance( NEXUS_SimpleAudioDecoderHandle handle, uint32_t pts )
{
    NEXUS_Error rc;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->playback.state != state_stopped) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
        rc = NEXUS_AudioDecoder_Advance(handle->serverSettings.primary, pts);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
        rc = NEXUS_AudioDecoder_Advance(handle->serverSettings.secondary, pts);
        if (rc) return BERR_TRACE(rc);
    }
    if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
        rc = NEXUS_AudioDecoder_Advance(handle->serverSettings.description, pts);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_SetCodecSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector, const NEXUS_AudioDecoderCodecSettings *pSettings)
{
    NEXUS_Error rc;

    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(pSettings);
    if (!CONNECTED(handle) && handle->serverSettings.disableMode == NEXUS_SimpleDecoderDisableMode_eFail) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    if(pSettings->codec >= NEXUS_AudioCodec_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(selector >= NEXUS_SimpleAudioDecoderSelector_eMax) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if(selector == NEXUS_SimpleAudioDecoderSelector_ePrimary) {
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_ePrimary].state == state_started) {
            rc = NEXUS_AudioDecoder_SetCodecSettings(handle->serverSettings.primary, pSettings);
            if (rc) return BERR_TRACE(rc);
        }
        handle->codecSettings.primary.codecSettings[pSettings->codec] = *pSettings;
    }
    else if (selector == NEXUS_SimpleAudioDecoderSelector_eSecondary) {
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eSecondary].state == state_started) {
            rc = NEXUS_AudioDecoder_SetCodecSettings(handle->serverSettings.secondary, pSettings);
            if (rc) return BERR_TRACE(rc);
        }
        handle->codecSettings.primary.codecSettings[pSettings->codec] = *pSettings;
    }
    else if (selector == NEXUS_SimpleAudioDecoderSelector_eDescription) {
        if (handle->decoders[NEXUS_SimpleAudioDecoderSelector_eDescription].state == state_started) {
            rc = NEXUS_AudioDecoder_SetCodecSettings(handle->serverSettings.description, pSettings);
            if (rc) return BERR_TRACE(rc);
        }
        handle->codecSettings.description.codecSettings[pSettings->codec] = *pSettings;
    }

    return NEXUS_SUCCESS;
}

void NEXUS_SimpleAudioDecoder_GetCodecSettings( NEXUS_SimpleAudioDecoderHandle handle, NEXUS_SimpleAudioDecoderSelector selector, NEXUS_AudioCodec codec, NEXUS_AudioDecoderCodecSettings *pSettings)
{
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

    if(selector == NEXUS_SimpleAudioDecoderSelector_ePrimary) {
        if(handle->codecSettings.primary.codecSettings[codec].codec != NEXUS_AudioCodec_eUnknown) {
            *pSettings = handle->codecSettings.primary.codecSettings[codec];
        } else if (g_default) {
            *pSettings = g_default->codecSettings.codecSettings[codec];
        }
    }
    else if(selector == NEXUS_SimpleAudioDecoderSelector_eSecondary) {
        if(handle->codecSettings.secondary.codecSettings[codec].codec != NEXUS_AudioCodec_eUnknown) {
            *pSettings = handle->codecSettings.secondary.codecSettings[codec];
        } else if (g_default) {
            *pSettings = g_default->codecSettings.codecSettings[codec];
        }
    }
    else if(selector == NEXUS_SimpleAudioDecoderSelector_eDescription) {
        if(handle->codecSettings.description.codecSettings[codec].codec != NEXUS_AudioCodec_eUnknown) {
            *pSettings = handle->codecSettings.description.codecSettings[codec];
        } else if (g_default) {
            *pSettings = g_default->codecSettings.codecSettings[codec];
        }
    }
    return;
}

BDBG_FILE_MODULE(nexus_simple_decoder_proc);

void NEXUS_SimpleDecoderModule_P_PrintAudioDecoder(void)
{
#if BDBG_DEBUG_BUILD
    NEXUS_SimpleAudioDecoderHandle handle;
    NEXUS_SimpleAudioPlaybackHandle audioPlayback;

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
    for (audioPlayback = nexus_simple_audio_playback_p_first(); audioPlayback; audioPlayback = nexus_simple_audio_playback_p_next(audioPlayback)) {
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("audiopb %d(%p)", audioPlayback->index, (void *)audioPlayback));
        BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  acquired %d, started %d, clientStarted %d",   audioPlayback->acquired,
            audioPlayback->started, audioPlayback->clientStarted));
        if (audioPlayback->suspended) {
            BDBG_MODULE_LOG(nexus_simple_decoder_proc, ("  suspended"));
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
#if NEXUS_HAS_HDMI_INPUT
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
        NEXUS_SimpleAudioDecoder_Suspend(handle);
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
        NEXUS_SimpleAudioDecoder_Resume(handle);
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
#if NEXUS_HAS_HDMI_INPUT
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
    if ( handle->serverSettings.passthroughPlayback ) {
        return NEXUS_AudioPlayback_GetBuffer(handle->serverSettings.passthroughPlayback, pBuffer, pSize);
    }
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
    if ( handle->serverSettings.passthroughPlayback ) {
        return NEXUS_AudioPlayback_WriteComplete(handle->serverSettings.passthroughPlayback, amountWritten);
    }
    return BERR_SUCCESS;
}

NEXUS_Error NEXUS_SimpleAudioDecoder_AddOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output,
    NEXUS_SimpleAudioDecoderSelector selector,
    NEXUS_AudioConnectorType connectorType
    )
{
    NEXUS_AudioDecoderHandle decoder = NULL;
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(output != NULL);

    if ( handle->serverSettings.type != NEXUS_SimpleAudioDecoderType_eStandalone ) {
        BDBG_ERR(("Outputs can only be directly added for eStandalone Simple Audio Decoders. Other types connect to the primary display."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    switch ( selector ) {
    case NEXUS_SimpleAudioDecoderSelector_ePrimary:
        decoder = handle->serverSettings.primary;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eSecondary:
        decoder = handle->serverSettings.secondary;
        break;
    case NEXUS_SimpleAudioDecoderSelector_eDescription:
    default:
        BDBG_ERR(("Invalid Selector. Only ePrimary or eSecondary (Passthrough) are valid for Standalone Simple Audio Decoders."));
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        break;
    }

    if ( !decoder ) {
        BDBG_ERR(("No primary decoder in this Simple Decoder. Cannot add output."));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    return BERR_TRACE(NEXUS_AudioOutput_AddInput(output, NEXUS_AudioDecoder_GetConnector(decoder, connectorType)));
}

NEXUS_Error NEXUS_SimpleAudioDecoder_RemoveOutput(
    NEXUS_SimpleAudioDecoderHandle handle,
    NEXUS_AudioOutputHandle output
    )
{
    BDBG_OBJECT_ASSERT(handle, NEXUS_SimpleAudioDecoder);
    BDBG_ASSERT(output != NULL);

    BSTD_UNUSED(handle);

    return BERR_TRACE(NEXUS_AudioOutput_RemoveAllInputs(output));
}
