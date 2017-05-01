/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

// Nexus or Magnum should really define this before using UINT32_C, but don't currently
#define __STDC_CONSTANT_MACROS

/* TODO: This is a hack moved from old version of eglext.h to keep using
 * the EGL extensions directly here (i.e. not via eglGetProcAddress()).
 * This hack should be removed and code should be reworked to access the
 * relevant functionality properly (i.e. via eglGetProcAddress()).
 */
#define EGL_EGLEXT_PROTOTYPES 1

#include "bsg_platform.h"

#include "bsg_video_decoder.h"
#include "bsg_exception.h"
#include "bsg_platform_nexus.h"
#include "bsg_application.h"

#include "bsg_media_prober_nexus.h"

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

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_video_window.h"
#include "nexus_mosaic_display.h"
#include "nexus_mosaic_video_decoder.h"
#include "nexus_playpump.h"
#include "nexus_playback.h"
#include "nexus_stc_channel.h"
#include "nexus_file.h"
#include "nexus_base_types.h"
#include "nexus_core_utils.h"

#include "nexus_timebase.h"

#ifndef SINGLE_PROCESS
#include "nxclient.h"
#endif

namespace bsg
{

   class VideoTextureFence
   {
   public:
      VideoTextureFence() :
         m_fence(EGL_NO_SYNC_KHR),
         m_dpy(EGL_NO_DISPLAY)
      {
      };
      ~VideoTextureFence()
      {
         if (m_fence != EGL_NO_SYNC_KHR)
            eglDestroySyncKHR(m_dpy, m_fence);
      };
      void Create(EGLDisplay  dpy)
      {
         if (m_fence != EGL_NO_SYNC_KHR)
            eglDestroySyncKHR(m_dpy, m_fence);

         m_fence  = eglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, NULL);
         m_dpy    = dpy;
      };
      void Destroy()
      {
         eglDestroySyncKHR(m_dpy, m_fence);
         m_fence = EGL_NO_SYNC_KHR;
      };
      void Wait()
      {
         eglClientWaitSyncKHR(m_dpy, m_fence, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, EGL_FOREVER_KHR);
      };
      bool Valid()
      {
         return m_fence != EGL_NO_SYNC_KHR;
      };
      bool Signaled()
      {
         EGLint value;
         eglGetSyncAttribKHR(m_dpy, m_fence, EGL_SYNC_STATUS_KHR, &value);

         return value == EGL_SIGNALED_KHR ? true : false;
      };
   private:
      EGLSyncKHR     m_fence;
      EGLDisplay     m_dpy;
   };

   static void complete(void *data, int unused)
   {
      BSTD_UNUSED(unused);
      BKNI_SetEvent((BKNI_EventHandle)data);
   }

   static bool NeedsPowerOfTwoWidth(NativePixmap::ePixmapFormat pixmapFormat, GLTexture::eVideoTextureMode mode)
   {
#ifndef BSG_VC5
      // Only VC4 YUV422 EGLImage based textures are now required to be power-of-two width
      if (pixmapFormat == NativePixmap::eYUV422_TEXTURE && (mode == GLTexture::eEGL_IMAGE || mode == GLTexture::eEGL_IMAGE_EXPLICIT))
         return true;
#endif

      return false;
   }


   class VideoOutputInformation;
   class NexusVideoFilePlayback;

   /////////////////////////////////////////////////////////////////////////////////
   //          NexusVideoStream
   /////////////////////////////////////////////////////////////////////////////////

   class NexusVideoStream
   {
   public:
      enum eVideoMode
      {
         eSINGLE,
         eMOSAIC
      };

      //! Default constructor
      NexusVideoStream(NexusVideoFilePlayback &filePlayback, uint32_t fileStreamIndex) :
         m_filePlayback(filePlayback),
         m_stcChannel(NULL),
         m_videoPidChannel(NULL),
         m_audioPidChannel(NULL),
         m_nexusDisplay(NULL),
         m_sourceWidth(0),
         m_sourceHeight(0),
         m_outputWidth(0),
         m_outputHeight(0),
         m_decodeFormat(NONE),
         m_gfx2d(NULL),
         m_gfxDone(NULL),
         m_playStarted(false),
         m_streamIndex(fileStreamIndex),
         m_globalStreamIndex(m_totalStreamCreated++)
      {

      };

      //! Destructor
      virtual ~NexusVideoStream() { };

      //! Creates the necessary buffers and configure the decoder
      //! @param outputInfo
      //! @param numDecodeBuffers This is the number of buffer used by the decoder
      //! @param gfxDone Event used when the 2D operations are finished
      //! @param gfx2d Handle to the 2D graphic hardware
      //! @param nexusDisplay  Nexus display is needed in single process mode
      //! @param playFullScreenToo  Open a full screen hardware video window for this stream
      virtual void Configure(const VideoOutputInformation &outputInfo, uint32_t numDecodeBuffers,
                              NEXUS_Graphics2DHandle gfx2d, BKNI_EventHandle gfxDone, NEXUS_DisplayHandle nexusDisplay) = 0;

      //! Updates the current texture with a new frame if there is one available
      //! @param mode specifies if the function blocks while waiting for a new available video frame
      //! @param updateMode specifies the synchronisation used to update the texture
      virtual VideoDecoder::eFrameStatus UpdateFrame(VideoDecoder::eMode mode, eVideoUpdateMode updateMode) = 0;

      //! Updates the current texture using a striped surface passed an argument
      //! @param updateMode specifies the synchronisation used to update the texture
      //! @param stripedSurfaces surface to destripe
      virtual VideoDecoder::eFrameStatus UpdateFrame(eVideoUpdateMode updateMode, NEXUS_StripedSurfaceHandle stripedSurfaces) = 0;

      //! Returns the decoder for that stream
      virtual NEXUS_VideoDecoderHandle GetVideoDecoder() { return NULL; }
      virtual NEXUS_SimpleVideoDecoderHandle GetSimpleVideoDecoder() { return NULL; }

      //! Starts decoder and playback
      virtual void StartPlayback() = 0;

      //! Stops decoder and playback
      virtual void StopPlayback() = 0;

      //! Returns a handle to the last updates texture
      virtual TextureHandle &GetCurrentTexture() = 0;

      //! Returns a handle to the last updates pixmap
      virtual NativePixmap *GetCurrentPixmap() = 0;

      //! Moves the current index to the next buffer
      virtual void ChangeCurrentBuffer() = 0 ;

      //! Returns the width of the source video
      virtual uint32_t SourceWidth() { return m_sourceWidth; };

      //! Returns the height of the source video
      virtual uint32_t SourceHeight() { return m_sourceHeight; };

      //! Returns the width of the output video
      virtual uint32_t DestWidth() { return m_outputWidth; };

      //! Returns the width of the output video
      virtual uint32_t DestHeight() { return m_outputHeight; };

      virtual void SetAudioDelay(uint32_t ms) = 0;

      virtual void CreateFenceForSyncUpdate(EGLDisplay dpy)
      {
         m_videoTexFences[m_curbufferIndex].Create(dpy);
      };

   protected:

      //! Creates all the associated buffers: pixmap, texture handle
      //! @param widthOutput is the width of the output frame
      //! @param heightOutput is the height of the output frame
      //! @param pixmapFormat is the format used to create the pixmap
      //! @param textureMode is the type of texture used
      //! @param numDecodeBuffers is the number of buffers created of each type
      virtual void CreateBuffers(uint32_t widthOutput, uint32_t heightOutput,
                                 NativePixmap::ePixmapFormat pixmapFormat,
                                 GLTexture::eVideoTextureMode textureMode,
                                 uint32_t numDecodeBuffers)
      {
         // Set the output sizes
         SetOutputParameters(widthOutput, heightOutput, pixmapFormat, textureMode);

         m_pixmaps.clear();
         m_videoTex.clear();
         m_videoTexFences.clear();

         m_curbufferIndex.Setup(0, numDecodeBuffers - 1);

         // For each buffer
         for (uint32_t bufferIndex = 0; bufferIndex <= m_curbufferIndex.GetMax(); bufferIndex++)
         {
            // Make the native pixmaps which we will use as the video target surface & as a texture
            m_pixmaps.push_back(new NativePixmap(m_outputWidth, m_outputHeight, pixmapFormat));

            m_videoTex.push_back(TextureHandle(New));

            m_videoTexFences.push_back(VideoTextureFence());

            // Clear black the pixmap
            NEXUS_Graphics2DFillSettings fillSettings;
            NEXUS_Graphics2D_GetDefaultFillSettings(&fillSettings);
            fillSettings.surface = ((NexusPixmapData*)m_pixmaps[bufferIndex]->GetNativePixmapData())->m_surface;
            fillSettings.rect.width = m_outputWidth;
            fillSettings.rect.height = m_outputHeight;
            fillSettings.color = 0;
            NEXUS_Error rc = NEXUS_Graphics2D_Fill(m_gfx2d, &fillSettings);
            BDBG_ASSERT(!rc);
         }

         // We must wait for the fill to complete now (Do we?)
         NEXUS_Error rc;
         do
         {
            rc = NEXUS_Graphics2D_Checkpoint(m_gfx2d, NULL);
            if (rc == NEXUS_GRAPHICS2D_QUEUED)
               rc = BKNI_WaitForEvent(m_gfxDone, 1000);
         }
         while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

      };

      //! Sets the output size for the buffers according to the reuqested sizes
      //! and pixmap format
      //!@param widthArg is the requested width for the output
      //!@param heightArg is the requested height for the output
      //!@param pixmapFormat is the format of the pixmap buffer
      //!@param mode is the type of texture used
      virtual void SetOutputParameters(uint32_t widthArg, uint32_t heightArg, NativePixmap::ePixmapFormat pixmapFormat,
                                    GLTexture::eVideoTextureMode mode)
      {
         if (widthArg == 0 || widthArg == 0)
         {
            m_outputWidth = m_sourceWidth;
            m_outputHeight = m_sourceHeight;
         }
         else
         {
            m_outputWidth = widthArg;
            m_outputHeight = heightArg;
         }

         if (m_outputWidth & 1)
            m_outputWidth += 1;

         if (NeedsPowerOfTwoWidth(pixmapFormat, mode))
         {
            // x must be power2 due to a hardware limitation with raster textures
            uint32_t fixWidth = 0x80000000U;
            while((fixWidth & m_outputWidth) == 0)
               fixWidth >>= 1;
            m_outputWidth = fixWidth;
         }
         else if (pixmapFormat == NativePixmap::eABGR8888_TEXTURE) //video format eYUVX444
         {
            // Must be multiple of 4
            m_outputWidth = m_outputWidth & ~3;
         }
         else if (pixmapFormat == NativePixmap::eRGB565_TEXTURE)
         {
            // Must be multiple of 8
            m_outputWidth = m_outputWidth & ~7;
         }
         else if (pixmapFormat == NativePixmap::eABGR8888_TEXTURE)
         {
            // Must be multiple of 4
            m_outputWidth = m_outputWidth & ~3;
         }
         else if (pixmapFormat == NativePixmap::eYV12_TEXTURE)
         {
            // even height for YV12
            if (m_outputHeight & 1)
               m_outputHeight += 1;
         }
      };

      //! Virtual cleanup function
      virtual void Cleanup() = 0;

   protected:
      NexusVideoFilePlayback           &m_filePlayback;
      NEXUS_StcChannelHandle           m_stcChannel;
      NEXUS_PidChannelHandle           m_videoPidChannel;
      NEXUS_PidChannelHandle           m_audioPidChannel;
      NEXUS_DisplayHandle              m_nexusDisplay;

      uint32_t                         m_sourceWidth;
      uint32_t                         m_sourceHeight;
      uint32_t                         m_outputWidth;
      uint32_t                         m_outputHeight;

      eVideoFrameFormat                m_decodeFormat;
      std::vector<NativePixmap*>       m_pixmaps;
      std::vector<TextureHandle>       m_videoTex;
      std::vector<VideoTextureFence>   m_videoTexFences;
      CircularIndex                    m_curbufferIndex;

      NEXUS_Graphics2DHandle           m_gfx2d;
      BKNI_EventHandle                 m_gfxDone;

      bool                             m_playStarted;

      uint32_t                         m_streamIndex;         // Index of the streams with a video file
      uint32_t                         m_globalStreamIndex;   // Total number of streams accross multiple video files
      static uint32_t                  m_totalStreamCreated;  // Number of instances for this class
   };

   uint32_t NexusVideoStream::m_totalStreamCreated = 0;

   /////////////////////////////////////////////////////////////////////////////////
   //          VideoOutputInformation
   /////////////////////////////////////////////////////////////////////////////////

   // Helper class
   class VideoOutputInformation
   {
   public:
      NexusVideoStream::eVideoMode           m_mode;           // Mosaic, Single video
      uint32_t                               m_widthOutput;
      uint32_t                               m_heightOutput;
      NativePixmap::ePixmapFormat            m_pixmapFormat;
      GLTexture::eVideoTextureMode           m_textureMode;
   };

   /////////////////////////////////////////////////////////////////////////////////
   //          NexusVideoStreamSingleProcess
   /////////////////////////////////////////////////////////////////////////////////

	// Helper class to use on pump, playback per video file

   class NexusVideoFilePlayback
   {
   public:

      //! Constructor
      NexusVideoFilePlayback(std::vector<MediaData> &mediaDataList) :
         m_filePlay(NULL),
         m_playPump(NULL),
         m_playBack(NULL),
         m_mediaDataList(mediaDataList),
         m_playStarted(false)
      {
         if (m_mediaDataList.size() <= 0)
            BSG_THROW("No video stream provided");

         NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
         NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
         m_playPump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
         if (m_playPump == NULL)
            BSG_THROW("NEXUS_Playpump_Open failed");

         m_playBack = NEXUS_Playback_Create();
         if (m_playBack == NULL)
            BSG_THROW("NEXUS_Playback_Create failed");

         NEXUS_PlaybackSettings playbackSettings;
         NEXUS_Playback_GetSettings(m_playBack, &playbackSettings);
         playbackSettings.playpump = m_playPump;
         NEXUS_Playback_SetSettings(m_playBack, &playbackSettings);

         /* open stream file */
         /* 2nd argument is the index */
         printf("File Name: %s\n", m_mediaDataList[0].m_filename.c_str());
         m_filePlay = NEXUS_FilePlay_OpenPosix(m_mediaDataList[0].m_filename.c_str(), m_mediaDataList[0].m_filename.c_str());
         if (m_filePlay == NULL)
            BSG_THROW("Failed to open video file");

         // Use time slice if there are more than one video
         if (m_mediaDataList.size() > 1)
         {
            NEXUS_Timebase timebase = NEXUS_Timebase_e0;
            NEXUS_TimebaseSettings timebaseSettings;
            NEXUS_Timebase_GetSettings(timebase, &timebaseSettings);
            timebaseSettings.sourceType = NEXUS_TimebaseSourceType_eFreeRun;
            NEXUS_Timebase_SetSettings(timebase, &timebaseSettings);
         }

      }

      //!Destructor
      ~NexusVideoFilePlayback()
      {
         CleanUp();
      }

      //! Returns the number of stream contained in the video file
      uint32_t GetNumberStreams() const
      {
         return m_mediaDataList.size();
      }

      //!
      const MediaData &GetMediaData(uint32_t index) const
      {
         if (m_mediaDataList.size() <= index)
            BSG_THROW("Media data index out of range");
         return m_mediaDataList[index];
      }

      //! Starts playback
      void StartPlayback()
      {
         if (!m_playStarted)
         {
            m_playStarted = true;

            if (m_playBack)
               if (NEXUS_Playback_Start(m_playBack, m_filePlay, NULL) != NEXUS_SUCCESS)
                  printf("Not started playback\n");
         }
      }

      //! Stops playback
      void StopPlayback()
      {
         if (m_playStarted)
         {
            m_playStarted = false;

            if (m_playBack)
               NEXUS_Playback_Stop(m_playBack);
         }
      }

      //! Accessor functions
      NEXUS_FilePlayHandle GetFilePlay() const { return m_filePlay; }
      NEXUS_PlaypumpHandle GetPlayPump() const { return m_playPump; }
      NEXUS_PlaybackHandle GetPlayback() const { return m_playBack; }

private:

      //! Free ressources
      void CleanUp()
      {
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

      // Default Constructor: Not implemented
      NexusVideoFilePlayback();

      NEXUS_FilePlayHandle             m_filePlay;
      NEXUS_PlaypumpHandle             m_playPump;
      NEXUS_PlaybackHandle             m_playBack;

      std::vector<MediaData>           m_mediaDataList;

      bool                             m_playStarted;
   };


   /////////////////////////////////////////////////////////////////////////////////
   //          NexusVideoStreamSingleProcess
   /////////////////////////////////////////////////////////////////////////////////
#ifdef SINGLE_PROCESS
   class NexusVideoStreamSingleProcess : public NexusVideoStream
   {
   public:
      //! Default Constructor
      NexusVideoStreamSingleProcess(NexusVideoFilePlayback &filePlayback, uint32_t fileStreamIndex, NEXUS_VideoWindowHandle videoWindow,
                                    int32_t fullScreenStreamIndex) :
         NexusVideoStream(filePlayback, fileStreamIndex),
         m_videoDecoder(NULL),
         m_audioDecoder(NULL),
         m_videoWindow(videoWindow),
         m_mosaicVideoWindow(NULL),
         m_fullScreenStreamIndex(fullScreenStreamIndex)
      {
         const MediaData & mediaData = m_filePlayback.GetMediaData(m_streamIndex);

         m_sourceWidth  = mediaData.m_width;
         m_sourceHeight = mediaData.m_height;
      };

      //! Destructor
      virtual ~NexusVideoStreamSingleProcess()
      {
         Cleanup();
      };

      //! Clean up the member variables
      virtual void Cleanup()
      {
         /* Clear out the EGL images */
         for (uint32_t i = 0; i < m_videoTex.size(); ++i)
            m_videoTex[i].Delete();

         m_videoTex.clear();

         for (uint32_t bufferIndex = 0; bufferIndex < m_pixmaps.size(); bufferIndex++)
         {
            if (m_pixmaps[bufferIndex])
               delete m_pixmaps[bufferIndex];
         }

         m_pixmaps.clear();

         m_videoTexFences.clear();

         StopPlayback();

         if (m_mosaicVideoWindow)
         {
            NEXUS_VideoWindow_RemoveInput(m_mosaicVideoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
            NEXUS_VideoWindow_Close(m_mosaicVideoWindow);
            m_mosaicVideoWindow = NULL;
         }

         if (m_audioPidChannel)
         {
            NEXUS_Playback_ClosePidChannel(m_filePlayback.GetPlayback(), m_audioPidChannel);
            m_audioPidChannel = NULL;
         }

         if (m_videoPidChannel)
         {
            NEXUS_Playback_ClosePidChannel(m_filePlayback.GetPlayback(), m_videoPidChannel);
            m_videoPidChannel = NULL;
         }

         if (m_stcChannel)
         {
            NEXUS_StcChannel_Close(m_stcChannel);
            m_stcChannel = NULL;
         }

#if NEXUS_HAS_AUDIO
         if (m_audioDecoder)
         {
            NEXUS_AudioDecoder_Close(m_audioDecoder);
            m_audioDecoder = NULL;
         }
#endif

         if (m_videoDecoder)
         {
            // Check comment for NEXUS_Display_ConnectVideoInput
            if (m_globalStreamIndex == 0)
            {
               if (m_videoWindow == NULL)
                  NEXUS_Display_DisconnectVideoInput(m_nexusDisplay, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
               else
               {
                  NEXUS_VideoWindow_RemoveInput(m_videoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
                  NEXUS_VideoDecoder_SetPowerState(m_videoDecoder, false);
               }
            }
         }

         if (m_videoDecoder)
         {
            NEXUS_VideoDecoder_Close(m_videoDecoder);
            m_videoDecoder = NULL;
         }
      };

      //! Creates the necessary buffers and configure the decoder
      //! @param widthOutput is the width of the output frame
      //! @param heightOutput is the height of the output frame
      //! @param pixmapFormat is the format used to create the pixmap
      //! @param textureMode is the type of texture used
      //! @param numDecodeBuffers is the number of buffers created of each type
      virtual void Configure(const VideoOutputInformation &outputInfo, uint32_t numDecodeBuffers,
                           NEXUS_Graphics2DHandle gfx2d, BKNI_EventHandle gfxDone, NEXUS_DisplayHandle nexusDisplay)
      {
         try
         {
            // Get the stream information from the video file
            const MediaData & mediaData = m_filePlayback.GetMediaData(m_streamIndex);

            m_nexusDisplay = nexusDisplay;

            m_gfxDone = gfxDone;
            m_gfx2d = gfx2d;

            Cleanup();

            // Creates a number of pixmap and wraps them into video buffers
            CreateBuffers(outputInfo.m_widthOutput, outputInfo.m_heightOutput,
                           outputInfo.m_pixmapFormat, outputInfo.m_textureMode, numDecodeBuffers);

            /* Bring up audio decoders and outputs */
            NEXUS_PlatformConfiguration platformConfig;
            NEXUS_Platform_GetConfiguration(&platformConfig);

            if (outputInfo.m_mode != eMOSAIC || m_fullScreenStreamIndex == (int32_t)m_globalStreamIndex)
            {
#if NEXUS_HAS_AUDIO
               if (mediaData.m_audioCodec != NEXUS_AudioCodec_eUnknown)
               {
                  m_audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
                  if (m_audioDecoder == NULL)
                     BSG_THROW("NEXUS_AudioDecoder_Open failed");

#if (NEXUS_NUM_AUDIO_DACS > 0)
                  NEXUS_AudioOutput_AddInput(
                     NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]),
                     NEXUS_AudioDecoder_GetConnector(m_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
                  NEXUS_AudioOutput_AddInput(
                     NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]),
                     NEXUS_AudioDecoder_GetConnector(m_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
                  NEXUS_AudioOutput_AddInput(
                     NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]),
                     NEXUS_AudioDecoder_GetConnector(m_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo));
#endif
               }
#endif
            }

            // bring up video decoder
            if (outputInfo.m_mode == eMOSAIC)
            {
               NEXUS_VideoDecoderOpenMosaicSettings   videoDecoderOpenMosaicSettings;
               NEXUS_VideoDecoder_GetDefaultOpenMosaicSettings(&videoDecoderOpenMosaicSettings);
               videoDecoderOpenMosaicSettings.maxWidth = m_sourceWidth;
               videoDecoderOpenMosaicSettings.maxHeight = m_sourceHeight;
               m_videoDecoder = NEXUS_VideoDecoder_OpenMosaic(0, m_globalStreamIndex, &videoDecoderOpenMosaicSettings);
               if (m_videoDecoder == NULL)
                  BSG_THROW("NEXUS_VideoDecoder_OpenMosaic failed");
            }
            else
            {
               NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
               NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);

#ifdef NEXUS_VIDEO_SECURE_HEAP
               if (Platform::Instance()->GetOptions().GetSecure())
               {
                  NEXUS_PlatformConfiguration platformConfig;
                  NEXUS_Platform_GetConfiguration(&platformConfig);
                  videoDecoderOpenSettings.secureVideo = Platform::Instance()->GetOptions().GetSecure();
                  videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
               }
#endif

               m_videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
               if (m_videoDecoder == NULL)
                  BSG_THROW("NEXUS_VideoDecoder_Open failed");
            }

            if (m_videoWindow != NULL)
            {
               if (outputInfo.m_mode == eMOSAIC)
               {
                  NEXUS_VideoWindowSettings windowSettings;

                  m_mosaicVideoWindow = NEXUS_VideoWindow_OpenMosaic(m_videoWindow, m_globalStreamIndex);
                  NEXUS_VideoWindow_AddInput(m_mosaicVideoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));

                  NEXUS_VideoWindow_GetSettings(m_mosaicVideoWindow, &windowSettings);
                  windowSettings.position.x = 0;
                  windowSettings.position.y = 0;
                  windowSettings.position.width = 0;
                  windowSettings.position.height = 0;
                  windowSettings.visible = false;

                  if (m_fullScreenStreamIndex == (int32_t)m_globalStreamIndex)
                  {
                     NEXUS_GraphicsSettings settings;
                     NEXUS_Display_GetGraphicsSettings(nexusDisplay, &settings);
                     windowSettings.position.x = settings.position.x;
                     windowSettings.position.y = settings.position.y;
                     windowSettings.position.width = settings.position.width;
                     windowSettings.position.height = settings.position.height;
                     windowSettings.visible = true;
                  }
                  NEXUS_VideoWindow_SetSettings(m_mosaicVideoWindow, &windowSettings);
               }
               else
               {
                  NEXUS_VideoWindow_AddInput(m_videoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
               }
            }

            /* Tell Display module to connect to the VideoDecoder module and supply the
            L1 INT id's from BVDC_Display_GetInterrupt. Display will not register for the data ready ISR callback. */
            if (m_globalStreamIndex == 0 && m_videoWindow == NULL)
            {
               NEXUS_Display_ConnectVideoInput(m_nexusDisplay, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
            }
            else
            {
               // Comment from decode_mosaic_video_via_graphics.c
               /* For a main video-as-graphics window, we call NEXUS_Display_ConnectVideoInput to connect
                  display back to video decoder. However, for any secondary window (e.g. PIP, second mosaic),
                  we cannot call NEXUS_Display_ConnectVideoInput. The main window will have already made the
                  connection. Instead, call NEXUS_VideoDecoder_SetPowerState with true so that the XVD
                  channel is created */
               NEXUS_VideoDecoder_SetPowerState(m_videoDecoder, true);
            }

            // StcChannel
            NEXUS_StcChannelSettings stcSettings;
            NEXUS_StcChannel_GetDefaultSettings(m_globalStreamIndex, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            stcSettings.stcIndex = m_streamIndex;           // actual stream index in the video file
            m_stcChannel = NEXUS_StcChannel_Open(m_globalStreamIndex, &stcSettings);
            if (m_stcChannel == NULL)
               BSG_THROW("NEXUS_StcChannel_Open failed");

            // Setting the playback
            NEXUS_PlaybackSettings playbackSettings;
            NEXUS_Playback_GetSettings(m_filePlayback.GetPlayback(), &playbackSettings);
            playbackSettings.playpumpSettings.transportType = mediaData.m_transportType;
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            playbackSettings.stcChannel = m_stcChannel;
            playbackSettings.stcTrick = m_stcChannel;
            NEXUS_Playback_SetSettings(m_filePlayback.GetPlayback(), &playbackSettings);

            // video
            NEXUS_PlaybackPidChannelSettings pidChannelSettings;
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pidChannelSettings.pidTypeSettings.video.codec = mediaData.m_videoCodec;
            pidChannelSettings.pidTypeSettings.video.decoder       = m_videoDecoder;
            pidChannelSettings.pidTypeSettings.video.index = true;
            m_videoPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(), mediaData.m_videoPid, &pidChannelSettings);
            if (m_videoPidChannel == NULL)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed for video");

            if (outputInfo.m_mode != eMOSAIC || m_fullScreenStreamIndex == (int32_t)m_globalStreamIndex)
            {
               // audio
               NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
               pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
               pidChannelSettings.pidTypeSettings.audio.primary = m_audioDecoder;
               m_audioPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(), mediaData.m_audioPid, &pidChannelSettings);
               if (m_audioPidChannel == NULL)
                  BSG_THROW("NEXUS_Playback_OpenPidChannel failed for audio");
            }

            NEXUS_VideoDecoder_GetDefaultStartSettings(&m_videoProgram);
            m_videoProgram.codec          = mediaData.m_videoCodec;
            m_videoProgram.pidChannel     = m_videoPidChannel;
            m_videoProgram.stcChannel     = m_stcChannel;

#if NEXUS_HAS_AUDIO
            if (outputInfo.m_mode != eMOSAIC || m_fullScreenStreamIndex == (int32_t)m_globalStreamIndex)
            {
               NEXUS_AudioDecoder_GetDefaultStartSettings(&m_audioProgram);
               m_audioProgram.codec          = mediaData.m_audioCodec;
               m_audioProgram.pidChannel     = m_audioPidChannel;
               m_audioProgram.stcChannel     = m_stcChannel;
            }
#endif
         }
         catch (Exception e)
         {
            Cleanup();
            throw e;
         }
      };

      //! Starts decoder and playback
      virtual void StartPlayback()
      {
         if (!m_playStarted)
         {
            m_playStarted = true;

            if (m_videoDecoder)
               if (NEXUS_VideoDecoder_Start(m_videoDecoder, &m_videoProgram) != NEXUS_SUCCESS)
                  printf("Not started video decoder\n");

#if NEXUS_HAS_AUDIO
            if (m_audioDecoder)
               if (NEXUS_AudioDecoder_Start(m_audioDecoder, &m_audioProgram) != NEXUS_SUCCESS)
                  printf("Not started audio decoder\n");
#endif

            m_filePlayback.StartPlayback();
         }
      }

      //! Stops decoder and playback
      virtual void StopPlayback()
      {
         if (m_playStarted)
         {
            m_playStarted = false;

            if (m_videoDecoder)
               NEXUS_VideoDecoder_Stop(m_videoDecoder);

#if NEXUS_HAS_AUDIO
            if (m_audioDecoder)
               NEXUS_AudioDecoder_Stop(m_audioDecoder);
#endif

            m_filePlayback.StopPlayback();
         }
      }

      //! Set audio delay
      virtual void SetAudioDelay(uint32_t ms)
      {
#if NEXUS_HAS_AUDIO
         if (m_audioDecoder)
         {
            NEXUS_PlatformConfiguration platformConfig;
            NEXUS_Platform_GetConfiguration(&platformConfig);
            NEXUS_AudioOutputSettings aoSettings;

#if (NEXUS_NUM_AUDIO_DACS > 0)
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
#endif
         }
#endif
      }

      void DestripeToYV12(NEXUS_StripedSurfaceHandle src, NEXUS_SurfaceHandle dst)
      {
         NEXUS_Error rc;

         // For YV12 testing, we need to bodge up a YV12 format image from what we have.
         // To do that, we'll need a few image transformations.
         NEXUS_StripedSurfaceCreateSettings  srcSettings;
         NEXUS_SurfaceCreateSettings         settings;
         NEXUS_SurfaceCreateSettings         dstSettings;

         NEXUS_Surface_GetCreateSettings(dst, &dstSettings);

         NEXUS_StripedSurface_GetCreateSettings(src, &srcSettings);
         NEXUS_Surface_GetDefaultCreateSettings(&settings);

         settings.pixelFormat = NEXUS_PixelFormat_eY8;
         settings.width = dstSettings.width;
         settings.height = dstSettings.height;
         settings.pitch = (settings.width + 15) & ~15;

         NEXUS_SurfaceHandle Y8Surf = NEXUS_Surface_Create(&settings);

         settings.pixelFormat = NEXUS_PixelFormat_eCr8;
         settings.width /= 2;
         settings.height /= 2;
         settings.pitch = (settings.pitch / 2 + 15) & ~15;

         NEXUS_SurfaceHandle CrSurf = NEXUS_Surface_Create(&settings);

         settings.pixelFormat = NEXUS_PixelFormat_eCb8;

         NEXUS_SurfaceHandle CbSurf = NEXUS_Surface_Create(&settings);

         if (Y8Surf != NULL && CrSurf != NULL && CbSurf != NULL)
         {
            NEXUS_Graphics2D_DestripeToSurface(m_gfx2d, src, Y8Surf, NULL);
            NEXUS_Graphics2D_DestripeToSurface(m_gfx2d, src, CrSurf, NULL);
            NEXUS_Graphics2D_DestripeToSurface(m_gfx2d, src, CbSurf, NULL);

            do
            {
               rc = NEXUS_Graphics2D_Checkpoint(m_gfx2d, NULL);
               if (rc == NEXUS_GRAPHICS2D_QUEUED)
                  rc = BKNI_WaitForEvent(m_gfxDone, 1000);
            }
            while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

            NEXUS_SurfaceMemory  dstMem, Y8Mem, CrMem, CbMem;
            NEXUS_Surface_GetMemory(dst, &dstMem);
            NEXUS_Surface_GetMemory(Y8Surf, &Y8Mem);
            NEXUS_Surface_GetMemory(CrSurf, &CrMem);
            NEXUS_Surface_GetMemory(CbSurf, &CbMem);

            NEXUS_Surface_Flush(Y8Surf);
            NEXUS_Surface_Flush(CrSurf);
            NEXUS_Surface_Flush(CbSurf);

            // This is creating YCrCb buffer (matching Android YV12 format)
            memcpy(dstMem.buffer, Y8Mem.buffer, Y8Mem.bufferSize);
            memcpy((uint8_t*)dstMem.buffer + Y8Mem.bufferSize, CrMem.buffer, CrMem.bufferSize);
            memcpy((uint8_t*)dstMem.buffer + Y8Mem.bufferSize + CrMem.bufferSize, CbMem.buffer, CbMem.bufferSize);

            NEXUS_Surface_Flush(dst);
         }

         if (Y8Surf != NULL)
            NEXUS_Surface_Destroy(Y8Surf);
         if (CrSurf != NULL)
            NEXUS_Surface_Destroy(CrSurf);
         if (CbSurf != NULL)
            NEXUS_Surface_Destroy(CbSurf);
      }

      //! Updates the current texture with a new frame if there is one available
      //! @param mode specifies if the function blocks while waiting for a new available video frame
      //! @param updateMode specifies the synchronisation used to update the texture
      virtual VideoDecoder::eFrameStatus UpdateFrame(VideoDecoder::eMode mode, eVideoUpdateMode updateMode)
      {
         NEXUS_StripedSurfaceHandle stripedSurface = NULL;

         if (mode != VideoDecoder::BLOCK)
         {
            // We may not have a new video frame ready to use, in which case NEXUS_VideoDecoder_CreateStripedSurface
            // will return NULL. Check now so we can avoid any destination surface lock if we don't yet have a buffer.
            stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(m_videoDecoder);
            if (stripedSurface == NULL)
            {
               // No more frames ready yet. Pixmap should still have valid data from last time.
               return VideoDecoder::eFRAME_REPEAT;
            }
         }

         // A frame is available (in no block mode) and if there is a destination surface
         NativePixmap::ePixmapFormat pixmapFormat = m_pixmaps[m_curbufferIndex.Plus1()]->GetFormat();

         NEXUS_SurfaceHandle dstSurface = ((NexusPixmapData*)m_pixmaps[m_curbufferIndex.Plus1()]->GetNativePixmapData())->m_surface;
         if (dstSurface)
         {
            bool waited_for_texture_buffer = false;
            m_curbufferIndex.Increment();

            // Wait for the surface to be out of the pipeline
            if (updateMode == eEGL_SYNC && m_videoTexFences[m_curbufferIndex].Valid())
            {
               // If it is not signaled already
               if (!m_videoTexFences[m_curbufferIndex].Signaled())
               {
                  // wait for the texture to be out of the pipeline
                  m_videoTexFences[m_curbufferIndex].Wait();
                  waited_for_texture_buffer = true;
               }
            }

            // Lock the destination buffer for writing. This might take some time if the 3D core is
            // using it right now.
            if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
               m_videoTex[m_curbufferIndex]->Lock();

            if (mode == VideoDecoder::BLOCK)
            {
               // Wait for a new video frame
               while (stripedSurface == NULL)
                  stripedSurface = NEXUS_VideoDecoder_CreateStripedSurface(m_videoDecoder);
            }
            else
            {
               BDBG_ASSERT(stripedSurface != NULL);

               // We MUST check for another new frame now, even if we got a surface prior to the lock.
               // The video buffer chain may have looped around and we could end up using a surface
               // that's being written into otherwise, resulting in nasty tearing.
               NEXUS_StripedSurfaceHandle nextStripe = NEXUS_VideoDecoder_CreateStripedSurface(m_videoDecoder);
               if (nextStripe == NULL)
               {
                  if (waited_for_texture_buffer)
                  {
                     // If we have waited and don't have a new frame we need to repeat the previous frame as the video
                     // frame retrieved before the wait might be already over written
                     NEXUS_VideoDecoder_DestroyStripedSurface(m_videoDecoder, stripedSurface);

                     // Send the previous texture down the pipeline not the one that has not been updated
                     m_curbufferIndex.Decrement();

                     return VideoDecoder::eFRAME_REPEAT;
                  }
                  // else
                  // We can use the striped surface we got before the lock/wait as there isn't a newer one.
               }
               else
               {
                  // Use the newest surface (nextStripe), but first destroy the buffer we got before the lock (if we got one)
                  NEXUS_VideoDecoder_DestroyStripedSurface(m_videoDecoder, stripedSurface);

                  stripedSurface = nextStripe;
               }
            }

            // Now destripe the surface into our destination buffer
            NEXUS_Error rc;

            if (pixmapFormat == NativePixmap::eYV12_TEXTURE)
            {
               // YV12 hackery
               DestripeToYV12(stripedSurface, dstSurface);
            }
            else
            {
               rc = NEXUS_Graphics2D_DestripeToSurface(m_gfx2d, stripedSurface, dstSurface, NULL);
               BDBG_ASSERT(!rc);
            }

            // We must wait for the destripe to complete now
            do
            {
               rc = NEXUS_Graphics2D_Checkpoint(m_gfx2d, NULL);
               if (rc == NEXUS_GRAPHICS2D_QUEUED)
                  rc = BKNI_WaitForEvent(m_gfxDone, 1000);
            }
            while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

            // We're done with the striped surface, so kill it.
            NEXUS_VideoDecoder_DestroyStripedSurface(m_videoDecoder, stripedSurface);

            // Tell V3D we've changed it
            if (updateMode == eEXT_SYNC || updateMode == eEXT)
               m_videoTex[m_curbufferIndex]->SetUpdatedRegion(0, 0, m_outputWidth, m_outputHeight);

            // Unlock the image so V3D can use it.
            if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
               m_videoTex[m_curbufferIndex]->Unlock();

            return VideoDecoder::eFRAME_NEW;
         }

         return VideoDecoder::eFRAME_ERROR;
      };

      //! Function for Mosaic mode
      //! Updates the current texture using a striped surface passed an argument
      //! @param updateMode specifies the synchronisation used to update the texture
      //! @param stripedSurfaces surface to destripe
      virtual VideoDecoder::eFrameStatus UpdateFrame(eVideoUpdateMode updateMode, NEXUS_StripedSurfaceHandle stripedSurfaces)
      {
         // If there is a destination surface
         NEXUS_SurfaceHandle dstSurface = ((NexusPixmapData*)m_pixmaps[m_curbufferIndex.Plus1()]->GetNativePixmapData())->m_surface;
         if (dstSurface)
         {
            m_curbufferIndex.Increment();

            // Lock the destination buffer for writing. This might take some time if the 3D core is
            // using it right now.
            if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
               m_videoTex[m_curbufferIndex]->Lock();

            // If the striped surface actually exist
            if (stripedSurfaces == NULL)
            {
               // Still no striped surface ready. Unlock and return.
               if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
                  m_videoTex[m_curbufferIndex]->Unlock();

               m_curbufferIndex.Decrement();

               return VideoDecoder::eFRAME_REPEAT;
            }

            // Wait for the surface to be out of the pipeline
            if (updateMode == eEGL_SYNC && m_videoTexFences[m_curbufferIndex].Valid())
               m_videoTexFences[m_curbufferIndex].Wait();

            // Now destripe the surface into our destination buffer
            NEXUS_Error rc;
            rc = NEXUS_Graphics2D_DestripeToSurface(m_gfx2d, stripedSurfaces, dstSurface, NULL);
            BDBG_ASSERT(!rc);

            // We must wait for the destripe to complete now
            do
            {
               rc = NEXUS_Graphics2D_Checkpoint(m_gfx2d, NULL);
               if (rc == NEXUS_GRAPHICS2D_QUEUED)
                  rc = BKNI_WaitForEvent(m_gfxDone, 1000);
            }
            while (rc == NEXUS_GRAPHICS2D_QUEUE_FULL);

            // Tell V3D we've changed it
            if (updateMode == eEXT_SYNC || updateMode == eEXT)
               m_videoTex[m_curbufferIndex]->SetUpdatedRegion(0, 0, m_outputWidth, m_outputHeight);

            // Unlock the image so V3D can use it.
            if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
               m_videoTex[m_curbufferIndex]->Unlock();

            return VideoDecoder::eFRAME_NEW;
         }
         else
         {
            return VideoDecoder::eFRAME_ERROR;
         }
      };

      //! Returns a handle to the last updates texture
      virtual TextureHandle &GetCurrentTexture() { return m_videoTex[m_curbufferIndex]; };

      //! Returns a handle to the last updates pixmap
      virtual NativePixmap *GetCurrentPixmap() { return m_pixmaps[m_curbufferIndex]; };

      //! Moves the current index to the next buffer
      virtual void ChangeCurrentBuffer() { m_curbufferIndex.Increment(); };

      //! Returns the decoder for the stream
      virtual NEXUS_VideoDecoderHandle GetVideoDecoder() { return m_videoDecoder; };


   private:
      NEXUS_VideoDecoderHandle                     m_videoDecoder;
      NEXUS_VideoDecoderStartSettings              m_videoProgram;
      NEXUS_AudioDecoderHandle                     m_audioDecoder;
      NEXUS_AudioDecoderStartSettings              m_audioProgram;
      NEXUS_VideoWindowHandle                      m_videoWindow;
      NEXUS_VideoWindowHandle                      m_mosaicVideoWindow;
      int32_t                                      m_fullScreenStreamIndex;
   };

#endif // #ifdef SINGLE_PROCESS

   /////////////////////////////////////////////////////////////////////////////////
   //          NexusVideoStreamMultiProcess
   /////////////////////////////////////////////////////////////////////////////////
#ifndef SINGLE_PROCESS

   static const uint32_t MAX_CURRENT_CAPTURES = 2;

   class CaptureFrame
   {
   public:
      CaptureFrame() :
         m_surface(NULL),
         m_textureIndex(0)
      {

      }

      NEXUS_SurfaceHandle  m_surface;
      uint32_t             m_textureIndex;
   };


   class NexusVideoStreamMultiProcess : public NexusVideoStream
   {
   public:

      //! Default constructor
      NexusVideoStreamMultiProcess(NexusVideoFilePlayback &filePlayback, uint32_t fileStreamIndex) :
         NexusVideoStream(filePlayback, fileStreamIndex),
         m_videoDecoder(NULL),
         m_audioDecoder(NULL),
         m_connectId(0),
         m_numberOfCaptures(0)
      {
         const MediaData & mediaData = m_filePlayback.GetMediaData(m_streamIndex);

         m_sourceWidth  = mediaData.m_width;
         m_sourceHeight = mediaData.m_height;
      };

      //! Destructor
      virtual ~NexusVideoStreamMultiProcess()
      {
         Cleanup();
      };

      //! Clean up the member variables
      virtual void Cleanup()
      {
         /* Clear out the EGL images */
         for (uint32_t i = 0; i < m_videoTex.size(); ++i)
            m_videoTex[i].Delete();

         m_videoTex.clear();

         for (uint32_t bufferIndex = 0; bufferIndex < m_pixmaps.size(); bufferIndex++)
         {
            if (m_pixmaps[bufferIndex])
               delete m_pixmaps[bufferIndex];
         }

         m_pixmaps.clear();

         StopPlayback();

         if (m_videoPidChannel)
         {
            NEXUS_Playback_ClosePidChannel(m_filePlayback.GetPlayback(), m_videoPidChannel);
            m_videoPidChannel = NULL;
         }

         if (m_audioPidChannel)
         {
            NEXUS_Playback_ClosePidChannel(m_filePlayback.GetPlayback(), m_audioPidChannel);
            m_audioPidChannel = NULL;
         }

         if (m_stcChannel)
         {
            NEXUS_StcChannel_Close(m_stcChannel);
            m_stcChannel = NULL;
         }

         if (m_connectId)
               NxClient_Disconnect(m_connectId);

         if (m_videoDecoder)
         {
            NEXUS_SimpleVideoDecoder_Release(m_videoDecoder);
            m_videoDecoder = NULL;
         }

         if (m_audioDecoder)
         {
            NEXUS_SimpleAudioDecoder_Release(m_audioDecoder);
            m_audioDecoder = NULL;
         }

         NxClient_Free(&m_allocResults);
      };

      //! Creates the necessary buffers and configure the decoder
      //! @param widthOutput is the width of the output frame
      //! @param heightOutput is the height of the output frame
      //! @param pixmapFormat is the format used to create the pixmap
      //! @param textureMode is the type of texture used
      //! @param numDecodeBuffers is the number of buffers created of each type
      virtual void Configure(const VideoOutputInformation &outputInfo, uint32_t numDecodeBuffers,
                           NEXUS_Graphics2DHandle gfx2d, BKNI_EventHandle gfxDone, NEXUS_DisplayHandle nexusDisplay)
      {
         try
         {
            // Get the stream information from the video file
            const MediaData & mediaData = m_filePlayback.GetMediaData(m_streamIndex);

            m_lastCaptureIndex.Setup(0, MAX_CURRENT_CAPTURES-1);

            m_nexusDisplay = nexusDisplay;

            m_gfxDone = gfxDone;
            m_gfx2d = gfx2d;

            Cleanup();

            // Creates a number of pixmap and wraps them into video buffers
            CreateBuffers(outputInfo.m_widthOutput, outputInfo.m_heightOutput,
                           outputInfo.m_pixmapFormat, outputInfo.m_textureMode, numDecodeBuffers);

            if (outputInfo.m_mode == eMOSAIC)
               BSG_THROW("Mosaic mode not supported in multiprocess");

            /*Allocate the number of video streams / decoders*/
            NxClient_AllocSettings allocSettings;
            NxClient_GetDefaultAllocSettings(&allocSettings);
            allocSettings.simpleVideoDecoder = 1;
            allocSettings.simpleAudioDecoder = 1;

            NEXUS_Error rc;
            rc = NxClient_Alloc(&allocSettings, &m_allocResults);
            if (rc != NEXUS_SUCCESS)
               BSG_THROW("Nxclient simple decoder allocations failed");

            if (m_allocResults.simpleVideoDecoder[0].id)
               m_videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(m_allocResults.simpleVideoDecoder[0].id);

            if (m_allocResults.simpleAudioDecoder.id)
               m_audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(m_allocResults.simpleAudioDecoder.id);

            if (m_videoDecoder == NULL)
               BSG_THROW("Acquire Nxclient video decoder failed");

            if (m_audioDecoder == NULL)
               BSG_THROW("Acquire Nxclient audio decoder failed");

            NEXUS_StcChannelSettings               stcSettings;
            NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
            stcSettings.timebase = NEXUS_Timebase_e0;
            stcSettings.mode = NEXUS_StcChannelMode_eAuto;
            m_stcChannel = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
            if (m_stcChannel == NULL)
               BSG_THROW("NEXUS_StcChannel_Open failed");

            /* setup playback */
            NEXUS_PlaybackSettings playbackSettings;
            NEXUS_Playback_GetSettings(m_filePlayback.GetPlayback(), &playbackSettings);
            playbackSettings.playpumpSettings.transportType = mediaData.m_transportType;
            playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
            playbackSettings.stcChannel = m_stcChannel;
            NEXUS_Playback_SetSettings(m_filePlayback.GetPlayback(), &playbackSettings);

            /* video */
            NEXUS_PlaybackPidChannelSettings pidChannelSettings;
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eVideo;
            pidChannelSettings.pidTypeSettings.video.codec = mediaData.m_videoCodec;
            pidChannelSettings.pidTypeSettings.video.simpleDecoder = m_videoDecoder;
            pidChannelSettings.pidTypeSettings.video.index = true;
            m_videoPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(), mediaData.m_videoPid, &pidChannelSettings);
            if (m_videoPidChannel == NULL)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed for video");

            // audio
            NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
            pidChannelSettings.pidSettings.pidType = NEXUS_PidType_eAudio;
            pidChannelSettings.pidTypeSettings.audio.simpleDecoder = m_audioDecoder;
            m_audioPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(), mediaData.m_audioPid, &pidChannelSettings);
            if (m_audioPidChannel == NULL)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed for audio");

            NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&m_videoProgram);
            m_videoProgram.settings.codec      = mediaData.m_videoCodec;
            m_videoProgram.settings.pidChannel = m_videoPidChannel;
            m_videoProgram.settings.stcChannel = m_stcChannel;
            m_videoProgram.displayEnabled      = false;

#if NEXUS_HAS_AUDIO
            NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&m_audioProgram);
            m_audioProgram.primary.codec      = mediaData.m_audioCodec;
            m_audioProgram.primary.pidChannel = m_audioPidChannel;
            m_audioProgram.primary.stcChannel = m_stcChannel;
#endif

            /* connect client resources to server's resources */
            NxClient_ConnectSettings            connectSettings;
            NxClient_GetDefaultConnectSettings(&connectSettings);

            if (m_videoProgram.settings.pidChannel)
            {
               connectSettings.simpleAudioDecoder.id = m_allocResults.simpleAudioDecoder.id;
               connectSettings.simpleVideoDecoder[0].id = m_allocResults.simpleVideoDecoder[0].id;
               connectSettings.simpleVideoDecoder[0].windowId = 0;
               connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth = m_sourceWidth;
               connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = m_sourceHeight;
            }

            connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = Platform::Instance()->GetOptions().GetSecure();

            rc = NxClient_Connect(&connectSettings, &m_connectId);
            if (rc != NEXUS_SUCCESS)
               BSG_THROW("NEXUS_Playback_OpenPidChannel failed");

            NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&m_videoCaptureSettings);
            m_videoCaptureSettings.displayEnabled = true;
            m_videoCaptureSettings.secure         = Platform::Instance()->GetOptions().GetSecure();

            for (unsigned index=0; index <= m_curbufferIndex.GetMax(); index++)
            {
               m_videoCaptureSettings.surface[index] = ((NexusPixmapData*)m_pixmaps[index]->GetNativePixmapData())->m_surface;
            }
         }
         catch (Exception e)
         {
            Cleanup();
            throw e;
         }
      };

      //! Starts decoder and playback
      virtual void StartPlayback()
      {
         if ((m_videoDecoder) && (!m_playStarted))
         {
            m_playStarted = true;

            // Start decode
            NEXUS_SimpleVideoDecoder_Start(m_videoDecoder, &m_videoProgram);
            NEXUS_SimpleVideoDecoder_StartCapture(m_videoDecoder, &m_videoCaptureSettings);

            if (m_audioDecoder)
               if (NEXUS_SimpleAudioDecoder_Start(m_audioDecoder, &m_audioProgram) != NEXUS_SUCCESS)
                  printf("Not started audio decoder\n");

            // start playback
            m_filePlayback.StartPlayback();
         }
      }

      //! Stops decoder and playback
      virtual void StopPlayback()
      {
         if ((m_videoDecoder) && (m_playStarted))
         {
            m_playStarted = false;

            m_filePlayback.StopPlayback();

            NEXUS_SimpleVideoDecoder_StopCapture(m_videoDecoder);
            NEXUS_SimpleVideoDecoder_Stop(m_videoDecoder);

            if (m_audioDecoder)
               NEXUS_SimpleAudioDecoder_Stop(m_audioDecoder);
         }
      }

      //! Set audio delay
      virtual void SetAudioDelay(uint32_t ms)
      {
/*
         if (m_audioDecoder)
         {
            NEXUS_PlatformConfiguration platformConfig;
            NEXUS_Platform_GetConfiguration(&platformConfig);

            NEXUS_AudioOutputSettings aoSettings;
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);

            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);

#if NEXUS_NUM_HDMI_OUTPUTS
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
#endif
         }
*/
      }

      //! Updates the current texture with a new frame if there is one available
      //! @param mode specifies if the function blocks while waiting for a new available video frame
      //! @param updateMode specifies the synchronisation used to update the texture
      virtual VideoDecoder::eFrameStatus UpdateFrame(VideoDecoder::eMode mode, eVideoUpdateMode updateMode)
      {
         unsigned                               numReturned = 0;        // Number of frames captured
         NEXUS_SurfaceHandle                    captureSurfaces[NEXUS_SIMPLE_DECODER_MAX_SURFACES];
         NEXUS_SimpleVideoDecoderCaptureStatus  captureStatus;

         if (mode != VideoDecoder::BLOCK)
         {
            // We may not have a new video frame ready to use, in which case numReturned = 0 from
            // NEXUS_SimpleVideoDecoder_GetCapturedSurfaces. Check now so we can avoid any destination surface lock if we don't yet have a buffer.
            // Try to capture N frames from the simple decoder
            NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(m_videoDecoder, captureSurfaces, &captureStatus, NEXUS_SIMPLE_DECODER_MAX_SURFACES, &numReturned);

            // If not frame available
            if (numReturned == 0)
               return VideoDecoder::eFRAME_REPEAT;
         }
         else
         {
            // Wait for a new video frame
            while (numReturned == 0)
               NEXUS_SimpleVideoDecoder_GetCapturedSurfaces(m_videoDecoder, captureSurfaces, &captureStatus, NEXUS_SIMPLE_DECODER_MAX_SURFACES, &numReturned);
         }

         // Queue the most recent frame
         m_curbufferIndex.Set(GetSurfaceIndex(captureSurfaces[numReturned-1]));
         m_lastCaptureIndex.Increment();
         m_numberOfCaptures++;
         m_captureQueue[m_lastCaptureIndex].m_textureIndex = m_curbufferIndex;
         m_captureQueue[m_lastCaptureIndex].m_surface = captureSurfaces[numReturned-1];

         // Check if a captured frame need to be Lock and recycled
         if (MAX_CURRENT_CAPTURES <= m_numberOfCaptures)
         {
            // Lock it before to give it back to the decoder
            if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
               m_videoTex[m_captureQueue[m_lastCaptureIndex.Minus1()].m_textureIndex]->Lock();

            if (updateMode == eEGL_SYNC && m_videoTexFences[m_captureQueue[m_lastCaptureIndex.Minus1()].m_textureIndex].Valid())
               m_videoTexFences[m_captureQueue[m_lastCaptureIndex.Minus1()].m_textureIndex].Wait(); // Wait for this surface to be out of the pipeline

            // Recycle the surface to be used by the decoder
            NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(m_videoDecoder, &m_captureQueue[m_lastCaptureIndex.Minus1()].m_surface, 1);

            // Remove the capture from the queue
            m_numberOfCaptures--;
         }

         // Recycle all the surfaces returned by the decoder except the most recent one
         if (numReturned > 1)
            // Recycle the surface to be used by the decoder
            NEXUS_SimpleVideoDecoder_RecycleCapturedSurfaces(m_videoDecoder, captureSurfaces, numReturned-1);

         /* Tell V3D we've changed it */
         if (updateMode == eEXT_SYNC || updateMode == eEXT)
            m_videoTex[m_curbufferIndex]->SetUpdatedRegion(0, 0, m_outputWidth, m_outputHeight);

         /* Unlock the image so V3D can use it. */
         if (updateMode == eEXT_SYNC || updateMode == eALWAYS_SYNC)
            m_videoTex[m_curbufferIndex]->Unlock();

         return VideoDecoder::eFRAME_NEW;
      };

      //! Function for Mosaic mode
      //! Updates the current texture using a striped surface passed an argument
      //! @param updateMode specifies the synchronisation used to update the texture
      //! @param stripedSurfaces surface to destripe
      virtual VideoDecoder::eFrameStatus UpdateFrame(eVideoUpdateMode updateMode, NEXUS_StripedSurfaceHandle stripedSurfaces)
      {
         return VideoDecoder::eFRAME_REPEAT;
      }

      virtual TextureHandle &GetCurrentTexture() { return m_videoTex[m_curbufferIndex]; };

      virtual NativePixmap *GetCurrentPixmap() { return m_pixmaps[m_curbufferIndex]; };

      virtual void ChangeCurrentBuffer() { m_curbufferIndex.Increment(); };

      //! Returns the decoder for the stream
      virtual NEXUS_SimpleVideoDecoderHandle GetSimpleVideoDecoder() { return m_videoDecoder; };
//      virtual NEXUS_VideoDecoderHandle GetVideoDecoder() { return m_videoDecoder; };

   private:
      // Private functions

      //! Helper function returning the buffer index of the surface
      //! according to its address
      //! @param surface is the handle of the surface to find the index for
      uint32_t GetSurfaceIndex(NEXUS_SurfaceHandle surface)
      {
         for (uint32_t surfaceIndex = 0; surfaceIndex <= m_curbufferIndex.GetMax(); ++surfaceIndex)
         {
            if (surface == ((NexusPixmapData*)m_pixmaps[surfaceIndex]->GetNativePixmapData())->m_surface)
               return surfaceIndex;
         }
         BSG_THROW("Surface Not found");
         return 0;
      }

      // Private members

      NEXUS_SimpleVideoDecoderHandle               m_videoDecoder;
      NEXUS_SimpleAudioDecoderHandle               m_audioDecoder;
      unsigned                                     m_connectId;
      NxClient_AllocResults                        m_allocResults;
      NEXUS_SimpleVideoDecoderStartSettings        m_videoProgram;
      NEXUS_SimpleAudioDecoderStartSettings        m_audioProgram;
      NEXUS_SimpleVideoDecoderStartCaptureSettings m_videoCaptureSettings;

      CaptureFrame                                 m_captureQueue[MAX_CURRENT_CAPTURES];
      CircularIndex                                m_lastCaptureIndex;
      uint32_t                                     m_numberOfCaptures;

   };
#endif // #ifndef SINGLE_PROCESS


   /////////////////////////////////////////////////////////////////////////////////
   //          NexusVideoDecoderPrivate
   /////////////////////////////////////////////////////////////////////////////////

   VideoDecoderPrivate::~VideoDecoderPrivate()
   {

   }

   class NexusVideoDecoderPrivate : public VideoDecoderPrivate
   {
   public:
      //! Constructor
      NexusVideoDecoderPrivate(const std::vector <std::string> &filePaths, int32_t fullScreenVideoIndex, NEXUS_DisplayHandle nexusDisplay) :
         m_decodeFormat(NONE),
         m_numDecodeBuffers(0),
         m_numStreams(0),
         m_audioDelay(0),
         m_gfxDone(NULL),
         m_gfx2d(NULL),
         m_videoWindow(NULL),
         m_fullScreenVideoIndex(fullScreenVideoIndex)
      {
            // Get media data from the files
            MediaProber                   *prober = MediaProber::Instance();
            // Extract all the streams from the video file
            bool justFirstStream = false;
            std::vector<MediaData> mediaDataList;
            // Going through all the video files
            for (uint32_t fileIndex = 0; fileIndex < filePaths.size(); ++fileIndex)
            {
               mediaDataList.clear();

               // One or multiple streams per files
               prober->GetStreamData(filePaths[fileIndex], mediaDataList, justFirstStream);

               m_numStreams += mediaDataList.size();

               m_nexusFilePlaybacklist.push_back(new NexusVideoFilePlayback(mediaDataList));
            }

#ifdef SINGLE_PROCESS

            if (fullScreenVideoIndex != -1)
               m_videoWindow = NEXUS_VideoWindow_Open(nexusDisplay, 0);

            m_numDecodeBuffers = 2;

            // Go through all the files
            //    For each file go through all the stream within that file
            //       Create a new stream
            m_streams.resize(m_numStreams);
            uint32_t streamIndex = 0;
            for (uint32_t fileIndex =0; fileIndex < m_nexusFilePlaybacklist.size(); ++fileIndex)
            {
               for (uint32_t fileStreamIndex = 0; fileStreamIndex < m_nexusFilePlaybacklist[fileIndex]->GetNumberStreams(); ++fileStreamIndex)
               {
                  m_streams[streamIndex] = new NexusVideoStreamSingleProcess(*m_nexusFilePlaybacklist[fileIndex], fileStreamIndex,
                                                                             m_videoWindow, fullScreenVideoIndex);
                  streamIndex++;
               }
            }
#else
            m_numDecodeBuffers = NEXUS_SIMPLE_DECODER_MAX_SURFACES;

            m_streams.resize(m_numStreams);
            uint32_t streamIndex = 0;
            for (uint32_t fileIndex =0; fileIndex < m_nexusFilePlaybacklist.size(); ++fileIndex)
            {
               for (uint32_t fileStreamIndex = 0; fileStreamIndex < m_nexusFilePlaybacklist[fileIndex]->GetNumberStreams(); ++fileStreamIndex)
               {
                  printf("streamIndex: %d, fileStreamIndex: %d, fileIndex: %d\n", streamIndex, fileStreamIndex,fileIndex);
                  m_streams[streamIndex] = new NexusVideoStreamMultiProcess(*m_nexusFilePlaybacklist[fileIndex], fileStreamIndex);
                  streamIndex++;
               }
            }

#endif
      }

      //! Destructor
      virtual ~NexusVideoDecoderPrivate()
      {
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

         for (uint32_t streamIndex = 0; streamIndex < m_numStreams; ++streamIndex)
         {
            delete m_streams[streamIndex];
         }

         m_streams.clear();

         if (m_videoWindow)
         {
            NEXUS_VideoWindow_Close(m_videoWindow);
            m_videoWindow = NULL;
         }

         for (uint32_t fileIndex = 0; fileIndex < m_nexusFilePlaybacklist.size(); fileIndex++)
         {
            delete m_nexusFilePlaybacklist[fileIndex];
         }

         m_nexusFilePlaybacklist.clear();
      }

      //! Configures the decoder for each stream
      //! @param decodeFormat requested format for decoding the video
      //! @param widthOutput is the width of the output frame
      //! @param heightOutput is the height of the output frame
      //! @param textureMode is the type of texture used
      //! @param nexusDisplay
      //! @param fullScreenVideoStreamIndex Index of stream to play full screen, or -1 for none
      virtual void Configure(eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                     GLTexture::eVideoTextureMode textureMode, uint32_t numBuffers, NEXUS_DisplayHandle nexusDisplay)
      {
         try
         {
            m_decodeFormat = decodeFormat;

            if (m_gfx2d == NULL)
            {
               // Create an evemt to signal the end of a 2D graphics task
               BKNI_CreateEvent(&m_gfxDone);

               NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;

               NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
               graphics2dOpenSettings.secure = Platform::Instance()->GetOptions().GetSecure();

               // Open the 2D graphics hardware
               NEXUS_Graphics2DSettings    gfxSettings;
               m_gfx2d = NEXUS_Graphics2D_Open(0, &graphics2dOpenSettings);
               if (m_gfx2d == NULL)
                  BSG_THROW("NEXUS_Graphics2D_Open failed");

               // Setting up the event signaling
               NEXUS_Graphics2D_GetSettings(m_gfx2d, &gfxSettings);
               gfxSettings.checkpointCallback.callback = complete;
               gfxSettings.checkpointCallback.context = m_gfxDone;
               NEXUS_Graphics2D_SetSettings(m_gfx2d, &gfxSettings);
            }

            NexusVideoStream::eVideoMode  mode = NexusVideoStream::eMOSAIC;
            if (m_numStreams == 1)
               mode = NexusVideoStream::eSINGLE;
            else
               mode = NexusVideoStream::eMOSAIC;

            NativePixmap::ePixmapFormat   pixmapFormat = NativePixmap::eYUV422_TEXTURE;
            switch (m_decodeFormat)
            {
            case eYUV422   : pixmapFormat = NativePixmap::eYUV422_TEXTURE; break;
            case eYUVX444  : pixmapFormat = NativePixmap::eABGR8888_TEXTURE; break;
            case eYV12     : pixmapFormat = NativePixmap::eYV12_TEXTURE; break;
            case eRGB565   : pixmapFormat = NativePixmap::eRGB565_TEXTURE; break;
            case eRGBX8888 : pixmapFormat = NativePixmap::eABGR8888_TEXTURE; break;
            default        :
            case eNONE     : BSG_THROW("Bad decode format"); break;
            }

            VideoOutputInformation outputInfo;
            outputInfo.m_widthOutput      = widthOutput;
            outputInfo.m_heightOutput     = heightOutput;
            outputInfo.m_mode             = mode;
            outputInfo.m_textureMode      = textureMode;
            outputInfo.m_pixmapFormat     = pixmapFormat;

#ifdef SINGLE_PROCESS
            if (numBuffers != 0)
               m_numDecodeBuffers = numBuffers;
#endif

            for (uint32_t streamIndex = 0; streamIndex < m_numStreams; streamIndex++)
            {
               m_streams[streamIndex]->Configure(outputInfo, m_numDecodeBuffers, m_gfx2d, m_gfxDone, nexusDisplay);
            }
         }
         catch (Exception e)
         {
            if (m_gfxDone)
               BKNI_DestroyEvent(m_gfxDone);

            throw e;
         }
      }

      //! Starts decoder and playback
      virtual void StartPlayback()
      {
         for (uint32_t streamIndex = 0; streamIndex < m_numStreams; ++streamIndex)
         {
            m_streams[streamIndex]->StartPlayback();
         }
      }

      //! Stops decoder and playback
      virtual void StopPlayback()
      {
         for (uint32_t streamIndex = 0; streamIndex < m_numStreams; ++streamIndex)
         {
            m_streams[streamIndex]->StopPlayback();
         }
      }

      //! Updates the current texture with a new frame if there is one available
      //! @param mode specifies if the function blocks while waiting for a new available video frame
      //! @param updateMode specifies the synchronisation used to update the texture
      virtual VideoDecoder::eFrameStatus UpdateFrame(VideoDecoder::eMode mode, eVideoUpdateMode updateMode)
      {
         // TODO: check stream index

         return m_streams[0]->UpdateFrame(mode, updateMode);
      }

      //! Updates the current textures with the last decoded video frames (in mosaic mode).
      //! Returns Status to indicate what has happened.
      virtual void UpdateMosaicFrames(eVideoUpdateMode updateMode, std::vector<VideoDecoder::eFrameStatus> &results)
      {
         // Clear all the results
         results.clear();

         std::vector < NEXUS_StripedSurfaceHandle > stripedSurfaces(m_numStreams);

         // Initialiase the striped surfaces to NULL
         // as NEXUS_VideoDecoder_CreateStripedMosaicSurfaces might (doesn't) not do it
         for (uint32_t s = 0; s < m_numStreams; s++)
            stripedSurfaces[s] = NULL;

         // We may not have new video frames ready to use, in which case NEXUS_VideoDecoder_CreateStripedMosaicSurfaces
         // will return all NULL.
         uint32_t numStripedSurfaces = 0;
         NEXUS_VideoDecoder_CreateStripedMosaicSurfaces(m_streams[0]->GetVideoDecoder(), &stripedSurfaces[0],
               m_numStreams, &numStripedSurfaces);

         // The number of return striped surfaces should be equal to the number of streams
         // and the striped surface will be NULL if there is not new video frame
			BDBG_ASSERT (numStripedSurfaces == m_numStreams);

         for (uint32_t streamIndex = 0; streamIndex < m_numStreams; ++streamIndex)
         {
            // Add the default result
            results.push_back(VideoDecoder::eFRAME_REPEAT);

            results[streamIndex] = m_streams[streamIndex]->UpdateFrame(updateMode, stripedSurfaces[streamIndex]);
         }

         // We're done with the striped surface, so kill it.
         NEXUS_VideoDecoder_DestroyStripedMosaicSurfaces(m_streams[0]->GetVideoDecoder(), &stripedSurfaces[0], numStripedSurfaces);

      }

      //! Returns a handle to the last updates texture
      virtual TextureHandle &GetCurrentTexture(uint32_t streamIndex)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->GetCurrentTexture();
      }

      //! Returns a handle to the last updates pixmap
      virtual NativePixmap *GetCurrentPixmap(uint32_t streamIndex)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->GetCurrentPixmap();
      }

      //! Returns the number of video streams. Always 1 in non-mosaic mode.
      virtual uint32_t NumStreams() const { return m_numStreams; };

      //! Returns the width of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceWidth(uint32_t streamIndex = 0)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->SourceWidth();
      };

      //! Returns the height of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceHeight(uint32_t streamIndex = 0)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->SourceHeight();
      };

      //! Returns the width of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestWidth(uint32_t streamIndex = 0)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->DestWidth();
      };

      //! Returns the height of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestHeight(uint32_t streamIndex = 0)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         return m_streams[streamIndex]->DestHeight();
      };

      //! Moves the current index to the next buffer
      virtual void ChangeCurrentBuffer(uint32_t streamIndex)
      {
         if (streamIndex >= m_numStreams)
            BSG_THROW("Index out of range");

         m_streams[streamIndex]->ChangeCurrentBuffer();
      };

      // Returns the number of stream within the video decoder
      virtual uint32_t NumBuffersPerStream() { return m_numDecodeBuffers; };

      // Get the audio stream delay in ms
      virtual uint32_t GetAudioDelay() const { return m_audioDelay; }

      // Set the audio stream delay in ms
      virtual void SetAudioDelay(uint32_t ms)
      {
         m_audioDelay = ms;

         for (uint32_t streamIndex = 0; streamIndex < m_numStreams; ++streamIndex)
         {
            m_streams[streamIndex]->SetAudioDelay(ms);
         }
      }

      virtual void CreateFenceForSyncUpdate(EGLDisplay dpy)
      {
         m_streams[0]->CreateFenceForSyncUpdate(dpy);
      }

   private:
      std::vector< NexusVideoStream * >   m_streams;
      eVideoFrameFormat                   m_decodeFormat;

      std::vector< NexusVideoFilePlayback * >  m_nexusFilePlaybacklist;
      uint32_t                            m_numDecodeBuffers;
      uint32_t                            m_numStreams;
      uint32_t                            m_audioDelay;

      BKNI_EventHandle                    m_gfxDone;
      NEXUS_Graphics2DHandle              m_gfx2d;

      NEXUS_VideoWindowHandle             m_videoWindow;
      int32_t                             m_fullScreenVideoIndex;
   };


   /////////////////////////////////////////////////////////////////////////////////
   //          VideoDecoder
   /////////////////////////////////////////////////////////////////////////////////

   //! Constructor|
   VideoDecoder::VideoDecoder(const std::string &videoFileName, eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                              GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers,
                              bool playFullScreenToo) :
      m_videoDecoderPrivate(NULL),
      m_updateMode(eEXT_SYNC)
   {
      std::vector<std::string> fileNames;

      fileNames.push_back(Application::Instance()->FindResource(videoFileName));

      NEXUS_DisplayHandle nexusDisplay = NULL;
#ifdef SINGLE_PROCESS
      nexusDisplay = ((PlatformDataNexus*)Application::Instance()->m_platform.GetPlatformData())->m_nexusDisplay;
#endif

      m_videoDecoderPrivate = new NexusVideoDecoderPrivate(fileNames, playFullScreenToo ? 0 : -1, nexusDisplay);

      m_updateMode = mode;

      (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->Configure(
            decodeFormat, widthOutput, heightOutput, textureMode, numBuffers, nexusDisplay);

   }

   //! Constructor
   VideoDecoder::VideoDecoder(const std::vector<std::string> &videoFileNames, eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
         GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers, int32_t fullScreenVideoStreamIndex) :
         m_videoDecoderPrivate(NULL),
         m_updateMode(eEXT_SYNC)
   {
      std::vector<std::string> fileNames;

      for (uint32_t i = 0; i < videoFileNames.size(); ++i)
         fileNames.push_back(Application::Instance()->FindResource(videoFileNames[i]));

      NEXUS_DisplayHandle nexusDisplay = NULL;
#ifdef SINGLE_PROCESS
      nexusDisplay = ((PlatformDataNexus*)Application::Instance()->m_platform.GetPlatformData())->m_nexusDisplay;
#endif

      m_videoDecoderPrivate = new NexusVideoDecoderPrivate(fileNames, fullScreenVideoStreamIndex, nexusDisplay);

      m_updateMode = mode;

      (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->Configure(
            decodeFormat, widthOutput, heightOutput, textureMode, numBuffers, nexusDisplay);

   }

   //! Destructor
   VideoDecoder::~VideoDecoder()
   {
      delete m_videoDecoderPrivate;
   }

   //! Used to reconfigure the video decoder and recreate the insternal buffers
   //! @param eVideoFrameFormat is the format to decode the video file
   //! @param widthOutput width of the video buffer used to grab frames - if 0 the size of the video will be used
   //! @param heightOutput height of the video buffer used to grab frames - if 0 the size of the video will be used
   //! @param textureMode describes how the texture is going to be used
   //! @param mode describes the type of synchronisation during texture update
   void VideoDecoder::OutputInitialisation(eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                           GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers)
   {
      m_updateMode = mode;

      NEXUS_DisplayHandle nexusDisplay = NULL;
#ifdef SINGLE_PROCESS
      nexusDisplay = ((PlatformDataNexus*)Application::Instance()->m_platform.GetPlatformData())->m_nexusDisplay;
#endif

      (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->Configure(
            decodeFormat, widthOutput, heightOutput, textureMode, numBuffers, nexusDisplay);

   }

   //! Starts the decoder and the playback
   void VideoDecoder::StartPlayback()
   {
      (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->StartPlayback();
   }

   //! Stops the decoder and the playback
   void VideoDecoder::StopPlayback()
   {
      (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->StopPlayback();
   }

   //! Updates the current texture with a new frame if there is one available
   //! @param mode specifies if the function blocks while waiting for a new available video frame
   //! @param updateMode specifies the synchronisation used to update the texture
   VideoDecoder::eFrameStatus VideoDecoder::UpdateFrame(eMode mode)
   {
      return (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->UpdateFrame(mode, m_updateMode);
   }

   //! Updates the current texture with the last decoded video frames (in mosaic mode).
   //! The video frames will be decoded directly into the given VideoBuffers
   //! Returns Status to indicate what has happened.
   void VideoDecoder::UpdateMosaicFrames(std::vector<eFrameStatus> &results)
   {
      return (static_cast<NexusVideoDecoderPrivate *> (m_videoDecoderPrivate))->UpdateMosaicFrames(m_updateMode, results);
   }

   //! Returns a handle to the last updates texture
   TextureHandle &VideoDecoder::GetCurrentTexture(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->GetCurrentTexture(streamIndex);
   }

   //! Returns a handle to the last updates pixmap
   NativePixmap *VideoDecoder::GetCurrentPixmap(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->GetCurrentPixmap(streamIndex);
   }

   uint32_t VideoDecoder::NumStreams() const
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->NumStreams();
   }

   //! Returns the width of the source video
   uint32_t VideoDecoder::SourceWidth(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->SourceWidth(streamIndex);
   }

   //! Returns the height of the source video
   uint32_t VideoDecoder::SourceHeight(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->SourceHeight(streamIndex);
   }

   //! Returns the width of the output video
   uint32_t VideoDecoder::DestWidth(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->DestWidth(streamIndex);
   }

   //! Returns the width of the output video
   uint32_t VideoDecoder::DestHeight(uint32_t streamIndex)
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->DestHeight(streamIndex);
   }

   //! Moves the current index to the next buffer
   void VideoDecoder::ChangeCurrentBuffer(uint32_t streamIndex)
   {
      (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->ChangeCurrentBuffer(streamIndex);
   }

   // Returns the number of stream within the video decoder
   uint32_t VideoDecoder::NumBuffersPerStream()
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->NumBuffersPerStream();
   }

   uint32_t VideoDecoder::GetAudioDelay() const
   {
      return (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->GetAudioDelay();
   }

   void VideoDecoder::SetAudioDelay(uint32_t ms)
   {
      (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->SetAudioDelay(ms);
   }

   void VideoDecoder::CreateFenceForSyncUpdate(EGLDisplay dpy)
   {
      (static_cast<NexusVideoDecoderPrivate * > (m_videoDecoderPrivate))->CreateFenceForSyncUpdate(dpy);
   }
}

#endif // BSG_STAND_ALONE
