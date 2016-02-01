/******************************************************************************
 *    (c)2008-2013 Broadcom Corporation
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
******************************************************************************/

#include "nexus_platform.h"
#include <stdio.h>
#if NEXUS_HAS_DISPLAY && NEXUS_HAS_SECURITY
#include "nexus_pid_channel.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif
#include "nexus_video_window.h"
#include "bstd.h"
#include "bkni.h"
#include "nexus_security.h"
#include <string.h>

/* the following define the input and its characteristics -- these will vary by input */
#define TRANSPORT_TYPE NEXUS_TransportType_eTs
#define VIDEO_CODEC NEXUS_VideoCodec_eMpeg2
#define VIDEO_PID 0x11

int main(void)
{
    NEXUS_VideoDecoderHandle vdecode;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_ParserBand parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoWindowSettings windowSettings;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputStatus hdmiStatus;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_Error rc;
#endif

    unsigned char   VidEvenControlWord[] = { 0xbe, 0xf9, 0xb0, 0x67,0x13, 0xb8, 0xbc, 0x87};
    unsigned char VidOddControlWord[] = { 0xbc, 0xfb, 0xb2, 0x69,0x13, 0xba, 0xbe, 0x8b};
    unsigned char AudEvenControlWord[] = { 0xd3, 0x94, 0xdd, 0x44,0x13, 0xd5, 0xd1, 0xb9};
    unsigned char AudOddControlWord[] = { 0xd6, 0x91, 0xd8, 0x3f,0x13, 0xd0, 0xd4, 0xb7};
    NEXUS_KeySlotHandle videoKeyHandle = NULL;
    NEXUS_KeySlotHandle audioKeyHandle = NULL;
    NEXUS_SecurityAlgorithmSettings AlgConfig;
    NEXUS_SecurityClearKey key;
    NEXUS_PidChannelStatus pidStatus;
    NEXUS_SecurityKeySlotSettings keySlotSettings;

    /* Bring up all modules for a platform in a default configuraiton for this platform */
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    NEXUS_Platform_Init(&platformSettings);
    NEXUS_Platform_GetConfiguration(&platformConfig);

    /* Map input band and parser band. This could be eliminated because a straight mapping and TS config is a good default. */
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
    NEXUS_Platform_GetStreamerInputBand(0, &parserBandSettings.sourceTypeSettings.inputBand);
    parserBandSettings.transportType = TRANSPORT_TYPE;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    /* Open the pid channels */
    NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
    videoProgram.codec = VIDEO_CODEC;
    videoProgram.pidChannel = NEXUS_PidChannel_Open(parserBand, VIDEO_PID, NULL);

    /* By default, StcChannel will configure NEXUS_Timebase with the info it has */
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcSettings.mode = NEXUS_StcChannelMode_ePcr; /* live */
    stcSettings.modeSettings.pcr.pidChannel = videoProgram.pidChannel; /* PCR on video pid */
    videoProgram.stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /***************************************************************************************
        Config CA descrambler
    ***************************************************************************************/

    NEXUS_Security_GetDefaultKeySlotSettings(&keySlotSettings);
    keySlotSettings.keySlotEngine = NEXUS_SecurityEngine_eCa;
    /* Allocate AV keyslots */
    videoKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !videoKeyHandle)
    {
        printf("\nAllocate CA video keyslot failed \n");
        return 1;
    }
    audioKeyHandle = NEXUS_Security_AllocateKeySlot(&keySlotSettings);
    if ( !audioKeyHandle)
    {
        printf("\nAllocate CA audio keyslot failed \n");
        return 1;
    }

    /* Config AV algorithms */
    NEXUS_Security_GetDefaultAlgorithmSettings(&AlgConfig);
    AlgConfig.algorithm = NEXUS_SecurityAlgorithm_eDvb;
    AlgConfig.dvbScramLevel = NEXUS_SecurityDvbScrambleLevel_eTs;
    if ( NEXUS_Security_ConfigAlgorithm (videoKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm failed \n");
        return 1;
    }
    if ( NEXUS_Security_ConfigAlgorithm (audioKeyHandle, &AlgConfig) != 0 )
    {
        printf("\nConfig video CA Algorithm failed \n");
        return 1;
    }

    /* Load AV keys */
    NEXUS_Security_GetDefaultClearKey(&key);
    key.keySize = 8;
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,VidEvenControlWord,sizeof(VidEvenControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video EVEN key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,VidOddControlWord,sizeof(VidOddControlWord));
    if ( NEXUS_Security_LoadClearKey (videoKeyHandle, &key) != 0 )
    {
        printf("\nLoad video ODD key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eEven;
    memcpy(key.keyData,AudEvenControlWord,sizeof(AudEvenControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio EVEN key failed \n");
        return 1;
    }
    key.keyEntryType = NEXUS_SecurityKeyType_eOdd;
    memcpy(key.keyData,AudOddControlWord,sizeof(AudOddControlWord));
    if ( NEXUS_Security_LoadClearKey (audioKeyHandle, &key) != 0 )
    {
        printf("\nLoad audio ODD key failed \n");
        return 1;
    }

    /* Add video PID channel to keyslot */
    NEXUS_PidChannel_GetStatus (videoProgram.pidChannel, &pidStatus);
    if ( NEXUS_Security_AddPidChannelToKeySlot(videoKeyHandle, pidStatus.pidChannelIndex)!= 0 )
    {
        printf("\nConfigPIDPointerTable failed \n");
        return 1;
    }

    printf ("\nSecurity Config OK\n");

    /* Add audio PID channel to keyslot */
    /* NEXUS_Security_AddPidChannelToKeySlot(audioKeyHandle, audioProgram.pidChannel); */

    /* bring up display */
    display = NEXUS_Display_Open(0, NULL);
    window = NEXUS_VideoWindow_Open(display, 0);

#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
    rc = NEXUS_HdmiOutput_GetStatus(platformConfig.outputs.hdmi[0], &hdmiStatus);
    if ( !rc && hdmiStatus.connected )
    {
        /* If current display format is not supported by monitor, switch to monitor's preferred format. 
           If other connected outputs do not support the preferred format, a harmless error will occur. */
        NEXUS_Display_GetSettings(display, &displaySettings);
        if ( !hdmiStatus.videoFormatSupported[displaySettings.format] ) {
            displaySettings.format = hdmiStatus.preferredVideoFormat;
            NEXUS_Display_SetSettings(display, &displaySettings);
		}
    }
#endif

    /* bring up decoder and connect to display */
    vdecode = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
    NEXUS_VideoWindow_GetSettings(window, &windowSettings);
    windowSettings.visible = true;
    NEXUS_VideoWindow_SetSettings(window, &windowSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(vdecode));

    /* Start Decoders */
    NEXUS_VideoDecoder_Start(vdecode, &videoProgram);

    for (;;) {
        NEXUS_VideoDecoderStatus status;
        uint32_t stc;

        NEXUS_VideoDecoder_GetStatus(vdecode, &status);
        NEXUS_StcChannel_GetStc(videoProgram.stcChannel, &stc);
        printf("decode %dx%d, pts %#x, stc %#x (diff %d)\n",
            status.source.width, status.source.height, status.pts, stc, status.ptsStcDifference);
        BKNI_Sleep(1000);
    }

    getchar();

    NEXUS_Security_RemovePidChannelFromKeySlot(videoKeyHandle, pidStatus.pid);
    NEXUS_Security_FreeKeySlot(videoKeyHandle);
    return 0;
}
#else
int main(void)
{
    printf("This application is not supported on this platform\n");
    return 0;
}
#endif
