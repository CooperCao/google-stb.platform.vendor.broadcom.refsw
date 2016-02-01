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
******************************************************************************/
#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_AUDIO && (NEXUS_AUDIO_MODULE_FAMILY == NEXUS_AUDIO_MODULE_FAMILY_APE_RAAGA)
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_decoder.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_encoder.h"
#include "nexus_audio_mixer.h"
#include "nexus_audio_output.h"
#include "nexus_spdif_output.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#include "nexus_hdmi_output_hdcp.h"
#endif
#include "nexus_audio_input.h"
#include "nexus_dolby_digital_reencode.h"
#include "nexus_dolby_volume.h"
#include "bstd.h"
#include "bkni.h"
#if NEXUS_HAS_PLAYBACK
#include "nexus_playback.h"
#include "nexus_file.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

BDBG_MODULE(ms12_dual);

/* Change this to true to enable SPDIF re-encoding to AC3 */
static bool g_compressed = false;
char * fname = NULL;

#if NEXUS_NUM_HDMI_OUTPUTS
static bool hdmiHdcpEnabled = false;
static NEXUS_PlatformConfiguration platformConfig;

/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
/* HDMI Support */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/
#define USE_PRODUCTION_KEYS 0

#if USE_PRODUCTION_KEYS

/*****************************/
/* INSERT PRODUCTION KeySet HERE */
/*****************************/

#else


/**************************************/
/* HDCP Specification Test Key Set    */
/*                                    */
/* NOTE: the default declared Test    */
/* KeySet below is from the HDCP Spec */
/* and it *IS NOT* compatible with    */
/* production devices                 */
/**************************************/


static NEXUS_HdmiOutputHdcpKsv hdcpTxAksv =
{    {0x14, 0xF7, 0x61, 0x03, 0xB7} };

static NEXUS_HdmiOutputHdcpKey encryptedTxKeySet[NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS] =
{
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x691e138f, 0x58a44d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0950e658, 0x35821f00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x0d98b9ab, 0x476a8a00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcac5cb52, 0x1b18f300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb4d89668, 0x7f14fb00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x818f4878, 0xc98be000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412c11c8, 0x64d0a000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x44202428, 0x5a9db300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b56adbd, 0xb228b900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf6e46c4a, 0x7ba49100},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x589d5e20, 0xf8005600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xa03fee06, 0xb77f8c00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x28bc7c9d, 0x8c2dc000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x059f4be5, 0x61125600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xcbc1ca8c, 0xdef07400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6adbfc0e, 0xf6b83b00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xd72fb216, 0xbb2ba000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x98547846, 0x8e2f4800},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x38472762, 0x25ae6600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf2dd23a3, 0x52493d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x543a7b76, 0x31d2e200},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x2561e6ed, 0x1a584d00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf7227bbf, 0x82603200},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6bce3035, 0x461bf600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x6b97d7f0, 0x09043600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xf9498d61, 0x05e1a100},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x063405d1, 0x9d8ec900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x90614294, 0x67c32000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xc34facce, 0x51449600},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x8a8ce104, 0x45903e00},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xfc2d9c57, 0x10002900},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x80b1e569, 0x3b94d700},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x437bdd5b, 0xeac75400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xba90c787, 0x58fb7400},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xe01d4e36, 0xfa5c9300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xae119a15, 0x5e070300},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x01fb788a, 0x40d30500},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0xb34da0d7, 0xa5590000},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x409e2c4a, 0x633b3700},
    { 0, 0, 0, 0, 0, 0, 0, 0, 0x412056b4, 0xbb732500}
};

 #endif

/*
from HDCP Spec:
Table 51 gives the format of the HDCP SRM. All values are stored in big endian format.

Specify KSVs here in big endian;
*/
#define NUM_REVOKED_KSVS 3
static uint8_t NumRevokedKsvs = NUM_REVOKED_KSVS ;
static const NEXUS_HdmiOutputHdcpKsv RevokedKsvs[NUM_REVOKED_KSVS] =
{
    /* MSB ... LSB */
    {{0xa5, 0x1f, 0xb0, 0xc3, 0x72}},
    {{0x65, 0xbf, 0x04, 0x8a, 0x7c}},
    {{0x65, 0x65, 0x1e, 0xd5, 0x64}}
};

static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplayHandle display = (NEXUS_DisplayHandle)iParam;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings    ;

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    NEXUS_Display_GetSettings(display, &displaySettings);
    if ( !status.videoFormatSupported[displaySettings.format] )
    {
        BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d",
            status.preferredVideoFormat)) ;
        displaySettings.format = status.preferredVideoFormat;
    }
    NEXUS_Display_SetSettings(display, &displaySettings);

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings) ;
    NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings) ;

    /* restart HDCP if it was previously enabled */
    if (hdmiHdcpEnabled)
    {
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}

static void hdmiOutputHdcpStateChanged(void *pContext, int param)
{
    bool success = false ;
    NEXUS_HdmiOutputHandle handle = pContext;
    NEXUS_HdmiOutputHdcpStatus hdcpStatus;

    BSTD_UNUSED(param) ;

    NEXUS_HdmiOutput_GetHdcpStatus(handle, &hdcpStatus);
    switch (hdcpStatus.hdcpError)
    {
    case NEXUS_HdmiOutputHdcpError_eSuccess :
        BDBG_WRN(("HDCP Authentication Successful\n"));
        success = true ;
        hdmiHdcpEnabled = true ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvError :
        BDBG_ERR(("HDCP Rx BKsv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvRevoked :
        BDBG_ERR(("HDCP Rx BKsv/Keyset Revoked")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxBksvI2cReadError :
    case NEXUS_HdmiOutputHdcpError_eTxAksvI2cWriteError :
        BDBG_ERR(("HDCP I2C Read Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eTxAksvError :
        BDBG_ERR(("HDCP Tx Aksv Error")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eReceiverAuthenticationError :
        BDBG_ERR(("HDCP Receiver Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterAuthenticationError :
    case NEXUS_HdmiOutputHdcpError_eRepeaterLinkFailure :    /* Repeater Error; unused */
        BDBG_ERR(("HDCP Repeater Authentication Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRxDevicesExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Devices Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDepthExceeded :
        BDBG_ERR(("HDCP Repeater MAX Downstram Levels Exceeded")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterFifoNotReady :
        BDBG_ERR(("Timeout waiting for Repeater")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eRepeaterDeviceCount0 : /* unused */
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkRiFailure :
        BDBG_ERR(("HDCP Ri Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eLinkPjFailure :
        BDBG_ERR(("HDCP Pj Integrity Check Failure")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eFifoUnderflow :
    case NEXUS_HdmiOutputHdcpError_eFifoOverflow :
        BDBG_ERR(("Video configuration issue")) ;
        break ;

    case NEXUS_HdmiOutputHdcpError_eMultipleAnRequest : /* Should not reach here; but flag if occurs */
        BDBG_WRN(("Multiple Authentication Request... ")) ;

    default :
        BDBG_WRN(("Unknown HDCP Authentication Error %d", hdcpStatus.hdcpError)) ;
    }

    if (!success)
    {
        fprintf(stderr, "\nHDCP Authentication Failed.  Current State %d\n", hdcpStatus.hdcpState);

        /* always retry */
        NEXUS_HdmiOutput_StartHdcpAuthentication(platformConfig.outputs.hdmi[0]);
    }
}

static void initializeHdmiOutputHdcpSettings(void)
{
    NEXUS_HdmiOutputHdcpSettings hdmiOutputHdcpSettings;

    NEXUS_HdmiOutput_GetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

        /* copy the encrypted key set and its Aksv here  */
        BKNI_Memcpy(hdmiOutputHdcpSettings.encryptedKeySet, encryptedTxKeySet,
            NEXUS_HDMI_OUTPUT_HDCP_NUM_KEYS * sizeof(NEXUS_HdmiOutputHdcpKey)) ;
        BKNI_Memcpy(&hdmiOutputHdcpSettings.aksv, &hdcpTxAksv,
            NEXUS_HDMI_OUTPUT_HDCP_KSV_LENGTH) ;

        /* install HDCP success  callback */
        hdmiOutputHdcpSettings.successCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.successCallback.context = platformConfig.outputs.hdmi[0];

        /* install HDCP failure callback */
        hdmiOutputHdcpSettings.failureCallback.callback = hdmiOutputHdcpStateChanged ;
        hdmiOutputHdcpSettings.failureCallback.context = platformConfig.outputs.hdmi[0];

    NEXUS_HdmiOutput_SetHdcpSettings(platformConfig.outputs.hdmi[0], &hdmiOutputHdcpSettings);

    /* install list of revoked KSVs from SRMs (System Renewability Message) if available */
    NEXUS_HdmiOutput_SetHdcpRevokedKsvs(platformConfig.outputs.hdmi[0],
        RevokedKsvs, NumRevokedKsvs) ;

}
#endif
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------*/


int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_InputBand inputBand;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    #if NEXUS_HAS_PLAYBACK
    NEXUS_FilePlayHandle file = NULL;
    NEXUS_PlaypumpHandle playpump;
    NEXUS_PlaybackHandle playback;
    NEXUS_PlaybackSettings playbackSettings;
    NEXUS_PlaybackPidChannelSettings playbackPidSettings;
    #endif
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_PidChannelHandle videoPidChannel=NULL;
    NEXUS_PidChannelHandle mainPidChannel=NULL;
    NEXUS_PidChannelHandle secondaryPidChannel=NULL;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderHandle mainDecoder, secondaryDecoder;
    NEXUS_AudioDecoderStartSettings mainProgram, secondaryProgram;
    NEXUS_AudioDecoderCodecSettings primaryCodecSettings;
    NEXUS_AudioDecoderOpenSettings audioOpenSettings;
    NEXUS_AudioMixerSettings mixerSettings;
    NEXUS_AudioMixerHandle mixer;
    NEXUS_DolbyDigitalReencodeHandle ddre;
    NEXUS_DolbyDigitalReencodeSettings ddreSettings;
    NEXUS_AudioCodec codec = NEXUS_AudioCodec_eAc3Plus;
    NEXUS_AudioCodec secondaryCodec = NEXUS_AudioCodec_eAc3Plus;
    NEXUS_AudioCodec encodeCodec = NEXUS_AudioCodec_eAc3Plus;
    NEXUS_VideoCodec videoCodec = NEXUS_VideoCodec_eH264;
    unsigned audioPid=48, secondaryPid=49, videoPid=50;
    bool multich71 = false;
    bool enableDV = false;
    bool enableDE = false;
    unsigned dialogBoost = 0;
    unsigned contentCut = 0;
    bool certMode = false;
    bool startDecoders = true;
    bool enableHdmi = true;
    bool enableDac = true;
    bool enableSpdif = true;
    int msBalance = 0;
    int i;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_HdmiOutputSettings hdmiSettings;
#endif

    /* Bring up all modules for a platform in a default configuration for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    /* Parse command-line arguments */
    for ( i = 1; i < argc; i++ )
    {
        if ( !strcmp(argv[i], "-7.1") )
        {
            multich71 = true;
        }
        else if ( !strcmp(argv[i], "-compressed_audio") )
        {
            printf("Compressed Audio Requested\n");
            g_compressed = true;
        }
        else if ( !strcmp(argv[i], "-file") && (i+1) < argc )
        {
            i++;
            #if NEXUS_HAS_PLAYBACK
            fname = argv[i];
            #endif
        }
        else if ( !strcmp(argv[i], "-audio_type") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "ac3") || !strcmp(argv[i], "ac3plus") )
            {
                printf("DDP Decoder\n");
                codec = NEXUS_AudioCodec_eAc3Plus;
            }
            else if ( !strcmp(argv[i], "aac") || !strcmp(argv[i], "aac_adts") )
            {
                printf("AACHE ADTS Decoder\n");
                codec = NEXUS_AudioCodec_eAacPlusAdts;
            }
            else if ( !strcmp(argv[i], "aacplus") || !strcmp(argv[i], "aacplus_loas") )
            {
                printf("AACHE LOAS Decoder\n");
                codec = NEXUS_AudioCodec_eAacPlusLoas;
            }
            else
            {
                BDBG_ERR(("Codec %s is not supported by this application", argv[i]));
                return -1;
            }
        }
        else if ( !strcmp(argv[i], "-secondary_audio_type") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "ac3") || !strcmp(argv[i], "ac3plus") )
            {
                printf("DDP Secondary Decoder\n");
                secondaryCodec = NEXUS_AudioCodec_eAc3Plus;
            }
            else if ( !strcmp(argv[i], "aac") || !strcmp(argv[i], "aac_adts") )
            {
                printf("AACHE ADTS Secondary Decoder\n");
                secondaryCodec = NEXUS_AudioCodec_eAacPlusAdts;
            }
            else if ( !strcmp(argv[i], "aacplus") || !strcmp(argv[i], "aacplus_loas") )
            {
                printf("AACHE LOAS Secondary Decoder\n");
                secondaryCodec = NEXUS_AudioCodec_eAacPlusLoas;
            }
            else
            {
                BDBG_ERR(("Secondary Codec %s is not supported by this application", argv[i]));
                return -1;
            }
        }
        else if ( !strcmp(argv[i], "-video_type") && (i+1) < argc )
        {
            videoCodec = atoi(argv[++i]);
            printf("Video Codec %d\n", videoCodec);
        }
        else if ( !strcmp(argv[i], "-enc_type") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "ac3") )
            {
                printf("Encode AC3 Only\n");
                encodeCodec = NEXUS_AudioCodec_eAc3;
            }
        }
        else if ( !strcmp(argv[i], "-audio") && (i+1) < argc )
        {
            i++;
            audioPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-video") && (i+1) < argc )
        {
            i++;
            videoPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-secondary_audio") && (i+1) < argc )
        {
            i++;
            secondaryPid = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-no_secondary_audio") )
        {
            secondaryPid = 0;
        }
        else if ( !strcmp(argv[i], "-de") && (i+2) < argc )
        {
            i++;
            enableDE = true;
            dialogBoost = (unsigned)strtoul(argv[i++], NULL, 0);
            contentCut = (unsigned)strtoul(argv[i], NULL, 0);
        }
        else if ( !strcmp(argv[i], "-dv") )
        {
            i++;
            enableDV = true;
        }
        else if ( !strcmp(argv[i], "-no_decoded_audio") )
        {
            startDecoders = true;
        }
        else if ( !strcmp(argv[i], "-disable_dac") )
        {
            enableDac = false;
        }
        else if ( !strcmp(argv[i], "-disable_hdmi") )
        {
            enableHdmi = false;
        }
        else if ( !strcmp(argv[i], "-disable_spdif") )
        {
            enableSpdif = false;
        }
        else if ( !strcmp(argv[i], "-loudness") && (i+1) < argc )
        {
            i++;
            if ( !strcmp(argv[i], "atsc") )
            {
                platformSettings.audioModuleSettings.loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eAtscA85;
            }
            else if ( !strcmp(argv[i], "ebu") )
            {
                platformSettings.audioModuleSettings.loudnessMode = NEXUS_AudioLoudnessEquivalenceMode_eEbuR128;
            }
        }
        else if ( !strcmp(argv[i], "-ms_bal") && (i+1) < argc )
        {
            msBalance = atoi(argv[++i]);
            printf("Multi Stream Balance %d\n", msBalance);
        }
        else
        {
            BDBG_WRN(("usage: %s [-file <file>] [-7.1] [-compressed_audio] [-audio_type <ac3|ac3plus|aac|aac_adts|aacplus|aacplus_loas>] [-secondary_audio_type <ac3|ac3plus|aac|aac_adts|aacplus|aacplus_loas>] [-video_type <int>] [-enc_type <ac3|ac3plus>] [-audio <pid>] [-secondary_audio <pid>] [-video] [-no_secondary_audio] [-dv] [-de <dialogboost> <contentcut>] [-disable_dac] [-disable_spdif] [-disable_hdmi]", argv[0]));
            return -1;
        }
    }

    platformSettings.openFrontend = false;
    if (multich71)
    {
        platformSettings.audioModuleSettings.numPcmBuffers = 5;
    }
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    if (fname != NULL)
    {
        #if NEXUS_HAS_PLAYBACK
        playpump = NEXUS_Playpump_Open(0, NULL);
        assert(playpump);
        playback = NEXUS_Playback_Create();
        assert(playback);
        #endif
    }

    /* Bring up the primary audio decoder */
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioOpenSettings);
    if (codec == NEXUS_AudioCodec_eAc3Plus && multich71)
    {
        audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e7_1;
    }
    else
    {
        audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;
    }
    audioOpenSettings.type = NEXUS_AudioDecoderType_eDecode;
    mainDecoder = NEXUS_AudioDecoder_Open(0, &audioOpenSettings);

    /* Open audio descriptor decoder */
    /*audioOpenSettings.multichannelFormat = NEXUS_AudioMultichannelFormat_e5_1;*/
    audioOpenSettings.type = NEXUS_AudioDecoderType_eAudioDescriptor;
    secondaryDecoder = NEXUS_AudioDecoder_Open(1, &audioOpenSettings);

    /* Open mixer to mix the description and primary audio */
    NEXUS_AudioMixer_GetDefaultSettings(&mixerSettings);
    mixerSettings.mixUsingDsp = true;
    /* Configure mixer post processing */
    if (enableDV || enableDE)
    {
        mixerSettings.dolby.enablePostProcessing = true;
        mixerSettings.dolby.volumeLimiter.enableVolumeLimiting = enableDV;
        mixerSettings.dolby.dialogEnhancer.enableDialogEnhancer = enableDE;
        mixerSettings.dolby.dialogEnhancer.dialogEnhancerLevel = dialogBoost;
        mixerSettings.dolby.dialogEnhancer.contentSuppressionLevel = contentCut;
    }
    mixerSettings.dolby.certificationMode = certMode;
    mixerSettings.multiStreamBalance = msBalance;
    printf("\nMS12 DAP Mixer settings:\n");
    printf("-------------------------\n");
    printf("\tcertificationMode %d\n", certMode);
    printf("\tenableDV          %d\n", enableDV);
    printf("\tenableDE          %d\n", enableDE);
    printf("\tdialogBoost       %d\n", dialogBoost);
    printf("\tcontentCut        %d\n", contentCut);
    printf("-------------------------\n");
    printf("\tmsBalance         %d\n", msBalance);
    printf("-------------------------\n");
    mixer = NEXUS_AudioMixer_Open(&mixerSettings);

    NEXUS_DolbyDigitalReencode_GetDefaultSettings(&ddreSettings);
    ddreSettings.certificationMode = certMode;
    printf("\nDDRE settings:\n");
    printf("-------------------------\n");
    printf("\tcertificationMode %d\n", certMode);
    printf("-------------------------\n");
    ddre = NEXUS_DolbyDigitalReencode_Open(&ddreSettings);

    /* Make our connections */
    NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioConnectorType_eMultichannel));
    if ( secondaryPid > 0 )
    {
        NEXUS_AudioMixer_AddInput(mixer, NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioConnectorType_eMultichannel));
    }

    /* Set the Mixer to use DSP mixing */
    NEXUS_AudioMixer_GetSettings(mixer, &mixerSettings);
    mixerSettings.master = NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioConnectorType_eMultichannel);
    NEXUS_AudioMixer_SetSettings(mixer, &mixerSettings);

    NEXUS_DolbyDigitalReencode_AddInput(ddre, NEXUS_AudioMixer_GetConnector(mixer));


    printf("\nMS12 output connections:\n");
    printf("-------------------------\n");
    #if NEXUS_NUM_AUDIO_DACS
    if (enableDac)
    {
        printf("\tDAC Stereo PCM\n");
        NEXUS_AudioOutput_AddInput(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                                   NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eStereo));
    }
    #endif

    #if NEXUS_NUM_SPDIF_OUTPUTS
    /* Dolby AACHE codec does not support downmixing */
    if (enableSpdif)
    {
        if (g_compressed)
        {
            printf("\tSPDIF Ac3 Compressed\n");
            /* Send compressed AC3 data to SPDIF */
            NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                       NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed));
        }
        else
        {
            printf("\tSPDIF Stereo PCM\n");
            NEXUS_AudioOutput_AddInput(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                                       NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eStereo));
        }
    }
    #endif

    #if NEXUS_NUM_HDMI_OUTPUTS
    if (enableHdmi)
    {
        if (encodeCodec == NEXUS_AudioCodec_eAc3Plus && g_compressed)
        {
            printf("\tHDMI DDP Compressed\n");
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                       NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed4x));
        }
        else if (encodeCodec == NEXUS_AudioCodec_eAc3 && g_compressed)
        {
            printf("\tHDMI Ac3 Compressed\n");
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                       NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eCompressed));
        }
        else
        {
            printf("\tHDMI %s PCM\n", (codec == NEXUS_AudioCodec_eAc3Plus && multich71) ? "7.1ch" : "5.1ch");
            NEXUS_AudioOutput_AddInput(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                                       NEXUS_DolbyDigitalReencode_GetConnector(ddre, NEXUS_AudioConnectorType_eMultichannel));
        }
    }
    #endif
    printf("-------------------------\n\n");

    /* Bring up video display and outputs */
    NEXUS_Display_GetDefaultSettings(&displaySettings);
#if NEXUS_NUM_HDMI_OUTPUTS > 0
    NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( hdmiStatus.connected ) {
        displaySettings.format = hdmiStatus.preferredVideoFormat;
    }
#endif
    display = NEXUS_Display_Open(0, &displaySettings);
    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutput_GetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);
    hdmiSettings.hotplugCallback.callback = hotplug_callback;
    hdmiSettings.hotplugCallback.context = platformConfig.outputs.hdmi[0];
    hdmiSettings.hotplugCallback.param = (int)display;
    NEXUS_HdmiOutput_SetSettings(platformConfig.outputs.hdmi[0], &hdmiSettings);

    /* initalize HDCP settings, keys, etc. */
    initializeHdmiOutputHdcpSettings() ;

    /* Force a hotplug to switch to preferred format */
    hotplug_callback(platformConfig.outputs.hdmi[0], (int)display);
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    if (fname == NULL) /* live input */
    {
        /* Map a parser band to the streamer input band. */
        parserBand = NEXUS_ParserBand_e0;
        NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);

        /* Get the streamer input band from Platform. Platform has already configured the FPGA with a default streamer routing */
        NEXUS_Platform_GetStreamerInputBand(0, &inputBand);
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = inputBand;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

        /* Open the audio and video pid channels */
        videoPidChannel = NEXUS_PidChannel_Open(parserBand, videoPid, NULL);
        mainPidChannel = NEXUS_PidChannel_Open(parserBand, audioPid, NULL);
        if ( secondaryPid > 0 )
        {
            secondaryPidChannel = NEXUS_PidChannel_Open(parserBand, secondaryPid, NULL);
        }
        else
        {
            secondaryPidChannel = NULL;
        }

        /* Open the StcChannel to do lipsync with the PCR */
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_ePcr;
        stcSettings.modeSettings.pcr.pidChannel = videoPidChannel; /* PCR happens to be on video pid */
        stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
    }
    else /* playback */
    {
        #if NEXUS_HAS_PLAYBACK
        file = NEXUS_FilePlay_OpenPosix(fname, NULL);
        if (!file) {
            fprintf(stderr, "can't open file:%s\n", fname);
            return -1;
        }

        /* Open the StcChannel to do lipsync with the PCR */
        NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
        stcSettings.timebase = NEXUS_Timebase_e0;
        stcSettings.mode = NEXUS_StcChannelMode_eAuto;
        stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
        stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

        /* Configure Playback */
        NEXUS_Playback_GetSettings(playback, &playbackSettings);
        playbackSettings.playpump = playpump;
        /* set a stream format, it could be any audio video transport type or file format, i.e NEXUS_TransportType_eMp4, NEXUS_TransportType_eAvi ... */
        playbackSettings.playpumpSettings.transportType = NEXUS_TransportType_eTs;
        playbackSettings.stcChannel = stcChannel;
        NEXUS_Playback_SetSettings(playback, &playbackSettings);

        /* Open the audio and video pid channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
        playbackPidSettings.pidTypeSettings.video.codec = videoCodec;
        playbackPidSettings.pidTypeSettings.video.index = true;
        playbackPidSettings.pidTypeSettings.video.decoder = videoDecoder;
        videoPidChannel = NEXUS_Playback_OpenPidChannel(playback, videoPid, &playbackPidSettings);

        /* Open the audio and video pid channels */
        NEXUS_Playback_GetDefaultPidChannelSettings(&playbackPidSettings);
        playbackPidSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
        playbackPidSettings.pidTypeSettings.audio.primary = mainDecoder;
        mainPidChannel = NEXUS_Playback_OpenPidChannel(playback, audioPid, &playbackPidSettings);
        if ( secondaryPid > 0 )
        {
            secondaryPidChannel = NEXUS_Playback_OpenPidChannel(playback, secondaryPid, &playbackPidSettings);
        }
        #endif
    }

    /* Set up decoder Start structures now. We need to know the audio codec to properly set up the audio outputs. */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = videoCodec;
    videoProgram.pidChannel = videoPidChannel;
    videoProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&mainProgram);
    mainProgram.codec = codec;
    mainProgram.pidChannel = mainPidChannel;
    mainProgram.stcChannel = stcChannel;
    NEXUS_AudioDecoder_GetDefaultStartSettings(&secondaryProgram);
    secondaryProgram.codec = secondaryCodec;
    secondaryProgram.pidChannel = secondaryPidChannel;
    secondaryProgram.stcChannel = stcChannel;
    secondaryProgram.secondaryDecoder = true;   /* Indicate this is a secondary channel for STC Channel/PCRlib functions */

    /* Set up Codec Settings */
    NEXUS_AudioDecoder_GetCodecSettings(mainDecoder, codec, &primaryCodecSettings);
    if ( !secondaryPidChannel && codec == NEXUS_AudioCodec_eAc3Plus )
    {
        primaryCodecSettings.codecSettings.ac3Plus.enableAtmosProcessing = true;
    }
    NEXUS_AudioDecoder_SetCodecSettings(mainDecoder, &primaryCodecSettings);

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);

    if (startDecoders)
    {
        BDBG_WRN(("Starting Main"));
        NEXUS_AudioDecoder_Start(mainDecoder, &mainProgram);

        if ( secondaryPidChannel )
        {
            BDBG_WRN(("Starting Secondary"));
            NEXUS_AudioDecoder_Start(secondaryDecoder, &secondaryProgram);
        }

        if (fname != NULL)
        {
            #if NEXUS_HAS_PLAYBACK
            /* Start playback */
            NEXUS_Playback_Start(playback, file, NULL);
            #endif
        }
    }

    printf("Press ENTER to stop decode\n");
    getchar();

    /* Stop */
    if (file)
    {
        #if NEXUS_HAS_PLAYBACK
        NEXUS_Playback_Stop(playback);
        #endif
    }

    NEXUS_VideoDecoder_Stop(videoDecoder);

    if (startDecoders)
    {
        if (fname != NULL)
        {
            #if NEXUS_HAS_PLAYBACK
            NEXUS_Playback_Stop(playback);
            #endif
        }

        if ( secondaryPidChannel )
        {
            NEXUS_AudioDecoder_Stop(secondaryDecoder);
        }
        NEXUS_AudioDecoder_Stop(mainDecoder);
    }

    /* Teardown */
    if (file)
    {
        #if NEXUS_HAS_PLAYBACK
        NEXUS_Playback_CloseAllPidChannels(playback);
        NEXUS_FilePlay_Close(file);
        NEXUS_Playback_Destroy(playback);
        NEXUS_Playpump_Close(playpump);
        #endif
    }
    else
    {
        NEXUS_PidChannel_Close(videoPidChannel);
        NEXUS_PidChannel_Close(mainPidChannel);
        if ( secondaryPidChannel )
        {
            NEXUS_PidChannel_Close(secondaryPidChannel);
        }
    }

    #if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
    #endif
    #if NEXUS_NUM_SPDIF_OUTPUTS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
    #endif
    #if NEXUS_NUM_AUDIO_DACS
    NEXUS_AudioOutput_RemoveAllInputs(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
    #endif
    NEXUS_DolbyDigitalReencode_Close(ddre);

    NEXUS_AudioMixer_RemoveAllInputs(mixer);
    NEXUS_AudioMixer_Close(mixer);

    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(mainDecoder, NEXUS_AudioConnectorType_eMultichannel));
    NEXUS_AudioDecoder_Close(mainDecoder);
    NEXUS_AudioInput_Shutdown(NEXUS_AudioDecoder_GetConnector(secondaryDecoder, NEXUS_AudioConnectorType_eMultichannel));
    NEXUS_AudioDecoder_Close(secondaryDecoder);
    NEXUS_VideoWindow_Close(window);
    NEXUS_Display_Close(display);
    NEXUS_VideoDecoder_Close(videoDecoder);
    NEXUS_StcChannel_Close(stcChannel);
    NEXUS_Platform_Uninit();

    return 0;
}
#else
#include "bstd.h"

int main(int argc, char **argv)
{
    BSTD_UNUSED(argc);
    BSTD_UNUSED(argv);
    return 0;
}
#endif
