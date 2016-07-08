/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
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

// Nexus or Magnum should really define this before using UINT32_C, but don't currently
#define __STDC_CONSTANT_MACROS

#include "bsg_video.h"
#include "bsg_exception.h"
#include "bsg_platform_nexus.h"
#include "bsg_application.h"

#include "bsg_media_prober_nexus.h"

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_stc_channel.h"
#include "nexus_file.h"
#include "nexus_base_types.h"
#include "nexus_core_utils.h"

#ifndef SINGLE_PROCESS
#include "nxclient.h"
#endif


namespace bsg
{
   static void complete(void *data, int unused)
   {
      BSTD_UNUSED(unused);
      BKNI_SetEvent((BKNI_EventHandle)data);
   }

   VideoPrivate::~VideoPrivate()
   {
   }

   class NexusVideoStream
   {
   public:
      enum eVideoMode
      {
         eSINGLE,
         eMOSAIC
      };

   public:
      NexusVideoStream() :
         m_videoDecoder(NULL),
         m_filePlay(NULL),
         m_playPump(NULL),
         m_playBack(NULL),
         m_stcChannel(NULL),
         m_videoPidChannel(NULL),
         m_nexusDisplay(NULL),
         m_sourceWidth(0),
         m_sourceHeight(0),
         m_outputWidth(0),
         m_outputHeight(0),
         m_index(0)
      {
#ifndef SINGLE_PROCESS
         m_connectId = 0;
         for (unsigned int i = 0; i < NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
         {
            m_videoSurfaces[i] = NULL;
         }
#endif
      }

      void Configure(eVideoMode mode, uint32_t indx, const MediaData &mediaData, NEXUS_DisplayHandle nexusDisplay)
      {
         try
         {
            NEXUS_StcChannelSettings               stcSettings;
            NEXUS_PlaybackSettings                 playbackSettings;
            NEXUS_PlaybackPidChannelSettings       pidChannelSettings;
#ifdef SINGLE_PROCESS
            NEXUS_VideoDecoderOpenMosaicSettings   videoDecoderOpenMosaicSettings;
#else
            NEXUS_Error rc;
#endif

            m_index = indx;
            m_nexusDisplay = nexusDisplay;

            Cleanup();

            m_sourceWidth  = mediaData.m_width;
            m_sourceHeight = mediaData.m_height;
            m_outputWidth  = m_sourceWidth;
            m_outputHeight = m_sourceHeight;

            m_playPump = NEXUS_Playpump_Open(m_index, NULL);
            if (m_playPump == NULL)
               BSG_THROW("NEXUS_Playpump_Open failed");

            m_playBack = NEXUS_Playback_Create();
            if (m_playBack == NULL)
               BSG_THROW("NEXUS_Playback_Create failed");

            /* open stream file */
            /* 2nd argument is the index */
            m_filePlay = NEXUS_FilePlay_OpenPosix(mediaData.m_filename.c_str(), mediaData.m_filename.c_str());
            if (m_filePlay == NULL)
               BSG_THROW("Failed to open video file");

#ifdef SINGLE_PROCESS
            /* bring up video decoder */
            if (mode == eMOSAIC)
            {
               NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&videoDecoderOpenMosaicSettings);
               videoDecoderOpenMosaicSettings.maxWidth = m_sourceWidth;
               videoDecoderOpenMosaicSettings.maxHeight = m_sourceHeight;
               m_videoDecoder = NEXUS_VideoDecoder_OpenMosaic(0, m_index, &videoDecoderOpenMosaicSettings);
               if (m_videoDecoder == NULL)
                  BSG_THROW("NEXUS_VideoDecoder_OpenMosaic failed");
            }
            else
            {
               m_videoDecoder = NEXUS_VideoDecoder_Open(0, NULL); /* take default capabilities */
               if (m_videoDecoder == NULL)
                  BSG_THROW("NEXUS_VideoDecoder_Open failed");
            }
#else
            if (mode == eMOSAIC)
               BSG_THROW("Mosaic mode not supported in multiprocess");

            /*Allocate the number of video streams / decoders*/
            NxClient_AllocSettings allocSettings;
            NxClient_GetDefaultAllocSettings(&allocSettings);
            allocSettings.simpleVideoDecoder = 1;

            rc = NxClient_Alloc(&allocSettings, &m_allocResults);
            if (rc != NEXUS_SUCCESS)
               BSG_THROW("Nxclient simple decoder allocation failed");

            if (m_allocResults.simpleVideoDecoder[0].id)
               m_videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(m_allocResults.simpleVideoDecoder[0].id);

            if (m_videoDecoder == NULL)
               BSG_THROW("Acquire Nxclient decoder failed");
#endif

            NEXUS_StcChannel_GetDefaultSettings(m_index, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            m_stcChannel = NEXUS_StcChannel_Open(m_index, &stcSettings);
            if (m_stcChannel == NULL)
               BSG_THROW("NEXUS_StcChannel_Open failed");

            /* setup playback */
            NEXUS_Playback_GetSettings(m_playBack, &playbackSettings);
            playbackSettings.playpump = m_playPump;
            playbackSettings.playpumpSettings.transportType = mediaData.m_transportType;
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            playbackSettings.stcChannel = m_stcChannel;
            NEXUS_Playback_SetSettings(m_playBack, &playbackSettings);

            /* video */
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pidChannelSettings.pidTypeSettings.video.codec = mediaData.m_videoCodec;
#ifdef SINGLE_PROCESS
            pidChannelSettings.pidTypeSettings.video.decoder       = m_videoDecoder;
#else
            pidChannelSettings.pidTypeSettings.video.simpleDecoder = m_videoDecoder;
#endif
            pidChannelSettings.pidTypeSettings.video.index = true;
            m_videoPidChannel = NEXUS_Playback_OpenPidChannel(m_playBack, mediaData.m_videoPid, &pidChannelSettings);
            if (m_videoPidChannel == NULL)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed");

#ifdef SINGLE_PROCESS
            NEXUS_VideoDecoderStartSettings        videoProgram;

            NEXUS_VideoDecoder_GetDefaultStartSettings(&videoProgram);
            videoProgram.codec          = mediaData.m_videoCodec;
            videoProgram.pidChannel     = m_videoPidChannel;
            videoProgram.stcChannel     = m_stcChannel;

            /* Tell Display module to connect to the VideoDecoder module and supply the
            L1 INT id's from BVDC_Display_GetInterrupt. Display will not register for the data ready ISR callback. */
            NEXUS_Display_ConnectVideoInput(m_nexusDisplay, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));

            /* Start decode */
            NEXUS_VideoDecoder_Start(m_videoDecoder, &videoProgram);
#else
            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&m_videoProgram);
            m_videoProgram.settings.codec      = mediaData.m_videoCodec;
            m_videoProgram.settings.pidChannel = m_videoPidChannel;
            m_videoProgram.settings.stcChannel = m_stcChannel;
            m_videoProgram.displayEnabled      = false;

            /* connect client resources to server's resources */
            NxClient_ConnectSettings            connectSettings;
            NxClient_GetDefaultConnectSettings(&connectSettings);

            if (m_videoProgram.settings.pidChannel)
            {
               connectSettings.simpleVideoDecoder[0].id = m_allocResults.simpleVideoDecoder[0].id;
               connectSettings.simpleVideoDecoder[0].windowId = 0;
               connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = m_sourceWidth;
               connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = m_sourceHeight;
            }

            rc = NxClient_Connect(&connectSettings, &m_connectId);
            if (rc != NEXUS_SUCCESS)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed");

            /* Need to create surfaces for the capture*/
            NEXUS_SurfaceCreateSettings createSettings;
            NEXUS_Surface_GetDefaultCreateSettings(&createSettings);

            createSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
            createSettings.width       = m_outputWidth;
            createSettings.height      = m_outputHeight;

            NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&m_videoCaptureSettings);
            m_videoCaptureSettings.displayEnabled = false;

            for (unsigned index=0; index < NEXUS_SIMPLE_DECODER_MAX_SURFACES; index++)
            {
               m_videoSurfaces[index] = NEXUS_Surface_Create(&createSettings);
               m_videoCaptureSettings.surface[index] = m_videoSurfaces[index];
            }

            /* Start decode */
            NEXUS_SimpleVideoDecoder_Start(m_videoDecoder, &m_videoProgram);
            NEXUS_SimpleVideoDecoder_StartCapture(m_videoDecoder, &m_videoCaptureSettings);

#endif

            /* start playback */
            NEXUS_Playback_Start(m_playBack, m_filePlay, NULL);
         }
         catch (Exception e)
         {
            Cleanup();
            throw e;
         }
      }

      void SetOutputs(uint32_t width, uint32_t height, eVideoFrameFormat)
      {
         m_outputWidth = width;
         m_outputHeight = height;

#if 0
        // NOTE:
        // Multiprocess is not optimal at the moment; we really want to cut out some
        // of the intermediate surfaces.  At the moment the video goes from
        // the striped format to RGBA8888, then blitted to the native pixmap
        // then tf converted.  The blit should not be necessary, but the architecture
        // of the software doesn't make it easy to avoid.
        // This resize causes a glitch in the display.  We really need to choose the
        // size before we configure the video.
#ifndef SINGLE_PROCESS

         NEXUS_PixelFormat pixmapFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;

         switch (format)
         {
         case eYUV422   : pixmapFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8; break;
         case eYUVX444  : pixmapFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;     break;
         case eRGB565   : pixmapFormat = NEXUS_PixelFormat_eR5_G6_B5;        break;
         case eRGBX8888 : pixmapFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;     break;
         default        : BSG_THROW("Video format not supported");           break;
         }

         /* Stop and start the decoder to change the settings */

         /*Destroy the surfaces used by the decoder for the captures*/
         for (unsigned int i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
         {
            if (m_videoSurfaces[i])
            {
               NEXUS_Surface_Destroy(m_videoSurfaces[i]);
               m_videoSurfaces[i] = NULL;
            }
         }

         if (m_playBack)
            NEXUS_Playback_Stop(m_playBack);

         if (m_videoDecoder)
         {
            NEXUS_SimpleVideoDecoder_StopCapture(m_videoDecoder);
            NEXUS_SimpleVideoDecoder_Stop(m_videoDecoder);
         }

         /* Need to create surfaces for the capture*/
         NEXUS_SurfaceCreateSettings createSettings;
         NEXUS_Surface_GetDefaultCreateSettings(&createSettings);
         createSettings.pixelFormat = pixmapFormat;
         createSettings.width = m_outputWidth;
         createSettings.height = m_outputHeight;
         for (unsigned index=0; index < NEXUS_SIMPLE_DECODER_MAX_SURFACES; index++)
         {
            m_videoSurfaces[index] = NEXUS_Surface_Create(&createSettings);
         }
         BKNI_Memcpy(m_videoCaptureSettings.surface, m_videoSurfaces, sizeof(m_videoSurfaces));
         NEXUS_SimpleVideoDecoder_StartCapture(m_videoDecoder, &m_videoCaptureSettings);

         /* Start decode */
         NEXUS_SimpleVideoDecoder_Start(m_videoDecoder, &m_videoProgram);

         /* start playback */
         NEXUS_Playback_Start(m_playBack, m_filePlay, NULL);
#endif
#endif
      }

      ~NexusVideoStream()
      {
         Cleanup();
      }

      void Cleanup()
      {
         if (m_playBack)
            NEXUS_Playback_Stop(m_playBack);

         if (m_videoDecoder)
         {
#ifdef SINGLE_PROCESS
            NEXUS_VideoDecoder_Stop(m_videoDecoder);
#else
            NEXUS_SimpleVideoDecoder_StopCapture(m_videoDecoder);
            NEXUS_SimpleVideoDecoder_Stop(m_videoDecoder);

            /* Destroy the surfaces used by the decoder for the captures */
            for (unsigned int i=0; i<NEXUS_SIMPLE_DECODER_MAX_SURFACES; i++)
            {
               if (m_videoSurfaces[i])
               {
                  NEXUS_Surface_Destroy(m_videoSurfaces[i]);
                  m_videoSurfaces[i] = NULL;
               }
            }
#endif
         }

         if (m_videoPidChannel)
         {
            NEXUS_Playback_ClosePidChannel(m_playBack, m_videoPidChannel);
            m_videoPidChannel = NULL;
         }

         if (m_stcChannel)
         {
            NEXUS_StcChannel_Close(m_stcChannel);
            m_stcChannel = NULL;
         }

#ifdef SINGLE_PROCESS
         if (m_videoDecoder)
            NEXUS_Display_DisconnectVideoInput(m_nexusDisplay, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));

         if (m_videoDecoder)
         {
            NEXUS_VideoDecoder_Close(m_videoDecoder);
            m_videoDecoder = NULL;
         }
#else
         if (m_connectId)
               NxClient_Disconnect(m_connectId);

         if (m_videoDecoder)
         {
            NEXUS_SimpleVideoDecoder_Release(m_videoDecoder);
            m_videoDecoder = NULL;
         }

         NxClient_Free(&m_allocResults);
#endif

         if (m_filePlay)
         {
            NEXUS_FilePlay_Close(m_filePlay);
            m_filePlay = NULL;
         }

         if (m_playBack)
         {
            NEXUS_Playback_Destroy(m_playBack);
            m_playBack = NULL;
         }

         if (m_playPump)
         {
            NEXUS_Playpump_Close(m_playPump);
            m_playPump = NULL;
         }
      }

#ifdef SINGLE_PROCESS
      NEXUS_VideoDecoderHandle                     m_videoDecoder;
#else
      NEXUS_SimpleVideoDecoderHandle               m_videoDecoder;
      unsigned                                     m_connectId;
      NEXUS_SimpleVideoDecoderStartSettings        m_videoProgram;
      NEXUS_SimpleVideoDecoderStartCaptureSettings m_videoCaptureSettings;
      NEXUS_SurfaceHandle                          m_videoSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
      NxClient_AllocResults                        m_allocResults;
#endif
      NEXUS_FilePlayHandle       m_filePlay;
      NEXUS_PlaypumpHandle       m_playPump;
      NEXUS_PlaybackHandle       m_playBack;
      NEXUS_StcChannelHandle     m_stcChannel;
      NEXUS_PidChannelHandle     m_videoPidChannel;
      NEXUS_DisplayHandle        m_nexusDisplay;

      uint32_t                   m_sourceWidth;
      uint32_t                   m_sourceHeight;
      uint32_t                   m_outputWidth;
      uint32_t                   m_outputHeight;
      uint32_t                   m_index;
   };

   class NexusVideoPrivate : public VideoPrivate
   {
   public:
      NexusVideoPrivate(const std::string &videoFileName, NEXUS_DisplayHandle nexusDisplay) :
         m_gfxDone(NULL),
         m_gfx2d(NULL),
         m_numStreams(0)
#ifdef SINGLE_PROCESS
         ,m_stripedSurfaces(NULL)
#endif
      {

         std::vector<std::string> names;
         names.push_back(videoFileName);

         Init(names, nexusDisplay);
      }
#ifdef SINGLE_PROCESS
      NexusVideoPrivate(const std::vector<std::string> &videoFileNames, NEXUS_DisplayHandle nexusDisplay) :
         m_gfxDone(NULL),
         m_gfx2d(NULL),
         m_numStreams(0),
         m_stripedSurfaces(NULL)
      {
         Init(videoFileNames, nexusDisplay);
      }
#endif

      void Init(const std::vector<std::string> &videoFileNames, NEXUS_DisplayHandle nexusDisplay)
      {
         MediaProber                   *prober = MediaProber::Instance();
         std::vector<MediaData>        mediaDataList;
         uint32_t                      s;
         NexusVideoStream::eVideoMode  mode = NexusVideoStream::eMOSAIC;

         // When using multiple pids from a single video file, the video is much choppier than when
         // using multiple files for some reason. So, we will disable the extraction of multiple streams.
         bool                          justFirstStream = true;

         for (s = 0; s < videoFileNames.size(); s++)
            prober->GetStreamData(videoFileNames[s], mediaDataList, justFirstStream);

         m_numStreams = mediaDataList.size();
         m_streams = new NexusVideoStream[m_numStreams];

         if (m_numStreams == 1)
            mode = NexusVideoStream::eSINGLE;

         for (s = 0; s < m_numStreams; s++)
            m_streams[s].Configure(mode, s, mediaDataList[s], nexusDisplay);

#ifdef SINGLE_PROCESS
         m_stripedSurfaces = new NEXUS_StripedSurfaceHandle[m_numStreams];
#endif
      }

      ~NexusVideoPrivate()
      {
         for (uint32_t s = 0; s < m_numStreams; s++)
            m_streams[s].Cleanup();

#ifdef SINGLE_PROCESS
         if (m_stripedSurfaces)
         {
            delete [] m_stripedSurfaces;
            m_stripedSurfaces = NULL;
         }
#endif

         delete [] m_streams;
         m_streams = NULL;

         if (m_gfx2d)
         {
            NEXUS_Graphics2D_Close(m_gfx2d);
            m_gfx2d = NULL;
         }

         if (m_gfxDone)
         {
            BKNI_DestroyEvent(m_gfxDone);
            m_gfxDone = NULL;
         }
      }

   public:
      BKNI_EventHandle           m_gfxDone;
      NEXUS_Graphics2DHandle     m_gfx2d;
      NexusVideoStream           *m_streams;
      uint32_t                   m_numStreams;
#ifdef SINGLE_PROCESS
      NEXUS_StripedSurfaceHandle *m_stripedSurfaces;
#endif
   };

#define priv ((NexusVideoPrivate*)m_priv)

/////////////////////////////////////////////////////////////////////////////////

#ifdef SINGLE_PROCESS

Video::Video(const std::string &videoFileName) :
   m_format(eNONE),
   m_updateMode(eEXT_SYNC)
{
   NEXUS_DisplayHandle nexusDisplay = ((PlatformDataNexus*)Application::Instance()->m_platform.GetPlatformData())->m_nexusDisplay;

   std::string fileName = Application::Instance()->FindResource(videoFileName);

   m_priv = new NexusVideoPrivate(fileName, nexusDisplay);
}

Video::Video(const std::vector<std::string> &videoFileNames) :
   m_format(eNONE),
   m_updateMode(eEXT_SYNC)
{
   NEXUS_DisplayHandle nexusDisplay = ((PlatformDataNexus*)Application::Instance()->m_platform.GetPlatformData())->m_nexusDisplay;

   std::vector<std::string> fileNames;

   for (uint32_t i = 0; i < videoFileNames.size(); ++i)
      fileNames.push_back(Application::Instance()->FindResource(videoFileNames[i]));

   m_priv = new NexusVideoPrivate(fileNames, nexusDisplay);
}

#else

Video::Video(const std::string &videoFileName) :
   m_format(eNONE),
   m_updateMode(eEXT_SYNC)
{
   std::string fileName = Application::Instance()->FindResource(videoFileName);

   m_priv = new NexusVideoPrivate(fileName, NULL);
}

Video::Video(const std::vector<std::string> &videoFileNames) :
   m_format(eNONE),
   m_updateMode(eEXT_SYNC)
{
   std::vector<std::string> fileNames;

   for (uint32_t i = 0; i < videoFileNames.size(); ++i)
      fileNames.push_back(Application::Instance()->FindResource(videoFileNames[i]));

   m_priv = new NexusVideoPrivate(fileNames[0], NULL);
}

#endif

Video::~Video()
{
   delete m_priv;
}

void Video::Cleanup()
{
}

uint32_t Video::NumStreams() const
{
   return priv->m_numStreams;
}

uint32_t Video::SourceWidth(uint32_t vidIndx)
{
   return priv->m_streams[vidIndx].m_sourceWidth;
}

uint32_t Video::SourceHeight(uint32_t vidIndx)
{
   return priv->m_streams[vidIndx].m_sourceHeight;
}

uint32_t Video::DestWidth(uint32_t vidIndx)
{
   return priv->m_streams[vidIndx].m_outputWidth;
}

uint32_t Video::DestHeight(uint32_t vidIndx)
{
   return priv->m_streams[vidIndx].m_outputHeight;
}

static bool NeedsPowerOfTwoWidth(eVideoFrameFormat format, GLTexture::eVideoTextureMode mode)
{
   // Only YUV422 EGLImage based textures are now required to be power-of-two width
   if (format == eYUV422 && (mode == GLTexture::eEGL_IMAGE || mode == GLTexture::eEGL_IMAGE_EXPLICIT))
      return true;

   return false;
}

void Video::SetOutputParameters(uint32_t widthArg, uint32_t heightArg, eVideoFrameFormat format, GLTexture::eVideoTextureMode mode)
{
   uint32_t width, height;

   m_format = format;

   for (uint32_t s = 0; s < priv->m_numStreams; s++)
   {
      if (widthArg == 0 || widthArg == 0)
      {
         width = SourceWidth(s);
         height = SourceHeight(s);
      }
      else
      {
         width = widthArg;
         height = heightArg;
      }

      if (width & 1)
         width += 1;

      if (NeedsPowerOfTwoWidth(format, mode))
      {
         // x must be power2 due to a hardware limitation with raster textures
         uint32_t fixWidth = 0x80000000U;
         while((fixWidth & width) == 0)
            fixWidth >>= 1;
         width = fixWidth;
      }
      else if (format == eYUVX444)
      {
         // Must be multiple of 4
         width = width & ~3;
      }
      else if (format == eRGB565)
      {
         // Must be multiple of 8
         width = width & ~7;
      }
      else if (format == eRGBX8888)
      {
         // Must be multiple of 4
         width = width & ~3;
      }

      priv->m_streams[s].SetOutputs(width, height, format);
   }

   try
   {
      NEXUS_Graphics2DSettings    gfxSettings;

      BKNI_CreateEvent(&priv->m_gfxDone);

      priv->m_gfx2d = NEXUS_Graphics2D_Open(0, NULL);
      if (priv->m_gfx2d == NULL)
         BSG_THROW("NEXUS_Graphics2D_Open failed");

      NEXUS_Graphics2D_GetSettings(priv->m_gfx2d, &gfxSettings);
      gfxSettings.checkpointCallback.callback = complete;
      gfxSettings.checkpointCallback.context = priv->m_gfxDone;
      NEXUS_Graphics2D_SetSettings(priv->m_gfx2d, &gfxSettings);
   }
   catch (Exception e)
   {
      if (priv->m_gfxDone)
         BKNI_DestroyEvent(priv->m_gfxDone);

      throw e;
   }
}

void Video::SetOutputFormat(eVideoFrameFormat format, GLTexture::eVideoTextureMode mode)
{
   SetOutputParameters(0, 0, format, mode);
}

#ifdef SINGLE_PROCESS

Video::eFrameStatus Video::GetFrame(eMode mode, VideoBuffer *buffer, TextureHandle texture)
{
   NEXUS_StripedSurfaceHandle stripedSurface = NULL;

   if (buffer == NULL)
      BSG_THROW("NULL buffer passed to GetFrame");

   if (buffer->GetWidth() != priv->m_streams[0].m_outputWidth || buffer->GetHeight() != priv->m_streams[0].m_outputHeight)
      BSG_THROW("Video buffer size does not match decoder");

   if (priv->m_numStreams > 1)
      BSG_THROW("You are using the single stream GetFrame() method with Mosaic video");

   if (mode != BLOCK)
   {
      // We may not have a new video frame ready to use, in which case NEXUS_VideoDecoder_CreateStripedSurface
      // will return NULL. Check now so we can avoid any destination surface lock if we don't yet have a buffer.
      stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(priv->m_streams[0].m_videoDecoder);
      if (stripedSurface == NULL)
      {
         // No more frames ready yet. Pixmap should still have valid data from last time.
         return Video::eFRAME_REPEAT;
      }
   }

   NEXUS_SurfaceHandle dstSurface = ((NexusPixmapData*)buffer->GetNativePixmap()->GetNativePixmapData())->m_surface;
   if (dstSurface)
   {
      NEXUS_Error rc;

      // Lock the destination buffer for writing. This might take some time if the 3D core is
      // using it right now.
      if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
         texture->Lock();

      if (mode == BLOCK)
      {
         // Wait for a new video frame
         while (stripedSurface == NULL)
            stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(priv->m_streams[0].m_videoDecoder);
      }
      else
      {
         // We MUST check for another new frame now, even if we got a surface prior to the lock.
         // The video buffer chain may have looped around and we could end up using a surface
         // that's being written into otherwise, resulting in nasty tearing.
         NEXUS_StripedSurfaceHandle nextStripe = NEXUS_VideoDecoder_CreateStripedSurface(priv->m_streams[0].m_videoDecoder);
         if (nextStripe == NULL)
         {
            if (stripedSurface == NULL)
            {
               // Still no striped surface ready. Unlock and return.
               if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
                  texture->Unlock();
               return Video::eFRAME_REPEAT;
            }
            // else
            // We can use the striped surface we got before the lock as there isn't a newer one. That's already stripedSurface.
         }
         else
         {
            // Use the newest surface (nextStripe), but first destroy the buffer we got before the lock (if we got one)
            if (stripedSurface != NULL)
               NEXUS_VideoDecoder_DestroyStripedSurface(priv->m_streams[0].m_videoDecoder, stripedSurface);

            stripedSurface = nextStripe;
         }
      }

      // Now destripe the surface into our destination buffer
      rc = NEXUS_Graphics2D_DestripeToSurface(priv->m_gfx2d, stripedSurface, dstSurface, NULL);
      BDBG_ASSERT(!rc);

      // We must wait for the destripe to complete now
      do
      {
         rc = NEXUS_Graphics2D_Checkpoint(priv->m_gfx2d, NULL);
         if (rc == NEXUS_GRAPHICS2D_QUEUED)
            rc = BKNI_WaitForEvent(priv->m_gfxDone, 1000);
      }
      while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

      // We're done with the striped surface, so kill it.
      NEXUS_VideoDecoder_DestroyStripedSurface(priv->m_streams[0].m_videoDecoder, stripedSurface);

      // Tell V3D we've changed it
      if (m_updateMode == eEXT_SYNC || m_updateMode == eEXT)
         texture->SetUpdatedRegion(0, 0, priv->m_streams[0].m_outputWidth, priv->m_streams[0].m_outputHeight);

      // Unlock the image so V3D can use it.
      if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
         texture->Unlock();

      return Video::eFRAME_NEW;
   }

   return Video::eFRAME_ERROR;
}

void Video::GetMosaicFrames(std::vector<eFrameStatus> &results,
                            const std::vector<VideoBuffer *> &buffers, const std::vector<TextureHandle> &textures)
{
   uint32_t numStripedSurfaces;
   uint32_t s;

   // Initialiase the striped surface to NULL
   // as NEXUS_VideoDecoder_CreateStripedMosaicSurfaces might not do it
   for (s = 0; s < priv->m_numStreams; s++)
      priv->m_stripedSurfaces[s] = NULL;

   // We may not have a new video frame ready to use, in which case NEXUS_VideoDecoder_CreateStripedMosaicSurfaces
   // will return all NULL. Check now so we can avoid any destination surface lock if we don't yet have a buffer.
   NEXUS_VideoDecoder_CreateStripedMosaicSurfaces(priv->m_streams[0].m_videoDecoder, priv->m_stripedSurfaces,
                                                  priv->m_numStreams, &numStripedSurfaces);

   results.clear();

   bool gotSome = false;
   for (s = 0; s < priv->m_numStreams; s++)
   {
      results.push_back(Video::eFRAME_REPEAT);

      if (priv->m_stripedSurfaces[s] != NULL)
         gotSome = true;
   }

   if (!gotSome)
      return;

   for (s = 0; s < priv->m_numStreams; s++)
   {
      if (buffers[s]->GetWidth() != priv->m_streams[s].m_outputWidth || buffers[s]->GetHeight() != priv->m_streams[s].m_outputHeight)
         BSG_THROW("Video buffer size does not match decoder");

      if (priv->m_stripedSurfaces[s] != NULL)
      {
         NEXUS_SurfaceHandle dstSurface = ((NexusPixmapData*)buffers[s]->GetNativePixmap()->GetNativePixmapData())->m_surface;
         if (dstSurface)
         {
            NEXUS_Error rc;

            // Lock the destination buffer for writing. This might take some time if the 3D core is
            // using it right now.
            if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
               textures[s]->Lock();

            // Now destripe the surface into our destination buffer
            rc = NEXUS_Graphics2D_DestripeToSurface(priv->m_gfx2d, priv->m_stripedSurfaces[s], dstSurface, NULL);
            BDBG_ASSERT(!rc);

            // We must wait for the destripe to complete now
            do
            {
               rc = NEXUS_Graphics2D_Checkpoint(priv->m_gfx2d, NULL);
               if (rc == NEXUS_GRAPHICS2D_QUEUED)
                  rc = BKNI_WaitForEvent(priv->m_gfxDone, 1000);
            }
            while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

            // Tell V3D we've changed it
            if (m_updateMode == eEXT_SYNC || m_updateMode == eEXT)
               textures[s]->SetUpdatedRegion(0, 0, priv->m_streams[s].m_outputWidth, priv->m_streams[s].m_outputHeight);

            // Unlock the image so V3D can use it.
            if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
               textures[s]->Unlock();

            results[s] = Video::eFRAME_NEW;
         }
      }
   }

   NEXUS_VideoDecoder_DestroyStripedMosaicSurfaces(priv->m_streams[0].m_videoDecoder, priv->m_stripedSurfaces, numStripedSurfaces);
}

#else

Video::eFrameStatus Video::GetFrame(eMode mode, VideoBuffer *buffer, TextureHandle texture)
{
   unsigned                               numReturned = 0;        // Number of frames captured
   NEXUS_SurfaceHandle                    captureSurface;
   NEXUS_SimpleVideoDecoderCaptureStatus  captureStatus;

   NEXUS_Error rc;

   NEXUS_SurfaceHandle dstSurface = ((NexusPixmapData*)buffer->GetNativePixmap()->GetNativePixmapData())->m_surface;

   if (buffer == NULL)
      BSG_THROW("NULL buffer passed to GetFrame");

   if (buffer->GetWidth() != priv->m_streams[0].m_outputWidth ||
       buffer->GetHeight() != priv->m_streams[0].m_outputHeight)
   {
      BSG_THROW("Video buffer size does not match decoder");
   }

   if (priv->m_numStreams > 1)
   {
      BSG_THROW("You are using the single stream GetFrame() method with Mosaic video");
   }

   if (mode != BLOCK)
   {
      /* We may not have a new video frame ready to use, in which case numReturned = 0 from */
      /* NEXUS_SimpleVideoDecoder_GetCapturedSurfaces. Check now so we can avoid any destination surface lock if we don't yet have a buffer. */
      /* Try to capture a frame from the simple decoder */
      NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(priv->m_streams[0].m_videoDecoder, &captureSurface, &captureStatus, 1, &numReturned);

      /* If not frame available*/
      if (numReturned == 0)
      {
         /* If this buffer never been cleared, fill it with black */
         if ((!buffer->Cleared()) && dstSurface)
         {
            texture->Lock();

            NEXUS_Graphics2DFillSettings fillSettings;
            NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
            fillSettings.surface = dstSurface;
            fillSettings.rect.width = priv->m_streams[0].m_outputWidth;
            fillSettings.rect.height = priv->m_streams[0].m_outputHeight;
            fillSettings.color = 0;
            rc = NEXUS_Graphics2D_Fill(priv->m_gfx2d, &fillSettings);
            BDBG_ASSERT(!rc);

            // We must wait for the fill to complete now
            do
            {
               rc = NEXUS_Graphics2D_Checkpoint(priv->m_gfx2d, NULL);
               if (rc == NEXUS_GRAPHICS2D_QUEUED)
                  rc = BKNI_WaitForEvent(priv->m_gfxDone, 1000);
            }
            while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

            // Buffer cleared
            buffer->SetCleared(true);

            texture->SetUpdatedRegion(0, 0, priv->m_streams[0].m_outputWidth, priv->m_streams[0].m_outputHeight);

            texture->Unlock();

            return Video::eFRAME_NEW;
         }
         else
         {
            /* No more frames ready yet. Pixmap should still have valid data from last time. */
            return Video::eFRAME_REPEAT;
         }
      }
   }

   /* If no error and there is a destination surface */
   if (dstSurface)
   {
      /* Lock the destination buffer for writing. This might take some time if the 3D core is */
      /* using it right now. */
      if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
         texture->Lock();

      if (mode == BLOCK)
      {
         /* Wait for a new video frame */
         while (numReturned == 0)
            NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(priv->m_streams[0].m_videoDecoder, &captureSurface,&captureStatus, 1, &numReturned);
      }

      /* Blit the captured frame onto a texture */
      NEXUS_Graphics2DBlitSettings          blitSettings;
      NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);
      blitSettings.source.surface = captureSurface;
      blitSettings.output.surface = dstSurface;
      rc = NEXUS_Graphics2D_Blit(priv->m_gfx2d, &blitSettings);
      BDBG_ASSERT(!rc);

      /* We must wait for the blit to complete now */
      do
      {
         rc = NEXUS_Graphics2D_Checkpoint(priv->m_gfx2d, NULL);
         if (rc == NEXUS_GRAPHICS2D_QUEUED)
            rc = BKNI_WaitForEvent(priv->m_gfxDone, 1000);
      }
      while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

      /* Release the capture surface for further capture */
       NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(priv->m_streams[0].m_videoDecoder, &captureSurface, numReturned);

      /* Tell V3D we've changed it */
      if (m_updateMode == eEXT_SYNC || m_updateMode == eEXT)
         texture->SetUpdatedRegion(0, 0, priv->m_streams[0].m_outputWidth, priv->m_streams[0].m_outputHeight);

      /* Unlock the image so V3D can use it. */
      if (m_updateMode == eEXT_SYNC || m_updateMode == eALWAYS_SYNC)
         texture->Unlock();

      return Video::eFRAME_NEW;
   }

   return Video::eFRAME_ERROR;
}

void Video::GetMosaicFrames(std::vector<eFrameStatus> &,
                            const std::vector<VideoBuffer *> &, const std::vector<TextureHandle> &)
{
   BSG_THROW("Mosaic mode not supported");
}

#endif
}

#endif /* BSG_STAND_ALONE */
