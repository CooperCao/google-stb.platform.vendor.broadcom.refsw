/******************************************************************************
* Broadcom Proprietary and Confidential. (c)2016 Broadcom. All rights reserved.
*
* This program is the proprietary software of Broadcom and/or its
* licensors, and may only be used, duplicated, modified or distributed pursuant
* to the terms and conditions of a separate, written license agreement executed
* between you and Broadcom (an "Authorized License").  Except as set forth in
* an Authorized License, Broadcom grants no license (express or implied), right
* to use, or waiver of any kind with respect to the Software, and Broadcom
* expressly reserves all rights in and to the Software and all intellectual
* property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
* HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
* NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
*
* Except as expressly set forth in the Authorized License,
*
* 1. This program, including its structure, sequence and organization,
*    constitutes the valuable trade secrets of Broadcom, and you shall use all
*    reasonable efforts to protect the confidentiality thereof, and to use
*    this information only in connection with your use of Broadcom integrated
*    circuit products.
*
* 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT
*    TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED
*    WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A
*    PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
*    ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
*    THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
*
* 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
*    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
*    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
*    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
*    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
*    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
*    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
*    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
******************************************************************************/

#ifndef _BIP_PLAY_H_
#define _BIP_PLAY_H_

#if NXCLIENT_SUPPORT
    #include "nxclient.h"
    #include "nexus_simple_video_decoder_server.h"
    #include "nexus_simple_audio_decoder_server.h"
    #include "nexus_platform_client.h"
#else
    #include "nexus_platform.h"
    #include "nexus_video_window.h"
    #include "nexus_video_adj.h"
    #include "nexus_picture_ctrl.h"
#endif

#if NEXUS_HAS_SYNC_CHANNEL
    #include "nexus_sync_channel.h"
#endif

#include "binput.h"
#include "cmd_parsing.h"
#include "gui.h"

#define MAX_NUM_URLS                    20
#define TEST_ROOT_CA_PATH               "./host.cert"
#define BIP_PLAY_STATUS_PRINT_PERIOD    2000 /* 2 seconds */

typedef struct BIP_Play_Context {
    /* BIP Player handle */
    BIP_PlayerHandle hPlayer;
    BIP_PlayerStartSettings startSettings;

    /* Nexus params and object handles */
    NEXUS_PidChannelHandle videoPidChannel;
    NEXUS_PidChannelHandle audioPidChannel;
    NEXUS_AudioCodec audioCodec;
    NEXUS_VideoCodec videoCodec;
    NEXUS_SyncChannelSettings syncChannelSettings;
    NEXUS_SyncChannelHandle syncChannel;

#if NXCLIENT_SUPPORT
    unsigned connectId;
    NEXUS_SimpleVideoDecoderHandle hSimpleVideoDecoder;
    NEXUS_SimpleAudioDecoderHandle hSimpleAudioDecoder;
    NEXUS_SimpleStcChannelHandle hSimpleStcChannel;
    NxClient_AllocSettings allocSettings;
    NxClient_AllocResults allocResults;
    NxClient_ConnectSettings connectSettings;
#else
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle pcmDecoder;
#endif
    /* Video size */
    unsigned short maxWidth;
    unsigned short maxHeight;

    B_EventHandle hCompletionEvent; /* Playback completion event */

    BIP_StringHandle hUrls[MAX_NUM_URLS]; /* List of URLs specified at cmd line */
    unsigned int numUrls;        /* Number of URLs specified at cmd line */
    BIP_Play_Options options;    /* Command line options */

    /* Player and Server status */
    BIP_PlayerStatus playerStatus;
    BIP_PlayerStatusFromServer serverStatus;

    /* Run time state */
    float rate;                    /* Current stream playback rate 0 - paused, 1 - play , 2 - 2x FF ... */
    BIP_StringHandle currentUrl; /* Currently streaming URL */
    BIP_Play_ParsedRuntimeCmdInfo lastCmd; /* Last parsed runtime commdand info */

    /* Stream probe info */
    BIP_MediaInfoHandle hMediaInfo;
    BIP_PlayerStreamInfo playerStreamInfo;
    unsigned streamDurationInMs;

    bool playbackDone;
    unsigned int playingNow; /* Index of the stream playing now in the list of input urls */

    /* Remote handling */
    binput_t input;

    /* GUI handle */
    BIP_Play_GuiHandle hGui;
} BIP_Play_Context;

BIP_Status BIP_Play_Init(BIP_Play_Context** pCtx);
void BIP_Play_Uninit(BIP_Play_Context* pCtx);

#endif /* _BIP_PLAY_H_ */
