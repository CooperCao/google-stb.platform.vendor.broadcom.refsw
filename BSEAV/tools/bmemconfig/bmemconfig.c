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
/*
    To add new fields:
 1) add html input type (checkbox, radio, text, dropdown, etc. to page_transport(), page_display(), etc
 2) make sure the input type has a unique id ... e.g. vdec0maxformat, disp0win1used, etc
 3) modify update_settings() to process the id correctly
 4) modify write_settings to write the value to state file
 4) modify read_settings to read the value from state file
 5) in js file, modify the setSettings() function to handle the tag correctly
*/
#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "nexus_transport_capabilities.h"
#include "nexus_video_decoder.h"
#include "bmemconfig_videoformats.h"
#include "boxmodes_defines.h"
#include "memusage.h"
#include "bstd.h"
#include "bdbg.h"
#include "bkni.h"
#include "bmedia_types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "../../../nexus/utils/namevalue.inc"
#include "bmemperf_utils.h"
#include "bmemperf_info.h"

BDBG_MODULE(bmemconfig);
/**
This app queries the default heaps per platform, typically one heap per memory controller.
**/

#define OUTPUT_STATE_FILE        "bmemconfig.state"
#define STATE_FILE_FULL_PATH_LEN 64
#define STATE_FILE_RESTORED      "bmemconfig.state.restored"
#define VDEC_FORMAT              "vdec %u %u %u %u %u %u %u %u\n"
#define VDEC_FORMAT_CDB          "%u %u %u %u %u %u %u\n"
#define ADEC_FORMAT              "adec %u %u\n"
#define ADEC_MAX_NUM_FORMAT      "numadec %u %u %u %u\n"
#define ADEC_MISC_FORMAT         "%u %u %u %u %u %u %u %u %u\n"
#define ADEC_PRI_SEC_FORMAT      "%u %u %u %u %u\n"
#define SDEC_FORMAT              "sdec %u %u %u %u\n"  /* 4 fewer than VDEC (no mosaic; no colorDepth ) */
#define SDEC_FORMAT_CDB          "%u %u %u %u %u %u\n" /* 1 fewer than VDEC (no mosaic) */
#define VENC_FORMAT              "venc %u %u %u %u %u %u\n"
#define REC_FORMAT               "rec %u %u %u\n"
#define PB_FORMAT                "pb %u %u %u\n"
#define MSG_FORMAT               "msg %u %u\n"
#define LIVE_FORMAT              "live %u %u\n"
#define REMUX_FORMAT             "remux %u %u\n"
#define GRAPHICS_FORMAT          "gfx %u %u %u %u %u %u %u\n"
#define GRAPHICS_3D_FORMAT       "g3d %u\n"
#define GRAPHICS_ENCODER_FORMAT  "genc %u %u %u %u %u\n"
#define USAGE_FORMAT_BYTES       "%09u "
#define SID_FORMAT               "sid %u\n"
#define VIDEO_INPUT_FORMAT       "vinp %u %u\n"
#define LOG_FILE_FULL_PATH_LEN   64

#define TITLE_WIDTH_SIZE "width:600px;font-size:22pt;"
static char *VideoDecoderPropertyStr[Memconfig_VideoDecoderProperty_eMax] = {"ERROR", "Main", "PIP", "Transcode", "Graphics PIP" };
static char *DisplayPropertyStr[Memconfig_DisplayProperty_eMax] = {"ERROR", "Primary", "Secondary", "Tertiary", "Transcode", "Graphics PIP" };
typedef enum
{
    MEMCONFIG_AUDIO_USAGE_PRIMARY,
    MEMCONFIG_AUDIO_USAGE_SECONDARY,
    MEMCONFIG_AUDIO_USAGE_PASSTHROUGH,
    MEMCONFIG_AUDIO_USAGE_TRANSCODE,
    MEMCONFIG_AUDIO_USAGE_MAX
} Memconfig_AudioUsages;
static char *postProcessing[NEXUS_AudioPostProcessing_eMax] = {"SampleRateConverter", "CustomVoice", "AutoVolumeLevel", "TrueSurround", "TruVolume", "Dsola", "Btsc"};
static char *dolbyCodecVersion[NEXUS_AudioDolbyCodecVersion_eMax] = {"Ac3", "Ac3Plus", "MS10", "MS11", "MS12"};

#define PRINTF                        noprintf
#define NEXUS_MAX_LIVECHANNELS        6
#define MAX_NUM_GRAPHICS_ENCODERS     2
#define NUMBER_MEMC                   3
#define HEAP_INDEX_SECONDARY_GRAPHICS 5
#define MEGABYTES                     ( 1024/1024 )

static char state_filename[] = OUTPUT_STATE_FILE ".ip address goes here                 ";

static unsigned int maxVdecsEnabledByDefault = 0;                       /* 7445 has 6 vdecs but user can only configure 4 of them */
static unsigned int maxVdecFormatResolutionByDefault[NEXUS_MAX_VIDEO_DECODERS]; /* don't display 3820 if default max is only 480 */
#if NEXUS_NUM_STILL_DECODES
static unsigned int maxSdecsEnabledByDefault = 0;
static unsigned int maxSdecFormatResolutionByDefault[NEXUS_MAX_VIDEO_DECODERS]; /* don't display 3820 if default max is only 480 */
#endif /* if NEXUS_NUM_STILL_DECODES */
static unsigned int      maxDisplayFormatByDefault[NEXUS_MAX_DISPLAYS]; /* used on the Display Settings page */
static unsigned int      maxDisplayFormatByDefault[NEXUS_MAX_DISPLAYS]; /* used on the Display Settings page */
static unsigned int      maxGraphicsEnabledByDefault = 0;               /* used on the Graphics page (only show graphics that are on by default) */
static unsigned int      maxNumTranscodesByDefault   = 0;
static bool              videoDecodeCodecEnabledDefaults[NEXUS_MAX_VIDEO_DECODERS][NEXUS_VideoCodec_eMax];
static bool              stillDecodeCodecEnabledDefaults[NEXUS_MAX_STILL_DECODERS][NEXUS_VideoCodec_eMax];
static bool              audioDecodeCodecEnabledDefaults[NEXUS_AudioCodec_eMax];
static bool              audioEncodeCodecEnabledDefaults[NEXUS_AudioCodec_eMax];
static unsigned int      heap_sizes[NEXUS_MAX_HEAPS][NEXUS_NUM_MEMC];   /* set in print_heaps() and saved in write_settings() */
static unsigned long int heap_totals[NEXUS_MAX_HEAPS];
static unsigned long int heap_usage[NEXUS_MAX_HEAPS];
static unsigned long int heap_padding[NEXUS_MAX_HEAPS];
static unsigned long int heap_totals_previous[NEXUS_MAX_HEAPS];
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
static Memconfig_AudioChannelInfo audioChannelInfo[MEMCONFIG_NUM_AUDIO_CHANNEL_TYPES] = {{6, "5.1"}, {8, "7.1"}};
#define MEMCONFIG_NUM_AUDIO_SAMPLE_RATES 2
static unsigned int audioSampleRates[MEMCONFIG_NUM_AUDIO_SAMPLE_RATES] = {48000, 96000};

/* these global variables are solely needed to compile with bmemperf_utils.c */
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
int           g_MegaBytes           = 0; /* set to 1 when user wants data displayed in megabytes instead of megabits (default) */
int           g_MegaBytesDivisor[2] = {1, 8};
char         *g_MegaBytesStr[2] = {"Mbps", "MBps"};
char         *g_client_name[BMEMPERF_MAX_NUM_CLIENT];
bmemperf_info g_bmemperf_info;

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
    if (( strcmp( appName, "bmemconfig.cgi" ) == 0 ) || ( strcmp( appName, "logger" ) == 0 ))
    {
        return( -1 );
    }

    strncat( pOpenNexusApp, appName, bufferLen - strlen( pOpenNexusApp ) - 1 );
    strncat( pOpenNexusApp, " (", bufferLen - strlen( pOpenNexusApp ) - 1 );
    strncat( pOpenNexusApp, pid, bufferLen - strlen( pOpenNexusApp ) - 1 ); /* the PID */
    strncat( pOpenNexusApp, "):", bufferLen - strlen( pOpenNexusApp ) - 1 );

    return( 0 );
} /* addOpenNexusApp */

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
    char     line[MAX_LENGTH_LSOF]; /* e.g: 3747 /mnt/nfs/playback */
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
                *posSpace = 0; /* null-terminate the PID string */

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
} /* getOpenNexusApps */

/**
 *  Function: This function determines whether or not the specified heap index is the main heap
 **/
static bool isHeapMain(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_MEMC0_MAIN_HEAP )
    if (heapIdx==NEXUS_MEMC0_MAIN_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function determines whether or not the specified heap index is the secure heap
 **/
static bool isHeapSecure(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_VIDEO_SECURE_HEAP )
    if (heapIdx==NEXUS_VIDEO_SECURE_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function determines whether or not the specified heap index is the sage heap
 **/
static bool isHeapSage(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_SAGE_SECURE_HEAP )
    if (heapIdx==NEXUS_SAGE_SECURE_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function determines whether or not the specified heap index is the driver heap
 **/
static bool isHeapDriver(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_MEMC1_DRIVER_HEAP )
    if (heapIdx==NEXUS_MEMC1_DRIVER_HEAP) {rc = true; }
#endif
#if defined ( NEXUS_MEMC2_DRIVER_HEAP )
    if (heapIdx==NEXUS_MEMC2_DRIVER_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function determines whether or not the specified heap index is the graphics heap
 **/
static bool isHeapGraphics(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_MEMC0_GRAPHICS_HEAP )
    if (heapIdx==NEXUS_MEMC0_GRAPHICS_HEAP) {rc = true; }
#endif
#if defined ( NEXUS_MEMC1_GRAPHICS_HEAP )
    if (heapIdx==NEXUS_MEMC1_GRAPHICS_HEAP) {rc = true; }
#endif
#if defined ( NEXUS_MEMC2_GRAPHICS_HEAP )
    if (heapIdx==NEXUS_MEMC2_GRAPHICS_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function determines whether or not the specified heap index is the picture heap
 **/
static bool isHeapPicture(
    unsigned int heapIdx
    )
{
    bool rc = heapIdx==9999;

#if defined ( NEXUS_MEMC0_PICTURE_BUFFER_HEAP )
    if (heapIdx==NEXUS_MEMC0_PICTURE_BUFFER_HEAP) {rc = true; }
#endif
#if defined ( NEXUS_MEMC1_PICTURE_BUFFER_HEAP )
    if (heapIdx==NEXUS_MEMC1_PICTURE_BUFFER_HEAP) {rc = true; }
#endif
#if defined ( NEXUS_MEMC2_PICTURE_BUFFER_HEAP )
    if (heapIdx==NEXUS_MEMC2_PICTURE_BUFFER_HEAP) {rc = true; }
#endif

    return( rc );
}

/**
 *  Function: This function will return heap information to the user based on the heap
 *  index provided by the user.
 **/
static int Memconfig_GetHeapInfo(
    int                 heapIndex,
    Memconfig_HeapInfo *pHeapInfo
    )
{
    int rc = 0;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_HeapHandle            heap = NULL;
    NEXUS_MemoryStatus          status;

    BKNI_Memset( &platformConfig, 0, sizeof( platformConfig ));
    NEXUS_Platform_GetConfiguration( &platformConfig );
    heap = platformConfig.heap[heapIndex];
    rc   = NEXUS_Heap_GetStatus( heap, &status );

    strncpy( pHeapInfo->heapMapping, "Unmapped", sizeof( pHeapInfo->heapMapping ));

    switch (heapIndex)
    {
        case NEXUS_MEMC0_MAIN_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Main", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "Driver,App", sizeof( pHeapInfo->heapName ));
            break;
        }
#ifdef NEXUS_VIDEO_SECURE_HEAP
        case NEXUS_VIDEO_SECURE_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Secure", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_VIDEO_SECURE_HEAP */
#ifdef NEXUS_MEMC0_PICTURE_BUFFER_HEAP
        case NEXUS_MEMC0_PICTURE_BUFFER_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Picture", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC0_PICTURE_BUFFER_HEAP */
#ifdef NEXUS_MEMC0_GRAPHICS_HEAP
        case NEXUS_MEMC0_GRAPHICS_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Graphics", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "App", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC0_GRAPHICS_HEAP */
#ifdef NEXUS_SAGE_SECURE_HEAP
        case NEXUS_SAGE_SECURE_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Sage", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_SAGE_SECURE_HEAP */
#ifdef NEXUS_MEMC1_GRAPHICS_HEAP
        case NEXUS_MEMC1_GRAPHICS_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Graphics", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "App", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC1_GRAPHICS_HEAP */
#ifdef NEXUS_MEMC1_PICTURE_BUFFER_HEAP
        case NEXUS_MEMC1_PICTURE_BUFFER_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Picture", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC1_PICTURE_BUFFER_HEAP */
#ifdef NEXUS_MEMC1_DRIVER_HEAP
        case NEXUS_MEMC1_DRIVER_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Driver", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "Driver,App", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC1_DRIVER_HEAP */
#ifdef NEXUS_MEMC2_GRAPHICS_HEAP
        case NEXUS_MEMC2_GRAPHICS_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Graphics", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "App", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC2_GRAPHICS_HEAP */
#ifdef NEXUS_MEMC2_PICTURE_BUFFER_HEAP
        case NEXUS_MEMC2_PICTURE_BUFFER_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Picture", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC2_PICTURE_BUFFER_HEAP */
#ifdef NEXUS_MEMC2_DRIVER_HEAP
        case NEXUS_MEMC2_DRIVER_HEAP:
        {
            pHeapInfo->memcIndex = status.memcIndex;
            strncpy( pHeapInfo->heapName, "Driver", sizeof( pHeapInfo->heapName ));
            strncpy( pHeapInfo->heapMapping, "Driver,App", sizeof( pHeapInfo->heapName ));
            break;
        }
#endif /* ifdef NEXUS_MEMC2_DRIVER_HEAP */
        default:
        {
            snprintf( pHeapInfo->heapName, sizeof( pHeapInfo->heapName ), "Non standard heap naming heap#%d", heapIndex );
            rc = -1;
            break;
        }
    } /* switch */
    return( rc );
} /* Memconfig_GetHeapInfo */

/**
 *  Function: This function will return a string to the caller that describes the video decoder property.
 **/
char *Memconfig_GetVideoDecoderPropertyStr(
    int propertyIdx
    )
{
    if (propertyIdx < Memconfig_VideoDecoderProperty_eMax)
    {
        return( VideoDecoderPropertyStr[propertyIdx] );
    }
    return( "unknown property" );
}

/**
 *  Function: This function will return a string to the user describing the display property of the specified
 *  display index.
 **/
char *Memconfig_GetDisplayPropertyStr(
    int propertyIdx
    )
{
    if (propertyIdx < Memconfig_DisplayProperty_eMax)
    {
        return( DisplayPropertyStr[propertyIdx] );
    }
    return( "unknown property" );
}

/**
 *  Function: This function will determine the number of transcodes that are associated with the 97435 platform.
 *  This code is only needed in User Mode. It is not needed in Kernel Mode.
 **/
#if ( NEXUS_PLATFORM == 97435 )
#if ( NEXUS_MODE_proxy )
#else
static void GetNumTranscodes(
    unsigned *pNumTranscodes
    )
{
    BREG_Handle hReg     = NULL;
    void       *pMem     = NULL;
    int         memFd    = 0;
    uint32_t    regValue = 0, blockout = 0;

    if (hReg == NULL)
    {
        /* Open driver for memory mapping */
        memFd = bmemperfOpenDriver();

        PRINTF( "%s: memFd %d\n", __FUNCTION__, memFd );
        if (memFd == 0)
        {
            PRINTF( "Failed to open(/dev/mem)\n" );
            return;
        }

        fcntl( memFd, F_SETFD, FD_CLOEXEC );

        pMem = bmemperfMmap( memFd );

        PRINTF( "%s: pMem %p\n", __FUNCTION__, (void *) pMem );
        if (!pMem)
        {
            PRINTF( "Failed to mmap64 fd=%d, addr 0x%08x\n", memFd, BCHP_PHYSICAL_OFFSET );
            return;
        }
        BKNI_Init();
        BREG_Open( &hReg, (void *)pMem, (size_t)( 0x01f80000 ), NULL );

        if (hReg == NULL)
        {
            PRINTF( "Failed to BREG_OPen()\n" );
            return;
        }
        PRINTF( "%s: hReg %p\n", __FUNCTION__, (void *) hReg );
    }
    /* Read RTS settings to find the number of hardware (ViCE) video encode channels */
    regValue = BREG_Read32( hReg, BCHP_MEMC_ARB_0_CLIENT_INFO_72 );
    blockout = BCHP_GET_FIELD_DATA( regValue, MEMC_ARB_0_CLIENT_INFO_72, BO_VAL );
    PRINTF( "%s: blockout BCHP_MEMC_ARB_0_CLIENT_INFO_72 %x\n", __FUNCTION__, blockout );
    if (blockout==0x7fff)     /* MCVP0 off */
    {
        *pNumTranscodes = 4;
    }
    else
    {
        regValue = BREG_Read32( hReg, BCHP_MEMC_ARB_0_CLIENT_INFO_20 );
        blockout = BCHP_GET_FIELD_DATA( regValue, MEMC_ARB_0_CLIENT_INFO_20, BO_VAL );
        PRINTF( "%s: blockout BCHP_MEMC_ARB_0_CLIENT_INFO_20 %x\n", __FUNCTION__, blockout );
        if (blockout==0x7fff) /* MADR3_WR off */
        {
            PRINTF( "dual transcode RTS is no longer supported.  Please use CFE>cfgrts to select single or quad transcode\n" );
            *pNumTranscodes = 2;
        }
        else
        {
            *pNumTranscodes = 1;
        }
    }
} /* GetNumTranscodes */

#endif /* NEXUS_MODE_proxy */
#endif /* (NEXUS_PLATFORM == 97435) */

/**
 *  Function: This function will determine the box mode for all platforms. It will either use the environment
 *  variable B_REFSW_BOXMODE or the value BOLT has written to the proc file system. A special case is for the
 *  97435 platform. This function was copied from the NEXUS private API that does the same thing.
 **/
static int ReadBoxMode(
    void
    )
{
#if ( NEXUS_MODE_proxy )
    const char *env = NEXUS_GetEnv( "B_REFSW_BOXMODE" );
    PRINTF( "%s: B_REFSW_BOXMODE (%s)\n", __FUNCTION__, env );
    return( env ? NEXUS_atoi( env ) : 0 );

#else /* if ( NEXUS_MODE_proxy is not defined ) */
    int         boxMode = 0;
    const char *override;
    FILE       *pFile = NULL;
    override = NEXUS_GetEnv( "B_REFSW_BOXMODE" );
    if (override)
    {
        boxMode = atoi( override );
    }
    PRINTF( "%s: boxMode %d\n", __FUNCTION__, boxMode );
    /* if the environment variable is not set, try to figure it out from the proc file system */
    if (boxMode == 0)
    {
        pFile = fopen( "/proc/device-tree/bolt/rts", "r" );

        /* if the bolt file exists */
        if (pFile)
        {
            char buf[64];
            if (fgets( buf, sizeof( buf ), pFile ))
            {
                /* example: 20140402215718_7445_box1, but skip over everything (which may change) and get to _box# */
                char *p = strstr( buf, "_box" );
                if (p)
                {
                    boxMode = atoi( &p[4] );
                }
            }
            fclose( pFile );
        }
#if ( NEXUS_PLATFORM == 97435 )
        else /* proc file does not exist; try to figure it out from CFE values. */
        {
            static bool set = false;
            /* only read once per run */
            if (!set)
            {
                unsigned num_xcodes = 0;
                GetNumTranscodes( &num_xcodes );
                boxMode = ( num_xcodes == 1 ) ? 2 : 1;
                printf( "Box Mode %d set by cfgrts in CFE. num_xcodes %d\n", boxMode, num_xcodes );
                set = true;
            }
        }
#endif /* (NEXUS_PLATFORM == 97435) */
    }
    PRINTF( "%s: returning boxMode %d\n", __FUNCTION__, boxMode );
    return( boxMode );

#endif /* if ( NEXUS_MODE_proxy ) */
} /* ReadBoxMode */

/**
 *  Function: This function returns to the caller the number of megabytes of system memory used.
 **/
static unsigned long int getSystemMemoryMB(
    NEXUS_PlatformStatus *pPlatformStatus
    )
{
    unsigned int memc  = 0;
    unsigned int total = 0;

    PRINTF( "%s: MAX_MEMC %u; NUM_MEMC %u\n", __FUNCTION__, NEXUS_MAX_MEMC, NEXUS_NUM_MEMC );
    for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
    {
        total += pPlatformStatus->memc[memc].size;
        PRINTF( "%s: platformStatus->memc[%u].size %u; total %u\n", __FUNCTION__, memc, pPlatformStatus->memc[memc].size, total );
    }

    return( total / 1024 / 1024 );
} /* getSystemMemoryMB */

#if 0
/**
 *  Function: This function remove white space from the end of the specified string. White space includes
 *  both carriage returns and line feeds. Of course, the space character is also considered white space.
 **/
static int trimln(
    char *const line
    )
{
    int           rc      = 0;
    __SIZE_TYPE__ linelen = 0;

    if (line==NULL) {return( 0 ); }

    linelen = strlen( line );
    if (( linelen>0 ) && (( line[linelen-1]=='\n' ) || ( line[linelen-1]=='\r' )))
    {
        line[linelen-1] = '\0';
        rc              = 1;
    }

    linelen = strlen( line );
    if (( linelen>0 ) && (( line[linelen-1]=='\n' ) || ( line[linelen-1]=='\r' )))
    {
        line[linelen-1] = '\0';
        rc              = 1;
    }

    linelen = strlen( line );
    while (( linelen > 0 ) && ( line[linelen-1] == ' ' ))
    {
        line[linelen-1] = '\0';
        linelen--;
    }

    return( rc );
} /* trimln */

#endif /* if 0 */

/**
 *  Function: This function returns to the caller the number of bytes contained in the specified file.
 **/
static int getFileSize(
    const char *filename
    )
{
    struct stat file_stats;

    if (filename == NULL)
    {
        return( 0 );
    }

    if (stat( filename, &file_stats ) != 0)
    {
        PRINTF( "<!-- ERROR getting stats for file (%s) -->\n", filename );
        return( 0 );
    }

    return( file_stats.st_size );
} /* getFileSize */

/**
 *  Function: This function returns to the caller the contents of the specified file. Space will malloc'ed
 *  for the file size. It is up to the caller to free the contents pointer.
 **/
static char *getFileContents(
    const char *filename
    )
{
    FILE             *pFile    = NULL;
    char             *contents = NULL;
    unsigned long int filesize = 0;

    if (filename == NULL)
    {
        return( contents );
    }

    filesize = getFileSize( filename );
    if (filesize == 0)
    {
        return( contents );
    }
    contents = malloc( filesize + 1 );
    /* if we successfully allocated space for te file*/
    if (contents)
    {
        memset( contents, 0, filesize + 1 );
        pFile = fopen( filename, "r" );
        fread( contents, 1, filesize, pFile );
        fclose( pFile );
    }
    return( contents );
} /* getFileContents */

#if 0
/**
 *  Function: This function will search the provided name-value string for the specified search keyword
 *  and return to the user the value associated with the keyword. For example, if the name-value string
 *  is abc=123, this function will return the string 123 to the user.
 **/
static char *getNamedValue(
    const char *query_string,
    const char *searchTag
    )
{
    char *token        = NULL;
    char *pos          = NULL;
    int   count        = 0;
    char *posvalue     = NULL;
    char *lQueryString = malloc( strlen( query_string ) + 1 );

    strncpy( lQueryString, query_string, strlen( query_string ));
    token = strtok( lQueryString, "&" );
    count = 0;
    while (token)
    {
        /*printf("%s: token %d is (%s)\n", __FUNCTION__, count+1, token );*/
        /* does this name-value pair match the one we are looking for */
        if (( pos = strstr( token, searchTag )))
        {
            posvalue = strstr( pos, "=" );
            if (posvalue)
            {
                posvalue++;
            }
            /*printf("%s: returning (%s)\n", __FUNCTION__, posvalue );*/
            break;
        }

        token = strtok( NULL, "&" );
        count++;
    }
    if (lQueryString)
    {
        free( lQueryString );
    }
    return( posvalue );
} /* getNamedValue */

#endif /* if 0 */

/**
 *  Function: This function will return to the user a pointer to a string that contains the hours, minutes, and
 *  seconds of the current time.
 **/
static char *lHhMmSs(
    void
    )
{
    static char    fmt[64];
    struct timeval tv;
    struct tm     *tm;

    memset( fmt, 0, sizeof( fmt ));

    gettimeofday( &tv, NULL );
    if (( tm = localtime( &tv.tv_sec )) != NULL)
    {
        strftime( fmt, sizeof fmt, "%H:%M:%S", tm );
    }

    return( fmt );
}

/**
 *  Function: This function will search through the list of heaps for the one that matches the memc number and
 *  the heap name. If a match is found (it is normal for one to not be found), the function will output the size
 *  of the heap specified.
 *
 **/
static int OutputHeapSize(
    int                          memcIndex,
    const char                  *heapName,
    NEXUS_PlatformConfiguration *platformConfig,
    unsigned int                 heapIndex,
    int                          heapInfoMemcIndex,
    unsigned int                 usage
    )
{
    unsigned int       heapIdx = 0;
    int                rc      = 0;
    Memconfig_HeapInfo heapInfo;
    NEXUS_HeapHandle   heap;
    NEXUS_MemoryStatus status;
    unsigned int       megabytes = 0;

    PRINTF( "<!-- %s: looking for memc %u named (%s); platformConfig %p -->\n", __FUNCTION__, memcIndex, heapName, (void *) platformConfig );
    /* search through all heaps for the one that matches this memc */
    for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
    {
        if (Memconfig_GetHeapInfo( heapIdx, &heapInfo ) == 0)
        {
            if (( heapInfo.memcIndex == memcIndex ) && ( strcmp( heapName, heapInfo.heapName )==0 ) && ( heapIdx == heapIndex ))
            {
                heap = platformConfig->heap[heapIdx];
                if (!heap)
                {
                    continue;
                }

                PRINTF( "%s: for memc %u; name (%s); heapInfoMemcIndex %u<br>\n", __FUNCTION__, memcIndex, heapName, heapInfoMemcIndex );
                rc = NEXUS_Heap_GetStatus( heap, &status );
                BDBG_ASSERT( !rc );
                megabytes = status.size/1024/1024;
                PRINTF( "%s: memc %u; heap %u; megabytes %u; name (%s)<br>\n", __FUNCTION__, memcIndex, heapIdx, megabytes, heapName );
                PRINTF( "<!-- %s: match heapIdx %u -->\n", __FUNCTION__, heapIdx );
                break;
            }
        }
    }

    if (megabytes == 0)
    {
        printf( "<td>&nbsp;</td>" );
    }
    else
    {
        if (isHeapPicture( heapIndex ))
        {
            /* use megabytes from NEXUS_Heap_GetStatus above */
        }
        else
        {
            megabytes = ( usage + heap_padding[heapIdx] + ( 1024*1024-1 ) /* round up */ ) /1024/1024;
        }

        printf( "<td %s >%u MB</td>", ( megabytes!=heap_totals_previous[heapIdx] ) ? BACKGROUND_YELLOW : BACKGROUND_WHITE, megabytes );
        g_heap_info[heapIndex].memcIndex = memcIndex;
        g_heap_info[heapIndex].megabytes = megabytes;
    }

    return( megabytes );
} /* OutputHeapSize */

static int outputTotal(
    unsigned long int numBytes
    )
{
    float megaBytes = 1.0 * numBytes / 1024.0 /1024.0;

    if (numBytes)
    {
        printf( "TOTAL: <span %s >%5.1f MB</span>", BACKGROUND_WHITE, megaBytes );
    }

    return( 0 );
}

static int outputUsage(
    const char       *description,
    unsigned long int numBytes,
    unsigned long int numBytesPrevious
    )
{
    float megaBytes = 1.0 * numBytes / 1024.0 /1024.0;
    float kiloBytes = 1.0 * numBytes / 1024.0;

    if (megaBytes >= 1.)
    {
        if (numBytes == numBytesPrevious)
        {
            printf( "%s <span %s >%3.1f MB</span>; ", description, BACKGROUND_WHITE, megaBytes );
        }
        else
        {
            printf( "%s <span %s >%3.1f MB</span>; ", description, BACKGROUND_YELLOW, megaBytes );
        }
    }
    else if (kiloBytes >= 1.)
    {
        PRINTF( "%s: (%s); numBytes %lu; prev %lu\n", __FUNCTION__, description, numBytes, numBytesPrevious );
        if (numBytes == numBytesPrevious)
        {
            printf( "%s <span %s >%3.1f KB</span>; ", description, BACKGROUND_WHITE, kiloBytes );
        }
        else
        {
            printf( "%s <span %s >%3.1f KB</span>; ", description, BACKGROUND_YELLOW, kiloBytes );
        }
    }
    else if (numBytes > 0)
    {
        if (numBytes == numBytesPrevious)
        {
            printf( "%s <span %s >%lu bytes</span>; ", description, BACKGROUND_WHITE, numBytes );
        }
        else
        {
            printf( "%s <span %s >%lu bytes</span>; ", description, BACKGROUND_YELLOW, numBytes );
        }
    }
    return( 0 );
} /* outputUsage */

typedef struct
{
    unsigned long int bytesGeneral[NEXUS_NUM_DISPLAYS]; /* primary and secondary */
    unsigned          displayIdx;
} calculateUsageGraphicsSettings;

static unsigned long int calculateUsageGraphics(
    int                               heapIdx,
    const Memconfig_AppUsageSettings *pInput,
    const Memconfig_AppMemUsage      *pOutput,
    const Memconfig_BoxMode          *pBoxModeSettings,
    calculateUsageGraphicsSettings   *pSettings
    )
{
    int               displayIdx   = 0;
    unsigned long int bytesGeneral = 0;

    for (displayIdx = 0; displayIdx<NEXUS_NUM_DISPLAYS; displayIdx++)
    {
        PRINTF( "disp:%u ... heap %u vs %u; used %u; ", displayIdx, pBoxModeSettings->graphics[displayIdx].heapIdx, heapIdx, pBoxModeSettings->graphics[displayIdx].used );
        pSettings->bytesGeneral[displayIdx] = 0;
        if (pBoxModeSettings->graphics[displayIdx].used && ( pBoxModeSettings->graphics[displayIdx].heapIdx == heapIdx ))
        {
            BSTD_UNUSED( pOutput );

            pSettings->bytesGeneral[displayIdx] = pInput->surfaceSettings[displayIdx].width * pInput->surfaceSettings[displayIdx].height *
                pInput->surfaceSettings[displayIdx].bits_per_pixel * pInput->surfaceSettings[displayIdx].numFrameBufs / 8 /* bits per bytes */;
            bytesGeneral += pSettings->bytesGeneral[displayIdx];
            PRINTF( "%s: disp:%u: (%s):%lu bytes;<br>", __FUNCTION__, displayIdx, pBoxModeSettings->graphics[displayIdx].usage, bytesGeneral );
            pSettings->displayIdx = displayIdx;
        }
    }
    PRINTF( "%s: heap %u: %lu\n", __FUNCTION__, heapIdx, *pSettings->bytesGeneral );
    return( 0 );
} /* calculateUsageGraphics */

static int outputUsageGraphics(
    int                         heapIdx,
    Memconfig_AppUsageSettings *pInput,
    Memconfig_AppMemUsage      *pOutput,
    Memconfig_AppMemUsage      *pOutputPrevious,
    Memconfig_BoxMode          *pBoxModeSettings
    )
{
    unsigned int                   gfx              = 0;
    bool                           outputFirstValue = false;
    calculateUsageGraphicsSettings settings;
    unsigned long int              bytesGeneral  = 0;
    unsigned long int              bytesPrevious = 0;

    memset( &settings, 0, sizeof( settings ));
    calculateUsageGraphics( heapIdx, pInput, pOutput, pBoxModeSettings, &settings );
    PRINTF( "heapIdx %u; general[0] %lu; general[1] %lu<br>", heapIdx, settings.bytesGeneral[0], settings.bytesGeneral[1] );
    if (settings.bytesGeneral[0] + settings.bytesGeneral[1])
    {
        float megabytes = 0.;
        /* there could be two graphics sizes on same heap (primary and secondary on same) */
        for (gfx = 0; gfx<NEXUS_NUM_DISPLAYS; gfx++)
        {
            PRINTF( "heap %u; box->heapIdx %u;<br>", heapIdx, pBoxModeSettings->graphics[gfx].heapIdx );
            if (pBoxModeSettings->graphics[gfx].used && ( pBoxModeSettings->graphics[gfx].heapIdx == heapIdx ))
            {
                if (outputFirstValue == true)
                {
                    printf( ", " );
                }
                printf( "%s", pBoxModeSettings->graphics[gfx].usage );
                outputFirstValue = true;
                bytesGeneral    += settings.bytesGeneral[gfx];
                bytesPrevious   += pOutputPrevious->surfaceSettings[gfx].bytesGeneral;
                pOutput->surfaceSettings[gfx].bytesGeneral = settings.bytesGeneral[gfx];
                PRINTF( "%lu,%lu;prev %u,%u; bytesGen %lu)<br>", settings.bytesGeneral[0], settings.bytesGeneral[1],
                    pOutputPrevious->surfaceSettings[0].bytesGeneral, pOutputPrevious->surfaceSettings[1].bytesGeneral, bytesGeneral );
            }
        }
        megabytes = 1.0 * bytesGeneral / 1024. / 1024.;
        printf( ": <span %s>%3.1f MB</span>; ", ( bytesGeneral!=bytesPrevious ) ? BACKGROUND_YELLOW : BACKGROUND_WHITE, megabytes );
    }

    for (gfx = 0; gfx<NEXUS_NUM_DISPLAYS; gfx++)
    {
        /* if disabled, clear bytesGeneral so yellow highlighting works */
        if (pBoxModeSettings->graphics[gfx].used == false)
        {
            pOutput->surfaceSettings[gfx].bytesGeneral = 0;
        }
    }

    return( 0 );
} /* outputUsageGraphics */

static int outputUsageBoxModes(
    int                heapIdx,
    Memconfig_BoxMode *pBoxModeSettings
    )
{
    unsigned int idx = 0;

#if NEXUS_HAS_VIDEO_DECODER
    for (idx = 0; idx<NEXUS_NUM_VIDEO_DECODERS; idx++)
    {
        if (( pBoxModeSettings->videoDecoder[idx].pictureBufferHeapIdx == heapIdx ) && pBoxModeSettings->videoDecoder[idx].usage)
        {
            printf( "%s; ", pBoxModeSettings->videoDecoder[idx].usage );
        }
        if (( pBoxModeSettings->videoDecoder[idx].secondaryPictureBufferHeapIdx == heapIdx ) && pBoxModeSettings->videoDecoder[idx].secondaryUsage &&
            ( strcmp( pBoxModeSettings->videoDecoder[idx].usage, pBoxModeSettings->videoDecoder[idx].secondaryUsage ) != 0 ))
        {
            printf( "%s; ", pBoxModeSettings->videoDecoder[idx].secondaryUsage );
        }
    }
#endif /* NEXUS_HAS_VIDEO_DECODER*/

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (idx = 0; idx<NEXUS_NUM_DISPLAYS; idx++)
    {
        if (( pBoxModeSettings->display[idx].mainPictureBufferHeapIdx == heapIdx ) && pBoxModeSettings->display[idx].usageMain)
        {
            printf( "%s; ", pBoxModeSettings->display[idx].usageMain );
        }
        if (( pBoxModeSettings->display[idx].pipPictureBufferHeapIdx == heapIdx ) && pBoxModeSettings->display[idx].usagePip &&
            ( strcmp( pBoxModeSettings->display[idx].usageMain, pBoxModeSettings->display[idx].usagePip ) != 0 ))
        {
            printf( "%s; ", pBoxModeSettings->display[idx].usagePip );
        }
    }
#endif /* NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    return( 0 );
} /* outputUsageBoxmodes */

static char *useNbsp(
    const char *heapName
    )
{
    unsigned int spaceCount   = 0; /* used to compute size for malloc() */
    char        *pos          = (char *) heapName;
    char        *heapNameNbsp = NULL;
    int          newlen       = 0;

    if (heapName == NULL)
    {
        return( NULL );
    }
    while (*pos)
    {
        /* if next char is a space */
        if (*pos == ' ')
        {
            spaceCount++;
        }
        pos++;
    }

    newlen       = strlen( heapName ) + spaceCount * 6 /* &nbsp; */ + 1;
    heapNameNbsp = malloc( newlen );
    if (heapNameNbsp == NULL)
    {
        return( heapNameNbsp );
    }

    memset( heapNameNbsp, 0, newlen );

    /* if string does have at least one space in it */
    if (spaceCount > 0)
    {
        unsigned int newNameIdx = 0;
        pos = (char *) heapName;
        while (*pos)
        {
            /* if next char is a space */
            if (*pos == ' ')
            {
                strncat( &heapNameNbsp[newNameIdx], "&nbsp;", newlen );
                newNameIdx += strlen( "&nbsp;" );
            }
            else
            {
                heapNameNbsp[newNameIdx] = *pos;
                newNameIdx++;
            }
            pos++;
        }
    }
    else /* string does not have any space in it */
    {
        strncpy( heapNameNbsp, heapName, newlen );
    }

    return( heapNameNbsp );
} /* useNbsp */

static void print_heaps(
    Memconfig_AppMemUsage             *pOutput,
    Memconfig_AppUsageSettings        *pInput,
    Memconfig_BoxMode                 *pBoxModeSettings,
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppMemUsage             *pOutputPrevious,
    NEXUS_PlatformStatus              *pPlatformStatusPrevious,
    bool                               useNexusMemconfigDefaults
    )
{
    NEXUS_PlatformConfiguration     platformConfig;
    NEXUS_PlatformStatus            platformStatus;
    NEXUS_AudioModuleMemoryEstimate audioEstimate;
    NEXUS_AudioModuleUsageSettings  audioUsageSettingsSummation;
    unsigned           i;
    int                memc       = 0;
    unsigned int       codec      = 0;
    unsigned           heapIdx    = 0;
    unsigned           codecCount = 0;
    NEXUS_Error        rc;
    NEXUS_MemoryStatus status;
    NEXUS_HeapHandle   heap;
    unsigned int       mainTotalVdec      = 0, secureTotalVdec = 0;
    unsigned int       mainTotalVencPlat  = 0, secureTotalVencPlat = 0;
    unsigned int       mainTotalAudioPlat = 0;
    unsigned int       mainTotalDisplay   = 0;
    unsigned int       mainTotalRave      = 0, secureTotalRave = 0;
    unsigned int       mainTotalAudioXpt  = 0, secureTotalAudioXpt = 0;
    unsigned int       mainTotalTeletext  = 0;

    unsigned long int              memc_totals[NEXUS_NUM_MEMC];
    unsigned long int              systemTotal       = 0; /* megabytes of system memory */
    unsigned long int              heapTotal         = 0;
    long int                       linuxTotal        = 0;
    unsigned long int              appTotalGeneral   = 0;
    unsigned long int              appTotalSecure    = 0;
    unsigned long int              secureHeapTotalMb = 0; /* chips may or may not have secure heap */
    unsigned long int              graphicsTotal     = 0;
    unsigned long int              graphics3dTotal   = 0;
    float                          additionalMemory  = 0.;
    float                          tempf1            = 0;
    calculateUsageGraphicsSettings graphicsSettings;

    PRINTF( "~%s: top: previous->audioDecoder[0].general %u\n~", __FUNCTION__, pOutputPrevious->audioDecoder[0].bytesGeneral );
    NEXUS_Platform_GetStatus( &platformStatus );

    systemTotal = getSystemMemoryMB( &platformStatus );

    /* call NEXUS_Platform_GetConfiguration to get the heap handles */
    NEXUS_Platform_GetConfiguration( &platformConfig );

    memset( &heap_sizes, 0, sizeof( heap_sizes ));
    memset( &memc_totals, 0, sizeof( memc_totals ));
    memset( &heap_totals, 0, sizeof( heap_totals ));
    memset( &g_heap_info, 0, sizeof( g_heap_info ));
    memset( &audioEstimate, 0, sizeof( audioEstimate ));
    memset( &audioUsageSettingsSummation, 0, sizeof( audioUsageSettingsSummation ));

#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_MAX_MEMC; i++)
    {
        mainTotalVdec += platformStatus.estimatedMemory.memc[i].videoDecoder.general;
        PRINTF( "~memc %u: mainTotalVdec (%9u); += %9u<br>~", i, mainTotalVdec, platformStatus.estimatedMemory.memc[i].videoDecoder.general );
        secureTotalVdec += platformStatus.estimatedMemory.memc[i].videoDecoder.secure;
    }
    appTotalGeneral += mainTotalVdec;
    PRINTF( "~mainTotalVdec (%u); appTotalGeneral %lu<br>~", mainTotalVdec, appTotalGeneral );
    appTotalSecure += secureTotalVdec;
    tempf1          = secureTotalVdec / 1024./1024.;
    PRINTF( "%s: vdec appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, secureTotalVdec, appTotalSecure, tempf1 );
#endif /* if NEXUS_HAS_VIDEO_DECODER */
#if NEXUS_HAS_VIDEO_ENCODER
    for (i = 0; i<NEXUS_MAX_MEMC; i++)
    {
        mainTotalVencPlat   += platformStatus.estimatedMemory.memc[i].videoEncoder.general;
        secureTotalVencPlat += platformStatus.estimatedMemory.memc[i].videoEncoder.secure;
    }
    appTotalGeneral += mainTotalVencPlat;
    appTotalSecure  += secureTotalVencPlat;
    tempf1           = secureTotalVencPlat/ 1024./1024.;
    PRINTF( "%s: venc appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, secureTotalVencPlat, appTotalSecure, tempf1  );
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    /* merge all of the audio decoder selections into one structure to get memory estimates */
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        if (pInput->audioDecoder[i].enabled)
        {
            PRINTF( "~%s: adec %u, enabled %u; primary %u; secondary %u\n~", __FUNCTION__, i, pInput->audioDecoder[i].enabled, pInput->audioDecoder[i].bPassthru, pInput->audioDecoder[i].bSecondary );
            audioUsageSettingsSummation.numDecoders++; /* if decoder is enabled, increment count; increment again if secondary is set */
            if ((( i==0 ) && pInput->audioDecoder[i].bPassthru ))
            {
                audioUsageSettingsSummation.numPassthroughDecoders++;
            }
            if (( i==0 ) && pInput->audioDecoder[i].bSecondary)
            {
                audioUsageSettingsSummation.numDecoders++;
            }

            if (pSettings->audio.maxIndependentDelay > audioUsageSettingsSummation.maxIndependentDelay)
            {
                audioUsageSettingsSummation.maxIndependentDelay = pSettings->audio.maxIndependentDelay;
            }
            if (pSettings->audio.maxDecoderOutputChannels> audioUsageSettingsSummation.maxDecoderOutputChannels)
            {
                audioUsageSettingsSummation.maxDecoderOutputChannels = pSettings->audio.maxDecoderOutputChannels;
            }
            if (pSettings->audio.maxDecoderOutputSamplerate> audioUsageSettingsSummation.maxDecoderOutputSamplerate)
            {
                audioUsageSettingsSummation.maxDecoderOutputSamplerate = pSettings->audio.maxDecoderOutputSamplerate;
            }
            if (pSettings->audio.dolbyCodecVersion> audioUsageSettingsSummation.dolbyCodecVersion)
            {
                audioUsageSettingsSummation.dolbyCodecVersion = pSettings->audio.dolbyCodecVersion;
            }
            if (pSettings->audio.numHbrPassthroughDecoders> audioUsageSettingsSummation.numHbrPassthroughDecoders)
            {
                audioUsageSettingsSummation.numHbrPassthroughDecoders = pSettings->audio.numHbrPassthroughDecoders;
            }
            if (pSettings->audio.numDspMixers> audioUsageSettingsSummation.numDspMixers)
            {
                audioUsageSettingsSummation.numDspMixers = pSettings->audio.numDspMixers;
            }
            if (pSettings->audio.numEchoCancellers> audioUsageSettingsSummation.numEchoCancellers)
            {
                audioUsageSettingsSummation.numEchoCancellers = pSettings->audio.numEchoCancellers;
            }
            for (codec = 0; codec<NEXUS_AudioPostProcessing_eMax; codec++)
            {
                audioUsageSettingsSummation.postProcessingEnabled[codec] |= pSettings->audio.postProcessingEnabled[codec];
            }
            /* the decoder codec is saved in array[0]; the encoder codec is saved in array[1] */
            if (( i==0 ) || ( i==1 ))
            {
                for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
                {
                    /* only decoders other than Main is used for transcode */
                    if (i == 0)
                    {
                        audioUsageSettingsSummation.decodeCodecEnabled[codec] |= pSettings->audio.decodeCodecEnabled[codec];
                    }
                    else
                    {
                        audioUsageSettingsSummation.encodeCodecEnabled[codec] |= pSettings->audio.encodeCodecEnabled[codec];
                    }
                }
            }
        }
    }
    codecCount = 0;
    PRINTF( "~%s: audioSummation decodeCodecs: ", __FUNCTION__ );
    for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
    {
        PRINTF( "%u ", audioUsageSettingsSummation.decodeCodecEnabled[codec] );
        codecCount += audioUsageSettingsSummation.decodeCodecEnabled[codec];
    }
    PRINTF( " ... num %u\n~", codecCount );

    PRINTF( "~%s: audioSummation decodeDefalt: ", __FUNCTION__ );
    codecCount = 0;
    for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
    {
        PRINTF( "%u ", audioDecodeCodecEnabledDefaults[codec] );
        codecCount += audioDecodeCodecEnabledDefaults[codec];
    }
    PRINTF( " ... num %u\n~", codecCount );

    codecCount = 0;
    PRINTF( "%s: audioSummation encodeCodecs: ", __FUNCTION__ );
    for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
    {
        PRINTF( "%u ", audioUsageSettingsSummation.encodeCodecEnabled[codec] );
        codecCount += audioUsageSettingsSummation.encodeCodecEnabled[codec];
    }
    PRINTF( " ... num %u\n~", codecCount );

    PRINTF( "~%s: audioSummation encodeDefalt: ", __FUNCTION__ );
    codecCount = 0;
    for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
    {
        PRINTF( "%u ", audioEncodeCodecEnabledDefaults[codec] );
        codecCount += audioEncodeCodecEnabledDefaults[codec];
    }
    PRINTF( " ... num %u\n~", codecCount );

    audioUsageSettingsSummation.numPostProcessing = 0;
    PRINTF( "%s: audioSummation postProcessing: ", __FUNCTION__ );
    /* now compute how many postProcessing bits are enabled */
    for (codec = 0; codec<NEXUS_AudioPostProcessing_eMax; codec++)
    {
        PRINTF( "%u ", audioUsageSettingsSummation.postProcessingEnabled[codec] );
        audioUsageSettingsSummation.numPostProcessing += audioUsageSettingsSummation.postProcessingEnabled[codec];
    }
    PRINTF( " ... num %u\n~", audioUsageSettingsSummation.numPostProcessing );

#if NEXUS_HAS_VIDEO_ENCODER
    /* count the number of encoders being used */
    for (i = 0; i<NEXUS_MAX_VIDEO_ENCODERS; i++)
    {
        if (pSettings->videoEncoder[i].used)
        {
            audioUsageSettingsSummation.numEncoders++;
        }
    }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    PRINTF( "~%s: postProcessing %u; numDecoders %u; passThrus %u; numEncoders %u; \n", __FUNCTION__,
        audioUsageSettingsSummation.numPostProcessing, audioUsageSettingsSummation.numDecoders, audioUsageSettingsSummation.numPassthroughDecoders,
        audioUsageSettingsSummation.numEncoders );

    for (i = 0; i<NEXUS_MAX_MEMC; i++)
    {
        mainTotalAudioPlat += platformStatus.estimatedMemory.memc[i].audio.general;
        PRINTF( "~%s: mainTotalAudioPlat %u += platformStatus[%u] = %u\n~", __FUNCTION__, mainTotalAudioPlat, i, platformStatus.estimatedMemory.memc[i].audio.general );
    }
    appTotalGeneral += mainTotalAudioPlat;

    /*  display bytes are handled separately in the "Driver" section */
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    /* only include MEMC0 in the usage count for Main; MEMC1 and MEMC2 get apportioned with Driver heap */
    i                 = 0;
    mainTotalDisplay += platformStatus.estimatedMemory.memc[i].display.general;
    PRINTF( "%s: display[%u] %u; sum %u\n", __FUNCTION__, i, platformStatus.estimatedMemory.memc[i].display.general, mainTotalDisplay );
    appTotalGeneral += mainTotalDisplay;
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    appTotalGeneral += pOutput->record.bytesGeneral;
    appTotalGeneral += pOutput->playback.bytesGeneral;
    appTotalSecure  += pOutput->playback.bytesSecure;
    tempf1           = pOutput->playback.bytesSecure/ 1024./1024.;
    PRINTF( "%s: play appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, pOutput->playback.bytesSecure, appTotalSecure, tempf1  );
    appTotalGeneral += pOutput->message.bytesGeneral;
    appTotalSecure  += pOutput->message.bytesSecure;
    tempf1           = pOutput->message.bytesSecure/ 1024./1024.;
    PRINTF( "%s: mess appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, pOutput->message.bytesSecure, appTotalSecure, tempf1 );
    appTotalGeneral += pOutput->live.bytesGeneral;
    appTotalSecure  += pOutput->live.bytesSecure;
    tempf1           = pOutput->live.bytesSecure/ 1024./1024.;
    PRINTF( "%s: live appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, pOutput->live.bytesSecure, appTotalSecure, tempf1 );
    appTotalGeneral += pOutput->remux.bytesGeneral;
    appTotalSecure  += pOutput->remux.bytesSecure;
    tempf1           = pOutput->remux.bytesSecure/ 1024./1024.;
    PRINTF( "%s: remx appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, pOutput->remux.bytesSecure, appTotalSecure, tempf1  );

#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        mainTotalRave   += pOutput->videoDecoder[i].bytesGeneral;
        secureTotalRave += pOutput->videoDecoder[i].bytesSecure;
        pInput->videoDecoder[i].bSecure = ( pOutput->videoDecoder[i].bytesSecure/1024/1024 > 0 );
    }
    appTotalGeneral += mainTotalRave;
    appTotalSecure  += secureTotalRave;
    tempf1           = secureTotalRave/ 1024./1024.;
    PRINTF( "%s: rave appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, secureTotalRave, appTotalSecure, tempf1  );
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        mainTotalAudioXpt              += pOutput->audioDecoder[i].bytesGeneral;
        secureTotalAudioXpt            += pOutput->audioDecoder[i].bytesSecure;
        pInput->audioDecoder[i].bSecure = ( pOutput->audioDecoder[i].bytesSecure/1024/1024 > 0 );
        PRINTF( "~%s: audioDecoder[%u]: general %u; secure %u; total %u/%u\n~", __FUNCTION__, i, pOutput->audioDecoder[i].bytesGeneral, pOutput->audioDecoder[i].bytesSecure,
            mainTotalAudioXpt, secureTotalAudioXpt );
    }
    appTotalGeneral += mainTotalAudioXpt;
    appTotalSecure  += secureTotalAudioXpt;
    tempf1           = secureTotalAudioXpt/ 1024./1024.;
    PRINTF( "%s: axpt appTotalSecure += %8u ... %lu (%5.1f) \n", __FUNCTION__, secureTotalAudioXpt, appTotalSecure, tempf1  );
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_HAS_VIDEO_ENCODER
    appTotalGeneral += pOutput->encode.bytesGeneral;
    appTotalSecure  += 0;
#endif

    if (pInput->sidSettings.enabled)
    {
        appTotalGeneral += pOutput->sidSettings.bytesGeneral;
    }

    if (pBoxModeSettings->frontend.used)
    {
        appTotalGeneral += pBoxModeSettings->frontend.usageSizeBytes;
        PRINTF( "%s: appTotalGeneral += %u (%lu)\n", __FUNCTION__, pBoxModeSettings->frontend.usageSizeBytes, appTotalGeneral );
    }

    /* compute teletext usage */
    {
        unsigned int i;
        for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
        {
            if (( pSettings->display[i].maxFormat!= NEXUS_VideoFormat_eUnknown ) &&
                (( pBoxModeSettings->display[i].property==Memconfig_DisplayProperty_ePrimary ) ||
                 ( pBoxModeSettings->display[i].property==Memconfig_DisplayProperty_eSecondary )) &&
                ( pInput->surfaceSettings[i].teletext /* enabled */ ))
            {
                mainTotalTeletext += 1556;
            }
        }
        appTotalGeneral               += mainTotalTeletext;
        pOutput->teletext.bytesGeneral = mainTotalTeletext;
    }

    printf( "~HEAPS~<table cols=1 border=0><tr><th align=left valign=top ><table cols=%u border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >",
        NEXUS_NUM_MEMC+5 /* col0:heapNum, col1:description, col:Mapping, col:Additional, col:Usage + 1 column for each MEMC*/ );
    printf( "<tr><th align=center>#</th><th align=left >Heap&nbsp;Name</th>" );
    /* create a column for each MEMC (could be 1, 2, or 3) */
    for (i = 0; i<NEXUS_NUM_MEMC; i++)
    {
        printf( "<th>MEMC&nbsp;%u</th>", i );
    }
    printf( "<th align=left ><a href=# id=pageHelp2 onclick=\"MyClick(event);\" >Mapping</a></th>" /* this numberic tag should match bmemconfig_faq.txt */
            "<th align=left ><a href=# id=pageHelp501 onclick=\"MyClick(event);\" >Additional</a></th>" /* this numberic tag should match bmemconfig_faq.txt */
            "<th align=left >Usage</th></tr>" );

    {
        int numMemcs = 1;
        printf("<tr><td colspan=8>");
        for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
        {
            Memconfig_HeapInfo heapInfo;

            BKNI_Memset( &heapInfo, 0, sizeof( heapInfo ));

            heap = platformConfig.heap[heapIdx];
            /*printf("heapIdx:%d returned heap %p<br>\n", heapIdx, heap );*/
            if (!heap)
            {
                continue;
            }

            BKNI_Memset( &status, 0, sizeof( status ));
            rc = NEXUS_Heap_GetStatus( heap, &status );
            BDBG_ASSERT( !rc );
            if (status.memcIndex == 1 && numMemcs <= 1) { numMemcs = 2; }
            if (status.memcIndex == 2 && numMemcs <= 2) { numMemcs = 3; }
            PRINTF("heapIdx:%d memcIndex %d ... numMemcs %d<br>\n", heapIdx, status.memcIndex, numMemcs );
        }
        printf("</tr>\n");
    }

    for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
    {
        Memconfig_HeapInfo heapInfo;

        BKNI_Memset( &heapInfo, 0, sizeof( heapInfo ));

        heap = platformConfig.heap[heapIdx];
        if (!heap)
        {
            continue;
        }

        BKNI_Memset( &status, 0, sizeof( status ));
        rc = NEXUS_Heap_GetStatus( heap, &status );
        BDBG_ASSERT( !rc );

        /*printf("%s: got heap status for idx %u\n", __FUNCTION__, heapIdx );*/
        if (Memconfig_GetHeapInfo( heapIdx, &heapInfo ) == 0)
        {
            unsigned long int usageBytes   = 0;
            int               megabytes    = 0;
            char             *heapNameNbsp = useNbsp( heapInfo.heapName ); /* replace spaces with &nbsp; to prevent word wrap in narrow columns */

            /* start a new row for the next heap */
            printf( "<tr><td>%u</td>", heapIdx );
            if (isHeapMain( heapIdx ))
            {
                usageBytes  = appTotalGeneral;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data;
                printf( "<th align=left ><a href=# id=pageHelp502 onclick=\"MyClick(event);\" >%s</a></th>", heapNameNbsp ); /* numberic tag must match bmemconfig_faq.txt */
            }
            else if (isHeapDriver( heapIdx ))
            {
                usageBytes  = platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index;
                usageBytes += platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data;
                PRINTF( "%s: driver heap audioEstimate.memc[%u]=%u<br>\n", __FUNCTION__, heapInfo.memcIndex, audioEstimate.memc[heapInfo.memcIndex].general );
                usageBytes += audioEstimate.memc[heapInfo.memcIndex].general;
                printf( "<td align=left >%s</td>", heapNameNbsp );
            }
            else if (isHeapSecure( heapIdx ))
            {
                usageBytes = appTotalSecure;
                printf( "<th align=left ><a href=# id=pageHelp502 onclick=\"MyClick(event);\" >%s</a></th>", heapNameNbsp ); /* numberic tag must match bmemconfig_faq.txt */
            }
            else if (isHeapGraphics( heapIdx ))
            {
                graphics3dTotal = 0;
                calculateUsageGraphics( heapIdx, pInput, pOutput, pBoxModeSettings, &graphicsSettings );
                usageBytes = graphicsTotal = graphicsSettings.bytesGeneral[0] + graphicsSettings.bytesGeneral[1];
                PRINTF( "%s: heapIdx %u; gfx[0] %lu + gfx[1] %lu = %lu<br>\n", __FUNCTION__, heapIdx,
                    graphicsSettings.bytesGeneral[0], graphicsSettings.bytesGeneral[1], usageBytes );

                if (( heapIdx == pBoxModeSettings->graphics3d.heapIdx ) && ( pBoxModeSettings->graphics3d.used ) &&
                    ( pBoxModeSettings->graphics3d.usage != NULL ))
                {
                    graphics3dTotal = 16 * 1024 * 1024;
                }

                usageBytes += graphics3dTotal;
                printf( "<th align=left ><a href=# id=pageHelp504 onclick=\"MyClick(event);\" >%s</a></th>", heapNameNbsp ); /* numberic tag must match bmemconfig_faq.txt */
            }
            else if (isHeapPicture( heapIdx )) /* no padding entry box */
            {
                usageBytes = heap_totals[heapIdx];
                printf( "<td align=left >%s</td>", heapNameNbsp );
            }
            else if (isHeapSage( heapIdx ))
            {
                usageBytes = heap_totals[heapIdx];
                printf( "<th align=left ><a href=# id=pageHelp503 onclick=\"MyClick(event);\" >%s</a></th>", heapNameNbsp ); /* numberic tag must match bmemconfig_faq.txt */
            }
            else /* unknown */
            {
                usageBytes = heap_totals[heapIdx];
                printf( "<td align=left >%s</td>", heapNameNbsp );
            }
            PRINTF( "%s: heap %u: usageBytes %08lu; memc[%u].display %08u<br>\n", __FUNCTION__, heapIdx, usageBytes, heapInfo.memcIndex,
                platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general );

            if (heapNameNbsp)
            {
                free( heapNameNbsp );
            }
            /* output columns before this one ... could be no cols or 1 col */
            for (memc = 0; memc<heapInfo.memcIndex; memc++)
            {
                printf( "<td>&nbsp;</td>" );
            }

            PRINTF( "%s: useNexusMemconfigDefaults %u; heap size %u<br>\n", __FUNCTION__, useNexusMemconfigDefaults, status.size );
            if (useNexusMemconfigDefaults == true)
            {
                heap_padding[heapIdx] = ( status.size - usageBytes );
                PRINTF( "%s: useNexusMemconfigDefaults %u; true; heapsize %u; usage %lu; padding %lu<br>\n", __FUNCTION__, useNexusMemconfigDefaults,
                    status.size, usageBytes/1024/1024, heap_padding[heapIdx] );
            }

            /* output this column */
            megabytes = OutputHeapSize( memc, heapInfo.heapName, &platformConfig, heapIdx, heapInfo.memcIndex, usageBytes );

            memc_totals[memc] += megabytes;
            PRINTF( "%s: memc_totals[%u] %lu += %u<br>\n", __FUNCTION__, memc, memc_totals[memc], megabytes );
            heap_totals[heapIdx]     += megabytes;
            heap_sizes[heapIdx][memc] = megabytes;

            /* output columns after this one ... could be no cols or 1 col */
            for (memc = heapInfo.memcIndex+1; memc<NEXUS_NUM_MEMC; memc++)
            {
                printf( "<td>&nbsp;</td>" );
            }

            printf( "<td>%s</td>", heapInfo.heapMapping );
            if (g_heap_info[heapIdx].memoryType == NULL)
            {
                unsigned int typelen = strlen( heapInfo.heapMapping )+1;
                g_heap_info[heapIdx].memoryType = malloc( typelen );
                strncpy( g_heap_info[heapIdx].memoryType, heapInfo.heapMapping, typelen-1 );
                g_heap_info[heapIdx].memoryType[typelen-1] = '\0';
            }

            additionalMemory = heap_padding[heapIdx] * 1.0 / 1024./1024.;

            PRINTF( "%s: heapIdx %u; pOutput->teletext.bytesGeneral %u(%p)<br>\n", __FUNCTION__, heapIdx, pOutput->teletext.bytesGeneral,
                (void *) &pOutput->teletext.bytesGeneral );
            if (isHeapMain( heapIdx ))
            {
                /* padding column */
                printf( "<td ><input type=text id=heap%uaddl size=2 onchange=\"MyClick(event);\" value=%5.1f %s ></td>", heapIdx,
                    additionalMemory, ( additionalMemory<0 ) ? BACKGROUND_RED : BACKGROUND_WHITE );

                /* usage column */
                printf( "<td >" );
                /* there are 22 different usage: 4 from platform.estimated general; 3 platform.estimated secure; 9 memconfig gen; 6 memconfig secure */
                outputUsage( "Video Decoder", mainTotalVdec, pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.general );
                pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.general = mainTotalVdec;
                outputUsage( "Video Encoder", mainTotalVencPlat, pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.general  );
                pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.general = mainTotalVencPlat;
                outputUsage( "Audio Plat", mainTotalAudioPlat, pOutputPrevious->audioDecoder[0].bytesGeneral );
                pOutput->audioDecoder[0].bytesGeneral = mainTotalAudioPlat;
                if (heapInfo.memcIndex == 0)
                {
                    outputUsage( "Display", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].display.general );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].display.general =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general;

                    outputUsage( "Video Encoder Firmware", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware;

                    outputUsage( "Video Encoder Index", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index;

                    outputUsage( "Video Encoder Data", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data;
                }
                #if 1 /* RJ asked to add back in  (6/16/2014) */
                outputUsage( "Video Rave", mainTotalRave, pOutputPrevious->videoDecoder[0].bytesGeneral );
                pOutput->videoDecoder[0].bytesGeneral = mainTotalRave;
                #endif
                outputUsage( "Record", pOutput->record.bytesGeneral, pOutputPrevious->record.bytesGeneral  );
                outputUsage( "Playback", pOutput->playback.bytesGeneral, pOutputPrevious->playback.bytesGeneral   );
                outputUsage( "Message", pOutput->message.bytesGeneral, pOutputPrevious->message.bytesGeneral   );
                outputUsage( "Live", pOutput->live.bytesGeneral, pOutputPrevious->live.bytesGeneral   );
                outputUsage( "Remux", pOutput->remux.bytesGeneral, pOutputPrevious->remux.bytesGeneral   );
                outputUsage( "Encode", pOutput->encode.bytesGeneral, pOutputPrevious->encode.bytesGeneral );
                PRINTF( "out->vdec %u; out->encode %u; ", mainTotalRave, pOutput->encode.bytesGeneral );
                outputUsage( "Audio Rave", mainTotalAudioXpt, pOutputPrevious->audioDecoder[2].bytesGeneral );
                pOutput->audioDecoder[2].bytesGeneral = mainTotalAudioXpt;
                outputUsageGraphics( heapIdx, pInput, pOutput, pOutputPrevious, pBoxModeSettings );
                outputUsageBoxModes( heapIdx, pBoxModeSettings );
                outputUsage( pBoxModeSettings->frontend.usage, pBoxModeSettings->frontend.usageSizeBytes, pOutputPrevious->frontend.bytesGeneral );
                pOutput->frontend.bytesGeneral = pBoxModeSettings->frontend.usageSizeBytes;
                outputUsage( "SID ", pInput->sidSettings.enabled * pOutput->sidSettings.bytesGeneral, pOutputPrevious->sidSettings.bytesGeneral );
                pOutput->sidSettings.bytesGeneral = pInput->sidSettings.enabled * pOutput->sidSettings.bytesGeneral;

                outputUsage( "Teletext ", mainTotalTeletext, pOutputPrevious->teletext.bytesGeneral );
                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            else if (isHeapDriver( heapIdx ))
            {
                /* padding column */
                printf( "<td ><input type=text id=heap%uaddl size=2 onchange=\"MyClick(event);\" value=%5.1f ></td>", heapIdx, additionalMemory );

                /* usage column */
                printf( "<td>" );
                if (heapInfo.memcIndex > 0)
                {
                    outputUsage( "Display", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general,
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].display.general );
                    outputUsageBoxModes( heapIdx, pBoxModeSettings );

                    outputUsage( "Video Encoder Firmware", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.firmware;

                    outputUsage( "Video Encoder Index", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.index;

                    outputUsage( "Video Encoder Data", platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data,
                        pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data );
                    pPlatformStatusPrevious->estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data =
                        platformStatus.estimatedMemory.memc[heapInfo.memcIndex].videoEncoder.data;
                }
                else
                {
                    printf( "&nbsp;" );
                }
                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            else if (isHeapSecure( heapIdx ))
            {
                secureHeapTotalMb = heap_totals[heapIdx];
                /* padding column */
                printf( "<td ><input type=text id=heap%uaddl size=2 onchange=\"MyClick(event);\" value=%5.1f %s ></td>", heapIdx,
                    additionalMemory, ( additionalMemory<0 ) ? BACKGROUND_RED : BACKGROUND_WHITE );

                /* usage column */
                printf( "<td >" );
                outputUsage( "Video Decoder", secureTotalVdec, pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.secure );
                pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.secure = secureTotalVdec;
                outputUsage( "Video Encoder", secureTotalVencPlat, pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.secure );
                pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.secure = secureTotalVencPlat;

                outputUsage( "Playback", pOutput->playback.bytesSecure, pOutputPrevious->playback.bytesSecure  );
                outputUsage( "Message", pOutput->message.bytesSecure, pOutputPrevious->message.bytesSecure   );
                outputUsage( "Live", pOutput->live.bytesSecure, pOutputPrevious->live.bytesSecure   );
                outputUsage( "Remux", pOutput->remux.bytesSecure, pOutputPrevious->remux.bytesSecure   );
                outputUsage( "Video Rave", secureTotalRave, pOutputPrevious->videoDecoder[0].bytesSecure );
                pOutput->videoDecoder[0].bytesSecure = secureTotalRave;
                outputUsage( "Audio Rave", secureTotalAudioXpt, pOutputPrevious->audioDecoder[0].bytesSecure );
                pOutput->audioDecoder[0].bytesSecure = secureTotalAudioXpt;
                outputUsageGraphics( heapIdx, pInput, pOutput, pOutputPrevious, pBoxModeSettings );
                outputUsageBoxModes( heapIdx, pBoxModeSettings );

                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            else if (isHeapGraphics( heapIdx ))
            {
                /* padding column */
                printf( "<td ><input type=text id=heap%uaddl size=2 onchange=\"MyClick(event);\" value=%5.1f %s ></td>", heapIdx,
                    additionalMemory, ( additionalMemory<0 ) ? BACKGROUND_RED : BACKGROUND_WHITE );

                printf( "<td >" ); /* usage */
                outputUsageGraphics( heapIdx, pInput, pOutput, pOutputPrevious, pBoxModeSettings );
                outputUsage( pBoxModeSettings->graphics3d.usage, graphics3dTotal /* bytes */, pOutputPrevious->graphics3d.bytesGeneral );

                if (( heapIdx == pBoxModeSettings->graphics3d.heapIdx ) && ( pBoxModeSettings->graphics3d.used ) &&
                    ( pBoxModeSettings->graphics3d.usage!= NULL ))
                {
                    pOutput->graphics3d.bytesGeneral = graphics3dTotal;
                }
                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            else if (isHeapPicture( heapIdx )) /* no padding entry box */
            {
                /* padding */
                printf( "<td >&nbsp;</td>" );
                printf( "<td >" ); /* usage */
                outputUsageBoxModes( heapIdx, pBoxModeSettings );
                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            else /* Sage */
            {
                printf( "<td ><input type=text id=heap%uaddl size=2 onchange=\"MyClick(event);\" value=%5.1f ></td>", heapIdx, additionalMemory );
                printf( "<td >" ); /* usage */
                outputUsageBoxModes( heapIdx, pBoxModeSettings );
                outputTotal( usageBytes );
                printf( "</td >" ); /* usage */
            }
            printf( "</tr>\n" );
        }
        else
        {
            PRINTF( "<!-- %s: heap idx (%d) could not be found in heapInfo table-->\n", __FUNCTION__, heapIdx );
        }
    }

    /* once all of the heaps have been examined, we will know if we have secure heap or not */
    pInput->bSecure = ( secureHeapTotalMb > 0 );
    PRINTF( "%s: transInput->bSecure %u; totalMB %lu\n", __FUNCTION__, pInput->bSecure, secureHeapTotalMb );

#if 0
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pInput->videoDecoder[i].bSecure = pInput->bSecure;
        PRINTF( "%s: vdec[%u].bSecure %u\n", __FUNCTION__, i, pInput->videoDecoder[i].bSecure );
    }
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        pInput->audioDecoder[i].bSecure = pInput->bSecure;
        PRINTF( "%s: adec[%u].bSecure %u\n", __FUNCTION__, i, pInput->audioDecoder[i].bSecure );
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

    printf( "<tr><th>&nbsp;</th><th align=left>TOTALS</th>" );
    for (memc = 0; memc<NEXUS_NUM_MEMC; memc++)
    {
        if (memc_totals[memc] == 0)
        {
            printf( "<th>&nbsp;</th>" );
        }
        else
        {
            long int spare = ( platformStatus.memc[memc].size/1024/1024 ) - memc_totals[memc];
            heapTotal += memc_totals[memc];
            printf( "<th %s >%lu&nbsp;MB</th>", ( spare<=0 ) ? BACKGROUND_RED : BACKGROUND_WHITE, memc_totals[memc] );
        }
    }
    printf( "<th>&nbsp;</th><th>&nbsp;</th>" );
    printf( "</tr>\n" );
    printf( "</table><br>\n" );
    linuxTotal = systemTotal - heapTotal;
    printf( "SYSTEM TOTAL (%ld MB) minus HEAP TOTAL (%ld MB)&nbsp;=&nbsp;<a href=# id=pageHelp3 onclick=\"MyClick(event);\" >LINUX TOTAL</a> (<span %s>%ld MB</span>)", systemTotal, heapTotal,
        ( linuxTotal < 0 ) ? BACKGROUND_RED : BACKGROUND_WHITE, linuxTotal );
    printf( " </td>\n" );

    #if 0
    printf( "<tr><td align=left colspan=2 >" );
    printf( "Heap TOTAL: %ld MB&nbsp;&nbsp;SYSTEM TOTAL: <span>%ld MB</span>&nbsp;&nbsp;Linux TOTAL:%ld MB", heapTotal, systemTotal, systemTotal - heapTotal );
    #endif
    printf( "</tr>" );
    printf( "</table>\n" );

    printf( "~STBTIME~%s~\n", lHhMmSs());
} /* print_heaps */

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

static int getBoxModeMemc(
    int boxmode
    )
{
    unsigned int idx = 0;
    int          numMemc = 0;
    /* loop through global array to find the specified boxmode */
    for (idx = 0; idx < (sizeof(g_bmemconfig_box_info)/sizeof(g_bmemconfig_box_info[0])); idx++)
    {
        PRINTF("~%s: g_bmemconfig_box_info[%d].boxmode is %d ... comparing with %d~\n", __FUNCTION__, idx, g_bmemconfig_box_info[idx].boxmode, boxmode );
        if (g_bmemconfig_box_info[idx].boxmode == boxmode)
        {
            numMemc = g_bmemconfig_box_info[idx].numMemc;
            break;
        }
    }

    /* we did not find the specified box mode */
    return ( numMemc );
}

static int getBoxModeDropdown(
    char *buf,
    int   len,
    int   boxmodePlatform
    )

{
    int   boxmode   = 0;
    int   remaining = len;
    int   lNumberMemcs = 1;
    char *output    = buf;

    BKNI_Memset( buf, '\0', len );

    lNumberMemcs = getProductIdMemc();

    for (boxmode = 0; boxmode<1010; boxmode++)
    {
        int               written   = 0;
        Memconfig_BoxMode settings;

        /* attempt to do some error checking on the box mode */
        if (( boxmode>30 ) && ( boxmode<1000 ))
        {
            boxmode = 999;
            continue;
        }

        BKNI_Memset( &settings, 0, sizeof( settings ));
        if (Memconfig_GetBoxModeDefaultSettings( boxmode, &settings )== -1)
        {
#if ( NEXUS_MODE_proxy )
            if (boxmode == 0)
            {
                written = snprintf( output, remaining, "<option value=%d %s >BoxMode %d:%s</option>", boxmode,  ( boxmode == boxmodePlatform ) ? "selected" : "",
                        boxmode, "None selected  ( via env B_REFSW_BOXMODE )" );
                remaining -= written;
                output    += written;
            }
#else /* if ( NEXUS_MODE_proxy ) */
#endif /* if ( NEXUS_MODE_proxy ) */

            continue;
        }

        /* if this boxmode's memc count matches the one we are compiling for */
        if ( getBoxModeMemc( boxmode ) == lNumberMemcs )
        {
            written = snprintf( output, remaining, "<option value=%d %s >BoxMode %d:%s</option> ", boxmode,  ( boxmode == boxmodePlatform ) ? "selected" : "",
                    boxmode, settings.boxModeDescription );
        }

        if (written>=remaining)
        {
            remaining = 0;
            break;
        }
        /* if something was written to the output buffer, increment the count and pointer for next write */
        if (written)
        {
            remaining -= written;
            output    += written;
        }
    }

    return( len-remaining );
} /* getBoxModeDropdown */

static int page_home(
    void
    )
{
    int         boxmode      = 0;
    char       *fileContents = NULL;
    char        platform[10];
    struct stat statbuf;
    char        strImageName[32];

    memset( platform, 0, sizeof( platform ));
    printf( "~HOME~" );
    boxmode = ReadBoxMode();
    printf( "<span style=\"%s\" >General Settings</span><br><br>\n", TITLE_WIDTH_SIZE );

    snprintf( strImageName, sizeof( strImageName ), "boxmode%u.png", boxmode );
    if (lstat( strImageName, &statbuf ) == -1)
    {
        /* could not find image file */
        /*printf( "<h2>Could not find image %s</h2>", strImageName );*/
    }
    else
    {
        printf( "<img src=\"%s\" width=800px ><br>", strImageName );
    }

    PRINTF( "boxmode: %d<br>\n", boxmode );

    fileContents = getFileContents( "/proc/device-tree/bolt/rts" );
    if (fileContents)
    {
        printf( "rts: %s<br>\n", fileContents );
        free( fileContents );
    }

    fileContents = getFileContents( "/proc/device-tree/bolt/board" );
    if (fileContents)
    {
        printf( "board: %s<br>\n", fileContents );
        strncpy( platform, &fileContents[3], 5 ); /* skip the BCM part */
        free( fileContents );
    }

    fileContents = getFileContents( "/proc/device-tree/bolt/tag" );
    if (fileContents)
    {
        printf( "bolt version: %s dated ", fileContents );
        free( fileContents );
    }

    fileContents = getFileContents( "/proc/device-tree/bolt/date" );
    if (fileContents)
    {
        printf( "%s<br>\n", fileContents );
        free( fileContents );
    }

    printf( "~" );
    printf( "PLATFORM~%s~", getPlatform());
    printf( "PLATVER~%s~", getPlatformVersion());
    return( 0 );
} /* page_home */

static int page_transport(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
    printf( "~TRANSPORT~" );
    printf( "<span style=\"%s\" >Transport Settings</span>\n", TITLE_WIDTH_SIZE );
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 ><!-- maxFormat %u -->", pSettings->videoDecoder[0].maxFormat );

    printf( "<tr ><td align=left >Records</td>" );
    printf( "<td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
    printf( "<td style=\"width:150px\" nowrap align=right >numRecords&nbsp;(%d):</td><td style=\"width:50px;\" ><input type=text id=rec0numRecords size=2 onchange=\"MyClick(event);\" value=%d ></td><td>&nbsp;</td>",
        pInput->record.max, pInput->record.number );
    printf( "<td style=\"width:70px\" nowrap align=right >bitRate:</td><td style=\"width:50px;\" ><input type=text id=rec0bitRate size=2 onchange=\"MyClick(event);\" value=%d ></td><td>(Mbps)</td>",
        pInput->record.bitRate );
    printf( "<td style=\"width:100px\" nowrap align=right >latency:</td><td style=\"width:50px;\" ><input type=text id=rec0latency size=2 onchange=\"MyClick(event);\" value=%d ></td><td>(msec)</td>",
        pInput->record.latency );
    printf( "</table></td></tr>\n" );

    printf( "<tr ><td align=left >Playbacks</td>" );
    printf( "<td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
    printf( "<td style=\"width:150px\" nowrap align=right >numPlaybacks&nbsp;(%d):</td><td style=\"width:50px;\" ><input type=text id=pb0numPlaybacks size=2 onchange=\"MyClick(event);\" value=%d ></td><td>&nbsp;</td>",
        pInput->playback.max, pInput->playback.number );
    printf( "<td style=\"width:70px\" nowrap align=right >size:</td><td style=\"width:50px;\" ><input type=text id=pb0size size=6 onchange=\"MyClick(event);\" value=%d ></td><td>(bytes)</td>",
        pInput->playback.size );
    printf( "</table></td></tr>\n" );

    printf( "<tr ><td align=left >Messages</td>" );
    printf( "<td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
    printf( "<td style=\"width:150px\" nowrap align=right >numMessages&nbsp;(%d):</td><td style=\"width:50px;\" ><input type=text id=msg0numMessages size=2 onchange=\"MyClick(event);\" value=%d ></td><td>&nbsp;</td>",
        pInput->message.max, pInput->message.number );
    printf( "<td style=\"width:70px\" nowrap align=right >size:</td><td style=\"width:50px;\" ><input type=text id=msg0size size=2 onchange=\"MyClick(event);\" value=%d ></td><td>(bytes)</td>",
        pInput->message.size );
    printf( "</table></td></tr>\n" );

    printf( "<tr ><td align=left >Live</td>" );
    printf( "<td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
    printf( "<td style=\"width:150px\" nowrap align=right >numLiveChannels&nbsp;(%d):</td><td style=\"width:50px;\" ><input type=text id=live0numLiveChannels size=2 onchange=\"MyClick(event);\" value=%d ></td><td>&nbsp;</td>",
        pInput->live.max, pInput->live.number );
    printf( "<td style=\"width:70px\" nowrap align=right >size:</td><td style=\"width:50px;\" ><input type=text id=live0size size=6 onchange=\"MyClick(event);\" value=%d title=\"This value is presented for informational purposes only; it is READONLY. It includes IB/Demod/Remux.\" disabled ></td><td>(bytes)</td>",
        pInput->live.size );
    printf( "</table></td></tr>\n" );

    printf( "<tr ><td align=left >Remux&nbsp;Loopback</td>" );
    printf( "<td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
    printf( "<td style=\"width:150px\" nowrap align=right >numRemuxes&nbsp;(%d):</td><td style=\"width:50px;\" ><input type=text id=rmux0numRemuxes size=2 onchange=\"MyClick(event);\" value=%d title=\"Remux only requires additional memory if it loops around to the input band.\" ></td><td>&nbsp;</td>",
        pInput->remux.max, pInput->remux.number );
    printf( "<td style=\"width:70px\" nowrap align=right >size:</td><td style=\"width:50px;\" ><input type=text id=rmux0size size=6 onchange=\"MyClick(event);\" value=%d disabled title=\"The Remux size is always the same as the Live size.\" ></td><td>(bytes)</td>",
        pInput->live.size /* remux size is always same as live.size */ );
    printf( "</table></td></tr>\n" );

    printf( "</table>\n" );

    return( 0 );
} /* page_transport */

#define SIZE_HTML_VIDEO_CODECS ( 8 *1024 )
#define VIDEO_CODECS_PER_LINE  8

static char *HtmlVideoCodecs(
    unsigned int                       whichDecIdx,
    unsigned int                       vdecOrSdec, /* 0 is vdec; 1 is sdec */
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    unsigned int idx          = 0;
    unsigned int displayCount = 0;
    char        *htmlBuffer   = malloc( SIZE_HTML_VIDEO_CODECS );
    char         whichDecStr[5];
    char         oneLine[1024];
    int          used_codecs[NEXUS_VideoCodec_eMax];

    memset( htmlBuffer, 0, SIZE_HTML_VIDEO_CODECS );
    memset( &used_codecs, 0, sizeof( used_codecs ));
    memset( &whichDecStr, 0, sizeof( whichDecStr ));

    if (vdecOrSdec==0)
    {
        strncpy( whichDecStr, "vdec", sizeof( whichDecStr ));
    }
    else
    {
        strncpy( whichDecStr, "sdec", sizeof( whichDecStr ));
    }
    snprintf( oneLine, sizeof( oneLine ), "<table cols=%d>\n", VIDEO_CODECS_PER_LINE );
    strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
    for (idx = 0; idx<( sizeof( g_videoCodecStrs )/sizeof( g_videoCodecStrs[0] )); idx++)
    {
        /* beginning of a row */
        if (( g_videoCodecStrs[idx].value > 0 ) && ( g_videoCodecStrs[idx].value!=NEXUS_VideoCodec_eH264_Svc ) &&
            ( g_videoCodecStrs[idx].value!=NEXUS_VideoCodec_eMotionJpeg ))
        {
            /* some enums have the same value as others; only include unique enums, and only include those codecs that were on by default */
            if (( used_codecs[g_videoCodecStrs[idx].value] == 0 ) &&
                ((( vdecOrSdec == 0 ) && videoDecodeCodecEnabledDefaults[whichDecIdx][g_videoCodecStrs[idx].value] ) ||
                 (( vdecOrSdec == 1 ) && stillDecodeCodecEnabledDefaults[whichDecIdx][g_videoCodecStrs[idx].value] )))
            {
                unsigned int checked = 0;
                if (vdecOrSdec == 0 /* creating for vdec */)
                {
                    checked = pSettings->videoDecoder[whichDecIdx].supportedCodecs[g_videoCodecStrs[idx].value];
                }
                else
                {
                    checked = pSettings->stillDecoder[whichDecIdx].supportedCodecs[g_videoCodecStrs[idx].value];
                }
                used_codecs[g_videoCodecStrs[idx].value] = g_videoCodecStrs[idx].value;
                if (displayCount%VIDEO_CODECS_PER_LINE == 0)
                {
                    strncat( htmlBuffer, "<tr>", SIZE_HTML_VIDEO_CODECS-1 );
                }

                sprintf( oneLine, "<td align=right style=\"width:100px;\" >%s&nbsp;<input type=checkbox id=%s%dcodec%03d onclick=\"MyClick(event);\" %s ></td>\n",
                    g_videoCodecStrs[idx].name, whichDecStr, whichDecIdx, g_videoCodecStrs[idx].value, ( checked ) ? "checked" : "" );
                strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
                displayCount++;
                if (displayCount%VIDEO_CODECS_PER_LINE ==0) {strncat( htmlBuffer, "</tr>\n", SIZE_HTML_VIDEO_CODECS-1 ); }
            }
        }
    }
    strncat( htmlBuffer, "</table>\n", SIZE_HTML_VIDEO_CODECS-1 );
    return( htmlBuffer );
} /* HtmlVideoCodecs */

static char *HtmlAudioCodecs(
    unsigned int                       whichDecIdx,
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    unsigned int idx          = 0;
    unsigned int displayCount = 0;
    char        *htmlBuffer   = malloc( SIZE_HTML_VIDEO_CODECS );
    char         oneLine[1024];
    int          used_codecs[NEXUS_AudioCodec_eMax];
    char         decodeOrEncode[5]; /* either adec or aenc */

    memset( htmlBuffer, 0, SIZE_HTML_VIDEO_CODECS );
    memset( &used_codecs, 0, sizeof( used_codecs ));

    snprintf( oneLine, sizeof( oneLine ), "<table cols=%d>\n", VIDEO_CODECS_PER_LINE );
    strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
    if (whichDecIdx == 0)
    {
        strncpy( decodeOrEncode, "adec", sizeof( decodeOrEncode ));
    }
    else
    {
        strncpy( decodeOrEncode, "aenc", sizeof( decodeOrEncode ));
    }

    for (idx = 0; idx<( sizeof( g_audioCodecStrs )/sizeof( g_audioCodecStrs[0] )); idx++)
    {
        /* beginning of a row */
        if (( g_audioCodecStrs[idx].value > 0 ))
        {
            PRINTF( "%s: audioStr[%u] = %u (%s); ", __FUNCTION__, idx, g_audioCodecStrs[idx].value, g_audioCodecStrs[idx].name );
            /* some enums have the same value as others; only include the first of duplicate enums. */
            if (( used_codecs[g_audioCodecStrs[idx].value] == 0 ) && ((( whichDecIdx == 0 ) && audioDecodeCodecEnabledDefaults[g_audioCodecStrs[idx].value] ) ||
                                                                      (( whichDecIdx == 1 ) && audioEncodeCodecEnabledDefaults[g_audioCodecStrs[idx].value] )))
            {
                unsigned int enabled = 0;

                PRINTF( "ADDED" );
                if (whichDecIdx == 0)
                {
                    enabled = pSettings->audio.decodeCodecEnabled[g_audioCodecStrs[idx].value];
                }
                else
                {
                    enabled = pSettings->audio.encodeCodecEnabled[g_audioCodecStrs[idx].value];
                }
                used_codecs[g_audioCodecStrs[idx].value] = g_audioCodecStrs[idx].value;
                if (displayCount%VIDEO_CODECS_PER_LINE == 0)
                {
                    strncat( htmlBuffer, "<tr>", SIZE_HTML_VIDEO_CODECS-1 );
                }

                sprintf( oneLine, "<td align=right style=\"width:110px;\" >%s&nbsp;<input type=checkbox id=%s%dcodec%03d onclick=\"MyClick(event);\" %s ></td>\n",
                    g_audioCodecStrs[idx].name, decodeOrEncode, whichDecIdx, g_audioCodecStrs[idx].value, ( enabled ) ? "checked" : "" );
                strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
                displayCount++;
                if (displayCount%VIDEO_CODECS_PER_LINE ==0) {strncat( htmlBuffer, "</tr>\n", SIZE_HTML_VIDEO_CODECS-1 ); }
            }
            PRINTF( "<br>\n" );
        }
    }
    strncat( htmlBuffer, "</table>\n", SIZE_HTML_VIDEO_CODECS-1 );
    return( htmlBuffer );
} /* HtmlAudioCodecs */

static char *HtmlAudioPostProcessing(
    unsigned int                       whichDecIdx,
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    unsigned int idx          = 0;
    unsigned int displayCount = 0;
    unsigned int enabled      = 0;
    char        *htmlBuffer   = malloc( SIZE_HTML_VIDEO_CODECS );
    char         oneLine[1024];
    int          used_codecs[NEXUS_AudioCodec_eMax];

    memset( htmlBuffer, 0, SIZE_HTML_VIDEO_CODECS );
    memset( &used_codecs, 0, sizeof( used_codecs ));

#define POST_PROCESSING_PER_LINE 4
    snprintf( oneLine, sizeof( oneLine ), "<table cols=%d>\n", POST_PROCESSING_PER_LINE );
    strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
    for (idx = 0; idx<NEXUS_AudioPostProcessing_eMax; idx++)
    {
        if (displayCount%POST_PROCESSING_PER_LINE == 0)
        {
            strncat( htmlBuffer, "<tr>", SIZE_HTML_VIDEO_CODECS-1 );
        }

        enabled = pSettings->audio.postProcessingEnabled[idx];
        sprintf( oneLine, "<td align=right style=\"width:180px;\" >%s&nbsp;<input type=checkbox id=adec%dpostpro%02d onclick=\"MyClick(event);\" %s ></td>\n",
            postProcessing[idx], whichDecIdx, idx, ( enabled ) ? "checked" : "" );
        strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_CODECS-1 );
        displayCount++;
        if (displayCount%POST_PROCESSING_PER_LINE ==0) {strncat( htmlBuffer, "</tr>\n", SIZE_HTML_VIDEO_CODECS-1 ); }
    }
    strncat( htmlBuffer, "</table>\n", SIZE_HTML_VIDEO_CODECS-1 );
    return( htmlBuffer );
} /* HtmlAudioPostProcessing */

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
} /* findWidthHeightHertz */

#define SIZE_HTML_VIDEO_FORMATS ( 8 *1024 )
#define VIDEO_FORMATS_PER_LINE  6
static char *HtmlVideoFormats(
    unsigned int disp,
    unsigned int maxResolution,
    int          currentFormat
    )
{
    int               debug      = 0;
    unsigned int      idx        = 0;
    char             *htmlBuffer = malloc( SIZE_HTML_VIDEO_FORMATS );
    char              oneLine[128];
    int               enumValues[NEXUS_VideoFormat_eMax];
    int               enumEnums[NEXUS_VideoFormat_eMax];
    char             *enumNames[NEXUS_VideoFormat_eMax];
    unsigned int      enumCount     = 0;
    static bool       printedValues = true; /* only print debug the first time through */
    int               used_formats[NEXUS_VideoFormat_eMax];
    unsigned          widthTmp       = 0, heightTmp = 0, hertzTmp = 0;
    unsigned long int thisResolution = 0;
    int               c, d;

    BSTD_UNUSED( disp );
    memset( htmlBuffer, 0, SIZE_HTML_VIDEO_FORMATS );
    memset( enumValues, 0, sizeof( enumValues ));
    memset( enumEnums, 0, sizeof( enumEnums ));
    memset( enumNames, 0, sizeof( enumNames ));
    memset( &used_formats, 0, sizeof( used_formats ));

    PRINTF( "%s: maxResolution %u<br>\n", __FUNCTION__, maxResolution );
    /* create an array of valid video format enums; it will be sorted right after creation */
    for (idx = 0; idx < ( sizeof( g_videoFormatStrs )/sizeof( g_videoFormatStrs[0] )); idx++)
    {
        PRINTF( "%s: findWidthHeightHertz for (%s)<br>\n", __FUNCTION__, g_videoFormatStrs[idx].name );
        widthTmp       = 0; heightTmp = 0; hertzTmp = 0;
        thisResolution = findWidthHeightHertz( g_videoFormatStrs[idx].name, &widthTmp, &heightTmp, &hertzTmp );
        PRINTF( "max: %u ... g_str[%u] is %u (%s); maxRes %lu<br>\n", maxResolution, idx, g_videoFormatStrs[idx].value, g_videoFormatStrs[idx].name,
            thisResolution );
        if (g_videoFormatStrs[idx].value && ( thisResolution <= maxResolution ) && ( used_formats[idx] == 0 ))
        {
            unsigned int j = 0;
            used_formats[idx] = g_videoFormatStrs[idx].value;
            if (debug) {printf( "adding new format (%u) to idx (%u)<br>\n", used_formats[idx], idx ); }
            if (thisResolution > 0)
            {
                if (printedValues==false)
                {
                    if (debug)
                    {
                        printf( "for %s (%u), scannedValue %d; count %d<br>\n", g_videoFormatStrs[idx].name,
                            g_videoFormatStrs[idx].value, heightTmp, enumCount );
                    }
                }

                enumValues[enumCount] = heightTmp;
                enumEnums[enumCount]  = g_videoFormatStrs[idx].value;
                enumNames[enumCount]  = (char *)g_videoFormatStrs[idx].name;
                enumCount++;
                if (debug) {printf( "enumValues[%d] is (%d) new (%d)<br>\n", j, enumValues[j], enumEnums[j] ); }
            }
        }
        else
        {
            if (debug) {printf( "skipped value %u; max %u<br>\n", g_videoFormatStrs[idx].value, maxResolution ); }
        }
    }
    if (debug) {printf( "sorting list<br>\n" ); }
    /* sort the list based on 480, 720, 1080, 2160, etc */
    {
        int   n = enumCount;
        int   swap1, swap2;
        char *swap3;
        for (c = 0; c < ( n - 1 ); c++)
        {
            for (d = 0; d < n - c - 1; d++)
            {
                if (enumValues[d] > enumValues[d+1]) /* For decreasing order use < */
                {
                    swap1           = enumValues[d];
                    swap2           = enumEnums[d];
                    swap3           = enumNames[d];
                    enumValues[d]   = enumValues[d+1];
                    enumEnums[d]    = enumEnums[d+1];
                    enumNames[d]    = enumNames[d+1];
                    enumValues[d+1] = swap1;
                    enumEnums[d+1]  = swap2;
                    enumNames[d+1]  = swap3;
                }
            }
        }
    }

    printedValues = true;
    for (idx = 0; idx<enumCount; idx++)
    {
        if (debug) {printf( "enumValues[%d] is (%d) (%d)<br>\n", idx, enumValues[idx], enumEnums[idx] ); }
    }

    sprintf( oneLine, "<option value=%d %s>%s</option>\n", 0, ( 0 == currentFormat ) ? "selected" : "", "Disabled" );
    strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_FORMATS-1 );
    for (idx = 0; idx<enumCount; idx++)
    {
        PRINTF( "%s: currentFormat %u (%s); loop[%u] format %u<br>\n", __FUNCTION__, currentFormat, lookup_name( g_videoFormatStrs, currentFormat ),
            idx, enumEnums[idx] );
        sprintf( oneLine, "<option value=%d %s>%s</option>\n", enumEnums[idx], ( enumEnums[idx] == currentFormat ) ? "selected" : "", enumNames[idx] );
        strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_FORMATS-1 );
    }
    sprintf( oneLine, "<!-- strlen(html for video formats == %d) -->\n", (int) strlen( htmlBuffer ));
    strncat( htmlBuffer, oneLine, SIZE_HTML_VIDEO_FORMATS-1 );
    return( htmlBuffer );
} /* HtmlVideoFormats */

static int print_vdecs(
    NEXUS_MemoryConfigurationSettings *pSettings,
    const Memconfig_BoxMode           *pBoxModeSettings,
    const char                        *description
    )
{
    #if 0
    unsigned int i;

    printf( "~%s: max %u\n~", description, maxVdecsEnabledByDefault );
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        printf( "~%s: vdec%u property %-9s; enabled %u\n~", description, i,  VideoDecoderPropertyStr[pBoxModeSettings->videoDecoder[i].property], pSettings->videoDecoder[i].used );
    }
    #else /* if 0 */
    BSTD_UNUSED( pSettings );
    BSTD_UNUSED( pBoxModeSettings );
    BSTD_UNUSED( description );
    #endif /* if 0 */

    return( 0 );
} /* print_vdecs */

static int page_video_decoder(
    Memconfig_BoxMode                 *pBoxModeSettings,
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
    unsigned int i                = sizeof( nexusVideoFormatIds )/sizeof( nexusVideoFormatIds[0] );
    char        *lHtmlVideoCodecs = NULL;
    int          rc               = 0;

    printf( "~%s: for boxmode %d; maxVdecsEnabledByDefault %u\n~", __FUNCTION__, pBoxModeSettings->boxModeId, maxVdecsEnabledByDefault );
    if (rc==-1)
    {
        printf( "~ALERT~Could not read BoxMode default settings~" );
    }

    PRINTF( "%s: numMosaic[0] %u\n", __FUNCTION__, pInput->videoDecoder[0].numMosaic );
    PRINTF( "%s: pSettings->mvc %u; pInput->bMvc %u\n", __FUNCTION__, pSettings->videoDecoder[0].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc],
        pInput->videoDecoder[0].bMvc );
    print_vdecs( pSettings, pBoxModeSettings, "top page_video_decoder" );

    printf( "~VIDEODEC~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Video Decoder Settings</span>\n", TITLE_WIDTH_SIZE );

    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 ><!-- maxVdecsEnabledByDefault %d -->", maxVdecsEnabledByDefault );

    /* output various video decoders */
    for (i = 0; i< maxVdecsEnabledByDefault; i++)
    {
        /* property zero is invalid; others are main, secondary, transcode */
        if (pBoxModeSettings->videoDecoder[i].property)
        {
            printf( "<tr bgcolor=lightgray ><td align=left >videoDecoder %d</td>", i );
            printf( "<td>used: <input type=checkbox id=vdec%dused onclick=\"MyClick(event);\" %s >&nbsp;&nbsp;(%s)</td></tr>\n", i,
                ( pSettings->videoDecoder[i].used ) ? "checked" : "", VideoDecoderPropertyStr[pBoxModeSettings->videoDecoder[i].property] );
            if (pSettings->videoDecoder[i].used)
            {
                lHtmlVideoCodecs = HtmlVideoCodecs( i, 0 /* prepare for vdec */, pSettings );

                /* video decoder x */
                printf( "<tr id=vdec%drow1 ><td align=right>supportedCodecs:</td><td>%s</td></tr>\n", i, lHtmlVideoCodecs );
                if (lHtmlVideoCodecs)
                {
                    free( lHtmlVideoCodecs );
                    lHtmlVideoCodecs = NULL;
                }
                printf( "<tr id=vdec%drow2 ><td align=right>mosaic:</td><td><table cols=3><tr>\n", i );
                printf( "<td style=\"width:150px\" nowrap >maxNumber (%u): <input type=text id=vdec%dmaxNumber size=2 onchange=\"MyClick(event);\" value=%d ></td>",
                    pSettings->videoDecoder[i].mosaic.maxNumber, i, pInput->videoDecoder[i].numMosaic );
                printf( "<td style=\"width:150px\" nowrap >maxWidth: <input type=text id=vdec%dmaxWidth size=2 onchange=\"MyClick(event);\" value=%d ></td>",
                    i, pSettings->videoDecoder[i].mosaic.maxWidth );
                printf( "<td style=\"width:150px\" nowrap >maxHeight: <input type=text id=vdec%dmaxHeight size=2 onchange=\"MyClick(event);\" value=%d ></td>",
                    i, pSettings->videoDecoder[i].mosaic.maxHeight );
                printf( "</tr></table></td></tr>\n" );
                printf( "<tr id=vdec%drow3 ><td align=right>&nbsp;</td><td><table cols=2><tr>\n", i );
                printf( "<td style=\"width:150px\" >avc51Supported: <input type=checkbox id=vdec%davc51Supported onclick=\"MyClick(event);\" %s ></td>",
                    i, ( pSettings->videoDecoder[i].avc51Supported ) ? "checked" : "" );
                printf( "<td style=\"width:150px\" >colorDepth: <input type=text id=vdec%dcolorDepth onchange=\"MyClick(event);\" value=%u size=2 ></td>",
                    i, pSettings->videoDecoder[i].colorDepth );
                printf( "</tr></table>\n" );
                printf( "</td></tr>\n" );
                printf( "<tr id=vdec%drow4 ><td align=right>maxFormat:(%u)</td><td><table cols=5 ><tr>\n", i,  maxVdecFormatResolutionByDefault[i] );
                printf( "<td style=\"width:150px\" >480i/p&nbsp;<input type=radio name=vdec%dformat id=vdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                    i, i, NEXUS_VideoFormat_e480p, ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e480p ) ? "checked" : "" );
                if (maxVdecFormatResolutionByDefault[i] >= 720)
                {
                    printf( "<td style=\"width:150px\" >720p&nbsp;<input type=radio name=vdec%dformat id=vdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                        i, i, NEXUS_VideoFormat_e720p, ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e720p ) ? "checked" : "" );
                }
                if (maxVdecFormatResolutionByDefault[i] >= 1080)
                {
                    printf( "<td style=\"width:150px\" >1080i/p&nbsp;<input type=radio name=vdec%dformat id=vdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                        i, i, NEXUS_VideoFormat_e1080p, ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e1080p ) ? "checked" : "" );
                }
                if (maxVdecFormatResolutionByDefault[i] >= 3840)
                {
                    printf( "<td style=\"width:150px\" >3840x2160p30&nbsp;<input type=radio name=vdec%dformat id=vdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                        i, i, NEXUS_VideoFormat_e3840x2160p30hz,
                        ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e3840x2160p30hz ) ? "checked" : "" );
                    printf( "<td style=\"width:150px\" >3840x2160p60&nbsp;<input type=radio name=vdec%dformat id=vdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                        i, i, NEXUS_VideoFormat_e3840x2160p60hz,
                        ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e3840x2160p60hz ) ? "checked" : "" );
                }
                printf( "</tr></table></td></tr>\n" );

                printf( "<tr id=vdec%drow5 ><td align=left>cdb/itb</td><td><table cols=7><tr>", i );
                printf( "<td style=\"width:80px\" >ip&nbsp;<input type=checkbox id=vdec%dip onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].bIp ) ? "checked" : "" );
                #if 0
                printf( "<td style=\"width:140px\" >3840x2160&nbsp;<input type=checkbox id=vdec%db3840x2160 onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].b3840x2160 ) ? "checked" : "" );
                printf( "<td style=\"width:100px\" >mosaic&nbsp;<input type=checkbox id=vdec%dnumMosaic onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].numMosaic>0 ) ? "checked" : "" );
                printf( "<td style=\"width:80px\" >soft&nbsp;<input type=checkbox id=vdec%dsoft onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].bSoft ) ? "checked" : "" );
                printf( "<td style=\"width:80px\" >mvc&nbsp;<input type=checkbox id=vdec%dmvc onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].bMvc ) ? "checked" : "" );
                #endif /* if 0 */
                printf( "<td style=\"width:100px\" >secure&nbsp;<input type=checkbox id=vdec%dsecure onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->videoDecoder[i].bSecure ) ? "checked" : "" );
                printf( "<td style=\"width:150px\" >numPrimers(fcc)&nbsp;<input type=text id=vdec%dfcc size=2 onchange=\"MyClick(event);\" value=%d ></td>", i,
                    pInput->videoDecoder[i].numFcc );
                printf( "</tr></table>\n" );

                printf( "</td></tr>\n"  );
            }
        }
    }

#if NEXUS_NUM_STILL_DECODES
    /* output various still decoders */
    for (i = 0; i< /*NEXUS_MAX_VIDEO_DECODERS*/ maxSdecsEnabledByDefault; i++)
    {
        printf( "<tr bgcolor=lightgray ><td align=left >stillDecoder %d</td>", i );
        printf( "<td>used: <input type=checkbox id=sdec%dused onclick=\"MyClick(event);\" %s ></td></tr>\n", i, ( pSettings->stillDecoder[i].used ) ? "checked" : "" );
        if (pSettings->stillDecoder[i].used)
        {
            lHtmlVideoCodecs = HtmlVideoCodecs( i, 1 /* prepare for sdec */, pSettings );

            /* video decoder x */
            printf( "<tr id=sdec%drow1 ><td align=right>supportedCodecs:</td><td>%s</td></tr>\n", i, lHtmlVideoCodecs );
            if (lHtmlVideoCodecs)
            {
                free( lHtmlVideoCodecs );
                lHtmlVideoCodecs = NULL;
            }
            printf( "<tr id=sdec%drow3 ><td align=right>&nbsp;</td><td><table cols=2><tr>\n", i );
            printf( "<td style=\"width:150px\" >avc51Supported: <input type=checkbox id=sdec%davc51Supported onclick=\"MyClick(event);\" %s ></td>",
                i, ( pSettings->stillDecoder[i].avc51Supported ) ? "checked" : "" );
            printf( "</tr></table>\n" );
            printf( "</td></tr>\n" );
            printf( "<tr id=sdec%drow4 ><td align=right>maxFormat:(%u)</td><td><table cols=5 ><tr>\n", i,  maxSdecFormatResolutionByDefault[i] );
            printf( "<td style=\"width:150px\" >480i/p&nbsp;<input type=radio name=sdec%dformat id=sdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                i, i, NEXUS_VideoFormat_e480p, ( pSettings->stillDecoder[i].maxFormat==NEXUS_VideoFormat_e480p ) ? "checked" : "" );
            if (maxSdecFormatResolutionByDefault[i] >= 720)
            {
                printf( "<td style=\"width:150px\" >720p&nbsp;<input type=radio name=sdec%dformat id=sdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                    i, i, NEXUS_VideoFormat_e720p, ( pSettings->stillDecoder[i].maxFormat==NEXUS_VideoFormat_e720p ) ? "checked" : "" );
            }
            if (maxSdecFormatResolutionByDefault[i] >= 1080)
            {
                printf( "<td style=\"width:150px\" >1080i/p&nbsp;<input type=radio name=sdec%dformat id=sdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                    i, i, NEXUS_VideoFormat_e1080p, ( pSettings->stillDecoder[i].maxFormat==NEXUS_VideoFormat_e1080p ) ? "checked" : "" );
            }
            if (maxSdecFormatResolutionByDefault[i] >= 3840)
            {
                printf( "<td style=\"width:150px\" >3840x2160p30&nbsp;<input type=radio name=sdec%dformat id=sdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                    i, i, NEXUS_VideoFormat_e3840x2160p30hz,
                    ( pSettings->stillDecoder[i].maxFormat==NEXUS_VideoFormat_e3840x2160p30hz ) ? "checked" : "" );
                printf( "<td style=\"width:150px\" >3840x2160p60&nbsp;<input type=radio name=sdec%dformat id=sdec%dformat value=%d onclick=\"MyClick(event);\" %s ></td>\n",
                    i, i, NEXUS_VideoFormat_e3840x2160p60hz,
                    ( pSettings->stillDecoder[i].maxFormat==NEXUS_VideoFormat_e3840x2160p60hz ) ? "checked" : "" );
            }
            printf( "</tr></table></td></tr>\n" );

            printf( "<tr id=sdec%drow5 ><td align=left>cdb/itb</td><td><table cols=7><tr>", i );
            printf( "<td style=\"width:80px\" >ip&nbsp;<input type=checkbox id=sdec%dip onclick=\"MyClick(event);\" %s ></td>", i,
                ( pInput->stillDecoder[i].bIp ) ? "checked" : "" );
            #if 0
            printf( "<td style=\"width:140px\" >3840x2160&nbsp;<input type=checkbox id=sdec%db3840x2160 onclick=\"MyClick(event);\" %s ></td>", i,
                ( pInput->stillDecoder[i].b3840x2160 ) ? "checked" : "" );
            printf( "<td style=\"width:80px\" >soft&nbsp;<input type=checkbox id=sdec%dsoft onclick=\"MyClick(event);\" %s ></td>", i,
                ( pInput->stillDecoder[i].bSoft ) ? "checked" : "" );
            printf( "<td style=\"width:80px\" >mvc&nbsp;<input type=checkbox id=sdec%dmvc onclick=\"MyClick(event);\" %s ></td>", i,
                ( pInput->stillDecoder[i].bMvc ) ? "checked" : "" );
            #endif /* if 0 */
            printf( "<td style=\"width:100px\" >secure&nbsp;<input type=checkbox id=sdec%dsecure onclick=\"MyClick(event);\" %s ></td>", i,
                ( pInput->stillDecoder[i].bSecure ) ? "checked" : "" );
            printf( "<td style=\"width:150px\" >numPrimers(fcc)&nbsp;<input type=text id=sdec%dfcc size=2 onchange=\"MyClick(event);\" value=%d ></td>", i,
                pInput->stillDecoder[i].numFcc );
            printf( "</tr></table>\n" );

            printf( "</td></tr>\n"  );
        }
    }
#else /* if NEXUS_NUM_STILL_DECODES */
#endif /* if NEXUS_NUM_STILL_DECODES */

    printf( "</table>\n" );

    return( 0 );
} /* page_video_decoder */

static int outputAudioDecoderHtml(
    unsigned int                       i,
    NEXUS_MemoryConfigurationSettings *pSettings,
    bool                               bDecodePage
    )
{
    unsigned int channel = 0;
    char        *lHtmlAudioCodecs         = NULL;
    char        *lHtmlAudioPostProcessing = NULL;

    lHtmlAudioCodecs = HtmlAudioCodecs( i, pSettings );

    /* decoder num */
    printf( "<tr id=adec%drow2 ><td align=left >", i );
    if (bDecodePage)
    {
        printf( "audio decodeCodecEnabled" );
    }
    else
    {
        printf( "audio encodeCodecEnabled" );
    }
    printf( ":</td><td>%s</td></tr>\n", lHtmlAudioCodecs );
    if (lHtmlAudioCodecs)
    {
        free( lHtmlAudioCodecs );
        lHtmlAudioCodecs = NULL;
    }

    /* do not include for transcodes */
    if (bDecodePage)
    {
        /* post processing row is NOT used for decoders that will be feeding encoder; only used for decoder Main */
        if (i==0)
        {
            lHtmlAudioPostProcessing = HtmlAudioPostProcessing( i, pSettings );
            printf( "<tr id=adec%drow3 ><td align=right>postProcessingEnabled:</td><td>%s</td></tr>\n", i, lHtmlAudioPostProcessing );
            if (lHtmlAudioPostProcessing)
            {
                free( lHtmlAudioPostProcessing );
                lHtmlAudioPostProcessing = NULL;
            }
        }
        printf( "<tr id=adec%drow4 ><td align=left>&nbsp;</td><td><table ><tr>", i );
        printf( "<td style=\"width:220px\" align=right >maxIndependentDelay&nbsp;<input type=text id=adec%dmaxIndependentDelay size=2 onchange=\"MyClick(event);\" value=%d "
                "title=\"Max independent output delay [0-500] milliseconds.\" ></td>", i, pSettings->audio.maxIndependentDelay );
        printf( "<td style=\"width:270px\" align=right >maxDecoderOutputSamplerate"
                "<select id=adec%dmaxDecoderOutputSamplerate onchange=\"MyClick(event);\" "
                "title=\"Max decoder output samplerate [48000, 96000] Hz\" >", i );
        for (channel = 0; channel<MEMCONFIG_NUM_AUDIO_SAMPLE_RATES; channel++)
        {
            printf( "<option value=%u ", audioSampleRates[channel] );
            if (audioSampleRates[channel] == pSettings->audio.maxDecoderOutputSamplerate) {printf( "SELECTED " ); }
            printf( ">%u</option>\n", audioSampleRates[channel] );
        }
        printf( "</select></td>\n" );

        printf( "<td style=\"width:270px\" align=right >maxDecoderOutputChannels&nbsp;"
                "<select id=adec%dmaxDecoderOutputChannels onchange=\"MyClick(event);\" "
                "title=\"Max decoder output channel count. specify 6 for 5.1 ch, 8 for 7.1 ch.\" >", i );
        for (channel = 0; channel<MEMCONFIG_NUM_AUDIO_CHANNEL_TYPES; channel++)
        {
            printf( "<option value=%u ", audioChannelInfo[channel].numOutputChannels );
            if (audioChannelInfo[channel].numOutputChannels == pSettings->audio.maxDecoderOutputChannels) {printf( "SELECTED " ); }
            printf( ">%s</option>\n", audioChannelInfo[channel].description );
        }
        printf( "</select></td>\n" );
        printf( "</tr></table></td></tr>\n" );

        /* this row is only used for decoder Main */
        if (i==0)
        {
            printf( "<tr id=adec%drow5 ><td align=left>&nbsp;</td><td><table ><tr>", i );
            printf( "<td style=\"width:220px\" align=right >numPassthroughDecoders&nbsp;<input type=text id=adec%dnumPassthroughDecoders size=2 onchange=\"MyClick(event);\" "
                    "value=%d title=\"Number of IEC-61937 passthrough that will be supported over SPDIF and/or HDMI.\" ></td>", i,  pSettings->audio.numPassthroughDecoders );
            printf( "<td style=\"width:270px\" align=right >numHbrPassthroughDecoders&nbsp;<input type=text id=adec%dnumHbrPassthroughDecoders size=3 onchange=\"MyClick(event);\" "
                    "value=%d title=\"Number of HBR passthroughs enabled (Dolby TrueHD and/or DTS HD MA passthrough over HDMI)\" ></td>", i,
                pSettings->audio.numHbrPassthroughDecoders );
            /* how many HDMI outputs chip has */
            printf( "<td style=\"width:270px\" align=right >numEchoCancellers&nbsp;<input type=text id=adec%dnumEchoCancellers size=2 value=%d disabled "
                    "title=\"Set to NEXUS_NUM_HDMI_OUTPUTS\" ></td>", i,  pSettings->audio.numEchoCancellers );
            printf( "</tr></table></td></tr>\n" );
        }

        printf( "<tr id=adec%drow6 ><td align=left>&nbsp;</td><td><table ><tr>", 0 );
        printf( "<td style=\"width:220px\" align=right >dolbyCodecVersion: <select id=adec%ddolbyCodecVersion onchange=\"MyClick(event);\" >", 0 );
        for (i = 0; i<NEXUS_AudioDolbyCodecVersion_eMax; i++)
        {
            printf( "<option value=%d ", i );
            if (pSettings->audio.dolbyCodecVersion == i)
            {
                printf( " selected" );
            }
            printf( ">%s</option>", dolbyCodecVersion[i] );
        }
        printf( "</select></td>" );
        printf( "</tr></table></td></tr>\n" );
    }

    return( 0 );
} /* outputAudioDecoderHtml */

/**
 *  Function: This function will return a 1 if one of the video decoders is used for the Main display; otherwise
 *  it will return a 0. For headless box modes, we need to know if one of the video decoders is used for the main
 *  display or not.
 **/
static unsigned int  IsVideoDecoderUsedForMain(
    Memconfig_BoxMode *pBoxModeSettings
    )
{
    unsigned int i;

    if (pBoxModeSettings == NULL)
    {
        return( 0 );
    }
#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        if (pBoxModeSettings->videoDecoder[i].property == Memconfig_VideoDecoderProperty_eMain)
        {
            return( 1 );
        }
    }

    return( 0 );

#endif /* if NEXUS_HAS_VIDEO_DECODER */
} /* IsVideoDecoderUsedForMain */

static int page_audio_decoder(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_BoxMode                 *pBoxModeSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
    unsigned int                    i       = 0;
    bool                            enabled = false;
    NEXUS_AudioModuleMemoryEstimate audioMemoryEstimate;
    char                            whichDecoder[14];
    unsigned int                    numEnabled              = 0;
    unsigned int                    videoDecoderUsedForMain = IsVideoDecoderUsedForMain( pBoxModeSettings ); /* 0 or 1 */

    memset( &audioMemoryEstimate, 0, sizeof( audioMemoryEstimate ));

    printf( "~AUDIODEC~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Audio Decoder Settings</span>\n", TITLE_WIDTH_SIZE );
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >" );

    printf( "<tr bgcolor=lightgray ><td align=left >generalSettings</td>" "<td>&nbsp;</td></tr>\n" );

    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        if (( i==0 ) &&  ( pInput->audioDecoder[i].enabled ))
        {
            numEnabled++;
        }
        else
        {
#if NEXUS_HAS_VIDEO_ENCODER
            if (pInput->audioDecoder[i].enabled)
            {
                numEnabled++;
            }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
        }
    }

    /* if at least one decoder is enabled, display the possible codecs */
    if (numEnabled > 0)
    {
        outputAudioDecoderHtml( 0, pSettings, true /* called from decoder page */ );
    }

    /* do for each audio decoder */
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        if (i==0)
        {
            enabled = pInput->audioDecoder[i].enabled;
        }
        else
        {
#if NEXUS_HAS_VIDEO_ENCODER
            enabled =  pInput->audioDecoder[i].enabled;
#else
            enabled =  false;
#endif
        }

        PRINTF( "%s: adec %u; enabled %u;<br>\n", __FUNCTION__, i, enabled );
        memset( whichDecoder, 0, sizeof( whichDecoder ));
        if (i==0)
        {
            sprintf( whichDecoder, VideoDecoderPropertyStr[pBoxModeSettings->videoDecoder[i].property] );
        }
        else
        {
            sprintf( whichDecoder, "Transcode" );
        }
        /* only show rows for each transcode plus 1 for the main decode if main display is being used. */
        if (i < ( maxNumTranscodesByDefault + videoDecoderUsedForMain ))
        {
            /* audio decoder is needed for Main Video and one for each transcode */
            printf( "<tr bgcolor=lightgray ><td align=left >audioDecoder %u</td>", i );
            printf( "<td>used: <input type=checkbox id=adec%dused onclick=\"MyClick(event);\" %s >&nbsp;&nbsp;(%s)", i, ( enabled ) ? "checked" : "", whichDecoder );
            printf( "</td></tr>\n" );
            if (enabled)
            {
                printf( "<tr><td>&nbsp;</td><td><table><tr>" );
                printf( "<td style=\"width:110px\" align=right >ip&nbsp;<input type=checkbox id=adec%dip onclick=\"MyClick(event);\" %s ></td>", i,
                    ( pInput->audioDecoder[i].bIp ) ? "checked" : "" );
                /* only Main audio decoder shoule have these elements (not for transcodes) */
                if (pBoxModeSettings->videoDecoder[i].property != Memconfig_VideoDecoderProperty_eTranscode)
                {
                    printf( "<td style=\"width:120px\" align=right >secondary&nbsp;<input type=checkbox id=adec%dsecondary onclick=\"MyClick(event);\" %s ></td>\n",
                        i, ( pInput->audioDecoder[i].bSecondary==true ) ? "checked" : "" );
                    printf( "<td style=\"width:120px\" align=right >passthrough&nbsp;<input type=checkbox id=adec%dpassthru onclick=\"MyClick(event);\" %s ></td>\n",
                        i, ( pInput->audioDecoder[i].bPassthru==true ) ? "checked" : "" );
                    printf( "<td style=\"width:160px\" align=right >numPrimers&nbsp;<input type=text id=adec%dfcc size=2 onchange=\"MyClick(event);\" value=%d ></td>", i,
                        pInput->audioDecoder[i].numFcc );
                }
                printf( "</tr></table></td></tr>\n" );
            }
        }
    }

    printf( "</table>\n" );

    return( 0 );
} /* page_audio_decoder */

static int page_display(
    Memconfig_BoxMode                 *pBoxModeSettings,
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
    unsigned int displayIdx, windowIdx;
    char        *lHtmlVideoFormats = NULL;

    printf( "~DISPLAY~\n" ); /* needs to match javascript */

    printf( "<span style=\"%s\" >Display Settings</span>\n", TITLE_WIDTH_SIZE );
#if NEXUS_HAS_DISPLAY
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 ><!-- NEXUS_NUM_VIDEO_WINDOWS %d; NEXUS_NUM_DISPLAYS %d -->",
        NEXUS_NUM_VIDEO_WINDOWS, NEXUS_NUM_DISPLAYS );
    for (displayIdx = 0; displayIdx<NEXUS_NUM_DISPLAYS; displayIdx++)
    {
        if (pBoxModeSettings->display[displayIdx].property)
        {
            PRINTF( "%s: max for disp[%d] is %u\n", __FUNCTION__, displayIdx, maxDisplayFormatByDefault[displayIdx] );
            lHtmlVideoFormats = HtmlVideoFormats( displayIdx, maxDisplayFormatByDefault[displayIdx], pSettings->display[displayIdx].maxFormat );

            printf( "<tr bgcolor=white ><td align=left >Display %d</td>", displayIdx );
            printf( "<td><table cols=4 border=0 ><tr>" );
            printf( "<td align=left  style=\"width:120px\" >maxFormat&nbsp;(%u):</td><td align=left style=\"width:260px\" >"
                    "<select id=disp%dmaxFormat onchange=\"MyClick(event);\" style=\"width:140px\" size=1 >%s</select>&nbsp;&nbsp;(%s)</td>",
                ( maxDisplayFormatByDefault[displayIdx]/100 )%10000, displayIdx, lHtmlVideoFormats,
                DisplayPropertyStr[pBoxModeSettings->display[displayIdx].property]   );
            if (lHtmlVideoFormats) {free( lHtmlVideoFormats ); lHtmlVideoFormats = NULL; }
            if (( pBoxModeSettings->display[displayIdx].property==Memconfig_DisplayProperty_ePrimary ) ||
                ( pBoxModeSettings->display[displayIdx].property==Memconfig_DisplayProperty_eSecondary ))
            {
                printf( "<td align=right ><input type=checkbox id=disp%dteletext onclick=\"MyClick(event);\" %s></td>"
                        "<td align=left >teletext</td>",
                    displayIdx, ( pInput->surfaceSettings[displayIdx].teletext ) ? "checked" : "" );
            }

            printf( "</tr></table></td></tr>\n" );

            if (pSettings->display[displayIdx].maxFormat)
            {
                for (windowIdx = 0; windowIdx<NEXUS_NUM_VIDEO_WINDOWS; windowIdx++)
                {
                    bool enabled = ( pSettings->display[displayIdx].window[windowIdx].used && pSettings->display[displayIdx].maxFormat > 0 );
                    printf( "<tr><td align=left >&nbsp;</td>" );
                    printf( "<td bgcolor=lightgray >Window %d used: <input type=checkbox id=disp%dwin%dused onclick=\"MyClick(event);\" %s></td></tr>\n",
                        windowIdx, displayIdx, windowIdx, ( enabled ) ? "checked" : "" );
                    if (enabled)
                    {
                        /* window x */
                        printf( "<tr><td>&nbsp;</td><td>" );
                        printf( "<table cols=3 style=\"border-collapse:collapse;\" border=0 >\n" );
                        printf( "<tr><td style=\"width:150;\"><input type=checkbox id=disp%dwin%ddeinterlacer %s onclick=\"MyClick(event);\" >deinterlacer</td>\n", displayIdx, windowIdx,
                            ( pSettings->display[displayIdx].window[windowIdx].deinterlacer ) ? "checked" : "" );
                        printf( "<td style=\"width:250;\"><input type=checkbox id=disp%dwin%dcapture %s onclick=\"MyClick(event);\" >capture</td>\n", displayIdx, windowIdx,
                            ( pSettings->display[displayIdx].window[windowIdx].capture ) ? "checked" : "" );
                        printf( "<td style=\"width:250;\"><input type=checkbox id=disp%dwin%dconvert %s onclick=\"MyClick(event);\" >"
                                "<span title=\"conversion between 50Hz and 60Hz inputs/outputs requires extra memory.\">convertAnyFrameRate</span></td>\n", displayIdx, windowIdx,
                            ( pSettings->display[displayIdx].window[windowIdx].convertAnyFrameRate ) ? "checked" : "" );
                        printf( "</tr><tr>\n" );
                        printf( "<tr><td style=\"width:150;\"><input type=checkbox id=disp%dwin%dlipsync %s onclick=\"MyClick(event);\" ><span title=\"required if using SyncChannel\" >precisionLipSync</span></td>\n", displayIdx, windowIdx,
                            ( pSettings->display[displayIdx].window[windowIdx].precisionLipSync ) ? "checked" : "" );
                        printf( "<td style=\"width:250;\"><input type=text id=disp%dwin%dusercapture size=2 value=%u  onchange=\"MyClick(event);\" >&nbsp;userCaptureBufferCount</td>\n",
                            displayIdx, windowIdx, pSettings->display[displayIdx].window[windowIdx].userCaptureBufferCount );
                        printf( "<td style=\"width:250;\">&nbsp;</td>\n" );
                        printf( "</tr>\n" );
                        printf( "</table>\n" );
                        printf( "</td>\n" );
                        printf( "</tr>\n" );
                    }
                }
            }
        }
    }

    printf( "</table>\n" );
#else /* if NEXUS_HAS_DISPLAY */
    BSTD_UNUSED( pBoxModeSettings );
    BSTD_UNUSED( pSettings );
    BSTD_UNUSED( pInput );
    printf( "<h3>NEXUS_HAS_DISPLAY is not defined</h3>\n" );
#endif /* if NEXUS_HAS_DISPLAY */

    return( 0 );
} /* page_display */

static int page_graphics(
    Memconfig_BoxMode          *pBoxModeSettings,
    Memconfig_AppUsageSettings *pInput
    )
{
    unsigned int gfx       = 0;
    unsigned int gfxActive = 0; /* when array of active gfx are not contiguous, we cannot count on the gfx index to determine how many are active */

    PRINTF( "~%s: NEXUS_NUM_DISPLAYS %u; maxDefault %u\n~", __FUNCTION__, NEXUS_NUM_DISPLAYS, maxGraphicsEnabledByDefault );
    printf( "~GRAPHICS~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Graphics Settings</span>\n", TITLE_WIDTH_SIZE );
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >" );

    for (gfx = 0; gfx<NEXUS_NUM_DISPLAYS; gfx++)
    {
        /* only show graphics entries that were enabled by default */
        if (( gfxActive < maxGraphicsEnabledByDefault ) && pBoxModeSettings->graphics[gfx].property)
        {
            gfxActive++;

            printf( "<tr bgcolor=lightgray ><td align=left >Graphics %u</td>",  gfx );
            printf( "<td>used: <input type=checkbox id=gfx%dused onclick=\"MyClick(event);\" %s >(%s)</td></tr>\n",
                gfx, ( pBoxModeSettings->graphics[gfx].used ) ? "checked" : "",
                DisplayPropertyStr[pBoxModeSettings->graphics[gfx].property]   );

            if (pBoxModeSettings->graphics[gfx].used)
            {
                printf( "<td>&nbsp;</td><td><table cols=9 border=0 style=\"border-collapse:collapse;\" ><tr>" ); /* each entry has 3 cols: label left: entry box; label right */
                printf( "<td style=\"width:80px\" nowrap align=right >maxWidth:</td><td style=\"width:50px;\" ><input type=text id=gfx%dmaxWidth size=2 onchange=\"MyClick(event);\" value=%d ></td><td>&nbsp;</td>",
                    gfx, pInput->surfaceSettings[gfx].width );
                printf( "<td style=\"width:80px\" nowrap align=right >maxHeight:</td><td style=\"width:50px;\" ><input type=text id=gfx%dmaxHeight size=2 onchange=\"MyClick(event);\" value=%d ></td><td></td>",
                    gfx, pInput->surfaceSettings[gfx].height );
                printf( "<td style=\"width:100px\" nowrap align=right >numberOfFb:</td><td style=\"width:50px;\" ><input type=text id=gfx%dnumberOfFb size=2 onchange=\"MyClick(event);\" value=%d ></td><td></td>",
                    gfx, pInput->surfaceSettings[gfx].numFrameBufs );
                printf( "<td style=\"width:100px\" nowrap align=right >bitsPerPixel:</td><td style=\"width:50px;\" ><input type=text id=gfx%dbitsPerPixel size=2 onchange=\"MyClick(event);\" value=%d ></td><td></td>",
                    gfx, pInput->surfaceSettings[gfx].bits_per_pixel );
                printf( "</table></td></tr>\n" );
            }
        }
    }

    /* if graphics 3d is used on this chip */
    if (pBoxModeSettings->graphics3d.usage)
    {
        printf( "<tr bgcolor=lightgray ><td align=left >Graphics 3D</td>" );
        printf( "<td>used: <input type=checkbox id=g3d0used onclick=\"MyClick(event);\" %s ></td></tr>\n",
            ( pBoxModeSettings->graphics3d.used ) ? "checked" : "" );
    }

    printf( "</table>\n" );

    return( 0 );
} /* page_graphics */

static int page_encoder(
    Memconfig_BoxMode                 *pBoxModeSettings,
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
#if NEXUS_HAS_VIDEO_ENCODER
    int          i               = 0;
    bool         bAtLeastOneUsed = false;
    unsigned int numEnabled      = 0;
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BSTD_UNUSED( pBoxModeSettings );
    BSTD_UNUSED( pSettings );
    BSTD_UNUSED( pInput );
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    printf( "~ENCODER~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Encoder Settings</span>\n", TITLE_WIDTH_SIZE );

#if NEXUS_HAS_VIDEO_ENCODER
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >" );
    printf( "<tr bgcolor=lightgray ><td align=left >generalSettings</td><td>&nbsp;</td></tr>\n" );

    PRINTF( "<tr><td colspan=2>\n" );
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        int videoDecoderIdx = pBoxModeSettings->transcoders[i].videoDecoder;
        PRINTF( "trancoder->videoDecoder[%u] %d; used %d; audioDecoder[%u] %d; ", i, pBoxModeSettings->transcoders[i].videoDecoder,
            pSettings->videoEncoder[i].used, i, pInput->audioDecoder[i].enabled );
        PRINTF( "property[%u]: %d<br>", videoDecoderIdx, pBoxModeSettings->videoDecoder[videoDecoderIdx].property );
    }

    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        bool enabled = pSettings->videoEncoder[i].used && pInput->audioDecoder[i].enabled;

        if (enabled)
        {
            numEnabled++;
        }
        PRINTF( " videoEncoder[%u] %d; audioDecoder[%u] %d; enabled %d; num %d<br>", i, pSettings->videoEncoder[i].used, i, pInput->audioDecoder[i].enabled, enabled, numEnabled );
    }
    PRINTF( "</td></tr>" );

    /* if at least one encoder is enabled, display the possible codecs */
    if (numEnabled > 0)
    {
        outputAudioDecoderHtml( 1, pSettings, false ); /* adec 0 used for main/pip; adec1-6 are used for transcodes 0-5*/
    }

    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        int videoDecoderIdx = pBoxModeSettings->transcoders[i].videoDecoder;
        if (pSettings->videoEncoder[i].used && ( pBoxModeSettings->videoDecoder[videoDecoderIdx].property == Memconfig_VideoDecoderProperty_eTranscode ))
        {
            bool enabled = pSettings->videoEncoder[i].used && pInput->audioDecoder[i].enabled;
            bAtLeastOneUsed = true;

            printf( "<tr bgcolor=lightgray ><td align=left >Encoder %d</td>", i );
            printf( "<td>used: <input type=checkbox id=venc%dused onclick=\"MyClick(event);\" %s >(Uses 1 Video Decoder and 1 Audio Decoder)</td></tr>\n", i,
                ( enabled ) ? "checked" : "" );
            {
                if (enabled)
                {
                    printf( "<tr id=venc%drow1 ><td align=left >videoSettings</td><td><table ><tr>", i );
                    printf( "<td style=\"width:150px\" nowrap >maxWidth: <input type=text id=venc%dmaxWidth size=2 onchange=\"MyClick(event);\" value=%d ></td>",
                        i, pSettings->videoEncoder[i].maxWidth );
                    printf( "<td style=\"width:150px\" nowrap >maxHeight: <input type=text id=venc%dmaxHeight size=2 onchange=\"MyClick(event);\" value=%d ></td>",
                        i, pSettings->videoEncoder[i].maxHeight );
                    printf( "<td style=\"width:150px\" >interlaced&nbsp;<input type=checkbox id=venc%dinterlaced onclick=\"MyClick(event);\" %s ></td>", i,
                        ( pSettings->videoEncoder[i].interlaced ) ? "checked" : "" );
                    printf( "</tr></table></td></tr>" );

                    printf( "<tr id=venc%drow2 ><td align=left>cdb/itb</td><td><table cols=7><tr>", i );
                    printf( "<td style=\"width:120px\" >streamMux&nbsp;<input type=checkbox id=venc%dstreamMux onclick=\"MyClick(event);\" %s ></td>", i,
                        ( pInput->encoders[i].streamMux ) ? "checked" : "" );
                    printf( "</tr></table>\n" );

                    printf( "</td></tr>\n"  );
                }
            }
        }
    }
    printf( "</table>\n" );
    if (bAtLeastOneUsed == false)
    {
        printf( "<h3>None of the %u encoders are available</h3>\n", NEXUS_NUM_VIDEO_ENCODERS );
    }
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    BSTD_UNUSED( pSettings );
    printf( "<h3>NEXUS_HAS_VIDEO_ENCODER is not defined</h3>\n" );
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    return( 0 );
} /* page_encoder */

static int page_file_management(
    const char                        *whichButton,
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_BoxMode                 *pBoxModeSettings
    )
{
    unsigned heapIdx = 0;

    if (whichButton == NULL) {return( 0 ); }

    printf( "~FILES~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >File Management</span>\n", TITLE_WIDTH_SIZE );
    printf( "<table cols=1>" );
    printf( "<tr><td><form method=\"get\" action=\"/cgi/bmemconfig.cgi\" ><table cols=4><tr>\n" );
    printf( "<td><input type=button value=GenerateCcode id=generateCcode onclick=\"MyClick(event);\" ><input type=hidden name=variable value=generateCcode > </td>" );
    printf( "<td><input type=button value=\"Get Log Files\" id=getLogFiles onclick=\"MyClick(event);\" ></td>" );
    printf( "</tr></table></form></td></tr>\n" );

    printf( "<!-- whichButton (%s) -->\n", whichButton ); fflush( stdout ); fflush( stderr );
    if (strstr( whichButton, "getLogFiles" ))
    {
        char *fileContents = NULL;
        char  logFilename[LOG_FILE_FULL_PATH_LEN];

        PrependTempDirectory( logFilename, sizeof( logFilename ), "boa_error.log" );

        fileContents = getFileContents( logFilename );
        if (fileContents)
        {
            printf( "<tr><td><h3>Console Log</h3><textarea id=consolelog cols=120 rows=20 >%s</textarea></td></tr>\n", fileContents );
            free( fileContents );
        }

        PrependTempDirectory( logFilename, sizeof( logFilename ), "boa_access.log" );

        fileContents = getFileContents( logFilename );
        if (fileContents)
        {
            printf( "<tr><td><h3>Access Log</h3><textarea id=accesslog cols=120 rows=20 >%s</textarea></td></tr>\n", fileContents );
            free( fileContents );
        }
    }
    else if (strstr( whichButton, "saveState" ))
    {
        /* this case is handled in main(). it has to be processed before the Content-type header gets sent back */
    }
    else if (strstr( whichButton, "generateCcode" ))
    {
        NEXUS_PlatformConfiguration platformConfig;

        /* call NEXUS_Platform_GetConfiguration to get the heap handles */
        NEXUS_Platform_GetConfiguration( &platformConfig );

        /* this case is handled in main(). it has to be processed before the Content-type header gets sent back */
        {
            printf( "<tr><td><h3>C Code</h3><textarea id=ccode cols=120 rows=200 >\n" );
            printf( "/* Generated by bmemconfig.html on %s */\n", DateYyyyMmDdHhMmSs());
            printf( "/*     Platform: %s %s */\n", getPlatform(), getPlatformVersion());
            printf( "/*     Boxmode: %u, %s */\n", pBoxModeSettings->boxModeId, pBoxModeSettings->boxModeDescription );
            printf( "/*     File: %s */\n", __FILE__ );
            printf( "#include \"nexus_platform.h\"\n" );
            printf( "#include \"nexus_core_utils.h\"\n" );
            printf( "#include \"nexus_surface.h\"\n" );
            printf( "#include \"bstd.h\"\n" );
            printf( "#include \"bkni.h\"\n" );
            printf( "#include <stdlib.h>\n" );
            printf( "#include <stdio.h>\n" );
            printf( "#include <string.h>\n\n" );

            printf( "static void mem_usage( NEXUS_PlatformSettings *pPlatformSettings, NEXUS_MemoryConfigurationSettings *pMemConfigSettings ) {\n" );
#if NEXUS_HAS_VIDEO_DECODER
            {
                int vdec, codec;
                printf( "#if NEXUS_HAS_VIDEO_DECODER\n" );
                for (vdec = 0; vdec<NEXUS_NUM_VIDEO_DECODERS; vdec++)
                {
                    printf( "    pMemConfigSettings->videoDecoder[%u].used      = %u;\n", vdec, pSettings->videoDecoder[vdec].used );
                    printf( "    memset( pMemConfigSettings->videoDecoder[%u].supportedCodecs, 0, sizeof( pMemConfigSettings->videoDecoder[%u].supportedCodecs ));\n", vdec, vdec );
                    if (pSettings->videoDecoder[vdec].used)
                    {
                        printf( "    pMemConfigSettings->videoDecoder[%u].maxFormat = %u;", vdec, pSettings->videoDecoder[vdec].maxFormat );
                        if (pSettings->videoDecoder[vdec].maxFormat)
                        {
                            printf( " /* %s */", lookup_name( g_videoFormatStrs, pSettings->videoDecoder[vdec].maxFormat ));
                        }
                        printf( "\n" );
                        for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
                        {
                            if (pSettings->videoDecoder[vdec].supportedCodecs[codec])
                            {
                                printf( "    pMemConfigSettings->videoDecoder[%u].supportedCodecs[%-2u] = %u; /* %s */\n", vdec, codec,
                                    pSettings->videoDecoder[vdec].supportedCodecs[codec], lookup_name( g_videoCodecStrs, codec ));
                            }
                        }
                        printf( "    pMemConfigSettings->videoDecoder[%u].colorDepth = %u;\n", vdec, pSettings->videoDecoder[vdec].colorDepth );
                        printf( "    pMemConfigSettings->videoDecoder[%u].mosaic.maxNumber = %u;\n", vdec, pSettings->videoDecoder[vdec].mosaic.maxNumber );
                        printf( "    pMemConfigSettings->videoDecoder[%u].mosaic.maxWidth = %u;\n", vdec, pSettings->videoDecoder[vdec].mosaic.maxWidth );
                        printf( "    pMemConfigSettings->videoDecoder[%u].mosaic.maxHeight = %u;\n", vdec, pSettings->videoDecoder[vdec].mosaic.maxHeight );
                        printf( "    pMemConfigSettings->videoDecoder[%u].avc51Supported = %u;\n", vdec, pSettings->videoDecoder[vdec].avc51Supported );
                    }
                }
                printf( "#endif /* if NEXUS_HAS_VIDEO_DECODER */\n" );
                printf( "\n" );
            }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

#if NEXUS_NUM_STILL_DECODES
            {
                unsigned int sdec, codec;
                printf( "#if NEXUS_NUM_STILL_DECODES\n" );
                for (sdec = 0; sdec<maxSdecsEnabledByDefault; sdec++)
                {
                    printf( "    pMemConfigSettings->stillDecoder[%u].used      = %u;\n", sdec, pSettings->stillDecoder[sdec].used );
                    printf( "    memset( pMemConfigSettings->stillDecoder[%u].supportedCodecs, 0, sizeof( pMemConfigSettings->stillDecoder[%u].supportedCodecs ));\n", sdec, sdec );
                    if (pSettings->stillDecoder[sdec].used)
                    {
                        printf( "    pMemConfigSettings->stillDecoder[%u].maxFormat = %u;", sdec, pSettings->stillDecoder[sdec].maxFormat );
                        if (pSettings->stillDecoder[sdec].maxFormat)
                        {
                            printf( " /* %s */", lookup_name( g_videoFormatStrs, pSettings->stillDecoder[sdec].maxFormat ));
                        }
                        printf( "\n" );
                        if (pSettings->stillDecoder[sdec].maxFormat)
                        {
                        }
                        for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
                        {
                            if (pSettings->stillDecoder[sdec].supportedCodecs[codec])
                            {
                                printf( "    pMemConfigSettings->stillDecoder[%u].supportedCodecs[%-2u] = %u; /* %s */\n", sdec, codec,
                                    pSettings->stillDecoder[sdec].supportedCodecs[codec], lookup_name( g_videoCodecStrs, codec ));
                            }
                        }
                        printf( "    pMemConfigSettings->stillDecoder[%u].avc51Supported = %u;\n", sdec, pSettings->stillDecoder[sdec].avc51Supported );
                    }
                }
                printf( "#endif /* if NEXUS_NUM_STILL_DECODES*/\n" );
                printf( "\n" );
            }
#endif /* if NEXUS_NUM_STILL_DECODES*/

#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
            {
                unsigned disp, win;
                printf( "#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS\n" );
                for (disp = 0; disp<NEXUS_NUM_DISPLAYS; disp++)
                {
                    /* if the display is enabled in this boxmode */
                    if (pBoxModeSettings->display[disp].property)
                    {
                        printf( "    pMemConfigSettings->display[%u].maxFormat = %u;", disp, pSettings->display[disp].maxFormat );
                        if (pSettings->display[disp].maxFormat)
                        {
                            printf( " /* %s */", lookup_name( g_videoFormatStrs, pSettings->display[disp].maxFormat ));
                        }
                        printf( "\n" );
                        /* if the display is enabled */
                        if (pSettings->display[disp].maxFormat != NEXUS_VideoFormat_eUnknown)
                        {
                            for (win = 0; win<NEXUS_NUM_VIDEO_WINDOWS; win++)
                            {
                                printf( "    pMemConfigSettings->display[%u].window[%u].used = %u;\n", disp, win, pSettings->display[disp].window[win].used );
                                if (pSettings->display[disp].window[win].used)
                                {
                                    printf( "    pMemConfigSettings->display[%u].window[%u].convertAnyFrameRate = %u;\n", disp, win,
                                        pSettings->display[disp].window[win].convertAnyFrameRate );
                                    printf( "    pMemConfigSettings->display[%u].window[%u].precisionLipSync    = %u;\n", disp, win,
                                        pSettings->display[disp].window[win].precisionLipSync );
                                    printf( "    pMemConfigSettings->display[%u].window[%u].capture             = %u;\n", disp, win,
                                        pSettings->display[disp].window[win].capture );
                                    printf( "    pMemConfigSettings->display[%u].window[%u].deinterlacer        = %u;\n", disp, win,
                                        pSettings->display[disp].window[win].deinterlacer );
                                    printf( "    pMemConfigSettings->display[%u].window[%u].userCaptureBufferCount = %u;\n", disp, win,
                                        pSettings->display[disp].window[win].userCaptureBufferCount );
                                }
                            }
                        }
                    }
                    else
                    {
                        printf( "    /* display[%u] is disabled in boxmodes.c */\n", disp );
                    }
                }
                printf( "#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */\n" );
                printf( "\n" );
            }
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */
#if NEXUS_HAS_VIDEO_ENCODER
            {
                int venc;
                printf( "#if NEXUS_HAS_VIDEO_ENCODER\n" );
                for (venc = 0; venc<NEXUS_NUM_VIDEO_ENCODERS; venc++)
                {
                    printf( "    pMemConfigSettings->videoEncoder[%u].used = %u;\n", venc, pSettings->videoEncoder[venc].used );
                }
                printf( "#endif /* if NEXUS_HAS_VIDEO_ENCODER */\n" );
                printf( "\n" );
            }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
            {
                printf( "    pMemConfigSettings->videoInputs.hdDvi = %u;\n", pSettings->videoInputs.hdDvi );
                printf( "    pMemConfigSettings->videoInputs.ccir656 = %u;\n", pSettings->videoInputs.ccir656 );
                printf( "\n" );
            }

            for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
            {
                if (g_heap_info[heapIdx].megabytes)
                {
                    NEXUS_Error        rc;
                    NEXUS_MemoryStatus status;
                    NEXUS_HeapHandle   heap;
                    Memconfig_HeapInfo heapInfo;

                    memset( &heapInfo, 0, sizeof( heapInfo ));
                    Memconfig_GetHeapInfo( heapIdx, &heapInfo );

                    heap = platformConfig.heap[heapIdx];
                    if (!heap) {continue; }
                    rc = NEXUS_Heap_GetStatus( heap, &status );
                    BDBG_ASSERT( !rc );

                    printf( "    pPlatformSettings->heap[%u].size = %u*1024*1024; /* %s */\n", heapIdx, ( isHeapPicture( heapIdx )) ? 0 : g_heap_info[heapIdx].megabytes,
                        heapInfo.heapName );
                    printf( "\n" );
                }
            }

            printf( "} /* mem_usage */\n" );

            printf( "int main(void)\n" );
            printf( "{\n" );
            printf( "    NEXUS_PlatformSettings platformSettings;\n" );
            printf( "    NEXUS_MemoryConfigurationSettings memConfigSettings;\n" );
            printf( "    int rc;\n\n" );
            printf( "    NEXUS_Platform_GetDefaultSettings(&platformSettings);\n" );
            printf( "    platformSettings.openFrontend = false;\n\n" );
            printf( "    NEXUS_GetDefaultMemoryConfigurationSettings(&memConfigSettings);\n\n" );
            printf( "    mem_usage(&platformSettings, &memConfigSettings);\n\n" );
            printf( "    NEXUS_SetEnv(\"NEXUS_BASE_ONLY_INIT\",\"y\");\n" );
            printf( "    rc = NEXUS_Platform_MemConfigInit(&platformSettings, &memConfigSettings);\n" );
            printf( "    BDBG_ASSERT(!rc);\n\n" );
            printf( "    NEXUS_Platform_Uninit();\n\n" );
            printf( "    return 0;\n" );
            printf( "}" );

            printf( "~" );
        }
    }
    printf( "</table><!-- %s done -->", __FUNCTION__ );

    return( 0 );
} /* page_file_management */

static int page_picture_decoder(
    Memconfig_AppUsageSettings *pInput
    )
{
    printf( "~PICDECODER~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Picture Decoder (SID) Settings</span>\n", TITLE_WIDTH_SIZE );
#if NEXUS_NUM_STILL_DECODES
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >" );
    printf( "<tr bgcolor=lightgray ><td align=left >SID</td>" );
    printf( "<td><table cols=4><tr>" );
    printf( "<td align=right >enabled:</td><td align=left style=\"width:100px\" ><input type=checkbox id=sid0enabled onclick=\"MyClick(event);\" %s ></td>",
        ( pInput->sidSettings.enabled ) ? "checked" : "" );
    #if 0
    /* 2014-06-24: daily call ... remove it */
    printf( "<td align=right >maxChannels:</td><td align=left style=\"width:100px\" ><input type=text readonly disabled size=2 value=%u></td>\n",
        ( pInput->sidSettings.enabled ) ?  pInput->sidSettings.maxChannels : 0 );
    #endif
    printf( "</tr></table></td></tr>" );
    printf( "</table>" );
#else /* if NEXUS_NUM_STILL_DECODES */
    BSTD_UNUSED( pInput );
    printf( "<h3>NEXUS_NUM_STILL_DECODES is not defined.</h3>\n" );
#endif /* if NEXUS_NUM_STILL_DECODES */

    return( 0 );
} /* page_picture_decoder */

static int page_video_input(
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    printf( "~VIDEOINPUT~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Video Input Settings</span>\n", TITLE_WIDTH_SIZE );
    printf( "<table cols=2 border=\"1\" style=\"border-collapse:collapse;\" cellpadding=5 >" );
    printf( "<tr bgcolor=lightgray ><td align=left >Video Inputs</td>" );
    printf( "<td><table cols=4><tr>" );
    printf( "<td align=right >hdDvi (HDMI rx):</td><td align=left style=\"width:100px\" ><input type=checkbox id=vinp0hdDvi onclick=\"MyClick(event);\" %s ></td>",
        ( pSettings->videoInputs.hdDvi ) ? "checked" : "" );
    printf( "<td align=right >ccir656:</td><td align=left style=\"width:100px\" ><input type=checkbox id=vinp0ccir656 onclick=\"MyClick(event);\" %s ></td>",
        ( pSettings->videoInputs.ccir656 ) ? "checked" : "" );
    printf( "</tr></table></td></tr>" );
    printf( "</table>" );

    return( 0 );
} /* page_picture_decoder */

/**
 *  Function: This function parses the file bmemconfig_faq.txt and converts the text file to a numbered HTML
 *  list. The file is expected to be in a format similar to this:
 *      <question id=1 >What is the "Additional" column used for?</question>
 *      <answer>The "Additional" column is ...</answer>
 **/
static int page_help(
    unsigned int helpnum
    )
{
    struct stat statbuf;

    printf( "~HELP~" ); /* needs to match javascript */
    printf( "<span style=\"%s\" >Frequently Asked Questions</span>\n", TITLE_WIDTH_SIZE );

#define FAQ_FILE "bmemconfig_faq.txt"
    /* see if faq file exists */
    if (lstat( FAQ_FILE, &statbuf ) == -1)
    {
        /* could not find faq file */
        printf( "~ALERT~The FAQ text file (%s) could not be found!~", FAQ_FILE );
    }
    else
    {
        char *fileContents = NULL;

        fileContents = getFileContents( FAQ_FILE );
        if (fileContents)
        {
            char             *pos      = strstr( fileContents, "<question" );
            char             *posEnd   = NULL;
            unsigned long int faqCount = 53783000;

            printf( "<style type=\"text/css\" >\n.divgray {\nbackground:white;\nwidth:90%%;font-weight:normal;\n}\n</style>\n" );
            printf( "<ol type=1>\n" );
            /* repeat for each question in the text file */
            while (pos)
            {
                /* find the end of the question */
                posEnd = strstr( pos, "</question>" );
                if (posEnd)
                {
                    char             *posIdTag = NULL;
                    unsigned long int numIdTag = 0;

                    *posEnd = '\0';
                    posEnd++;
                    faqCount++;
                    /* see if there is an id= tag in the question */
                    posIdTag = strstr( pos, " id=" );
                    if (posIdTag)
                    {
                        posIdTag += strlen( " id=" );
                        sscanf( posIdTag, "%lu", &numIdTag );
                        PRINTF( "%s: id tag (%lu)<br>\n", __FUNCTION__, numIdTag );
                    }
                    else
                    {
                        numIdTag = faqCount;
                    }
                    /* find the end of the <question> tag */
                    pos = strstr( pos, ">" );
                    pos++;

                    PRINTF( "%s: found end of question (%s)<br>\n", __FUNCTION__, pos );
                    printf( "<li ><a href=# onclick=\"displayAnswer('answer%lu')\" >%s</a></li>\n", numIdTag, pos );

                    /* find the answer to this question */
                    pos = strstr( posEnd, "<answer>" );
                    if (pos)
                    {
                        /* find the end of the answer */
                        posEnd = strstr( pos, "</answer>" );
                        if (posEnd)
                        {
                            *posEnd = '\0';
                            posEnd++;
                            PRINTF( "%s: found end of answer (%s)\n", __FUNCTION__, pos );
                            printf( "<div class=divgray  id=\"answer%lu\" style=\"display:%s\" >%s</div>\n", numIdTag, ( numIdTag == helpnum ) ? "display" : "none", pos );

                            pos = posEnd;
                        }
                    }
                }
                pos = strstr( pos, "<question" );
            }

            free( fileContents );
        }
    }

    return( 0 );
} /* page_help */

static int read_boxmode_from_state_file(
    void
    )
{
    FILE *pFile = NULL;
    char  oneline[1024];
    int   boxmode = 0;

    /* if the IP address of the caller is unknown, don't bother trying to find a state file */
    if (strstr( state_filename, "unknown" ))
    {
        return( 0 );
    }

    pFile = fopen( state_filename, "r" );

    if (pFile == NULL)
    {
        printf( "%s: could not open (%s)\n", __FUNCTION__, state_filename );
        return( 0 );
    }

    PRINTF( "%s: successfully opened (%s)\n", __FUNCTION__, state_filename );
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "boxmode %d\n", &boxmode );
    fclose( pFile );

    return( boxmode );
} /* read_boxmode_from_state_file */

static int read_settings(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput,
    Memconfig_BoxMode                 *pBoxModeSettings,
    Memconfig_AppMemUsage             *pOutputPrevious,
    NEXUS_PlatformStatus              *pPlatformStatusPrevious
    )
{
    unsigned int i, j;
    int          debug   = 0;
    int          boxmode = 0;
    FILE        *pFile   = NULL;
    char         oneline[1024];
    unsigned int used  = 0, avc51Supported = 0;
    unsigned int value = 0;
    unsigned int numEntriesInFile       = 0;
    unsigned int audioCodecMax          = 0;
    unsigned int audioPostProcessingMax = 0;
    char        *pos                    = NULL;

    /* if the IP address of the caller is unknown, don't bother trying to find a state file */
    if (strstr( state_filename, "unknown" ))
    {
        return( 0 );
    }

    pFile = fopen( state_filename, "r" );

    if (pFile == NULL)
    {
        printf( "%s: could not open (%s)\n", __FUNCTION__, state_filename );

        pInput->surfaceSettings[0].teletext = 1;
        pInput->surfaceSettings[1].teletext = 1;

        PRINTF( "%s: surfaceSettings[0].teletext %d; surfaceSettings[0].teletext %d\n", __FUNCTION__, pInput->surfaceSettings[0].teletext,
            pInput->surfaceSettings[1].teletext );
        return( -1 );
    }

    PRINTF( "%s: successfully opened (%s)\n", __FUNCTION__, state_filename );
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "boxmode %d\n", &boxmode );

    /* if user is changing the boxmode */
    if (boxmode != ReadBoxMode())
    {
        PRINTF( "DEBUG DEBUG DEBUG: removing file (%s)\n", state_filename );
        remove( state_filename );
        return( 0 );
    }

    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "numvdec %u", &numEntriesInFile );
    PRINTF( "numEntries %u<br>\n", numEntriesInFile );
    for (i = 0; i<numEntriesInFile; i++)
    {
        fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_HAS_VIDEO_DECODER
        sscanf( oneline, VDEC_FORMAT, &i, &used, &pSettings->videoDecoder[i].maxFormat,
            &pSettings->videoDecoder[i].colorDepth, &pSettings->videoDecoder[i].mosaic.maxNumber, &pSettings->videoDecoder[i].mosaic.maxWidth,
            &pSettings->videoDecoder[i].mosaic.maxHeight, &avc51Supported );
        pSettings->videoDecoder[i].used           = used;
        pInput->videoDecoder[i].enabled           = used;
        pSettings->videoDecoder[i].avc51Supported = avc51Supported;
        pInput->videoDecoder[i].b3840x2160        = ( pSettings->videoDecoder[i].maxFormat== NEXUS_VideoFormat_e3840x2160p30hz ) ||
            ( pSettings->videoDecoder[i].maxFormat==NEXUS_VideoFormat_e3840x2160p60hz );
        pInput->videoDecoder[i].numMosaic = pSettings->videoDecoder[i].mosaic.maxNumber;
        PRINTF( VDEC_FORMAT, i, pSettings->videoDecoder[i].used, pSettings->videoDecoder[i].maxFormat,
            pSettings->videoDecoder[i].colorDepth, pSettings->videoDecoder[i].mosaic.maxNumber, pSettings->videoDecoder[i].mosaic.maxWidth,
            pSettings->videoDecoder[i].mosaic.maxHeight, pSettings->videoDecoder[i].avc51Supported );
        PRINTF( "<br>\n" );
#endif /* if NEXUS_HAS_VIDEO_DECODER */

        fgets( oneline, sizeof( oneline ), pFile );
        /*printf("line for vdec %d (%s)<br>\n", i, oneline );*/
        pos = oneline;
#if NEXUS_HAS_VIDEO_DECODER
        for (j = 0; j<NEXUS_VideoCodec_eMax; j++)
        {
            sscanf( pos, "%u ", &value );
            pSettings->videoDecoder[i].supportedCodecs[j] = value;
            /*if(i==0) printf( "sscanf; vdec[%d].codec[%d] = %d\n", i, j, pSettings->videoDecoder[i].supportedCodecs[j] );*/
            pos += 2;
        }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

        /* read cdb/itb parameters */
        {
            unsigned int bIp, b3840x2160, bSecure, bSoft, bMvc; /* used to remove compile warning about unsigned int versus Bool */
            fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_HAS_VIDEO_DECODER
            sscanf( oneline, VDEC_FORMAT_CDB, &bIp, &b3840x2160,  &bSecure, &bSoft, &bMvc,
                &pInput->videoDecoder[i].numMosaic, &pInput->videoDecoder[i].numFcc );
            pInput->videoDecoder[i].bIp        = bIp;
            pInput->videoDecoder[i].b3840x2160 = b3840x2160;
            pInput->videoDecoder[i].bSecure    = bSecure;
            pInput->videoDecoder[i].bSoft      = bSoft;
            pInput->videoDecoder[i].bMvc       = bMvc;
            PRINTF( "%s: maxNumber %u; numMosaic %u<br>\n", __FUNCTION__, pSettings->videoDecoder[i].mosaic.maxNumber, pInput->videoDecoder[i].numMosaic );
#endif /* if NEXUS_HAS_VIDEO_DECODER */
        }
    }

    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "numsdec %u", &numEntriesInFile );
    PRINTF( "numEntries %u<br>\n", numEntriesInFile );
    for (i = 0; i<numEntriesInFile; i++)
    {
        char *pos = NULL;
        fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_NUM_STILL_DECODES
        sscanf( oneline, SDEC_FORMAT, &i, &used, &pSettings->stillDecoder[i].maxFormat, &avc51Supported );
        pSettings->stillDecoder[i].used           = used;
        pSettings->stillDecoder[i].avc51Supported = avc51Supported;
#endif /* if NEXUS_NUM_STILL_DECODES */

        fgets( oneline, sizeof( oneline ), pFile );
        /*printf("line for vdec %d (%s)<br>\n", i, oneline );*/
        pos = oneline;
#if NEXUS_NUM_STILL_DECODES
        for (j = 0; j<NEXUS_VideoCodec_eMax; j++)
        {
            sscanf( pos, "%u ", &value );
            pSettings->stillDecoder[i].supportedCodecs[j] = value;
            /*if(i==0) printf( "sscanf (%s); vdec[%d].codec[%d] = %d <br>\n", pos, i, j, pSettings->stillDecoder[i].supportedCodecs[j] );*/
            pos += 2;
        }
#endif /* if NEXUS_NUM_STILL_DECODES */

        /* read cdb/itb parameters */
        {
            fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_NUM_STILL_DECODES
            {
                unsigned int bIp, b3840x2160, bSecure, bSoft, bMvc; /* used to remove compile warning about unsigned int versus Bool */
                sscanf( oneline, SDEC_FORMAT_CDB, &bIp, &b3840x2160,  &bSecure, &bSoft, &bMvc, &pInput->stillDecoder[i].numFcc );
                pInput->stillDecoder[i].bIp        = bIp;
                pInput->stillDecoder[i].b3840x2160 = b3840x2160;
                pInput->stillDecoder[i].bSecure    = bSecure;
                pInput->stillDecoder[i].bSoft      = bSoft;
                pInput->stillDecoder[i].bMvc       = bMvc;
            }
#endif /* if NEXUS_NUM_STILL_DECODES */
        }
    }

    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "numdisp %u", &numEntriesInFile );
    PRINTF( "numEntries %u<br>\n", numEntriesInFile );
    for (i = 0; i<numEntriesInFile; i++)
    {
        fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
        sscanf( oneline, "disp %u %u ", &i, &pSettings->display[i].maxFormat );
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */
        if (( i==0 ) && debug) {printf( "disp %u; got line (%s): maxFormat %u<br>\n", i, oneline, pSettings->display[i].maxFormat ); }
        if (pSettings->display[i].maxFormat != NEXUS_VideoFormat_eUnknown)
        {
            char        *pos    = NULL;
            unsigned int window = 0;
            for (j = 0; j<NEXUS_NUM_VIDEO_WINDOWS; j++)
            {
                fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
                if (( i==0 ) && debug) {printf( "disp %u; win %u: got line (%s)<br>\n", i, j, oneline ); }
                pos = &oneline[4];

                sscanf( pos, "%u ", &value );
                window = value;
                pos   += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: window %u<br>\n", i, j, window ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].used = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: used %u<br>\n", i, j, pSettings->display[i].window[j].used ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].deinterlacer = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: deinterlacer %u<br>\n", i, j, pSettings->display[i].window[j].deinterlacer ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].capture = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: capture %u<br>\n", i, j, pSettings->display[i].window[j].capture ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].convertAnyFrameRate = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: convertAnyFrameRate %u<br>\n", i, j, pSettings->display[i].window[j].convertAnyFrameRate ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].precisionLipSync = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: precisionLipSync %u<br>\n", i, j, pSettings->display[i].window[j].precisionLipSync ); }

                sscanf( pos, "%u ", &value );
                pSettings->display[i].window[j].userCaptureBufferCount = value;
                pos += 2;
                if (( i==0 ) && debug) {printf( "disp %u; win %u: userCaptureBufferCount %u<br>\n", i, j, pSettings->display[i].window[j].userCaptureBufferCount ); }
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */
            }
        }
    }

    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "numvenc %u", &numEntriesInFile );
    PRINTF( "numEntries %u<br>\n", numEntriesInFile );
    for (i = 0; i<numEntriesInFile; i++)
    {
        memset( oneline, 0, sizeof( oneline ));

        fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_HAS_VIDEO_ENCODER
        {
            unsigned int venc = 0, used = 0, interlaced = 0, streamMux = 0;
            sscanf( oneline, VENC_FORMAT, &venc, &used, &interlaced, &pSettings->videoEncoder[i].maxWidth, &pSettings->videoEncoder[i].maxHeight, &streamMux );
            pSettings->videoEncoder[i].used       = used;
            pInput->encoders[i].enabled           = used;
            pSettings->videoEncoder[i].interlaced = interlaced;
            pInput->encoders[i].streamMux         = streamMux;
            if (( i==0 ) && debug)
            {
                PRINTF( "%s: enc %d; used %u; width %u; height %u; inter %u; mux %u<br>\n", __FUNCTION__, venc, used,
                    pSettings->videoEncoder[i].maxWidth, pSettings->videoEncoder[i].maxHeight, interlaced, pInput->encoders[i].streamMux    );
            }
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }

    /* records */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, REC_FORMAT, &pInput->record.number, &pInput->record.bitRate, &pInput->record.latency );
    PRINTF( "%s: recs %u; bitrate %u; latency %u (%s)<br>\n", __FUNCTION__, pInput->record.number, pInput->record.bitRate, pInput->record.latency, oneline );

    /* playbacks */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, PB_FORMAT, &pInput->playback.number, &pInput->playback.size, &pInput->playback.max );

    /* messages */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, MSG_FORMAT, &pInput->message.number, &pInput->message.size );
    PRINTF( "%s: msg %u; size %u (%s)<br>\n", __FUNCTION__, pInput->message.number, pInput->message.size, oneline );

    /* liveChannels */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, LIVE_FORMAT, &pInput->live.number, &pInput->live.size );

    /* remuxes */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, REMUX_FORMAT, &pInput->remux.number, &pInput->remux.size );
    PRINTF( "%s: remux %u; size %u<br>\n", __FUNCTION__, pInput->remux.number, pInput->remux.size );

    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, "numgfx  %u", &numEntriesInFile );
    PRINTF( "numEntries %u<br>\n", numEntriesInFile );
    for (i = 0; i<numEntriesInFile; i++)
    {
        unsigned int used = 0, teletext = 0;
        memset( oneline, 0, sizeof( oneline ));
        if (i < NEXUS_NUM_DISPLAYS)
        {
            fgets( oneline, sizeof( oneline ), pFile );
            sscanf( oneline, GRAPHICS_FORMAT, &i, &used, &pInput->surfaceSettings[i].width,
                &pInput->surfaceSettings[i].height, &pInput->surfaceSettings[i].numFrameBufs,
                &pInput->surfaceSettings[i].bits_per_pixel, &teletext );
            pBoxModeSettings->graphics[i].used  = used;
            pInput->surfaceSettings[i].teletext = teletext;
            PRINTF( "%s: gfx %u; numFrameBufs %u; used %d<br>\n", __FUNCTION__, i, pInput->surfaceSettings[i].numFrameBufs, pBoxModeSettings->graphics[i].used );
        }
    }

    {
        unsigned int used = 0;
        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
        sscanf( oneline, GRAPHICS_3D_FORMAT, &used );
        pBoxModeSettings->graphics3d.used = used;
    }

    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    sscanf( oneline, ADEC_MAX_NUM_FORMAT, &numEntriesInFile, &audioCodecMax, &audioPostProcessingMax, &pSettings->audio.dolbyCodecVersion );
    PRINTF( "~%s: numEntries %u; audioCodecMax %u; postProcessingMax %u; dolby %u\n~", __FUNCTION__, numEntriesInFile, audioCodecMax, audioPostProcessingMax,
        pSettings->audio.dolbyCodecVersion );
    for (i = 0; i<numEntriesInFile; i++)
    {
        unsigned int enabled = 0;
        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
        sscanf( oneline, ADEC_FORMAT, &i, &enabled );
        pInput->audioDecoder[i].enabled = enabled;

        /* if (enabled) */ /* we want to read the settings even when disabled so values can get restored when user checks enabled */
        {
            unsigned int codec = 0, post = 0, enabled = 0;

            PRINTF( "~%s: for adec %u: enabled %u\n~", __FUNCTION__, i, enabled );
            memset( oneline, 0, sizeof( oneline ));
            fgets( oneline, sizeof( oneline ), pFile );
            pos = oneline;
            for (codec = 0; codec<NEXUS_AudioCodec_eMax && codec<audioCodecMax; codec++)
            {
                sscanf( pos, "%u ", &enabled );
                if (i==0)
                {
                    pSettings->audio.decodeCodecEnabled[codec] = enabled;
                }
                else
                {
                    pSettings->audio.encodeCodecEnabled[codec] = enabled;
                }
                pos += 2;
            }

            memset( oneline, 0, sizeof( oneline ));
            fgets( oneline, sizeof( oneline ), pFile );
            pos = oneline;
            for (post = 0; post<NEXUS_AudioPostProcessing_eMax && post<audioPostProcessingMax; post++)
            {
                sscanf( pos, "%u ", &enabled );
                pSettings->audio.postProcessingEnabled[post] = enabled;
                pos += 2;
            }

            memset( oneline, 0, sizeof( oneline ));
            fgets( oneline, sizeof( oneline ), pFile );
            sscanf( oneline, ADEC_MISC_FORMAT, &pSettings->audio.maxIndependentDelay, &pSettings->audio.maxDecoderOutputChannels,
                &pSettings->audio.maxDecoderOutputSamplerate, &pSettings->audio.numDecoders, &pSettings->audio.numPassthroughDecoders,
                &pSettings->audio.numHbrPassthroughDecoders, &pSettings->audio.numDspMixers, &pSettings->audio.numPostProcessing,
                &pSettings->audio.numEchoCancellers );

            PRINTF( "%s: adec[%u].codec[1] %u\n", __FUNCTION__, i, pSettings->audio.decodeCodecEnabled[1] );

            memset( oneline, 0, sizeof( oneline ));
            fgets( oneline, sizeof( oneline ), pFile );
            {
                unsigned int passthru = 0, secondary = 0, ip = 0, secure = 0;
                sscanf( oneline, ADEC_PRI_SEC_FORMAT, &passthru, &secondary, &ip, &secure, &pInput->audioDecoder[i].numFcc );
                pInput->audioDecoder[i].bPassthru  = passthru;
                pInput->audioDecoder[i].bSecondary = secondary;
                pInput->audioDecoder[i].bIp        = ip;
                pInput->audioDecoder[i].bSecure    = secure;
            }
        }
    }

    /* read the previous byte values for record, playback, live, remux, etc. */
    {
        unsigned int fieldLength = 10;
        unsigned int bytes       = 0;

        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
        pos = oneline;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pOutputPrevious->record.bytesGeneral = bytes;
        PRINTF( "%s: record " USAGE_FORMAT_BYTES "; (%s) <br>\n", __FUNCTION__, bytes, pos );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pOutputPrevious->playback.bytesGeneral = bytes;
        PRINTF( "%s: playback general " USAGE_FORMAT_BYTES "; (%s) <br>\n", __FUNCTION__, bytes, pos );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pOutputPrevious->playback.bytesSecure = bytes;
        PRINTF( "%s: playback secure " USAGE_FORMAT_BYTES "; (%s) <br>\n", __FUNCTION__, bytes, pos );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->message.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->message.bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->live.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->live.bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->remux.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->remux.bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->videoDecoder[0].bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->videoDecoder[0].bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->audioDecoder[0].bytesGeneral = bytes; /* "Audio Plat" */

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->audioDecoder[0].bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->audioDecoder[1].bytesGeneral = bytes; /* "Audio Module" */

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->audioDecoder[2].bytesGeneral = bytes; /* "Audio Rave" */

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->encode.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->encode.bytesSecure = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->surfaceSettings[0].bytesGeneral = bytes; /* primary */

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->surfaceSettings[1].bytesGeneral = bytes; /* secondary */

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->teletext.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->graphics3d.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->frontend.bytesGeneral = bytes;

        sscanf( pos, USAGE_FORMAT_BYTES, &bytes );
        pos += fieldLength;
        pOutputPrevious->sidSettings.bytesGeneral = bytes;
    }

    /* read the heap totals */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    {
        unsigned int value       = 0;
        unsigned int fieldLength = 10;
        pos = oneline;
        for (i = 0; i<NEXUS_MAX_HEAPS; i++)
        {
            sscanf( pos, USAGE_FORMAT_BYTES, &value );
            heap_totals_previous[i] = value;
            PRINTF( "%s: heap_previous[%u] %u<br>\n", __FUNCTION__, i, value );
            pos += fieldLength;
        }
    }

    /* read the heap paddings */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    {
        unsigned int value       = 0;
        unsigned int fieldLength = 10;
        pos = oneline;
        for (i = 0; i<NEXUS_MAX_HEAPS; i++)
        {
            sscanf( pos, USAGE_FORMAT_BYTES, &value );
            heap_padding[i] = value;
            PRINTF( "%s: heap_padding[%u] %u<br>\n", __FUNCTION__, i, value );
            pos += fieldLength;
        }
    }

    /* now read the platformStatusPrevious values */
    {
        unsigned int fieldLength = 10;

        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
        pos = oneline;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.general );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.secure );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.general );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.secure );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].audio.general );
        pos += fieldLength;

        sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[0].display.general );
        pos += fieldLength;
    }

    /* read the videoEncoder values */
    memset( oneline, 0, sizeof( oneline ));
    fgets( oneline, sizeof( oneline ), pFile );
    {
        unsigned int fieldLength = 10;
        pos = oneline;
        for (i = 0; i<NEXUS_MAX_MEMC; i++)
        {
            sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.firmware );
            pos += fieldLength;
            sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.index );
            pos += fieldLength;
            sscanf( pos, USAGE_FORMAT_BYTES, &pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.data );
            pos += fieldLength;
            PRINTF( "%s: memc[%u].videoEncoder: %u; %u; %u\n", __FUNCTION__, i, pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.firmware,
                pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.index, pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.data );
        }
    }

    {
        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
#if NEXUS_NUM_STILL_DECODES
        {
            unsigned int enabled = 0;
            sscanf( oneline, SID_FORMAT, &enabled );
            pInput->sidSettings.enabled = enabled;
            PRINTF( "%s: SID enabled %u<br>\n", __FUNCTION__, pInput->sidSettings.enabled );
        }
#endif /* if NEXUS_NUM_STILL_DECODES */
    }

    {
        unsigned int hdDvi = 0, ccir656 = 0;
        memset( oneline, 0, sizeof( oneline ));
        fgets( oneline, sizeof( oneline ), pFile );
        sscanf( oneline, VIDEO_INPUT_FORMAT, &hdDvi, &ccir656 );
        pSettings->videoInputs.hdDvi   = hdDvi;
        pSettings->videoInputs.ccir656 = ccir656;
    }
    fclose( pFile );

    return( 0 );
} /* read_settings */

static int write_settings(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput,
    Memconfig_BoxMode                 *pBoxModeSettings,
    Memconfig_AppMemUsage             *pOutput,
    NEXUS_PlatformStatus              *pPlatformStatusPrevious
    )
{
    unsigned int         i, j;
    int                  debug = 1;
    FILE                *pFile = NULL;
    NEXUS_PlatformStatus platformStatus;
    char                 oneline[1024];

    pFile = fopen( state_filename, "w+" );

    if (pFile == NULL)
    {
        printf( "%s: could not open file %s\n", __FUNCTION__, state_filename );
        return( -1 );
    }

    NEXUS_Platform_GetStatus( &platformStatus );

    chmod( state_filename, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "boxmode %d\n", ReadBoxMode());
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "numvdec %u\n", NEXUS_NUM_VIDEO_DECODERS );
    fwrite( oneline, 1, strlen( oneline ), pFile );
#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        const char *formatName = NULL;

        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), VDEC_FORMAT, i, pSettings->videoDecoder[i].used, pSettings->videoDecoder[i].maxFormat,
            pSettings->videoDecoder[i].colorDepth, pSettings->videoDecoder[i].mosaic.maxNumber, pSettings->videoDecoder[i].mosaic.maxWidth,
            pSettings->videoDecoder[i].mosaic.maxHeight, pSettings->videoDecoder[i].avc51Supported );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        if (i==0) {PRINTF( "vdec0 codecs: " ); }
        for (j = 0; j<NEXUS_VideoCodec_eMax; j++)
        {
            snprintf( oneline, sizeof( oneline ), "%d ", pSettings->videoDecoder[i].supportedCodecs[j] );
            fwrite( oneline, 1, strlen( oneline ), pFile );
            formatName = lookup_name( g_videoCodecStrs, j );
            if (i==0) {PRINTF( "codec %u:%u(%s); ", j, pSettings->videoDecoder[i].supportedCodecs[j], formatName ); }
        }
        snprintf( oneline, sizeof( oneline ), "\n" );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        if (i==0) {PRINTF( "\n" ); }

        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), VDEC_FORMAT_CDB, pInput->videoDecoder[i].bIp, pInput->videoDecoder[i].b3840x2160,
            pInput->videoDecoder[i].bSecure, pInput->videoDecoder[i].bSoft, pInput->videoDecoder[i].bMvc,
            pInput->videoDecoder[i].numMosaic, pInput->videoDecoder[i].numFcc );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        PRINTF( "%s: maxNumber %u; numMosaic %u<br>\n", __FUNCTION__, pSettings->videoDecoder[i].mosaic.maxNumber, pInput->videoDecoder[i].numMosaic );
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "numsdec %u\n", NEXUS_NUM_STILL_DECODES );
    fwrite( oneline, 1, strlen( oneline ), pFile );
#if NEXUS_NUM_STILL_DECODES
    for (i = 0; i<NEXUS_NUM_STILL_DECODES; i++)
    {
        memset( oneline, 0, sizeof( oneline ));
        PRINTF( "%s: sdec %u; used %u; maxFormat %u; avc51 %u\n", __FUNCTION__, i, pSettings->stillDecoder[i].used, pSettings->stillDecoder[i].maxFormat,
            pSettings->stillDecoder[i].avc51Supported );
        snprintf( oneline, sizeof( oneline ), SDEC_FORMAT, i, pSettings->stillDecoder[i].used, pSettings->stillDecoder[i].maxFormat,
            pSettings->stillDecoder[i].avc51Supported );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        for (j = 0; j<NEXUS_VideoCodec_eMax; j++)
        {
            snprintf( oneline, sizeof( oneline ), "%d ", pSettings->stillDecoder[i].supportedCodecs[j] );
            fwrite( oneline, 1, strlen( oneline ), pFile );
        }
        snprintf( oneline, sizeof( oneline ), "\n" );
        fwrite( oneline, 1, strlen( oneline ), pFile );

        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), SDEC_FORMAT_CDB, pInput->stillDecoder[i].bIp, pInput->stillDecoder[i].b3840x2160,
            pInput->stillDecoder[i].bSecure, pInput->stillDecoder[i].bSoft, pInput->stillDecoder[i].bMvc,
            pInput->stillDecoder[i].numFcc );
        fwrite( oneline, 1, strlen( oneline ), pFile );
    }
#endif /* if NEXUS_NUM_STILL_DECODES */

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "numdisp %u\n", NEXUS_NUM_DISPLAYS );
    fwrite( oneline, 1, strlen( oneline ), pFile );
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        snprintf( oneline, sizeof( oneline ), "disp %d %d\n", i, pSettings->display[i].maxFormat );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        if (( i==0 ) && debug) {PRINTF( "writing: (%s)<br>\n", oneline ); }
        if (pSettings->display[i].maxFormat != NEXUS_VideoFormat_eUnknown)
        {
            for (j = 0; j<NEXUS_NUM_VIDEO_WINDOWS; j++)
            {
                snprintf( oneline, sizeof( oneline ), "win %d %d %d %d %d %d %d\n", j, pSettings->display[i].window[j].used,
                    pSettings->display[i].window[j].deinterlacer, pSettings->display[i].window[j].capture,
                    pSettings->display[i].window[j].convertAnyFrameRate, pSettings->display[i].window[j].precisionLipSync,
                    pSettings->display[i].window[j].userCaptureBufferCount );
                fwrite( oneline, 1, strlen( oneline ), pFile );
                if (( i==0 ) && debug) {PRINTF( "writing: (%s)<br>\n", oneline ); }
            }
        }
    }
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    memset( oneline, 0, sizeof( oneline ));
#if NEXUS_HAS_VIDEO_ENCODER
    /* on 7563, NEXUS_MAX_VIDEO_ENCODERS is set to 4 even when NEXUS_HAS_VIDEO_ENCODER is false */
    snprintf( oneline, sizeof( oneline ), "numvenc %u\n", NEXUS_NUM_VIDEO_ENCODERS );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        memset( oneline, 0, sizeof( oneline ));

        snprintf( oneline, sizeof( oneline ), VENC_FORMAT, i, pSettings->videoEncoder[i].used, pSettings->videoEncoder[i].interlaced,
            pSettings->videoEncoder[i].maxWidth, pSettings->videoEncoder[i].maxHeight, pInput->encoders[i].streamMux );
        fwrite( oneline, 1, strlen( oneline ), pFile );
    }
#else /* if NEXUS_HAS_VIDEO_ENCODER */
    snprintf( oneline, sizeof( oneline ), "numvenc %u\n", 0 );
    fwrite( oneline, 1, strlen( oneline ), pFile );
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

    /* records */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), REC_FORMAT, pInput->record.number, pInput->record.bitRate, pInput->record.latency );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    PRINTF( "%s: recs %u; bitrate %u; latency %u<br>\n", __FUNCTION__, pInput->record.number, pInput->record.bitRate, pInput->record.latency );

    /* playbacks */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), PB_FORMAT, pInput->playback.number, pInput->playback.size,
        pInput->playback.max );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* messages */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), MSG_FORMAT, pInput->message.number, pInput->message.size );
    PRINTF( "%s: msg0 size %u<br>\n", __FUNCTION__, pInput->message.size );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* liveChannels */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), LIVE_FORMAT, pInput->live.number, pInput->live.size );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* remuxes */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), REMUX_FORMAT, pInput->remux.number, pInput->remux.size );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    PRINTF( "%s: remux %u; size %u<br>\n", __FUNCTION__, pInput->remux.number, pInput->remux.size );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "numgfx  %u\n", NEXUS_NUM_DISPLAYS );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        memset( oneline, 0, sizeof( oneline ));
        if (i < NEXUS_NUM_DISPLAYS)
        {
            snprintf( oneline, sizeof( oneline ), GRAPHICS_FORMAT, i, pBoxModeSettings->graphics[i].used, pInput->surfaceSettings[i].width,
                pInput->surfaceSettings[i].height, pInput->surfaceSettings[i].numFrameBufs,
                pInput->surfaceSettings[i].bits_per_pixel, pInput->surfaceSettings[i].teletext );
            fwrite( oneline, 1, strlen( oneline ), pFile );
            PRINTF( "%s: gfx %u; numFrameBufs %u; used %d<br>\n", __FUNCTION__, i, pInput->surfaceSettings[i].numFrameBufs, pBoxModeSettings->graphics[i].used );
        }
    }

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), GRAPHICS_3D_FORMAT, pBoxModeSettings->graphics3d.used );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), ADEC_MAX_NUM_FORMAT, NEXUS_NUM_AUDIO_DECODERS, NEXUS_AudioCodec_eMax, NEXUS_AudioPostProcessing_eMax,
        pSettings->audio.dolbyCodecVersion );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    PRINTF( "~%s: dolbyCodecVersion %u\n~", __FUNCTION__, pSettings->audio.dolbyCodecVersion );
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        bool enabled = false;

        memset( oneline, 0, sizeof( oneline ));
        if (i==0)
        {
            enabled = pInput->audioDecoder[i].enabled;
        }
        else
        {
#if NEXUS_HAS_VIDEO_ENCODER
            enabled = pSettings->videoEncoder[i-1].used && pInput->audioDecoder[i].enabled;
#else
            enabled = false;
#endif
        }
        snprintf( oneline, sizeof( oneline ), ADEC_FORMAT, i, enabled );
        fwrite( oneline, 1, strlen( oneline ), pFile );
        /* if (enabled) */ /* we want to write the settings even when disabled so values can get restored when user checks enabled */
        {
            unsigned int codec = 0, post = 0;
            for (codec = 0; codec<NEXUS_AudioCodec_eMax; codec++)
            {
                if (i==0)
                {
                    snprintf( oneline, sizeof( oneline ), "%u ", pSettings->audio.decodeCodecEnabled[codec] );
                }
                else
                {
                    snprintf( oneline, sizeof( oneline ), "%u ", pSettings->audio.encodeCodecEnabled[codec] );
                }
                fwrite( oneline, 1, strlen( oneline ), pFile );
            }
            snprintf( oneline, sizeof( oneline ), "\n" );
            fwrite( oneline, 1, strlen( oneline ), pFile );
            for (post = 0; post<NEXUS_AudioPostProcessing_eMax; post++)
            {
                snprintf( oneline, sizeof( oneline ), "%u ", pSettings->audio.postProcessingEnabled[post] );
                fwrite( oneline, 1, strlen( oneline ), pFile );
            }
            snprintf( oneline, sizeof( oneline ), "\n" );
            fwrite( oneline, 1, strlen( oneline ), pFile );
            snprintf( oneline, sizeof( oneline ), ADEC_MISC_FORMAT, pSettings->audio.maxIndependentDelay, pSettings->audio.maxDecoderOutputChannels,
                pSettings->audio.maxDecoderOutputSamplerate, pSettings->audio.numDecoders, pSettings->audio.numPassthroughDecoders,
                pSettings->audio.numHbrPassthroughDecoders, pSettings->audio.numDspMixers, pSettings->audio.numPostProcessing,
                pSettings->audio.numEchoCancellers );
            fwrite( oneline, 1, strlen( oneline ), pFile );
            snprintf( oneline, sizeof( oneline ), ADEC_PRI_SEC_FORMAT, pInput->audioDecoder[i].bPassthru, pInput->audioDecoder[i].bSecondary, pInput->audioDecoder[i].bIp,
                pInput->audioDecoder[i].bSecure, pInput->audioDecoder[i].numFcc );
            PRINTF( "%s: adec[%u]->secure %u\n", __FUNCTION__, i, pInput->audioDecoder[i].bSecure );
            fwrite( oneline, 1, strlen( oneline ), pFile );
        }
    }

    /* record    playbackg playbacks messageg  messages  liveg     lives     remuxg    remuxs    vdecgen   vravesec  audiogen  audiosec  aravesec  encodegen surfaceg  teletext  */
    /* 010559488 006160384 000065536 000016384 000851968 000000000 001638400 000000000 000000000 000393216 008257536 000000000 000393216 000000000 005783552 011059200 000000000 */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->record.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->playback.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->playback.bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->message.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->message.bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->live.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->live.bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->remux.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->remux.bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->videoDecoder[0].bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->videoDecoder[0].bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->audioDecoder[0].bytesGeneral ); /* "Audio Plat" */
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->audioDecoder[0].bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->audioDecoder[1].bytesGeneral ); /* "Audio Module" */
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->audioDecoder[2].bytesGeneral ); /* "Audio Rave" */
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->encode.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->encode.bytesSecure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->surfaceSettings[0].bytesGeneral ); /* primary */
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->surfaceSettings[1].bytesGeneral ); /* secondary */
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->teletext.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );
    PRINTF( "%s: teletext.bytesGeneral %u\n", __FUNCTION__, pOutput->teletext.bytesGeneral );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->graphics3d.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->frontend.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pOutput->sidSettings.bytesGeneral );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* force USAGE_FORMAT_BYTES to end the line */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "\n" );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* write the heap totals on a new line */
    for (i = 0; i<NEXUS_MAX_HEAPS; i++)
    {
        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, (unsigned int) heap_totals[i] );
        fwrite( oneline, 1, strlen( oneline ), pFile );
    }

    /* force USAGE_FORMAT_BYTES to end the line after heap totals are written */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "\n" );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* write the heap padding on a new line */
    for (i = 0; i<NEXUS_MAX_HEAPS; i++)
    {
        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, (unsigned int) heap_padding[i] );
        fwrite( oneline, 1, strlen( oneline ), pFile );
    }

    /* force USAGE_FORMAT_BYTES to end the line after heap paddings are written */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "\n" );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* now write the platformStatusPrevious values */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.general );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].videoDecoder.secure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.general );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].videoEncoder.secure );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].audio.general );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, pPlatformStatusPrevious->estimatedMemory.memc[0].display.general );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* force USAGE_FORMAT_BYTES to end the line after platformStatusPrevious totals are written */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "\n" );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    /* write the videoEncoder on a new line */
    for (i = 0; i<NEXUS_NUM_MEMC; i++)
    {
        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, (unsigned int) pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.firmware );
        fwrite( oneline, 1, strlen( oneline ), pFile );

        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, (unsigned int) pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.index );
        fwrite( oneline, 1, strlen( oneline ), pFile );

        memset( oneline, 0, sizeof( oneline ));
        snprintf( oneline, sizeof( oneline ), USAGE_FORMAT_BYTES, (unsigned int) pPlatformStatusPrevious->estimatedMemory.memc[i].videoEncoder.data );
        fwrite( oneline, 1, strlen( oneline ), pFile );
    }

    /* force USAGE_FORMAT_BYTES to end the line after heap paddings are written */
    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), "\n" );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    memset( oneline, 0, sizeof( oneline ));
#if NEXUS_NUM_STILL_DECODES
    snprintf( oneline, sizeof( oneline ), SID_FORMAT, pInput->sidSettings.enabled );
#else
    snprintf( oneline, sizeof( oneline ), SID_FORMAT, 0 );
#endif
    fwrite( oneline, 1, strlen( oneline ), pFile );
    PRINTF( "%s: SID enabled %u<br>\n", __FUNCTION__, pInput->sidSettings.enabled );

    memset( oneline, 0, sizeof( oneline ));
    snprintf( oneline, sizeof( oneline ), VIDEO_INPUT_FORMAT, pSettings->videoInputs.hdDvi, pSettings->videoInputs.ccir656 );
    fwrite( oneline, 1, strlen( oneline ), pFile );

    fclose( pFile );

    return( 0 );
} /* write_settings */

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
            #if 0
            /* was causing values to be reset to 640x480 when graphics disabled and then immediately enabled */
            pInput->surfaceSettings[i].numFrameBufs   = 0;
            pInput->surfaceSettings[i].width          = 0;
            pInput->surfaceSettings[i].height         = 0;
            pInput->surfaceSettings[i].bits_per_pixel = 0;
            #endif /* if 0 */
            pOutput->surfaceSettings[0].bytesGeneral = 0;
            pOutput->surfaceSettings[1].bytesGeneral = 0;
        }
    }
#endif /*NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    return( 0 );
} /* transfer_settings */

#if NEXUS_HAS_VIDEO_ENCODER
static int get_non_video_encoder_count(
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
#endif /* if NEXUS_HAS_VIDEO_ENCODER */

static int update_audio_decoder_codecs(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput
    )
{
    unsigned int adec;

    /* loop through all audio decodes to determine if one is active; if at least 1 is active, bail out ... don't do anything else */
    for (adec = 0; adec<NEXUS_NUM_AUDIO_DECODERS; adec++)
    {
        if (pInput->audioDecoder[adec].enabled)
        {
            PRINTF( "~%s: adec %u is enabled; bailing out\n", __FUNCTION__, adec );
            return( 0 );
        }
    }

    PRINTF( "~%s: no adecs are enabled; clearing pSettings->audio structure; %u bytes\n", __FUNCTION__, sizeof( pSettings->audio ));
    memset( &pSettings->audio, 0, sizeof( pSettings->audio ));

    return( 0 );
} /* update_audio_decoder_codecs */

static int update_settings(
    NEXUS_MemoryConfigurationSettings *pSettings,
    Memconfig_AppUsageSettings        *pInput,
    const char                        *variable_name_value,
    Memconfig_BoxMode                 *pBoxModeSettings
    )
{
    int          rc    = -1;
    int          debug = 0;
    char        *pos   = NULL;
    unsigned int value = 0;

    if (variable_name_value == NULL)
    {
        return( rc );
    }

    if (debug==1) {printf( "%s: varname (%s)\n", __FUNCTION__, variable_name_value ); }

    pos = strstr( variable_name_value, ":" );

    if (pos == NULL)
    {
        printf( "could not find value separator<br>\n" );
        return( rc );
    }

    pos++;
    sscanf( pos, "%u", &value );

    rc = 0;

    if (strncmp( variable_name_value, "vdec", 4 ) == 0)
    {
        unsigned int vdec  = 0;
        unsigned int codec = 0;
        sscanf( &variable_name_value[4], "%u", &vdec );
        if (debug) {printf( "%s: vdec (%u); for pos (%s), value (%u)<br>\n", __FUNCTION__, vdec, pos, value ); }
        if (strncmp( &variable_name_value[5], "used", 4 ) == 0)
        {
            pSettings->videoDecoder[vdec].used = value;
            pInput->videoDecoder[vdec].enabled = value;
            printf( "~%s: videoDecoder[%u] %u; pInput->videoDecoder[%u] %u\n~", __FUNCTION__, vdec, pSettings->videoDecoder[vdec].used, vdec, pInput->videoDecoder[vdec].enabled );

#if NEXUS_HAS_VIDEO_ENCODER
            {
                unsigned int nonEncoderCount = get_non_video_encoder_count( pBoxModeSettings );
                unsigned     encoderIdx      = vdec - nonEncoderCount;

                /* if this decoder is used for transcode, enable/disable the associated encoder */
                printf( "~%s: nonEncoderCount %u; encoderIdx %u\n~", __FUNCTION__, nonEncoderCount, encoderIdx );
                pSettings->videoEncoder[encoderIdx].used = value;
                pInput->encoders[encoderIdx].enabled     = value;
                printf( "~%s: videoEncoder[%u] %u; encoders[%u] %u; audioDecoder[%u] %u\n~", __FUNCTION__, encoderIdx, pSettings->videoEncoder[encoderIdx].used,
                    encoderIdx, pInput->encoders[encoderIdx].enabled, encoderIdx+1, pInput->audioDecoder[encoderIdx+1].enabled );

#if NEXUS_HAS_AUDIO
                pInput->audioDecoder[encoderIdx+1].enabled = value;
#endif /* if NEXUS_HAS_AUDIO */
            }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
        }
        else if (strncmp( &variable_name_value[5], "avc51", 5 ) == 0)
        {
            pSettings->videoDecoder[vdec].avc51Supported = value;
            /* if we are enabling AVC 5.1, also enable H264 (but don't turn it off ) */
            if (value == 1)
            {
                pSettings->videoDecoder[vdec].supportedCodecs[NEXUS_VideoCodec_eH264] = 1;
            }
        }
        else if (strncmp( &variable_name_value[5], "codec", 5 ) == 0)
        {
            sscanf( &variable_name_value[10], "%u", &codec );
            if (codec < NEXUS_VideoCodec_eMax)
            {
                PRINTF( "vdec[%d].codec[%d] is %d<br>\n", vdec, codec, value );
                pSettings->videoDecoder[vdec].supportedCodecs[codec] = value;
                if (codec == NEXUS_VideoCodec_eH264_Mvc)
                {
                    pInput->videoDecoder[vdec].bMvc = value;
                }
                else if (( codec == NEXUS_VideoCodec_eMpeg4Part2 ) || ( codec == NEXUS_VideoCodec_eDivx311 ) || ( codec == NEXUS_VideoCodec_eVc1SimpleMain ))
                {
                    pInput->videoDecoder[vdec].bSoft = value;
                }
            }
        }
        else if (strncmp( &variable_name_value[5], "maxFormat", 9 ) == 0)
        {
            pSettings->videoDecoder[vdec].maxFormat = value;
        }
        else if (strncmp( &variable_name_value[5], "maxNumber", 9 ) == 0)
        {
            if (pSettings->videoDecoder[vdec].mosaic.maxNumber > 0)
            {
                if (value > NEXUS_NUM_MOSAIC_DECODES)
                {
                    printf( "~ALERT~Number of mosaics cannot exceed the maxNumber (%u)!~", NEXUS_NUM_MOSAIC_DECODES );
                    pInput->videoDecoder[vdec].numMosaic = NEXUS_NUM_MOSAIC_DECODES;
                }
                else
                {
                    pInput->videoDecoder[vdec].numMosaic = value;
                }
            }
            else
            {
                pInput->videoDecoder[vdec].numMosaic = 0;
                if (value > 0)
                {
                    printf( "~ALERT~Number of mosaics cannot exceed the maxNumber (%u)!~", pSettings->videoDecoder[vdec].mosaic.maxNumber );
                }
            }
        }
        else if (strncmp( &variable_name_value[5], "maxWidth", 8 ) == 0)
        {
            pSettings->videoDecoder[vdec].mosaic.maxWidth = value;
        }
        else if (strncmp( &variable_name_value[5], "maxHeight", 9 ) == 0)
        {
            pSettings->videoDecoder[vdec].mosaic.maxHeight = value;
        }
        else if (strncmp( &variable_name_value[5], "format", 6 ) == 0)
        {
            pSettings->videoDecoder[vdec].maxFormat = value;
            pInput->videoDecoder[vdec].b3840x2160   = 0;
            if (( pSettings->videoDecoder[vdec].maxFormat == NEXUS_VideoFormat_e3840x2160p30hz ) ||
                ( pSettings->videoDecoder[vdec].maxFormat == NEXUS_VideoFormat_e3840x2160p60hz ))
            {
                pInput->videoDecoder[vdec].b3840x2160 = 1;
            }
            PRINTF( "%s: vdec %u ... b3840x2160 is %u\n", __FUNCTION__, vdec, pInput->videoDecoder[vdec].b3840x2160 );
        }
        else if (strncmp( &variable_name_value[5], "colorDepth", 10 ) == 0)
        {
            if (( value !=8 ) && ( value != 10 ))
            {
                printf( "~ALERT~Color Depth must be either 8 or 10!~" );
                pSettings->videoDecoder[vdec].colorDepth = 8;
            }
            else
            {
                pSettings->videoDecoder[vdec].colorDepth = value;
            }
        }
        else if (strncmp( &variable_name_value[5], "ip", 2 ) == 0)
        {
            pInput->videoDecoder[vdec].bIp = value;
        }
        /* b3840x2160 and numMosaic are mutually exclusive; only one can be enabled at a time */
        else if (strncmp( &variable_name_value[5], "b3840x2160", 10 ) == 0)
        {
            PRINTF( "%s: 3840 value %d; line (%s)\n", __FUNCTION__, value, variable_name_value );
            if (pInput->videoDecoder[vdec].numMosaic && ( value==1 ))
            {
                printf( "~ALERT~3840x2160 cannot be enabled while mosaic is enabled~" );
            }
            else
            {
                pInput->videoDecoder[vdec].b3840x2160 = value;
            }
        }
        else if (strncmp( &variable_name_value[5], "numMosaic", 9 ) == 0)
        {
            /* if user enables numMosaic here, use the numMosaic from memconfig struct */
            if (value)
            {
                if (pInput->videoDecoder[vdec].b3840x2160)
                {
                    printf( "~ALERT~mosaic cannot be enabled while 3840x2160 is enabled~" );
                }
                else
                {
                    if (pSettings->videoDecoder[vdec].mosaic.maxNumber == 0)
                    {
                        printf( "~ALERT~You cannot enable mosaic while mosaic.maxNumber is zero.~" );
                    }
                    else
                    {
                        pInput->videoDecoder[vdec].numMosaic = pSettings->videoDecoder[vdec].mosaic.maxNumber;
                    }
                }
            }
            else
            {
                pInput->videoDecoder[vdec].numMosaic = value;
            }
            PRINTF( "%s: numMosaic now is %u\n", __FUNCTION__, pInput->videoDecoder[vdec].numMosaic );
        }
        else if (strncmp( &variable_name_value[5], "secure", 6 ) == 0)
        {
            pInput->videoDecoder[vdec].bSecure = value;
        }
        else if (strncmp( &variable_name_value[5], "soft", 4 ) == 0)
        {
            pInput->videoDecoder[vdec].bSoft = value;
        }
        else if (strncmp( &variable_name_value[5], "mvc", 3 ) == 0)
        {
            pInput->videoDecoder[vdec].bMvc = value;
            pSettings->videoDecoder[vdec].supportedCodecs[NEXUS_VideoCodec_eH264_Mvc] = value;
        }
        else if (strncmp( &variable_name_value[5], "fcc", 3 ) == 0)
        {
            pInput->videoDecoder[vdec].numFcc = value;
        }
    }
#if NEXUS_NUM_STILL_DECODES
    else if (strncmp( variable_name_value, "sdec", 4 )==0)
    {
        unsigned int sdec  = 0;
        unsigned int codec = 0;
        sscanf( &variable_name_value[4], "%u", &sdec );
        PRINTF( "%s: sdec (%u); for pos (%s), value (%u)<br>\n", __FUNCTION__, sdec, pos, value );
        if (strncmp( &variable_name_value[5], "used", 4 ) == 0)
        {
            pSettings->stillDecoder[sdec].used = value;
            pInput->stillDecoder[sdec].enabled = value;
        }
        else if (strncmp( &variable_name_value[5], "avc51", 5 ) == 0)
        {
            pSettings->stillDecoder[sdec].avc51Supported = value;
            PRINTF( "%s: sdec %u: avc %u\n", __FUNCTION__, sdec, pSettings->stillDecoder[sdec].avc51Supported );
            /* if we are enabling AVC 5.1, also enable H264 (but don't turn it off ) */
            if (value == 1)
            {
                pSettings->stillDecoder[sdec].supportedCodecs[NEXUS_VideoCodec_eH264] = 1;
            }
        }
        else if (strncmp( &variable_name_value[5], "codec", 5 ) == 0)
        {
            sscanf( &variable_name_value[10], "%u", &codec );
            if (codec < NEXUS_VideoCodec_eMax)
            {
                PRINTF( "sdec[%d].codec[%d] is %d<br>\n", sdec, codec, value );
                pSettings->stillDecoder[sdec].supportedCodecs[codec] = value;
            }
            else if (( codec == NEXUS_VideoCodec_eMpeg4Part2 ) || ( codec == NEXUS_VideoCodec_eDivx311 ) || ( codec == NEXUS_VideoCodec_eVc1SimpleMain ))
            {
                pInput->stillDecoder[sdec].bSoft = value;
            }
        }
        else if (strncmp( &variable_name_value[5], "maxFormat", 9 ) == 0)
        {
            pSettings->stillDecoder[sdec].maxFormat = value;
        }
        else if (strncmp( &variable_name_value[5], "format", 6 ) == 0)
        {
            pSettings->stillDecoder[sdec].maxFormat = value;
            pInput->stillDecoder[sdec].b3840x2160   = 0;
            if (( pSettings->stillDecoder[sdec].maxFormat == NEXUS_VideoFormat_e3840x2160p30hz ) ||
                ( pSettings->stillDecoder[sdec].maxFormat == NEXUS_VideoFormat_e3840x2160p60hz ))
            {
                pInput->stillDecoder[sdec].b3840x2160 = 1;
            }
            PRINTF( "%s: sdec %u ... b3840x2160 is %u\n", __FUNCTION__, sdec, pInput->stillDecoder[sdec].b3840x2160 );
        }
        else if (strncmp( &variable_name_value[5], "ip", 2 ) == 0)
        {
            pInput->stillDecoder[sdec].bIp = value;
        }
        else if (strncmp( &variable_name_value[5], "b3840x2160", 10 ) == 0)
        {
            PRINTF( "%s: 3840 value %d; line (%s)\n", __FUNCTION__, value, variable_name_value );
            pInput->stillDecoder[sdec].b3840x2160 = value;
        }
        else if (strncmp( &variable_name_value[5], "secure", 6 ) == 0)
        {
            pInput->stillDecoder[sdec].bSecure = value;
        }
        else if (strncmp( &variable_name_value[5], "soft", 4 ) == 0)
        {
            pInput->stillDecoder[sdec].bSoft = value;
        }
        else if (strncmp( &variable_name_value[5], "mvc", 3 ) == 0)
        {
            pInput->stillDecoder[sdec].bMvc = value;
        }
        else if (strncmp( &variable_name_value[5], "fcc", 3 ) == 0)
        {
            pInput->stillDecoder[sdec].numFcc = value;
        }
    }
#endif /* if NEXUS_NUM_STILL_DECODES */
    else if (strstr( variable_name_value, "disp" ))
    {
        unsigned int display = 0;
        unsigned int window  = 0;
        sscanf( &variable_name_value[4], "%u", &display );
        sscanf( &variable_name_value[8], "%u", &window );
        PRINTF( "%s: display (%u); window (%u); for pos (%s), value (%u)<br>\n", __FUNCTION__, display, window, pos, value );
        if (strncmp( &variable_name_value[9], "used", 4 ) == 0)
        {
            pSettings->display[display].window[window].used = value;
        }
        else if (strncmp( &variable_name_value[5], "maxFormat", 9 ) == 0) /* disp5maxFormat */
        {
            pSettings->display[display].maxFormat = value;
            printf( "%s: maxFormat %u<br>\n", __FUNCTION__, value );
            /* if we set maxFormat to 0, disable both windows */
            if (value == 0)
            {
                printf( "%s: maxFormat %u; turning both windows off<br>\n", __FUNCTION__, value );
                pSettings->display[display].window[0].used = 0;
                pSettings->display[display].window[1].used = 0;
            }
        }
        else if (strncmp( &variable_name_value[5], "teletext", 8 ) == 0)
        {
            pInput->surfaceSettings[display].teletext = value;
            PRINTF( "%s: display %u; teletext %u<br>\n", __FUNCTION__, display, value );
        }
        else if (strncmp( &variable_name_value[9], "deinterlacer", 12 ) == 0)
        {
            pSettings->display[display].window[window].deinterlacer = value;
        }
        else if (strncmp( &variable_name_value[9], "capture", 7 ) == 0)
        {
            pSettings->display[display].window[window].capture = value;
        }
        else if (strncmp( &variable_name_value[9], "lipsync", 7 ) == 0)
        {
            pSettings->display[display].window[window].precisionLipSync = value;
        }
        else if (strncmp( &variable_name_value[9], "convert", 7 ) == 0)
        {
            pSettings->display[display].window[window].convertAnyFrameRate = value;
        }
        else if (strncmp( &variable_name_value[9], "usercapture", 11 ) == 0)
        {
            pSettings->display[display].window[window].userCaptureBufferCount = value;
        }
    }
    else if (strstr( variable_name_value, "venc" ))
    {
#if NEXUS_HAS_VIDEO_ENCODER
        unsigned int venc = 0;
        sscanf( &variable_name_value[4], "%u", &venc );
        if (debug) {printf( "%s: venc (%u); for pos (%s), value (%u)<br>\n", __FUNCTION__, venc, pos, value ); }
        if (strncmp( &variable_name_value[5], "used", 4 ) == 0)
        {
            unsigned int i;
            unsigned int encoderCount = 0;

            pSettings->videoEncoder[venc].used = value;
            pInput->encoders[venc].enabled     = value;
#if NEXUS_HAS_AUDIO
            /* each encode must have a corresponding audio decoder */
            pInput->audioDecoder[venc+1].enabled = value;
#endif /* if NEXUS_HAS_AUDIO */

            printf( "%s: encoder[%u] %u; audioDecoder[%u] %u\n", __FUNCTION__, venc, pInput->encoders[venc].enabled, venc+1, pInput->audioDecoder[venc+1].enabled );

            /* each encode must have a corresponding video decoder; loop through each decoder to see which one is associated with this encoder */
            for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++)
            {
                if (pBoxModeSettings->videoDecoder[i].property == Memconfig_VideoDecoderProperty_eTranscode)
                {
                    /* if this decoder matches the encoder that the user is modifying */
                    if (encoderCount == venc)
                    {
                        pSettings->videoDecoder[i].used = value;
                        pInput->videoDecoder[i].enabled = value;
                        printf( "%s: videoDecoder[%u] %u\n", __FUNCTION__, i, pSettings->videoDecoder[i].used );
                    }
                    encoderCount++;
                }
            }
            printf( "~%s: after venc update; vdec[3] %u\n~", __FUNCTION__, pSettings->videoDecoder[3].used );
        }
        else if (strncmp( &variable_name_value[5], "maxWidth", 8 ) == 0)
        {
            pSettings->videoEncoder[venc].maxWidth = value;
        }
        else if (strncmp( &variable_name_value[5], "maxHeight", 9 ) == 0)
        {
            pSettings->videoEncoder[venc].maxHeight = value;
        }
        else if (strncmp( &variable_name_value[5], "interlaced", 10 ) == 0)
        {
            pSettings->videoEncoder[venc].interlaced = value;
        }
        else if (strncmp( &variable_name_value[5], "streamMux", 9 ) == 0)
        {
            pInput->encoders[venc].streamMux = value;
            PRINTF( "%s: venc %u: streamMux %u\n", __FUNCTION__, venc, pInput->encoders[venc].streamMux );
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }
    else if (strstr( variable_name_value, "rec0" ))
    {
        if (debug) {printf( "%s: for pos (%s), value (%u)<br>\n", __FUNCTION__, pos, value ); }
        if (strncmp( &variable_name_value[4], "numRecords", 10 ) == 0)
        {
            if (value <= pInput->record.max)
            {
                pInput->record.number = value;
            }
            else
            {
                pInput->record.number = pInput->record.max;
                printf( "~ALERT~Number of records cannot exceed %u~", pInput->record.max );
            }
        }
        else if (strncmp( &variable_name_value[4], "bitRate", 7 ) == 0)
        {
            pInput->record.bitRate = value;
        }
        else if (strncmp( &variable_name_value[4], "latency", 7 ) == 0)
        {
            pInput->record.latency = value;
        }
    }
    else if (strstr( variable_name_value, "pb0" ))
    {
        if (debug) {printf( "%s: for pos (%s), value (%u)<br>\n", __FUNCTION__, pos, value ); }
        if (strncmp( &variable_name_value[3], "numPlaybacks", 12 ) == 0)
        {
            if (value <= pInput->playback.max)
            {
                pInput->playback.number = value;
            }
            else
            {
                pInput->playback.number = pInput->playback.max;
                printf( "~ALERT~Number of playbacks cannot exceed %u~", pInput->playback.max );
            }
        }
        else if (strncmp( &variable_name_value[3], "size", 4 ) == 0)
        {
            pInput->playback.size = value;
        }
    }
    else if (strstr( variable_name_value, "msg0" ))
    {
        if (debug) {printf( "%s:%d for varname (%s), value (%u)<br>\n", __FUNCTION__, __LINE__, variable_name_value, value ); }
        if (strncmp( &variable_name_value[4], "numMessages", 11 ) == 0)
        {
            if (value <= pInput->message.max)
            {
                pInput->message.number = value;
            }
            else
            {
                pInput->message.number = pInput->message.max;
                printf( "~ALERT~Number of messages cannot exceed %u~", pInput->message.max );
            }
        }
        else if (strncmp( &variable_name_value[4], "size", 4 ) == 0)
        {
            pInput->message.size = value;
            PRINTF( "%s: msg0 size %u<br>\n", __FUNCTION__, value );
        }
    }
    else if (strstr( variable_name_value, "live0" ))
    {
        if (debug) {printf( "%s:%d for varname (%s), value (%u)<br>\n", __FUNCTION__, __LINE__, variable_name_value, value ); }
        if (strncmp( &variable_name_value[5], "numLiveChannels", 15 ) == 0)
        {
            if (value <= pInput->live.max)
            {
                pInput->live.number = value;
            }
            else
            {
                pInput->live.number = pInput->live.max;
                printf( "~ALERT~Number of live channels cannot exceed %u~", pInput->live.max );
            }
        }
        else if (strncmp( &variable_name_value[5], "size", 4 ) == 0)
        {
            pInput->live.size = value;
        }
    }
    else if (strstr( variable_name_value, "rmux0" ))
    {
        if (debug) {printf( "%s:%d for varname (%s), value (%u)<br>\n", __FUNCTION__, __LINE__, variable_name_value, value ); }
        if (strncmp( &variable_name_value[5], "numRemuxes", 10 ) == 0)
        {
            PRINTF( "%s: remux value %u; max %u\n", __FUNCTION__, value, pInput->remux.max );
            if (value <= pInput->remux.max)
            {
                pInput->remux.number = value;
                PRINTF( "%s: remux value now is %u; max %u\n", __FUNCTION__, value, pInput->remux.max );
            }
            else
            {
                pInput->remux.number = pInput->remux.max;
                PRINTF( "%s: remux value max is %u; max %u\n", __FUNCTION__, value, pInput->remux.max );
                printf( "~ALERT~Remux number cannot exceed %u~",  pInput->remux.max );
            }
        }
        else if (strncmp( &variable_name_value[5], "size", 4 ) == 0)
        {
            pInput->remux.size = value;
            PRINTF( "%s: remux size now is %u\n", __FUNCTION__, value );
        }
    }
    else if (strstr( variable_name_value, "gfx" ))
    {
        unsigned int gfx = 0;
        debug = 1;
        sscanf( &variable_name_value[3], "%u", &gfx );
        if (debug) {printf( "%s:%d for varname (%s), value (%u); gfx (%u)<br>\n", __FUNCTION__, __LINE__, variable_name_value, value, gfx ); }
        if (strncmp( &variable_name_value[4], "used", 4 ) == 0)
        {
            pBoxModeSettings->graphics[gfx].used = value;
            /* if turning usage on, make sure numFrameBufs is non-zero */
            if (pBoxModeSettings->graphics[gfx].used)
            {
                if (pInput->surfaceSettings[gfx].numFrameBufs == 0)
                {
                    pInput->surfaceSettings[gfx].numFrameBufs = 2;
                }
                if (pInput->surfaceSettings[gfx].width == 0)
                {
                    pInput->surfaceSettings[gfx].width = 640;
                }
                if (pInput->surfaceSettings[gfx].height == 0)
                {
                    pInput->surfaceSettings[gfx].height = 480;
                }
                if (pInput->surfaceSettings[gfx].bits_per_pixel == 0)
                {
                    pInput->surfaceSettings[gfx].bits_per_pixel = 16;
                }
            }
        }
        else if (strncmp( &variable_name_value[4], "maxWidth", 8 ) == 0)
        {
            pInput->surfaceSettings[gfx].width = value;
        }
        else if (strncmp( &variable_name_value[4], "maxHeight", 9 ) == 0)
        {
            pInput->surfaceSettings[gfx].height = value;
        }
        else if (strncmp( &variable_name_value[4], "numberOfFb", 10 ) == 0)
        {
            if (( value != 0 ) && ( value !=2 ) && ( value != 3 ))
            {
                printf( "~ALERT~Number of frame buffers must be either 2 or 3!~" );
                pInput->surfaceSettings[gfx].numFrameBufs = 2;
            }
            else
            {
                pInput->surfaceSettings[gfx].numFrameBufs = value;
            }

            /* if turning off all frame buffers, set used flag to off */
            if (pInput->surfaceSettings[gfx].numFrameBufs == 0)
            {
                pBoxModeSettings->graphics[gfx].used = 0;
            }
            PRINTF( "%s: gfx %u; numFrameBufs %u<br>\n", __FUNCTION__, gfx, pInput->surfaceSettings[gfx].numFrameBufs );
        }
        else if (strncmp( &variable_name_value[4], "bitsPerPixel", 12 ) == 0)
        {
            if (( value != 8 ) && ( value !=16 ) && ( value != 32 ))
            {
                printf( "~ALERT~Bits per pixel must be either 8, 16, or 32!~" );
                pInput->surfaceSettings[gfx].bits_per_pixel = 8;
            }
            else
            {
                pInput->surfaceSettings[gfx].bits_per_pixel = value;
            }
        }
    }
    else if (strstr( variable_name_value, "adec" ) || strstr( variable_name_value, "aenc" ))
    {
        unsigned int adec = 0, codec = 0;
        sscanf( &variable_name_value[4], "%u", &adec );
        PRINTF( "%s: adec (%u); for pos (%s), value (%u)\n", __FUNCTION__, adec, pos, value );
        if (strncmp( &variable_name_value[5], "used", 4 ) == 0)
        {
            pInput->audioDecoder[adec].enabled = value;
            PRINTF( "%s: adec (%u) used now is %u\n", __FUNCTION__, adec, value );
#if NEXUS_HAS_VIDEO_ENCODER
            /* if decoder used for transcoding, turn off transcoder */
            if (adec>0)
            {
                unsigned int nonEncoderCount = get_non_video_encoder_count( pBoxModeSettings );
                unsigned     encoderIdx      = adec - 1;

                pSettings->videoEncoder[encoderIdx].used = value;
                pInput->encoders[encoderIdx].enabled     = value;

                PRINTF( "~%s: nonEncoderCount %u; encoderIdx %u\n~", __FUNCTION__, nonEncoderCount, encoderIdx );
                pSettings->videoDecoder[nonEncoderCount+encoderIdx].used = value;
                pInput->videoDecoder[nonEncoderCount+encoderIdx].enabled = value;
                PRINTF( "~%s: videoDecoder[%u] %u; encoders[%u] %u; audioDecoder[%u] %u\n~", __FUNCTION__, nonEncoderCount+encoderIdx,
                    pSettings->videoDecoder[nonEncoderCount+encoderIdx].used,
                    encoderIdx, pInput->encoders[encoderIdx].enabled, adec, pInput->audioDecoder[adec].enabled );
            }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
        }
        else if (strncmp( &variable_name_value[5], "codec", 5 ) == 0)
        {
            sscanf( &variable_name_value[10], "%u", &codec );
            if (codec < NEXUS_AudioCodec_eMax)
            {
                printf( "adec[%d].codec[%d] is %d<br>\n", adec, codec, value );
                if (adec==0)
                {
                    pSettings->audio.decodeCodecEnabled[codec] = value;
                }
                else
                {
                    pSettings->audio.encodeCodecEnabled[codec] = value;
                }
            }
        }
        else if (strncmp( &variable_name_value[5], "postpro", 7 ) == 0)
        {
            unsigned int postpro = 0;
            /* adec0postpro00:1 */
            sscanf( &variable_name_value[12], "%u", &postpro );
            if (postpro < NEXUS_AudioPostProcessing_eMax)
            {
                printf( "adec[%d].postpro[%d] is %d<br>\n", adec, postpro, value );
                pSettings->audio.postProcessingEnabled[postpro] = value;
            }
        }
        else if (strncmp( &variable_name_value[5], "passthru", 8 ) == 0)
        {
            pInput->audioDecoder[adec].bPassthru = value;
        }
        else if (strncmp( &variable_name_value[5], "secondary", 9 ) == 0)
        {
            pInput->audioDecoder[adec].bSecondary = value;
        }
        else if (strncmp( &variable_name_value[5], "ip", 2 ) == 0)
        {
            pInput->audioDecoder[adec].bIp = value;
        }
        else if (strncmp( &variable_name_value[5], "secure", 6 ) == 0)
        {
            pInput->audioDecoder[adec].bSecure = value;
            PRINTF( "%s: adec[%u]->secure %u\n", __FUNCTION__, adec, pInput->audioDecoder[adec].bSecure );
        }
        else if (strncmp( &variable_name_value[5], "fcc", 3 ) == 0)
        {
            pInput->audioDecoder[adec].numFcc = value;
            PRINTF( "%s: adec[%u]->numFcc %u\n", __FUNCTION__, adec, value );
        }
        else if (strncmp( &variable_name_value[5], "maxIndependentDelay", 19 ) == 0)
        {
            pSettings->audio.maxIndependentDelay = value;
        }
        else if (strncmp( &variable_name_value[5], "maxDecoderOutputChannels", 24 ) == 0)
        {
            pSettings->audio.maxDecoderOutputChannels = value;
            PRINTF( "%s: adec[%u]->maxDecoderOutputChannels %u\n", __FUNCTION__, adec, pSettings->audio.maxDecoderOutputChannels );
        }
        else if (strncmp( &variable_name_value[5], "maxDecoderOutputSamplerate", 26 ) == 0)
        {
            pSettings->audio.maxDecoderOutputSamplerate = value;
        }
        else if (strncmp( &variable_name_value[5], "numDecoders", 11 ) == 0)
        {
            pSettings->audio.numDecoders = value;
        }
        else if (strncmp( &variable_name_value[5], "numPassthroughDecoders", 22 ) == 0)
        {
            pSettings->audio.numPassthroughDecoders = value;
        }
        else if (strncmp( &variable_name_value[5], "numHbrPassthroughDecoders", 25 ) == 0)
        {
            pSettings->audio.numHbrPassthroughDecoders = value;
        }
        else if (strncmp( &variable_name_value[5], "numDspMixers", 12 ) == 0)
        {
            pSettings->audio.numDspMixers = value;
        }
        else if (strncmp( &variable_name_value[5], "numPostProcessing", 17 ) == 0)
        {
            pSettings->audio.numPostProcessing = value;
        }
        else if (strncmp( &variable_name_value[5], "numEchoCancellers", 17 ) == 0)
        {
            pSettings->audio.numEchoCancellers = value;
        }
        else if (strncmp( &variable_name_value[5], "dolbyCodecVersion", 17 ) == 0)
        {
            pSettings->audio.dolbyCodecVersion = value;
            PRINTF( "%s: dolbyCodecVersion %u\n", __FUNCTION__, pSettings->audio.dolbyCodecVersion );
        }
    }
    else if (strstr( variable_name_value, "g3d" ))
    {
        if (strncmp( &variable_name_value[4], "used", 4 ) == 0)
        {
            pBoxModeSettings->graphics3d.used = value;
        }
    }
    else if (strstr( variable_name_value, "sid" ))
    {
        if (strncmp( &variable_name_value[4], "enabled", 4 ) == 0)
        {
            pInput->sidSettings.enabled = value;
        }
    }
    else if (strstr( variable_name_value, "heap" ))
    {
        unsigned int heapIdx = 0;
        float        value   = 0;
        sscanf( &variable_name_value[4], "%u", &heapIdx );
        if (heapIdx < NEXUS_MAX_HEAPS)
        {
            if (strncmp( &variable_name_value[5], "addl", 4 ) == 0)
            {
                sscanf( pos, "%f", &value );
                heap_padding[heapIdx] = value * 1024 * 1024;
                PRINTF( "%s: heap_padding[%u] = %lu\n", __FUNCTION__, heapIdx, heap_padding[heapIdx] );
            }
        }
    }
    else if (strstr( variable_name_value, "vinp" ))
    {
        if (strncmp( &variable_name_value[5], "hdDvi", 5 ) == 0)
        {
            pSettings->videoInputs.hdDvi = value;
        }
        else if (strncmp( &variable_name_value[5], "ccir656", 7 ) == 0)
        {
            pSettings->videoInputs.ccir656 = value;
        }
    }
    else
    {
        rc = -1;
    }

    return( rc );
} /* update_settings */

#if 0
static int print_settings(
    NEXUS_MemoryConfigurationSettings *pSettings
    )
{
    unsigned int i, j;

    printf( "~SETTINGS~" );
#if NEXUS_HAS_VIDEO_DECODER
    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        printf( VDEC_FORMAT, i, pSettings->videoDecoder[i].used, pSettings->videoDecoder[i].maxFormat,
            pSettings->videoDecoder[i].colorDepth, pSettings->videoDecoder[i].mosaic.maxNumber, pSettings->videoDecoder[i].mosaic.maxWidth,
            pSettings->videoDecoder[i].mosaic.maxHeight, pSettings->videoDecoder[i].avc51Supported );
        for (j = 0; j<NEXUS_VideoCodec_eMax; j++)
        {
            printf( "%d", pSettings->videoDecoder[i].supportedCodecs[j] );
        }
    }
#endif /* if NEXUS_HAS_VIDEO_DECODER */
#if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        printf( "&disp%d=%d", i, pSettings->display[i].maxFormat );
        if (pSettings->display[i].maxFormat != NEXUS_VideoFormat_eUnknown)
        {
            for (j = 0; j<NEXUS_NUM_VIDEO_WINDOWS; j++)
            {
                printf( "win%d=%d%d%d%d%d%03d,", j, pSettings->display[i].window[j].used, pSettings->display[i].window[j].deinterlacer,
                    pSettings->display[i].window[j].capture, pSettings->display[i].window[j].convertAnyFrameRate,
                    pSettings->display[i].window[j].precisionLipSync, pSettings->display[i].window[j].userCaptureBufferCount );
            }
        }
    }
#endif /* if NEXUS_HAS_DISPLAY && NEXUS_NUM_VIDEO_WINDOWS */

    printf( "~" );
    return( 0 );
} /* print_settings */

#endif /* if 0 */

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
} /* compute_max_video_decoders */

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
            formatName = lookup_name( g_videoFormatStrs, maxFormat );
            PRINTF( "vdec %u: used %u; maxFormat %u; name (%s); sizeof(strs) %u<br>\n", i, pSettings->videoDecoder[i].used, maxFormat,
                formatName, sizeof( g_videoFormatStrs )/sizeof( g_videoFormatStrs[0] ));

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
            formatName = lookup_name( g_videoFormatStrs, maxFormat );
            PRINTF( "vdec %u: used %u; maxFormat %u; name (%s); sizeof(strs) %u<br>\n", i, pSettings->stillDecoder[i].used, maxFormat,
                formatName, sizeof( g_videoFormatStrs )/sizeof( g_videoFormatStrs[0] ));

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
            formatName = lookup_name( g_videoFormatStrs, maxFormat );
        }
        PRINTF( "display %u: maxFormat %u; name (%s); sizeof(strs) %u<br>\n", i, maxFormat, formatName,
            sizeof( g_videoFormatStrs )/sizeof( g_videoFormatStrs[0] ));

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
} /* compute_max_video_formats */

static int notify_of_fail(
    const char *functionName,
    int         rc
    )
{
    PRINTF( "%s: name (%s); rc %d<br>\n", __FUNCTION__, functionName, rc );
    if (rc != 0)
    {
        printf( "~FATAL~%s returned error code %d~", functionName, rc );
        fflush( stdout ); fflush( stderr );

        printf( "%s: NEXUS_Platform_Uninit\n", __FUNCTION__ );
        NEXUS_Platform_Uninit();

        exit( -1 );
    }
    /*BDBG_ASSERT( !rc );*/ /* calling ASSERT prevents any response from being sent to the browser */
    return( 0 );
} /* notify_of_fail */

static bool Memconfig_SecureHeapUsed(
    void
    )
{
    int                         heapIdx           = 0;
    int                         rc                = 0;
    unsigned long int           secureHeapTotalMb = 0; /* chips may or may not have secure heap */
    NEXUS_HeapHandle            heap;
    NEXUS_PlatformStatus        platformStatus;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_MemoryStatus          status;

    BKNI_Memset( &platformStatus, 0, sizeof( platformStatus ));
    BKNI_Memset( &platformConfig, 0, sizeof( platformConfig ));

    NEXUS_Platform_GetStatus( &platformStatus );

    /* call NEXUS_Platform_GetConfiguration to get the heap handles */
    NEXUS_Platform_GetConfiguration( &platformConfig );

    for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
    {
        Memconfig_HeapInfo heapInfo;

        BKNI_Memset( &heapInfo, 0, sizeof( heapInfo ));
        BKNI_Memset( &status, 0, sizeof( status ));

        heap = platformConfig.heap[heapIdx];
        PRINTF( "%s: heapIdx %u; heap %p<br>\n", __FUNCTION__, heapIdx, (void *) heap );
        if (!heap)
        {
            continue;
        }

        rc = NEXUS_Heap_GetStatus( heap, &status );
        PRINTF( "%s: heapIdx %u; status.size %u<br>\n", __FUNCTION__, heapIdx, status.size );
        BDBG_ASSERT( !rc );

        if (Memconfig_GetHeapInfo( heapIdx, &heapInfo ) == 0)
        {
            PRINTF( "%s: heap name (%s); size %u<br>\n", __FUNCTION__, heapInfo.heapName, status.size );
            if (isHeapSecure( heapIdx ))
            {
                secureHeapTotalMb = status.size/1024/1024;
                if (secureHeapTotalMb > 0)
                {
                    break;
                }
            }
        }
    }

    PRINTF( "%s: returning %s<br>\n", __FUNCTION__, ( secureHeapTotalMb > 0 ) ? "true" : "false" );
    return( secureHeapTotalMb > 0 );
} /* Memconfig_SecureHeapUsed */

static int readFileFromBrowser2(
    const char *contentType,
    const char *contentLength
    )
{
    int   ContentLengthInt;
    char *boundaryString = NULL;
    char *cgi_query      = NULL;
    FILE *fOut           = NULL;
    char  stateFilenameRestored[STATE_FILE_FULL_PATH_LEN];

    /* How much data do we expect? */

    printf( "Content-type: text/html\n\n" );
    PRINTF( "len (%s); type (%s); statefile (%s)<br>\n", contentLength, contentType, state_filename );
    if (( contentLength == NULL ) ||
        ( sscanf( contentLength, "%d", &ContentLengthInt ) != 1 ))
    {
        PRINTF( "could not sscanf CONTENT_LENGTH<br>\n" );
        return( -1 );
    }
    /* Create space for it: */

    cgi_query = malloc( ContentLengthInt + 1 );
    PRINTF( "malloc'ed space for %d bytes<br>\n", ContentLengthInt + 1 );

    if (cgi_query == NULL)
    {
        PRINTF( "could not malloc(%d) bytes<br>\n", ContentLengthInt + 1 );
        return( -1 );
    }
    /* Read it in: */

    boundaryString = strstr( contentType, "boundary=" );
    if (boundaryString == NULL)
    {
        PRINTF( "could not find boundary string<br>\n" );
        return( -1 );
    }
    boundaryString += strlen( "boundary=" );

    {
        int           byteCount        = 0;
        int           fileBytes        = 0;
        int           fgetBytes        = 0;
        int           headerBytes      = 0;
        unsigned char endOfHeaderFound = 0; /* set to true when \r\n found on a line by itself */
        unsigned char endOfFileFound   = 0; /* set to true when file contents have been read */

        while (byteCount < ContentLengthInt && endOfFileFound == 0)
        {
            if (endOfHeaderFound == 0)
            {
                fgets( &cgi_query[fileBytes], ContentLengthInt + 1, stdin );
                fgetBytes  = strlen( &cgi_query[fileBytes] );
                byteCount += fgetBytes;
                PRINTF( "got %u bytes (%s)\n", fgetBytes, &cgi_query[fileBytes] );

                /* if next line has a boundary directive on it */
                if (strstr( &cgi_query[fileBytes], boundaryString ))
                {
                    headerBytes += strlen( cgi_query ) + strlen( boundaryString ) + strlen( "--" ) + strlen( "--" ) + strlen( "\r\n" ) + strlen( "\r\n" );
                    PRINTF( "found boundary (%s); len %d; headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], strlen( cgi_query ), headerBytes, ContentLengthInt - headerBytes );
                    cgi_query[fileBytes] = '\0';
                }
                else if (strncmp( &cgi_query[fileBytes], "Content-", 8 ) == 0)  /* if next line has a Content directive on it */
                {
                    headerBytes += strlen( cgi_query );
                    PRINTF( "found directive (%s); len %d; headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], strlen( cgi_query ), headerBytes, ContentLengthInt - headerBytes );
                    cgi_query[fileBytes] = '\0';
                }
                else if (( endOfHeaderFound == 0 ) && ( cgi_query[fileBytes] == '\r' ) && ( cgi_query[fileBytes + 1] == '\n' ))
                {
                    headerBytes += strlen( cgi_query );
                    PRINTF( "found end of header (%s) (skipping these bytes); headerBytes %d; remaining %d<br>\n", &cgi_query[fileBytes], headerBytes,
                        ContentLengthInt - headerBytes );
                    endOfHeaderFound     = 1;
                    cgi_query[fileBytes] = '\0';
                }
            }
            else
            {
                fgetBytes  = fread( &cgi_query[fileBytes], 1, ContentLengthInt - headerBytes, stdin );
                byteCount += fgetBytes;
                PRINTF( "added %d bytes to file contents<br>\n", fgetBytes );
                fileBytes       += fgetBytes;
                endOfHeaderFound = 0;
                endOfFileFound   = 1;
            }

            PRINTF( "byteCount %d; ContentLengthInt %d; fileBytes %d<br>\n", byteCount, ContentLengthInt, fileBytes );
        }

        PrependTempDirectory( stateFilenameRestored, sizeof( stateFilenameRestored ), STATE_FILE_RESTORED );
        PRINTF( "%s: fopen (%s)<br>\n", __FUNCTION__, stateFilenameRestored );
        fOut = fopen( stateFilenameRestored, "w" );
        if (fOut)
        {
            PRINTF( "%s: fwrite (%d) bytes<br>\n", __FUNCTION__, fileBytes );
            fwrite( cgi_query, 1, fileBytes, fOut );
            fclose( fOut );
            PRINTF( "<h3>Output file is (%s)</h3>\n", stateFilenameRestored );
        }
        /* free the malloc'ed space if it was malloc'ed successfully */
        if (cgi_query)
        {
            free( cgi_query );
        }
    }

    return( 0 );
} /* readFileFromBrowser2 */

int main(
    int   argc,
    char *argv[]
    )
{
    NEXUS_PlatformSettings            platformSettings;
    NEXUS_PlatformStatus              platformStatusPrevious;
    NEXUS_MemoryConfigurationSettings memConfigSettings;
    NEXUS_MemoryConfigurationSettings memConfigSettings2; /* used to zero out audio codecs when all adecs are disabled */
    Memconfig_AppUsageSettings        transInput;
    Memconfig_AppMemUsage             transOutput;
    Memconfig_AppMemUsage             transOutputPrevious;
    Memconfig_BoxMode                 boxModeSettings;
    int      i;
    int      rc;
    unsigned codec;
    int      boxmode = 0;
    int      pagenum = 0;
    int      helpnum = 0;
    char    *queryString;
    char    *contentType;
    char    *contentLength;
    char    *remoteAddress;
    char    *pos = NULL;
    char     boxmodeStr[5];
    bool     useNexusMemconfigDefaults = false;
    char     datetime[20];
    char     stateFilenameRestored[STATE_FILE_FULL_PATH_LEN];
    char     strTemp[128];

    BKNI_Init();
    BDBG_Init();

    BKNI_Memset( &platformSettings, 0, sizeof( platformSettings ));
    BKNI_Memset( &platformStatusPrevious, 0, sizeof( platformStatusPrevious ));
    BKNI_Memset( &memConfigSettings, 0, sizeof( memConfigSettings ));
    BKNI_Memset( &transInput, 0, sizeof( transInput ));
    BKNI_Memset( &transOutput, 0, sizeof( transOutput ));
    BKNI_Memset( &transOutputPrevious, 0, sizeof( transOutputPrevious ));
    BKNI_Memset( &boxModeSettings, 0, sizeof( boxModeSettings ));
    BKNI_Memset( &heap_padding, 0, sizeof( heap_padding ));
    BKNI_Memset( &heap_usage, 0, sizeof( heap_usage ));
    BKNI_Memset( datetime, 0, sizeof( datetime ));
    BKNI_Memset( strTemp, 0, sizeof( strTemp ));
    BKNI_Memset( videoDecodeCodecEnabledDefaults, 0, sizeof( videoDecodeCodecEnabledDefaults ));
    BKNI_Memset( stillDecodeCodecEnabledDefaults, 0, sizeof( stillDecodeCodecEnabledDefaults ));
    BKNI_Memset( audioDecodeCodecEnabledDefaults, 0, sizeof( audioDecodeCodecEnabledDefaults ));
    BKNI_Memset( audioEncodeCodecEnabledDefaults, 0, sizeof( audioEncodeCodecEnabledDefaults ));

    /* added this ifdef to help simplify debugging why tool does not work in user mode; works fine in kernel mode. */
    #if 1
    queryString   = getenv( "QUERY_STRING" );
    contentType   = getenv( "CONTENT_TYPE" );
    contentLength = getenv( "CONTENT_LENGTH" );

    if (queryString == NULL)
    {
        /* if query string is null, check to see if we are being sent a multipart file */
        contentType = getenv( "CONTENT_TYPE" );
        if (contentType == NULL)
        {
            printf( "Content-type: text/html\n\n" );
            printf( "<h3>QUERY_STRING and CONTENT_TYPE must not both be null</h3>\n" );
            goto end;
        }

        contentLength = getenv( "CONTENT_LENGTH" );
        if (contentLength == NULL)
        {
            printf( "Content-type: text/html\n\n" );
            printf( "<h3>CONTENT_LENGTH is null</h3>\n" );
        }
    }

    remoteAddress = getenv( "REMOTE_ADDR" );
    if (remoteAddress== NULL)
    {
        unsigned int strlength = strlen( "unknown" ) + 1;
        remoteAddress = malloc( strlength );
        strncpy( remoteAddress, "unknown", strlength );
    }

    strncpy( state_filename, GetTempDirectoryStr(), sizeof( state_filename ));
    strncat( state_filename, OUTPUT_STATE_FILE, sizeof( state_filename ));
    strncat( state_filename, ".", sizeof( state_filename ));
    strncat( state_filename, remoteAddress, sizeof( state_filename ));
    strncat( state_filename, ".txt", sizeof( state_filename ));

    /* this is a special request to upload the state file. once uploaded, we need to exit. */
    if (queryString && strstr( queryString, "variable=saveState" ))
    {
        sendFileToBrowser( state_filename );
        return( 0 );
    }
    /* if the browser is sending us a file */
    else if (contentType && strstr( contentType, "multipart" ))
    {
        readFileFromBrowser2( contentType, contentLength );

        /* refresh the page that brought us here */
        printf( "<META HTTP-EQUIV=refresh CONTENT=\"0;URL=/bmemconfig.html\">\n" );

        goto end;
    }

    printf( "Content-type: text/html\n\n" );
    BDBG_LOG(("%s: NEXUS_MAX_MEMC %d; NEXUS_NUM_MEMC %d ", argv[0], NEXUS_MAX_MEMC, NEXUS_NUM_MEMC ));

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
    scanForTag( queryString, "boxmode=", &boxmode );
    scanForTag( queryString, "page=", &pagenum );
    scanForTag( queryString, "=pageHelp", &helpnum );
    scanForStr( queryString, "datetime=", sizeof( datetime ), datetime );
    PRINTF( "%s: argc %d; helpnum %u\n", argv[0], argc, helpnum );

    /* if browser provided a new date/time value */
    if (strlen( datetime ))
    {
        char cmd[64];
        sprintf( cmd, "export TZ=\"GST-8\" && date %s > /dev/null", datetime );
        system( cmd );
    }

    if (boxmode == -1)
    {
#define BOXMODE_DROPDOWN_SIZE 2048
        struct stat statbuf;
        char        buf[BOXMODE_DROPDOWN_SIZE];

        /* delete the existing state file */
        remove( state_filename );

        printf( "%s: boxmode %d; pagenum %d\n", argv[0], boxmode, pagenum );

        useNexusMemconfigDefaults = true;

        transInput.surfaceSettings[0].teletext = 1;
        transInput.surfaceSettings[1].teletext = 1;
        PRINTF( "%s: surfaceSettings[0].teletext %d; surfaceSettings[0].teletext %d\n", argv[0], transInput.surfaceSettings[0].teletext,
            transInput.surfaceSettings[1].teletext );

        PRINTF( "lstat(%s)\n", stateFilenameRestored );
        /* see if a state file is being restored by the user */
        if (lstat( stateFilenameRestored, &statbuf ) == -1)
        {
            printf( "%s:%u no state file\n", argv[0], __LINE__ );
            /* could not find state file; read the box mode from nexus */
            boxmode = ReadBoxMode();
            printf( "%s: nexus boxmode %d; pagenum %d\n", argv[0], boxmode, pagenum );
        }
        else
        {
            char *fileContents = NULL;

            printf( "%s:%u\n", argv[0], __LINE__ );
            /* replace state file with the one the user is trying to restore */
            printf( "%s: restoring existing state file (%s) with new state file (%s)\n", argv[0], state_filename, stateFilenameRestored );

            fileContents = getFileContents( stateFilenameRestored );
            if (fileContents)
            {
                int rc = 0;

                sscanf( fileContents, "boxmode %d", &boxmode );
                printf( "%s: restoring ... sscanf boxmode %d; pagenum %d\n", argv[0], boxmode, pagenum );
                free( fileContents );

                rc = Memconfig_GetBoxModeDefaultSettings( boxmode, &boxModeSettings );
                /* if the boxmode specified in the file is not known to this platform */
                if (rc == -1)
                {
                    printf( "~ALERT~The boxmode you are trying to restore (%d) is not valid for this platform!~", boxmode );

                    remove( stateFilenameRestored );

                    boxmode = ReadBoxMode();
                    printf( "%s: nexus boxmode %d; pagenum %d\n", argv[0], boxmode, pagenum );
                }
                else
                {
                    /* use the restored state file */
                    rename( stateFilenameRestored, state_filename );

                    /* do not use Nexus memconfig default padding values; use the ones specified in the state file */
                    useNexusMemconfigDefaults = false;
                }

                BKNI_Memset( &boxModeSettings, 0, sizeof( boxModeSettings ));
            }
        }

        printf( "~PLATFORM~%s~", getPlatform());
        printf( "~PLATVER~%s~", getPlatformVersion());
        printf( "~BOXMODE~%d~", boxmode );

        getBoxModeDropdown( buf, sizeof( buf ), boxmode );
        printf( "~BOXMODEHTML~%s~", buf );
    }

    {
        int fboxmode;
        if (( fboxmode = read_boxmode_from_state_file()) != boxmode)
        {
            PRINTF( "%s: changing box mode: local %u; file %u\n", argv[0], boxmode, fboxmode );
            useNexusMemconfigDefaults = true;

            /* teletext for Main and PIP are enabled by default */
            transInput.surfaceSettings[0].teletext = 1;
            transInput.surfaceSettings[1].teletext = 1;
            PRINTF( "%s: surfaceSettings[0].teletext %d; surfaceSettings[0].teletext %d\n", argv[0], transInput.surfaceSettings[0].teletext,
                transInput.surfaceSettings[1].teletext );
        }
    }

    sprintf( boxmodeStr, "%u", boxmode );
    NEXUS_SetEnv( "B_REFSW_BOXMODE", boxmodeStr );

    Memconfig_GetBoxModeDefaultSettings( boxmode, &boxModeSettings );

    BDBG_ERR(("%d calling NEXUS_Platform_GetDefaultSettings() ", __LINE__ ));
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

    BDBG_ERR(("%d calling NEXUS_GetDefaultMemoryConfigurationSettings() ", __LINE__ ));
    NEXUS_GetDefaultMemoryConfigurationSettings( &memConfigSettings );

    /* save the default video decoder codecs to help us NOT display codecs that are initially turned off by default */
    for (codec = 0; codec<NEXUS_VideoCodec_eMax; codec++)
    {
        /* sometimes (7250) the codecs for Main differ from the ones for PIP (... no MVC) */
        for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++)
        {
            videoDecodeCodecEnabledDefaults[i][codec] = memConfigSettings.videoDecoder[i].supportedCodecs[codec];
        }
        for (i = 0; i<NEXUS_MAX_STILL_DECODERS; i++)
        {
            stillDecodeCodecEnabledDefaults[i][codec] = memConfigSettings.stillDecoder[i].supportedCodecs[codec];
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

    read_settings( &memConfigSettings, &transInput, &boxModeSettings, &transOutputPrevious, &platformStatusPrevious );

    if (( pos = strstr( queryString, "variable=" )))
    {
        update_settings( &memConfigSettings, &transInput, pos+strlen( "variable=" ), &boxModeSettings );
    }

    /* a few settings need to be copied from memconfigSettings to transInput */
    transfer_settings( &memConfigSettings, &transInput, &transOutput, &boxModeSettings );

    /* save the settings that will be used to write to state machine */
    memcpy( &memConfigSettings2, &memConfigSettings, sizeof( memConfigSettings ));

    /* if all audio decoders are disabled, clear out pSettings codecs */
    update_audio_decoder_codecs( &memConfigSettings, &transInput );

    BDBG_LOG(( "~%s: NEXUS_SetEnv( NEXUS_BASE_ONLY_INIT, y ); used to set baseOnlyInit ~", argv[0] ));
    NEXUS_SetEnv( "NEXUS_BASE_ONLY_INIT", "y" );
    BDBG_LOG(( "~%s: NEXUS_BASE_ONLY_INIT: calling NEXUS_Platform_MemConfigInit(); uses baseOnlyInit~", argv[0] ));
    BDBG_ERR(("%d calling NEXUS_Platform_MemConfigInit() ", __LINE__ ));
    rc = NEXUS_Platform_MemConfigInit( &platformSettings, &memConfigSettings );
    BDBG_LOG(( "~%s: NEXUS_BASE_ONLY_INIT: NEXUS_Platform_MemConfigInit() returned %d; uses baseOnlyInit~", argv[0], rc ));

    notify_of_fail( "NEXUS_Platform_MemConfigInit", rc );

    /* restore the settings that will be used to write to state machine */
    memcpy( &memConfigSettings, &memConfigSettings2, sizeof( memConfigSettings ));

    transInput.bSecure = Memconfig_SecureHeapUsed();
    PRINTF( "~%s: transInput.bSecure %u\n~", argv[0], transInput.bSecure );

    /* if audio decoders are disabled ... disable video encoders */
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        memConfigSettings.audio.numEchoCancellers = NEXUS_NUM_HDMI_OUTPUTS;
#if NEXUS_HAS_VIDEO_ENCODER
        if (i > 0)
        {
            PRINTF( "~%s: videoEncoder[%u] %u; audioDecoder[%u] %u;\n~", argv[0], i-1, memConfigSettings.videoEncoder[i-1].used, i, transInput.audioDecoder[i].enabled );
            transInput.audioDecoder[i].enabled       |= memConfigSettings.videoEncoder[i-1].used;
            memConfigSettings.videoEncoder[i-1].used |= transInput.audioDecoder[i].enabled;
        }
#endif /* if NEXUS_HAS_VIDEO_ENCODER */
    }

    Memconfig_AppUsageCalculate( &transInput, &transOutput );

    BDBG_ERR(("%d calling print_heaps() ", __LINE__ ));
    print_heaps( &transOutput, &transInput, &boxModeSettings, &memConfigSettings, &transOutputPrevious, &platformStatusPrevious,
        useNexusMemconfigDefaults );
    BDBG_ERR(("%d print_heaps() done", __LINE__ ));

    write_settings( &memConfigSettings, &transInput, &boxModeSettings, &transOutput, &platformStatusPrevious );

    /* if user selected a tag linked to a faq on the help page, switch pages */
    if (helpnum > 0)
    {
        pagenum = 11;
    }

    switch (pagenum)
    {
        case 1:
        {page_home(); break; }

        case 2:
        {page_transport( &memConfigSettings, &transInput ); break; }

        case 3:
        {page_video_decoder( &boxModeSettings, &memConfigSettings, &transInput ); break; }

        case 4:
        {page_audio_decoder( &memConfigSettings, &boxModeSettings, &transInput ); break; }

        case 5:
        {page_display( &boxModeSettings, &memConfigSettings, &transInput ); break; }

        case 6:
        {page_graphics( &boxModeSettings, &transInput ); break; }

        case 7:
        {page_encoder( &boxModeSettings, &memConfigSettings, &transInput ); break; }

        case 8:
        {if (pos) {page_file_management( pos+strlen( "variable=" ), &memConfigSettings, &boxModeSettings ); } break; }

        case 9:
        {page_picture_decoder( &transInput ); break; }

        case 10:
        {page_video_input( &memConfigSettings ); break; }

        case 11:
        {page_help( helpnum ); break; }
    } /* switch */

    {
        unsigned int heapIdx;
        for (heapIdx = 0; heapIdx<NEXUS_MAX_HEAPS; heapIdx++)
        {
            if (g_heap_info[heapIdx].memoryType != NULL)
            {
                free( g_heap_info[heapIdx].memoryType );
            }
        }
    }

    NEXUS_Platform_Uninit();

end:
    printf( "</body></html>" );
    #else /* if 1 */
    boxmode = ReadBoxMode();
    printf( "ReadBoxMode %d\n", boxmode );

    sprintf( boxmodeStr, "%u", boxmode );
    NEXUS_SetEnv( "B_REFSW_BOXMODE", boxmodeStr );
    printf( "NEXUS_SetEnv(B_REFSW_BOXMODE=%u)\n", boxmode );

    printf( "calling NEXUS_Platform_GetDefaultSettings()\n" );
    NEXUS_Platform_GetDefaultSettings( &platformSettings );
    printf( "after NEXUS_Platform_GetDefaultSettings(), errno %d (%s)\n", errno, strerror( errno ));

    NEXUS_Platform_Uninit();
    fflush( stdout ); fflush( stderr );
    printf( "all done\n" );
    #endif /* if 1 */

    return( 0 );
} /* main */
