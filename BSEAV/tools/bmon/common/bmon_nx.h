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
#include "nxclient.h"
#include "nexus_platform.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_playpump.h"
#include "transport.h"
#endif

#if NEXUS_HAS_VIDEO_DECODER
#include "video_decoder.h"
#endif

#include "nexus_pid_channel.h"
#include "nexus_stc_channel.h"
#include "nexus_surface_types.h"

#define  XPT_CHANNELS_MAX                            56
#define  PLAYBACK_CHANNEL_OFFSET                     24
#define  CONVERT_TO_MEGABITS(bytes_now,bytes_prev)   ((bytes_prev)?MIN(( bytes_now - bytes_prev ) /1024/128,50):0)
#define  ROUNDUP( value )                            (( value + 0.5 ))
#define  MAX_OBJECTS                                 16

#if NEXUS_HAS_VIDEO_DECODER
typedef struct
{
    int              teiErrors;
    int              ccErrors;
    int              cdbOverflowErrors;
    int              itbOverflowErrors;
    int              outOfSyncErrors;
    int              pusiErrors; /* CAD should be pacing */
} errors_of_interest_t;

typedef struct
{
    int                  videoDecoderIndex;
    NEXUS_ParserBand     band;
    int                  pidChannelIndex;
    errors_of_interest_t errors;
    unsigned             fifoDepth;       /* depth in bytes of the compressed buffer */
    unsigned             fifoSize;        /* size in bytes of the compressed buffer */
    unsigned             queueDepth;      /* the number of decoded pictures currently ready to be displayed */
    unsigned             numDecodeErrors; /* total number of decoder errors since Start */
    unsigned             numDisplayUnderflows;  /* total number of times the display manager was unable to deliver a new picture since Start */
    bool started;           /* true if decoder was started */

    struct
    {
        unsigned width, height; /* dimensions in pixels */
    } source, coded, display; /* source is the active area size.
                                 coded is the coded area. it could be greater than the active area because of clipping.
                                 display is the display size needed to preserve aspect ratio. the actual display size is
                                   determined by the NEXUS_VideoWindowSettings. */

    NEXUS_AspectRatio aspectRatio;  /* aspect ratio of the source */
    NEXUS_VideoFrameRate frameRate; /* frame rate of the source */
    bool interlaced;          /* true if the source is interlaced, regardless of scanout requirements.
                                 Note that VideoInputStatus.videoFormat will report interlaced or progressive based on scanout,
                                 which is not necessarily the source coding. Some progressive streams must be scanned out as fields. */
    NEXUS_VideoFormat format; /* The video format is derived from source.height, frameRate and interlaced fields. */
    NEXUS_VideoProtocolLevel protocolLevel;
    NEXUS_VideoProtocolProfile protocolProfile;

    bool muted;               /* VideoDecoder is feeding muted pictures to Display */
    NEXUS_VideoDecoderTimeCode timeCode; /* last encoded timecode found in the stream */
    unsigned pictureTag; /* last picture tag found in the stream */
    NEXUS_PictureCoding pictureCoding;

    /* TSM status */
    bool tsm;                 /* VideoDecoder is in TSM (time stamp managed) mode. TSM may be not applicable (e.g. ES streams), may be disabled directly
                                 by the user (setting stcChannel = NULL) or indirectly by other API's (e.g. trick modes, ASTM, etc.) */
    unsigned int pts;             /* Current PTS of the video decoder
                                 Note: the stc is available from NEXUS_StcChannel_GetStc */
    int ptsStcDifference; /* Current PTS-STC difference including lipsync adjustments */
    NEXUS_PtsType ptsType;    /* Type of the current PTS */
    bool firstPtsPassed;      /* The first PTS for this stream has passed. This means that the information in this status structure is for pictures currently being displayed.
                                 See NEXUS_VideoDecoderSettings.firstPtsPassed for a callback related to this status. */

    /* buffer status */
    unsigned cabacBinDepth;   /* depth in bytes of the cabac bin buffer */
    unsigned enhancementFifoDepth; /* depth in bytes of the enhancement compressed buffer */
    unsigned enhancementFifoSize; /* size in bytes of the enhancement compressed buffer */
    /* unsigned bufferLatency was moved for performance reasons.  See NEXUS_VideoDecoderFifoStatus.bufferLatency. */

    /* cumulative status */
    unsigned numDecoded;      /* total number of decoded pictures since Start */
    unsigned numDisplayed;    /* total number of display pictures since Start */
    unsigned numIFramesDisplayed; /* total number of displayed I-Frames pictures since Start */
    unsigned numDecodeOverflows; /* total number of overflows of the input to the decoder since Start */
    unsigned numDisplayErrors;/* total number of display manager errors since Start. This includes parity mismatches which may result in glitching on the display. */
    unsigned numDecodeDrops;  /* total number of pictures dropped by the decoder since Start */
    unsigned numPicturesReceived; /* total number of pictures which the decoder has received since Start. This includes pictures which are skipped due to TSM, NEXUS_VideoDecoderDecodeMode,
                                     or other factors. */
    unsigned numDisplayDrops; /* total number of pictures dropped by the display manager due to TSM failures since Start */
    unsigned ptsErrorCount;   /* counter for number of PTS error interrupts since Start */
    unsigned avdStatusBlock;  /* snap shot of the AVD channel status. See NEXUS_CHANNELSTATUS_AVD_XXX macros for possible values. */
    unsigned numWatchdogs;    /* total number of watchdog events for the device since NEXUS_VideoDecoderModule_Init. the count is per AVD device, not per VideoDecoder channel. */
    unsigned long long numBytesDecoded; /* total number of ES bytes decoded since last start or flush */
    unsigned fifoEmptyEvents; /* total number of fifoEmpty events since last start or flush */
    unsigned fifoNoLongerEmptyEvents; /* total number of fifoNoLongerEmpty events since last start or flush.  If fifoEmptyEvents > fifoNoLongerEmptyEvents, then fifo is still empty */

} video_decoder_info_t;
#endif

typedef struct
{
    int                  pidChannelIndex;
    NEXUS_ParserBand     band;
#if NEXUS_HAS_VIDEO_DECODER
    errors_of_interest_t errors;
#endif
} pid_channel_info_t;

#if NEXUS_HAS_TRANSPORT
void bmon_print_playpump( int i, NEXUS_PlaypumpHandle playpump, unsigned short int *pinstant_rate );
int  bmon_process_pid_channel( int i, NEXUS_PidChannelHandle hPidChannel, pid_channel_info_t *pPidInfo );
void bmon_print_parser_band(NEXUS_ParserBand parserBand);
int  bmon_find_and_process_playpump( NEXUS_ParserBand band, bmon_transport_err_params_t *pxpt_err, unsigned short int *pinstant_rate );
unsigned long int bmon_compute_corruption_level( int index, bmon_transport_t *pdatabuffer );
int  bmon_parser_band_init( int parserBand );
int  bmon_pid_channel_init( int pidChannelIndex, unsigned char *PidChannelInitialization );
NEXUS_PidChannelHandle bmon_get_pid_channel_handle( int pidChannelIndex );
#endif /* NEXUS_HAS_TRANSPORT */

#if NEXUS_HAS_VIDEO_DECODER
int bmon_process_video_decoder( int i, NEXUS_VideoDecoderHandle videoDecoder, int *pch_number, video_decoder_info_t *pVideoDecoderInfo );
#endif
