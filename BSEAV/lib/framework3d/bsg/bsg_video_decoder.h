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
      int32_t                             m_fullScreenStreamIndex;
   };
}

#endif  // __BSG_VIDEO_DECODER_H__

#endif // BSG_STAND_ALONE
