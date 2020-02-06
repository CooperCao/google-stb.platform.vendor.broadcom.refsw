/***************************************************************************
 *  Copyright (C) 2019 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * Module Description:
 *
 **************************************************************************/
#include "bstd.h"
#include "bkni.h"
#include "nexus_simple_decoder_module.h"
#include "nexus_simple_decoder_impl.h"
#include "nexus_base.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "priv/nexus_video_decoder_priv.h"
#endif
#if NEXUS_HAS_AUDIO
#include "priv/nexus_audio_decoder_priv.h"
#endif

NEXUS_ModuleHandle g_NEXUS_simpleDecoderModule;
NEXUS_SimpleDecoderModuleSettings g_NEXUS_simpleDecoderModuleSettings;

static struct {
    /* DSP is always single-core, and even multi-core HVD does a unified watchdog because of SVP, so there's
    no reason for per-instance watchdog handling. If any decoder watchdogs, we notify simple_encoder and it
    can decide what must restart. */
    BKNI_EventHandle watchdogEvent;
    NEXUS_EventCallbackHandle watchdogEventCallback;
} g_simpledecoder_state;

void NEXUS_SimpleDecoderModule_GetDefaultSettings( NEXUS_SimpleDecoderModuleSettings *pSettings )
{
    BKNI_Memset(pSettings, 0, sizeof(*pSettings));
}

#if BDBG_DEBUG_BUILD
static void NEXUS_SimpleDecoderModule_P_Print(void)
{
    NEXUS_SimpleDecoderModule_P_PrintVideoDecoder();
    NEXUS_SimpleDecoderModule_P_PrintAudioDecoder();
    NEXUS_SimpleDecoderModule_P_PrintEncoder();
}
#endif

NEXUS_ModuleHandle NEXUS_SimpleDecoderModule_Init( const NEXUS_SimpleDecoderModuleSettings *pSettings )
{
    NEXUS_Error rc;
    NEXUS_ModuleSettings moduleSettings;

    BDBG_ASSERT(!g_NEXUS_simpleDecoderModule);
    g_NEXUS_simpleDecoderModuleSettings = *pSettings;

    /* init global module handle */
    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eDefault;
    moduleSettings.passthroughCallbacks = true;
#if BDBG_DEBUG_BUILD
    moduleSettings.dbgPrint = NEXUS_SimpleDecoderModule_P_Print;
    moduleSettings.dbgModules = "nexus_simple_decoder_proc";
#endif
    g_NEXUS_simpleDecoderModule = NEXUS_Module_Create("simple_decoder", &moduleSettings);
    if (!g_NEXUS_simpleDecoderModule) {
        return NULL;
    }

    NEXUS_LockModule();
    rc = BKNI_CreateEvent(&g_simpledecoder_state.watchdogEvent);
    if (rc) {
        BERR_TRACE(rc); /* not fatal for system */
    }
    else {
        g_simpledecoder_state.watchdogEventCallback = NEXUS_RegisterEvent(g_simpledecoder_state.watchdogEvent, nexus_simpleencoder_p_decoder_watchdog, NULL);
#if NEXUS_HAS_VIDEO_DECODER
        NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.videoDecoder);
        NEXUS_VideoDecoder_SetExternalWatchdogEvent_priv(g_simpledecoder_state.watchdogEvent);
        NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.videoDecoder);
#endif
#if NEXUS_HAS_AUDIO
        NEXUS_Module_Lock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
        NEXUS_AudioDecoder_SetExternalWatchdogEvent_priv(g_simpledecoder_state.watchdogEvent);
        NEXUS_Module_Unlock(g_NEXUS_simpleDecoderModuleSettings.modules.audio);
#endif
    }
    NEXUS_UnlockModule();

    return g_NEXUS_simpleDecoderModule;
}

void NEXUS_SimpleDecoderModule_Uninit(void)
{
    NEXUS_LockModule();
    if (g_simpledecoder_state.watchdogEventCallback) {
        NEXUS_UnregisterEvent(g_simpledecoder_state.watchdogEventCallback);
        g_simpledecoder_state.watchdogEventCallback = NULL;
    }
    if (g_simpledecoder_state.watchdogEvent) {
        BKNI_DestroyEvent(g_simpledecoder_state.watchdogEvent);
        g_simpledecoder_state.watchdogEvent = NULL;
    }
    nexus_simplevideodecoder_p_remove_settings_from_cache();
    NEXUS_SimpleAudioDecoderModule_P_UnloadDefaultSettings();
    NEXUS_UnlockModule();
    NEXUS_Module_Destroy(g_NEXUS_simpleDecoderModule);
    g_NEXUS_simpleDecoderModule = NULL;
}
