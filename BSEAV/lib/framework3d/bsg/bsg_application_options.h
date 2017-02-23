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
#ifndef __BSG_APPLICATION_OPTIONS_H__
#define __BSG_APPLICATION_OPTIONS_H__

#include "bsg_common.h"
#include "bsg_vector.h"

#include <stdint.h>
#include <string>
#include <vector>

// MS namespace pollution
#undef FindResource

namespace bsg
{

typedef enum {
   eGLES_1_1 = 1,
   eGLES_2_0 = 2,
   eGLES_3_0 = 4,
   eOPVG_1_1 = -1
} eBSGAPIVersion;

//! Interface for application level extra argument parsing.
//! Derive from this class to have unknown arguments passed to your application for handling.
class ArgumentParser
{
public:
   virtual ~ArgumentParser() {}
   //! Process a command line argument.
   //! Return true if you recognize the argument and have handled it.
   //! Return false to indicate this is an option you don't recognize - an error.
   virtual bool ParseArgument(const std::string &arg) = 0;

   //! Return a string containing usage descriptions of the extra arguments you can handle.
   virtual std::string UsageString() const = 0;
};

//! Encapsulates all of the application level options that can be used during initialization of
//! Application.
//!
//! The options can be initialized by parsing the standard command line options using ParseCommandLine.
//! Any overrides your application wants to make should be done after parsing the command line.
class ApplicationOptions
{
public:
   ApplicationOptions();

   //! Parse command line. Any unrecognized arguments will be sent via the extraParser (if given)
   //! for the application to process itself.
   bool ParseCommandLine(int argc, char **argv, ArgumentParser *extraParser = NULL);

   //! Tests the argument string for a match against the given str.
   static bool ArgMatch(const char *arg, const char *str);

   static bool UIntMatch(uint32_t *x, const char *arg, const char *fmt);
   static bool IntMatch(int32_t *x, const char *arg, const char *fmt);
   static bool FloatMatch(float *x, const char *arg, const char *fmt);
   static bool StringMatch(std::vector<char> *str, const char *arg, const char *fmt);
   static bool CoordMatch(int32_t *x, int32_t *y, const char *arg, const char *fmt);
   static bool FlagMatch(bool *flag, const char *arg, const char *name);

   //! Returns a string containing the full path and name of the resource file if it can be found in the list of resource folders.
   //! Returns an empty string otherwise.
   const std::string FindResource(const std::string &file) const;

   //! Returns a string containing the full path of the folder containing the resource file if it can be found in the list of resource folders.
   //! Returns an empty string otherwise.
   const std::string FindResourceFolder(const std::string &file) const;

   //! Add a path to the resource file search list
   void AddResourcePath(const std::string &path);

   //! Shortcut for setting width, height and, optionally, x & y
   void SetDisplayDimensions(uint32_t w, uint32_t h, int32_t x = 0, int32_t y = 0)
   {
      m_vpW = w;
      m_vpH = h;
      m_x   = x;
      m_y   = y;
   }

   uint32_t GetWidth()   const { return m_vpW;  }
   uint32_t GetHeight()  const { return m_vpH;  }
   int32_t  GetOffsetX() const { return m_x;    }
   int32_t  GetOffsetY() const { return m_y;    }

   //! Shortcut for setting color bits, depth bits and stencil bits requested
   void SetDisplayBits(uint32_t colorBits, int32_t depthBits = -1, int32_t stencilBits = -1)
   {
      m_bpp = colorBits;

      // Check that it is not VG
      if (m_apiVersion != eOPVG_1_1)
      {
         if (depthBits >= 0)
            m_depthBits = depthBits;

         if (stencilBits >= 0)
            m_stencilBits = stencilBits;
      }
      else
      {
         // If it is VG: no depth (or stencil) except
         // if a alpha mask is requested and in this case
         // SetVGAlphaMask should  be called
         m_depthBits = 0;
         m_stencilBits = 0;
      }
   }

   void SetVGAlphaMask(uint32_t colorBits, uint32_t almask_bits)
   {
      m_bpp = colorBits;

      m_depthBits = almask_bits;
      m_stencilBits = 0;
   }

   uint32_t GetColorBits() const    { return m_bpp;                     }
   uint32_t GetAlphaBits() const    { return m_alphaBits;               }
   uint32_t GetDepthBits() const    { return m_depthBits;               }
   uint32_t GetStencilBits() const  { return m_stencilBits;             }

   void SetMultisample(bool set)    { m_useMultisample = set;           }
   bool GetMultisample() const      { return m_useMultisample;          }

   //! Sets the number of vsyncs between displayed frames. e.g.
   //! - 0 = un-synced - frames are displayed as quickly as possible. Tearing may result. Only really useful for benchmarking.
   //! - 1 = swap every vsync - maximum display rate of 60fps.
   //! - 2 = swap every 2 vsycns - maximum display rate of 30fps.
   //! - 5 = swap every 5 vsycns - maximum display rate of 12fps.
   void SetSwapInterval(uint32_t i) { m_swapInterval = i;               }
   uint32_t GetSwapInterval() const { return m_swapInterval;            }

   //! Turns on stereoscopic display mode.
   //! Each scene is rendered twice, once for the left eye, once for the right, and displayed
   //! side-by-side. On a 3DTV, stereo mode is invoked and with appropriate glasses, the depth effect can be seen.
   //!
   //! This mode can also be changed in the live head-up-display.
   void SetStereo(bool set)         { m_stereo = set;                   }
   bool GetStereo() const           { return m_stereo;                  }

   //! Sets the display to stretch the content to fit, rather than displayed a 1:1 pixel ratio.
   void SetStretch(bool set)        { m_stretchToFit = set;             }
   bool GetStretch() const          { return m_stretchToFit;            }

   //! Preserves the framebuffer during swap. This has a HUGE impact on performance, so use at your own risk!!
   void SetPreserve(bool set)       { m_usePreservingSwap = set;        }
   bool GetPreserve() const         { return m_usePreservingSwap;       }

   //! Sets logging to file on or off.
   void SetLog(bool set)            { m_log = set;                      }
   bool GetLog() const              { return m_log;                     }

   //! If on, this prints the current FPS to the console once per second.
   void SetShowFPS(bool set)        { m_showFps = set;                  }
   bool GetShowFPS() const          { return m_showFps;                 }

   //! Turn on if rendering to a pixmap. This mode is NOT the recommended way to do composited rendering! It's slow.
   void SetRenderPixmap(bool set)   { m_renderToPixmap = set;           }
   bool GetRenderPixmap() const     { return m_renderToPixmap;          }

   //! Turn the head-up-display on or off. This can also be achieved with the appropriate remote control key sequence.
   void SetShowHUD(bool set)        { m_showDevHUD = set;               }
   bool GetShowHUD() const          { return m_showDevHUD;              }

   //! Turn the fps head-up-display on or off.
   void SetShowFpsHUD(bool set)     { m_showFpsHUD = set;               }
   bool GetShowFpsHUD() const       { return m_showFpsHUD;              }

   //! Print the GL renderer information string.
   void SetShowRenderer(bool set)   { m_showRenderer = set;             }
   bool GetShowRenderer() const     { return m_showRenderer;            }

   //! Sets the animation rate multiplier. A multiplier of 10 will force the animation to run 10 times faster than normal.
   //! Values between 0 and 1 will make the animation run slower.
   //! Note: the animation rate is not the same as frame-rate.
   void SetRate(float rate)         { m_rateMultiplier = rate;          }
   float GetRate() const            { return m_rateMultiplier;          }

   //! Sets the client id used for multi-process initialization
   void SetClientID(int32_t val)    { m_clientID = val;                 }
   int32_t GetClientID() const      { return m_clientID;                }

   //! Uses authenticated server join or not. For refsw_server integration.
   bool GetAuthenticatedClient() const  { return m_authenticatedClient; }
   void SetAuthenticatedClient(bool val) { m_authenticatedClient = val; }

   //! Force HDMI output at requested resolution. Ignore HDMI hotplug callback.
   void SetForceHDMI(bool tf)       { m_forceHDMI = tf;                 }
   bool GetForceHDMI() const        { return m_forceHDMI;               }

   //! Set HDMI output refresh rate
   void SetDisplayRefreshRate(uint8_t hz) { m_displayRefreshRate = hz;   }
   uint8_t GetDisplayRefreshRate() const  { return m_displayRefreshRate; }

   //! Select an interlaced display or not
   void SetDisplayInterlace(bool tf)       { m_displayInterlace = tf;    }
   bool GetDisplayInterlace() const        { return m_displayInterlace;  }

   // @cond
   void SetBandwidth(uint32_t bw, uint32_t freq)
   {
      m_bandwidth     = bw;
      m_memcFrequency = freq;
   }

   uint32_t GetBandwidth() const    { return m_bandwidth;               }
   uint32_t GetMemFrequency() const { return m_memcFrequency;           }

   bool GetNoAutoContext() const    { return m_noAutoContext;           }
   void SetNoAutoContext(bool val)  { m_noAutoContext = val;            }

   // @endcond

   //! Dump performance monitoring statistics
   void SetPerfMonitoring(bool set) { m_perfMonitoring = set;           }
   bool GetPerfMonitoring() const   { return m_perfMonitoring;          }

   //! Show the run-time on exit.
   void SetShowRuntime(bool set)    { m_showRuntime = set;              }
   bool GetShowRuntime() const      { return m_showRuntime;             }

   //! Sets the last and first frames to be displayed.
   //! Useful for terminating an application after a certain number of frames.
   void SetLastFirstFrames(uint32_t last, uint32_t first = 0)
   {
      m_lastFrame = last;
      if (m_firstFrame != 0)
         m_firstFrame = first;
   }

   void GetLastFirstFrames(uint32_t *last, uint32_t *first) const
   {
      if (last != 0)
         *last = m_lastFrame;

      if (first != 0)
         *first = m_firstFrame;
   }

   uint32_t GetMonitorInterval() const   { return m_monitorInterval; }
   uint32_t GetMonitorHw() const         { return m_monitorHw; }
   uint32_t GetMonitorL3c() const        { return m_monitorL3c; }

   void SetMonitorInterval(uint32_t monitorInterval) { m_monitorInterval = monitorInterval; }
   void SetMonitorHw(uint32_t monitorHw) { m_monitorHw = monitorHw; }
   void SetMonitorL3c(uint32_t monitorL3c) { m_monitorL3c = monitorL3c; }

   bool GetHeadless() const { return m_headless; }
   void SetHeadless(bool val) { m_headless = val; }

   std::vector<char *> GetPlatformArgs() const;

   bool GetProgramArguments(int &argc, char ***argv)
   {
      argc = m_argc;
      *argv = m_argv;
      return (m_argc > 0);
   }

   eBSGAPIVersion GetApiVersion() const { return m_apiVersion; }
   void SetApiVersion(eBSGAPIVersion apiVersion) { m_apiVersion = apiVersion; }

   bool AutoSwapBuffer() const { return m_autoSwapbuffer; }
   void SetAutoSwapBuffer(bool autoSwapbuffer) { m_autoSwapbuffer = autoSwapbuffer; }

   ApplicationOptions CalculateDerived() const;

   const Vec4 &GetBGColour()   const { return m_bgColour;   }
   float       GetFinalAlpha() const { return m_finalAlpha; }
   uint32_t    GetZOrder()     const { return m_zOrder;     }

   void SetBGColour(const Vec4 &colour)   { m_bgColour = colour;   }
   void SetFinalAlpha(float colour)       { m_finalAlpha = colour; }
   void SetZOrder(uint32_t order)         { m_zOrder = order;      }

   bool GetConformant() const { return m_conformant; }
   void SetConformant(bool conformant) { m_conformant = conformant; }

   bool GetSecure() const { return m_secure; }
   void SetSecure(bool secure) { m_secure = secure; }

private:
   bool ProcessArg(char *arg);
   void PrintUsage(const char *progName, const char *badArg, ArgumentParser *extraParser);
   void AddDefaultPath(const std::string &path);
   bool CheckSanity() const;

private:
   bool           m_useMultisample;
   bool           m_stretchToFit;
   bool           m_usePreservingSwap;
   bool           m_renderToPixmap;
   bool           m_showFps;
   bool           m_showDevHUD;
   bool           m_showFpsHUD;
   bool           m_log;
   bool           m_showRuntime;
   bool           m_perfMonitoring;
   bool           m_stereo;
   eBSGAPIVersion m_apiVersion;
   bool           m_noAutoContext;
   bool           m_forceHDMI;
   uint8_t        m_displayRefreshRate;
   bool           m_displayInterlace;
   bool           m_showRenderer;
   bool           m_authenticatedClient;
   uint32_t       m_depthBits;
   uint32_t       m_stencilBits;
   uint32_t       m_alphaBits;
   uint32_t       m_lastFrame;
   uint32_t       m_firstFrame;
   int32_t        m_x;
   int32_t        m_y;
   uint32_t       m_vpW;
   uint32_t       m_vpH;
   uint32_t       m_bpp;
   uint32_t       m_swapInterval;
   uint32_t       m_monitorInterval;
   uint32_t       m_monitorHw;
   uint32_t       m_monitorL3c;
   uint32_t       m_bandwidth;
   uint32_t       m_memcFrequency;
   int32_t        m_clientID;
   float          m_rateMultiplier;
   bool           m_autoSwapbuffer;
   bool           m_headless;
   Vec4           m_bgColour;
   float          m_finalAlpha;
   uint32_t       m_zOrder;
   bool           m_conformant;
   bool           m_secure;

   std::vector<std::string>   m_resourcePaths;
   std::vector<std::string>   m_defaultResourcePaths;
   std::vector<std::string>   m_platformArgs;

   // Use for the initialisation of Trellis platform
   int         m_argc;
   char        **m_argv;
};


}

#endif /* __BSG_APPLICATION_OPTIONS_H__ */


