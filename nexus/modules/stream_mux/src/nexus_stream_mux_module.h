/***************************************************************************
 *     (c)2010-2014 Broadcom Corporation
 *
 *  This program is the proprietary software of Broadcom Corporation and/or its licensors,
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
 **************************************************************************/
#ifndef NEXUS_STREAM_MUX_MODULE_H__
#define NEXUS_STREAM_MUX_MODULE_H__

#include "nexus_base.h"
#include "nexus_stream_mux_thunks.h"
#include "nexus_stream_mux.h"
#include "nexus_stream_mux_init.h"
#include "priv/nexus_playpump_priv.h"
#include "priv/nexus_tsmux_priv.h"

#include "priv/nexus_core.h"
#include "bmuxlib_ts.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NEXUS_P_StreamMux_ManagedPtr {
    NEXUS_MemoryBlockHandle block;
    void *ptr;
} NEXUS_P_StreamMux_ManagedPtr;

typedef struct NEXUS_P_StreamMux_AudioEncoderState {
    NEXUS_AudioMuxOutputHandle audioMuxOutput;
    NEXUS_StreamMuxHandle mux;
    NEXUS_P_StreamMux_ManagedPtr frame;
    NEXUS_P_StreamMux_ManagedPtr meta;
} NEXUS_P_StreamMux_AudioEncoderState;

typedef struct NEXUS_P_StreamMux_VideoEncoderState {
    NEXUS_VideoEncoderHandle videoEncoder;
    NEXUS_StreamMuxHandle mux;
    NEXUS_P_StreamMux_ManagedPtr frame;
    NEXUS_P_StreamMux_ManagedPtr meta;
} NEXUS_P_StreamMux_VideoEncoderState;

#define NEXUS_STREAM_MUX_P_MAX_DESCRIPTORS  16
struct NEXUS_StreamMux {
    NEXUS_OBJECT(NEXUS_StreamMux);
    BMUXlib_TS_Handle mux;
    NEXUS_TimerHandle muxTimer;
    NEXUS_TsMuxHandle tsMux;
    NEXUS_StreamMuxCreateSettings createSettings;
    NEXUS_StreamMuxStartSettings startSettings;
    BMUXlib_TS_StartSettings muxStartSettings;
    NEXUS_TaskCallbackHandle finishedCallback;
    NEXUS_P_StreamMux_VideoEncoderState videoState[NEXUS_MAX_MUX_PIDS];
    NEXUS_P_StreamMux_AudioEncoderState audioState[NEXUS_MAX_MUX_PIDS];
    bool started;
    bool mcpbStarted;
    unsigned completedDuration;
};

NEXUS_OBJECT_CLASS_DECLARE(NEXUS_StreamMux);

typedef struct NEXUS_StreamMux_P_State {
    NEXUS_ModuleHandle module;
    NEXUS_StreamMuxModuleSettings config;
    BPVRlib_Feed_ExtendedOffsetEntry entries[NEXUS_STREAM_MUX_P_MAX_DESCRIPTORS];
    /* instead of allocating large data structures on the stack, they are allocated  in the static data (it's safe since module is serialized)
       data is kept in union with entry per function and we use 'ccokie' to verify that union was not stoled */
    union {
        struct {
            void (*cookie)(NEXUS_StreamMuxStartSettings *);
            BMUXlib_TS_StartSettings muxStartSettings;
        } NEXUS_StreamMux_GetDefaultStartSettings;
        struct {
            void (*cookie)(NEXUS_StreamMuxConfiguration *);
            BMUXlib_TS_StartSettings startSettings;
            BMUXlib_TS_MuxSettings muxSettings;
        } NEXUS_StreamMux_GetDefaultConfiguration;
        struct {
            void (*cookie)(const NEXUS_StreamMuxConfiguration *, NEXUS_StreamMuxMemoryConfiguration *);
            BMUXlib_TS_MuxConfig muxConfig;
            BMUXlib_TS_MemoryConfig memoryConfig;
        } NEXUS_StreamMux_GetMemoryConfiguration;
    } functionData;
} NEXUS_StreamMux_P_State;

extern NEXUS_StreamMux_P_State g_NEXUS_StreamMux_P_State;
#define NEXUS_MODULE_NAME stream_mux
#define NEXUS_MODULE_SELF g_NEXUS_StreamMux_P_State.module


#ifdef __cplusplus
}
#endif


#endif /* NEXUS_STREAM_MUX_MODULE_H__ */

