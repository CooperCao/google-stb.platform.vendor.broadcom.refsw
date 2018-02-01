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

#include "standby.h"
#include "util.h"


BDBG_MODULE(display);

extern B_StandbyNexusHandles g_StandbyNexusHandles;
extern B_DeviceState g_DeviceState;



void display_open(unsigned id)
{
    NEXUS_DisplaySettings displaySettings;

    NEXUS_Display_GetDefaultSettings(&displaySettings);
    switch(id) {
        case 0:
            if(!g_StandbyNexusHandles.displayHD ) {
#if NEXUS_HAS_HDMI_OUTPUT
                NEXUS_HdmiOutputStatus status;
                NEXUS_DisplayCapabilities displayCap;
                NEXUS_HdmiOutput_GetStatus(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &status);
                NEXUS_GetDisplayCapabilities(&displayCap);
                if (status.connected && displayCap.displayFormatSupported[status.preferredVideoFormat]) {
                    displaySettings.format = status.preferredVideoFormat;
                }
#else
                displaySettings.format = NEXUS_VideoFormat_e1080i;
#endif
                g_StandbyNexusHandles.displayHD = NEXUS_Display_Open(id, &displaySettings);
                BDBG_ASSERT(g_StandbyNexusHandles.displayHD);
            }
            break;
        case 1:
            if(!g_StandbyNexusHandles.displaySD ) {
                displaySettings.format = NEXUS_VideoFormat_eNtsc;
                g_StandbyNexusHandles.displaySD = NEXUS_Display_Open(id, &displaySettings);
                BDBG_ASSERT(g_StandbyNexusHandles.displaySD);
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void display_close(unsigned id)
{
    switch(id) {
        case 0:
            if(g_StandbyNexusHandles.displayHD) {
                NEXUS_Display_Close(g_StandbyNexusHandles.displayHD);
                g_StandbyNexusHandles.displayHD = NULL;
            }
            break;
        case 1:
            if(g_StandbyNexusHandles.displaySD) {
                NEXUS_Display_Close(g_StandbyNexusHandles.displaySD);
                g_StandbyNexusHandles.displaySD = NULL;
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void add_window_input(unsigned decoder_id)
{
    BDBG_ASSERT(decoder_id <= MAX_CONTEXTS);

    if(!g_StandbyNexusHandles.videoDecoder[decoder_id])
        return;

    if(g_DeviceState.decoder_connected[decoder_id])
        return;

    if(g_StandbyNexusHandles.windowHD[decoder_id])
        NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowHD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));
    if(g_StandbyNexusHandles.windowSD[decoder_id])
        NEXUS_VideoWindow_AddInput(g_StandbyNexusHandles.windowSD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));

    g_DeviceState.decoder_connected[decoder_id] = true;
}

void remove_window_input(unsigned decoder_id)
{
    BDBG_ASSERT(decoder_id <= MAX_CONTEXTS);

    if(!g_StandbyNexusHandles.videoDecoder[decoder_id])
        return;

    if(!g_DeviceState.decoder_connected[decoder_id])
        return;

    if(g_StandbyNexusHandles.windowHD[decoder_id])
        NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowHD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));
    if(g_StandbyNexusHandles.windowSD[decoder_id])
        NEXUS_VideoWindow_RemoveInput(g_StandbyNexusHandles.windowSD[decoder_id], NEXUS_VideoDecoder_GetConnector(g_StandbyNexusHandles.videoDecoder[decoder_id]));

    g_DeviceState.decoder_connected[decoder_id] = false;
}

void window_open(unsigned window_id, unsigned display_id)
{
    BDBG_ASSERT(window_id <= MAX_CONTEXTS);

    switch(display_id) {
        case 0:
            if(g_StandbyNexusHandles.displayHD) {
                g_StandbyNexusHandles.windowHD[window_id] = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displayHD, window_id);
                BDBG_ASSERT(g_StandbyNexusHandles.windowHD[window_id]);
            } else
                BDBG_WRN(("HD Display not opened"));
            break;
        case 1:
            if(g_StandbyNexusHandles.displaySD) {
                g_StandbyNexusHandles.windowSD[window_id] = NEXUS_VideoWindow_Open(g_StandbyNexusHandles.displaySD, window_id);
                BDBG_ASSERT(g_StandbyNexusHandles.windowSD[window_id]);
            } else
                BDBG_WRN(("SD Display not opened"));
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

void window_close(unsigned window_id, unsigned display_id)
{
    BDBG_ASSERT(window_id <= MAX_CONTEXTS);

    switch(display_id) {
        case 0:
            if(g_StandbyNexusHandles.windowHD[window_id]) {
                NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowHD[window_id]);
                g_StandbyNexusHandles.windowHD[window_id] = NULL;
            }
            break;
        case 1:
            if(g_StandbyNexusHandles.windowSD[window_id]) {
                NEXUS_VideoWindow_Close(g_StandbyNexusHandles.windowSD[window_id]);
                g_StandbyNexusHandles.windowSD[window_id] = NULL;
            }
            break;
        default:
            BDBG_WRN(("Unsupported display id"));
    }
}

#if NEXUS_HAS_CEC && NEXUS_HAS_HDMI_OUTPUT
void cecDeviceReady_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_WRN(("BCM%d Logical Address <%d> Acquired",
                BCHP_CHIP,
                status.logicalAddress)) ;

    BDBG_WRN(("BCM%d Physical Address: %X.%X.%X.%X",
                BCHP_CHIP,
                (status.physicalAddress[0] & 0xF0) >> 4,
                (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4,
                (status.physicalAddress[1] & 0x0F))) ;

    if ((status.physicalAddress[0] = 0xFF)
            && (status.physicalAddress[1] = 0xFF))
    {
        BDBG_WRN(("CEC Device Ready!")) ;
        g_DeviceState.cecDeviceReady = true ;
    }
}

void cecMsgReceived_callback(void *context, int param)
{
    NEXUS_CecStatus status;
    NEXUS_CecReceivedMessage receivedMessage;
    NEXUS_Error rc ;
    uint8_t i, j ;
    char msgBuffer[3*(NEXUS_CEC_MESSAGE_DATA_SIZE +1)];

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_MSG(("Message Received: %s", status.messageReceived ? "Yes" : "No")) ;

    rc = NEXUS_Cec_ReceiveMessage(g_StandbyNexusHandles.hCec, &receivedMessage);
    BDBG_ASSERT(!rc);

    /* For debugging purposes */
    for (i = 0, j = 0; i < receivedMessage.data.length ; i++)
    {
        j += BKNI_Snprintf(msgBuffer + j, sizeof(msgBuffer)-j, "%02X ",
                receivedMessage.data.buffer[i]) ;
    }
    BDBG_MSG(("CEC Message Length %d Received: %s",
                receivedMessage.data.length, msgBuffer)) ;

    BDBG_MSG(("Msg Recd Status from Phys/Logical Addrs: %X.%X.%X.%X / %d",
                (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
                status.logicalAddress)) ;

    if((receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x40) || /* User Control Pressed : Power */
       (receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x6b) || /* User Control Pressed : Power Toggle */
       (receivedMessage.data.buffer[0] == 0x44 && receivedMessage.data.buffer[1] == 0x6d) || /* User Control Pressed : Power On */
       (receivedMessage.data.buffer[0] == 0x86 && receivedMessage.data.buffer[1] == 0x0 && receivedMessage.data.buffer[2] == 0x0)   /* Set Stream Path */
      ) {
        if(g_DeviceState.power_mode == ePowerModeS1 && g_StandbyNexusHandles.s1Event) {
            BKNI_SetEvent(g_StandbyNexusHandles.s1Event);
        }
    }
}

void cecMsgTransmitted_callback(void *context, int param)
{
    NEXUS_CecStatus status;

    BSTD_UNUSED(param);
    BSTD_UNUSED(context);
    /* BKNI_SetEvent((BKNI_EventHandle)context); */
    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &status);

    BDBG_WRN(("Msg Xmit Status for Phys/Logical Addrs: %X.%X.%X.%X / %d",
                (status.physicalAddress[0] & 0xF0) >> 4, (status.physicalAddress[0] & 0x0F),
                (status.physicalAddress[1] & 0xF0) >> 4, (status.physicalAddress[1] & 0x0F),
                status.logicalAddress)) ;

    BDBG_WRN(("Xmit Msg Acknowledged: %s",
                status.transmitMessageAcknowledged ? "Yes" : "No")) ;
    BDBG_WRN(("Xmit Msg Pending: %s",
                status.messageTransmitPending ? "Yes" : "No")) ;
}

void cec_setup(void)
{
    NEXUS_CecSettings cecSettings;
    NEXUS_CecStatus cecStatus;
    NEXUS_HdmiOutputStatus status;
    unsigned loops;

    g_StandbyNexusHandles.hCec = g_StandbyNexusHandles.platformConfig.outputs.cec[0];

    NEXUS_HdmiOutput_GetStatus(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &status);

    NEXUS_Cec_GetSettings(g_StandbyNexusHandles.hCec, &cecSettings);
    cecSettings.messageReceivedCallback.callback = cecMsgReceived_callback ;
    cecSettings.messageReceivedCallback.context = g_StandbyNexusHandles.event;

    cecSettings.messageTransmittedCallback.callback = cecMsgTransmitted_callback;
    cecSettings.messageTransmittedCallback.context = g_StandbyNexusHandles.event;

    cecSettings.logicalAddressAcquiredCallback.callback = cecDeviceReady_callback ;
    cecSettings.logicalAddressAcquiredCallback.context = g_StandbyNexusHandles.event;

    cecSettings.physicalAddress[0]= (status.physicalAddressA << 4)
        | status.physicalAddressB;
    cecSettings.physicalAddress[1]= (status.physicalAddressC << 4)
        | status.physicalAddressD;

    NEXUS_Cec_SetSettings(g_StandbyNexusHandles.hCec, &cecSettings);

    /* Enable CEC core */
    NEXUS_Cec_GetSettings(g_StandbyNexusHandles.hCec, &cecSettings);
    cecSettings.enabled = true;
    NEXUS_Cec_SetSettings(g_StandbyNexusHandles.hCec, &cecSettings);

    printf("Wait for logical address before starting test...\n");
    for (loops = 0; loops < 20; loops++) {
        if (g_DeviceState.cecDeviceReady)
            break;
        BKNI_Sleep(100);
    }

    NEXUS_Cec_GetStatus(g_StandbyNexusHandles.hCec, &cecStatus);
    if (cecStatus.logicalAddress == 0xFF)
    {
        printf("No CEC capable device found on HDMI output\n");
    }
}
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
static void hotplug_callback(void *pParam, int iParam)
{
    NEXUS_HdmiOutputStatus status;
    NEXUS_HdmiOutputHandle hdmi = pParam;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_HdmiOutputSettings hdmiSettings;

    BSTD_UNUSED(iParam);

    NEXUS_HdmiOutput_GetStatus(hdmi, &status);
    /* the app can choose to switch to the preferred format, but it's not required. */
    if ( !status.connected )
    {
        BDBG_WRN(("No RxDevice Connected")) ;
        return ;
    }

    NEXUS_Display_GetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
    if ( !status.videoFormatSupported[displaySettings.format] )
    {
        BDBG_ERR(("Current format not supported by attached monitor. Switching to preferred format %d", status.preferredVideoFormat));
        displaySettings.format = status.preferredVideoFormat;
        NEXUS_Display_SetSettings(g_StandbyNexusHandles.displayHD, &displaySettings);
    }

    /* force HDMI updates after a hotplug */
    NEXUS_HdmiOutput_GetSettings(hdmi, &hdmiSettings);
    NEXUS_HdmiOutput_SetSettings(hdmi, &hdmiSettings);
}

static void mhl_standby_callback(void *pParam, int iParam)
{
    BSTD_UNUSED(pParam);
    BSTD_UNUSED(iParam);
    BDBG_MSG(("MHL standby callback"));

    g_DeviceState.power_mode = ePowerModeS3;
    if(g_StandbyNexusHandles.event)
        BKNI_SetEvent(g_StandbyNexusHandles.event);
}
#endif

void add_hdmi_output(void)
{
#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_HdmiOutputSettings hdmiSettings;

    if (g_DeviceState.hdmi_connected)
        return;

    if (g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        NEXUS_Display_AddOutput(g_StandbyNexusHandles.displayHD, NEXUS_HdmiOutput_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]));

        NEXUS_HdmiOutput_GetSettings(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &hdmiSettings);
        hdmiSettings.hotplugCallback.callback = hotplug_callback;
        hdmiSettings.hotplugCallback.context = g_StandbyNexusHandles.platformConfig.outputs.hdmi[0];

        /* MHL support */
        hdmiSettings.mhlStandbyCallback.callback = mhl_standby_callback;
        hdmiSettings.mhlStandbyCallback.context = g_StandbyNexusHandles.platformConfig.outputs.hdmi[0];

        NEXUS_HdmiOutput_SetSettings(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0], &hdmiSettings);
    }

    if (g_StandbyNexusHandles.audioDecoder[0] && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_HdmiOutput_GetAudioConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

        g_DeviceState.hdmi_connected = true;
    }

#endif
}

void remove_hdmi_output(void)
{
#if NEXUS_NUM_HDMI_OUTPUTS
    if (!g_DeviceState.hdmi_connected)
        return;

    if (g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displayHD, NEXUS_HdmiOutput_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]));
    }

    if (g_StandbyNexusHandles.audioDecoder[0] && g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]) {
        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[0]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_HdmiOutput_GetAudioConnector(g_StandbyNexusHandles.platformConfig.outputs.hdmi[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[0], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[0] && g_DeviceState.opts[0].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[0], &g_StandbyNexusHandles.audioProgram[0]);

        g_DeviceState.hdmi_connected = false;
    }
#endif
}

void add_component_output(void)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.component_connected)
        return;

    if(g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.component[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displayHD, NEXUS_ComponentOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.component[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.component_connected = true;
    }
#endif
}

void remove_component_output(void)
{
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.component_connected)
        return;

    if(g_StandbyNexusHandles.displayHD && g_StandbyNexusHandles.platformConfig.outputs.component[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displayHD, NEXUS_ComponentOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.component[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.component_connected = false;
    }
#endif
}

void add_composite_output(void)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.composite_connected)
        return;

    if(g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.composite[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displaySD, NEXUS_CompositeOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.composite[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.composite_connected = true;
    }
#endif
}

void remove_composite_output(void)
{
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.composite_connected)
        return;

    if(g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.composite[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displaySD, NEXUS_CompositeOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.composite[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.composite_connected = false;
    }
#endif
}

void add_rfm_output(void)
{
#if NEXUS_NUM_RFM_OUTPUTS
    NEXUS_Error rc;
    if (g_DeviceState.rfm_connected)
        return;

    if (g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.rfm[0]) {
        rc = NEXUS_Display_AddOutput(g_StandbyNexusHandles.displaySD, NEXUS_Rfm_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.rfm[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.rfm_connected = true;
    }
#endif
}

void remove_rfm_output(void)
{
#if NEXUS_NUM_RFM_OUTPUTS
    NEXUS_Error rc;
    if (!g_DeviceState.rfm_connected)
        return;

    if (g_StandbyNexusHandles.displaySD && g_StandbyNexusHandles.platformConfig.outputs.rfm[0]) {
        rc = NEXUS_Display_RemoveOutput(g_StandbyNexusHandles.displaySD, NEXUS_Rfm_GetVideoConnector(g_StandbyNexusHandles.platformConfig.outputs.rfm[0]));

        if(rc == NEXUS_SUCCESS)
            g_DeviceState.rfm_connected = false;
    }
#endif
}



void add_dac_output(unsigned id)
{
#if NEXUS_NUM_AUDIO_DACS
    if (g_DeviceState.dac_connected[id])
        return;

    if(g_StandbyNexusHandles.audioDecoder[id] && g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.dac_connected[id] = true;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void remove_dac_output(unsigned id)
{
#if NEXUS_NUM_AUDIO_DACS
    if (!g_DeviceState.dac_connected[id])
        return;

    if(g_StandbyNexusHandles.audioDecoder[id] && g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_AudioDac_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.dac_connected[id] = false;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void add_spdif_output(unsigned id)
{
#if NEXUS_NUM_SPDIF_OUTPUTS
    if (g_DeviceState.spdif_connected[id])
        return;

    if(g_StandbyNexusHandles.audioDecoder[id] && g_StandbyNexusHandles.platformConfig.outputs.spdif[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_AddInput(
                NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.spdif_connected[id] = true;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void remove_spdif_output(unsigned id)
{
#if NEXUS_NUM_SPDIF_OUTPUTS
    if (!g_DeviceState.spdif_connected[id])
        return;

    if(g_StandbyNexusHandles.audioDecoder[id] && g_StandbyNexusHandles.platformConfig.outputs.spdif[0]) {
        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Stop(g_StandbyNexusHandles.audioDecoder[id]);

        NEXUS_AudioOutput_RemoveInput(
                NEXUS_SpdifOutput_GetConnector(g_StandbyNexusHandles.platformConfig.outputs.spdif[0]),
                NEXUS_AudioDecoder_GetConnector(g_StandbyNexusHandles.audioDecoder[id], NEXUS_AudioDecoderConnectorType_eStereo));

        if(g_DeviceState.decode_started[id] && g_DeviceState.opts[id].audioPid)
            NEXUS_AudioDecoder_Start(g_StandbyNexusHandles.audioDecoder[id], &g_StandbyNexusHandles.audioProgram[id]);

        g_DeviceState.spdif_connected[id] = false;
    }
#else
    BSTD_UNUSED(id);
#endif
}

void add_outputs(void)
{
    add_hdmi_output();
    add_component_output();
    add_composite_output();
    add_rfm_output();

    add_dac_output(0);
    add_spdif_output(0);
}

void remove_outputs(void)
{
    remove_hdmi_output();
    remove_component_output();
    remove_composite_output();
    remove_rfm_output();

    remove_dac_output(0);
    remove_spdif_output(0);
}
