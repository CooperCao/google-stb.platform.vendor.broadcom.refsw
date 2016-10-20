/***************************************************************************
 *  Copyright (C) 2010-2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#ifndef NEXUS_FILE_MUX_MODULE_H__
#define NEXUS_FILE_MUX_MODULE_H__

#include "nexus_base.h"
#include "nexus_file_mux_thunks.h"
#include "nexus_file_mux.h"
#include "nexus_file_mux_init.h"
#include "priv/nexus_playpump_priv.h"
#include "priv/nexus_tsmux_priv.h"

#include "priv/nexus_core.h"
#include "bmuxlib_file_mp4.h"

#include "bmuxlib_file_pes.h"
#include "bmuxlib_file_ivf.h"

#define B_FILE_MUX_HAS_ASF  0
#if B_FILE_MUX_HAS_ASF
#include "bmuxlib_file_asf.h"
#endif

#include "bfifo.h"
#include "blst_list.h"

#ifdef __cplusplus
extern "C" {
#endif

BDBG_OBJECT_ID_DECLARE(NEXUS_FileMux);

#define NEXUS_P_FILEMUX_QUEUE_SIZE  64
typedef struct NEXUS_P_FileMux_File {
    BDBG_OBJECT(NEXUS_P_FileMux_File)
    NEXUS_MuxFileHandle file;
    NEXUS_FileMuxHandle mux;
    unsigned completed;
    ssize_t lastResult;
    bool ioError;
    bool ioInProgress;
    bool ioCanceled;
    BFIFO_HEAD(NEXUS_P_FileMux_Fifo, const BMUXlib_StorageDescriptor *) fifo;
    const BMUXlib_StorageDescriptor *descriptors[NEXUS_P_FILEMUX_QUEUE_SIZE];
} NEXUS_P_FileMux_File;

typedef struct NEXUS_P_FileMux_TempFile {
    NEXUS_P_FileMux_File file;
    BLST_D_ENTRY(NEXUS_P_FileMux_TempFile) link;
    bool destroyed; /* if file was destroyed while I/O is in progress  it's marked as destroyed */
} NEXUS_P_FileMux_TempFile;

typedef struct NEXUS_P_FileMux_TempStorage {
    BDBG_OBJECT(NEXUS_P_FileMux_TempStorage)
    NEXUS_FileMuxHandle mux;
    BLST_D_HEAD(NEXUS_P_FileMux_TempFiles, NEXUS_P_FileMux_TempFile) files;
} NEXUS_P_FileMux_TempStorage;

typedef struct NEXUS_P_FileMux_ManagedPtr {
    NEXUS_MemoryBlockHandle block;
    void *ptr;
} NEXUS_P_FileMux_ManagedPtr;

typedef struct NEXUS_P_FileMux_EncoderState {
    unsigned index; /* encoder index */
    NEXUS_FileMuxHandle mux;
    NEXUS_P_FileMux_ManagedPtr frame;
    NEXUS_P_FileMux_ManagedPtr meta;
} NEXUS_P_FileMux_EncoderState;

#define NEXUS_FILE_MUX_P_MAX_DESCRIPTORS  16
typedef struct NEXUS_P_FileMux_MemoryBlock {
    /* BMMA_Handle mma; */
    BMMA_Block_Handle mmaBlock;
    NEXUS_MemoryBlockHandle block;
} NEXUS_P_FileMux_MemoryBlock;

struct NEXUS_FileMux {
    BDBG_OBJECT(NEXUS_FileMux)
    NEXUS_TimerHandle muxTimer;
    NEXUS_FileMuxCreateSettings createSettings;
    NEXUS_FileMuxStartSettings startSettings;
    NEXUS_TaskCallbackHandle finishedCallback;
    NEXUS_P_FileMux_TempStorage tempStorage;
    NEXUS_P_FileMux_File outputFile;
    struct {
        BMUXlib_File_MP4_Handle mux;
        bool active;
    } mp4;
    struct {
        BMUXlib_File_PES_Handle mux;
        bool active;
    } pes;
    struct {
        BMUXlib_File_IVF_Handle mux;
        bool active;
    } ivf;
#if B_FILE_MUX_HAS_ASF
    struct {
        BMUXlib_File_ASF_Handle mux;
        bool active;
    } asf;
#endif
    bool stopping;
    bool started;
    bool ioWaitingThreshold;
    unsigned duration;
    struct {
        NEXUS_P_FileMux_EncoderState audio[NEXUS_MAX_MUX_PIDS];
        NEXUS_P_FileMux_EncoderState video[NEXUS_MAX_MUX_PIDS];
    } state;
    NEXUS_P_FileMux_MemoryBlock videoFrame;
    NEXUS_P_FileMux_MemoryBlock videoMeta;
    NEXUS_P_FileMux_MemoryBlock simpleVideoFrame;
    NEXUS_P_FileMux_MemoryBlock simpleVideoMeta;
    NEXUS_P_FileMux_MemoryBlock audioFrame;
    NEXUS_P_FileMux_MemoryBlock audioMeta;
    NEXUS_P_FileMux_MemoryBlock simpleAudioFrame;
    NEXUS_P_FileMux_MemoryBlock simpleAudioMeta;
};

typedef struct NEXUS_FileMux_P_State {
    NEXUS_ModuleHandle module;
    NEXUS_FileMuxModuleSettings config;
    BMMA_Handle mma;
} NEXUS_FileMux_P_State;

extern NEXUS_FileMux_P_State g_NEXUS_FileMux_P_State;
#define NEXUS_MODULE_NAME file_mux
#define NEXUS_MODULE_SELF g_NEXUS_FileMux_P_State.module


#ifdef __cplusplus
}
#endif


#endif /* NEXUS_FILE_MUX_MODULE_H__ */

