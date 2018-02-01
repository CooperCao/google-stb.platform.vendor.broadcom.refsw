/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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

 ******************************************************************************/

#ifndef __NEXUS_EXAMPLES_SECURITY_SETTINGS_H__
#define __NEXUS_EXAMPLES_SECURITY_SETTINGS_H__

#if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 2) && NEXUS_HAS_VIDEO_DECODER

#include "nexus_platform.h"
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_record.h"
#include "nexus_memory.h"
#if NEXUS_HAS_XPT_DMA
    #include "nexus_dma.h"
#endif

/* Nexus security include */
#include "nexus_security.h"
#include "nexus_keyladder.h"

#    define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) { int x_offset;       \
            printf("[%s][%d]", description_txt, in_size );                      \
            for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
            {                                                                   \
                if( x_offset%16 == 0 ) printf("\n");                            \
                                                                                \
                printf("%02X ", in_ptr[x_offset] );                             \
            }                                                                   \
            printf("\n");                                                       \
}

#define ALGORITHM_BLOCK_SIZE (16)

/* Test stream information */
#define TRANSPORT_TYPE                  NEXUS_TransportType_eTs
#define TRANSPORT_TYPE                  NEXUS_TransportType_eTs
#define VIDEO_CODEC                     NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC                     NEXUS_AudioCodec_eMpeg

/* Input stream file Pids */
#define VIDEO_PID                       2316
#define AUDIO_PID                       2317

/* Random clear session keys for video and audio encryption AND decryption. */
#define VIDEO_ENCRYPTION_KEY            "video encryptKey"
#define AUDIO_ENCRYPTION_KEY            "audio encryptKey"

/* The misc setups for the video encryption/decryption, playback/record, and display. */
typedef struct NEXUS_ExampleSecuritySettings
{
    /* Security configurations. */
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_KeySlotHandle videoKeyHandle;
    NEXUS_KeySlotHandle audioKeyHandle;
    NEXUS_VirtualKeyLadderHandle vklHandle;

    /* Platform config. */
    NEXUS_PlatformConfiguration platformConfig;

    /* Video output config, audio outputs are ingored. */
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;

    /* Audio and video decoders configs. */
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;

    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderStartSettings audioProgram;

    /* PidChannelIndexs */
    unsigned int    videoPID;
    unsigned int    audioPID;

    /* playpump and playback configs. */
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;

    NEXUS_FilePlayHandle playfile;
    const char     *playfname;

    /* record pump and record configs. */
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordCfg;
    NEXUS_RecordPidChannelSettings pidSettings;
    NEXUS_RecpumpOpenSettings recpumpOpenSettings;

    NEXUS_FileRecordHandle recordfile;
    const char     *recfname;

} NEXUS_ExampleSecuritySettings;

#if NEXUS_HAS_XPT_DMA

typedef struct sampleDmaTransferManagerInstance *sampleDmaTransferManagerHandle;

typedef struct {
    NEXUS_KeySlotHandle keySlotHandle;
} sampleDmaTransferManagerAllocSettings;

typedef struct {
    NEXUS_KeySlotHandle keySlotHandle;

    NEXUS_DmaHandle dmaHandle;
    BKNI_EventHandle dmaEvent;
    NEXUS_DmaJobHandle dmaJobHandle;
    NEXUS_DmaJobBlockSettings blockSettings[ALGORITHM_BLOCK_SIZE];  /* cached data. */
    unsigned      numBlocks;                               /* number of configure blocks. */
    unsigned      numCachedBytes;                          /* number of bytes currently cached. */
    char         *pTransferCache;                          /* cache for data remnant that is not a multiple of ALGORITHM_BLOCK_SIZE.
                                                            * - Max size will be alogorithm block size.
                                                            * - needs to be nexus device memory. */
    bool          resetCrypto;
} sampleDmaTransferManagerInstance;


sampleDmaTransferManagerHandle sampleDmaTransferManager_Create( sampleDmaTransferManagerAllocSettings * pSettings );
void sampleDmaTransferManager_Destroy( sampleDmaTransferManagerHandle handle );
unsigned sampleDmaTransferManager_Transfer(
    sampleDmaTransferManagerHandle handle,
    char *pSource,
    char *pDestination,
    unsigned size );
unsigned sampleDmaTransferManager_Flush( sampleDmaTransferManagerHandle handle );
#endif /* #if NEXUS_HAS_XPT_DMA */

NEXUS_Error     SecurityExampleInitPlatform ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error     SecurityExampleShutdown ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error     SecurityExampleSetupPlayback ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error     SecurityExampleSetupDecodersDisplays ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error     SecurityExampleSetupPlaybackPidChannels ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error     SecurityExampleSetupRecord4Encrpytion ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error     SecurityExampleStartRecord ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error     SecurityExampleStartPlayback ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error     SecurityExampleStartDecoders ( NEXUS_ExampleSecuritySettings * pSettings );

#endif /* #if NEXUS_HAS_SECURITY && NEXUS_HAS_PLAYBACK NEXUS_HAS_RECORD && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 2) && NEXUS_HAS_VIDEO_DECODER */

#endif /* #ifndef __NEXUS_EXAMPLES_SECURITY_SETTINGS_H__ */
