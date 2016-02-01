/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/
#ifndef BSG_STAND_ALONE

#include "bsg_media_prober_nexus.h"

#include "bsg_exception.h"

namespace bsg
{

   MediaProber *MediaProber::m_instance = nullptr;

   MediaData::MediaData() :
         m_width(0), m_height(0), m_duration(0), m_videoPid(0), m_pcrPid(0), m_audioPid(0),
         m_extVideoPid(0), m_transportType(NEXUS_TransportType_eUnknown),
         m_tsTimestampType(NEXUS_TransportTimestampType_eNone),
         m_videoCodec(NEXUS_VideoCodec_eUnknown), m_extVideoCodec(NEXUS_VideoCodec_eUnknown),
         m_audioCodec(NEXUS_AudioCodec_eUnknown),
         m_decoderTimestampMode(NEXUS_VideoDecoderTimestampMode_eDecode), m_filename("")
   {

   }

   MediaProber::MediaProber()
   {
      m_videoCodecMap[bvideo_codec_none] = NEXUS_VideoCodec_eUnknown;
      m_videoCodecMap[bvideo_codec_unknown] = NEXUS_VideoCodec_eUnknown;
      m_videoCodecMap[bvideo_codec_mpeg1] = NEXUS_VideoCodec_eMpeg1;
      m_videoCodecMap[bvideo_codec_mpeg2] = NEXUS_VideoCodec_eMpeg2;
      m_videoCodecMap[bvideo_codec_mpeg4_part2] = NEXUS_VideoCodec_eMpeg4Part2;
      m_videoCodecMap[bvideo_codec_h263] = NEXUS_VideoCodec_eH263;
      m_videoCodecMap[bvideo_codec_h264] = NEXUS_VideoCodec_eH264;
      m_videoCodecMap[bvideo_codec_h264_svc] = NEXUS_VideoCodec_eH264_Svc;
      m_videoCodecMap[bvideo_codec_h264_mvc] = NEXUS_VideoCodec_eH264_Mvc;
      m_videoCodecMap[bvideo_codec_vc1] = NEXUS_VideoCodec_eVc1;
      m_videoCodecMap[bvideo_codec_vc1_sm] = NEXUS_VideoCodec_eVc1SimpleMain;
      m_videoCodecMap[bvideo_codec_divx_311] = NEXUS_VideoCodec_eDivx311;
      m_videoCodecMap[bvideo_codec_rv40] = NEXUS_VideoCodec_eRv40;
      m_videoCodecMap[bvideo_codec_vp6] = NEXUS_VideoCodec_eVp6;
      m_videoCodecMap[bvideo_codec_vp8] = NEXUS_VideoCodec_eVp8;
      m_videoCodecMap[bvideo_codec_vp9] = NEXUS_VideoCodec_eVp9;
      m_videoCodecMap[bvideo_codec_spark] = NEXUS_VideoCodec_eSpark;
      m_videoCodecMap[bvideo_codec_avs] = NEXUS_VideoCodec_eAvs;
      m_videoCodecMap[bvideo_codec_mjpeg] = NEXUS_VideoCodec_eMotionJpeg;
      m_videoCodecMap[bvideo_codec_h265] = NEXUS_VideoCodec_eH265;

      m_audioCodecMap[baudio_format_unknown] = NEXUS_AudioCodec_eUnknown;
      m_audioCodecMap[baudio_format_mpeg] = NEXUS_AudioCodec_eMpeg;
      m_audioCodecMap[baudio_format_mp3] = NEXUS_AudioCodec_eMp3;
      m_audioCodecMap[baudio_format_aac] = NEXUS_AudioCodec_eAac;
      m_audioCodecMap[baudio_format_aac_plus] = NEXUS_AudioCodec_eAacPlus;
      m_audioCodecMap[baudio_format_aac_plus_adts] = NEXUS_AudioCodec_eAacPlusAdts;
      m_audioCodecMap[baudio_format_aac_plus_loas] = NEXUS_AudioCodec_eAacPlusLoas;
      m_audioCodecMap[baudio_format_ac3] = NEXUS_AudioCodec_eAc3;
      m_audioCodecMap[baudio_format_ac3_plus] = NEXUS_AudioCodec_eAc3Plus;
      m_audioCodecMap[baudio_format_dts] = NEXUS_AudioCodec_eDts;
      m_audioCodecMap[baudio_format_lpcm_hddvd] = NEXUS_AudioCodec_eLpcmHdDvd;
      m_audioCodecMap[baudio_format_lpcm_bluray] = NEXUS_AudioCodec_eLpcmBluRay;
      m_audioCodecMap[baudio_format_dts_hd] = NEXUS_AudioCodec_eDtsHd;
      m_audioCodecMap[baudio_format_wma_std] = NEXUS_AudioCodec_eWmaStd;
      m_audioCodecMap[baudio_format_wma_pro] = NEXUS_AudioCodec_eWmaPro;
      m_audioCodecMap[baudio_format_lpcm_dvd] = NEXUS_AudioCodec_eLpcmDvd;
      m_audioCodecMap[baudio_format_avs] = NEXUS_AudioCodec_eAvs;
      m_audioCodecMap[baudio_format_amr] = NEXUS_AudioCodec_eAmr;
      m_audioCodecMap[baudio_format_dra] = NEXUS_AudioCodec_eDra;
      m_audioCodecMap[baudio_format_cook] = NEXUS_AudioCodec_eCook;
      m_audioCodecMap[baudio_format_pcm] = NEXUS_AudioCodec_ePcmWav;
      m_audioCodecMap[baudio_format_adpcm] = NEXUS_AudioCodec_eAdpcm;
      m_audioCodecMap[baudio_format_dvi_adpcm] = NEXUS_AudioCodec_eAdpcm;
      m_audioCodecMap[baudio_format_vorbis] = NEXUS_AudioCodec_eVorbis;

      m_transportMap[bstream_mpeg_type_unknown] = NEXUS_TransportType_eTs;
      m_transportMap[bstream_mpeg_type_es] = NEXUS_TransportType_eEs;
      m_transportMap[bstream_mpeg_type_bes] = NEXUS_TransportType_eTs;
      m_transportMap[bstream_mpeg_type_pes] = NEXUS_TransportType_eMpeg2Pes;
      m_transportMap[bstream_mpeg_type_ts] = NEXUS_TransportType_eTs;
      m_transportMap[bstream_mpeg_type_dss_es] = NEXUS_TransportType_eDssEs;
      m_transportMap[bstream_mpeg_type_dss_pes] = NEXUS_TransportType_eDssPes;
      m_transportMap[bstream_mpeg_type_vob] = NEXUS_TransportType_eVob;
      m_transportMap[bstream_mpeg_type_asf] = NEXUS_TransportType_eAsf;
      m_transportMap[bstream_mpeg_type_avi] = NEXUS_TransportType_eAvi;
      m_transportMap[bstream_mpeg_type_mpeg1] = NEXUS_TransportType_eMpeg1Ps;
      m_transportMap[bstream_mpeg_type_mp4] = NEXUS_TransportType_eMp4;
      m_transportMap[bstream_mpeg_type_mkv] = NEXUS_TransportType_eMkv;
      m_transportMap[bstream_mpeg_type_wav] = NEXUS_TransportType_eWav;
      m_transportMap[bstream_mpeg_type_mp4_fragment] = NEXUS_TransportType_eMp4Fragment;
      m_transportMap[bstream_mpeg_type_rmff] = NEXUS_TransportType_eRmff;
      m_transportMap[bstream_mpeg_type_flv] = NEXUS_TransportType_eFlv;
      m_transportMap[bstream_mpeg_type_ogg] = NEXUS_TransportType_eOgg;
   }

   void MediaProber::GetStreamData(const std::string &filename, std::vector<MediaData> &dataList, bool justFirstStream)
   {
      bmedia_probe_t probe = nullptr;
      bmedia_probe_config probe_config;
      const bmedia_probe_stream *stream = nullptr;
      const bmedia_probe_track *track = nullptr;
      bfile_io_read_t fd = nullptr;
      bpcm_file_t pcm_file = nullptr;
      bool foundAudio = false, foundVideo = false;
      bool detectAvcExtension = true;
      FILE *fin;
      MediaData data;

      data.m_filename = filename;

      probe = bmedia_probe_create();

      fin = fopen64(filename.c_str(), "rb");

      if (!fin)
         BSG_THROW("Cannot open video file");

      fd = bfile_stdio_read_attach(fin);

      bmedia_probe_default_cfg(&probe_config);

      probe_config.file_name = filename.c_str();
      probe_config.type = bstream_mpeg_type_unknown;
      stream = bmedia_probe_parse(probe, pcm_file ? bpcm_file_get_file_interface( pcm_file ) : fd, &probe_config);

      if (stream && stream->type == bstream_mpeg_type_cdxa)
      {
         bcdxa_file_t cdxa_file;
         cdxa_file = bcdxa_file_create(fd);

         if (cdxa_file)
         {
            const bmedia_probe_stream *cdxa_stream;
            cdxa_stream = bmedia_probe_parse(probe, bcdxa_file_get_file_interface(cdxa_file), &probe_config);
            bcdxa_file_destroy(cdxa_file);

            if (cdxa_stream)
            {
               bmedia_probe_stream_free(probe, stream);
               stream = cdxa_stream;
            }
         }
      }

      if (pcm_file)
         bpcm_file_destroy(pcm_file);

      // Now stream is either nullptr, or stream descriptor with linked list of audio/video tracks
      bfile_stdio_read_detach(fd);

      fclose(fin);

      if (!stream)
         BSG_THROW("Video stream cannot be parsed");

      /*
      char stream_info[1024];
      bmedia_stream_to_string(stream, stream_info, sizeof(stream_info));
      printf("\n\n%s\n\n", stream_info);
      */

      data.m_duration = stream->duration;
      data.m_transportType = m_transportMap[stream->type];

      if (stream->type == bstream_mpeg_type_ts && ((bmpeg2ts_probe_stream*)stream)->pkt_len == 192)
      {
         if (data.m_tsTimestampType == NEXUS_TransportTimestampType_eNone)
            data.m_tsTimestampType = NEXUS_TransportTimestampType_eMod300;
      }

      for (track = BLST_SQ_FIRST(&stream->tracks); track; track = BLST_SQ_NEXT(track, link))
      {
         foundAudio = false;
         foundVideo = false;

         switch (track->type)
         {
         case bmedia_track_type_audio:
            if (track->info.audio.codec != baudio_format_unknown && !foundAudio)
            {
               data.m_audioPid = track->number;
               data.m_audioCodec = m_audioCodecMap[track->info.audio.codec];
               foundAudio = true;
            }
            break;

         case bmedia_track_type_video:
            data.m_width = track->info.video.width;
            data.m_height = track->info.video.height;

            if (track->info.video.codec == bvideo_codec_h264_svc || track->info.video.codec == bvideo_codec_h264_mvc )
            {
               if (detectAvcExtension)
               {
                  data.m_extVideoPid = track->number;
                  data.m_extVideoCodec = m_videoCodecMap[track->info.video.codec];
               }
            }
            else if (track->info.video.codec != bvideo_codec_unknown && !foundVideo)
            {
               data.m_videoPid = track->number;
               data.m_videoCodec = m_videoCodecMap[track->info.video.codec];
               foundVideo = true;

               /* timestamp reordering can be done at the host or decoder.
                  to do it at the decoder, disable it at the host and use media_probe to
                  determine the correct decoder timestamp mode */
               if (false) //playpumpTimestampReordering == false)
                  data.m_decoderTimestampMode = (NEXUS_VideoDecoderTimestampMode)track->info.video.timestamp_order;
            }

            break;

         case bmedia_track_type_pcr:
            data.m_pcrPid = track->number;
            break;

         default:
            break;
         }

         if (foundVideo)
         {
            dataList.push_back(data);
            if (justFirstStream)
               break;
         }
      }

      if (probe)
      {
         if (stream)
            bmedia_probe_stream_free(probe, stream);

         bmedia_probe_destroy(probe);
      }
   }
}

#endif // BSG_STAND_ALONE
