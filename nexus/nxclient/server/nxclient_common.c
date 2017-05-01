/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nxclient.h"
#include "nexus_base_os.h"
#include "bkni.h"
#include "nxserverlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif

BDBG_MODULE(nxclient_common);

int nxclient_p_parse_password_file(const char *filename, NEXUS_Certificate *certificate)
{
    FILE *f = fopen(filename, "r");
    if (!f) {
        BDBG_MSG(("unable to open %s", filename));
        return -1;
    }
    while (!feof(f)) {
        char buf[256];
        unsigned len;
        fgets(buf, sizeof(buf), f);
        len = strlen(buf);
        if (len && buf[len-1] == '\n') buf[--len] = 0;
        if (len && buf[len-1] == '\r') buf[--len] = 0;
        if (!len || buf[0] == '#') continue;
        if (strstr(buf, "trusted:") == buf) {
            const char *password = &buf[8];
            certificate->length = strlen(password);
            memcpy(certificate->data, password, certificate->length);
        }
    }
    fclose(f);
    return 0;
}

void NxClient_GetDefaultJoinSettings(NxClient_JoinSettings *pSettings)
{
    const char *session = getenv("NXCLIENT_SESSION");
    const char *password = getenv("NXCLIENT_PASSWORD");
    memset(pSettings, 0, sizeof(*pSettings));
    if (session) {
        pSettings->session = atoi(session);
    }
    pSettings->mode = NEXUS_ClientMode_eProtected;
    if (password) {
        if (nxclient_p_parse_password_file(password, &pSettings->certificate)) {
           pSettings->mode = NEXUS_ClientMode_eUntrusted;
        } else {
           pSettings->mode = NEXUS_ClientMode_eVerified;
        }
    }
}

void NxClient_GetDefaultAllocSettings(NxClient_AllocSettings *pSettings)
{
    memset(pSettings, 0, sizeof(*pSettings));
}

void NxClient_GetDefaultConnectSettings( NxClient_ConnectSettings *pSettings )
{
    unsigned i;
    memset(pSettings, 0, sizeof(*pSettings));
    for (i=0;i<NXCLIENT_MAX_IDS;i++) {
        pSettings->simpleVideoDecoder[i].windowCapabilities.maxWidth = 1920;
        pSettings->simpleVideoDecoder[i].windowCapabilities.maxHeight = 1080;
        pSettings->simpleVideoDecoder[i].decoderCapabilities.userDataBufferSize = 16 * 1024;
        /* leave encoderCapabilities max values defaulted to 0, which picks any, so we can adapt to various box modes and configurations */
    }
}

void NxClient_GetDefaultStandbySettings(NxClient_StandbySettings *pSettings)
{
     memset(pSettings, 0, sizeof(*pSettings));
}

void NxClient_GetDefaultCallbackThreadSettings( NxClient_CallbackThreadSettings *pSettings )
{
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->interval = 250; /* milliseconds */
}

void NxClient_GetDefaultReconfigSettings( NxClient_ReconfigSettings *pSettings )
{
    memset(pSettings, 0, sizeof(*pSettings));
}

void NxClient_GetDefaultScreenshotSettings( NxClient_ScreenshotSettings *pSettings )
{
    memset(pSettings, 0, sizeof(*pSettings));
}

static struct {
    pthread_t thread;
    bool done;
    NxClient_CallbackThreadSettings settings;
    NxClient_CallbackStatus lastStatus;
} g_callbackThread;
static void *nxclient_p_callback_thread(void *context)
{
    BSTD_UNUSED(context);
    while (!g_callbackThread.done) {
        int rc;
        NxClient_CallbackStatus status;
        rc = NxClient_GetCallbackStatus(&status);
        if (rc) {
            BERR_TRACE(rc);
        }
        else {
            if (g_callbackThread.settings.hdmiOutputHotplug.callback && status.hdmiOutputHotplug != g_callbackThread.lastStatus.hdmiOutputHotplug) {
                (g_callbackThread.settings.hdmiOutputHotplug.callback)(g_callbackThread.settings.hdmiOutputHotplug.context, g_callbackThread.settings.hdmiOutputHotplug.param);
            }
            if (g_callbackThread.settings.hdmiOutputHdcpChanged.callback && status.hdmiOutputHdcpChanged != g_callbackThread.lastStatus.hdmiOutputHdcpChanged) {
                (g_callbackThread.settings.hdmiOutputHdcpChanged.callback)(g_callbackThread.settings.hdmiOutputHdcpChanged.context, g_callbackThread.settings.hdmiOutputHdcpChanged.param);
            }
            if (g_callbackThread.settings.displaySettingsChanged.callback && status.displaySettingsChanged != g_callbackThread.lastStatus.displaySettingsChanged) {
                (g_callbackThread.settings.displaySettingsChanged.callback)(g_callbackThread.settings.displaySettingsChanged.context, g_callbackThread.settings.displaySettingsChanged.param);
            }
            g_callbackThread.lastStatus = status;
        }
        BKNI_Sleep(g_callbackThread.settings.interval);
    }
    return NULL;
}
NEXUS_Error NxClient_StartCallbackThread( const NxClient_CallbackThreadSettings *pSettings )
{
    int rc;
    if (g_callbackThread.thread) {
        return BERR_TRACE(NEXUS_NOT_AVAILABLE);
    }
    g_callbackThread.settings = *pSettings;
    g_callbackThread.done = false;
    rc = NxClient_GetCallbackStatus(&g_callbackThread.lastStatus);
    if (rc) return BERR_TRACE(rc);
    rc = pthread_create(&g_callbackThread.thread, NULL, nxclient_p_callback_thread, NULL);
    if (rc) return BERR_TRACE(rc);
    return 0;
}

void NxClient_StopCallbackThread(void)
{
    if (g_callbackThread.thread) {
        g_callbackThread.done = true;
        pthread_join(g_callbackThread.thread, NULL);
        g_callbackThread.thread = 0;
    }
}

void NxClient_GetDefaultClientModeSettings( NxClient_ClientModeSettings *pSettings )
{
    NxClient_JoinSettings joinSettings;
    NxClient_GetDefaultJoinSettings(&joinSettings);
    memset(pSettings, 0, sizeof(*pSettings));
    pSettings->mode = joinSettings.mode;
    pSettings->certificate = joinSettings.certificate;
}
