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

#include "bsg_application.h"
#include "bsg_exception.h"
#include "bsg_context.h"
#include "bsg_geometry.h"
#include "bsg_time.h"
#include "bsg_hud.h"
#include "bsg_library.h"
#include "bsg_image_png.h"
#include "bsg_parse_utils.h"

#include <iostream>
#include <fstream>

namespace bsg
{

Platform *Platform::m_instance = NULL;

Platform::Platform(const ApplicationOptions &options) :
   m_options(options.CalculateDerived()),
   m_exitCode(0xF0F0F0F0),
   m_platform(0),
   m_mouseMoveMerging(true),
   m_stereoEye(eMONO),
   m_showFpsHUD(options.GetShowFpsHUD()),
   m_showedHudLastFrame(false),
   m_frameGrabPending(false),
   m_swapIntervalPending(false),
   m_lastFPSFrame(0),
   m_lastFPS(0),
   m_frame(0),
   m_statsFile(NULL),
   m_app(0),
   m_keyEvenHandler(0),
   m_keyEvenHandler2(0),
   m_mouseButtonHandler(0),
   m_mouseMoveHandler(0)
{
   m_rateMultiplier.Set(options.GetRate());

   m_swapInterval   = options.GetSwapInterval();
   m_stereo         = options.GetStereo();
   m_showHUD        = options.GetShowHUD();
   m_windowWidth    = options.GetWidth();
   m_windowHeight   = options.GetHeight();

   InitializePlatform();

   m_instance = this;

   if (options.GetNoAutoContext())
      InitializeDisplayNoContext();
   else
      InitializeDisplay();

#if EGL_BRCM_driver_monitor
   m_eglInitDriverMonitorBRCM =   (PFNEGLINITDRIVERMONITORBRCMPROC  )eglGetProcAddress("eglInitDriverMonitorBRCM");
   m_eglGetDriverMonitorXMLBRCM = (PFNEGLGETDRIVERMONITORXMLBRCMPROC)eglGetProcAddress("eglGetDriverMonitorXMLBRCM");
   m_eglTermDriverMonitorBRCM =   (PFNEGLTERMDRIVERMONITORBRCMPROC  )eglGetProcAddress("eglTermDriverMonitorBRCM");
#endif

   if (options.GetShowRenderer())
   {
      // Print the renderer string
      const GLubyte *renderer = glGetString(GL_RENDERER);
      printf("GL renderer: %s\n", renderer);
   }
}

Platform::~Platform()
{
   TerminateDisplay();
   TerminatePlatform();

   if (m_statsFile != NULL)
      fclose(m_statsFile);

   m_instance = NULL;
}

const ApplicationOptions &Platform::GetOptions() const
{
   return m_options;
}

uint32_t Platform::Exec()
{
   if (m_app == 0)
      BSG_THROW("Application not installed in platform");

   // Establish initial window size
   Resize(GetOptions().GetWidth(), GetOptions().GetHeight());

   m_exitCode = 0xF0F0F0F0;

   try
   {
      m_frameTimestamp = Time::Now();
      m_animTimestamp = m_frameTimestamp * m_rateMultiplier;
      m_lastFPSTime = m_frameTimestamp;

      // Main loop
      RunPlatformMainLoop();
   }
   catch (Exception e)
   {
      std::cerr << "Exception in Exec() : " <<  e.Message();
      return 0;
   }

   return m_exitCode;
}

void Platform::Stop(uint32_t exitCode)
{
   m_exitCode = exitCode;
}

void Platform::InitializeDisplay()
{
   InitializePlatformDisplay();
   SetStereoscopic(m_options.GetStereo());

   if (m_options.GetFinalAlpha() != -1.0f || m_options.GetZOrder() != ~0u)
   {
      ConfigureVideoGraphicsBlending(m_options.GetFinalAlpha() == -1.0f ? eUSE_CONSTANT_ALPHA : eUSE_GRAPHICS_ALPHA,
                                     1.0f,
                                     m_options.GetZOrder());
   }

   if (m_options.GetRenderPixmap())
   {
      uint32_t w = m_options.GetWidth();
      uint32_t h = m_options.GetHeight();
      NativePixmap::ePixmapFormat format = m_options.GetColorBits() == 16 ? NativePixmap::eRGB565_TEXTURE : NativePixmap::eABGR8888_TEXTURE;
      NativePixmap *pixmap = new NativePixmap(w, h, format);

      if (!pixmap)
         BSG_THROW("Failed to create a Pixmap.");

      assert(m_nativePixmaps.size() == 0);
      m_nativePixmaps.push_back(pixmap);

      m_platform->InitializeContext(m_context, m_options, pixmap);
   }
   else
   {
      m_platform->InitializeContext(m_context, m_options, m_nativeWindows.front());
   }
}

void Platform::InitializeDisplayNoContext()
{
   InitializePlatformDisplay();
}

void Platform::TerminateDisplay()
{
   m_context.Terminate();

   std::list<NativePixmap*>::iterator iter;
   for (iter = m_nativePixmaps.begin(); iter != m_nativePixmaps.end(); ++iter)
      delete(*iter);

   TerminatePlatformDisplay();
}

void Platform::PushKeyEvent(const KeyEvent &event)
{
   if (!m_showHUD &&
        ((event.Code() == KeyEvent::eKEY_FAV4) ||
        (event.Code() == KeyEvent::KEY_5 && event.State() == KeyEvent::eKEY_STATE_DOWN && m_prevKey.Code() == KeyEvent::KEY_5 &&
        (event.GetTimestamp() - m_prevKey.GetTimestamp()).Seconds() < 1.0f)))
   {
      m_showHUD = true;
   }
   else if (m_showHUD && m_app != 0)
      m_app->HandleKeyHud(event);  // Pass events to hud while open
   else
      m_keyEvents.Push(event);

   m_prevKey = event;
}

void Platform::PushMouseEvent(const MouseEvent &event)
{
   m_mouseEvents.Push(event);
}

void Platform::ProcessKeyEvents()
{
   if (m_keyEvents.Pending())
       m_app->KeyEventHandler(m_keyEvents);

   if (m_mouseEvents.Pending())
       m_app->MouseEventHandler(m_mouseEvents);
}

float Platform::GetFPS()
{
   float elapsed = (m_frameTimestamp - m_lastFPSTime).FloatSeconds();
   if (elapsed > 0.5f)
   {
      m_lastFPS = (float)(m_frame - m_lastFPSFrame) / elapsed;
      m_lastFPSFrame = m_frame;
      m_lastFPSTime  = m_frameTimestamp;
   }

   return m_lastFPS;
}

void Platform::ShowFPS()
{
   if (m_options.GetShowFPS())
   {
      Time now = Time::Now();
      float elapsed = (now - m_lastFPSTime).FloatSeconds();

      if (elapsed > 1.0f)
      {
         float fps = (float)(m_frame - m_lastFPSFrame) / elapsed;
         std::cout << "Frames per second: " << fps << std::endl;

         m_lastFPSFrame = m_frame;
         m_lastFPSTime  = now;
      }
   }
}

void Platform::RenderFrameSequenceMono()
{
}

void Platform::RenderFrameSequenceStereo()
{
}

void Platform::PreFrame()
{
   const Vec4 &clearColour = m_options.GetBGColour();

   if (clearColour.X() != -1.0f)
      glClearColor(clearColour.X(), clearColour.Y(), clearColour.Z(), clearColour.W());
}

void Platform::PostFrame()
{
   float finalAlpha = m_options.GetFinalAlpha();

   if (finalAlpha != -1.0f)
   {
      GLboolean oldMask[4];
      GLfloat   oldColor[4];

      glGetBooleanv(GL_COLOR_WRITEMASK, oldMask);
      glGetFloatv(GL_COLOR_CLEAR_VALUE, oldColor);

      glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
      glClearColor(0.0f, 0.0f, 0.0f, finalAlpha);
      glClear(GL_COLOR_BUFFER_BIT);

      glColorMask(oldMask[0], oldMask[1], oldMask[2], oldMask[3]);
      glClearColor(oldColor[0], oldColor[1], oldColor[2], oldColor[3]);
   }
}

void Platform::RenderFrameSequence()
{
   int32_t msToNextAnim = 0;

   // Update the frame
   bool changed = m_app->UpdateFrame(&msToNextAnim);

   // There are a couple of cases where we want to draw even if the app says nothing changed
   if (m_frame == 0)
      changed = true;   // We always want to draw the first frame!

   if (m_showedHudLastFrame != m_showHUD)
      changed = true;   // HUD has just changed state, ensure we draw the frame

   m_showedHudLastFrame = m_showHUD;

   if (m_frameGrabPending)
      changed = true;

   if (changed || m_showHUD)
   {
      PreFrame();

      // Call the client application's render frame
      if (m_stereo)
      {
         SetupEye(eLEFT);
         m_app->RenderFrame();
         PostFrame();

         SetupEye(eRIGHT);
         m_app->RenderFrame();
         PostFrame();
      }
      else
      {
         SetupEye(eMONO);
         m_app->RenderFrame();
         PostFrame();
      }
   }

   if (m_showHUD && !m_frameGrabPending)
   {
      if (m_stereo)
      {
         SetupEye(eLEFT);
         m_app->DrawHud();

         SetupEye(eRIGHT);
         m_app->DrawHud();
      }
      else
      {
         SetupEye(eMONO);
         m_app->DrawHud();
      }
   }

   if (m_showFpsHUD && !m_frameGrabPending)
   {
      if (m_stereo)
      {
         SetupEye(eLEFT);
         m_app->DrawFpsHud();

         SetupEye(eRIGHT);
         m_app->DrawFpsHud();
      }
      else
      {
         SetupEye(eMONO);
         m_app->DrawFpsHud();
      }
   }

   if (m_frameGrabPending)
   {
      DoFrameGrab();
      m_frameGrabPending = false;
   }

   if (changed || m_showHUD)
   {
      // If the swap interval has been changed then set it now
      // Can't be set by callback routine since this may run
      // in a different thread
      if (m_swapIntervalPending)
      {
         m_swapIntervalPending = false;
         m_context.SetSwapInterval(m_swapInterval);
      }

      // Swap now
      if (m_options.AutoSwapBuffer())
      {
         m_context.SwapBuffers();
      }
   }

   m_frame++;

   ShowFPS();
   DoMonitorStats();

   if (!changed)
   {
      if (msToNextAnim > 20)
         GoIdle(msToNextAnim - 20);
      else if (msToNextAnim < 0)
         GoIdle(0x0FFFFFFF);
   }

   uint32_t first;
   uint32_t last;

   m_app->GetOptions().GetLastFirstFrames(&last, &first);

   if (m_frame == last)
      BSG_THROW("Terminated normally");
}

#ifndef BSG_STAND_ALONE

void Platform::DoFrameGrab()
{
   uint32_t width  = m_app->GetWindowWidth();
   uint32_t height = m_app->GetWindowHeight();

   ImagePNG pngImage(width, height, Image::eRGBA8888);
   glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pngImage.GetData());

   char file[1024];
   int  cnt = 0;
   do
   {
      sprintf(file, "framegrab%d.png", cnt);
      cnt++;
   }
   while (ParseUtils::IsFile(file));

   pngImage.Save(file);
}

#else

// Stubbed version.
void Platform::DoFrameGrab()
{
}

#endif

void Platform::DumpMonitorStats()
{
#if EGL_BRCM_driver_monitor
   if (m_eglGetDriverMonitorXMLBRCM != NULL)
   {
      char *xml = (char *)malloc(4096 * sizeof(char));
      char *s, *e;
      char tag[256];
      char val[128];
      uint64_t ret;

      if (xml != NULL)
      {
         m_eglGetDriverMonitorXMLBRCM(GetContext().GetDisplay(), 4096, NULL, xml);

         s = strstr(xml, "<");
         while (s)
         {
            s++;
            e = strstr(s, ">");
            *e = '\0';

            strcpy(tag, s);

            s = e + 1;
            e = strstr(s, ">");
            *e = '\0';

            strcpy(val, s);

            sscanf(val, "0x%llX", &ret);
            printf("%-40s = %llu\n", tag, ret);

            s = strstr(e + 1, "<");
         }

         free(xml);
      }
      printf("\n");
   }
#endif
}

void Platform::InitStatsSaving(const std::string &csvFilename)
{
#if EGL_BRCM_driver_monitor
   if (m_statsFile == NULL)
   {
      m_statsFile = fopen(csvFilename.c_str(), "w");
      if (m_statsFile == NULL)
         BSG_THROW("Failed to open stats file for writing");

      fprintf(m_statsFile, "milliseconds");
      if (m_eglGetDriverMonitorXMLBRCM != NULL)
      {
         char *xml = (char *)malloc(4096 * sizeof(char));
         char *s, *e;
         char tag[256];

         if (xml != NULL)
         {
            m_eglInitDriverMonitorBRCM(GetContext().GetDisplay(), m_options.GetMonitorHw(), m_options.GetMonitorL3c());
            m_eglGetDriverMonitorXMLBRCM(GetContext().GetDisplay(), 4096, NULL, xml);

            s = strstr(xml, "<");
            while (s)
            {
               s++;
               e = strstr(s, ">");
               *e = '\0';

               strcpy(tag, s);

               s = e + 1;
               e = strstr(s, ">");
               *e = '\0';

               fprintf(m_statsFile, ",%s", tag);

               s = strstr(e + 1, "<");
            }

            free(xml);
         }

         fprintf(m_statsFile, "\n");
      }
   }
#endif
}

void Platform::SaveMonitorStats()
{
#if EGL_BRCM_driver_monitor
   if (m_statsFile != NULL && m_eglGetDriverMonitorXMLBRCM != NULL)
   {
      char *xml = (char *)malloc(4096 * sizeof(char));
      char *s, *e;
      char val[128];
      uint64_t ret;

      fprintf(m_statsFile, "%d", (int32_t)Time::Now().Milliseconds());

      if (xml != NULL)
      {
         m_eglGetDriverMonitorXMLBRCM(GetContext().GetDisplay(), 4096, NULL, xml);

         s = strstr(xml, "<");
         while (s)
         {
            s++;
            e = strstr(s, ">");
            *e = '\0';

            s = e + 1;
            e = strstr(s, ">");
            *e = '\0';

            strcpy(val, s);

            sscanf(val, "0x%llX", &ret);
            fprintf(m_statsFile, ",%llu", ret);

            s = strstr(e + 1, "<");
         }

         free(xml);
      }
      fprintf(m_statsFile, "\n");
      fflush(m_statsFile);
   }
#endif
}

void Platform::DoMonitorStats()
{
   if (m_options.GetPerfMonitoring())
   {
      InitStatsSaving("monitor_stats.csv");

      Time now = Time::Now();

      if (m_options.GetMonitorInterval() > 0 &&
          m_options.GetMonitorInterval() < (now - m_lastMonitorDumpTime).Seconds())
      {
         if (m_lastMonitorDumpTime.Milliseconds() != 0)  // Not first time around
         {
            // Dump the stats now
            DumpMonitorStats();
            SaveMonitorStats();
         }

#if EGL_BRCM_driver_monitor
         // Reset the stats counters
         m_eglTermDriverMonitorBRCM(GetContext().GetDisplay());
         m_eglInitDriverMonitorBRCM(GetContext().GetDisplay(), m_options.GetMonitorHw(), m_options.GetMonitorL3c());
#endif
         m_lastMonitorDumpTime = now;
      }
   }
}

int64_t Platform::GetDriverMonitorValue(const char *name, bool resetMonitor)
{
#if EGL_BRCM_driver_monitor
   static bool inited = false;

   if (m_eglGetDriverMonitorXMLBRCM != NULL)
   {
      char *xml = (char *)malloc(4096 * sizeof(char));
      char *s, *e;
      int64_t ret = -1;

      if (xml != NULL)
      {
         if (!inited)
         {
            m_eglInitDriverMonitorBRCM(GetContext().GetDisplay(), m_options.GetMonitorHw(), m_options.GetMonitorL3c());
            inited = true;
         }

         m_eglGetDriverMonitorXMLBRCM(GetContext().GetDisplay(), 4096, NULL, xml);

         s = strstr(xml, name) + strlen(name) + 1;
         if (s != NULL)
         {
            e = strstr(s, "<");
            *e = '\0';
            sscanf(s, "0x%llX", (uint64_t*)&ret);
         }

         free(xml);
      }

      if (resetMonitor)
      {
         // Reset the stats counters
         m_eglTermDriverMonitorBRCM(GetContext().GetDisplay());
         m_eglInitDriverMonitorBRCM(GetContext().GetDisplay(), m_options.GetMonitorHw(), m_options.GetMonitorL3c());
      }
      return ret;
   }
#endif
   return -1;
}

void Platform::SetupEye(eStereoEye eye)
{
   m_stereoEye = eye;

   if (!m_options.GetNoAutoContext())
   {
      if (eye == eMONO)
      {
         glViewport(0,                0,
                    GetWindowWidth(), GetWindowHeight());
      }
      else if (eye == eLEFT)
      {
         glViewport(0,                    0,
                    GetWindowWidth() / 2, GetWindowHeight());
      }
      else if (eye == eRIGHT)
      {
         glViewport(GetWindowWidth() / 2, 0,
                    GetWindowWidth() / 2, GetWindowHeight());
      }
   }
}

void Platform::UpdateFrameTimestamp()
{
   Time now = Time::Now();
   Time delta = now - m_frameTimestamp;
   m_frameTimestamp = now;
   m_animTimestamp = m_animTimestamp + (delta * m_rateMultiplier);
}

Time Platform::NowTimestamp() const
{
   Time now = Time::Now();
   Time ret;
   Time delta = now - m_frameTimestamp;
   ret = m_animTimestamp + (delta * m_rateMultiplier);
   return ret;
}

void Platform::Resize(uint32_t width, uint32_t height)
{
   m_windowWidth = width;
   m_windowHeight = height;

   ResizeNativeWindow(width, height);

   if (m_app != 0)
      m_app->HandleInternalResize(width, height);
}

void Platform::RestartPlatformDisplay()
{
   TerminatePlatformDisplay();
   InitializePlatformDisplay();
}

void Platform::CreateContext (Context & context, eBSGAPIVersion version)
{
   ApplicationOptions   options(m_options);

   options.SetApiVersion(version);

   if (options.GetRenderPixmap())
      m_platform->InitializeContext(context, options, m_nativePixmaps.front());
   else
      m_platform->InitializeContext(context, options, m_nativeWindows.front());
}

}
