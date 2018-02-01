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

#if NEXUS_HAS_ETBG
#include "nexus_frontend.h"
#include "nexus_parser_band.h"
#include "nexus_tsmf.h"
#include "nexus_etbg.h"
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

BDBG_MODULE(tsmf_mmt_live);

/**
  * tsmf_mmt_live app decodes and renders AV from a live
  * extended TSMF mpeg2ts stream (3 byte header + 185 bytes TLV
  * data per 188 byte packet)
  *
  * Broadcom extended TSMF streamer is used to feed the stream
  * to BCM7278-MTSIF interface
  *
  * The test stream is created from the TLV files obtained from
  * APAB.
  */


#define MAX_PACKAGES 8
#define NUM_BANDS_IN_GROUP  3
#define RELATIVE_TS_NUM 1
typedef struct band_config
{
   bool isPrimary;
   unsigned bitRate;
   unsigned numSubFrameSlots;
}band_config;

typedef struct input_band
{
    NEXUS_TsmfHandle tsmf;
    NEXUS_FrontendHandle frontend;
    NEXUS_ParserBand parserBand;
    BKNI_EventHandle lockChanged;
}input_band;

static void print_usage(void)
{
    printf(
        "Usage: mmt_live \n"
        "  --help or -h for help\n"
        "  -tlv_pid # pid carrying the TLV data \n"
         );
}

static void lock_changed_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, char **argv)
{
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    band_config input_config[NUM_BANDS_IN_GROUP] = {
       {true, 40, 4},
       {false, 40, 4},
       {false,  40, 4}
    };
    input_band input[NUM_BANDS_IN_GROUP];
    NEXUS_ParserBand parserBand;
    NEXUS_ParserBandSettings parserBandSettings;
    NEXUS_EtbgHandle hEtbg;
    NEXUS_Etbg_GroupSettings groupSettings;
    NEXUS_RemuxHandle hRmx;
    NEXUS_PidChannelSettings pidChannelOpenSettings;
    NEXUS_PidChannelHandle framePidChnl;
    NEXUS_TsmfSettings tsmfSettings;
    NEXUS_FrontendQamStatus qamStatus;

    NEXUS_StcChannelHandle stcChannel;
    NEXUS_StcChannelSettings stcSettings;
    NEXUS_DisplayHandle display;
    NEXUS_DisplaySettings displaySettings;
    NEXUS_VideoWindowHandle window;
    NEXUS_VideoDecoderHandle videoDecoder;
    NEXUS_AudioDecoderHandle audioDecoder;
    NEXUS_AudioDecoderOpenSettings audioDecoderOpenSettings;
    NEXUS_Error rc;
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
    bool acquired = true;
    unsigned i = 0;
    int curarg = 1;
    NEXUS_RemuxParserBandwidth remuxParserBandwidth;

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
        else if (!strcmp("-tlv_pid",argv[curarg]) && argc>curarg+1) {
             open_settings.tlv_pid = strtol(argv[++curarg],NULL,0);
        }
        else {
            print_usage();
            return 1;
        }
        curarg++;
    }

    NEXUS_Platform_GetDefaultSettings(&platformSettings);

    for( i = 0; i < NEXUS_NUM_PARSER_BANDS; i++ )
    {
        platformSettings.transportModuleSettings.maxDataRate.parserBand[ i ] = 108000000;
    }
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;

    hRmx = NEXUS_Remux_Open(NEXUS_ANY_ID, NULL);
    BDBG_ASSERT( hRmx );
    NEXUS_Remux_GetDefaultParserBandwidth(&remuxParserBandwidth);
    remuxParserBandwidth.maxDataRate = 108000000;
    remuxParserBandwidth.parserBand = NEXUS_ParserBand_e0;
    NEXUS_Remux_SetParserBandwidth(hRmx,&remuxParserBandwidth);

    rc = NEXUS_Remux_Start(hRmx);
    BDBG_ASSERT(!rc);

    hEtbg = NEXUS_Etbg_Open( NULL );
    BDBG_ASSERT( hEtbg );
    NEXUS_PidChannel_GetDefaultSettings(&pidChannelOpenSettings);
    pidChannelOpenSettings.continuityCountEnabled = false;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    for (i=0;i<NUM_BANDS_IN_GROUP;i++)
    {
        NEXUS_FrontendQamSettings qamSettings;
        input[i].frontend = platformConfig.frontend[i];
        BKNI_CreateEvent(&input[i].lockChanged);
        input[i].tsmf = NEXUS_Tsmf_Open( NEXUS_TSMF_INDEX(NEXUS_TsmfType_eBackend, i), NULL );
        BDBG_ASSERT(input[i].tsmf);
        NEXUS_Tsmf_GetSettings( input[i].tsmf, &tsmfSettings );
        tsmfSettings.sourceType = NEXUS_TsmfSourceType_eMtsifRx; /* Stream from MTSIF, parsed on the backend chip */
        tsmfSettings.sourceTypeSettings.mtsif = NEXUS_Frontend_GetConnector( input[i].frontend );
        tsmfSettings.enabled = true;
        tsmfSettings.fieldVerifyConfig.versionChangeMode = NEXUS_TsmfVersionChangeMode_eAllFrame;
        tsmfSettings.relativeTsNum = RELATIVE_TS_NUM;
        tsmfSettings.semiAutomaticMode = false;    /* Go full auto */
        rc = NEXUS_Tsmf_SetSettings( input[i].tsmf, &tsmfSettings );
        BDBG_ASSERT( !rc );

        input[i].parserBand = NEXUS_ParserBand_Open( NEXUS_ANY_ID );
        NEXUS_ParserBand_GetSettings( input[i].parserBand, &parserBandSettings );
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.sourceType = NEXUS_ParserBandSourceType_eTsmf;
        parserBandSettings.sourceTypeSettings.tsmf = input[i].tsmf;
        parserBandSettings.transportType = NEXUS_TransportType_eTs;
        parserBandSettings.teiIgnoreEnabled = true;
        parserBandSettings.acceptAdapt00 = true;
        parserBandSettings.continuityCountEnabled = false;
        parserBandSettings.acceptNullPackets = false;
        rc = NEXUS_ParserBand_SetSettings( input[i].parserBand, &parserBandSettings );
        BDBG_ASSERT( !rc );
        rc = NEXUS_Etbg_AddParserBand(hEtbg, input[i].parserBand, input_config[i].bitRate, input_config[i].numSubFrameSlots);
        BDBG_ASSERT( !rc );
        if (input_config[i].isPrimary)
        {
            framePidChnl = NEXUS_PidChannel_Open( input[i].parserBand, 0x2F, &pidChannelOpenSettings );
            BDBG_ASSERT( framePidChnl );
            rc = NEXUS_Remux_AddPidChannel(hRmx, framePidChnl);
            BDBG_ASSERT( !rc );
            NEXUS_Etbg_GetGroupSettings(hEtbg, &groupSettings);
            groupSettings.primary = input[i].parserBand;
            NEXUS_Etbg_SetGroupSettings(hEtbg, &groupSettings);
        }
        BKNI_ResetEvent( input[i].lockChanged );
        NEXUS_Frontend_GetDefaultQamSettings(&qamSettings);
        qamSettings.frequency = 765 * 1000000;
        qamSettings.mode = NEXUS_FrontendQamMode_e256;
        qamSettings.annex = NEXUS_FrontendQamAnnex_eB;
        qamSettings.bandwidth = NEXUS_FrontendQamBandwidth_e6Mhz;
        qamSettings.lockCallback.callback = lock_changed_callback;
        qamSettings.lockCallback.context = input[i].lockChanged;
        rc = NEXUS_Frontend_TuneQam( input[i].frontend, &qamSettings );
        BDBG_ASSERT( !rc );
        do
        {
            BKNI_WaitForEvent(input[i].lockChanged, BKNI_INFINITE);
            NEXUS_Frontend_GetQamStatus(input[i].frontend, &qamStatus);
            fprintf(stderr, "QAM Lock callback, frontend %p - lock status %d, %d\n", (void*)input[i].frontend,
            qamStatus.fecLock, qamStatus.receiverLock);
        }while ( !qamStatus.fecLock || !qamStatus.receiverLock);
    }
    rc = NEXUS_Etbg_Start(hEtbg);
    BDBG_ASSERT( !rc );
    NEXUS_Etbg_GetGroupSettings(hEtbg, &groupSettings);
    parserBand = groupSettings.primary;
    /**
      * Open display and add outputs
      **/
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

    NEXUS_StcChannel_GetSettings(stcChannel, &stcSettings);
    stcSettings.mode = NEXUS_StcChannelMode_eAuto;
    stcSettings.modeSettings.Auto.behavior = NEXUS_StcChannelAutoModeBehavior_eFirstAvailable;
    rc = NEXUS_StcChannel_SetSettings(stcChannel, &stcSettings);
    if(rc){rc = BERR_TRACE(rc); goto done;}
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
    /*for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i++) ;*/
    for (i=mp_table[0].num_of_assets;(i>0 && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i--) ;
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
    if (hRmx)
    {
       NEXUS_Remux_Stop(hRmx);
       NEXUS_Remux_RemoveAllPidChannels(hRmx);
       NEXUS_Remux_Close(hRmx);
    }
    if (hEtbg) {
       NEXUS_Etbg_Stop(hEtbg);
       NEXUS_Etbg_Close(hEtbg);
    }
    for (i=0;i<NUM_BANDS_IN_GROUP;i++)
    {
          BKNI_DestroyEvent(input[i].lockChanged);
          NEXUS_Tsmf_Close(input[i].tsmf);
    }
    NEXUS_Platform_Uninit();
    return 0;
}

#else  /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
int main(void)
{
    printf("ERROR: This platform doesn't include tsmf.inc \n");
    return -1;
}
#endif /* if NEXUS_HAS_FRONTEND && NEXUS_HAS_VIDEO_DECODER */
