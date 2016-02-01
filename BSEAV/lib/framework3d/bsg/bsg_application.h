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

#ifndef __BSG_APPLICATION_H__
#define __BSG_APPLICATION_H__

#include "bsg_common.h"
#include "bsg_platform.h"
#include "bsg_scene_node.h"
#include "bsg_constraint.h"
#include "bsg_gather_visitor.h"
#include "bsg_font.h"
#include "bsg_fbo.h"
#include "bsg_print.h"
#include "bsg_hud.h"
#include "bsg_render_options.h"
#include "bsg_quad_render.h"

#include <stdint.h>

#undef FindResource

/*! \mainpage Broadcom Scene Graph
 *
 * Welcome to BSG, an OpenGL ES2 3D Application Framework.  BSG aims to facilitate the construction of 3D applications written
 * over OpenGL ES 2.0.  It implements the boiler-plate code that is needed in every application, and hides the details of
 * specific patforms behind various class interfaces.
 *
 * To make full use of BSG, a knowledge of the fundamentals of 3D graphics and the OpenGL ES 2.0 API is necessary.  BSG abstracts
 * many of the details of OpenGL ES into class-based interfaces, but developers will need to understand concepts such as "shaders" and
 * the OpenGL ES 2.0 state.
 *
 * BSG is a C++ API and makes use of some modern C++ programming techniques including templates.  It has been tested under the Visual Studio 10
 * compiler and GCC (4.2.1), but older compilers may not be standards compliant and may not be able to build the software.
 *
 * BSG helps developers by removing much of the tedium of building an application.  Some of the highlights are:
 * - Application options for screen resolution and other settings.
 * - The scene graph which contains geometry, materials and cameras and their geometric relationships.
 * - Handle-based objects which are reference counted and deleted automatically.
 * - BSG effects files which are an easy to use text file containing shaders and GL state settings.
 * - The animation framework which allows time-based control over most of the elements in the graph using a comprehensive built in animators.
 * - User interactions from the IR remote or keyboard are abstracted into a single mechanism.
 * - Textures and geometry loaders.
 * - Graph sorting for best rendering performance.
 * - Animation time - you get the same animation rate regardless of rendering speed.
 * - Support for extensibility.
 *
 * \section Info_sec Finding More Information
 *
 * BSG ships with example applications of various levels of complexity.  The programmers' guide "Broadcom Scene Graph.docx" gives details of
 * how to build the applications, and has a walk-through of the hello_cube examples to get you up started.
 *
 * The example include:
 * - hello_cube is a step-by-step set of examples which introduce the core BSG features
 * - hello_text shows how to render text
 * - hello_fbo shows how to render to a texture
 * - hello_task is a simple example of BSG's multithreading support
 * - geom_viewer shows the various built-in geometry factories and the carousel object
 * - video_texturing shows how video can be decoded and used as a texture
 * - stereo_demo demonstrates how BSG supports stereoscopic rendering
 * - pg is an example implementation of an EPG
 * - planets is a solar system model which employs many of the mmore advanced BSG techniques including callbacks and LOD control.
 *
 * More details on specific aspects of BSG can be found here:
 * - bsg::Application
 * - \ref handles
 * - \ref animation
 * - \ref scenegraph
 * - \ref math
 */

namespace bsg
{

class ViewVolume;

//! The main BSG application interface.
//!
//! Derive your own application object from this and implement the pure virtual methods UpdateFrame() and
//! RenderFrame() in order to create the base of your application.
//!
//! Here's how your application's main function might look:
//! \code
//! using namespace bsg;
//! int main(int argc, char *argv[])
//! {
//!    ApplicationOptions   options;
//!    options.SetDisplayDimensions(1280, 720);
//!
//!    if (!options.ParseCommandLine(argc, argv))
//!       return 1;
//!
//!    Platform platform(options);
//!    MyApp    app(platform);
//!
//!    return platform.Exec();
//! }
//! \endcode
class Application
{
private:
   static Application   *m_instance;

public:
   //! Construct the base application. A valid bsg::Platform object is required.
   Application(Platform &platform);
   virtual ~Application();

   //! @name Pure virtual methods you must provide
   //@{

   //! Called by the framework to update the next frame.
   //! You should update the scene graph and animations to represent the new frame time, which can be accessed
   //! using FrameTimestamp().
   //!
   //! This should return true if anything changed, false otherwise. The RenderFrame() method will only be called
   //! if this returns true. If you return false, you should fill in idleTimeMs with the period until the
   //! next animation/update will begin. The app framework will idle (but still process events) until this
   //! time elapses.
   virtual bool UpdateFrame(int32_t *idleTimeMs) = 0;

   //! Called by the framework to render the next frame. By default you should not call eglSwapBuffers (or glFinish)
   //! at the end, as this will be done automatically by the framework. The default behaviours can be changed through
   //! application option SetAutoSwapBuffer(false)
   virtual void RenderFrame() = 0;
   //@}

   //! @name Overridable virtual methods
   //@{

   //! Called before the start of a frame if any key events are pending
   virtual void KeyEventHandler(KeyEvents &events)
   {
      // If the app isn't interested, we should at least remove events
      events.Clear();
   }

   //! Called before the start of a frame if any mouse events are pending
   virtual void MouseEventHandler(MouseEvents &events)
   {
      // If the app isn't interested, we should at least remove events
      events.Clear();
   }

   //! Called when the window is resized
   virtual void ResizeHandler(uint32_t /* width */, uint32_t /* height */) {}

   //@}

   //! @name Services
   //@{

   //! Get the options
   const ApplicationOptions &GetOptions() const       { return m_platform.GetOptions();      }

   //! Returns a string containing the full path and name of the resource file if it can be found in the list of resource folders.
   //! Returns an empty string otherwise.
   const std::string FindResource(const std::string &file) const { return m_platform.GetOptions().FindResource(file); }

   //! Returns a string containing the full path of the folder containing the resource file if it can be found in the list of resource folders.
   //! Returns an empty string otherwise.
   const std::string FindResourceFolder(const std::string &file) const { return m_platform.GetOptions().FindResourceFolder(file); }

   //! Call to prompt the main loop to exit, and start the termination sequence
   void Stop(uint32_t exitCode)                       { m_platform.Stop(exitCode);           }

   //! Returns half second averaged FPS counter
   float GetFPS() const                               { return m_platform.GetFPS();          }

   //! Access gl context
   Context &GetContext()                              { return m_platform.GetContext();      }

   //! Return the common timestamp for this render frame
   Time FrameTimestamp() const                        { return m_platform.FrameTimestamp();  }

   //! Return the timestamp for this moment in time (includes any rate multipler - use Time::Now() to get non-multiplied time).
   Time NowTimestamp() const                          { return m_platform.NowTimestamp();    }

   //! Set the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   void SetRateMultiplier(float val) { m_platform.SetRateMultiplier(val); }

   //! Get the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   const AnimatableFloat &GetRateMultiplier() const { return m_platform.GetRateMultiplier(); }

   //! Get the animation-rate multiplier (this is also animatable so you can transition the rate smoothly)
   AnimatableFloat &GetRateMultiplier() { return m_platform.GetRateMultiplier(); }

   //! Sets the display to stereoscopic mode (or not). Default is off.
   void SetStereoscopic(bool on) { m_platform.SetStereoscopic(on);   }

   //! Returns if set to stereoscopic mode
   bool IsStereo() const         { return m_platform.IsStereo();     }

   //! Returns if set to stereoscopic mode
   bool IsQuad() const           { return m_platform.IsQuad();       }

   //! Set and get the swap interval
   uint32_t GetSwapInterval() const { return m_platform.GetSwapInterval(); }
   void SetSwapInterval(uint32_t s) { m_platform.SetSwapInterval(s);       }

   //! Show the HUD?
   bool GetShowHUD() const    { return m_platform.GetShowHUD();   }
   void SetShowHUD(bool show) { m_platform.SetShowHUD(show);      }

   //! Show the FpsHUD?
   bool GetShowFpsHUD() const    { return m_platform.GetShowFpsHUD();   }
   void SetShowFpsHUD(bool show) { m_platform.SetShowFpsHUD(show);      }

   //! Get the width/height allowing for stereo
   uint32_t GetStereoWidth() const  { return m_platform.GetStereoWidth();   }
   uint32_t GetStereoHeight() const { return m_platform.GetStereoHeight();  }

   //! Get the current width/height of the window
   uint32_t GetWindowWidth() const     { return m_platform.GetWindowWidth();  }
   uint32_t GetWindowHeight() const    { return m_platform.GetWindowHeight(); }
   void GetWindowSize(uint32_t *width, uint32_t *height) { m_platform.GetWindowSize(width, height); }

   //! Set the current render target
   void SetRenderTarget(FramebufferHandle *handle = 0);
   const FramebufferHandle *GetRenderTarget() const { return m_renderTarget; }

   //! Render a scene graph to the current render target starting at rootNode with default render options.
   //! The optional camera argument can be used to selection a particular camera in the graph.  By default, the first camera found is used.
   void RenderSceneGraph(const SceneNodeHandle &rootNode, const CameraHandle &camera = m_nullCamera);

   //! Render a scene graph to the current render target starting at rootNode with specific render options.
   //! The optional camera argument can be used to selection a particular camera in the graph.  By default, the first camera found is used.
   void RenderSceneGraph(const SceneNodeHandle &rootNode, const RenderOptions &options, const CameraHandle &camera = m_nullCamera);

#ifndef BSG_STAND_ALONE
   //! Draw screen aligned 2D text at an exact pixel location
   //! Note - the result will change if your screen resolution changes
   void DrawTextStringAbs(const std::string &str, float xpos, float ypos,
                          FontHandle font, const Vec4 &color) const;

   //! Draw screen aligned 2D text (at a normalized float position - 0 to 1 in x & y)
   void DrawTextString(const std::string &str,
                       float xpos, float ypos,
                       FontHandle font, const Vec4 &color) const;
#endif

   //! Draw the HUD
   void DrawHud()                                  { m_devHud->Draw(); }

   //! Draw the FPS HUD
   void DrawFpsHud()                               { m_fpsHud->Draw();                          }

   //! Handle a keypress while the HUD is active
   void HandleKeyHud(const KeyEvent &event)        { m_devHud->HandleKey(event);                }

   //! Initiates a grab of the next rendered frame
   void InitiateFrameGrab()                        { m_platform.InitiateFrameGrab();            }

   //! Returns an estimate of the average CPU usage since this was last called as a percentage
   float EstimateCPUPercentage()                   { return m_platform.EstimateCPUPercentage(); }

   //! Returns true if the system is big-endian, false if little-endian
   bool IsBigEndian() const;

   //! Returns true if a USB keyboard is detected
   bool IsKeyboardAttached() const                 { return m_platform.IsKeyboardAttached();    }

   //! Returns true if a USB mouse is detected
   bool IsMouseAttached() const                    { return m_platform.IsMouseAttached();       }

   //! Add a resource path to the default list
   void AddResourcePath(const std::string &path)   { m_platform.AddResourcePath(path);          }

   //! Configure blending of graphics plane with video plane (on supported platforms)
   void ConfigureVideoGraphicsBlending(Platform::eVideoGraphicsBlend type, float graphicsPlaneAlpha, uint32_t zOrder = ~0u)
                                                   { m_platform.ConfigureVideoGraphicsBlending(type, graphicsPlaneAlpha, zOrder); }

   //! Test whether this is a left or right frame.  Always returns true in non-stereo mode.
   //! Left frames are drawn first.  Returns true in "normal mode".
   bool IsLeftFrame() const;

   //! Test if a frame is the first of a set of renders e.g. is it the left stereo frame
   //! or is it the first panel in quad mode.  Returns true in "normal mode".
   bool IsBeginFrame() const;

   //! Test if a frame is the last of a set of renders e.g. it is the right stero frame
   //! or is it the last panel in quad mode.  Returns true in "normal mode".
   bool IsEndFrame() const;

   //! Tell BSG that the GL state has been meddled with
   void ResetGLState()
   {
      ShadowState().SetToDefault(true);
      SetRenderTarget();
      GLStateMirrorReset();
      // BSG assumes this row alignment
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
   }

   //@}

   //! @name Special GL overrides to help support stereo
   //@{
   void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) const;
   void glClear(GLbitfield mask) const;
   //@}

   //! The singleton instance accessor
   static Application *Instance()
   {
      if (m_instance == nullptr)
         BSG_THROW("Application hasn't been created.  No instance available");

      return m_instance;
   }

   QuadRender &GetQuadRender() const { return *m_quad; }

   friend class NativePixmap;
   friend class Video;
   friend class VideoDecoder;
   friend class Platform;

private:
   void SetupCamera(Mat4 *view, Mat4 *proj, ViewVolume *frustum, GatherVisitor &gather, const CameraHandle &camera, const RenderOptions &options) const;
   void AdjustStereoEyePosition(const Camera *cam, Mat4 *viewMx) const;
   void HandleInternalResize(uint32_t w, uint32_t h);

private:
   Platform                   &m_platform;
   DevHud                     *m_devHud;
   FpsHud                     *m_fpsHud;
   bool                       m_bigEndian;
   FramebufferHandle          *m_renderTarget;

   static CameraHandle        m_nullCamera;
   std::unique_ptr<QuadRender>  m_quad;
};

class RawApplication : public Application
{
public:
   //! Construct a raw application. This is designed for basic test display bringup and not much more.
   //! A valid bsg::Platform object is required.
   RawApplication(Platform &platform) : Application(platform) {}
   virtual ~RawApplication() {}

   //! Called by the framework to update the next frame.
   //! You should update the scene graph and animations to represent the new frame time, which can be accessed
   //! using FrameTimestamp().
   //!
   //! This should return true if anything changed, false otherwise. The RenderFrame() method will only be called
   //! if this returns true. If you return false, you should fill in idleTimeMs with the period until the
   //! next animation/update will begin. The app framework will idle (but still process events) until this
   //! time elapses.
   virtual bool UpdateFrame(int32_t *idleTimeMs) { return true; }

   //! Called by the framework to render the next frame. By default you should not call eglSwapBuffers (or glFinish)
   //! at the end, as this will be done automatically by the framework. The default behaviours can be changed through
   //! application option SetAutoSwapBuffer(false)
   virtual void RenderFrame() {}
};

}

#endif

