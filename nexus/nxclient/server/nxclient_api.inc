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
 ******************************************************************************/
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
    LOCK();
    nxclient_p_get_surface_client_composition(NXCLIENT_STATE_RESTRICTED, surfaceClientId, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetSurfaceClientComposition( unsigned surfaceClientId, const NEXUS_SurfaceComposition *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_surface_client_composition(NXCLIENT_STATE_RESTRICTED, surfaceClientId, pSettings);
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
    unsigned id;
    rc = nxclient_p_register_acknowledge_standby(NXCLIENT_STATE_REGULAR, &id);
    return rc ? 0 : id;
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
    LOCK_STANDBY();
    nxclient_p_unregister_acknowledge_standby(NXCLIENT_STATE_REGULAR, id);
    UNLOCK_STANDBY();
}

void NxClient_AcknowledgeStandby(unsigned id)
{
    LOCK_STANDBY();
    if (nxclient_state.implicitAckStandby.id && !nxclient_state.implicitAckStandby.used) {
        /* if NxClient_RegisterAcknowledgeStandby was never called, then for backward compat we ignore the id given. */
        id = nxclient_state.implicitAckStandby.id;
    }
    nxclient_p_acknowledge_standby(NXCLIENT_STATE_REGULAR, id);
    UNLOCK_STANDBY();
    return;
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
    nxclient_p_teletext_data data;

    numLines = MIN(numLines, sizeof(data.lines)/sizeof(data.lines[0]));
    memcpy(data.lines, pLines, numLines * sizeof(*pLines));
    LOCK();
    rc = nxclient_p_write_teletext(NXCLIENT_STATE_RESTRICTED, &data, numLines, pNumLinesWritten);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_WriteClosedCaption( const NEXUS_ClosedCaptionData *pEntries, size_t numEntries, size_t *pNumEntriesWritten )
{
    nxclient_p_closecaption_data data;
    NEXUS_Error rc;

    numEntries = MIN(numEntries, sizeof(data.entries)/sizeof(data.entries[0]));
    memcpy(data.entries, pEntries, numEntries * sizeof(*pEntries));
    LOCK();
    rc = nxclient_p_write_closedcaption(NXCLIENT_STATE_RESTRICTED, &data, numEntries, pNumEntriesWritten);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetWss( uint16_t wssData )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_display_set_wss(NXCLIENT_STATE_RESTRICTED, wssData);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetCgms( uint32_t cgmsData )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_display_set_cgms(NXCLIENT_STATE_RESTRICTED, cgmsData);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetCgmsB( const uint32_t *pCgmsData, unsigned size )
{
    NEXUS_Error rc;
    nxclient_p_set_cgms_b_data data;
    if (size > sizeof(data.data)/sizeof(data.data[0])) {
        return BERR_TRACE(NEXUS_INVALID_PARAMETER);
    }
    BKNI_Memcpy(data.data, pCgmsData, size * sizeof(data.data[0]));
    data.size = size;
    LOCK();
    rc = nxclient_p_display_set_cgms_b(NXCLIENT_STATE_RESTRICTED, &data);
    UNLOCK();
    return rc;
}

void NxClient_GetAudioProcessingSettings( NxClient_AudioProcessingSettings *pSettings )
{
    LOCK();
    nxclient_p_get_audio_processing_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
}

NEXUS_Error NxClient_SetAudioProcessingSettings( const NxClient_AudioProcessingSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_audio_processing_settings(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Reconfig( const NxClient_ReconfigSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_reconfig(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Screenshot( const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface )
{
    NEXUS_Error rc;
    NxClient_ScreenshotSettings settings;

    if (pSettings) {
        settings = *pSettings;
    }
    else {
        NxClient_GetDefaultScreenshotSettings(&settings);
    }
    LOCK();
    rc = nxclient_p_screenshot(NXCLIENT_STATE_RESTRICTED, &settings, surface);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_Display_SetMacrovision( NEXUS_DisplayMacrovisionType type, const NEXUS_DisplayMacrovisionTables *pTable )
{
    NEXUS_Error rc;
    NEXUS_DisplayMacrovisionTables  table;
    bool pTable_isNull = false;

    if (pTable==NULL) {
        pTable = &table;
        pTable_isNull = true;
        BKNI_Memset(&table, 0, sizeof(table));
    }
    LOCK();
    rc = nxclient_p_display_set_macrovision(NXCLIENT_STATE_RESTRICTED, type, pTable_isNull, pTable);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GrowHeap( unsigned heapIndex )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_grow_heap(NXCLIENT_STATE_RESTRICTED, heapIndex);
    UNLOCK();
    return rc;
}

void NxClient_ShrinkHeap( unsigned heapIndex )
{
    LOCK();
    nxclient_p_shrink_heap(NXCLIENT_STATE_RESTRICTED, heapIndex);
    UNLOCK();
}

NEXUS_ClientHandle NxClient_Config_LookupClient( unsigned pid )
{
    NEXUS_Error rc;
    NEXUS_ClientHandle handle;
    LOCK();
    rc = nxclient_p_config_lookup_client(NXCLIENT_STATE_RESTRICTED, pid, &handle);
    UNLOCK();
    return rc ? NULL : handle;
}

NEXUS_Error NxClient_Display_GetCrcData( unsigned displayIndex, NxClient_DisplayCrcData *pData )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_display_get_crc_data(NXCLIENT_STATE_RESTRICTED, displayIndex, pData);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_HdmiOutput_GetCrcData( NxClient_HdmiOutputCrcData *pData )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_hdmi_output_get_crc_data(NXCLIENT_STATE_RESTRICTED, pData);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_LoadHdcpKeys( NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset, unsigned size )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_load_hdcp_keys(NXCLIENT_STATE_RESTRICTED, hdcpType, block, blockOffset, size);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_SetHdmiInputRepeater(NEXUS_HdmiInputHandle hdmiInput){
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_hdmi_input_repeater(NXCLIENT_STATE_RESTRICTED, hdmiInput);
    UNLOCK();
    return rc;
}


NEXUS_Error NxClient_SetSlaveDisplayGraphics( unsigned slaveDisplay, NEXUS_SurfaceHandle surface )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_slave_display_graphics(NXCLIENT_STATE_RESTRICTED, slaveDisplay, surface);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetStatus( NxClient_Status *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_status(NXCLIENT_STATE_RESTRICTED, pStatus);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_SetClientMode( const NxClient_ClientModeSettings *pSettings )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_client_mode(NXCLIENT_STATE_RESTRICTED, pSettings);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_GetThermalStatus( NxClient_ThermalStatus *pStatus )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_thermal_status(NXCLIENT_STATE_RESTRICTED, pStatus);
    UNLOCK();
    return rc;
}

void NxClient_GetThermalConfigurationList( NxClient_ThermalConfigurationList *pConfigList )
{
    LOCK();
    nxclient_p_get_thermal_configuration_list(NXCLIENT_STATE_RESTRICTED, pConfigList);
    UNLOCK();
}

NEXUS_Error NxClient_GetThermalConfiguration( unsigned tempThreshold, NxClient_ThermalConfiguration *pConfig )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_get_thermal_configuration(NXCLIENT_STATE_RESTRICTED, tempThreshold, pConfig);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_SetThermalConfiguration( unsigned tempThreshold, const NxClient_ThermalConfiguration *pConfig )
{
    NEXUS_Error rc;
    NxClient_ThermalConfiguration config;
    bool pConfig_isNull = false;

    if (pConfig==NULL) {
        pConfig = &config;
        pConfig_isNull = true;
        BKNI_Memset(&config, 0, sizeof(config));
    }

    LOCK();
    rc = nxclient_p_set_thermal_configuration(NXCLIENT_STATE_RESTRICTED, tempThreshold, pConfig_isNull, pConfig);
    UNLOCK();
    return rc;
}

NEXUS_Error NxClient_SetWatchdogTimeout( unsigned timeout )
{
    NEXUS_Error rc;
    LOCK();
    rc = nxclient_p_set_watchdog_timeout(NXCLIENT_STATE_RESTRICTED, timeout);
    UNLOCK();
    return rc;
}
