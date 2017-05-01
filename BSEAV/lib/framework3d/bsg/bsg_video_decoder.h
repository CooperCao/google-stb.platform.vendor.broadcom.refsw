/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BSG_STAND_ALONE

#ifndef __BSG_VIDEO_DECODER_H__
#define __BSG_VIDEO_DECODER_H__

#include <string>

#include "bsg_common.h"
#include "bsg_platform.h"

namespace bsg
{

   enum eVideoFrameFormat
   {
      NONE = 0,   //!< \deprecated
      YUV422,     //!< \deprecated
      YUVX444,    //!< \deprecated
      RGB565,     //!< \deprecated
      RGB888,     //!< \deprecated
      RGBX8888,   //!< \deprecated

      eNONE     = NONE,
      eYUV422   = YUV422,
      eYUVX444  = YUVX444,
      eRGB565   = RGB565,
      eRGB888   = RGB888,
      eRGBX8888 = RGBX8888,
      eYV12
   };

   enum eVideoUpdateMode
   {
      eEXT_SYNC = 0,     //!< Use the EGL image update extension with lock/unlock primitives
      eEXT,              //!< Use the EGL image update extension without lock/unlock
      eALWAYS_SYNC,      //!< Don't use the EGL image update extension. Update every frame with lock/unlock available
      eALWAYS,           //!< Don't use the EGL image update extension. Update every frame - no lock/unlock
      eEGL_SYNC          //!< Use EGL_KHR_fence_sync extension
   };

   class VideoDecoderPrivate
   {
   public:
      virtual ~VideoDecoderPrivate() = 0;
   };


   class VideoDecoder
   {
   public:
      enum eMode
      {
         NO_BLOCK = 0,  //!< \deprecated
         BLOCK,         //!< \deprecated

         eNO_BLOCK = NO_BLOCK,
         eBLOCK    = BLOCK
      };

      enum eFrameStatus
      {
         FRAME_NEW = 0, //!< \deprecated
         FRAME_REPEAT,  //!< \deprecated
         FRAME_ERROR,   //!< \deprecated

         eFRAME_NEW = 0, //!< A new frame is returned
         eFRAME_REPEAT,  //!< The same frame as last time is returned
         eFRAME_ERROR    //!< An error has occurred
      };

      //! Constructor
      //! @param videoFileName is the path to the video file
      //! @param eVideoFrameFormat is the format to decode the video file
      //! @param widthOutput width of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param heightOutput height of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param textureMode describes how the texture is going to be used
      //! @param mode describes the type of synchronisation during texture update
      //! @param numBuffers if equal to 0 the number of buffer will have a hard-coded default value
      //! @param playFullScreenToo plays the video in a full screen hardware window on the video plane
      VideoDecoder(const std::string &videoFileName, eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                  GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers = 0, bool playFullScreenToo = false);

      //! Constructor
      //! @param videoFileName is the paths to the video files
      //! @param eVideoFrameFormat is the format to decode the video file
      //! @param widthOutput width of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param heightOutput height of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param textureMode describes how the texture is going to be used
      //! @param mode describes the type of synchronisation during texture update
      //! @param numBuffers if equal to 0 the number of buffer will have a hard-coded default value
      //! @param fullScreenVideoStreamIndex index of video stream to play in the full screen hardware video plane
      VideoDecoder(const std::vector<std::string> &videoFileNames, eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                  GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers = 0,
                  int32_t fullScreenVideoStreamIndex = -1);

      //! Destructor
      virtual ~VideoDecoder();

      //! Used to reconfigure the video decoder and recreate the internal buffers
      //! @param eVideoFrameFormat is the format to decode the video file
      //! @param widthOutput width of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param heightOutput height of the video buffer used to grab frames - if 0 the size of the video will be used
      //! @param textureMode describes how the texture is going to be used
      //! @param mode describes the type of synchronisation during texture update
      //! @param numBuffers if equal to 0 the number of buffer will have a hard-coded default value
      virtual void OutputInitialisation(eVideoFrameFormat decodeFormat, uint32_t widthOutput, uint32_t heightOutput,
                                       GLTexture::eVideoTextureMode textureMode, eVideoUpdateMode mode, uint32_t numBuffers = 0);

      //! Starts the decoder and the playback
      virtual void StartPlayback();

      //! Stops the decoder and the playback
      virtual void StopPlayback();

      //! Returns the number of video streams. Always 1 in non-mosaic mode.
      virtual uint32_t NumStreams() const;

      //! Returns the width of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceWidth(uint32_t streamIndex = 0);

      //! Returns the height of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceHeight(uint32_t streamIndex = 0);

      //! Returns the width of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestWidth(uint32_t streamIndex = 0);

      //! Returns the height of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestHeight(uint32_t streamIndex = 0);

      //! Updates the current texture with the last decoded video frame.
      //! If mode==BLOCK, will wait until a different frame from the last one returned is available.
      //! If mode==NO_BLOCK, will return the last available frame immediately. This may result in duplicate
      //! frames being returned.
      //! The video frame will be decoded directly into the given VideoBuffer
      //! Returns Status to indicate what has happened.
      virtual eFrameStatus UpdateFrame(eMode mode);

      //! Updates the current texture with the last decoded video frames (in mosaic mode).
      //! The video frames will be decoded directly into the given VideoBuffers
      //! Returns Status to indicate what has happened.
      virtual void UpdateMosaicFrames(std::vector<eFrameStatus> &results);

      //! Returns a reference to the current texture handle
      //! of the stream with the index streamIndex
      //! @param streamIndex is the index of the stream within the decoder
      TextureHandle &GetCurrentTexture(uint32_t streamIndex);

      //! Returns a reference to the current pixmap
      //! of the stream with the index streamIndex
      //! @param streamIndex is the index of the stream within the decoder
      NativePixmap *GetCurrentPixmap(uint32_t streamIndex);

      //! Makes the next list of buffers as current
      //! @param streamIndex is the index of the stream within the decoder
      void ChangeCurrentBuffer(uint32_t streamIndex);

      //! Returns the number of buffers used within a stream
      uint32_t NumBuffersPerStream();

      //! Get the audio stream delay in ms
      uint32_t GetAudioDelay() const;

      //! Set the audio stream delay in ms
      void SetAudioDelay(uint32_t ms);

      virtual void CreateFenceForSyncUpdate(EGLDisplay dpy);


   private:
      // Private member implementing the video decoder according to the platform
      VideoDecoderPrivate                 *m_videoDecoderPrivate;

      eVideoUpdateMode                    m_updateMode;
   };
}

#endif  // __BSG_VIDEO_DECODER_H__

#endif // BSG_STAND_ALONE
