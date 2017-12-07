/******************************************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************************************/

#pragma once

#include <string>
#include <vector>
#include <cassert>

#include "nexus_display.h"
#include "default_nexus.h"

#include "nexus_video_decoder.h"
#include "nexus_simple_video_decoder.h"

#include "berr.h"
#include "bkni.h"

class NexusVideoStream;
class NexusVideoFilePlayback;
class MediaData;

#ifdef SINGLE_PROCESS
#define NEXUS_VIDEODECODERHANDLE NEXUS_VideoDecoderHandle
#else
#define NEXUS_VIDEODECODERHANDLE NEXUS_SimpleVideoDecoderHandle
#endif

// Helper class that encapsulates the handling of Nexus video playback and capture
// to avoid polluting the main example code with such intricacies. This also abstracts
// exclusive and nxclient mode into a single interface.
class VideoDecoder
{
public:
   VideoDecoder(const MediaData &mediaData,
                std::vector<NEXUS_SurfaceHandle> decodeSurfaces,
                bool showVideo, bool secure, bool ambisonic, bool stereoAudio,
                NEXUS_DISPLAYHANDLE nexusDisplay);

   ~VideoDecoder();

   // Starts the decoder and the playback
   void StartPlayback();

   // Stops the decoder and the playback
   void StopPlayback();

   // Returns the width of the source video data
   uint32_t Width();

   // Returns the height of the source video data
   uint32_t Height();

   // Get the video decoder for a given stream
   NEXUS_VIDEODECODERHANDLE GetVideoDecoder();

   // Set spatial audio orientation
   void SetSpatialAudio(float yawDegrees, float pitchDegrees, float rollDegrees);

   // Get the audio stream delay in ms
   uint32_t GetAudioDelay() const;

   // Set the audio stream delay in ms
   void SetAudioDelay(uint32_t ms);

private:
   NexusVideoStream        *m_stream;
   NexusVideoFilePlayback  *m_nexusFilePlayback;
   uint32_t                 m_audioDelay;
   NEXUS_VideoWindowHandle  m_videoWindow;
   bool                     m_showVideo;
   bool                     m_secure;
};
