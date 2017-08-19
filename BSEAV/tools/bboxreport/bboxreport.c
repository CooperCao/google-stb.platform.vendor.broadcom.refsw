/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "nexus_transport_capabilities.h"
#include "nexus_video_decoder.h"
#if NEXUS_HAS_VIDEO_ENCODER
#include "nexus_video_encoder.h"
#endif /* NEXUS_HAS_VIDEO_ENCODER */
#include "boxmodes_defines.h"
#include "bmedia_types.h"
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "bmemperf_utils.h"
#include "bmemperf_info.h"
#include "memusage.h"
#include "bboxreport_svgplot.h"

BDBG_MODULE( bboxreport );
/**
This app queries the default heaps per platform, typically one heap per memory controller.
**/

#define STATE_FILE_RESTORED     "bmemconfig.state.restored"
#define VDEC_FORMAT             "vdec %u %u %u %u %u %u %u %u\n"
#define VDEC_FORMAT_CDB         "%u %u %u %u %u %u %u\n"
#define ADEC_FORMAT             "adec %u %u\n"
#define ADEC_MAX_NUM_FORMAT     "numadec %u %u %u %u\n"
#define ADEC_MISC_FORMAT        "%u %u %u %u %u %u %u %u %u\n"
#define ADEC_PRI_SEC_FORMAT     "%u %u %u %u %u\n"
#define SDEC_FORMAT             "sdec %u %u %u %u\n"       /* 4 fewer than VDEC (no mosaic; no colorDepth ) */
#define SDEC_FORMAT_CDB         "%u %u %u %u %u %u\n"      /* 1 fewer than VDEC (no mosaic) */
#define VENC_FORMAT             "venc %u %u %u %u %u %u\n"
#define REC_FORMAT              "rec %u %u %u\n"
#define PB_FORMAT               "pb %u %u %u\n"
#define MSG_FORMAT              "msg %u %u\n"
#define LIVE_FORMAT             "live %u %u\n"
#define REMUX_FORMAT            "remux %u %u\n"
#define GRAPHICS_FORMAT         "gfx %u %u %u %u %u %u %u\n"
#define GRAPHICS_3D_FORMAT      "g3d %u\n"
#define GRAPHICS_ENCODER_FORMAT "genc %u %u %u %u %u\n"
#define USAGE_FORMAT_BYTES      "%09u "
#define SID_FORMAT              "sid %u\n"
#define VIDEO_INPUT_FORMAT      "vinp %u %u\n"
#define LOG_FILE_FULL_PATH_LEN  64

#define TITLE_WIDTH_SIZE        "width:600px;font-size:22pt;"
#define BOXMODE_TABLE_HTML_SIZE 4096

typedef enum
{
    MEMCONFIG_AUDIO_USAGE_PRIMARY,
    MEMCONFIG_AUDIO_USAGE_SECONDARY,
    MEMCONFIG_AUDIO_USAGE_PASSTHROUGH,
    MEMCONFIG_AUDIO_USAGE_TRANSCODE,
    MEMCONFIG_AUDIO_USAGE_MAX
} Memconfig_AudioUsages;

#define NEXUS_MAX_LIVECHANNELS        6
#define MAX_NUM_GRAPHICS_ENCODERS     2
#define NUMBER_MEMC                   3
#define HEAP_INDEX_SECONDARY_GRAPHICS 5
#define MEGABYTES                     ( 1024/1024 )

static unsigned int maxVdecsEnabledByDefault = 0;                               /* 7445 has 6 vdecs but user can only configure 4 of them */
static unsigned int maxVdecFormatResolutionByDefault[NEXUS_MAX_VIDEO_DECODERS]; /* don't display 3820 if default max is only 480 */
#if NEXUS_NUM_STILL_DECODES
static unsigned int maxSdecsEnabledByDefault = 0;
static unsigned int maxSdecFormatResolutionByDefault[NEXUS_MAX_VIDEO_DECODERS]; /* don't display 3820 if default max is only 480 */
#endif /* if NEXUS_NUM_STILL_DECODES */
static unsigned int      maxDisplayFormatByDefault[NEXUS_MAX_DISPLAYS];         /* used on the Display Settings page */
static unsigned int      maxDisplayFormatByDefault[NEXUS_MAX_DISPLAYS];         /* used on the Display Settings page */
static unsigned int      maxGraphicsEnabledByDefault = 0;                       /* used on the Graphics page (only show graphics that are on by default) */
static unsigned int      maxNumTranscodesByDefault   = 0;
static bool              audioDecodeCodecEnabledDefaults[NEXUS_AudioCodec_eMax];
static bool              audioEncodeCodecEnabledDefaults[NEXUS_AudioCodec_eMax];
static unsigned long int heap_usage[NEXUS_MAX_HEAPS];
static unsigned long int heap_padding[NEXUS_MAX_HEAPS];
typedef struct
{
    unsigned char memcIndex;
    unsigned int  megabytes;
    char         *memoryType;
} memconfig_size;

static memconfig_size g_heap_info[NEXUS_MAX_HEAPS];

typedef struct
{
    unsigned char numOutputChannels;
    char         *description;
} Memconfig_AudioChannelInfo;

#define MEMCONFIG_NUM_AUDIO_CHANNEL_TYPES 2
#define MEMCONFIG_NUM_AUDIO_SAMPLE_RATES  2

/* these global variables are solely needed to compile with bmemperf_utils.c */
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0;                   /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;
bsysperf_netStatistics g_netStats[NET_STATS_MAX];
int                    g_netStatsIdx = -1;                 /* index to entries added to g_netStats array */

/**
 *  Function: This function creates a time string that matches the time string that is created by kernel calls.
 **/
char *print_time(
    void
    )
{
    static char     timestring[32];
    struct timespec time1;

    clock_gettime( CLOCK_REALTIME, &time1 );
    snprintf( timestring, sizeof( timestring ), "<!>[%ld.%06ld]", time1.tv_sec, time1.tv_nsec/1000 );
    return( timestring );
}

/**
 *  Function: This function adds the specified app name to the running list of open Nexus apps.
 **/
static int addOpenNexusApp(
    char       *pOpenNexusApp,
    const char *appName,
    const char *pid,
    int         bufferLen
    )
{
    PRINTF( "nexus app is (%s)\n", appName );

    /* do not worry about these apps being open */
    if (( strcmp( appName, "bboxreport.cgi" ) == 0 ) || ( strcmp( appName, "logger" ) == 0 ))
    {
        return( -1 );
    }

    strncat( pOpenNexusApp, appName, bufferLen - strlen( pOpenNexusApp ) - 1 );
    strncat( pOpenNexusApp, " (", bufferLen - strlen( pOpenNexusApp ) - 1 );
    strncat( pOpenNexusApp, pid, bufferLen - strlen( pOpenNexusApp ) - 1 ); /* the PID */
    strncat( pOpenNexusApp, "):", bufferLen - strlen( pOpenNexusApp ) - 1 );

    return( 0 );
}                                                          /* addOpenNexusApp */

/**
 *  Function: This function will use the 'lsof' (list open files) utility to determine if some other app is
 *  currently using Nexus. This tool cannot run if some other app has initialized Nexus. Sample input:
 *  2947 /mnt/nfs/playback
 *  2976 /mnt/nfs/logger
 */
int getOpenNexusApps(
    char *pOpenNexusApp,
    int   bufferLen
    )
{
#define MAX_LENGTH_LSOF 256
    FILE    *cmd = NULL;
    char     line[MAX_LENGTH_LSOF];                        /* e.g: 3747 /mnt/nfs/playback */
    int      OpenNexusAppCount = 0;
    long int pid               = 0;

    if (pOpenNexusApp == NULL)
    {
        return( -1 );
    }

    snprintf( line, bufferLen - 1, "lsof | grep nexus.log | awk '{print $1 \" \" $2;}' | uniq" );
    PRINTF( "system(%s)\n", line );
    cmd = popen( line, "r" );

    do {
        memset( line, 0, sizeof( line ));
        fgets( line, MAX_LENGTH_LSOF, cmd );
        PRINTF( "got line (%s); len %d\n", line, strlen( line ));
        if (strlen( line ))
        {
            int   rc       = 0;
            char *pos      = strchr( line, '\n' );
            char *posSpace = strchr( line, ' ' );
            if (pos)
            {
                *pos = 0;
            }
            PRINTF( "found (%s)\n", line );
            sscanf( line, "%ld", &pid );

            /* if a space between the PID and the app name was found */
            if (posSpace)
            {
                *posSpace = 0;                             /* null-terminate the PID string */

                /* start searching for the app name after the space */
                posSpace++;
            }
            else
            {
                /* start searching for the app name at the beginning of the line */
                posSpace = line;
            }
            pos = strrchr( posSpace, '/' );
            if (pos)
            {
                ++pos;
                rc = addOpenNexusApp( pOpenNexusApp, pos, line, bufferLen );
                if (rc == 0)
                {
                    OpenNexusAppCount++;
                }
            }
            else
            {
                rc = addOpenNexusApp( pOpenNexusApp, posSpace, line, bufferLen );
                if (rc == 0)
                {
                    OpenNexusAppCount++;
                }
            }
        }
    } while (strlen( line ));
    PRINTF( "returning (%s)\n", pOpenNexusApp );

    pclose( cmd );
    return( OpenNexusAppCount );
}                                                          /* getOpenNexusApps */

static int scanForTag(
    const char *queryRequest,
    const char *formatStr,
    int        *returnValue
    )
{
    char  newstr[32];
    char *pos = strstr( queryRequest, formatStr );

    /*printf("looking for (%s) in (%s); pos is %p<br>\n", formatStr, queryRequest, pos );*/
    if (pos)
    {
        strncpy( newstr, formatStr, sizeof( newstr ));
        strncat( newstr, "%d", sizeof( newstr ));
        sscanf( pos, newstr, returnValue );
        /*printf("%s is %d\n", newstr, *returnValue );*/
    }
    return( 0 );
}

extern bmemconfig_box_info g_bmemconfig_box_info[BMEMCONFIG_MAX_BOXMODES];

static int getBoxModeMemc(
    int boxmode
    )
{
    unsigned int idx     = 0;
    int          numMemc = 0;

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        PRINTF( "~DEBUG~%s: g_bmemconfig_box_info[%d].boxmode is %d ... comparing with %d~", __FUNCTION__, idx, g_bmemconfig_box_info[idx].boxmode, boxmode );
        if (g_bmemconfig_box_info[idx].boxmode == boxmode)
        {
            numMemc = g_bmemconfig_box_info[idx].numMemc;
            break;
        }

    }
    /* we did not find the specified box mode */
    return( numMemc );
}

static int getBoxModeDdrFreq(
    int boxmode
    )
{
    unsigned int idx     = 0;
    int          ddrFreq = 0;

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        if (g_bmemconfig_box_info[idx].boxmode == boxmode)
        {
            ddrFreq = g_bmemconfig_box_info[idx].ddrFreq;
            break;
        }

    }
    /* we did not find the specified box mode */
    return( ddrFreq );
}

static int getBoxModeScbFreq(
    int boxmode
    )
{
    unsigned int idx     = 0;
    int          scbFreq = 0;

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        if (g_bmemconfig_box_info[idx].boxmode == boxmode)
        {
            scbFreq = g_bmemconfig_box_info[idx].scbFreq;
            break;
        }

    }
    /* we did not find the specified box mode */
    return( scbFreq );
}

static int getProductIdMemc(
    void
    )
{
    unsigned int idx     = 0;
    int          numMemc = 1;

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        PRINTF( "~DEBUG~%s: g_bmemconfig_box_info[%d].strProductId is %s ... comparing with %s~", __FUNCTION__, idx, g_bmemconfig_box_info[idx].strProductId, getProductIdStr() );
        if ( strcmp ( g_bmemconfig_box_info[idx].strProductId, getProductIdStr() ) == 0 )
        {
            numMemc = g_bmemconfig_box_info[idx].numMemc;
            break;
        }
    }

    /* we did not find the specified product id */
    return( numMemc );
}  /* getBoxModeMemc */

static char *getBoxModeDdrScbString(
    int boxmode
    )
{
    unsigned int idx = 0;

    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < ( sizeof( g_bmemconfig_box_info )/sizeof( g_bmemconfig_box_info[0] )); idx++)
    {
        PRINTF( "~DEBUG~%s: g_bmemconfig_box_info[%d].boxmode is %d ... comparing with %d~", __FUNCTION__, idx, g_bmemconfig_box_info[idx].boxmode, boxmode );
        if (g_bmemconfig_box_info[idx].boxmode == boxmode)
        {
            return( g_bmemconfig_box_info[idx].strDdrScb ); /* similar to: DDR3@1067MHz SCB@432MHz */
        }
    }

    /* we did not find the specified box mode */
    return( "unknown ddr" );
}                                                          /* getBoxModeDdrScbString */

static int findWidthHeightHertz(
    const char   *name,
    unsigned int *width,
    unsigned int *height,
    unsigned int *hertz
    )
{
    char *posp = NULL;
    char *posx = NULL;

    if (name == NULL)
    {
        return( 0 );
    }

    posp = strstr( name, "p" );
    posx = strstr( name, "x" );

    if (( strcmp( name, "ntsc" ) == 0 ) || ( strcmp( name, "480i" ) == 0 ))
    {
        *width  = 640;
        *height = 480;
        *hertz  = 60;
    }
    else if (( strcmp( name, "480p" ) == 0 ))
    {
        *width  = 640;
        *height = 480;
        *hertz  = 61;
    }
    else if (strcmp( name, "pal" ) == 0)
    {
        *width  = 700;
        *height = 525;
        *hertz  = 50;
    }
    else if (strcmp( name, "1080i" ) == 0)
    {
        *width  = 1920;
        *height = 1080;
    }
    else if (strcmp( name, "1080p" ) == 0)
    {
        *width  = 1920;
        *height = 1080;
        *hertz  = 30;
    }
    else
    {
        if (posx)
        {
            sscanf( name, "%u", width );
            posx++;
            sscanf( posx, "%u", height );
        }
        else
        {
            sscanf( name, "%u", height );
        }
        if (posp)
        {
            posp++;
            /* if string has p separator */
            if (*posp)
            {
                sscanf( posp, "%u", hertz );
                if (*hertz ==0) {*hertz = 61; }
                if (*hertz ==30) {*hertz = 61; }
                if (*hertz ==60) {*hertz = 62; }
            }
            else
            {
            }
        }
    }
    PRINTF( "%s: width %u; height %u; hertz %u<br>\n", __FUNCTION__, *width, *height, *hertz );
    if (*width == 0)
    {
        *width = *height * 16 / 9;
    }
    if (*hertz == 0)
    {
        if (*height < 525)
        {
            *hertz = 60;
        }
        else if (*height==720)
        {
            *hertz = 30;
        }
        else if (*height==1080)
        {
            *hertz = 60;
        }
        else
        {
            *hertz = 50;
        }
    }
    PRINTF( "%s: width %u; height %u; hertz %u<br>\n", __FUNCTION__, *width, *height, *hertz );
    if (*width == 0)
    {
        *width = *height * 16 / 9;
    }
    {
        unsigned int ratio2 = 0;
        float        ratio  = ( *width )*1.0;
        ratio += 1.;
        ratio /= *height;
        ratio *= 9.0;
        ratio2 = ratio;
        PRINTF( "%04u%04u%02u (%-14s) width %04u; height %04u; hertz %02u; %02dx9; \n", *width, *height, *hertz, name, *width, *height, *hertz, ratio2 );
    }
    return( *width*100*10000 + *height*100 + *hertz );
}                                                          /* findWidthHeightHertz */

static int transfer_settings(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput,
    Memconfig_AppMemUsage             *pOutput,
    Memconfig_BoxMode                 *pBoxModeSettings
    )
{
    unsigned int i = 0;

#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pInput->videoDecoder[i].enabled    = pSettings->videoDecoder[i].used;
        pInput->videoDecoder[i].b3840x2160 = ( pSettings->videoDecoder[i].maxFormat == NEXUS_VideoFormat_e3840x2160p30hz ) ||
            ( pSettings->videoDecoder[i].maxFormat == NEXUS_VideoFormat_e3840x2160p60hz );
    }

#if NEXUS_NUM_STILL_DECODES
    for (i = 0; i<NEXUS_NUM_STILL_DECODES; i++)
    {
        pInput->stillDecoder[i].enabled    = pSettings->stillDecoder[i].used;
        pInput->stillDecoder[i].b3840x2160 = ( pSettings->stillDecoder[i].maxFormat == NEXUS_VideoFormat_e3840x2160p30hz ) ||
            ( pSettings->stillDecoder[i].maxFormat == NEXUS_VideoFormat_e3840x2160p60hz );
    }
#endif /* if NEXUS_NUM_STILL_DECODES */
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_HAS_VIDEO_ENCODER
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        pInput->encoders[i].enabled = pSettings->videoEncoder[i].used;
    }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        if (pBoxModeSettings->graphics[i].used == false)
        {
            pOutput->surfaceSettings[0].bytesGeneral = 0;
            pOutput->surfaceSettings[1].bytesGeneral = 0;
        }
    }
#endif /*NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    return( 0 );
}                                                          /* transfer_settings */

int get_non_video_encoder_count(
    Memconfig_BoxMode *pBoxModeSettings
    )
{
    unsigned int i = 0, nonEncoderCount = 0;

    for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++)
    {
        if (( pBoxModeSettings->videoDecoder[i].property == Memconfig_VideoDecoderProperty_eMain ) ||
            ( pBoxModeSettings->videoDecoder[i].property == Memconfig_VideoDecoderProperty_ePip ))
        {
            nonEncoderCount++;
            PRINTF( "~%s: videoDecoder[%u].property %u; nonEncoderCount %u\n~", __FUNCTION__, i, pBoxModeSettings->videoDecoder[i].property, nonEncoderCount );
        }
    }

    return( nonEncoderCount );
}

static int compute_max_video_decoders(
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    int i;

#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        if (pSettings->videoDecoder[i].used)
        {
            maxVdecsEnabledByDefault++;
        }
        PRINTF( "~%s: videoDecoder %u; used %u; maxVdecsEnabledByDefault %u\n~", __FUNCTION__, i, pSettings->videoDecoder[i].used, maxVdecsEnabledByDefault );
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_NUM_STILL_DECODES
    for (i = 0; i<NEXUS_NUM_STILL_DECODES; i++)
    {
        if (pSettings->stillDecoder[i].used)
        {
            maxSdecsEnabledByDefault++;
        }
    }
#endif /* if NEXUS_HAS_STILL_DECODER */

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        maxDisplayFormatByDefault[i] = pSettings->display[i].maxFormat;
    }
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    return( 0 );
}                                                          /* compute_max_video_decoders */

static int compute_max_video_formats(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_BoxMode                 *pBoxModeSettings
    )
{
    int          i;
    unsigned int maxFormat  = 0;
    const char  *formatName = NULL;

#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        if (pSettings->videoDecoder[i].used)
        {
            unsigned int value = 0;

            maxFormat  = pSettings->videoDecoder[i].maxFormat;
            formatName = get_maxformat_name( maxFormat );
            PRINTF( "vdec %u: used %u; maxFormat %u; name (%s)<br>\n", i, pSettings->videoDecoder[i].used, maxFormat );

            /* 480:enum24, 720p:enum34, 1080p:enum32, 3840p60:enum47 */
            /* maxFormat for 720 (34) is numerically greater than maxFormat for 1080p (32); need to determine lines of resolution */
            sscanf( formatName, "%u", &value );
            maxVdecFormatResolutionByDefault[i] = value;
            PRINTF( "%s: vdec %u: maxFormat %u: str (%s); resolution (%u)<br>\n", __FUNCTION__, i, pSettings->videoDecoder[i].maxFormat, formatName, value );
        }
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_NUM_STILL_DECODES
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        if (pSettings->stillDecoder[i].used)
        {
            unsigned int value = 0;

            maxFormat  = pSettings->stillDecoder[i].maxFormat;
            formatName = get_maxformat_name( maxFormat );
            PRINTF( "vdec %u: used %u; maxFormat %u; name (%s)<br>\n", i, pSettings->stillDecoder[i].used, maxFormat );

            if (formatName)
            {
                /* 480:enum24, 720p:enum34, 1080p:enum32, 3840p60:enum47 */
                /* maxFormat for 720 (34) is numerically greater than maxFormat for 1080p (32); need to determine lines of resolution */
                sscanf( formatName, "%u", &value );
                maxSdecFormatResolutionByDefault[i] = value;
                PRINTF( "%s: vdec %u: maxFormat %u: str (%s); resolution (%u)<br>\n", __FUNCTION__, i, pSettings->stillDecoder[i].maxFormat, formatName, value );
            }
        }
    }
#endif /* if NEXUS_NUM_STILL_DECODES */

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        maxFormat = pSettings->display[i].maxFormat;
        if (pSettings->display[i].maxFormat == NEXUS_VideoFormat_ePal)
        {
            formatName = "576";
        }
        else
        {
            formatName = get_maxformat_name( maxFormat );
        }
        PRINTF( "display %u: maxFormat %u; name (%s)<br>\n", i, maxFormat, formatName );

        if (maxFormat)
        {
            unsigned int      value         = 0;
            unsigned          widthTmp      = 0, heightTmp = 0, hertzTmp = 0;
            unsigned long int maxResolution = 0;
            /* 480:enum24, 720p:enum34, 1080p:enum32, 3840p60:enum47 */
            /* maxFormat for 720 (34) is numerically greater than maxFormat for 1080p (32); need to determine lines of resolution */
            maxResolution = findWidthHeightHertz( formatName, &widthTmp, &heightTmp, &hertzTmp );
            PRINTF( "%s: maxResolution %lu<br>\n", __FUNCTION__, maxResolution );
            value = heightTmp;
            maxDisplayFormatByDefault[i] = maxResolution;
            PRINTF( "%s: display %u: maxFormat %u: str (%s); resolution (%u)<br>\n", __FUNCTION__, i, pSettings->display[i].maxFormat, formatName, value );
        }

        if (pBoxModeSettings->graphics[i].used)
        {
            maxGraphicsEnabledByDefault++;
        }
    }
    PRINTF( "%s: graphics->used default %u<br>\n", __FUNCTION__, maxGraphicsEnabledByDefault );
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    return( 0 );
}                                                          /* compute_max_video_formats */

static bool supportsH265Compression(
    NEXUS_MemoryConfigurationSettings *memConfigSettings
    )
{
    int idx = 0;

    /* loop through all of the decoders to see if one of them supports H265 */
    for (idx = 0; idx<NEXUS_MAX_VIDEO_DECODERS; idx++)
    {
        if (memConfigSettings->videoDecoder[idx].used &&
            ( NEXUS_VideoFormat_e3840x2160p24hz <= memConfigSettings->videoDecoder[idx].maxFormat ) &&
            ( memConfigSettings->videoDecoder[idx].maxFormat <= NEXUS_VideoFormat_e4096x2160p60hz ))
        {
            return( true );
        }
    }

    return( false );
} /* supportsH265Compression */

int main(
    int   argc,
    char *argv[]
    )
{
    NEXUS_PlatformSettings            platformSettings;
    NEXUS_PlatformStatus              platformStatusPrevious;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    Memconfig_AppUsageSettings        transInput;
    Memconfig_AppMemUsage             transOutput;
    Memconfig_BoxMode                 boxModeSettings;
    NEXUS_VideoDecoderCapabilities    lVideoDecoderCapabilities;
#if NEXUS_HAS_VIDEO_ENCODER
    NEXUS_VideoEncoderCapabilities    lVideoEncoderCapabilities;
#endif /* NEXUS_HAS_VIDEO_ENCODER */
    NEXUS_DisplayCapabilities         lDisplayCapabilities;
    Bboxreport_SvgPlot                svgPlot;
    bool     main_used = false;
    bool     pip_used  = false;
    int      i;
    int      rc;
    unsigned codec;
    int      boxmode    = -1;
    int      initValue  = 0;
    int      displayIdx = 0;
    int      lNumberMemcs = 1;
    char     boxmodeTableHtml[BOXMODE_TABLE_HTML_SIZE];
    char    *queryString;
    char    *remoteAddress;
    char     boxmodeStr[5];
    char     datetime[20];
    char     strTemp[128];
    volatile unsigned int *g_pMem = NULL;

    i = argc;
    i = 0;
	BKNI_Init();
	BDBG_Init();

    /* Open driver for memory mapping and mmap() it */
    g_pMem = bmemperf_openDriver_and_mmap();

    if (g_pMem == NULL)
    {
        printf( "Failed to bmemperf_openDriver_and_mmap() ... g_pMem %p \n", (void*) g_pMem );
        return( -1 );
    }

    BKNI_Memset( &platformSettings, 0, sizeof( platformSettings ));
    BKNI_Memset( &platformStatusPrevious, 0, sizeof( platformStatusPrevious ));
    BKNI_Memset( &memConfigSettings, 0, sizeof( memConfigSettings ));
    BKNI_Memset( &transInput, 0, sizeof( transInput ));
    BKNI_Memset( &transOutput, 0, sizeof( transOutput ));
    BKNI_Memset( &boxModeSettings, 0, sizeof( boxModeSettings ));
    BKNI_Memset( &heap_padding, 0, sizeof( heap_padding ));
    BKNI_Memset( &heap_usage, 0, sizeof( heap_usage ));
    BKNI_Memset( datetime, 0, sizeof( datetime ));
    BKNI_Memset( strTemp, 0, sizeof( strTemp ));
    BKNI_Memset( audioDecodeCodecEnabledDefaults, 0, sizeof( audioDecodeCodecEnabledDefaults ));
    BKNI_Memset( audioEncodeCodecEnabledDefaults, 0, sizeof( audioEncodeCodecEnabledDefaults ));
    BKNI_Memset( &lVideoDecoderCapabilities, 0, sizeof( lVideoDecoderCapabilities ));
#if NEXUS_HAS_VIDEO_ENCODER
    BKNI_Memset( &lVideoEncoderCapabilities, 0, sizeof( lVideoEncoderCapabilities ));
#endif /* NEXUS_HAS_VIDEO_ENCODER */
    BKNI_Memset( &lDisplayCapabilities, 0, sizeof( lDisplayCapabilities ));
    BKNI_Memset( &svgPlot, 0, sizeof( svgPlot ));

    queryString   = getenv( "QUERY_STRING" );
    remoteAddress = getenv( "REMOTE_ADDR" );

    if (remoteAddress== NULL)
    {
        unsigned int strlength = strlen( "unknown" ) + 1;
        remoteAddress = malloc( strlength );
        strncpy( remoteAddress, "unknown", strlength );
    }

    printf( "Content-type: text/html\n\n" );
    BDBG_LOG(( "%s:%u CGI BEGINNING ================================================", argv[0], __LINE__ ));
    fflush( stdout ); fflush( stderr );
    /*printf( "argc %d; NEXUS_PLATFORM %u; NEXUS_MAX_MEMC %d; NEXUS_NUM_MEMC %d \n", argc, NEXUS_PLATFORM, NEXUS_MAX_MEMC, NEXUS_NUM_MEMC );*/

    /* determine if any apps are using Nexus at the moment */
    if (getOpenNexusApps( strTemp, sizeof( strTemp )))
    {
        char *token = NULL;
        int   count = 0;

        printf( "~OPENAPPS~" );
        token = strtok( strTemp, ":" );
        count = 0;
        while (token)
        {
            if (count == 0)
            {
                printf( "<div style=\"color:red;\" ><h2>ERROR: The following app(s) are using Nexus</h2><ol type=1 >" );
            }

            printf( "<li>%s</li>", token );
            token = strtok( NULL, ":" );
            count++;
        }
        printf( "</ol><br>Please stop the app%s and refresh the page.</div>~", ( count>1 ) ? "s" : "" );

        return( 0 );
    }
    PRINTF( "~DEBUG~queryString (%s)~", queryString );
    if (queryString == NULL )
    {
        printf( "<h2 class=margin20 >QUERY_STRING has not been set.</h2>\n" );
        return -1;
    }
    scanForTag( queryString, "boxmode=", &boxmode );
    scanForTag( queryString, "boxmodefirst=", &svgPlot.boxmodeFirst );
    scanForTag( queryString, "init=", &initValue );
    scanForStr( queryString, "datetime=", sizeof( datetime ), datetime );

    /* if browser provided a new date/time value */
    if (strlen( datetime ))
    {
        char cmd[64];
        sprintf( cmd, "export TZ=\"GST-8\" && date %s > /dev/null", datetime );
        system( cmd );
    }

    /* if we are initializing the browser page */
    if (initValue == 1)
    {
        int  idx = 0;
        char boxmodes[1024];

        memset( boxmodes, 0, sizeof( boxmodes ));

        lNumberMemcs = getProductIdMemc();

        PRINTF( "~DEBUG~detected init; NEXUS_NUM_MEMC %u; lNumberMemcs %d ~", NEXUS_NUM_MEMC, lNumberMemcs );
        /* loop through global array to find all boxmodes that will run on this platform */
        for (idx = 0; idx < BMEMCONFIG_MAX_BOXMODES; idx++)
        {
            /* if the boxmode is compatible with this platform AND is not one of the DEMO box modes */
            if ( ( g_bmemconfig_box_info[idx].numMemc == lNumberMemcs ) && ( 0 < g_bmemconfig_box_info[idx].boxmode ) && ( g_bmemconfig_box_info[idx].boxmode < 1000 ) )
            {
                /* if something is already in the list, add the delimiter */
                if (boxmodes[0] != '\0')
                {
                    strncat( boxmodes, "|", sizeof( boxmodes ) - strlen( boxmodes ) -1 );
                }
                sprintf( boxmodeStr, "%u", g_bmemconfig_box_info[idx].boxmode );
                strncat( boxmodes, boxmodeStr, sizeof( boxmodes ) - strlen( boxmodes ) -1 );
            }
        }
        /* if no boxmodes were found, default to boxmode 0 */
        if (boxmodes[0] == '\0')
        {
            boxmodes[0] = '0';
        }
        strcpy( svgPlot.platform, getPlatform());
        #if 1
        printf( "~boxmodes~%s~", boxmodes );
        #else /* for debug purposes */
        printf( "~boxmodes~1|3~" );
        #endif
        BKNI_Memset( strTemp, 0, sizeof( strTemp ));
        NEXUS_Platform_GetReleaseVersion( strTemp, sizeof( strTemp )); /* returns platform, chip_ver, and ursr version */
        sprintf( boxmodeTableHtml, "<table border=0 style=\"border-collapse:collapse;\" ><tr>"
                 "<td><span style=\"font-size:24pt;font-weight:bold;\" >BoxMode Summary for %s (Variant %s)</span></td>"
                 "<td><span id=COMPLETION_PCT style=\"color:red;font-size:16pt;font-weight:bold;\" ></span></td></tr></table>"
                 "<span id=SUPPORTED_CODECS style=\"font-size:16pt;font-weight:bold;\" ></span><br>&nbsp;<table style=\"border-collapse:collapse;\" >"
                 "<tr bgcolor=\"lightgray\" ><th class=border >BOX MODE</th><th class=border width=50 >PIP</th>", strTemp, getProductIdStr() );
        for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++)
        {
            sprintf( strTemp, "<th class=border width=100 >DECODE %u</th>", i );
            strncat( boxmodeTableHtml, strTemp, sizeof( boxmodeTableHtml ) - strlen( boxmodeTableHtml ) -1 );
        }
        sprintf( strTemp, "<th class=border >HDMI IN</th><th class=border >ENCODE</th></tr>\n" );
        strncat( boxmodeTableHtml, strTemp, sizeof( boxmodeTableHtml ) - strlen( boxmodeTableHtml ) -1 );
    }
    else if (boxmode >= 0) /* needs to be >= for chips like 7425 to work because they only have box mode 0 in their list */
    {
        lNumberMemcs = getProductIdMemc();

        PRINTF("~DEBUG~boxmode %d; lNumberMemcs %d; getBoxModeMemc %d ~", boxmode, lNumberMemcs, getBoxModeMemc( boxmode ) );
        /* do not worry about mismatched MEMCs for boxmode 0 */
        if ( ( boxmode!= 0 ) && ( getBoxModeMemc( boxmode ) != lNumberMemcs ))
        {
            /*printf( "<h2 class=margin20 >BoxMode %u has %u MEMCs but the %s has %u.</h2>\n", boxmode, getBoxModeMemc( boxmode ), getPlatform(), lNumberMemcs );*/
        }
        else
        {
            sprintf( boxmodeStr, "%u", boxmode );
            NEXUS_SetEnv( "B_REFSW_BOXMODE", boxmodeStr );

            Memconfig_GetBoxModeDefaultSettings( boxmode, &boxModeSettings );

            /* API is void; does not return anything */
            NEXUS_Platform_GetDefaultSettings( &platformSettings );

            /* the LpcmDvd is enabled by default, but we don't support it anymore; turn it off. */
            audioDecodeCodecEnabledDefaults[NEXUS_AudioCodec_eLpcmDvd] = false;

            /* determine how many transcodes this boxmode has */
            for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
            {
                if (boxModeSettings.videoDecoder[i].property == Memconfig_VideoDecoderProperty_eTranscode)
                {
                    maxNumTranscodesByDefault++;
                }
            }

            platformSettings.openFrontend = false;

            NEXUS_GetDefaultMemoryConfigurationSettings( &memConfigSettings );

            /* create a combined superset of known codecs by looking at each decoder and creating a new superset */
            for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++)
            {
                for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
                {
                    /*printf("~DEBUG~videoDecoder[0].supported[%u:%s]:%u~", idx, get_codec_name(idx), memConfigSettings.videoDecoder[0].supportedCodecs[idx] );*/
                    svgPlot.supportedCodecsSuperset[codec] |= memConfigSettings.videoDecoder[i].supportedCodecs[codec];
                }

                /* 7439 boxmode 24: is headless and has 4 video decoders ... 2 are for transcode and 2 are for graphics PIP */
                if (boxModeSettings.videoDecoder[i].property == Memconfig_VideoDecoderProperty_eGraphicsPip)
                {
                    svgPlot.num_decoders_graphics_pip++;
                }
            }

            /* save the default audio decoder codecs to help us NOT display codecs that are initially turned off by default */
            for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
            {
                audioDecodeCodecEnabledDefaults[codec] = memConfigSettings.audio.decodeCodecEnabled[codec];
                audioEncodeCodecEnabledDefaults[codec] = memConfigSettings.audio.encodeCodecEnabled[codec];
            }

            compute_max_video_decoders( &memConfigSettings );

            compute_max_video_formats( &memConfigSettings, &boxModeSettings );

            Memconfig_AppUsageGetDefaultSettings( &transInput );

            /* a few settings need to be copied from memconfigSettings to transInput */
            transfer_settings( &memConfigSettings, &transInput, &transOutput, &boxModeSettings );

            rc = NEXUS_Platform_MemConfigInit( &platformSettings, &memConfigSettings );
            /*printf( "~%s: NEXUS_BASE_ONLY_INIT: NEXUS_Platform_MemConfigInit() returned %d; uses baseOnlyInit~", argv[0], rc );*/

            if (rc != 0)
            {
                char *errorLogContents = bmemperf_get_boa_error_log( argv[0] );
                unsigned int errorLogLineCount = bmemperf_get_boa_error_log_line_count( errorLogContents );
                printf( "~FATALNOALERT~NEXUS_Platform_MemConfigInit() returned error code %d for %s BoxMode %d<br><textarea cols=150 rows=%u >%s</textarea>~",
                        rc, getPlatform(), boxmode, errorLogLineCount, errorLogContents );
                Bsysperf_Free( errorLogContents );
            }
            else
            {
                char strDdrScbFreq[BMEMCONFIG_MAX_DDR_SCB_STRING];

                memset ( strDdrScbFreq, 0, sizeof(strDdrScbFreq) );

                /* if audio decoders are disabled ... disable video encoders */
                for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
                {
                    memConfigSettings.audio.numEchoCancellers = NEXUS_NUM_HDMI_OUTPUTS;
                }

                {
                    unsigned int heapIdx;
                    for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
                    {
                        Bsysperf_Free( g_heap_info[heapIdx].memoryType );
                    }
                }

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
                NEXUS_GetDisplayCapabilities( &lDisplayCapabilities );
                PRINTF( "~DEBUG~boxmode:%u NEXUS_NUM_DISPLAYS %u; NEXUS_MAX_DISPLAYS %u~", boxmode, NEXUS_NUM_DISPLAYS, NEXUS_MAX_DISPLAYS );
                for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
                {
                    PRINTF( "~DEBUG~boxmode:%u display[%d].numVideoWindows %u gfx.width %-4u height %-4u maxFormat:%u(%s)~", boxmode, i,
                        lDisplayCapabilities.display[i].numVideoWindows, lDisplayCapabilities.display[i].graphics.width,
                        lDisplayCapabilities.display[i].graphics.height, memConfigSettings.display[i].maxFormat,
                        get_maxformat_name( memConfigSettings.display[i].maxFormat ));
                    svgPlot.display[i].numVideoWindows = lDisplayCapabilities.display[i].numVideoWindows;
                    #if 1
                    if (memConfigSettings.display[i].maxFormat)
                    {
                        strncpy( svgPlot.display[i].str_max_format,              get_maxformat_name( memConfigSettings.display[i].maxFormat ),
                            sizeof ( svgPlot.display[i].str_max_format ) -1 );
                    }
                    #endif /* if 1 */
                }
#endif /* NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

#if NEXUS_HAS_VIDEO_DECODER
                NEXUS_GetVideoDecoderCapabilities( &lVideoDecoderCapabilities );
                PRINTF( "~DEBUG~boxmode:%u numVideoDecoders %u; NEXUS_MAX_VIDEO_DECODERS %u", boxmode, lVideoDecoderCapabilities.numVideoDecoders, NEXUS_MAX_VIDEO_DECODERS );
                displayIdx = 0;
                for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
                {
                    if (transInput.videoDecoder[i].enabled)
                    {
#if 0
                        printf( "~DEBUG~boxmode:%u videoDecoder[%d].enabled %u; b2160 %u; numMosaic %u; colorDepth %-2u maxFmt %u (%s)~", boxmode, i, transInput.videoDecoder[i].enabled,
                            transInput.videoDecoder[i].b3840x2160, transInput.videoDecoder[i].numMosaic, lVideoDecoderCapabilities.videoDecoder[i].feeder.colorDepth,
                            memConfigSettings.videoDecoder[i].maxFormat,  get_maxformat_name( memConfigSettings.videoDecoder[i].maxFormat ));
#endif
                        svgPlot.num_decoders++;
                        svgPlot.videoDecoder[displayIdx].enabled     = true;
                        svgPlot.videoDecoder[displayIdx].color_depth = lVideoDecoderCapabilities.videoDecoder[i].feeder.colorDepth;
                        if (supportsH265Compression( &memConfigSettings ))
                        {
                            strncpy( svgPlot.videoDecoder[displayIdx].str_compression, "HEVC", sizeof( svgPlot.videoDecoder[displayIdx].str_compression ) - 1 );
                        }
                        else
                        {
                            strncpy( svgPlot.videoDecoder[displayIdx].str_compression, "AVC", sizeof( svgPlot.videoDecoder[displayIdx].str_compression ) - 1 );
                        }
                        sprintf( svgPlot.videoDecoder[displayIdx].str_max_resolution, "%s",  get_maxformat_name( memConfigSettings.videoDecoder[i].maxFormat ));
                        /* the videoDecoder index and display index only match up to and including the PIP index ... [1] */
                        if (displayIdx <= 1)
                        {
                            sprintf( svgPlot.videoDecoder[displayIdx].str_gfx_resolution, "%ux%u", lDisplayCapabilities.display[displayIdx].graphics.width,
                                lDisplayCapabilities.display[displayIdx].graphics.height );
                            PRINTF( "~DEBUG~boxmode:%u videoDecoder[%d].str_gfx_resolution (%s)~", boxmode, i, svgPlot.videoDecoder[displayIdx].str_gfx_resolution );
                        }

                        /* for 97439 boxmode 9: videoDecoder[1] is skipped; videoDecoder[2] is enabled but need to use displayCapabilities[1] */
                        displayIdx++;
                    }
                }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_HAS_VIDEO_ENCODER
                NEXUS_GetVideoEncoderCapabilities( &lVideoEncoderCapabilities );
                PRINTF( "~DEBUG~boxmode:%u NEXUS_MAX_VIDEO_ENCODERS %u", boxmode, NEXUS_MAX_VIDEO_ENCODERS );
                for (i = 0; i< (int) NEXUS_MAX_VIDEO_ENCODERS; i++)
                {
                    PRINTF( "~DEBUG~boxmode:%d NEXUS videoEncoder[%d].supported %u; displayIndex %u; used:%u; interlaced:%d; wid:%u; hgt:%u~",
                        boxmode, i, lVideoEncoderCapabilities.videoEncoder[i].supported, lVideoEncoderCapabilities.videoEncoder[i].displayIndex,
                        lVideoEncoderCapabilities.videoEncoder[i].memory.used, lVideoEncoderCapabilities.videoEncoder[i].memory.interlaced,
                        lVideoEncoderCapabilities.videoEncoder[i].memory.maxWidth, lVideoEncoderCapabilities.videoEncoder[i].memory.maxHeight );
                    if (lVideoEncoderCapabilities.videoEncoder[i].memory.used)
                    {
                        sprintf( svgPlot.videoEncoder[i].str_max_resolution, "%ux%u%s",  lVideoEncoderCapabilities.videoEncoder[i].memory.maxWidth,
                            lVideoEncoderCapabilities.videoEncoder[i].memory.maxHeight, ( lVideoEncoderCapabilities.videoEncoder[i].memory.interlaced ) ? "p/i" : "" );
                        svgPlot.videoEncoder[i].displayIndex = lVideoEncoderCapabilities.videoEncoder[i].displayIndex;
                        PRINTF( "~DEBUG~boxmode:%u videoEecoder[%d].str_max_resolution (%s)~", boxmode, i, svgPlot.videoEncoder[i].str_max_resolution );
                    }
                }

                for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
                {
                    if (transInput.encoders[i].enabled)
                    {
                        svgPlot.num_encoders++;
                    }
                }

                main_used = (( svgPlot.num_decoders - svgPlot.num_encoders ) >= 1 );
                pip_used  = (( svgPlot.num_decoders - svgPlot.num_encoders ) >= 2 );

                /* if there are encoders active, determine the graphics resolution */
                if (svgPlot.num_encoders && ( NEXUS_NUM_DISPLAYS >= 2 ))
                {
                    /*
                    boxmode:7 NEXUS_NUM_DISPLAYS 7; NEXUS_MAX_DISPLAYS 7       boxmode:14 NEXUS_NUM_DISPLAYS 7; NEXUS_MAX_DISPLAYS 7
                    boxmode:7 display[0].gfx.width 1280; height 720            boxmode:14 display[0].gfx.width 1920; height 1080
                    boxmode:7 display[1].gfx.width 0;    height 0              boxmode:14 display[1].gfx.width 0;    height 0
                    boxmode:7 display[2].gfx.width 1280; height 720            boxmode:14 display[2].gfx.width 0;    height 0
                    boxmode:7 display[3].gfx.width 1280; height 720            boxmode:14 display[3].gfx.width 0;    height 0
                    boxmode:7 display[4].gfx.width 1280; height 720            boxmode:14 display[4].gfx.width 1920; height 1080
                    boxmode:7 display[5].gfx.width 1280; height 720            boxmode:14 display[5].gfx.width 1920; height 1080
                    boxmode:7 display[6].gfx.width 1280; height 720            boxmode:14 display[6].gfx.width 1920; height 1080
                    */
                    for (i = 0; i< (int) NEXUS_MAX_VIDEO_ENCODERS; i++)
                    {
                        if (lVideoEncoderCapabilities.videoEncoder[i].supported)
                        {
                            displayIdx = lVideoEncoderCapabilities.videoEncoder[i].displayIndex;
                            if (lDisplayCapabilities.display[displayIdx].graphics.width && lDisplayCapabilities.display[displayIdx].graphics.height)
                            {
                                sprintf( svgPlot.encoder_gfx_resolution, "%ux%u", lDisplayCapabilities.display[displayIdx].graphics.width,
                                    lDisplayCapabilities.display[displayIdx].graphics.height );
                                svgPlot.num_encoders_graphics++;
                                PRINTF( "~DEBUG~boxmode:%u; encoderGfx[%u]: %ux%u; num_encoders_graphics %u~", boxmode, i, lDisplayCapabilities.display[displayIdx].graphics.width,
                                    lDisplayCapabilities.display[displayIdx].graphics.height, svgPlot.num_encoders_graphics );
                            }
                        }
                    }
                }
#endif /* NEXUS_HAS_VIDEO_ENCODER */

                PRINTF( "~DEBUG~boxmode:%u; NEXUS_NUM_HDMI_INPUTS %u; hdDvi %u~", boxmode, NEXUS_NUM_HDMI_INPUTS, memConfigSettings.videoInputs.hdDvi );

                strncpy( svgPlot.platform, getPlatform(), sizeof( svgPlot.platform ) - 1 );
                svgPlot.boxmode = boxmode;
                strncpy( svgPlot.strDdrScb, getBoxModeDdrScbString( boxmode ), sizeof( svgPlot.strDdrScb ) - 1 );

                /* append the actual frequencies that we get from BOLT */
                snprintf( strDdrScbFreq, sizeof(strDdrScbFreq) - 1, "%s@%uMHz SCB@%uMHz", bmemperf_get_ddrType( 0, g_pMem ),
                          bmemperf_get_ddrFreqInMhz( getBoxModeDdrFreq ( boxmode ) ), bmemperf_get_scbFreqInMhz( getBoxModeScbFreq ( boxmode ) ) );

                /* if the actual matches the expected */
                if (strcmp( strDdrScbFreq, svgPlot.strDdrScb ) == 0 )
                {
                    strDdrScbFreq[0] = 0; /* empty out the string so that nothing will display (strings match) */
                }
                else /* display the different frequencies in red italic */
                {
                    snprintf( strDdrScbFreq, sizeof(strDdrScbFreq) - 1, "<i style=\"color:red;\" ><small> (BOLT %s@%03uMHz SCB@%03uMHz)</small></i>", bmemperf_get_ddrType( 0, g_pMem ),
                              bmemperf_get_ddrFreqInMhz( getBoxModeDdrFreq (boxmode ) ), bmemperf_get_scbFreqInMhz( getBoxModeScbFreq ( boxmode ) ) );
                }
                strncat( svgPlot.strDdrScb, strDdrScbFreq, sizeof( svgPlot.strDdrScb ) - 1 );
                /* e.g. DDR3@1067MHz SCB@432MHz (BOLT DDR@1067MHz SCB@432MHz) */

#if NEXUS_HAS_HDMI_INPUT
                svgPlot.num_hdmi_inputs = memConfigSettings.videoInputs.hdDvi;
#endif
                PRINTF( "~DEBUG~boxmode:%u main %u; pip %u; decoders %u; hdmi %u~",
                    boxmode, main_used, pip_used, svgPlot.num_decoders,  memConfigSettings.videoInputs.hdDvi );
#if NEXUS_HAS_VIDEO_ENCODER
                PRINTF( "~DEBUG~boxmode:%u encoders %u; gfx_encoders %u; ~", boxmode, svgPlot.num_encoders, svgPlot.num_encoders_graphics );
#endif /* NEXUS_HAS_VIDEO_ENCODER */

                Add_BoxmodeTableHtml( &svgPlot, boxmodeTableHtml, &memConfigSettings );

                printf( "~PAGECONTENTS~" );
                Create_SvgPlot( &svgPlot, &memConfigSettings );
                printf( "~" );

                PRINTF( "~DEBUG~boxmodeFirst %u~", svgPlot.boxmodeFirst );

                /* send back the superset of codecs for this box mode. the javascript file will compare with previous codec list */
                /* overwrite the supportedCodecs for decoder[0] before calling getVideoCodecsStr() */
                for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
                {
                    memConfigSettings.videoDecoder[0].supportedCodecs[codec] = svgPlot.supportedCodecsSuperset[codec];
                }
                printf( "~SUPPORTED_CODECS~Supported Codecs: %s~", getVideoCodecsStr( 0, &memConfigSettings ));

                PRINTF("~DEBUG~calling NEXUS_Platform_Uninit()~");
                NEXUS_Platform_Uninit();
            }                                              /* NEXUS_Platform_MemConfigInit() was successful */
        }
    }
    printf( "~BOXMODETABLE~%s~", boxmodeTableHtml );

    printf( "~ALLDONE~for boxmode %d~", boxmode );

    return( 0 );
}                                                          /* main */
