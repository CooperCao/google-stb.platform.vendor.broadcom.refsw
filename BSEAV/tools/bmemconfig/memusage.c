/******************************************************************************
 *    (c)2008-2014 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its licensors,
 * and may only be used, duplicated, modified or distributed pursuant to the terms and
 * conditions of a separate, written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 * no license (express or implied), right to use, or waiver of any kind with respect to the
 * Software, and Broadcom expressly reserves all rights in and to the Software and all
 * intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 * HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 * NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 * secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 * OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 * LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 * OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 * USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 * LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 * EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 * USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 * ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 * LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 * ANY LIMITED REMEDY.
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
 *****************************************************************************/

#include "nexus_platform.h"
#include "nexus_core_utils.h"
#include "nexus_surface.h"
#include "bstd.h"
#include "bkni.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "memusage.h"
typedef enum Memconfig_XCBufType {
    Memconfig_XCBufType_eRave = 0,
    Memconfig_XCBufType_eMsg,
    Memconfig_XCBufType_eRmx0,
    Memconfig_XCBufType_eRmx1,
    Memconfig_XCBufType_eMax
} Memconfig_XCBufType;

typedef enum Memconfig_VidDecType {
    Memconfig_VidDecType_eMain = 0,
    Memconfig_VidDecType_ePip,
    Memconfig_VidDecType_eThird,
    Memconfig_VidDecType_eMax
} Memconfig_VidDecType;

typedef enum Memconfig_XVDBufType {
    Memconfig_XVDBufType_eGeneral = 0,
    Memconfig_XVDBufType_eCabac,
    Memconfig_XVDBufType_ePicture,
    Memconfig_XVDBufType_eMax
} Memconfig_XVDBufType;

#define PRINTF 0
/* default CDB and ITB sizes */
#define FOUR_K_CDB_SIZE           ( 5*1024*1024 )
#define   IP_CDB_SIZE             ( 10*1024*1024 )
#define   VIDEOPRIMER_CDB_SIZE    ( 8*1024*1024 )
#define   NEXUS_RECPUMP_CDB_SIZE  ( 2310212 )
#define   NEXUS_RECPUMP_ITB_SIZE  ( 18432 )
#define   NEXUS_PLAYPUMP_CDB_SIZE ( B_PVR_PLAYBACK_BUFFER )
#define BSID_MAX_CHANNELS         16
#define BSID_ARC_CODE_SIZE        512 * 1024 /* byte */
#if 0
#ifdef BSID_USE_MMA_MEMORY
#define BSID_ARC_CODE_ALIGN 1<<10  /* bit */
#else
#define BSID_ARC_CODE_ALIGN 10 /* bit */
#endif
#endif /* if 0 */
#define BSID_MBX_MEMORY_SIZE       1024       /* byte */
#define BSID_DATA_MEMORY_SIZE      100 * 1024 /* byte */
#define BSID_INPUT_DMA_MEMORY_SIZE 1024       /* byte */

#define B_PVR_ATOM_SIZE (( 188/4 )*4096 )
/* This is the size of the buffer we'll allocate for playback operations */
#define B_PVR_PLAYBACK_BUFFER ( B_PVR_ATOM_SIZE*8 ) /* = 1540096 or 1504K or 1.5MB */

#define   STILL_IMAGE_CDB_SIZE  ( 1024*1024 )
#define   STILL_IMAGE_ITB_SIZE  (( 1024*1024 )/10 )
#define   APE_CDB_SIZE          ( 256*1024 )
#define   APE_ITB_SIZE          ( 128*1024 )
#define   AUDIO_ENCODE_CDB_SIZE ( 650 *1024 )
#define   AUDIO_ENCODE_ITB_SIZE ( 325 *1024 )
#define   PLAYPUMP_SG_NUM_DESC  ( 10 )
#define   PLAYPUMP_SG_BUF_SIZE  ( 128*1024 )
#define   MESSAGE_BUF_SIZE      ( 4*1024 )
#define INPUT_BAND_BUF_SIZE     ( 200* 1024 )
#define PLAYBACK_BUF_SIZE       ( 8 * 1024 )
#define MINIMUM_BUF_SIZE        ( 256 )
/* CDB and ITB sizes */
#define B_XVD_VIDEO_CDB_SIZE ( 3*1024*1024/2 ) /* 1.5 MB for main video decode CDB */
#define B_XVD_VIDEO_ITB_SIZE ( 512*1024 )      /* 512 KB for main video decode ITB */
#define V3D_SIZE             ( 16*1024*1204 )

/**
Summary:
Input from user for
**/

typedef struct Memconfig_AppMemoryConfigurationSettings
{
    /* can only adjust number of RS and XC bufs if you change the defines in nexus_platform_features*/
    struct {
        bool     enabled;
        bool     secureHeap;      /* default main */
        unsigned maxPlaybacks;    /*  32 bxpt_num_playbacks or NEXUS_NUM_PLAYPUMPS*/
        unsigned numPlaybacks;    /* currently can only have */
        unsigned playbackBufSize; /* not configurable, set by define*/

        unsigned maxIBPIDParsers;  /*24 BXPT_NUM_PID_PARSERS or NEXUS_NUM_PARSER_BANDS */
        unsigned numIBPIDParsers;
        unsigned inputBandBufSize; /* not configurable, set by define*/

        /* For not don't show these in gui */
        bool mpodEnabled;  /* default is false. multiply the inputBand claculation for RS Buf by 2.  */

        unsigned numMpodIBPIDParsers;
        unsigned mpodInputBandBufSize;
    } RSBuf_Settings;

    /* See the different types of allocations at Memconfig_XCBufType */
    struct {
        bool     enabled;
        bool     secureHeap;   /* default main */
        unsigned maxPlaybacks; /*  32 bxpt_num_playbacks or NEXUS_NUM_PLAYPUMPS */
        unsigned numPlaybacks;
        unsigned playbackBufSize; /* not configurable, set by define*/

        unsigned maxIBPIDParsers;  /*24 BXPT_NUM_PID_PARSERS or NEXUS_NUM_PARSER_BANDS */
        unsigned numIBPIDParsers;
        unsigned inputBandBufSize; /* not configurable, set by define*/
    } XCBuf_Settings[Memconfig_XCBufType_eMax];

    unsigned maxRAVEContext;

    struct
    {
        bool enabled;
        bool secureHeap;  /* default main */
        /* off by default. turn on for these codecs:
                NEXUS_VideoCodec_eMpeg4Part2:
                NEXUS_VideoCodec_eDivx311:
                NEXUS_VideoCodec_eVc1SimpleMain: */
        bool softContext;
        bool MVC;         /*enhancementPidChannelSupported; */

        /* Enable boxes  these change the video  cdb sizes per rave context*/
        bool     ipMode;    /*10 MB looks like, set also in atlas, ipclient*/
        bool     fourKMode; /*  5MB */
        unsigned numFCCprimers;
        unsigned numMosaics; /* if mosiac is enabled thn there can't be a any regular decode*/
        /* For not don't show these in gui */
        unsigned cdbBufSize;   /* There are defaults in xpt, but is overwritten by fifosize in atlas, ip its a big number*/
        unsigned itbBufSize;
        /* defaults
                     still image
                          unsigned cdbBufSize;   default is 1MB
                          unsigned itbBufSize;   dfault is 1MB/10

                      4K:
                        cdb 5MB ITB 500Kb  soft itb 500 kb    if (videoFormatInfo.width > 1920) {
                       default to larger CDB 5MB for 4K2K, same ITB

                      FCC:   8MB larger CDB needed for AVC HD fcc
                          then determine how many primers and how many decoders. example had 1 decode and 4 primers.

                      atlas or ip_mode  believe for jitter
                          cdb: 10 MB

                       mosaic /defaults:
                          cdb: 1.5 MB                            B_XVD_VIDEO_CDB_SIZE
                          itb : 512 MB                            B_XVD_VIDEO_ITB_SIZE
                  */
    } videoRaveSettings[NEXUS_NUM_VIDEO_DECODERS];  /* main 0, pip 1, ,and third 2*/

    /* Assigning video and audio decoders to encode. Encode just means to add the audio mux and stream mux(RAVE record context) */

#if NEXUS_NUM_VIDEO_ENCODERS
    struct
    {
        bool enabled;
        bool streamMux;

        /* For not don't show these in gui */
        unsigned cdbBufSize;   /* There are defaults in xpt, but is overwritten by fifosize in atlas, ip its a big number*/
        unsigned itbBufSize;
    } encodeRaveSettings [NEXUS_NUM_VIDEO_ENCODERS]; /* depends how many */
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    struct {
        bool     enabled;
        bool     secureHeap; /* default main*/
        bool     ipMode;
        bool     passthru;
        bool     secondary;
        unsigned numFCCprimers;
        /* For not don't show these in gui */
        unsigned cdbBufSize;                       /*ape sets to 256kb */
        unsigned itbBufSize;                       /*ape sets to 128 kb*/
    } audioRaveSettings[NEXUS_NUM_AUDIO_DECODERS]; /*main, secodary, passthrough*/

    struct {
        bool     enabled;
        bool     secureHeap; /* default main*/
        unsigned numRecpumps;
        unsigned bitRate; /* in Mbps */
        unsigned latency; /* in msec/segment*/
        /*
             Recpump's data.bufferSize (aka CDB) should be chosen based on maximum stream bitrate and worst-case latency in the system and
             a minimum segmentation of the buffer (required for pipelining I/O requests). It must be big enough to avoid overflow in all cases. For instance:

                 20Mbps * 250msec/segment * 4 segments = 20 * 1024 * 1024 / 8 * 250 / 1000 * 4 = 2.5M
       NEXUS_RECPUMP_CDB_SIZE  ( 2310212 )
       NEXUS_RECPUMP_ITB_SIZE  ( 18432 )

             Recpump's index.bufferSize (aka ITB) should hold the maximum number of start codes and other SCT entries that can fit in a full
             data buffer. Worst case is a low bitrate stream. The following is an approximation:

                 2.5M CDB / 10K per picture * (16 slices/picture + 5 SC's for various overhead) = 5376 ITB's * 24 bytes/ITB = 126K


             */

        /* For not don't show these in gui */
        unsigned maxRecpumps;
        unsigned cdbBufSize;  /*default  is 2310212   */
        unsigned itbBufSize;  /* default is 18432  */
    } recordRaveSettings;

    /* this is for the AVD still  decoder */
    struct {
        bool     enabled;
        bool     secureHeap;  /* default main*/
        bool     softContext; /* on by default,*/
        unsigned numSID;
        /* For not don't show these in gui */
        unsigned maxSID;     /* channels is not give a rave context */
        unsigned cdbBufSize; /* default  is 1MB */
        unsigned itbBufSize; /* default is 1MB/10 */
    } stillImageRaveSettings;

    struct {
        bool     secureHeap; /* default main*/
        unsigned numPlaypumps;
        unsigned maxPlaypumps;
        unsigned fifoSize;  /*  1540096 */

        bool     scatterGatherMode;
        unsigned numSGDescriptors; /* default 10*/
        unsigned SGBufSize;        /*default    is */
    } nexus_playpump_settings;

    struct {
        unsigned numMessageBuffers;  /* allocated 2*/
        unsigned maxMessageBuffers;
        unsigned size;  /* default 4k form get defatult settings */
    } messageBuffers;

    struct {
        bool     enabled;
        unsigned size; /*16MB*/
    } v3dSettings;

    /* Nexus picture decoder HW */
    struct {
        bool     enabled;   /* mandatory is fw and 3 other allocations*/
        unsigned fwSize;
        unsigned maxChannels;  /*16 max channels*/
        unsigned numChannels;
        unsigned size_per_channel;
        unsigned other_allocations; /* 2*BSID_MBX_MEMORY_SIZE +BSID_INPUT_DMA_MEMORY_SIZE */
    } sidSettings;

    struct {
        bool     ttEnabled;
        bool     scteEnabled;
        unsigned ttSize; /*  2 x  778 per display */
        unsigned scteSize;
    } vbiSettings;
#if 1

    struct {
        unsigned raagaSize;
        unsigned apeSize;
    } audioSettings;

    struct {
        unsigned numFrameBufs;
        unsigned width;
        unsigned height;
        unsigned bits_per_pixel;
    } surfaceSettings[NEXUS_NUM_DISPLAYS];

    struct {
        unsigned fifoSize;
        unsigned numPacketContext;
        unsigned packetBuffer;    /* 131252  */
        unsigned packetSyncAlloc; /* 1024 */
    } grcSettings;
#endif /* if 1 */
} Memconfig_AppMemoryConfigurationSettings;
Memconfig_AppMemoryConfigurationSettings gSettings;

typedef struct Memconfig_AppMemoryConfigurationOutput
{
    unsigned mainHeapTotal;
    unsigned secureHeapTotal;
} Memconfig_AppMemoryConfigurationOutput;

/* This sets the default settings set by ReferenceSoftware*/
static void getAppMemoryConfigurationDefaultSettings(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings
    )
{
    int i;

    BKNI_Memset( pConfigSettings, 0, sizeof( *pConfigSettings ));
    /*RSBuf_Settings */
    pConfigSettings->RSBuf_Settings.enabled         = true;
    pConfigSettings->RSBuf_Settings.maxPlaybacks    = NEXUS_NUM_PLAYPUMPS;
    pConfigSettings->RSBuf_Settings.numPlaybacks    = 4;    /*NEXUS_NUM_PLAYPUMPS;*/
    pConfigSettings->RSBuf_Settings.playbackBufSize = PLAYBACK_BUF_SIZE;

    pConfigSettings->RSBuf_Settings.maxIBPIDParsers  = NEXUS_NUM_PARSER_BANDS;
    pConfigSettings->RSBuf_Settings.numIBPIDParsers  = 4;  /*NEXUS_NUM_PARSER_BANDS; */
    pConfigSettings->RSBuf_Settings.inputBandBufSize = INPUT_BAND_BUF_SIZE;

    pConfigSettings->RSBuf_Settings.mpodEnabled          = false;
    pConfigSettings->RSBuf_Settings.numMpodIBPIDParsers  = 4; /*NEXUS_NUM_PARSER_BANDS; */
    pConfigSettings->RSBuf_Settings.mpodInputBandBufSize = INPUT_BAND_BUF_SIZE;

    /*XCBuf_Settings */
    for (i = 0; i<Memconfig_XCBufType_eMax; i++)
    {
        pConfigSettings->XCBuf_Settings[i].enabled         = true;
        pConfigSettings->XCBuf_Settings[i].maxPlaybacks    = NEXUS_NUM_PLAYPUMPS;
        pConfigSettings->XCBuf_Settings[i].numPlaybacks    = 4; /*NEXUS_NUM_PLAYPUMPS; */
        pConfigSettings->XCBuf_Settings[i].playbackBufSize = PLAYBACK_BUF_SIZE;

        pConfigSettings->XCBuf_Settings[i].maxIBPIDParsers  = NEXUS_NUM_PARSER_BANDS;
        pConfigSettings->XCBuf_Settings[i].numIBPIDParsers  = 4; /*NEXUS_NUM_PARSER_BANDS; */
        pConfigSettings->XCBuf_Settings[i].inputBandBufSize = INPUT_BAND_BUF_SIZE;
    }
    /* Turn off remux by default */
    pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx0].enabled = false;
    pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx1].enabled = false;

    pConfigSettings->maxRAVEContext = NEXUS_NUM_RAVE_CONTEXTS;

    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pConfigSettings->videoRaveSettings[i].cdbBufSize = B_XVD_VIDEO_CDB_SIZE;
        pConfigSettings->videoRaveSettings[i].itbBufSize = B_XVD_VIDEO_ITB_SIZE;
    }
    pConfigSettings->videoRaveSettings[Memconfig_VidDecType_eMain].enabled = true;
    /* pConfigSettings->videoRaveSettings[Memconfig_VidDecType_ePip].enabled = true;  */

#if NEXUS_NUM_VIDEO_ENCODERS
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        pConfigSettings->encodeRaveSettings[i].cdbBufSize = B_XVD_VIDEO_CDB_SIZE;
        pConfigSettings->encodeRaveSettings[i].itbBufSize = B_XVD_VIDEO_ITB_SIZE;
    }
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    /* BAPE_Decoder_GetDefaultCdbItbConfig */
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        pConfigSettings->audioRaveSettings[i].cdbBufSize = APE_CDB_SIZE;
        pConfigSettings->audioRaveSettings[i].itbBufSize = APE_ITB_SIZE;
    }

    pConfigSettings->audioRaveSettings[0].enabled = true;

    /*NEXUS_Recpump_GetDefaultOpenSettings */
    pConfigSettings->recordRaveSettings.maxRecpumps = NEXUS_NUM_RECPUMPS;

    pConfigSettings->recordRaveSettings.numRecpumps = 4; /*NEXUS_NUM_RECPUMPS;  */
    pConfigSettings->recordRaveSettings.latency     = 250;
    pConfigSettings->recordRaveSettings.bitRate     = 20;
    pConfigSettings->recordRaveSettings.cdbBufSize  = NEXUS_RECPUMP_CDB_SIZE;
    pConfigSettings->recordRaveSettings.itbBufSize  = NEXUS_RECPUMP_ITB_SIZE;

    /*NEXUS_StillDecoder_GetDefaultOpenSettings /  NEXUS_HwStillDecoder_P_Open_Avd*/
    pConfigSettings->stillImageRaveSettings.maxSID     = NEXUS_NUM_STILL_DECODES;
    pConfigSettings->stillImageRaveSettings.numSID     = 0; /* NEXUS_NUM_STILL_DECODES;  */
    pConfigSettings->stillImageRaveSettings.cdbBufSize = STILL_IMAGE_CDB_SIZE;
    pConfigSettings->stillImageRaveSettings.itbBufSize = STILL_IMAGE_ITB_SIZE;

    /*NEXUS_Playpump_GetDefaultOpenSettings */
    pConfigSettings->nexus_playpump_settings.maxPlaypumps = NEXUS_NUM_PLAYPUMPS;
    pConfigSettings->nexus_playpump_settings.numPlaypumps = 4;  /*NEXUS_NUM_PLAYPUMPS; */

    pConfigSettings->nexus_playpump_settings.fifoSize          = B_PVR_PLAYBACK_BUFFER;
    pConfigSettings->nexus_playpump_settings.scatterGatherMode = false;
    /*defaults from examples. playpump_scatter_gather.c  */
    pConfigSettings->nexus_playpump_settings.numSGDescriptors = PLAYPUMP_SG_NUM_DESC;
    pConfigSettings->nexus_playpump_settings.SGBufSize        = PLAYPUMP_SG_BUF_SIZE;

    /*NEXUS_Message_GetDefaultSettings */
    pConfigSettings->messageBuffers.numMessageBuffers = 4;
    pConfigSettings->messageBuffers.maxMessageBuffers = NEXUS_NUM_MESSAGE_FILTERS;
    pConfigSettings->messageBuffers.size              = MESSAGE_BUF_SIZE;

    /* V3D */
    pConfigSettings->v3dSettings.size = V3D_SIZE;

    /* BSID_P_SetFwHwDefault  */
    pConfigSettings->sidSettings.enabled           = true;
    pConfigSettings->sidSettings.fwSize            = BSID_ARC_CODE_SIZE;
    pConfigSettings->sidSettings.maxChannels       = 16;
    pConfigSettings->sidSettings.numChannels       = 1;
    pConfigSettings->sidSettings.size_per_channel  = BSID_DATA_MEMORY_SIZE;
    pConfigSettings->sidSettings.other_allocations = ( 2*BSID_MBX_MEMORY_SIZE )+BSID_INPUT_DMA_MEMORY_SIZE;

    /* VBI */
    /* displays  * ttSize */

    /*Raaga */
    pConfigSettings->audioSettings.raagaSize =  14476904;
    pConfigSettings->audioSettings.apeSize   =  2125824;

    /*Surface */
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        pConfigSettings->surfaceSettings[i].numFrameBufs   = 2;
        pConfigSettings->surfaceSettings[i].width          = 720;
        pConfigSettings->surfaceSettings[i].height         = 480;
        pConfigSettings->surfaceSettings[i].bits_per_pixel = 32;
    }

    pConfigSettings->surfaceSettings[0].width  = 1280;
    pConfigSettings->surfaceSettings[0].height = 720;

    /*GRC */
    pConfigSettings->grcSettings.fifoSize         = 147456;
    pConfigSettings->grcSettings.numPacketContext =  0;
    pConfigSettings->grcSettings.packetBuffer     =  131252;
    pConfigSettings->grcSettings.packetSyncAlloc  =  1024;
} /* getAppMemoryConfigurationDefaultSettings */

static void    translateGUIPageToMemConfig(
    Memconfig_AppUsageSettings               *pAppUsageSettings,
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings
    )
{
    int i;

    if (pAppUsageSettings->record.number)
    {
        pConfigSettings->recordRaveSettings.enabled = true;
    }
    else
    {
        pConfigSettings->recordRaveSettings.enabled = false;
    }
    pConfigSettings->recordRaveSettings.bitRate     = pAppUsageSettings->record.bitRate;
    pConfigSettings->recordRaveSettings.latency     = pAppUsageSettings->record.latency;
    pConfigSettings->recordRaveSettings.numRecpumps = pAppUsageSettings->record.number;

    pConfigSettings->RSBuf_Settings.secureHeap          = pAppUsageSettings->bSecure;
    pConfigSettings->RSBuf_Settings.numPlaybacks        = pAppUsageSettings->playback.number;
    pConfigSettings->RSBuf_Settings.numIBPIDParsers     = pAppUsageSettings->live.number;
    pConfigSettings->RSBuf_Settings.numMpodIBPIDParsers = pAppUsageSettings->live.number;

    for (i = 0; i<Memconfig_XCBufType_eMax; i++)
    {
        pConfigSettings->XCBuf_Settings[i].secureHeap      = pAppUsageSettings->bSecure;
        pConfigSettings->XCBuf_Settings[i].numPlaybacks    = pAppUsageSettings->playback.number;
        pConfigSettings->XCBuf_Settings[i].numIBPIDParsers = pAppUsageSettings->live.number;
    }

    pConfigSettings->nexus_playpump_settings.numPlaypumps = pAppUsageSettings->playback.number;
    pConfigSettings->nexus_playpump_settings.fifoSize     = pAppUsageSettings->playback.size;

    if (pAppUsageSettings->message.number)
    {
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eMsg].enabled = true;
    }
    else
    {
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eMsg].enabled = false;
    }

    pConfigSettings->messageBuffers.numMessageBuffers = pAppUsageSettings->message.number;
    pConfigSettings->messageBuffers.size              = pAppUsageSettings->message.size;

    if (pAppUsageSettings->remux.number ==1)
    {
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx0].enabled = true;
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx1].enabled = false;
    }
    else if (pAppUsageSettings->remux.number ==2)
    {
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx0].enabled = true;
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx1].enabled = true;
    }
    else
    {
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx0].enabled = false;
        pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx1].enabled = false;
    }

    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pConfigSettings->videoRaveSettings[i].enabled       = pAppUsageSettings->videoDecoder[i].enabled;
        pConfigSettings->videoRaveSettings[i].ipMode        = pAppUsageSettings->videoDecoder[i].bIp;
        pConfigSettings->videoRaveSettings[i].fourKMode     = pAppUsageSettings->videoDecoder[i].b3840x2160;
        pConfigSettings->videoRaveSettings[i].secureHeap    = pAppUsageSettings->videoDecoder[i].bSecure;
        pConfigSettings->videoRaveSettings[i].softContext   = pAppUsageSettings->videoDecoder[i].bSoft;
        pConfigSettings->videoRaveSettings[i].MVC           = pAppUsageSettings->videoDecoder[i].bMvc;
        pConfigSettings->videoRaveSettings[i].numMosaics    = pAppUsageSettings->videoDecoder[i].numMosaic;
        pConfigSettings->videoRaveSettings[i].numFCCprimers = pAppUsageSettings->videoDecoder[i].numFcc;
    }

    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        pConfigSettings->audioRaveSettings[i].enabled       = pAppUsageSettings->audioDecoder[i].enabled;
        pConfigSettings->audioRaveSettings[i].passthru      = pAppUsageSettings->audioDecoder[i].bPassthru;
        pConfigSettings->audioRaveSettings[i].secondary     = pAppUsageSettings->audioDecoder[i].bSecondary;
        pConfigSettings->audioRaveSettings[i].ipMode        = pAppUsageSettings->audioDecoder[i].bIp;
        pConfigSettings->audioRaveSettings[i].secureHeap    = pAppUsageSettings->audioDecoder[i].bSecure;
        pConfigSettings->audioRaveSettings[i].numFCCprimers = pAppUsageSettings->audioDecoder[i].numFcc;
    }

#if NEXUS_NUM_VIDEO_ENCODERS
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        pConfigSettings->encodeRaveSettings[i].enabled   = pAppUsageSettings->encoders[i].enabled;
        pConfigSettings->encodeRaveSettings[i].streamMux = pAppUsageSettings->encoders[i].streamMux;
    }
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        pConfigSettings->surfaceSettings[i].numFrameBufs   =  pAppUsageSettings->surfaceSettings[i].numFrameBufs;
        pConfigSettings->surfaceSettings[i].width          =  pAppUsageSettings->surfaceSettings[i].width;
        pConfigSettings->surfaceSettings[i].height         =  pAppUsageSettings->surfaceSettings[i].height;
        pConfigSettings->surfaceSettings[i].bits_per_pixel =  pAppUsageSettings->surfaceSettings[i].bits_per_pixel;
    }

    pConfigSettings->sidSettings.maxChannels = pAppUsageSettings->sidSettings.maxChannels;
    pConfigSettings->sidSettings.numChannels = pAppUsageSettings->sidSettings.numChannels;
} /* translateGUIPageToMemConfig */

static void    translateMemConfigToGUIPage(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppUsageSettings               *pAppUsageSettings
    )
{
    int i;

    pAppUsageSettings->bSecure = pConfigSettings->RSBuf_Settings.secureHeap;

    pAppUsageSettings->record.number  = pConfigSettings->recordRaveSettings.numRecpumps;
    pAppUsageSettings->record.max     = pConfigSettings->recordRaveSettings.maxRecpumps;
    pAppUsageSettings->record.bitRate = pConfigSettings->recordRaveSettings.bitRate;
    pAppUsageSettings->record.latency = pConfigSettings->recordRaveSettings.latency;

    pAppUsageSettings->playback.number =   pConfigSettings->nexus_playpump_settings.numPlaypumps;
    pAppUsageSettings->playback.size   = pConfigSettings->nexus_playpump_settings.fifoSize;
    pAppUsageSettings->playback.max    = pConfigSettings->nexus_playpump_settings.maxPlaypumps;

    pAppUsageSettings->live.number = pConfigSettings->RSBuf_Settings.numIBPIDParsers;
    pAppUsageSettings->live.size   = pConfigSettings->RSBuf_Settings.inputBandBufSize;
    pAppUsageSettings->live.max    = pConfigSettings->RSBuf_Settings.maxIBPIDParsers;

    pAppUsageSettings->message.number = pConfigSettings->messageBuffers.numMessageBuffers;
    pAppUsageSettings->message.max    = pConfigSettings->messageBuffers.maxMessageBuffers;
    pAppUsageSettings->message.size   = pConfigSettings->messageBuffers.size;

    /* Remux */
    pAppUsageSettings->remux.number = 0;
    if (pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx0].enabled)
    {
        pAppUsageSettings->remux.number++;
    }
    if (pConfigSettings->XCBuf_Settings[Memconfig_XCBufType_eRmx1].enabled)
    {
        pAppUsageSettings->remux.number++;
    }

    pAppUsageSettings->remux.max = NEXUS_NUM_REMUX;

    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pAppUsageSettings->videoDecoder[i].enabled    = pConfigSettings->videoRaveSettings[i].enabled;
        pAppUsageSettings->videoDecoder[i].bIp        =  pConfigSettings->videoRaveSettings[i].ipMode;
        pAppUsageSettings->videoDecoder[i].b3840x2160 = pConfigSettings->videoRaveSettings[i].fourKMode;
        pAppUsageSettings->videoDecoder[i].bSecure    = pConfigSettings->videoRaveSettings[i].secureHeap;
        pAppUsageSettings->videoDecoder[i].bSoft      = pConfigSettings->videoRaveSettings[i].softContext;
        pAppUsageSettings->videoDecoder[i].bMvc       = pConfigSettings->videoRaveSettings[i].MVC;
        pAppUsageSettings->videoDecoder[i].numMosaic  = pConfigSettings->videoRaveSettings[i].numMosaics;
        pAppUsageSettings->videoDecoder[i].numFcc     = pConfigSettings->videoRaveSettings[i].numFCCprimers;
    }

    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        pAppUsageSettings->audioDecoder[i].enabled    = pConfigSettings->audioRaveSettings[i].enabled;
        pAppUsageSettings->audioDecoder[i].bPassthru  = pConfigSettings->audioRaveSettings[i].passthru;
        pAppUsageSettings->audioDecoder[i].bSecondary = pConfigSettings->audioRaveSettings[i].secondary;
        pAppUsageSettings->audioDecoder[i].bIp        = pConfigSettings->audioRaveSettings[i].ipMode;
        pAppUsageSettings->audioDecoder[i].bSecure    = pConfigSettings->audioRaveSettings[i].secureHeap;
        pAppUsageSettings->audioDecoder[i].numFcc     = pConfigSettings->audioRaveSettings[i].numFCCprimers;
    }

#if NEXUS_NUM_VIDEO_ENCODERS
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        pAppUsageSettings->encoders[i].enabled   =  pConfigSettings->encodeRaveSettings[i].enabled;
        pAppUsageSettings->encoders[i].streamMux = pConfigSettings->encodeRaveSettings[i].streamMux;
    }
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        pAppUsageSettings->surfaceSettings[i].numFrameBufs   = pConfigSettings->surfaceSettings[i].numFrameBufs;
        pAppUsageSettings->surfaceSettings[i].width          = pConfigSettings->surfaceSettings[i].width;
        pAppUsageSettings->surfaceSettings[i].height         = pConfigSettings->surfaceSettings[i].height;
        pAppUsageSettings->surfaceSettings[i].bits_per_pixel = pConfigSettings->surfaceSettings[i].bits_per_pixel;
    }

    pAppUsageSettings->sidSettings.maxChannels =  pConfigSettings->sidSettings.maxChannels;
    pAppUsageSettings->sidSettings.numChannels =  pConfigSettings->sidSettings.numChannels;
} /* translateMemConfigToGUIPage */

/**
1.Error code if exceeds max
2. Return number of bytes used give the settings.
**/

static int calcRSBuf(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    int total = 0;

    if (pConfigSettings->RSBuf_Settings.enabled)
    {
        if (pConfigSettings->RSBuf_Settings.numPlaybacks > pConfigSettings->RSBuf_Settings.maxPlaybacks)
        {
            return( -1 );  /*too many playbacks allocated */
        }
        total = pConfigSettings->RSBuf_Settings.numPlaybacks * pConfigSettings->RSBuf_Settings.playbackBufSize;

        if (pConfigSettings->RSBuf_Settings.numIBPIDParsers > pConfigSettings->RSBuf_Settings.maxIBPIDParsers)
        {
            return( -1 );  /*too many inputband parsers allocated */
        }
        total += pConfigSettings->RSBuf_Settings.numIBPIDParsers * pConfigSettings->RSBuf_Settings.inputBandBufSize;

        if (pConfigSettings->RSBuf_Settings.mpodEnabled)
        {
            total += pConfigSettings->RSBuf_Settings.numMpodIBPIDParsers * pConfigSettings->RSBuf_Settings.mpodInputBandBufSize;
        }

        if (pConfigSettings->RSBuf_Settings.secureHeap)
        {
            pOutputSettings->secureHeapTotal      += total;
            pOutputSettings->live.bytesSecure     += pConfigSettings->RSBuf_Settings.numIBPIDParsers * pConfigSettings->RSBuf_Settings.inputBandBufSize;
            pOutputSettings->playback.bytesSecure +=  pConfigSettings->RSBuf_Settings.numPlaybacks * pConfigSettings->RSBuf_Settings.playbackBufSize;
        }
        else
        {
            pOutputSettings->mainHeapTotal         += total;
            pOutputSettings->live.bytesGeneral     += pConfigSettings->RSBuf_Settings.numIBPIDParsers * pConfigSettings->RSBuf_Settings.inputBandBufSize;
            pOutputSettings->playback.bytesGeneral +=  pConfigSettings->RSBuf_Settings.numPlaybacks * pConfigSettings->RSBuf_Settings.playbackBufSize;
        }
    }

    #if ( PRINTF==1 )
    printf( "RSBUF end total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif
    return( 0 );
} /* calcRSBuf */

static int calcXCBuf(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    int i;
    int end_total = 0;

    pOutputSettings->remux.bytesGeneral = 0;
    for (i = 0; i<Memconfig_XCBufType_eMax; i++)
    {
        if (pConfigSettings->XCBuf_Settings[i].enabled)
        {
            int bytesPerXCBufType = 0;
            int total             = 0;
            if (pConfigSettings->XCBuf_Settings[i].numPlaybacks > pConfigSettings->XCBuf_Settings[i].maxPlaybacks)
            {
                return( -1 );  /*too many playbacks allocated */
            }
            total             += pConfigSettings->XCBuf_Settings[i].numPlaybacks *pConfigSettings->XCBuf_Settings[i].playbackBufSize;
            bytesPerXCBufType += pConfigSettings->XCBuf_Settings[i].numPlaybacks *pConfigSettings->XCBuf_Settings[i].playbackBufSize;

            if (pConfigSettings->XCBuf_Settings[i].numIBPIDParsers > pConfigSettings->XCBuf_Settings[i].maxIBPIDParsers)
            {
                return( -1 );  /*too many inputband parsers allocated */
            }
            total             += pConfigSettings->XCBuf_Settings[i].numIBPIDParsers * pConfigSettings->XCBuf_Settings[i].inputBandBufSize;
            bytesPerXCBufType += pConfigSettings->XCBuf_Settings[i].numIBPIDParsers * pConfigSettings->XCBuf_Settings[i].inputBandBufSize;

            if (pConfigSettings->XCBuf_Settings[i].secureHeap)
            {
                pOutputSettings->secureHeapTotal += total;
                if (i==Memconfig_XCBufType_eRave)
                {
                    pOutputSettings->playback.bytesSecure += pConfigSettings->XCBuf_Settings[i].numPlaybacks *pConfigSettings->XCBuf_Settings[i].playbackBufSize;
                    pOutputSettings->live.bytesSecure     += pConfigSettings->XCBuf_Settings[i].numIBPIDParsers * pConfigSettings->XCBuf_Settings[i].inputBandBufSize;
                }
                else if (i==Memconfig_XCBufType_eMsg)
                {
                    pOutputSettings->message.bytesSecure += bytesPerXCBufType;
                }
                else if (i==Memconfig_XCBufType_eRmx0)
                {
                    pOutputSettings->remux.bytesSecure += bytesPerXCBufType;
                }
                else if (i==Memconfig_XCBufType_eRmx1)
                {
                    pOutputSettings->remux.bytesSecure += bytesPerXCBufType;
                }
                else
                {printf( "ERROR\n" ); }
            }
            else
            {
                pOutputSettings->mainHeapTotal += total;
                if (i==Memconfig_XCBufType_eRave)
                {
                    pOutputSettings->playback.bytesGeneral += pConfigSettings->XCBuf_Settings[i].numPlaybacks *pConfigSettings->XCBuf_Settings[i].playbackBufSize;
                    pOutputSettings->live.bytesGeneral     += pConfigSettings->XCBuf_Settings[i].numIBPIDParsers * pConfigSettings->XCBuf_Settings[i].inputBandBufSize;
                }
                else if (i==Memconfig_XCBufType_eMsg)
                {
                    pOutputSettings->message.bytesGeneral += bytesPerXCBufType;
                }
                else if (i==Memconfig_XCBufType_eRmx0)
                {
                    pOutputSettings->remux.bytesGeneral += bytesPerXCBufType;
                }
                else if (i==Memconfig_XCBufType_eRmx1)
                {
                    pOutputSettings->remux.bytesGeneral += bytesPerXCBufType;
                }
                else
                {printf( "ERROR\n" ); }
            }
            end_total += total;
        }
    }
    #if ( PRINTF==1 )
    printf( "XCBUF  end_total %d main %d, secure %d\n", end_total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    return( 0 );
} /* calcXCBuf */

static int  calcRAVEBufs(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    int      i;
    unsigned totalNumRaveContext = 0;
    unsigned total               = 0;

    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        pOutputSettings->videoDecoder[i].bytesSecure  = 0;
        pOutputSettings->videoDecoder[i].bytesGeneral = 0;
        if (pConfigSettings->videoRaveSettings[i].enabled)
        {
            int      cdbSize, itbSize;
            int      currNumRaveContext;
            unsigned bytesPerDecoder = 0;

            if (pConfigSettings->videoRaveSettings[i].ipMode)
            {
                pConfigSettings->videoRaveSettings[i].cdbBufSize = IP_CDB_SIZE;
            }
            else if (pConfigSettings->videoRaveSettings[i].fourKMode)
            {
                pConfigSettings->videoRaveSettings[i].cdbBufSize = FOUR_K_CDB_SIZE;
            }

            cdbSize = pConfigSettings->videoRaveSettings[i].cdbBufSize;
            itbSize = pConfigSettings->videoRaveSettings[i].itbBufSize;

            currNumRaveContext++;
            if (pConfigSettings->videoRaveSettings[i].softContext)
            {
                itbSize = 2*  pConfigSettings->videoRaveSettings[i].itbBufSize;
                currNumRaveContext++;
            }

            total           += ( itbSize + cdbSize );
            bytesPerDecoder += ( itbSize + cdbSize );

            /* double it again */
            if (pConfigSettings->videoRaveSettings[i].MVC)
            {
                total             += ( itbSize + cdbSize );
                bytesPerDecoder   += ( itbSize + cdbSize );
                currNumRaveContext = 2 * currNumRaveContext;
            }
            if (( pConfigSettings->videoRaveSettings[i].numMosaics > 0 ) && ( pConfigSettings->videoRaveSettings[i].fourKMode ==false ))
            {
                total             += pConfigSettings->videoRaveSettings[i].numMosaics *( pConfigSettings->videoRaveSettings[i].cdbBufSize + pConfigSettings->videoRaveSettings[i].itbBufSize );
                bytesPerDecoder   += pConfigSettings->videoRaveSettings[i].numMosaics *( pConfigSettings->videoRaveSettings[i].cdbBufSize + pConfigSettings->videoRaveSettings[i].itbBufSize );
                currNumRaveContext = currNumRaveContext * pConfigSettings->videoRaveSettings[i].numMosaics;
            }
            /* video primer doesn't have MVC ? */
            total              += pConfigSettings->videoRaveSettings[i].numFCCprimers * ( VIDEOPRIMER_CDB_SIZE + itbSize );
            bytesPerDecoder    += pConfigSettings->videoRaveSettings[i].numFCCprimers * ( VIDEOPRIMER_CDB_SIZE + itbSize );
            currNumRaveContext += pConfigSettings->videoRaveSettings[i].numFCCprimers;

            if (pConfigSettings->videoRaveSettings[i].secureHeap)
            {
                pOutputSettings->secureHeapTotal            += total;
                pOutputSettings->videoDecoder[i].bytesSecure = bytesPerDecoder;
            }
            else
            {
                pOutputSettings->mainHeapTotal               += total;
                pOutputSettings->videoDecoder[i].bytesGeneral = bytesPerDecoder;
            }

            totalNumRaveContext += currNumRaveContext;
        }
    }
    #if ( PRINTF==1 )
    printf( "videoRave total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    /*
       Each encode has a we allocate one video encoder, one audio encode and one recpumpe
     */
#if NEXUS_NUM_VIDEO_ENCODERS
    total = 0;
    for (i = 0; i<NEXUS_NUM_VIDEO_ENCODERS; i++)
    {
        if (pConfigSettings->encodeRaveSettings[i].enabled)
        {
            int currNumRaveContext = 0;

            /* Adding for  audio encoder mux */
            total += APE_CDB_SIZE + APE_ITB_SIZE;
            currNumRaveContext++;

            /* Adding for record setttings */
            if (pConfigSettings->encodeRaveSettings[i].streamMux)
            {
                /* NEXUS_RECPUMP_CDB_SIZE   NEXUS_RECPUMP_ITB_SIZE */
                total += NEXUS_RECPUMP_CDB_SIZE + NEXUS_RECPUMP_ITB_SIZE;
                currNumRaveContext++;
            }
            pOutputSettings->mainHeapTotal += total;

            totalNumRaveContext += currNumRaveContext;
        }
    }
    pOutputSettings->encode.bytesGeneral = total;
    #if ( PRINTF==1 )
    printf( "encodeRave total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    total = 0;

    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        pOutputSettings->audioDecoder[i].bytesSecure  = 0;
        pOutputSettings->audioDecoder[i].bytesGeneral = 0;
        if (pConfigSettings->audioRaveSettings[i].enabled)
        {
            int      cdbSize, itbSize;
            int      currNumRaveContext = 0;
            unsigned bytesPerDecoder    = 0;

            if (pConfigSettings->audioRaveSettings[i].ipMode)
            {
                pConfigSettings->audioRaveSettings[i].cdbBufSize += APE_CDB_SIZE;
            }

            cdbSize = pConfigSettings->audioRaveSettings[i].cdbBufSize;

            itbSize = pConfigSettings->audioRaveSettings[i].itbBufSize;
            if (pConfigSettings->audioRaveSettings[i].passthru)
            {
                currNumRaveContext++;
                total           +=  ( cdbSize + itbSize );
                bytesPerDecoder +=  ( cdbSize + itbSize );
            }

            if (pConfigSettings->audioRaveSettings[i].secondary)
            {
                currNumRaveContext++;
                total           +=  ( cdbSize + itbSize );
                bytesPerDecoder +=  ( cdbSize + itbSize );
            }

            /* typical case for audio */
            currNumRaveContext++;
            total           +=  ( cdbSize + itbSize );
            bytesPerDecoder +=  ( cdbSize + itbSize );

            total           += pConfigSettings->audioRaveSettings[i].numFCCprimers * ( APE_CDB_SIZE + itbSize );
            bytesPerDecoder += pConfigSettings->audioRaveSettings[i].numFCCprimers * ( APE_CDB_SIZE + itbSize );

            #if 0
            /* CAD 8/26/2014: PK and DE both say audio should never be in secure heap */
            if (pConfigSettings->audioRaveSettings[i].secureHeap)
            {
                pOutputSettings->secureHeapTotal            += total;
                pOutputSettings->audioDecoder[i].bytesSecure = bytesPerDecoder;
            }
            else
            #endif /* if 0 */
            {
                pOutputSettings->mainHeapTotal               += total;
                pOutputSettings->audioDecoder[i].bytesGeneral = bytesPerDecoder;
            }

            totalNumRaveContext += currNumRaveContext;
        }
    }

    #if ( PRINTF==1 )
    printf( "audioRave total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    total = 0;

    if (pConfigSettings->recordRaveSettings.enabled)
    {
        int cdbSize, itbSize;

        if (pConfigSettings->recordRaveSettings.numRecpumps > pConfigSettings->recordRaveSettings.maxRecpumps)
        {
            printf( "pConfigSettings->recordRaveSettings.numRecpumps(%d) > pConfigSettings->recordRaveSettings.maxRecpumps (%d)",
                pConfigSettings->recordRaveSettings.numRecpumps, pConfigSettings->recordRaveSettings.maxRecpumps );
        }
        else
        {
            pConfigSettings->recordRaveSettings.cdbBufSize = ( pConfigSettings->recordRaveSettings.bitRate*1024*1024/8 ) *
                pConfigSettings->recordRaveSettings.latency/1000*4;
#if 0
            /* 2.5M CDB / 10K per picture * (16 slices/picture + 5 SC's for various overhead) = 5376 ITB's * 24 bytes/ITB = 126K */
            pConfigSettings->recordRaveSettings.itbBufSize =  ( pConfigSettings->recordRaveSettings.cdbBufSize *1024*1024 )/( 10*1024 ) *
                ( 16+5 ) *24;
#endif

            cdbSize = pConfigSettings->recordRaveSettings.cdbBufSize;
            itbSize = pConfigSettings->recordRaveSettings.itbBufSize;

            total =  pConfigSettings->recordRaveSettings.numRecpumps * ( cdbSize + itbSize );

            if (pConfigSettings->recordRaveSettings.secureHeap)
            {
                pOutputSettings->secureHeapTotal += total;
            }
            else
            {
                pOutputSettings->mainHeapTotal += total;
            }

            totalNumRaveContext += pConfigSettings->recordRaveSettings.numRecpumps;
        }
    }
    pOutputSettings->record.bytesGeneral = total;
    #if ( PRINTF==1 )
    printf( "recordRave total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    total = 0;

    /* Nexus still decoder, use AVD still decoder */
    if (pConfigSettings->stillImageRaveSettings.enabled)
    {
        if (pConfigSettings->stillImageRaveSettings.numSID > pConfigSettings->stillImageRaveSettings.maxSID)
        {
            printf( "pConfigSettings->stillImageRaveSettings.numSID (%d) > pConfigSettings->stillImageRaveSettings.maxSID (%d)",
                pConfigSettings->stillImageRaveSettings.numSID, pConfigSettings->stillImageRaveSettings.maxSID );
        }
        else
        {
            int cdbSize, itbSize;
            int currNumRaveContext = 0;
            cdbSize = pConfigSettings->stillImageRaveSettings.cdbBufSize;
            itbSize = pConfigSettings->stillImageRaveSettings.itbBufSize;
            currNumRaveContext++;
            if (pConfigSettings->stillImageRaveSettings.softContext)
            {
                itbSize            = 2*itbSize;
                currNumRaveContext = 2*currNumRaveContext;
            }
            total =  pConfigSettings->stillImageRaveSettings.numSID * ( cdbSize + itbSize );

            if (pConfigSettings->stillImageRaveSettings.secureHeap)
            {
                pOutputSettings->secureHeapTotal += total;
            }
            else
            {
                pOutputSettings->mainHeapTotal += total;
            }

            totalNumRaveContext += currNumRaveContext;
        }
    }
    #if ( PRINTF==1 )
    printf( "sidRave total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    if (totalNumRaveContext > pConfigSettings->maxRAVEContext)
    {
        return( -1 );
    }
    else
    {
        return( 0 );
    }
} /* calcRAVEBufs */

static int calcNexusPlaypumpBuf(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;

    if (pConfigSettings->nexus_playpump_settings.numPlaypumps > pConfigSettings->nexus_playpump_settings.maxPlaypumps)
    {
        return( -1 );
    }

    if (pConfigSettings->nexus_playpump_settings.scatterGatherMode)
    {
        total += pConfigSettings->nexus_playpump_settings.numSGDescriptors * pConfigSettings->nexus_playpump_settings.SGBufSize;
    }
    else
    {
        total += pConfigSettings->nexus_playpump_settings.numPlaypumps * pConfigSettings->nexus_playpump_settings.fifoSize;
    }

    if (pConfigSettings->nexus_playpump_settings.secureHeap)
    {
        pOutputSettings->secureHeapTotal += total;
    }
    else
    {
        pOutputSettings->mainHeapTotal += total;
    }
    pOutputSettings->playback.bytesGeneral = total;
    #if ( PRINTF==1 )
    printf( "playpump total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    return( 0 );
} /* calcNexusPlaypumpBuf */

static int calcNexusMessageBuf(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;

    if (pConfigSettings->messageBuffers.numMessageBuffers > pConfigSettings->messageBuffers.maxMessageBuffers)
    {
        return( -1 );
    }

    total += pConfigSettings->messageBuffers.numMessageBuffers * pConfigSettings->messageBuffers.size;
    pOutputSettings->mainHeapTotal        += total;
    pOutputSettings->message.bytesGeneral += total;
    #if ( PRINTF==1 )
    printf( "messageBuffer total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    return( 0 );
} /* calcNexusMessageBuf */

static unsigned calcSurfaceAllocations(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;
    int      i;

    for (i = 0; i < NEXUS_NUM_DISPLAYS; i++)
    {
        unsigned bytesPerSurfaceType = 0;
        bytesPerSurfaceType = pConfigSettings->surfaceSettings[i].numFrameBufs * pConfigSettings->surfaceSettings[i].width *
            pConfigSettings->surfaceSettings[i].height * ( pConfigSettings->surfaceSettings[i].bits_per_pixel/8 );

        total += bytesPerSurfaceType;

        pOutputSettings->surfaceSettings[i].bytesGeneral = bytesPerSurfaceType;
    }

    /* Surfaces don't have a patricular heap in can go into. Can go to main or graphics heap */
    /*pOutputSettings->mainHeapTotal += total;  */

    #if ( PRINTF==1 )
    printf( "SurfaceAllocations( Not Added to any Heap) total %d main %d, secure %d\n", total, pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    #endif

    return( 0 );
} /* calcSurfaceAllocations */

/* nexus picture decoder */
static unsigned calcSIDAllocations(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;

    if (pConfigSettings->sidSettings.enabled)
    {
        total +=  pConfigSettings->sidSettings.fwSize;
        total +=  pConfigSettings->sidSettings.other_allocations;
        total +=  pConfigSettings->sidSettings.numChannels * pConfigSettings->sidSettings.size_per_channel;
    }
    pOutputSettings->sidSettings.bytesGeneral =  total;

    return( 0 );
}

#if 0

static unsigned calcV3d(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    pOutputSettings->mainHeapTotal += pConfigSettings->v3dSettings.size;

    printf( "v3d total %d main %d, secure %d\n", ( pConfigSettings->v3dSettings.size ), pOutputSettings->mainHeapTotal, pOutputSettings->secureHeapTotal );
    return( 0 );
}

static unsigned calcVBIAllocations(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    return( 0 );
}

static unsigned calcAudioAllocations(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;

    total += pConfigSettings->audioSettings.apeSize  + pConfigSettings->audioSettings.raagaSize;

    pOutputSettings->mainHeapTotal += total;
    return( 0 );
}

static unsigned calcGRCAllocations(
    Memconfig_AppMemoryConfigurationSettings *pConfigSettings,
    Memconfig_AppMemUsage                    *pOutputSettings
    )
{
    unsigned total = 0;

    total += pConfigSettings->grcSettings.fifoSize +
        ( pConfigSettings->grcSettings.numPacketContext * ( pConfigSettings->grcSettings.packetBuffer + pConfigSettings->grcSettings.packetSyncAlloc ));

    pOutputSettings->mainHeapTotal += total;
    return( 0 );
}

#endif /* if 0 */
static void printSettings(
    Memconfig_AppUsageSettings *pAppUsageSettings,
    Memconfig_AppMemUsage      *pMemUsage
    )
{
    int videoTotal = 0, audioTotal = 0, i;

    printf( ":: Input Structure:: \n" );

    printf( "Secure Flag %d \n", pAppUsageSettings->bSecure );
    printf( "record.number %d\n", pAppUsageSettings->record.number );

    printf( "playback.number %d\n", pAppUsageSettings->playback.number );

    printf( "message.number %d\n", pAppUsageSettings->message.number );
    printf( "message.size %d \n", pAppUsageSettings->message.size );

    printf( "live.number %d\n", pAppUsageSettings->live.number );

    printf( "remux.number %d\n", pAppUsageSettings->remux.number );

    printf( "videoDecoder[0].enabled %d\n", pAppUsageSettings->videoDecoder[0].enabled );
    printf( "videoDecoder[0].bIp %d\n",   pAppUsageSettings->videoDecoder[0].bIp );
    printf( "videoDecoder[0].b3840x2160 %d\n",     pAppUsageSettings->videoDecoder[0].b3840x2160 );
    printf( "videoDecoder[0].bSecure %d\n",      pAppUsageSettings->videoDecoder[0].bSecure );
    printf( "videoDecoder[0].bSoft %d\n",       pAppUsageSettings->videoDecoder[0].bSoft );
    printf( "videoDecoder[0].bMvc %d\n",       pAppUsageSettings->videoDecoder[0].bMvc );
    printf( "videoDecoder[0].numMosaic %d\n",       pAppUsageSettings->videoDecoder[0].numMosaic );
    printf( "videoDecoder[0].numFcc %d\n",      pAppUsageSettings->videoDecoder[0].numFcc );

    printf( "audioDecoder[0].enabled %d\n",  pAppUsageSettings->audioDecoder[0].enabled );
    printf( "audioDecoder[0].bPassthru %d\n",     pAppUsageSettings->audioDecoder[0].bPassthru );
    printf( "audioDecoder[0].bSecondary %d\n",    pAppUsageSettings->audioDecoder[0].bSecondary );
    printf( "audioDecoder[0].bIp %d\n",    pAppUsageSettings->audioDecoder[0].bIp );
    printf( "audioDecoder[0].bSecure %d\n",     pAppUsageSettings->audioDecoder[0].bSecure );
    printf( "audioDecoder[0].numFcc %d\n",    pAppUsageSettings->audioDecoder[0].numFcc );

#if NEXUS_NUM_VIDEO_ENCODERS
    printf( "encoders[0].enabled %d\n",  pAppUsageSettings->encoders[0].enabled );
    printf( "encoders[0].streamMux %d\n", pAppUsageSettings->encoders[0].streamMux );
#endif /* NEXUS_NUM_VIDEO_ENCODERS */

    printf( "record.latency %d  record.bitRate %d\n", pAppUsageSettings->record.latency, pAppUsageSettings->record.bitRate );

    /*Surface */
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        printf( "surface[%d].numFrameBufs %d\n", i, pAppUsageSettings->surfaceSettings[i].numFrameBufs );
        printf( "surface[%d].width %d\n", i, pAppUsageSettings->surfaceSettings[i].width );
        printf( "surface[%d].height %d\n", i, pAppUsageSettings->surfaceSettings[i].height );
        printf( "surface[%d].bits_per_pixel %d\n", i, pAppUsageSettings->surfaceSettings[i].bits_per_pixel );
    }

    /*SID Picture decoder */

    printf( "sid (maxChannels %d), numChannels %d\n", pAppUsageSettings->sidSettings.maxChannels, pAppUsageSettings->sidSettings.numChannels );

    printf( ":: Output Structure:: \n" );

    printf( "playpack.bytesGeneral %d\n", pMemUsage->playback.bytesGeneral );
    printf( "playpack.bytesSecure %d\n", pMemUsage->playback.bytesSecure );
    printf( "message.bytesGeneral %d\n", pMemUsage->message.bytesGeneral );

    printf( "message.bytesSecure %d\n", pMemUsage->message.bytesSecure );
    printf( "live.bytesGeneral %d\n", pMemUsage->live.bytesGeneral );
    printf( "live.bytesSecure %d\n", pMemUsage->live.bytesSecure );

    printf( "record.bytesGeneral %d\n", pMemUsage->record.bytesGeneral );
    printf( "remux.bytesGeneral %d\n", pMemUsage->remux.bytesGeneral );
    printf( "remux.bytesSecure %d\n", pMemUsage->remux.bytesSecure );

    printf( "encode.bytesGeneral %d\n", pMemUsage->encode.bytesGeneral );
    for (i = 0; i<NEXUS_NUM_AUDIO_DECODERS; i++)
    {
        audioTotal += pMemUsage->audioDecoder[i].bytesGeneral;
        printf( "audioDecoder[%d].bytesGeneral %d\n", i, pMemUsage->audioDecoder[i].bytesGeneral );
        audioTotal += pMemUsage->audioDecoder[i].bytesSecure;
        printf( "audioDecoder[%d].bytesSecure %d\n", i, pMemUsage->audioDecoder[i].bytesSecure );
    }
    printf( "audio.total %d\n", audioTotal );

    for (i = 0; i<NEXUS_NUM_VIDEO_DECODERS; i++)
    {
        videoTotal += pMemUsage->videoDecoder[i].bytesGeneral;

        printf( "videoDecoder[%d].bytesGeneral %d\n", i, pMemUsage->videoDecoder[i].bytesGeneral );
        videoTotal += pMemUsage->videoDecoder[i].bytesSecure;
        printf( "videoDecoder[%d].bytesSecure %d\n", i, pMemUsage->videoDecoder[i].bytesSecure );
    }
    printf( "video.total %d\n", videoTotal );

    printf( "individual total %d\n", pMemUsage->playback.bytesGeneral+ pMemUsage->message.bytesGeneral+pMemUsage->live.bytesGeneral+pMemUsage->record.bytesGeneral+pMemUsage->remux.bytesGeneral +
        pMemUsage->encode.bytesGeneral + audioTotal + videoTotal );
    printf( "totalGeneral %d total Secure %d \n", pMemUsage->mainHeapTotal, pMemUsage->secureHeapTotal );

    /*Surface */
    for (i = 0; i<NEXUS_NUM_DISPLAYS; i++)
    {
        printf( "surface[%d].bytesGeneral %d\n", i, pMemUsage->surfaceSettings[i].bytesGeneral );
    }

    printf( "sid.bytesGeneral %d\n", pMemUsage->sidSettings.bytesGeneral );
} /* printSettings */

int Memconfig_AppUsageGetDefaultSettings(
    Memconfig_AppUsageSettings *settings
    )
{
    getAppMemoryConfigurationDefaultSettings( &gSettings );

    /* move results back into Transport Input Struct*/
    translateMemConfigToGUIPage( &gSettings, settings );

    return( 0 );
}

int Memconfig_AppUsageCalculate(
    Memconfig_AppUsageSettings *settings,
    Memconfig_AppMemUsage      *pMemUsage
    )
{
    #if ( PRINTF==1 )
    printf( "\n Before Calculator: msg0 size %u; %ux%ux%ux%u\n ", settings->message.size,
        settings->surfaceSettings[0].width, settings->surfaceSettings[0].height, settings->surfaceSettings[0].bits_per_pixel,
        settings->surfaceSettings[0].numFrameBufs    );
    printSettings( settings, pMemUsage );
    #else /* if ( PRINTF==1 ) */
    BSTD_UNUSED( printSettings );
    #endif /* if ( PRINTF==1 ) */
    /* After you set gui inputs Must translate into memconfig */
    translateGUIPageToMemConfig( settings, &gSettings );

    /* calculate */
    /* Todo do I need to reset the values to pMemUsage before going through calculate */
    calcNexusPlaypumpBuf( &gSettings, pMemUsage );
    calcRAVEBufs( &gSettings, pMemUsage );
    calcRSBuf( &gSettings, pMemUsage );
    calcXCBuf( &gSettings, pMemUsage );
    calcNexusMessageBuf( &gSettings, pMemUsage );

    calcSurfaceAllocations( &gSettings, pMemUsage );

    calcSIDAllocations( &gSettings, pMemUsage );
    /* move results back into Transport Input Struct*/
    translateMemConfigToGUIPage( &gSettings, settings );
    #if ( PRINTF==1 )
    printf( "\nAfter Calculator:\n " );
    printSettings( settings, pMemUsage );
    #endif

    return( 0 );
} /* Memconfig_AppUsageCalculate */