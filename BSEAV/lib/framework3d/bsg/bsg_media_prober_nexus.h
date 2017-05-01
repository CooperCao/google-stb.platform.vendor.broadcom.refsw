/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_MEDIA_PROBE_NEXUS_H__
#define __BSG_MEDIA_PROBE_NEXUS_H__

#include <string>
#include <vector>
#include <map>

#include <stdint.h>

#include "bfile_stdio.h"
#include "bmedia_pcm.h"
#include "bmedia_probe.h"
#include "bmpeg2ts_probe.h"
#include "bmedia_cdxa.h"
#if B_HAS_ASF
#include "basf_probe.h"
#endif
#if B_HAS_AVI
#include "bavi_probe.h"
#endif

#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"

namespace bsg
{
   class MediaData
   {
   public:
      MediaData();

      uint32_t                         m_width;
      uint32_t                         m_height;
      uint32_t                         m_duration;
      uint32_t                         m_videoPid;
      uint32_t                         m_pcrPid;
      uint32_t                         m_audioPid;
      uint32_t                         m_extVideoPid;
      NEXUS_TransportType              m_transportType;
      NEXUS_TransportTimestampType     m_tsTimestampType;
      NEXUS_VideoCodec                 m_videoCodec;
      NEXUS_VideoCodec                 m_extVideoCodec;
      NEXUS_AudioCodec                 m_audioCodec;
      NEXUS_VideoDecoderTimestampMode  m_decoderTimestampMode;
      std::string                      m_filename;
   };

   class MediaProber
   {
      typedef std::map<bstream_mpeg_type, NEXUS_TransportType>    TransportMap;
      typedef std::map<bvideo_codec, NEXUS_VideoCodec>            VideoCodecMap;
      typedef std::map<baudio_format, NEXUS_AudioCodec>           AudioCodecMap;

   private:
      MediaProber();

   public:
      static MediaProber *Instance()
      {
         if (m_instance == NULL)
            m_instance = new MediaProber();
         return m_instance;
      }

      void GetStreamData(const std::string &filename, std::vector<MediaData> &dataList, bool justFirstStream);

   private:
      static MediaProber   *m_instance;

      TransportMap         m_transportMap;
      VideoCodecMap        m_videoCodecMap;
      AudioCodecMap        m_audioCodecMap;
   };

}
#endif // __BSG_MEDIA_PROBE_NEXUS_H__

#endif // BSG_STAND_ALONE
