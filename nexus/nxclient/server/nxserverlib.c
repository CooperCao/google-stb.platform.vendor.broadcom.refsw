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
#include "nexus_display_vbi.h"
#if NEXUS_HAS_DISPLAY
#include "nexus_display_private.h"
#endif
#include "nexus_ccir656_output.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "bstd.h"


BDBG_MODULE(nxserverlib);
#define BDBG_MSG_TRACE(X) /* BDBG_MSG(X) */

#if NEXUS_HAS_HDMI_OUTPUT
#if NEXUS_HAS_HDCP_2X_SUPPORT
static const char HDCP2x_DEFAULT_BIN[] =  "./drm.bin" ;
#endif
static const char HDCP1x_DEFAULT_BIN[] = "./hdcp1xKeys.bin" ;
#endif

static void b_disconnect(nxclient_t client, struct b_connect *connect);
static int init_session(nxserver_t server, unsigned index);
static void uninit_session(struct b_session *session);
static NEXUS_Error NxClient_P_SetSessionAudioSettings(struct b_session *session, const NxClient_AudioSettings *pSettings);
#if NEXUS_HAS_HDMI_OUTPUT
static void initializeHdmiOutputHdcpSettings(struct b_session *session, NxClient_HdcpVersion version_select);
static void nxserver_hdcp_mute(struct b_session *session);
static NEXUS_Error nxserver_load_hdcp_keys(struct b_session *session, NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset, unsigned size);
static NEXUS_Error nxserver_set_hdmi_input_repeater(nxclient_t client, NEXUS_HdmiInputHandle hdmiInput);
static void nxserver_check_hdcp(struct b_session *session);
#endif

static NEXUS_Error bserver_set_standby_settings(nxserver_t server, const NxClient_StandbySettings *pSettings);
static void nxserver_p_set_sd_outputs(struct b_session *session, const NxClient_DisplaySettings *pSettings);
static NEXUS_VideoFormat nxserver_p_default_sd_format(struct b_session *session);
static void nxserver_p_immediate_watchdog(nxclient_t client);
static bool nxserver_is_standby_pending(nxserver_t server);

#undef MIN
#define MIN(A,B) ((A)<(B)?(A):(B))

BDBG_OBJECT_ID(b_req);
BDBG_OBJECT_ID(b_connect);
BDBG_OBJECT_ID(b_client);

static const char *b_resource_str[b_resource_max] = {
    "client",
    "surface_client",
    "simple_video_decoder",
    "simple_audio_decoder",
    "simple_audio_playback",
    "input_client",
    "simple_encoder",
    "audio_capture",
    "audio_crc",
    "register_standby"
};

static const char *clientModeStr[NEXUS_ClientMode_eMax] = {"unprotected","verified","protected","untrusted"};

void inc_id(nxserver_t server, enum b_resource r)
{
    if (++server->nextId[r] == 0) {
        ++server->nextId[r];
    }
}

static bool nxserver_p_valid_certificate(nxserver_t server, const NEXUS_Certificate *pCertificate)
{
    return server->settings.certificate.length == pCertificate->length &&
        !memcmp(server->settings.certificate.data, pCertificate->data, server->settings.certificate.length);
}

nxclient_t NxClient_P_CreateClient(nxserver_t server, const NxClient_JoinSettings *pJoinSettings, NEXUS_Certificate *pCert, unsigned pid)
{
    nxclient_t client;
    unsigned sessionIndex;
    int rc;
    NxClient_JoinSettings defaultJoinSettings;

    if (!pJoinSettings) {
        NxClient_GetDefaultJoinSettings(&defaultJoinSettings);
        pJoinSettings = &defaultJoinSettings;
    }

    sessionIndex = pJoinSettings->session;
    if (sessionIndex >= NXCLIENT_MAX_SESSIONS) {
        BDBG_WRN(("session %d not supported", pJoinSettings->session));
        return NULL;
    }

    if (!server->session[sessionIndex]) {
        rc = init_session(server, sessionIndex);
        if (rc) {
            BERR_TRACE(rc);
            return NULL;
        }
    }

    client = BKNI_Malloc(sizeof(*client));
    if (!client) {
        BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        return NULL;
    }
    BKNI_Memset(client, 0, sizeof(*client));
    BDBG_OBJECT_SET(client, b_client);
    BLST_D_INSERT_HEAD(&server->clients, client, link);
    client->server = server;
    client->session = server->session[sessionIndex];
    client->pid = pid;
    client->joinSettings = *pJoinSettings;
    client->joinSettings.name[sizeof(client->joinSettings.name)-1]='\0';
    client->connect_settings.allow_grab = true;
    BLST_D_INSERT_HEAD(&client->session->clients, client, session_link);

    /* if pid 0, this is a local connection */
    if (pid) {
        client->ipc = true;
        BKNI_Memset(pCert, 0, sizeof(pCert));
        pCert->length = BKNI_Snprintf((char *)pCert->data, sizeof(pCert->data), "%u,%#x%#x,%s", server->nextId[b_resource_client], (unsigned)lrand48(), (unsigned)lrand48(), client->joinSettings.name);
        if(pCert->length>=sizeof(pCert->data)-1) {
            BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }

        inc_id(server, b_resource_client);
        NEXUS_Platform_GetDefaultClientSettings(&client->clientSettings);
        client->clientSettings.authentication.certificate = *pCert;

        /* server may require cert for eProtected */
        if (server->settings.certificate.length && client->joinSettings.mode != NEXUS_ClientMode_eUntrusted) {
            if (nxserver_p_valid_certificate(server, &pJoinSettings->certificate)) {
                client->clientSettings.configuration.mode = client->joinSettings.mode;
            }
            else {
                client->clientSettings.configuration.mode = NEXUS_ClientMode_eUntrusted;
            }
        }
        else if (server->settings.client_mode != NEXUS_ClientMode_eMax) {
            /* force the given mode */
            client->clientSettings.configuration.mode = server->settings.client_mode;
        }
        else {
            /* set requested mode, but connect callback may modify */
            client->clientSettings.configuration.mode = client->joinSettings.mode;
        }

        BKNI_Memcpy(client->clientSettings.configuration.heap, server->settings.client.heap, sizeof(client->clientSettings.configuration.heap));
        if (server->settings.client.connect) {
            rc = server->settings.client.connect(client, &client->joinSettings, &client->clientSettings, &client->connect_settings);
            if (rc) {
                BDBG_WRN(("client connection refused by server: %d", rc));
                goto error;
            }
        }
        if (client->joinSettings.name[0]) {
            BDBG_LOG(("%s(%p) registering as '%s' %s", client->joinSettings.name, (void*)client, (char *)pCert->data,
                clientModeStr[client->clientSettings.configuration.mode]));
        }
        client->nexusClient = NEXUS_Platform_RegisterClient(&client->clientSettings);
        if(!client->nexusClient) {
            (void)BERR_TRACE(BERR_NOT_SUPPORTED);
            goto error;
        }
    }

    return client;

error:
    NxClient_P_DestroyClient(client);
    return NULL;
}

static int b_add_id_list(NEXUS_ClientResourceIdList *pResourceList, unsigned id)
{
    if (pResourceList->total >= NEXUS_MAX_IDS) return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    pResourceList->id[pResourceList->total++] = id;
    return 0;
}

static void b_remove_id_list(NEXUS_ClientResourceIdList *pResourceList, unsigned id)
{
    unsigned i;
    bool found = false;
    for (i=0;i<pResourceList->total;i++) {
        if (found) {
            pResourceList->id[i-1] = pResourceList->id[i];
        }
        else if (pResourceList->id[i] == id) {
            found = true;
        }
    }
    if (found) {
        pResourceList->total--;
        pResourceList->id[pResourceList->total] = 0;
    }
}

static int b_grant_resource_id(nxclient_t client, enum b_resource resource_type, unsigned id)
{
    NEXUS_ClientResources resources;
    int rc;

    if (!client->nexusClient) return 0; /* local */
    if (client->clientSettings.configuration.mode != NEXUS_ClientMode_eUntrusted) return 0;

    NEXUS_Platform_GetClientResources(client->nexusClient, &resources);
    switch (resource_type) {
    case b_resource_surface_client:
        rc = b_add_id_list(&resources.surfaceClient, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_simple_video_decoder:
        rc = b_add_id_list(&resources.simpleVideoDecoder, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_simple_audio_decoder:
        rc = b_add_id_list(&resources.simpleAudioDecoder, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_input_client:
        rc = b_add_id_list(&resources.inputClient, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_simple_encoder:
        rc = b_add_id_list(&resources.simpleEncoder, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_audio_capture:
        rc = b_add_id_list(&resources.audioCapture, id);
        if (rc) return BERR_TRACE(rc);
        break;
    case b_resource_audio_crc:
        rc = b_add_id_list(&resources.audioCrc, id);
        if (rc) return BERR_TRACE(rc);
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_Platform_SetClientResources(client->nexusClient, &resources);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

static int b_revoke_resource_id(nxclient_t client, enum b_resource resource_type, unsigned id)
{
    NEXUS_ClientResources resources;

    /* if the client is already unregistered, there's no need to revoke */
    if (!client->nexusClient) return 0;
    if (client->clientSettings.configuration.mode != NEXUS_ClientMode_eUntrusted) return 0;

    NEXUS_Platform_GetClientResources(client->nexusClient, &resources);
    switch (resource_type) {
    case b_resource_surface_client:
        b_remove_id_list(&resources.surfaceClient, id);
        break;
    case b_resource_simple_video_decoder:
        b_remove_id_list(&resources.simpleVideoDecoder, id);
        break;
    case b_resource_simple_audio_decoder:
        b_remove_id_list(&resources.simpleAudioDecoder, id);
        break;
    case b_resource_input_client:
        b_remove_id_list(&resources.inputClient, id);
        break;
    case b_resource_simple_encoder:
        b_remove_id_list(&resources.simpleEncoder, id);
        break;
    case b_resource_audio_capture:
        b_remove_id_list(&resources.audioCapture, id);
        break;
    case b_resource_audio_crc:
        b_remove_id_list(&resources.audioCrc, id);
        break;
    default:
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    return NEXUS_Platform_SetClientResources(client->nexusClient, &resources);
}

static void b_release_request(nxclient_t client, struct b_req *req)
{
    unsigned i;
    int rc;
    bool revoke_failed = false;

    BDBG_OBJECT_ASSERT(req, b_req);

    BDBG_MSG(("b_release_request req %p", (void*)req));
    /* disconnect from any connects */
    {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect;) {
            unsigned i;
            struct b_connect *next = BLST_D_NEXT(connect, link);
            for (i=0;i<b_resource_max;i++) {
                if (connect->req[i] == req) {
                    b_disconnect(client, connect);
                    break;
                }
            }
            connect = next;
        }
    }

    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        if (req->handles.surfaceClient[i].handle) {
            NEXUS_SurfaceCompositor_DestroyClient(req->handles.surfaceClient[i].handle);
            rc = b_revoke_resource_id(client, b_resource_surface_client, req->handles.surfaceClient[i].id);
            if (rc) revoke_failed = true;
        }
    }
#if NEXUS_HAS_SIMPLE_DECODER
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        if (req->handles.simpleVideoDecoder[i].handle) {
            NEXUS_SimpleVideoDecoder_Destroy(req->handles.simpleVideoDecoder[i].handle);
            rc = b_revoke_resource_id(client, b_resource_simple_video_decoder, req->handles.simpleVideoDecoder[i].id);
            if (rc) revoke_failed = true;
        }
    }
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        if (req->handles.simpleAudioPlayback[i].handle) {
            NEXUS_SimpleAudioPlayback_Destroy(req->handles.simpleAudioPlayback[i].handle);
        }
    }
    if (req->handles.simpleAudioDecoder.handle) {
        NEXUS_SimpleAudioDecoder_Destroy(req->handles.simpleAudioDecoder.handle);
        rc = b_revoke_resource_id(client, b_resource_simple_audio_decoder, req->handles.simpleAudioDecoder.id);
        if (rc) revoke_failed = true;
    }
#endif
#if NEXUS_HAS_INPUT_ROUTER
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        if (req->handles.inputClient[i].handle) {
            NEXUS_InputRouter_DestroyClient(req->handles.inputClient[i].handle);
            rc = b_revoke_resource_id(client, b_resource_input_client, req->handles.inputClient[i].id);
            if (rc) revoke_failed = true;
        }
    }
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        if (req->handles.simpleEncoder[i].handle) {
            NEXUS_SimpleEncoder_Destroy(req->handles.simpleEncoder[i].handle);
            rc = b_revoke_resource_id(client, b_resource_simple_encoder, req->handles.simpleEncoder[i].id);
            if (rc) revoke_failed = true;
        }
    }
#endif
#if NEXUS_HAS_AUDIO
    if (req->handles.audioCapture.handle) {
        nxserverlib_close_audio_capture(client->session, req->handles.audioCapture.id);
        rc = b_revoke_resource_id(client, b_resource_audio_capture, req->results.audioCapture.id);
        if (rc) revoke_failed = true;
    }

    if (req->handles.audioCrc.handle) {
        nxserverlib_close_audio_crc(client->session, req->handles.audioCrc.id);
        rc = b_revoke_resource_id(client, b_resource_audio_crc, req->results.audioCrc.id);
        if (rc) revoke_failed = true;
    }
#endif

    BLST_D_REMOVE(&client->requests, req, link);
    BDBG_OBJECT_DESTROY(req, b_req);
    BKNI_Free(req);
    if (revoke_failed) {
        BDBG_ERR(("client %p will be destroyed because it has revoked resources", (void*)client));
        nxserver_ipc_close_client(client);
    }
}

static void nxserverlib_p_stop_crc_capture(nxclient_t client)
{
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        if (client->session->display[i].crc_client == client) {
            NEXUS_DisplaySettings settings;
            NEXUS_Display_GetSettings(client->session->display[i].display, &settings);
            settings.crcQueueSize = 0;
            (void)NEXUS_Display_SetSettings(client->session->display[i].display, &settings);
            client->session->display[i].crc_client = NULL;
        }
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (client->session->hdmiOutput_crc_client == client) {
        NEXUS_HdmiOutputSettings settings;
        NEXUS_HdmiOutput_GetSettings(client->session->hdmiOutput, &settings);
        settings.crcQueueSize = 0;
        (void)NEXUS_HdmiOutput_SetSettings(client->session->hdmiOutput, &settings);
        client->session->hdmiOutput_crc_client = NULL;
    }
#endif
}

void NxClient_P_DestroyClient(nxclient_t client)
{
    struct b_req *req;
    struct b_connect *connect;
    nxserver_t server;
    struct b_client_standby_ack *ack;

    BDBG_OBJECT_ASSERT(client, b_client);

    server = client->server;

    if (client->watchdog.timeout) {
        nxserver_p_immediate_watchdog(client);
    }

    if (server->standby.standbySettings.settings.mode != NEXUS_PlatformStandbyMode_eOn) {
        client->zombie = true;
        return;
    }
    if (client->joinSettings.name[0]) {
        BDBG_LOG(("%s(%p) unregistered", client->joinSettings.name, (void*)client));
    }
    BDBG_MSG(("NxClient_P_DestroyClient %p", (void*)client));

#if NEXUS_HAS_HDMI_INPUT
    if (client == client->session->hdmi.repeater.client) {
        NEXUS_HdmiOutput_SetRepeaterInput(client->session->hdmiOutput, NULL);
        client->session->hdmi.repeater.client = NULL;
        client->session->hdmiInput = NULL;
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    if (client->hdcp_level != NxClient_HdcpLevel_eNone) {
        client->hdcp_level = NxClient_HdcpLevel_eNone;
        nxserver_check_hdcp(client->session);
    }
#endif
    nxserverlib_p_stop_crc_capture(client);

    /* unregister client first so that decoder is stopped before destroying surfacecmp client to avoid flash of video */
    if (client->nexusClient) {
        if (server->settings.client.disconnect) {
            server->settings.client.disconnect(client, &client->joinSettings);
        }
        NEXUS_Platform_UnregisterClient(client->nexusClient);
        client->nexusClient = NULL;
    }

    while ((connect = BLST_D_FIRST(&client->connects))) {
        b_disconnect(client, connect);
    }

    while ((req = BLST_D_FIRST(&client->requests))) {
        b_release_request(client, req);
    }

    BLST_D_REMOVE(&server->clients, client, link);
    BLST_D_REMOVE(&client->session->clients, client, session_link);
    if (!BLST_D_FIRST(&client->session->clients) && !IS_SESSION_DISPLAY_ENABLED(server->settings.session[client->session->index])) {
        /* encode-only sessions are dynamically created and destroyed, unless they are session0 */
        if (client->session->index) {
            uninit_session(client->session);
        }
    }
    while ((ack = BLST_S_FIRST(&client->standby.acks))) {
        BLST_S_REMOVE_HEAD(&client->standby.acks, link);
        BKNI_Free(ack);
    }
    if (client->server->settings.growHeapBlockSize) {
        NEXUS_Platform_ShrinkHeap(client->server->settings.client.heap[NXCLIENT_DYNAMIC_HEAP], client->server->settings.growHeapBlockSize, client->server->settings.growHeapBlockSize);
    }
    BDBG_OBJECT_DESTROY(client, b_client);
    BKNI_Free(client);

    return;
}

NEXUS_Error NxClient_P_Alloc(nxclient_t client, const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults)
{
    nxserver_t server = client->server;
    unsigned i;
    struct b_req *req;
    int rc;

    req = BKNI_Malloc(sizeof(*req));
    if (!req) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(req, 0, sizeof(*req));
    BDBG_OBJECT_SET(req, b_req);
    BLST_D_INSERT_HEAD(&client->requests, req, link);
    req->client = client;
    req->settings = *pSettings;
    BKNI_Memset(pResults, 0, sizeof(*pResults));

    for (i=0;i<pSettings->surfaceClient && i<NXCLIENT_MAX_IDS;i++) {
        if (!client->session->surfaceCompositor) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }

        req->handles.surfaceClient[i].handle = NEXUS_SurfaceCompositor_CreateClient(client->session->surfaceCompositor, server->nextId[b_resource_surface_client]);
        if (!req->handles.surfaceClient[i].handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->surfaceClient[i].id = req->handles.surfaceClient[i].id = server->nextId[b_resource_surface_client];
        inc_id(server, b_resource_surface_client);

        b_grant_resource_id(client, b_resource_surface_client, pResults->surfaceClient[i].id);
    }
    if (i) BDBG_MSG(("  %u surfaceClient", i));
#if NEXUS_HAS_SIMPLE_DECODER
    for (i=0;i<pSettings->simpleAudioPlayback && i<NXCLIENT_MAX_IDS;i++) {
        req->handles.simpleAudioPlayback[i].handle = NEXUS_SimpleAudioPlayback_Create(client->session->audio.playbackServer, server->nextId[b_resource_simple_audio_playback], NULL);
        if (!req->handles.simpleAudioPlayback[i].handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->simpleAudioPlayback[i].id = req->handles.simpleAudioPlayback[i].id = server->nextId[b_resource_simple_audio_playback];
        inc_id(server, b_resource_simple_audio_playback);
    }
    if (i) BDBG_MSG(("  %u simpleAudioPlayback", i));

    if (pSettings->simpleAudioDecoder) {
        NEXUS_SimpleAudioDecoderServerSettings settings;

        if (req->handles.simpleAudioDecoder.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }

        NEXUS_SimpleAudioDecoder_GetDefaultServerSettings(&settings);
        /* Set type to eMax here. Connect will set a valid type */
        settings.type = NEXUS_SimpleAudioDecoderType_eMax;
        settings.simplePlayback = client->session->audio.playbackServer;
        req->handles.simpleAudioDecoder.handle = NEXUS_SimpleAudioDecoder_Create(client->session->audio.server, server->nextId[b_resource_simple_audio_decoder], &settings);
        if (!req->handles.simpleAudioDecoder.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->simpleAudioDecoder.id = req->handles.simpleAudioDecoder.id = server->nextId[b_resource_simple_audio_decoder];
        inc_id(server, b_resource_simple_audio_decoder);
        b_grant_resource_id(client, b_resource_simple_audio_decoder, pResults->simpleAudioDecoder.id);
        BDBG_MSG(("  1 simpleAudioDecoder"));
    }

    for (i=0;i<pSettings->simpleVideoDecoder && i<NXCLIENT_MAX_IDS;i++) {
        NEXUS_SimpleVideoDecoderServerSettings settings;
        NEXUS_SimpleVideoDecoder_GetDefaultServerSettings(&settings);
        req->handles.simpleVideoDecoder[i].handle = NEXUS_SimpleVideoDecoder_Create(client->session->video.server, server->nextId[b_resource_simple_video_decoder], &settings);
        if (!req->handles.simpleVideoDecoder[i].handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->simpleVideoDecoder[i].id = req->handles.simpleVideoDecoder[i].id = server->nextId[b_resource_simple_video_decoder];
        inc_id(server, b_resource_simple_video_decoder);
        BDBG_ASSERT(!req->handles.simpleVideoDecoder[i].r);
        b_grant_resource_id(client, b_resource_simple_video_decoder, pResults->simpleVideoDecoder[i].id);
    }
    if (i) BDBG_MSG(("  %u simpleVideoDecoder", i));
#endif
#if NEXUS_HAS_INPUT_ROUTER
    for (i=0;i<pSettings->inputClient && i<NXCLIENT_MAX_IDS;i++) {
        if (client->session->input.router) {
            req->handles.inputClient[i].handle = NEXUS_InputRouter_CreateClient(client->session->input.router, server->nextId[b_resource_input_client]);
        }
        if (!req->handles.inputClient[i].handle) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        pResults->inputClient[i].id = req->handles.inputClient[i].id = server->nextId[b_resource_input_client];
        inc_id(server, b_resource_input_client);
        b_grant_resource_id(client, b_resource_input_client, pResults->inputClient[i].id);
    }
    if (i) BDBG_MSG(("  %u inputClient", i));
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    for (i=0;i<pSettings->simpleEncoder && i<NXCLIENT_MAX_IDS;i++) {
        req->handles.simpleEncoder[i].handle = NEXUS_SimpleEncoder_Create(client->session->encoder.server, server->nextId[b_resource_simple_encoder]);
        if (!req->handles.simpleEncoder[i].handle) {
            return BERR_TRACE(NEXUS_NOT_AVAILABLE);
        }
        pResults->simpleEncoder[i].id = req->handles.simpleEncoder[i].id = server->nextId[b_resource_simple_encoder];
        inc_id(server, b_resource_simple_encoder);
        b_grant_resource_id(client, b_resource_simple_encoder, pResults->simpleEncoder[i].id);
    }
    if (i) BDBG_MSG(("  %u simpleEncoder", i));
#endif
#if NEXUS_HAS_AUDIO
    if (pSettings->audioCapture) {
        if (req->handles.audioCapture.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        req->handles.audioCapture.handle = nxserverlib_open_audio_capture(client->session, &req->handles.audioCapture.id, pSettings->audioCaptureType.type);
        if (!req->handles.audioCapture.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->audioCapture.id = req->handles.audioCapture.id + NEXUS_ALIAS_ID;
        b_grant_resource_id(client, b_resource_audio_capture, pResults->audioCapture.id);
    }
    if (i) BDBG_MSG(("  %u audioCapture", i));

    if (pSettings->audioCrc) {
        if (req->handles.audioCrc.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        req->handles.audioCrc.handle = nxserverlib_open_audio_crc(client->session, &req->handles.audioCrc.id, pSettings->audioCrcType.type);
        if (!req->handles.audioCrc.handle) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        pResults->audioCrc.id = req->handles.audioCrc.id + NEXUS_ALIAS_ID;
        b_grant_resource_id(client, b_resource_audio_crc, pResults->audioCrc.id);
    }
    if (i) BDBG_MSG(("  %u audioCrc", i));

#endif

    req->results = *pResults;
    BDBG_MSG(("NxClient_P_Alloc req %p, client %p", (void*)req, (void*)client));
    if (client->server->settings.client.nxclient_alloc && client->ipc) {
        client->server->settings.client.nxclient_alloc(client, pSettings, pResults);
    }

    return 0;

error:
    b_release_request(client, req);
    return rc;
}

void NxClient_P_Free(nxclient_t client, const NxClient_AllocResults *pResults)
{
    struct b_req *req;
    if (client->server->settings.client.nxclient_free && client->ipc) {
        client->server->settings.client.nxclient_free(client, pResults);
    }
    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        BDBG_OBJECT_ASSERT(req, b_req);
        if (!BKNI_Memcmp(&req->results, pResults, sizeof(*pResults))) {
            BDBG_MSG(("NxClient_P_Free client %p, req %p", (void*)client, (void*)req));
            b_release_request(client, req);
            break;
        }
    }
}

unsigned get_videodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i) { return pSettings->simpleVideoDecoder[i].id == NXCLIENT_CONNECT_BACKEND_MOSAIC ? 0 : pSettings->simpleVideoDecoder[i].id; }
unsigned get_audiodecoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i) { return i==0?pSettings->simpleAudioDecoder.id:0; }
unsigned get_audioplayback_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i) { return pSettings->simpleAudioPlayback[i].id; }
unsigned get_encoder_connect_id(const NxClient_ConnectSettings *pSettings, unsigned i) { return pSettings->simpleEncoder[i].id; }

typedef unsigned (*get_alloc_results_id_func)(const NxClient_AllocResults *pResults, unsigned i);
static unsigned get_videodecoder_alloc_results_id(const NxClient_AllocResults *pResults, unsigned i) { return pResults->simpleVideoDecoder[i].id; }
static unsigned get_audiodecoder_alloc_results_id(const NxClient_AllocResults *pResults, unsigned i) { BSTD_UNUSED(i); return pResults->simpleAudioDecoder.id; }
static unsigned get_audioplayback_alloc_results_id(const NxClient_AllocResults *pResults, unsigned i) { return pResults->simpleAudioPlayback[i].id; }
static unsigned get_encoder_alloc_results_id(const NxClient_AllocResults *pResults, unsigned i) { return pResults->simpleEncoder[i].id; }

int get_req_index(struct b_req *req, get_req_id_func func, unsigned target_id, unsigned *pIndex)
{
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        unsigned id = func(req, i);
        if (!id) break;
        if (id == target_id) {
            *pIndex = i;
            return 0;
        }
    }
    return -1;
}

bool is_transcode_connect(const struct b_connect *connect)
{
    return connect->settings.simpleVideoDecoder[0].windowCapabilities.encoder || connect->settings.simpleAudioDecoder.decoderCapabilities.encoder;
}

bool is_video_request(const struct b_connect *connect)
{
    return connect->settings.simpleVideoDecoder[0].id;
}

static NEXUS_Error nxclient_get_surfaceclient_req(nxclient_t client, unsigned surfaceClientId, struct b_req **out_req, unsigned *out_i)
{
    struct b_req *req;
    if (!surfaceClientId) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (req->results.surfaceClient[i].id == surfaceClientId) {
                *out_req = req;
                *out_i = i;
                return 0;
            }
            else if (!req->results.surfaceClient[i].id) {
                i = NXCLIENT_MAX_IDS;
            }
        }
    }
    return BERR_TRACE(NEXUS_INVALID_PARAMETER);
}

/* release resources from a connect */
void b_connect_release(nxclient_t client, struct b_connect *connect)
{
    struct b_connect *search_connect=NULL;
    nxclient_t search_client;

    BDBG_OBJECT_ASSERT(connect, b_connect);
    if (is_video_request(connect)) {
        release_video_decoders(connect);

        /* find first video decoder connect which we can satisfy and transfer video to it */
        for (search_client = BLST_D_FIRST(&client->server->clients); search_client; search_client = BLST_D_NEXT(search_client, link)) {
            for (search_connect = BLST_D_FIRST(&search_client->connects); search_connect; search_connect = BLST_D_NEXT(search_connect, link)) {
                if (search_connect != connect && lacks_video(search_connect)) break;
            }
            if (search_connect) {
                acquire_video_decoders(search_connect, false);
            }
        }
    }

    if (has_audio(connect)) {
        /* find decoder request to transfer audio to */
        for (search_client = BLST_D_FIRST(&client->server->clients); search_client; search_client = BLST_D_NEXT(search_client, link)) {
            for (search_connect = BLST_D_FIRST(&search_client->connects); search_connect; search_connect = BLST_D_NEXT(search_connect, link)) {
                if (search_connect != connect && lacks_audio(search_connect) && connect->client->session == search_connect->client->session) break;
            }
            if (search_connect) break;
        }
        if (search_connect) {
            (void)acquire_audio_decoders(search_connect, true);
            /* if acquire fails, we must still release. if acquire succeeds, release is no-op. */
        }
        release_audio_decoders(connect);
    }
    else {
        release_audio_decoders(connect);
    }

    release_audio_playbacks(connect);

    release_video_encoders(connect);
}

static void b_disconnect(nxclient_t client, struct b_connect *connect)
{
    BDBG_OBJECT_ASSERT(connect, b_connect);
    BDBG_MSG(("b_disconnect %p", (void*)connect));
    b_connect_release(client, connect);
    BLST_D_REMOVE(&client->connects, connect, link);
    BDBG_OBJECT_DESTROY(connect, b_connect);
    BKNI_Free(connect);
}

static bool find_matching_req_aux(const char *name, const NxClient_ConnectSettings *pSettings, const NxClient_AllocResults *pResults,
    get_connect_id_func get_connect_id, get_alloc_results_id_func get_alloc_results_id)
{
    unsigned i;
    BSTD_UNUSED(name);
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        unsigned j;
        unsigned id = get_connect_id(pSettings, i);
        if (!id) break; /* assuming arrays are packed: we can break if we hit id == 0 */
        BDBG_MSG_TRACE(("  testing %s[%u].%u", name, i, id));
        for (j=0;j<NXCLIENT_MAX_IDS;j++) { /* don't assume same order. must search. */
            unsigned results_id = get_alloc_results_id(pResults, j);
            if (results_id == 0) {
                j = NXCLIENT_MAX_IDS;
                break;
            }
            else if (results_id == id) {
                /* found */
                break;
            }
        }
        if (j == NXCLIENT_MAX_IDS) {
            return false;
        }
    }
    return true;
}

/*
find client b_req for each b_resource type in NxClient_ConnectSettings
*/
static int find_matching_reqs(struct b_connect *connect, const NxClient_ConnectSettings *pSettings)
{
    int rc = 0;
    struct b_req *req;
    unsigned i;
    static const struct {
        enum b_resource resource;
        get_connect_id_func get_connect_id;
        get_alloc_results_id_func get_alloc_results_id;
    } accessor[] = {
        {b_resource_simple_video_decoder, get_videodecoder_connect_id, get_videodecoder_alloc_results_id},
        {b_resource_simple_audio_decoder, get_audiodecoder_connect_id, get_audiodecoder_alloc_results_id},
        {b_resource_simple_audio_playback, get_audioplayback_connect_id, get_audioplayback_alloc_results_id},
        {b_resource_simple_encoder, get_encoder_connect_id, get_encoder_alloc_results_id}
    };
    for (req = BLST_D_FIRST(&connect->client->requests); req; req = BLST_D_NEXT(req, link)) {
        BDBG_MSG_TRACE(("find_matching_req: req %p", req));
        for (i=0;i<sizeof(accessor)/sizeof(accessor[0]);i++) {
            unsigned res = accessor[i].resource;
            if (!connect->req[res] && accessor[i].get_connect_id(pSettings,0)) {
                if (find_matching_req_aux(b_resource_str[res], pSettings, &req->results, accessor[i].get_connect_id, accessor[i].get_alloc_results_id)) {
                    connect->req[res] = req;
                }
            }
        }
    }
    for (i=0;i<sizeof(accessor)/sizeof(accessor[0]);i++) {
        unsigned res = accessor[i].resource;
        if (connect->req[res]) {
            BDBG_MSG(("  found req %p for %s", (void*)connect->req[res], b_resource_str[res]));
        }
        else if (accessor[i].get_connect_id(pSettings,0)) {
            /* no need to unwind. the connect will be destroyed. also, don't short circuit so we can get full output */
            BDBG_WRN(("  unable to find req for %s", b_resource_str[res]));
            rc = NEXUS_NOT_AVAILABLE;
        }
    }
    return rc;
}

/* acquires resources for a connect */
int b_connect_acquire(nxclient_t client, struct b_connect *connect)
{
    int rc;
    bool transcode;

    if (client->server->settings.externalApp.enabled && !client->server->externalApp.allow_decode) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }

    BDBG_OBJECT_ASSERT(connect, b_connect);
    BDBG_ASSERT(connect->client == client);
    transcode = is_transcode_connect(connect);

    rc = acquire_video_decoders(connect, !transcode);
    if (rc) {
        BDBG_WRN(("no video decoders available"));
        return rc;
    }

    rc = acquire_audio_decoders(connect, false);
    if (rc) {
        BDBG_WRN(("no audio decoders available"));
        /* TODO: for now, audio is only required for session 0 */
        if (client->session->index == 0) {
            return rc;
        }
    }

    /* video_encoder must be acquired after audio_decoder to link mixer */
    rc = acquire_video_encoders(connect);
    if (rc) {
        BDBG_WRN(("no video encoders available"));
        return rc;
    }

    rc = acquire_audio_playbacks(connect);
    if (rc) {
        BDBG_WRN(("no audio playbacks available"));
        return rc;
    }

    return 0;
}

static unsigned nxserver_p_alloc_connect_id(nxserver_t server)
{
    static unsigned g_id = 0;
    unsigned start_id = g_id;
    nxclient_t client;
    do {
        if (++g_id == 0) g_id = 1;
        if (g_id == start_id) { BERR_TRACE(NEXUS_NOT_AVAILABLE); return 0; }
        for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
            struct b_connect *connect;
            for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
                if (connect->id == g_id) break;
            }
            if (connect) break;
        }
    } while (client);
    return g_id;
}

NEXUS_Error NxClient_P_Connect(nxclient_t client, const NxClient_ConnectSettings *pSettings, unsigned *pConnectId)
{
    int rc;
    struct b_connect *connect;

    connect = BKNI_Malloc(sizeof(*connect));
    if (!connect) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(connect, 0, sizeof(*connect));
    BDBG_OBJECT_SET(connect, b_connect);
    BLST_D_INSERT_HEAD(&client->connects, connect, link);
    BDBG_MSG(("NxClient_P_Connect %p, client %p", (void*)connect, (void*)client));

    connect->settings = *pSettings;
    /* adjust settings for backward compat */
    if (connect->settings.simpleVideoDecoder[0].windowCapabilities.maxWidth == 0 ||
        connect->settings.simpleVideoDecoder[0].windowCapabilities.maxHeight == 0)
    {
        connect->settings.simpleVideoDecoder[0].windowCapabilities.type = NxClient_VideoWindowType_eNone;
    }
    connect->client = client;
    *pConnectId = connect->id = nxserver_p_alloc_connect_id(client->server);

    rc = find_matching_reqs(connect, pSettings);
    if (rc) {
        BDBG_WRN(("NxClient_Connect failed: matching NxClient_Alloc not found"));
        goto error;
    }

    rc = b_connect_acquire(client, connect);
    if (rc) {
        BDBG_WRN(("NxClient_Connect failed: resources not available (%d)", rc));
        goto error;
    }
    if (client->server->settings.client.nxclient_connect && client->ipc) {
        client->server->settings.client.nxclient_connect(client, pSettings, *pConnectId);
    }

    return 0;

error:
    b_disconnect(client, connect);
    return rc;
}

static struct b_connect *lookup_connect(nxclient_t client, unsigned connectId)
{
    struct b_connect *connect;
    for (connect = BLST_D_FIRST(&client->connects); connect; connect = BLST_D_NEXT(connect, link)) {
        if (connect->id == connectId) {
            return connect;
        }
    }
    return NULL;
}

NEXUS_Error NxClient_P_RefreshConnect(nxclient_t client, unsigned connectId)
{
    struct b_connect *connect = lookup_connect(client, connectId);
    if (connect) {
        int rc;
        BDBG_MSG(("NxClient_P_RefreshConnect %p", (void*)connect));
        rc = b_connect_acquire(client, connect);
        if (rc) {
            BDBG_WRN(("NxClient_RefreshConnect failed: resources not available (%d)", rc));
        }
        return rc;
    }
    return -1;
}

void NxClient_P_Disconnect(nxclient_t client, unsigned connectId)
{
    struct b_connect *connect = lookup_connect(client, connectId);
    if (connect) {
        BDBG_MSG(("NxClient_P_Disconnect %p", (void*)connect));
        if (client->server->settings.client.nxclient_disconnect && client->ipc) {
            client->server->settings.client.nxclient_disconnect(client, connectId);
        }
        b_disconnect(client, connect);
    }
}

void NxClient_P_GetSurfaceClientComposition(nxclient_t client, unsigned surfaceClientId, NEXUS_SurfaceComposition *pComposition)
{
    NEXUS_Error rc;
    struct b_req *req;
    unsigned i;

    BDBG_OBJECT_ASSERT(client, b_client);

    rc = nxclient_get_surfaceclient_req(client, surfaceClientId, &req, &i);
    if (rc) {
        BKNI_Memset(pComposition, 0, sizeof(*pComposition));
    }
    else {
        NEXUS_SurfaceCompositorClientSettings client_settings;
        NEXUS_SurfaceCompositor_GetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[i].handle, &client_settings);
        *pComposition = client_settings.composition;
    }
}

NEXUS_Error NxClient_P_SetSurfaceClientComposition(nxclient_t client, unsigned surfaceClientId, const NEXUS_SurfaceComposition *pComposition)
{
    NEXUS_SurfaceCompositorClientSettings client_settings;
    NEXUS_Error rc;
    struct b_req *req;
    unsigned i;
    NEXUS_SurfaceComposition copy;

    if (client->server->settings.client.nxclient_setSurfaceComposition && client->ipc) {
        /* allow callback to change */
        copy = *pComposition;
        pComposition = &copy;
        if (client->server->settings.client.nxclient_setSurfaceComposition(client, surfaceClientId, &copy)) {
            return 0;
        }
    }

    rc = nxclient_get_surfaceclient_req(client, surfaceClientId, &req, &i);
    if (rc) return BERR_TRACE(rc);

    BDBG_OBJECT_ASSERT(client, b_client);
    NEXUS_SurfaceCompositor_GetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[i].handle, &client_settings);
    client_settings.composition = *pComposition;
    rc = NEXUS_SurfaceCompositor_SetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[i].handle, &client_settings);
    if (rc) return BERR_TRACE(rc);

    return 0;
}

int nxserverlib_set_server_alpha(nxclient_t client, unsigned alpha)
{
    NEXUS_SurfaceCompositorClientSettings client_settings;
    NEXUS_Error rc;
    struct b_req *req;

    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (!req->results.surfaceClient[i].id) continue;

            BDBG_OBJECT_ASSERT(client, b_client);
            NEXUS_SurfaceCompositor_GetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[i].handle, &client_settings);
            /* this assumes the client isn't using colorMatrix */
            client_settings.composition.colorMatrixEnabled = alpha < 0xFF;
            if (client_settings.composition.colorMatrixEnabled) {
                NEXUS_Graphics2DColorMatrix *pMatrix = &client_settings.composition.colorMatrix;
                BKNI_Memset(pMatrix, 0, sizeof(*pMatrix));
                pMatrix->shift = 8; /* 2^8 == 256. this causes alpha to be 0%...100%. */
                pMatrix->coeffMatrix[0] =
                pMatrix->coeffMatrix[6] =
                pMatrix->coeffMatrix[12] = 0xff; /* don't modify color */
                pMatrix->coeffMatrix[18] = alpha; /* reduce by server-side alpha */
            }
            rc = NEXUS_SurfaceCompositor_SetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[i].handle, &client_settings);
            if (rc) return BERR_TRACE(rc);
        }
    }

    return 0;
}

static NEXUS_DisplayHandle b_get_vbi_display(struct b_session *session)
{
    return session->display[1].display ? session->display[1].display : session->display[0].display;
}

static int nxserverlib_p_reconfig(nxclient_t client, const NxClient_ReconfigSettings *pSettings)
{
    unsigned i;
    int rc;
    for (i=0;i<sizeof(pSettings->command)/sizeof(pSettings->command[0]);i++) {
        struct b_connect *connect1;
        struct b_connect *connect2;
        if (!pSettings->command[i].connectId1 && !pSettings->command[i].connectId2 && i > 0) break;
        connect1 = lookup_connect(client, pSettings->command[i].connectId1);
        connect2 = lookup_connect(client, pSettings->command[i].connectId2);
        if (!connect1 || !connect2) return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        switch (pSettings->command[i].type) {
        case NxClient_ReconfigType_eRerouteVideo:
            rc = nxserverlib_p_swap_video_windows(connect1, connect2);
            if (rc) return BERR_TRACE(rc);
            break;
        case NxClient_ReconfigType_eRerouteVideoAndAudio:
            rc = nxserverlib_p_swap_video_windows(connect1, connect2);
            if (rc) return BERR_TRACE(rc);
            rc = nxserverlib_p_swap_audio(connect1, connect2);
            if (rc) return BERR_TRACE(rc);
            break;
        case NxClient_ReconfigType_eRerouteAudio:
            rc = nxserverlib_p_swap_audio(connect1, connect2);
            if (rc) return BERR_TRACE(rc);
            break;
        default:
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }

    return 0;
}

static void nxserver_p_polling_checkpoint(NEXUS_Graphics2DHandle gfx)
{
    while (1) {
        int rc = NEXUS_Graphics2D_Checkpoint(gfx, NULL);
        if (rc != NEXUS_GRAPHICS2D_BUSY) {
            BERR_TRACE(rc);
            break;
        }
        BKNI_Sleep(1);
    }
}

#if NEXUS_HAS_VIDEO_DECODER
static void nxserver_p_scale_rect(unsigned old_w, unsigned old_h, const NEXUS_Rect *pOldRect,
    unsigned new_w, unsigned new_h, NEXUS_Rect *pNewRect)
{
    if (!old_w || !old_h) {
        BKNI_Memset(pNewRect, 0, sizeof(*pNewRect));
    }
    else {
        pNewRect->width = pOldRect->width * new_w / old_w;
        pNewRect->height = pOldRect->height * new_h / old_h;
        pNewRect->x = pOldRect->x * new_w / old_w;
        pNewRect->y = pOldRect->y * new_h / old_h;
    }
}
#endif

static int nxserverlib_p_screenshot(nxclient_t client, const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface)
{
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings graphics2DSettings;
    NEXUS_Graphics2DFillSettings fillSettings;
    NEXUS_SurfaceCreateSettings surfaceCreateSettings;
    int rc;
    unsigned i;

    BSTD_UNUSED(pSettings);

    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    if (!gfx) BERR_TRACE(NEXUS_NOT_AVAILABLE);
    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.pollingCheckpoint = true;
    NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);

    NEXUS_Surface_GetCreateSettings(surface, &surfaceCreateSettings);

    /* fill black, blit video, then blend graphics */
    NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
    fillSettings.surface = surface;
    fillSettings.color = 0;
    rc = NEXUS_Graphics2D_Fill(gfx, &fillSettings);
    if (rc) {rc = BERR_TRACE(rc); goto done;}

#if NEXUS_HAS_VIDEO_DECODER
    for (i=0;i<NEXUS_NUM_VIDEO_WINDOWS;i++) {
        if (client->session->window[i].connect) {
            /* TODO: mosaic */
            NEXUS_VideoDecoderHandle videoDecoder = nxserver_p_get_video_decoder(client->session->window[i].connect);
            NEXUS_VideoWindowHandle window = client->session->display[0].window[i][0];
            NEXUS_VideoWindowSettings windowSettings;
            NEXUS_VideoDecoderFrameStatus frameStatus;
            NEXUS_StripedSurfaceHandle stripedSurface;
            unsigned num;

            if (!videoDecoder || !window) continue;
            rc = NEXUS_VideoDecoder_GetDecodedFrames(videoDecoder, &frameStatus, 1, &num);
            if (rc || !num) continue;

            stripedSurface = NEXUS_StripedSurface_Create(&frameStatus.surfaceCreateSettings);
            if (stripedSurface) {
                NEXUS_Rect rect;
                NEXUS_VideoWindow_GetSettings(window, &windowSettings);
                nxserver_p_scale_rect(
                    client->session->display[0].formatInfo.width, client->session->display[0].formatInfo.height,
                    &windowSettings.position,
                    surfaceCreateSettings.width, surfaceCreateSettings.height,
                    &rect);
                NEXUS_Graphics2D_DestripeToSurface(gfx, stripedSurface, surface, &rect);
                BDBG_MSG_TRACE(("screenshot window%d: %d,%d,%d,%d", i, rect.x, rect.y, rect.width, rect.height));

                nxserver_p_polling_checkpoint(gfx);
                NEXUS_StripedSurface_Destroy(stripedSurface);
            }
            NEXUS_VideoDecoder_ReturnDecodedFrames(videoDecoder, NULL, 1);

        }
    }
#else
    BSTD_UNUSED(i);
#endif
    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    rc = NEXUS_SurfaceCompositor_GetCurrentFramebuffer(client->session->surfaceCompositor, &blitSettings.source.surface);
    if (rc) {rc = BERR_TRACE(rc); goto done;}
    if (!blitSettings.source.surface) {
        rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
        goto done;
    }
    blitSettings.source.rect.x = 0;
    blitSettings.source.rect.y = 0;
    NEXUS_Surface_GetCreateSettings(blitSettings.source.surface, &surfaceCreateSettings);
    blitSettings.source.rect.width = MIN(surfaceCreateSettings.width, client->session->display[0].formatInfo.width);
    blitSettings.source.rect.height = MIN(surfaceCreateSettings.height, client->session->display[0].formatInfo.height);
    blitSettings.dest.surface = surface;
    blitSettings.output.surface = surface;
    blitSettings.constantColor = 0xFF << 24;
    /* assume framebuffer has alpha per pixel to blend on top of video */
    blitSettings.colorOp = NEXUS_BlitColorOp_eUseSourceAlpha;
    blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopyConstant;

    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    if (rc) {rc = BERR_TRACE(rc); goto done;}
    nxserver_p_polling_checkpoint(gfx);

done:
    NEXUS_Graphics2D_Close(gfx);
    return rc;
}

static NEXUS_Error nxserverlib_p_set_macrovision(struct b_session *session, NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable)
{
    unsigned i;
    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        int rc;
        if (!session->display[i].display) continue;
        rc = NEXUS_Display_SetMacrovision(session->display[i].display, type, pTable);
        if (rc) return BERR_TRACE(rc);
    }
    return 0;
}

static NEXUS_ClientHandle nxserverlib_p_lookup_client(nxserver_t server, unsigned pid)
{
    nxclient_t client;
    for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
        if (client->pid == pid) {
            return client->nexusClient;
        }
    }
    return NULL;
}

static bool is_trusted_client(nxclient_t client)
{
    return !client || client->clientSettings.configuration.mode != NEXUS_ClientMode_eUntrusted;
}

static NEXUS_Error nxserver_get_status(nxclient_t client, NxClient_Status *pStatus)
{
    nxserver_t server = client->session->server;
    BKNI_Memset(pStatus, 0, sizeof(*pStatus));
    if (!server->settings.externalApp.enabled) {
        pStatus->externalAppState = NxClientExternalAppState_eNone;
    }
    else {
        pStatus->externalAppState = server->externalApp.allow_decode?NxClientExternalAppState_eDecode:NxClientExternalAppState_eGraphicsOnly;
    }
    return NEXUS_SUCCESS;
}

static int nxserver_set_slave_display_graphics(nxclient_t client, unsigned slaveDisplay, NEXUS_SurfaceHandle surface)
{
    struct b_session *session = client->session;
    NEXUS_SurfaceCompositorStatus status;
    NEXUS_Graphics2DHandle gfx;
    NEXUS_Graphics2DSettings graphics2DSettings;
    NEXUS_Graphics2DBlitSettings blitSettings;
    NEXUS_GraphicsSettings graphicsSettings;
    int rc;
    NEXUS_InterfaceName interfaceName;

    if (slaveDisplay >= NXCLIENT_MAX_DISPLAYS-1) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (!session->display[slaveDisplay+1].display) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (session->nxclient.displaySettings.slaveDisplay[slaveDisplay].mode != NxClient_SlaveDisplayMode_eGraphics) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    rc = NEXUS_SurfaceCompositor_GetStatus(session->surfaceCompositor, &status);
    if (rc) return BERR_TRACE(rc);

    if (!session->display[slaveDisplay+1].graphic) {
        NEXUS_SurfaceCreateSettings createSettings;
        NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
        createSettings.width = 720;
        createSettings.height = 480;
        createSettings.heap = NEXUS_Platform_GetFramebufferHeap(slaveDisplay+1);
        /* memory is freed on session uninit */
        session->display[slaveDisplay+1].graphic = NEXUS_Surface_Create(&createSettings);
        if (!session->display[slaveDisplay+1].graphic) {
            return BERR_TRACE(NEXUS_OUT_OF_DEVICE_MEMORY);
        }
    }

    /* copy surface */
    gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
    if (!gfx) BERR_TRACE(NEXUS_NOT_AVAILABLE);
    NEXUS_Graphics2D_GetSettings(gfx, &graphics2DSettings);
    graphics2DSettings.pollingCheckpoint = true;
    rc = NEXUS_Graphics2D_SetSettings(gfx, &graphics2DSettings);
    if (rc) {BERR_TRACE(rc); goto err_gfxsettings;}

    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_Surface");
    rc = NEXUS_Platform_AcquireObject(client->nexusClient, &interfaceName, surface);
    if (rc) {BERR_TRACE(rc); goto err_acquire;}

    NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
    blitSettings.source.surface = surface;
    /* TODO: double buffering */
    blitSettings.output.surface = session->display[slaveDisplay+1].graphic;
    rc = NEXUS_Graphics2D_Blit(gfx, &blitSettings);
    if (rc) {BERR_TRACE(rc); goto err_blit;}
    nxserver_p_polling_checkpoint(gfx);

    NEXUS_Display_GetGraphicsSettings(session->display[slaveDisplay+1].display, &graphicsSettings);
    graphicsSettings.enabled = true;
    rc = NEXUS_Display_SetGraphicsSettings(session->display[slaveDisplay+1].display, &graphicsSettings);
    if (rc) {BERR_TRACE(rc); goto err_fb;}
    rc = NEXUS_Display_SetGraphicsFramebuffer(session->display[slaveDisplay+1].display, blitSettings.output.surface);
    if (rc) {BERR_TRACE(rc); goto err_fb;}

err_fb:
err_blit:
    NEXUS_Platform_ReleaseObject(&interfaceName, surface);
err_acquire:
err_gfxsettings:
    NEXUS_Graphics2D_Close(gfx);
    return rc;
}

NEXUS_Error NxClient_P_WriteTeletext(nxclient_t client, const nxclient_p_teletext_data *data, size_t numLines, size_t *pNumLinesWritten)
{
    NEXUS_DisplayVbiSettings settings;
    NEXUS_Error rc;
    NEXUS_DisplayHandle display = b_get_vbi_display(client->session);

    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }

    NEXUS_Display_GetVbiSettings(display, &settings);
    if (!settings.teletextEnabled) {
        settings.teletextEnabled = true;
        rc = NEXUS_Display_SetVbiSettings(display, &settings);
        if (rc) return BERR_TRACE(rc);
    }
    return NEXUS_Display_WriteTeletext(display, data->lines, numLines, pNumLinesWritten);
}

NEXUS_Error NxClient_P_WriteClosedCaption(nxclient_t client, const nxclient_p_closecaption_data *data, size_t numEntries, size_t *pNumEntriesWritten )
{
    NEXUS_DisplayVbiSettings settings;
    NEXUS_Error rc;
    NEXUS_DisplayHandle display = b_get_vbi_display(client->session);

    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }

    NEXUS_Display_GetVbiSettings(display, &settings);
    if (!settings.closedCaptionEnabled) {
        settings.closedCaptionEnabled = true;
        rc = NEXUS_Display_SetVbiSettings(display, &settings);
        if (rc) BERR_TRACE(rc); /* keep going */
    }
    return NEXUS_Display_WriteClosedCaption(display, data->entries, numEntries, pNumEntriesWritten);
}

NEXUS_Error NxClient_P_Display_SetWss(nxclient_t client, uint16_t wssData)
{
    unsigned i;

    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }

    /* wss to all displays with SD format */
    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        NEXUS_DisplayHandle display = client->session->display[i].display;
        bool isSd = client->session->display[i].formatInfo.height <= 576 && client->session->display[i].formatInfo.interlaced;
        bool is576p = client->session->display[i].formatInfo.height == 576 && !client->session->display[i].formatInfo.interlaced;
        if (display && (isSd || is576p)) {
            NEXUS_Error rc;
            NEXUS_DisplayVbiSettings settings;
            NEXUS_Display_GetVbiSettings(display, &settings);
            if (!settings.wssEnabled) {
                settings.wssEnabled = true;
                rc = NEXUS_Display_SetVbiSettings(display, &settings);
                if (rc) BERR_TRACE(rc); /* keep going */
            }
            rc = NEXUS_Display_SetWss(display, wssData);
            if (rc) return BERR_TRACE(rc);
        }
    }
    return NEXUS_SUCCESS;
}

NEXUS_Error NxClient_P_Display_SetCgmsAorB(nxclient_t client, uint32_t cgmsData, const nxclient_p_set_cgms_b_data *pdata)
{
    unsigned i;

    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }

    /* cgms to all displays */
    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        NEXUS_DisplayHandle display = client->session->display[i].display;
        if (display) {
            NEXUS_Error rc;
            NEXUS_DisplayVbiSettings settings;
            NEXUS_Display_GetVbiSettings(display, &settings);
            if (!settings.cgmsEnabled) {
                settings.cgmsEnabled = true;
                rc = NEXUS_Display_SetVbiSettings(display, &settings);
                if (rc) BERR_TRACE(rc); /* keep going */
            }
            if (pdata) {
                rc = NEXUS_Display_SetCgmsB(display, pdata->data, pdata->size);
                if (rc) return BERR_TRACE(rc);
            }
            else {
                rc = NEXUS_Display_SetCgms(display, cgmsData);
                if (rc) return BERR_TRACE(rc);
            }
        }
    }
    return NEXUS_SUCCESS;
}

void NxClient_P_GetAudioProcessingSettings(nxclient_t client, NxClient_AudioProcessingSettings *pSettings )
{
    /* non-destructive or per-client functions */
    nxserverlib_p_audio_get_audio_procesing_settings(client->session, pSettings);
    return;
}

NEXUS_Error NxClient_P_SetAudioProcessingSettings(nxclient_t client, const NxClient_AudioProcessingSettings *pSettings )
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserverlib_p_audio_set_audio_procesing_settings(client->session, pSettings);
}

NEXUS_Error NxClient_P_Reconfig(nxclient_t client, const NxClient_ReconfigSettings *pSettings)
{
    return nxserverlib_p_reconfig(client, pSettings);
}

NEXUS_Error NxClient_P_Screenshot(nxclient_t client,  const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserverlib_p_screenshot(client, pSettings, surface);
}

NEXUS_Error NxClient_P_Display_SetMacrovision(nxclient_t client, NEXUS_DisplayMacrovisionType type, bool pTable_isNull, const NEXUS_DisplayMacrovisionTables *pTable)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserverlib_p_set_macrovision(client->session, type, pTable_isNull?NULL:pTable);
}

NEXUS_Error NxClient_P_GrowHeap(nxclient_t client, unsigned heapIndex )
{
    /* non-destructive or per-client functions */
    if (heapIndex >= NEXUS_MAX_HEAPS || !client->server->settings.client.heap[heapIndex] ||
        heapIndex != NXCLIENT_DYNAMIC_HEAP) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    else {
        return NEXUS_Platform_GrowHeap(client->server->settings.client.heap[heapIndex], client->server->settings.growHeapBlockSize);
    }
}

void NxClient_P_ShrinkHeap(nxclient_t client, unsigned heapIndex )
{
    if (!is_trusted_client(client)) {
        BERR_TRACE(NXCLIENT_NOT_ALLOWED);
        return;
    }
    if (heapIndex >= NEXUS_MAX_HEAPS || !client->server->settings.client.heap[heapIndex] ||
        heapIndex != NXCLIENT_DYNAMIC_HEAP) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    else {
        NEXUS_Platform_ShrinkHeap(client->server->settings.client.heap[heapIndex], client->server->settings.growHeapBlockSize, client->server->settings.growHeapBlockSize);
        return;
    }
}

NEXUS_Error NxClient_P_Config_LookupClient(nxclient_t client, unsigned pid, NEXUS_ClientHandle *pHandle)
{
    *pHandle = 0;
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    *pHandle = nxserverlib_p_lookup_client(client->server, pid);
    return *pHandle ? 0 : NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NxClient_P_Display_GetCrcData(nxclient_t client, unsigned displayIndex, NxClient_DisplayCrcData *pData)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    if (displayIndex >= NXCLIENT_MAX_DISPLAYS) {
        return NEXUS_NOT_AVAILABLE;
    }
    else {
        NEXUS_DisplayHandle display = client->session->display[displayIndex].display;
        NEXUS_DisplaySettings settings;
        NEXUS_Error rc;
        if (!display) {
            return NEXUS_NOT_AVAILABLE;
        }
        NEXUS_Display_GetSettings(display, &settings);
        if (settings.crcQueueSize == 0) {
            settings.crcQueueSize = 32;
            rc = NEXUS_Display_SetSettings(display, &settings);
            if (rc) return BERR_TRACE(rc);
            /* when this client exits, we shutdown */
            client->session->display[displayIndex].crc_client = client;
        }
        return NEXUS_Display_GetCrcData(display,
                                        pData->data,
                                        sizeof(pData->data)/sizeof(pData->data[0]),
                                        &pData->numEntries);
    }
}

NEXUS_Error NxClient_P_HdmiOutput_GetCrcData(nxclient_t client, NxClient_HdmiOutputCrcData *pData )
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (!client->session->hdmiOutput) {
        return NEXUS_NOT_AVAILABLE;
    }
    else {
        NEXUS_HdmiOutputSettings settings;
        NEXUS_Error rc;
        NEXUS_HdmiOutput_GetSettings(client->session->hdmiOutput, &settings);
        if (settings.crcQueueSize == 0) {
            settings.crcQueueSize = 32;
            rc = NEXUS_HdmiOutput_SetSettings(client->session->hdmiOutput, &settings);
            if (rc) return BERR_TRACE(rc);
            /* when this client exits, we shutdown */
            client->session->hdmiOutput_crc_client = client;
        }
        return NEXUS_HdmiOutput_GetCrcData(client->session->hdmiOutput,
                                           pData->data,
                                           sizeof(pData->data)/sizeof(pData->data[0]),
                                           &pData->numEntries);
    }
#else
    BSTD_UNUSED(pData);
    return NEXUS_NOT_AVAILABLE;
#endif
}

NEXUS_Error NxClient_P_RegisterAcknowledgeStandby_ipc(nxclient_t client, unsigned *id)
{
    struct b_client_standby_ack *ack;

    /* non-destructive or per-client functions */
    ack = BKNI_Malloc(sizeof(*ack));
    if (!ack) return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    BLST_S_INSERT_HEAD(&client->standby.acks, ack, link);
    ack->id = client->server->nextId[b_resource_register_standby];
    inc_id(client->server, b_resource_register_standby);
    *id = ack->id;
    return NEXUS_SUCCESS;
}

void NxClient_P_UnregisterAcknowledgeStandby(nxclient_t client, unsigned id )
{
    struct b_client_standby_ack *ack;
    /* non-destructive or per-client functions */

    for (ack = BLST_S_FIRST(&client->standby.acks); ack; ack = BLST_S_NEXT(ack, link)) {
        if (ack->id == id) {
            BLST_S_REMOVE(&client->standby.acks, ack, b_client_standby_ack, link);
            BKNI_Free(ack);
            break;
        }
    }
    if (!ack) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    return;
}

void NxClient_P_AcknowledgeStandby(nxclient_t client, unsigned id )
{
    struct b_client_standby_ack *ack;
    /* non-destructive or per-client functions */
    for (ack = BLST_S_FIRST(&client->standby.acks); ack; ack = BLST_S_NEXT(ack, link)) {
        if (ack->id == id) {
            ack->waiting = false;
            break;
        }
    }
    if (!ack) {
        (void)BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }
    return;
}

NEXUS_Error NxClient_P_LoadHdcpKeys(nxclient_t client, NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset,unsigned size)
{
#if NEXUS_HAS_HDMI_OUTPUT
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserver_load_hdcp_keys(client->session, hdcpType, block, blockOffset, size);
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(hdcpType);
    BSTD_UNUSED(block);
    BSTD_UNUSED(blockOffset);
    BSTD_UNUSED(size);
    return NEXUS_NOT_AVAILABLE;
#endif
}

NEXUS_Error NxClient_P_SetHdmiInputRepeater(nxclient_t client, NEXUS_HdmiInputHandle hdmiInput)
{
#if NEXUS_HAS_HDMI_OUTPUT
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserver_set_hdmi_input_repeater(client, hdmiInput);
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(hdmiInput);
    return NEXUS_NOT_AVAILABLE;
#endif
}

NEXUS_Error NxClient_P_SetSlaveDisplayGraphics(nxclient_t client, unsigned slaveDisplay, NEXUS_SurfaceHandle surface)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserver_set_slave_display_graphics(client, slaveDisplay, surface);
}

NEXUS_Error NxClient_P_GetStatus(nxclient_t client, NxClient_Status *pStatus)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return nxserver_get_status(client, pStatus);
}

static NEXUS_VideoFormat get_4k_matching_format(NEXUS_VideoFormat format)
{
    switch (format) {
    case NEXUS_VideoFormat_e3840x2160p24hz:
    case NEXUS_VideoFormat_e4096x2160p24hz:
        return NEXUS_VideoFormat_e1080p24hz;
    case NEXUS_VideoFormat_e3840x2160p25hz:
    case NEXUS_VideoFormat_e4096x2160p25hz:
        return NEXUS_VideoFormat_e1080p25hz;
    case NEXUS_VideoFormat_e3840x2160p30hz:
    case NEXUS_VideoFormat_e4096x2160p30hz:
        return NEXUS_VideoFormat_e1080p30hz;
    case NEXUS_VideoFormat_e3840x2160p50hz:
    case NEXUS_VideoFormat_e4096x2160p50hz:
        return NEXUS_VideoFormat_e1080p50hz;
    case NEXUS_VideoFormat_e3840x2160p60hz:
    case NEXUS_VideoFormat_e4096x2160p60hz:
        return NEXUS_VideoFormat_e1080p;
    default:
        return NEXUS_VideoFormat_eUnknown; /* not 4K */
    }
}

static NEXUS_VideoFormat nxserver_p_supported_bvn_format(struct b_session *session, NEXUS_VideoFormat format)
{
    if (session->server->display.cap.displayFormatSupported[format]) {
        return format;
    }
    format = get_4k_matching_format(format);
    if (session->server->display.cap.displayFormatSupported[format]) {
        return format;
    }
    return NEXUS_VideoFormat_eUnknown;
}

#define COPY_STRUCT_FIELD(TO, FROM, FIELD) do {if ((TO)->FIELD != (FROM)->FIELD) { (TO)->FIELD = (FROM)->FIELD; change = true; }}while(0)

static int nxserver_p_apply_graphics_settings(NEXUS_GraphicsSettings *pSettings, const NxClient_GraphicsSettings *pGraphicsSettings)
{
    bool change = false;
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, sourceBlendFactor);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, destBlendFactor);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, constantAlpha);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, horizontalFilter);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, verticalFilter);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, horizontalCoeffIndex);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, verticalCoeffIndex);
    COPY_STRUCT_FIELD(pSettings, pGraphicsSettings, alpha);
    COPY_STRUCT_FIELD(&pSettings->sdrToHdr, &pGraphicsSettings->sdrToHdr, y);
    COPY_STRUCT_FIELD(&pSettings->sdrToHdr, &pGraphicsSettings->sdrToHdr, cb);
    COPY_STRUCT_FIELD(&pSettings->sdrToHdr, &pGraphicsSettings->sdrToHdr, cr);
    return change?1:0;
}

static NEXUS_HeapHandle nxserver_p_framebuffer_heap(struct b_session *session, unsigned displayIndex, bool secure)
{
    NEXUS_HeapHandle heap;
    heap = NEXUS_Platform_GetFramebufferHeap(session->display[displayIndex].global_index);
    if (heap && secure && displayIndex == 0) {
        NEXUS_MemoryStatus status;
        NEXUS_Error rc;
        rc = NEXUS_Heap_GetStatus(heap, &status);
        if (rc) {
            BERR_TRACE(rc);
            heap = NULL;
        }
        else {
            switch (status.memcIndex) {
            case 0: heap = session->server->platformConfig.heap[NEXUS_MEMC0_SECURE_GRAPHICS_HEAP]; break;
            case 1: heap = session->server->platformConfig.heap[NEXUS_MEMC1_SECURE_GRAPHICS_HEAP]; break;
            case 2: heap = session->server->platformConfig.heap[NEXUS_MEMC2_SECURE_GRAPHICS_HEAP]; break;
            default: heap = NULL; break;
            }
        }
        if (!heap) {
            BDBG_ERR(("no MEMC%u secure graphics heap exists for display 0", status.memcIndex));
        }
        else {
            BDBG_WRN(("NSC framebuffer from secure graphics heap on MEMC%u", status.memcIndex));
        }
    }
    return heap;
}

struct nxserver_display_format
{
    NEXUS_VideoFormat videoFormat;
    NEXUS_ColorSpace colorSpace;
    unsigned colorDepth;
};

static void nxserver_p_get_target_format(struct b_session *session, const NxClient_DisplaySettings *pSettings, struct nxserver_display_format *target_format)
{
    if (session->hdmi.defaultSdActive) {
        target_format->videoFormat = pSettings->defaultSdFormat;
        target_format->colorSpace = NEXUS_ColorSpace_eAuto;
        target_format->colorDepth = 0;
    }
    else {
        target_format->videoFormat = pSettings->format;
        target_format->colorSpace = pSettings->hdmiPreferences.colorSpace;
        target_format->colorDepth = pSettings->hdmiPreferences.colorDepth;
    }
}

static int b_display_format_change(struct b_session *session, const NxClient_DisplaySettings *pDisplaySettings)
{
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    int rc;
    unsigned i;
    NEXUS_VideoFormat bvnFormat;
    struct nxserver_display_format target_format;

    NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
    surface_compositor_settings.enabled = true;

    nxserver_p_get_target_format(session, pDisplaySettings, &target_format);

    /* If CMP does not support the format, see if 4K upscale is an option.
    We have already validated the format with HDMI EDID at this point. */
    bvnFormat = nxserver_p_supported_bvn_format(session, target_format.videoFormat);
    if (!bvnFormat) {
        /* if not 4K, then it's just an unsupported format. */
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
    if (bvnFormat == target_format.videoFormat) {
        bvnFormat = NEXUS_VideoFormat_eUnknown;
    }

#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiOutputSettings;
        NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiOutputSettings);
        if (hdmiOutputSettings.outputFormat) {
            hdmiOutputSettings.outputFormat = NEXUS_VideoFormat_eUnknown;
            rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiOutputSettings);
            if (rc) BERR_TRACE(rc); /* fall through */
        }
    }
#endif

    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        NEXUS_DisplayHandle display;
        NEXUS_DisplaySettings displaySettings;
        const NxClient_GraphicsSettings *pGraphicsSettings;

        display = session->display[i].display;
        if (!display) continue;

        NEXUS_Display_GetSettings(display, &displaySettings);
        if (i == 0) {
            displaySettings.format = bvnFormat ? bvnFormat : target_format.videoFormat;
        }
        else {
            displaySettings.format = pDisplaySettings->slaveDisplay[i-1].format;
        }
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[i].formatInfo);
        if (i == 0 && session->server->settings.native_3d) {
            /* overrideOrientation is only for halfres */
            displaySettings.display3DSettings.overrideOrientation = pDisplaySettings->display3DSettings.orientation!=NEXUS_VideoOrientation_e2D && !session->display[i].formatInfo.isFullRes3d;
            displaySettings.display3DSettings.orientation = pDisplaySettings->display3DSettings.orientation;
        }
        /* must be set with format if format changed */
        displaySettings.dynamicRangeMode = pDisplaySettings->hdmiPreferences.dynamicRangeMode;
        rc = NEXUS_Display_SetSettings(display, &displaySettings);
        if (rc) return BERR_TRACE(rc);

        NEXUS_Display_GetGraphicsSettings(display, &surface_compositor_settings.display[i].graphicsSettings);
        if (i == 0) {
            surface_compositor_settings.display[i].graphicsSettings.clip.width = MIN(surface_compositor_settings.display[i].framebuffer.width,session->display[i].formatInfo.width);
            surface_compositor_settings.display[i].graphicsSettings.clip.height = MIN(surface_compositor_settings.display[i].framebuffer.height,session->display[i].formatInfo.height);
            if(!session->display[0].formatInfo.isFullRes3d ) {
                if(pDisplaySettings->display3DSettings.orientation == NEXUS_VideoOrientation_e3D_LeftRight ) {
                    surface_compositor_settings.display[0].graphicsSettings.clip.width /= 2;
                } else if( pDisplaySettings->display3DSettings.orientation == NEXUS_VideoOrientation_e3D_OverUnder ) {
                    surface_compositor_settings.display[0].graphicsSettings.clip.height /= 2;
                }
            }
            if (!session->server->settings.native_3d) {
                surface_compositor_settings.display[0].display3DSettings.overrideOrientation = (pDisplaySettings->display3DSettings.orientation!=NEXUS_VideoOrientation_e2D);
                surface_compositor_settings.display[0].display3DSettings.orientation = pDisplaySettings->display3DSettings.orientation;
            }
            pGraphicsSettings = &pDisplaySettings->graphicsSettings;
        }
        else if (pDisplaySettings->slaveDisplay[i-1].mode == NxClient_SlaveDisplayMode_eGraphics) {
            NEXUS_GraphicsSettings graphicsSettings = surface_compositor_settings.display[i].graphicsSettings;
            surface_compositor_settings.display[i].display = NULL;
            graphicsSettings.enabled = false;
            NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
        }
        else {
            /* for SD displays, realloc if going from NTSC to PAL. */
            surface_compositor_settings.display[i].display = session->display[i].display;
            if (!surface_compositor_settings.display[i].manualPosition) {
                if (session->display[i].formatInfo.height > surface_compositor_settings.display[i].framebuffer.height) {
                    surface_compositor_settings.display[i].framebuffer.width = session->display[i].formatInfo.width;
                    surface_compositor_settings.display[i].framebuffer.height = session->display[i].formatInfo.height;
                }
                surface_compositor_settings.display[i].graphicsSettings.clip.width = session->display[i].formatInfo.width;
                surface_compositor_settings.display[i].graphicsSettings.clip.height = session->display[i].formatInfo.height;
            }
            pGraphicsSettings = &pDisplaySettings->slaveDisplay[i-1].graphicsSettings;
        }
        nxserver_p_apply_graphics_settings(&surface_compositor_settings.display[i].graphicsSettings, pGraphicsSettings);
        surface_compositor_settings.display[i].enabled = pGraphicsSettings->enabled;
        surface_compositor_settings.display[i].framebuffer.heap = nxserver_p_framebuffer_heap(session, i, pDisplaySettings->secure);
    }
    surface_compositor_settings.allowCompositionBypass = session->server->settings.allowCompositionBypass;
    rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
    if (rc) return BERR_TRACE(rc);
    BDBG_MSG(("enabled session %d surface_compositor again", session->index));

#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiOutputSettings;
        NEXUS_VideoFormat outputFormat = bvnFormat ? target_format.videoFormat : NEXUS_VideoFormat_eUnknown;
        NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiOutputSettings);
        if (target_format.colorSpace != hdmiOutputSettings.colorSpace ||
            target_format.colorDepth != hdmiOutputSettings.colorDepth ||
            outputFormat != hdmiOutputSettings.outputFormat)
        {
            hdmiOutputSettings.colorSpace = target_format.colorSpace;
            hdmiOutputSettings.colorDepth = target_format.colorDepth;
            hdmiOutputSettings.outputFormat = outputFormat;
            BDBG_LOG(("changing HdmiOutput%d to %d bit %s, upscale %s", session->index,
                target_format.colorDepth,
                lookup_name(g_colorSpaceStrs, target_format.colorSpace),
                lookup_name(g_videoFormatStrs,hdmiOutputSettings.outputFormat)));
            rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiOutputSettings);
            if (rc) BERR_TRACE(rc); /* fall through */
        }
    }
#endif

    /* possible change SD output on graphics-only SD system */
    nxserver_p_set_sd_outputs(session, pDisplaySettings);

    return 0;
}

bool nxserverlib_p_native_3d_active(struct b_session *session)
{
    NxClient_DisplaySettings *pDisplaySettings = &session->nxclient.displaySettings;
    return session->server->settings.native_3d &&
        (pDisplaySettings->display3DSettings.orientation != NEXUS_VideoOrientation_e2D ||
        session->display[0].formatInfo.isFullRes3d);
}

static void make_cursor(NEXUS_SurfaceHandle surface, const NEXUS_SurfaceCreateSettings *settings)
{
    NEXUS_SurfaceMemory memory;
    unsigned i;

    NEXUS_Surface_GetMemory(surface, &memory);
    BKNI_Memset(memory.buffer, 0, memory.pitch*settings->height);
    /* just a cross */
    for(i=0;i<settings->height;i++) {
        NEXUS_Pixel *pixel = (NEXUS_Pixel*)memory.buffer + i*memory.pitch/sizeof(NEXUS_Pixel) + settings->width/2;
        *pixel = 0xFFFFFFFF;
    }
    for(i=0;i<settings->width;i++) {
        NEXUS_Pixel *pixel = (NEXUS_Pixel *)memory.buffer + (settings->height/2)*memory.pitch/sizeof(NEXUS_Pixel) + i;
        *pixel = 0xFFFFFFFF;
    }
    NEXUS_Surface_Flush(surface);
    return;
}

#if NEXUS_HAS_HDMI_OUTPUT
static const char *g_hdcpLevelStr[NxClient_HdcpLevel_eMax] = {"off","optional","mandatory"};
static const char *g_hdcpSelectStr[NxClient_HdcpVersion_eMax] = {"auto", "hdcp1x", "hdcp22type0", "hdcp22"};
static const char *g_nxserver_hdcp_str[nxserver_hdcp_max] = {
    "not_pending",
    "begin",
    "follow",
    "pending_start_retry",
    "pending_start",
    "success"
};

static const char *g_hdcpStateStr[NEXUS_HdmiOutputHdcpState_eMax+1] = {
    "Unpowered",
    "Unauthenticated",
    "WaitForValidVideo",
    "InitializedAuthentication",
    "WaitForReceiverAuthentication",
    "ReceiverR0Ready",
    "R0LinkFailure",
    "ReceiverAuthenticated",
    "WaitForRepeaterReady",
    "CheckForRepeaterReady",
    "RepeaterReady",
    "LinkAuthenticated",
    "EncryptionEnabled",
    "RepeaterAuthenticationFailure",
    "RiLinkIntegrityFailure",
    "PjLinkIntegrityFailure",
    "UNKNOWN_STATE"
};

static const char *g_hdcpErrorStr[NEXUS_HdmiOutputHdcpError_eMax+1] = {
    "Success",
    "RxBksvError",
    "RxBksvRevoked",
    "RxBksvI2cReadError",
    "TxAksvError",
    "TxAksvI2cWriteError",
    "ReceiverAuthenticationError",
    "RepeaterAuthenticationError",
    "RxDevicesExceeded",
    "RepeaterDepthExceeded",
    "RepeaterFifoNotReady",
    "RepeaterDeviceCount0",
    "RepeaterLinkFailure",
    "LinkRiFailure",
    "LinkPjFailure",
    "FifoUnderflow",
    "FifoOverflow",
    "MultipleAnRequest",
    "UNKNOWN_ERROR"
};

static const char *nxserver_hdcp_state_str[nxserver_hdcp_max] = {"not_pending", "begin", "follow", "pending_start_retry", "pending_start", "success"};

static const char *get_hdcp_state_str(NEXUS_HdmiOutputHdcpState state) {
    if (state < NEXUS_HdmiOutputHdcpState_eMax) return g_hdcpStateStr[state];
    return g_hdcpStateStr[NEXUS_HdmiOutputHdcpState_eMax];
}

static const char *get_hdcp_error_str(NEXUS_HdmiOutputHdcpError error) {
    if (error < NEXUS_HdmiOutputHdcpError_eMax) return g_hdcpErrorStr[error];
    return g_hdcpErrorStr[NEXUS_HdmiOutputHdcpError_eMax];
}

static bool is_hdcp_start_complete(const NEXUS_HdmiOutputHdcpStatus *pHdcpStatus)
{
    bool is_final_state;

    is_final_state = (pHdcpStatus->hdcpState == NEXUS_HdmiOutputHdcpState_eEncryptionEnabled);
    BDBG_MSG(("is_hdcp_start_complete = %s(%s, %s)", is_final_state ? "true" : "false", get_hdcp_state_str(pHdcpStatus->hdcpState), get_hdcp_error_str(pHdcpStatus->hdcpError)));

    return (is_final_state);
}

/* return CLOCK_MONOTONIC timestamp in milliseconds */
static unsigned nxserver_p_timestamp(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000 / 1000;
}

static void nxserver_check_hdcp(struct b_session *session)
{
    int rc;
    nxclient_t client;
    nxclient_t repeaterClient;
    NxClient_HdcpLevel curr_hdcp_level;
    NxClient_HdcpVersion curr_version_select;
    NEXUS_HdmiOutputHandle hdmiOutput;
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;
    enum nxserver_hdcp_state curr_version_state = session->hdcp.version_state;


    if (!session->nxclient.displaySettings.hdmiPreferences.enabled) return;
    if (!session->hdmiOutput) return;
    if (nxserver_is_standby_pending(session->server)) return;

    hdmiOutput = session->hdmiOutput;
    repeaterClient = session->hdmi.repeater.client;

    if (repeaterClient) {
        curr_hdcp_level = repeaterClient->hdcp_level;
    }
    else {
        /* find highest hdcp level among all clients */
        curr_hdcp_level = session->server->settings.hdcp.alwaysLevel;
        for (client = BLST_D_FIRST(&session->server->clients); client; client = BLST_D_NEXT(client, link)) {
            if (client->hdcp_level > curr_hdcp_level) {
                curr_hdcp_level = client->hdcp_level;
            }
        }
    }
    session->hdcp.level = curr_hdcp_level;


    /* Skip, if no hdmi receiver available */
    rc = NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &hdmiStatus);
    if (rc) { BERR_TRACE(rc); goto done; }
    if (!hdmiStatus.rxPowered) {
        BDBG_MSG(("hdmiStatus.rxPowered == 0, do nothing"));
        if (repeaterClient) {
            repeaterClient->hdcp_level = NxClient_HdcpLevel_eNone;
        }

        if (session->hdcp.version_state != nxserver_hdcp_not_pending) {
            session->callbackStatus.hdmiOutputHdcpChanged++;
            session->hdcp.version_state = nxserver_hdcp_not_pending;
        }
        goto done;
    }

    /* Skip, if no hdcp authentication is desired */
    if (curr_hdcp_level == NxClient_HdcpLevel_eNone) {
        BDBG_MSG(("curr_hdcp_level == NxClient_HdcpLevel_eNone, disable hdcp authentication"));
        rc = NEXUS_HdmiOutput_DisableHdcpAuthentication(hdmiOutput);
        if (rc) BDBG_ERR(("NEXUS_HdmiOutput_DisableHdcpAuthentication failed: %d", rc));
        if (session->hdcp.version_state != nxserver_hdcp_not_pending) {
            session->callbackStatus.hdmiOutputHdcpChanged++;
        }
        session->hdcp.version_state = nxserver_hdcp_not_pending;
        goto done;
    }

#if NEXUS_HAS_SECURITY || defined(NEXUS_HAS_HDCP_ASTRA_SUPPORT)
    rc = NEXUS_HdmiOutput_GetHdcpStatus(hdmiOutput, &hdcpStatus);
#else
    rc = NEXUS_NOT_SUPPORTED;
#endif
    if (rc) {
        /* if unable to get status, the HDCP circuit is not functioning properly. wait for hotplug or format change. */
        BERR_TRACE(rc);
        goto done;
    }

    if (session->server->settings.hdcp.alwaysLevel != NxClient_HdcpLevel_eNone)
        curr_version_select = session->server->settings.hdcp.versionSelect;
    else
        curr_version_select = NxClient_HdcpVersion_eAuto;
    for (client = BLST_D_FIRST(&session->server->clients); client; client = BLST_D_NEXT(client, link)) {
        if (client->hdcp_level != NxClient_HdcpLevel_eNone && client->hdcp_version > curr_version_select) {
            curr_version_select = client->hdcp_version;
        }
    }

    /* If new selection or not currently running, startup */
    if (session->hdcp.version_select != curr_version_select ||
        session->hdcp.version_state == nxserver_hdcp_not_pending) {
        bool no_change = false;
        BDBG_MSG(("Change in version_select: %s (%s)", g_hdcpSelectStr[curr_version_select], g_hdcpLevelStr[curr_hdcp_level]));

        /* when changing version and already authenticated, if new version is compatible with current state, no need to reauthenticate */
        if (is_hdcp_start_complete(&hdcpStatus)) {
            NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;
            NEXUS_HdmiOutput_GetHdcpSettings(session->hdmiOutput, &hdmiOutputHdcpSettings);
            switch (curr_version_select) {
            case NxClient_HdcpVersion_eAuto:
                /* any authentication is good except hdcp22 cst 1 with a downstream 1x device */
                no_change = !(hdcpStatus.selectedHdcpVersion == NEXUS_HdcpVersion_e2x &&
                    hdmiOutputHdcpSettings.hdcp2xContentStreamControl == NEXUS_Hdcp2xContentStream_eType1 &&
                    hdcpStatus.hdcp2_2RxInfo.hdcp1_xDeviceDownstream);
                break;
            case NxClient_HdcpVersion_eHdcp1x:
                /* must be hdcp1x */
                if (hdcpStatus.selectedHdcpVersion != NEXUS_HdcpVersion_e2x) no_change = true;
                break;
            case NxClient_HdcpVersion_eAutoHdcp22Type0:
                /* any authentication is good except hdcp22 cst 1 */
                if (hdcpStatus.selectedHdcpVersion == NEXUS_HdcpVersion_e2x) {
                    if (hdmiOutputHdcpSettings.hdcp2xContentStreamControl == NEXUS_Hdcp2xContentStream_eType0) no_change = true;
                }
                else {
                    no_change = true;
                }
                break;
            case NxClient_HdcpVersion_eHdcp22:
                /* only hdcp22 cst 1 allowed */
                if (hdcpStatus.selectedHdcpVersion == NEXUS_HdcpVersion_e2x) {
                    if (hdmiOutputHdcpSettings.hdcp2xContentStreamControl == NEXUS_Hdcp2xContentStream_eType1) no_change = true;
                }
                break;
            default:
                BERR_TRACE(NEXUS_INVALID_PARAMETER);
                goto done;
            }
        }
        session->hdcp.version_select = curr_version_select;
        if (no_change) {
            BDBG_LOG(("No re-authentication needed"));
            goto done;
        }
        /* start authentication */
        rc = NEXUS_HdmiOutput_DisableHdcpAuthentication(hdmiOutput);
        if (rc) BDBG_ERR(("NEXUS_HdmiOutput_DisableHdcpAuthentication failed: %d", rc));
        session->hdcp.version_state = nxserver_hdcp_begin;
    }


    if ((session->hdcp.version_state == nxserver_hdcp_success) && (!is_hdcp_start_complete(&hdcpStatus))) {
        session->hdcp.version_state = nxserver_hdcp_begin;
    }

    curr_version_state = session->hdcp.version_state;
    BDBG_LOG(("nxserver_check_hdcp: %s, %s, %s %u", get_hdcp_state_str(hdcpStatus.hdcpState), get_hdcp_error_str(hdcpStatus.hdcpError), nxserver_hdcp_state_str[curr_version_state], session->hdcp.cnt));

    switch (hdcpStatus.hdcpState) {
    case NEXUS_HdmiOutputHdcpState_eUnpowered :
    case NEXUS_HdmiOutputHdcpState_eEncryptionEnabled :
    case NEXUS_HdmiOutputHdcpState_eUnauthenticated :
    case NEXUS_HdmiOutputHdcpState_eR0LinkFailure :
    case NEXUS_HdmiOutputHdcpState_eRepeaterAuthenticationFailure :
    case NEXUS_HdmiOutputHdcpState_eRiLinkIntegrityFailure :
    case NEXUS_HdmiOutputHdcpState_ePjLinkIntegrityFailure :
        /* terminal state. nexus will not callback again. we must be happy or take action. */
        break;
    default:
        /* not terminal state. nexus will callback again. */
        goto done;
    }

    session->hdcp.cnt++;
    switch (curr_version_state) {
    case nxserver_hdcp_not_pending:
    case nxserver_hdcp_begin:
        session->hdcp.lastHdcpError = NEXUS_HdmiOutputHdcpError_eSuccess;
        break;
    default:
        if (hdcpStatus.hdcpError) {
            unsigned now = nxserver_p_timestamp();
            unsigned first_start_time = (now - session->hdcp.first_start_timestamp) / 1000;
            session->hdcp.lastHdcpError = hdcpStatus.hdcpError;
            if (session->server->settings.hdcp.immediateRetries && session->hdcp.cnt < session->server->settings.hdcp.immediateRetries) {
                BDBG_MSG(("HDCP re-auth immediately"));
            }
            else if (now - session->hdcp.start_timestamp < 1400) {
                /* rely on hdmi_output's hdcpFailedStartTimer firing every 2 seconds */
                BDBG_MSG(("skip HDCP re-auth because it was requested too soon"));
                goto done;
            }
            else if (session->server->settings.hdcp.failureTimeout && first_start_time >= session->server->settings.hdcp.failureTimeout) {
                BDBG_LOG(("failed HDCP re-authentication reached %u second timeout. disabling HDCP.", session->server->settings.hdcp.failureTimeout));
                /* we will restart from the beginning if ever nxserver_check_hdcp is called again from hotplug or format change
                but not from hdcpStateChanged callback. */
                NEXUS_HdmiOutput_DisableHdcpAuthentication(session->hdmiOutput);
                session->hdcp.version_state = nxserver_hdcp_not_pending;
                goto done;
            }

            BDBG_MSG(("allow HDCP re-auth after %u msec", now - session->hdcp.start_timestamp));
        }
        break;
    }

    switch (curr_version_state) {
    case nxserver_hdcp_not_pending:
        /* Do nothing, no authentication desired */
        break;

    case nxserver_hdcp_begin:
        /* API assumptions:
        After DisableHdcpAuthentication is called, we should expect nothing from HdmiOutput. Therefore we only call when the user has disabled hdcp.
        We can call StartHdcpAuthentication at any time and the hdcp state machine will restart.
        After calling StartHdcpAuthentication, we will keep getting HDCP callbacks until authenticated or DisableHdcpAuthentication is called (keep alive timer).
        */
        session->hdcp.first_start_timestamp = nxserver_p_timestamp();
        session->hdcp.cnt = 0;
        if (!is_hdcp_start_complete(&hdcpStatus))
        {
            BDBG_LOG(("Start hdcp authentication(version=%s level=%s)", g_hdcpSelectStr[curr_version_select], g_hdcpLevelStr[curr_hdcp_level]));
            initializeHdmiOutputHdcpSettings(session, curr_version_select);
            rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
            if (rc) BDBG_ERR(("nxserver_check_hdcp: %s: NEXUS_HdmiOutput_StartHdcpAuthentication failed: %d", g_nxserver_hdcp_str[curr_version_state], rc));
            if (curr_version_select == NxClient_HdcpVersion_eAuto) {
                session->hdcp.version_state = nxserver_hdcp_follow;
            }
            else {
                session->hdcp.version_state = nxserver_hdcp_pending_start;
            }
        }
        else
        {
            session->callbackStatus.hdmiOutputHdcpChanged++;
            session->hdcp.version_state = nxserver_hdcp_success;
        }

        break;

    case nxserver_hdcp_follow:
        if (!is_hdcp_start_complete(&hdcpStatus)) {
            curr_version_state = session->hdcp.version_state = nxserver_hdcp_begin;
            initializeHdmiOutputHdcpSettings(session, curr_version_select);
            rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
            if (rc) BDBG_ERR(("nxserver_check_hdcp: %s: NEXUS_HdmiOutput_StartHdcpAuthentication failed: %d", g_nxserver_hdcp_str[curr_version_state], rc));
            session->hdcp.version_state = nxserver_hdcp_follow;
            session->callbackStatus.hdmiOutputHdcpChanged++;
        }
        /* if we've authenticated with an HDCP 2.2 repeater with a downstream 1.x deivce, we must re-authenticate using content stream type 0 */
        else if (hdcpStatus.isHdcpRepeater && hdcpStatus.hdcp2_2Features && hdcpStatus.hdcp2_2RxInfo.hdcp1_xDeviceDownstream) {
            initializeHdmiOutputHdcpSettings(session, curr_version_select);
            rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
            if (rc) BDBG_ERR(("nxserver_check_hdcp: %s: NEXUS_HdmiOutput_StartHdcpAuthentication failed: %d", g_nxserver_hdcp_str[curr_version_state], rc));
            session->hdcp.version_state = nxserver_hdcp_pending_start;
            session->callbackStatus.hdmiOutputHdcpChanged++;
        }
        else {
            /* we started with HDCP 1.x or HDCP 2.2 and content stream type 1, so we're done */
            session->callbackStatus.hdmiOutputHdcpChanged++;
            session->hdcp.version_state = nxserver_hdcp_success;
        }
        break;

    case nxserver_hdcp_pending_start:
    case nxserver_hdcp_pending_start_retry:
        if (!is_hdcp_start_complete(&hdcpStatus)) {
            initializeHdmiOutputHdcpSettings(session, curr_version_select);
            rc = NEXUS_HdmiOutput_StartHdcpAuthentication(hdmiOutput);
            if (rc) BDBG_ERR(("nxserver_check_hdcp: %s: NEXUS_HdmiOutput_StartHdcpAuthentication failed: %d", g_nxserver_hdcp_str[curr_version_state], rc));
            session->hdcp.version_state = nxserver_hdcp_pending_start_retry;
            session->callbackStatus.hdmiOutputHdcpChanged++;
        }
        else {
            session->callbackStatus.hdmiOutputHdcpChanged++;
            session->hdcp.version_state = nxserver_hdcp_success;
        }
        break;

    case nxserver_hdcp_success:
        break;

    default:
        BDBG_ERR(("nxserver_check_hdcp: ERROR curr_version_state=%s", g_nxserver_hdcp_str[curr_version_state]));
        break;
    }

done:

    if (curr_version_state != session->hdcp.version_state) {
        BDBG_LOG(("nxserver_check_hdcp: *** (%s --> %s)", g_nxserver_hdcp_str[curr_version_state], g_nxserver_hdcp_str[session->hdcp.version_state]));
        if (session->hdcp.version_state == nxserver_hdcp_success) {
            BDBG_LOG(("HDCP authentication successful"));
            session->hdcp.lastHdcpError = NEXUS_HdmiOutputHdcpError_eSuccess;
        }
    }

    /* any change in pending or level may change mute, so recheck */
    nxserver_hdcp_mute(session);
}

static void hotplug_callback_locked(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    nxclient_t repeaterClient;
    struct b_session *session = pParam;
    int rc;

    if (!session->hdmiOutput) {
        return;
    }

    BSTD_UNUSED(iParam);
    session->callbackStatus.hdmiOutputHotplug++;
    rc = NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &status);
    if (rc!=NEXUS_SUCCESS) {
        return;
    }

    repeaterClient = session->hdmi.repeater.client;
    if (repeaterClient)
    {
        repeaterClient->hdcp_level = NxClient_HdcpLevel_eNone;
    }

    if(status.connected) {
        nxserverlib_dynrng_p_hotplug_callback_locked(&session->hdmi.dynrng);
        bserver_hdmi_edid_audio_config(session, &status);
        if (session->main_audio) {
            bserver_set_audio_config(session->main_audio, false);
        }

        if (!status.videoFormatSupported[session->nxclient.displaySettings.format]) {
            if (session->hdmi.defaultSdActive ||
                (session->nxclient.displaySettings.hdmiPreferences.followPreferredFormat &&
                ! session->server->settings.hdmi.ignoreVideoEdid)) {
                NEXUS_VideoFormat bvnFormat = nxserver_p_supported_bvn_format(session, status.preferredVideoFormat);
                if (bvnFormat) {
                    NxClient_DisplaySettings settings = session->nxclient.displaySettings;

                    BDBG_WRN(("Current format %s not supported by attached monitor; Use preferred format %s",
                        lookup_name(g_videoFormatStrs, session->nxclient.displaySettings.format),
                        lookup_name(g_videoFormatStrs, status.preferredVideoFormat))) ;
                    settings.format = status.preferredVideoFormat;
                    settings.hdmiPreferences.colorSpace = NEXUS_ColorSpace_eAuto;
                    settings.hdmiPreferences.colorDepth = 0;
                    session->hdmi.defaultSdActive = false;
                    NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &settings);
                }
            }
        }
        else if (session->hdmi.defaultSdActive) {
            session->hdmi.defaultSdActive = false;
            rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &session->nxclient.displaySettings);
            if (rc) BERR_TRACE(rc);
        }
    }
    else {
        /* defaultSdFormat feature switches display[0] to SD format if HDMI is disconnected */
        if (session->server->settings.display.defaultSdFormat != NEXUS_VideoFormat_eUnknown) {
            session->hdmi.defaultSdActive = true;
            rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &session->nxclient.displaySettings);
            if (rc) BERR_TRACE(rc);
        }
    }

    session->hdcp.version_state = nxserver_hdcp_begin;
    nxserver_check_hdcp(session);
}

static void hotplug_callback(void *pParam, int iParam)
{
    struct b_session *session = pParam;
    nxserver_t server = session->server;
    BKNI_AcquireMutex(server->settings.lock);
    hotplug_callback_locked(pParam, iParam);
    BKNI_ReleaseMutex(server->settings.lock);
}
#else
void nxserver_check_hdcp(struct b_session *session)
{
    BSTD_UNUSED(session);
}
#endif

static void b_surface_compositor_inactive(void *context, int param)
{
    struct b_session *session = context;
    BSTD_UNUSED(param);
    BKNI_SetEvent(session->inactiveEvent);
}

static void nxserver_p_acquire_release_all_resources(struct b_session *session, bool acquire)
{
    nxclient_t client;
    for (client = BLST_D_FIRST(&session->clients); client; client = BLST_D_NEXT(client, session_link)) {
        struct b_connect *connect;
        for (connect = BLST_D_FIRST(&client->connects); connect; ) {
            struct b_connect *next = BLST_D_NEXT(connect, link);
            if (!acquire) {
                b_connect_release(client, connect);
            }
            else {
                b_connect_acquire(client, connect);
            }
            connect = next;
        }
    }
}

#if NEXUS_HAS_HDMI_OUTPUT
static void nxserver_hdcp_mute(struct b_session *session)
{
    int rc;
    bool mute = session->hdmiOutput && session->hdcp.level == NxClient_HdcpLevel_eMandatory;
#if NEXUS_HAS_SAGE
    /* for SAGE_SECURE_MODE 6/9, we need to mute on nxserver_hdcp_pending_start as well. */
    mute = mute && (session->hdcp.version_state < nxserver_hdcp_success);
#else
    /* assuming order of enum has pending_start and success as the highest values */
    mute = mute && (session->hdcp.version_state < nxserver_hdcp_pending_start);
#endif
    if (mute != session->hdcp.mute) {
        NEXUS_SurfaceCompositorSettings surface_compositor_settings;

        session->hdcp.mute = mute;
        BDBG_LOG(("nxserver_hdcp_mute: %s (%p)", session->hdcp.mute ? "MUTING_HDMI" : "UNMUTING_HDMI", (void*)session));

        NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
        /* only mute HD (HDMI) no SD (composite) */
        surface_compositor_settings.muteVideo[0] = session->hdcp.mute;
        surface_compositor_settings.display[0].graphicsSettings.visible = !session->hdcp.mute;
        rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
        if (rc) {BERR_TRACE(rc);}
    }
}

static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    struct b_session *session = pContext;
    nxserver_t server = session->server;

    BSTD_UNUSED(param);

    BKNI_AcquireMutex(server->settings.lock);
    if (!session->hdmiOutput) goto done;
    if (!session->hdcp.level) goto done;
    if (session->hdcp.version_state == nxserver_hdcp_not_pending) goto done;

    nxserver_check_hdcp(session);
done:
    BKNI_ReleaseMutex(server->settings.lock);
}

static void nxserver_clear_hdcp1xKeys(struct b_session *session)
{
    if (session->hdcpKeys.hdcp1x.buffer) {
        BKNI_Free(session->hdcpKeys.hdcp1x.buffer);
        session->hdcpKeys.hdcp1x.buffer = NULL;
    }
    session->hdcpKeys.hdcp1x.size = 0;
}

static void nxserver_clear_hdcp2xKeys(struct b_session *session)
{
    if (session->hdcpKeys.hdcp2x.buffer) {
        BKNI_Free(session->hdcpKeys.hdcp2x.buffer);
        session->hdcpKeys.hdcp2x.buffer = NULL;
    }
    session->hdcpKeys.hdcp2x.size = 0;
}

static void nxserver_clear_hdcpkeys(struct b_session *session)
{
    nxserver_clear_hdcp1xKeys( session );
    nxserver_clear_hdcp2xKeys( session );
}

static void nxserver_load_hdcpkey_files(struct b_session *session)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    const char *keyfile ;
    int fd;
    off_t seekPos;
    int n;
    const struct nxserver_settings *psettings = &session->server->settings;

    if (!session->hdmiOutput) return;
    nxserver_clear_hdcpkeys(session);

#if NEXUS_HAS_HDCP_2X_SUPPORT
    keyfile = psettings->hdcp.hdcp2xBinFile[0] ? psettings->hdcp.hdcp2xBinFile : HDCP2x_DEFAULT_BIN ;
    fd = open(keyfile, O_RDONLY);
    if (fd < 0) {
        if (psettings->hdcp.hdcp2xBinFile[0]) {
            BDBG_ERR(("Cannot open hdcp2x bin file %s", keyfile));
        }
        rc = NEXUS_OS_ERROR;
        goto loadHdcp1xKeys;
    }

    seekPos = lseek(fd, 0, SEEK_END);
    if (seekPos < 0) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto done;
    }
    session->hdcpKeys.hdcp2x.size = seekPos;
    BDBG_MSG(("loading %u bytes of HDCP2.x keys from '%s'", session->hdcpKeys.hdcp2x.size, keyfile)) ;
    if (lseek(fd, 0, SEEK_SET) < 0) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto done;
    }

    session->hdcpKeys.hdcp2x.buffer = BKNI_Malloc(session->hdcpKeys.hdcp2x.size);
    if (!session->hdcpKeys.hdcp2x.buffer) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }
    n = read(fd, session->hdcpKeys.hdcp2x.buffer, session->hdcpKeys.hdcp2x.size);
    if (n != (int)session->hdcpKeys.hdcp2x.size) {
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto done;
    }

    close(fd);

loadHdcp1xKeys:
#else
    if (psettings->hdcp.hdcp2xBinFile[0]) {
        BDBG_ERR(("HDCP2.x not supported. Recompile with NEXUS_HDCP_SUPPORT=y."));
    }
    BSTD_UNUSED(seekPos) ;
#endif
    if ( rc ) {
        /* error reading HDCP 2.2 Keyset; clear keyset  */
        nxserver_clear_hdcp2xKeys(session);

        /* reset status for HDCP 1.x Keyset */
        rc = NEXUS_SUCCESS ;
    }

    keyfile = psettings->hdcp.hdcp1xBinFile[0] ? psettings->hdcp.hdcp1xBinFile : HDCP1x_DEFAULT_BIN ;
    fd = open(keyfile, O_RDONLY);
    if (fd < 0) {
        if (psettings->hdcp.hdcp1xBinFile[0]) {
            BDBG_ERR(("Cannot open hdcp1x bin file %s", keyfile));
        }
        rc = NEXUS_OS_ERROR;
        goto done;
    }

    session->hdcpKeys.hdcp1x.size = sizeof(((NEXUS_HdmiOutputHdcpSettings*)0)->aksv.data) + 3 + sizeof(((NEXUS_HdmiOutputHdcpSettings*)0)->encryptedKeySet);
    BDBG_MSG(("loading %u bytes of HDCP1.x keys from '%s'", session->hdcpKeys.hdcp1x.size, keyfile));
    session->hdcpKeys.hdcp1x.buffer = BKNI_Malloc(session->hdcpKeys.hdcp1x.size);
    if (!session->hdcpKeys.hdcp1x.buffer) {
        rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
        goto done;
    }

    n = read(fd, session->hdcpKeys.hdcp1x.buffer, session->hdcpKeys.hdcp1x.size);
    if (n != (int)session->hdcpKeys.hdcp1x.size) {
        BDBG_ERR(("Unable to read hdcp1x from file %s, %u != %u\n",
            keyfile, n, session->hdcpKeys.hdcp1x.size));
        rc = BERR_TRACE(NEXUS_OS_ERROR);
        goto done;
    }

done:
    if (fd != -1) {
        close(fd);
    }
    if (rc) {
        nxserver_clear_hdcp1xKeys(session);
    }
}

static NEXUS_Error nxserver_load_hdcp_keys(struct b_session *session, NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset, unsigned size)
{
    int rc;
    void *ptr;
    NEXUS_MemoryBlockProperties prop;

    if (hdcpType == NxClient_HdcpType_1x) {
        nxserver_clear_hdcp1xKeys(session);
    }
    else {
        nxserver_clear_hdcp2xKeys(session);
    }
    NEXUS_MemoryBlock_GetProperties(block, &prop);
    if (prop.size < blockOffset + size) {
        rc = BERR_TRACE(NEXUS_INVALID_PARAMETER);
        goto err_prop;
    }
    rc = NEXUS_MemoryBlock_Lock(block, &ptr);
    if (rc) {
        rc = BERR_TRACE(rc);
        goto err_lock;
    }
    ptr = &((uint8_t*)ptr)[blockOffset];
    if (hdcpType == NxClient_HdcpType_1x) {
        session->hdcpKeys.hdcp1x.size = size;
        session->hdcpKeys.hdcp1x.buffer = BKNI_Malloc(size);
        if (!session->hdcpKeys.hdcp1x.buffer) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc;
        }
        BKNI_Memcpy(session->hdcpKeys.hdcp1x.buffer, ptr, size);
    }
    else {
        session->hdcpKeys.hdcp2x.size = size;
        session->hdcpKeys.hdcp2x.buffer = BKNI_Malloc(size);
        if (!session->hdcpKeys.hdcp2x.buffer) {
            rc = BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
            goto err_malloc;
        }
        BKNI_Memcpy(session->hdcpKeys.hdcp2x.buffer, ptr, size);
    }
    NEXUS_MemoryBlock_Unlock(block);
    session->hdcp.version_state = nxserver_hdcp_begin;
    nxserver_check_hdcp(session);
    return NEXUS_SUCCESS;

err_malloc:
    NEXUS_MemoryBlock_Unlock(block);
    if (hdcpType == NxClient_HdcpType_1x) {
        nxserver_clear_hdcp1xKeys(session);
    }
    else {
        nxserver_clear_hdcp2xKeys(session);
    }

err_lock:
err_prop:
    return rc;
}

static NEXUS_Error nxserver_set_hdmi_input_repeater(nxclient_t client, NEXUS_HdmiInputHandle hdmiInput)
{
    struct b_session *session = client->session;
    NEXUS_Error rc = NEXUS_SUCCESS;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;
    if (!session->hdmiOutput) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }
#if NEXUS_HAS_HDMI_INPUT
    session->hdmi.repeater.client = client;
    NEXUS_HdmiOutput_SetRepeaterInput(session->hdmiOutput, hdmiInput);
    session->hdmiInput = hdmiInput;
#else
    BSTD_UNUSED(hdmiInput);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif

    /* Start downstream authentication if not yet authenticated */
    if (client)
    {
        rc = NEXUS_HdmiOutput_GetHdcpStatus(session->hdmiOutput, &hdcpStatus);
        if (rc) {
            BERR_TRACE(rc);
        }
        else if (!is_hdcp_start_complete(&hdcpStatus)) {
            client->hdcp_level = NxClient_HdcpLevel_eMandatory;
            session->hdcp.version_state = nxserver_hdcp_not_pending;
            nxserver_check_hdcp(session);
        }
    }
    return rc;
}

static void initializeHdmiOutputHdcpSettings(struct b_session *session, NxClient_HdcpVersion version_select)
{
#if NEXUS_HAS_SECURITY
    NEXUS_Error rc;
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    session->hdcp.start_timestamp = nxserver_p_timestamp();
    NEXUS_HdmiOutput_GetHdcpSettings(session->hdmiOutput,  &hdmiOutputHdcpSettings);

    if (version_select == NxClient_HdcpVersion_eAuto && session->hdcpKeys.hdcp2x.size == 0) {
        version_select = NxClient_HdcpVersion_eHdcp1x;
    }

    switch (version_select) {
    case NxClient_HdcpVersion_eAuto:
        hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_eAuto;
        if (session->hdcp.version_state != nxserver_hdcp_begin) {
            /* second pass */
            hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType0;
        }
        else {
            /* first pass */
            hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType1;
        }
        break;
    case NxClient_HdcpVersion_eHdcp1x:
        hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_e1_x;
        /* hdcp2xContentStreamControl is a don't care */
        break;
    case NxClient_HdcpVersion_eAutoHdcp22Type0:
        hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_eAuto;
        hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType0;
        break;
    case NxClient_HdcpVersion_eHdcp22:
        hdmiOutputHdcpSettings.hdcp_version = NEXUS_HdmiOutputHdcpVersion_e2_2;
        hdmiOutputHdcpSettings.hdcp2xContentStreamControl = NEXUS_Hdcp2xContentStream_eType1;
        break;
    case NxClient_HdcpVersion_eMax:
        BERR_TRACE(NEXUS_INVALID_PARAMETER);
        return;
    }

    if (session->hdcpKeys.hdcp1x.size && hdmiOutputHdcpSettings.hdcp_version != NEXUS_HdmiOutputHdcpVersion_e2_2) {
        BKNI_Memcpy(hdmiOutputHdcpSettings.aksv.data, session->hdcpKeys.hdcp1x.buffer, sizeof(hdmiOutputHdcpSettings.aksv.data));
        BKNI_Memcpy(&hdmiOutputHdcpSettings.encryptedKeySet, &((uint8_t*)session->hdcpKeys.hdcp1x.buffer)[sizeof(hdmiOutputHdcpSettings.aksv.data)+3], sizeof(hdmiOutputHdcpSettings.encryptedKeySet));
    }
    else {
        BKNI_Memset(hdmiOutputHdcpSettings.aksv.data, 0, sizeof(hdmiOutputHdcpSettings.aksv.data));
        BKNI_Memset(&hdmiOutputHdcpSettings.encryptedKeySet, 0, sizeof(hdmiOutputHdcpSettings.encryptedKeySet));
    }

    hdmiOutputHdcpSettings.stateChangedCallback.callback = hdmiOutputHdcpStateChanged;
    hdmiOutputHdcpSettings.stateChangedCallback.context = session;
    rc = NEXUS_HdmiOutput_SetHdcpSettings(session->hdmiOutput, &hdmiOutputHdcpSettings);
    if (rc) {
        BDBG_ERR(("Error setting Hdcp1x keys. HDCP1.x will not work."));
        /* fall through */
    }

    if (session->hdcpKeys.hdcp2x.size) {
        rc = NEXUS_HdmiOutput_SetHdcp2xBinKeys(session->hdmiOutput, session->hdcpKeys.hdcp2x.buffer, session->hdcpKeys.hdcp2x.size);
        if (rc) {
            BDBG_ERR(("Error setting Hdcp2x encrypted keys. HDCP2.x will not work."));
            /* fall through for HDCP 1.x */
        }
    }
#else
    BSTD_UNUSED(session);
    BSTD_UNUSED(version_select);
#endif
}
#endif

void nxserver_get_default_settings(struct nxserver_settings *settings)
{
    unsigned i;
    memset(settings, 0, sizeof(*settings));
    settings->cursor = false;
    settings->framebuffers = 0;
    settings->pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
    settings->client_mode = NEXUS_ClientMode_eMax; /* don't change */
#if NEXUS_HAS_HDMI_OUTPUT
    settings->hdmi.dolbyVision.blendInIpt = true;
    settings->hdcp.immediateRetries = 25;
#endif
    settings->display.display3DSettings.orientation = NEXUS_VideoOrientation_e2D;
    settings->display.format = NEXUS_VideoFormat_eUnknown; /* use HDMI preferred format, else 720p if supported, else SD */
    settings->display.defaultSdFormat = NEXUS_VideoFormat_eMax; /* allow cmdline or pick default in init_session() */
    settings->display.graphicsSettings.alpha = 0xFF;
    settings->display.graphicsSettings.enabled = true;
    settings->display.graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
    settings->display.graphicsSettings.destBlendFactor = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
    settings->display.graphicsSettings.constantAlpha = 0xFF;
    for (i=0;i<NXCLIENT_MAX_DISPLAYS-1;i++) {
        settings->display.slaveDisplay[i].format = NEXUS_VideoFormat_eNtsc;
        settings->display.slaveDisplay[i].graphicsSettings = settings->display.graphicsSettings;
    }
    settings->display.componentPreferences.enabled = true;
    settings->display.compositePreferences.enabled = true;
    settings->display.compositePreferences.rfmChannel = 3;
#if NEXUS_HAS_HDMI_OUTPUT
    settings->display.hdmiPreferences.enabled = true;
    settings->display.hdmiPreferences.followPreferredFormat = true;
    settings->display.hdmiPreferences.preventUnsupportedFormat = true;
    settings->display.hdmiPreferences.colorSpace = NEXUS_ColorSpace_eAuto;
    settings->display.hdmiPreferences.colorDepth = 0;
    settings->display.hdmiPreferences.matrixCoefficients = NEXUS_MatrixCoefficients_eMax; /* means input */
    settings->display.dropFrame = NEXUS_TristateEnable_eEnable;
    settings->display.priority = NEXUS_DisplayPriority_eAuto;
    nxserverlib_dyrnng_p_get_default_settings(&settings->display);
#endif
#if NEXUS_NUM_VIDEO_ENCODERS
    settings->transcode = nxserver_transcode_on;
#endif
    settings->fbsize.width = 1920;
    settings->fbsize.height = 1080;
    for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
        if (i == 0) {
/* For generic platforms which do not have BCG-like displays */
#if (BCHP_CHIP != 11360)
            settings->session[i].output.hd = true;
            settings->session[i].output.sd = true;
#else
            settings->session[i].output.hd = false;
            settings->session[i].output.sd = false;
#endif
#if NEXUS_HAS_IR_INPUT
            settings->session[i].ir_input.mode[0] = NEXUS_IrInputMode_eCirNec; /* silver */
#endif
            settings->session[i].evdevInput = true;
            settings->session[i].keypad = false;
            settings->session[i].audioPlaybacks = NEXUS_NUM_AUDIO_PLAYBACKS; /* by default, give all to session 0 */
        }
        else {
            settings->session[i].output.encode = true;
#if NEXUS_HAS_IR_INPUT
            if (i == 1) {
                /* unfortunately, eCirNec doesn't work with eRemoteA on the same channel. */
                settings->session[i].ir_input.mode[0] = NEXUS_IrInputMode_eRemoteA; /* black */
            }
            else {
                settings->session[i].ir_input.mode[0] = NEXUS_IrInputMode_eMax; /* none */
            }
#endif
        }
        settings->session[i].ir_input.mode[1] = NEXUS_IrInputMode_eMax; /* none */
    }
    settings->native_3d = true;
    settings->standby_timeout = 10;
    /* for now, default to 0: settings->growHeapBlockSize = 8*1024*1024; */
    settings->grab = true;
    settings->thermal.thermal_config_file = "nxclient/thermal.cfg";
}

bool nxserver_p_video_only_display(struct b_session *session, unsigned displayIndex)
{
    return session->display[displayIndex].created_by_encoder && session->server->settings.session[session->index].output.encode_video_only;
}

#if NEXUS_HAS_AUDIO
static bool nxserver_p_has_spdif_output(struct b_session *session)
{
    return nxserverlib_p_session_has_sd_audio(session) && session->server->platformConfig.outputs.spdif[0];
}
#endif

/* a local display is one that can be connected to HDMI/component/composite/rfm outputs */
static bool is_local_display(nxserver_t server, unsigned displayIndex)
{
    /* A local display can be used for output.hd or output.sd.
    It must support video, graphics and not be for encode */
    /* with one exception: allow graphics-only SD */
    return (server->display.cap.display[displayIndex].numVideoWindows || displayIndex == 1) &&
           server->display.cap.display[displayIndex].graphics.width &&
           !server->platformCap.display[displayIndex].encoder;
}

/* if HD is 50Hz, default for SD should also be 50Hz */
static NEXUS_VideoFormat nxserver_p_default_sd_format(struct b_session *session)
{
    if ((session->display[0].formatInfo.verticalFreq && session->display[0].formatInfo.verticalFreq%2500 == 0) ||
        !session->server->display.cap.displayFormatSupported[NEXUS_VideoFormat_eNtsc]) {
        return NEXUS_VideoFormat_ePal;
    }
    else {
        return NEXUS_VideoFormat_eNtsc;
    }
}

#if BRDC_USE_CAPTURE_BUFFER && NEXUS_MODE_proxy
#include "bdbg_fifo.h"
static bool g_bExit = false;
NEXUS_ThreadHandle rulCaptureThread = NULL;
static void read_rul_capture_fifo(void *context) {
    NEXUS_PlatformStatus status;
    void *buffer;
    BDBG_FifoReader_Handle fifoReader;
    int rc;
    static FILE *file = NULL;
    static int filesize = 0;
    const char *filename = NEXUS_GetEnv("capture_ruls");
    BSTD_UNUSED(context);

    rc = NEXUS_Platform_GetStatus(&status);
    BDBG_ASSERT(!rc);
    if (!status.displayModuleStatus.rulCapture.memory) {
        BDBG_ERR(("missing rul capture memory"));
        return;
    }
    NEXUS_MemoryBlock_Lock(status.displayModuleStatus.rulCapture.memory, &buffer);
    rc = BDBG_FifoReader_Create(&fifoReader, buffer);
    BDBG_ASSERT(!rc);
open_newfile:
    if(filename) {
        if (!file) {
            static int filecnt = 0;
            char buf[256];
            BKNI_Snprintf(buf, 256, "nxs_%s%d", filename, filecnt++);
            file = fopen(buf, "w+");
            BDBG_WRN(("opened rul log file %s", buf));
        }
    }
    while (!g_bExit) {
#define BUFSIZE 4096
        char buf[BUFSIZE];
        int rc;

        /* simplistic state machine. you may want something more sophisticated. */
        rc = BDBG_FifoReader_Read(fifoReader, buf, BUFSIZE);
        switch(rc) {
        case BERR_SUCCESS:
            BDBG_MSG(("rul capture: got %d bytes", BUFSIZE));
            if(file) {
                fwrite(buf, BUFSIZE, 1, file);
                filesize += BUFSIZE;
            }
            break;
        case BERR_FIFO_NO_DATA:
        case BERR_FIFO_BUSY:
            BKNI_Sleep(100);
            break;
        case BERR_FIFO_OVERFLOW:
            BERR_TRACE(rc);
            break;
        default:
            BERR_TRACE(rc);
            break;
        }
        if (filesize > 200*1024*1024) {
           if(file) fclose(file);
           file = NULL;
           filesize = 0;
           /* on the next write, a new file will be opened. */
           goto open_newfile;
        }
    }
    BDBG_FifoReader_Destroy(fifoReader);
    if(file) fclose(file);
    file = NULL;
    filesize = 0;
    /* on the next write, a new file will be opened. */
}
#endif

#if NEXUS_HAS_HDMI_OUTPUT
static void nxserver_p_assign_hdmi_output(struct b_session *session, NEXUS_VideoFormat *format)
{
    nxserver_t server = session->server;
    if (session->index < NEXUS_NUM_HDMI_OUTPUTS && server->settings.session[session->index].output.hd) {
        int rc;

        session->hdmiOutput = server->platformConfig.outputs.hdmi[session->index];
        if (format && *format == NEXUS_VideoFormat_eUnknown && session->server->settings.display.hdmiPreferences.followPreferredFormat) {
            NEXUS_HdmiOutputStatus status;
            rc = NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &status);
            if (!rc && status.connected) {
                *format = status.preferredVideoFormat;
            }
        }
    }
}

static void nxserver_p_init_hdmi_output(struct b_session *session)
{
    NEXUS_HdmiOutputSettings hdmiSettings;
    NEXUS_HdmiSpdInfoFrame hdmiSpdInfoFrame;
    int rc;

    if (!session->hdmiOutput) {
        BDBG_WRN(("no HDMI output for session %d", session->index));
        return;
    }

    session->hdcp.version_select = session->server->settings.hdcp.versionSelect;
    NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = hotplug_callback;
    hdmiSettings.hotplugCallback.context = session;
    hdmiSettings.colorDepth = session->server->settings.display.hdmiPreferences.colorDepth ;
    hdmiSettings.colorSpace = session->server->settings.display.hdmiPreferences.colorSpace ;
    hdmiSettings.enableOnlySupportedFormats = (session->server->settings.display.hdmiPreferences.preventUnsupportedFormat && !session->server->settings.hdmi.ignoreVideoEdid);
    rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiSettings);
    if (rc) rc = BERR_TRACE(rc);

    session->audioSettings.hdmi.channelStatusInfo = hdmiSettings.audioChannelStatusInfo;

    rc = NEXUS_HdmiOutput_GetSpdInfoFrame(session->hdmiOutput, &hdmiSpdInfoFrame);
    if (rc) {
        rc = BERR_TRACE(rc); /* keep going */
    }
    else {
        if (session->server->settings.hdmi.spd.vendorName[0]) {
            strncpy((char*)hdmiSpdInfoFrame.vendorName, session->server->settings.hdmi.spd.vendorName, sizeof(hdmiSpdInfoFrame.vendorName));
        }
        if (session->server->settings.hdmi.spd.description[0]) {
            strncpy((char*)hdmiSpdInfoFrame.description, session->server->settings.hdmi.spd.description, sizeof(hdmiSpdInfoFrame.description));
        }
        rc = NEXUS_HdmiOutput_SetSpdInfoFrame(session->hdmiOutput, &hdmiSpdInfoFrame);
        if (rc) BERR_TRACE(rc); /* keep going */
    }

    nxserver_load_hdcpkey_files(session);
    nxserverlib_dynrng_p_session_initialized(session);
}
#else
static void nxserver_p_assign_hdmi_output(struct b_session *session, NEXUS_VideoFormat *format)
{
    BSTD_UNUSED(session);
    BSTD_UNUSED(format);
}
static void nxserver_p_init_hdmi_output(struct b_session *session)
{
    BSTD_UNUSED(session);
}
#endif

static int init_session(nxserver_t server, unsigned index)
{
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    NEXUS_DisplaySettings displaySettings;
    struct b_session *session;
    int rc;
    unsigned session_display_index = 0;
    NEXUS_VideoFormat hdmiOutputFormat = NEXUS_VideoFormat_eUnknown;

    BDBG_MSG(("init_session %d", index));
    session = BKNI_Malloc(sizeof(*session));
    if (!session) {
        return BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY);
    }
    BKNI_Memset(session, 0, sizeof(*session));
    session->server = server;
    session->index = index;
    session->audioSettings.dac.outputMode = NxClient_AudioOutputMode_ePcm;
    session->audioSettings.dac.transcodeCodec = NEXUS_AudioCodec_eMax;
    session->audioSettings.i2s[0].outputMode = NxClient_AudioOutputMode_ePcm;
    session->audioSettings.i2s[0].transcodeCodec = NEXUS_AudioCodec_eMax;
    session->audioSettings.i2s[1].outputMode = NxClient_AudioOutputMode_ePcm;
    session->audioSettings.i2s[1].transcodeCodec = NEXUS_AudioCodec_eMax;
    session->audioSettings.hdmi.outputMode = NxClient_AudioOutputMode_eAuto;
    session->audioSettings.hdmi.transcodeCodec = NEXUS_AudioCodec_eAc3Plus;
    session->audioSettings.spdif.outputMode = NxClient_AudioOutputMode_eAuto;
    session->audioSettings.spdif.transcodeCodec = NEXUS_AudioCodec_eAc3;
    session->audioSettings.rfm.outputMode = NxClient_AudioOutputMode_ePcm;
    session->audioSettings.rfm.transcodeCodec = NEXUS_AudioCodec_eMax;
#if NEXUS_HAS_AUDIO
    NEXUS_AudioModule_GetLoudnessSettings(&session->audioSettings.loudnessSettings);
    bserver_set_default_audio_settings(session);
#endif

    if (server->settings.externalApp.enabled) {
        if (index != 0) {
            return BERR_TRACE(NEXUS_NOT_SUPPORTED);
        }
        for (session_display_index=0;session_display_index<NXCLIENT_MAX_DISPLAYS;session_display_index++) {
            if (!server->settings.externalApp.display[session_display_index].handle) break;
            session->display[session_display_index].global_index = server->global_display_index;
            server->global_display_index++;
            session->display[session_display_index].display = server->settings.externalApp.display[session_display_index].handle;
            NEXUS_Display_GetSettings(session->display[session_display_index].display, &displaySettings);
            NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[session_display_index].formatInfo);
            /* inherit settings instead of setting them */
            if (session_display_index == 0) {
                session->server->settings.display.format = displaySettings.format;
                session->server->settings.display.aspectRatio = displaySettings.aspectRatio;
            }
            else {
                session->server->settings.display.slaveDisplay[session_display_index-1].format = displaySettings.format;
            }
        }
        session->numWindows = NEXUS_NUM_VIDEO_WINDOWS;
        session->window[0].capabilities.deinterlaced = true; /* for now, only one MAD for session 0 main */

        if (server->settings.externalApp.video_outputs) {
            nxserver_p_assign_hdmi_output(session, NULL);
            nxserver_p_init_hdmi_output(session);
        }
        goto after_display_open;
    }

    /*
     * Set up primary session display
     */
    session_display_index = 0;

    if (IS_SESSION_DISPLAY_ENABLED(server->settings.session[index]) && is_local_display(server, server->global_display_index)) {
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        if (session->index == 0) {
            displaySettings.format = session->server->settings.display.format;
        }
        session->display[session_display_index].priority = NEXUS_DisplayPriority_eAuto;

        nxserver_p_assign_hdmi_output(session, &displaySettings.format);
        if (session->index == 0) {
            NEXUS_VideoFormat bvnFormat;
            if (displaySettings.format == NEXUS_VideoFormat_eUnknown) {
                if (server->display.cap.displayFormatSupported[NEXUS_VideoFormat_e720p]) {
                    displaySettings.format = NEXUS_VideoFormat_e720p;
                }
                else {
                    displaySettings.format = NEXUS_VideoFormat_e720p50hz;
                }
            }
            bvnFormat = nxserver_p_supported_bvn_format(session, displaySettings.format);
            if (!bvnFormat) {
                NEXUS_VideoFormat prev_format = displaySettings.format;
                /* if format is not supported, revert to SD which matches the framerate of default HD format */
                NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[session_display_index].formatInfo);
                displaySettings.format = nxserver_p_default_sd_format(session);
                BDBG_WRN(("default display format %s not supported, switching to %s",
                    lookup_name(g_videoFormatStrs, prev_format),
                    lookup_name(g_videoFormatStrs, displaySettings.format)));
                bvnFormat = displaySettings.format;
            }
            hdmiOutputFormat = displaySettings.format;
            displaySettings.format = bvnFormat;
        }
        displaySettings.aspectRatio = session->server->settings.display.aspectRatio;
        displaySettings.dropFrame = session->server->settings.display.dropFrame;
		displaySettings.dynamicRangeMode = session->server->settings.display.hdmiPreferences.dynamicRangeMode;
        session->display[session_display_index].global_index = server->global_display_index;
        session->display[session_display_index].display = NEXUS_Display_Open(server->global_display_index, &displaySettings);
        server->global_display_index++;
        if (!session->display[session_display_index].display) {
            BDBG_ERR(("unable to open session %d, display %d, display %d", session->index, session_display_index, session->display[session_display_index].global_index));
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[session_display_index].formatInfo);
        session->numWindows = NEXUS_NUM_VIDEO_WINDOWS;
        session->window[0].capabilities.deinterlaced = true; /* for now, only one MAD for session 0 main */

        nxserver_p_init_hdmi_output(session);

        session_display_index++;
    }

#if NEXUS_NUM_DISPLAYS > 1
    /*
     * Set-up session slave display: when session is requested with additional displays
     * (e.g. HD/SD, HD/Transcode or SD/Transcode)
     */
#if NEXUS_NUM_DSP_VIDEO_ENCODERS
    if (server->settings.session[index].output.sd && server->settings.transcode == nxserver_transcode_on) {
        BDBG_WRN(("display 1 is reserved for DSP transcode, so it cannot be a slave display"));
        server->settings.session[index].output.sd = false;
    }
#endif
    if (server->settings.allowCompositionBypass) {
        /* if bypass, only allow main display */
        server->settings.session[index].output.sd = false;
    }
    if (server->settings.session[index].output.hd && server->settings.session[index].output.sd && is_local_display(server, server->global_display_index)) {
        if (session->server->settings.display.slaveDisplay[session_display_index-1].format == NEXUS_VideoFormat_eNtsc) {
            session->server->settings.display.slaveDisplay[session_display_index-1].format = nxserver_p_default_sd_format(session);
        }
        NEXUS_Display_GetDefaultSettings(&displaySettings);
        displaySettings.format = session->server->settings.display.slaveDisplay[session_display_index-1].format;
        session->display[session_display_index].global_index = server->global_display_index;
        if (server->settings.display_init.sd.dedicatedTimebase) {
            nxserver_p_reserve_timebase(NEXUS_Timebase_e1);
            displaySettings.timebase = NEXUS_Timebase_e1;
        }
        session->display[session_display_index].display = NEXUS_Display_Open(server->global_display_index, &displaySettings);
        server->global_display_index++;
        if (session->display[session_display_index].display) {
            NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[session_display_index].formatInfo);
        }
        else {
            BDBG_WRN(("unable to open display 1 as slave display"));
            /* but don't fail */
        }
        session->display[session_display_index].graphicsOnly = (server->display.cap.display[session_display_index].numVideoWindows == 0);
        session_display_index++;
    }
#endif

    if (session->display[1].display && !session->display[1].graphicsOnly) {
        /* On an HD/SD simul system, SD outputs will be attached to the SD display, so there's
        no value in defaultSdFormat. */
        if (server->settings.display.defaultSdFormat != NEXUS_VideoFormat_eUnknown &&
            server->settings.display.defaultSdFormat != NEXUS_VideoFormat_eMax) {
            BDBG_WRN(("forcing defaultSdFormat off for HD/SD simul system"));
        }
        server->settings.display.defaultSdFormat = NEXUS_VideoFormat_eUnknown;
    }
    else if (server->settings.display.defaultSdFormat == NEXUS_VideoFormat_eMax) {
        /* if have component, default off. else default on. */
        if (server->platformConfig.outputs.component[0]) {
            server->settings.display.defaultSdFormat = NEXUS_VideoFormat_eUnknown;
        }
        else {
            server->settings.display.defaultSdFormat = nxserver_p_default_sd_format(session);
        }
    }

    /*
     * Set-up session slave display
     */

    if (server->settings.session[index].output.encode) {
#if NEXUS_NUM_VIDEO_ENCODERS
        struct video_encoder_status status;
        session->encoder.encoder = video_encoder_create(server->settings.session[index].output.encode_video_only, session, NULL);
        if (!session->encoder.encoder) {
            rc = BERR_TRACE(NEXUS_NOT_AVAILABLE);
            goto error;
        }

        video_encoder_get_status(session->encoder.encoder, &status);
        session->display[session_display_index].display = status.display;
        session->display[session_display_index].global_index = status.displayIndex;
        session->display[session_display_index].created_by_encoder = true;
        NEXUS_Display_GetSettings(status.display, &displaySettings);
        NEXUS_VideoFormat_GetInfo(displaySettings.format, &session->display[session_display_index].formatInfo);
        session->numWindows = 1; /* no PIP if encoding the display */
        session_display_index++;
#else
        rc = BERR_TRACE(NEXUS_NOT_SUPPORTED);
        goto error;
#endif
    }

after_display_open:
#if NEXUS_HAS_VIDEO_DECODER
    /* enable video-as-graphics with main display */
    if (index == 0 && session->display[0].display) {
        NEXUS_Display_DriveVideoDecoder(session->display[0].display);
    }
#endif

    BKNI_CreateEvent(&session->inactiveEvent);

#if NEXUS_HAS_SIMPLE_DECODER
    session->video.server = NEXUS_SimpleVideoDecoderServer_Create();
    session->audio.server = NEXUS_SimpleAudioDecoderServer_Create();
    session->audio.playbackServer = NEXUS_SimpleAudioPlaybackServer_Create();
    session->encoder.server = NEXUS_SimpleEncoderServer_Create();
#endif
#if NEXUS_HAS_AUDIO
    if (!server->settings.externalApp.enabled) {
        /* must create audio after session->hdmiOutput is set */
        /* and don't create main_audio for headless systems */
        if (session->display[0].display) {
            b_audio_decoder_create_settings create_settings;
            audio_decoder_get_default_create_settings(&create_settings);
            session->main_audio = audio_decoder_create(session, b_audio_decoder_type_regular, &create_settings);
            if (!session->main_audio) {rc = BERR_TRACE(-1); goto error;}
        }
    }
    if (nxserver_p_has_spdif_output(session)) {
        NEXUS_SpdifOutputSettings settings;
        NEXUS_SpdifOutput_GetSettings(session->server->platformConfig.outputs.spdif[0], &settings);
        session->audioSettings.spdif.channelStatusInfo = settings.channelStatusInfo;
        session->audioSettings.spdif.ditherEnabled = settings.dither;
    }
#endif

#if (BCHP_CHIP == 11360)
/* Ignore check for display if session_display_index is 0 for non-BCG displays -
 * this check was introduced first in URSR 15.1.
 */
    if (session->display[0].display || !session_display_index) {
#else
    if (session->display[0].display) {
#endif
        unsigned i;
        session->surfaceCompositor = NEXUS_SurfaceCompositor_Create(0);
        NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
        for (i=0;i<session_display_index;i++) {
            BDBG_ASSERT(session->display[i].display);
            if (nxserver_p_video_only_display(session, i)) {
                continue;
            }
            NEXUS_Display_GetGraphicsSettings(session->display[i].display, &surface_compositor_settings.display[i].graphicsSettings);
            surface_compositor_settings.display[i].graphicsSettings.enabled = true;
            surface_compositor_settings.display[i].display = session->display[i].display;
            if (i == 0) {
                surface_compositor_settings.display[i].framebuffer.pixelFormat = server->settings.pixelFormat;
            }
            if (i == 0 || session->display[i].created_by_encoder) {
                /* HD or encode display */
                surface_compositor_settings.display[i].framebuffer.width = MIN(server->settings.fbsize.width, server->display.cap.display[session->display[i].global_index].graphics.width);
                surface_compositor_settings.display[i].framebuffer.height = MIN(server->settings.fbsize.height, server->display.cap.display[session->display[i].global_index].graphics.height);
            }
            else {
                /* NTSC or PAL SD display */
                if (!server->settings.display_init.sd.graphicsPosition.width) {
                    surface_compositor_settings.display[i].framebuffer.width = session->display[i].formatInfo.width;
                    surface_compositor_settings.display[i].framebuffer.height = session->display[i].formatInfo.height;
                }
                else {
                    surface_compositor_settings.display[i].manualPosition = true;
                    surface_compositor_settings.display[i].graphicsSettings.position = server->settings.display_init.sd.graphicsPosition;
                    surface_compositor_settings.display[i].framebuffer.width = surface_compositor_settings.display[i].graphicsSettings.position.width;
                    surface_compositor_settings.display[i].framebuffer.height = surface_compositor_settings.display[i].graphicsSettings.position.height;
                }
            }
            surface_compositor_settings.display[i].graphicsSettings.clip.width = MIN(surface_compositor_settings.display[i].framebuffer.width,session->display[i].formatInfo.width);
            surface_compositor_settings.display[i].graphicsSettings.clip.height = MIN(surface_compositor_settings.display[i].framebuffer.height,session->display[i].formatInfo.height);
            surface_compositor_settings.display[i].framebuffer.backgroundColor = 0x00000000; /* transparent background */
            surface_compositor_settings.display[i].framebuffer.heap = nxserver_p_framebuffer_heap(session, i, false);
        }
        surface_compositor_settings.inactiveCallback.callback = b_surface_compositor_inactive;
        surface_compositor_settings.inactiveCallback.context = session;
        if (server->settings.framebuffers) {
            surface_compositor_settings.display[0].framebuffer.number = server->settings.framebuffers;
        }
        rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
        BDBG_ASSERT(!rc);
        if(server->settings.cursor) {
            NEXUS_SurfaceCreateSettings settings;
            NEXUS_SurfaceCursorCreateSettings cursorSettings;
            NEXUS_SurfaceCursorSettings config;

            NEXUS_Surface_GetDefaultCreateSettings(&settings);

            settings.width = 8;
            settings.height = 8;
            session->cursor.surface = NEXUS_Surface_Create(&settings);
            BDBG_ASSERT(session->cursor.surface);
            make_cursor(session->cursor.surface, &settings);
            NEXUS_SurfaceCursor_GetDefaultCreateSettings(&cursorSettings);

            cursorSettings.surface = session->cursor.surface;
            session->cursor.cursor = NEXUS_SurfaceCursor_Create(session->surfaceCompositor, &cursorSettings);
            BDBG_ASSERT(session->cursor.cursor);
            NEXUS_SurfaceCursor_GetSettings(session->cursor.cursor, &config);
            config.composition.visible = true;
            config.composition.virtualDisplay.width  = 400;
            config.composition.virtualDisplay.height = 200;
            config.composition.position.x = 100;
            config.composition.position.y = 100;
            config.composition.position.width = 8;
            config.composition.position.height = 8;
            NEXUS_SurfaceCursor_SetSettings(session->cursor.cursor, &config);
        }
    }

    if (session->display[0].display) {
        /* copy settings already set, then allow NxClient_P_SetDisplaySettings to make changes */
        NxClient_DisplaySettings settings = session->server->settings.display;
        if (hdmiOutputFormat) {
            settings.format = hdmiOutputFormat;
        }
        else {
            NEXUS_DisplaySettings displaySettings;
            NEXUS_Display_GetSettings(session->display[0].display, &displaySettings);
            settings.format = displaySettings.format;
        }
#if NEXUS_HAS_HDMI_OUTPUT
        settings.hdmiPreferences.version = session->hdcp.version_select;
#endif
        rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &settings);
        if (rc) {
#if NEXUS_HAS_HDMI_OUTPUT
            if (session->hdmiOutput) {
                NEXUS_HdmiOutputStatus status;
                /* if specified format is supported by chip but not TV, revert to preferred and try one more time.
                better to start with a different format than not to start. */
                if (!NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &status) && status.connected) {
                    settings.format = nxserver_p_supported_bvn_format(session, status.preferredVideoFormat);
                    if (!settings.format) {
                        settings.format = nxserver_p_default_sd_format(session);
                        settings.hdmiPreferences.colorSpace = NEXUS_ColorSpace_eAuto;
                        settings.hdmiPreferences.colorDepth = 0;
                    }
                    rc = NxClient_P_SetDisplaySettingsNoRollback(NULL, session, &settings);
                    if (rc) BERR_TRACE(rc); /* handle rc below */
                }
            }
#endif
            /* nxserver must come up, even if HDMI has problems */
            if (rc) {
                /* if even NxClient_P_SetDisplaySettingsNoRollback fails, we still want to store settings and go forward. */
                session->nxclient.displaySettings = settings;
            }
        }
    }
    init_input_devices(session);

#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        /* force call to hotplug handler to configure HDMI audio outputs and preferred video format */
        hotplug_callback_locked(session, 0);
    }
#endif

#if BRDC_USE_CAPTURE_BUFFER && NEXUS_MODE_proxy /* user mode rul is captured in nexus_display_module */
    if(NEXUS_GetEnv("capture_ruls")) {
        g_bExit = false;
        rulCaptureThread = NEXUS_Thread_Create("rulCapture", read_rul_capture_fifo, NULL, NULL);
    }
#endif
    BDBG_ASSERT(!server->session[index]);
    server->session[index] = session;
    return 0;

error:
    uninit_session(session);
    return rc;
}

static void uninit_session(struct b_session *session)
{
    unsigned i;
    BDBG_MSG(("uninit_session %d", session->index));
#if BRDC_USE_CAPTURE_BUFFER && NEXUS_MODE_proxy
    if(rulCaptureThread) {
        g_bExit = true;
        BKNI_Sleep(1000);
        NEXUS_Thread_Destroy(rulCaptureThread);
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        NEXUS_StopCallbacks(session->hdmiOutput);
    }
#endif
    if (session->main_audio) {
        audio_decoder_destroy(session->main_audio);
    }
    uninit_input_devices(session);
    if(session->cursor.cursor) {
        NEXUS_SurfaceCursor_Destroy(session->cursor.cursor);
        NEXUS_Surface_Destroy(session->cursor.surface);
    }
    if (session->surfaceCompositor) {
        NEXUS_SurfaceCompositor_Destroy(session->surfaceCompositor);
    }
    if (session->inactiveEvent) {
        BKNI_DestroyEvent(session->inactiveEvent);
    }
    uninit_session_video(session);
    if (!session->server->settings.externalApp.enabled) {
        for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
            if (session->display[i].display && !session->display[i].created_by_encoder) {
                NEXUS_Display_Close(session->display[i].display);
            }
        }
    }
    for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
        if (session->display[i].graphic) {
            NEXUS_Surface_Destroy(session->display[i].graphic);
        }
    }
#if NEXUS_NUM_VIDEO_ENCODERS
    if (session->encoder.encoder) {
        video_encoder_destroy(session->encoder.encoder);
    }
#endif
#if NEXUS_HAS_SIMPLE_DECODER
    if (session->video.server) {
        NEXUS_SimpleVideoDecoderServer_Destroy(session->video.server);
    }
    if (session->audio.server) {
        NEXUS_SimpleAudioDecoderServer_Destroy(session->audio.server);
    }
    if (session->audio.playbackServer) {
        NEXUS_SimpleAudioPlaybackServer_Destroy(session->audio.playbackServer);
    }
    if (session->encoder.server) {
        NEXUS_SimpleEncoderServer_Destroy(session->encoder.server);
    }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
    nxserver_clear_hdcpkeys(session);
#endif
    session->server->session[session->index] = NULL;
    BKNI_Free(session);
}

static nxserver_t g_server = NULL;
nxserver_t nxserverlib_get_singleton(void)
{
    return g_server;
}

void nxserverlib_get_settings(nxserver_t server, struct nxserver_settings *settings)
{
    *settings = server->settings;
}

nxserver_t nxserverlib_init(const struct nxserver_settings *settings)
{
    NEXUS_PlatformStartServerSettings serverSettings;
    NEXUS_Error rc;
    nxserver_t server;
    unsigned i;
    unsigned sd_count = 0;

    BDBG_CASSERT(NEXUS_SURFACE_COMPOSITOR_MAX_DISPLAYS == NXCLIENT_MAX_DISPLAYS);
    server = BKNI_Malloc(sizeof(*server));
    if (!server) {BERR_TRACE(NEXUS_OUT_OF_SYSTEM_MEMORY); return NULL;}

    memset(server, 0, sizeof(*server));

    NEXUS_Platform_GetSettings(&server->platformSettings);
    NEXUS_Platform_GetConfiguration(&server->platformConfig);
    NEXUS_Platform_GetStatus(&server->platformStatus);
    NEXUS_GetPlatformCapabilities(&server->platformCap);
    server->settings = *settings;

    /* 0 is reserved for "no id". intentionally start with wraparound being near to flush out app assumptions. */
    for (i=0;i<b_resource_max;i++) {
        server->nextId[i] = 1; /* to avoid dependency on id's, use: (unsigned)-10; */
    }

    BKNI_AcquireMutex(server->settings.lock);
    rc = stc_pool_init(server);
    if (rc) {goto error;}

    NEXUS_GetDisplayCapabilities(&server->display.cap);
    if (!server->settings.externalApp.enabled) {
        rc = video_init(server);
        if (rc) {goto error;}

        rc = audio_init(server);
        if (rc) {goto error;}
    }

    if (server->display.cap.display[0].numVideoWindows == 0 || server->platformCap.display[0].encoder) {
        BDBG_WRN(("******************************************************************************************"));
        BDBG_WRN(("* This is a headless system, so only encoder-based apps are supported.                   *"));
        BDBG_WRN(("******************************************************************************************"));
        for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
            server->settings.session[i].output.hd = false;
            server->settings.session[i].output.sd = false;
        }
    }
    for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
        if (IS_SESSION_DISPLAY_ENABLED(server->settings.session[i])) {
            if (server->settings.session[i].output.hd) {
                if (sd_count > 0) {
                    BDBG_ERR(("hd session [%d] must be specified before sd sessions", i));
                    BERR_TRACE(NEXUS_INVALID_PARAMETER);
                    goto error;
                }
            }
            if (server->settings.session[i].output.sd) {
                sd_count++;
            }
            rc = init_session(server, i);
            if (rc) {
                rc = BERR_TRACE(rc);
                if (rc) {goto error;}
            }
        }
    }
    BKNI_ReleaseMutex(server->settings.lock);

    NEXUS_Platform_GetDefaultStartServerSettings(&serverSettings);
    serverSettings.allowUnauthenticatedClients = false;
    rc = NEXUS_Platform_StartServer(&serverSettings);
    BDBG_ASSERT(!rc);

    NEXUS_Platform_GetStandbySettings(&server->standby.standbySettings.settings);

    rc = nxserver_p_thermal_init(server);
    if (rc == NEXUS_NOT_AVAILABLE) {
        BDBG_WRN(("Thermal Monitoring is not supported on this platform"));
    } else {
        BDBG_ASSERT(!rc);
    }

    g_server = server;
    return server;

error:
    BKNI_ReleaseMutex(server->settings.lock);
    return NULL;
}

void nxserverlib_uninit(nxserver_t server)
{
    unsigned i;
    NxClient_StandbySettings standbySettings;

    nxserver_p_thermal_uninit(server);

    /* Bring server out of standby so that uninit can complete */
    standbySettings.settings.mode = NEXUS_PlatformStandbyMode_eOn;
    bserver_set_standby_settings(server, &standbySettings);

    /* stop the server before closing resources that may be in use by clients.
    if it's an untrusted client, handle verification may fail the call. but a trusted client bypasses the
    check and could kill the server-> */
    NEXUS_Platform_StopServer();

    nxserverlib_p_clear_video_cache();

    for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
        if (server->session[i]) {
            uninit_session(server->session[i]);
        }
    }
    video_uninit();
    audio_uninit();
    stc_pool_uninit(server);

    g_server = NULL;
    BKNI_Free(server);
}

void NxClient_P_GetDisplaySettings(nxclient_t client, struct b_session *session, NxClient_DisplaySettings *pSettings)
{
    if (!session) {
        session = client->session;
    }
    *pSettings = session->nxclient.displaySettings;
    if (client) {
        /* hdcp is per-client */
        pSettings->hdmiPreferences.hdcp = client->hdcp_level;
        pSettings->hdmiPreferences.version = client->hdcp_version;
    }
}


#if NEXUS_HAS_HDMI_OUTPUT
static void nxserverlib_p_apply_matrix_coeffs(const struct b_session * session, const NxClient_DisplaySettings * pSettings)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    if (session->hdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiOutputSettings;
        NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiOutputSettings);
        if
        (
            pSettings->hdmiPreferences.colorSpace != hdmiOutputSettings.colorSpace
            ||
            pSettings->hdmiPreferences.colorDepth != hdmiOutputSettings.colorDepth
            ||
            (
                /* want to set back to non-override (from input), and current settings have override set */
                (pSettings->hdmiPreferences.matrixCoefficients == NEXUS_MatrixCoefficients_eMax)
                &&
                hdmiOutputSettings.overrideMatrixCoefficients
            )
            ||
            (
                /* want to set to override, and current settings are not overridden yet */
                (pSettings->hdmiPreferences.matrixCoefficients != NEXUS_MatrixCoefficients_eMax)
                &&
                !hdmiOutputSettings.overrideMatrixCoefficients
            )
            ||
            (
                /* want to set to override, current settings are already overridden, but with a different set of coeffs */
                (pSettings->hdmiPreferences.matrixCoefficients != NEXUS_MatrixCoefficients_eMax)
                &&
                hdmiOutputSettings.overrideMatrixCoefficients
                &&
                pSettings->hdmiPreferences.matrixCoefficients != hdmiOutputSettings.matrixCoefficients
            )
        )
        {
            if (pSettings->hdmiPreferences.matrixCoefficients == NEXUS_MatrixCoefficients_eMax)
            {
                hdmiOutputSettings.overrideMatrixCoefficients = false;
            }
            else
            {
                hdmiOutputSettings.overrideMatrixCoefficients = true;
                hdmiOutputSettings.matrixCoefficients = pSettings->hdmiPreferences.matrixCoefficients;
            }
            BDBG_LOG(("changing HdmiOutput%d matrix coefficients to %s", session->index,
                pSettings->hdmiPreferences.matrixCoefficients == NEXUS_MatrixCoefficients_eMax ? "input" :
                lookup_name(g_matrixCoeffStrs, pSettings->hdmiPreferences.matrixCoefficients)));
            rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiOutputSettings);
            if (rc) BERR_TRACE(rc); /* fall through */
        }
    }
}
#endif

static void nxserver_p_set_sd_outputs(struct b_session *session, const NxClient_DisplaySettings *pSettings)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS || NEXUS_NUM_RFM_OUTPUTS || NEXUS_NUM_656_OUTPUTS
    nxserver_t server = session->server;
    NEXUS_DisplayHandle display;

    if (server->settings.externalApp.enabled && !server->settings.externalApp.video_outputs) return;

    /* SD outputs */
    if (server->settings.session[session->index].output.sd && pSettings->compositePreferences.enabled) {
        NEXUS_VideoFormatInfo info;
        bool hdDisplayisSdFormat;
        NEXUS_VideoFormat_GetInfo(pSettings->format, &info);
        hdDisplayisSdFormat = (info.height <= 576) && info.interlaced;
        /* for graphics-only SD system move SD outputs to HD display if HD display is in SD format */
        if (session->display[1].display && !(session->display[1].graphicsOnly && hdDisplayisSdFormat)) {
            display = session->display[1].display;
        }
        else {
            display = session->display[0].display;
        }
    }
    else {
        display = NULL;
    }

#if NEXUS_NUM_COMPOSITE_OUTPUTS
    if (server->platformConfig.outputs.composite[0]) {
        if ((session->composite.display != display) && session->composite.display) {
            NEXUS_Display_RemoveOutput(session->composite.display, NEXUS_CompositeOutput_GetConnector(server->platformConfig.outputs.composite[0]));
        }
        if ((session->composite.display != display) && display) {
            NEXUS_DisplayVbiSettings vbiSettings;
            NEXUS_Error rc;

            NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(server->platformConfig.outputs.composite[0]));
            NEXUS_Display_GetVbiSettings(display, &vbiSettings);
            vbiSettings.closedCaptionEnabled = true;
            rc = NEXUS_Display_SetVbiSettings(display, &vbiSettings);
            if (rc) BERR_TRACE(rc); /* keep going */
        }
    }
#endif
#if NEXUS_NUM_RFM_OUTPUTS
    if (server->platformConfig.outputs.rfm[0]) {
        if (pSettings->compositePreferences.enabled) {
            NEXUS_RfmSettings rfmSettings;
            NEXUS_Rfm_GetSettings(server->platformConfig.outputs.rfm[0], &rfmSettings);
            if (rfmSettings.channel != pSettings->compositePreferences.rfmChannel) {
                rfmSettings.channel = pSettings->compositePreferences.rfmChannel;
                NEXUS_Rfm_SetSettings(server->platformConfig.outputs.rfm[0], &rfmSettings);
            }
        }
        if ((session->composite.display != display) && session->composite.display) {
            NEXUS_Display_RemoveOutput(session->composite.display, NEXUS_Rfm_GetVideoConnector(server->platformConfig.outputs.rfm[0]));
        }
        if ((session->composite.display != display) && display) {
            NEXUS_Display_AddOutput(display, NEXUS_Rfm_GetVideoConnector(server->platformConfig.outputs.rfm[0]));
        }
    }
#endif
#if NEXUS_NUM_656_OUTPUTS
    /* NOTE: for 7216, both HDMI and 656 connect to display0 and nexus_video_outputs.c decides which is enabled based on HDMI connectivity. */
    if (server->platformConfig.outputs.ccir656[0]) {
        if ((session->composite.display != display) && session->composite.display) {
            NEXUS_Display_RemoveOutput(session->composite.display, NEXUS_Ccir656Output_GetConnector(server->platformConfig.outputs.ccir656[0]));
        }
        if ((session->composite.display != display) && display) {
            NEXUS_Display_AddOutput(display, NEXUS_Ccir656Output_GetConnector(server->platformConfig.outputs.ccir656[0]));
        }
    }
#endif
    session->composite.display = display;
#else
    BSTD_UNUSED(session);
    BSTD_UNUSED(pSettings);
#endif
}

static NEXUS_Error nxserver_p_wait_for_inactive(struct b_session *session)
{
    NEXUS_Error rc;
    unsigned i;
    for (i=0;i<5;i++) {
        NEXUS_SurfaceCompositorStatus status;
        rc = BKNI_WaitForEvent(session->inactiveEvent, 1000);
        if (!rc) {
            return NEXUS_SUCCESS;
        }
        rc = NEXUS_SurfaceCompositor_GetStatus(session->surfaceCompositor, &status);
        if (rc) return BERR_TRACE(rc);
        if (!status.active) {
            /* this can be caused by calling NxClient_SetDisplaySettings from a callback. */
            BDBG_WRN(("SurfaceCompositor inactive callback delayed"));
            return NEXUS_SUCCESS;
        }
    }
    return BERR_TRACE(NEXUS_NOT_AVAILABLE);
}

NEXUS_Error NxClient_P_SetDisplaySettingsNoRollback(nxclient_t client, struct b_session *session, const NxClient_DisplaySettings *pSettings)
{
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    int rc;
    nxserver_t server;
    NEXUS_DisplaySettings displaySettings;
    bool format_change = false;
    bool native_3d_active_before;
    unsigned i;
    bool waitForInactive = false;
    struct nxserver_display_format target_format;
    bool check_hdcp_flag = false;

    if (!session->display[0].display) {
        /* headless */
        return 0;
    }
    server = session->server;

    nxserver_p_get_target_format(session, pSettings, &target_format);
    native_3d_active_before = nxserverlib_p_native_3d_active(session);

    NEXUS_Display_GetSettings(session->display[0].display, &displaySettings);
    displaySettings.background = pSettings->backgroundColor;
    displaySettings.aspectRatio = pSettings->aspectRatio;
    displaySettings.sampleAspectRatio.x = pSettings->sampleAspectRatio.x;
    displaySettings.sampleAspectRatio.y = pSettings->sampleAspectRatio.y;
    if (!displaySettings.sampleAspectRatio.x || !displaySettings.sampleAspectRatio.y) {
        displaySettings.sampleAspectRatio.x = 1;
        displaySettings.sampleAspectRatio.y = 1;
    }
    displaySettings.dropFrame = pSettings->dropFrame;
    displaySettings.priority = pSettings->priority;
    rc = NEXUS_Display_SetSettings(session->display[0].display, &displaySettings);
    if (rc) return BERR_TRACE(rc);
    format_change = (displaySettings.format != target_format.videoFormat ||
        pSettings->display3DSettings.orientation != session->nxclient.displaySettings.display3DSettings.orientation ||
        pSettings->secure != session->nxclient.displaySettings.secure);
#if NEXUS_HAS_HDMI_OUTPUT
    if (!format_change && session->hdmiOutput) {
        NEXUS_HdmiOutputSettings hdmiOutputSettings;
        NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiOutputSettings);
        format_change =
            hdmiOutputSettings.colorSpace != target_format.colorSpace ||
            hdmiOutputSettings.colorDepth != target_format.colorDepth;
    }
#endif

    for (i=1;i<NXCLIENT_MAX_DISPLAYS;i++) {
        if (!session->display[i].display) continue;
        NEXUS_Display_GetSettings(session->display[i].display, &displaySettings);
        displaySettings.background = pSettings->slaveDisplay[i-1].backgroundColor;
        displaySettings.aspectRatio = pSettings->slaveDisplay[i-1].aspectRatio;
        displaySettings.sampleAspectRatio.x = pSettings->slaveDisplay[i-1].sampleAspectRatio.x;
        displaySettings.sampleAspectRatio.y = pSettings->slaveDisplay[i-1].sampleAspectRatio.y;
        if (!displaySettings.sampleAspectRatio.x || !displaySettings.sampleAspectRatio.y) {
            displaySettings.sampleAspectRatio.x = 1;
            displaySettings.sampleAspectRatio.y = 1;
        }
        rc = NEXUS_Display_SetSettings(session->display[i].display, &displaySettings);
        if (rc) return BERR_TRACE(rc);

        if (pSettings->slaveDisplay[i-1].format != session->nxclient.displaySettings.slaveDisplay[i-1].format ||
            pSettings->slaveDisplay[i-1].mode != session->nxclient.displaySettings.slaveDisplay[i-1].mode) {
            format_change = true;
        }
    }

    if (!format_change) {
        bool change = false;
        NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
        for (i=0;i<NXCLIENT_MAX_DISPLAYS;i++) {
            const NxClient_GraphicsSettings *pGraphicsSettings;
            if (i == 0) {
                pGraphicsSettings = &pSettings->graphicsSettings;
            }
            else {
                pGraphicsSettings = &pSettings->slaveDisplay[i-1].graphicsSettings;
            }
            if (nxserver_p_apply_graphics_settings(&surface_compositor_settings.display[i].graphicsSettings, pGraphicsSettings)) {
                change = true;
            }
            if( surface_compositor_settings.display[i].enabled != pGraphicsSettings->enabled) {
                surface_compositor_settings.display[i].enabled = pGraphicsSettings->enabled;
                change = true;
            }
        }
        if (surface_compositor_settings.allowCompositionBypass != session->server->settings.allowCompositionBypass) {
            surface_compositor_settings.allowCompositionBypass = session->server->settings.allowCompositionBypass;
            change = true;
        }
        if (change) {
            rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
            if (rc) return BERR_TRACE(rc);
        }
    }

#if NEXUS_HAS_HDMI_OUTPUT
    /* must validate format change as well as initial display format */
    if (format_change || !server->session[session->index]) {
        if (session->hdmiOutput) {
            NEXUS_HdmiOutputVendorSpecificInfoFrame vsi;
            if (pSettings->hdmiPreferences.preventUnsupportedFormat && !session->server->settings.hdmi.ignoreVideoEdid) {
                int rc;
                NEXUS_HdmiOutputStatus status;
                rc = NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &status);
                if (rc) return BERR_TRACE(rc);
                if (status.connected) {
                    /* NTSC/PAL is our baseline, so don't allow EDID to deny */
                    status.videoFormatSupported[nxserver_p_default_sd_format(session)] = true;
                    if (!status.videoFormatSupported[pSettings->format]) {
                        BDBG_WRN(("HDMI Rx does not support %s", lookup_name(g_videoFormatStrs, pSettings->format)));
                        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
                    }
                }
            }

            NEXUS_HdmiOutput_GetVendorSpecificInfoFrame(session->hdmiOutput, &vsi);
            if (session->display[0].formatInfo.isFullRes3d) {
                vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
                vsi.hdmi3DStructure = NEXUS_Video3DStructure_eFramePacking;
            }
            else {
                switch (pSettings->display3DSettings.orientation) {
                default:
                case NEXUS_VideoOrientation_e2D:
                    vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_eNone;
                    break;
                case NEXUS_VideoOrientation_e3D_LeftRight:
                    vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
                    vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eSidexSideHalf;
                    break;
                case NEXUS_VideoOrientation_e3D_OverUnder:
                    vsi.hdmiVideoFormat = NEXUS_HdmiVendorSpecificInfoFrame_HDMIVideoFormat_e3DFormat;
                    vsi.hdmi3DStructure = NEXUS_HdmiVendorSpecificInfoFrame_3DStructure_eTopAndBottom;
                    break;
                }
            }
            NEXUS_HdmiOutput_SetVendorSpecificInfoFrame(session->hdmiOutput, &vsi);
        }
    }

    if (!format_change)
    {
        /* depends on format: if no format change, dynrng can be set here */
        rc = nxserverlib_dynrng_p_set_mode(&session->hdmi.dynrng, pSettings);
        if (rc) { return BERR_TRACE(rc); }
    }

    /* does not depend on format, can be set here */
    rc = nxserverlib_dynrng_p_set_drmif(&session->hdmi.dynrng, pSettings);
    if (rc) { return BERR_TRACE(rc); }

    /* matrix coeffs is not a format change */
    nxserverlib_p_apply_matrix_coeffs(session, pSettings);
#endif

    if (format_change) {
        BDBG_WRN(("session %d: changing display format %s -> %s",
            session->index,
            lookup_name(g_videoFormatStrs, session->nxclient.displaySettings.format),
            lookup_name(g_videoFormatStrs, target_format.videoFormat)));

        BKNI_ResetEvent(session->inactiveEvent);
        waitForInactive = true;

        NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
        surface_compositor_settings.enabled = false;
        rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
        if (rc) return BERR_TRACE(rc);
    }

    if (server->settings.externalApp.enabled && !server->settings.externalApp.video_outputs) goto skip_outputs;

#if NEXUS_NUM_COMPONENT_OUTPUTS
    if (session->index < NEXUS_NUM_COMPONENT_OUTPUTS && server->platformConfig.outputs.component[session->index]) {
        NEXUS_DisplayHandle display;

        /* primary HD output */
        if (server->settings.session[session->index].output.hd) {
            display = session->display[pSettings->componentPreferences.sdDisplay?1:0].display;
        }
        else {
            display = NULL;
        }

        if (display && pSettings->componentPreferences.enabled) {
            NEXUS_ComponentOutputSettings settings;
            NEXUS_ComponentOutput_GetSettings(server->platformConfig.outputs.component[session->index], &settings);
            if (settings.mpaaDecimationEnabled != pSettings->componentPreferences.mpaaDecimationEnabled) {
                settings.mpaaDecimationEnabled = pSettings->componentPreferences.mpaaDecimationEnabled;
                NEXUS_ComponentOutput_SetSettings(server->platformConfig.outputs.component[session->index], &settings);
            }
        }
        if ((!pSettings->componentPreferences.enabled || session->component.display != display) && session->component.display) {
            NEXUS_Display_RemoveOutput(session->component.display, NEXUS_ComponentOutput_GetConnector(server->platformConfig.outputs.component[session->index]));
            session->component.display = NULL;
        }
        if (pSettings->componentPreferences.enabled && !session->component.display && display) {
            session->component.display = display;
            NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(server->platformConfig.outputs.component[session->index]));
        }
    }
#endif

    nxserver_p_set_sd_outputs(session, pSettings);

#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        if (pSettings->hdmiPreferences.enabled && !session->nxclient.displaySettings.hdmiPreferences.enabled) {
            NEXUS_Display_AddOutput(session->display[0].display, NEXUS_HdmiOutput_GetVideoConnector(session->hdmiOutput));
            check_hdcp_flag = true;
        }
        else if (!pSettings->hdmiPreferences.enabled && session->nxclient.displaySettings.hdmiPreferences.enabled) {
            NEXUS_HdmiOutput_DisableHdcpAuthentication(session->hdmiOutput);
            NEXUS_Display_RemoveOutput(session->display[0].display, NEXUS_HdmiOutput_GetVideoConnector(session->hdmiOutput));
        }
    }
#endif

skip_outputs:
    if (waitForInactive) {
        rc = nxserver_p_wait_for_inactive(session);
        if (rc) {
            BERR_TRACE(rc);
        }
        else {
            if (!native_3d_active_before && nxserverlib_p_native_3d_active(session) && session->display[1].display) {
                /* need to drop SD display before going into native 3D mode */
                /* TODO: There is no code to reconnect the SD path going out of native 3D. For now, app must reconnect. */
                nxserverlib_video_disconnect_display(server, session->display[1].display);
            }
            rc = b_display_format_change(session, pSettings);
            if (rc) {
                BERR_TRACE(rc);
            }
        }
    }
    else {
        rc = NEXUS_SUCCESS;
    }

    if (client) {
        if (client->hdcp_version != pSettings->hdmiPreferences.version) {
            /* hdcp is per-client */
            client->hdcp_version = pSettings->hdmiPreferences.version;
            check_hdcp_flag = true;
        }
        if (client->hdcp_level != pSettings->hdmiPreferences.hdcp) {
            /* hdcp is per-client */
            client->hdcp_level = pSettings->hdmiPreferences.hdcp;
            check_hdcp_flag = true;
        }
    }

    if (!rc) {
        session->nxclient.displaySettings = *pSettings;
        session->nxclient.displaySettings.sequenceNumber++;
        session->callbackStatus.displaySettingsChanged++;
    }

    if (check_hdcp_flag) {
        nxserver_check_hdcp(session);
    }

    return rc;
}

/* only support rollback from public API calls */
NEXUS_Error NxClient_P_SetDisplaySettings(nxclient_t client, struct b_session *session, const NxClient_DisplaySettings *pSettings)
{
    NxClient_DisplaySettings org;
    int rc;
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    if (!session) {
        session = client->session;
    }
    if (pSettings->sequenceNumber != session->nxclient.displaySettings.sequenceNumber) {
        return BERR_TRACE(NXCLIENT_BAD_SEQUENCE_NUMBER);
    }
    if (pSettings->secure && !nxserver_p_framebuffer_heap(session, 0, true)) {
        return BERR_TRACE(NEXUS_NOT_SUPPORTED);
    }

    org = session->nxclient.displaySettings;
    rc = NxClient_P_SetDisplaySettingsNoRollback(client, session, pSettings);
    if (rc) {
        /* roll back everything except format change, which is deferred and is last - so it should not need rolling back */
        unsigned i;
        session->nxclient.displaySettings = *pSettings;
        session->nxclient.displaySettings.format = org.format;
        for (i=1;i<NXCLIENT_MAX_DISPLAYS;i++) {
            session->nxclient.displaySettings.slaveDisplay[i-1].format = org.slaveDisplay[i-1].format;
        }
        (void)NxClient_P_SetDisplaySettingsNoRollback(client, session, &org);
    }
    return rc;
}

NEXUS_Error NxClient_P_GetDisplayStatus(struct b_session *session, NxClient_DisplayStatus *pStatus)
{
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;

    BKNI_Memset(pStatus, 0, sizeof(*pStatus));

    NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
    pStatus->framebuffer.number = surface_compositor_settings.display[0].framebuffer.number;
#if NEXUS_NUM_VIDEO_ENCODERS
    video_encoder_get_num(session->server, &pStatus->transcodeDisplays.total, &pStatus->transcodeDisplays.used);
#endif

#if NEXUS_HAS_HDMI_OUTPUT
    if (session->hdmiOutput) {
        NEXUS_DisplayStatus status;
        NEXUS_Display_GetStatus(session->display[0].display, &status);
        NEXUS_HdmiOutput_GetStatus(session->hdmiOutput, &pStatus->hdmi.status);
#if NEXUS_HAS_SECURITY
        NEXUS_HdmiOutput_GetHdcpStatus(session->hdmiOutput, &pStatus->hdmi.hdcp);
        pStatus->hdmi.lastHdcpError = session->hdcp.lastHdcpError;
        pStatus->hdmi.dynamicRangeMode = status.dynamicRangeMode;
#endif
    }
#endif
    return 0;
}

void NxClient_P_GetAudioSettings(nxclient_t client, NxClient_AudioSettings *pSettings)
{
    BSTD_UNUSED(client);
    *pSettings = client->session->audioSettings;
}

#if NEXUS_HAS_AUDIO
static bool nxserver_p_is_unity_volume(NEXUS_AudioVolumeType volumeType, int32_t leftVolume, int32_t rightVolume)
{
    if (volumeType == NEXUS_AudioVolumeType_eLinear) {
        return leftVolume == NEXUS_AUDIO_VOLUME_LINEAR_NORMAL && rightVolume == NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
    }
    else {
        return leftVolume == 0 && rightVolume == 0;
    }
}

static int nxserver_p_set_audio_output(struct b_session *session, NEXUS_AudioOutput audioOutput, const NxClient_AudioSettings *pSettings, const NxClient_AudioOutputSettings *pOutputSettings)
{
    NEXUS_AudioOutputSettings outputSettings;
    BSTD_UNUSED(session);
    NEXUS_AudioOutput_GetSettings(audioOutput, &outputSettings);
    if (nxserver_p_is_unity_volume(pOutputSettings->volumeType, pOutputSettings->leftVolume, pOutputSettings->rightVolume)) {
        outputSettings.volumeType = pSettings->volumeType;
        outputSettings.leftVolume = pSettings->leftVolume;
        outputSettings.rightVolume = pSettings->rightVolume;
    }
    else if ( pSettings->volumeType == pOutputSettings->volumeType ) {
        outputSettings.volumeType = pOutputSettings->volumeType;
        if ( pSettings->volumeType == NEXUS_AudioVolumeType_eDecibel ) {
            /* For decibel values, just add the two values. */
            outputSettings.leftVolume = pSettings->leftVolume + pOutputSettings->leftVolume;
            outputSettings.rightVolume = pSettings->rightVolume + pOutputSettings->rightVolume;
            /* Make sure dB values don't go out of range */
            if ( outputSettings.leftVolume < NEXUS_AUDIO_VOLUME_DB_MIN ) { outputSettings.leftVolume = NEXUS_AUDIO_VOLUME_DB_MIN; }
            if ( outputSettings.leftVolume > 0 ) { outputSettings.leftVolume = 0; }
            if ( outputSettings.rightVolume < NEXUS_AUDIO_VOLUME_DB_MIN ) { outputSettings.rightVolume = NEXUS_AUDIO_VOLUME_DB_MIN; }
            if ( outputSettings.rightVolume > 0 ) { outputSettings.rightVolume = 0; }
        }
        else {
            int64_t left, right;
            /* For linear, multiply the two coefs together and divide by the normal value (0x800000) */
            left = (int64_t)pSettings->leftVolume * (int64_t)pOutputSettings->leftVolume;
            right = (int64_t)pSettings->rightVolume * (int64_t)pOutputSettings->rightVolume;
            left /= NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
            right /= NEXUS_AUDIO_VOLUME_LINEAR_NORMAL;
            outputSettings.leftVolume = (int32_t)left;
            outputSettings.rightVolume = (int32_t)right;
        }
    }
    else {
        /* Can't mix volume of different types */
        outputSettings.volumeType = pOutputSettings->volumeType;
        outputSettings.leftVolume = pOutputSettings->leftVolume;
        outputSettings.rightVolume = pOutputSettings->rightVolume;
    }
    outputSettings.muted = pSettings->muted || pOutputSettings->muted;
    outputSettings.additionalDelay = pOutputSettings->additionalDelay;
    outputSettings.channelMode = pOutputSettings->channelMode;
    return NEXUS_AudioOutput_SetSettings(audioOutput, &outputSettings);
}

static bool nxserver_p_is_compressed_override_change(const NxClient_AudioOutputSettings *pOldOutputSettings, const NxClient_AudioOutputSettings *pNewOutputSettings)
{
    return (BKNI_Memcmp(&pOldOutputSettings->compressedOverride, &pNewOutputSettings->compressedOverride, sizeof(pOldOutputSettings->compressedOverride)) != 0);
}

static bool nxserver_p_is_mode_change(const NxClient_AudioOutputSettings *pOldOutputSettings, const NxClient_AudioOutputSettings *pNewOutputSettings)
{
    return
        pOldOutputSettings->outputMode != pNewOutputSettings->outputMode ||
        pOldOutputSettings->channelMode != pNewOutputSettings->channelMode ||
        pOldOutputSettings->transcodeCodec != pNewOutputSettings->transcodeCodec;
}

static bool nxserver_p_is_any_mode_change(struct b_session *session, const NxClient_AudioSettings *pSettings)
{
    if (session->server->settings.audioOutputs.hdmiEnabled[0] &&
         (nxserver_p_is_mode_change(&session->audioSettings.hdmi, &pSettings->hdmi) ||
          nxserver_p_is_compressed_override_change(&session->audioSettings.hdmi, &pSettings->hdmi))) {
        return true;
    }

    if (session->server->settings.audioOutputs.spdifEnabled[0] &&
         (nxserver_p_is_mode_change(&session->audioSettings.spdif, &pSettings->spdif) ||
          nxserver_p_is_compressed_override_change(&session->audioSettings.spdif, &pSettings->spdif))) {
        return true;
    }

    if (session->server->settings.audioOutputs.dacEnabled[0] &&
         nxserver_p_is_mode_change(&session->audioSettings.dac, &pSettings->dac)) {
        return true;
    }

    if (session->server->settings.audioOutputs.i2sEnabled[0]) {
        if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == true &&
            nxserver_p_is_mode_change(&session->audioSettings.dac, &pSettings->dac)) {
            return true;
        }
        else if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == false &&
                 nxserver_p_is_mode_change(&session->audioSettings.i2s[0], &pSettings->i2s[0])) {
            return true;
        }
    }

    if (session->server->settings.audioOutputs.i2sEnabled[1] &&
        nxserver_p_is_mode_change(&session->audioSettings.i2s[1], &pSettings->i2s[1])) {
        return true;
    }

    if (session->server->settings.audioOutputs.rfmEnabled &&
        nxserver_p_is_mode_change(&session->audioSettings.rfm, &pSettings->rfm)) {
        return true;
    }

    return false;
}

static bool nxserver_p_is_equalizer_change(const NxClient_AudioOutputSettings *pOldOutputSettings, const NxClient_AudioOutputSettings *pNewOutputSettings)
{
    #if NEXUS_HAS_AUDIO
    return (BKNI_Memcmp(&pOldOutputSettings->equalizer, &pNewOutputSettings->equalizer, sizeof(pOldOutputSettings->equalizer)) != 0);
    #else
    BSTD_UNUSED(pOldOutputSettings);
    BSTD_UNUSED(pNewOutputSettings);
    return false;
    #endif
}

static bool nxserver_p_is_any_equalizer_change(struct b_session *session, const NxClient_AudioSettings *pSettings)
{
    if (session->server->settings.audioOutputs.hdmiEnabled[0] &&
         (nxserver_p_is_equalizer_change(&session->audioSettings.hdmi, &pSettings->hdmi))) {
        return true;
    }

    if (session->server->settings.audioOutputs.spdifEnabled[0] &&
         (nxserver_p_is_equalizer_change(&session->audioSettings.spdif, &pSettings->spdif))) {
        return true;
    }

    if (session->server->settings.audioOutputs.dacEnabled[0] &&
         nxserver_p_is_equalizer_change(&session->audioSettings.dac, &pSettings->dac)) {
        return true;
    }

    if (session->server->settings.audioOutputs.i2sEnabled[0]) {
        if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == true &&
            nxserver_p_is_equalizer_change(&session->audioSettings.dac, &pSettings->dac)) {
            return true;
        }
        else if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == false &&
                 nxserver_p_is_equalizer_change(&session->audioSettings.i2s[0], &pSettings->i2s[0])) {
            return true;
        }
    }

    if (session->server->settings.audioOutputs.i2sEnabled[1] &&
        nxserver_p_is_equalizer_change(&session->audioSettings.i2s[1], &pSettings->i2s[1])) {
        return true;
    }

    if (session->server->settings.audioOutputs.rfmEnabled &&
        nxserver_p_is_equalizer_change(&session->audioSettings.rfm, &pSettings->rfm)) {
        return true;
    }

    return false;
}

static bool nxserver_p_is_delay_change(const NxClient_AudioOutputSettings *pOldOutputSettings, const NxClient_AudioOutputSettings *pNewOutputSettings)
{
    return pOldOutputSettings->additionalDelay != pNewOutputSettings->additionalDelay;
}

static bool nxserver_p_is_any_delay_change(struct b_session *session, const NxClient_AudioSettings *pSettings)
{
    if (session->server->settings.audioOutputs.hdmiEnabled[0] &&
        nxserver_p_is_delay_change(&session->audioSettings.hdmi, &pSettings->hdmi)) {
        return true;
    }

    if (session->server->settings.audioOutputs.spdifEnabled[0] &&
        nxserver_p_is_delay_change(&session->audioSettings.spdif, &pSettings->spdif)) {
        return true;
    }

    if (session->server->settings.audioOutputs.dacEnabled[0] &&
        nxserver_p_is_delay_change(&session->audioSettings.dac, &pSettings->dac)) {
        return true;
    }

    if (session->server->settings.audioOutputs.i2sEnabled[0]) {
        if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == true &&
            nxserver_p_is_delay_change(&session->audioSettings.dac, &pSettings->dac)) {
            return true;
        }
        else if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == false &&
            nxserver_p_is_delay_change(&session->audioSettings.i2s[0], &pSettings->i2s[0])) {
            return true;
        }
    }

    if (session->server->settings.audioOutputs.i2sEnabled[1] &&
        nxserver_p_is_delay_change(&session->audioSettings.i2s[1], &pSettings->i2s[1])) {
        return true;
    }

    if (session->server->settings.audioOutputs.rfmEnabled &&
        nxserver_p_is_delay_change(&session->audioSettings.rfm, &pSettings->rfm)) {
        return true;
    }

    return false;
}

static bool nxserver_p_is_audio_presentation_change(const NxClient_AudioOutputSettings *pOldOutputSettings, const NxClient_AudioOutputSettings *pNewOutputSettings)
{
    return pOldOutputSettings->presentation != pNewOutputSettings->presentation;
}

static bool nxserver_p_is_any_audio_presentation_change(struct b_session *session, const NxClient_AudioSettings *pSettings)
{
    if (session->server->settings.audioOutputs.dacEnabled[0] &&
        nxserver_p_is_audio_presentation_change(&session->audioSettings.dac, &pSettings->dac)) {
        return true;
    }

    if (session->server->settings.audioOutputs.i2sEnabled[0]) {
        if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == true &&
            nxserver_p_is_audio_presentation_change(&session->audioSettings.dac, &pSettings->dac)) {
            return true;
        }
        else if (nxserverlib_p_audio_i2s0_shares_with_dac(session) == false &&
            nxserver_p_is_audio_presentation_change(&session->audioSettings.i2s[0], &pSettings->i2s[0])) {
            return true;
        }
    }

    if (session->server->settings.audioOutputs.i2sEnabled[1] &&
        nxserver_p_is_audio_presentation_change(&session->audioSettings.i2s[1], &pSettings->i2s[1])) {
        return true;
    }
    /* Currently only supports DAC and I2S  outputs*/
    return false;
}
#endif

static NEXUS_Error NxClient_P_SetSessionAudioSettings(struct b_session *session, const NxClient_AudioSettings *pSettings)
{
#if NEXUS_HAS_AUDIO
    int rc;
    bool reconfig_audio = false;
    bool restart = false;
    NEXUS_AudioCapabilities audioCapabilities;

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    if (pSettings->sequenceNumber != session->audioSettings.sequenceNumber) {
        return BERR_TRACE(NXCLIENT_BAD_SEQUENCE_NUMBER);
    }

    if (pSettings->dac.outputMode > NxClient_AudioOutputMode_ePcm) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (pSettings->i2s[0].outputMode > NxClient_AudioOutputMode_ePcm) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (pSettings->i2s[1].outputMode > NxClient_AudioOutputMode_ePcm) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (pSettings->rfm.outputMode > NxClient_AudioOutputMode_ePcm) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }

    if (nxserverlib_p_session_has_sd_audio(session)) {
        if (audioCapabilities.numOutputs.dac > 0 && session->server->settings.audioOutputs.dacEnabled [0]) {
            NEXUS_AudioOutput audioOutput = NEXUS_AudioDac_GetConnector(session->server->platformConfig.outputs.audioDacs[0]);
            rc = nxserver_p_set_audio_output(session, audioOutput, pSettings, &pSettings->dac);
            if (rc) {return BERR_TRACE(rc);}
        }
        /* I2S0 - If -i2s0 was not specified at init time then use DAC's setting as backward compatabililty */

        if (audioCapabilities.numOutputs.i2s > 0 && session->server->settings.audioOutputs.i2sEnabled[0]) {
            if (nxserverlib_p_audio_i2s0_shares_with_dac(session)) {
                NEXUS_AudioOutput audioOutput = NEXUS_I2sOutput_GetConnector(session->server->platformConfig.outputs.i2s[0]);
                rc = nxserver_p_set_audio_output(session, audioOutput, pSettings, &pSettings->dac);
                if (rc) {return BERR_TRACE(rc);}
            }
            else {
                NEXUS_AudioOutput audioOutput = NEXUS_I2sOutput_GetConnector(session->server->platformConfig.outputs.i2s[0]);
                rc = nxserver_p_set_audio_output(session, audioOutput, pSettings, &pSettings->i2s[0]);
                if (rc) {return BERR_TRACE(rc);}
            }
        }

        if (audioCapabilities.numOutputs.i2s > 1 && session->server->settings.audioOutputs.i2sEnabled[1]) {
            NEXUS_AudioOutput audioOutput = NEXUS_I2sOutput_GetConnector(session->server->platformConfig.outputs.i2s[1]);
            rc = nxserver_p_set_audio_output(session, audioOutput, pSettings, &pSettings->i2s[1]);
            if (rc) {return BERR_TRACE(rc);}
        }

        #if NEXUS_NUM_RFM_OUTPUTS
        if (session->server->platformConfig.outputs.rfm[0] && session->server->settings.audioOutputs.rfmEnabled) {
            rc = nxserver_p_set_audio_output(session, NEXUS_Rfm_GetAudioConnector(session->server->platformConfig.outputs.rfm[0]), pSettings, &pSettings->rfm);
            if (rc) {return BERR_TRACE(rc);}
        }
        #endif
    }

    if (audioCapabilities.numOutputs.spdif > 0) {
        if (nxserver_p_has_spdif_output(session)) {
            NEXUS_SpdifOutputSettings settings;
            bool spdifChanged = false;
            rc = nxserver_p_set_audio_output(session, NEXUS_SpdifOutput_GetConnector(session->server->platformConfig.outputs.spdif[0]), pSettings, &pSettings->spdif);
            if (rc) {return BERR_TRACE(rc);}

            NEXUS_SpdifOutput_GetSettings(session->server->platformConfig.outputs.spdif[0], &settings);
            if (BKNI_Memcmp(&settings.channelStatusInfo, &pSettings->spdif.channelStatusInfo, sizeof(settings.channelStatusInfo))) {
                settings.channelStatusInfo = pSettings->spdif.channelStatusInfo;
                spdifChanged = true;
            }

            if (settings.dither != pSettings->spdif.ditherEnabled) {
                settings.dither = pSettings->spdif.ditherEnabled;
                spdifChanged = true;
            }

            if (spdifChanged) {
                rc = NEXUS_SpdifOutput_SetSettings(session->server->platformConfig.outputs.spdif[0], &settings);
                if (rc) {return BERR_TRACE(rc);}
            }
        }
    }
    #if NEXUS_NUM_HDMI_OUTPUTS
    if (audioCapabilities.numOutputs.hdmi > 0) {
        if (session->hdmiOutput) {
            NEXUS_HdmiOutputSettings settings;
            bool hdmiChanged = false;
            rc = nxserver_p_set_audio_output(session, NEXUS_HdmiOutput_GetAudioConnector(session->hdmiOutput), pSettings, &pSettings->hdmi);
            if (rc) {return BERR_TRACE(rc);}

            NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &settings);
            if (BKNI_Memcmp(&settings.audioChannelStatusInfo, &pSettings->hdmi.channelStatusInfo, sizeof(settings.audioChannelStatusInfo))) {
                settings.audioChannelStatusInfo = pSettings->hdmi.channelStatusInfo;
                hdmiChanged = true;
            }

            if (settings.audioDitherEnabled != pSettings->hdmi.ditherEnabled) {
                settings.audioDitherEnabled = pSettings->hdmi.ditherEnabled;
                hdmiChanged = true;
            }

            if (settings.loudnessDeviceMode != pSettings->hdmi.loudnessDeviceMode) {
                settings.loudnessDeviceMode = pSettings->hdmi.loudnessDeviceMode;
                hdmiChanged = true;
            }
            if (hdmiChanged) {
                rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &settings);
                if (rc) {return BERR_TRACE(rc);}
            }
        }
    }
#endif
    if (nxserver_p_is_any_mode_change(session, pSettings) ||
        nxserver_p_is_any_equalizer_change(session, pSettings) ||
        nxserver_p_is_any_audio_presentation_change(session, pSettings)) {
        reconfig_audio = true;
    }
    else if (nxserver_p_is_any_delay_change(session, pSettings)) {
        restart = true;
    }

    if (BKNI_Memcmp(&session->audioSettings.loudnessSettings, &pSettings->loudnessSettings, sizeof(pSettings->loudnessSettings)) != 0) {
        NEXUS_AudioLoudnessSettings loudnessSettings;
        NEXUS_AudioModule_GetLoudnessSettings(&loudnessSettings);
        BKNI_Memcpy(&loudnessSettings, &pSettings->loudnessSettings, sizeof(pSettings->loudnessSettings));
        rc = NEXUS_AudioModule_SetLoudnessSettings(&loudnessSettings);
        if (rc) {return BERR_TRACE(rc);}

        /* If a full reconfigure is not required at least restart */
        if (!reconfig_audio) {
            restart = true;
        }
    }

    if (session->audioSettings.dolbyMsAllowed != pSettings->dolbyMsAllowed &&
        b_dolby_ms_capable(session)) {
        reconfig_audio = true;
    }

    if (pSettings != &session->audioSettings) {
        session->audioSettings = *pSettings;
        session->audioSettings.sequenceNumber++;
        session->callbackStatus.audioSettingsChanged++;
    }

    /* apply mixer settings */
    if ( session->main_audio ) {
        bserver_set_audio_mixer_config(session->main_audio);
    }

    if (reconfig_audio && session->main_audio) {
        rc = bserver_set_audio_config(session->main_audio, false);
        if (rc) return BERR_TRACE(rc);
    }
    else if (restart) {
        nxserverlib_p_restart_audio(session);
    }
#else
    BSTD_UNUSED(session);
    BSTD_UNUSED(pSettings);
#endif

    return 0;
}


NEXUS_Error NxClient_P_SetAudioSettings(nxclient_t client, const NxClient_AudioSettings *pSettings)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return NxClient_P_SetSessionAudioSettings(client->session, pSettings);
}

void NxClient_P_GetPictureQualitySettings(nxclient_t client, NxClient_PictureQualitySettings *pSettings )
{
    BSTD_UNUSED(client);
    *pSettings = client->session->pictureQualitySettings;
}

NEXUS_Error NxClient_P_SetPictureQualitySettings(nxclient_t client, const NxClient_PictureQualitySettings *pSettings )
{
    NEXUS_Error rc;

    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    rc = NEXUS_Display_SetGraphicsColorSettings(client->session->display[0].display, &pSettings->graphicsColor);
    if (rc) return BERR_TRACE(rc);

    client->session->pictureQualitySettings = *pSettings;
    return 0;
}

NEXUS_Error NxClient_P_GetStandbyStatus(nxclient_t client, NxClient_StandbyStatus *pStatus)
{
    NEXUS_Error rc = NEXUS_SUCCESS;
    struct b_client_standby_ack *ack;

    NEXUS_Platform_GetStandbyStatus(&pStatus->status);
    pStatus->settings = client->server->standby.standbySettings.settings;
    switch (client->server->standby.state) {
        default:
        case b_standby_state_none:
            pStatus->transition = NxClient_StandbyTransition_eNone;
            break;
        case b_standby_state_pending:
            for (ack = BLST_S_FIRST(&client->standby.acks); ack; ack = BLST_S_NEXT(ack, link)) {
                if (ack->waiting) break;
            }
            if (ack) pStatus->transition = NxClient_StandbyTransition_eAckNeeded;
            else pStatus->transition = NxClient_StandbyTransition_ePending;
            break;
        case b_standby_state_exit:
            pStatus->transition = NxClient_StandbyTransition_eDone;
            break;
    }
    pStatus->standbyTransition =  pStatus->transition == NxClient_StandbyTransition_eDone?true:false;
    pStatus->lastStandbyTimestamp = client->server->standby.last_standby_timestamp;
    pStatus->lastResumeTimestamp = client->server->standby.last_resume_timestamp;
    pStatus->lastStandbyMode = client->server->standby.last_standby_mode;
    return rc;
}

/* check or set client ack */
static int ack_state(nxserver_t server, bool check)
{
    nxclient_t client;
    for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_client_standby_ack *ack;
        for (ack = BLST_S_FIRST(&client->standby.acks); ack; ack = BLST_S_NEXT(ack, link)) {
            if (check) {
                if (ack->waiting) break;
            }
            else {
                ack->waiting = true;
            }
        }
        if (ack) break;
    }
    return client ? 1 : 0;
}

static void standby_thread(void *context)
{
    nxserver_t server = (nxserver_t) context;
    NEXUS_StandbySettings standbySettings;
    unsigned tries = server->settings.standby_timeout*10;
    NEXUS_Error rc;
    unsigned i;
    const char *mode_str = lookup_name(g_platformStandbyModeStrs, server->standby.standbySettings.settings.mode);

    BDBG_WRN(("Entering standby mode %s after %d second ack timeout", mode_str, server->settings.standby_timeout));
    while(tries) {
        BKNI_AcquireMutex(server->settings.lock);
        rc = ack_state(server, true);
        BKNI_ReleaseMutex(server->settings.lock);
        /* exit loop if no client waiting */
        if (!rc) break;
        tries--;
        BKNI_Sleep(100);
    }

    if(!tries){
        BDBG_WRN(("Timeout waiting for clients after %d seconds", server->settings.standby_timeout));
    }

    BKNI_AcquireMutex(server->settings.lock);

    NEXUS_Platform_GetStandbySettings(&standbySettings);

    if(standbySettings.mode == NEXUS_PlatformStandbyMode_eOn) {
        for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
            struct b_session *session = server->session[i];
            if (session) {
                NEXUS_SurfaceCompositorSettings surface_compositor_settings;

                nxserver_p_acquire_release_all_resources(session, false);

#if NEXUS_HAS_IR_INPUT
                if(session->input.irInput[0]) {
                    /* TODO: filter for each input */
                    NEXUS_IrInputDataFilter filter;
                    NEXUS_IrInput_GetDefaultDataFilter(&filter);
                    filter.filterWord[0].patternWord = server->settings.session[i].ir_input.standby_filter;
                    filter.filterWord[0].enabled = true;
                    NEXUS_IrInput_EnableDataFilter(session->input.irInput[0], &filter);
                }
#endif
                if(session->surfaceCompositor) {
                    NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
                    surface_compositor_settings.enabled = false;
                    NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
                }
#if NEXUS_HAS_HDMI_OUTPUT
                if(session->hdmiOutput) {
                    NEXUS_HdmiOutputSettings hdmiSettings;
                    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

                    NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiSettings);
                    hdmiSettings.hotplugCallback.callback = NULL;
                    rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiSettings);
                    if (rc) rc = BERR_TRACE(rc);

                    NEXUS_HdmiOutput_GetHdcpSettings(session->hdmiOutput,  &hdmiOutputHdcpSettings);
                    if (hdmiOutputHdcpSettings.stateChangedCallback.callback) {
                        hdmiOutputHdcpSettings.stateChangedCallback.callback = NULL;
                        rc = NEXUS_HdmiOutput_SetHdcpSettings(session->hdmiOutput, &hdmiOutputHdcpSettings);
                        if (rc) rc = BERR_TRACE(rc);
                    }
                }
#endif
                bserver_acquire_audio_mixers(session->main_audio, false);
            }
        }
        for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
            struct b_session *session = server->session[i];
            if (session && session->surfaceCompositor) {
                rc = nxserver_p_wait_for_inactive(session);
                if (rc) rc = BERR_TRACE(rc);
            }
        }
    }

    BDBG_WRN(("Entering standby mode: %s", mode_str));

    standbySettings = server->standby.standbySettings.settings;
    for(i=0;i<10;i++) { /* try for 1 second (18 x 100 msec) */
        standbySettings.timeout = 100;
        rc = NEXUS_Platform_SetStandbySettings(&standbySettings);
        if(rc==NEXUS_TIMEOUT) {
            BDBG_WRN(("Timeout on SetStandbySettings, wait and try again"));
            BKNI_ReleaseMutex(server->settings.lock);
            BKNI_Sleep(100);
            BKNI_AcquireMutex(server->settings.lock);
        } else {
            break;
        }
    }
    if (rc) {
        BDBG_ERR(("Failed to enter standby mode: %s", mode_str));
        goto done;
    }
    for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
        if (server->session[i]) server->session[i]->callbackStatus.standbyStateChanged++;
    }

    server->standby.state = b_standby_state_applied;
    NEXUS_GetTimestamp(&server->standby.last_standby_timestamp);
    server->standby.last_standby_mode = server->standby.standbySettings.settings.mode;

done:
    if(rc) NEXUS_Platform_GetStandbySettings(&server->standby.standbySettings.settings);

    server->standby.state = b_standby_state_exit;

    BKNI_ReleaseMutex(server->settings.lock);
}

static NEXUS_Error bserver_set_standby_settings(nxserver_t server, const NxClient_StandbySettings *pSettings)
{
    const char *mode_str = lookup_name(g_platformStandbyModeStrs, pSettings->settings.mode);
    NEXUS_Error rc=0;

    if(server->standby.state == b_standby_state_pending) {
        /* Nexus has not yet entered the requested state. Cannot transition yet */
        BDBG_ERR(("Previous standby setting not completed. Cannot apply new standby setting"));
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    } else {
        /* Cannot be in this state */
        BDBG_ASSERT(server->standby.state != b_standby_state_applied);
    }

    if(server->standby.thread_id) {
        NEXUS_Thread_Destroy(server->standby.thread_id);
        server->standby.thread_id = NULL;
    }

    /* For S0 we can transition right away.
       For all other modes we start a thread to monitor
       the client state and wait for clients to acknowledge */
    if(pSettings->settings.mode == NEXUS_PlatformStandbyMode_eOn) {
        NEXUS_StandbySettings standbySettings;
        unsigned i;

        if(server->standby.standbySettings.settings.mode == NEXUS_PlatformStandbyMode_eOn)
            return rc;

        BDBG_ASSERT(server->standby.state == b_standby_state_exit);

        BDBG_WRN(("Entering mode: %s", mode_str));

        NEXUS_Platform_GetStandbySettings(&standbySettings);
        standbySettings = pSettings->settings;
        rc = NEXUS_Platform_SetStandbySettings(&standbySettings);
        if (rc) { rc = BERR_TRACE(rc);goto done; }
        for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
            if (server->session[i]) server->session[i]->callbackStatus.standbyStateChanged++;
        }

        for (i=0;i<NXCLIENT_MAX_SESSIONS;i++) {
            NEXUS_SurfaceCompositorSettings surface_compositor_settings;
            struct b_session *session = server->session[i];
            if (!session) continue;
            if(session->surfaceCompositor) {
                NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
                surface_compositor_settings.enabled = true;
                NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
            }

#if NEXUS_HAS_IR_INPUT
            if(session->input.irInput[0]) {
                NEXUS_IrInput_DisableDataFilter(session->input.irInput[0]);
            }
#endif
#if NEXUS_HAS_HDMI_OUTPUT
            if(session->hdmiOutput) {
                NEXUS_HdmiOutputSettings hdmiSettings;
                NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

                NEXUS_HdmiOutput_GetHdcpSettings(session->hdmiOutput,  &hdmiOutputHdcpSettings);
                hdmiOutputHdcpSettings.stateChangedCallback.callback = hdmiOutputHdcpStateChanged;
                hdmiOutputHdcpSettings.stateChangedCallback.context = session;
                rc = NEXUS_HdmiOutput_SetHdcpSettings(session->hdmiOutput, &hdmiOutputHdcpSettings);
                if (rc) rc = BERR_TRACE(rc);

                NEXUS_HdmiOutput_GetSettings(session->hdmiOutput, &hdmiSettings);
                hdmiSettings.hotplugCallback.callback = hotplug_callback;
                rc = NEXUS_HdmiOutput_SetSettings(session->hdmiOutput, &hdmiSettings);
                if (rc) rc = BERR_TRACE(rc);

                hotplug_callback_locked(session, 0);
            }
#endif
            bserver_acquire_audio_mixers(session->main_audio, true);
            nxserver_p_acquire_release_all_resources(session, true);
        }

        server->standby.state = b_standby_state_none;
        server->standby.standbySettings = *pSettings;
        NEXUS_GetTimestamp(&server->standby.last_resume_timestamp);

        /* destroy zombie clients */
        {
            nxclient_t client;
            for (client = BLST_D_FIRST(&server->clients); client;) {
                nxclient_t next = BLST_D_NEXT(client, link);
                if (client->zombie) {
                    NxClient_P_DestroyClient(client);
                }
                client = next;
            }
        }
    } else {
        /* we are waiting on clients */
        (void)ack_state(server, false);
        server->standby.standbySettings = *pSettings;
        server->standby.state = b_standby_state_pending;
        server->standby.thread_id = NEXUS_Thread_Create("standby", standby_thread, server, NULL);
    }

done:
    return rc;
}

NEXUS_Error NxClient_P_SetStandbySettings(nxclient_t client, const NxClient_StandbySettings *pSettings)
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    return bserver_set_standby_settings(client->server, pSettings);
}

bool nxserver_is_standby(nxserver_t server)
{
    return (server->standby.standbySettings.settings.mode != NEXUS_PlatformStandbyMode_eOn && server->standby.state >= b_standby_state_applied?true:false);
}

static bool nxserver_is_standby_pending(nxserver_t server)
{
    return (server->standby.standbySettings.settings.mode != NEXUS_PlatformStandbyMode_eOn && server->standby.state >= b_standby_state_pending);
}

static nxclient_t lookup_client(nxserver_t server, NEXUS_ClientHandle nexusClient)
{
    nxclient_t client;
    for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
        if (client->nexusClient == nexusClient) {
            return client;
        }
    }
    return NULL;
}

NEXUS_Error NxClient_P_Config_GetJoinSettings(nxclient_t client, NEXUS_ClientHandle nexusClient, NxClient_JoinSettings *pSettings )
{
    client = lookup_client(client->server, nexusClient);
    if (client) {
        *pSettings = client->joinSettings;
        return 0;
    }
    return NEXUS_INVALID_PARAMETER;
}

static NEXUS_Error nxclient_get_surfaceclient_id(nxclient_t client, NEXUS_SurfaceClientHandle surfaceClient, unsigned *pId)
{
    struct b_req *req;
    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (req->handles.surfaceClient[i].handle == surfaceClient) {
                *pId = req->handles.surfaceClient[i].id;
                return 0;
            }
            else if (!req->handles.surfaceClient[i].id) {
                i = NXCLIENT_MAX_IDS;
            }
        }
    }
    return NEXUS_INVALID_PARAMETER;
}

void NxClient_P_Config_GetSurfaceClientComposition(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition )
{
    if (!nexusClient) {
        nexusClient = client->nexusClient;
    }
    client = lookup_client(client->server, nexusClient);
    if (client) {
        unsigned id;
        /* lookup is inefficient but required for handle verification */
        int rc = nxclient_get_surfaceclient_id(client, surfaceClient, &id);
        if (!rc) {
            NxClient_P_GetSurfaceClientComposition(client, id, pComposition);
            return;
        }
    }
    BKNI_Memset(pComposition, 0, sizeof(*pComposition));
}

NEXUS_Error NxClient_P_Config_SetSurfaceClientComposition(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition )
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    if (!nexusClient) {
        nexusClient = client->nexusClient;
    }
    client = lookup_client(client->server, nexusClient);
    if (client) {
        unsigned id;
        /* lookup is inefficient but required for handle verification */
        int rc = nxclient_get_surfaceclient_id(client, surfaceClient, &id);
        if (!rc) {
            return NxClient_P_SetSurfaceClientComposition(client, id, pComposition);
        }
    }
    return NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NxClient_P_Config_GetConnectList(nxclient_t client, NEXUS_ClientHandle nexusClient, NxClient_ConnectList *pList )
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    client = lookup_client(client->server, nexusClient);
    if (client) {
        struct b_connect *connect;
        unsigned total = 0;
        BKNI_Memset(pList, 0, sizeof(*pList));
        for (connect = BLST_D_FIRST(&client->connects); connect && total<NXCLIENT_MAX_IDS; connect = BLST_D_NEXT(connect, link)) {
            pList->connectId[total++] = connect->id;
        }
        return 0;
    }
    return NEXUS_INVALID_PARAMETER;
}

NEXUS_Error NxClient_P_Config_RefreshConnect(nxclient_t client, NEXUS_ClientHandle nexusClient, unsigned connectId )
{
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    client = lookup_client(client->server, nexusClient);
    if (client) {
        return NxClient_P_RefreshConnect(client, connectId);
    }
    return NEXUS_INVALID_PARAMETER;
}

void NxClient_P_Config_GetConnectSettings(nxclient_t client, NEXUS_ClientHandle nexusClient, unsigned connectId, NxClient_ConnectSettings *pSettings )
{
    client = lookup_client(client->server, nexusClient);
    if (client) {
        struct b_connect *connect;
        connect = lookup_connect(client, connectId);
        if (connect) {
            *pSettings = connect->settings;
            return;
        }
    }
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if NEXUS_HAS_INPUT_ROUTER
static struct b_req *lookup_input_client(nxclient_t client, NEXUS_InputClientHandle inputClient)
{
    struct b_req *req;
    for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
        unsigned i;
        for (i=0;i<NXCLIENT_MAX_IDS;i++) {
            if (req->handles.inputClient[i].id && req->handles.inputClient[i].handle == inputClient) {
                return req;
            }
        }
    }
    return NULL;
}
#endif

void NxClient_P_Config_GetInputClientServerFilter(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned *pFilter )
{
#if NEXUS_HAS_INPUT_ROUTER
    client = lookup_client(client->server, nexusClient);
    if (client) {
        struct b_req *req = lookup_input_client(client, inputClient);
        if (req) {
            NEXUS_InputRouterClientSettings settings;
            NEXUS_InputRouter_GetClientSettings(client->session->input.router, inputClient, &settings);
            *pFilter = settings.filterMask;
        }
    }
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(nexusClient);
    BSTD_UNUSED(inputClient);
    BKNI_Memset(pFilter, 0, sizeof(*pFilter));
#endif
}

NEXUS_Error NxClient_P_Config_SetInputClientServerFilter(nxclient_t client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned filter )
{
#if NEXUS_HAS_INPUT_ROUTER
    if (!is_trusted_client(client)) {
        return BERR_TRACE(NXCLIENT_NOT_ALLOWED);
    }
    client = lookup_client(client->server, nexusClient);
    if (client) {
        struct b_req *req = lookup_input_client(client, inputClient);
        if (req) {
            NEXUS_InputRouterClientSettings settings;
            NEXUS_InputRouter_GetClientSettings(client->session->input.router, inputClient, &settings);
            settings.filterMask = filter;
            return NEXUS_InputRouter_SetClientSettings(client->session->input.router, inputClient, &settings);
        }
    }
    return NEXUS_INVALID_PARAMETER;
#else
    BSTD_UNUSED(client);
    BSTD_UNUSED(nexusClient);
    BSTD_UNUSED(inputClient);
    BSTD_UNUSED(filter);
    return NEXUS_NOT_SUPPORTED;
#endif
}

struct b_session *nxserver_get_client_session(nxclient_t client)
{
    return client->session;
}

NEXUS_Error NxClient_P_GetCallbackStatus(nxclient_t client, NxClient_CallbackStatus *pStatus )
{
    *pStatus = client->session->callbackStatus;
    return 0;
}

NEXUS_Error NxClient_P_GetAudioStatus(nxclient_t client, NxClient_AudioStatus *pStatus )
{
    return nxserverlib_audio_get_status(client->session, pStatus);
}

int nxserverlib_allow_decode(nxserver_t server, bool allow)
{
    int rc;
    struct b_session *session = server->session[0];
    BKNI_AcquireMutex(server->settings.lock);
    if (allow) {
        b_audio_decoder_create_settings create_settings;
        audio_decoder_get_default_create_settings(&create_settings);

        rc = video_init(server);
        if (rc) {rc = BERR_TRACE(rc); goto err_video_init;}

        rc = audio_init(server);
        if (rc) {rc = BERR_TRACE(rc); goto err_audio_init;}

        session->main_audio = audio_decoder_create(session, b_audio_decoder_type_regular, &create_settings);
        if (!session->main_audio) {rc = BERR_TRACE(-1); goto err_audio_create;}
    }
    else {
        nxclient_t client;

        /* disconnect all clients */
        for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
            struct b_connect *connect;
            for (connect = BLST_D_FIRST(&client->connects); connect; ) {
                struct b_connect *next = BLST_D_NEXT(connect, link);
                b_disconnect(client, connect);
                connect = next;
            }
        }
        uninit_session_video(session);
        audio_decoder_destroy(session->main_audio);
        session->main_audio = NULL;
        video_uninit();
        audio_uninit();
    }
    server->externalApp.allow_decode = allow;
    BKNI_ReleaseMutex(server->settings.lock);
    return 0;

err_audio_create:
    audio_uninit();
err_audio_init:
    video_uninit();
err_video_init:
#if NEXUS_HAS_HDMI_OUTPUT
    session->hdmiOutput = NULL;
#endif
    BKNI_ReleaseMutex(server->settings.lock);
    return rc;
}

void nxclient_get_status(nxclient_t client, struct nxclient_status *pstatus)
{
    BDBG_OBJECT_ASSERT(client, b_client);
    BKNI_Memset(pstatus, 0, sizeof(*pstatus));
    pstatus->handle = client->nexusClient;
    pstatus->pid = client->pid;
}

NEXUS_Error nxserver_p_focus_input_client(nxclient_t c)
{
#if NEXUS_HAS_INPUT_ROUTER
    nxclient_t client;
    if (!c->session->input.router) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    for (client = BLST_D_FIRST(&c->server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_req *req;
        if (client->session != c->session) continue;
        for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
            NEXUS_InputRouterClientSettings settings;
            if (!req->handles.inputClient[0].id) continue;
            NEXUS_InputRouter_GetClientSettings(client->session->input.router, req->handles.inputClient[0].handle, &settings);
            settings.filterMask = (client==c)?0xFFFFFFFF/*all*/: 0x0/*none*/;
            NEXUS_InputRouter_SetClientSettings(client->session->input.router, req->handles.inputClient[0].handle, &settings);
        }
    }
    return 0;
#else
    BSTD_UNUSED(c);
    return BERR_TRACE(NEXUS_NOT_SUPPORTED);
#endif
}

NEXUS_Error nxserver_p_focus_surface_client(nxclient_t c)
{
    nxclient_t client;
    unsigned max = 0;
    struct b_req *req;
    if (!c->session->surfaceCompositor) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    for (client = BLST_D_FIRST(&c->server->clients); client; client = BLST_D_NEXT(client, link)) {
        if (client->session != c->session || client == c) continue;
        for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
            NEXUS_SurfaceCompositorClientSettings settings;
            if (!req->handles.surfaceClient[0].id) continue;
            NEXUS_SurfaceCompositor_GetClientSettings(client->session->surfaceCompositor, req->handles.surfaceClient[0].handle, &settings);
            if (settings.composition.zorder > max) {
                max = settings.composition.zorder;
            }
        }
    }
    for (req = BLST_D_FIRST(&c->requests); req; req = BLST_D_NEXT(req, link)) {
        if (req->handles.surfaceClient[0].id) {
            NEXUS_SurfaceCompositorClientSettings settings;
            NEXUS_SurfaceCompositor_GetClientSettings(c->session->surfaceCompositor, req->handles.surfaceClient[0].handle, &settings);
            if (settings.composition.zorder == max) {
                max++;
            }
            settings.composition.zorder = max;
            NEXUS_SurfaceCompositor_SetClientSettings(c->session->surfaceCompositor, req->handles.surfaceClient[0].handle, &settings);
            break;
        }
    }
    return 0;
}

#if NEXUS_HAS_ASTM
void nxserverlib_get_astm_settings(NEXUS_AstmSettings *pSettings)
{
    NEXUS_Astm_GetDefaultSettings(pSettings);
}

void nxserverlib_set_astm_settings(const NEXUS_AstmSettings *pSettings)
{
    NEXUS_SimpleStcChannel_SetDefaultAstmSettings_priv(pSettings);
}
#endif

int nxserver_p_reenable_local_display(nxserver_t server)
{
    int rc;
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    struct b_session *session = server->disabled_local_display.session;
    unsigned d = server->disabled_local_display.local_display_index;

    /* only reenable what nxserver_p_disable_local_display closed */
    if (!session) return NEXUS_SUCCESS;
    server->disabled_local_display.session = NULL; /* only try once */

    session->display[d].display = NEXUS_Display_Open(session->display[d].global_index, &server->disabled_local_display.settings);
    if (!session->display[d].display) {
        return BERR_TRACE(NEXUS_UNKNOWN);
    }
    NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
    surface_compositor_settings.enabled = false;
    rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
    if (!rc) {
        rc = nxserver_p_wait_for_inactive(session);
    }
    if (!rc) {
        surface_compositor_settings.display[d].display = session->display[d].display;
        surface_compositor_settings.enabled = true;
        NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
    }
    nxserver_p_set_sd_outputs(session, &session->nxclient.displaySettings);

    /* Do not reenable video. That requires a reconnect. */
    return NEXUS_SUCCESS;
}

static void nxserverlib_graphics_disconnect_display(struct b_session *session, unsigned local_display_index)
{
    int rc;
    NEXUS_SurfaceCompositorSettings surface_compositor_settings;
    nxclient_t client;

    NEXUS_SurfaceCompositor_GetSettings(session->surfaceCompositor, &surface_compositor_settings);
    surface_compositor_settings.enabled = false;
    rc = NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
    if (!rc) {
        rc = nxserver_p_wait_for_inactive(session);
    }
    if (!rc) {
        surface_compositor_settings.display[local_display_index].display = NULL;
        surface_compositor_settings.enabled = true;
        NEXUS_SurfaceCompositor_SetSettings(session->surfaceCompositor, &surface_compositor_settings);
    }

    for (client = BLST_D_FIRST(&session->server->clients); client; client = BLST_D_NEXT(client, link)) {
        struct b_req *req;
        if (client->session != session) continue;
        for (req = BLST_D_FIRST(&client->requests); req; req = BLST_D_NEXT(req, link)) {
            unsigned i;
            for (i=0;i<NXCLIENT_MAX_IDS;i++) {
                if (req->handles.surfaceClient[i].id) {
                    NEXUS_SurfaceCompositorClientSettings settings;
                    NEXUS_SurfaceClientHandle surfaceClient = req->handles.surfaceClient[i].handle;
                    NEXUS_SurfaceCompositor_GetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
                    if (settings.display[1].window[0].window) {
                        unsigned j;
                        for (j=0;j<NEXUS_SURFACE_COMPOSITOR_VIDEO_WINDOWS;j++) {
                            settings.display[1].window[j].window = NULL;
                        }
                        NEXUS_SurfaceCompositor_SetClientSettings(session->surfaceCompositor, surfaceClient, &settings);
                    }
                }
            }
        }
    }

}

void nxserver_p_disable_local_display(nxserver_t server, unsigned displayIndex)
{
    unsigned d;
    struct b_session *session = server->session[0];

    /* only support disabling and reenabling one */
    if (server->disabled_local_display.session) return;

    /* for now, only allow session[0].display[1]. makes handling outputs easier. */
    d = 1;
    if (!session || !session->display[d].display || session->display[d].global_index != displayIndex) {
        return;
    }

    /* remember what we disable */
    server->disabled_local_display.session = session;
    server->disabled_local_display.local_display_index = d;
    NEXUS_Display_GetSettings(session->display[d].display, &server->disabled_local_display.settings);

    /* remove from simple_decoder and surface_compositor */
    nxserverlib_video_disconnect_display(server, session->display[d].display);
    nxserverlib_graphics_disconnect_display(session, d);

    /* close windows, display, and associated resources */
    nxserverlib_video_close_windows(session, d);
    NEXUS_Display_Close(session->display[d].display);
    if (session->component.display == session->display[d].display) {
        session->component.display = NULL;
    }
    if (session->composite.display == session->display[d].display) {
        session->composite.display = NULL;
    }
    session->display[d].display = NULL;
    session->display[d].crc_client = NULL;
    if (session->display[d].graphic) {
        NEXUS_Surface_Destroy(session->display[d].graphic);
        session->display[d].graphic = NULL;
    }
}

#if NEXUS_HAS_IR_INPUT
NEXUS_IrInputHandle nxserver_get_ir_input(nxserver_t server, unsigned session, unsigned input)
{
    if (session < NXCLIENT_MAX_SESSIONS && server->session[session] && input < NXSERVER_IR_INPUTS) {
        return server->session[session]->input.irInput[input];
    }
    return NULL;
}
#endif

NEXUS_Error NxClient_P_SetClientMode(nxclient_t client, const NxClient_ClientModeSettings *pSettings)
{
    NEXUS_Error rc;
    if (client->server->settings.certificate.length) {
        if (!nxserver_p_valid_certificate(client->server, &pSettings->certificate)) {
            return BERR_TRACE(NEXUS_INVALID_PARAMETER);
        }
    }
    rc = NEXUS_Platform_SetClientMode(client->nexusClient, pSettings->mode);
    if (rc) return BERR_TRACE(rc);
    /* update only the mode */
    client->joinSettings.mode = pSettings->mode;
    return NEXUS_SUCCESS;
}

int nxserver_parse_password_file(nxserver_t server, const char *filename)
{
    NEXUS_Certificate certificate;

    if (nxclient_p_parse_password_file(filename, &certificate)) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    /* change default client mode and required cert */
    server->settings.client_mode = NEXUS_ClientMode_eUntrusted;
    server->settings.certificate = certificate;
    return NEXUS_SUCCESS;
}

bool nxserver_p_allow_grab(nxclient_t client)
{
    return client->server->settings.grab && client->connect_settings.allow_grab;
}

NEXUS_Error NxClient_P_GetThermalStatus(nxclient_t client, NxClient_ThermalStatus *pStatus)
{
    return nxserver_get_thermal_status(client, pStatus);
}

void NxClient_P_GetThermalConfigurationList(nxclient_t client, NxClient_ThermalConfigurationList *pConfigList)
{
    nxserver_get_thermal_configuration_list(client, pConfigList);
}

NEXUS_Error NxClient_P_GetThermalConfiguration(nxclient_t client, unsigned tempThreshold, NxClient_ThermalConfiguration *pConfig)
{
    return nxserver_get_thermal_configuration(client, tempThreshold, pConfig);
}

NEXUS_Error NxClient_P_SetThermalConfiguration(nxclient_t client, unsigned tempThreshold, bool pConfig_isNull, const NxClient_ThermalConfiguration *pConfig)
{
    return nxserver_set_thermal_configuration(client, tempThreshold, pConfig_isNull?NULL:pConfig);
}

unsigned nxserver_p_millisecond_tick(void)
{
    int rc;
    struct timespec now;
    rc = clock_gettime(CLOCK_MONOTONIC, &now);
    if (rc) {
        rc = BERR_TRACE(BERR_OS_ERROR);
        return 0;
    }
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

#define BDBG_MSG_WATCHDOG(X) /* BDBG_LOG(X) */

static int nxserver_p_calc_watchdog_timeout(struct nxserver_watchdog_client *watchdog, unsigned now, unsigned *smallest_timeout)
{
    unsigned diff;
    if (!watchdog->timeout) return 0;
    diff = now - watchdog->pettime;
    if (diff > watchdog->timeout) {
        /* if any client has exceeded its timeout, we cannot pet */
        BDBG_MSG_WATCHDOG(("cannot pet: %p %u > %u", (void*)watchdog, diff, watchdog->timeout));
        return -1;
    }
    if (watchdog->timeout && watchdog->timeout - diff < *smallest_timeout) {
        *smallest_timeout = watchdog->timeout - diff;
    }
    BDBG_MSG_WATCHDOG(("check_watchdog: %p timeout %u, diff %u, smallest %u", (void*)watchdog, watchdog->timeout, diff, *smallest_timeout));
    return 0;
}

static NEXUS_Error nxserver_p_check_watchdog(nxserver_t server)
{
    /* set system-wide watchdog based on clients and server param */
    nxclient_t client;
    unsigned now;
    unsigned smallest_timeout = NEXUS_WATCHDOG_MAX_TIMEOUT;
    bool touch_watchdog_hw = true; /* touch means start or stop */

    if (server->watchdog.shutdown) {
        return NEXUS_SUCCESS;
    }

    now = nxserver_p_millisecond_tick() / 1000;
    for (client = BLST_D_FIRST(&server->clients); client; client = BLST_D_NEXT(client, link)) {
        if (nxserver_p_calc_watchdog_timeout(&client->watchdog, now, &smallest_timeout)) {
            touch_watchdog_hw = false;
            break;
        }
    }
    if (server->watchdog.state.timeout) {
        if (nxserver_p_calc_watchdog_timeout(&server->watchdog.state, now, &smallest_timeout)) {
            touch_watchdog_hw = false;
        }
    }

    if (touch_watchdog_hw) {
        NEXUS_Error rc;
        if (!server->watchdog.handle) {
            /* defer open so that clients can use legacy NEXUS_Watchdog without
            conflicting with nxserver if NxClient watchdog is unused. */
            server->watchdog.handle = NEXUS_WatchdogInterface_Open(0);
            if (!server->watchdog.handle) {
                return BERR_TRACE(NEXUS_NOT_AVAILABLE);
            }
        }
        /* any error here may be an omen of a system reset, or may indicate a watchdog will never happen. */
        rc = NEXUS_WatchdogInterface_StopTimer(server->watchdog.handle);
        if (rc) return BERR_TRACE(rc);
        if (smallest_timeout < NEXUS_WATCHDOG_MAX_TIMEOUT) {
            /* No harm in bumping up the min */
            if (smallest_timeout < NEXUS_WATCHDOG_MIN_TIMEOUT) {
                smallest_timeout = NEXUS_WATCHDOG_MIN_TIMEOUT;
            }
            rc = NEXUS_WatchdogInterface_SetTimeout(server->watchdog.handle, smallest_timeout);
            if (rc) return BERR_TRACE(rc);
            rc = NEXUS_WatchdogInterface_StartTimer(server->watchdog.handle);
            if (rc) return BERR_TRACE(rc);
            BDBG_MSG_WATCHDOG(("start watchdog %u", smallest_timeout));
        }
        else {
            BDBG_MSG_WATCHDOG(("stop watchdog"));
        }
    }
    return NEXUS_SUCCESS;
}

void nxserverlib_shutdown_watchdog(nxserver_t server)
{
    BKNI_AcquireMutex(server->settings.lock);
    server->watchdog.shutdown = true; /* prevent clients from restarting */
    if (server->watchdog.handle) {
        NEXUS_WatchdogInterface_Close(server->watchdog.handle);
        server->watchdog.handle = NULL;
    }
    BKNI_ReleaseMutex(server->settings.lock);
}

static void nxserver_p_immediate_watchdog(nxclient_t client)
{
    if (client->server->watchdog.handle) {
        BDBG_ERR(("client %p with pending watchdog %u has just exited.", (void*)client, client->watchdog.timeout));
        BDBG_ERR(("This watchdog cannot be cleared; therefore, nxserver is waiting for it to occur."));
        while (1) BKNI_Sleep(1000);
    }
}

NEXUS_Error nxserver_p_pet_watchdog(nxserver_t server, struct nxserver_watchdog_client *watchdog, unsigned timeout)
{
    /* don't allow a repeated Set of 0 to prevent watchdogs */
    if (timeout > NEXUS_WATCHDOG_MAX_TIMEOUT) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    if (timeout || watchdog->timeout) {
        watchdog->timeout = timeout;
        watchdog->pettime = nxserver_p_millisecond_tick() / 1000;
        return nxserver_p_check_watchdog(server);
    }
    else {
        return NEXUS_SUCCESS;
    }
}

NEXUS_Error NxClient_P_SetWatchdogTimeout(nxclient_t client, unsigned timeout)
{
    return nxserver_p_pet_watchdog(client->server, &client->watchdog, timeout);
}

void nxserver_p_lock(nxserver_t server)
{
    BDBG_ASSERT(server);
    BDBG_ASSERT(server->settings.lock);
    BKNI_AcquireMutex(server->settings.lock);
}

void nxserver_p_unlock(nxserver_t server)
{
    BDBG_ASSERT(server);
    BDBG_ASSERT(server->settings.lock);
    BKNI_ReleaseMutex(server->settings.lock);
}

NEXUS_DisplayHandle nxserverlib_session_p_get_primary_display(struct b_session * session)
{
    return session->display[0].display;
}

NEXUS_HdmiOutputHandle nxserverlib_session_p_get_hdmi_output(struct b_session * session)
{
    return session?session->hdmiOutput:NULL;
}

nxserver_t nxserverlib_session_p_get_server(struct b_session * session)
{
    return session->server;
}
