/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/
#pragma once

#include "texture_data.h"
#include "fence.h"
#include "geometry.h"
#include "camera.h"
#include "media_data.h"

#include "binput.h"        // Remote key-press handler

#include <memory>
#include <thread>
#include <chrono>

namespace video_texturing
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// NoCopy
///////////////////////////////////////////////////////////////////////////////////////////////////
class NoCopy
{
public:
   NoCopy() {}
   NoCopy(const NoCopy &) = delete;
   NoCopy &operator=(const NoCopy &) = delete;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Animation helper
///////////////////////////////////////////////////////////////////////////////////////////////////
class AnimationStep
{
public:
   AnimationStep(float minAngle, float maxAngle);

   void SetHermite(bool tf) { m_hermite = tf; }

   void Set(float startAngle, float endAngle,
            std::chrono::time_point<std::chrono::steady_clock> startTime,
            std::chrono::time_point<std::chrono::steady_clock> endTime);
   void Update(float endAngle, std::chrono::time_point<std::chrono::steady_clock> endTime,
               std::chrono::time_point<std::chrono::steady_clock> now);
   float AngleAtTime(std::chrono::time_point<std::chrono::steady_clock> time) const;
   float EndAngle() const { return m_endAngle; }

private:
   float Sanitise(float angle);

private:
   float                                              m_minAngle;
   float                                              m_maxAngle;
   bool                                               m_hermite;
   float                                              m_startAngle;
   float                                              m_endAngle;
   std::chrono::time_point<std::chrono::steady_clock> m_startTime;
   std::chrono::time_point<std::chrono::steady_clock> m_endTime;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// APPLICATION
///////////////////////////////////////////////////////////////////////////////////////////////////
class Application : public NoCopy
{
public:
   Application(int argc, const char *argv[]);
   ~Application();

   void Run();

   enum DemoMode
   {
      Demo = 0,
      Zoom,
      FullScreen,
      Compare,
      e360
   };

private:
   void ProcessArgs(int argc, const char *argv[]);
   void PollKeyPress();
   void RunBenchmark();

   void CreateVideoTextures();
   void InitGLState();
   void TerminateGLState();
   void InitGLExtensions();
   void InitGLViewPort();
   void Resize();
   void Display();
   void UpdateAnimation();
   void DrawTextOverlay();
   void InitEGL();
   void InitPlatformAndDisplay(float *aspect);
   void InitGraphics2D();
   void TermGraphics2D();

private:
   // From command line arguments
   bool                          m_showFPS;
   bool                          m_useMultisample;
   bool                          m_stretchToFit;
   bool                          m_benchmark;
   bool                          m_secure;
   bool                          m_showVideo;
   bool                          m_ambisonic;
   bool                          m_stereoAudio;
   bool                          m_anisoFiltering;
   uint32_t                      m_vpX;
   uint32_t                      m_vpY;
   uint32_t                      m_vpW;
   uint32_t                      m_vpH;
   uint32_t                      m_bpp;
   uint32_t                      m_frames;
   uint32_t                      m_swapInterval;
   uint32_t                      m_clientId;
   uint32_t                      m_texW;
   uint32_t                      m_texH;
   uint32_t                      m_maxMiplevels;
#if SINGLE_PROCESS
   uint32_t                      m_decodeBuffers;
#else
   uint32_t                      m_decodeBuffers;
#endif
   BEGL_BufferFormat             m_decodeFormat;
   std::string                   m_videoFile;
   MediaData                     m_mediaData;

   DemoMode                      m_mode;
   Format360                     m_360Format;

   // Internal state
   TextureData                   m_texture;

   std::unique_ptr<VideoDecoder> m_videoDecoder;
   NEXUS_DISPLAYHANDLE           m_nexusDisplay;
   EGLDisplay                    m_eglDisplay;
   void                         *m_nativeWindow;
   NXPL_PlatformHandle           m_nxplHandle;

   ESMatrix                      m_projectionMatrix;
   ESMatrix                      m_modelviewMatrix;
   ESMatrix                      m_modelMatrix;
   ESMatrix                      m_mvpMatrix;

   Camera                        m_camera;

   GLint                         m_mvpMatrixLoc;
   GLint                         m_positionLoc;
   GLint                         m_tcLoc;
   GLint                         m_texUnitLoc;
   GLint                         m_programObject;
   GLuint                        m_overlayTextureId;

   std::unique_ptr<Geometry>     m_geometry;

   BKNI_EventHandle              m_m2mcDone;
   NEXUS_Graphics2DHandle        m_gfx2d;
   bool                          m_usingMipmapM2MC;

   binput_t                      m_remoteInput;
   AnimationStep                 m_animYaw;
   AnimationStep                 m_animPitch;
   AnimationStep                 m_animRoll;
   AnimationStep                 m_animFov;

   bool                          m_exit;

   typedef void (GL_APIENTRYP PFNGLDRAWTEXIOESPROC) (GLint x, GLint y, GLint z, GLint width, GLint height);
   PFNGLDRAWTEXIOESPROC          m_glDrawTexiOESFunc;

   std::chrono::time_point<std::chrono::steady_clock> m_compareModeSwitchTime;
};

} // namespace video_texturing
