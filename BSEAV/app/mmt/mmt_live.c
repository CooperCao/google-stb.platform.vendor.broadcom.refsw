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

#include "nexus_platform.h"
#include <stdio.h>

#if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_video_decoder.h"
#include "nexus_stc_channel.h"
#include "nexus_display.h"
#include "nexus_video_window.h"
#include "nexus_video_input.h"
#include "nexus_audio_dac.h"
#include "nexus_audio_decoder.h"
#include "nexus_audio_output.h"
#include "nexus_audio_input.h"
#include "nexus_composite_output.h"
#include "nexus_component_output.h"
#include "nexus_core_utils.h"
#if NEXUS_HAS_HDMI_OUTPUT
#include "nexus_hdmi_output.h"
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "bmmt.h"

BDBG_MODULE(mmt_live);

/**
  * mmt_live app decodes and renders AV from live mpeg2ts stream
  * (3 byte header + 185 bytes TLV data per 188 byte packet)
  *
  * DekTek DTU-215 streamer is used to live stream a file to a
  * QAM tuner/demod on the reference board.
  *
  * In the streamXpress app for DTU-215 modulator:
  * 1. rmx should be turned off
  * 2. Modulation parameters should be QAM-C and QAM-256
  *
  *  live stream file brcm_mmt_1pgm_36mbps_hevc_aac.v2.ts is
  *  a broadcom generated stream with 38.8Mbps.
  *
  **/

#define MAX_ACQUIRE_TIME 20000
#define MAX_PACKAGES 8
static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void async_status_ready_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_complete_callback(void *context, int param)
{
    BSTD_UNUSED(param);

    BDBG_WRN(("cppm_complete_callback"));
    BKNI_SetEvent((BKNI_EventHandle)context);
}

static void cppm_callback(void *context, int param)
{
    NEXUS_Error rc;
    NEXUS_FrontendHandle frontend = (NEXUS_FrontendHandle) context;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    BSTD_UNUSED(param);

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        BDBG_ERR(("Unable to retrieve frontend device handle.\n"));
    }

    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = true;
    calibrateSettings.cppm.threshold = 250;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) {
        BDBG_ERR(("NEXUS_FrontendDevice_Recalibrate exited with rc = %d", rc));
    }
}

static unsigned b_get_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000 + tv.tv_usec/1000;
}

static void print_usage(void)
{
    printf(
        "Usage: mmt_live \n"
        "  --help or -h for help\n"
        "  -freq   #   frequency in MHz\n"
        "  -tlv_pid # pid carrying the TLV data \n"
         );
}

static NEXUS_Error get_status(NEXUS_FrontendHandle frontend, BKNI_EventHandle statusEvent)
{
    NEXUS_FrontendQamStatus qamStatus;
    int rc;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceStatus deviceStatus;

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    rc = NEXUS_Frontend_RequestQamAsyncStatus(frontend);
    if(rc == NEXUS_SUCCESS){
        rc = BKNI_WaitForEvent(statusEvent, 1000);
        if (rc) {
            printf("Status not returned\n");
            return BERR_TRACE(rc);
        }
        NEXUS_Frontend_GetQamAsyncStatus(frontend , &qamStatus);

        if (deviceHandle) {
            rc = NEXUS_FrontendDevice_GetStatus(deviceHandle, &deviceStatus);
            if(rc) return BERR_TRACE(rc);
        }
    }
    else if(rc == NEXUS_NOT_SUPPORTED){
        rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
        if (rc) return BERR_TRACE(rc);
    }
    else {
        return BERR_TRACE(rc);
    }

    printf("\nDownstream lock = %d\n", qamStatus.fecLock);
    printf("Frequency = %d\n", qamStatus.settings.frequency);
    if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e64)
        printf("Mode = NEXUS_FrontendQamMode_e64\n");
    else if(qamStatus.settings.mode == NEXUS_FrontendQamMode_e256)
        printf("Mode  = NEXUS_FrontendQamMode_e256\n");
    else
        printf("Mode = %d\n", qamStatus.settings.mode);
    if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eA)
        printf("Annex = NEXUS_FrontendQamAnnex_eA\n");
    else if(qamStatus.settings.annex == NEXUS_FrontendQamAnnex_eB)
        printf("Annex  = NEXUS_FrontendQamAnnex_eB\n");
    else
        printf("Annex = %d\n", qamStatus.settings.annex);
    printf("Symbol rate = %d\n", qamStatus.symbolRate);
    printf("Snr estimate = %d\n", qamStatus.snrEstimate/100 );
    printf("FecCorrected = %d\n", qamStatus.fecCorrected);
    printf("FecUncorrected = %d\n", qamStatus.fecUncorrected);
    printf("DsChannelPower in dBmV = %d\n", qamStatus.dsChannelPower/10);
    printf("DsChannelPower in dBm = %d\n", qamStatus.dsChannelPower/10 - 48);
    if (deviceHandle) {
        printf("AVS enabled = %d\n", deviceStatus.avs.enabled);
        printf("AVS voltage = %d\n", deviceStatus.avs.voltage);
        printf("Device temperature = %d\n", deviceStatus.temperature);
    }
    return 0;
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_FrontendUserParameters userParams;
    NEXUS_FrontendHandle frontend = NULL;
    NEXUS_FrontendQamSettings qamSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_FrontendDeviceHandle deviceHandle;
    NEXUS_FrontendDeviceRecalibrateSettings calibrateSettings;
    NEXUS_Error rc;
    /* default freq & qam mode */
    unsigned freq = 576000000;
    BKNI_EventHandle statusEvent, lockChangedEvent, cppmEvent;
    bool done = false;
    bool cppm = false;
    bool waitForCppm = false;
    int curarg = 1;
    unsigned  mode = 256;
    unsigned maxAcquireTime = MAX_ACQUIRE_TIME;
    bmmt_t mmt = NULL;
    bmmt_open_settings open_settings;
    bmmt_stream_settings video_stream_settings;
    bmmt_stream_t video_stream;
    bmmt_stream_settings audio_stream_settings;
    bmmt_stream_t audio_stream;
    bmmt_msg_settings msg_settings;
    bmmt_msg_t amt_msg=NULL;
    bmmt_msg_t plt_msg=NULL;
    bmmt_msg_t mpt_msg=NULL;
    uint8_t mmt_si_buf[BMMT_MAX_MMT_SI_BUFFER_SIZE];
    uint8_t tlv_si_buf[BMMT_MAX_TLV_SI_BUFFER_SIZE];
    uint8_t msg_r = 0;
    bmmt_pl_table pl_table;
    bmmt_mp_table mp_table[MAX_PACKAGES];
    btlv_am_table am_table;
    btlv_ip_address ip_addr;
    NEXUS_VideoDecoderSettings videoDecoderSettings;
    NEXUS_AudioDecoderSettings audioDecoderSettings;
    NEXUS_FrontendAcquireSettings acquireSettings;
    NEXUS_PidChannelHandle videoPidChannel, audioPidChannel;
    NEXUS_VideoDecoderStartSettings videoProgram;
    NEXUS_AudioDecoderStartSettings audioProgram;
    bool acquired = false;
    bool firstTune = true;
    unsigned i = 0;
    unsigned start_time;

    bmmt_get_default_open_settings(&open_settings);
    open_settings.tlv_pid = 0x2d;
    /**
      *  read command line parameters
      **/
    while (curarg < argc) {
        if (!strcmp(argv[curarg], "-h") || !strcmp(argv[curarg], "--help")) {
            print_usage();
            return 0;
        }
        else if (!strcmp(argv[curarg], "-freq") && argc>curarg+1) {
            float f;
            if (sscanf(argv[++curarg], "%f", &f) != 1) f = 0;
            if (f < 1000000)
                freq = (unsigned)(f*1000) * 1000;
            else
                freq = (unsigned)f;
        }
        else if (!strcmp("-tlv_pid",argv[curarg]) && argc>curarg+1) {
             open_settings.tlv_pid = strtol(argv[++curarg],NULL,0);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    /**
      * Increase the bit rate support for a parser band to 50Mbps to
      * support 38.8Mpbs input stream rate and initialize the nexus
      * platform
      **/
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    platformSettings.transportModuleSettings.maxDataRate.parserBand[NEXUS_ParserBand_e0] = 50 * 1000 * 1000;
    platformSettings.videoDecoderModuleSettings.deferInit = true;
#if NEXUS_HAS_VIDEO_ENCODER
    platformSettings.videoEncoderSettings.deferInit = true;
#endif
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    /**
      * Open display and add outputs
      **/
    NEXUS_Platform_GetConfiguration(&platformConfig);
    NEXUS_Display_GetDefaultSettings(&displaySettings);
    displaySettings.format = NEXUS_VideoFormat_e720p;
    display = NEXUS_Display_Open(0, &displaySettings);
#if NEXUS_NUM_COMPOSITE_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_CompositeOutput_GetConnector(platformConfig.outputs.composite[0]));
#endif
#if NEXUS_NUM_COMPONENT_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_ComponentOutput_GetConnector(platformConfig.outputs.component[0]));
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
    NEXUS_Display_AddOutput(display, NEXUS_HdmiOutput_GetVideoConnector(platformConfig.outputs.hdmi[0]));
#endif

    /**
      * initialize the nexus frontend device and acquire a QAM
      * channel
      **/
    rc = NEXUS_Platform_InitFrontend();
    if(rc){rc = BERR_TRACE(rc); goto done;}
    NEXUS_Frontend_GetDefaultAcquireSettings(&acquireSettings);
    acquireSettings.capabilities.qam = true;
    frontend = NEXUS_Frontend_Acquire(&acquireSettings);
    if (!frontend)
    {
        BDBG_ERR(("Unable to find QAM-capable frontend"));
        return -1;
    }
    BKNI_CreateEvent(&statusEvent);
    BKNI_CreateEvent(&lockChangedEvent);
    BKNI_CreateEvent(&cppmEvent);

    /**
      * Open nexus STC channel for AV sync
      **/
    NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
    stcSettings.timebase = NEXUS_Timebase_e0;
    stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);

    /**
      *  Open nexus audio decoder and add outputs
      **/
    NEXUS_AudioDecoder_GetDefaultOpenSettings(&audioDecoderOpenSettings);
    audioDecoderOpenSettings.fifoSize = 512*1024;
    audioDecoder = NEXUS_AudioDecoder_Open(0,&audioDecoderOpenSettings);
    if (platformConfig.outputs.audioDacs[0]) {
        if (NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0])) {
            NEXUS_AudioOutput_AddInput(
                NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
        }
    }
    if (NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#if NEXUS_HAS_HDMI_OUTPUT
    if (NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0])) {
        NEXUS_AudioOutput_AddInput(
            NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
            NEXUS_AudioDecoder_GetConnector(audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
    }
#endif

    /**
      *  Open nexus video decoder and nexus video window and connect
      *  them
      **/
    window = NEXUS_VideoWindow_Open(display, 0);
    videoDecoder = NEXUS_VideoDecoder_Open(0, NULL);
    NEXUS_VideoDecoder_GetSettings(videoDecoder, &videoDecoderSettings);
    videoDecoderSettings.maxWidth = 3840;
    videoDecoderSettings.maxHeight = 2160;
    videoDecoderSettings.discardThreshold = 10*45000;
    NEXUS_VideoDecoder_SetSettings(videoDecoder, &videoDecoderSettings);
    NEXUS_VideoWindow_AddInput(window, NEXUS_VideoDecoder_GetConnector(videoDecoder));

    NEXUS_AudioDecoder_GetSettings(audioDecoder,&audioDecoderSettings);
    audioDecoderSettings.discardThreshold = 10*1000;
    NEXUS_AudioDecoder_SetSettings(audioDecoder,&audioDecoderSettings);

    deviceHandle = NEXUS_Frontend_GetDevice(frontend);
    if (!deviceHandle) {
        fprintf(stderr, "Unable to retrieve frontend device handle.\n");
    }

    /**
      *  set nexus front end channel parameters
      * */
    NEXUS_FrontendDevice_GetDefaultRecalibrateSettings(&calibrateSettings);
    calibrateSettings.cppm.enabled = false;
    calibrateSettings.cppm.threshold = 260;
    calibrateSettings.cppm.thresholdHysteresis = 50;
    calibrateSettings.cppm.powerLevelChange.callback = cppm_callback;
    calibrateSettings.cppm.powerLevelChange.context = (void *) frontend;
    calibrateSettings.cppm.calibrationComplete.callback = cppm_complete_callback;
    calibrateSettings.cppm.calibrationComplete.context = cppmEvent;
    rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
    if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);
    NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
    qamSettings.frequency = freq;
    qamSettings.mode = NEXUS_FrontendQamMode_e256;
    qamSettings.symbolRate = 5360537;
    qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
    qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
    qamSettings.lockCallback.callback = lock_changed_callback;
    qamSettings.lockCallback.context = lockChangedEvent;
    qamSettings.asyncStatusReadyCallback.callback = async_status_ready_callback;
    qamSettings.asyncStatusReadyCallback.context = statusEvent;
    qamSettings.autoAcquire = true;
    NEXUS_Frontend_GetUserParameters(frontend, &userParams);
    /**
      *  Map a parser band to the demod's input band.
      **/
    parserBand = NEXUS_ParserBand_e0;
    NEXUS_ParserBand_GetSettings(parserBand, &parserBandSettings);
    if (userParams.isMtsif)
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eMtsif;
        parserBandSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector(frontend);
    }
    else
    {
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eInputBand;
        parserBandSettings.sourceTypeSettings.inputBand = userParams.param1;
    }
    /**
      * set the parser band parameters so that no packets are dropped
      * because of continuity count errors and missing adaptation
      * field.
      **/
    parserBandSettings.transportType = NEXUS_TransportType_eTs;
    parserBandSettings.allPass=true;
    parserBandSettings.teiIgnoreEnabled = true;
    parserBandSettings.acceptAdapt00 = true;
    parserBandSettings.continuityCountEnabled = false;
    parserBandSettings.acceptNullPackets = false;
    NEXUS_ParserBand_SetSettings(parserBand, &parserBandSettings);

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    BKNI_ResetEvent(lockChangedEvent);
tune:
    BDBG_WRN(("tuning %d MHz... mode = %d", freq/1000000, qamSettings.mode));
    rc = NEXUS_Frontend_TuneQam(frontend, &qamSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
    start_time = b_get_time();
    while (1)
    {
        unsigned current_time = b_get_time();
        NEXUS_FrontendFastStatus status;
        if (current_time - start_time > maxAcquireTime)
        {
            BDBG_WRN(("application timeout. cannot acquire."));
            break;
        }
        rc = BKNI_WaitForEvent(lockChangedEvent, maxAcquireTime - (current_time - start_time));
        if (rc == BERR_TIMEOUT)
        {
            BDBG_WRN(("application timeout. cannot acquire."));
            break;
        }
        BDBG_ASSERT(!rc);
        rc = NEXUS_Frontend_GetFastStatus(frontend, &status);
        if (rc == NEXUS_SUCCESS)
        {
            if (status.lockStatus == NEXUS_FrontendLockStatus_eLocked)
            {
                BDBG_WRN(("frontend locked"));
                acquired = true;
                break;
            }
            else if (status.lockStatus == NEXUS_FrontendLockStatus_eUnlocked)
                {
                    BDBG_WRN(("frontend unlocked (acquireInProgress=%d)", status.acquireInProgress));
                    /* Wait for maxAcquireTime when unlocked*/
                }
                else if (status.lockStatus == NEXUS_FrontendLockStatus_eNoSignal)
                    {
                        if(!qamSettings.autoAcquire)
                        {
                            BDBG_WRN(("No signal at the tuned frequency."));
                            break;
                        }
                        else
                            BDBG_WRN(("No signal at the tuned frequency. Waiting till the application timesout to allow auto acquire to try again."));
                    }
                    else
                    {
                        BDBG_ERR(("unknown status: %d", status.lockStatus));
                    }
        }
        else if (rc == NEXUS_NOT_SUPPORTED)
            {
                NEXUS_FrontendQamStatus qamStatus;
                rc = NEXUS_Frontend_GetQamStatus(frontend, &qamStatus);
                if (rc) {rc = BERR_TRACE(rc); return false;}
                BDBG_WRN(("frontend %s (fecLock=%d)", qamStatus.receiverLock?"locked":"unlocked", qamStatus.fecLock));
                if (qamStatus.receiverLock)
                {
                    acquired = true;
                    break;
                }
                    /* we can't conclude no lock until application timeout */
            }
            else
            {
                BERR_TRACE(rc);
            }
    }
    if (acquired)
    {
        /**
          *  mmt module instantiation
          **/
        open_settings.parserBand = parserBand;
        open_settings.input_format = ebmmt_input_format_mpeg2ts;
        mmt =  bmmt_open(&open_settings);
        BDBG_ASSERT(mmt);
        /**
          * open TLV AMT message context
          **/
        bmmt_msg_get_default_settings(&msg_settings);
        msg_settings.msg_type = ebmmt_msg_type_tlv;
        amt_msg = bmmt_msg_open(mmt,&msg_settings);
        /**
          * start the mmt module
          **/
        bmmt_start(mmt);
        /** extract service ids from input stream and their
          *   network addresses from TLV SI AMT
          **/
        while (msg_r < BMMT_MAX_MSG_BUFFERS )
        {
            uint8_t *buf = tlv_si_buf;
            size_t len;
            msg_read1:
            len = bmmt_msg_get_buffer(amt_msg,buf,BMMT_MAX_TLV_SI_BUFFER_SIZE);
            if (len)
            {
                if (bmmt_get_am_table(buf,len,&am_table))
                    break;
                msg_r +=1;
            }
            else
            {
                BKNI_Sleep(50);
                goto msg_read1;
            }
        }
        /**
          *  close TLV msg context
          **/
        bmmt_msg_close(amt_msg);
        if (msg_r == BMMT_MAX_MSG_BUFFERS)
        {
            BDBG_ERR(("TLV SI AMT not found"));
            goto done_mmt;
        }
        /**
          *  set IP filtering for the TLV packets
          **/
        if (am_table.num_of_service_id)
        {
            if (am_table.services[0].is_ipv6)
            {
                ip_addr.type = btlv_ip_address_ipv6;
                BKNI_Memcpy(&ip_addr.address.ipv6.addr,&am_table.services[0].addr.ipv6.dst_addr,sizeof(ip_addr.address.ipv6.addr));
                ip_addr.address.ipv6.port = 0x0; /* ignore port since AMT doesn't provide port number */
            }
            else
            {
                ip_addr.type = btlv_ip_address_ipv4;
                BKNI_Memcpy(&ip_addr.address.ipv4.addr,&am_table.services[0].addr.ipv4.dst_addr,sizeof(ip_addr.address.ipv4.addr));
                ip_addr.address.ipv4.port = 0x0; /* ignore port since AMT doesn't provide port number */
            }
        }
        else
        {
            BDBG_WRN(("no services found in AMT"));
            goto done_mmt;
        }
        bmmt_set_ip_filter(mmt, &ip_addr);
        /**
          *  open PLT message context
          **/
        bmmt_msg_get_default_settings(&msg_settings);
        msg_settings.msg_type = ebmmt_msg_type_mmt;
        msg_settings.pid = 0x0;
        plt_msg = bmmt_msg_open(mmt,&msg_settings);
        /**
          *  extract PLT from PA message
          **/
        msg_r = 0;
        while (msg_r < BMMT_MAX_MSG_BUFFERS)
        {
            uint8_t *buf = mmt_si_buf;
            size_t len;
            msg_read2:
            len = bmmt_msg_get_buffer(plt_msg, buf,BMMT_MAX_MMT_SI_BUFFER_SIZE);
            if (len)
            {
                if (bmmt_get_pl_table(buf,len,&pl_table))
                    break;
                msg_r +=1;
            }
            else
            {
                BKNI_Sleep(50);
                goto msg_read2;
            }
        }
        /**
          *  close plt message context
          **/
        bmmt_msg_close(plt_msg);
        if (msg_r == BMMT_MAX_MSG_BUFFERS)
        {
            BDBG_ERR(("MMT SI PLT not found"));
            goto done_mmt;
        }
        else
        {
            if (pl_table.num_of_packages)
            {
               /*for (i=0;i<pl_table.num_of_packages;i++) */
               i = 0;
               {
                   bmmt_msg_get_default_settings(&msg_settings);
                   msg_settings.msg_type = ebmmt_msg_type_mmt;
                   switch (pl_table.packages[i].location_info.location_type)
                   {
                   case bmmt_general_location_type_id:
                       msg_settings.pid = pl_table.packages[i].location_info.data.packet_id;
                       break;
                   case bmmt_general_location_type_ipv4:
                       msg_settings.pid = pl_table.packages[i].location_info.data.mmt_ipv4.packet_id;
                       break;
                   case bmmt_general_location_type_ipv6:
                       msg_settings.pid = pl_table.packages[i].location_info.data.mmt_ipv6.packet_id;
                       break;
                   default:
                       BDBG_WRN(("MPT packet ID not known"));
                       goto done_mmt;
                    }
                    /**
                      * open MPT message context
                      **/
                    mpt_msg = bmmt_msg_open(mmt,&msg_settings);
                    /**
                      * extract MPT from PA message
                      **/
                    msg_r = 0;
                    while (msg_r < BMMT_MAX_MSG_BUFFERS )
                    {
                        uint8_t *buf = mmt_si_buf;
                        size_t len;
                        msg_read3:
                        len = bmmt_msg_get_buffer(mpt_msg, buf,BMMT_MAX_MMT_SI_BUFFER_SIZE);
                        if (len)
                        {
                            if (bmmt_get_mp_table(buf,len,&mp_table[i]))
                                break;
                             msg_r +=1;
                        }
                        else
                        {
                            BKNI_Sleep(50);
                            goto msg_read3;
                        }
                    }
                    if (msg_r == BMMT_MAX_MSG_BUFFERS)
                    {
                        BDBG_ERR(("MMT SI PMT not found in MMT PID %u",msg_settings.pid));
                        goto done_mmt;
                    }
                    /*bmmt_msg_close(mpt_msg);*/
                }
            }
            else
            {
                BDBG_WRN(("no packages found in the PLT"));
                goto done_mmt;
            }
        }
        /**
          * find video asset index in the 1st MPT
          **/
        for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hev1"));i++);
        /**
          * some sample streams have video string as hvc1
          **/
        if (i==mp_table[0].num_of_assets)
        {
            for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hvc1"));i++);
        }
        /**
          * open video stream context
          **/
        if (i!=mp_table[0].num_of_assets)
        {
            /**
              *   open video stream context for the video packet ID in the
              *   1st asset of MPT
              **/
            bmmt_stream_get_default_settings(&video_stream_settings);
            switch (mp_table[0].assets[i].location_info[0].location_type)
            {
            case bmmt_general_location_type_id:
                video_stream_settings.pid = mp_table[0].assets[i].id[0] << 8 | mp_table[0].assets[i].id[1];
                /*video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.packet_id;*/
                break;
            case bmmt_general_location_type_ipv4:
                video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv4.packet_id;
                break;
            case bmmt_general_location_type_ipv6:
                video_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv6.packet_id;
                break;
            default:
                BDBG_WRN(("video stream location ID not supported"));
                goto done_mmt;
            }
            if (video_stream_settings.pid )
            {
                video_stream_settings.stream_type = bmmt_stream_type_h265;
                BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
                BDBG_WRN(("video_stream_settings.pid %04x",video_stream_settings.pid));
                video_stream = bmmt_stream_open(mmt,&video_stream_settings);
                BDBG_ASSERT(video_stream);
                videoPidChannel = bmmt_stream_get_pid_channel(video_stream);
                NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
                videoProgram.codec = NEXUS_VideoCodec_eH265;
                videoProgram.pidChannel = videoPidChannel;
                videoProgram.stcChannel = stcChannel;
                NEXUS_VideoDecoder_Start(videoDecoder, &videoProgram);
            }
            else
            {
                BDBG_WRN(("video stream location ID not supported"));
            }
        }
        else
        {
            BDBG_WRN(("no video asset was found"));
        }
        /**
          * find audio asset index in the 1st MPT
          **/
        for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i++) ;
        /**
          *   open audio stream context
          **/
        if(i!=mp_table[0].num_of_assets)
        {
            /**
              * open video stream context for the video packet ID in the 1st
              * asset of MPT
              **/
            bmmt_stream_get_default_settings(&audio_stream_settings);
            switch (mp_table[0].assets[i].location_info[0].location_type)
            {
            case bmmt_general_location_type_id:
                audio_stream_settings.pid = mp_table[0].assets[i].id[0] << 8 | mp_table[0].assets[i].id[1];
                /*audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.packet_id;*/
                break;
            case bmmt_general_location_type_ipv4:
                audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv4.packet_id;
                break;
            case bmmt_general_location_type_ipv6:
                audio_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv6.packet_id;
                break;
            default:
                BDBG_WRN(("audio stream location ID not supported"));
                goto done_mmt;
            }
            if (audio_stream_settings.pid )
            {
                audio_stream_settings.stream_type = bmmt_stream_type_aac;
                BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
                BDBG_WRN(("audio_stream_settings.pid %04x",audio_stream_settings.pid));
                audio_stream = bmmt_stream_open(mmt,&audio_stream_settings);
                BDBG_ASSERT(audio_stream);
                audioPidChannel = bmmt_stream_get_pid_channel(audio_stream);
                NEXUS_AudioDecoder_GetDefaultStartSettings(&audioProgram);
                audioProgram.codec = NEXUS_AudioCodec_eAacLoas;
                audioProgram.pidChannel = audioPidChannel;
                audioProgram.stcChannel = stcChannel;
                NEXUS_AudioDecoder_Start(audioDecoder, &audioProgram);

            }
            else
            {
                BDBG_WRN(("audio stream location ID not supported"));
            }
        }
        else
        {
            BDBG_WRN(("no audio asset was found"));
        }
done_mmt:
        BDBG_WRN(("done with mmt"));
    }
    else
    {
        if (firstTune)
        {
            /* trigger CPPM due to lack of signal */
            firstTune = false;
            if (cppm)
            {
                calibrateSettings.cppm.enabled = true;
                rc = NEXUS_FrontendDevice_Recalibrate(deviceHandle, &calibrateSettings);
                if(rc) return BERR_TRACE(BERR_NOT_INITIALIZED);
                if (cppm && waitForCppm)
                {
                    BDBG_WRN(("Waiting for CPPM to complete..."));
                    BKNI_WaitForEvent(cppmEvent, 5000);
                }
                goto tune;
            }
        }
            BDBG_WRN(("not starting decode because frontend not acquired"));
    }
    get_status(frontend, statusEvent);
    BDBG_WRN(("Press any key to exit the app"));
    getchar();
    NEXUS_AudioDecoder_Stop(audioDecoder);
    NEXUS_VideoDecoder_Stop(videoDecoder);
    bmmt_stop(mmt);
    bmmt_close(mmt);
done:
    if(window)NEXUS_VideoWindow_RemoveAllInputs(window);
    if(window)NEXUS_VideoWindow_Close(window);
    if(display)NEXUS_Display_Close(display);
    if(videoDecoder)NEXUS_VideoDecoder_Close(videoDecoder);
    if(audioDecoder)NEXUS_AudioDecoder_Close(audioDecoder);
    if(statusEvent)BKNI_DestroyEvent(statusEvent);
    if(lockChangedEvent)BKNI_DestroyEvent(lockChangedEvent);
    if(cppmEvent)BKNI_DestroyEvent(cppmEvent);
    NEXUS_Platform_Uninit();
    return 0;
}

#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include frontend.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
