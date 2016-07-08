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

#ifndef __BSG_VIDEO_H__
#define __BSG_VIDEO_H__

#include <stdint.h>
#include <string>

#include "bsg_common.h"
#include "bsg_platform.h"
#include "bsg_gl_texture.h"

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
      eRGBX8888 = RGBX8888
   };

   enum eVideoUpdateMode
   {
      eEXT_SYNC = 0,     //!< Use the EGL image update extension with lock/unlock primitives
      eEXT,              //!< Use the EGL image update extension without lock/unlock
      eALWAYS_SYNC,      //!< Don't use the EGL image update extension. Update every frame with lock/unlock available
      eALWAYS            //!< Don't use the EGL image update extension. Update every frame - no lock/unlock
   };

   //! Represents a buffer able to hold one frame of video, with timestamp information
   class VideoBuffer
   {
   public:
      VideoBuffer(uint32_t width, uint32_t height, eVideoFrameFormat format, NativePixmap *nativePixmap) :
         m_width(width),
            m_height(height),
            m_format(format),
            m_nativePixmap(nativePixmap),
            m_cleared(false)
         {
         }

         NativePixmap *GetNativePixmap() const { return m_nativePixmap; }
         uint32_t GetWidth() const { return m_width; }
         uint32_t GetHeight() const { return m_height; }
         eVideoFrameFormat GetFormat() const { return m_format; }
         bool Cleared() const { return m_cleared; };
         void SetCleared(bool clear) { m_cleared = clear; };

   private:
      uint32_t          m_width;
      uint32_t          m_height;
      eVideoFrameFormat m_format;
      NativePixmap      *m_nativePixmap;
      bool				m_cleared;
   };

   // @cond

   class VideoPrivate
   {
   public:
      virtual ~VideoPrivate() = 0;
   };

   // @endcond

   //! Manages reading video files and producing frame data
   class Video
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

      //! Open the video file ready for streaming.
      //! Will throw an exception on any failure.
      Video(const std::string &videoFileName);

      //! Open the video files in mosaic mode ready for streaming.
      //! Will throw an exception on any failure.
      Video(const std::vector<std::string> &videoFileNames);

      //! Destructor
      virtual ~Video();

      //! Returns the number of video streams. Always 1 in non-mosaic mode.
      virtual uint32_t NumStreams() const;

      //! Returns the width of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceWidth(uint32_t vidIndx = 0);

      //! Returns the height of the source video data. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t SourceHeight(uint32_t vidIndx = 0);

      //! Returns the width of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestWidth(uint32_t vidIndx = 0);

      //! Returns the height of the destination video frame. In mosaic mode, pass the index of the video of interest.
      virtual uint32_t DestHeight(uint32_t vidIndx = 0);

      //! Sets the resolution and pixel format of the scaled output.
      //! The VideoBuffers you provide to receive decoded frames must match this.
      virtual void SetOutputParameters(uint32_t width, uint32_t height, eVideoFrameFormat format, GLTexture::eVideoTextureMode mode);

      //! Sets the pixel format of the scaled output. The size will be determined from the input source.
      //! NOTE: the output size may not match the input. Be sure to use output size for your frames.
      virtual void SetOutputFormat(eVideoFrameFormat format, GLTexture::eVideoTextureMode mode);

      //! Sets the update mode
      virtual void SetUpdateMode(eVideoUpdateMode mode) { m_updateMode = mode; }

      //! Gets the last decoded video frame.
      //! If mode==BLOCK, will wait until a different frame from the last one returned is available.
      //! If mode==NO_BLOCK, will return the last available frame immediately. This may result in duplicate
      //! frames being returned.
      //! The video frame will be decoded directly into the given VideoBuffer
      //! Returns Status to indicate what has happened.
      virtual eFrameStatus GetFrame(eMode mode, VideoBuffer *buffer, TextureHandle texture);

      //! Gets the last decoded video frames (in mosaic mode).
      //! The video frames will be decoded directly into the given VideoBuffers
      //! Returns Status to indicate what has happened.
      virtual void GetMosaicFrames(std::vector<eFrameStatus> &results,
                                   const std::vector<VideoBuffer *> &buffers,
                                   const std::vector<TextureHandle> &textures);

      //! Access the abstract platform specific data
      virtual VideoPrivate *Platform() { return m_priv; }

   protected:
      void Cleanup();

   protected:
      VideoPrivate      *m_priv;

      uint32_t          m_outputWidth;
      uint32_t          m_outputHeight;
      eVideoFrameFormat m_format;
      eVideoUpdateMode  m_updateMode;
   };
}

#endif /* __BSG_VIDEO_H__ */

#endif /* BSG_STAND_ALONE */

