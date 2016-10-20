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
 *
 * Module Description:
 *
 *****************************************************************************/
#ifndef NXSERVER_IPC_TYPES_H__
#define NXSERVER_IPC_TYPES_H__

#include "nexus_platform_server.h"

/* multiplex multiple nxclient calls through a single enum/struct for lighter touch on adding api's */
enum nxclient_p_general_param_type
{
    nxclient_p_general_param_type_get_composition,
    nxclient_p_general_param_type_set_composition,
    nxclient_p_general_param_type_write_teletext,
    nxclient_p_general_param_type_write_closed_caption,
    nxclient_p_general_param_type_set_wss,
    nxclient_p_general_param_type_set_cgms,
    nxclient_p_general_param_type_set_cgms_b,
    nxclient_p_general_param_type_get_audio_processing_settings,
    nxclient_p_general_param_type_set_audio_processing_settings,
    nxclient_p_general_param_type_reconfig,
    nxclient_p_general_param_type_screenshot,
    nxclient_p_general_param_type_set_macrovision,
    nxclient_p_general_param_type_grow_heap,
    nxclient_p_general_param_type_shrink_heap,
    nxclient_p_general_param_type_lookup_client,
    nxclient_p_general_param_type_display_get_crc_data,
    nxclient_p_general_param_type_hdmi_output_get_crc_data,
    nxclient_p_general_param_type_register_acknowledge_standby,
    nxclient_p_general_param_type_unregister_acknowledge_standby,
    nxclient_p_general_param_type_acknowledge_standby,
    nxclient_p_general_param_type_load_hdcp_keys,
    nxclient_p_general_param_type_set_slave_display_graphics,
    nxclient_p_general_param_type_get_status,
    nxclient_p_general_param_type_max
};
typedef union nxclient_p_general_param {
    struct {
        unsigned surfaceClientId;
        NEXUS_SurfaceComposition composition;
    } set_composition;
    struct {
        unsigned surfaceClientId;
    } get_composition;
    struct {
        NEXUS_TeletextLine lines[4];
        size_t numLines;
    } write_teletext;
    struct {
        NEXUS_ClosedCaptionData entries[4];
        size_t numEntries;
    } write_closed_caption;
    struct {
        uint16_t data;
    } set_wss;
    struct {
        uint32_t data;
    } set_cgms;
    struct {
        uint32_t data[10];
        unsigned size;
    } set_cgms_b;
    struct {
        NxClient_AudioProcessingSettings settings;
    } set_audio_processing_settings;
    struct {
        NxClient_ReconfigSettings settings;
    } reconfig;
    struct {
        NxClient_ScreenshotSettings settings;
        NEXUS_SurfaceHandle surface;
    } screenshot;
    struct {
        NEXUS_DisplayMacrovisionType type;
        bool table_isnull;
        NEXUS_DisplayMacrovisionTables table;
    } set_macrovision;
    struct {
        unsigned heapIndex;
    } grow_heap, shrink_heap;
    struct {
        unsigned pid;
    } lookup_client;
    struct {
        unsigned displayIndex;
    } display_get_crc_data;
    struct {
        unsigned id;
    } unregister_acknowledge_standby, acknowledge_standby;
    struct {
        NxClient_HdcpType hdcpType;
        NEXUS_MemoryBlockHandle block;
        unsigned blockOffset;
        unsigned size;
    } load_hdcp_keys;
    struct {
        unsigned slaveDisplay;
        NEXUS_SurfaceHandle surface;
    } set_slave_display_graphics;
} nxclient_p_general_param;
typedef union nxclient_p_general_output {
    struct {
        NEXUS_SurfaceComposition composition;
    } get_composition;
    struct {
        size_t numLinesWritten;
    } write_teletext;
    struct {
        size_t numEntriesWritten;
    } write_closed_caption;
    struct {
        NxClient_AudioProcessingSettings settings;
    } get_audio_processing_settings;
    struct {
        NEXUS_ClientHandle handle;
    } lookup_client;
    struct {
        NxClient_DisplayCrcData data;
    } display_get_crc_data;
    struct {
        NxClient_HdmiOutputCrcData data;
    } hdmi_output_get_crc_data;
    struct {
        unsigned id;
    } register_acknowledge_standby;
    struct {
        NxClient_Status status;
    } get_status;
} nxclient_p_general_output;

#endif /* NXSERVER_IPC_TYPES_H__ */
