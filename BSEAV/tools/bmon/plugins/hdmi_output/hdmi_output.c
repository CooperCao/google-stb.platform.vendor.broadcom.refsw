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
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "bstd.h"
#include "bkni.h"
#include "bkni_multi.h"

#if NXCLIENT_SUPPORT
#include "nxclient.h"
#include "nexus_platform_client.h"
#else
#include "nexus_platform.h"
#endif /* if NXCLIENT_SUPPORT */

#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include "hdmi_output.h"

/*#define BMON_TRACE_ENABLE*/
#include "bmon_defines.h"
#include "bmon_convert.h"
#include "bmon_json.h"

#define FPRINTF  NOFPRINTF /* fprintf to enable, NOFPRINTF to disable */

/**
 *  Function: This function will collect all requred hdmi output data and output in JSON format
 **/
int hdmi_output_get_data(
        const char * filter,
        char *       data,
        size_t       data_size
        )
{
    int                          rc         = 0;
    cJSON *                      objectRoot = NULL;
    cJSON *                      objectData = NULL;
    size_t                       numObjects = 0;
    int                          i          = 0;
    NEXUS_InterfaceName          interfaceName;
    NEXUS_PlatformObjectInstance objects[MAX_OBJECTS];
    NEXUS_Error                  nxrc;

    assert(NULL != filter);
    assert(NULL != data);
    assert(0 < data_size);

    /* initialize cJSON */
    objectRoot = json_Initialize();
    CHECK_PTR_ERROR_GOTO("Unable to allocate JSON object", objectRoot, rc, -1, error);

    /* generate JSON header */
    objectData = json_GenerateHeader(objectRoot, HDMI_OUTPUT_PLUGIN_NAME, HDMI_OUTPUT_PLUGIN_DESCRIPTION, NULL, HDMI_OUTPUT_PLUGIN_VERSION);
    CHECK_PTR_ERROR_GOTO("Unable to generate JSON header", objectData, rc, -1, error);

#if NEXUS_HAS_HDMI_OUTPUT
    NEXUS_Platform_GetDefaultInterfaceName(&interfaceName);
    strcpy(interfaceName.name, "NEXUS_HdmiOutput");

    nxrc = NEXUS_Platform_GetObjects(&interfaceName, objects, MAX_OBJECTS, &numObjects);
    CHECK_NEXUS_ERROR_GOTO("Failure retrieving Nexus platform objects", rc, nxrc, error);

    for (i = 0; (i < numObjects) && (i < NEXUS_NUM_HDMI_OUTPUTS); i++)
    {
        cJSON *                objectHdmiOutput = NULL;
        cJSON *                pRetJSON         = NULL;
        NEXUS_HdmiOutputHandle hHdmiOutput      = (NEXUS_HdmiOutputHandle)objects[i].object;
        NEXUS_HdmiOutputStatus status;

        if (NULL == hHdmiOutput)
        {
            continue;
        }

        /* get HDMI data from Nexus */
        nxrc = NEXUS_HdmiOutput_GetStatus(hHdmiOutput, &status);
        CHECK_NEXUS_ERROR_GOTO("Failure retrieving HDMI output status", rc, nxrc, error);

        /* generate JSON for HDMI status */

        /* do not use filter for hdmi_output - we always want this included for each output */
        objectHdmiOutput = json_AddObject(objectData, NO_FILTER, objectData, "output");
        CHECK_PTR_ERROR_GOTO("Failure adding Hdmi Output JSON object", objectHdmiOutput, rc, -1, error);

        /* do not use filter for index - we always want this included */
        json_AddNumber(objectHdmiOutput, NO_FILTER, objectData, "index", status.index);

        json_AddBool(objectHdmiOutput, filter, objectData, "connected", status.connected);
        json_AddBool(objectHdmiOutput, filter, objectData, "rxPowered", status.rxPowered);
        json_AddString(objectHdmiOutput, filter, objectData, "videoFormat", videoFormatToString(status.videoFormat));
        json_AddString(objectHdmiOutput, filter, objectData, "aspectRatio", aspectRatioToString(status.aspectRatio));
        json_AddString(objectHdmiOutput, filter, objectData, "colorSpace", colorSpaceToString(status.colorSpace));
        json_AddNumber(objectHdmiOutput, filter, objectData, "colorDepth", status.colorDepth);
        json_AddString(objectHdmiOutput, filter, objectData, "audioFormat", audioCodecToString(status.audioFormat));
        json_AddNumber(objectHdmiOutput, filter, objectData, "audioSamplingRate", status.audioSamplingRate);
        json_AddNumber(objectHdmiOutput, filter, objectData, "audioSamplingSize", status.audioSamplingSize);
        json_AddNumber(objectHdmiOutput, filter, objectData, "audioChannelCount", status.audioChannelCount);

        /* generate JSON for TX hardware status */
        {
            cJSON * txHardware = NULL;

            txHardware = json_AddObject(objectHdmiOutput, filter, objectData, "txHardware");
            CHECK_PTR_GOTO(txHardware, skipTxHardware);

            json_AddBool(txHardware, filter, objectData, "clockPower", status.txHardwareStatus.clockPower);

            /* check filter once for entire array */
            if (true == json_CheckFilter(txHardware, filter, objectData, "channelPower"))
            {
                cJSON * channelPowerArray = NULL;

                channelPowerArray = json_AddArray(txHardware, NO_FILTER, objectData, "channelPower");
                CHECK_PTR_GOTO(channelPowerArray, skipChannelPower);

                for (i = 0; i < sizeof(status.txHardwareStatus.channelPower) / sizeof(status.txHardwareStatus.channelPower[0]); i++)
                {
                    json_AddBool(channelPowerArray, NO_FILTER, objectData, NULL, status.txHardwareStatus.channelPower[i]);
                }
            }
skipChannelPower:

            json_AddBool(txHardware, filter, objectData, "hotplugInterruptEnabled", status.txHardwareStatus.hotplugInterruptEnabled);
            json_AddNumber(txHardware, filter, objectData, "hotplugCounter", status.txHardwareStatus.hotplugCounter);
            json_AddNumber(txHardware, filter, objectData, "rxSenseCounter", status.txHardwareStatus.rxSenseCounter);
            json_AddBool(txHardware, filter, objectData, "scrambling", status.txHardwareStatus.scrambling);
            json_AddNumber(txHardware, filter, objectData, "unstableFormatDetectedCounter", status.txHardwareStatus.unstableFormatDetectedCounter);
        }
skipTxHardware:

        /* generate JSON for RX hardware status */
        {
            cJSON * rxHardware = NULL;

            rxHardware = json_AddObject(objectHdmiOutput, filter, objectData, "rxHardware");
            CHECK_PTR_GOTO(rxHardware, skipRxHardware);

            json_AddBool(rxHardware, filter, objectData, "descrambling", status.rxHardwareStatus.descrambling);
        }
skipRxHardware:

        /* generate JSON for RX EDID status */
        {
            cJSON * rxEdid = NULL;

            rxEdid = json_AddObject(objectHdmiOutput, filter, objectData, "rxEdid");
            CHECK_PTR_GOTO(rxEdid, skipRxEdid);

            json_AddBool(rxEdid, filter, objectData, "hdmiDevice", status.hdmiDevice);
            json_AddString(rxEdid, filter, objectData, "monitorName", status.monitorName);

            {
                cJSON * monitorRange = NULL;

                monitorRange = json_AddObject(rxEdid, filter, objectData, "monitorRange");
                CHECK_PTR_GOTO(monitorRange, skipMonitorRange);

                json_AddNumber(monitorRange, filter, objectData, "minVertical", status.monitorRange.minVertical);
                json_AddNumber(monitorRange, filter, objectData, "maxVertical", status.monitorRange.maxVertical);
                json_AddNumber(monitorRange, filter, objectData, "minHorizontal", status.monitorRange.minHorizontal);
                json_AddNumber(monitorRange, filter, objectData, "maxHorizontal", status.monitorRange.maxHorizontal);
                json_AddNumber(monitorRange, filter, objectData, "maxPixelClock", status.monitorRange.maxPixelClock);
                json_AddNumber(monitorRange, filter, objectData, "secondaryTiming", status.monitorRange.secondaryTiming);

                /* check filter once for entire array */
                if (true == json_CheckFilter(monitorRange, filter, objectData, "secondaryTimingParameters"))
                {
                    cJSON * secondaryTimingParameters = NULL;

                    secondaryTimingParameters = json_AddArray(monitorRange, NO_FILTER, objectData, "secondaryTimingParameters");
                    CHECK_PTR_GOTO(secondaryTimingParameters, skipMonitorRange);

                    for (i = 0; i < sizeof(status.monitorRange.secondaryTimingParameters) / sizeof(status.monitorRange.secondaryTimingParameters[0]); i++)
                    {
                        json_AddNumber(secondaryTimingParameters, NO_FILTER, objectData, NULL, status.monitorRange.secondaryTimingParameters[i]);
                    }
                }
            }
skipMonitorRange:

            json_AddString(rxEdid, filter, objectData, "preferredVideoFormat", videoFormatToString(status.preferredVideoFormat));

            /* check filter once for entire array */
            if (true == json_CheckFilter(rxEdid, filter, objectData, "videoFormatSupported"))
            {
                cJSON * videoFormatSupported = NULL;

                videoFormatSupported = json_AddArray(rxEdid, NO_FILTER, objectData, "videoFormatSupported");
                CHECK_PTR_GOTO(videoFormatSupported, skipVideoFormatSupported);

                for (i = 0; i < NEXUS_VideoFormat_eMax; i++)
                {
                    cJSON * objectString = NULL;

                    if (false == status.videoFormatSupported[i])
                    {
                        continue;
                    }

                    objectString = cJSON_CreateString(videoFormatToString((NEXUS_VideoFormat)i));
                    CHECK_PTR_GOTO(objectString, skipVideoFormatSupported);

                    cJSON_AddItemToArray(videoFormatSupported, objectString);
                }
            }
skipVideoFormatSupported:

            /* check filter once for entire array */
            if (true == json_CheckFilter(rxEdid, filter, objectData, "hdmi3dFormatSupported"))
            {
                cJSON * hdmi3dFormatsSupported = NULL;

                hdmi3dFormatsSupported = json_AddArray(rxEdid, NO_FILTER, objectData, "hdmi3dFormatSupported");
                CHECK_PTR_GOTO(hdmi3dFormatsSupported, skipHdmi3dFormatsSupported);

                for (i = 0; i < NEXUS_VideoFormat_eMax; i++)
                {
                    cJSON * objectString = NULL;

                    if (false == status.hdmi3DFormatsSupported[i])
                    {
                        continue;
                    }

                    objectString = cJSON_CreateString(videoFormatToString((NEXUS_VideoFormat)i));
                    CHECK_PTR_GOTO(objectString, skipHdmi3dFormatsSupported);

                    cJSON_AddItemToArray(hdmi3dFormatsSupported, objectString);
                }
            }
skipHdmi3dFormatsSupported:

            {
                cJSON * monitorColorimetry = NULL;

                monitorColorimetry = json_AddObject(rxEdid, filter, objectData, "monitorColorimetry");
                CHECK_PTR_GOTO(monitorColorimetry, skipMonitorColorimetry);

                /* check filter once for entire array */
                if (true == json_CheckFilter(monitorColorimetry, filter, objectData, "extColorimetrySupport"))
                {
                    cJSON * extColorimetrySupport = NULL;

                    extColorimetrySupport = json_AddArray(monitorColorimetry, NO_FILTER, objectData, "extColorimetrySupport");
                    CHECK_PTR_GOTO(extColorimetrySupport, skipMonitorColorimetry);

                    for (i = 0; i < NEXUS_HdmiEdidColorimetryDbSupport_eMax; i++)
                    {
                        cJSON * objectString = NULL;

                        if (false == status.monitorColorimetry.extendedColorimetrySupported[i])
                        {
                            continue;
                        }

                        objectString = cJSON_CreateString(edidColorimetryDbToString((NEXUS_HdmiEdidColorimetryDbSupport)i));
                        CHECK_PTR_GOTO(objectString, skipMonitorColorimetry);

                        cJSON_AddItemToArray(extColorimetrySupport, objectString);
                    }
                }

                /* check filter once for entire array */
                if (true == json_CheckFilter(monitorColorimetry, filter, objectData, "metadataProfileSupport"))
                {
                    cJSON * metadataProfileSupport = NULL;

                    metadataProfileSupport = json_AddArray(monitorColorimetry, NO_FILTER, objectData, "metadataProfileSupport");
                    CHECK_PTR_GOTO(metadataProfileSupport, skipMonitorColorimetry);

                    for (i = 0; i < NEXUS_HdmiEdidColorimetryDbMetadataProfile_eMax; i++)
                    {
                        cJSON * objectString = NULL;

                        if (false == status.monitorColorimetry.metadataProfileSupported[i])
                        {
                            continue;
                        }

                        objectString = cJSON_CreateString(edidColorimetryDbMetadataProfileToString((NEXUS_HdmiEdidColorimetryDbMetadataProfile)i));
                        CHECK_PTR_GOTO(objectString, skipMonitorColorimetry);

                        cJSON_AddItemToArray(metadataProfileSupport, objectString);
                    }
                }
            }
skipMonitorColorimetry:

            /* check filter once for entire array */
            if (true == json_CheckFilter(rxEdid, filter, objectData, "audioCodecSupported"))
            {
                cJSON * audioCodecSupported = NULL;

                audioCodecSupported = json_AddArray(rxEdid, NO_FILTER, objectData, "audioCodecSupported");
                CHECK_PTR_GOTO(audioCodecSupported, skipAudioCodecSupported);

                for (i = 0; i < NEXUS_AudioCodec_eMax; i++)
                {
                    cJSON * objectString = NULL;

                    if (false == status.audioCodecSupported[i])
                    {
                        continue;
                    }

                    objectString = cJSON_CreateString(audioCodecToString((NEXUS_AudioCodec)i));
                    CHECK_PTR_GOTO(objectString, skipAudioCodecSupported);

                    cJSON_AddItemToArray(audioCodecSupported, objectString);
                }
            }
skipAudioCodecSupported:

            json_AddNumber(rxEdid, filter, objectData, "maxAudioPcmChannels", status.maxAudioPcmChannels);
            json_AddNumber(rxEdid, filter, objectData, "physicalAddressA", status.physicalAddressA);
            json_AddNumber(rxEdid, filter, objectData, "physicalAddressB", status.physicalAddressB);
            json_AddNumber(rxEdid, filter, objectData, "physicalAddressC", status.physicalAddressC);
            json_AddNumber(rxEdid, filter, objectData, "physicalAddressD", status.physicalAddressD);

            {
                cJSON * autoLipsyncInfo = NULL;

                autoLipsyncInfo = json_AddObject(rxEdid, filter, objectData, "autoLipsyncInfo");
                CHECK_PTR_GOTO(autoLipsyncInfo, skipAutoLipsyncInfo);

                json_AddNumber(autoLipsyncInfo, filter, objectData, "videoLatency", status.autoLipsyncInfo.videoLatency);
                json_AddNumber(autoLipsyncInfo, filter, objectData, "audioLatency", status.autoLipsyncInfo.audioLatency);
                json_AddNumber(autoLipsyncInfo, filter, objectData, "interlacedVideoLatency", status.autoLipsyncInfo.interlacedVideoLatency);
                json_AddNumber(autoLipsyncInfo, filter, objectData, "interlacedAudioLatency", status.autoLipsyncInfo.interlacedAudioLatency);
            }
skipAutoLipsyncInfo:

            json_AddString(rxEdid, filter, objectData, "eotf", eotfToString(status.eotf));
        }
skipRxEdid:

        /* generate JSON for basic EDID status */
        {
            NEXUS_HdmiOutputBasicEdidData basicEdidData;
            cJSON * basicEdid = NULL;

            basicEdid = json_AddObject(objectHdmiOutput, filter, objectData, "basicEdid");
            CHECK_PTR_GOTO(basicEdid, skipBasicEdid);

            nxrc = NEXUS_HdmiOutput_GetBasicEdidData(hHdmiOutput, &basicEdidData);
            if (NEXUS_SUCCESS != nxrc)
            {
                /* failure getting basic edid data so add JSON status indicating failure */
                json_AddString(basicEdid, NO_FILTER, objectData, "status", "Failure retreiving HDMI basic EDID data");
                goto error;
            }

            /* check filter once for entire array */
            if (true == json_CheckFilter(basicEdid, filter, objectData, "vendorID"))
            {
                cJSON * vendorID = NULL;

                vendorID = json_AddArray(basicEdid, NO_FILTER, objectData, "vendorID");
                CHECK_PTR_GOTO(vendorID, skipVendorID);

                json_AddNumber(vendorID, NO_FILTER, objectData, NULL, basicEdidData.vendorID[0]);
                json_AddNumber(vendorID, NO_FILTER, objectData, NULL, basicEdidData.vendorID[1]);
            }
skipVendorID:

            /* check filter once for entire array */
            if (true == json_CheckFilter(basicEdid, filter, objectData, "productID"))
            {
                cJSON * productID = NULL;

                productID = json_AddArray(basicEdid, NO_FILTER, objectData, "productID");
                CHECK_PTR_GOTO(productID, skipProductID);

                json_AddNumber(productID, NO_FILTER, objectData, NULL, basicEdidData.productID[0]);
                json_AddNumber(productID, NO_FILTER, objectData, NULL, basicEdidData.productID[1]);
            }
skipProductID:

            /* check filter once for entire array */
            if (true == json_CheckFilter(basicEdid, filter, objectData, "serialNum"))
            {
                cJSON * serialNum = NULL;

                serialNum = json_AddArray(basicEdid, NO_FILTER, objectData, "serialNum");
                CHECK_PTR_GOTO(serialNum, skipSerialNum);

                json_AddNumber(serialNum, NO_FILTER, objectData, NULL, basicEdidData.serialNum[0]);
                json_AddNumber(serialNum, NO_FILTER, objectData, NULL, basicEdidData.serialNum[1]);
                json_AddNumber(serialNum, NO_FILTER, objectData, NULL, basicEdidData.serialNum[2]);
                json_AddNumber(serialNum, NO_FILTER, objectData, NULL, basicEdidData.serialNum[3]);
            }
skipSerialNum:

            json_AddNumber(basicEdid, filter, objectData, "manufWeek", basicEdidData.manufWeek);
            json_AddNumber(basicEdid, filter, objectData, "manufYear", basicEdidData.manufYear);
            json_AddNumber(basicEdid, filter, objectData, "edidVersion", basicEdidData.edidVersion);
            json_AddNumber(basicEdid, filter, objectData, "edidRevision", basicEdidData.edidRevision);
            json_AddNumber(basicEdid, filter, objectData, "maxHorizSize", basicEdidData.maxHorizSize);
            json_AddNumber(basicEdid, filter, objectData, "maxVertSize", basicEdidData.maxVertSize);
            json_AddNumber(basicEdid, filter, objectData, "extensions", basicEdidData.extensions);
            json_AddNumber(basicEdid, filter, objectData, "features", basicEdidData.features);
        }
skipBasicEdid:
        continue;
    }
#endif /* NEXUS_HAS_HDMI_OUTPUT */

error:
    /* copy JSON data to supplied buffer */
    rc = json_Print(objectRoot, data, data_size);
    CHECK_ERROR("Failure printing JSON to allocated buffer", rc);

    json_Uninitialize(&objectRoot);

    if (0 <= rc)
    {
        /* return size of data if not error value */
        rc = strlen(data);
    }

    return(rc);
} /* hdmi_output_get_data */

#if defined (BMON_PLUGIN)
#define PAYLOAD_SIZE  (4 * 1024)
/**
 *  Function: This function will coordinate collecting hdmi output data and send the JSON data back
 *            to the browser or curl or wget.
 **/
int main(
        int    argc,
        char * argv[],
        char * envv[]
        )
{
    int    rc              = 0;
    char   filterDefault[] = "/";
    char * pFilter         = filterDefault;
    char   payload[PAYLOAD_SIZE];

#if NEXUS_HAS_HDMI_OUTPUT
    {
        NxClient_JoinSettings joinSettings;
        NEXUS_Error           nxrc;

        /* connect to NxServer */
        NxClient_GetDefaultJoinSettings(&joinSettings);
        joinSettings.mode = NEXUS_ClientMode_eVerified;
        nxrc              = NxClient_Join(&joinSettings);
        CHECK_NEXUS_ERROR_GOTO("Failure NxClient Join", rc, nxrc, errorNxClient);
    }
#endif /* if NEXUS_HAS_HDMI_OUTPUT */

    /* if the caller provided a filter, use the caller's filter */
    if (argc > 1)
    {
        pFilter = argv[1];
    }

    memset(payload, 0, sizeof(payload));

    rc = hdmi_output_get_data(pFilter, payload, PAYLOAD_SIZE);
    CHECK_ERROR_GOTO("Failure retrieving HDMI output data from Nexus", rc, error);

    /* send response back to user */
    printf("%s\n", payload);
    fflush(stdout);

error:

#if NEXUS_HAS_HDMI_OUTPUT
    NxClient_Uninit();
errorNxClient:
#endif

    return(rc);
} /* main */

#endif /* defined(BMON_PLUGIN) */