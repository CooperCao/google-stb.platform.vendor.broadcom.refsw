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

#include "spytool_replay.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#ifndef BSG_STAND_ALONE
#include "bsg_image_png.h"
#endif // BSG_STAND_ALONE
#include "bsg_exception.h"

#include "Loader.h"
#include "Command.h"

#ifdef HAS_MD5
#include "md5.h"
#endif

#include <iostream>
#include <istream>
#include <fstream>
#include <set>

#include <sys/types.h>
#include <sys/stat.h>

#include "EGL/egl.h"

#include <inttypes.h>

using namespace bsg;

SpyToolReplay *gReplay;

static std::string sReplayFile("");
static uint32_t    sBufferMB = 64;
static uint32_t    sIOBufferKB = 64;
static uint32_t    sMaxCmds = 200000;
static bool        sReprime = true;
static bool        sShowFPS = false;
static uint32_t    sViewportRescaleX = 0;
static uint32_t    sViewportRescaleY = 0;
static uint32_t    sSkipFrames = 0;
static uint32_t    sSkipDraws = 0;
static bool        sWaitEachFrame = false;
static bool        sTiming = false;
static bool        sTimingToFiles = false;
static uint32_t    sDisplaySurface = 0;
static Command     *sExecutingCmd = NULL;
static std::set<uint32_t>  sSpecificFrames;
static std::set<uint32_t>  sSpecificDraws;
static bool        sRematchConfigs = true;
#ifndef BSG_STAND_ALONE
static bool        sSavePng = false;
#endif // BSG_STAND_ALONE

#ifndef WIN32
#include <dlfcn.h>
#endif

void *GetFunctionAddress(const char *funcName);

static void GetIntegerParamSet(const std::string &arg, std::set<uint32_t> &set)
{
   char *a = (char *)arg.c_str();
   char *token;

   token = strtok(a, "=,");
   while (token != NULL)
   {
      token = strtok(NULL, ",");
      if (token != NULL)
      {
         char *dash = strchr(token, '-');
         if (dash)
         {
            char firstStr[32];
            uint32_t first, second, a;

            memset(firstStr, 0, 32);
            strncpy(firstStr, token, (dash - token));

            first = atoi(firstStr);
            second = atoi(dash + 1);

            for (a = first; a <= second; a++)
               set.insert(a);
         }
         else
            set.insert(atoi(token));
      }
   }
}

/////////////////////////////////////////////////////////////////////
class CustomArgumentParser : public ArgumentParser
{
public:
   //! Process a command line argument.
   //! Return true if you recognize the argument and have handled it.
   //! Return false to indicate this is an option you don't recognize - an error.
   virtual bool ParseArgument(const std::string &arg)
   {
      char     c[1024];
      uint32_t d, e;

      if (ApplicationOptions::ArgMatch(arg.c_str(), "replay="))
      {
         if (sscanf(arg.c_str(), "replay=%s", c) == 1)
         {
            sReplayFile = c;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "bufferMB="))
      {
         if (sscanf(arg.c_str(), "bufferMB=%d", &d) == 1)
         {
            sBufferMB = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "ioBufferKB="))
      {
         if (sscanf(arg.c_str(), "ioBufferKB=%d", &d) == 1)
         {
            sIOBufferKB = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "maxCmds="))
      {
         if (sscanf(arg.c_str(), "maxCmds=%d", &d) == 1)
         {
            sMaxCmds = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "lowBuffer="))
      {
         if (sscanf(arg.c_str(), "lowBuffer=%s", c) == 1)
         {
            sReprime = !strcmp(c, "REPRIME");
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "rematchConfigs="))
      {
         if (sscanf(arg.c_str(), "rematchConfigs=%d", &d) == 1)
         {
            sRematchConfigs = (d != 0);
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "v="))
      {
         if (sscanf(arg.c_str(), "v=%dx%d", &d, &e) == 2)
         {
            sViewportRescaleX = d;
            sViewportRescaleY = e;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "skipFrames="))
      {
         if (sscanf(arg.c_str(), "skipFrames=%d", &d) == 1)
         {
            sSkipFrames = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "skipDraws="))
      {
         if (sscanf(arg.c_str(), "skipDraws=%d", &d) == 1)
         {
            sSkipDraws = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "waitEachFrame="))
      {
         if (sscanf(arg.c_str(), "waitEachFrame=%d", &d) == 1)
         {
            sWaitEachFrame = (d != 0);
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "displaySurface="))
      {
         if (sscanf(arg.c_str(), "displaySurface=%d", &d) == 1)
         {
            sDisplaySurface = d;
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "timing="))
      {
         if (sscanf(arg.c_str(), "timing=%d", &d) == 1)
         {
            sTiming = (d != 0);
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "timingToFiles="))
      {
         if (sscanf(arg.c_str(), "timingToFiles=%d", &d) == 1)
         {
            sTimingToFiles = (d != 0);
            return true;
         }
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "frames="))
      {
         GetIntegerParamSet(arg, sSpecificFrames);
         return true;
      }
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "draws="))
      {
         GetIntegerParamSet(arg, sSpecificDraws);
         return true;
      }
#ifndef BSG_STAND_ALONE
      else if (ApplicationOptions::ArgMatch(arg.c_str(), "savepng="))
      {
         if (sscanf(arg.c_str(), "savepng=%d", &d) == 1)
         {
            sSavePng = (d != 0);
            return true;
         }
      }
#endif // BSG_STAND_ALONE
      return false;
   }

   //! Return a string containing usage descriptions of the extra arguments you can handle.
   virtual std::string UsageString() const
   {
#ifndef BSG_STAND_ALONE
      return "replay=<filename>   set the capture file to use\n"
             "v=XxY               rescale viewport for an XxY screen\n"
             "skipFrames=N        don't render first N frames\n"
             "skipDraws=N         ignore the first N draw calls\n"
             "waitEachFrame=[0|1] wait for a key-press between frames\n"
             "frames=<N,M,A-B,..> only render frames in list\n"
             "draws=<N,M,A-B,..>  only process draw calls numbers in list\n"
             "bufferMB=N          use N MB for replay buffer (default 64MB)\n"
             "ioBufferKB=N        use N KB as read chunk size (default 64KB)\n"
             "maxCmds=N           set maximum number of commands in replay buffer (default 200000)\n"
             "lowBuffer=<REPRIME|CONTINUE> re-prime buffers when low (default) or carry on running in background\n"
             "rematchConfigs=[0|1]         if 1 will try to find a matching config. Will use the original config id otherwise\n"
             "savepng=[0|1]       if 1 will take a hash of the frame data and save as <hash>.png\n"
             "displaySurface=N    if multiple surfaces are created, which one should be on the display (0 is the first)\n"
             "timing=[0|1]        show the timing of each API call as it is executed\n"
             "timingToFiles=[0|1] autogenerate the timing data into seperate frames as CSV files\n"
         ;
#else
      return "replay=<filename>   set the capture file to use\n"
             "v=XxY               rescale viewport for an XxY screen\n"
             "skipFrames=N        don't render first N frames\n"
             "skipDraws=N         ignore the first N draw calls\n"
             "waitEachFrame=[0|1] wait for a key-press between frames\n"
             "frames=<N,M,A-B,..> only render frames in list\n"
             "draws=<N,M,A-B,..>  only process draw calls numbers in list\n"
             "bufferMB=N          use N MB for replay buffer (default 64MB)\n"
             "ioBufferKB=N        use N KB as read chunk size (default 64KB)\n"
             "maxCmds=N           set maximum number of commands in replay buffer (default 200000)\n"
             "lowBuffer=<REPRIME|CONTINUE> re-prime buffers when low (default) or carry on running in background\n"
             "rematchConfigs=[0|1]         if 1 will try to find a matching config. Will use the original config id otherwise\n"
             "displaySurface=N    if multiple surfaces are created, which one should be on the display (0 is the first)\n"
             "timing=[0|1]        show the timing of each API call as it is executed\n"
             "timingToFiles=[0|1] autogenerate the timing data into seperate frames as CSV files\n"
         ;
#endif // BSG_STAND_ALONE
   }
};

/////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////

SpyToolReplay::SpyToolReplay(Platform &platform) :
   Application(platform),
   m_platform(platform),
   m_data(NULL),
   //m_curDrawSurface(EGL_NO_SURFACE),
   m_hasWindow(false),
   m_curFrameFunc(NULL),
   m_loadedSwapCount(0),
   m_framesSinceLastFPS(0),
   m_displaySurfaceIndex(0),
   m_curSurfaceIndex(0),
   m_curDrawCall(0)
{
   m_loader = new Loader(this);

   m_scratchMemPtr = new uint8_t[2048 * 2048 * 4];

   m_displayMap[0] = 0;
   m_surfaceMap[0] = 0;
   m_contextMap[0] = 0;
   m_configMap[0] = 0;
   m_clientBufferMap[0] = 0;
   m_shaderMap[0] = 0;
   m_programMap[0] = 0;
   m_uniformMap[std::make_pair(0, 0)] = 0;

   gReplay = this;

   AddDisplayMapping(0, EGL_NO_DISPLAY);
   AddSurfaceMapping(0, EGL_NO_SURFACE);
   AddContextMapping(0, EGL_NO_CONTEXT);

   m_timerStart = bsg::Time::Now();
   m_frameStart = bsg::HighResTime::Now();
   m_timeprobes.clear();
}

SpyToolReplay::~SpyToolReplay()
{
   for (std::set<EGLDisplay>::const_iterator display = m_initedDisplays.begin();
      display != m_initedDisplays.end(); ++display)
   {
      eglTerminate(*display);
   }

   eglReleaseThread();

   delete [] m_scratchMemPtr;
   delete [] m_data;
   delete m_loader;
}

void SpyToolReplay::KeyEventHandler(KeyEvents &queue)
{
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
            Stop(255);
            break;
         default :
            break;
         }
      }
   }
}

void SpyToolReplay::ResizeHandler(uint32_t width, uint32_t height)
{
}

bool SpyToolReplay::UpdateFrame(int32_t * /*idleMs*/)
{
   return true;
}

void SpyToolReplay::AddDisplayMapping(uint32_t handle, EGLDisplay real) { m_displayMap[handle] = real; }
EGLDisplay SpyToolReplay::MapDisplay(uint32_t handle)                   { return m_displayMap[handle]; }
bool SpyToolReplay::HasDisplayMapping(uint32_t handle)                  { return m_displayMap.find(handle) != m_displayMap.end(); }

void SpyToolReplay::AddSurfaceMapping(uint32_t handle, EGLSurface real) { m_surfaceMap[handle] = real; }
EGLSurface SpyToolReplay::MapSurface(uint32_t handle)                   { return m_surfaceMap[handle]; }

void SpyToolReplay::AddContextMapping(uint32_t handle, EGLContext real) { m_contextMap[handle] = real; }
EGLContext SpyToolReplay::MapContext(uint32_t handle)                   { return m_contextMap[handle]; }

void SpyToolReplay::AddConfigMapping(uint32_t handle, EGLConfig real)
{
   if (sRematchConfigs)
      m_configMap[handle] = real;
}

bool SpyToolReplay::HasConfigMapping(uint32_t handle)
{
   return m_configMap.find(handle) != m_configMap.end();
}

EGLConfig SpyToolReplay::MapConfig(uint32_t handle)
{
   if (!sRematchConfigs)
      return (EGLConfig)(uintptr_t)handle;

   std::map<uint32_t, EGLConfig>::iterator iter = m_configMap.find(handle);
   if (iter != m_configMap.end())
      return iter->second;

   return (EGLConfig)(uintptr_t)(handle); // Special case to handle calls to eglGetConfigAttrib before eglCreateWindowSurface
}

void SpyToolReplay::AddClientBufferMapping(uint32_t handle, EGLClientBuffer real)   { m_clientBufferMap[handle] = real; }
EGLClientBuffer SpyToolReplay::MapClientBuffer(uint32_t handle)                     { return m_clientBufferMap[handle]; }

void SpyToolReplay::AddShaderMapping(uint32_t handle, GLuint real) { m_shaderMap[handle] = real; }
GLuint SpyToolReplay::MapShader(uint32_t handle)                   { return m_shaderMap[handle]; }

void SpyToolReplay::AddEGLSyncMapping(uint32_t handle, EGLSyncKHR real) { m_eglSyncMap[handle] = real; }
EGLSyncKHR SpyToolReplay::MapEGLSync(uint32_t handle)                   { return m_eglSyncMap[handle]; }

void SpyToolReplay::AddEGLImageMapping(uint32_t handle, EGLImageKHR real) { m_eglImageMap[handle] = real; }
EGLImageKHR SpyToolReplay::MapEGLImage(uint32_t handle)                   { return m_eglImageMap[handle]; }

#if V3D_TECH_VERSION == 3
void SpyToolReplay::AddSyncMapping(uint32_t handle, GLsync real)  { m_syncMap[handle] = real; }
GLsync SpyToolReplay::MapSync(uint32_t handle)                    { return m_syncMap[handle]; }

void SpyToolReplay::AddQueryMapping(uint32_t handle, GLuint real)       { m_queryMap[handle] = real; }
GLuint SpyToolReplay::MapQuery(uint32_t handle)                         { return m_queryMap[handle]; }

void SpyToolReplay::AddVertexArrayMapping(uint32_t handle, GLuint real) { m_vertexArrayMap[handle] = real; }
GLuint SpyToolReplay::MapVertexArray(uint32_t handle)                   { return m_vertexArrayMap[handle]; }

void SpyToolReplay::AddSamplerMapping(uint32_t handle, GLuint real)     { m_samplerMap[handle] = real; }
GLuint SpyToolReplay::MapSampler(uint32_t handle)                       { return m_samplerMap[handle]; }

void SpyToolReplay::AddTFMapping(uint32_t handle, GLuint real)          { m_tfMap[handle] = real; }
GLuint SpyToolReplay::MapTF(uint32_t handle)                            { return m_tfMap[handle]; }

void SpyToolReplay::AddProgramPipelineMapping(uint32_t handle, GLuint real) { m_programPipelineMap[handle] = real; }
GLuint SpyToolReplay::MapProgramPipeline(uint32_t handle)                   { return m_programPipelineMap[handle]; }
#endif

void SpyToolReplay::AddProgramMapping(uint32_t handle, GLuint real)  { m_programMap[handle] = real; }
GLuint SpyToolReplay::MapProgram(uint32_t handle)                    { return m_programMap[handle]; }

void SpyToolReplay::AddUniformMapping(uint32_t handle, GLuint real, uint32_t program)
{
   m_uniformMap[std::make_pair(program, handle)] = real;
}

GLuint SpyToolReplay::MapUniform(uint32_t handle, uint32_t program)
{
   if (handle == (uint32_t)-1)
      return (uint32_t)-1;
   return m_uniformMap[std::make_pair(program, handle)];
}

void SpyToolReplay::AddLocationMapping(uint32_t handle, GLuint real, uint32_t program)
{
   //printf("LocMap(p=%d) %d->%d\n", program, handle, real);
   m_locationMap[std::make_pair(program, handle)] = real;
}

GLuint SpyToolReplay::MapLocation(uint32_t handle, uint32_t program)
{
   //printf("p=%d using %d->%d\n", program, handle,  m_locationMap[std::make_pair(program, handle)]);
   if (handle == (uint32_t)-1)
      return (uint32_t)-1;

   std::map<std::pair<uint32_t, uint32_t>, GLuint>::iterator iter = m_locationMap.find(std::make_pair(program, handle));
   if (iter != m_locationMap.end())
      return iter->second;
   else
      return handle;
}

void SpyToolReplay::DisplayInited(EGLDisplay display)
{
   m_initedDisplays.insert(display);
}

void SpyToolReplay::DisplayTermed(EGLDisplay display)
{
   m_initedDisplays.erase(display);
}

void *SpyToolReplay::MapDataImp(uint32_t handle, const char *file, uint32_t line)
{
   if (handle == 0)
      return NULL;

   if (handle < m_dataSize)
      return (void*)&m_data[handle];
   else
   {
      BSG_THROW("Data access out of range in " << file << " at line " << line);
      return NULL;
   }
}

EGLNativeWindowType SpyToolReplay::GetNativeWindow(uint32_t id, uint32_t w, uint32_t h, uint32_t /*r*/, uint32_t /*g*/, uint32_t /*b*/, uint32_t /*a*/)
{
   bool firstWindow = !m_hasWindow;

   m_hasWindow = true;

   if (firstWindow)
   {
      if (m_platform.GetWindowWidth() != w || m_platform.GetWindowHeight() != h)
         m_platform.Resize(w, h);

      return m_platform.GetNativeWindow();
   }
   else
      return m_platform.NewNativeWindow(0, 0, w, h);
}

EGLNativePixmapType SpyToolReplay::GetNativePixmap(uint32_t id, uint32_t w, uint32_t h, uint32_t r, uint32_t /*g*/, uint32_t /*b*/, uint32_t a)
{
   std::map<uint32_t, NativePixmap*>::iterator iter = m_pixmaps.find(id);
   if (iter != m_pixmaps.end())
   {
      NativePixmap *pix = iter->second;
      delete pix;
   }

   if (r == 5 && a == 0)
      m_pixmaps[id] = new NativePixmap(w, h, NativePixmap::RGB565_TEXTURE);
   else
      m_pixmaps[id] = new NativePixmap(w, h, NativePixmap::ABGR8888_TEXTURE);

   return m_pixmaps[id]->EGLPixmap();
}

void SpyToolReplay::SetCaptureDataFile(const char *file)
{
   struct stat buf;
   if (stat(file, &buf) != 0)
      BSG_THROW("Unable to stat capture data file");

   FILE *fp = fopen(file, "rb");
   if (fp == NULL)
      BSG_THROW("Unable to open capture data file");

   m_dataSize = buf.st_size;

   m_data = new uint8_t[m_dataSize];

   if (fread(m_data, 1, m_dataSize, fp) != m_dataSize)
      BSG_THROW("Unable to read content of capture data file");

   fclose(fp);
}

/*
// Overridden to allow intercept of pixmap display
void SpyToolReplay::eglWaitClient()
{
   ::eglWaitClient();
   if (!m_hasWindow && m_curDrawSurface != EGL_NO_SURFACE)
   {
      std::map<uint32_t, NativePixmap*>::iterator iter = m_pixmaps.find((uint32_t)m_curDrawSurface);
      if (iter != m_pixmaps.end())
      {
         // Display this pixmap
         m_platform.BlitPixmap(iter->second);
      }
   }
}

// Overridden egl call
void SpyToolReplay::eglMakeCurrent(EGLDisplay disp, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   ::eglMakeCurrent(disp, draw, read, ctx);
   m_curDrawSurface = draw;
}
*/

void SpyToolReplay::SetCurrentFrameFunc(FrameFunc func)
{
   m_curFrameFunc = func;
}

// called prior to dispatching the real swapbuffers call in the replay
// can be used as an intercept.  Note! no display or surface passed as the
// intent is not to call real swapbuffers here.
void SpyToolReplay::FrameDone(uint32_t frameNum)
{
#ifndef BSG_STAND_ALONE
   if (sSavePng)
   {
      uint8_t result[16] = {0};
      const uint32_t width  = m_platform.GetWindowWidth();
      const uint32_t height = m_platform.GetWindowHeight();

      ImagePNG pngImage(width, height, Image::eRGBA8888);
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pngImage.GetData());

      // get a unique file name
#ifdef HAS_MD5
      md5((uint8_t *)pngImage.GetData(), pngImage.GetSize(), result);
#endif

      char file[1024];
      sprintf(file, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x.png",
         result[ 0], result[ 1], result[ 2], result[ 3],
         result[ 4], result[ 5], result[ 6], result[ 7],
         result[ 8], result[ 9], result[10], result[11],
         result[12], result[13], result[14], result[15]
         );
      printf("Saving file - %s\n", file);

      pngImage.Save(file);
   }
#endif // BSG_STAND_ALONE

   m_okToWait = true;
}

bool SpyToolReplay::ReplaceShaderSource(GLuint shader)
{
#if 0
   // This is an example of replacing a specific pair of shaders
   // with new code loaded from two text files.
   if (shader == 5560 || shader == 5561)
   {
      const uint32_t bufSize = 8 * 1024;
      char buf[bufSize];

      GLchar *ptrs[1];

      memset(buf, 0, bufSize);

      FILE *fp = NULL;
      if (shader == 5560)
         fp = fopen("vert.txt", "r");
      else
         fp = fopen("frag.txt", "r");

      if (fp)
      {
         fread(buf, 1, bufSize, fp);
         fclose(fp);
      }

      ptrs[0] = &buf[0];
      glShaderSource(shader, 1, ptrs, NULL);

      return true;
   }
#endif
   return false;
}

static const char *commandNames[] =
{
   "no_command", "v", "w", "",

   // This table content is auto-generated by running ./gen_hook_tables.py in v3dv3/tools/v3d/hook_codegen
#include "commandmap.inc"
};

void SpyToolReplay::RenderFrame()
{
   if (sReplayFile == "")
   {
      FrameFunc render = m_curFrameFunc;
      m_curFrameFunc = NULL;

      if (render)
         render();
      else
         Stop(0);
   }
   else
   {
      Command *cmd = NULL;
      bool    isSwapBuffers;

      while (m_loader->LoadCommand(&cmd))
      {
         // Count the number of swaps we load
         if (cmd->GetPacket().Item(0).GetFunc() == cmd_eglSwapBuffers)
            m_loadedSwapCount++;

         // isSwapBuffers will be set if we actually will swap (i.e. unskipped)
         isSwapBuffers = cmd->Execute(this, sTiming);
         sExecutingCmd = cmd;
         cmd->Clear();

         if (isSwapBuffers)
         {
            m_framesSinceLastFPS++;

            float elapsed = (bsg::Time::Now() - m_timerStart).FloatSeconds();
            if (elapsed > 1.0f || sWaitEachFrame)
            {
               printf("%.1f fps - frameCount %d, drawCount %d\n", (float)m_framesSinceLastFPS / elapsed, m_loadedSwapCount, m_curDrawCall);
               m_timerStart = bsg::Time::Now();
               m_framesSinceLastFPS = 0;
            }

            if (m_timeprobes.size() > 0)
            {
               if (sTimingToFiles)
               {
                  FILE *fp = stdout;
                  std::string csvFile;
                  std::string baseName;

                  std::size_t slashFound = sReplayFile.find_last_of("/\\");
                  baseName = sReplayFile.substr(slashFound+1);

                  std::size_t periodFound = baseName.find_last_of(".");
                  baseName = baseName.substr(0, periodFound);

                  csvFile = baseName + "_" + std::to_string(m_loadedSwapCount) + ".csv";
                  fp = fopen(csvFile.c_str(), "w");
                  if (fp == NULL)
                  {
                     printf("error opening file %s - %s\n", csvFile.c_str(), strerror(errno));
                     exit(0);
                  }

                  // Dump the timeprobes if any
                  for (auto& probe : m_timeprobes)
                  {
                     fprintf(fp, "%016" PRIi64 ", %08" PRIi64 ", %s\n",
                        probe.fromSOF, probe.cmdCost,
                        commandNames[probe.cmd * 4]);
                  }

                  fclose(fp);
               }
               else
               {
                  // Dump the timeprobes if any
                  for (auto& probe : m_timeprobes)
                  {
                     printf("[%016" PRIi64 "] [%08" PRIi64 "] %s\n",
                        probe.fromSOF, probe.cmdCost,
                        commandNames[probe.cmd * 4]);
                  }
               }
            }

            if (m_okToWait && sWaitEachFrame)
            {
               printf("Press return for next frame\r");
               fflush(stdout);
               getchar();
            }

            m_timeprobes.clear();
            m_timeprobes.reserve(1024);
            m_frameStart = bsg::HighResTime::Now();
         }
      }

      Stop(0);
   }
}

void SpyToolReplay::SetReplayFile(const std::string &filename)
{
   if (!m_loader->Open(filename, sBufferMB * 1024 * 1024, sIOBufferKB * 1024, sMaxCmds, sReprime))
      BSG_THROW("Unable to open capture data file");
}

void SpyToolReplay::SetFPSTimer(bool onOff)
{
   if (onOff)
   {
      bsg::Time pausedFor = bsg::Time::Now() - m_fpsPause;
      m_timerStart = m_timerStart + pausedFor;
   }
   else
      m_fpsPause = bsg::Time::Now();
}

bool SpyToolReplay::IncrSkipDrawCall()
{
   if (m_curDrawCall++ < sSkipDraws)
      return true;

   if (sSpecificDraws.size() > 0)
   {
      if (sSpecificDraws.find(m_curDrawCall - 1) == sSpecificDraws.end())
         return true;
   }

   // Get some info about this draw call
   // This is useful if you've isolated a draw call and want to get its
   // shader info
#if 0
   GLint i;
   GLuint u[2];
   glGetIntegerv(GL_CURRENT_PROGRAM, &i);
   printf("Current program = %d\n", i);

   glGetAttachedShaders(i, 2, NULL, u);
   printf("Shaders = %d, %d\n", u[0], u[1]);

   GLchar buf[10000];
   GLsizei len;
   glGetShaderSource(u[0], 10000, &len, buf);
   printf("Len = %d, Source = \n%s\n", len, buf);

   glGetShaderSource(u[1], 10000, &len, buf);
   printf("\n\nLen = %d, Source = \n%s\n", len, buf);
#endif

   return false;
}

bool SpyToolReplay::SkipFrame(uint32_t frameNumber)
{
   m_okToWait = false;

   if (frameNumber < sSkipFrames)
      return true;

   if (sSpecificFrames.size() > 0)
   {
      if (sSpecificFrames.find(frameNumber) == sSpecificFrames.end())
         return true;
   }

   return false;
}

void SpyToolReplay::RescaleViewport(EGLSurface surf, uint32_t *x, uint32_t *y, uint32_t *w, uint32_t *h)
{
   if (sViewportRescaleX != 0 && sViewportRescaleY != 0)
   {
      const WindowSize &size(MapWindowSize(surf));

      *x = (sViewportRescaleX * *x) / size.m_w;
      *y = (sViewportRescaleY * *y) / size.m_h;
      *w = (sViewportRescaleX * *w) / size.m_w;
      *h = (sViewportRescaleY * *h) / size.m_h;
   }
}

void SpyToolReplay::AddWindowSizeMapping(EGLSurface surface, const WindowSize &vp)
{
   m_windowSizeMap[surface] = vp;
}

bool SpyToolReplay::HasWindowSizeMapping(EGLSurface surface)
{
   return m_windowSizeMap.find(surface) != m_windowSizeMap.end();
}

const WindowSize &SpyToolReplay::MapWindowSize(EGLSurface surface)
{
   return m_windowSizeMap[surface];
}

int main(int argc, char **argv)
{
   uint32_t ret = 0;

   try
   {
      // Create the default application options object
      ApplicationOptions   options;
      CustomArgumentParser extra;
      uint32_t             w = 1920, h = 1080;

      // Parse command lines
      if (!options.ParseCommandLine(argc, argv, &extra))
         return 1;

      if (sReplayFile == "")
      {
         SizeFunc getDefSize = (SizeFunc)GetFunctionAddress("GetDefaultSize");
         if (getDefSize)
            getDefSize(&w, &h);
      }

      // Request a specific display size
      options.SetDisplayDimensions(w, h);
      options.SetNoAutoContext(true);

      // Re-read any command-line options (possibly overriding the display size)
      if (!options.ParseCommandLine(argc, argv, &extra))
         return 1;

      if (options.GetShowFPS())
      {
         sShowFPS = true;
         options.SetShowFPS(false);
      }

      // Initialise the platform
      Platform       platform(options);

      // Initialise the application
      SpyToolReplay  app(platform);

      if (sReplayFile == "")
      {
         app.SetCurrentFrameFunc((FrameFunc)GetFunctionAddress("RenderCaptureFrame_000001"));
      }
      else
      {
         app.SetReplayFile(sReplayFile);
      }

      app.SetDisplaySurfaceIndex(sDisplaySurface);

      // Run the application
      ret = platform.Exec();
   }
   catch (const Exception &e)
   {
      // BSG will throw exceptions of type bsg::Exception if anything goes wrong
      std::cerr << "Exception : " << e.Message() << "\n";
   }
   catch (const char *e)
   {
      std::cerr << "Exception : " << e << "\n";
   }
   catch (...)
   {
      std::cerr << "Unhandled exception\n";
   }

   return ret;
}

#ifdef WIN32
#include <windows.h>

void *GetFunctionAddress(const char *funcName)
{
   void *res = (void*)GetProcAddress(GetModuleHandle(NULL), funcName);

   if (res == NULL)
   {
      DWORD err = GetLastError();
      LPTSTR errorText = NULL;

      FormatMessage(
         FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
         NULL,
         err,
         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
         (LPTSTR)&errorText,
         0,
         NULL);

      if (errorText)
      {
         std::cout << "GetFunctionAddress generated error : " << errorText << std::endl;
         LocalFree(errorText);
         errorText = NULL;
      }
      else
         std::cout << "GetFunctionAddress generated error : " << err << std::endl;
   }
   return res;
}

#else
void *GetFunctionAddress(const char *funcName)
{
   return (void*)dlsym(RTLD_DEFAULT, funcName);
}

#endif

