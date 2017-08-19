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
#include "nexus_audio_module.h"
#include "nexus_audio_debug.h"

BDBG_MODULE(nexus_audio_debug);

NEXUS_ModuleHandle g_NEXUS_audioDebug;
NEXUS_ModuleHandle g_NEXUS_audioGraphDebug;
NEXUS_ModuleHandle g_NEXUS_audioVolumeDebug;

/* required for the debug output */
extern NEXUS_AudioDecoderHandle g_decoders[NEXUS_NUM_AUDIO_DECODERS];

#if BDBG_DEBUG_BUILD

void NEXUS_AudioDebug_GetOutputStatus(void);
void NEXUS_AudioDebug_P_PrintDigitalOutputStatus(BAPE_DebugDigitalOutputStatus *status);


void NEXUS_AudioDebug_P_PrintDigitalOutputStatus(BAPE_DebugDigitalOutputStatus *status)
{

    char temp[40];
    char *ptrTemp = temp;
    char binaryArray[33];

    BDBG_LOG(("        Output Status (%s)",status->pName));
    BDBG_LOG(("---------------------------------------------"));
    BDBG_LOG(("         Channel Status"));

    BDBG_LOG(("          (hex):   0  1  2  3  4  5  6  7"));
    BDBG_LOG(("          (hex):  %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x",
                    (status->cbits[0]&0x000000FF), ((status->cbits[0]&0x0000FF00) >> 8),
                    ((status->cbits[0]&0x00FF0000) >> 16), ((status->cbits[0]&0xFF000000)>>24),
                    (status->cbits[1]&0x000000FF), ((status->cbits[1]&0x0000FF00) >> 8),
                    ((status->cbits[1]&0x00FF0000) >> 16), ((status->cbits[1]&0xFF000000)>>24)));

    BDBG_LOG(("          (bin): 0.............................31"));
    BDBG_LOG(("          (bin): %s",
        BAPE_Debug_IntToBinaryString(status->cbits[0],32,binaryArray)));

    BDBG_LOG(("          (bin): 32............................63"));
    BDBG_LOG(("          (bin): %s",
         BAPE_Debug_IntToBinaryString(status->cbits[1],32,binaryArray)));

    BDBG_LOG(("---------------------------------------------"));

    BDBG_LOG(("            formatId: %s (%db)",
        status->formatId ? "PROFESSIONAL":"CONSUMER", status->formatId));
    if (status->formatId == 0) { /* Consumer*/
        BDBG_LOG(("     audio/non-audio: %sAUDIO (%db)",
            status->channelStatus.consumer.audio ? "NON-":"DIGITAL ", status->channelStatus.consumer.audio));
        BDBG_LOG(("           copyright: COPYRIGHT %sASSERTED (%db)",
            status->channelStatus.consumer.copyright ? "":"NOT ", status->channelStatus.consumer.copyright));

        if (status->channelStatus.consumer.audio == 0) { /* Digital Audio */
            switch(status->channelStatus.consumer.emphasis) {
            case 0:
                ptrTemp = "NONE";
                break;
            case 1:
                ptrTemp = "50/15 microseconds";
                break;
            default:
                ptrTemp = "Reserved (Audio Data)";
                break;
            }
        }
        else { /* NON AUDIO */
            switch(status->channelStatus.consumer.emphasis) {
            case 0:
                ptrTemp = "DIGITAL DATA";
                break;
            default:
                ptrTemp = "Reserved (Digital Data)";
                break;
            }
        }
        BDBG_LOG(("            emphasis: Pre-emphasis: %s (%sb)",ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.emphasis,3,binaryArray)));
        BDBG_LOG(("                mode: MODE %s (%db)", status->channelStatus.consumer.mode ? "RESERVED":"0", status->channelStatus.consumer.mode));

        switch(status->channelStatus.consumer.categoryCode) {
        case 0:
            ptrTemp = "GENERAL";
            break;
        case 4:
        case 12:
        case 14:
            ptrTemp = "BROADCAST RECP. OF DIGITAL AUDIO";
            break;
        default:
            ptrTemp = "Other";
            break;
        }
        BDBG_LOG(("        categoryCode: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.categoryCode,8,binaryArray)));

        switch(status->channelStatus.consumer.samplingFrequency) {
        case 0:
            ptrTemp = "44.1kHz";
            break;
        case 1:
            ptrTemp = "NOT INDICATED";
            break;
        case 2:
            ptrTemp = "48kHz";
            break;
        case 3:
            ptrTemp = "32kHz";
            break;
        case 4:
            ptrTemp = "22.05kHz";
            break;
        case 6:
            ptrTemp = "24kHz";
            break;
        case 8:
            ptrTemp = "88.2kHz";
            break;
        case 9:
            ptrTemp = "768kHz";
            break;
        case 10:
            ptrTemp = "96kHz";
            break;
        case 12:
            ptrTemp = "176.4kHz";
            break;
        case 14:
            ptrTemp = "192kHz";
            break;
        default:
            ptrTemp = "RESERVED";
            break;
        }
        BDBG_LOG(("        samplingFreq: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.samplingFrequency,4,binaryArray)));

        if (status->channelStatus.consumer.audio == 0) {
            BDBG_LOG(("       pcmWordLength: MAX WORD LENGTH %s bits (%db)", status->channelStatus.consumer.pcmWordLength ? "24":"20", status->channelStatus.consumer.pcmWordLength));

            switch(status->channelStatus.consumer.pcmSampleWordLength) {
            case 0:
                ptrTemp = "SAMPLE WORD LENGTH NOT INDICATED";
                break;
            case 1:
                ptrTemp = status->channelStatus.consumer.pcmWordLength ? "20 bits":"16 bits";
                break;
            case 2:
                ptrTemp = status->channelStatus.consumer.pcmWordLength ? "22 bits":"18 bits";
                break;
            case 4:
                ptrTemp = status->channelStatus.consumer.pcmWordLength ? "23 bits":"19 bits";
                break;
            case 5:
                ptrTemp = status->channelStatus.consumer.pcmWordLength ? "24 bits":"20 bits";
                break;
            case 6:
                ptrTemp = status->channelStatus.consumer.pcmWordLength ? "21 bits":"17 bits";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
            }
            BDBG_LOG((" pcmSampleWordLength: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.pcmSampleWordLength,3,binaryArray)));

            switch(status->channelStatus.consumer.pcmOrigSamplingFrequency) {
            case 0:
                ptrTemp = "NOT INDICATED";
                break;
            case 1:
                ptrTemp = "192kHz";
                break;
            case 2:
                ptrTemp = "12kHz";
                break;
            case 3:
                ptrTemp = "176.4kHz";
                break;
            case 5:
                ptrTemp = "96kHz";
                break;
            case 6:
                ptrTemp = "8kHz";
                break;
            case 7:
                ptrTemp = "88.2kHz";
                break;
            case 8:
                ptrTemp = "16kHz";
                break;
            case 9:
                ptrTemp = "24kHz";
                break;
            case 10:
                ptrTemp = "11.025kHz";
                break;
            case 11:
                ptrTemp = "22.05kHz";
                break;
            case 12:
                ptrTemp = "32kHz";
                break;
            case 13:
                ptrTemp = "48kHz";
                break;
            case 15:
                ptrTemp = "44.1kHz";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
            }
            BDBG_LOG((" pcmOrigSamplingFreq: %s (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.pcmOrigSamplingFrequency,4,binaryArray)));

            switch(status->channelStatus.consumer.pcmCgmsA) {
            case 0:
                ptrTemp = "COPY PREMITTED NO RESTRICTIONS";
                break;
            case 1:
                ptrTemp = "ONE GENERATION PERMITTED";
                break;
            case 2:
                ptrTemp = "UNUSED";
                break;
            case 3:
                ptrTemp = "NO COPY PREMITTED";
                break;
            }
            BDBG_LOG(("            pcmCgmsA: %s (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.consumer.pcmCgmsA,2,binaryArray)));
        }
    }
    else { /* Professional */
        BDBG_LOG(("    normal/non audio: %sAUDIO (%db)",
            status->channelStatus.professional.audio ? "NON-":"NORMAL ", status->channelStatus.professional.audio));

        switch(status->channelStatus.professional.emphasis) {
        case 0:
            ptrTemp = "Not Indicated";
            break;
        case 1:
            ptrTemp = "None";
            break;
        case 3:
            ptrTemp = "50/15 microseconds";
            break;
        case 7:
            ptrTemp = "CCITT J.17";
            break;
        default:
            ptrTemp = "Reserved";
            break;
        }

        BDBG_LOG(("            emphasis: Pre-emphasis: %s (%sb)",ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.emphasis,3,binaryArray)));

        BDBG_LOG(("  Sample Freq Locked: %s (%db)",status->channelStatus.professional.sampleRateLocked ? "UNLOCKED":"LOCKED", status->channelStatus.professional.sampleRateLocked));

        switch(status->channelStatus.professional.legacySampleRate) {
        case 0:
            ptrTemp = "NOT INDICATED";
            break;
        case 1:
            ptrTemp = "44.1kHz";
            break;
        case 2:
            ptrTemp = "48kHz";
            break;
        case 3:
            ptrTemp = "32kHz";
            break;
        default:
            ptrTemp = "INVALID";
            break;
        }
        BDBG_LOG(("    Sample Frequency: %s (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.professional.legacySampleRate,2,binaryArray)));

        switch(status->channelStatus.professional.mode) {
        case 0:
            ptrTemp = "NOT INDICATED";
            break;
        case 2:
            ptrTemp = "Stereophonic";
            break;
        case 4:
            ptrTemp = "Single Channel";
            break;
        case 8:
            ptrTemp = "Two-Channel";
            break;
        case 12:
            ptrTemp = "Primary/Secondary";
            break;
        case 1:
        case 9:
        case 14:
            ptrTemp = "Single Channel Double Frequency";
            break;
        case 15:
            ptrTemp = "Multichannel";
            break;
        default:
            ptrTemp = "Reserved";
            break;
        }
        BDBG_LOG(("Encoded Channel Mode: %s (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.professional.mode,4,binaryArray)));

        switch(status->channelStatus.professional.user) {
        case 0:
            ptrTemp = "NOT INDICATED";
            break;
        case 2:
            ptrTemp = "Conforms to IEC-60958-3";
            break;
        case 4:
            ptrTemp = "HDLC protocol";
            break;
        case 8:
            ptrTemp = "192 bit Block Structure";
            break;
        case 10:
            ptrTemp = "Reserved for Metadata";
            break;
        case 12:
            ptrTemp = "User Defined";
            break;
        default:
            ptrTemp = "Reserved";
            break;
        }
        BDBG_LOG(("   Encoded User Bits: %s (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.professional.user,4,binaryArray)));

        switch(status->channelStatus.professional.auxSampleBits) {
        case 0:
            ptrTemp = "Not Defined, 20 bits";
            break;
        case 2:
            ptrTemp = "20 bits";
            break;
        case 4:
            ptrTemp = "24 bits";
            break;
        case 12:
            ptrTemp = "User Defined";
            break;
        default:
            ptrTemp = "Reserved";
            break;
        }
        BDBG_LOG(("     AUX Sample Bits: MAX WORD LENGTH %s bits (%sb)", ptrTemp,
                BAPE_Debug_IntToBinaryString(status->channelStatus.professional.auxSampleBits,3,binaryArray)));

        if (status->channelStatus.professional.auxSampleBits == 0 || status->channelStatus.professional.auxSampleBits == 2) {
            switch (status->channelStatus.professional.wordLength) {
            case 0:
                ptrTemp = "SAMPLE WORD LENGTH NOT INDICATED";
                break;
            case 2:
                ptrTemp = "18 bits";
                break;
            case 4:
                ptrTemp = "19 bits";
                break;
            case 5:
                ptrTemp = "20 bits";
                break;
            case 6:
                ptrTemp = "16 bits";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
            }
        }
        else if (status->channelStatus.professional.auxSampleBits == 4) {
            switch (status->channelStatus.professional.wordLength) {
            case 0:
                ptrTemp = "SAMPLE WORD LENGTH NOT INDICATED";
                break;
            case 2:
                ptrTemp = "22 bits";
                break;
            case 4:
                ptrTemp = "23 bits";
                break;
            case 5:
                ptrTemp = "24 bits";
                break;
            case 6:
                ptrTemp = "20 bits";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
            }
        }
        else
        {
            ptrTemp = "Reserved";
        }
        BDBG_LOG(("  Source Word Length: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.wordLength,3,binaryArray)));

        switch (status->channelStatus.professional.alignment) {
        case 0:
            ptrTemp = "NOT INDICATED";
            break;
        case 1:
            ptrTemp = "SMPTE RP155";
            break;
        case 2:
            ptrTemp = "EBU R68";
            break;
        default:
            ptrTemp = "RESERVED";
            break;
        }
        BDBG_LOG(("     Alignment Level: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.alignment,2,binaryArray)));

        if (status->channelStatus.professional.multichannelMode == 0) {
            BDBG_LOG(("      Channel Number: %d (%sb)", status->channelStatus.professional.channelNumber, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.channelNumber,7,binaryArray)));
        }
        else {
            BDBG_LOG(("      Channel Number: %d (%sb)", status->channelStatus.professional.channelNumber, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.channelNumber,4,binaryArray)));
            switch (status->channelStatus.professional.alignment) {
            case 0:
                ptrTemp = "Mode 0";
                break;
            case 1:
                ptrTemp = "Mode 1";
                break;
            case 2:
                ptrTemp = "Mode 2";
                break;
            case 3:
                ptrTemp = "Mode 3";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
            }
            BDBG_LOG(("  Multichan Mode Num: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.multichannelModeNum,3,binaryArray)));
        }
        BDBG_LOG(("   Multichannel Mode: %s (%db)", status->channelStatus.professional.multichannelModeNum ? "DEFINED":"UNDEFINED", status->channelStatus.professional.multichannelModeNum));

        switch (status->channelStatus.professional.audioReference) {
        case 0:
            ptrTemp = "NOT A REFERENCE SIGNAL";
            break;
        case 1:
            ptrTemp = "GRADE 1";
            break;
        case 2:
            ptrTemp = "GRADE 2";
            break;
        default:
            ptrTemp = "RESERVED";
            break;
        }
        BDBG_LOG(("Dig. Audio Reference: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->channelStatus.professional.audioReference,2,binaryArray)));

        switch(status->channelStatus.professional.samplingFrequencyExt) {
        case 0:
            ptrTemp = "NOT INDICATED";
            break;
        case 1:
            ptrTemp = "24kHz";
            break;
        case 2:
            ptrTemp = "96kHz";
            break;
        case 3:
            ptrTemp = "192kHz";
            break;
        case 9:
            ptrTemp = "22.05kHz";
            break;
        case 10:
            ptrTemp = "88.8kHz";
            break;
        case 11:
            ptrTemp = "176.4kHz";
            break;
        default:
            ptrTemp = "RESERVED";
            break;
        }
        BDBG_LOG(("    Sample Frequency: %s (%sb)", ptrTemp,
                  BAPE_Debug_IntToBinaryString(status->channelStatus.professional.legacySampleRate,2,binaryArray)));

        BDBG_LOG(("    Freqency Scaling: %s (%db)", status->channelStatus.professional.sampleFreqScaling ? "ENABLED":"DISABLED", status->channelStatus.professional.sampleFreqScaling));
    }

    BDBG_LOG(("---------------------------------------------"));
    BDBG_LOG(("         Interface Configuration"));
    switch (status->type)
    {
        case 0:
            BDBG_LOG(("          PCM Mono"));
            break;
        case 1:
            BDBG_LOG(("          PCM Stereo"));
            break;
        case 2:
            BDBG_LOG(("          PCM 5.1"));
            break;
        case 3:
            BDBG_LOG(("          PCM 7.1"));
            break;
        case 5:
            BDBG_LOG(("          Compressed"));
            break;
        case 6:
            BDBG_LOG(("          Compressed 4x (HRA)"));
            break;
        case 7:
            BDBG_LOG(("          Compressed 16x (HBR)"));
            break;
        default:
            BDBG_LOG(("          Unknown output type"));
            break;
    }

    BDBG_LOG(("         %s",status->compressedAsPcm?"DTS-CD":""));
    BDBG_LOG(("         Sample Rate %d", status->sampleRate));
    BDBG_LOG(("---------------------------------------------"));
}

void NEXUS_AudioDebug_GetOutputStatus()
{

    BAPE_DebugStatus status;
    unsigned i;
    BERR_Code errCode;
    NEXUS_AudioCapabilities audioCapabilities;


    NEXUS_GetAudioCapabilities(&audioCapabilities);
    errCode = BAPE_Debug_GetStatus( g_NEXUS_audioModuleData.debugHandle,
                                    BAPE_DebugSourceType_eOutput,
                                    &status);

    if ( errCode == BERR_SUCCESS)
    {
#if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
        for (i=0; i < audioCapabilities.numOutputs.spdif; i++)
        {
            if (status.status.outputStatus.spdif[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintDigitalOutputStatus(&status.status.outputStatus.spdif[i]);
            }
        }
#endif

#if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
        for (i=0; i < audioCapabilities.numOutputs.hdmi; i++)
        {
            if (status.status.outputStatus.hdmi[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintDigitalOutputStatus(&status.status.outputStatus.hdmi[i]);
            }
        }
#endif
    }
}

static void NEXUS_AudioDebug_OutputPrint(void)
{
    NEXUS_AudioDebug_GetOutputStatus();
}

static void NEXUS_AudioDebug_P_ConfigPrint(NEXUS_AudioInputHandle input, int level)
{
    NEXUS_AudioInputHandle downstreamObject;
    NEXUS_AudioOutputHandle audioOutput;
    downstreamObject = NEXUS_AudioInput_P_LocateDownstream(input, NULL);
    while ( downstreamObject )
    {
        switch (downstreamObject->objectType)
        {
            case NEXUS_AudioInputType_eEncoder:
                {
                    NEXUS_AudioEncoderSettings encoderSettings;

                    NEXUS_AudioEncoder_GetSettings( downstreamObject->pObjectHandle, &encoderSettings);
                    BDBG_LOG(("%*s%s (%p)%s %s", level*4, "", downstreamObject->pName, (void *)downstreamObject->pObjectHandle, NEXUS_AudioInputFormatToString(downstreamObject->format), NEXUS_AudioCodecToString(encoderSettings.codec)));
                    NEXUS_AudioDebug_P_ConfigPrint(downstreamObject,level+1);
                }
                break;
            case NEXUS_AudioInputType_eDolbyDigitalReencode:
                {
                    NEXUS_DolbyDigitalReencodeHandle handle = downstreamObject->pObjectHandle;
                    BDBG_LOG(("%*s%s (%p)", level*4, "", downstreamObject->pName, (void *)downstreamObject->pObjectHandle));
                    NEXUS_AudioDebug_P_ConfigPrint(NEXUS_DolbyDigitalReencode_GetConnector(handle, NEXUS_AudioConnectorType_eStereo),level+1);
                    NEXUS_AudioDebug_P_ConfigPrint(NEXUS_DolbyDigitalReencode_GetConnector(handle, NEXUS_AudioConnectorType_eMultichannel),level+1);
                    NEXUS_AudioDebug_P_ConfigPrint(NEXUS_DolbyDigitalReencode_GetConnector(handle, NEXUS_AudioConnectorType_eCompressed),level+1);
                    if ( BAPE_GetDolbyMSVersion() == BAPE_DolbyMSVersion_eMS12 )
                    {
                        NEXUS_AudioDebug_P_ConfigPrint(NEXUS_DolbyDigitalReencode_GetConnector(handle, NEXUS_AudioConnectorType_eCompressed4x), level + 1);
                    }
                }
                break;
            default:
                BDBG_LOG(("%*s%s (%p) %s", level*4, "", downstreamObject->pName, (void *)downstreamObject->pObjectHandle, NEXUS_AudioInputFormatToString(downstreamObject->format)));
                NEXUS_AudioDebug_P_ConfigPrint(downstreamObject,level+1);
                break;
        }
        downstreamObject = NEXUS_AudioInput_P_LocateDownstream(input, downstreamObject);
    }

    audioOutput = NEXUS_AudioInput_P_LocateOutput(input, NULL);
    while ( audioOutput )
    {
        NEXUS_AudioOutputSettings outputSettings;
        NEXUS_AudioOutput_GetSettings(audioOutput, &outputSettings);
        switch ( audioOutput->objectType )
        {
            case NEXUS_AudioOutputType_eI2s:
            case NEXUS_AudioOutputType_eI2sMulti:
            case NEXUS_AudioOutputType_eSpdif:
                BDBG_LOG(("%*s%s: pll %d %s", level*4, "", audioOutput->pName,outputSettings.pll, NEXUS_AudioInputFormatToString(input->format)));
                break;
            case NEXUS_AudioOutputType_eHdmi:
            case NEXUS_AudioOutputType_eDummy:
                BDBG_LOG(("%*s%s: pll %d nco %d %s", level*4, "", audioOutput->pName,outputSettings.pll,outputSettings.nco, NEXUS_AudioInputFormatToString(input->format)));
                break;
            default:
                BDBG_LOG(("%*s%s: %s", level*4, "", audioOutput->pName, NEXUS_AudioInputFormatToString(input->format)));
                break;

        }
        audioOutput = NEXUS_AudioInput_P_LocateOutput(input, audioOutput);
    }
}

static uint32_t NEXUS_AudioDebug_BAPE_ChannelModeToInt(
    BAPE_ChannelMode mode
    )
{
    switch (mode)
    {
        case BAPE_ChannelMode_e1_0:
        case BAPE_ChannelMode_e1_1:
            return 1;
        case BAPE_ChannelMode_e2_0:
            return 2;
        case BAPE_ChannelMode_e2_1:
        case BAPE_ChannelMode_e3_0:
            return 3;
        case BAPE_ChannelMode_e2_2:
        case BAPE_ChannelMode_e3_1:
            return 4;
        case BAPE_ChannelMode_e3_2:
            return 5;
        case BAPE_ChannelMode_e3_4:
            return 7;
        default:
            return 0;
    }
}

static uint32_t NEXUS_AudioDebug_NumChannels(
    const BAPE_DecoderStatus *status
    )
{
    int count = 0;
    switch (status->codec)
    {
        case BAVC_AudioCompressionStd_eMpegL1:
        case BAVC_AudioCompressionStd_eMpegL2:
        case BAVC_AudioCompressionStd_eMpegL3:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.mpeg.channelMode);
            break;
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.aac.channelMode);
            if (count != 0)
            {
                count += (status->codecStatus.aac.lfe?1:0);
            }
            break;
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.ac3.channelMode);
            if (count != 0)
            {
                count += (status->codecStatus.ac3.lfe?1:0);
            }
            break;
        case BAVC_AudioCompressionStd_eAc4:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.ac4.channelMode);
            if (count != 0)
            {
                count += (status->codecStatus.ac4.lfe?1:0);
            }
            break;
        case BAVC_AudioCompressionStd_eDts:
        case BAVC_AudioCompressionStd_eDtshd:
        case BAVC_AudioCompressionStd_eDtsLegacy:
            return status->codecStatus.dts.numChannels;
        case BAVC_AudioCompressionStd_eWmaStd:
        case BAVC_AudioCompressionStd_eWmaStdTs:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.wma.channelMode);
            break;
        case BAVC_AudioCompressionStd_eWmaPro:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.wmaPro.channelMode);
            if (count != 0)
            {
                count += (status->codecStatus.wmaPro.lfe?1:0);
            }
            break;
        case BAVC_AudioCompressionStd_ePcmWav:
            return status->codecStatus.pcmWav.numChannels;
        case BAVC_AudioCompressionStd_eAmrNb:
        case BAVC_AudioCompressionStd_eAmrWb:
            return 1; /* Always Mono */
        case BAVC_AudioCompressionStd_eDra:
            switch (status->codecStatus.dra.acmod)
            {
                case BAPE_DraAcmod_e1_0_C:
                    return (1+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e2_0_LR:
                    return (2+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e2_1_LRS:
                    return (3+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e2_2_LRLrRr:
                    return (4+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e3_2_LRLrRrC:
                    return (5+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e3_3_LRLrRrCrC:
                    return (6+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e5_2_LRLrRrLsRsC:
                    return (7+(status->codecStatus.dra.lfe?1:0));
                case BAPE_DraAcmod_e5_3_LRLrRrLsRsCrC:
                    return (8+(status->codecStatus.dra.lfe?1:0));
                default:
                    return 0;
            }
        case BAVC_AudioCompressionStd_eCook:
            return (status->codecStatus.cook.stereo?2:1);
        case BAVC_AudioCompressionStd_eAls:
        case BAVC_AudioCompressionStd_eAlsLoas:
            count = NEXUS_AudioDebug_BAPE_ChannelModeToInt(status->codecStatus.als.channelMode);
            break;
        default:
            break;
    }
    return count;
}

static uint32_t NEXUS_AudioDebug_InputSampleRate(
    const BAPE_DecoderStatus *status
    )
{
    switch (status->codec)
    {
        case BAVC_AudioCompressionStd_eAacAdts:
        case BAVC_AudioCompressionStd_eAacLoas:
        case BAVC_AudioCompressionStd_eAacPlusAdts:
        case BAVC_AudioCompressionStd_eAacPlusLoas:
            return status->codecStatus.aac.samplingFrequency;
        case BAVC_AudioCompressionStd_eAc3:
        case BAVC_AudioCompressionStd_eAc3Plus:
            return status->codecStatus.ac3.samplingFrequency;
        case BAVC_AudioCompressionStd_eAc4:
            return status->codecStatus.ac4.samplingFrequency;
        case BAVC_AudioCompressionStd_eDts:
        case BAVC_AudioCompressionStd_eDtshd:
        case BAVC_AudioCompressionStd_eDtsLegacy:
            return status->codecStatus.dts.samplingFrequency;
        case BAVC_AudioCompressionStd_ePcmWav:
            return status->codecStatus.pcmWav.samplingFrequency;
        case BAVC_AudioCompressionStd_eAmrNb:
        case BAVC_AudioCompressionStd_eAmrWb:
            return 8000; /* Always 8k */
        case BAVC_AudioCompressionStd_eDra:
            return status->codecStatus.dra.samplingFrequency;
        case BAVC_AudioCompressionStd_eCook:
            return status->codecStatus.cook.samplingFrequency;
        case BAVC_AudioCompressionStd_eAls:
        case BAVC_AudioCompressionStd_eAlsLoas:
            return status->codecStatus.als.samplingFrequency;
        default:
            return 0;
    }

}

static const char * NEXUS_AudioDebug_ChannelModeToString(
        const BAPE_DecoderStatus *status
    )
{
    switch (status->mode)
    {
        case BAPE_ChannelMode_e1_0:
            return "Mono";
        case BAPE_ChannelMode_e1_1:
            return "Dual Mono";
        case BAPE_ChannelMode_e2_0:
            return "Stereo";
        case BAPE_ChannelMode_e3_0:
        case BAPE_ChannelMode_e2_1:
        case BAPE_ChannelMode_e3_1:
        case BAPE_ChannelMode_e2_2:
        case BAPE_ChannelMode_e3_2:
            return "5.1";
        case BAPE_ChannelMode_e3_4:
            return "7.1";
        case BAPE_ChannelMode_eMax:
        default:
            return "Unknown";
    }
}

static void NEXUS_AudioDebug_PrintGraph(void)
{
    unsigned i,j;
    NEXUS_AudioCapabilities audioCapabilities;

    BDBG_LOG(("Audio Filter Graph:"));

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    for (i=0; i < audioCapabilities.numDecoders; i++)
    {
        BAPE_DecoderStatus decoderStatus;
        NEXUS_AudioDecoderHandle handle = g_decoders[i];
        char inputChannels[15]="";
        char inputSampleRate[15]="";

        if (handle && handle->running)
        {
            BAPE_Decoder_GetStatus(handle->channel, &decoderStatus);

            if (NEXUS_AudioDebug_NumChannels(&decoderStatus) != 0)
            {
                BKNI_Snprintf(inputChannels, sizeof(inputChannels), "%dChs", NEXUS_AudioDebug_NumChannels(&decoderStatus));
            }

            if (NEXUS_AudioDebug_InputSampleRate(&decoderStatus) != 0)
            {
                BKNI_Snprintf(inputSampleRate, sizeof(inputSampleRate), "%dHz", NEXUS_AudioDebug_InputSampleRate(&decoderStatus));
            }

            BDBG_LOG((" Decoder%d: (%p) %s %s %s", i, (void *)handle->channel, NEXUS_AudioCodecToString(NEXUS_Audio_P_MagnumToCodec(decoderStatus.codec)), inputSampleRate, inputChannels));
            BDBG_LOG(("%*sOutput Format %dHz %s", 2, "", decoderStatus.sampleRate, NEXUS_AudioDebug_ChannelModeToString(&decoderStatus)));

            for (j=0; j < NEXUS_AudioDecoderConnectorType_eMax; j++)
            {
                NEXUS_AudioDebug_P_ConfigPrint(&handle->connectors[j],1);
            }
        }
    }

    for (i=0; i < audioCapabilities.numPlaybacks; i++)
    {
        NEXUS_AudioPlaybackHandle playbackHandle = NEXUS_AudioPlayback_P_GetPlaybackByIndex(i);
        if (playbackHandle)
        {
            if (NEXUS_AudioPlayback_P_IsRunning(playbackHandle))
            {
                NEXUS_AudioPlaybackStatus playbackStatus;
                NEXUS_AudioPlaybackSettings playbackSettings;
                NEXUS_AudioInputHandle connector;

                NEXUS_AudioPlayback_GetStatus(playbackHandle, &playbackStatus);
                NEXUS_AudioPlayback_GetSettings(playbackHandle, &playbackSettings);

                BDBG_LOG((" Audio Playback%d: (%p)  %dHz %dbps %s %s",
                    i, (void *)playbackHandle, playbackStatus.startSettings.sampleRate!=0?playbackStatus.startSettings.sampleRate:playbackSettings.sampleRate,
                    playbackStatus.startSettings.bitsPerSample,playbackStatus.startSettings.stereo?"Stereo":"Mono",playbackStatus.startSettings.endian==NEXUS_EndianMode_eLittle?"LE":"BE"));
                connector = NEXUS_AudioPlayback_GetConnector(playbackHandle);
                NEXUS_AudioDebug_P_ConfigPrint(connector,1);
            }
        }
    }

    for (i=0; i < audioCapabilities.numInputCaptures; i++)
    {
        NEXUS_AudioInputCaptureHandle inputCaptureHandle = NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(i);
        if (inputCaptureHandle)
        {
            if (NEXUS_AudioInputCapture_P_IsRunning(inputCaptureHandle))
            {
                NEXUS_AudioInputCaptureStatus captureStatus;
                NEXUS_Error errCode;
                NEXUS_AudioInputHandle connector;

                errCode = NEXUS_AudioInputCapture_GetStatus(inputCaptureHandle, &captureStatus);
                if (errCode)
                {
                    BDBG_LOG((" input capture%d: can't get input capture status, errCode=%x", i, errCode));
                    continue;
                }
                BDBG_LOG((" Audio Input Capture%d: (%p) %s %dHz",
                    i, (void *)inputCaptureHandle, NEXUS_AudioCodecToString(captureStatus.codec), captureStatus.sampleRate));
                connector = NEXUS_AudioInputCapture_GetConnector(inputCaptureHandle);
                NEXUS_AudioDebug_P_ConfigPrint(connector,1);
            }
        }
    }
}

#define PRINT_FORMAT_COEFF(base, ptr, val) val>0?(ptr += BKNI_Snprintf(ptr, sizeof(base)-(ptr-base),"%06x ", val)):(ptr += BKNI_Snprintf(ptr, sizeof(base)-(ptr-base),"   *   "))



static void NEXUS_AudioDebug_P_PrintOutputVolume(BAPE_DebugOutputVolume *debugVolume)
{
    char volumePrint[60];
    char * pVolumePrint = volumePrint;
    switch (debugVolume->type)
    {
    case BAPE_DataType_eIec61937:
        BDBG_LOG(("%s Compressed Volume %sMUTED", debugVolume->pName, debugVolume->outputVolume.muted ? "" : "UN"));
        break;
    case BAPE_DataType_eIec61937x4:
        BDBG_LOG(("%s Compressedx4 Volume %sMUTED", debugVolume->pName, debugVolume->outputVolume.muted ? "" : "UN"));
        break;
    case BAPE_DataType_eIec61937x16:
        BDBG_LOG(("%s Compressedx16 Volume %sMUTED", debugVolume->pName, debugVolume->outputVolume.muted ? "" : "UN"));
        break;
    case BAPE_DataType_ePcmMono:
    case BAPE_DataType_ePcmStereo:
        BDBG_LOG(("%s PCM %s Volume %sMUTED", debugVolume->pName, (debugVolume->type == BAPE_DataType_ePcmMono ? "Mono" : "Stereo"), debugVolume->outputVolume.muted ? "" : "UN"));
        BDBG_LOG(("   L      R"));
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeft]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRight]);
        BDBG_LOG(("%s", volumePrint));
        break;
    case BAPE_DataType_ePcm5_1:
        BDBG_LOG(("%s PCM 5.1 Volume %sMUTED", debugVolume->pName, debugVolume->outputVolume.muted ? "" : "UN"));
        BDBG_LOG(("   L      R      Ls     Rs     C     Lfe"));
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeft]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRight]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeftSurround]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRightSurround]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eCenter]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLfe]);
        BDBG_LOG(("%s", volumePrint));
        break;
    case BAPE_DataType_ePcm7_1:
        BDBG_LOG(("%s PCM 7.1 Volume %sMUTED", debugVolume->pName, debugVolume->outputVolume.muted ? "" : "UN"));
        BDBG_LOG(("   L      R      Ls     Rs     C     Lfe    Lr     Rr"));
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeft]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRight]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeftSurround]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRightSurround]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eCenter]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLfe]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eLeftRear]);
        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, debugVolume->outputVolume.volume[BAPE_Channel_eRightRear]);
        BDBG_LOG(("%s", volumePrint));
        break;
    default:
        BDBG_LOG(("%s Unknown Type ", debugVolume->pName));
        break;

    }
    BDBG_LOG((" "));
}

static void NEXUS_AudioDebug_PrintVolume(void)
{
    unsigned i,j;
    bool mixerFound = false;
    BAPE_DebugStatus debugStatus;
    NEXUS_AudioCapabilities audioCapabilities;
    BERR_Code errCode;

    BDBG_LOG(("Audio Volume Matrices (all values in hex, * means 0):"));
    BDBG_LOG((" "));

    NEXUS_GetAudioCapabilities(&audioCapabilities);

    #if 0 /* TBD add support for printing the mixer inputs */
    for (i=0; i < audioCapabilities.numMixers; i++) {
        NEXUS_AudioMixerHandle mixerHandle = NEXUS_AudioMixer_P_GetMixerByIndex(i);
        NEXUS_AudioMixerSettings mixerSettings;
        if (mixerHandle)
        {
            mixerFound = true;
        }
    }
    #endif

    if (!mixerFound) {
        for (i=0; i < audioCapabilities.numDecoders; i++)
        {
            NEXUS_AudioDecoderHandle handle = g_decoders[i];
            if (handle && handle->running)
            {
                #if NEXUS_HAS_SYNC_CHANNEL
                BDBG_LOG(("Decoder%d: Mute:%s Trick Mute:%s Sync Mute:%s", i,
                          handle->settings.muted ? "Y" : "N",
                          handle->trickMute ? "Y" : "N",
                          handle->sync.mute ? "Y" : "N"));
                #else
                BDBG_LOG(("Decoder%d: Mute:%s Trick Mute:%s", i,
                          handle->settings.muted ? "Y" : "N",
                          handle->trickMute ? "Y" : "N"));
                #endif

                if(handle->outputLists[NEXUS_AudioConnectorType_eMono].outputs[0] ||
                   handle->outputLists[NEXUS_AudioConnectorType_eStereo].outputs[0] ||
                   handle->outputLists[NEXUS_AudioConnectorType_eMultichannel].outputs[0])
                {
                    BDBG_LOG(("       L      R      Ls     Rs     C     Lfe    Lr     Rr"));
                    for (j = 0; j < NEXUS_AudioChannel_eMax; j++) {
                        char volumePrint[70];
                        char * pVolumePrint = volumePrint;
                        switch (j) {
                        case NEXUS_AudioChannel_eLeft:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "L   ");
                            break;
                        case NEXUS_AudioChannel_eRight:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "R   ");
                            break;
                        case NEXUS_AudioChannel_eCenter:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "C   ");
                            break;
                        case NEXUS_AudioChannel_eLfe:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Lfe ");
                            break;
                        case NEXUS_AudioChannel_eLeftSurround:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Ls  ");
                            break;
                        case NEXUS_AudioChannel_eRightSurround:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Rs  ");
                            break;
                        case NEXUS_AudioChannel_eLeftRear:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Lr  ");
                            break;
                        case NEXUS_AudioChannel_eRightRear:
                            pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Rr  ");
                            break;
                        }
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eLeft]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eRight]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eLeftSurround]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eRightSurround]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eCenter]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eLfe]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eLeftRear]);
                        PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, handle->settings.volumeMatrix[j][BAPE_Channel_eRightRear]);
                        BDBG_LOG(("%s", volumePrint));
                    }
                }
                else
                {
                    BDBG_LOG(("Compressed Decoder"));
                }
                BDBG_LOG((" "));
            }
        }

        for (i=0; i < audioCapabilities.numPlaybacks; i++)
        {
            NEXUS_AudioPlaybackHandle playbackHandle = NEXUS_AudioPlayback_P_GetPlaybackByIndex(i);
            if (playbackHandle)
            {
                if (NEXUS_AudioPlayback_P_IsRunning(playbackHandle))
                {
                    NEXUS_AudioPlaybackSettings playbackSettings;
                    char volumePrint[20];
                    char * pVolumePrint = volumePrint;

                    NEXUS_AudioPlayback_GetSettings(playbackHandle, &playbackSettings);
                    BDBG_LOG(("Audio Playback%d: Mute:%s Content Reference -%u", i, playbackSettings.muted ? "Y":"N", playbackSettings.contentReferenceLevel));
                    BDBG_LOG(("   L      R"));
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, playbackSettings.leftVolume);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, playbackSettings.rightVolume);
                    BDBG_LOG(("%s", volumePrint));
                    BDBG_LOG((" "));

                }
            }
        }

        for (i=0; i < audioCapabilities.numInputCaptures; i++)
        {
            NEXUS_AudioInputCaptureHandle inputCaptureHandle = NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(i);
            if (inputCaptureHandle)
            {
                NEXUS_AudioInputCaptureSettings captureSettings;
                NEXUS_AudioInputCapture_GetSettings(inputCaptureHandle, &captureSettings);
                BDBG_LOG(("Audio Input Capture%d: Mute:%s", i, captureSettings.muted ? "Y":"N"));
                BDBG_LOG(("       L      R      Ls     Rs     C     Lfe    Lr     Rr"));
                for (j = 0; j < NEXUS_AudioChannel_eMax; j++) {
                    char volumePrint[70];
                    char * pVolumePrint = volumePrint;
                    switch (j) {
                    case NEXUS_AudioChannel_eLeft:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "L   ");
                        break;
                    case NEXUS_AudioChannel_eRight:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "R   ");
                        break;
                    case NEXUS_AudioChannel_eCenter:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "C   ");
                        break;
                    case NEXUS_AudioChannel_eLfe:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Lfe ");
                        break;
                    case NEXUS_AudioChannel_eLeftSurround:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Ls  ");
                        break;
                    case NEXUS_AudioChannel_eRightSurround:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Rs  ");
                        break;
                    case NEXUS_AudioChannel_eLeftRear:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Lr  ");
                        break;
                    case NEXUS_AudioChannel_eRightRear:
                        pVolumePrint += BKNI_Snprintf(pVolumePrint, sizeof(volumePrint)-(pVolumePrint-volumePrint), "Rr  ");
                        break;
                    }
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eLeft]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eRight]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eLeftSurround]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eRightSurround]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eCenter]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eLfe]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eLeftRear]);
                    PRINT_FORMAT_COEFF(volumePrint, pVolumePrint, captureSettings.volumeMatrix[j][BAPE_Channel_eRightRear]);
                    BDBG_LOG(("%s", volumePrint));
                }
                BDBG_LOG((" "));
            }
        }
    }

    /* Outputs - HDMI, Spdif, DAC, I2S */

    errCode = BAPE_Debug_GetStatus( g_NEXUS_audioModuleData.debugHandle,
                                    BAPE_DebugSourceType_eVolume,
                                    &debugStatus);

    if ( errCode == BERR_SUCCESS)
    {
        #if BAPE_CHIP_MAX_SPDIF_OUTPUTS > 0
        for (i=0; i < audioCapabilities.numOutputs.spdif; i++)
        {
            if (debugStatus.status.volumeStatus.spdif[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintOutputVolume(&debugStatus.status.volumeStatus.spdif[i]);
            }
        }
        #endif

        #if BAPE_CHIP_MAX_MAI_OUTPUTS > 0
        for (i=0; i < audioCapabilities.numOutputs.hdmi; i++)
        {
            if (debugStatus.status.volumeStatus.hdmi[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintOutputVolume(&debugStatus.status.volumeStatus.hdmi[i]);
            }
        }
        #endif
        #if BAPE_CHIP_MAX_DACS > 0
        for (i=0; i < audioCapabilities.numOutputs.dac; i++)
        {
            if (debugStatus.status.volumeStatus.dac[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintOutputVolume(&debugStatus.status.volumeStatus.dac[i]);
            }
        }
        #endif
        #if BAPE_CHIP_MAX_I2S_OUTPUTS > 0
        for (i=0; i < audioCapabilities.numOutputs.i2s; i++)
        {
            if (debugStatus.status.volumeStatus.i2s[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintOutputVolume(&debugStatus.status.volumeStatus.i2s[i]);
            }
        }
        #endif
    }

}

BERR_Code NEXUS_AudioDebug_Init(void)
{
    BERR_Code errCode;

    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "audio_output", "nexus_audio_debug", NEXUS_AudioDebug_OutputPrint);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "audio_graph", "nexus_audio_debug", NEXUS_AudioDebug_PrintGraph);
    NEXUS_Module_RegisterProc(NEXUS_MODULE_SELF, "audio_volume", "nexus_audio_debug", NEXUS_AudioDebug_PrintVolume);

    errCode = BAPE_Debug_Open(
        NEXUS_AUDIO_DEVICE_HANDLE,
        NULL,
        &g_NEXUS_audioModuleData.debugHandle);
    if ( errCode )
    {
        NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_output");
        NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_graph");
        NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_volume");
        (void)BERR_TRACE(errCode);
    }

    return errCode;
}

void NEXUS_AudioDebug_Uninit(void)
{
    BAPE_Debug_Close(g_NEXUS_audioModuleData.debugHandle);
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_output");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_graph");
    NEXUS_Module_UnregisterProc(NEXUS_MODULE_SELF, "audio_volume");
}

void NEXUS_AudioDebug_AddOutput(void *outputPort )
{
        BAPE_Debug_AddOutput(g_NEXUS_audioModuleData.debugHandle, (BAPE_OutputPort)outputPort);
}


void NEXUS_AudioDebug_RemoveOutput(void *outputPort)
{
    BAPE_Debug_RemoveOutput( g_NEXUS_audioModuleData.debugHandle, (BAPE_OutputPort)outputPort);
}

#else

BERR_Code NEXUS_AudioDebug_Init(void)
{
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
    return BERR_NOT_SUPPORTED;
}

void NEXUS_AudioDebug_Uninit(void)
{
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}

void NEXUS_AudioDebug_AddOutput(void *outputPort )
{
    BSTD_UNUSED(outputPort);
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}


void NEXUS_AudioDebug_RemoveOutput(void *outputPort)
{
    BSTD_UNUSED(outputPort);
    (void) BERR_TRACE(BERR_NOT_SUPPORTED);
}

#endif
