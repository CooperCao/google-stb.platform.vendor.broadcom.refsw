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
#ifndef __BSG_PLATFORM_H__
#define __BSG_PLATFORM_H__

#include "bsg_common.h"
#include "bsg_context.h"
#include "bsg_gl.h"
#include "bsg_time.h"
#include "bsg_key_events.h"
#include "bsg_mouse_events.h"
#include "bsg_application_options.h"
#include "bsg_hud.h"
#include "bsg_animatable.h"

#include <stdint.h>

namespace bsg
{

typedef void (* externalKeyEventHandlerCallback)(unsigned int);
typedef void (* externalKeyEventHandlerCallback2)(unsigned int, unsigned int);
typedef void (* externalMouseButtonEventHandlerCallback)(unsigned int, unsigned int);
typedef void (* externalMouseMoveEventHandlerCallback)(int, int);


class Application;

//! Helper interface for platform support
class NativePixmapData
{
public:
   virtual ~NativePixmapData() = 0;
};

//! Wraps a platform dependent native pixmap type
class NativePixmap
{
public:
   enum ePixmapFormat
   {
      RGB565_TEXTURE = 0,  //!< \deprecated
      YUV422_TEXTURE,      //!< \deprecated
      ABGR8888_TEXTURE,    //!< \deprecated

      eRGB565_TEXTURE = 0,
      eYUV422_TEXTURE,
      eABGR8888_TEXTURE,
      eYV12_TEXTURE
   };

   NativePixmap(uint32_t w, uint32_t h, ePixmapFormat format);
   virtual ~NativePixmap();

   EGLNativePixmapType EGLPixmap() const { return m_eglPixmap; }
   NativePixmapData *GetNativePixmapData() { return m_priv; }
   NativePixmap::ePixmapFormat GetFormat() const { return m_format; }
   uint32_t GetWidth() const { return m_width; }
   uint32_t GetHeight() const { return m_height; }
   uint32_t GetStride() const { return m_stride; }

   void *GetPixelDataPtr() const;

private:
   uint32_t             m_width;
   uint32_t             m_height;
   uint32_t             m_stride;
   ePixmapFormat        m_format;
   EGLNativePixmapType  m_eglPixmap;
   NativePixmapData     *m_priv;
};

//! The Platform class abstracts the platform specific functionality behind a common
//! interface. bsg::Application will use the platform interface to interact with the
//! underlying physical platform. You need to pass a Platform object into the
//! bsg::Application constructor.
class Platform
{
private:
   static Platform             *m_instance;

public:
   enum eStereoEye
   {
      eMONO = 0,
      eLEFT,
      eRIGHT
   };

   enum eVideoGraphicsBlend
   {
      eUSE_GRAPHICS_ALPHA = 0,
      eUSE_CONSTANT_ALPHA
   };

public:
   Platform(const ApplicationOptions &options);
   virtual ~Platform();

   //! The singleton instance accessor
   static Platform *Instance()
   {
      if (m_instance == NULL)
         BSG_THROW("Platform hasn't been created.  No instance available");

      return m_instance;
   }

   void Install(Application *app) { m_app = app; }

   /////////////////////////////////////
   //! Methods the application can call
   /////////////////////////////////////

   //! Get the current options
   const ApplicationOptions &GetOptions() const;

   //! Execute the application main loop. Returns the exit code.
   uint32_t Exec();

   //! Call to prompt the main loop to exit, and start the termination sequence
   void Stop(uint32_t exitCode);

   //! Push a key event into the app queue
   void PushKeyEvent(const KeyEvent &event);

   //! Push a key event into the app queue
   void PushMouseEvent(const MouseEvent &event);

   //! Return the common timestamp for this render frame
   Time FrameTimestamp() const { return m_animTimestamp; }

   //! Return the timestamp now. Includes any rate multiplier.
   Time NowTimestamp() const;

   //! Set the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   void SetRateMultiplier(float val) { m_rateMultiplier.Set(val); }

   //! Get the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   const AnimatableFloat &GetRateMultiplier() const { return m_rateMultiplier; }

   //! Get the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   AnimatableFloat &GetRateMultiplier() { return m_rateMultiplier; }

   //! Returns half second averaged FPS counter
   float GetFPS();

   //! Access gl context
   Context &GetContext() { return m_context; }

   //! Create a new context
   void CreateContext (Context & context, eBSGAPIVersion version);

   //! Sets the display to stereoscopic mode (or not). Default is off.
   void SetStereoscopic(bool on);

   //! Returns if set to stereoscopic mode
   bool IsStereo() const { return m_stereo; }

   //! Get and set the swap interval
   uint32_t GetSwapInterval() const { return m_swapInterval; }
   void SetSwapInterval(uint32_t s) { m_swapIntervalPending = true; m_swapInterval = s; }

   //! Show the HUD?
   bool GetShowHUD() const    { return m_showHUD;  }
   void SetShowHUD(bool show) { m_showHUD = show;  }

   //! Show the FPS HUD?
   bool GetShowFpsHUD() const    { return m_showFpsHUD;  }
   void SetShowFpsHUD(bool show) { m_showFpsHUD = show;  }

   //! Pin the mouse cursor.
   bool GetPinCursor() const { return m_pinCursor; }
   void SetPinCursor(bool pin) { m_pinCursor = pin; }

   //! Return the width/height allowing for stereo
   uint32_t GetStereoWidth()  const { return m_stereo ? m_options.GetWidth() / 2 : m_options.GetWidth(); }
   uint32_t GetStereoHeight() const { return m_options.GetHeight();                                      }

   //! Return the current width/height of the window
   uint32_t GetWindowWidth() const   { return m_windowWidth;              }
   uint32_t GetWindowHeight() const  { return m_windowHeight;             }
   void GetWindowSize(uint32_t *width, uint32_t *height) { *width = m_windowWidth; *height = m_windowHeight; }

   void Resize(uint32_t width, uint32_t height);

   //! Returns which eye is currently being rendered
   eStereoEye CurrentEye() const    { return m_stereoEye; }

   //! Initiates a grab of the next rendered frame
   void InitiateFrameGrab() { m_frameGrabPending = true; }

   //! Returns an estimate of the average CPU usage since this was last called as a percentage
   float EstimateCPUPercentage();

   //! Return a single driver monitor value
   int64_t GetDriverMonitorValue(const char *name, bool resetMonitor);

   //! Returns true if a USB mouse is detected
   bool IsMouseAttached() const;

   //! Returns true if a USB keyboard is detected
   bool IsKeyboardAttached() const;

   //! Set mouse move merging. When true, all moves are collapsed into a single move event.
   //! When false, all moves appear as individual events in the queue. Default is to merge.
   void SetMouseMoveMerging(bool tf)               { m_mouseMoveMerging = tf; }

   //! Returns whether move merging is on
   bool IsMouseMoveMerging() const                 { return m_mouseMoveMerging; }

   //! Set the absolute position of the mouse from which all relative mouse moves will be taken.
   void SetAbsoluteMousePosition(const IVec3 &pos) { m_absMousePos = pos; }

   //! Add a resource path
   void AddResourcePath(const std::string &path)   { m_options.AddResourcePath(path); }

   //! Return the platform identification
   std::string GetPlatformName()                    { return m_platformName; }

   //! Restart the display
   void RestartPlatformDisplay();

   //! Configure blending of graphics plane with video plane (on supported platforms)
   void ConfigureVideoGraphicsBlending(eVideoGraphicsBlend type, float graphicsPlaneAlpha, uint32_t zOrder);

   // @cond
   IVec3 AbsMP() const { return m_absMousePos; }
   IVec3 &RelMP() { return m_relMousePos; }
   EGLNativeWindowType GetNativeWindow()    { return m_nativeWindows.front(); }
   EGLNativeWindowType NewNativeWindow(uint32_t x = 0, uint32_t y = 0, uint32_t w = 0, uint32_t h = 0);

   void BlitPixmap(NativePixmap *pixmap);
   // @endcond

   NativePixmap *GetNativePixmapTarget() const { return m_nativePixmaps.front(); }

   void RenderFrameSequence();
   void UpdateFrameTimestamp();

   void ProcessKeyEvents();

   void RegisterKeyHandlerCallback (externalKeyEventHandlerCallback callback)
   {
      m_keyEvenHandler = callback;
   }

   void RegisterKeyHandlerCallback2 (externalKeyEventHandlerCallback2 callback)
   {
      m_keyEvenHandler2 = callback;
   }

   void RegisterMouseButtonHandlerCallback(externalMouseButtonEventHandlerCallback callback)
   {
      m_mouseButtonHandler = callback;
   }

   void RegisterMouseMoveHandlerCallback(externalMouseMoveEventHandlerCallback callback)
   {
      m_mouseMoveHandler = callback;
   }

   externalKeyEventHandlerCallback GetKeyEventHandlerCallback()
   {
      return m_keyEvenHandler;
   }

   externalKeyEventHandlerCallback2 GetKeyEventHandlerCallback2()
   {
      return m_keyEvenHandler2;
   }

   externalMouseButtonEventHandlerCallback GetMouseButtonHandlerCallback()
   {
     return m_mouseButtonHandler;
   }

   externalMouseMoveEventHandlerCallback GetMouseMoveHandlerCallback()
   {
     return m_mouseMoveHandler;
   }

   void InitializeDisplay();
   void InitializeDisplayNoContext();
   void TerminateDisplay();

private:
   /////////////////////
   //! Internal methods
   /////////////////////
   void ShowFPS();
   void SetupEye(eStereoEye eye);
   void DoFrameGrab();
   void DoMonitorStats();
   void DumpMonitorStats();
   void InitStatsSaving(const std::string &filename);
   void SaveMonitorStats();
   void ResizeNativeWindow(uint32_t w, uint32_t h);

   void RenderFrameSequenceMono();
   void RenderFrameSequenceStereo();

   void PostFrame();
   void PreFrame();

   /////////////////////////////
   //! Platform specific methods
   /////////////////////////////
   void InitializePlatform();
   void TerminatePlatform();
   void InitializePlatformDisplay();
   void TerminatePlatformDisplay();

   void SetBandwidth();

   void RunPlatformMainLoop();
   void GoIdle(int32_t ms);

   void RateMultiplierChanged();

public:
   // @cond
   class PlatformData
   {
   public:
      // Abstract class
      virtual ~PlatformData() {}

      // This implementation is good for most platforms.
      virtual void InitializeContext(Context &context, const ApplicationOptions &options, EGLNativeWindowType nativeWindow)
      {
         context.Initialize(options, nativeWindow);
      }

      virtual void InitializeContext(Context &context, const ApplicationOptions &options, NativePixmap *nativePixmap)
      {
         context.Initialize(options, nativePixmap);
      }
   };

   const PlatformData *GetPlatformData() const { return m_platform; }
   // @endcond

private:
   ApplicationOptions   m_options;
   uint32_t             m_exitCode;
   PlatformData         *m_platform;
   std::list<EGLNativeWindowType> m_nativeWindows;
   std::list<NativePixmap*>       m_nativePixmaps;
   Context              m_context;
   KeyEvents            m_keyEvents;
   KeyEvent             m_prevKey;
   MouseEvents          m_mouseEvents;
   bool                 m_mouseMoveMerging;
   IVec3                m_absMousePos;
   IVec3                m_relMousePos;

   eStereoEye           m_stereoEye;
   bool                 m_stereo;

   uint32_t             m_swapInterval;
   bool                 m_showHUD;
   bool                 m_showFpsHUD;
   bool                 m_pinCursor;
   uint32_t             m_windowWidth;
   uint32_t             m_windowHeight;

   DevHud               m_devHud;
   bool                 m_showedHudLastFrame;

   bool                 m_frameGrabPending;
   bool                 m_swapIntervalPending;

   // For fps
   Time                 m_lastFPSTime;
   uint32_t             m_lastFPSFrame;
   float                m_lastFPS;
   uint32_t             m_frame;
   Time                 m_frameTimestamp;
   Time                 m_animTimestamp;
   AnimatableFloat      m_rateMultiplier;

   Time                 m_lastMonitorDumpTime;
   FILE                 *m_statsFile;

   std::string          m_platformName;

   // Callbacks to derived application class
   Application          *m_app;

   externalKeyEventHandlerCallback           m_keyEvenHandler;
   externalKeyEventHandlerCallback2          m_keyEvenHandler2;
   externalMouseButtonEventHandlerCallback   m_mouseButtonHandler;
   externalMouseMoveEventHandlerCallback     m_mouseMoveHandler;

   // Driver monitor extension functions
#if EGL_BRCM_driver_monitor
   PFNEGLINITDRIVERMONITORBRCMPROC    m_eglInitDriverMonitorBRCM;
   PFNEGLGETDRIVERMONITORXMLBRCMPROC  m_eglGetDriverMonitorXMLBRCM;
   PFNEGLTERMDRIVERMONITORBRCMPROC    m_eglTermDriverMonitorBRCM;
#endif
};

}

#endif

