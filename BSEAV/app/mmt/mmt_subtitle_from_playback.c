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
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#else
#include "nexus_platform.h"
#endif
#include "nexus_message.h"
#include "bmmt.h"
BDBG_MODULE(mmt_subtitle_from_playback);

/**
  * mmt_subtitle_from_playback extracts the subtitle data from
  * MMT in MPEG2 PES format. Output file is choosen to be .xml
  * so that users to open them as xml files. output file =
  * subtitle data bytes from table 9-1 in ARIB STB-B60.1.5.
  * Only one input stream is tested i.e.
  * Capture_20170221182645_20170221183215-bsc.tlv which has no
  * timing info.
  **/

#define MAX_PES_SIZE 32768*5

typedef struct _subtitle_info
{
   uint8_t tag;
   uint8_t sequence_number;
   uint8_t subsample_number;
   uint8_t last_subsample_number;
   uint8_t data_type;
   uint32_t data_size;
   bool length_extension_flag;
   bool info_list_flag;
}subtitle_info;

typedef struct _pes_packet_info
{
   uint16_t header_len;
   uint16_t payload_len;
   uint8_t flags;
   uint64_t pts;
   uint64_t dts;
   uint8_t stream_id;
   subtitle_info subtitle;
}pes_packet_info;


static void
usage(const char *name, const char *opt)
{
    if (opt) {
        printf("Unknown option %s\n", opt);
    }
    printf(
           "%s: MMT tool\n"
           "Usage: %s [options] <input> <pes_out>\n"
           "where options:\n",
           name, name);
    printf("-help - this help\n"
           "-input_format - MPEG2TS-1 TLV-2 \n"
           " tlv_pid - valid if input_format=1 \n");
    return;
}
#define MAX_PACKAGES 8

void message_callback(void *context, int param)
{
    BSTD_UNUSED(param);
    BKNI_SetEvent((BKNI_EventHandle)context);
}

int main(int argc, const char *argv[])
{
    int arg;
    int file_arg=0;
    bmmt_t mmt = NULL;
    bmmt_open_settings open_settings;
    bmmt_stream_settings subtitle_stream_settings;
    bmmt_stream_t subtitle_stream;
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
    NEXUS_PidChannelHandle subtitle_ch;
#ifdef NXCLIENT_SUPPORT
    NxClient_JoinSettings joinSettings;
#else
    NEXUS_PlatformSettings platformSettings;
    NEXUS_PlatformConfiguration platformConfig;
#endif
    NEXUS_MessageHandle msgHandle;
    NEXUS_MessageSettings msgSettings;
    NEXUS_MessageStartSettings msgStartSettings;
    BKNI_EventHandle msgEvent;
    NEXUS_Error rc;
    unsigned i;
    unsigned msg_count = 5;
    FILE *subtitle_pes_file = NULL;
    FILE *subtitle_xml_file = NULL;
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
        }else if (!strcmp("-loop",argv[arg])) {
           open_settings.loop = true;
        }
        else if (*argv[arg]!='\0' && (*argv[arg]!='-' || argv[arg][1]=='\0'))  {
            switch(file_arg) {
            case 0: strcpy(open_settings.fileName,argv[arg]);
                break;
            case 1: strcpy(open_settings.fileOut,argv[arg]);
                open_settings.pesOut = true;
                break;
            default:
                usage(argv[0], argv[arg]);
                return -1;
            }
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
#ifdef NXCLIENT_SUPPORT
    NxClient_GetDefaultJoinSettings(&joinSettings);
    snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", argv[0]);
    rc = NxClient_Join(&joinSettings);
    if (rc) return -1;
#else
    NEXUS_Platform_GetDefaultSettings(&platformSettings);
    platformSettings.openFrontend = false;
    rc = NEXUS_Platform_Init(&platformSettings);
    if (rc) return -1;
    NEXUS_Platform_GetConfiguration(&platformConfig);
#endif
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

    /**
      *  find subtitle asset index in the 1st MPT
     **/
    for (i=0;(i<mp_table[0].num_of_assets && strcmp((char *)mp_table[0].assets[i].type,"stpp"));i++);
    /**
     *  open subtitle stream context
     **/
    if (i!=mp_table[0].num_of_assets)
    {
        /**
          *  open subtitle stream context for the subtitle packet ID in
          *  the 1st asset of MPT
         **/
        bmmt_stream_get_default_settings(&subtitle_stream_settings);
        switch (mp_table[0].assets[i].location_info[0].location_type)
        {
        case bmmt_general_location_type_id:
            subtitle_stream_settings.pid = mp_table[0].assets[i].id[0] << 8 | mp_table[0].assets[i].id[1];
            break;
        case bmmt_general_location_type_ipv4:
            subtitle_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv4.packet_id;
            break;
        case bmmt_general_location_type_ipv6:
             subtitle_stream_settings.pid = mp_table[0].assets[i].location_info[0].data.mmt_ipv6.packet_id;
             break;
        default:
             BDBG_WRN(("subtitle stream location ID not supported"));
             goto done;
        }

        if (subtitle_stream_settings.pid )
        {
            subtitle_stream_settings.stream_type = bmmt_stream_type_subtitle;
            BDBG_WRN(("mp_table[0].assets[i].type %s",mp_table[0].assets[i].type));
            BDBG_WRN(("subtitle_stream_settings.pid %04x",subtitle_stream_settings.pid));
            subtitle_stream = bmmt_stream_open(mmt,&subtitle_stream_settings);
            BDBG_ASSERT(subtitle_stream);
            subtitle_ch = bmmt_stream_get_pid_channel(subtitle_stream);
            BKNI_CreateEvent(&msgEvent);
            NEXUS_Message_GetDefaultSettings(&msgSettings);
            msgSettings.dataReady.callback = message_callback;
            msgSettings.dataReady.context = msgEvent;
            msgSettings.bufferSize = 64 * 1024;
            msgHandle = NEXUS_Message_Open(&msgSettings);
            BDBG_ASSERT(msgHandle);
            NEXUS_Message_GetDefaultStartSettings(msgHandle, &msgStartSettings);
            msgStartSettings.pidChannel = subtitle_ch;
            msgStartSettings.format = NEXUS_MessageFormat_ePes;
            rc = NEXUS_Message_Start(msgHandle, &msgStartSettings);
            BDBG_ASSERT(!rc);
        }
        else
        {
            BDBG_WRN(("sub title stream location ID not supported"));
        }
    }
    else
    {
        BDBG_WRN(("no subtitle asset was found"));
    }

    subtitle_pes_file = fopen("subtitle.pes", "wb");
    BDBG_ASSERT((subtitle_pes_file));
    printf("\n subtitle download start\n");
    while (1) {
        const uint8_t *buffer;
        size_t size,n;
        unsigned i;

        rc = NEXUS_Message_GetBuffer(msgHandle, (const void **)&buffer, &size);
        BDBG_ASSERT(!rc);
        if (!size) {
            BERR_Code rc = BKNI_WaitForEvent(msgEvent, 5 * 1000); /* wait 5 seconds */
            if (rc) {BDBG_ERR(("no data")); rc = -1; break;}
            continue;
        }

        n = fwrite(buffer, 1, size, subtitle_pes_file);
        if (n < 0) {BDBG_ERR(("fwrite error")); break;}
        printf("##");
        msg_count--;
        rc = NEXUS_Message_ReadComplete(msgHandle, size);
        BDBG_ASSERT(!rc);
    }
    printf("\n subtitle download end \n");
    bmmt_stop(mmt);
    fflush(subtitle_pes_file);
    fclose(subtitle_pes_file);
    {
        uint8_t *buffer = NULL;
        uint32_t pes_4bytes;
        size_t  n;
        pes_packet_info pes_packet;
        buffer = ( uint8_t *)malloc(MAX_PES_SIZE);
        BDBG_ASSERT((buffer));
        subtitle_pes_file = fopen("subtitle.pes", "rb");
        subtitle_xml_file = fopen("subtitle.xml", "wb");
        n = fread(&pes_4bytes, sizeof(pes_4bytes), 1, subtitle_pes_file);
        while (n) {
           unsigned pes_packet_size = 0;
           uint8_t *tmp_buffer = NULL;
           unsigned j = 0;
           BDBG_MSG(("pes_4bytes : %x",pes_4bytes));
           buffer[0] = 0x0;
           buffer[1] = 0x0;
           buffer[2] = 0x1;
           buffer[3] = 0xbd;
           do {
              pes_4bytes = (pes_4bytes >> 8) & 0x00ffffff;
              n = fread(&buffer[pes_packet_size+4], 1, 1, subtitle_pes_file);
              if (n < 1) break;
              pes_4bytes |= buffer[pes_packet_size+4] << 24;
              pes_packet_size++;
           }while (pes_4bytes!=0xbd010000 && (pes_packet_size+4) < MAX_PES_SIZE);

           pes_packet.payload_len = buffer[4] << 8 | buffer[5];
           pes_packet.payload_len +=6;
           pes_packet.flags = buffer[6];
           pes_packet.header_len = buffer[8];
           pes_packet.header_len +=9;
           if (buffer[7] && 0x80) {
              pes_packet.pts = (buffer[9] & 0x0e);
              pes_packet.pts <<= 29;
              pes_packet.pts |= (buffer[10] << 22);
              pes_packet.pts |= ((buffer[11] & 0xfe) << 14);
              pes_packet.pts |= (buffer[12] << 7);
              pes_packet.pts |= ((buffer[13] & 0xfe) >> 1);
              j=14;
              if (buffer[7] && 0x40) {
                 pes_packet.dts = (buffer[14] & 0x0e);
                 pes_packet.dts <<= 29;
                 pes_packet.dts |= (buffer[15] << 22);
                 pes_packet.dts |= ((buffer[16] & 0xfe) << 14);
                 pes_packet.dts |= (buffer[17] << 7);
                 pes_packet.dts |= ((buffer[18] & 0xfe) >> 1);
              }
              j+=5;
           }
           else
           {
              j=9;
           }
           pes_packet.subtitle.tag = buffer[j++];
           pes_packet.subtitle.sequence_number = buffer[j++];
           pes_packet.subtitle.subsample_number = buffer[j++];
           pes_packet.subtitle.last_subsample_number = buffer[j++];
           pes_packet.subtitle.data_type = buffer[j++];
           pes_packet.subtitle.length_extension_flag = ( pes_packet.subtitle.data_type & 0x08)? true:false;
           pes_packet.subtitle.info_list_flag = ( pes_packet.subtitle.data_type & 0x04)?true:false;
            pes_packet.subtitle.data_type = (0xf0 &  pes_packet.subtitle.data_type) >> 4;
           if (pes_packet.subtitle.length_extension_flag) {
              pes_packet.subtitle.data_size = buffer[j++] << 24 | buffer[j++] << 16 | buffer[j++] << 8 | buffer[j++];
           }
           else{
              pes_packet.subtitle.data_size = buffer[j++] << 8 | buffer[j++];
           }

           if(( pes_packet.subtitle.subsample_number == 0) &&
              ( pes_packet.subtitle.last_subsample_number > 0) &&
              (pes_packet.subtitle.info_list_flag))
           {
              int idx;
              for( idx=0; idx<pes_packet.subtitle.last_subsample_number; idx++ )
              {
                 j++;
                 if( pes_packet.subtitle.length_extension_flag )
                 {
                    j+=4;
                 }
                 else
                 {
                    j+=2;
                 }
              }
           }
           BDBG_ASSERT((j<pes_packet_size));
           BDBG_MSG(("j %x  pes_packet_size-j : %x",j,pes_packet_size-j));
           tmp_buffer = &buffer[j];
           fwrite(tmp_buffer, 1, pes_packet.subtitle.data_size, subtitle_xml_file);
           pes_packet_size = 0;
        }
        fclose(subtitle_pes_file);
        fclose(subtitle_xml_file);
     }

    BDBG_WRN(("press enter to quit"));
    getchar();
    NEXUS_Message_Stop(msgHandle);
    NEXUS_Message_Close(msgHandle);
    BKNI_DestroyEvent(msgEvent);
done:
    bmmt_close(mmt);
#ifdef NXCLIENT_SUPPORT
    NxClient_Uninit();
#else
    NEXUS_Platform_Uninit();
#endif

    return 0;
}
