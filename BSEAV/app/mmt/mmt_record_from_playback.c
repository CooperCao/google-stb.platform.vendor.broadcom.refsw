/***************************************************************************
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
 **************************************************************************/

#include "nexus_platform.h"
#include "nexus_recpump.h"
#if NEXUS_HAS_RECORD
#include "nexus_record.h"
#endif
#include "bmmt.h"

BDBG_MODULE(mmt_record_from_playback);

static void
usage(const char *name, const char *opt)
{
    if (opt) {
        printf("Unknown option %s\n", opt);
    }
    printf(
           "%s: MMT tool\n"
           "Usage: %s [options] <input> \n"
           "where options:\n",
           name, name);
    printf("-help - this help\n"
           "-input_format - MPEG2TS-1 TLV-2 \n"
           " tlv_pid - valid if input_format=1 \n");
    return;
}
#define MAX_PACKAGES 8
int main(int argc, const char *argv[])
{
    int arg;
    int file_arg=0;
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
    NEXUS_PidChannelHandle video_ch;
    NEXUS_PidChannelHandle audio_ch;
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
    NEXUS_Error rc;
    unsigned i;
    NEXUS_FileRecordHandle file;
    NEXUS_RecpumpHandle recpump;
    NEXUS_RecordHandle record;
    NEXUS_RecordSettings recordSettings;
    const char *fname = "stream.mpg";
    const char *index = "stream.nav";

    bmmt_get_default_open_settings(&open_settings);
    /**
     * process input parameters
    **/
    arg=1;
    while (argc>arg) {
        if (!strcmp("-help",argv[arg])) {
            usage(argv[0], NULL);
            return 0;;
        } else if (!strcmp("-input_format",argv[arg]) && argc>arg+1) {
            arg++;
            open_settings.input_format = strtol(argv[arg],NULL,0)-1;
        } else if (!strcmp("-tlv_pid",argv[arg]) && argc>arg+1) {
            arg++;
            open_settings.tlv_pid = strtol(argv[arg],NULL,0);
        }else if (*argv[arg]!='\0' && (*argv[arg]!='-' || argv[arg][1]=='\0'))  {
            strcpy(open_settings.fileName,argv[arg]);
            file_arg++;
        } else {
            usage(argv[0], argv[arg]);
            return -1;
        }
        arg++;
    }
    BDBG_WRN(("input_format %d tlv_pid %x",open_settings.input_format, open_settings.tlv_pid));
    if (!file_arg || open_settings.input_format == ebmmt_input_format_max) {
        usage(argv[0], NULL);
        return -1;
    }
    if(open_settings.input_format == ebmmt_input_format_mpeg2ts && open_settings.tlv_pid==0) {
        usage(argv[0], NULL);
        return -1;
    }
    /**
     * nexus platform initialization
    **/
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);
    /**
     *  mmt module instantiation
    **/
    open_settings.playback = true;
    mmt =  bmmt_open(&open_settings);
    BDBG_ASSERT(mmt);
    /**
     *  open TLV AMT message context
    **/
    bmmt_msg_get_default_settings(&msg_settings);
    msg_settings.msg_type = ebmmt_msg_type_tlv;
    amt_msg = bmmt_msg_open(mmt,&msg_settings);
    /**
     *  start the mmt module
    **/
    bmmt_start(mmt);
    /**
     *  extract service ids from input stream and their network
     *  addresses from TLV SI AMT
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
     *   close TLV msg context
    **/
    bmmt_msg_close(amt_msg);
    if (msg_r == BMMT_MAX_MSG_BUFFERS)
    {
        BDBG_ERR(("TLV SI AMT not found"));
        goto done;
    }
    /**
     *   set IP filtering for the TLV packets
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
       goto done;
    }
    bmmt_set_ip_filter(mmt, &ip_addr);
    /**
     *   open PLT message context
    **/
    bmmt_msg_get_default_settings(&msg_settings);
    msg_settings.msg_type = ebmmt_msg_type_mmt;
    msg_settings.pid = 0x0;
    plt_msg = bmmt_msg_open(mmt,&msg_settings);
    /**
     *   extract PLT from PA message
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
     *   close plt message context
    **/
    bmmt_msg_close(plt_msg);
    if (msg_r == BMMT_MAX_MSG_BUFFERS)
    {
        BDBG_ERR(("MMT SI PLT not found"));
        goto done;
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
                 goto done;
              }
              /**
                *   open MPT message context
               **/
              mpt_msg = bmmt_msg_open(mmt,&msg_settings);
              /**
                *   extract MPT from PA message
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
                  goto done;
              }
              /*bmmt_msg_close(mpt_msg);*/
          }

       }
       else
       {
          BDBG_WRN(("no packages found in the PLT"));
          goto done;
       }
    }

    recpump = NEXUS_Recpump_Open(NEXUS_ANY_ID, NULL);

    record = NEXUS_Record_Create();

    NEXUS_Record_GetSettings(record, &recordSettings);
    recordSettings.recpump = recpump;
    NEXUS_Record_SetSettings(record, &recordSettings);

    file = NEXUS_FileRecord_OpenPosix(fname,NULL);

    /**
      *  find video asset index in the 1st MPT
     **/
    for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hev1"));i++);

    /**
      *  some sample streams have video string as hvc1
     **/
    if (i==mp_table[0].num_of_assets) {
         for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"hvc1"));i++);
    }
    /**
     *  open video stream context
     **/
    if (i!=mp_table[0].num_of_assets)
    {
        /**
          *  open video stream context for the video packet ID in the
          *  1st asset of MPT
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
             goto done;
        }

        if (video_stream_settings.pid )
        {
            video_stream_settings.stream_type = bmmt_stream_type_h265;
            BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
            BDBG_WRN(("video_stream_settings.pid %04x",video_stream_settings.pid));
            video_stream = bmmt_stream_open(mmt,&video_stream_settings);
            BDBG_ASSERT(video_stream);
            video_ch = bmmt_stream_get_pid_channel(video_stream);
            rc = NEXUS_Record_AddPidChannel(record,video_ch, NULL);

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
      *  find audio asset index in the 1st MPT
     **/
    for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"mp4a"));i++) ;
    /**
     *  open audio stream context
    **/
    if (i!=mp_table[0].num_of_assets)
    {
        /**
          *  open video stream context for the video packet ID in the
          *  1st asset of MPT
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
             BDBG_WRN(("video stream location ID not supported"));
             goto done;
        }

        if (audio_stream_settings.pid )
        {
            audio_stream_settings.stream_type = bmmt_stream_type_aac;
            audio_stream = bmmt_stream_open(mmt,&audio_stream_settings);
            BDBG_ASSERT(audio_stream);
            audio_ch = bmmt_stream_get_pid_channel(audio_stream);
            NEXUS_Record_AddPidChannel(record,audio_ch, NULL);

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

    NEXUS_Record_Start(record, file);
    BDBG_WRN(("press enter to quit"));
    getchar();
    NEXUS_Record_Stop(record);
    NEXUS_Record_RemoveAllPidChannels(record);
    NEXUS_FileRecord_Close(file);
    NEXUS_Record_Destroy(record);
    NEXUS_Recpump_Close(recpump);
done:
    bmmt_stop(mmt);
    bmmt_close(mmt);
    NEXUS_Platform_Uninit();

    return 0;
}
