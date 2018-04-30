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

#ifndef _SECURITY_TEST_FRAMEWORK_UTILITY__
#define _SECURITY_TEST_FRAMEWORK_UTILITY__

#if NEXUS_HAS_SECURITY && (NEXUS_SECURITY_ZEUS_VERSION_MAJOR >= 2)

#include "nexus_platform.h"
#include "nexus_dma.h"
#include "stdio.h"
#include <string.h>

BDBG_MODULE( secv2_examples );

/* Input stream file Pids */
#define VIDEO_PID 0x21
#define AUDIO_PID 0x22

#define XPT_TS_PACKET_SIZE (188)
#define XPT_TS_PACKET_HEAD_SIZE  (4)
#define IN_XPT_TYPE  NEXUS_TransportType_eTs
#define OUT_XPT_TYPE  NEXUS_TransportType_eTs
#define MIN(a,b) ((a)<(b)?(a):(b))

#define SECURITY_CHECK_RC(rc) {     \
    if (rc) {                       \
        BERR_TRACE( rc );           \
        goto exit;                  \
    }                               \
}

#define SECURITY_CHECK_TEST_RESULT(rc)   {                                                     \
    if (rc) {                                                                                      \
        BDBG_ERR( ( "<--- Failed: %s() Test#%d with %s.\n", BSTD_FUNCTION, count++,  pTestTitle)); \
        return BERR_TRACE( rc );                                        \
    } else  {                                                           \
        BDBG_LOG(("<--- Success: %s() Test#%d with %s.\n",  BSTD_FUNCTION, count++, pTestTitle));  \
    }                                                                   \
}

#define DEBUG_PRINT_ARRAY(description_txt,in_size,in_ptr) {             \
    int x_offset;                                                       \
    printf("[%s][%u]", description_txt, (unsigned int) in_size );       \
    for( x_offset = 0; x_offset < (int)(in_size); x_offset++ )          \
    {                                                                   \
        if( x_offset%16 == 0 ) { printf("\n"); }                        \
        printf("%02X ", in_ptr[x_offset] );                             \
    }                                                                   \
    printf("\n");                                                       \
}

#define PRINT_TS_PACKET(xptTSPackets) {                                 \
    int             ix;                                                 \
    printf ("\n---------------------------\n");                         \
    for ( ix = 0; ix < XPT_TS_PACKET_SIZE; ix++ ) {                     \
        if ( ix % 4 == 0 )                                              \
            printf ( " " );                                             \
        printf ( "%02X", xptTSPackets[ix] );                            \
        if ( ix % 16 == 15 )                                            \
            printf ( "\n" );                                            \
    }                                                                   \
    printf ( "\n" );                                                    \
}


int securityUtil_PlatformInit( bool useDisplay );
int securityUtil_PlatformUnInit( void );

/* Handle security DMA transfers */
int securityUtil_DmaTransfer( NEXUS_KeySlotHandle keyslotHandle,
                              uint8_t * pSrc,
                              uint8_t * pDest,
                              NEXUS_DmaDataFormat dataFormat,
                              size_t dataSize,
                              bool securityBpt );

/*
   Composit a single TS packet or multiple packets for tests.
   XPT_TS_PACKET_NUM defines the number of the packets.
*/
void CompositTSPackets( uint8_t * xptTSPackets, unsigned int packetSize, unsigned int scValue );

size_t securityGetAlogrithmKeySize(
    NEXUS_CryptographicAlgorithm algorithm );

#if NEXUS_HAS_PLAYBACK && NEXUS_HAS_RECORD && NEXUS_HAS_VIDEO_DECODER

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

#define ALGORITHM_BLOCK_SIZE (16)

/* Test stream information */
#define TRANSPORT_TYPE                  NEXUS_TransportType_eTs
#define TRANSPORT_TYPE                  NEXUS_TransportType_eTs
#define VIDEO_CODEC                     NEXUS_VideoCodec_eMpeg2
#define AUDIO_CODEC                     NEXUS_AudioCodec_eMpeg


/* Random clear session keys for video and audio encryption AND decryption. */
#define VIDEO_ENCRYPTION_KEY            "video encryptKey"
#define AUDIO_ENCRYPTION_KEY            "audio encryptKey"

typedef enum security_OtpDirectKeyId
{
    security_OtpDirectKeyId_eOtpKeyA = 0,
    security_OtpDirectKeyId_eOtpKeyB,
    security_OtpDirectKeyId_eOtpKeyC,
    security_OtpDirectKeyId_eOtpKeyD,
    security_OtpDirectKeyId_eOtpKeyE,
    security_OtpDirectKeyId_eOtpKeyF,
    security_OtpDirectKeyId_eOtpKeyG,
    security_OtpDirectKeyId_eOtpKeyH,
    security_OtpDirectKeyId_eMax
}   security_OtpDirectKeyId;

/* The misc setups for the video encryption/decryption, playback/record, and display. */
typedef struct NEXUS_ExampleSecuritySettings
{
    /* Security configurations. */
    NEXUS_SecurityKeySlotSettings keySlotSettings;
    NEXUS_KeySlotHandle videoKeyHandle;
    NEXUS_KeySlotHandle audioKeyHandle;

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

NEXUS_Error SecurityExampleInitPlatform ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error SecurityExampleShutdown ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error SecurityExampleSetupPlayback ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error SecurityExampleSetupDecodersDisplays ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error SecurityExampleSetupDecoders ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error SecurityExampleSetupPlaybackPidChannels ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error SecurityExampleSetupRecord4Encrpytion ( NEXUS_ExampleSecuritySettings * videoSecSettings );
NEXUS_Error SecurityExampleStartRecord ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error SecurityExampleStartPlayback ( NEXUS_ExampleSecuritySettings * pSettings );
NEXUS_Error SecurityExampleStartDecoders ( NEXUS_ExampleSecuritySettings * pSettings );

#endif /* #if NEXUS_HAS_PLAYBACK NEXUS_HAS_RECORD && NEXUS_HAS_VIDEO_DECODER */
#endif
#endif
