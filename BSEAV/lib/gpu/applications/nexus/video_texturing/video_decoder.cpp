/******************************************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************************************/

#include "video_decoder.h"
#include "media_data.h"

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"
#include "nexus_video_decoder.h"
#include "nexus_video_decoder_extra.h"
#include "nexus_video_window.h"
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

#include <math.h>
#include <cstdio>

#define ENABLE_AMBISONIC_AUDIO 1

#if ENABLE_AMBISONIC_AUDIO
static unsigned CleanAudioAngle(float angle)
{
   // Must be between 0 & 359 degrees
   float a = fmodf(angle, 360.0f);
   if (a < 0.0f)
      a = 360.0f + a;
   assert((unsigned)a >= 0 && (unsigned)a <= 359);
   return (unsigned)a;
}
#endif

/////////////////////////////////////////////////////////////////////////////////
// NexusVideoStream
//   Abstract base class for a stream in either exclusive or nxclient mode
/////////////////////////////////////////////////////////////////////////////////

class NexusVideoStream
{
public:
   // Default constructor
   NexusVideoStream(NexusVideoFilePlayback &filePlayback, NEXUS_DISPLAYHANDLE nexusDisplay,
                    bool ambisonic);

   // Destructor
   virtual ~NexusVideoStream() { };

   // Starts decoder and playback
   virtual void StartPlayback() = 0;

   // Stops decoder and playback
   virtual void StopPlayback() = 0;

   // Returns the width of the source video
   virtual uint32_t Width() { return m_sourceWidth; };

   // Returns the height of the source video
   virtual uint32_t Height() { return m_sourceHeight; };

   // Set the spatial audio orientation
   virtual void SetSpatialAudio(float yawDegrees, float pitchDegrees, float rollDegrees) = 0;

   // Set the audio delay in ms
   virtual void SetAudioDelay(uint32_t ms) = 0;

   // Get the video decoder for this stream
   virtual NEXUS_VIDEODECODERHANDLE GetVideoDecoder() = 0;

protected:
   // Virtual cleanup function
   virtual void Cleanup() = 0;

protected:
   NexusVideoFilePlayback &m_filePlayback;
   NEXUS_StcChannelHandle  m_stcChannel;
   NEXUS_PidChannelHandle  m_videoPidChannel;
   NEXUS_PidChannelHandle  m_audioPidChannel;
   NEXUS_DISPLAYHANDLE     m_nexusDisplay;
   uint32_t                m_sourceWidth;
   uint32_t                m_sourceHeight;
   bool                    m_playStarted;
   bool                    m_ambisonicWanted;
};

NexusVideoStream::NexusVideoStream(NexusVideoFilePlayback &filePlayback, NEXUS_DISPLAYHANDLE nexusDisplay,
                                   bool ambisonic) :
   m_filePlayback(filePlayback),
   m_stcChannel(NULL),
   m_videoPidChannel(NULL),
   m_audioPidChannel(NULL),
   m_nexusDisplay(nexusDisplay),
   m_sourceWidth(0),
   m_sourceHeight(0),
   m_playStarted(false),
   m_ambisonicWanted(ambisonic)
{
};

/////////////////////////////////////////////////////////////////////////////////
// NexusVideoFilePlayback
/////////////////////////////////////////////////////////////////////////////////

class NexusVideoFilePlayback
{
public:
   NexusVideoFilePlayback() = delete;

   NexusVideoFilePlayback(const MediaData &mediaData) :
      m_filePlay(NULL),
      m_playPump(NULL),
      m_playBack(NULL),
      m_mediaData(mediaData),
      m_playStarted(false)
   {
      NEXUS_PlaypumpOpenSettings playpumpOpenSettings;
      NEXUS_Playpump_GetDefaultOpenSettings(&playpumpOpenSettings);
      m_playPump = NEXUS_Playpump_Open(NEXUS_ANY_ID, &playpumpOpenSettings);
      if (m_playPump == NULL)
         throw "NEXUS_Playpump_Open failed";

      m_playBack = NEXUS_Playback_Create();
      if (m_playBack == NULL)
         throw "NEXUS_Playback_Create failed";

      NEXUS_PlaybackSettings playbackSettings;
      NEXUS_Playback_GetSettings(m_playBack, &playbackSettings);
      playbackSettings.playpump = m_playPump;
      NEXUS_Playback_SetSettings(m_playBack, &playbackSettings);

      // Open stream file - 2nd argument is the index
      printf("File Name: %s\n", m_mediaData.filename.c_str());
      m_filePlay = NEXUS_FilePlay_OpenPosix(m_mediaData.filename.c_str(), m_mediaData.filename.c_str());
      if (m_filePlay == NULL)
         throw "Failed to open video file";
   }

   ~NexusVideoFilePlayback()
   {
      CleanUp();
   }

   const MediaData &GetMediaData() const
   {
      return m_mediaData;
   }

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

   void StopPlayback()
   {
      if (m_playStarted)
      {
         m_playStarted = false;

         if (m_playBack)
            NEXUS_Playback_Stop(m_playBack);
      }
   }

   NEXUS_FilePlayHandle GetFilePlay() const { return m_filePlay; }
   NEXUS_PlaypumpHandle GetPlayPump() const { return m_playPump; }
   NEXUS_PlaybackHandle GetPlayback() const { return m_playBack; }

private:
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

   NEXUS_FilePlayHandle m_filePlay;
   NEXUS_PlaypumpHandle m_playPump;
   NEXUS_PlaybackHandle m_playBack;
   MediaData            m_mediaData;
   bool                 m_playStarted;
};


/////////////////////////////////////////////////////////////////////////////////
// NexusVideoStreamSingleProcess
/////////////////////////////////////////////////////////////////////////////////
#ifdef SINGLE_PROCESS
class NexusVideoStreamSingleProcess : public NexusVideoStream
{
public:
   // Default Constructor
   NexusVideoStreamSingleProcess(NEXUS_DISPLAYHANDLE nexusDisplay, NexusVideoFilePlayback &filePlayback,
                                 bool secure, bool ambisonic, bool stereoAudio,
                                 NEXUS_VideoWindowHandle videoWindow, bool showVideo) :
      NexusVideoStream(filePlayback, nexusDisplay, ambisonic),
      m_videoDecoder(NULL),
      m_audioDecoder(NULL),
      m_ambisonic(NULL),
      m_videoWindow(videoWindow),
      m_showVideo(showVideo)
   {
      const MediaData &mediaData = m_filePlayback.GetMediaData();

      m_sourceWidth  = mediaData.data.video[0].width;
      m_sourceHeight = mediaData.data.video[0].height;

      Configure(nexusDisplay, secure, stereoAudio);
   };

   // Destructor
   virtual ~NexusVideoStreamSingleProcess()
   {
      Cleanup();
   };

   // Clean up the member variables
   virtual void Cleanup()
   {
      StopPlayback();

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
      // range based for not available on GCC4.5
      for (std::vector<NEXUS_AudioOutputHandle>::const_iterator it = m_audioConnections.begin(); it != m_audioConnections.end(); ++it)
         NEXUS_AudioOutput_RemoveAllInputs(*it);

#if ENABLE_AMBISONIC_AUDIO
      if (m_ambisonic)
      {
         NEXUS_AudioProcessor_RemoveAllInputs(m_ambisonic);
         NEXUS_AudioProcessor_Close(m_ambisonic);
      }
#endif
      if (m_audioDecoder)
      {
         NEXUS_AudioDecoder_Close(m_audioDecoder);
         m_audioDecoder = NULL;
      }
#endif

      if (m_videoDecoder)
      {
         // Check comment for NEXUS_Display_ConnectVideoInput
         if (m_videoWindow == NULL)
         {
            NEXUS_Display_DisconnectVideoInput(m_nexusDisplay, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
         }
         else
         {
            NEXUS_VideoWindow_RemoveInput(m_videoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));
            NEXUS_VideoDecoder_SetPowerState(m_videoDecoder, false);
         }
      }

      if (m_videoDecoder)
      {
         NEXUS_VideoDecoder_Close(m_videoDecoder);
         m_videoDecoder = NULL;
      }
   };

private:
   virtual void Configure(NEXUS_DISPLAYHANDLE nexusDisplay, bool secure, bool stereoAudio)
   {
      try
      {
         // Get the stream information from the video file
         const MediaData &mediaData = m_filePlayback.GetMediaData();

         m_nexusDisplay = nexusDisplay;

         Cleanup();

         // Bring up audio decoders and outputs
         NEXUS_PlatformConfiguration platformConfig;
         NEXUS_Platform_GetConfiguration(&platformConfig);

#if NEXUS_HAS_AUDIO
         if (mediaData.data.audio[0].codec != NEXUS_AudioCodec_eUnknown)
         {
            m_audioDecoder = NEXUS_AudioDecoder_Open(0, NULL);
            if (m_audioDecoder == NULL)
               throw "NEXUS_AudioDecoder_Open failed";

            // Connect Decoder -> (Optional)Effects -> Mixer
            NEXUS_AudioInputHandle mixerConnector = NULL;

#if ENABLE_AMBISONIC_AUDIO
            if (m_ambisonicWanted)
            {
               // Setup Ambisonic audio processing
               NEXUS_AudioProcessorOpenSettings processorOpenSettings;
               NEXUS_AudioProcessorSettings     processorSettings;

               NEXUS_AudioProcessor_GetDefaultOpenSettings(&processorOpenSettings);
               processorOpenSettings.type = NEXUS_AudioPostProcessing_eAmbisonic;
               m_ambisonic = NEXUS_AudioProcessor_Open(&processorOpenSettings);

               if (m_ambisonic)
               {
                  NEXUS_AudioProcessor_GetSettings(m_ambisonic, &processorSettings);
                  processorSettings.settings.ambisonic.contentType = NEXUS_AmbisonicContentType_eAmbisonic;
                  processorSettings.settings.ambisonic.yaw   = 0;
                  processorSettings.settings.ambisonic.pitch = 0;
                  processorSettings.settings.ambisonic.roll  = 0;
                  NEXUS_AudioProcessor_SetSettings(m_ambisonic, &processorSettings);

                  // Feed the audioDecode output to the Ambisonic processor
                  NEXUS_AudioProcessor_AddInput(m_ambisonic, NEXUS_AudioDecoder_GetConnector(m_audioDecoder,
                                                                    NEXUS_AudioConnectorType_eMultichannel));

                  mixerConnector = NEXUS_AudioProcessor_GetConnectorByType(m_ambisonic,
                                                stereoAudio ? NEXUS_AudioConnectorType_eStereo :
                                                              NEXUS_AudioConnectorType_eMultichannel);
               }
               else
                  fprintf(stderr, "NO AMBISONIC PROCESSING AVAILABLE - USING STEREO\n");
            }
#endif
            if (m_ambisonic == NULL)
               // Source will just be the audio decoder - no processing
               mixerConnector = NEXUS_AudioDecoder_GetConnector(m_audioDecoder, NEXUS_AudioDecoderConnectorType_eStereo);

            if (stereoAudio)
            {
#if (NEXUS_NUM_AUDIO_DACS > 0)
               if (platformConfig.outputs.audioDacs[0])
               {
                  m_audioConnections.push_back(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]));
                  NEXUS_AudioOutput_AddInput(m_audioConnections.back(), mixerConnector);
               }
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
               if (platformConfig.outputs.spdif[0])
               {
                  m_audioConnections.push_back(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]));
                  NEXUS_AudioOutput_AddInput(m_audioConnections.back(), mixerConnector);
               }
#endif
            }

#if NEXUS_NUM_HDMI_OUTPUTS
            if (platformConfig.outputs.hdmi[0])
            {
               m_audioConnections.push_back(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]));
               NEXUS_AudioOutput_AddInput(m_audioConnections.back(), mixerConnector);
            }
#endif
         }
#endif

         // Bring up video decoder
         NEXUS_VideoDecoderOpenSettings videoDecoderOpenSettings;
         NEXUS_VideoDecoder_GetDefaultOpenSettings(&videoDecoderOpenSettings);
#ifdef NEXUS_VIDEO_SECURE_HEAP
         if (secure)
         {
            NEXUS_PlatformConfiguration platformConfig;
            NEXUS_Platform_GetConfiguration(&platformConfig);
            videoDecoderOpenSettings.secureVideo = secure;
            videoDecoderOpenSettings.cdbHeap = platformConfig.heap[NEXUS_VIDEO_SECURE_HEAP];
         }
#endif

         m_videoDecoder = NEXUS_VideoDecoder_Open(0, &videoDecoderOpenSettings);
         if (m_videoDecoder == NULL)
            throw "NEXUS_VideoDecoder_Open failed";

         NEXUS_VideoDecoderSettings videoDecoderSettings;
         NEXUS_VideoDecoder_GetSettings(m_videoDecoder, &videoDecoderSettings);
         videoDecoderSettings.maxWidth  = m_sourceWidth;
         videoDecoderSettings.maxHeight = m_sourceHeight;
         NEXUS_VideoDecoder_SetSettings(m_videoDecoder, &videoDecoderSettings);

         if (m_videoWindow != NULL)
            NEXUS_VideoWindow_AddInput(m_videoWindow, NEXUS_VideoDecoder_GetConnector(m_videoDecoder));

         // Tell Display module to connect to the VideoDecoder module and supply the
         // L1 INT id's from BVDC_Display_GetInterrupt. Display will not register for the data ready ISR callback.
         NEXUS_Display_DriveVideoDecoder(m_nexusDisplay);

         // StcChannel
         NEXUS_StcChannelSettings stcSettings;
         NEXUS_StcChannel_GetDefaultSettings(0, &stcSettings);
         stcSettings.timebase = NEXUS_Timebase_e0;
         stcSettings.mode = NEXUS_StcChannelMode_eAuto;
         stcSettings.stcIndex = 0;           // actual stream index in the video file
         m_stcChannel = NEXUS_StcChannel_Open(0, &stcSettings);
         if (m_stcChannel == NULL)
            throw "NEXUS_StcChannel_Open failed";

         // Setting the playback
         NEXUS_PlaybackSettings playbackSettings;
         NEXUS_Playback_GetSettings(m_filePlayback.GetPlayback(), &playbackSettings);
         playbackSettings.playpumpSettings.transportType = mediaData.data.transportType;
         playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
         playbackSettings.stcChannel = m_stcChannel;
         playbackSettings.stcTrick   = m_stcChannel;
         playbackSettings.enableStreamProcessing = mediaData.data.enableStreamProcessing;
         NEXUS_Playback_SetSettings(m_filePlayback.GetPlayback(), &playbackSettings);

         // video
         NEXUS_PlaybackPidChannelSettings pidChannelSettings;
         NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
         pidChannelSettings.pidSettings.pidType           = NEXUS_PidType_eVideo;
         pidChannelSettings.pidTypeSettings.video.codec   = mediaData.data.video[0].codec;
         pidChannelSettings.pidTypeSettings.video.decoder = m_videoDecoder;
         pidChannelSettings.pidTypeSettings.video.index   = true;
         m_videoPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(),
                                          mediaData.data.video[0].pid, &pidChannelSettings);
         if (m_videoPidChannel == NULL)
            throw "NEXUS_Playback_OpenPidChannel failed for video";

         // audio
         NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
         pidChannelSettings.pidSettings.pidType           = NEXUS_PidType_eAudio;
         pidChannelSettings.pidTypeSettings.audio.primary = m_audioDecoder;
         m_audioPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(),
                                       mediaData.data.audio[0].pid, &pidChannelSettings);
         if (m_audioPidChannel == NULL)
            throw "NEXUS_Playback_OpenPidChannel failed for audio";

         NEXUS_VideoDecoder_GetDefaultStartSettings(&m_videoProgram);
         m_videoProgram.codec      = mediaData.data.video[0].codec;
         m_videoProgram.pidChannel = m_videoPidChannel;
         m_videoProgram.stcChannel = m_stcChannel;


#if NEXUS_HAS_AUDIO
         NEXUS_AudioDecoder_GetDefaultStartSettings(&m_audioProgram);
         m_audioProgram.codec      = mediaData.data.audio[0].codec;
         m_audioProgram.pidChannel = m_audioPidChannel;
         m_audioProgram.stcChannel = m_stcChannel;
#endif
      }
      catch (...)
      {
         Cleanup();
         throw;
      }
   };

public:
   // Starts decoder and playback
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

   // Stops decoder and playback
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

   virtual void SetSpatialAudio(float yawDegrees, float pitchDegrees, float rollDegrees)
   {
#if ENABLE_AMBISONIC_AUDIO
      if (m_ambisonic)
      {
         NEXUS_AudioProcessorSettings processorSettings;
         NEXUS_AudioProcessor_GetSettings(m_ambisonic, &processorSettings);
         processorSettings.settings.ambisonic.contentType = NEXUS_AmbisonicContentType_eAmbisonic;
         processorSettings.settings.ambisonic.yaw   = CleanAudioAngle(yawDegrees);
         processorSettings.settings.ambisonic.pitch = CleanAudioAngle(pitchDegrees);
         processorSettings.settings.ambisonic.roll  = CleanAudioAngle(rollDegrees);
         NEXUS_AudioProcessor_SetSettings(m_ambisonic, &processorSettings);
      }
#endif
   }

   // Set audio delay
   virtual void SetAudioDelay(uint32_t ms)
   {
#if NEXUS_HAS_AUDIO
      if (m_audioDecoder)
      {
         NEXUS_PlatformConfiguration platformConfig;
         NEXUS_Platform_GetConfiguration(&platformConfig);
         NEXUS_AudioOutputSettings aoSettings;

#if (NEXUS_NUM_AUDIO_DACS > 0)
         if (platformConfig.outputs.audioDacs[0])
         {
            NEXUS_AudioOutput_GetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_AudioDac_GetConnector(platformConfig.outputs.audioDacs[0]), &aoSettings);
         }
#endif

#if NEXUS_NUM_SPDIF_OUTPUTS
         if (platformConfig.outputs.spdif[0])
         {
            NEXUS_AudioOutput_GetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_SpdifOutput_GetConnector(platformConfig.outputs.spdif[0]), &aoSettings);
         }
#endif

#if NEXUS_NUM_HDMI_OUTPUTS
         if (platformConfig.outputs.hdmi[0])
         {
            NEXUS_AudioOutput_GetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
            aoSettings.additionalDelay = ms;
            NEXUS_AudioOutput_SetSettings(NEXUS_HdmiOutput_GetAudioConnector(platformConfig.outputs.hdmi[0]), &aoSettings);
         }
#endif
      }
#endif
   }

   // Get the video decoder for this stream
   virtual NEXUS_VIDEODECODERHANDLE GetVideoDecoder()
   {
      return m_videoDecoder;
   }

private:
   NEXUS_VideoDecoderHandle        m_videoDecoder;
   NEXUS_VideoDecoderStartSettings m_videoProgram;
   NEXUS_AudioDecoderHandle        m_audioDecoder;
   NEXUS_AudioProcessorHandle      m_ambisonic;
   NEXUS_AudioDecoderStartSettings m_audioProgram;
   NEXUS_VideoWindowHandle         m_videoWindow;
   bool                            m_showVideo;

   std::vector<NEXUS_AudioOutputHandle>   m_audioConnections;
};

#endif // #ifdef SINGLE_PROCESS

/////////////////////////////////////////////////////////////////////////////////
// NexusVideoStreamMultiProcess
/////////////////////////////////////////////////////////////////////////////////
#ifndef SINGLE_PROCESS

class NexusVideoStreamMultiProcess : public NexusVideoStream
{
public:

   // Default constructor
   NexusVideoStreamMultiProcess(NEXUS_DISPLAYHANDLE nexusDisplay, NexusVideoFilePlayback &filePlayback,
                                const std::vector<NEXUS_SurfaceHandle> decodeSurfaces,
                                bool secure, bool ambisonic, bool stereoAudio, bool showVideo) :
      NexusVideoStream(filePlayback, nexusDisplay, ambisonic),
      m_videoDecoder(NULL),
      m_audioDecoder(NULL),
      m_connectId(0),
      m_numberOfCaptures(0),
      m_showVideo(showVideo)
   {
      const MediaData &mediaData = m_filePlayback.GetMediaData();

      m_sourceWidth  = mediaData.data.video[0].width;
      m_sourceHeight = mediaData.data.video[0].height;

      Configure(nexusDisplay, decodeSurfaces, secure, stereoAudio);
   };

   // Destructor
   virtual ~NexusVideoStreamMultiProcess()
   {
      Cleanup();
   };

   // Clean up the member variables
   virtual void Cleanup()
   {
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

private:
   // Creates the necessary buffers and configure the decoder
   virtual void Configure(NEXUS_DISPLAYHANDLE nexusDisplay,
                          const std::vector<NEXUS_SurfaceHandle> decodeSurfaces,
                          bool secure, bool stereoAudio)
   {
      try
      {
         // Get the stream information from the video file
         const MediaData &mediaData = m_filePlayback.GetMediaData();

         m_nexusDisplay = nexusDisplay;

         Cleanup();

         // Allocate the number of video streams / decoders
         NxClient_AllocSettings allocSettings;
         NxClient_GetDefaultAllocSettings(&allocSettings);
         allocSettings.simpleVideoDecoder = 1;
         allocSettings.simpleAudioDecoder = 1;

         NEXUS_Error rc;
         rc = NxClient_Alloc(&allocSettings, &m_allocResults);
         if (rc != NEXUS_SUCCESS)
            throw "Nxclient simple decoder allocations failed";

         if (m_allocResults.simpleVideoDecoder[0].id)
            m_videoDecoder = NEXUS_SimpleVideoDecoder_Acquire(m_allocResults.simpleVideoDecoder[0].id);

         if (m_allocResults.simpleAudioDecoder.id)
            m_audioDecoder = NEXUS_SimpleAudioDecoder_Acquire(m_allocResults.simpleAudioDecoder.id);

         if (m_videoDecoder == NULL)
            throw "Acquire Nxclient video decoder failed";

         if (m_audioDecoder == NULL)
            throw "Acquire Nxclient audio decoder failed";

         NEXUS_StcChannelSettings stcSettings;
         NEXUS_StcChannel_GetDefaultSettings(NEXUS_ANY_ID, &stcSettings);
         stcSettings.timebase = NEXUS_Timebase_e0;
         stcSettings.mode     = NEXUS_StcChannelMode_eAuto;
         m_stcChannel         = NEXUS_StcChannel_Open(NEXUS_ANY_ID, &stcSettings);
         if (m_stcChannel == NULL)
            throw "NEXUS_StcChannel_Open failed";

         // setup playback
         NEXUS_PlaybackSettings playbackSettings;
         NEXUS_Playback_GetSettings(m_filePlayback.GetPlayback(), &playbackSettings);
         playbackSettings.playpumpSettings.transportType = mediaData.data.transportType;
         playbackSettings.endOfStreamAction = NEXUS_PlaybackLoopMode_eLoop;
         playbackSettings.stcChannel        = m_stcChannel;
         playbackSettings.enableStreamProcessing = mediaData.data.enableStreamProcessing;
         NEXUS_Playback_SetSettings(m_filePlayback.GetPlayback(), &playbackSettings);

         // video
         NEXUS_PlaybackPidChannelSettings pidChannelSettings;
         NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
         pidChannelSettings.pidSettings.pidType                 = NEXUS_PidType_eVideo;
         pidChannelSettings.pidTypeSettings.video.codec         = mediaData.data.video[0].codec;
         pidChannelSettings.pidTypeSettings.video.simpleDecoder = m_videoDecoder;
         pidChannelSettings.pidTypeSettings.video.index         = true;
         m_videoPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(),
                                       mediaData.data.video[0].pid, &pidChannelSettings);
         if (m_videoPidChannel == NULL)
            throw "NEXUS_Playback_OpenPidChannel failed for video";

         // audio
         NEXUS_Playback_GetDefaultPidChannelSettings(&pidChannelSettings);
         pidChannelSettings.pidSettings.pidType                 = NEXUS_PidType_eAudio;
         pidChannelSettings.pidTypeSettings.audio.simpleDecoder = m_audioDecoder;
         m_audioPidChannel = NEXUS_Playback_OpenPidChannel(m_filePlayback.GetPlayback(),
                                    mediaData.data.audio[0].pid, &pidChannelSettings);
         if (m_audioPidChannel == NULL)
            throw "NEXUS_Playback_OpenPidChannel failed for audio";

         NEXUS_SimpleVideoDecoder_GetDefaultStartSettings(&m_videoProgram);
         m_videoProgram.settings.codec      = mediaData.data.video[0].codec;
         m_videoProgram.settings.pidChannel = m_videoPidChannel;
         m_videoProgram.settings.stcChannel = m_stcChannel;
         m_videoProgram.displayEnabled      = m_showVideo;
         m_videoProgram.maxWidth            = m_sourceWidth;
         m_videoProgram.maxHeight           = m_sourceHeight;

#if NEXUS_HAS_AUDIO
         NEXUS_SimpleAudioDecoder_GetDefaultStartSettings(&m_audioProgram);
         m_audioProgram.primary.codec      = mediaData.data.audio[0].codec;
         m_audioProgram.primary.pidChannel = m_audioPidChannel;
         m_audioProgram.primary.stcChannel = m_stcChannel;

#if ENABLE_AMBISONIC_AUDIO
         if (m_ambisonicWanted)
         {
            NEXUS_SimpleAudioDecoderSettings audioSettings;
            NEXUS_SimpleAudioDecoder_GetSettings(m_audioDecoder, &audioSettings);
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connected = true;
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.contentType =
                                                                                 NEXUS_AmbisonicContentType_eAmbisonic;
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.connectorType =
                                       stereoAudio ? NEXUS_AudioConnectorType_eStereo : NEXUS_AudioConnectorType_eMultichannel;
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.yaw = 0;
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.pitch = 0;
            audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.roll = 0;
            NEXUS_SimpleAudioDecoder_SetSettings(m_audioDecoder, &audioSettings);
      }
#endif
#endif

         // connect client resources to server's resources
         NxClient_ConnectSettings            connectSettings;
         NxClient_GetDefaultConnectSettings(&connectSettings);

         if (m_videoProgram.settings.pidChannel)
         {
            connectSettings.simpleAudioDecoder.id          = m_allocResults.simpleAudioDecoder.id;
            connectSettings.simpleVideoDecoder[0].id       = m_allocResults.simpleVideoDecoder[0].id;
            connectSettings.simpleVideoDecoder[0].windowId = 0;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxWidth  = m_sourceWidth;
            connectSettings.simpleVideoDecoder[0].decoderCapabilities.maxHeight = m_sourceHeight;
         }

         connectSettings.simpleVideoDecoder[0].decoderCapabilities.secureVideo = secure;

         rc = NxClient_Connect(&connectSettings, &m_connectId);
         if (rc != NEXUS_SUCCESS)
            throw "NEXUS_Playback_OpenPidChannel failed";

         NEXUS_SimpleVideoDecoder_GetDefaultStartCaptureSettings(&m_videoCaptureSettings);

         // Tell the simpleDecoder about the available decode surfaces
         assert(decodeSurfaces.size() <= NEXUS_SIMPLE_DECODER_MAX_SURFACES);
         for (size_t i = 0; i < decodeSurfaces.size(); i++)
            m_videoCaptureSettings.surface[i] = decodeSurfaces[i];

         m_videoCaptureSettings.displayEnabled     = m_showVideo;
         m_videoCaptureSettings.secure             = secure;
         m_videoCaptureSettings.forceFrameDestripe = true;
      }
      catch (...)
      {
         Cleanup();
         throw;
      }
   };

public:
   // Starts decoder and playback
   virtual void StartPlayback()
   {
      if (m_videoDecoder && !m_playStarted)
      {
         m_playStarted = true;

         // Start decode
         NEXUS_SimpleVideoDecoder_Start(m_videoDecoder, &m_videoProgram);
         NEXUS_SimpleVideoDecoder_StartCapture(m_videoDecoder, &m_videoCaptureSettings);

         if (m_audioDecoder)
            if (NEXUS_SimpleAudioDecoder_Start(m_audioDecoder, &m_audioProgram) != NEXUS_SUCCESS)
               printf("Not started audio decoder\n");

         // Start playback
         m_filePlayback.StartPlayback();
      }
   }

   // Stops decoder and playback
   virtual void StopPlayback()
   {
      if (m_videoDecoder && m_playStarted)
      {
         m_playStarted = false;

         m_filePlayback.StopPlayback();

         NEXUS_SimpleVideoDecoder_StopCapture(m_videoDecoder);
         NEXUS_SimpleVideoDecoder_Stop(m_videoDecoder);

         if (m_audioDecoder)
            NEXUS_SimpleAudioDecoder_Stop(m_audioDecoder);
      }
   }

   virtual void SetSpatialAudio(float yawDegrees, float pitchDegrees, float rollDegrees)
   {
#if ENABLE_AMBISONIC_AUDIO
      if (m_ambisonicWanted)
      {
         NEXUS_SimpleAudioDecoderSettings audioSettings;
         NEXUS_SimpleAudioDecoder_GetSettings(m_audioDecoder, &audioSettings);
         audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.yaw =
                                                                                 CleanAudioAngle(yawDegrees);
         audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.pitch =
                                                                                 CleanAudioAngle(pitchDegrees);
         audioSettings.processorSettings[NEXUS_SimpleAudioDecoderSelector_ePrimary].ambisonic.settings.roll =
                                                                                 CleanAudioAngle(rollDegrees);
         NEXUS_SimpleAudioDecoder_SetSettings(m_audioDecoder, &audioSettings);
      }
#endif
   }

   // Set audio delay
   virtual void SetAudioDelay(uint32_t ms)
   {
      // Ignored in nxclient mode
   }

   // Get the video decoder for this stream
   virtual NEXUS_VIDEODECODERHANDLE GetVideoDecoder()
   {
      return m_videoDecoder;
   }

private:
   NEXUS_SimpleVideoDecoderHandle               m_videoDecoder;
   NEXUS_SimpleAudioDecoderHandle               m_audioDecoder;
   unsigned                                     m_connectId;
   NxClient_AllocResults                        m_allocResults;
   NEXUS_SimpleVideoDecoderStartSettings        m_videoProgram;
   NEXUS_SimpleAudioDecoderStartSettings        m_audioProgram;
   NEXUS_SimpleVideoDecoderStartCaptureSettings m_videoCaptureSettings;
   uint32_t                                     m_numberOfCaptures;
   bool                                         m_showVideo;
};
#endif // #ifndef SINGLE_PROCESS

/////////////////////////////////////////////////////////////////////////////////
// VideoDecoder
/////////////////////////////////////////////////////////////////////////////////


VideoDecoder::VideoDecoder(const MediaData &mediaData,
                           std::vector<NEXUS_SurfaceHandle> decodeSurfaces,
                           bool showVideo, bool secure, bool ambisonic, bool stereoAudio,
                           NEXUS_DISPLAYHANDLE nexusDisplay) :
   m_audioDelay(0),
   m_videoWindow(NULL),
   m_showVideo(showVideo),
   m_secure(secure)
{
   m_nexusFilePlayback = new NexusVideoFilePlayback(mediaData);

#ifdef SINGLE_PROCESS
   if (m_showVideo)
      m_videoWindow = NEXUS_VideoWindow_Open(nexusDisplay, 0);

   m_stream = new NexusVideoStreamSingleProcess(nexusDisplay, *m_nexusFilePlayback,
                                                secure, ambisonic, stereoAudio, m_videoWindow, m_showVideo);
#else
   m_stream = new NexusVideoStreamMultiProcess(nexusDisplay, *m_nexusFilePlayback,
                                               decodeSurfaces, secure, ambisonic, stereoAudio, m_showVideo);
#endif
}

VideoDecoder::~VideoDecoder()
{
   StopPlayback();

   delete m_stream;

   if (m_videoWindow)
   {
      NEXUS_VideoWindow_Close(m_videoWindow);
      m_videoWindow = NULL;
   }

   delete m_nexusFilePlayback;
}

// Starts decoder and playback
void VideoDecoder::StartPlayback()
{
   m_stream->StartPlayback();
}

// Stops decoder and playback
void VideoDecoder::StopPlayback()
{
   m_stream->StopPlayback();
}

// Returns the width of the source video data
uint32_t VideoDecoder::Width()
{
   return m_stream->Width();
};

// Returns the height of the source video data
uint32_t VideoDecoder::Height()
{
   return m_stream->Height();
};

NEXUS_VIDEODECODERHANDLE VideoDecoder::GetVideoDecoder()
{
   return m_stream->GetVideoDecoder();
}

void VideoDecoder::SetSpatialAudio(float yawDegrees, float pitchDegrees, float rollDegrees)
{
   m_stream->SetSpatialAudio(yawDegrees, pitchDegrees, rollDegrees);
}

// Get the audio stream delay in ms
uint32_t VideoDecoder::GetAudioDelay() const
{
   return m_audioDelay;
}

// Set the audio stream delay in ms
void VideoDecoder::SetAudioDelay(uint32_t ms)
{
   m_audioDelay = ms;
   m_stream->SetAudioDelay(ms);
}
