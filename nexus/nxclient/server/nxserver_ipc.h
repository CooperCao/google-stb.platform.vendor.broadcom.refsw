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
 ******************************************************************************/
#ifndef NXSERVER_IPC_H__
#define NXSERVER_IPC_H__

#include "bipc.h"
#include "nxclient.h"
#include "nxclient_config.h"
#include "nexus_platform_client.h"
#include "nexus_surface_compositor.h"
#if NEXUS_HAS_VIDEO_DECODER
#include "nexus_video_decoder.h"
#endif
#include "nxserver_ipc_types.h"

/*
internal IPC API for nxclient to nxserver
*/

typedef struct nxclient_ipc *nxclient_ipc_t;

typedef struct nxclient_p_client_info {
    NEXUS_Certificate certificate;
} nxclient_p_client_info;

typedef enum nxclient_ipc_thread {
    nxclient_ipc_thread_regular,     /* Regular clients which are never blocks Apis like join and standby */
    nxclient_ipc_thread_restricted,  /* Restricted clients which blocks all other Apis during standby */
    nxclient_ipc_thread_max
} nxclient_ipc_thread;

nxclient_ipc_t nxclient_p_create(bipc_t ipc, const NxClient_JoinSettings *pJoinSettings, nxclient_p_client_info *info, nxclient_ipc_thread id);
void  nxclient_p_destroy(nxclient_ipc_t client);

int nxclient_p_alloc(nxclient_ipc_t client, const NxClient_AllocSettings *pSettings, NxClient_AllocResults *pResults);
void nxclient_p_free(nxclient_ipc_t client, const NxClient_AllocResults *pResults);
int nxclient_p_connect(nxclient_ipc_t client, const NxClient_ConnectSettings *pSettings, unsigned *pConnectId);
int nxclient_p_refresh_connect(nxclient_ipc_t client, unsigned connectId);
void nxclient_p_disconnect(nxclient_ipc_t client, unsigned connectId);

void nxclient_p_get_display_settings(nxclient_ipc_t client, NxClient_DisplaySettings *pSettings);
int  nxclient_p_set_display_settings(nxclient_ipc_t client, const NxClient_DisplaySettings *pSettings);
int  nxclient_p_get_display_status(nxclient_ipc_t client, NxClient_DisplayStatus *pStatus);
void nxclient_p_get_audio_settings(nxclient_ipc_t client, NxClient_AudioSettings *pSettings);
int  nxclient_p_set_audio_settings(nxclient_ipc_t client, const NxClient_AudioSettings *pSettings);
void nxclient_p_get_surface_client_composition(nxclient_ipc_t client, unsigned surfaceClientId, NEXUS_SurfaceComposition *composition);
int  nxclient_p_set_surface_client_composition(nxclient_ipc_t client, unsigned surfaceClientId, const NEXUS_SurfaceComposition *composition);
int  nxclient_p_write_teletext(nxclient_ipc_t client, const nxclient_p_teletext_data *data, size_t numLines, size_t *pNumLinesWritten);
int  nxclient_p_write_closedcaption(nxclient_ipc_t client, const nxclient_p_closecaption_data *data, size_t numEntries, size_t *pNumEntriesWritten );
int  nxclient_p_display_set_wss(nxclient_ipc_t client, uint16_t wssData);
int  nxclient_p_display_set_cgms(nxclient_ipc_t client, uint32_t cgmsData);
int  nxclient_p_display_set_cgms_b(nxclient_ipc_t client, const nxclient_p_set_cgms_b_data *pdata);
void nxclient_p_get_audio_processing_settings(nxclient_ipc_t client, NxClient_AudioProcessingSettings *pSettings);
int  nxclient_p_set_audio_processing_settings(nxclient_ipc_t client, const NxClient_AudioProcessingSettings *pSettings);
int  nxclient_p_reconfig(nxclient_ipc_t client, const NxClient_ReconfigSettings *pSettings);
int  nxclient_p_screenshot(nxclient_ipc_t client, const NxClient_ScreenshotSettings *pSettings, NEXUS_SurfaceHandle surface);
int  nxclient_p_display_set_macrovision(nxclient_ipc_t client, NEXUS_DisplayMacrovisionType type, bool pTable_isNull, const NEXUS_DisplayMacrovisionTables *pTable);
int  nxclient_p_grow_heap(nxclient_ipc_t client, unsigned heapIndex );
void nxclient_p_shrink_heap(nxclient_ipc_t client, unsigned heapIndex );
int  nxclient_p_config_lookup_client(nxclient_ipc_t client, unsigned pid, NEXUS_ClientHandle *pHandle);
int  nxclient_p_display_get_crc_data(nxclient_ipc_t client, unsigned displayIndex, NxClient_DisplayCrcData *pData);
int  nxclient_p_hdmi_output_get_crc_data(nxclient_ipc_t client, NxClient_HdmiOutputCrcData *pData );
int  nxclient_p_register_acknowledge_standby(nxclient_ipc_t client, unsigned *id);
void nxclient_p_unregister_acknowledge_standby(nxclient_ipc_t client, unsigned id );
void nxclient_p_acknowledge_standby(nxclient_ipc_t client, unsigned id );
int  nxclient_p_load_hdcp_keys(nxclient_ipc_t client, NxClient_HdcpType hdcpType, NEXUS_MemoryBlockHandle block, unsigned blockOffset,unsigned size);
int  nxclient_p_set_hdmi_input_repeater(nxclient_ipc_t client, NEXUS_HdmiInputHandle hdmiInput);
int  nxclient_p_set_slave_display_graphics(nxclient_ipc_t client, unsigned slaveDisplay, NEXUS_SurfaceHandle surface);
int  nxclient_p_get_status(nxclient_ipc_t client, NxClient_Status *pStatus);
int  nxclient_p_get_standby_status(nxclient_ipc_t _client, NxClient_StandbyStatus *pStatus);
int  nxclient_p_set_standby_settings(nxclient_ipc_t _client, const NxClient_StandbySettings *pSettings);

int  nxclient_p_config_get_join_settings(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NxClient_JoinSettings *pSettings );
void nxclient_p_config_get_surface_client_composition(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, NEXUS_SurfaceComposition *pComposition );
int  nxclient_p_config_set_surface_client_composition(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NEXUS_SurfaceClientHandle surfaceClient, const NEXUS_SurfaceComposition *pComposition );
int  nxclient_p_config_get_connect_list(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NxClient_ConnectList *pList );
int  nxclient_p_config_refresh_connect(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, unsigned connectId );
void nxclient_p_config_get_connect_settings(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, unsigned connectId, NxClient_ConnectSettings *pSettings );
void nxclient_p_config_get_input_client_server_filter(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned *pFilter );
int  nxclient_p_config_set_input_client_server_filter(nxclient_ipc_t _client, NEXUS_ClientHandle nexusClient, NEXUS_InputClientHandle inputClient, unsigned filter );

void nxclient_p_get_picture_quality_settings(nxclient_ipc_t _client, NxClient_PictureQualitySettings *pSettings );
int  nxclient_p_set_picture_quality_settings(nxclient_ipc_t _client, const NxClient_PictureQualitySettings *pSettings );

int  nxclient_p_get_callback_status(nxclient_ipc_t _client, NxClient_CallbackStatus *pStatus );
int  nxclient_p_get_audio_status(nxclient_ipc_t _client, NxClient_AudioStatus *pStatus );
int  nxclient_p_set_client_mode(nxclient_ipc_t _client, const NxClient_ClientModeSettings *pSettings );
int  nxclient_p_get_thermal_status(nxclient_ipc_t client, NxClient_ThermalStatus *pStatus);

#endif /* NXSERVER_IPC_H__ */

