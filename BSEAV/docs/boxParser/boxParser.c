/******************************************************************************
 * Copyright (C) 2017 Broadcom.  The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 * This program is the proprietary software of Broadcom and/or its licensors,
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
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bbox.h"
#include "boxParserFormat.h"

void getXvdCapabilities(BBOX_Xvd_Config xvdConfig)
{
    char buffer[TEMP_BUFFER_SIZE];

    startTable("vid_decoder_capabilities", "Hardware Decoder,Capabilities");

    for (int i = 0; i < BBOX_XVD_MAX_DECODERS; ++i) {
        startRow(0);
        if (xvdConfig.stInstance[i].stDevice.numChannels != 0) {

            snprintf( buffer, TEMP_BUFFER_SIZE, "%d", i);
            addElement("Hardware Decoder", buffer);

            startTable("hw_decode_capabilities", "Software Index,Max Decode Capabilities");

            for (int j = 0; j < xvdConfig.stInstance[i].stDevice.numChannels; ++j) {

                startRow(0);

                snprintf(buffer, TEMP_BUFFER_SIZE, "%d", xvdConfig.stInstance[i].stDevice.stChannel[j].nexusIndex);
                addElement("Software Instance", buffer);

                char * resolution;

                switch (xvdConfig.stInstance[i].stDevice.stChannel[j].DecoderUsage.maxResolution) {
                    default:
                    case BBOX_XVD_DecodeResolution_eHD:
                        resolution = "HD";
                        break;
                    case BBOX_XVD_DecodeResolution_eSD:
                        resolution = "SD";
                        break;
                    case BBOX_XVD_DecodeResolution_eCIF:
                        resolution = "CIF";
                        break;
                    case BBOX_XVD_DecodeResolution_eQCIF:
                        resolution = "QCIF";
                        break;
                    case BBOX_XVD_DecodeResolution_e4K:
                        resolution = "UHD";
                        break;
                }
                snprintf(buffer, TEMP_BUFFER_SIZE, "%s %dfps %dbit", resolution,
                        xvdConfig.stInstance[i].stDevice.stChannel[j].DecoderUsage.framerate,
                        xvdConfig.stInstance[i].stDevice.stChannel[j].DecoderUsage.bitDepth);
                addElement("Max Decode Capabilities", buffer);

                endRow();
            }
            endTable();
        }
        endRow();
    }
    endTable();
}

void getVdcDisplayCapabilities(BBOX_Vdc_Capabilities vdcCapabilities)
{
    BFMT_VideoInfo videoInfo;
    char buffer[TEMP_BUFFER_SIZE];

    startTable("display_capabilities", "Display,Max VEC Output Format,Max HDMI Output Format,Mosaic Support,Windows");

    for (int i = 0; i < BBOX_VDC_DISPLAY_COUNT; ++i)
    {
        startRow(0);
        if (vdcCapabilities.astDisplay[i].bAvailable) {

            BBOX_Vdc_Display_Capabilities display = vdcCapabilities.astDisplay[i];
            char input[32];
            char output[32];

            BFMT_GetVideoFormatInfo(display.eMaxVideoFmt,&videoInfo);
            strncpy(input,videoInfo.pchFormatStr,31);

            BFMT_GetVideoFormatInfo(display.eMaxHdmiDisplayFmt,&videoInfo);
            strncpy(output,videoInfo.pchFormatStr,31);

            snprintf(buffer, TEMP_BUFFER_SIZE, "%d", i);
            addElement("Display", buffer);

            addElement("Max VEC output format", (char*)&input[15]);
            addElement("Max HDMI output format", (char*)&output[15]);

            if (display.eMosaicModeClass && (display.eMosaicModeClass != BBOX_Vdc_MosaicModeClass_eDisregard))
            {
                snprintf(buffer, TEMP_BUFFER_SIZE, "%d", display.eMosaicModeClass);
                addElement("Mosaic Support", buffer);
            }
            else
            {
                addElement("Mosaic Support", "None");
            }

            startTable("windows", "Window Type,Size Limit");

            for (int j = 0 ; j < BBOX_VDC_WINDOW_COUNT_PER_DISPLAY; ++j) {
                startRow(0);
                if (display.astWindow[j].bAvailable) {
                    unsigned widthFraction = display.astWindow[j].stSizeLimits.ulWidthFraction;
                    unsigned heightFraction = display.astWindow[j].stSizeLimits.ulHeightFraction;

                    /* Window 2 for GFX, 0 and 1 for video. */
                    switch (j)
                    {
                        case 2:
                            addElement("Type", "Graphics");
                            break;
                        case 1:
                            addElement("Type", "PIP Video");
                            break;
                        case 0:
                            addElement("Type", "Main Video");
                            break;
                        default:
                            addElement("Type", "Video");
                            break;
                    }

                    switch (heightFraction)
                    {
                        case BBOX_VDC_DISREGARD: /* Intentional fall through */
                        case 1:
                            addElement("Size Limit", "Full screen");
                            break;
                        case 2:
                            addElement("Size Limit", "Quarter screen");
                            break;
                        default:
                            snprintf(buffer, TEMP_BUFFER_SIZE, "%dx%d", widthFraction, heightFraction);
                            addElement("Size Limit", buffer);

                            break;
                    }
                }
                endRow();
            }
            endTable();
        }
        endRow();
    }
    endTable();
}

bool getVceAvailable(unsigned int boxMode)
{
    unsigned int box = 1, boxMax = 64;

    /* Short circuit loop below if we know the box mode we're interested in. */
    /* 0 presumes check all box modes, > 0 just that particular mode */
    if (boxMode) {
        box = boxMode;
        boxMax = boxMode+1;
    }

    while (box < boxMax) {
        BBOX_Handle   boxHandle;
        BBOX_Settings boxSettings;

        boxSettings.ulBoxId = box++;

        if (!BBOX_Open(&boxHandle,&boxSettings)) {
            BBOX_Config boxConfig;

            BBOX_GetConfig(boxHandle,&boxConfig);

            if (boxConfig.stVce.stInstance[0].uiChannels)
                return true;
        }
    }

    return false;
}

void getVceCapabilities(BBOX_Vce_Capabilities vceCapabilities)
{
    if (!getVceAvailable(vceCapabilities.uiBoxId)) return;

    startTable("encoder_capabilities", "Hardware ID,Channels,Max format");

    for (int i = 0; i < BBOX_VCE_MAX_INSTANCE_COUNT; ++i) {

        if (vceCapabilities.stInstance[i].uiChannels) {

            BFMT_VideoInfo videoInfo;
            char buffer[TEMP_BUFFER_SIZE];
            char format[32];
            unsigned channelsBitMask = vceCapabilities.stInstance[i].uiChannels;
            unsigned channelCount = 0;

            startRow(0);

            snprintf(buffer, TEMP_BUFFER_SIZE, "%d", vceCapabilities.stInstance[i].uiInstance);
            addElement("Hardware ID", buffer);

            while (channelsBitMask) {
                ++channelCount;
                channelsBitMask = channelsBitMask >> 1;
            }

            snprintf(buffer, TEMP_BUFFER_SIZE, "%d", channelCount);
            addElement("Channels", buffer);

            BFMT_GetVideoFormatInfo(vceCapabilities.stInstance[i].eVideoFormat,&videoInfo);
            strncpy(format,videoInfo.pchFormatStr,31);
            addElement("Max format", (char*)&format[15]);

            endRow();
        }
    }

    endTable();
}

int main (int argc, char *argv[])
{
    BBOX_Handle   boxHandle;
    BBOX_Settings boxSettings;
    BBOX_Config   boxConfig;
    BERR_Code     err = 0;
    unsigned      rowCounter=0;
    char          buffer[TEMP_BUFFER_SIZE];

    startHeader();

    snprintf(buffer, TEMP_BUFFER_SIZE, "%s%s", "Box Mode,Num MEMC,Video Decoder Capabilities,Display Capabilities,Num RAAGA",((getVceAvailable(0))? ",Hardware Encoder Capabilities":""));
    startTable("box_modes",buffer);

/* Hopefully we won't have more than 63 box modes for a chip.
 * This also won't cover temp modes which have 4 digits
 * Box 0 also has special meaning as the default RTS for a chip. */
    for (int box = 1; box < 64; ++box)
    {
        boxSettings.ulBoxId = box;
        err = BBOX_Open(&boxHandle,&boxSettings);

        if (!err)
        {
            err = BBOX_GetConfig(boxHandle,&boxConfig);

            if (!err)
            {
                ++rowCounter;
                startRow(((rowCounter %2) == 0 )); /* Highlight alternate rows */

                snprintf( buffer, TEMP_BUFFER_SIZE, "%d", box);
                addElement("Box Mode", buffer);

                snprintf( buffer, TEMP_BUFFER_SIZE, "%d", boxConfig.stMemConfig.ulNumMemc);
                addElement("Num MEMC", buffer);

                getXvdCapabilities(boxConfig.stXvd);
                getVdcDisplayCapabilities(boxConfig.stVdc);

                snprintf( buffer, TEMP_BUFFER_SIZE, "%d", boxConfig.stAudio.numDsps);
                addElement("Num RAAGA", buffer);

                getVceCapabilities(boxConfig.stVce);

                endRow();
            }
        }

        BBOX_Close(boxHandle);
    }

    endTable();
    endHeader();

    return 0;
}
