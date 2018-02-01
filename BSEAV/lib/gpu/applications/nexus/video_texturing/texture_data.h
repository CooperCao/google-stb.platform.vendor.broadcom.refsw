/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "nexus_platform.h"
#include "nexus_display.h"
#include "nexus_graphics2d.h"

#include "default_nexus.h"
#include "video_decoder.h"
#include "fence.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#ifdef VC5
#include <GLES3/gl3.h>
#else // VC4
#include <GLES2/gl2.h>
#endif
#include <GLES2/gl2ext.h>  // Used for ES2 and ES3

#include <thread>

namespace video_texturing
{

////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class that encapsulates the surface and texture data for EGLimage based texturing.
// Internally the surfaces are multi-buffered to enable overlap of GL rendering and video
// decoding.
////////////////////////////////////////////////////////////////////////////////////////////////
class TextureData
{
public:
   TextureData() :
      m_data(), m_platform(NULL), m_decoder(NULL), m_curIndex(-1), m_oldestBuffer(-1),
      m_buffersAcquired(0), m_colorimetryValid(false), m_exitReleaseThread(false),
      m_glEGLImageTargetTexture2DOESFunc(NULL), m_eglCreateImageKHRFunc(NULL),
      m_eglDestroyImageKHRFunc(NULL)
   {};

   void Create(NXPL_PlatformHandle platform, NEXUS_Graphics2DHandle gfx2d,
               BKNI_EventHandle destripeDone, uint32_t numBuffers,
               uint32_t mediaW, uint32_t mediaH, uint32_t texW, uint32_t texH,
               uint32_t numMiplevels, BEGL_BufferFormat format,
               bool aniso, bool secure);
   void Destroy();

   void SetDecoder(NEXUS_VIDEODECODERHANDLE decoder) { m_decoder = decoder; }

   bool AcquireVideoFrame();
   void ReleaseVideoFrame(uint32_t indx);

   uint32_t NumBuffers() const { return m_data.size(); }

   void InsertFence();              // Creates and inserts a fence in the render pipeline
   bool BindTexture(GLenum target); // Binds the current active texture for rendering

   // Returns the native surface at a given buffer index
   NEXUS_SurfaceHandle GetNativePixmap(uint32_t indx) { return m_data.at(indx).nativePixmap; }

   bool ExitReleaseThread() const { return m_exitReleaseThread; }

   // Should only be called by the buffer release thread
   bool WaitForOldestBufferFenceAndRecycle();

private:
   void InitGLExtensions();
   void WaitForM2MCCompletion(uint32_t timeoutMs = 0xFFFFFFFF);
   uint32_t FindSurfaceIndex(NEXUS_SurfaceHandle surf) const;
   void DetermineColorimetry(NEXUS_MatrixCoefficients nmc);

private:
   struct PerBufferData
   {
      EGLNativePixmapType eglPixmap;
      NEXUS_SurfaceHandle nativePixmap;
      EGLImageKHR         eglImage;
      GLuint              textureID;
      Fence               fence;
      bool                acquired;
   };

   std::vector<PerBufferData>  m_data;
   NXPL_PlatformHandle         m_platform;
   NEXUS_VIDEODECODERHANDLE    m_decoder;
   int32_t                     m_curIndex;
   int32_t                     m_oldestBuffer;
   uint32_t                    m_buffersAcquired;
   BEGL_BufferFormat           m_format;
   bool                        m_colorimetryValid;

   BKNI_EventHandle            m_m2mcDone;
   NEXUS_Graphics2DHandle      m_gfx2d;
   volatile bool               m_exitReleaseThread;
   std::thread                 m_bufferReleaseThread;

   // Extension function pointers
   PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_glEGLImageTargetTexture2DOESFunc;
   PFNEGLCREATEIMAGEKHRPROC            m_eglCreateImageKHRFunc;
   PFNEGLDESTROYIMAGEKHRPROC           m_eglDestroyImageKHRFunc;
};

} // namespace video_texturing
