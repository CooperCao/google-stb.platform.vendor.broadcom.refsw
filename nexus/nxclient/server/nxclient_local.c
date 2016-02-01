/******************************************************************************
 *    (c)2010-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 *****************************************************************************/
#include "nxclient.h"
#include "nxserver_ipc_types.h"
#include "nxserverlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static unsigned NxClient_P_RegisterAcknowledgeStandby(void);

static struct {
    nxclient_t client;
    nxserver_t server;
    bool external_init;
    unsigned refcnt;
    BKNI_MutexHandle mutex;
    struct {
        unsigned id;
        bool used;
    } implicitAckStandby;
} g_state;

#define LOCK() BKNI_AcquireMutex(g_state.mutex)
#define UNLOCK() BKNI_ReleaseMutex(g_state.mutex)
static pthread_mutex_t g_joinMutex = PTHREAD_MUTEX_INITIALIZER;

NEXUS_Error NxClient_Join(const NxClient_JoinSettings *pSettings)
{
    int rc = 0;
    /* init the server with default params.
    require statically-initialized mutex to avoid race on refcnt. */
    pthread_mutex_lock(&g_joinMutex);
    if (!g_state.external_init && !g_state.refcnt) {
        struct nxserver_settings settings;
        g_state.server = nxserverlib_get_singleton();
        if (g_state.server) {
            /* if nxserverlib is already up, we use it and won't close it */
            g_state.external_init = true;
        }
        else {
            g_state.server = nxserver_init(0, NULL, false);
            if (!g_state.server) {
                pthread_mutex_unlock(&g_joinMutex);
                fprintf(stderr, "### nxserver_init failed\n");
                return -1;
            }
        }
        nxserverlib_get_settings(g_state.server, &settings);
        g_state.mutex = settings.lock;
    }
    pthread_mutex_unlock(&g_joinMutex);

    LOCK();
    if (!g_state.client) {
        g_state.client = NxClient_P_CreateClient(g_state.server, pSettings, NULL, 0);
        if (!g_state.client) {
            rc = BERR_TRACE(NEXUS_UNKNOWN);
            goto done;
        }
        if (!pSettings || !pSettings->ignoreStandbyRequest) {
            g_state.implicitAckStandby.id = NxClient_P_RegisterAcknowledgeStandby();
            g_state.implicitAckStandby.used = false;
        }
    }
    g_state.refcnt++;
done:
    UNLOCK();
    return rc;
}

void NxClient_Uninit(void)
{
    LOCK();
    if (--g_state.refcnt == 0) {
        NxClient_P_DestroyClient(g_state.client);
        g_state.client = NULL;
    }
    UNLOCK();

    if (!g_state.external_init && !g_state.refcnt) {
        nxserver_uninit(g_state.server);
    }
    return;
}

NEXUS_Error NxClient_Alloc(const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults)
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Alloc(g_state.client, pSettings, pResults);
    UNLOCK();
    return rc;
}

void NxClient_Free(const NxClient_AllocResults *pResults)
{
    LOCK();
    NxClient_P_Free(g_state.client, pResults);
    UNLOCK();
}

NEXUS_Error NxClient_Connect( const NxClient_ConnectSettings *pSettings, unsigned *pRequestId )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Connect(g_state.client, pSettings, pRequestId);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_RefreshConnect(unsigned connectId)
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_RefreshConnect(g_state.client, connectId);
    UNLOCK();
    return rc;
}

void NxClient_Disconnect(unsigned requestId)
{
    LOCK();
    NxClient_P_Disconnect(g_state.client, requestId);
    UNLOCK();
}

void NxClient_GetAudioSettings( NxClient_AudioSettings *pSettings )
{
    LOCK();
    NxClient_P_GetAudioSettings(g_state.client, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetAudioSettings( const NxClient_AudioSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_SetAudioSettings(g_state.client, pSettings);
    UNLOCK();
    return rc;
}

void NxClient_GetDisplaySettings( NxClient_DisplaySettings *pSettings )
{
    LOCK();
    NxClient_P_GetDisplaySettings(g_state.client, NULL, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetDisplaySettings( const NxClient_DisplaySettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_SetDisplaySettings(g_state.client, NULL, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetDisplayStatus( NxClient_DisplayStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_GetDisplayStatus(nxserver_get_client_session(g_state.client), pStatus);
    UNLOCK();
    return rc;
}

void NxClient_GetSurfaceClientComposition( unsigned surfaceClientId, NEXUS_SurfaceComposition *pSettings )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.get_composition.surfaceClientId = surfaceClientId;
    LOCK();
    NxClient_P_General(g_state.client, nxclient_p_general_param_type_get_composition, &param, &output);
    UNLOCK();
    *pSettings = output.get_composition.composition;
}

NEXUS_Error NxClient_SetSurfaceClientComposition( unsigned surfaceClientId, const NEXUS_SurfaceComposition *pSettings )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    NEXUS_Error rc;
    param.set_composition.surfaceClientId = surfaceClientId;
    param.set_composition.composition = *pSettings;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_composition, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetStandbyStatus(NxClient_StandbyStatus *pStatus)
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_GetStandbyStatus(g_state.server, pStatus); 
    UNLOCK();
    return rc;
}

static unsigned NxClient_P_RegisterAcknowledgeStandby(void)
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_register_acknowledge_standby, &param, &output);
    return rc ? 0 : output.register_acknowledge_standby.id;
}

unsigned NxClient_RegisterAcknowledgeStandby(void)
{
    unsigned id;
    LOCK();
    if (g_state.implicitAckStandby.id && !g_state.implicitAckStandby.used) {
        g_state.implicitAckStandby.used = true;
        id = g_state.implicitAckStandby.id;
    }
    else {
        id = NxClient_P_RegisterAcknowledgeStandby();
    }
    UNLOCK();
    return id;
}

void NxClient_UnregisterAcknowledgeStandby( unsigned id )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.unregister_acknowledge_standby.id = id;
    LOCK();
    (void)NxClient_P_General(g_state.client, nxclient_p_general_param_type_unregister_acknowledge_standby, &param, &output);
    UNLOCK();
}

void NxClient_AcknowledgeStandby(unsigned id)
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK();
    if (g_state.implicitAckStandby.id && !g_state.implicitAckStandby.used) {
        /* if NxClient_RegisterAcknowledgeStandby was never called, then for backward compat we ignore the id given. */
        param.acknowledge_standby.id = g_state.implicitAckStandby.id;
    }
    else {
        param.acknowledge_standby.id = id;
    }
    (void)NxClient_P_General(g_state.client, nxclient_p_general_param_type_acknowledge_standby, &param, &output);
    UNLOCK();
}

NEXUS_Error NxClient_SetStandbySettings(const NxClient_StandbySettings *pSettings)
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_SetStandbySettings(g_state.client, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Config_GetJoinSettings( NEXUS_ClientHandle client, NxClient_JoinSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Config_GetJoinSettings(g_state.client, client, pSettings);
    UNLOCK();
    return rc;
}

void NxClient_Config_GetSurfaceClientComposition(NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition )
{
    LOCK();
    NxClient_P_Config_GetSurfaceClientComposition(g_state.client, client, surfaceClient, pComposition);
    UNLOCK();
}

NEXUS_Error NxClient_Config_SetSurfaceClientComposition(NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Config_SetSurfaceClientComposition(g_state.client, client, surfaceClient, pComposition);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Config_GetConnectList( NEXUS_ClientHandle client, NxClient_ConnectList *pList )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Config_GetConnectList(g_state.client, client, pList);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Config_RefreshConnect( NEXUS_ClientHandle client, unsigned connectId )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Config_RefreshConnect(g_state.client, client, connectId);
    UNLOCK();
    return rc;
}

void NxClient_Config_GetConnectSettings( NEXUS_ClientHandle client, unsigned connectId, NxClient_ConnectSettings *pSettings )
{
    LOCK();
    NxClient_P_Config_GetConnectSettings(g_state.client, client, connectId, pSettings);
    UNLOCK();
}

void NxClient_Config_GetInputClientServerFilter( NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned *pFilter )
{
    LOCK();
    NxClient_P_Config_GetInputClientServerFilter(g_state.client, client, inputClient, pFilter);
    UNLOCK();
}

NEXUS_Error NxClient_Config_SetInputClientServerFilter( NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned filter )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_Config_SetInputClientServerFilter(g_state.client, client, inputClient, filter);
    UNLOCK();
    return rc;
}

void NxClient_GetPictureQualitySettings( NxClient_PictureQualitySettings *pSettings )
{
    LOCK();
    NxClient_P_GetPictureQualitySettings(g_state.client, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetPictureQualitySettings( const NxClient_PictureQualitySettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_SetPictureQualitySettings(g_state.client, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetCallbackStatus( NxClient_CallbackStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_GetCallbackStatus(g_state.client, pStatus);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetAudioStatus(NxClient_AudioStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = NxClient_P_GetAudioStatus(g_state.client, pStatus);
    UNLOCK();
    return rc;
}

#undef MIN
#define MIN(A,B) ((A)>(B)?(B):(A))

NEXUS_Error NxClient_Display_WriteTeletext( const NEXUS_TeletextLine *pLines, size_t numLines, size_t *pNumLinesWritten )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.write_teletext.numLines = MIN(numLines, sizeof(param.write_teletext.lines)/sizeof(param.write_teletext.lines[0]));
    memcpy(param.write_teletext.lines, pLines, param.write_teletext.numLines * sizeof(param.write_teletext.lines[0]));
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_write_teletext, &param, &output);
    UNLOCK();
    *pNumLinesWritten = output.write_teletext.numLinesWritten;
    return rc;
}

NEXUS_Error NxClient_Display_WriteClosedCaption( const NEXUS_ClosedCaptionData *pEntries, size_t numEntries, size_t *pNumEntriesWritten )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.write_closed_caption.numEntries = MIN(numEntries, sizeof(param.write_closed_caption.entries)/sizeof(param.write_closed_caption.entries[0]));
    memcpy(param.write_closed_caption.entries, pEntries, param.write_closed_caption.numEntries * sizeof(param.write_closed_caption.entries[0]));
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_write_closed_caption, &param, &output);
    UNLOCK();
    *pNumEntriesWritten = output.write_closed_caption.numEntriesWritten;
    return rc;
}

NEXUS_Error NxClient_Display_SetWss( uint16_t wssData )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_wss.data = wssData;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_wss, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetCgms( uint32_t cgmsData )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_cgms.data = cgmsData;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_cgms, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetCgmsB( const uint32_t *pCgmsData, unsigned size )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    if (size > sizeof(param.set_cgms_b.data)/sizeof(param.set_cgms_b.data[0])) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    memcpy(param.set_cgms_b.data, pCgmsData, size*sizeof(param.set_cgms_b.data[0]));
    param.set_cgms_b.size = size;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_cgms_b, &param, &output);
    UNLOCK();
    return rc;
}

void NxClient_GetAudioProcessingSettings( NxClient_AudioProcessingSettings *pSettings )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK();
    NxClient_P_General(g_state.client, nxclient_p_general_param_type_get_audio_processing_settings, &param, &output);
    UNLOCK();
    *pSettings = output.get_audio_processing_settings.settings;
}

NEXUS_Error NxClient_SetAudioProcessingSettings( const NxClient_AudioProcessingSettings *pSettings )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_audio_processing_settings.settings = *pSettings;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_audio_processing_settings, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Reconfig( const NxClient_ReconfigSettings *pSettings )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.reconfig.settings = *pSettings;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_reconfig, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Screenshot( const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    if (pSettings) {
        param.screenshot.settings = *pSettings;
    }
    else {
        NxClient_GetDefaultScreenshotSettings(&param.screenshot.settings);
    }
    param.screenshot.surface = surface;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_screenshot, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetMacrovision( NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_macrovision.type = type;
    param.set_macrovision.table_isnull = pTable == NULL;
    if (pTable) {
        param.set_macrovision.table = *pTable;
    }
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_set_macrovision, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GrowHeap( unsigned heapIndex )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.grow_heap.heapIndex = heapIndex;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_grow_heap, &param, &output);
    UNLOCK();
    return rc;
}

void NxClient_ShrinkHeap( unsigned heapIndex )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.shrink_heap.heapIndex = heapIndex;
    LOCK();
    NxClient_P_General(g_state.client, nxclient_p_general_param_type_shrink_heap, &param, &output);
    UNLOCK();
}

NEXUS_ClientHandle NxClient_Config_LookupClient( unsigned pid )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.lookup_client.pid = pid;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_lookup_client, &param, &output);
    UNLOCK();
    return rc ? NULL : output.lookup_client.handle;
}

NEXUS_Error NxClient_Display_GetCrcData( unsigned displayIndex, NxClient_DisplayCrcData *pData )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.display_get_crc_data.displayIndex = displayIndex;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_display_get_crc_data, &param, &output);
    UNLOCK();
    if (!rc) *pData = output.display_get_crc_data.data;
    return rc;
}

NEXUS_Error NxClient_HdmiOutput_GetCrcData( NxClient_HdmiOutputCrcData *pData )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_hdmi_output_get_crc_data, &param, &output);
    UNLOCK();
    if (!rc) *pData = output.hdmi_output_get_crc_data.data;
    return rc;
}

NEXUS_Error NxClient_LoadHdcpKeys( NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset, unsigned size )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.load_hdcp_keys.hdcpType = hdcpType;
    param.load_hdcp_keys.block = block;
    param.load_hdcp_keys.blockOffset = blockOffset;
    param.load_hdcp_keys.size = size;
    LOCK();
    rc = NxClient_P_General(g_state.client, nxclient_p_general_param_type_load_hdcp_keys, &param, &output);
    UNLOCK();
    return rc;
}
