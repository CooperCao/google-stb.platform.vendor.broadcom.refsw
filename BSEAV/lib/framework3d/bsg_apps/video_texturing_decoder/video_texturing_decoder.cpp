/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "video_texturing_decoder.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"
#include "bsg_parse_utils.h"
#include "bsg_circular_index.h"

#include <iostream>
#include <istream>
#include <fstream>

using namespace bsg;

// Texturing configuration (not all variants are valid)

#ifdef EMULATED
/* Must use eTEX_IMAGE_2D & RGBX8888 when emulated */
static GLTexture::eVideoTextureMode  s_textureMode = GLTexture::eTEX_IMAGE_2D;
static eVideoFrameFormat s_decodeFormat = eRGBX8888;
#else
// Texturing mode [eEGL_IMAGE|eEGL_IMAGE_EXPLICIT|eTEX_IMAGE_2D]
static GLTexture::eVideoTextureMode  s_textureMode = GLTexture::eEGL_IMAGE_EXPLICIT;

// Video format [YUV422|RGB565|RGBX8888]
static eVideoFrameFormat s_decodeFormat = eRGBX8888;
#endif

// Update mode [EXT_SYNC|EXT|ALWAYS_SYNC|ALWAYS|EGL_SYNC]
static eVideoUpdateMode s_updateMode = eEGL_SYNC;

const float carouselPaneX = 0.52f;
const float idleTimeToAuto = 20.0f;
const float autoChangeTime = 7.0f;

static std::vector<std::string>   s_videoFiles;

static uint32_t      s_decodeWidth = 0;
static uint32_t      s_decodeHeight = 0;
static uint32_t      s_decodeBuffers = 2;
static uint32_t      s_audioDelay = 16;      // 1 frame delay for audio initially

static bool          s_bench       = false;
static bool          s_oldBackdrop = false;
static bool          s_highQuality = true;
static uint32_t      s_benchFrameCnt = 0;
static Time          s_benchStartTime;
static FILE          *s_benchFp = NULL;

static Platform      *s_platform;

static float         s_titleX = 0.68f;
static float         s_titleY = 0.82f;
static float         s_filterX = 0.68f;
static float         s_filterY = 0.75f;

static float         s_monitorX      = 0.04f;
static float         s_monitorRotMin = -4.0f;
static float         s_monitorRotMax = 45.0f;

static unsigned int s_countFramesBeforePlayback = 0;
static unsigned int s_nbFramesDelay = 2;

/////////////////////////////////////////////////////////////////////
class CustomArgumentParser : public ArgumentParser
{
public:
   //! Process a command line argument.
   //! Return true if you recognize the argument and have handled it.
   //! Return false to indicate this is an option you don't recognize - an error.
   virtual bool ParseArgument(const std::string &arg)
   {
      char c[1024];
      if (ApplicationOptions::ArgMatch(arg.c_str(), "+bench"))
      {
         s_bench = true;
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "+old"))
      {
         s_oldBackdrop = true;
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "video="))
      {
         if (sscanf(arg.c_str(), "video=%s", c) == 1)
         {
            s_videoFiles.push_back(c);
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "quality="))
      {
         s_highQuality = arg.size() >= 9 && tolower(arg[8]) == 'h';
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "audio_delay="))
      {
         if (sscanf(arg.c_str(), "audio_delay=%d", (int*)&s_audioDelay) == 1)
            return true;
         return false;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "t="))
      {
         int w, h;
         if (sscanf(arg.c_str(), "t=%dx%d", &w, &h) == 2)
         {
            s_decodeWidth = w;
            s_decodeHeight = h;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "decode_buffers="))
      {
         if (sscanf(arg.c_str(), "decode_buffers=%d", (int*)&s_decodeBuffers) == 1)
         {
            if (s_decodeBuffers <= VideoTexturingApp::MaxDecodeBuffers)
               return true;
            else
            {
               s_decodeBuffers = VideoTexturingApp::MaxDecodeBuffers;
               return false;
            }
         }
      }
#ifndef EMULATED
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "texture_mode="))
      {
         if (sscanf(arg.c_str(), "texture_mode=%s", c) == 1)
         {
            if (!strcmp(c, "TEX_IMAGE_2D"))
               s_textureMode = GLTexture::eTEX_IMAGE_2D;
            else if (!strcmp(c, "EGL_IMAGE"))
               s_textureMode = GLTexture::eEGL_IMAGE_EXPLICIT;
            else
               return false;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "update_mode="))
      {
         if (sscanf(arg.c_str(), "update_mode=%s", c) == 1)
         {
            if (!strcmp(c, "EXT_SYNC"))
               s_updateMode = eEXT_SYNC;
            else if (!strcmp(c, "EXT"))
               s_updateMode = eEXT;
            else if (!strcmp(c, "ALWAYS_SYNC"))
               s_updateMode = eALWAYS_SYNC;
            else if (!strcmp(c, "ALWAYS"))
               s_updateMode = eALWAYS;
            else if (!strcmp(c, "EGL_SYNC"))
               s_updateMode = eEGL_SYNC;
            else
               return false;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "decode_format="))
      {
         if (sscanf(arg.c_str(), "decode_format=%s", c) == 1)
         {
            if (!strcmp(c, "YUV422"))
               s_decodeFormat = eYUV422;
            else if (!strcmp(c, "YUV444"))
               s_decodeFormat = eYUVX444;
            else if (!strcmp(c, "RGB565"))
               s_decodeFormat = eRGB565;
            else if (!strcmp(c, "RGBX8888"))
               s_decodeFormat = eRGBX8888;
            else if (!strcmp(c, "YV12"))
               s_decodeFormat = eYV12;
            else
               return false;
            return true;
         }
      }
#endif
      return false;
   }

   //! Return a string containing usage descriptions of the extra arguments you can handle.
   virtual std::string UsageString() const
   {
      return "video=<filename>                        set the video file to use\n"
             "t=WWWxHHH                               set decode texture size (WWW, HHH)\n"
             "decode_buffers=N                        number of decode buffers to use\n"
             "+bench                                  run in benchmark mode and gather benchmark numbers\n"
             "+old                                    run using old backdrop style\n"
             "quality=[high|low]                      use high/low quality rendering options\n"
             "audio_delay=N                           audio delay in ms\n"
#ifndef EMULATED
             "texture_mode=[TEX_IMAGE_2D|EGL_IMAGE]          which texture mode to use\n"
             "decode_format=[YUV422|YUV444|YV12|RGB565|RGBX8888]  what format to decode into\n"
             "update_mode=[EXT_SYNC|EXT|ALWAYS_SYNC|ALWAYS|EGL_SYNC]  EGLImage update mode (default EGL_SYNC)\n"
#endif
             ;
   }
};

/////////////////////////////////////////////////////////////////////

static Vec2 s_control1[] =
{
   Vec2(0.05f, -0.18f),
   Vec2(0.00f, -0.12f),
   Vec2(0.10f, -0.15f),
   Vec2(0.20f, -0.13f),
   Vec2(0.35f, -0.18f),
   Vec2(0.50f, -0.20f),
   Vec2(0.65f, -0.175f),
   Vec2(0.70f, -0.10f),
   Vec2(0.80f, -0.15f),
   Vec2(0.90f, -0.20f),
   Vec2(1.00f, -0.20f)
};

static Vec2 s_control2[] =
{
   Vec2(0.00f,  0.05f),
   Vec2(0.10f,  0.15f),
   Vec2(0.20f, -0.05f),
   Vec2(0.30f,  0.00f),
   Vec2(0.42f,  0.00f),
   Vec2(0.35f,  0.00f),
   Vec2(0.52f,  0.05f),
   Vec2(0.65f,  0.02f),
   Vec2(0.73f,  0.10f),
   Vec2(0.85f,  0.05f),
   Vec2(1.00f,  0.00f)
};

static Vec4 MkColor(uint32_t r, uint32_t g, uint32_t b)
{
   return Vec4(r / 255.0f, g / 255.0f, b / 255.0f, 0.7f);
}

//static float Rand(float low, float high)
//{
//   float    randf = (((float)rand() / RAND_MAX) * (high - low)) + low;
//
//   return randf;
//}

// Randomize
//
// Could add some noise to the curve, but at the moment doesn't do anything
//
static Vec2 *Randomize(uint32_t numControls, Vec2 *arr)
{
   //for (uint32_t i = 0; i < numControls; ++i)
   //{
   //   arr[i].X() = arr[i].X() + Rand(-0.05, 0.05);
   //   arr[i].Y() = arr[i].Y() + Rand(-0.05, 0.05);
   //}

   return arr;
}

BCMGuilloche::BCMGuilloche(Application &app) :
   m_app(app),
   m_numControls(sizeof(s_control1) / sizeof(Vec2)),
   m_guilloche(m_numControls, Randomize(m_numControls, s_control1), Randomize(m_numControls, s_control2), 100, 15),
   m_guillochePanel(m_guilloche, Vec2(-0.15f, 0.9f), 0.0f)
{
   Vec4  botColor = MkColor(  0, 85, 104);
   Vec4  topColor = MkColor(227, 24, 55 );

   m_guilloche.SetColorRamp(botColor, botColor, topColor, topColor);
   m_guilloche.SetScale(1.5f, 1.3f, 1.8f);
   m_guilloche.SetWidth(0.007f);

   Time now = m_app.FrameTimestamp();

   AnimBindingLerpFloat *anim1 = new AnimBindingLerpFloat(&m_guilloche.GetOffsetX());
   anim1->Interpolator()->Init(now, now + 60.0f, BaseInterpolator::eREPEAT);
   anim1->Evaluator()->Init(0.0f, 1.0f);
   m_animList.Append(anim1);
}

void BCMGuilloche::Render()
{
   m_app.RenderSceneGraph(m_guillochePanel.GetRoot());
}

void BCMGuilloche::UpdateAnimationList(const Time &now)
{
   m_animList.UpdateTime(now);
}

/////////////////////////////////////////////////////////////////////

const EffectHandle &EffectCache::GetEffect(uint32_t indx) const
{
   return m_cache[indx];
}

void EffectCache::Load(const std::vector<MenuItem> &items)
{
   std::vector<std::string>   defines;
#ifndef BSG_VC5
   if (s_decodeFormat != eYUV422 && s_decodeFormat != eYUVX444)
#endif
      defines.push_back("RGB_ALREADY");

   m_cache.clear();

   for (uint32_t i = 0; i < items.size(); ++i)
   {
      EffectHandle   effect(New);
      effect->Load(items[i].bfxFile, defines);

      m_cache.push_back(effect);
   }
}

/////////////////////////////////////////////////////////////////////

void FullScreenAnimDone::Notify(const Time &time)
{
   if (m_app->m_fullScreen)
   {
      m_app->ConfigureVideoGraphicsBlending(Platform::eUSE_CONSTANT_ALPHA, 0.0f);
      m_app->m_stopRendering = true;
   }
   else
      m_app->ChangeFilter(m_app->m_carousel->CurrentIndex());

   m_app->m_inTransition = false;
   Reset();
}

/////////////////////////////////////////////////////////////////////

VideoTexturingApp::VideoTexturingApp(Platform &platform) :
   Application(platform),
   m_root      (New),
   m_cameraNode(New),
   m_monitorCameraPosNode(New),
   m_monitorLookatNode(New),
   m_rootLookatNode(New),
   m_floatingCameraPosNode(New),
   m_floatingLookatNode(New),
   m_floatingCameraNode(New),
   m_vidNode   (New),
   m_xformNode (New),
   m_bgNode    (New),
   m_carouselRoot(New),
   m_floatingCamera(New),
   m_cameraPosConstraint(New),
   m_cameraLookatConstraint(New),
   m_carouselCamNode(New),
   m_carouselCamera(New),
   m_sequence  (&m_animList),
   m_font      (New),
   m_smallFont (New),
   m_midFont   (New),
   //m_videoEffect    -- deliberately "null",
   //m_rawVideoEffect -- deliberately "null",
   m_lastKeyPressTime(Time::Now()),
   m_idling(false),
   m_backdrop(64, 0.865f, 1.0f, 1, 57),
   m_guilloche(*this),
   m_helpMenu(0),
   m_fullScreen(false),
   m_fullScreenAnimDone(this),
   m_transitionCountdown(-1),
   m_stopRendering(false),
   m_inTransition(false)
{
   if (s_bench)
   {
      bool writeHeader = true;
      s_benchFp = fopen("benchmark.csv", "r");
      if (s_benchFp)
      {
         writeHeader = false;
         fclose(s_benchFp);
      }

      s_benchFp = fopen("benchmark.csv", "a");
      if (!s_benchFp)
         BSG_THROW("Could not open benchmark.csv for appending");

      if (writeHeader)
         fprintf(s_benchFp, "Display Resolution,Texture Mode,Format,Source Video,Destripe Size,FPS,CPU %%,Bandwidth MB/frame\n");

      fprintf(s_benchFp, "%dx%d,", GetOptions().GetWidth(), GetOptions().GetHeight());
   }

   m_font->SetFontInPercent("DroidSans-Bold.ttf", 6.0f, GetOptions().GetHeight());
   m_midFont->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, GetOptions().GetHeight());
   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.0f, GetOptions().GetHeight());

   if ((s_updateMode == eALWAYS || s_updateMode == eALWAYS_SYNC || s_updateMode == eEGL_SYNC) && s_textureMode == GLTexture::eEGL_IMAGE_EXPLICIT)
      s_textureMode = GLTexture::eEGL_IMAGE;

   if (s_textureMode == GLTexture::eEGL_IMAGE_EXPLICIT || s_textureMode == GLTexture::eEGL_IMAGE)
   {
      if (s_decodeFormat != eYUV422 && s_decodeFormat != eRGBX8888 && s_decodeFormat != eYUVX444 &&
          s_decodeFormat != eRGB565 && s_decodeFormat != eYV12)
         BSG_THROW("EGLImage only supports YUV422, YUVX444, YV12, RGB565 and RGBX8888.\n");
   }
   else
      s_decodeBuffers = 1;    // No need for multiple buffers in TEX_IMAGE_2D mode

   if (s_benchFp)
   {
      fprintf(s_benchFp, "%s,", s_textureMode != GLTexture::eTEX_IMAGE_2D ? "EGLImage" : "TexImage2D");

      switch (s_decodeFormat)
      {
      case eYUV422   : fprintf(s_benchFp, "eYUV422,"); break;
      case eYUVX444  : fprintf(s_benchFp, "eYUVX444,"); break;
      case eYV12     : fprintf(s_benchFp, "eYV12,"); break;
      case eRGB565   : fprintf(s_benchFp, "eRGB565,"); break;
      case eRGBX8888 : fprintf(s_benchFp, "eRGBX8888,"); break;
      default        :
      case eNONE     : BSG_THROW("Bad decode format"); break;
      }
   }

   // Add camera into its node and position it
   m_cameraNode->SetName("m_cameraNode");
   m_cameraNode->SetTransform(
      CameraTransformHelper::Lookat(Vec3(0.0f, 0.1f, 0.95f),   // Where
                                    Vec3(0.2f, 0.0f, 0.0f),    // Lookat
                                    Vec3(0.0f, 1.0f, 0.0f)));  // Up-vector

   m_rootLookatNode->SetPosition(Vec3(0.2f, 0.0f, 0.0f));
   m_root->AppendChild(m_rootLookatNode);

   // Setup our animatable camera
   m_floatingCameraPosNode->SetPosition(Vec3(0.0f, 0.1f, 0.95f));

   m_floatingCameraPosNode->SetName("m_floatingCameraPosNode");
   m_floatingCameraNode->SetName("m_floatingCameraNode");
   m_floatingLookatNode->SetName("m_floatingLookatNode");

   m_animFadeOpacity.Set(1.0f);

   ConstraintLookAtHandle        constraintLookAt(New);

   // Constrain "m_floatingCameraNode" to lie between m_cameraNode and m_monitorCameraPosNode.
   m_cameraPosConstraint->Install(m_cameraPosConstraint, m_floatingCameraPosNode, m_cameraNode, m_monitorCameraPosNode, 0.0f);

   // Constrain "m_floatingLookatNode" to lie between m_rootLookatNode and m_monitorLookatNode.
   m_cameraLookatConstraint->Install(m_cameraLookatConstraint, m_floatingLookatNode, m_rootLookatNode, m_monitorLookatNode, 0.0f);

   // Setup a lookat constraint that will control the camera animation
   constraintLookAt->Install(constraintLookAt, m_floatingCameraNode, m_floatingCameraPosNode, m_floatingLookatNode, Vec3(0.0f, 1.0f, 0.0f));

   m_floatingCamera->SetClippingPlanes(0.1f, 15.0f);
   m_floatingCamera->SetYFov(45.0f);
   m_floatingCameraNode->SetName("m_floatingCameraNode");
   m_floatingCameraNode->SetCamera(m_floatingCamera);

   // And the carousel camera
   m_carouselCamera->SetNearClippingPlane(0.05f);
   m_carouselCamera->SetFarClippingPlane(10.0f);
   m_carouselCamera->SetFocalPlane(0.1f);
   m_carouselCamera->SetAspectRatio((float)(GetOptions().GetWidth() * (1.0f - carouselPaneX)) / (float)GetOptions().GetHeight());

   m_carouselCamNode->SetName("m_carouselCamNode");
   m_carouselCamNode->SetCamera(m_carouselCamera);

   // Load the menu config
#ifndef BSG_VC5
   if (!LoadMenu(s_decodeFormat == eYUV422 ? "menu_config.txt" : "menu_config_rgb.txt"))
      throw("Failed to load menu");
#else
   if (!LoadMenu("menu_config_rgb.txt"))
      throw("Failed to load menu");
#endif

   m_effectCache.Load(m_menuItems);
   m_videoEffect    = m_effectCache.GetEffect(3);
   m_rawVideoEffect = m_effectCache.GetEffect(3);

   // Setup the video stream(s)
   if (s_videoFiles.size() == 0)
      s_videoFiles.push_back("video.mkv");

   uint32_t vidW = 0;
   uint32_t vidH = 0;

   if (s_decodeWidth > 0)
      vidW = s_decodeWidth;
   if (s_decodeHeight > 0)
      vidH = s_decodeHeight;

   // Start just showing graphics
   ConfigureVideoGraphicsBlending(Platform::eUSE_CONSTANT_ALPHA, 1.0f);

   // Build and configure the video decoder (Not started)
   if (s_videoFiles.size() == 1)
      m_videoDecoder = new VideoDecoder(s_videoFiles[0], s_decodeFormat, vidW, vidH, s_textureMode, s_updateMode, 0, true);
   else
      m_videoDecoder = new VideoDecoder(s_videoFiles, s_decodeFormat, vidW, vidH, s_textureMode, s_updateMode, 0, 0);

   m_videoDecoder->SetAudioDelay(s_audioDelay);

   if (m_videoDecoder->NumStreams() > 1)
   {
      // Mosaic mode
      s_titleX += 0.05f;
      s_filterX += 0.05f;
      s_titleY -= 0.5f;
      s_filterY -= 0.5f;
   }

   m_videoMat.push_back(MaterialHandle(New, "LCD_032Screen"));
   m_videoMat[0]->SetEffect(m_videoEffect);

   for (uint32_t v = 1; v < m_videoDecoder->NumStreams(); v++)
   {
      m_videoMat.push_back(MaterialHandle(New));
      m_videoMat[v]->SetEffect(m_rawVideoEffect);
   }

   vidW = m_videoDecoder->SourceWidth(0);
   vidH = m_videoDecoder->SourceHeight(0);

   if (s_benchFp)
      fprintf(s_benchFp, "%dx%d,", vidW, vidH);

   // This demo uses approximately 2/3 of the display at most for video, so scale appropriately
   vidW = std::min(vidW, 2 * GetOptions().GetWidth() / 3);
   vidH = std::min(vidH, 2 * GetOptions().GetHeight() / 3);
   // width must be loadable by the TLB
   vidW = (vidW + 7) & ~7;

   if (s_decodeWidth > 0)
      vidW = s_decodeWidth;
   if (s_decodeHeight > 0)
      vidH = s_decodeHeight;

   // Reconfigure the video decoder
   m_videoDecoder->OutputInitialisation(s_decodeFormat, vidW, vidH, s_textureMode, s_updateMode, s_decodeBuffers);

   // If running the benchmark
   // Start the video early for comparison to the original implementation
   if (s_benchFp)
      m_videoDecoder->StartPlayback();

   if (s_benchFp)
      fprintf(s_benchFp, "%dx%d,", m_videoDecoder->DestWidth(), m_videoDecoder->DestHeight());

   // Go through all the streams to set uniforms, textures and materials
   for (uint32_t videoIndex = 0; videoIndex < m_videoDecoder->NumStreams(); videoIndex++)
   {
      uint32_t videoDestWidth    = m_videoDecoder->DestWidth(videoIndex);
      uint32_t videoDestHeight   = m_videoDecoder->DestHeight(videoIndex);

      printf("Video %d decode size = %dx%d\n", videoIndex, videoDestWidth, videoDestHeight);

      m_videoMat[videoIndex]->SetUniformValue("u_texelSize", Vec2(1.0f / (float)videoDestWidth, 1.0f / (float)videoDestHeight));

      // Go through all the buffer to initialise them as TextImage2D
      for (uint32_t bufferIndex = 0; bufferIndex < m_videoDecoder->NumBuffersPerStream(); ++bufferIndex)
      {
         // Make the pixmap into a texture & set on the material
         m_videoDecoder->GetCurrentTexture(videoIndex)->TexImage2D(m_videoDecoder->GetCurrentPixmap(videoIndex), s_textureMode);

         // Change which internal buffer is currently used
         m_videoDecoder->ChangeCurrentBuffer(videoIndex);
      }

      // Set the current texture as a material
      m_videoMat[videoIndex]->SetTexture("u_tex", m_videoDecoder->GetCurrentTexture(videoIndex));
   }

   MaterialHandle plainMat(New);
   EffectHandle   plainEffect(New);
   plainEffect->Load("flat_color.bfx");
   plainMat->SetEffect(plainEffect);

   EffectHandle   litEffect(New);
   litEffect->Load("lit.bfx");

   EffectHandle   floorEffect(New);
   floorEffect->Load(s_oldBackdrop ? "floor.bfx" : "floor_new.bfx");

   MaterialHandle shellMat(New, "Monitor_032Shell");
   shellMat->SetEffect(litEffect);
   shellMat->SetUniformValue("u_color", Vec4(0.1f, 0.1f, 0.1f, 1.0f));

   MaterialHandle buttonMat(New, "Monitor_032Buttons");
   buttonMat->SetEffect(litEffect);
   buttonMat->SetUniformValue("u_color", Vec4(0.6f, 0.6f, 0.6f, 1.0f));

   TextureHandle floorTex(New);
   floorTex->TexImage2D(ImageSet("bake", "pkm", Image::eETC1));

   MaterialHandle floorMat(New, "Ground");
   floorMat->SetEffect(floorEffect);
   floorMat->SetTexture("u_tex", floorTex);

   EffectHandle   mugEffect(New);
   mugEffect->Load("mug.bfx");

   TextureHandle mugTex(New);

   if (s_highQuality)
   {
      mugTex->SetAutoMipmap(true);
      mugTex->TexImage2D(Image("bcm_logo0", "png", Image::eRGB565));
   }
   else
   {
      mugTex->TexImage2D(ImageSet("bcm_logo", "pkm", Image::eETC1));
   }

   MaterialHandle mugMat(New, "Mug");
   mugMat->SetEffect(mugEffect);
   mugMat->SetTexture("u_tex", mugTex);

   GeometryHandle monitorGeom = ObjFactory("MonitorMug_16x9.obj").MakeGeometry();

   m_vidNode->SetName("m_vidNode");
   m_vidNode->AppendGeometry(monitorGeom);
   m_vidNode->GetTransform().SetPosition(Vec3(s_monitorX, -0.3f, 0.0f));

   m_xformNode->SetName("m_xformNode");
   m_xformNode->AppendChild(m_vidNode);
   m_xformNode->GetTransform().SetRotation(s_monitorRotMin, Vec3(0.0f, 1.0f, 0.0f));

   m_xformNode->AppendChild(m_monitorCameraPosNode);
   m_xformNode->AppendChild(m_monitorLookatNode);

   // Magic transform for a camera position that looks directly at screen down its normal
   m_monitorCameraPosNode->SetName("m_monitorCameraPosNode");
   m_monitorCameraPosNode->GetTransform().SetPosition(Vec3(s_monitorX, 0.136f, 0.5025f));
   m_monitorCameraPosNode->GetTransform().SetRotation(-11.0f, Vec3(1.0f, 0.0f, 0.0f));

   // Center of monitor screen
   m_monitorLookatNode->SetName("m_monitorLookatNode");
   m_monitorLookatNode->GetTransform().SetPosition(Vec3(s_monitorX, 0.037f, 0.0f));

   // Load effects for the background
   MaterialHandle cylMat(New);
   EffectHandle   cylEffect(New);
   cylEffect->Load("bg.bfx");
   cylMat->SetEffect(cylEffect);
   cylMat->SetUniformValue("u_leftColor", Vec3(0.914f, 0.0f, 0.0314f));
   cylMat->SetUniformValue("u_rightColor", Vec3(0.4f, 0.1f, 0.11f));

   // Small monitor positions
   Vec3 monPos[7];
   monPos[0] = Vec3( 0.2f, -0.3f, -1.8f);
   monPos[1] = Vec3(-0.5f, -0.3f, -1.8f);
   monPos[2] = Vec3( 0.9f, -0.3f, -1.8f);
   monPos[3] = Vec3( 0.2f,  0.2f, -3.0f);
   monPos[4] = Vec3( 0.9f,  0.2f, -3.0f);
   monPos[5] = Vec3(-0.5f,  0.2f, -3.0f);
   monPos[6] = Vec3( 1.6f,  0.2f, -3.0f);

   for (uint32_t v = 1; v < m_videoDecoder->NumStreams(); v++)
   {
      GeometryHandle geom = ObjFactory("Monitor.obj").MakeGeometry();

      for (uint32_t s = 0; s < geom->NumSurfaces(); s++)
      {
         if (geom->GetMaterial(s) == m_videoMat[0])
            geom->SetMaterial(s, m_videoMat[v]);
      }

      SceneNodeHandle xlat(New);
      SceneNodeHandle rot(New);
      if (v < 4)
      {
         xlat->GetTransform().SetPosition(monPos[(v - 1) % 7]);
         rot->GetTransform().SetRotation(-40.0f, Vec3(0, 1, 0));
      }
      else if (v >= 4)
      {
         xlat->GetTransform().SetPosition(monPos[(v - 1) % 7]);
         rot->GetTransform().SetRotation(-30.0f, Vec3(0, 1, 0));
      }

      rot->AppendChild(xlat);
      xlat->AppendGeometry(geom);
      m_bgNode->AppendChild(rot);
   }

   // Create animators
   Time now = FrameTimestamp();

   Transform next;
   uint32_t label = m_sequence.AppendLabel();
   m_curTransform = m_xformNode->GetTransform();
   next.SetRotation(s_monitorRotMax, Vec3(0, 1, 0));
   m_sequence.AppendAnimation(GenerateNextAnim(next, m_xformNode), 10.0f);
   next.SetRotation(s_monitorRotMin, Vec3(0, 1, 0));
   m_sequence.AppendAnimation(GenerateNextAnim(next, m_xformNode), 10.0f);
   m_sequence.AppendGoto(label);

   m_sequence.Start(now);

   AnimBindingLerpFloat *timerAnim = new AnimBindingLerpFloat;
   timerAnim->Init(0.0f, now, 1.0f, now + Time(10.0f), BaseInterpolator::eREPEAT);
   timerAnim->Bind(&m_animFloat);
   m_animList.Append(timerAnim);

   // Build the carousel
   m_carousel = new MenuCarousel(7, m_midFont, Vec2(s_filterX, s_filterY));

   for (uint32_t i = 0; i < m_menuItems.size(); i++)
      m_carousel->AddEntry(m_menuItems[i].text);

   m_carouselRoot->AppendChild(m_carousel->RootNode());

   // Ensure each graph has a camera
   m_carouselRoot->AppendChild(m_carouselCamNode);

   // Construct the scene graph (root with a camera and cube child)
   m_root->SetName("m_root");
   m_root->AppendChild(m_cameraNode);
   m_root->AppendChild(m_xformNode);
   m_root->AppendChild(m_bgNode);

   if (s_oldBackdrop)
   {
      m_root->AppendChild(m_backdrop.RootNode());

      // Configure the backdrop
      Vec3 grey(0.4f, 0.4f, 0.4f);
      Vec3 white(1.0f, 1.0f, 1.0f);
      m_backdrop.AddCenterPanel(0.45f, 0.861f, BCMBackdrop::eBCM_TR, BCMBackdrop::eBCM_TR);
      m_backdrop.AddShadedPanel(0.861f, 0.865f, grey, white, grey, white);
   }

   if (!s_bench)
   {
      // Making the help menu if we are not trying keep the old style
      m_helpMenu = new HelpMenu(this, eHELP_BUTTON_RED, "Help", "DroidSans.ttf", Vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.025f, 0.93f, 0.03f, true);
      m_helpMenu->SetMenuItemHeaderColour(Vec4(1.0f, 0.5f, 0.0f, 1.0f));
      m_helpMenu->SetMenuItemTextColour(Vec4(1.0f, 1.0f, 1.0f, 1.0f));

      m_helpMenu->AddMenuItem("Up", "Previous filter");
      m_helpMenu->AddMenuItem("Down", "Next filter");
      m_helpMenu->AddMenuItem("Select", "Full screen video");
      m_helpMenu->AddMenuItem(eHELP_BUTTON_RED, "Help menu");
      m_helpMenu->AddMenuItem("Clear", "Exit");

      m_helpMenu->SetMenuPosition(eMENU_POSITION_BOT_RIGHT, Vec2(0.0f));

      unsigned int animationType = eMENU_ANIM_FADE | eMENU_ANIM_SCALE | eMENU_ANIM_MOVE_FROM_BOT_RIGHT;
      m_helpMenu->SetAnimationType(animationType);

      m_helpMenu->FinaliseMenuLayout();
   }

   // Init any global GL state
   glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

   ChangeFilter(3);
}

AnimBindingBase *VideoTexturingApp::GenerateNextAnim(Transform &next, SceneNodeHandle node)
{
   AnimBindingHermiteTransform *anim = new AnimBindingHermiteTransform;
   anim->Evaluator()->Init(m_curTransform, next);
   anim->Bind(&node->GetTransform());
   m_curTransform = next;
   return anim;
}

VideoTexturingApp::~VideoTexturingApp()
{
   if (s_benchFp)
      fclose(s_benchFp);

   delete m_videoDecoder;

   delete m_helpMenu;
}

void VideoTexturingApp::DrawTime(const Vec2 &pos)
{
   std::string timeStr = Time::Now().CalendarTimeString("%d %B %Y - %X");

   DrawTextString(timeStr, pos[0], pos[1], m_smallFont, Vec4(1, 1, 1, 0.7f));
}

void VideoTexturingApp::StartNonFullScreenTransition()
{
   Time now = FrameTimestamp();
   float animSecs = 2.0f;

   ConfigureVideoGraphicsBlending(Platform::eUSE_CONSTANT_ALPHA, 1.0f);

   AnimBindingHermiteFloat *anim = new AnimBindingHermiteFloat(&m_cameraPosConstraint->GetAlpha());
   anim->Init(1.0f, now, 0.0f, now + animSecs);
   m_animList.Append(anim);

   AnimBindingHermiteFloat *anim2 = new AnimBindingHermiteFloat(&m_cameraLookatConstraint->GetAlpha());
   anim2->Init(1.0f, now, 0.0f, now + animSecs);
   m_animList.Append(anim2);

   m_carousel->FadeIn(now, Time(animSecs, Time::eSECONDS));

   AnimBindingLerpFloat *anim3 = new AnimBindingLerpFloat;
   anim3->Init(0.0f, now, 1.0f, now + animSecs);
   anim3->Bind(&m_animFadeOpacity);
   m_animList.Append(anim3);

   AnimBindingLerpFloat *anim4 = new AnimBindingLerpFloat;
   anim4->Init(0.0f, now, 1.0f, now + animSecs, BaseInterpolator::eLIMIT, &m_fullScreenAnimDone);
   anim4->Bind(&m_helpMenu->GetRootNode()->GetOpacity());
   m_animList.Append(anim4);
}

void VideoTexturingApp::ToggleFullScreenVideo()
{
   Time now = FrameTimestamp();
   float animSecs = 2.0f;

   m_fullScreen = !m_fullScreen;

   m_inTransition = true;

   if (m_fullScreen)
   {
      ChangeFilter(3);

      AnimBindingHermiteFloat *anim = new AnimBindingHermiteFloat(&m_cameraPosConstraint->GetAlpha());
      anim->Init(0.0f, now, 1.0f, now + animSecs);
      m_animList.Append(anim);

      AnimBindingHermiteFloat *anim2 = new AnimBindingHermiteFloat(&m_cameraLookatConstraint->GetAlpha());
      anim2->Init(0.0f, now, 1.0f, now + animSecs);
      m_animList.Append(anim2);

      m_carousel->FadeOut(now, Time(animSecs, Time::eSECONDS));

      AnimBindingLerpFloat *anim3 = new AnimBindingLerpFloat;
      anim3->Init(1.0f, now, 0.0f, now + animSecs);
      anim3->Bind(&m_animFadeOpacity);
      m_animList.Append(anim3);

      AnimBindingLerpFloat *anim4 = new AnimBindingLerpFloat;
      anim4->Init(1.0f, now, 0.0f, now + animSecs, BaseInterpolator::eLIMIT, &m_fullScreenAnimDone);
      anim4->Bind(&m_helpMenu->GetRootNode()->GetOpacity());
      m_animList.Append(anim4);
   }
   else
   {
      // We need to render 3 frames to fill the 3D pipeline before starting the return animation
      m_transitionCountdown = 3;
      m_stopRendering = false;
   }
}

bool VideoTexturingApp::UpdateFrame(int32_t *idleMs)
{
   Time now = FrameTimestamp();

   if (!m_stopRendering)
   {
      // If there is more than one stream
      if (m_videoDecoder->NumStreams() > 1)
      {
         std::vector<VideoDecoder::eFrameStatus> results;

         // Update the texture if a frame is available from the decoder
         m_videoDecoder->UpdateMosaicFrames(results);

         // Go through all the streams to associate a new texture if a new frame is available
         for (uint32_t videoIndex = 0; videoIndex < m_videoDecoder->NumStreams(); videoIndex++)
         {
            if (results[videoIndex] == VideoDecoder::eFRAME_NEW)
            {
               if (s_textureMode == GLTexture::eTEX_IMAGE_2D)
                  m_videoDecoder->GetCurrentTexture(videoIndex)->TexImage2D(m_videoDecoder->GetCurrentPixmap(videoIndex), s_textureMode);

               m_videoMat[videoIndex]->SetTexture("u_tex", m_videoDecoder->GetCurrentTexture(videoIndex));
            }
            else if (videoIndex == 0 && m_transitionCountdown > 0)
               m_transitionCountdown++; // We might need to wait a little longer as we don't have a new video frame yet
         }
      }
      else
      {
         // Only one video stream
         const uint32_t   videoIndex = 0;

         // If new frame available
         if (m_videoDecoder->UpdateFrame(VideoDecoder::eNO_BLOCK) == VideoDecoder::eFRAME_NEW)
         {
            if (s_textureMode == GLTexture::eTEX_IMAGE_2D)
               m_videoDecoder->GetCurrentTexture(videoIndex)->TexImage2D(m_videoDecoder->GetCurrentPixmap(videoIndex), s_textureMode);

            m_videoMat[videoIndex]->SetTexture("u_tex", m_videoDecoder->GetCurrentTexture(videoIndex));
         }
         else if (m_transitionCountdown > 0)
            m_transitionCountdown++;   // We might need to wait a little longer as we don't have a new video frame yet
      }
   }

   m_animList.UpdateTime(now);
   m_carousel->UpdateAnimationList(now);

   if (!m_stopRendering)
   {
      for (uint32_t v = 0; v < m_videoDecoder->NumStreams(); v++)
         m_videoMat[v]->SetUniformValue("u_timer", m_animFloat);
   }

   if (s_oldBackdrop)
      m_backdrop.UpdateAnimationList(now);
   else
      m_guilloche.UpdateAnimationList(now);

   if (m_helpMenu)
         m_helpMenu->UpdateTime();

   if (!m_stopRendering)
   {
      // Auto animate if no recent key presses
      if (m_idling)
      {
         if (!s_bench && (now - m_lastKeyPressTime).Seconds() > autoChangeTime)
         {
            m_carousel->Prev(now, Time(0.3f));
            ChangeFilter(m_carousel->CurrentIndex());
            m_lastKeyPressTime = now;
         }
      }
      else
      {
         if (!s_bench && (now - m_lastKeyPressTime).Seconds() > idleTimeToAuto)
         {
            m_idling = true;
            m_carousel->Prev(now, Time(0.3f));
            ChangeFilter(m_carousel->CurrentIndex());
            m_lastKeyPressTime = now;
         }
      }
   }

   if (m_stopRendering)
   {
      *idleMs = 32;
      return false;
   }

   return true;
}

void VideoTexturingApp::RenderFrame()
{
   // If it is not a benchmark start the video at the end of the initialisation
   if (!s_benchFp)
      if (s_countFramesBeforePlayback <= s_nbFramesDelay)
      {
         s_countFramesBeforePlayback++;
         if (s_countFramesBeforePlayback >= s_nbFramesDelay)
         m_videoDecoder->StartPlayback();
      }

   m_transitionCountdown--;
   if (m_transitionCountdown == 0)
      StartNonFullScreenTransition();

   // Clear all the buffers (this is the most efficient thing to do)
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

   if (!s_oldBackdrop)
      m_guilloche.Render();

   // Draw the main scene
   RenderSceneGraph(m_root, m_floatingCamera);

   if (s_updateMode == eEGL_SYNC)
      m_videoDecoder->CreateFenceForSyncUpdate(eglGetDisplay(EGL_DEFAULT_DISPLAY));

   // Don't draw the carousel, we just want the text really
   //   glViewport((GLint)(GetOptions().GetWidth() * carouselPaneX), 0,
   //              (GLsizei)(GetOptions().GetWidth() * (1.0f - carouselPaneX)), GetOptions().GetHeight());
   //   RenderSceneGraph(m_carouselRoot);

   // Render text layers
   m_carousel->RenderText();

   DrawTextString("Video Texturing", 0.65f, 0.985f, m_font, Vec4(1, 1, 1, m_animFadeOpacity.Get()));
   DrawTextString("Filter:", s_titleX, s_titleY, m_midFont, Vec4(1, 1, 1, m_animFadeOpacity.Get()));
   //DrawTime(Vec2(0.05f, 0.97f));

   if (m_helpMenu)
      RenderSceneGraph(m_helpMenu->GetRootNode());

   DoBenchmarking();
}

// Benchmarking code
void VideoTexturingApp::DoBenchmarking()
{
   if (s_bench)
   {
      if (s_benchFrameCnt == 0)
      {
         s_benchStartTime = FrameTimestamp();
         EstimateCPUPercentage();   // Restart the estimation from now
      }

      s_benchFrameCnt++;
      float seconds = (FrameTimestamp() - s_benchStartTime).FloatSeconds();
      if (seconds > 15.0f)
      {
         // Done : print benchmark data & exit
         float fps = (float)s_benchFrameCnt / seconds;
         float cpu = EstimateCPUPercentage();

         printf("Average FPS = %f\n", fps);
         printf("Average CPU%% = %f\n", cpu);

         if (s_benchFp)
         {
            fprintf(s_benchFp, "%f,", fps);
            fprintf(s_benchFp, "%f,", cpu);
         }

         // Reset the counters
         s_benchFrameCnt = 0;
         s_benchStartTime = FrameTimestamp();
         EstimateCPUPercentage();   // Restart the estimation from now

         Stop(255);
      }
   }
}

void VideoTexturingApp::ChangeFilter(uint32_t indx)
{
   if (indx < m_menuItems.size())
   {
      m_videoEffect = m_effectCache.GetEffect(indx);

      m_videoMat[0]->SetEffect(m_videoEffect);
      m_videoMat[0]->SetUniformValue("u_texelSize", Vec2(1.0f / (float)m_videoDecoder->DestWidth(0), 1.0f / (float)m_videoDecoder->DestHeight(0)));
      m_videoMat[0]->SetTexture("u_tex", m_videoDecoder->GetCurrentTexture(0));
   }
}

void VideoTexturingApp::KeyEventHandler(KeyEvents &queue)
{
   m_lastKeyPressTime = Time::Now();
   m_idling = false;

   // Service one pending key event
   while (queue.Pending())
   {
      KeyEvent ev = queue.Pop();

      if (ev.State() == KeyEvent::eKEY_STATE_DOWN)
      {
         switch (ev.Code())
         {
         case KeyEvent::eKEY_EXIT :
         case KeyEvent::eKEY_ESC :
         case KeyEvent::eKEY_POWER :
            Stop(255);
            break;
         case KeyEvent::eKEY_UP :
            if (!m_fullScreen)
            {
               m_carousel->Prev(FrameTimestamp(), Time(0.3f));
               ChangeFilter(m_carousel->CurrentIndex());
            }
            break;
         case KeyEvent::eKEY_DOWN :
            if (!m_fullScreen)
            {
               m_carousel->Next(FrameTimestamp(), Time(0.3f));
               ChangeFilter(m_carousel->CurrentIndex());
            }
            break;
         case KeyEvent::eKEY_RED :
         case KeyEvent::eKEY_F1 :
            if (m_helpMenu)
               m_helpMenu->ToggleMenu();
            break;
         case KeyEvent::eKEY_OK :
            if (!m_inTransition)
               ToggleFullScreenVideo();
            break;
         default :
            break;
         }
      }
   }
}

void VideoTexturingApp::MouseEventHandler(MouseEvents &queue)
{
   // Service pending mouse events
   while (queue.Pending())
   {
      MouseEvent ev = queue.Pop();

      if (ev.Type() == MouseEvent::eMOUSE_EVENT_WHEEL)
      {
         IVec2 vec = ev.RelativeWheelVector();

         if (vec.Y() > 0)
         {
            m_carousel->Prev(FrameTimestamp(), Time(0.3f));
            ChangeFilter(m_carousel->CurrentIndex());
         }
         else if (vec.Y() < 0)
         {
            m_carousel->Next(FrameTimestamp(), Time(0.3f));
            ChangeFilter(m_carousel->CurrentIndex());
         }
      }
   }
}

void VideoTexturingApp::ResizeHandler(uint32_t width, uint32_t height)
{
   m_floatingCamera->SetAspectRatio(width, height);

   m_font->SetFontInPercent("DroidSans-Bold.ttf", 6.0f, height);
   m_midFont->SetFontInPercent("DroidSans-Bold.ttf", 4.0f, height);
   m_smallFont->SetFontInPercent("DroidSans-Bold.ttf", 2.0f, height);

   if (s_oldBackdrop)
      m_backdrop.ResizeHandler(width, height);

   if (m_helpMenu)
      m_helpMenu->Resize();
}

bool VideoTexturingApp::LoadMenu(const std::string &file)
{
   std::string	fileName = FindResource(file);

   FILE *fp = fopen(fileName.c_str(), "r");
   if (fp == NULL)
   {
      printf("Could not open %s\n", file.c_str());
      return false;
   }

   m_menuItems.clear();

   char buf[4096];

   while (fgets(buf, 4096, fp))
   {
      MenuItem item;
      char     *s = buf;
      char     *p;

      p = strstr(s, ",");
      *p = '\0';
      item.text = s;
      s = p + 1;

      item.bfxFile = ParseUtils::StripWhite(s);

      m_menuItems.push_back(item);
   }

   fclose(fp);

   return true;
}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      // Create the default application options object
      ApplicationOptions   options;

      // Request a specific display size
      options.SetDisplayDimensions(1920, 1080);

      // Read any command-line options (possibly overriding the display size)
      CustomArgumentParser customParser;
      if (!options.ParseCommandLine(argc, argv, &customParser))
         return 1;

      if (s_bench)
         options.SetSwapInterval(0);

      // Initialise the platform
      Platform  platform(options);

      s_platform = &platform;

      // Initialise the application
      VideoTexturingApp  app(platform);

      // Run the application
      ret = platform.Exec();
   }
   catch (const Exception &e)
   {
      // BSG will throw exceptions of type bsg::Exception if anything goes wrong
      std::cerr << "Exception : " << e.Message() << "\n";
   }

   return ret;
}
