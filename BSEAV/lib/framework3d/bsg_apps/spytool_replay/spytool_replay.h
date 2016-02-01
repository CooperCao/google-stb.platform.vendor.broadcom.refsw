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

#ifndef __SPYTOOL_REPLAY_H__
#define __SPYTOOL_REPLAY_H__

#define BSG_NO_NAME_MANGLING

#include "bsg_application.h"
#include "bsg_animatable.h"
#include "bsg_animator.h"
#include "bsg_animation_list.h"
#include "bsg_time.h"

#include "Loader.h"

#include <map>
#include <set>
#include <vector>

using namespace bsg;

namespace bsg
{
   class NativePixmap;
}

typedef void (*FrameFunc)();
typedef void (*SizeFunc)(uint32_t *, uint32_t *h);

class WindowSize
{
public:
   WindowSize() {}

   WindowSize(uint32_t w, uint32_t h) :
      m_w(w), m_h(h)
   {
   }

   uint32_t m_w;
   uint32_t m_h;
};

class SpyToolReplay : public bsg::Application
{
public:
   SpyToolReplay(bsg::Platform &platform);
   ~SpyToolReplay();

public:
   void AddDisplayMapping(uint32_t handle, EGLDisplay real);
   EGLDisplay MapDisplay(uint32_t handle);
   bool HasDisplayMapping(uint32_t handle);

   void AddSurfaceMapping(uint32_t handle, EGLSurface real);
   EGLSurface MapSurface(uint32_t handle);

   void AddWindowSizeMapping(EGLSurface surface, const WindowSize &vp);
   const WindowSize &MapWindowSize(EGLSurface surface);
   bool HasWindowSizeMapping(EGLSurface surface);

   void AddContextMapping(uint32_t handle, EGLContext real);
   EGLContext MapContext(uint32_t handle);

   void AddConfigMapping(uint32_t handle, EGLConfig real);
   EGLConfig MapConfig(uint32_t handle);
   bool HasConfigMapping(uint32_t handle);

   void AddClientBufferMapping(uint32_t handle, EGLClientBuffer real);
   EGLClientBuffer MapClientBuffer(uint32_t handle);

   void AddShaderMapping(uint32_t handle, GLuint real);
   GLuint MapShader(uint32_t handle);

   void AddEGLSyncMapping(uint32_t handle, EGLSyncKHR real);
   EGLSyncKHR MapEGLSync(uint32_t handle);

   void AddEGLImageMapping(uint32_t handle, EGLImageKHR real);
   EGLSyncKHR MapEGLImage(uint32_t handle);

#if V3D_TECH_VERSION == 3
   void AddSyncMapping(uint32_t handle, GLsync real);
   GLsync MapSync(uint32_t handle);

   void AddQueryMapping(uint32_t handle, GLuint real);
   GLuint MapQuery(uint32_t handle);

   void AddVertexArrayMapping(uint32_t handle, GLuint real);
   GLuint MapVertexArray(uint32_t handle);

   void AddSamplerMapping(uint32_t handle, GLuint real);
   GLuint MapSampler(uint32_t handle);

   void AddTFMapping(uint32_t handle, GLuint real);
   GLuint MapTF(uint32_t handle);

   void AddProgramPipelineMapping(uint32_t handle, GLuint real);
   GLuint MapProgramPipeline(uint32_t handle);
#endif

   void AddProgramMapping(uint32_t handle, GLuint real);
   GLuint MapProgram(uint32_t handle);

   void AddUniformMapping(uint32_t handle, GLuint real, uint32_t program);
   GLuint MapUniform(uint32_t handle, uint32_t program);

   void AddLocationMapping(uint32_t handle, GLuint real, uint32_t program);
   GLuint MapLocation(uint32_t handle, uint32_t program);

   void DisplayInited(EGLDisplay display);
   void DisplayTermed(EGLDisplay display);

   void SetCaptureDataFile(const char *file);
   void *MapDataImp(uint32_t handle, const char *file, uint32_t line);

   EGLNativeWindowType GetNativeWindow(uint32_t id, uint32_t w, uint32_t h, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
   EGLNativePixmapType GetNativePixmap(uint32_t id, uint32_t w, uint32_t h, uint32_t r, uint32_t g, uint32_t b, uint32_t a);

   void *ScratchMem() { return m_scratchMemPtr; }

   void SetCurrentFrameFunc(FrameFunc func);

   void SetReplayFile(const std::string &filename);

   bool SkipFrame(uint32_t frameNumber);
   void RescaleViewport(EGLSurface surf, uint32_t *x, uint32_t *y, uint32_t *w, uint32_t *h);

   bool ReplaceShaderSource(GLuint shader);

   // Overridden methods
   virtual void RenderFrame();
   virtual void KeyEventHandler(bsg::KeyEvents &queue);
   virtual bool UpdateFrame(int32_t *idleMs);
   virtual void ResizeHandler(uint32_t width, uint32_t height);

/*
   void eglWaitClient();
   void eglMakeCurrent(EGLDisplay disp, EGLSurface draw, EGLSurface read, EGLContext ctx);
*/
   void FrameDone(uint32_t frameNum);

   bsg::Platform &GetPlatform() { return m_platform; }

   void SetFPSTimer(bool onOff);

   void SetDisplaySurfaceIndex(uint32_t index) { m_displaySurfaceIndex = index; }
   uint32_t GetCurSurfaceIndex() const { return m_curSurfaceIndex; }

   bool DisplayCurSurface() const { return m_curSurfaceIndex == m_displaySurfaceIndex; }
   void IncrementCurSurfaceIndex() { m_curSurfaceIndex++; }

   bool IncrSkipDrawCall();

   bsg::HighResTime FrameStartTime() const { return m_frameStart; };

   struct timeprobe
   {
      eGLCommand cmd;
      int64_t fromSOF;
      int64_t cmdCost;
   };

   void AddTiming(eGLCommand cmd, int64_t fromSOF, int64_t cmdCost)
   {
      m_timeprobes.push_back({ cmd, fromSOF, cmdCost });
   }

private:
   bsg::Platform  &m_platform;
   uint8_t        *m_scratchMemPtr;
   uint8_t        *m_data;
   uint32_t       m_dataSize;
   //EGLSurface        m_curDrawSurface;
   bool           m_hasWindow;
   FrameFunc      m_curFrameFunc;

   Loader         *m_loader;
   uint32_t       m_loadedSwapCount;
   uint32_t       m_framesSinceLastFPS;
   bsg::Time      m_timerStart;
   bsg::Time      m_fpsPause;
   bsg::HighResTime  m_frameStart;

   uint32_t       m_displaySurfaceIndex;
   uint32_t       m_curSurfaceIndex;
   uint32_t       m_curDrawCall;
   bool              m_okToWait;

   std::map<uint32_t, EGLDisplay>       m_displayMap;
   std::map<uint32_t, EGLSurface>       m_surfaceMap;
   std::map<uint32_t, EGLContext>       m_contextMap;
   std::map<uint32_t, EGLConfig>        m_configMap;
   std::map<uint32_t, EGLClientBuffer>  m_clientBufferMap;
   std::map<uint32_t, GLuint>           m_shaderMap;
   std::map<uint32_t, GLuint>           m_programMap;
   std::map<uint32_t, GLuint>           m_bufferMap;
   std::map<uint32_t, EGLSyncKHR>       m_eglSyncMap;
   std::map<uint32_t, EGLImageKHR>      m_eglImageMap;
#if V3D_TECH_VERSION == 3
   std::map<uint32_t, GLsync>           m_syncMap;
   std::map<uint32_t, GLuint>           m_queryMap;
   std::map<uint32_t, GLuint>           m_vertexArrayMap;
   std::map<uint32_t, GLuint>           m_samplerMap;
   std::map<uint32_t, GLuint>           m_tfMap;
   std::map<uint32_t, GLuint>           m_programPipelineMap;
#endif
   std::map<EGLSurface, WindowSize>     m_windowSizeMap;   // Maps surface id to viewport

   std::map<std::pair<uint32_t, uint32_t>, GLuint> m_uniformMap;
   std::map<std::pair<uint32_t, uint32_t>, GLuint> m_locationMap;

   std::map<uint32_t, bsg::NativePixmap *>         m_pixmaps;

   /* Keep track of which displays have been inited so we can cleanup properly
    * on exit -- we can't rely on the replay to do that. */
   std::set<EGLDisplay> m_initedDisplays;

   /* list of timing information */
   std::vector<timeprobe> m_timeprobes;
};


#endif /* __SPYTOOL_REPLAY_H__ */
