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
#include "bmon_types64.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <stdbool.h>
#include <sys/klog.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "bmon_utils.h"
#include "bmon_nx.h"

#include "nxclient.h"
#include "nexus_platform.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_playpump.h"
#endif
#include "nexus_parser_band.h"

#include "nexus_platform_generic_features_priv.h"
#include "namevalue.inc"

#ifdef PRINTF
#undef PRINTF
#define PRINTF noprintf
#endif
#undef  PACING_ERROR_INJECTION
#undef  CC_ERROR_INJECTION
#undef  TEI_ERROR_INJECTION
#undef  OOS_ERROR_INJECTION
#define INJECTION_EVERY_SECONDS 5
#define NUMBER_TO_INJECT        1000

#ifdef PACING_ERROR_INJECTION
#define PACKET_LENGTH 192
#else
#define PACKET_LENGTH 188
#endif

#if NEXUS_HAS_TRANSPORT
static unsigned long int  playpump_count = 0;
static unsigned long long bytesPlayedPrevious[XPT_CHANNELS_MAX];
typedef struct
{
    bmon_transport_err_params_t xpt_errors;
#if 0
    ch_decode_params_t decode_ch;
#endif
} Channel_Previous_t;
static Channel_Previous_t Channel_Previous[XPT_CHANNELS_MAX];

#if NEXUS_HAS_TRANSPORT
int bmon_find_and_process_playpump( NEXUS_ParserBand band, bmon_transport_err_params_t *pxpt_err, unsigned short int *pinstant_rate )
{
    NEXUS_PlaypumpStatus status;
    int                           i = 0;
    int                           rc   = 0;
    int                           nxrc = 0;
    size_t                        num;
    NEXUS_InterfaceName           interfaceName;
    NEXUS_PlatformObjectInstance  objects[MAX_OBJECTS];

    memset( &status,        0, sizeof(status) );
    memset( &interfaceName, 0, sizeof(interfaceName) );
    memset( &objects,       0, sizeof(objects) );

    NEXUS_Platform_GetDefaultInterfaceName( &interfaceName );
    strcpy( interfaceName.name, "NEXUS_Playpump" );
    nxrc = NEXUS_Platform_GetObjects( &interfaceName, objects, MAX_OBJECTS, &num );
    BDBG_ASSERT( !nxrc );
    printf( "%-18s ... num %d\n", interfaceName.name, (int) num );
    for (i = 0; i<num; i++)
    {
        rc = NEXUS_Playpump_GetStatus( objects[i].object, &status);
        if ( rc == 0 )
        {
            fprintf( stderr, "%s - band %d ... status.index %d \n", __FUNCTION__, (int) band, status.index );
            if ( status.index == band )
            {
                bmon_print_playpump( i, objects[i].object, pinstant_rate );

                pxpt_err->oos_err   = status.syncErrors;
                /*pxpt_err->tei_err   = status.teiErrors;*/ /* overwrites error count from pidChannel */
                pxpt_err->pusi_err  = status.pacingTsRangeError; /* CAD ... should be pacing_err */
            }
        }
    }

    return ( 0 );
}
#endif

void bmon_print_playpump( int i, NEXUS_PlaypumpHandle playpump, unsigned short int *pinstant_rate )
{
    NEXUS_PlaypumpStatus  status;
    unsigned char        *data = NULL;
    int                   rc;
    unsigned short int    Mbps = 0;

#if 0
    if ( playpump_count == 0 )
    {
        memset( &bytesPlayedPrevious, 0, sizeof( bytesPlayedPrevious ));
        fprintf( stderr, "DEBUG memset(0, bytesPlayedPrevious) \n" );
    }
#endif

    memset( &status, 0, sizeof( status ) );

    rc = NEXUS_Playpump_GetStatus(playpump, &status);
    if (rc) {BERR_TRACE(rc); return;}

    fprintf( stderr, "%s: index %d ... XPT_MAX %d ... rc %d ... base %p~ \n", __FUNCTION__, status.index, XPT_CHANNELS_MAX, rc, status.bufferBase );
    if ( status.index < XPT_CHANNELS_MAX )
    {
        fprintf( stderr, "DEBUG %p bytesPlayedPrevious[%d] = %lld \n", playpump, status.index, bytesPlayedPrevious[ status.index ] );
        Mbps = CONVERT_TO_MEGABITS( status.bytesPlayed, bytesPlayedPrevious[ status.index ] );
        *pinstant_rate = Mbps;
    }

    printf(
        "NEXUS_Playpump:     %u: %s\t"
        "fifo %7u/%7u (%3u%%)\t"
        "descfifo %7u/%7u (%3u%%)\t"
        "OOS %d ... cnt %ld ... KB played %8u ... Mbps %2d\n",
        status.index,status.started?"started":"stopped",
        (unsigned)status.fifoDepth, (unsigned)status.fifoSize, status.fifoSize?(unsigned)(status.fifoDepth*100/status.fifoSize):0,
        (unsigned)status.descFifoDepth, (unsigned)status.descFifoSize, status.descFifoSize?(unsigned)(status.descFifoDepth*100/status.descFifoSize):0,
        status.syncErrors, playpump_count, (unsigned)(status.bytesPlayed/1024), Mbps );

    data = (unsigned char*) status.bufferBase;
    fprintf( stderr, "%s: index %d ... data %p ... started %d~ \n", __FUNCTION__, status.index, data, status.started );
    if ( data && status.started && playpump_count%INJECTION_EVERY_SECONDS == (INJECTION_EVERY_SECONDS -1) )
    {
        int pkt_beginning = 0;
        /* make sure the buffer starts at the beginning of a packet */
        while ( data[pkt_beginning] != 0x47 || data[pkt_beginning+PACKET_LENGTH] != 0x47 )
        {
            fprintf( stderr, "NEXUS_Playpump: idx %-3d ... 0x%02x + %d ... 0x%02x\n", pkt_beginning, data[pkt_beginning], PACKET_LENGTH, data[pkt_beginning+PACKET_LENGTH]  );
            pkt_beginning++;
            if ( pkt_beginning >= PACKET_LENGTH ) break;
        }
#ifdef TEI_ERROR_INJECTION
        unsigned long int rand = random();

        printf( "NEXUS_Playpump: TEI data[0] 0x%02x ... count %ld ... count^%d %ld\n", data[pkt_beginning],
                playpump_count, INJECTION_EVERY_SECONDS, playpump_count%INJECTION_EVERY_SECONDS  );
        if ( data[pkt_beginning] == 0x47 && data[pkt_beginning+PACKET_LENGTH] == 0x47 )
        {
            int idx = rand;
            printf( "NEXUS_Playpump: corruption inserted %ld ... count %u TEI %p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n",
                    playpump_count, NUMBER_TO_INJECT, &data[pkt_beginning+1] );
            for (idx=0; idx<NUMBER_TO_INJECT; idx++ ) /* 10000 -> frame freezes for half sec ... 1000 -> 5 inches of corruption ... 100 -> square inch block */
            {
                data[pkt_beginning+1 + (idx*PACKET_LENGTH) ] |= 0x80 ;
            }
        }
#endif /* TEI_ERROR_INJECTION */

#ifdef CC_ERROR_INJECTION
        unsigned long int rand = random();

        printf( "NEXUS_Playpump: CC data[0] 0x%02x ... count %ld ... count^%d %ld\n", data[pkt_beginning],
                playpump_count, INJECTION_EVERY_SECONDS, playpump_count%INJECTION_EVERY_SECONDS  );
        if ( data[pkt_beginning] == 0x47 && data[pkt_beginning+PACKET_LENGTH] == 0x47 )
        {
            int idx = rand;
            unsigned char current_value = 0;
            unsigned char cc            = 0;
            printf( "NEXUS_Playpump: corruption inserted ... count %lu CC %p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", playpump_count, &data[pkt_beginning+3] );
            for (idx=0; idx<NUMBER_TO_INJECT; idx++ )
            {
                current_value = data[pkt_beginning+3 + (idx*PACKET_LENGTH) ];
                cc = data[pkt_beginning+3 + (idx*PACKET_LENGTH) ] & 0x0F;
                fprintf( stderr, "NEXUS_Playpump: corruption inserted ... count %lu CC 0x%02x \n", playpump_count, cc );
                data[pkt_beginning+3 + (idx*PACKET_LENGTH) ] = current_value & 0xF0;
                if ( idx < 10 ) printf( "NEXUS_Playpump: corruption inserted ... count %lu ... was 0x%02x ... is CC 0x%02x \n",
                        playpump_count, current_value, data[pkt_beginning+3 + (idx*PACKET_LENGTH) ] );
            }
        }
#endif /* CC_ERROR_INJECTION */

#ifdef OOS_ERROR_INJECTION
        unsigned long int rand = random();

        printf( "NEXUS_Playpump: OOS data[0] 0x%02x + %d 0x%02x ... count %ld ... count^%d %ld\n", data[pkt_beginning],
                PACKET_LENGTH, data[pkt_beginning+PACKET_LENGTH], playpump_count, INJECTION_EVERY_SECONDS, playpump_count%INJECTION_EVERY_SECONDS  );
        if ( data[pkt_beginning] == 0x47 && data[pkt_beginning+PACKET_LENGTH] == 0x47 )
        {
            int idx = rand;
            unsigned char oos = 0;
            printf( "NEXUS_Playpump: corruption inserted ... count %lu OOS %p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", playpump_count, &data[pkt_beginning+3] );
            for (idx=0; idx<NUMBER_TO_INJECT; idx++ )
            {
                oos = data[pkt_beginning + (idx*PACKET_LENGTH) ];
                data[pkt_beginning + (idx*PACKET_LENGTH) ] = 0x00;
                if ( idx < 10 ) printf( "NEXUS_Playpump: corruption inserted ... count %lu ... was 0x%02x ... is OOS 0x%02x \n",
                        playpump_count, oos, data[pkt_beginning + (idx*PACKET_LENGTH) ] );
            }
        }
#endif /* OOS_ERROR_INJECTION */

#ifdef PACING_ERROR_INJECTION
        unsigned long int rand = random();

        printf( "NEXUS_Playpump: PACING data[0] 0x%02x + %d 0x%02x ... count %ld ... count^%d %ld\n", data[pkt_beginning], PACKET_LENGTH,
                data[pkt_beginning+PACKET_LENGTH], playpump_count, INJECTION_EVERY_SECONDS, playpump_count%INJECTION_EVERY_SECONDS  );
        if ( data[pkt_beginning] == 0x47 && data[pkt_beginning+PACKET_LENGTH] == 0x47 )
        {
            int idx = rand;
            unsigned long int *ptimestamp = NULL;
            unsigned long int timestamp = 0;
            printf( "NEXUS_Playpump: corruption inserted ... count %lu PACING %p <<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", playpump_count, &data[pkt_beginning] );
            for (idx=0; idx<2; idx++ ) /* cannot inject too many or the stream will stop */
            {
                ptimestamp = (unsigned long int*) &(data[pkt_beginning - 4 + (idx*PACKET_LENGTH) ]); /* pacing timestamp begins 4 bytes before the sync byte */
                timestamp = *ptimestamp;
                *ptimestamp = timestamp & 0xffffff00;
                if ( idx < 10 ) printf( "NEXUS_Playpump: corruption inserted ... count %lu ... PACING was 0x%08lx ... is 0x%08lx \n",
                        playpump_count, timestamp, *ptimestamp );
            }
        }
#endif /* PACING_ERROR_INJECTION */

#if 1
        printf( "NEXUS_Playpump:  %p ... ", data );
        for(rc=0; rc<16; rc++ )
        {
            printf( "%02x ", data[rc] );
        }
        printf("\n");
#endif
    }
    if ( status.index < XPT_CHANNELS_MAX )
    {
        bytesPlayedPrevious[ status.index ] = status.bytesPlayed;
        fprintf( stderr, "DEBUG %u bytesPlayedPrevious[%d] = %lld \n", __LINE__, status.index, bytesPlayedPrevious[ status.index ] );
    }
    playpump_count++;
} /* bmon_print_playpump */

void bmon_print_parser_band(NEXUS_ParserBand parserBand)
{
    int rc;
    NEXUS_ParserBandStatus status;
    NEXUS_ParserBandSettings settings;

    memset( &settings, 0, sizeof( settings ) );

    rc = NEXUS_ParserBand_GetStatus(parserBand, &status);
    if (rc) {BERR_TRACE(rc); return;}

    NEXUS_ParserBand_GetSettings(parserBand, &settings);

#if DEBUG
    fprintf( stderr,"NEXUS_ParserBand: %d\toverflow=%d\tsrc:%s  transport:%s                                 OOS %d ... TEI %d\n",
        status.index, status.rsBufferStatus.overflowErrors,
#ifdef g_parserBandSourceTypeStrs
        lookup_name( g_parserBandSourceTypeStrs, settings.sourceType),
#else
        "UNK",
#endif
        lookup_name( g_transportTypeStrs, settings.transportType ), status.syncErrors, status.teiErrors );
#endif
}

int bmon_process_pid_channel( int i, NEXUS_PidChannelHandle hPidChannel, pid_channel_info_t *pPidInfo )
{
    int                    index = i;
    NEXUS_PidChannelStatus status;
    NEXUS_ParserBand       band;
    char *pbtype[2] = { "IBPB","PLPB"};
    int rc = 0;

    if ( hPidChannel == NULL ) return ( index );

    memset( &status, 0, sizeof(status) );
    rc = NEXUS_PidChannel_GetStatus(hPidChannel, &status);
    if (rc) {BERR_TRACE(rc); return ( index );}

    band = (status.playback)?status.playbackIndex:status.parserBand;
#if DEBUG
    fprintf(stderr,
        "NEXUS_PidChannel: %p \t"
        "pid=%d(0x%x)\thwPidChIndex=%d ... %s %u ... CC %d/%d TEI %d CDB %d ITB %d %s\n",
        hPidChannel, status.pid, status.pid, status.pidChannelIndex, (status.playback)?pbtype[1]:pbtype[0], (int) band,
        status.continuityCountErrors, status.raveStatus.continuityCountErrors, status.raveStatus.teiErrors,
        status.raveStatus.cdbOverflowErrors, status.raveStatus.itbOverflowErrors,
        lookup_name( g_transportTypeStrs, status.transportType ) );
#else
    FPRINTF( stderr, "%s - pbtype %s \n", __FUNCTION__, pbtype[0] );
#endif

    bmon_print_parser_band( band );

    if ( status.playback ) index += PLAYBACK_CHANNEL_OFFSET; /* playback channels start at 24 ... live start at 0 */

    if ( pPidInfo )
    {
        pPidInfo->band = band;
        pPidInfo->pidChannelIndex = status.pidChannelIndex;
        pPidInfo->errors.teiErrors = status.raveStatus.teiErrors;
        pPidInfo->errors.ccErrors = status.continuityCountErrors;
        pPidInfo->errors.cdbOverflowErrors = status.raveStatus.cdbOverflowErrors;
        pPidInfo->errors.itbOverflowErrors = status.raveStatus.itbOverflowErrors;
    } else {
        fprintf( stderr, "%s - pPidInfo cannot be NULL \n", __FUNCTION__ );
    }

    return ( index );
}  /* bmon_process_pid_channel */
#endif

#if 0 /* wait for ASP to be integrated into baseline */
#ifdef ASP_SUPPORT
void bmon_print_asp_channel( int i, NEXUS_AspChannelHandle hAspChannel)
{
    NEXUS_AspChannelStatus status;
    NEXUS_ParserBand       band;
    char *pbtype[2] = { "IB","PB"};
    int rc = 0;

    if ( hAspChannel == NULL ) return;

    memset( &status, 0, sizeof(status) );
    rc = NEXUS_AspChannel_GetStatus(hAspChannel, &status);
    if (rc) {BERR_TRACE(rc); return;}

    band = (status.playback)?status.playbackIndex:status.parserBand;
    printf(
        "NEXUS_AspChannel:   %d\t"
        "asp=%d(0x%x)\thwAspChIndex=%d ... %s %u ... CC %d TEI %d CDB %d ITB %d %s\n",
        i, status.asp, status.asp, status.AspChannelIndex, (status.playback)?pbtype[1]:pbtype[0], (int) band,
        status.continuityCountErrors, status.raveStatus.teiErrors, status.raveStatus.cdbOverflowErrors, status.raveStatus.itbOverflowErrors,
        lookup_name( g_transportTypeStrs, status.transportType ) );
    bmon_print_parser_band( band );
}
#endif /* ASP_SUPPORT */
#endif /* if 0 */

#if NEXUS_HAS_VIDEO_DECODER
int bmon_process_video_decoder( int i, NEXUS_VideoDecoderHandle hVideoDecoder, int *pch_number, video_decoder_info_t *pVideoDecoderInfo )
{
    int index                        = 0;
    int rc                           = 0;
    NEXUS_VideoDecoderStatus         status;
    NEXUS_VideoDecoderSettings       settings;
    NEXUS_VideoDecoderExtendedStatus ExtendedStatus;
    pid_channel_info_t               PidChannelInfo;

    if ( hVideoDecoder == NULL ) return ( 0 );

    /*fprintf( stderr, "%s - pVideoDecoderInfo %p \n", __FUNCTION__, pVideoDecoderInfo );*/
    memset( &status, 0, sizeof(status) );
    memset( &settings, 0, sizeof(settings) );
    memset( &ExtendedStatus, 0, sizeof(ExtendedStatus) );
    memset( &PidChannelInfo, 0, sizeof(PidChannelInfo) );

    rc = NEXUS_VideoDecoder_GetStatus( hVideoDecoder, &status);
    if (rc) {BERR_TRACE(rc); return ( 0 );}

    NEXUS_VideoDecoder_GetSettings( hVideoDecoder, &settings );
    NEXUS_VideoDecoder_GetExtendedStatus( hVideoDecoder, &ExtendedStatus );

    *pch_number = i;

    if ( ExtendedStatus.pidChannelIndex != -1 )
    {
        NEXUS_PidChannelHandle hPidChannelHandle = bmon_get_pid_channel_handle( ExtendedStatus.pidChannelIndex );
        index = bmon_process_pid_channel( 0, hPidChannelHandle, &PidChannelInfo );
    }
    /*fprintf( stderr, "%s - PidInfo.pidChannelIndex %d \n", __FUNCTION__, PidChannelInfo.pidChannelIndex );*/
    pVideoDecoderInfo->pidChannelIndex = PidChannelInfo.pidChannelIndex;

#if DEBUG
    fprintf(stderr,
        "NEXUS_VideoDecoder: %d %s %s  "
        "fifo %7u/%7u (%3u%%)  "
        "PTS %#6x, PTS/STC diff %-6d %4dx%4d pidCh %d band %d  qDepth %d\n",
        i, status.started?"started":"stopped",status.tsm?"tsm":"vsync",
        status.fifoDepth, status.fifoSize, status.fifoSize?status.fifoDepth*100/status.fifoSize:0,
        status.pts, status.ptsStcDifference, settings.maxWidth, settings.maxHeight, ExtendedStatus.pidChannelIndex, (int) PidChannelInfo.band,
        status.queueDepth );
#endif

    pVideoDecoderInfo->band                 = PidChannelInfo.band;
    pVideoDecoderInfo->errors.teiErrors     = PidChannelInfo.errors.teiErrors;
    pVideoDecoderInfo->fifoDepth            = status.fifoDepth;
    pVideoDecoderInfo->fifoSize             = status.fifoSize;
    pVideoDecoderInfo->queueDepth           = status.queueDepth;
    pVideoDecoderInfo->numDecodeErrors      = status.numDecodeErrors;
    pVideoDecoderInfo->numDisplayUnderflows = status.numDisplayUnderflows;
	pVideoDecoderInfo->started              = status.started;
	pVideoDecoderInfo->source.width         = status.source.width;
	pVideoDecoderInfo->source.height        = status.source.height;
	pVideoDecoderInfo->coded.width          = status.coded.width;
	pVideoDecoderInfo->coded.height         = status.coded.height;
	pVideoDecoderInfo->display.width        = status.display.width;
	pVideoDecoderInfo->display.height       = status.display.height;
	pVideoDecoderInfo->aspectRatio          = status.aspectRatio;
	pVideoDecoderInfo->frameRate            = status.frameRate;
	pVideoDecoderInfo->interlaced           = status.interlaced;
	pVideoDecoderInfo->format               = status.format;
	pVideoDecoderInfo->protocolLevel        = status.protocolLevel;
	pVideoDecoderInfo->protocolProfile      = status.protocolProfile;
	pVideoDecoderInfo->muted                = status.muted;
	/*pVideoDecoderInfo->timeCode             = status.timeCode;*/
	pVideoDecoderInfo->pictureTag           = status.pictureTag;
	pVideoDecoderInfo->pictureCoding        = status.pictureCoding;
	pVideoDecoderInfo->tsm                  = status.tsm;
	pVideoDecoderInfo->pts                  = status.pts;
	pVideoDecoderInfo->ptsStcDifference     = status.ptsStcDifference;
	pVideoDecoderInfo->ptsType              = status.ptsType;
	pVideoDecoderInfo->firstPtsPassed       = status.firstPtsPassed;
	pVideoDecoderInfo->cabacBinDepth        = status.cabacBinDepth;
	pVideoDecoderInfo->enhancementFifoDepth = status.enhancementFifoDepth;
	pVideoDecoderInfo->enhancementFifoSize  = status.enhancementFifoSize;
	pVideoDecoderInfo->numDecoded           = status.numDecoded;
	pVideoDecoderInfo->numDisplayed         = status.numDisplayed;
	pVideoDecoderInfo->numIFramesDisplayed  = status.numIFramesDisplayed;
	pVideoDecoderInfo->numDecodeOverflows   = status.numDecodeOverflows;
	pVideoDecoderInfo->numDisplayErrors     = status.numDisplayErrors;
	pVideoDecoderInfo->numDecodeDrops       = status.numDecodeDrops;
	pVideoDecoderInfo->numPicturesReceived  = status.numPicturesReceived;
	pVideoDecoderInfo->numDisplayDrops      = status.numDisplayDrops;
	pVideoDecoderInfo->ptsErrorCount        = status.ptsErrorCount;
	pVideoDecoderInfo->avdStatusBlock       = status.avdStatusBlock;
	pVideoDecoderInfo->numWatchdogs         = status.numWatchdogs;
	pVideoDecoderInfo->numBytesDecoded      = status.numBytesDecoded;
	pVideoDecoderInfo->fifoEmptyEvents      = status.fifoEmptyEvents;
	pVideoDecoderInfo->fifoNoLongerEmptyEvents = status.fifoNoLongerEmptyEvents;

    FPRINTF( stderr, "%s - VideoDecoderInfo->band %d ... pidChannelIndex %d end \n", __FUNCTION__, (int) pVideoDecoderInfo->band, pVideoDecoderInfo->pidChannelIndex );
    FPRINTF( stderr, "%s:%u TEI %d ... index %d\n", __FUNCTION__, __LINE__, PidChannelInfo.errors.teiErrors, index );
    return ( 0 );
}
#endif

unsigned long int bmon_compute_corruption_level( int index, bmon_transport_t *pdatabuffer )
{
    unsigned long int rc = 0;
    unsigned long int delta = 0;

#if NEXUS_HAS_TRANSPORT
    delta = pdatabuffer->transport[index].xpt_err.tei_err - Channel_Previous[index].xpt_errors.tei_err;
    printf("%s: TEI:%-3ld ... prev %5d ... current for idx %2d is %5d\n", __FUNCTION__, delta, Channel_Previous[index].xpt_errors.tei_err,
             index, pdatabuffer->transport[index].xpt_err.tei_err );
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta >= 100 ) rc = 2;
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta > 0   && delta < 100   ) rc = 1;
    Channel_Previous[index].xpt_errors.tei_err = pdatabuffer->transport[index].xpt_err.tei_err;

    delta = pdatabuffer->transport[index].xpt_err.oos_err - Channel_Previous[index].xpt_errors.oos_err;
    printf("%s: OOS:%-3ld ... prev %5d ... current for idx %2d is %5d\n", __FUNCTION__, delta, Channel_Previous[index].xpt_errors.oos_err,
             index, pdatabuffer->transport[index].xpt_err.oos_err );
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta >= 10 ) rc = 2;
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta > 0  && delta < 10  ) rc = 1;
    Channel_Previous[index].xpt_errors.oos_err = pdatabuffer->transport[index].xpt_err.oos_err;

    delta = pdatabuffer->transport[index].xpt_err.cc_err - Channel_Previous[index].xpt_errors.cc_err ;
    printf("%s: CC:%-4ld ... prev %5d ... current for idx %2d is %5d\n", __FUNCTION__, delta, Channel_Previous[index].xpt_errors.cc_err,
             index, pdatabuffer->transport[index].xpt_err.cc_err );
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta >= 10 ) rc = 2;
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta > 0  && delta < 10  ) rc = 1;
    Channel_Previous[index].xpt_errors.cc_err = pdatabuffer->transport[index].xpt_err.cc_err;

    delta = pdatabuffer->transport[index].xpt_err.pusi_err - Channel_Previous[index].xpt_errors.pusi_err ; /* CAD should be pacing */
    printf("%s: PAC:%-3ld ... prev %5d ... current for idx %2d is %5d\n", __FUNCTION__, delta, Channel_Previous[index].xpt_errors.pusi_err,
             index, pdatabuffer->transport[index].xpt_err.pusi_err );
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta >= 5 ) rc = 2;
    if ( rc < 2 && index < XPT_CHANNELS_MAX && delta > 0  && delta < 5  ) rc = 1;
    Channel_Previous[index].xpt_errors.pusi_err = pdatabuffer->transport[index].xpt_err.pusi_err;
#endif /* NEXUS_HAS_TRANSPORT */

#if 0
    /* examine any decoder errors */
    if ( pdatabuffer->transport[index].decode_ch.numDisplayUnderflows > Channel_Previous[index].decode_ch.numDisplayUnderflows )
    {
        delta = pdatabuffer->transport[index].decode_ch.numDisplayUnderflows - Channel_Previous[index].decode_ch.numDisplayUnderflows;
        printf("%s: DUN:%-3ld ... prev %5d ... current for idx %2d is %5d\n", __FUNCTION__, delta,
                Channel_Previous[index].decode_ch.numDisplayUnderflows, index, pdatabuffer->transport[index].decode_ch.numDisplayUnderflows );
        if ( rc < 2 && index < XPT_CHANNELS_MAX && delta >= 50 ) rc = 2;
        if ( rc < 2 && index < XPT_CHANNELS_MAX && delta > 0  && delta < 50  ) rc = 1;
        Channel_Previous[index].decode_ch.numDisplayUnderflows = pdatabuffer->transport[index].decode_ch.numDisplayUnderflows;
    }
#endif

    if( rc )
    {
        printf("%s: returning corruption level[%d] %lu ... TCP disUflow %lu\n", __FUNCTION__, index, rc, delta );
    }
    return ( rc );
}

#if 0
static int BytesPreviousRestore ( void )
{
    FILE *fp = NULL;

    fp = fopen( "bmon.dat", "r" );
    if ( fp )
    {
        fread( &bytesPlayedPrevious, sizeof(bytesPlayedPrevious), 1, fp );
        fclose( fp );
        fprintf( stderr, "DEBUG bytesPlayedPrevious restored ... [0] %lld \n", bytesPlayedPrevious[0] );
    }
    return ( 0 );
}

static int BytesPreviousSave ( void )
{
    FILE *fp = NULL;

    fp = fopen( "bmon.dat", "w" );
    if ( fp )
    {
        fprintf( stderr, "DEBUG bytesPlayedPrevious restored ... [0] %lld \n", bytesPlayedPrevious[0] );
        fwrite( &bytesPlayedPrevious, sizeof(bytesPlayedPrevious), 1, fp );
        fclose( fp );
    }
    return ( 0 );
}
#endif

NEXUS_PidChannelHandle bmon_get_pid_channel_handle( int pidChannelIndex )
{
    unsigned long int             i  = 0;
    unsigned long int             rc = 0;
    size_t                        num = 0;
    NEXUS_PidChannelHandle        hPidChannel = NULL;
    NEXUS_InterfaceName           interfaceName;
    NEXUS_PlatformObjectInstance  objects[MAX_OBJECTS];
    NEXUS_PidChannelSettings      lPidChannelSettings;
    NEXUS_PidChannelStatus        status;

    if ( pidChannelIndex < 0 ) return ( hPidChannel );

    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_PidChannel");
    rc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &num);
    BDBG_ASSERT(!rc);
    /* loop through all known pid channels and match with the one the caller is interested in */
    for (i=0;i<num;i++)
    {
        memset( &lPidChannelSettings, 0, sizeof(lPidChannelSettings) );
        memset( &status,              0, sizeof(status) );
        hPidChannel = (NEXUS_PidChannelHandle) objects[i].object;
        if ( hPidChannel )
        {
            rc = NEXUS_PidChannel_GetStatus(hPidChannel, &status);
            if (rc) {BERR_TRACE(rc); return ( hPidChannel );}

            /* if this matches the pidChannel we are looking for */
            if ( status.pidChannelIndex == pidChannelIndex )
            {
                break;
            }
        }
    }

    return ( hPidChannel );
} /* bmon_get_pid_channel_handle */

#if NEXUS_HAS_TRANSPORT
int bmon_parser_band_init(
    int parserBand
    )
{
    NEXUS_ParserBandSettings settings;

    if ( parserBand < 0 || parserBand >= NEXUS_NUM_PARSER_BANDS ) return 0;

    memset( &settings, 0, sizeof( settings ));

    NEXUS_ParserBand_GetSettings( parserBand, &settings );
    settings.continuityCountEnabled = true;
    NEXUS_ParserBand_SetSettings( parserBand, &settings );

    memset( &settings, 0, sizeof( settings ));

    NEXUS_ParserBand_GetSettings( parserBand, &settings );
    return( 0 );
}

int bmon_pid_channel_init(
    int            pidChannelIndex,
    unsigned char *PidChannelInitialization
    )
{
    int i  = 0;
    int rc = 0;

    if (pidChannelIndex && ( pidChannelIndex < NEXUS_NUM_PID_CHANNELS ) && ( PidChannelInitialization[pidChannelIndex] == 0 ))
    {
        PidChannelInitialization[pidChannelIndex] = 1;
        {
            size_t                       num;
            NEXUS_InterfaceName          interfaceName;
            NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
            NEXUS_PidChannelHandle       hPidChannel;
            NEXUS_PidChannelSettings     lPidChannelSettings;
            NEXUS_PidChannelStatus       status;

            NEXUS_Platform_GetDefaultInterfaceName( &interfaceName );
            strcpy( interfaceName.name, "NEXUS_PidChannel" );
            rc = NEXUS_Platform_GetObjects( &interfaceName, objects, MAX_OBJECTS, &num );
            BDBG_ASSERT( !rc );
            /* loop through all known pid channels and match with the one the caller is interested in */
            for (i = 0; i<num; i++)
            {
                memset( &lPidChannelSettings, 0, sizeof( lPidChannelSettings ));
                memset( &status,              0, sizeof( status ));
                hPidChannel = (NEXUS_PidChannelHandle) objects[i].object;
                if (hPidChannel)
                {
                    rc = NEXUS_PidChannel_GetStatus( hPidChannel, &status );
                    if (rc) {BERR_TRACE( rc ); return( -1 ); }

                    /* if this matches the pidChannel we are looking for */
                    if (status.pidChannelIndex == pidChannelIndex)
                    {
                        NEXUS_PidChannel_GetSettings( hPidChannel, &lPidChannelSettings );
                        lPidChannelSettings.continuityCountEnabled = true;
                        rc = NEXUS_PidChannel_SetSettings( hPidChannel, &lPidChannelSettings );
                        if (rc) {BERR_TRACE( rc ); return( -1 ); }
                        break;
                    }
                }
            }
        }
    }
    return( 0 );
}                                                          /* bmon_pid_channel_init */

#endif /* NEXUS_HAS_TRANSPORT */
