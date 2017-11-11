/******************************************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************************************/
#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "esutil.h"
#include "default_nexus.h"

#include "../common/init.h"

#include "video_decoder.h"
#include "video_texturing.h"
#include "texture_image_data.h"

#include "bchp_m2mc.h"
#include "geometry.h"
#include "formats_360.h"
#include "shader_programs.h"
#include "camera.h"

#define M2MC_HAS_UIF_SUPPORT (BCHP_M2MC_REVISION_MAJOR_DEFAULT >= 2)

namespace video_texturing
{

const uint32_t DemoTwistSeconds = 5;
const uint32_t ZoomSeconds      = 10;
const float    DefaultFov       = 45.0f;
const float    Default360Fov    = 80.0f;

template<typename T>
static T Clamp(T x, T mn, T mx)
{
   return std::min(std::max(x, mn), mx);
}

////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class to animation transitions in 360 video mode
////////////////////////////////////////////////////////////////////////////////////////////////
AnimationStep::AnimationStep(float minAngle, float maxAngle) :
   m_minAngle(minAngle),
   m_maxAngle(maxAngle),
   m_hermite(false), m_startAngle(0.0f), m_endAngle(0.0f),
   m_startTime(std::chrono::steady_clock::now()),
   m_endTime(std::chrono::steady_clock::now())
{
}

float AnimationStep::Sanitise(float angle)
{
   return Clamp(angle, m_minAngle, m_maxAngle);
}

void AnimationStep::Set(float startAngle, float endAngle,
                        std::chrono::time_point<std::chrono::steady_clock> startTime,
                        std::chrono::time_point<std::chrono::steady_clock> endTime)
{
   m_startAngle = Sanitise(startAngle);
   m_endAngle   = Sanitise(endAngle);
   m_startTime  = startTime;
   m_endTime    = endTime;
}

void AnimationStep::Update(float endAngle, std::chrono::time_point<std::chrono::steady_clock> endTime,
                           std::chrono::time_point<std::chrono::steady_clock> now)
{
   m_startAngle = AngleAtTime(now);
   m_endAngle   = Sanitise(endAngle);
   m_startTime  = now;
   m_endTime    = endTime;
}

float AnimationStep::AngleAtTime(std::chrono::time_point<std::chrono::steady_clock> time) const
{
   if (time >= m_endTime)
      return m_endAngle;

   if (time <= m_startTime)
      return m_startAngle;

   auto duration   = m_endTime - m_startTime;
   auto sinceStart = time - m_startTime;
   float t = (float)sinceStart.count() / duration.count();

   if (m_hermite)
   {
      // Use a hermite easing curve to get smooth acceleration in and out
      t = 3.0f * t * t - 2.0f * t * t * t;
   }

   return m_startAngle * (1.0f - t) + m_endAngle * t;
}

////////////////////////////////////////////////////////////////////////////////////////////////
// The Application
////////////////////////////////////////////////////////////////////////////////////////////////
Application::Application(int argc, const char *argv[]) :
      m_showFPS(false), m_useMultisample(false),
      m_stretchToFit(false), m_benchmark(false),
      m_secure(false), m_showVideo(false), m_ambisonic(false),
      m_stereoAudio(false), m_anisoFiltering(false), m_vpX(0), m_vpY(0),
      m_vpW(1280), m_vpH(720), m_bpp(32), m_frames(0),
      m_swapInterval(1), m_clientId(0),
      m_texW(0), m_texH(0), m_maxMiplevels(1),
#if SINGLE_PROCESS
      m_decodeBuffers(2),
#else
      m_decodeBuffers(3),
#endif
      m_decodeFormat(BEGL_BufferFormat_eA8B8G8R8),
      m_mode(Demo),
      m_360Format(FORMAT_EQUIRECT),
      m_nexusDisplay(), m_eglDisplay(EGL_NO_DISPLAY),
      m_nativeWindow(NULL), m_nxplHandle(),
      m_gfx2d(NULL), m_usingMipmapM2MC(false),
      m_animYaw(-std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
      m_animPitch(-89.9f, 89.9f),
      m_animRoll(-89.9f, 89.9f),
      m_animFov(5.0f, 140.0f),
      m_exit(false), m_glDrawTexiOESFunc(NULL),
      m_compareModeSwitchTime(std::chrono::steady_clock::now())
{
   ProcessArgs(argc, argv);

   if (m_mode == Compare)
   {
      m_texW      = m_vpW;
      m_texH      = m_vpH;
      m_showVideo = true;
   }
   else if (m_mode == FullScreen)
   {
      m_texW      = m_vpW;
      m_texH      = m_vpH;
      m_showVideo = false;
   }

   if (m_benchmark)
   {
      // Override command line
      m_mode           = FullScreen;
      m_vpW            = 1920;
      m_vpH            = 1080;
      m_useMultisample = false;
      m_showVideo      = false;
      m_bpp            = 32;
      m_stretchToFit   = false;
      m_showFPS        = false;
      m_swapInterval   = 0;
      m_texW           = 1920;
      m_texH           = 1080;
      m_maxMiplevels   = 1;
      m_anisoFiltering = false;
#if SINGLE_PROCESS
      m_decodeBuffers  = 2;
#else
      m_decodeBuffers  = 3;
#endif
#if VC5 && M2MC_HAS_UIF_SUPPORT
      m_decodeFormat   = BEGL_BufferFormat_eTILED;
#else
      m_decodeFormat   = BEGL_BufferFormat_eA8B8G8R8;
#endif
   }

   float panelAspect = 1.0f;

   // Setup the display and EGL
   InitPlatformAndDisplay(&panelAspect);
   InitGraphics2D();

   // Initialise the remote input handler
   m_remoteInput = binput_open(NULL);
   if (!m_remoteInput)
      throw "Couldn't open the remote control handler";
   binput_set_mask(m_remoteInput, 0xFFFFFFFF);  // All inputs

   // Initialise EGL now we have a 'window'
   InitEGL();

   // Probe the video
   MediaProber::Instance()->GetStreamData(m_videoFile, &m_mediaData);

   // Create texture surfaces etc for each video
   CreateVideoTextures();

   // Setup the local state
   InitGLState();
   InitGLViewPort();

   printf("Rendering ");
   if (m_frames != 0)
      printf("%u frames", m_frames);

   printf(" : press CTRL+C to terminate early\n");

   // Build the list of decode surfaces (the nxclient SimpleDecoder requires this)
   std::vector<NEXUS_SurfaceHandle> decodeSurfaces;
   for (uint32_t b = 0; b < m_texture.NumBuffers(); b++)
      decodeSurfaces.push_back(m_texture.GetNativePixmap(b));

   m_videoDecoder.reset(new VideoDecoder(m_mediaData, decodeSurfaces, m_showVideo,
                                         m_secure, m_ambisonic, m_stereoAudio, m_nexusDisplay));

   m_texture.SetDecoder(m_videoDecoder->GetVideoDecoder());

   m_videoDecoder->StartPlayback();
}

Application::~Application()
{
   binput_close(m_remoteInput);

   m_videoDecoder.reset();

   TerminateGLState();

   EGLDisplay disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   eglMakeCurrent(disp, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(disp);

   TermGraphics2D();

   NXPL_DestroyNativeWindow(m_nativeWindow);
   NXPL_UnregisterNexusDisplayPlatform(m_nxplHandle);

   // Close the platform
   TermPlatform(m_nexusDisplay);
}

// Poll for key presses on the remote
void Application::PollKeyPress()
{
   b_remote_key key;

   const float    inc = 10.0f;
   const uint32_t timeStep = 500;   // Milliseconds
   bool           repeat;

   // get next remote key if any
   if (binput_read(m_remoteInput, &key, &repeat) == 0)
   {
      std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
      auto endTime = now + std::chrono::milliseconds(timeStep);

      if (key == b_remote_key_power)
      {
         m_exit = true;
         return;
      }

      if (m_mode == e360)
      {
         switch (key)
         {
         case b_remote_key_up:     // More pitch change per frame
            m_animPitch.Update(m_animPitch.EndAngle() + inc, endTime, now);
            break;
         case b_remote_key_down:   // Less pitch change per frame
            m_animPitch.Update(m_animPitch.EndAngle() - inc, endTime, now);
            break;
         case b_remote_key_left:   // More left yaw per frame
            m_animYaw.Update(m_animYaw.EndAngle() - inc, endTime, now);
            break;
         case b_remote_key_right:  // More right yaw per frame
            m_animYaw.Update(m_animYaw.EndAngle() + inc, endTime, now);
            break;
         case b_remote_key_play:   // More anti-clockwise roll per frame
            m_animRoll.Update(m_animRoll.EndAngle() - inc, endTime, now);
            break;
         case b_remote_key_pause:  // More clockwise roll per frame
            m_animRoll.Update(m_animRoll.EndAngle() + inc, endTime, now);
            break;
         case b_remote_key_fast_forward: // Zoom in
            m_animFov.Update(m_animFov.EndAngle() - 5.0f, endTime, now);
            break;
         case b_remote_key_rewind:       // Zoom out
            m_animFov.Update(m_animFov.EndAngle() + 5.0f, endTime, now);
            break;

         // reset camera to center
         case b_remote_key_select:
         case b_remote_key_back:
         case b_remote_key_info:
         case b_remote_key_guide:
         case b_remote_key_menu:
         case b_remote_key_clear:
            m_animYaw.Set(m_animYaw.AngleAtTime(now), 0.0f, now, endTime);
            m_animPitch.Set(m_animPitch.AngleAtTime(now), 0.0f, now, endTime);
            m_animRoll.Set(m_animRoll.AngleAtTime(now), 0.0f, now, endTime);
            m_animFov.Set(m_animFov.AngleAtTime(now), Default360Fov, now, endTime);
            break;

         default:
            break;
         }
      }
   }
}

// Run the benchmark mode loop
void Application::RunBenchmark()
{
   // Run for 500 frames to ensure caches are primed, etc.
   printf("Priming...\n");
   uint32_t frame = 0;
   while (frame <= 500)
   {
      Display();
      frame++;
   }

   printf("Benchmarking...\n");

   std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();

   // Time the next 2000 frames
   while (frame <= 2500)
   {
      Display();
      frame++;
   }

   std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
   std::chrono::duration<float>                       elapsedSeconds = end - start;

   printf("Benchmark average = %f fps\n", 2000.0f / elapsedSeconds.count());
}

// The main application loop
void Application::Run()
{
   if (m_benchmark)
   {
      RunBenchmark();
   }
   else
   {
      uint32_t frame = 0;
      while (!m_exit && (m_frames == 0 || frame <= m_frames))
      {
         PollKeyPress();
         Display();
         frame++;
      }
   }
}

static void CompileShader(GLuint s)
{
   glCompileShader(s);

   GLint ret;
   glGetShaderiv(s, GL_COMPILE_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetShaderiv(s, GL_INFO_LOG_LENGTH, &ret);
      char *p = (char *)alloca(ret);
      glGetShaderInfoLog(s, ret, NULL, p);
      fprintf(stderr, "Shader compile error:\n%s\n", p);
      throw "Shader compilation failed";
   }
}

static void LinkProgram(GLint p)
{
   glLinkProgram(p);

   GLint ret;
   glGetProgramiv(p, GL_LINK_STATUS, &ret);
   if (ret == GL_FALSE)
   {
      glGetProgramiv(p, GL_INFO_LOG_LENGTH, &ret);
      if (ret > 0)
      {
         char *r = (char *)alloca(ret);
         glGetProgramInfoLog(p, ret, NULL, r);
         printf("Shader link error:\n%s\n", r);
      }
      throw "Shader link failed";
   }
}

// Initialize the GL state we require
void Application::InitGLState()
{
   glClearDepthf(1.0f);
   glDepthFunc(GL_LEQUAL); // To allow the overlay to be drawn at the same depth

   if (m_mode == Compare)
      glClearColor(0.2f, 0.2f, 0.2f, 0.0f);   // Fully transparent background
   else
      glClearColor(0.2f, 0.2f, 0.2f, 0.75f);  // Gray, slightly transparent background

   glEnable(GL_DEPTH_TEST);

   glFrontFace(GL_CW);        // Our geometry is configured for clockwise ordering
   glCullFace(GL_BACK);       // Don't draw back-facing geometry
   glEnable(GL_CULL_FACE);

   // Create geometry
   m_geometry.reset(new Geometry(m_mode == e360, m_360Format, m_texW, m_texH));

   GLuint v = glCreateShader(GL_VERTEX_SHADER);
   GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

   const char *vv = VertexShaderStr(m_mode == e360, m_360Format);
   const char *ff = FragmentShaderStr(m_mode == e360, m_360Format);

   glShaderSource(v, 1, &vv, NULL);
   glShaderSource(f, 1, &ff, NULL);

   // Compile the shaders
   CompileShader(v);
   CompileShader(f);

   // Link the program
   m_programObject = glCreateProgram();
   glAttachShader(m_programObject, v);
   glAttachShader(m_programObject, f);
   LinkProgram(m_programObject);

   // Get the attribute locations
   m_positionLoc = glGetAttribLocation(m_programObject, "a_position");
   m_tcLoc       = glGetAttribLocation(m_programObject, "a_texcoord");

   // Get the uniform locations
   m_mvpMatrixLoc = glGetUniformLocation(m_programObject, "u_mvpMatrix");
   m_texUnitLoc   = glGetUniformLocation(m_programObject, "u_textureUnit");

   // Make a texture for the "GL Texturing" image we want to display in "compare" mode
   glGenTextures(1, &m_overlayTextureId);
   glBindTexture(GL_TEXTURE_2D, m_overlayTextureId);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s_textImageData.width, s_textImageData.height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, s_textImageData.pixelData);

   esMatrixLoadIdentity(&m_projectionMatrix);
   esMatrixLoadIdentity(&m_modelviewMatrix);
   esMatrixLoadIdentity(&m_mvpMatrix);
   esMatrixLoadIdentity(&m_modelMatrix);

   std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

   m_camera.SetPosition(ESVec3{ 0.0f, 0.0f, 0.0f });
   m_camera.SetFov(DefaultFov);

   if (m_mode == Demo || m_mode == Zoom)
   {
      // Adjust for video aspect ratio
      float aspect = static_cast<float>(m_texW) / m_texH;
      esScale(&m_modelMatrix, 100, (uint32_t)(100.0f / aspect), (uint32_t)(100.0f / aspect));
      m_camera.SetPosition(ESVec3{ 0.0f, 0.0f, -240.0f });

      if (m_mode == Demo)
      {
         m_animYaw.SetHermite(true);
         m_animYaw.Update(87.0f, now + std::chrono::seconds(DemoTwistSeconds), now);
      }
      else if (m_mode == Zoom)
      {
         m_animFov.Set(DefaultFov, DefaultFov, now, now);
         m_animFov.SetHermite(true);
         m_animFov.Update(140.0f, now + std::chrono::seconds(ZoomSeconds), now);
      }
   }
   else if (m_mode == e360)
   {
      m_animFov.Set(Default360Fov, Default360Fov, now, now);
      esScale(&m_modelMatrix, 100, 100, 100);

      // The 360 geometry may need rotating to get the front face correct
      esRotate(&m_modelMatrix, m_geometry->GetModelYRotate(), 0.0f, 1.0f, 0.0f);
   }
}

void Application::TerminateGLState()
{
   glDeleteProgram(m_programObject);

   m_geometry.reset();

   m_texture.Destroy();
}

void Application::InitGLViewPort()
{
   glViewport(0, 0, m_vpW, m_vpH);
}

void Application::Resize()
{
   EGLint w = 0, h = 0;

   // As this is just an example, and we don't have any kind of resize event, we will
   // check whether the underlying window has changed size and adjust our viewport at the start of
   // each frame. Obviously, this would be more efficient if event driven.
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_WIDTH, &w);
   eglQuerySurface(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW), EGL_HEIGHT, &h);

   if ((uint32_t)w != m_vpW || (uint32_t)h != m_vpH)
   {
      m_vpW = w;
      m_vpH = h;
      InitGLViewPort();
   }
}

// Show the current frame-rate
static void ShowFPS()
{
   static int lastPrintFrame = 0;
   static int frame          = 0;

   static std::chrono::time_point<std::chrono::steady_clock> lastPrintTime;
   std::chrono::time_point<std::chrono::steady_clock>        now = std::chrono::steady_clock::now();
   std::chrono::duration<float>                              elapsedSeconds = now - lastPrintTime;

   frame++;

   if (elapsedSeconds.count() > 1.0f || lastPrintFrame == 0)
   {
      if (elapsedSeconds.count() != 0.0f && lastPrintTime != std::chrono::time_point<std::chrono::steady_clock>())
      {
         float fps = (float)(frame - lastPrintFrame) / elapsedSeconds.count();
         printf("Frames per second: %f   Average fence wait: %f ms\n", fps, Fence::AverageFenceWaitMS());
      }

      lastPrintFrame = frame;
      lastPrintTime  = now;
   }
}

static void BppToChannels(int bpp, int *r, int *g, int *b, int *a)
{
   switch (bpp)
   {
   default:
   case 16:   // 16-bit RGB (565)
      *r = 5;
      *g = 6;
      *b = 5;
      *a = 0;
      break;

   case 32:   // 32-bit RGBA
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 8;
      break;

   case 24:   // 24-bit RGB
      *r = 8;
      *g = 8;
      *b = 8;
      *a = 0;
      break;
   }
}

// Small helper class for building EGL attribute arrays
class AttribArray
{
public:
   void Append(EGLint key, EGLint value)
   {
      m_data.emplace_back(key);
      m_data.emplace_back(value);
   }

   void Append(EGLint value)
   {
      m_data.emplace_back(value);
   }

   operator const EGLint*() { return m_data.data(); }

private:
   std::vector<EGLint> m_data;
};

// Initialize EGL
void Application::InitEGL()
{
   EGLint     majorVersion;
   EGLint     minorVersion;

   // Specifies the required configuration attributes.
   // An EGL "configuration" describes the pixel format and type of
   // surfaces that can be used for drawing.
   // For now we just want to use a 16 bit RGB surface that is a
   // Window surface, i.e. it will be visible on screen. The list
   // has to contain key/value pairs, terminated with EGL_NONE.
   int wantRed   = 0;
   int wantGreen = 0;
   int wantBlue  = 0;
   int wantAlpha = 0;

   BppToChannels(m_bpp, &wantRed, &wantGreen, &wantBlue, &wantAlpha);

   // Step 1 - Get the EGL display.
   m_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   if (m_eglDisplay == EGL_NO_DISPLAY)
      throw "eglGetDisplay() failed, did you register any exclusive displays";

   // Step 2 - Initialize EGL
   if (!eglInitialize(m_eglDisplay, &majorVersion, &minorVersion))
      throw "eglInitialize() failed";

   // Step 3 - Get the number of configurations to correctly size the array used in step 4
   int numConfigs;
   if (!eglGetConfigs(m_eglDisplay, NULL, 0, &numConfigs))
      throw "eglGetConfigs() failed";

   std::vector<EGLConfig>  eglConfig(numConfigs);

   // Step 4 - Find a config that matches all requirements.
   {
      AttribArray attr;

      attr.Append(EGL_RED_SIZE,        wantRed);
      attr.Append(EGL_GREEN_SIZE,      wantGreen);
      attr.Append(EGL_BLUE_SIZE,       wantBlue);
      attr.Append(EGL_ALPHA_SIZE,      wantAlpha);
      attr.Append(EGL_DEPTH_SIZE,      24);
      attr.Append(EGL_STENCIL_SIZE,    0);
      attr.Append(EGL_SURFACE_TYPE,    EGL_WINDOW_BIT);
      attr.Append(EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT);

      if (m_useMultisample)
      {
         attr.Append(EGL_SAMPLE_BUFFERS, 1);
         attr.Append(EGL_SAMPLES,        4);
      }

      attr.Append(EGL_NONE);

      if (numConfigs == 0 || !eglChooseConfig(m_eglDisplay, attr, eglConfig.data(), numConfigs, &numConfigs))
         throw "eglChooseConfig() failed";
   }

   int configSelect;
   for (configSelect = 0; configSelect < numConfigs; configSelect++)
   {
      // Configs with deeper color buffers get returned first by eglChooseConfig.
      // Applications may find this counterintuitive, and need to perform additional processing on the list of
      // configs to find one best matching their requirements. For example, specifying RGBA depths of 565
      // could return a list whose first config has a depth of 888.

      // Check that config is an exact match
      EGLint redSize, greenSize, blueSize, alphaSize, depthSize;

      eglGetConfigAttrib(m_eglDisplay, eglConfig[configSelect], EGL_RED_SIZE,   &redSize);
      eglGetConfigAttrib(m_eglDisplay, eglConfig[configSelect], EGL_GREEN_SIZE, &greenSize);
      eglGetConfigAttrib(m_eglDisplay, eglConfig[configSelect], EGL_BLUE_SIZE,  &blueSize);
      eglGetConfigAttrib(m_eglDisplay, eglConfig[configSelect], EGL_ALPHA_SIZE, &alphaSize);
      eglGetConfigAttrib(m_eglDisplay, eglConfig[configSelect], EGL_DEPTH_SIZE, &depthSize);

      if ((redSize == wantRed) && (greenSize == wantGreen) && (blueSize == wantBlue) && (alphaSize == wantAlpha))
      {
         printf("Selected config: R=%u G=%u B=%u A=%u Depth=%u\n", redSize, greenSize, blueSize, alphaSize, depthSize);
         break;
      }
   }

   if (configSelect == numConfigs)
      throw "No suitable configs found";

   // Step 5 - Create a surface to draw to.
   // Use the config picked in the previous step and the native window
   // handle to create a window surface.
   EGLSurface eglSurface = 0;
   {
      AttribArray attr;
#ifdef EGL_PROTECTED_CONTENT_EXT
      if (m_secure)
         attr.Append(EGL_PROTECTED_CONTENT_EXT, EGL_TRUE);
#endif
      attr.Append(EGL_NONE);

      eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig[configSelect], m_nativeWindow, attr);
   }

   if (eglSurface == EGL_NO_SURFACE)
   {
      eglGetError(); // Clear error
      eglSurface = eglCreateWindowSurface(m_eglDisplay, eglConfig[configSelect], NULL, NULL);
   }

   if (eglSurface == EGL_NO_SURFACE)
      throw "eglCreateWindowSurface() failed";

   // Step 6 - Create a context.
   // EGL has to create a context for OpenGL ES. Our OpenGL ES resources
   // like textures will only be valid inside this context (or shared contexts)
   EGLContext eglContext = 0;
   {
      AttribArray attr;
#ifdef EGL_PROTECTED_CONTENT_EXT
      if (m_secure)
         attr.Append(EGL_PROTECTED_CONTENT_EXT, EGL_TRUE);
#endif
      attr.Append(EGL_CONTEXT_CLIENT_VERSION, 2);
      attr.Append(EGL_NONE);

      eglContext = eglCreateContext(m_eglDisplay, eglConfig[configSelect], EGL_NO_CONTEXT, attr);
      if (eglContext == EGL_NO_CONTEXT)
         throw "eglCreateContext() failed";
   }

   // Step 7 - Bind the context to the current thread and use our
   // window surface for drawing and reading.
   // We need to specify a surface that will be the target of all
   // subsequent drawing operations, and one that will be the source
   // of read operations. They can be the same surface.
   eglMakeCurrent(m_eglDisplay, eglSurface, eglSurface, eglContext);

   // Set the swap interval. 1 is almost always the right value. 0 is for benchmarking only, as tearing
   // will be introduced. Values >1 result in slower performance.
   eglSwapInterval(m_eglDisplay, m_swapInterval);
}

void Application::InitPlatformAndDisplay(float *aspect)
{
   NXPL_NativeWindowInfoEXT   winInfo;

   eInitResult res = InitPlatformAndDefaultDisplay(&m_nexusDisplay, aspect, m_vpW, m_vpH, m_secure);
   if (res != eInitSuccess)
      throw "Unable to initialize platform and default display";

   // Register the Nexus display with the platform layer. The platform layer then controls the display.
   NXPL_RegisterNexusDisplayPlatform(&m_nxplHandle, m_nexusDisplay);

   NXPL_GetDefaultNativeWindowInfoEXT(&winInfo);

   winInfo.x        = m_vpX;
   winInfo.y        = m_vpY;
   winInfo.width    = m_vpW;
   winInfo.height   = m_vpH;
   winInfo.stretch  = m_stretchToFit;
   winInfo.clientID = m_clientId;

   m_nativeWindow = NXPL_CreateNativeWindowEXT(&winInfo);

#if SINGLE_PROCESS
   // Enable blending with video plane
   NEXUS_GraphicsSettings graphicsSettings;
   NEXUS_Display_GetGraphicsSettings(m_nexusDisplay, &graphicsSettings);
   graphicsSettings.sourceBlendFactor = NEXUS_CompositorBlendFactor_eSourceAlpha;
   graphicsSettings.destBlendFactor   = NEXUS_CompositorBlendFactor_eInverseSourceAlpha;
   NEXUS_Display_SetGraphicsSettings(m_nexusDisplay, &graphicsSettings);
#endif
}

// NEXUS_Graphics2D is used to convert from the source striped video into destriped versions
// for the textures
void Application::InitGraphics2D()
{
   NEXUS_Graphics2DSettings     gfxSettings;
   NEXUS_Graphics2DOpenSettings graphics2dOpenSettings;

   NEXUS_Graphics2D_GetDefaultOpenSettings(&graphics2dOpenSettings);
   graphics2dOpenSettings.secure = m_secure;

   // Prepare the graphics2D module for destriping the video frames into our texture surface
   // Currently the mipmap M2MC is artifically limited to writing only UIF.
#if VC5
   if (m_decodeFormat == BEGL_BufferFormat_eTILED)
   {
      graphics2dOpenSettings.mode = NEXUS_Graphics2DMode_eMipmap;
      m_gfx2d = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
   }
#endif

   if (m_gfx2d == NULL)
   {
      // Use normal M2MC when mipmap version not available and for non TILED formats
      graphics2dOpenSettings.mode = NEXUS_Graphics2DMode_eBlitter;
      m_gfx2d = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, &graphics2dOpenSettings);
      if (m_gfx2d == NULL)
         throw "NEXUS_Graphics2D_Open failed";

      m_maxMiplevels = 1;
      printf("Found Blitter M2MC, destripe with mipmap generation NOT available\n");
   }
   else
   {
      printf("Found Mipmap M2MC, destripe with mipmap generation is available\n");
      m_usingMipmapM2MC = true;
   }

   // We'll need an event that will be triggered when the m2mc completes
   BKNI_CreateEvent(&m_m2mcDone);

   // Configure the gfx2d checkpoint to call BKNI_SetEvent
   NEXUS_Graphics2D_GetSettings(m_gfx2d, &gfxSettings);
   gfxSettings.checkpointCallback.callback = [](void *data, int) { BKNI_SetEvent((BKNI_EventHandle)data); };
   gfxSettings.checkpointCallback.context  = m_m2mcDone;
   NEXUS_Graphics2D_SetSettings(m_gfx2d, &gfxSettings);
}

void Application::TermGraphics2D()
{
   if (m_gfx2d)
   {
      NEXUS_Graphics2D_Close(m_gfx2d);
      m_gfx2d = NULL;
   }

   if (m_m2mcDone)
   {
      BKNI_DestroyEvent(m_m2mcDone);
      m_m2mcDone = NULL;
   }
}

static std::string SupportedDecodeFormats()
{
   std::string formats = "RGBA8888|RGB565";
#if VC5
   formats += "|RGBA4444|RGBA5551|YUV422";
#if M2MC_HAS_UIF_SUPPORT
   formats += "|TILED";
#endif
#endif
   return formats;
}

void Application::ProcessArgs(int argc, const char *argv[])
{
   bool badArgs = false;

   try
   {
      for (int a = 1; a < argc; ++a)
      {
         const char  *arg = argv[a];

         if (strcmp(arg, "+m") == 0)
            m_useMultisample = true;
         else if (strcmp(arg, "+fps") == 0)
            m_showFPS = true;
         else if (strcmp(arg, "+bench") == 0)
            m_benchmark = true;
         else if (strcmp(arg, "+video_plane") == 0)
            m_showVideo = true;
         else if (strcmp(arg, "+s") == 0)
            m_stretchToFit = true;
         else if (strncmp(arg, "d=", 2) == 0)
         {
            if (sscanf(arg, "d=%ux%u", &m_vpW, &m_vpH) != 2)
               throw false;
         }
         else if (strncmp(arg, "o=", 2) == 0)
         {
            if (sscanf(arg, "o=%ux%u", &m_vpX, &m_vpY) != 2)
               throw false;
         }
         else if (strncmp(arg, "bpp=", 4) == 0)
         {
            if (sscanf(arg, "bpp=%u", &m_bpp) != 1)
               throw false;
         }
         else if (strncmp(arg, "f=", 2) == 0)
         {
            if (sscanf(arg, "f=%u", &m_frames) != 1)
               throw false;
         }
         else if (strncmp(arg, "client=", 7) == 0)
         {
            if (sscanf(arg, "client=%u", &m_clientId) != 1)
               throw false;
         }
         else if (strncmp(arg, "video=", 6) == 0)
         {
            if (m_videoFile != "")
               throw false;

            char path[PATH_MAX];
            if (sscanf(arg, "video=%s", path) != 1)
               throw false;

            m_videoFile = path;
         }
         else if (strncmp(arg, "swap=", 5) == 0)
         {
            if (sscanf(arg, "swap=%u", &m_swapInterval) != 1)
               throw false;
         }
         else if (strncmp(arg, "t=", 2) == 0)
         {
            if (sscanf(arg, "t=%ux%u", &m_texW, &m_texH) != 2)
               throw false;
         }
         else if (strncmp(arg, "decode_buffers=", 15) == 0)
         {
            if (sscanf(arg, "decode_buffers=%u", &m_decodeBuffers) != 1)
               throw false;
            if (m_decodeBuffers <= 1)
               throw false;
#if !SINGLE_PROCESS
            if (m_decodeBuffers > NEXUS_SIMPLE_DECODER_MAX_SURFACES)
            {
               fprintf(stderr, "decode_buffers must <= %u\n", NEXUS_SIMPLE_DECODER_MAX_SURFACES);
               throw false;
            }
#endif
         }
         else if (strncmp(arg, "decode_format=", 14) == 0)
         {
            char fmtStr[32];
            if (sscanf(arg, "decode_format=%s", fmtStr) != 1)
               throw false;

            std::string fmt = fmtStr;

            bool littleEndian = true;
#ifdef BIG_ENDIAN_CPU
            littleEndian = false;
#endif
            if (fmt == "YUV422")
               m_decodeFormat = BEGL_BufferFormat_eYUV422;
            else if (fmt == "RGB565")
               m_decodeFormat = BEGL_BufferFormat_eR5G6B5;
            else if (fmt == "RGBA8888")
               m_decodeFormat = littleEndian ? BEGL_BufferFormat_eA8B8G8R8 : BEGL_BufferFormat_eR8G8B8A8;
#if VC5
            else if (fmt == "RGBA4444")
               m_decodeFormat = littleEndian ? BEGL_BufferFormat_eA4B4G4R4 : BEGL_BufferFormat_eR4G4B4A4;
            else if (fmt == "RGBA5551")
               m_decodeFormat = littleEndian ? BEGL_BufferFormat_eA1B5G5R5 : BEGL_BufferFormat_eR5G5B5A1;
#if M2MC_HAS_UIF_SUPPORT
            else if (fmt == "TILED")
               m_decodeFormat = BEGL_BufferFormat_eTILED;
#endif
#endif
            else
               throw false;
         }
         else if (strncmp(arg, "mode=", 5) == 0)
         {
            char modeStr[32];
            if (sscanf(arg, "mode=%s", modeStr) != 1)
               throw false;

            std::string mode = modeStr;
            if (mode == "demo")
               m_mode = Demo;
            else if (mode == "zoom")
               m_mode = Zoom;
            else if (mode == "fullscreen")
               m_mode = FullScreen;
            else if (mode == "compare")
               m_mode = Compare;
            else if (mode == "360")
               m_mode = e360;
            else
               throw false;
         }
         else if (strncmp(arg, "360_format=", 11) == 0)
         {
            if (sscanf(arg, "360_format=%u", (uint32_t*)&m_360Format) != 1)
               throw false;
         }
         else if (strcmp(arg, "+secure") == 0)
            m_secure = true;
         else if (strcmp(arg, "+ambisonic") == 0)
            m_ambisonic = true;
         else if (strcmp(arg, "+stereo_audio") == 0)
            m_stereoAudio = true;
         // The NxClient simple video decoder capture does not yet support
         // destriping to a mipmapped surface, so only allow mipmaps in single
         // process mode for now.
#if GL_ES_VERSION_3_0
         else if (strncmp(arg, "miplevels=", 10) == 0)
         {
            if (sscanf(arg, "miplevels=%u", &m_maxMiplevels) != 1)
               throw false;
         }
         else if (strncmp(arg, "+aniso", 6) == 0)
            m_anisoFiltering = true;
#endif
         else
            throw false;
      }
   }
   catch (...)
   {
      badArgs = true;
   }

#ifndef EGL_PROTECTED_CONTENT_EXT
   if (m_secure)
      printf("+secure selected, but headers not available in this driver version. defaulting off\n");
   m_secure = false;
#endif

   if (m_videoFile == "")
      badArgs = true;

   if (badArgs)
   {
      const char *progname = argc > 0 ? argv[0] : "";
      fprintf(stderr,
         "Usage: %s video=<filename> [OPTION]...\n"
         "  +video_plane        show a darkened real video plane in the background\n"
         "  +m                  use multi-sampling (defaults off)\n"
#if GL_ES_VERSION_3_0
         "  +aniso              use anisotropic texture filtering (defaults off)\n"
         "                      Requires decode format to be TILED and number of\n"
         "                      miplevels > 1 to have any effect on rendering\n"
#endif
         "  +s                  stretch to fit display (defaults off)\n"
         "  +fps                show frames-per-second on console (defaults off)\n"
         "  +bench              run benchmark. This will override most of the\n"
         "                      command-line options and time a run of 2000 frames.\n"
         "  +secure             use secure videos, textures and display (defaults off)\n"
         "  +ambisonic          use Ambisonic audio processing for 360 audio (defaults off)\n"
         "  +stereo_audio       forces stereo output instead of multi-channel.\n"
         "                      Multi-channel output only works over HDMI.\n"
         "  d=WxH               set output resolution (default 1280x720)\n"
         "  o=XxY               set output origin (default 0,0)\n"
         "  t=WxH               set texture resolution for video decode\n"
         "                      (defaults to half the display resolution)\n"
         "  swap=N              set the OpenGLES swap interval (default 1)\n"
         "  bpp=[16|24|32]      set output color depth in bits (default 32)\n"
         "  f=N                 exit after rendering N frames\n"
         "  client=N            set the nxclient id\n"
         "  decode_buffers=N    use N buffers for decoding (default 2 for single process,\n"
         "                      3 in nxclient multi-process mode)\n"
         "                      must be >=2\n"
         "  decode_format=[%s]\n"
         "                      set the texture format for the video decode\n"
         "                      (default RGBA8888)\n"
#if GL_ES_VERSION_3_0
         "  miplevels=N         use up to N miplevels when mipmapping\n"
         "                      destripe is available (default 1)\n"
#endif
         "  mode=[demo|zoom|fullscreen|compare|360]\n"
         "                      'demo' plays a video texture on a rotating surface\n"
         "                      'zoom' plays a video texture zooming in and out\n"
         "                      'fullscreen' plays a fullscreen video texture\n"
         "                      'compare' switches between fullscreen texture and\n"
         "                        fullscreen video once per second. The texture size is\n"
         "                        forced to match the display resolution in this mode.\n"
         "                      (default 'demo')\n"
         "                      '360' displays a 360 degree video\n"
         "  360_format=N        select the type of 360 video\n"
         "                      0=equirect(default)     1=cube_32_0     2=cube_32_90\n"
         "                      3=cube_32_270(YouTube)  4=cube_32_p270  5=cube_43_0\n"
         "                      6=fisheye               7=icosahedron   8=octahedron\n"
         "                      9=eap\n",
         progname, SupportedDecodeFormats().c_str());
      throw "Invalid arguments";
   }
}

// Create the multi-buffered texture objects (including underlying Nexus surfaces, sync fences etc.)
void Application::CreateVideoTextures()
{
   // Calculate a good size for the video texture. Don't make it bigger than the original though.
   float aspect = (float)m_mediaData.data.video[0].width / (float)m_mediaData.data.video[0].height;

   if (m_texW == 0)
   {
      if (m_mode == Demo)
      {
         m_texW = m_vpW / 2;
         if (m_texW > m_mediaData.data.video[0].width)
            m_texW = m_mediaData.data.video[0].width;
      }
      else
         m_texW = m_mediaData.data.video[0].width;
   }

   if (m_texH == 0)
      m_texH = m_texW / aspect;

   uint32_t mipLevels = 1;

#if VC5
   if (m_usingMipmapM2MC && m_decodeFormat == BEGL_BufferFormat_eTILED)
   {
#if GL_ES_VERSION_3_0
      while (mipLevels < m_maxMiplevels && ((m_texW >> mipLevels) != 0 || (m_texH >> mipLevels) != 0))
         mipLevels++;
#else
      // ES2 does not support GL_TEXTURE_MAX_LEVEL so all mipmap levels must be
      // fully populated.
      while ((m_texW >> mipLevels) != 0 || (m_texH >> mipLevels) != 0)
         mipLevels++;
#endif
   }
#endif

   printf("Texture size: %ux%u MipLevels: %u\n", m_texW, m_texH, mipLevels);

   // Create the underlying texture data and surfaces for all the buffers in the texture-chain.
   // If the textures are double-buffered (so that they can be being used by GL and de-striped
   // simultaneously) this will create multiple underlying surfaces etc.
   m_texture.Create(m_nxplHandle, m_gfx2d, m_m2mcDone, m_decodeBuffers,
                    m_mediaData.data.video[0].width, m_mediaData.data.video[0].height, m_texW, m_texH,
                    mipLevels, m_decodeFormat, m_anisoFiltering, m_secure);
}

// Update any animation for the next frame
void Application::UpdateAnimation()
{
   std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

   if (m_mode == e360)
   {
      m_camera.SetRotation(m_animYaw.AngleAtTime(now), m_animPitch.AngleAtTime(now), m_animRoll.AngleAtTime(now));
      m_camera.SetFov(m_animFov.AngleAtTime(now));
   }
   else if (m_mode == Demo)
   {
      float yaw = m_animYaw.AngleAtTime(now);
      if (yaw >= 87.0f)
         m_animYaw.Update(-87.0f, now + std::chrono::seconds(DemoTwistSeconds), now);
      else if (yaw <= -87.0f)
         m_animYaw.Update(87.0f, now + std::chrono::seconds(DemoTwistSeconds), now);

      m_camera.SetRotation(m_animYaw.AngleAtTime(now), 0.0f, 0.0f);
   }
   else if (m_mode == Zoom)
   {
      float fov = m_animFov.AngleAtTime(now);
      if (fov >= 140.0f)
         m_animFov.Update(20.0f, now + std::chrono::seconds(ZoomSeconds), now);
      else if (fov <= 20.0f)
         m_animFov.Update(140.0f, now + std::chrono::seconds(ZoomSeconds), now);

      m_camera.SetFov(m_animFov.AngleAtTime(now));
   }

   esMatrixLoadIdentity(&m_projectionMatrix);
   if (m_mode == Demo || m_mode == Zoom || m_mode == e360)
      esPerspective(&m_projectionMatrix, m_camera.GetFov(), (float)m_vpW / (float)m_vpH, 1, 5000);

   // Update spatial audio
   m_videoDecoder->SetSpatialAudio(m_camera.GetYaw(), m_camera.GetPitch(), m_camera.GetRoll());
}

// Draw the "GL Texturing" overlay to indicate when video-texturing is active
// in 'compare' mode
void Application::DrawTextOverlay()
{
   // Only valid when in fullscreen or compare mode as it uses the same rectangle geometry, just
   // in a smaller viewport with a different texture
   glViewport(10, 10, s_textImageData.width, s_textImageData.height);

   // Blend over video texture
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glEnable(GL_BLEND);

   glBindTexture(GL_TEXTURE_2D, m_overlayTextureId);
   m_geometry->Draw();

   // Restore changed state
   glDisable(GL_BLEND);
   glViewport(0, 0, m_vpW, m_vpH);
}

// Called once per frame to draw the content.
// Note: the GL display frame rate will not match the video frame rate.
void Application::Display()
{
   static bool justVideo = false;
   if (m_mode == Compare)
   {
      if (std::chrono::steady_clock::now() >= m_compareModeSwitchTime)
      {
         justVideo = !justVideo;
         m_compareModeSwitchTime += std::chrono::seconds(2);
      }
   }

   if (m_showFPS)
      ShowFPS();

   // Handle any resizing that may have occurred
   Resize();

   UpdateAnimation();

   // Update the video frame texture (will leave the texture untouched if no new frame is ready)
   m_texture.AcquireVideoFrame();

   // Compute the final MVP matrix
   ESMatrix viewMx = m_camera.GetViewMatrix();
   esMatrixMultiply(&m_modelviewMatrix, &m_modelMatrix, &viewMx);
   esMatrixMultiply(&m_mvpMatrix, &m_modelviewMatrix, &m_projectionMatrix);

   // Clear all the buffers we asked for during config to ensure fast-path
   glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(m_programObject);

   // Bind the geometry
   m_geometry->Bind(m_positionLoc, m_tcLoc);

   // Load the latest MVP matrix into its uniform
   glUniformMatrix4fv(m_mvpMatrixLoc, 1, GL_FALSE, (GLfloat*)&m_mvpMatrix.m[0][0]);
   glUniform1i(m_texUnitLoc, 0);

   // Bind the current buffer of the video-texture
   bool texValid = m_texture.BindTexture(GL_TEXTURE_2D);

   if (!justVideo && texValid)
   {
      // Draw the geometry
      m_geometry->Draw();

      if (m_mode == Compare)
         DrawTextOverlay();
   }

   // Put a fence in the timeline for the texture
   // The fence will trigger when all previous commands have been completed (i.e. when the
   // texture is no longer in use by GL and can be re-written).
   m_texture.InsertFence();

   // Post the framebuffer for display
   eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
}

} // namespace video_texturing

using namespace video_texturing;

#ifndef CLIENT_MAIN
#define CLIENT_MAIN main
#endif

int CLIENT_MAIN(int argc, const char** argv)
{
   try
   {
      Application app(argc, argv);
      app.Run();
   }
   catch (const char *err)
   {
      printf("Error : %s\n", err);
      return 1;
   }
   catch (...)
   {
      printf("Caught unknown error\n");
      return 2;
   }
   return 0;
}
