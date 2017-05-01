/***************************************************************************
 *  Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
 * Module Description:
 *
 **************************************************************************/
#ifndef NEXUS_STILL_DECODER_IMPL_H__
#define NEXUS_STILL_DECODER_IMPL_H__

#include "nexus_video_decoder_module.h"
#include "priv/nexus_core_video.h"

typedef struct NEXUS_StillDecoder_P_Interface {
    void (*Close)( NEXUS_StillDecoderHandle handle);
    NEXUS_Error (*Start)( NEXUS_StillDecoderHandle handle, const NEXUS_StillDecoderStartSettings *pSettings);
    void (*Stop)( NEXUS_StillDecoderHandle handle);
    NEXUS_Error (*GetStatus)( NEXUS_StillDecoderHandle handle, NEXUS_StillDecoderStatus *pStatus );
    NEXUS_Error (*GetStripedSurface)( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle *pStripedSurface);
    void (*ReleaseStripedSurface)( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle stripedSurface);
} NEXUS_StillDecoder_P_Interface;
extern const NEXUS_StillDecoder_P_Interface NEXUS_StillDecoder_P_Interface_Avd;

typedef struct NEXUS_HwStillDecoder *NEXUS_HwStillDecoderHandle;
BDBG_OBJECT_ID_DECLARE(NEXUS_HwStillDecoder);
struct NEXUS_HwStillDecoder {
    BDBG_OBJECT(NEXUS_HwStillDecoder)
    unsigned refcnt;
    NEXUS_StillDecoderHandle current;
    unsigned index; /* AVD index */
    struct NEXUS_VideoDecoderDevice *device;
    NEXUS_StillDecoderOpenSettings openSettings;
    NEXUS_StillDecoderStartSettings settings;
    bool started;
    NEXUS_Time startTime;
    NEXUS_StillDecoderStatus status;
    NEXUS_RaveHandle rave;
    NEXUS_TimerHandle timer;
    NEXUS_TaskCallbackHandle endCodeFoundCallback;
    NEXUS_IsrCallbackHandle stillPictureReadyCallback;
    BXVD_DecodeResolution eDecodeResolution;

    BXVD_ChannelHandle xvdChannel;

    struct {
        bool recreate;
        BXVD_StillPictureBuffers buffers;
        NEXUS_StripedSurfaceHandle handle;
    } stripedSurface;
    NEXUS_MemoryBlockHandle lumaBlock, chromaBlock;

    /* user allocated output buffer */
    struct {
        NEXUS_MemoryBlockHandle block;
        BMMA_Block_Handle mem;
        unsigned memoryOffset; /* offset into mem */
        unsigned size;
        BMMA_Block_Handle pictureMemory;
    } buffer;
};

struct NEXUS_StillDecoder {
    NEXUS_OBJECT(NEXUS_StillDecoder);
    BLST_S_ENTRY(NEXUS_StillDecoder) link; /* for g_decoders */
    const NEXUS_StillDecoder_P_Interface *intf;
    struct NEXUS_HwStillDecoder *hw;
    NEXUS_StillDecoderStartSettings settings;
    bool started;
    uint8_t endCode;
    BXVD_DecodeStillMode stillMode;
};

void NEXUS_StillDecoder_P_CheckForEndCode( void *context );
void NEXUS_VideoDecoder_P_StillReady_isr(void *context, int param, void *data);
void NEXUS_VideoDecoder_P_StillReady(void *context);

NEXUS_StillDecoderHandle NEXUS_StillDecoder_P_Open_Avd( NEXUS_VideoDecoderHandle parentDecoder, unsigned index, const NEXUS_StillDecoderOpenSettings *pSettings); 
void NEXUS_StillDecoder_P_Close_Avd( NEXUS_StillDecoderHandle handle);
NEXUS_Error NEXUS_StillDecoder_P_Start_Avd( NEXUS_StillDecoderHandle handle, const NEXUS_StillDecoderStartSettings *pSettings);
void NEXUS_StillDecoder_P_Stop_Avd( NEXUS_StillDecoderHandle handle);
NEXUS_Error NEXUS_StillDecoder_P_GetStatus_Avd( NEXUS_StillDecoderHandle handle, NEXUS_StillDecoderStatus *pStatus );
NEXUS_Error NEXUS_StillDecoder_P_GetStripedSurface_Avd( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle *pStripedSurface);
void NEXUS_StillDecoder_P_ReleaseStripedSurface_Avd( NEXUS_StillDecoderHandle handle, NEXUS_StripedSurfaceHandle stripedSurface);

#endif
