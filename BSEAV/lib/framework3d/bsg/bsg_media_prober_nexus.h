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
         if (m_instance == nullptr)
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
