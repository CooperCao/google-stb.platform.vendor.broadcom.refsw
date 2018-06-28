/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif

#include "nxclient.h"
#include "nexus_platform.h"
#include "thermal_config.h"
#include "nexus_pid_channel.h"
#include "nexus_base_os.h"
#if NEXUS_HAS_TRANSPORT
#include "nexus_playpump.h"
#endif
#include "nexus_parser_band.h"
#include "nexus_avs.h"
#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"
#include "power.h"
#include "power_utils.h"
#include "plugin_header.h"
#include "bmon_utils.h"
#include "bmon_cpu_defines.h"
#include "bmon_cpu_get_frequency.h"
#include "bmon_utils.h"
#include "bmon_defines.h"
#include "bmon_json.h"
#include "bchp_clkgen.h"
#include "bmon_get_uptime.h"

#define SYS_THERMAL_PATH "/sys/devices/virtual/thermal/"

#if NXCLIENT_SUPPORT
BDBG_MODULE( power_plugin );
#endif /* NXCLIENT_SUPPORT */

static void addPowerStatusToJson(
    const NEXUS_PowerStatus *resource,
    const char              *str,
    const char              *filter,
    cJSON                   *objectNexusPower,
    cJSON                   *objectData
    )
{
    int    rc         = 0;
    cJSON *objectItem = objectNexusPower;
    char   description[32];

    if (false == ( resource->clock || resource->sram || resource->phy || resource->frequency ))
    {
        return;
    }

    if (true == cJSON_IsArray( objectNexusPower ))
    {
        objectItem = cJSON_CreateObject();
        CHECK_PTR_ERROR_GOTO( "Unable to crate cJSON object", objectItem, rc, -1, error );
        cJSON_AddItemToArray( objectNexusPower, objectItem );
    }

    if (resource->clock)
    {
        snprintf( description, sizeof( description ), "clock", str );
        json_AddNumber( objectItem, filter, objectData, description, resource->clock==NEXUS_PowerState_eOn ? 1 : 0 );
    }
    if (resource->sram)
    {
        snprintf( description, sizeof( description ), "sram", str );
        json_AddNumber( objectItem, filter, objectData, description, resource->sram==NEXUS_PowerState_eOn ? 1 : 0 );
    }
    if (resource->phy)
    {
        snprintf( description, sizeof( description ), "phy", str );
        json_AddNumber( objectItem, filter, objectData, description, resource->phy==NEXUS_PowerState_eOn ? 1 : 0 );
    }
    if (resource->frequency)
    {
        snprintf( description, sizeof( description ), "freqHz", str );
        json_AddNumber( objectItem, filter, objectData, description, resource->frequency );
    }

error:
    return;
}                                                          /* addPowerStatusToJson */

static void addPowerStatus(
    NEXUS_PlatformStatus *platformStatus,
    const char           *filter,
    cJSON                *objectNexusPower,
    cJSON                *objectData
    )
{
    NEXUS_Error rc;
    unsigned    i;
    char        buf[64];

#ifndef NEXUS_POWER_MANAGEMENT
    /*fprintf(stderr, "NEXUS POWER MANAGEMENT IS DISABLED!!!\n");*/
#endif

#if NEXUS_HAS_DISPLAY
    {
        cJSON *objectNexusPowerDisplay = json_AddObject( objectNexusPower, filter, objectData, "display" );
        CHECK_PTR_GOTO( objectNexusPowerDisplay, skipDisplay );

        /*fprintf(stderr, "\nDisplay\n");*/
        cJSON *objectNexusPowerBvn = json_AddObject( objectNexusPowerDisplay, NO_FILTER, objectData, "bvn" );
        CHECK_PTR_ERROR_GOTO( "Failure adding objectNexusPowerRfm JSON object", objectNexusPowerBvn, rc, -1, error );
        addPowerStatusToJson( &platformStatus->displayModuleStatus.power.bvn, "bvn", filter, objectNexusPowerBvn, objectData );

        cJSON *arrayDisplayVec = json_AddArray( objectNexusPowerDisplay, NO_FILTER, objectData, "vecs" );
        for (i = 0; i<NEXUS_MAX_DISPLAYS; i++) {
            if (platformStatus->displayModuleStatus.power.vec[i].clock ||
                platformStatus->displayModuleStatus.power.vec[i].sram ||
                platformStatus->displayModuleStatus.power.vec[i].phy)
            {
                addPowerStatusToJson( &platformStatus->displayModuleStatus.power.vec[i], "vec", filter, arrayDisplayVec, objectData );
            }
        }
        cJSON *arrayDisplayDac = NULL;
        arrayDisplayDac = json_AddArray( objectNexusPowerDisplay, NO_FILTER, objectData, "dacs" );
        for (i = 0; i<NEXUS_MAX_VIDEO_DACS; i++) {
            if (platformStatus->displayModuleStatus.power.dacs[i].clock ||
                platformStatus->displayModuleStatus.power.dacs[i].sram ||
                platformStatus->displayModuleStatus.power.dacs[i].phy)
            {
                addPowerStatusToJson( &platformStatus->displayModuleStatus.power.dacs[i], "dac", filter, arrayDisplayDac, objectData );
            }
        }

        cJSON *objectNexusPower656 = json_AddObject( objectNexusPowerDisplay, NO_FILTER, objectData, "656" );
        CHECK_PTR_ERROR_GOTO( "Failure adding objectNexusPowerRfm JSON object", objectNexusPower656, rc, -1, error );
        addPowerStatusToJson( &platformStatus->displayModuleStatus.power.ccir656Output, "656", filter, objectNexusPower656, objectData );
    }

skipDisplay:

#endif /* if NEXUS_HAS_DISPLAY */
#if NEXUS_HAS_RFM
    {
        cJSON *objectNexusPowerRfm = json_AddObject( objectNexusPower, filter, objectData, "rfm" );
        CHECK_PTR_GOTO( objectNexusPowerVideoDecoder, skipRfm );

        addPowerStatusToJson( &platformStatus->rfmModuleStatus.power.core, "rfm", NO_FILTER, objectNexusPowerRfm, objectData );
    }

skipRfm:

#endif /* if NEXUS_HAS_RFM */
#if NEXUS_HAS_VIDEO_DECODER
    {
        /*fprintf(stderr, "\nVideo Decoder\n");*/
        cJSON *objectNexusPowerVideoDecoder = json_AddObject( objectNexusPower, NO_FILTER, objectData, "videoDecoder" );
        CHECK_PTR_GOTO( objectNexusPowerVideoDecoder, skipVideoDecoder );

        cJSON *arrayVideoDecoders = NULL;
        arrayVideoDecoders = json_AddArray( objectNexusPowerVideoDecoder, filter, objectData, "decoders" );
        for (i = 0; i<NEXUS_MAX_VIDEO_DECODERS; i++) {
            if  (platformStatus->videoDecoderModuleStatus.power.core[i].clock /* || platformStatus->videoDecoderModuleStatus.power.core[i].phy */)
            {
                addPowerStatusToJson( &platformStatus->videoDecoderModuleStatus.power.core[i], "decoder", filter, arrayVideoDecoders, objectData );
            }
        }
    }

skipVideoDecoder:

#endif /* if NEXUS_HAS_VIDEO_DECODER */
#if NEXUS_HAS_PICTURE_DECODER
    {
        /*fprintf(stderr, "\nPicture Decoder\n");*/
        cJSON *objectNexusPowerPictureDecoder = json_AddObject( objectNexusPower, NO_FILTER, objectData, "pictureDecoder" );
        CHECK_PTR_GOTO( objectNexusPowerPictureDecoder, skipPictureDecoder );

        addPowerStatusToJson( &platformStatus->pictureDecoderModuleStatus.power.core, "decoder", filter, objectNexusPowerPictureDecoder, objectData );
    }

skipPictureDecoder:

#endif /* if NEXUS_HAS_PICTURE_DECODER */

#if NEXUS_HAS_VIDEO_ENCODER
    {
        /*fprintf(stderr, "\nVideo Encoder\n");*/
        cJSON *objectNexusPowerVideoEncoder = json_AddObject( objectNexusPower, NO_FILTER, objectData, "videoEncoder" );
        CHECK_PTR_GOTO( objectNexusPowerVideoEncoder, skipVideoEncoder );

        cJSON *arrayVideoEncoders = NULL;
        arrayVideoEncoders = json_AddArray( objectNexusPowerVideoEncoder, NO_FILTER, objectData, "videoEncoders" );
        for (i = 0; i<NEXUS_MAX_VIDEO_ENCODERS; i++) {
            if (platformStatus->videoEncoderModuleStatus.power.core[i].clock || platformStatus->videoEncoderModuleStatus.power.core[i].phy)
            {
                addPowerStatusToJson( &platformStatus->videoEncoderModuleStatus.power.core[i], "encoder", filter, arrayVideoEncoders, objectData );
            }
        }
    }

skipVideoEncoder:

#endif /* if NEXUS_HAS_VIDEO_ENCODER */
#if NEXUS_HAS_AUDIO
    {
        cJSON *objectNexusPowerAudio = json_AddObject( objectNexusPower, filter, objectData, "audio" );
        CHECK_PTR_GOTO( objectNexusPowerAudio, skipAudio );

        cJSON *objectNexusPowerAudioAio = json_AddObject( objectNexusPowerAudio, filter, objectData, "aio" );
        CHECK_PTR_ERROR_GOTO( "Failure adding objectNexusPowerAudioAio JSON object", objectNexusPowerAudioAio, rc, -1, error );
        addPowerStatusToJson( &platformStatus->audioModuleStatus.power.aio, "aio", filter, objectNexusPowerAudioAio, objectData );

        cJSON *arrayAudioPll = NULL;
        arrayAudioPll = json_AddArray( objectNexusPowerAudio, NO_FILTER, objectData, "pll" );
        for (i = 0; i<NEXUS_MAX_AUDIO_PLLS; i++) {
            if (platformStatus->audioModuleStatus.power.pll[i].clock || platformStatus->audioModuleStatus.power.pll[i].phy)
            {
                addPowerStatusToJson( &platformStatus->audioModuleStatus.power.pll[i], "pll", filter, arrayAudioPll, objectData );
            }
        }

        cJSON *arrayAudioDecoder = NULL;
        arrayAudioDecoder = json_AddArray( objectNexusPowerAudio, NO_FILTER, objectData, "decoders" );
        for (i = 0; i<NEXUS_MAX_AUDIO_DECODERS; i++) {
            if (platformStatus->audioModuleStatus.power.decoder[i].clock || platformStatus->audioModuleStatus.power.decoder[i].phy)
            {
                addPowerStatusToJson( &platformStatus->audioModuleStatus.power.decoder[i], "decoder", filter, arrayAudioDecoder, objectData );
            }
        }

        cJSON *arrayAudioDac = NULL;
        arrayAudioDac = json_AddArray( objectNexusPowerAudio, filter, objectData, "dac" );
        for (i = 0; i<NEXUS_MAX_AUDIO_DAC_OUTPUTS; i++) {
            if (platformStatus->audioModuleStatus.power.dacs[i].clock || platformStatus->audioModuleStatus.power.dacs[i].phy)
            {
                addPowerStatusToJson( &platformStatus->audioModuleStatus.power.dacs[i], "dac", filter, arrayAudioDac, objectData );
            }
        }

        cJSON *arrayAudioI2s = NULL;
        arrayAudioI2s = json_AddArray( objectNexusPowerAudio, filter, objectData, "i2s" );
        for (i = 0; i<NEXUS_MAX_AUDIO_I2S_OUTPUTS; i++) {
            if (platformStatus->audioModuleStatus.power.i2s[i].clock || platformStatus->audioModuleStatus.power.i2s[i].phy)
            {
                addPowerStatusToJson( &platformStatus->audioModuleStatus.power.i2s[i], "i2s", filter, arrayAudioI2s, objectData );
            }
        }

        cJSON *arrayAudioSpdif = NULL;
        arrayAudioSpdif = json_AddArray( objectNexusPowerAudio, filter, objectData, "spdif" );
        for (i = 0; i<NEXUS_MAX_AUDIO_SPDIF_OUTPUTS; i++) {
            if (platformStatus->audioModuleStatus.power.spdif[i].clock || platformStatus->audioModuleStatus.power.spdif[i].phy)
            {
                addPowerStatusToJson( &platformStatus->audioModuleStatus.power.spdif[i], "spdif", filter, arrayAudioSpdif, objectData );
            }
        }
    }

skipAudio:

#endif /* if NEXUS_HAS_AUDIO */
    {
    cJSON *objectNexusPowerGraphics = NULL;
#if NEXUS_HAS_GRAPHICS2D
    {
        /*fprintf(stderr, "\nGraphics\n");*/
        objectNexusPowerGraphics = json_AddObject( objectNexusPower, filter, objectData, "graphics" );
        CHECK_PTR_GOTO( objectNexusPowerGraphics, skipGraphics2d );

        cJSON *arrayGraphics = NULL;
        arrayGraphics = json_AddArray( objectNexusPowerGraphics, filter, objectData, "2d" );
        FPRINTF( stderr, "%s - graphics2DModuleStatus (%u) \n", __FUNCTION__, platformStatus->graphics2DModuleStatus.power.core[0].clock );
        for (i = 0; i<NEXUS_MAX_GRAPHICS2D_CORES; i++) {
            if (platformStatus->graphics2DModuleStatus.power.core[i].clock || platformStatus->graphics2DModuleStatus.power.core[i].phy)
            {
                addPowerStatusToJson( &platformStatus->graphics2DModuleStatus.power.core[i], "core", filter, arrayGraphics, objectData );
            }
        }
    }

skipGraphics2d:

#endif /* if NEXUS_HAS_GRAPHICS2D */
#if NEXUS_HAS_GRAPHICSV3D
    {
        cJSON *objectNexusPowerGraphics3d = json_AddObject(objectNexusPowerGraphics, filter, objectData, "3d");
        CHECK_PTR_GOTO( objectNexusPowerGraphics3d, skipGraphics3d );
        FPRINTF( stderr, "%s - graphicsv3dModuleStatus (%u) \n", __FUNCTION__, platformStatus->graphicsv3dModuleStatus.power.core );
        addPowerStatusToJson( &platformStatus->graphicsv3dModuleStatus.power.core, "core", filter, objectNexusPowerGraphics3d, objectData );

    }

skipGraphics3d:
    FPRINTF(stderr, "%s \n", __FUNCTION__ );

#endif /* if NEXUS_HAS_GRAPHICSV3D */

    }

#if NEXUS_HAS_HDMI_INPUT
    {
        /*fprintf(stderr, "\nHdmi\n");*/
        cJSON *objectNexusPowerHdmiInput = json_AddObject( objectNexusPower, filter, objectData, "hdmiInput" );
        CHECK_PTR_GOTO( objectNexusPowerHdmiInput, skipHdmiInput );

        addPowerStatusToJson( &platformStatus->hdmiInputModuleStatus.power.core, "input", filter, objectNexusPowerHdmiInput, objectData );
    }

skipHdmiInput:

#endif /* if NEXUS_HAS_HDMI_INPUT */
#if NEXUS_HAS_HDMI_OUTPUT
    {
        cJSON *objectNexusPowerHdmiOutput = json_AddObject( objectNexusPower, filter, objectData, "hdmiOutput" );
        CHECK_PTR_GOTO( objectNexusPowerHdmiOutput, skipHdmiOutput );

        cJSON *arrayHdmiOutput = NULL;
        arrayHdmiOutput = json_AddArray( objectNexusPowerHdmiOutput, filter, objectData, "outputs" );
        for (i = 0; i<NEXUS_MAX_HDMI_OUTPUTS; i++) {
            if (platformStatus->hdmiOutputModuleStatus.power.core[i].clock ||
                platformStatus->hdmiOutputModuleStatus.power.core[i].phy)
            {
                addPowerStatusToJson( &platformStatus->hdmiOutputModuleStatus.power.core[i], "core", filter, arrayHdmiOutput, objectData );
            }
        }
    }

skipHdmiOutput:

#endif /* if NEXUS_HAS_HDMI_OUTPUT */
#if NEXUS_HAS_SMARTCARD
    {
        /*fprintf(stderr, "\nSmartcard\n");*/
        cJSON *objectNexusPowerSmartcard = json_AddObject( objectNexusPower, filter, objectData, "smartcard" );
        CHECK_PTR_GOTO( objectNexusPowerSmartcard, skipSmartcard );

        cJSON *arraySmartcard = NULL;
        arraySmartcard = json_AddArray( objectNexusPowerSmartcard, filter, objectData, "cards" );
        for (i = 0; i<NEXUS_MAX_SMARTCARD_CHANNELS; i++) {
            if (platformStatus->smartcardModuleStatus.power.core[i].clock ||
                platformStatus->smartcardModuleStatus.power.core[i].phy)
            {
                addPowerStatusToJson( &platformStatus->smartcardModuleStatus.power.core[i], "core", filter, arraySmartcard, objectData );
            }
        }
    }

skipSmartcard:

#endif /* if NEXUS_HAS_SMARTCARD */
#if NEXUS_HAS_TRANSPORT
    {
        cJSON *objectNexusPowerTransport = json_AddObject( objectNexusPower, filter, objectData, "transport" );
        CHECK_PTR_GOTO( objectNexusPowerTransport, skipTransport );

        addPowerStatusToJson( &platformStatus->transportModuleStatus.power.transport, "transport", filter, objectNexusPowerTransport, objectData );

        cJSON *objectNexusPowerTransportRemux = json_AddObject( objectNexusPowerTransport, filter, objectData, "remux" );
        CHECK_PTR_ERROR_GOTO( "Failure adding objectNexusPowerTransportRemux JSON object", objectNexusPowerTransportRemux, rc, -1, error );
        addPowerStatusToJson( &platformStatus->transportModuleStatus.power.remux, "remux", filter, objectNexusPowerTransportRemux, objectData );

        cJSON *objectNexusPowerTransportTsio = json_AddObject( objectNexusPowerTransport, filter, objectData, "tsio" );
        CHECK_PTR_ERROR_GOTO( "Failure adding objectNexusPowerTransportTsio JSON object", objectNexusPowerTransportTsio, rc, -1, error );
        addPowerStatusToJson( &platformStatus->transportModuleStatus.power.tsio, "tsio", filter, objectNexusPowerTransportTsio, objectData );
    }

skipTransport:

#endif /* if NEXUS_HAS_TRANSPORT */

error:
    return;
}                                                          /* addPowerStatus */

/**
 *  Function: This function will read an "signed long" integer value from the
 *  specified filename that is prepended with the specified path.
 **/
static int sysfsGetDlong(
    char *path,
    char *filename,
    long *p_dlong
    )
{
    FILE *fd;
    int   ret = -1;
    char  filepath[256];

    snprintf( filepath, 256, "%s%s", path, filename );

    fd = fopen( filepath, "r" );
    /*printf("%s: filepath (%s); fd %p\n", __FUNCTION__, filepath, (void*)fd );*/
    if (!fd)
    {
        return( ret );
    }
    ret = fscanf( fd, "%ld", p_dlong );
    fclose( fd );

    return( 0 );
}                                                          /* sysfsGetDlong */

/* copied from tmon.c */
/**
 *  Function: This function will read a "string" from the
 *  specified filename that is prepended with the specified path.
 **/
static int sysfs_get_string(
    char *path,
    char *filename,
    char *str
    )
{
    FILE *fd;
    int   ret = -1;
    char  filepath[256];

    snprintf( filepath, 256, "%s/%s", path, filename );

    fd = fopen( filepath, "r" );
    if (!fd)
    {
        return( ret );
    }
    ret = fscanf( fd, "%256s", str );
    fclose( fd );

    return( ret );
}                                                          /* sysfs_get_string */

/**
 *  Function: This function will read the temperature from the specified thermal zone.
 *  Thermal zones are generally numbered from 0 to 1.
 **/
static float getTemperature(
    unsigned int thermal_zone
    )
{
    int   rc           = 0;
    long  iTemperature = 0;
    float fTemperature = -999.0;
    char  thermal_zone_fn[64];
    char  thermal_mode[16];

    memset( thermal_mode, 0, sizeof( thermal_mode ));

    sprintf( thermal_zone_fn, "thermal_zone%u/mode", thermal_zone );
    rc = sysfs_get_string( SYS_THERMAL_PATH, thermal_zone_fn, thermal_mode );
    /*printf("%s: get_string returned (%d); mode (%s)\n", __FUNCTION__, rc, thermal_mode );*/
    if (rc != -1)
    {
        /* if the zone is enabled and active */
        if (strcmp( thermal_mode, "enabled" ) == 0)
        {
            sprintf( thermal_zone_fn, "thermal_zone%u/temp", thermal_zone );
            rc = sysfsGetDlong( SYS_THERMAL_PATH, thermal_zone_fn, &iTemperature );
            if (rc != -1)
            {
                fTemperature = (float)( iTemperature/1000 ) + ((float)( iTemperature%1000 ))/1000.;
                /*printf("iTemperature %ld; fTemperature is %5.1f; tz_fn (%s/%s) \n", iTemperature, fTemperature, SYS_THERMAL_PATH, thermal_zone_fn );*/
            }
        }
    }

    return( fTemperature /* to test trip_point colors ... + 56. + 15.*/ );
}                                                          /* getTemperature */

static double fprintfTimeOfDay(
    const char *caller,
    const char *description
    )
{
    double         rv = 0;
    struct timeval tv;

    gettimeofday( &tv, NULL );
    rv  = tv.tv_usec;
    rv /= 1000000;
    rv += tv.tv_sec;
    fprintf( stderr, "%s - %d.%06d ... %s \n", caller, tv.tv_sec, tv.tv_usec, description );
    return( rv );
}

static double convertUptimeToUnixTime(
    float uptime
    )
{
    struct timeval tv;
    float          f_uptime = 0.0;

    bmon_get_uptime_float( &f_uptime );
    gettimeofday( &tv, NULL );
    return( 0 );
}

/**
 *  Function: This function will
 *     1. parse the filter
 *     2. Collect required data
 *     3. Convert to json format
 *     4. Return number of bytes written to the buffer
 **/
int power_get_data(
    const char *filter,
    char       *data,
    size_t      data_size
    )
{
    int                    rc     = 0;
    NEXUS_Error            nxrc   = 0;
    double                 dTime  = 0.0;
    unsigned long long int uptime = 0;
    char                   errorMsg[64];
    cJSON                 *objectRoot  = NULL;
    cJSON                 *objectData  = NULL;
    cJSON                 *objectDvfs  = NULL;
    cJSON                 *objectPower = NULL;
    cJSON                 *objectNexusPower             = NULL;
    cJSON                 *objectThermal                = NULL;
    cJSON                 *objectVoltage                = NULL;
    cJSON                 *objectClkgen1                = NULL; /* results from power_tree.txt */
    cJSON                 *objectClkgen2                = NULL; /* results from power_clk_tree.txt */
    cJSON                 *objectStandby                = NULL;
    cJSON                 *arrayPowerSavingTechniques   = NULL;
    cJSON                 *arrayCpuFrequenciesSupported = NULL;
    cJSON                 *arrayCpuFrequenciesCurrently = NULL;
    cJSON                 *arrayThermalCoolingAgents    = NULL;

    if (( data == NULL ) || ( data_size ==0 ))
    {
        return( -1 );
    }

    bmon_get_uptime( &uptime );

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO( "Unable to allocate JSON object", objectRoot, rc, -1, error );

    /* generate JSON header */
    objectData = json_GenerateHeader( objectRoot, POWER_PLUGIN_NAME, POWER_PLUGIN_DESCRIPTION, NULL, POWER_PLUGIN_VERSION );
    CHECK_PTR_ERROR_GOTO( "Unable to generate JSON header", objectData, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectThermal = json_AddObject( objectData, NO_FILTER, objectData, "thermal" );
    CHECK_PTR_ERROR_GOTO( "Failure adding thermal JSON object", objectThermal, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectVoltage = json_AddObject( objectData, NO_FILTER, objectData, "voltage" );
    CHECK_PTR_ERROR_GOTO( "Failure adding voltage JSON object", objectVoltage, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectDvfs = json_AddObject( objectData, NO_FILTER, NULL, "dvfs" );
    CHECK_PTR_ERROR_GOTO( "Failure adding dvfs JSON object", objectDvfs, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectPower = json_AddObject( objectData, NO_FILTER, objectData, "power" );
    CHECK_PTR_ERROR_GOTO( "Failure adding power JSON object", objectPower, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectNexusPower = json_AddObject( objectData, NO_FILTER, objectData, "softwarePowerState" );
    CHECK_PTR_ERROR_GOTO( "Failure adding nexusPower JSON object", objectNexusPower, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectClkgen1 = json_AddArray( objectData, NO_FILTER, objectData, "chipPowerState" );
    CHECK_PTR_ERROR_GOTO( "Failure adding objectClkgen1 JSON object", objectClkgen1, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectClkgen2 = json_AddArray( objectData, NO_FILTER, objectData, "pllsTopLevel" );
    CHECK_PTR_ERROR_GOTO( "Failure adding objectClkgen2 JSON object", objectClkgen2, rc, -1, error );

    /* do not use filter for base object - we always want this included for each output */
    objectStandby = json_AddObject( objectData, NO_FILTER, NULL, "standby" );
    CHECK_PTR_ERROR_GOTO( "Failure adding standby JSON object", objectStandby, rc, -1, error );

    {
        char *contents = NULL;
        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", 128 );

        bmon_trim_line( contents );

        if (contents)
        {
            json_AddString( objectDvfs, filter, objectData, "powerSavingSetting", contents );
        }
        else
        {
            json_AddString( objectDvfs, filter, objectData, "powerSavingSetting", "NULL" );
        }
        FREE_SAFE( contents );
    }

    if (true == json_CheckFilter( objectDvfs, filter, objectData, "powerSavingTechniques" ))
    {
        char         *contents = NULL;
        char         *pos      = NULL;
        char         *space    = NULL;                     /* used to find delimiter between words */
        unsigned char count    = 0;

        arrayPowerSavingTechniques = json_AddArray( objectDvfs, filter, objectData, "powerSavingTechniques" );

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors", 128 );
        pos      = contents;                               /* start looking for words at the beginning of the file */

        bmon_trim_line( contents );                        /* conservative ondemand userspace powersave performance */

        while (pos && *pos) {
            cJSON *objectString = NULL;

            space = strchr( pos, ' ' );                    /* find the end of this word */
            if (space) {*space = '\0'; }

            objectString = cJSON_CreateString( pos );
            CHECK_PTR_GOTO( objectString, skipDvfs );

            cJSON_AddItemToArray( arrayPowerSavingTechniques, objectString );

            if (space)
            {
                pos = ++space;
            }
            else
            {
                pos = NULL;
            }
            count++;
        }
skipDvfs:
        FREE_SAFE( contents );
    }

    if (true == json_CheckFilter( objectDvfs, filter, objectData, "cpuFrequenciesKhz" ))
    {
        int          cpu         = 0;
        int          numCpusConf = sysconf( _SC_NPROCESSORS_CONF );
        unsigned int frequencies[BMON_CPU_MAX_NUM];
        if (numCpusConf > BMON_CPU_MAX_NUM)
        {
            numCpusConf = BMON_CPU_MAX_NUM;
        }

        arrayCpuFrequenciesCurrently = json_AddArray( objectDvfs, filter, objectData, "cpuFrequenciesKhz" );

        bmon_cpu_get_frequencies( &( frequencies[0] ));

        for (cpu = 0; cpu<numCpusConf; cpu++) {            /* 331000,331000,331000,331000 */
            cJSON *objectNum = NULL;

            objectNum = cJSON_CreateNumber( frequencies[cpu] );
            CHECK_PTR_GOTO( objectNum, skipCpuFreqs );

            cJSON_AddItemToArray( arrayCpuFrequenciesCurrently, objectNum );
        }
    }
skipCpuFreqs:

    if (true == json_CheckFilter( objectDvfs, filter, objectData, "cpuFrequenciesKhzSupported" ))
    {
        char        *contents = NULL;
        char        *pos      = NULL;
        char        *space    = NULL;                      /* used to find delimiter between words */
        unsigned int count    = 0;
        unsigned int freq     = 0;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies", 128 );

        if (contents)                                      /* 331000 552000 828000 1104000 1656000 */
        {
            bmon_trim_line( contents );

            arrayCpuFrequenciesSupported = json_AddArray( objectDvfs, filter, objectData, "cpuFrequenciesKhzSupported" );

            pos = contents;                                /* start looking for words at the beginning of the file */

            if (pos && strlen( pos ))
            {
                if (pos[strlen( pos ) -1] == '\n') {pos[strlen( pos ) -1] = '\0'; }
            }
            while (pos && *pos) {
                cJSON *objectNum = NULL;

                space = strchr( pos, ' ' );                /* find the end of this word */
                if (space) {*space = '\0'; }
                sscanf( pos, "%u", &freq );

                objectNum = cJSON_CreateNumber( freq );
                CHECK_PTR_GOTO( objectNum, skipCpuFreqsSupported );

                cJSON_AddItemToArray( arrayCpuFrequenciesSupported, objectNum );

                if (space)
                {
                    pos = ++space;
                }
                else
                {
                    pos = NULL;
                }
                count++;
            }
skipCpuFreqsSupported:
            FREE_SAFE( contents );
        }
    }

    {
        /* cat /sys/class/i2c-adapter/i2c-0/0-0040/iio:device0/in_power2_raw */
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/class/i2c-adapter/i2c-0/0-0040/iio:device0/in_power2_raw", 24 );
        bmon_trim_line( contents );

        if (contents && contents[0])
        {
            json_AddString( objectPower, filter, objectData, "systemPowerMw", contents );
        }
        else
        {
            json_AddNull( objectPower, filter, objectData, "systemPowerMw" );
        }
        FREE_SAFE( contents );
    }

#if WANT_POWER2_SCALE
    {
        /* cat /sys/class/i2c-adapter/i2c-0/0-0040/iio:device0/in_power2_scale */
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/class/i2c-adapter/i2c-0/0-0040/iio:device0/in_power2_scale", 24 );
        bmon_trim_line( contents );

        json_AddString( objectPower, filter, objectData, "inPower2Scale", contents ? contents : "could not open file" );
        FREE_SAFE( contents );
    }
#endif /* if WANT_POWER2_SCALE */

#if NXCLIENT_SUPPORT
    {
        int             rc;
        NEXUS_AvsStatus pStatus;

        rc = NEXUS_GetAvsStatus( &pStatus );
        if (rc)
        {
            json_AddNumber( objectPower, filter, objectData, "getAvsStatus", 0 );
        }
        else
        {
            json_AddNumber( objectPower, filter, objectData, "getAvsStatus", 1 );
            json_AddNumber( objectPower, filter, objectData, "voltageVolts", pStatus.voltage/1000. );                  /* %7.3fV */
            json_AddNumber( objectPower, filter, objectData, "temperatureDegreesCelcius", pStatus.temperature/1000. ); /* %7.2fC */
            json_AddString( objectPower, filter, objectData, "avsEnabled", pStatus.enabled ? "Enabled" : "Disabled" );
            json_AddString( objectPower, filter, objectData, "avsTracking", pStatus.tracking ? "Tracking" : "Idle" );
        }

        rc = NEXUS_GetAvsDomainStatus( NEXUS_AvsDomain_eCpu, &pStatus );
        if (!rc)
        {
            json_AddNumber( objectPower, filter, objectData, "cpuAvsDomainStatus", 1 );
            json_AddNumber( objectPower, filter, objectData, "cpuVoltageVolts", pStatus.voltage/1000. );                  /* %7.3fV */
            json_AddNumber( objectPower, filter, objectData, "cpuTemperatureDegreesCelcius", pStatus.temperature/1000. ); /* %7.2fC */
        }
        else
        {
            json_AddNumber( objectPower, filter, objectData, "cpuAvsDomainStatus", 0 );
        }
        json_AddNumber( objectPower, filter, objectData, "cpuAvsHeartbeat", pStatus.heartbeat );

        rc = NEXUS_GetAvsDomainStatus( NEXUS_AvsDomain_eMain, &pStatus );
        if (!rc)
        {
            json_AddNumber( objectPower, filter, objectData, "mainAvsDomainStatus", 1 );
            json_AddNumber( objectPower, filter, objectData, "mainVoltageVolts", pStatus.voltage/1000. );                  /* %7.3fV */
            json_AddNumber( objectPower, filter, objectData, "mainTemperatureDegreesCelcius", pStatus.temperature/1000. ); /* %7.2fC */
        }
        else
        {
            json_AddNumber( objectPower, filter, objectData, "mainAvsDomainStatus", 0 );
        }
        json_AddNumber( objectPower, filter, objectData, "mainAvsHeartbeat", pStatus.heartbeat );
    }
#endif /* NXCLIENT_SUPPORT */

#if NXCLIENT_SUPPORT
    {
        unsigned               agent = 0;
        struct timeval         tv    = {0, 0};
        char                   agentName[16];
        NxClient_ThermalStatus ThermalStatus;
        NEXUS_PlatformStatus   PlatformStatus;
        unsigned int           maxPriorities = 0;

        cJSON *objectThermalCoolingAgent[sizeof( ThermalStatus.priorityTable ) / sizeof( ThermalStatus.priorityTable[0] )];

        memset( errorMsg, 0, sizeof( errorMsg ));

        {
#if 0
            double time1 = 0, time2 = 0;
#endif
            memset( &ThermalStatus, 0, sizeof( ThermalStatus ));

#if 0
            time1 = fprintfTimeOfDay( __FUNCTION__, "calling NxClient_GetThermalStatus()" );
#endif
            nxrc = NxClient_GetThermalStatus( &ThermalStatus );
#if 0
            time2 = fprintfTimeOfDay( __FUNCTION__, "after   NxClient_GetThermalStatus()" );
            fprintf( stderr, "%s - delta %12.6f seconds \n\n", __FUNCTION__, time2-time1 );
#endif
            if (nxrc != NEXUS_SUCCESS)
            {
                snprintf( errorMsg, sizeof( errorMsg ), "Error - NxClient_GetThermalStatus() failed" );
                json_AddString( objectThermal, filter, objectData, "status", "NULL" );
            }
            else
            {
                memset( &PlatformStatus, 0, sizeof( PlatformStatus ));
                nxrc = NEXUS_Platform_GetStatus( &PlatformStatus );
                if (nxrc != NEXUS_SUCCESS)
                {
                    snprintf( errorMsg, sizeof( errorMsg ), "Error - NEXUS_Platform_GetStatus() failed" );
                    json_AddString( objectThermal, filter, objectData, "status", "NULL" );
                }
                else
                {
                    addPowerStatus( &PlatformStatus, filter, objectNexusPower, objectData );
                }
            }
        }

        if (errorMsg[0] == '\0')
        {
            snprintf( errorMsg, sizeof( errorMsg ), "success" );
            json_AddString( objectThermal, filter, objectData, "status", errorMsg );
        }

        /*fprintf(stderr, "%s - V3d (%d) ... M2MC (%d) \n", __FUNCTION__, PlatformStatus.graphicsv3dModuleStatus.power.core.frequency,
            PlatformStatus.graphics2DModuleStatus.power.core[0].frequency);*/

        maxPriorities = sizeof( ThermalStatus.priorityTable ) / sizeof( ThermalStatus.priorityTable[0] );

        /* 514285714 */
        json_AddNumber( objectPower, filter, objectData, "graphicsV3dCoreFrequencyMhz", PlatformStatus.graphicsv3dModuleStatus.power.core.frequency/1000000 );
        json_AddNumber( objectPower, filter, objectData, "graphics2dCoreFrequencyMhz", PlatformStatus.graphics2DModuleStatus.power.core[0].frequency/1000000 );

        json_AddNumber( objectThermal, filter, objectData, "temperatureMillidegrees", ThermalStatus.temperature );
        /* json_AddNumber( objectThermal, filter, objectData, "userDefined", ThermalStatus.userDefined);
        json_AddNumber( objectThermal, filter, objectData, "level", ThermalStatus.level);*/
        json_AddNumber( objectThermal, filter, objectData, "maxPriorities", maxPriorities );
        arrayThermalCoolingAgents = json_AddArray( objectThermal, filter, objectData, "coolingAgents" );
        const char *coolingAgentName[NxClient_CoolingAgent_eMax] = {"unknown", "cpuPstate", "cpuIdle", "graphics3D", "graphics2D", "display", "stopPip", "stopMain", "user"};
        /* thermal_config thermalConfig;*/
        NxClient_ThermalConfiguration thermalConfig;
        nxrc = NxClient_GetThermalConfiguration( ThermalStatus.activeTempThreshold, &thermalConfig );
        FPRINTF( stderr, "thermal agent maxPriorities (%d) \n", maxPriorities );

        FPRINTF( stderr, "Agents ... \n" );
        struct timeval tv_now;
        gettimeofday( &tv_now, NULL );
        for (agent = 0; agent<maxPriorities; agent++) {
            /* if the agent has a name */
            if ( /* thermalConfig.priority_table[agent].name[0] != '\0' && */ ThermalStatus.priorityTable[agent].inUse)
            {
                snprintf( agentName, sizeof( agentName ), "agent%u", agent );
                objectThermalCoolingAgent[agent] = json_AddObject( arrayThermalCoolingAgents, filter, objectData, agentName );
                json_AddString( objectThermalCoolingAgent[agent], filter, objectData, "name", coolingAgentName[thermalConfig.priorityTable[agent].agent] );
                json_AddNumber( objectThermalCoolingAgent[agent], filter, objectData, "priority", agent );
                /* json_AddNumber( objectThermalCoolingAgent[agent], filter, objectData, "level", thermalConfig.priority_table[agent].level );
                json_AddNumber( objectThermalCoolingAgent[agent], filter, objectData, "used", ThermalStatus.priorityTable[agent].inUse );*/
                /*FPRINTF(stderr, "%10s-%2d ", thermalConfig.priority_table[agent].name, agent );*/

                NEXUS_GetWallclockFromTimestamp( &ThermalStatus.priorityTable[agent].lastAppliedTime, &tv );
                dTime = (double)tv.tv_sec+(double)tv.tv_usec/1000000;
                json_AddNumber( objectThermalCoolingAgent[agent], filter, objectData, "lastAppliedTime", dTime );
                FPRINTF( stderr, "%5.0f (%3d) ", dTime, ( tv_now.tv_sec - tv.tv_sec ));

                NEXUS_GetWallclockFromTimestamp( &ThermalStatus.priorityTable[agent].lastRemovedTime, &tv );
                dTime = (double)tv.tv_sec+(double)tv.tv_usec/1000000;
                json_AddNumber( objectThermalCoolingAgent[agent], filter, objectData, "lastRemovedTime", dTime );
                FPRINTF( stderr, "%5.0f (%3d) ", dTime, ( tv_now.tv_sec - tv.tv_sec ));
                FPRINTF( stderr, "\n" );
            }
        }
#if 0
        typedef struct NxClient_ThermalStatus
        {
            unsigned temperature;                          /* Current Temperature in degrees C*/
            bool     userDefined;                          /* True if user defined Cooling Agent needs to be applied*/
            unsigned level;                                /* Level of throttling for user defined cooling agent */
            struct {
                bool     inUse;                            /* Indicates whether cooling agent is applied or not */
                unsigned lastAppliedTime;                  /* Timestamp in ms when cooling agent was last applied */
                unsigned lastRemovedTime;                  /* Timestamp in ms when cooling agent was last removed */
            } priorityTable[32];                           /* index is priority from thermal.cfg file */
        } NxClient_ThermalStatus;

        typedef struct thermal_config
        {
            unsigned over_temp_threshold;                  /* Over Temp Threshold in degrees C at which thermal throttling is applied */
            unsigned over_temp_reset;                      /* Temperature at which chip resets */
            unsigned hysteresis;                           /* Hysteresis in degree C */
            unsigned polling_interval;                     /* Thermal polling interval in seconds */
            unsigned temp_delay;                           /* Delay in seconds to wait for temp to reduce */
            unsigned theta_jc;                             /* Thermal Resistance of the Box*/
            struct {
                char     name[32];                         /* Cooling agent name */
                unsigned level;                            /* level of throttling for a given cooling agent */
            } priority_table[32];
        } thermal_config;
#endif /* if 0 */

        json_AddNumber( objectThermal, filter, objectData, "overTemperatureThresholdMillidegrees", thermalConfig.overTempThreshold );
        json_AddNumber( objectThermal, filter, objectData, "overTemperatureResetMillidegrees", thermalConfig.overTempReset );
        json_AddNumber( objectThermal, filter, objectData, "hysteresis", thermalConfig.hysteresis );
        json_AddNumber( objectThermal, filter, objectData, "pollingIntervalMilliseconds", thermalConfig.pollInterval );
        json_AddNumber( objectThermal, filter, objectData, "temperatureDelayMilliseconds", thermalConfig.tempDelay );
        json_AddNumber( objectThermal, filter, objectData, "thetaJcMilliseconds", thermalConfig.thetaJC );
    }
#endif /* NXCLIENT_SUPPORT */

#if NXCLIENT_SUPPORT
    {
        char *contents = NULL;
        NEXUS_PlatformStandbyStatus PlatformStandbyStatus;
        NxClient_StandbyStatus      NxClientStandbyStatus;
        NEXUS_StandbySettings       StandbySettings;
        struct timeval              tv1 = {0, 0};
        struct timeval              tv2 = {0, 0};

        memset( &PlatformStandbyStatus, 0, sizeof( PlatformStandbyStatus ));
        memset( &NxClientStandbyStatus, 0, sizeof( NxClientStandbyStatus ));
        memset( &StandbySettings, 0, sizeof( StandbySettings ));
#if 0
        typedef struct NEXUS_StandbySettings
        {
            NEXUS_StandbyMode mode;
            struct {
                bool     ir;
                bool     uhf;
                bool     keypad;
                bool     gpio;
                bool     nmi;
                bool     cec;
                bool     transport;
                unsigned timeout;                          /* in seconds */
            } wakeupSettings;
            bool     openFrontend;                         /* If true, NEXUS_Platform_SetStandbySettings will initialize the frontend. */
            unsigned timeout;                              /* time (in milliseconds) for nexus to wait its internal activity to wind down */
        } NEXUS_StandbySettings;
#endif /* if 0 */
        NEXUS_Platform_GetStandbySettings( &StandbySettings );
#if 0
        typedef struct {                     typedef struct NxClient_StandbyStatus {
                                                 bool ir;                             NEXUS_StandbySettings settings;       /* the desired standby state */
                                                 bool uhf;                            NEXUS_PlatformStandbyStatus status;   /* wake up status */
                                                 bool keypad;                         bool standbyTransition;               /* Deprecated. */
                                                 bool gpio;                           NxClient_StandbyTransition transition;
                                                 bool nmi;                            NEXUS_Timestamp lastStandbyTimestamp; /* Last timestamp in ms when system entered standby */
                                                 bool cec;                            NEXUS_Timestamp lastResumeTimestamp;  /* Last timestamp in ms when system woke up */
                                                 bool transport;                      NEXUS_StandbyMode lastStandbyMode;    /* Mode when system was last in standby */
                                                 bool timeout;
                                             } NxClient_StandbyStatus; } wakeupStatus;
#endif /* if 0 */
        dTime = (double)uptime /1000;
        json_AddNumber( objectStandby, filter, objectData, "uptimeSec", dTime ); /* used to compare with lastStandby */

        NEXUS_Platform_GetStandbyStatus( &PlatformStandbyStatus );

        NxClient_GetStandbyStatus( &NxClientStandbyStatus );

        NEXUS_GetWallclockFromTimestamp( &NxClientStandbyStatus.lastStandbyTimestamp, &tv1 );
        dTime = (double)tv1.tv_sec+(double)tv1.tv_usec/1000000;
        json_AddNumber( objectStandby, filter, objectData, "lastStandbyTime", dTime );

        NEXUS_GetWallclockFromTimestamp( &NxClientStandbyStatus.lastResumeTimestamp, &tv2 );
        dTime = (double)tv2.tv_sec+(double)tv2.tv_usec/1000000;
        json_AddNumber( objectStandby, filter, objectData, "lastResumeTime", dTime );
        json_AddNumber( objectStandby, filter, objectData, "lastStandbyMode", NxClientStandbyStatus.lastStandbyMode );

        contents = bmon_get_file_contents_proc( "/proc/device-tree/bolt/reset-list", 128 );
        bmon_trim_line( contents );

        json_AddString( objectStandby, filter, objectData, "resetList", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }
#endif /* NXCLIENT_SUPPORT */

    {
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/brcm_avs_voltage", 12 );
        bmon_trim_line( contents );

        json_AddString( objectVoltage, filter, objectData, "cpu0AvsVoltage", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }

    {
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/brcm_avs_mode", 12 );
        bmon_trim_line( contents );

        json_AddString( objectVoltage, filter, objectData, "cpu0AvsMode", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }

    {
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/brcm_avs_frequency", 24 );
        bmon_trim_line( contents );

        json_AddString( objectVoltage, filter, objectData, "cpu0AvsFrequency", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }

    {
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/brcm_avs_pstate", 24 );
        bmon_trim_line( contents );

        json_AddString( objectVoltage, filter, objectData, "cpu0AvsPstate", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }

    {
        char *contents = NULL;

        contents = bmon_get_file_contents_proc( "/sys/devices/system/cpu/cpu0/cpufreq/brcm_avs_pmap", 64 );
        bmon_trim_line( contents );

        json_AddString( objectVoltage, filter, objectData, "cpu0AvsPmap", contents ? contents : "NULL" );
        FREE_SAFE( contents );
    }

    {
        char *htmlBuffer = NULL;
        void *arg1 = json_CheckFilter( objectClkgen1, filter, objectData, "chipPowerState" ) ? objectClkgen1 : NULL;
        void *arg2 = json_CheckFilter( objectClkgen2, filter, objectData, "pllsTopLevel" ) ? objectClkgen2 : NULL;

        htmlBuffer = get_clock_tree( filter, arg1, arg2 );

        if (htmlBuffer)
        {
            FPRINTF( stderr, "get_clock_tree() buffer len %d \n", strlen( htmlBuffer ));
        }
    }

#if 0
    {
        /* changed to use value returned from gaurav's api */
        int currentTemperature = getTemperature( 0 );
        json_AddNumber( objectTemperature, filter, objectData, "chipTemperatureCelcius", currentTemperature );
    }
#endif /* if 0 */

error:
    /* copy JSON data to supplied buffer */
    rc = json_Print( objectRoot, data, data_size );
    CHECK_ERROR( "Failure printing JSON to allocated buffer", rc );

    json_Uninitialize( &objectRoot );

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen( data );
    }

    return( rc );
}                                                          /* power_get_data */

#if defined ( BMON_PLUGIN )
/**
 *  Function: This function will coordinate collecting power data and once that is done,
 *            it will convert the power data to a JSON format and send the JSON data back
 *            to the browser or curl or wget.
 **/
#define PAYLOAD_SIZE ( 20 * 1024 )
int main(
    int   argc,
    char *argv[],
    char *envv[]
    )
{
    int   rc              = 0;
    char  filterDefault[] = "/";
    char *pFilter         = filterDefault;
    char  payload[PAYLOAD_SIZE];

#if NEXUS_HAS_HDMI_OUTPUT
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings( &joinSettings );
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join( &joinSettings );
        CHECK_NEXUS_ERROR_GOTO( "Failure NxClient Join", rc, nxrc, errorNxClient );
    }
#endif /* if NEXUS_HAS_HDMI_OUTPUT */

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset( payload, 0, sizeof( payload ));

    rc = power_get_data( pFilter, payload, PAYLOAD_SIZE );
    CHECK_ERROR_GOTO( "Failure retrieving power data from Nexus", rc, error );

    /* send response back to user */
    printf( "%s\n", payload );
    fflush( stdout );

error:

#if NEXUS_HAS_HDMI_OUTPUT
    NxClient_Uninit();
errorNxClient:
#endif

    return( rc );
}                                                          /* main */

#endif /* defined(BMON_PLUGIN) */
