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
 *****************************************************************************/
#include "nexus_audio_module.h"
#include "nexus_audio_debug.h"

BDBG_MODULE(nexus_audio_debug);

NEXUS_ModuleHandle g_NEXUS_audioDebug;
NEXUS_ModuleHandle g_NEXUS_audioGraphDebug;

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
    BDBG_LOG(("     audio/non-audio: %sAUDIO (%db)",
        status->audio ? "NON-":"DIGITAL ", status->audio));
    BDBG_LOG(("           copyright: COPYRIGHT %sASSERTED (%db)",
        status->copyright ? "":"NOT ", status->copyright));

    if (status->audio == 0) /* Digital Audio */
    {
        switch(status->emphasis)
        {
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
    else  /* NON AUDIO */
    {
        switch(status->emphasis)
        {
            case 0:
                ptrTemp = "DIGITAL DATA";
                break;
            default:
                ptrTemp = "Reserved (Digital Data)";
                break;
        }
    }
    BDBG_LOG(("            emphasis: Pre-emphasis: %s (%sb)",ptrTemp, BAPE_Debug_IntToBinaryString(status->emphasis,3,binaryArray)));
    BDBG_LOG(("                mode: MODE %s (%db)", status->mode ? "RESERVED":"0", status->mode));

    switch(status->categoryCode)
    {
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
    BDBG_LOG(("        categoryCode: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->categoryCode,8,binaryArray)));

    switch(status->samplingFrequency)
    {
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
    BDBG_LOG(("        samplingFreq: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->samplingFrequency,4,binaryArray)));

    if (status->audio == 0)
    {
        BDBG_LOG(("       pcmWordLength: MAX WORD LENGTH %s bits (%db)", status->pcmWordLength ? "24":"20", status->pcmWordLength));;

        switch(status->pcmSampleWordLength)
        {
            case 0:
                ptrTemp = "SAMPLE WORD LENGTH NOT INDICATED";
                break;
            case 1:
                ptrTemp = status->pcmWordLength ? "20 bits":"16 bits";
                break;
            case 2:
                ptrTemp = status->pcmWordLength ? "22 bits":"18 bits";
                break;
            case 4:
                ptrTemp = status->pcmWordLength ? "23 bits":"19 bits";
                break;
            case 5:
                ptrTemp = status->pcmWordLength ? "24 bits":"20 bits";
                break;
            case 6:
                ptrTemp = status->pcmWordLength ? "21 bits":"17 bits";
                break;
            default:
                ptrTemp = "RESERVED";
                break;
        }
        BDBG_LOG((" pcmSampleWordLength: %s (%sb)", ptrTemp, BAPE_Debug_IntToBinaryString(status->pcmSampleWordLength,3,binaryArray)));

        switch(status->pcmOrigSamplingFrequency)
        {
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
            BAPE_Debug_IntToBinaryString(status->pcmOrigSamplingFrequency,4,binaryArray)));

        switch(status->pcmCgmsA)
        {
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
            BAPE_Debug_IntToBinaryString(status->pcmCgmsA,2,binaryArray)));
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
    int i;

    if (
        BAPE_Debug_GetStatus(
        g_NEXUS_audioModuleData.debugHandle,
        BAPE_DebugSourceType_eOutput,
        &status) == BERR_SUCCESS)
    {
#if NEXUS_NUM_SPDIF_OUTPUTS
        for (i=0; i < NEXUS_NUM_SPDIF_OUTPUTS; i++)
        {
            if (status.status.outputStatus.spdif[i].enabled)
            {
                NEXUS_AudioDebug_P_PrintDigitalOutputStatus(&status.status.outputStatus.spdif[i]);
            }
        }
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
        for (i=0; i < NEXUS_NUM_HDMI_OUTPUTS; i++)
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

void NEXUS_AudioDebug_P_ConfigPrint(NEXUS_AudioInput input, int level)
{
    NEXUS_AudioInput downstreamObject;
    NEXUS_AudioOutputHandle audioOutput;
    downstreamObject = NEXUS_AudioInput_P_LocateDownstream(input);
    if ( downstreamObject )
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

uint32_t NEXUS_AudioDebug_BAPE_ChannelModeToInt(
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

uint32_t NEXUS_AudioDebug_NumChannels(
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
        default:
            break;
    }
    return count;
}

uint32_t NEXUS_AudioDebug_InputSampleRate(
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
        case BAVC_AudioCompressionStd_eDts:
        case BAVC_AudioCompressionStd_eDtshd:
        case BAVC_AudioCompressionStd_eDtsLegacy:
            return status->codecStatus.dts.samplingFrequency;
        case BAVC_AudioCompressionStd_ePcmWav:
            return status->codecStatus.pcmWav.samplingFrequency;
        case BAVC_AudioCompressionStd_eAmrNb:
        case BAVC_AudioCompressionStd_eAmrWb:
            return 80000; /* Always 8k */
        case BAVC_AudioCompressionStd_eDra:
            return status->codecStatus.dra.samplingFrequency;
        case BAVC_AudioCompressionStd_eCook:
            return status->codecStatus.cook.samplingFrequency;
        default:
            return 0;
    }
}

const char * NEXUS_AudioDebug_ChannelModeToString(
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
    int i,j;
    BDBG_LOG(("Audio Filter Graph:"));

#if NEXUS_NUM_AUDIO_DECODERS
    for (i=0; i < NEXUS_NUM_AUDIO_DECODERS; i++)
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
#endif

#if NEXUS_NUM_AUDIO_PLAYBACKS
    for (i=0; i < NEXUS_NUM_AUDIO_PLAYBACKS; i++)
    {
        NEXUS_AudioPlaybackHandle playbackHandle = NEXUS_AudioPlayback_P_GetPlaybackByIndex(i);
        if (playbackHandle)
        {
            if (NEXUS_AudioPlayback_P_IsRunning(playbackHandle))
            {
                NEXUS_AudioPlaybackStatus playbackStatus;
                NEXUS_AudioPlaybackSettings playbackSettings;
                NEXUS_AudioInput connector;

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
#endif

#if NEXUS_NUM_AUDIO_INPUT_CAPTURES
    for (i=0; i < NEXUS_NUM_AUDIO_INPUT_CAPTURES; i++)
    {
        NEXUS_AudioInputCaptureHandle inputCaptureHandle = NEXUS_AudioInputCapture_P_GetInputCaptureByIndex(i);
        if (inputCaptureHandle)
        {
            if (NEXUS_AudioInputCapture_P_IsRunning(inputCaptureHandle))
            {
                NEXUS_AudioInputCaptureStatus captureStatus;
                NEXUS_Error errCode;
                NEXUS_AudioInput connector;

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
#endif
}

BERR_Code NEXUS_AudioDebug_Init(void)
{
    BERR_Code errCode;
    NEXUS_ModuleSettings moduleSettings;

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    moduleSettings.dbgPrint = NEXUS_AudioDebug_OutputPrint;
    moduleSettings.dbgModules = "nexus_audio_debug";
    g_NEXUS_audioDebug = NEXUS_Module_Create("audio_output", &moduleSettings);

    if ( NULL == g_NEXUS_audioDebug )
    {
        errCode=BERR_TRACE(BERR_OS_ERROR);
        return errCode;
    }

    NEXUS_Module_GetDefaultSettings(&moduleSettings);
    moduleSettings.priority = NEXUS_ModulePriority_eLow; /* decoder interface is slow */
    moduleSettings.dbgPrint = NEXUS_AudioDebug_PrintGraph;
    moduleSettings.dbgModules = "nexus_audio_debug";
    g_NEXUS_audioGraphDebug = NEXUS_Module_Create("audio_graph", &moduleSettings);

    if ( NULL == g_NEXUS_audioGraphDebug )
    {
        NEXUS_Module_Destroy(g_NEXUS_audioDebug);
        errCode=BERR_TRACE(BERR_OS_ERROR);
        return errCode;
    }


    errCode = BAPE_Debug_Open(
        NEXUS_AUDIO_DEVICE_HANDLE,
        NULL,
        &g_NEXUS_audioModuleData.debugHandle);
    if ( errCode )
    {
        NEXUS_Module_Destroy(g_NEXUS_audioDebug);
        NEXUS_Module_Destroy(g_NEXUS_audioGraphDebug);
        (void)BERR_TRACE(errCode);
    }

    return errCode;
}

void NEXUS_AudioDebug_Uninit(void)
{
    BAPE_Debug_Close(g_NEXUS_audioModuleData.debugHandle);
    NEXUS_Module_Destroy(g_NEXUS_audioDebug);
    NEXUS_Module_Destroy(g_NEXUS_audioGraphDebug);
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
