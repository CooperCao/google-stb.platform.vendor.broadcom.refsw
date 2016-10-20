/***************************************************************************
*  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
*
*  This program is the proprietary software of Broadcom and/or its licensors,
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
***************************************************************************/
NEXUS_Error NxClient_Alloc(const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults)
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_alloc(NXCLIENT_STATE_RESTRICTED, pSettings, pResults);
    UNLOCK();
    return rc;
}

void NxClient_Free(const NxClient_AllocResults *pResults)
{
    LOCK();
    nxclient_p_free(NXCLIENT_STATE_RESTRICTED, pResults);
    UNLOCK();
}

NEXUS_Error NxClient_Connect( const NxClient_ConnectSettings *pSettings, unsigned *pConnectId )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_connect(NXCLIENT_STATE_RESTRICTED, pSettings, pConnectId);
    UNLOCK();
    if (rc) *pConnectId = 0;
    return rc;
}

NEXUS_Error NxClient_RefreshConnect(unsigned connectId)
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_refresh_connect(NXCLIENT_STATE_RESTRICTED, connectId);
    UNLOCK();
    return rc;
}

void NxClient_Disconnect(unsigned connectId)
{
    LOCK();
    nxclient_p_disconnect(NXCLIENT_STATE_RESTRICTED, connectId);
    UNLOCK();
}

void NxClient_GetAudioSettings( NxClient_AudioSettings *pSettings )
{
    LOCK();
    nxclient_p_get_audio_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetAudioSettings( const NxClient_AudioSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_audio_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

void NxClient_GetDisplaySettings( NxClient_DisplaySettings *pSettings )
{
    LOCK();
    nxclient_p_get_display_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetDisplaySettings( const NxClient_DisplaySettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_display_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetDisplayStatus( NxClient_DisplayStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_display_status(NXCLIENT_STATE_RESTRICTED, pStatus);
    UNLOCK();
    return rc;
}

void NxClient_GetSurfaceClientComposition( unsigned surfaceClientId, NEXUS_SurfaceComposition *pSettings )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.get_composition.surfaceClientId = surfaceClientId;
    LOCK();
    nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_get_composition, &param, &output);
    UNLOCK();
    *pSettings = output.get_composition.composition;
}

NEXUS_Error NxClient_SetSurfaceClientComposition( unsigned surfaceClientId, const NEXUS_SurfaceComposition *pSettings )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_composition.surfaceClientId = surfaceClientId;
    param.set_composition.composition = *pSettings;
    LOCK();
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_composition, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetStandbyStatus(NxClient_StandbyStatus *pStatus)
{
    NEXUS_Error rc;
    LOCK_STANDBY();
    rc = nxclient_p_get_standby_status(NXCLIENT_STATE_REGULAR, pStatus);
    UNLOCK_STANDBY();
    return rc;
}

static unsigned NxClient_P_RegisterAcknowledgeStandby(void)
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    rc = nxclient_p_general(NXCLIENT_STATE_REGULAR, nxclient_p_general_param_type_register_acknowledge_standby, &param, &output);
    return rc ? 0 : output.register_acknowledge_standby.id;
}

unsigned NxClient_RegisterAcknowledgeStandby(void)
{
    unsigned id;
    LOCK_STANDBY();
    if (nxclient_state.implicitAckStandby.id && !nxclient_state.implicitAckStandby.used) {
        nxclient_state.implicitAckStandby.used = true;
        id = nxclient_state.implicitAckStandby.id;
    }
    else {
        id = NxClient_P_RegisterAcknowledgeStandby();
    }
    UNLOCK_STANDBY();
    return id;
}

void NxClient_UnregisterAcknowledgeStandby( unsigned id )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.unregister_acknowledge_standby.id = id;
    LOCK_STANDBY();
    (void)nxclient_p_general(NXCLIENT_STATE_REGULAR, nxclient_p_general_param_type_unregister_acknowledge_standby, &param, &output);
    UNLOCK_STANDBY();
}

void NxClient_AcknowledgeStandby(unsigned id)
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK_STANDBY();
    if (nxclient_state.implicitAckStandby.id && !nxclient_state.implicitAckStandby.used) {
        /* if NxClient_RegisterAcknowledgeStandby was never called, then for backward compat we ignore the id given. */
        param.acknowledge_standby.id = nxclient_state.implicitAckStandby.id;
    }
    else {
        param.acknowledge_standby.id = id;
    }
    (void)nxclient_p_general(NXCLIENT_STATE_REGULAR, nxclient_p_general_param_type_acknowledge_standby, &param, &output);
    UNLOCK_STANDBY();
}

NEXUS_Error NxClient_SetStandbySettings(const NxClient_StandbySettings *pSettings)
{
    NEXUS_Error rc;
    LOCK_STANDBY();
    rc = nxclient_p_set_standby_settings(NXCLIENT_STATE_REGULAR, pSettings);
    UNLOCK_STANDBY();
    return rc;
}

NEXUS_Error NxClient_Config_GetJoinSettings( NEXUS_ClientHandle client, NxClient_JoinSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_config_get_join_settings(NXCLIENT_STATE_RESTRICTED, client, pSettings);
    UNLOCK();
    return rc;
}

void NxClient_Config_GetSurfaceClientComposition(NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition )
{
    LOCK();
    nxclient_p_config_get_surface_client_composition(NXCLIENT_STATE_RESTRICTED, client, surfaceClient, pComposition);
    UNLOCK();
}

NEXUS_Error NxClient_Config_SetSurfaceClientComposition(NEXUS_ClientHandle client, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_config_set_surface_client_composition(NXCLIENT_STATE_RESTRICTED, client, surfaceClient, pComposition);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Config_GetConnectList( NEXUS_ClientHandle client, NxClient_ConnectList *pList )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_config_get_connect_list(NXCLIENT_STATE_RESTRICTED, client, pList);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Config_RefreshConnect( NEXUS_ClientHandle client, unsigned connectId )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_config_refresh_connect(NXCLIENT_STATE_RESTRICTED, client, connectId);
    UNLOCK();
    return rc;
}

void NxClient_Config_GetConnectSettings( NEXUS_ClientHandle client, unsigned connectId, NxClient_ConnectSettings *pSettings )
{
    LOCK();
    nxclient_p_config_get_connect_settings(NXCLIENT_STATE_RESTRICTED, client, connectId, pSettings);
    UNLOCK();
}

void NxClient_Config_GetInputClientServerFilter( NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned *pFilter )
{
    LOCK();
    nxclient_p_config_get_input_client_server_filter(NXCLIENT_STATE_RESTRICTED, client, inputClient, pFilter);
    UNLOCK();
}

NEXUS_Error NxClient_Config_SetInputClientServerFilter( NEXUS_ClientHandle client, NEXUS_InputClientHandle inputClient, unsigned filter )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_config_set_input_client_server_filter(NXCLIENT_STATE_RESTRICTED, client, inputClient, filter);
    UNLOCK();
    return rc;
}

void NxClient_GetPictureQualitySettings( NxClient_PictureQualitySettings *pSettings )
{
    LOCK();
    nxclient_p_get_picture_quality_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetPictureQualitySettings( const NxClient_PictureQualitySettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_picture_quality_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetCallbackStatus( NxClient_CallbackStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_callback_status(NXCLIENT_STATE_RESTRICTED, pStatus);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetAudioStatus( NxClient_AudioStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_audio_status(NXCLIENT_STATE_RESTRICTED, pStatus);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_write_teletext, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_write_closed_caption, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_wss, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_cgms, &param, &output);
    UNLOCK();
    return rc;
}

void NxClient_GetAudioProcessingSettings( NxClient_AudioProcessingSettings *pSettings )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK();
    nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_get_audio_processing_settings, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_audio_processing_settings, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_reconfig, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_screenshot, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_macrovision, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_grow_heap, &param, &output);
    UNLOCK();
    return rc;
}

void NxClient_ShrinkHeap( unsigned heapIndex )
{
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.shrink_heap.heapIndex = heapIndex;
    LOCK();
    nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_shrink_heap, &param, &output);
    UNLOCK();
}

NEXUS_ClientHandle NxClient_Config_LookupClient( unsigned pid )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.lookup_client.pid = pid;
    LOCK();
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_lookup_client, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_display_get_crc_data, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_hdmi_output_get_crc_data, &param, &output);
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
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_load_hdcp_keys, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_SetSlaveDisplayGraphics( unsigned slaveDisplay, NEXUS_SurfaceHandle surface )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    param.set_slave_display_graphics.slaveDisplay = slaveDisplay;
    param.set_slave_display_graphics.surface = surface;
    LOCK();
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_set_slave_display_graphics, &param, &output);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetStatus( NxClient_Status *pStatus )
{
    NEXUS_Error rc;
    nxclient_p_general_param param;
    nxclient_p_general_output output;
    LOCK();
    rc = nxclient_p_general(NXCLIENT_STATE_RESTRICTED, nxclient_p_general_param_type_get_status, &param, &output);
    UNLOCK();
    if (!rc) *pStatus = output.get_status.status;
    return rc;
}
